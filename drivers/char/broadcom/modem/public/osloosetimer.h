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
*   @file   osloosetimer.h
*
*   @brief  This file contains the interface to the Low Impact Timer functionality.  The Low 
*   Impact timers work similarly to normal timers except that they will not wake the system
*   from sleep, and they do not expire until the system background task runs.   They also
*   have a 500ms granularity, depending on system parameters.
*   
*   This means that the timers may be delayed significantly from their scheduled expiration 
*   time and should not be used where greater accuracy is required.
*
****************************************************************************/
/**

*   @defgroup   RTOSTimerGroup   Timer Services
*   @ingroup    RTOSGroup
*
*	The Timer API provides functions to create/destroy and control software
	timers.



*
****************************************************************************/
/**
 * @addtogroup RTOSTimerGroup
 * @{
 */

#ifndef _RTOS_OSLOOSETIMER_H_
#define _RTOS_OSLOOSETIMER_H_

#include "types.h"
#include "ostypes.h"
#include "ostimer.h"

//******************************************************************************
// Global Macros
//******************************************************************************

#define	OSLOOSETIMER_DEBUG_ID					0x544D4447	//TMDG


//******************************************************************************
// Global Typedefs
//******************************************************************************

/// Timer object returned from call to OSLOOSETIMER_Create.

typedef void* LooseTimer_t;


//******************************************************************************
// Global Function Prototypes
//******************************************************************************



#ifdef HISTORY_LOGGING
/**
	Create a timer that will call a function when the timer expires.  The timer
	can be either a one-shot or periodic timer.  Timer functions should be treated
	the same as an Interrupt Service Routine.  Timers will preempt tasks so processing
	done in the timer function will effect timing of high priority tasks.

	@param entry		(in)	Function that is called when timer expires.
	@param id			(in)	Id associated with timer.  This will be passed to the
								timer function when the timer expires.
	@param init_time	(in)	Number of Ticks until first time timer expires.
	@param repeat_time	(in)	Timer interval for periodic timers.
	@return LooseTimer_t	Timer object.
**/
#define	OSLOOSETIMER_Create(entry, id, init_time, repeat_time)	\
		OSLOOSETIMER_CreateDebug(entry, id, init_time, repeat_time, __FILE__, __LINENUM__)
LooseTimer_t OSLOOSETIMER_CreateDebug(	// returns a pointer to created task
	TimerEntry_t entry,					// timer task function entry point
	TimerID_t id,						// timer task ID (optional)
	Ticks_t init_time,					// timer task, first timeout
	Ticks_t repeat_time,				// timer task interval
	char *file,							// file that calls OSLOOSETIMER_Create()
	UInt32 line							// line number in above file
	);
#else // #ifdef HISTORY_LOGGING
/**
	Create a timer that will call a function when the timer expires.  The timer
	can be either a one-shot or periodic timer.  Timer functions should be treated
	the same as an Interrupt Service Routine.  Timers will preempt tasks so processing
	done in the timer function will effect timing of high priority tasks.

	@param entry		(in)	Function that is called when timer expires.
	@param id			(in)	Id associated with timer.  This will be passed to the
								timer function when the timer expires.
	@param init_time	(in)	Number of Ticks until first time timer expires.
	@param repeat_time	(in)	Timer interval for periodic timers.
	@return LooseTimer_t	Timer object.
**/
LooseTimer_t OSLOOSETIMER_Create(					// returns a pointer to created task
	TimerEntry_t entry,					// timer task function entry point
	TimerID_t id,						// timer task ID (optional)
	Ticks_t init_time,					// timer task, first timeout
	Ticks_t repeat_time					// timer task interval
	);
#endif // #ifdef HISTORY_LOGGING



#ifdef HISTORY_LOGGING
/**
	Start a timer.
	@param timer	(in)	Timer object.
**/
#define	OSLOOSETIMER_Start(timer)	\
		OSLOOSETIMER_StartDebug(timer, __FILE__, __LINENUM__)
void OSLOOSETIMER_StartDebug(				// enable/disable timer task
	LooseTimer_t timer,						// pointer to periodic timer
	char *file,							// file that calls OSLOOSETIMER_Start()
	UInt32 line							// line number in above file
	);
#else // #ifdef HISTORY_LOGGING
/**
	Start a timer.
	@param timer	(in)	Timer object.
**/
void OSLOOSETIMER_Start(						// enable/disable timer task
	LooseTimer_t timer						// pointer to periodic timer
	);
#endif // #ifdef HISTORY_LOGGING



#ifdef HISTORY_LOGGING
/**
	Stop a timer.
	@param timer	(in)	Timer object.
**/
#define	OSLOOSETIMER_Stop(timer)	\
		OSLOOSETIMER_StopDebug(timer, __FILE__, __LINENUM__)
void OSLOOSETIMER_StopDebug(					// enable/disable timer task
	LooseTimer_t timer,						// pointer to periodic timer
	char *file,							// file that calls OSLOOSETIMER_Stop()
	UInt32 line							// line number in above file
	);
#else // #ifdef HISTORY_LOGGING
/**
	Stop a timer.
	@param timer	(in)	Timer object.
**/
void OSLOOSETIMER_Stop(						// enable/disable timer task
	LooseTimer_t timer						// pointer to periodic timer
	);
#endif // #ifdef HISTORY_LOGGING



#ifdef HISTORY_LOGGING
/** Reconfigure an existing timer.
	@param timer		(in)	Timer object
	@param init_time	(in)	Initial timeout
	@param repeat_time	(in)	Interval for periodic timers
	@param start		(in)	If TRUE the timer is started with new parameters
								otherwise the timer remains stopped and can be
								started with a call to OSLOOSETIMER_Start
**/
#define	OSLOOSETIMER_Reconfig(timer, init_time, repeat_time, start)	\
		OSLOOSETIMER_ReconfigDebug(timer, init_time, repeat_time, start, __FILE__, __LINENUM__)
void OSLOOSETIMER_ReconfigDebug(
	LooseTimer_t timer,						// timer object
	Ticks_t init_time,					// first timeout
	Ticks_t repeat_time,				// Interval for periodic timers
	Boolean	start,						// TRUE = Start timer running
										// FALSE = Leave timer stopped.
	char *file,							// file that calls OSLOOSETIMER_Reconfig()
	UInt32 line							// line number in above file
	);
#else // #ifdef HISTORY_LOGGING
/** Reconfigure an existing timer.
	@param timer		(in)	Timer object
	@param init_time	(in)	Initial timeout
	@param repeat_time	(in)	Interval for periodic timers
	@param start		(in)	If TRUE the timer is started with new parameters
								otherwise the timer remains stopped and can be
								started with a call to OSLOOSETIMER_Start
**/
void OSLOOSETIMER_Reconfig(
	LooseTimer_t timer,						// timer object
	Ticks_t init_time,					// first timeout
	Ticks_t repeat_time,				// Interval for periodic timers
	Boolean	start						// TRUE = Start timer running
										// FALSE = Leave timer stopped.
	);
#endif // #ifdef HISTORY_LOGGING



#ifdef HISTORY_LOGGING
/**
	Stop, destroy, and deallocate the resources associated with a timer.
	@param timer	(in)	Timer object to destroy.
**/
#define	OSLOOSETIMER_Destroy(timer)	\
		OSLOOSETIMER_DestroyDebug(timer, __FILE__, __LINENUM__)
void OSLOOSETIMER_DestroyDebug(				// Destroy a previous created timer
	LooseTimer_t timer,						// pointer to periodic timer
	char *file,							// file that calls OSLOOSETIMER_Destroy()
	UInt32 line							// line number in above file
	);
#else // #ifdef HISTORY_LOGGING
/**
	Stop, destroy, and deallocate the resources associated with a timer.
	@param timer	(in)	Timer object to destroy.
**/
void OSLOOSETIMER_Destroy(					// Destroy a previous created timer
	LooseTimer_t timer						// pointer to periodic timer
	);
#endif // #ifdef HISTORY_LOGGING

/**
	Get the remaining time of a timer.
	@param timer		(in) Timer object
	@return UInt32	Remaining time of a timer
**/

UInt32 OSLOOSETIMER_GetRemainingTime(
	LooseTimer_t timer						// timer object
	);


/**
	Change the name of an existing timer.
	@param	timer	(in)	Timer object
	@param	name	(in)	New name of the timer
**/

void OSLOOSETIMER_ChangeName(
	LooseTimer_t timer,
	const char* name
	);

/** @} */

#endif

