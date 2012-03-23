/*******************************************************************************
Copyright 2012 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#include <linux/kernel.h>

#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/semaphore.h>
#include <linux/slab.h>

#include <linux/broadcom/vce.h>
#include <linux/broadcom/vtq.h>
#include "vtqbr.h"

/* a proxy */

/* take the mutex in the context for any creation/destruction
 * operation */

struct vtqb_image {
	vtq_image_id_t real_image;
	struct vtqb_image *next;
	struct vtqb_image **prevnext;
};

struct vtqb_task {
	vtq_task_id_t real_task;
	struct vtqb_task *next;
	struct vtqb_task **prevnext;
};

struct vtqb_context {
	struct mutex mutex;
	int cleaned;
	struct vtqb_image *imagelist;
	struct vtqb_task *tasklist;
	struct vtq_context *real_context;
	/* TODO: lose these arrays */
	struct vtqb_image *imagemap[VTQ_MAX_IMAGES];
	/* TODO: lose these arrays */
	struct vtqb_task *taskmap[VTQ_MAX_TASKS];
	vtq_job_id_t last_queued_job_id;
};

#define err_print(fmt, arg...) \
	printk(KERN_ERR "VTQ: ERROR: %s() %s:%d " fmt, \
		__func__, "vtqbr.c", __LINE__, ##arg)
#define dbg_print(fmt, arg...) \
	printk(KERN_WARNING "VTQ: debug: %s() %s:%d " fmt, \
		__func__, "vtqbr.c", __LINE__, ##arg)

struct vtqb_context *vtqb_create_context(struct vtq_vce *vtq_pervce_state)
{
	struct vtq_context *ctx;
	struct vtqb_context *wrapper;

	ctx = vtq_create_context(vtq_pervce_state);
	if (ctx == NULL) {
		err_print("failed to create VTQ context\n");
		goto err_create_context;
	}

	wrapper = kmalloc(sizeof(*wrapper), GFP_KERNEL);
	if (wrapper == NULL) {
		err_print("failed to create VTQ context\n");
		goto err_kmalloc;
	}

	wrapper->real_context = ctx;
	mutex_init(&wrapper->mutex);
	wrapper->tasklist = NULL;
	wrapper->imagelist = NULL;
	wrapper->cleaned = 0;
	/* It's not strictly correct to memset these to zero, but I
	 * won't fix it because these arrays are going to be
	 * deprecated soon anyway */
	memset(&wrapper->taskmap[0], 0, sizeof(wrapper->taskmap));
	memset(&wrapper->imagemap[0], 0, sizeof(wrapper->imagemap));
	wrapper->last_queued_job_id = vtq_get_read_pointer(ctx);

	/* success */
	return wrapper;

	/* error exit paths follow */

	/* kfree(wrapper); */
err_kmalloc:

	vtq_destroy_context(ctx);
err_create_context:

	return NULL;
}

void vtqb_destroy_context(struct vtqb_context *ctx)
{
	int s;
	int distance;

	mutex_lock(&ctx->mutex);
	ctx->cleaned = 1;
	mutex_unlock(&ctx->mutex);

	distance = ctx->last_queued_job_id
		- vtq_get_read_pointer(ctx->real_context);
	while (distance > 0) {
		dbg_print("Cleaning up VTQ context after lazy or buggy app -- "
			  "%d jobs are still outstanding\n", distance);
		s = vtq_await_job(ctx->real_context, ctx->last_queued_job_id);
		if (s != 0) {
			err_print("Failed to wait during context destruction.  "
				  "This is probably fatal.  BUG!  FIXME\n");
			dbg_print("will try again shortly\n");
			s = msleep_interruptible(500);
			if (s != 0) {
				err_print("No choice but to leak\n");
				return;
			}
		}
		distance = ctx->last_queued_job_id
			- vtq_get_read_pointer(ctx->real_context);
	}

	while (ctx->tasklist) {
		struct vtqb_task *next;
		next = ctx->tasklist->next;
		dbg_print("Cleaning up left over task %d\n",
			ctx->tasklist->real_task);
		vtq_destroy_task(ctx->real_context,
			ctx->tasklist->real_task);
		kfree(ctx->tasklist);
		ctx->tasklist = next;
	}

	while (ctx->imagelist) {
		struct vtqb_image *next;
		next = ctx->imagelist->next;
		dbg_print("Cleaning up left over image %d\n",
			ctx->imagelist->real_image);
		vtq_unregister_image(ctx->real_context,
			ctx->imagelist->real_image);
		kfree(ctx->imagelist);
		ctx->imagelist = next;
	}

	vtq_destroy_context(ctx->real_context);
	kfree(ctx);
}


/* **************** */

/* job loader -- copied from testloader */

vtq_image_id_t vtqb_register_image(struct vtqb_context *ctx,
				   const uint32_t *text,
				   size_t textsz,
				   const uint32_t *data,
				   size_t datasz,
				   size_t datamemreq)
{
	vtq_image_id_t image_id;
	struct vtqb_image *proxy;

	image_id = vtq_register_image(ctx->real_context,
				      text, textsz,
				      data, datasz,
				      datamemreq);
	if (image_id < 0) {
		err_print("failed to register image\n");
		goto err_register_image;
	}

	proxy = kmalloc(sizeof(*proxy), GFP_KERNEL);
	if (proxy == NULL) {
		err_print("kmalloc failure\n");
		goto err_kmalloc;
	}

	mutex_lock(&ctx->mutex);
	if (ctx->cleaned) {
		err_print("attempt to register image in closing context\n");
		goto err_cleaned;
	}

	if (ctx->imagemap[image_id] != NULL) {
		err_print("image id already taken!  BUG!\n");
		goto err_badimage;
	}

	proxy->real_image = image_id;
	proxy->next = ctx->imagelist;
	if (proxy->next != NULL)
		proxy->next->prevnext = &proxy->next;
	proxy->prevnext = &ctx->imagelist;
	ctx->imagelist = proxy;

	ctx->imagemap[image_id] = proxy;

	mutex_unlock(&ctx->mutex);

	/* success */

	BUG_ON(image_id < 0);
	return image_id;

	/*
	  error exit paths follow
	*/

err_badimage:
err_cleaned:

	mutex_unlock(&ctx->mutex);

	kfree(proxy);
err_kmalloc:

	vtq_unregister_image(ctx->real_context, image_id);
err_register_image:

	dbg_print("image registration failed\n");
	return -1;
}

void vtqb_unregister_image(struct vtqb_context *ctx, vtq_image_id_t image_id)
{
	struct vtqb_image *proxy;

	mutex_lock(&ctx->mutex);
	if (ctx->cleaned) {
		err_print("attempt to unregister image in closing context\n");
		return;
	}

	if (image_id < 0 || image_id >= VTQ_MAX_TASKS) {
		/* must be bad data from user space -- just ignore it */
		mutex_unlock(&ctx->mutex);
		err_print("bad image id");
		return;
	}

	proxy = ctx->imagemap[image_id];

	if (proxy == NULL) {
		/* must be bad data from user space -- just ignore it */
		mutex_unlock(&ctx->mutex);
		err_print("badd image id");
		return;
	}

	ctx->imagemap[image_id] = NULL;
	if (proxy->next != NULL)
		proxy->next->prevnext = proxy->prevnext;
	*proxy->prevnext = proxy->next;
	mutex_unlock(&ctx->mutex);

	kfree(proxy);

	vtq_unregister_image(ctx->real_context, image_id);
}

vtq_task_id_t vtqb_create_task(struct vtqb_context *ctx,
			       vtq_image_id_t image_id,
			       uint32_t entrypt)
{
	vtq_task_id_t task_id;
	struct vtqb_task *proxy;

	if (image_id < 0 || image_id > VTQ_MAX_IMAGES) {
		dbg_print("bad image ID\n");
		goto err_badimage1;
	}

	mutex_lock(&ctx->mutex);
	if (ctx->cleaned) {
		dbg_print("attempt to create task in closing context\n");
		goto err_cleaned;
	}

	/* Check whether caller attempts to create a task for an image
	 * he does not own */
	if (ctx->imagemap[image_id] == NULL) {
		dbg_print("bad image ID\n");
		goto err_badimage2;
	}

	task_id = vtq_create_task(ctx->real_context, image_id, entrypt);
	if (task_id < 0) {
		err_print("failed to create task\n");
		goto err_create_task;
	}

	proxy = kmalloc(sizeof(*proxy), GFP_KERNEL);
	if (proxy == NULL) {
		err_print("failed to create task proxy object\n");
		goto err_kmalloc;
	}


	if (ctx->taskmap[task_id] != NULL) {
		err_print("already got entry for this task -- "
			  "something went wrong!\n");
		goto err_badtask;
	}

	proxy->real_task = task_id;
	proxy->next = ctx->tasklist;
	if (proxy->next != NULL)
		proxy->next->prevnext = &proxy->next;
	proxy->prevnext = &ctx->tasklist;
	ctx->tasklist = proxy;

	ctx->taskmap[task_id] = proxy;

	mutex_unlock(&ctx->mutex);

	/* success */

	BUG_ON(task_id < 0);
	return task_id;

	/*
	  error exit paths follow
	*/

err_badtask:

	kfree(proxy);
err_kmalloc:

	vtq_destroy_task(ctx->real_context, task_id);
err_create_task:

err_badimage2:

err_cleaned:

	mutex_unlock(&ctx->mutex);

err_badimage1:

	dbg_print("task creation failed\n");

	return -1;
}

void vtqb_destroy_task(struct vtqb_context *ctx, vtq_task_id_t task_id)
{
	struct vtqb_task *proxy;

	mutex_lock(&ctx->mutex);
	if (ctx->cleaned) {
		dbg_print("should have destroyed the tasks first\n");
		mutex_unlock(&ctx->mutex);
		return;
	}

	if (task_id < 0 || task_id >= VTQ_MAX_TASKS) {
		/* must be bad data from user space -- just ignore it */
		dbg_print("bad task id\n");
		mutex_unlock(&ctx->mutex);
		return;
	}

	proxy = ctx->taskmap[task_id];

	if (proxy == NULL) {
		/* must be bad data from user space -- just ignore it */
		dbg_print("bad task id\n");
		mutex_unlock(&ctx->mutex);
		return;
	}

	ctx->taskmap[task_id] = NULL;
	if (proxy->next != NULL)
		proxy->next->prevnext = proxy->prevnext;
	*proxy->prevnext = proxy->next;
	mutex_unlock(&ctx->mutex);

	/* printk(KERN_ERR "TODO! Need to wait for job %u to complete "
	   "(if it has not already)\n", ctx->last_queued_job_id); */
	/* argh! we're recording queued jobs against context not task.
	 * We shouldn't wait for _other_ tasks' jobs to complete.  We
	 * have to fix that in order to protect against malicious
	 * apps.  At the moment, I think this is benign, but if we
	 * start having callbacks or such in the future, this may
	 * become an issue */

	kfree(proxy);

	vtq_destroy_task(ctx->real_context, task_id);
}

/* job */

int vtqb_queue_job(struct vtqb_context *ctx,
		vtq_task_id_t task_id,
		vtq_job_arg_t arg0,
		vtq_job_arg_t arg1,
		vtq_job_arg_t arg2,
		vtq_job_arg_t arg3,
		vtq_job_arg_t arg4,
		vtq_job_arg_t arg5,
		uint32_t flags_do_not_use_me,
		vtq_job_id_t *job_id_out)
{
	int s;
	vtq_job_id_t job_id;
	int distance;

	if (task_id < 0 || task_id >= VTQ_MAX_TASKS) {
		dbg_print("bad task ID\n");
		goto err_badtask1;
	}

	mutex_lock(&ctx->mutex);
	if (ctx->cleaned) {
		dbg_print("attempt to queue job on closing context\n");
		goto err_cleaned;
	}

	/* Check we're not being asked to queue a task they don't own */
	if (ctx->taskmap[task_id] == NULL) {
		dbg_print("bad task ID\n");
		goto err_badtask2;
	}

	s = vtq_queue_job(ctx->real_context,
		task_id, arg0, arg1, arg2,
		arg3, arg4, arg5, flags_do_not_use_me, &job_id);
	if (s != 0) {
		dbg_print("failed to queue the job\n");
		goto err_queue_job;
	}

	distance = job_id - ctx->last_queued_job_id;
	if (distance > 0)
		ctx->last_queued_job_id = job_id;
	/* TODO: need to track per-task too, for safe deletion of
	 * tasks -- benign at the moment, however */

	mutex_unlock(&ctx->mutex);

	/* success */

	if (job_id_out != NULL)
		*job_id_out = job_id;

	return 0;

	/*
	  error exit paths follow
	*/

err_queue_job:
err_badtask2:
err_cleaned:

	mutex_unlock(&ctx->mutex);

err_badtask1:

	return -1;
}

int vtqb_await_job(struct vtqb_context *ctx, vtq_job_id_t job)
{
	int distance;
	int s;

	distance = job - ctx->last_queued_job_id;
	if (distance > 0) {
		/* ignore bad user info */
		dbg_print("bad job id %u from caller (last_queued is %u)\n",
			  job, job - distance);
		return -1;
	}

	s = vtq_await_job(ctx->real_context, job);

	return s;
}
