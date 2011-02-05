/*****************************************************************************
*
*    (c) 2001-2010 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or
* its licensors, and may only be used, duplicated, modified or distributed
* pursuant to the terms and conditions of a separate, written license
* agreement executed between you and Broadcom (an "Authorized License").
* Except as set forth in an Authorized License, Broadcom grants no license
* (express or implied), right to use, or waiver of any kind with respect to
* the Software, and Broadcom expressly reserves all rights in and to the
* Software and all intellectual property rights therein.
* IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS
* SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
* ALL USE OF THE SOFTWARE.  
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*    IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
*    FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
*    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU
*    ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
*****************************************************************************/
/**
*
*   @file   ossemaphore.h
*
*   @brief  This file contains the public interface to the OSAL Semaphore functionality.
*
****************************************************************************/
/**

*   @defgroup   RTOSSemaphoreGroup   Semaphore Services
*   @ingroup    RTOSGroup
*
*
*	The Semaphore API provides interfaces to create/destroy and control
	counting semaphores and semaphore based mutexes.

*
****************************************************************************/
/*
 * This file is taken from //depot/Sources/SystemIntegration/msp/soc/os/public/
 * on 6th Oct 2010
 */

/**
 * @addtogroup RTOSSemaphoreGroup
 * @{
 */


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

static inline Semaphore_t OSSEMAPHORE_Create(			// returns newly-created semaphore
	SCount_t count,						// number of available semaphores
	OSSuspend_t mode					// suspending task mode
	)
{
	Semaphore_t *sem_ptr = (Semaphore_t *) kzalloc(sizeof(struct semaphore), GFP_KERNEL);
	sema_init((struct semaphore *)sem_ptr, count);
	return sem_ptr;
}

/**
	Deallocate and destroy a sempahore.
	@param	s	(in)	Semaphore object to destory.
**/

static inline void OSSEMAPHORE_Destroy( 				// Destroy a semaphore
	Semaphore_t s						// Semaphore to destroy
	)
{
	kfree(s);
}

/**
	Obtain a semaphore.  If the semaphore count is 0 the calling task will
	block until the semphore is released.
	@param s	(in)	Semaphore Object.
	@param timeout	(in)	Ticks to wait for semaphore.
	@return OSStatus_t	Status of operation.
**/

static inline OSStatus_t OSSEMAPHORE_Obtain(			// wait to obtain a semaphore
	Semaphore_t s,						// semaphore pointer
	Ticks_t timeout						// timeout to failed semaphore obtain
	)
{
	int status = down_timeout((struct semaphore *)s, timeout);
	if(!status)
	{
		return OSSTATUS_SUCCESS;
	}
	else if(status == -ETIME)
	{
		pr_info("OSSEMAPHORE_Obtain timeout\n");
		return OSSTATUS_TIMEOUT;
	}
	else
	{
		pr_info("OSSEMAPHORE_Obtain error:%d\n", status);
		return OSSTATUS_FAILURE;
	}
}

/**
	Release a semphore that was obtained via a call to OSSEMPAHORE_Obtain.
	@param	s	(in)	Sempahore Object.
	@return OSStatus_t	Status of operation.
**/

static inline OSStatus_t OSSEMAPHORE_Release(			// wait to release a semaphore
	Semaphore_t s						// semaphore pointer
	)
{
	up((struct semaphore *)s);
	return OSSTATUS_SUCCESS;
}

/**
	Create a Mutex.
	@param	mode	(in)	Suspension Mode
	@return Semaphore_t		Semaphore Object.
**/
 
static inline Semaphore_t OSSEMAPHORE_createMutex(	// returns newly-created "mutex"
	OSSuspend_t mode					// suspending task mode
	)
{
	Semaphore_t *mutex_ptr = (Semaphore_t) kzalloc(sizeof(struct mutex), GFP_KERNEL);
	mutex_init((struct mutex *)mutex_ptr);
	return mutex_ptr;
}

/**
	Deallocate and destroy resources associated with a Mutex.
	@param	s	(in)	Sempahore Object
**/

static inline void OSSEMAPHORE_destroyMutex( 			// Destroy a "mutex"
	Semaphore_t s						// "Mutex" to destroy
	)
{
	kfree(s);
}

/**
	Obtain a mutex.  If the mutex is already "locked" the calling task
	will block until the mutex is "unlocked".
	@param s (in)	Sempahore Object.
	@param timeout (in) Ticks to wait for mutex.
	@return OSSTATUS_t	Status of operation.
**/

static inline OSStatus_t OSSEMAPHORE_obtainMutex(		// wait to obtain a "mutex"
	Semaphore_t s,						// "Mutex" pointer
	Ticks_t timeout						// timeout to failed "mutex" obtain
	)
{
	return (mutex_lock_interruptible((struct mutex *)s)? OSSTATUS_FAILURE : OSSTATUS_SUCCESS);
}

/**
	Release a mutex that was obtained via a call to OSSEMPAHORE_obtainMutex.
	@param	s	(in)	Sempahore Object.
	@return OSStatus_t	Status of operation.
**/


static inline OSStatus_t OSSEMAPHORE_releaseMutex(	// wait to release a "mutex"
	Semaphore_t s						// "mutex" pointer
	)
{
	mutex_unlock((struct mutex *)s);
	return OSSTATUS_SUCCESS;
}

	
/**
	Get current count of a counting semaphore.
	@param passed_in_s	(in)	Semaphore Object.
	@return UInt32	Semaphore count.
**/

static inline UInt32 OSSEMAPHORE_GetCnt(				// return count of semaphore,
	Semaphore_t passed_in_s				// if count <= 0, return -1
	)
{
	/* Not supported*/
	return 0;
}


/**
	Reset the count of a semaphore to zero.
	This function has high risk. Use it with extreme caution
	@param passed_in_s	(in)	Semaphore Object.
	@return UInt32	Semaphore count.
**/

static inline UInt32 OSSEMAPHORE_ResetCnt(
	Semaphore_t passed_in_s
	)
{
	/* Not supported*/
	return 0;
}


/**
	Change the name of an existing semaphore.
	@param s	(in)	Semaphore Object.
	@param name	(in)	New name of the semaphore.
**/

static inline void OSSEMAPHORE_ChangeName(
	Semaphore_t s,
	const char* name
	)
{
	/* We should define a separate structure in case this needs to be implemented*/
}

/**
	Get the name of an existing semaphore.
	@param s		(in)	Semaphore Object.
	@param p_name	(in)	Name storage (minimum 9 characters).
	@return OSStatus_t	Status of operation.
**/

static inline OSStatus_t OSSEMAPHORE_GetName(		// get ASCII name of semaphore
	Semaphore_t s,					// semaphore pointer
	UInt8 *p_name					// location to store the ASCII name
	)
{
	/* We should define a separate structure in case this needs to be implemented */
	return OSSTATUS_FAILURE;
}

/** @} */

#endif
