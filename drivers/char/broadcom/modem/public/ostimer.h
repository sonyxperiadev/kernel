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
*   @file   ostimer.h
*
*   @brief  This file contains the interface to the Timer functionality.
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


#ifndef _RTOS_OSTIMER_H_
#define _RTOS_OSTIMER_H_

//#include "mobcom_types.h"
#include "ostypes.h"

//******************************************************************************
// Global Macros
//******************************************************************************

/// Do Not repeat timer task after first timeout.

#define NO_REPEAT_TIME					0	// do not repeat timer task, after
											// first timeout

#define	ENUM_TIMER_HISTORY_CREATE			1
#define	ENUM_TIMER_HISTORY_START			2
#define	ENUM_TIMER_HISTORY_STOP				3
#define	ENUM_TIMER_HISTORY_RECONFIG			4
#define	ENUM_TIMER_HISTORY_DESTROY			5
#define	ENUM_TIMER_HISTORY_EXPIRE			6
#define	ENUM_TIMER_HISTORY_SLEEP			7
#define	ENUM_TIMER_HISTORY_WAKEUP			8

#define	OSTIMER_DEBUG_ID					0x544D4447	//TMDG


//******************************************************************************
// Global Typedefs
//******************************************************************************

/// Identifier associated with timer.

typedef UInt32 TimerID_t;

/// A timer function is invoked with a timer expires.

typedef void (* TimerEntry_t)( TimerID_t id );

typedef struct TimerNode
{
	UInt32				tcb[OS_TIMER_SIZE];

	TimerEntry_t		entry;			// Pointer to timer callback
	Ticks_t				init_time;		// Initial time out
	Ticks_t				repeat_time;	// subsequent timeouts
	TimerID_t			id;				// Timer ID (optional)

	UInt32				debug_tcb_id;

	Task_t				create_task;
	char				*create_file;
	UInt32				create_line;
	UInt32				create_time;

	Task_t				start_task;
	char				*start_file;
	UInt32				start_line;
	UInt32				start_time;

	Task_t				stop_task;
	char				*stop_file;
	UInt32				stop_line;
	UInt32				stop_time;

	Task_t				reconfig_task;
	char				*reconfig_file;
	UInt32				reconfig_line;
	UInt32				reconfig_time;

	char				name[OS_MAX_NAME_LEN];	//reference timer name
	Ticks_t				interval_duration;		// Dration of current interval (zero when timer is inactive)
	Ticks_t				interval_start;			// Start of current interval
	struct TimerNode*	next_active;			// List of active timers
} TimerStruct_t;

/**
 * @addtogroup RTOSTimerGroup
 * @{
 */

//******************************************************************************
// Global Function Prototypes
//******************************************************************************

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
	@return Timer_t	Timer object.
**/

#ifdef HISTORY_LOGGING
#define	OSTIMER_Create(entry, id, init_time, repeat_time)	\
		OSTIMER_CreateDebug(entry, id, init_time, repeat_time, __FILE__, __LINENUM__)
Timer_t OSTIMER_CreateDebug(			// returns a pointer to created task
	TimerEntry_t entry,					// timer task function entry point
	TimerID_t id,						// timer task ID (optional)
	Ticks_t init_time,					// timer task, first timeout
	Ticks_t repeat_time,				// timer task interval
	char *file,							// file that calls OSTIMER_Create()
	UInt32 line							// line number in above file
	);
#else // #ifdef HISTORY_LOGGING
Timer_t OSTIMER_Create(					// returns a pointer to created task
	TimerEntry_t entry,					// timer task function entry point
	TimerID_t id,						// timer task ID (optional)
	Ticks_t init_time,					// timer task, first timeout
	Ticks_t repeat_time					// timer task interval
	);
#endif // #ifdef HISTORY_LOGGING

/**
	Start a timer.
	@param timer	(in)	Timer object.
**/

#ifdef HISTORY_LOGGING
#define	OSTIMER_Start(timer)	\
		OSTIMER_StartDebug(timer, __FILE__, __LINENUM__)
void OSTIMER_StartDebug(				// enable/disable timer task
	Timer_t timer,						// pointer to periodic timer
	char *file,							// file that calls OSTIMER_Start()
	UInt32 line							// line number in above file
	);
#else // #ifdef HISTORY_LOGGING
void OSTIMER_Start(						// enable/disable timer task
	Timer_t timer						// pointer to periodic timer
	);
#endif // #ifdef HISTORY_LOGGING

/**
	Stop a timer.
	@param timer	(in)	Timer object.
**/

#ifdef HISTORY_LOGGING
#define	OSTIMER_Stop(timer)	\
		OSTIMER_StopDebug(timer, __FILE__, __LINENUM__)
void OSTIMER_StopDebug(					// enable/disable timer task
	Timer_t timer,						// pointer to periodic timer
	char *file,							// file that calls OSTIMER_Stop()
	UInt32 line							// line number in above file
	);
#else // #ifdef HISTORY_LOGGING
void OSTIMER_Stop(						// enable/disable timer task
	Timer_t timer						// pointer to periodic timer
	);
#endif // #ifdef HISTORY_LOGGING

/** Stop a timer and reset it to the initial timeout value.
	@param timer	(in)	Timer object.
**/

#ifdef HISTORY_LOGGING
#define	OSTIMER_Reset(timer)	\
		OSTIMER_ResetDebug(timer, __FILE__, __LINENUM__)
void OSTIMER_ResetDebug(				// reset the initial timeout
	Timer_t timer,						// pointer to periodic timer
	char *file,							// file that calls OSTIMER_Reset()
	UInt32 line							// line number in above file
	);
#else // #ifdef HISTORY_LOGGING
void OSTIMER_Reset(						// reset the initial timeout
	Timer_t timer						// pointer to periodic timer
	);
#endif // #ifdef HISTORY_LOGGING

/** Reconfigure an existing timer.
	@param timer		(in)	Timer object
	@param init_time	(in)	Initial timeout
	@param repeat_time	(in)	Interval for periodic timers
	@param start		(in)	If TRUE the timer is started with new parameters
								otherwise the timer remains stopped and can be
								started with a call to OSTIMER_Start
**/

#ifdef HISTORY_LOGGING
#define	OSTIMER_Reconfig(timer, init_time, repeat_time, start)	\
		OSTIMER_ReconfigDebug(timer, init_time, repeat_time, start, __FILE__, __LINENUM__)
void OSTIMER_ReconfigDebug(
	Timer_t timer,						// timer object
	Ticks_t init_time,					// first timeout
	Ticks_t repeat_time,				// Interval for periodic timers
	Boolean	start,						// TRUE = Start timer running
										// FALSE = Leave timer stopped.
	char *file,							// file that calls OSTIMER_Reconfig()
	UInt32 line							// line number in above file
	);
#else // #ifdef HISTORY_LOGGING
void OSTIMER_Reconfig(
	Timer_t timer,						// timer object
	Ticks_t init_time,					// first timeout
	Ticks_t repeat_time,				// Interval for periodic timers
	Boolean	start						// TRUE = Start timer running
										// FALSE = Leave timer stopped.
	);
#endif // #ifdef HISTORY_LOGGING

/**
	Stop, destroy, and deallocate the resources associated with a timer.
	@param timer	(in)	Timer object to destroy.
**/

#ifdef HISTORY_LOGGING
#define	OSTIMER_Destroy(timer)	\
		OSTIMER_DestroyDebug(timer, __FILE__, __LINENUM__)
void OSTIMER_DestroyDebug(				// Destroy a previous created timer
	Timer_t timer,						// pointer to periodic timer
	char *file,							// file that calls OSTIMER_Destroy()
	UInt32 line							// line number in above file
	);
#else // #ifdef HISTORY_LOGGING
void OSTIMER_Destroy(					// Destroy a previous created timer
	Timer_t timer						// pointer to periodic timer
	);
#endif // #ifdef HISTORY_LOGGING

/**
	Retrieve system clock value in milliseconds.
	@return	system time in ms (UInt32)
**/

UInt32 OSTIMER_RetrieveClock ( void ) ;	// get clock --- added for sct 3.5

//MobC00075444 Begin, Hui Luo, 5/15/09
/**
	maintain high 32-bit of 64-bit clock.
**/

void OSTIMER_MaintainClock64 ( void ) ;	// maintain high 32-bit of 64-bit clock

/**
	Retrieve system clock value in milliseconds in two 32-bit values.
	@return	system time in ms in two 32-bit integers
**/

void OSTIMER_RetrieveClock64 ( UInt32*, UInt32* ) ;
//MobC00075444 End, Hui Luo, 5/15/09

/**
	Get the remaining time of a timer.
	@param timer		(in) Timer object
	@return UInt32	Remaining time of a timer
**/

UInt32 OSTIMER_GetRemainingTime(
	Timer_t timer						// timer object
	);

/**
	Convert a time in seconds and nanoseconds to timer ticks.
	@param	s	(in)	seconds
	@param	ns	(in)	nano seconds
	@return Timer ticks.
**/

UInt32 OSTIMER_Time2Ticks (				// time in seconds/nseconds to
	UInt32 s,							// ticks --- added for sct 3.5
	UInt32 ns
	);

/**
	Change the name of an existing timer.
	@param	timer	(in)	Timer object
	@param	name	(in)	New name of the timer
**/

void OSTIMER_ChangeName(
	Timer_t timer,
	const char* name
	);

/**
	Get the name of an existing timer.
	@param	t		(in)	Timer object
	@param	p_name	(in)	Name storage (minimum 9 characters)
	@return OSStatus_t		Status of operation.
**/

OSStatus_t OSTIMER_GetName(		// get ASCII name of Timer
	Timer_t t,					// Timer pointer
	UInt8 *p_name				// location to store the ASCII name
	);

/** @} */

#endif
