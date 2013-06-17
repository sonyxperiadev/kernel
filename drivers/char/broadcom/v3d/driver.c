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

#include <linux/slab.h> /* kmalloc, kfree */
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/ktime.h>
#include <linux/hrtimer.h>
#include <asm/uaccess.h>
#include "session.h"
#include "device.h"
#include "driver.h"


/* ================================================================ */

void v3d_driver_delete(v3d_driver_t *instance)
{
	switch (instance->initialised) {
	case 1:
		v3d_driver_delete_proc_entries(instance);

	case 0:
		kfree(instance);
		break;
	}
}

void v3d_driver_reset_statistics(v3d_driver_t *instance)
{
	statistics_initialise(&instance->bin_render.queue);
	statistics_initialise(&instance->bin_render.run);
	statistics_initialise(&instance->bin_render.binning_bytes);
	statistics_initialise(&instance->user.queue);
	statistics_initialise(&instance->user.run);
	instance->total_run = 0;
	instance->start = ktime_get();
}

v3d_driver_t *v3d_driver_create(void)
{
	v3d_driver_t *instance = kmalloc(sizeof(v3d_driver_t), GFP_KERNEL);
	if (instance == NULL)
		return NULL;
	instance->initialised = 0;

	/* Unconditional initialisation */
	spin_lock_init(&instance->job.free.lock);
	spin_lock_init(&instance->job.posted.lock);

	{
		unsigned int i;
		INIT_LIST_HEAD(&instance->job.free.list);
		for (i = 0 ; i < sizeof(instance->job.free_jobs) / sizeof(instance->job.free_jobs[0]) ; ++i)
			list_add_tail(&instance->job.free_jobs[i].link, &instance->job.free.list);
		sema_init(&instance->job.free.count, i);
	}
	INIT_LIST_HEAD(&instance->job.posted.user);
	INIT_LIST_HEAD(&instance->job.posted.bin_render);
	INIT_LIST_HEAD(&instance->job.posted.exclusive.user);
	INIT_LIST_HEAD(&instance->job.posted.exclusive.bin_render);
	mutex_init(&instance->job.posted.exclusive.lock);
	instance->job.posted.exclusive.owner = NULL;

	instance->device = NULL;

	v3d_driver_reset_statistics(instance);

	memset(&instance->sessions, 0, sizeof(instance->sessions));

	/* Stuff that can fail */
	if (v3d_driver_create_proc_entries(instance) != 0)
		return v3d_driver_delete(instance), NULL;
	++instance->initialised;

	return instance;
}


/* ================================================================ */

static v3d_session_t **get_session_entry(v3d_driver_t *instance, v3d_session_t *session)
{
	unsigned int i;
	for (i = 0 ; i < sizeof(instance->sessions) / sizeof(instance->sessions[0]) ; ++i)
		if (instance->sessions[i] == session)
			return &instance->sessions[i];
	return NULL;
}

/* TODO: Lock - potentially in the caller */
int v3d_driver_add_session(v3d_driver_t *instance, struct v3d_session_tag *session)
{
	v3d_session_t **entry = get_session_entry(instance, NULL);
	if (entry == NULL)
		return -ENOMEM;
	*entry = session;
	return 0;
}

void v3d_driver_remove_session(v3d_driver_t *instance, struct v3d_session_tag *session)
{
	v3d_session_t **entry = get_session_entry(instance, session);
	MY_ASSERT(session != NULL);
	*entry = NULL;
}


/* ================================================================ */

static v3d_driver_job_t *remove_head(struct list_head *list)
{
	v3d_driver_job_t *job;
	if (list_empty(list) != 0)
		return NULL;
	job = list_first_entry(list, v3d_driver_job_t, link);
	list_del(&job->link);
	return job;
}

/* Requires Instance->Job.Posted.Lock to be held */
v3d_driver_job_t *v3d_driver_job_get(v3d_driver_t *instance, unsigned int required)
{
	v3d_driver_job_t *job = NULL;
	int                exclusive;

	/* See if we've completed everything queued */
	/* when the exclusive lock was requested */
	exclusive = instance->job.posted.exclusive.owner != NULL
		&& instance->job.posted.exclusive.bin_render_count == 0
		&& instance->job.posted.exclusive.user_count      == 0;

	if ((required & V3D_JOB_USER) != 0)
		job = remove_head(exclusive != 0 ? &instance->job.posted.exclusive.user      : &instance->job.posted.user);
	if (job == NULL && (required & V3D_JOB_BIN_REND) != 0)
		job = remove_head(exclusive != 0 ? &instance->job.posted.exclusive.bin_render : &instance->job.posted.bin_render);

	/* Update counts for switching to exclusive queues */
	if (job != NULL && instance->job.posted.exclusive.owner != NULL && exclusive == 0) {
		if (job->user_job.job_type == V3D_JOB_USER)
			--instance->job.posted.exclusive.user_count;
		else
			--instance->job.posted.exclusive.bin_render_count;
	}
	return job;
}

void v3d_driver_job_complete(v3d_driver_t *instance, v3d_driver_job_t *job)
{
	/* Return the job to Free.List */
	unsigned long flags;
	MY_ASSERT(job != NULL);
	job->end = ktime_get();
	spin_lock_irqsave(&instance->job.posted.lock, flags);
	if (job->state == V3DDRIVER_JOB_INITIALISED) {
		/* Hasn't yet made it to the hardware, so needs dequeueing */
		/* Note: Even if a job is completed by both a time-out and */
		/*       normal completion, the ref count only hits zero   */
		/*       once, so we only complete once                    */
		list_del(&job->link);
	}

	if (job->state > V3DDRIVER_JOB_ACTIVE) {
		unsigned int queue = ktime_us_delta(job->start, job->queued);
		unsigned int run   = ktime_us_delta(job->end,   job->start);
		instance->total_run += run;
		if (job->user_job.job_type == V3D_JOB_USER) {
			statistics_add(&instance->user.queue, queue);
			statistics_add(&instance->user.run,   run);
			v3d_session_add_statistics(job->session.instance, 1 /* User? */, queue, run, 0U);
		} else {
			statistics_add(&instance->bin_render.queue, queue);
			statistics_add(&instance->bin_render.run,   run);
			statistics_add(&instance->bin_render.binning_bytes, job->binning_bytes);
			v3d_session_add_statistics(job->session.instance, 0 /* User? */, queue, run, job->binning_bytes);
		}
	}
	spin_unlock_irqrestore(&instance->job.posted.lock, flags);

	spin_lock_irqsave(&instance->job.free.lock, flags);
	list_add_tail(&job->link, &instance->job.free.list);
#if 1
	job->state = V3DDRIVER_JOB_INVALID;
#endif
	spin_unlock_irqrestore(&instance->job.free.lock, flags);
	up(&instance->job.free.count);
}


/* ================================================================ */

void v3d_driver_exclusive_start(v3d_driver_t *instance, struct v3d_session_tag *session)
{
	unsigned long     flags;
	mutex_lock(&instance->job.posted.exclusive.lock);
	MY_ASSERT(instance->job.posted.exclusive.owner == NULL);
	instance->job.posted.exclusive.owner = session;

	/* Count the number of jobs outstanding */
	instance->job.posted.exclusive.bin_render_count = 0;
	instance->job.posted.exclusive.user_count      = 0;
	flags = v3d_driver_job_lock(instance);
#if 0
	{
		struct list_head *current;
		list_for_each(current, &instance->job.posted.bin_render)
			++instance->job.posted.exclusive.bin_render_count;
		list_for_each(current, &instance->job.posted.user)
			++instance->job.posted.exclusive.user_count;
	}
#endif
	v3d_driver_job_unlock(instance, flags);
}

int v3d_driver_exclusive_stop(v3d_driver_t *instance, struct v3d_session_tag *session)
{
	unsigned long flags;
	int           empty;
	if (instance->job.posted.exclusive.owner != session)
		return -EPERM;

	flags = v3d_driver_job_lock(instance);
	empty = list_empty(&instance->job.posted.exclusive.user) != 0
		&& list_empty(&instance->job.posted.exclusive.bin_render) != 0;
	v3d_driver_job_unlock(instance, flags);
	if (empty == 0)
		return -EBUSY;

	instance->job.posted.exclusive.owner = NULL;
	mutex_unlock(&instance->job.posted.exclusive.lock);

	/* We'll be Idle now, but the normal lists may not be empty */
	v3d_device_job_posted(instance->device);
#if 0
	{
		unsigned int i;
		for (i = 0 ; i < instance->job.time_index ; ++i) {
			printk(KERN_ERR "j %2u q %9u r %9u\n",
				i, instance->job.times[i].queue, instance->job.times[i].run);
		}
	}
#endif
	return 0;
}

int v3d_driver_job_post(
	v3d_driver_t        *instance,
	v3d_session_t       *session,
	const v3d_job_post_t *user_job)
{
	v3d_driver_job_t     *job;
	unsigned long      flags;
	struct list_head  *list;
	if (session == instance->job.posted.exclusive.owner)
		list = user_job->job_type == V3D_JOB_USER ? &instance->job.posted.exclusive.user : &instance->job.posted.exclusive.bin_render;
	else
		list = user_job->job_type == V3D_JOB_USER ? &instance->job.posted.user           : &instance->job.posted.bin_render;

	if (list_empty(&instance->job.free.list))
		printk(KERN_ERR "%s: free list empty!\n", __func__);

	/* Get a free job structure */
	down(&instance->job.free.count);
	spin_lock_irqsave(&instance->job.free.lock, flags);
	job = list_first_entry(&instance->job.free.list, v3d_driver_job_t, link);
	list_del(&job->link);
	spin_unlock_irqrestore(&instance->job.free.lock, flags);

	/* Initialise and copy-in the user-side job information */
	init_waitqueue_head(&job->wait_for_completion);
	kref_init(&job->waiters); /* Starts at 1 - Completion reference */
	job->state = V3DDRIVER_JOB_INVALID;
	job->queued = job->start = ktime_get();
	job->binning_bytes = 0U;

	if ((uint32_t) user_job < PAGE_OFFSET) {
		int status = copy_from_user(&job->user_job, user_job, sizeof(job->user_job));
		if (status != 0) {
			printk(KERN_ERR "%s: j %p failed copy", __func__, job);
			v3d_driver_job_complete(instance, job);
			return status;
		}
	} else
		job->user_job = *user_job;
	job->state = V3DDRIVER_JOB_INITIALISED;
	job->session.instance = session;
	v3d_session_issued(job);

	/* Queue it */
	spin_lock_irqsave(&instance->job.posted.lock, flags);
	list_add_tail(&job->link, list);
	spin_unlock_irqrestore(&instance->job.posted.lock, flags);

	/* Kick consumer(s) */
	v3d_driver_kick_consumers(instance);
	return 0;
}


/* ================================================================ */
/* The following functions are hard-coded for a single V3D block    */

void v3d_driver_add_device(v3d_driver_t *instance, struct v3d_device_tag *device)
{
	MY_ASSERT(instance->device == NULL);
	instance->device = device;
}

void v3d_driver_kick_consumers(v3d_driver_t *instance)
{
	v3d_device_job_posted(instance->device);
}

void v3d_driver_counters_enable(v3d_driver_t *instance, uint32_t enables)
{
	v3d_device_counters_enable(instance->device, enables);
}

void v3d_driver_counters_disable(v3d_driver_t *instance)
{
	v3d_device_counters_disable(instance->device);
}
