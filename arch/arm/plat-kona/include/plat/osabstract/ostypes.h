/****************************************************************************
*
*     Copyright (c) 2007 Broadcom Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           Broadcom Corporation
*           5300 California Avenue
*           Irvine, California 92617
*
*     All information contained in this document is Broadcom Corporation
*     company private, proprietary, and trade secret.
*
****************************************************************************/
/**
*
*   @file   ostypes.h
*
*   @brief  This file contains type definitions for the RTOS API.
*
****************************************************************************/
/**

*   @defgroup   RTOSTypesGroup   RTOS Types
*   @ingroup    RTOSGroup
*
*	This group defines the global types used by the RTOS API.
*
****************************************************************************/
/* This file was taken from 
 * //depot/Sources/SystemIntegration/msp/soc/os/public/ostypes.h
 * on 6th Oct 2010
 */

#ifndef _OSTYPES_H_
#define _OSTYPES_H_

#include <linux/jiffies.h>
/**
 * @addtogroup RTOSTypesGroup
 * @{
 */

/**
 *  OSSuspend_t specifies type of suspension that is performed when tasks are waiting on
 *  an object. 
*/

// Hardcode the value same as underlying RTOS, currently NU. We MUST check for consistent value in osseamphore.c, JPG
#define OSSUSPEND_FIFO 6
#define OSSUSPEND_PRIORITY 11
typedef UInt32 OSSuspend_t;

/**
 *	@brief OSStatus_t is a general status type returned by the RTOS API. 
*/

typedef enum
{
	OSSTATUS_FAILURE,
	OSSTATUS_SUCCESS,
	OSSTATUS_TIMEOUT
} OSStatus_t;

/**
 *	@brief OSMutexMode_t defines whether the mutex is configured for priority inversion or not. 
*/

typedef enum
{
	OSMUTEX_NOINHERIT,
	OSMUTEX_INHERIT
} OSMutexMode_t;

#define	OS_MAX_NAME_LEN					8	// must be 8 chars
/* These need to be adjusted if remove define TX_DISABLE_NOTIFY_CALLBACKS from tx_user.h */
#define	OS_QUEUE_SIZE					18  // ulongs
#define	OS_EVENT_GROUP_SIZE				12  // ulongs
#define	OS_SEMAPHORE_SIZE				12  // ulongs
#define OS_TCB_HEADER_SIZE              216  //bytes
#define	OS_TIMER_SIZE					19  // ulongs
#define	OS_MUTEX_SIZE					18  // ulongs

#define	STACKSIZE_BASIC					512

// Set for ThreadX, JPG
#define	OS_PARTITION_POOL_SIZE			-1
#define	OS_MEMORY_POOL_SIZE				-1
#define OS_BLOCK_MEMORY_HEADER_SIZE     32  //bytes
#define OS_BYTE_MEMORY_HEADER_SIZE      32  //bytes

typedef void *Task_t;
typedef void *Queue_t;
typedef void *Event_t;
typedef void *Semaphore_t;
typedef void *Mutex_t;
typedef void *Timer_t;

/// Ticks are the basic time unit.

typedef UInt32 Ticks_t;

// Hardcode the value same as underlying RTOS, currently NU. We MUST check for consistent value in osseamphore.c, JPG
/// Wait forever
#define TICKS_FOREVER					MAX_SCHEDULE_TIMEOUT
/// No Wait
#define TICKS_NO_WAIT					0

/// ONE Second Time Constant

//because 32k clock can't provide exact 1ms timer, need change to 1024 ticks/s

#define TICKS_ONE_SECOND				msecs_to_jiffies(1000) //ticks per second
//Returns number of Ticks for x millisecs
#define TICKS_IN_MILLISECONDS(x)			msecs_to_jiffies(x)


/** @} */

#endif
