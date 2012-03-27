/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*             @file     arch/arm/plat-kona/include/plat/osabstract/ostypes.h
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

#ifndef _OSTYPES_H_
#define _OSTYPES_H_

#include <linux/sched.h>
#include <linux/jiffies.h>
/**
 * @addtogroup RTOSTypesGroup
 * @{
 */

/**
 *  OSSuspend_t specifies type of suspension that is performed when tasks are waiting on
 *  an object.
*/

/**
 * Hardcode the value same as underlying RTOS, currently NU.
 * We MUST check for consistent value in osseamphore.c, JPG
 */
#define OSSUSPEND_FIFO 6
#define OSSUSPEND_PRIORITY 11
typedef UInt32 OSSuspend_t;

/**
 * @brief OSStatus_t is a general status type returned by the RTOS API.
 */

typedef enum {
	OSSTATUS_FAILURE,
	OSSTATUS_SUCCESS,
	OSSTATUS_TIMEOUT
} OSStatus_t;

/**
 * @brief OSMutexMode_t defines whether the mutex is configured for
 * priority inversion or not.
 */

typedef enum {
	OSMUTEX_NOINHERIT,
	OSMUTEX_INHERIT
} OSMutexMode_t;

#define	OS_MAX_NAME_LEN 8	/* must be 8 chars */
/**
 * These need to be adjusted if remove define
 * TX_DISABLE_NOTIFY_CALLBACKS from tx_user.h
*/
#define	OS_QUEUE_SIZE				18	/* ulongs	*/
#define	OS_EVENT_GROUP_SIZE			12	/* ulongs	*/
#define	OS_SEMAPHORE_SIZE			12	/* ulongs	*/
#define OS_TCB_HEADER_SIZE			216	/* bytes	*/
#define	OS_TIMER_SIZE				19	/* ulongs	*/
#define	OS_MUTEX_SIZE				18	/* ulongs	*/

#define	STACKSIZE_BASIC				512

/* Set for ThreadX, JPG */
#define	OS_PARTITION_POOL_SIZE			-1
#define	OS_MEMORY_POOL_SIZE			-1
#define OS_BLOCK_MEMORY_HEADER_SIZE		32	/* bytes	*/
#define OS_BYTE_MEMORY_HEADER_SIZE		32	/* bytes	*/

typedef void *Task_t;
typedef void *Queue_t;
typedef void *Event_t;
typedef void *Semaphore_t;
typedef void *Mutex_t;
typedef void *Timer_t;

/* Ticks are the basic time unit. */
typedef UInt32 Ticks_t;

/**
 * Hardcode the value same as underlying RTOS, currently NU.
 * We MUST check for consistent value in osseamphore.c, JPG
 */
/* Wait forever */
#define TICKS_FOREVER				MAX_SCHEDULE_TIMEOUT

/* No Wait */
#define TICKS_NO_WAIT				0

/* ONE Second Time Constant */

/**
 * because 32k clock can't provide exact 1ms timer, need change to 1024 ticks/s
 */

/*ticks per second */
#define TICKS_ONE_SECOND			msecs_to_jiffies(1000)

/*Returns number of Ticks for x millisecs */
#define TICKS_IN_MILLISECONDS(x)		msecs_to_jiffies(x)

/** @} */

#endif /* _OSTYPES_H_ */
