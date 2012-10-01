#ifndef BRCM_VTQ_UTIL_H
#define BRCM_VTQ_UTIL_H

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

/* higher level priority lock shim */
/* TODO: move to higher level API, outside of VTQ: */
struct vtq_priority_lock;

/* Create a lock object */
extern struct vtq_priority_lock *vtq_util_priority_lock_create(void);

/* Destroy a lock object */
extern void vtq_util_priority_lock_destroy(struct vtq_priority_lock *lock);

/* Release a lock at a certain priority.  When the last reference at a
 * given priority is released, higher priority ones that were waiting
 * will proced */
extern void vtq_util_priority_unlock(struct vtq_priority_lock *lock,
		int priority);

/* Acquire a shared lock at a given priority.  The request will be
 * granted only if there is not a higher priority waiter waiting */
extern int vtq_util_priority_lock(struct vtq_priority_lock *lock,
		int priority);

#endif
