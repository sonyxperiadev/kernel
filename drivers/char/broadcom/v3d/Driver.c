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
#include "Session.h"
#include "Device.h"
#include "Driver.h"


/* ================================================================ */

void V3dDriver_Delete(V3dDriverType *Instance)
{
	switch (Instance->Initialised) {
	case 1:
		V3dDriver_DeleteProcEntries(Instance);

	case 0:
		kfree(Instance);
		break;
	}
}

void V3dDriver_ResetStatistics(V3dDriverType *Instance)
{
	Statistics_Initialise(&Instance->BinRender.Queue);
	Statistics_Initialise(&Instance->BinRender.Run);
	Statistics_Initialise(&Instance->User.Queue);
	Statistics_Initialise(&Instance->User.Run);
	Instance->TotalRun = 0;
	Instance->Start = ktime_get();
}

V3dDriverType *V3dDriver_Create(void)
{
	V3dDriverType *Instance = kmalloc(sizeof(V3dDriverType), GFP_KERNEL);
	if (Instance == NULL)
		return NULL;
	Instance->Initialised = 0;

	/* Unconditional initialisation */
	spin_lock_init(&Instance->Job.Free.Lock);
	spin_lock_init(&Instance->Job.Posted.Lock);

	{
		unsigned int i;
		INIT_LIST_HEAD(&Instance->Job.Free.List);
		for (i = 0 ; i < sizeof(Instance->Job.FreeJobs) / sizeof(Instance->Job.FreeJobs[0]) ; ++i)
			list_add_tail(&Instance->Job.FreeJobs[i].Link, &Instance->Job.Free.List);
		sema_init(&Instance->Job.Free.Count, i);
	}
	INIT_LIST_HEAD(&Instance->Job.Posted.User);
	INIT_LIST_HEAD(&Instance->Job.Posted.BinRender);
	INIT_LIST_HEAD(&Instance->Job.Posted.Exclusive.User);
	INIT_LIST_HEAD(&Instance->Job.Posted.Exclusive.BinRender);
	mutex_init(&Instance->Job.Posted.Exclusive.Lock);
	Instance->Job.Posted.Exclusive.Owner = NULL;

	Instance->Device = NULL;

	V3dDriver_ResetStatistics(Instance);

	memset(&Instance->Sessions, 0, sizeof(Instance->Sessions));

	/* Stuff that can fail */
	if (V3dDriver_CreateProcEntries(Instance) != 0)
		return V3dDriver_Delete(Instance), NULL;
	++Instance->Initialised;

	return Instance;
}


/* ================================================================ */

void V3dDriver_AddDevice(V3dDriverType *Instance, struct V3dDeviceTag *Device)
{
	BUG_ON(Instance->Device != NULL);
	Instance->Device = Device;
}

static V3dSessionType **GetSessionEntry(V3dDriverType *Instance, V3dSessionType *Session)
{
	unsigned int i;
	for (i = 0 ; i < sizeof(Instance->Sessions) / sizeof(Instance->Sessions[0]) ; ++i)
		if (Instance->Sessions[i] == Session)
			return &Instance->Sessions[i];
	return NULL;
}

/* TODO: Lock - potentially in the caller */
int V3dDriver_AddSession(V3dDriverType *Instance, struct V3dSessionTag *Session)
{
	V3dSessionType **Entry = GetSessionEntry(Instance, NULL);
	if (Entry == NULL)
		return -ENOMEM;
	*Entry = Session;
	return 0;
}

void V3dDriver_RemoveSession(V3dDriverType *Instance, struct V3dSessionTag *Session)
{
	V3dSessionType **Entry = GetSessionEntry(Instance, Session);
	BUG_ON(Session == NULL);
	*Entry = NULL;
}


/* ================================================================ */

static V3dDriver_JobType *RemoveHead(struct list_head *List)
{
	V3dDriver_JobType *Job;
	if (list_empty(List) != 0)
		return NULL;
	Job = list_first_entry(List, V3dDriver_JobType, Link);
	list_del(&Job->Link);
	return Job;
}

/* Requires Instance->Job.Posted.Lock to be held */
V3dDriver_JobType *V3dDriver_JobGet(V3dDriverType *Instance, unsigned int Required)
{
	V3dDriver_JobType *Job = NULL;
	int                Exclusive;

	/* See if we've completed everything queued */
	/* when the exclusive lock was requested */
	Exclusive = Instance->Job.Posted.Exclusive.Owner != NULL
		&& Instance->Job.Posted.Exclusive.BinRenderCount == 0
		&& Instance->Job.Posted.Exclusive.UserCount      == 0;

	if ((Required & V3D_JOB_USER) != 0)
		Job = RemoveHead(Exclusive != 0 ? &Instance->Job.Posted.Exclusive.User      : &Instance->Job.Posted.User);
	if (Job == NULL && (Required & V3D_JOB_BIN_REND) != 0)
		Job = RemoveHead(Exclusive != 0 ? &Instance->Job.Posted.Exclusive.BinRender : &Instance->Job.Posted.BinRender);

	/* Update counts for switching to exclusive queues */
	if (Job != NULL && Instance->Job.Posted.Exclusive.Owner != NULL && Exclusive == 0) {
		if (Job->UserJob.job_type == V3D_JOB_USER)
			--Instance->Job.Posted.Exclusive.UserCount;
		else
			--Instance->Job.Posted.Exclusive.BinRenderCount;
	}
	return Job;
}

void V3dDriver_JobComplete(V3dDriverType *Instance, V3dDriver_JobType *Job)
{
	/* Return the job to Free.List */
	unsigned long Flags;
	unsigned int Queue;
	unsigned int Run;
	Job->End = ktime_get();
	spin_lock_irqsave(&Instance->Job.Posted.Lock, Flags);
	if (Job->State == V3DDRIVER_JOB_ISSUED) {
		/* Hasn't yet made it to the hardware, so needs dequeueing */
		/* Note: Even if a job is completed by both a time-out and */
		/*       normal completion, the ref count only hits zero   */
		/*       once, so we only complete once                    */
		list_del(&Job->Link);
	}

	Queue = ktime_us_delta(Job->Start, Job->Queued);
	Run   = ktime_us_delta(Job->End,   Job->Start);
	Instance->TotalRun += Run;
	if (Job->UserJob.job_type == V3D_JOB_USER) {
		Statistics_Add(&Instance->User.Queue, Queue);
		Statistics_Add(&Instance->User.Run,   Run);
		V3dSession_AddStatistics(Job->Session.Instance, 1 /* User? */, Queue, Run);
	} else {
		Statistics_Add(&Instance->BinRender.Queue, Queue);
		Statistics_Add(&Instance->BinRender.Run,   Run);
		V3dSession_AddStatistics(Job->Session.Instance, 0 /* User? */, Queue, Run);
	}
	spin_unlock_irqrestore(&Instance->Job.Posted.Lock, Flags);

	spin_lock_irqsave(&Instance->Job.Free.Lock, Flags);
	list_add_tail(&Job->Link, &Instance->Job.Free.List);
	spin_unlock_irqrestore(&Instance->Job.Free.Lock, Flags);
	up(&Instance->Job.Free.Count);
}


/* ================================================================ */

void V3dDriver_ExclusiveStart(V3dDriverType *Instance, struct V3dSessionTag *Session)
{
	unsigned long     Flags;
	mutex_lock(&Instance->Job.Posted.Exclusive.Lock);
	BUG_ON(Instance->Job.Posted.Exclusive.Owner != NULL);
	Instance->Job.Posted.Exclusive.Owner = Session;

	/* Count the number of jobs outstanding */
	Instance->Job.Posted.Exclusive.BinRenderCount = 0;
	Instance->Job.Posted.Exclusive.UserCount      = 0;
	Flags = V3dDriver_JobLock(Instance);
#if 0
	{
		struct list_head *Current;
		list_for_each(Current, &Instance->Job.Posted.BinRender)
			++Instance->Job.Posted.Exclusive.BinRenderCount;
		list_for_each(Current, &Instance->Job.Posted.User)
			++Instance->Job.Posted.Exclusive.UserCount;
	}
#endif
	V3dDriver_JobUnlock(Instance, Flags);
}

int V3dDriver_ExclusiveStop(V3dDriverType *Instance, struct V3dSessionTag *Session)
{
	unsigned long Flags;
	int           Empty;
	if (Instance->Job.Posted.Exclusive.Owner != Session)
		return -EPERM;

	Flags = V3dDriver_JobLock(Instance);
	Empty = list_empty(&Instance->Job.Posted.Exclusive.User) != 0
		&& list_empty(&Instance->Job.Posted.Exclusive.BinRender) != 0;
	V3dDriver_JobUnlock(Instance, Flags);
	if (Empty == 0)
		return -EBUSY;

	Instance->Job.Posted.Exclusive.Owner = NULL;
	mutex_unlock(&Instance->Job.Posted.Exclusive.Lock);

	/* We'll be Idle now, but the normal lists may not be empty */
	V3dDevice_JobPosted(Instance->Device);
#if 0
	{
		unsigned int i;
		for (i = 0 ; i < Instance->Job.TimeIndex ; ++i) {
			printk(KERN_ERR "j %2u q %9u r %9u\n",
				i, Instance->Job.Times[i].Queue, Instance->Job.Times[i].Run);
		}
	}
#endif
	return 0;
}

int V3dDriver_JobPost(
	V3dDriverType        *Instance,
	V3dSessionType       *Session,
	const v3d_job_post_t *UserJob)
{
	V3dDriver_JobType *Job;
	unsigned long      Flags;
	struct list_head  *List;
	if (Session == Instance->Job.Posted.Exclusive.Owner)
		List = UserJob->job_type == V3D_JOB_USER ? &Instance->Job.Posted.Exclusive.User : &Instance->Job.Posted.Exclusive.BinRender;
	else
		List = UserJob->job_type == V3D_JOB_USER ? &Instance->Job.Posted.User           : &Instance->Job.Posted.BinRender;

	if (list_empty(&Instance->Job.Free.List))
		printk(KERN_ERR "%s: Free list empty!\n", __func__);

	/* Get a free job structure */
	down(&Instance->Job.Free.Count);
	spin_lock_irqsave(&Instance->Job.Free.Lock, Flags);
	Job = list_first_entry(&Instance->Job.Free.List, V3dDriver_JobType, Link);
	list_del(&Job->Link);
	spin_unlock_irqrestore(&Instance->Job.Free.Lock, Flags);

	/* Initialise and copy-in the user-side job information */
	init_waitqueue_head(&Job->WaitForCompletion);
	kref_init(&Job->Waiters); /* Starts at 1 - Completion reference */
	Job->State = V3DDRIVER_JOB_INITIALISED;
	if ((uint32_t) UserJob < PAGE_OFFSET) {
		int Status = copy_from_user(&Job->UserJob, UserJob, sizeof(Job->UserJob));
		if (Status != 0) {
			printk(KERN_ERR "%s: j %p failed copy", __func__, Job);
			V3dDriver_JobComplete(Instance, Job);
			return Status;
		}
	} else
		Job->UserJob = *UserJob;
	Job->Session.Instance = Session;
	V3dSession_Issued(Job);

	/* Queue it */
	Job->Queued = ktime_get();
	spin_lock_irqsave(&Instance->Job.Posted.Lock, Flags);
	list_add_tail(&Job->Link, List);
	spin_unlock_irqrestore(&Instance->Job.Posted.Lock, Flags);

	/* Kick consumer(s) */
	V3dDevice_JobPosted(Instance->Device);
	return 0;
}
