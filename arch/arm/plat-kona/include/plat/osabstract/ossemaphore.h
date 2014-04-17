
/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*             @file     arch/arm/plat-kona/include/plat/osabstract/ossemaphore.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#ifndef _OSSEMAPHORE_H_
#define _OSSEMAPHORE_H_

#include <plat/types.h>
#include <plat/osabstract/ostypes.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <linux/slab.h>

//******************************************************************************
// Global Typedefs
//******************************************************************************

/// Semaphore creation index.

typedef UInt32 SCount_t;

//******************************************************************************
// Global Function Prototypes
//******************************************************************************

/**
	Create a counting Semphore.
	@param count	(in)	Initial count for semaphore.  A value of 0 indicates
							the semaphore is already taken.
	@param mode		(in)	Suspension Mode.
	@return	Sempahore_t	A sempahore object.
**/

static inline Semaphore_t OSSEMAPHORE_Create(	// returns newly-created semaphore
						    SCount_t count,	// number of available semaphores
						    OSSuspend_t mode	// suspending task mode
    )
{
	Semaphore_t *sem_ptr =
	    (Semaphore_t *) kzalloc(sizeof(struct semaphore), GFP_KERNEL);
	if (sem_ptr == NULL) {
		pr_err("%s cannot create semaphore at %d\n", __func__,
		       __LINE__);
		return sem_ptr;
	}
	sema_init((struct semaphore *)sem_ptr, count);
	return sem_ptr;
}

/**
	Deallocate and destroy a sempahore.
	@param	s	(in)	Semaphore object to destory.
**/

static inline void OSSEMAPHORE_Destroy(	// Destroy a semaphore
					      Semaphore_t s	// Semaphore to destroy
    )
{
	if (s)
		kfree(s);
	else
		pr_err("%s cannot destroy semapthore(%p) at line %d\n",
		       __func__, s, __LINE__);
}

/**
	Obtain a semaphore.  If the semaphore count is 0 the calling task will
	block until the semphore is released.
	@param s	(in)	Semaphore Object.
	@param timeout	(in)	Ticks to wait for semaphore.
	@return OSStatus_t	Status of operation.
**/

static inline OSStatus_t OSSEMAPHORE_Obtain(	// wait to obtain a semaphore
						   Semaphore_t s,	// semaphore pointer
						   Ticks_t timeout	// timeout to failed semaphore obtain
    )
{
	int status = down_timeout((struct semaphore *)s, timeout);
	if (!status) {
		return OSSTATUS_SUCCESS;
	} else if (status == -ETIME) {
		pr_info("OSSEMAPHORE_Obtain timeout\n");
		return OSSTATUS_TIMEOUT;
	} else {
		pr_info("OSSEMAPHORE_Obtain error:%d\n", status);
		return OSSTATUS_FAILURE;
	}
}

/**
	Release a semphore that was obtained via a call to OSSEMPAHORE_Obtain.
	@param	s	(in)	Sempahore Object.
	@return OSStatus_t	Status of operation.
**/

static inline OSStatus_t OSSEMAPHORE_Release(	// wait to release a semaphore
						    Semaphore_t s	// semaphore pointer
    )
{
	up((struct semaphore *)s);
	return OSSTATUS_SUCCESS;
}

/**
	Change the name of an existing semaphore.
	@param s	(in)	Semaphore Object.
	@param name	(in)	New name of the semaphore.
**/

static inline void OSSEMAPHORE_ChangeName(Semaphore_t s, const char *name)
{
	/* We should define a separate structure in case this needs to be implemented */
}

/**
	Get the name of an existing semaphore.
	@param s		(in)	Semaphore Object.
	@param p_name	(in)	Name storage (minimum 9 characters).
	@return OSStatus_t	Status of operation.
**/

static inline OSStatus_t OSSEMAPHORE_GetName(	// get ASCII name of semaphore
						    Semaphore_t s,	// semaphore pointer
						    UInt8 * p_name	// location to store the ASCII name
    )
{
	/* We should define a separate structure in case this needs to be implemented */
	return OSSTATUS_FAILURE;
}

#endif
