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

#include "Session.h"


/* ================================================================ */

V3dSessionType *V3dSession_Create(V3dDriverType *Driver)
{
	V3dSessionType *Instance = (V3dSessionType *) kmalloc(sizeof(V3dSessionType), GFP_KERNEL);
	if (Instance == NULL)
		return NULL;

	Instance->Initialised = 0;

	/* Unconditional initialisation */
	Instance->Driver = Driver;
	Instance->LastId = 0;
	spin_lock_init(&Instance->Issued.Lock);
	INIT_LIST_HEAD(&Instance->Issued.List);

	/* Initialisation that can fail */
	return Instance;
}

void V3dSession_Delete(V3dSessionType *Instance)
{
	switch (Instance->Initialised) {
	case 0:
		kfree(Instance);
		break;
	}
}


/* ================================================================ */

void RemoveJob(struct kref *Reference)
{
	V3dDriver_JobType *Job = container_of(Reference, V3dDriver_JobType, Waiters);
	BUG_ON(Job == NULL);
	list_del(&Job->Session.Link);
}

void V3dSession_Issued(V3dDriver_JobType *Job)
{
	V3dSessionType *Instance = Job->Session.Instance;
	unsigned long   Flags;
	BUG_ON(Job == NULL);
	spin_lock_irqsave(&Instance->Issued.Lock, Flags);
	Instance->LastId = (int32_t) Job->UserJob.job_id;
	Job->State = V3DDRIVER_JOB_ISSUED;
	list_add_tail(&Job->Session.Link, &Instance->Issued.List);
	spin_unlock_irqrestore(&Instance->Issued.Lock, Flags);
}

void V3dSession_Complete(V3dDriver_JobType *Job)
{
	V3dSessionType *Instance;
	unsigned long   Flags;
	int             Last;
	BUG_ON(Job == NULL);
	Instance = Job->Session.Instance;
	Job->State = V3DDRIVER_JOB_COMPLETE;
	spin_lock_irqsave(&Instance->Issued.Lock, Flags);
	Last = kref_put(&Job->Waiters, &RemoveJob);
	spin_unlock_irqrestore(&Instance->Issued.Lock, Flags);
	if (Last != 0)
		V3dDriver_JobComplete(Instance->Driver, Job);
	else
		wake_up_all(&Job->WaitForCompletion);
}

#if 0
static V3dDriver_JobType *FindJob(V3dSessionType *Instance, int32_t Id)
{
	V3dDriver_JobType *Job = NULL;
	struct list_head  *Current;
	unsigned long      Flags;
	spin_lock_irqsave(&Instance->Issued.Lock, Flags);
	list_for_each(Current, &Instance->Issued.List) {
		Job = list_entry(Current, V3dDriver_JobType, Session.Link);
		if (Id == (int32_t) Job->UserJob.job_id) {
			kref_get(&Job->Waiters); /* Prevents freeing on completion */
			spin_unlock_irqrestore(&Instance->Issued.Lock, Flags);
			return Job;
		}
	}
	spin_unlock_irqrestore(&Instance->Issued.Lock, Flags);
	return NULL;
}
#endif

#if 0
int WaitWithTimeout(V3dDriver_JobType *Job)
{
	long ret = msecs_to_jiffies(JOB_TIMEOUT_MS);
	if (Job->State != V3DDRIVER_JOB_COMPLETE) {
		DEFINE_WAIT(Wait);
		for (;;) {
			prepare_to_wait(&Job->WaitForCompletion, &Wait, TASK_UNINTERRUPTIBLE);
			if (Job->State == V3DDRIVER_JOB_COMPLETE)
				break;
			ret = schedule_timeout(ret);
			if (!ret)
				break;
			if (Job->State != V3DDRIVER_JOB_COMPLETE)
				printk(KERN_ERR "%s: Woken j %p s %d\n", __func__, Job, Job->State);
		}
		finish_wait(&Job->WaitForCompletion, &Wait);
	}
	return ret;
}
#endif

static void WaitJob(V3dSessionType *Instance, V3dDriver_JobType *Job)
{
	int Last;
	unsigned long Flags;
	int RemainingTime = wait_event_timeout(Job->WaitForCompletion, Job->State == V3DDRIVER_JOB_COMPLETE, msecs_to_jiffies(JOB_TIMEOUT_MS));
	spin_lock_irqsave(&Instance->Issued.Lock, Flags);
	if (RemainingTime == 0) {
		(void) kref_put(&Job->Waiters, &RemoveJob); /* Remove completion reference as we timed-out */
		printk(KERN_ERR "V3D Job %p timed-out", Job);
	}
	Last = kref_put(&Job->Waiters, &RemoveJob);
	spin_unlock_irqrestore(&Instance->Issued.Lock, Flags);

	if (Last != 0)
		V3dDriver_JobComplete(Instance->Driver, Job);
}

int32_t V3dSession_Wait(V3dSessionType *Instance)
{
	int32_t       Id = Instance->LastId;
#if 0
	/* Find the last ID issued */
	V3dDriver_JobType *Job = FindJob(Instance, Id);
	if (Job != NULL) {
		/* Wait on this one first - will minimise the number of waits */
		WaitJob(Instance, Job);
	}
	return Id;
#else
	unsigned long Flags;
	#if 0
	{
		unsigned int Entries = 0;
		struct list_head  *Current;
		spin_lock_irqsave(&Instance->Issued.Lock, Flags);
		list_for_each(Current, &Instance->Issued.List)
			++Entries;

		if (Entries > 2) {
			printk("%s: %u entries\n", __func__, Entries);
			list_for_each(Current, &Instance->Issued.List) {
				V3dDriver_JobType *Job = list_entry(Current, V3dDriver_JobType, Session.Link);
				printk("  l %p j %p t %d\n", Current, Job, Job->UserJob.job_type);
			}
		}
		spin_unlock_irqrestore(&Instance->Issued.Lock, Flags);
	}
	#endif

	/* Wait on all previous jobs, if any - only required when multiple V3D blocks exist */
	do {
		V3dDriver_JobType *Job;
		spin_lock_irqsave(&Instance->Issued.Lock, Flags);
		if (list_empty(&Instance->Issued.List) != 0)
			break;

		Job = list_first_entry(&Instance->Issued.List, V3dDriver_JobType, Session.Link);
		if (Id - (int32_t) Job->UserJob.job_id < 0)
			break;

		kref_get(&Job->Waiters); /* Prevents freeing on completion */
		spin_unlock_irqrestore(&Instance->Issued.Lock, Flags);

		WaitJob(Instance, Job);
  #if 0
		printk(KERN_ERR "%s: Job %p:%08x / %08x Complete\n",
			__func__,
			Job,
			(int32_t) Job->UserJob.job_id, Id);
  #endif
	} while (1);
	spin_unlock_irqrestore(&Instance->Issued.Lock, Flags);
#endif
	return Id;
}
