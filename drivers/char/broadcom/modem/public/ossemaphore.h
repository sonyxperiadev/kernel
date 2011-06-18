/****************************************************************************
*
*     Copyright (c) 2004 Broadcom Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           Broadcom Corporation
*           16215 Alton Parkway
*           P.O. Box 57013
*           Irvine, California 92619-7013
*
*     All information contained in this document is Broadcom Corporation
*     company private, proprietary, and trade secret.
*
****************************************************************************/
/**
*
*   @file   ossemaphore.h
*
*   @brief  This file contains the interface to the Semaphore functionality.
*
****************************************************************************/
/**

*   @defgroup   RTOSSemaphoreGroup   Semaphore Services
*   @ingroup    RTOSGroup
*
*
*	The Semaphore API provides interfaces to create/destroy and control
	counting semaphores and mutexes.

*
****************************************************************************/
/**
 * @addtogroup RTOSSemaphoreGroup
 * @{
 */


#ifndef _RTOS_OSSEMAPHORE_H_
#define _RTOS_OSSEMAPHORE_H_

//#include "mobcom_types.h"
#include "ostypes.h"

//******************************************************************************
// Global Macros
//******************************************************************************

#define	ENUM_SEMAPHORE_HISTORY_CREATE			1
#define	ENUM_SEMAPHORE_HISTORY_BEFORE_OBTAIN	2
#define	ENUM_SEMAPHORE_HISTORY_AFTER_OBTAIN		3
#define	ENUM_SEMAPHORE_HISTORY_BEFORE_RELEASE	4
#define	ENUM_SEMAPHORE_HISTORY_AFTER_RELEASE	5
#define	ENUM_SEMAPHORE_HISTORY_DESTROY			6

#define	OSSEMAPHORE_DEBUG_ID					0x534D4447	//SMDG

//******************************************************************************
// Global Typedefs
//******************************************************************************

/// Count for a counting semaphore.

typedef UInt32 SCount_t;

typedef struct
{
	UInt32				Id;

	UInt32				scb[OS_SEMAPHORE_SIZE];

	UInt32				debug_scb_id;

	Task_t				create_task;
	char				*create_file;
	UInt32				create_line;
	UInt32				create_time;
	UInt32				create_count;

	Task_t				to_obtain_task;
	char				*to_obtain_file;
	UInt32				to_obtain_line;
	UInt32				to_obtain_time;
	UInt32				to_obtain_count;

	Task_t				obtained_task;
	char				*obtained_file;
	UInt32				obtained_line;
	UInt32				obtained_time;
	UInt32				obtained_count;

	Task_t				release_task;
	char				*release_file;
	UInt32				release_line;
	UInt32				release_time;
	UInt32				release_count;
} SemaphoreStruct_t;

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

#ifdef HISTORY_LOGGING
#define	OSSEMAPHORE_Create(count, mode)	OSSEMAPHORE_CreateDebug(count, mode, __FILE__, __LINENUM__)
Semaphore_t OSSEMAPHORE_CreateDebug(		// returns newly-created semaphore
	SCount_t count,							// number of available semaphores
	OSSuspend_t mode,						// suspending task mode
	char *file,								// file that calls OSSEMAPHORE_Obtain()
	UInt32 line								// line number in above file
	);
#else // #ifdef HISTORY_LOGGING
Semaphore_t OSSEMAPHORE_Create(			// returns newly-created semaphore
	SCount_t count,						// number of available semaphores
	OSSuspend_t mode					// suspending task mode
	);
#endif //#ifdef HISTORY_LOGGING

/**
	Deallocate and destroy a sempahore.
	@param	s	(in)	Semaphore object to destory.
**/

#ifdef HISTORY_LOGGING
#define	OSSEMAPHORE_Destroy(s)	OSSEMAPHORE_DestroyDebug(s, __FILE__, __LINENUM__)
void OSSEMAPHORE_DestroyDebug( 				// Destroy a semaphore
	Semaphore_t s,							// Semaphore to destroy
	char *file,								// file that calls OSSEMAPHORE_Obtain()
	UInt32 line								// line number in above file
	);
#else // #ifdef HISTORY_LOGGING
void OSSEMAPHORE_Destroy( 				// Destroy a semaphore
	Semaphore_t s						// Semaphore to destroy
	);
#endif // #ifdef HISTORY_LOGGING

/**
	Obtain a semaphore.  If the semaphore count is 0 the calling task will
	block until the semphore is released.
	@param s	(in)	Semaphore Object.
	@param timeout	(in)	Ticks to wait for semaphore.
	@return OSStatus_t	Status of operation.
**/

#ifdef HISTORY_LOGGING
#define	OSSEMAPHORE_Obtain(s, timeout)	OSSEMAPHORE_ObtainDebug(s, timeout, __FILE__, __LINENUM__)
OSStatus_t OSSEMAPHORE_ObtainDebug(			// wait to obtain a semaphore
	Semaphore_t s,							// semaphore pointer
	Ticks_t timeout,						// timeout to failed semaphore obtain
	char *file,								// file that calls OSSEMAPHORE_Obtain()
	UInt32 line								// line number in above file
	);
#else // #ifdef HISTORY_LOGGING
OSStatus_t OSSEMAPHORE_Obtain(			// wait to obtain a semaphore
	Semaphore_t s,						// semaphore pointer
	Ticks_t timeout						// timeout to failed semaphore obtain
	);
#endif // #ifdef HISTORY_LOGGING

/**
	Release a semphore that was obtained via a call to OSSEMPAHORE_Obtain.
	@param	s	(in)	Sempahore Object.
	@return OSStatus_t	Status of operation.
**/

#ifdef HISTORY_LOGGING
#define	OSSEMAPHORE_Release(s)	OSSEMAPHORE_ReleaseDebug(s, __FILE__, __LINENUM__)
OSStatus_t OSSEMAPHORE_ReleaseDebug(	// wait to release a semaphore
	Semaphore_t s,						// semaphore pointer
	char *file,							// file that calls OSSEMAPHORE_Obtain()
	UInt32 line							// line number in above file
	);
#else // #ifdef HISTORY_LOGGING
OSStatus_t OSSEMAPHORE_Release(			// wait to release a semaphore
	Semaphore_t s						// semaphore pointer
	);
#endif // #ifdef HISTORY_LOGGING

/**
	Create a Mutex.
	@param	mode	(in)	Suspension Mode
	@return Semaphore_t		Semaphore Object.
**/
 
#ifdef HISTORY_LOGGING
#define	OSSEMAPHORE_createMutex(mode)	OSSEMAPHORE_createMutexDebug(mode, __FILE__, __LINENUM__)
Semaphore_t OSSEMAPHORE_createMutexDebug(	// returns newly-created "mutex"
	OSSuspend_t mode,						// suspending task mode
	char *file,								// file that calls OSSEMAPHORE_Obtain()
	UInt32 line								// line number in above file
	);
#else // #ifdef HISTORY_LOGGING
Semaphore_t OSSEMAPHORE_createMutex(	// returns newly-created "mutex"
	OSSuspend_t mode					// suspending task mode
	);
#endif // #ifdef HISTORY_LOGGING

/**
	Deallocate and destroy resources associated with a Mutex.
	@param	s	(in)	Sempahore Object
**/

#ifdef HISTORY_LOGGING
#define	OSSEMAPHORE_destroyMutex(s)	OSSEMAPHORE_destroyMutexDebug(s, __FILE__, __LINENUM__)
void OSSEMAPHORE_destroyMutexDebug( 		// Destroy a "mutex"
	Semaphore_t s,							// "Mutex" to destroy
	char *file,								// file that calls OSSEMAPHORE_Obtain()
	UInt32 line								// line number in above file
	);
#else // #ifdef HISTORY_LOGGING
void OSSEMAPHORE_destroyMutex( 			// Destroy a "mutex"
	Semaphore_t s						// "Mutex" to destroy
	);
#endif // #ifdef HISTORY_LOGGING

/**
	Obtain a mutex.  If the mutex is already "locked" the calling task
	will block until the mutex is "unlocked".
	@param s (in)	Sempahore Object.
	@param timeout (in) Ticks to wait for mutex.
	@return OSSTATUS_t	Status of operation.
**/

#ifdef HISTORY_LOGGING
#define	OSSEMAPHORE_obtainMutex(s, timeout)	OSSEMAPHORE_obtainMutexDebug(s, timeout, __FILE__, __LINENUM__)
OSStatus_t OSSEMAPHORE_obtainMutexDebug(	// wait to obtain a "mutex"
	Semaphore_t s,							// "Mutex" pointer
	Ticks_t timeout,						// timeout to failed "mutex" obtain
	char *file,								// file that calls OSSEMAPHORE_Obtain()
	UInt32 line								// line number in above file
	);
#else // #ifdef HISTORY_LOGGING
OSStatus_t OSSEMAPHORE_obtainMutex(		// wait to obtain a "mutex"
	Semaphore_t s,						// "Mutex" pointer
	Ticks_t timeout						// timeout to failed "mutex" obtain
	);
#endif // #ifdef HISTORY_LOGGING

/**
	Release a mutex that was obtained via a call to OSSEMPAHORE_obtainMutex.
	@param	s	(in)	Sempahore Object.
	@return OSStatus_t	Status of operation.
**/


#ifdef HISTORY_LOGGING
#define	OSSEMAPHORE_releaseMutex(s)	OSSEMAPHORE_releaseMutexDebug(s, __FILE__, __LINENUM__)
OSStatus_t OSSEMAPHORE_releaseMutexDebug(	// wait to release a "mutex"
	Semaphore_t s,							// "mutex" pointer
	char *file,								// file that calls OSSEMAPHORE_Obtain()
	UInt32 line								// line number in above file
	);
#else // #ifdef HISTORY_LOGGING
OSStatus_t OSSEMAPHORE_releaseMutex(	// wait to release a "mutex"
	Semaphore_t s						// "mutex" pointer
	);
#endif //#ifdef HISTORY_LOGGING
/**
	Get current count of a counting semaphore.
	@param passed_in_s	(in)	Semaphore Object.
	@return UInt32	Semaphore count.
**/

UInt32 OSSEMAPHORE_GetCnt(				// return count of semaphore,
	Semaphore_t passed_in_s				// if count <= 0, return -1
	);


/**
	Change the name of an existing semaphore.
	@param s	(in)	Semaphore Object.
	@param name	(in)	New name of the semaphore.
**/

void OSSEMAPHORE_ChangeName(
	Semaphore_t s,
	const char* name
	);

/**
	Get the name of an existing semaphore.
	@param s		(in)	Semaphore Object.
	@param p_name	(in)	Name storage (minimum 9 characters).
	@return OSStatus_t	Status of operation.
**/

OSStatus_t OSSEMAPHORE_GetName(		// get ASCII name of semaphore
	Semaphore_t s,					// semaphore pointer
	UInt8 *p_name					// location to store the ASCII name
	);

/** @} */

#endif
