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

v3d_session_type *v3d_session_create(v3d_driver_type *driver, const char *name)
{
	v3d_session_type *instance = (v3d_session_type *) kmalloc(sizeof(v3d_session_type), GFP_KERNEL);
	if (instance == NULL)
		return NULL;

	instance->initialised = 0;

	/* Unconditional initialisation */
	instance->driver = driver;
	instance->name   = name;
	instance->last_id = 0;
	spin_lock_init(&instance->issued.lock);
	INIT_LIST_HEAD(&instance->issued.list);

	v3d_session_reset_statistics(instance);

	if (v3d_driver_add_session(instance->driver, instance) != 0)
		return v3d_session_delete(instance), NULL;
	++instance->initialised;

	/* Initialisation that can fail */
	return instance;
}

void v3d_session_delete(v3d_session_type *instance)
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

void v3d_session_reset_statistics(v3d_session_type *instance)
{
	instance->start = ktime_get();
	instance->total_run = 0;
	statistics_initialise(&instance->bin_render.queue);
	statistics_initialise(&instance->bin_render.run);

	statistics_initialise(&instance->user.queue);
	statistics_initialise(&instance->user.run);
}

void v3d_session_add_statistics(v3d_session_type *instance, int user, unsigned int queue, unsigned int run)
{
	instance->total_run += run;
	if (user != 0) {
		statistics_add(&instance->user.queue, queue);
		statistics_add(&instance->user.run,   run);
	} else {
		statistics_add(&instance->bin_render.queue, queue);
		statistics_add(&instance->bin_render.run,   run);
	}
}


/* ================================================================ */

int v3d_session_job_post(
	v3d_session_type *instance,
	const v3d_job_post_t *user_job)
{
	instance->last_id = user_job->job_id;
	return v3d_driver_job_post(instance->driver, instance, user_job);
}

void remove_job(struct kref *reference)
{
	v3d_driver_job_type *job = container_of(reference, v3d_driver_job_type, waiters);
	BUG_ON(job == NULL);
	list_del(&job->session.link);
}

void v3d_session_issued(v3d_driver_job_type *job)
{
	v3d_session_type *instance = job->session.instance;
	unsigned long   flags;
	BUG_ON(job == NULL);
	spin_lock_irqsave(&instance->issued.lock, flags);
	instance->last_id = (int32_t) job->user_job.job_id;
	job->state = V3DDRIVER_JOB_ISSUED;
	list_add_tail(&job->session.link, &instance->issued.list);
	spin_unlock_irqrestore(&instance->issued.lock, flags);
}

void v3d_session_complete(v3d_driver_job_type *job, int status)
{
	v3d_session_type *instance;
	unsigned long   flags;
	int             last;
	BUG_ON(job == NULL);
	instance = job->session.instance;
	job->state = status;
	spin_lock_irqsave(&instance->issued.lock, flags);
	last = kref_put(&job->waiters, &remove_job);
	spin_unlock_irqrestore(&instance->issued.lock, flags);
	if (last != 0)
		v3d_driver_job_complete(instance->driver, job);
	else
		wake_up_all(&job->wait_for_completion);
}

#if 0
static v3d_driver_job_type *find_job(v3d_session_type *instance, int32_t id)
{
	v3d_driver_job_type *job = NULL;
	struct list_head  *current;
	unsigned long      flags;
	spin_lock_irqsave(&instance->issued.lock, flags);
	list_for_each(current, &instance->issued.list) {
		job = list_entry(current, v3d_driver_job_type, session.link);
		if (id == (int32_t) job->user_job.job_id) {
			kref_get(&job->waiters); /* Prevents freeing on completion */
			spin_unlock_irqrestore(&instance->issued.lock, flags);
			return job;
		}
	}
	spin_unlock_irqrestore(&instance->issued.lock, flags);
	return NULL;
}
#endif

#if 0
int wait_with_timeout(v3d_driver_job_type *job)
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

static void wait_job(v3d_session_type *instance, v3d_driver_job_type *job)
{
	int last;
	unsigned long flags;
	int remaining_time = wait_event_timeout(job->wait_for_completion, job->state >= V3DDRIVER_JOB_COMPLETE, msecs_to_jiffies(JOB_TIMEOUT_MS));
	spin_lock_irqsave(&instance->issued.lock, flags);
	if (remaining_time == 0) {
		(void) kref_put(&job->waiters, &remove_job); /* Remove completion reference as we timed-out */
		printk(KERN_ERR "V3D job %p timed-out", job);
	}
	last = kref_put(&job->waiters, &remove_job);
	spin_unlock_irqrestore(&instance->issued.lock, flags);

	if (last != 0) {
		if (remaining_time == 0) {
			/* Timed-out - V3dDevice needs to be ready */
			/* for a new job */
			v3d_device_job_cancel(job->device);
			v3d_driver_kick_consumers(instance->driver);
		} else
			v3d_driver_job_complete(instance->driver, job);
	}
}

int32_t v3d_session_wait(v3d_session_type *instance)
{
	int32_t       id = instance->last_id;
#if 0
	/* Find the last ID issued */
	v3d_driver_job_type *job = find_job(instance, id);
	if (job != NULL) {
		/* Wait on this one first - will minimise the number of waits */
		wait_job(instance, job);
	}
	return id;
#else
	unsigned long flags;
	#if 0
	{
		unsigned int entries = 0;
		struct list_head  *current;
		spin_lock_irqsave(&instance->issued.lock, flags);
		list_for_each(current, &instance->issued.list)
			++entries;

		if (entries > 2) {
			printk("%s: %u entries\n", __func__, entries);
			list_for_each(current, &instance->issued.list) {
				v3d_driver_job_type *job = list_entry(current, v3d_driver_job_type, session.link);
				printk(KERN_ERR "  l %p j %p t %d\n", current, job, job->user_job.job_type);
			}
		}
		spin_unlock_irqrestore(&instance->issued.lock, flags);
	}
	#endif

	/* Wait on all previous jobs, if any - only required when multiple V3D blocks exist */
	do {
		v3d_driver_job_type *job;
		spin_lock_irqsave(&instance->issued.lock, flags);
		if (list_empty(&instance->issued.list) != 0)
			break;

		job = list_first_entry(&instance->issued.list, v3d_driver_job_type, session.link);
		if (id - (int32_t) job->user_job.job_id < 0)
			break;

		kref_get(&job->waiters); /* Prevents freeing on completion */
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
