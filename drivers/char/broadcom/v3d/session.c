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
#include <linux/jiffies.h>
#include <linux/sched.h>
#include <linux/wait.h>

#include "driver.h"
#include "device.h"
#include "session.h"


/* ================================================================ */

v3d_session_t *v3d_session_create(v3d_driver_t *driver, const char *name)
{
	v3d_session_t *instance = (v3d_session_t *) kmalloc(sizeof(v3d_session_t), GFP_KERNEL);
	if (instance == NULL)
		return NULL;

	instance->initialised = 0;

	/* Unconditional initialisation */
	instance->driver = driver;
	instance->name   = name;
	instance->last_id = 0;
	spin_lock_init(&instance->issued.lock);
	INIT_LIST_HEAD(&instance->issued.list);
	instance->performance_counter.enables = 0;

	v3d_session_reset_statistics(instance);

	if (v3d_driver_add_session(instance->driver, instance) != 0)
		return v3d_session_delete(instance), NULL;
	++instance->initialised;

	/* Initialisation that can fail */
	return instance;
}

void v3d_session_delete(v3d_session_t *instance)
{
	switch (instance->initialised) {
	case 1:
		/* Wait for all our jobs to complete */
		v3d_session_wait(instance);

		/* Ensure that any exclusive lock is released */
		(void) v3d_driver_exclusive_stop(instance->driver, instance);

		v3d_driver_remove_session(instance->driver, instance);

	case 0:
		kfree(instance);
		break;
	}
}

void v3d_session_reset_statistics(v3d_session_t *instance)
{
	instance->start = ktime_get();
	instance->total_run = 0;
	statistics_initialise(&instance->bin_render.queue);
	statistics_initialise(&instance->bin_render.run);

	statistics_initialise(&instance->user.queue);
	statistics_initialise(&instance->user.run);

	statistics_initialise(&instance->binning_bytes);
}

void v3d_session_add_statistics(v3d_session_t *instance, int user, unsigned int queue, unsigned int run, unsigned int binning_bytes)
{
	instance->total_run += run;
	if (user != 0) {
		statistics_add(&instance->user.queue, queue);
		statistics_add(&instance->user.run,   run);
		statistics_add(&instance->binning_bytes, binning_bytes);
	} else {
		statistics_add(&instance->bin_render.queue, queue);
		statistics_add(&instance->bin_render.run,   run);
	}
}


/* ================================================================ */

int v3d_session_job_post(
	v3d_session_t *instance,
	const v3d_job_post_t *user_job)
{
	instance->last_id = user_job->job_id;
	return v3d_driver_job_post(instance->driver, instance, user_job);
}


/* ================================================================ */

void remove_job(struct kref *reference)
{
	v3d_driver_job_t *job = container_of(reference, v3d_driver_job_t, waiters);
	MY_ASSERT(job != NULL);
	list_del(&job->session.link);
}

void v3d_session_job_reference(struct v3d_driver_job_tag *job, const char *name)
{
	MY_ASSERT(job != NULL);
#ifdef VERBOSE_DEBUG
	printk(KERN_ERR "%s: j %p %s\n", __func__, job, name);
#endif
	kref_get(&job->waiters); /* Prevents freeing on completion */
}

void v3d_session_job_release(struct v3d_driver_job_tag *job, const char *name)
{
	v3d_session_t *instance = job->session.instance;
	unsigned long  flags;
	int            last;
	MY_ASSERT(job != NULL);
	MY_ASSERT(job->state != V3DDRIVER_JOB_INVALID);
#ifdef VERBOSE_DEBUG
	printk(KERN_ERR "%s: j %p %s\n", __func__, job, name);
#endif
	if (job->state > V3DDRIVER_JOB_ACTIVE)
		wake_up_all(&job->wait_for_completion);
	spin_lock_irqsave(&instance->issued.lock, flags);
	last = kref_put(&job->waiters, &remove_job);
	spin_unlock_irqrestore(&instance->issued.lock, flags);
	if (last != 0) {
#ifdef VERBOSE_DEBUG
		printk(KERN_ERR "%s: j %p %s - completing\n", __func__, job, name);
#endif
		v3d_driver_job_complete(instance->driver, job);
	}
}


/* ================================================================ */

void v3d_session_issued(v3d_driver_job_t *job)
{
	v3d_session_t *instance = job->session.instance;
	unsigned long   flags;
	MY_ASSERT(job != NULL);
	spin_lock_irqsave(&instance->issued.lock, flags);
	instance->last_id = (int32_t) job->user_job.job_id;
	job->state = V3DDRIVER_JOB_ISSUED;
	list_add_tail(&job->session.link, &instance->issued.list);
	spin_unlock_irqrestore(&instance->issued.lock, flags);
}

void v3d_session_complete(v3d_driver_job_t *job, int status)
{
	v3d_session_t *instance;
	unsigned long   flags;
	MY_ASSERT(job != NULL);
#ifdef VERBOSE_DEBUG
	printk(KERN_ERR "%s: j %p s %d\n", __func__, job, status);
#endif
	instance = job->session.instance;
	if (job->state < status) /* Preserve any previous failure */
		job->state = status;
	spin_lock_irqsave(&instance->issued.lock, flags);
	if (instance->performance_counter.enables != 0)
		v3d_device_counters_add(job->device, &instance->performance_counter.count[0]);
	spin_unlock_irqrestore(&instance->issued.lock, flags);
	v3d_session_job_release(job, __func__);
}

#if 0
static v3d_driver_job_t *find_job(v3d_session_t *instance, int32_t id)
{
	v3d_driver_job_t *job = NULL;
	struct list_head  *current;
	unsigned long      flags;
	spin_lock_irqsave(&instance->issued.lock, flags);
	list_for_each(current, &instance->issued.list) {
		job = list_entry(current, v3d_driver_job_t, session.link);
		if (id == (int32_t) job->user_job.job_id) {
			v3d_session_job_reference(job, __func__); /* Prevents freeing on completion */
			spin_unlock_irqrestore(&instance->issued.lock, flags);
			return job;
		}
	}
	spin_unlock_irqrestore(&instance->issued.lock, flags);
	return NULL;
}
#endif

#if 0
int wait_with_timeout(v3d_driver_job_t *job)
{
	long ret = msecs_to_jiffies(JOB_TIMEOUT_MS);
	if (job->state != V3DDRIVER_JOB_COMPLETE) {
		DEFINE_WAIT(wait);
		for (;;) {
			prepare_to_wait(&job->wait_for_completion, &wait, TASK_UNINTERRUPTIBLE);
			if (job->state == V3DDRIVER_JOB_COMPLETE)
				break;
			ret = schedule_timeout(ret);
			if (!ret)
				break;
			if (job->state != V3DDRIVER_JOB_COMPLETE)
				printk(KERN_ERR "%s: woken j %p s %d\n", __func__, job, job->state);
		}
		finish_wait(&job->wait_for_completion, &wait);
	}
	return ret;
}
#endif

static void wait_job(v3d_session_t *instance, v3d_driver_job_t *job)
{
	int remaining_time;
	MY_ASSERT(job != NULL);
	remaining_time = wait_event_timeout(job->wait_for_completion, job->state >= V3DDRIVER_JOB_COMPLETE, msecs_to_jiffies(JOB_TIMEOUT_MS));
	if (remaining_time == 0) {
		printk(KERN_ERR "V3D job %p timed-out in state %d, active %p", job, job->state, job->device->in_progress.job);
		v3d_device_job_cancel(job->device, job, 1);
	} else
		v3d_session_job_release(job, __func__); /* Remove waiting reference */
}

int32_t v3d_session_wait(v3d_session_t *instance)
{
	int32_t       id = instance->last_id;
	unsigned long flags;
#if 0
	/* Find the last ID issued */
	v3d_driver_job_t *job = find_job(instance, id);
	if (job != NULL) {
		/* Wait on this one first - will minimise the number of waits */
		wait_job(instance, job);
	}
	return id;
#else
	/* Wait on all previous jobs, if any - only required when multiple V3D blocks exist */
	do {
		v3d_driver_job_t *job;
		spin_lock_irqsave(&instance->issued.lock, flags);
		if (list_empty(&instance->issued.list) != 0)
			break;

		job = list_first_entry(&instance->issued.list, v3d_driver_job_t, session.link);
		if (id - (int32_t) job->user_job.job_id < 0)
			break;

		v3d_session_job_reference(job, __func__); /* Prevents freeing on completion */
		spin_unlock_irqrestore(&instance->issued.lock, flags);

		wait_job(instance, job);
  #if 0
		printk(KERN_ERR "%s: job %p:%08x / %08x complete\n",
			__func__,
			job,
			(int32_t) job->user_job.job_id, id);
  #endif
	} while (1);
	spin_unlock_irqrestore(&instance->issued.lock, flags);
#endif
	return id;
}

int v3d_session_counters_enable(v3d_session_t *instance, uint32_t enables)
{
	if (instance->performance_counter.enables != 0)
		return -1;
	instance->performance_counter.enables = enables;
	memset(
		&instance->performance_counter.count[0],
		0,
		sizeof(instance->performance_counter.count));
	v3d_driver_counters_enable(instance->driver, enables);
	return 0;
}

int v3d_session_counters_disable(v3d_session_t *instance)
{
	if (instance->performance_counter.enables == 0)
		return -1;
	instance->performance_counter.enables = 0;
	return 0;
}

const uint32_t *v3d_session_counters_get(v3d_session_t *instance)
{
	return instance->performance_counter.enables == 0 ? &instance->performance_counter.count[0] : NULL;
}
