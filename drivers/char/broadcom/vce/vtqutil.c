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

#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/wait.h>

/* Internal utility functions */
#include "vtqutil.h"

/* PRIORITY LOCK -- UTILITY FUNCTION, NOT VTQ FUNCTIONALITY PER SE */

#define MAXPRIVLEVELSS 2

struct vtq_priority_lock {
	wait_queue_head_t waitq;
	atomic_t count[MAXPRIVLEVELSS];
	int highestpriority;
};

struct vtq_priority_lock *vtq_util_priority_lock_create(void)
{
	struct vtq_priority_lock *lock;
	int i;

	lock = kmalloc(sizeof(*lock), GFP_KERNEL);
	if (!lock)
		return NULL;

	init_waitqueue_head(&lock->waitq);
	for (i = 0; i < MAXPRIVLEVELSS; i++)
		atomic_set(&lock->count[i], 0);
	lock->highestpriority = -1;

	return lock;
}

void vtq_util_priority_lock_destroy(struct vtq_priority_lock *lock)
{
	int i;
	for (i = 0; i < MAXPRIVLEVELSS; i++)
		BUG_ON(atomic_read(&lock->count[i]) != 0);

	/* is there an opposite to init_waitqueue_head? TODO: check! */

	kfree(lock);
}

static int noonebelow(struct vtq_priority_lock *lock, int priority)
{
	int i;

	for (i = 0; i < priority; i++)
		if (atomic_read(&lock->count[i]) != 0)
			return 0;
	return 1;
}

static int someoneabove(struct vtq_priority_lock *lock, int priority)
{
	int i;

	for (i = priority + 1; i < MAXPRIVLEVELSS; i++)
		if (atomic_read(&lock->count[i]) != 0)
			return 1;
	return 0;
}

void vtq_util_priority_unlock(struct vtq_priority_lock *lock, int priority)
{
	if (priority < 0 || priority >= MAXPRIVLEVELSS)
		return;

	if (atomic_dec_and_test(&lock->count[priority]))
		wake_up_all(&lock->waitq);
}

int vtq_util_priority_lock_nowait(struct vtq_priority_lock *lock, int priority)
{
	if (priority < 0 || priority >= MAXPRIVLEVELSS)
		return -1;

	/* This would momentarily stall the higher-priority guy, but
	 * that's okay, we win overall by not needing a mutex */
	atomic_inc(&lock->count[priority]);
	if (someoneabove(lock, priority)) {
		if (atomic_dec_and_test(&lock->count[priority]))
			wake_up_all(&lock->waitq);
		return -1;
	}

	return 0;
}

int vtq_util_priority_lock_flush(struct vtq_priority_lock *lock, int priority)
{
	int s;
	s = wait_event_interruptible(lock->waitq,
			noonebelow(lock, priority));
	return s;
}

int vtq_util_priority_lock(struct vtq_priority_lock *lock, int priority)
{
	int s;
	s = vtq_util_priority_lock_nowait(lock, priority);
	if (s)
		return s;

	s = vtq_util_priority_lock_flush(lock, priority);
	if (s) {
		vtq_util_priority_unlock(lock, priority);
		return s;
	}

	return 0;
}
