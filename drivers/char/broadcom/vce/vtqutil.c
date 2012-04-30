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

#include <linux/mutex.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/wait.h>

/* Internal utility functions */
#include "vtqutil.h"

/* PRIORITY LOCK -- UTILITY FUNCTION, NOT VTQ FUNCTIONALITY PER SE */

struct vtq_priority_lock_owner_node {
	int priority;
	struct vtq_priority_lock_owner_node *next;
};

struct vtq_priority_lock {
	struct mutex mutex;
	wait_queue_head_t waitq;
	struct vtq_priority_lock_owner_node **tail;
	struct vtq_priority_lock_owner_node *head;
	int highestpriority;
};

/*
 * TODO: this lock is using kmalloc() which is most likely not
 * necessary... We should be able to avoid this.  We are also
 * acquiring a mutex in the unlock path, which is arguably wrong --
 * Hack Alert!  FIXME.  -- At least this provides a functionally
 * correct API which can be used to assist with arbitration of the VCE
 * resource.
 */

struct vtq_priority_lock *vtq_util_priority_lock_create(void)
{
	struct vtq_priority_lock *lock;

	lock = kmalloc(sizeof(*lock), GFP_KERNEL);
	if (!lock)
		return NULL;

	mutex_init(&lock->mutex);
	lock->head = NULL;
	lock->tail = &lock->head;
	init_waitqueue_head(&lock->waitq);
	lock->highestpriority = -1;

	return lock;
}

void vtq_util_priority_lock_destroy(struct vtq_priority_lock *lock)
{
	BUG_ON(lock->head != NULL);
	BUG_ON(lock->tail != &lock->head);
	mutex_destroy(&lock->mutex);

	/* is there an opposite to init_waitqueue_head? TODO: check! */

	kfree(lock);
}

void vtq_util_priority_unlock(struct vtq_priority_lock *lock, int priority)
{
	struct vtq_priority_lock_owner_node *this, **ptr;
	int found;
	int highestpri_left;
	int old_head_priority;

	mutex_lock(&lock->mutex);
	ptr = &lock->head;
	this = *ptr;
	found = 0;
	old_head_priority = lock->head->priority;
	while (this) {
		if (this->priority == priority) {
			*ptr = this->next;
			if (lock->tail == &this->next)
				lock->tail = ptr;
			kfree(this);
			found = 1;
			break;
		}
		ptr = &this->next;
		this = *ptr;
	}
	BUG_ON(!found);
	highestpri_left = -1;
	this = lock->head;
	while (this) {
		if (this->priority > highestpri_left)
			highestpri_left = this->priority;
		this = this->next;
	}
	BUG_ON(highestpri_left > lock->highestpriority);
	if (lock->highestpriority != highestpri_left)
		lock->highestpriority = highestpri_left;
	if (lock->head != NULL && lock->head->priority != old_head_priority)
		wake_up_all(&lock->waitq);
	mutex_unlock(&lock->mutex);
}

int vtq_util_priority_lock_nowait(struct vtq_priority_lock *lock, int priority)
{
	struct vtq_priority_lock_owner_node *new;

	if (lock->highestpriority > priority)
		return -1;

	new = kmalloc(sizeof(*new), GFP_KERNEL);
	if (!new)
		return -1;

	new->priority = priority;
	new->next = NULL;

	mutex_lock(&lock->mutex);
	if (lock->highestpriority > priority) {
		kfree(new);
		return -1;
	}

	lock->highestpriority = priority;
	*lock->tail = new;
	lock->tail = &new->next;
	mutex_unlock(&lock->mutex);

	return 0;
}

int vtq_util_priority_lock_flush(struct vtq_priority_lock *lock, int priority)
{
	int s;
	s = wait_event_interruptible(lock->waitq,
			lock->head->priority == priority);
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
