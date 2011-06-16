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

#ifndef _RTOS_OSTYPES_H_
#define _RTOS_OSTYPES_H_

#include "types.h"
/**
 * @addtogroup RTOSTypesGroup
 * @{
 */

/**
 *  OSSuspend_t specifies type of suspension that is performed when tasks are waiting on
 *  an object. 
*/


#ifdef WIN32
#define OSSUSPEND_FIFO 0
#define OSSUSPEND_PRIORITY 1
#else
// Hardcode the value same as underlying RTOS, currently NU. We MUST check for consistent value in osseamphore.c, JPG
#define OSSUSPEND_FIFO 6
#define OSSUSPEND_PRIORITY 11
#endif
typedef UInt32 OSSuspend_t;

/**
 *	@brief OSStatus_t is a general status type returned by the RTOS API. 
*/

typedef enum
{
	OSSTATUS_FAILURE,
	OSSTATUS_SUCCESS,
	OSSTATUS_DELETED,
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

// OS Structure sizes used for debug/dump - have CompileAssert calls to check they are correct

#define	OS_MAX_NAME_LEN					8	// must be 8 chars
/* These need to be adjusted if remove define TX_DISABLE_NOTIFY_CALLBACKS from tx_user.h */
#define	OS_QUEUE_SIZE					18  // ulongs
#define	OS_EVENT_GROUP_SIZE				12  // ulongs
#define	OS_SEMAPHORE_SIZE				12  // ulongs
#ifdef ARM_FPU

#ifdef VMF_INCLUDE_NEW 
#define OS_TCB_HEADER_SIZE              560  //bytes
#else
#define OS_TCB_HEADER_SIZE              556  //bytes
#endif // #ifdef VMF_INCLUDE_NEW

#else  // #ifdef ARM_FPU

#ifdef VMF_INCLUDE_NEW  
#define OS_TCB_HEADER_SIZE              284  //bytes
#else
#define OS_TCB_HEADER_SIZE              280  //bytes
#endif	// #ifdef VMF_INCLUDE_NEW

#endif  // #ifdef ARM_FPU
#define	OS_TIMER_SIZE					21  // ulongs
#define	OS_MUTEX_SIZE					18  // ulongs
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
//
// **FIXME**  get redefinition of UInt32 error when building this under Linux using typedef...
#ifndef Ticks_t
#define Ticks_t UInt32
#endif
//typedef UInt32 Ticks_t;

#ifdef WIN32
#define TICKS_FOREVER					((Ticks_t)0xFFFFFFFF)
#define TICKS_NO_WAIT					((Ticks_t)0)
#define TICKS_ONE_SECOND				((Ticks_t)100*10)	// ticks per second
#else

// Hardcode the value same as underlying RTOS, currently NU. We MUST check for consistent value in osseamphore.c, JPG
/// Wait forever
#define TICKS_FOREVER					0xFFFFFFFFUL
/// No Wait
#define TICKS_NO_WAIT					0

/// ONE Second Time Constant

//because 32k clock can't provide exact 1ms timer, need change to 1024 ticks/s

#define TICKS_ONE_SECOND				((Ticks_t)1024)	//ticks per second
															//  100 = 10MS ticker
															//  200 =  5MS ticker
															//  500 =  2MS ticker
															// 1000 =  1MS ticker
															// 2000 =  500US ticker

//Returns number of Ticks for x millisecs
#define TICKS_IN_MILLISECONDS(x)  ((Ticks_t)((TICKS_ONE_SECOND*(x))/1000))

#endif

/** @} */

#endif
