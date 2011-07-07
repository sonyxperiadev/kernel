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
*   @file   ostask.h
*
*   @brief  This file contains the interface to the Task functionality.
*
****************************************************************************/
/**

*   @defgroup   RTOSTaskGroup   Task Services
*   @ingroup    RTOSGroup
*
*	The Task API provides interfaces to the create and control tasks.  Tasks
	are separately scheduled threads of execution that have their own context.
*
****************************************************************************/

/**
 * @addtogroup RTOSTaskGroup
 * @{
 */

#ifndef _RTOS_OSTASK_H_
#define _RTOS_OSTASK_H_

//#include "mobcom_types.h"
#include "ostypes.h"
//#if (defined(PROCESSOR_MODEM)&&defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE)&&!defined(STANDALONE))
//#include "ossemaphore.h"
//#endif
#define  MAX_NU_TASK		200

//******************************************************************************
// Global Macros
//******************************************************************************

//******************************************************************************
// Global Typedefs
//******************************************************************************
#ifdef WIN32

//#include <windows.h>
#if 0
typedef enum { 
	IDLE			= THREAD_PRIORITY_IDLE,
	LOWEST			= THREAD_PRIORITY_LOWEST,
	BELOW_NORMAL	= THREAD_PRIORITY_BELOW_NORMAL,
	NORMAL			= THREAD_PRIORITY_NORMAL,
	ABOVE_NORMAL	= THREAD_PRIORITY_ABOVE_NORMAL,
	HIGHEST			= THREAD_PRIORITY_HIGHEST
} TPriority_t;
#endif
typedef enum { 
	IDLE			= 0,
	LOWEST			= 1,
	BELOW_NORMAL	= 2,
	NORMAL			= 3,
	ABOVE_NORMAL	= 4,
	HIGHEST			= 5
} TPriority_t;
#else

/**	
	Task priorities can be fine tuned by incrementing or decrementing the constants defined
	by TPriority_t.  Decrementing increases the priority and Incrementing decreases the priority.
	Tasks of lower priority can be preempted by tasks of higher priority.  Tasks of the same 
	priority are scheduled in a FIFO fashion.
**/
#ifdef FUSE_DUAL_PROCESSOR_ARCHITECTURE
typedef enum { 
	IDLE			= 70,	// Unused, JPG
	LOWEST			= 60,	
	BELOW_NORMAL	= 37,
	NORMAL			= 30,
	ABOVE_NORMAL	= 20,
	HIGHEST			= 11
} TPriority_t;
#else
typedef enum { 
	IDLE			= 50,
	LOWEST			= 40,
	BELOW_NORMAL	= 30,
	NORMAL			= 20,
	ABOVE_NORMAL	= 10,
	HIGHEST			= 0
} TPriority_t;
#endif

#endif

/// 8 characters + NULL

#define TASK_NAME_SIZE	9		//	8 characters + NULL

/// Task Name is a NULL terminated string, up to TASK_NAME_SIZE

typedef UInt8 *TName_t;

/// Identifies how many arguments are in the TArgv_t list.

typedef UInt32 TArgc_t;

/// Argument list passed to task entry point.
 
typedef void *TArgv_t;

/// Task entry point which has parameters.

typedef void (*TEntry_t)( void );

/// Task entry point which has no-parameters.

typedef void (*TEntryWArg_t)( TArgc_t, TArgv_t );

/// Task Stack Size.

typedef UInt32 TStackSize_t;


//******************************************************************************
// Global Function Prototypes
//******************************************************************************

/**	
	Creates a task without arguments.
	@param entry		(in) Entry point for task.
	@param task_name	(in) Tasks Name.
	@param priority		(in) Tasks priority, bigger is higher.
	@param stack_size	(in) Initial size of tasks stack.
	@return Task_t		A handle to a task object.
**/  


#ifdef HISTORY_LOGGING
#define	OSTASK_Create(e, n, p, s)	OSTASK_CreateDebug(e, n, p, s, __FILE__, __LINENUM__)

Task_t OSTASK_CreateDebug(				// returns the newly-created task
	TEntry_t entry,						// task function entry point
	TName_t task_name,					// task name
	TPriority_t priority,				// task priority, bigger is higher
	TStackSize_t stack_size,			// task stack size (in UInt8)
	char *file,							// file name in which the code calls OSTASK_Create()
	UInt32 line							// line number
	);
#else // #ifdef HISTORY_LOGGING
Task_t OSTASK_Create(					// returns the newly-created task
	TEntry_t entry,						// task function entry point
	TName_t task_name,					// task name
	TPriority_t priority,				// task priority, bigger is higher
	TStackSize_t stack_size				// task stack size (in UInt8)
	);
#endif // #ifdef HISTORY_LOGGING

/**	
	Creates a task with arguments and put it into ready state.
	@param entry		(in) Entry point for task.
	@param task_name	(in) Tasks Name.
	@param priority		(in) Tasks priority, bigger is higher.
	@param stack_size	(in) Initial size of tasks stack.
	@param argc			(in) Number of arguments in argument list.
	@param argv			(in) Argument List
	@return Task_t		A handle to a task object.
**/  

#ifdef HISTORY_LOGGING
#define	OSTASK_CreateWArg(e, n, p, s, c, v) OSTASK_CreateWArgDebug(e, n, p, s, c, v, __FILE__, __LINENUM__)

Task_t OSTASK_CreateWArgDebug(			// returns the newly-created task
	TEntryWArg_t entry,					// task function entry point
	TName_t  task_name,					// task name
	TPriority_t priority,				// task priority, bigger is higher
	TStackSize_t stack_size,			// task stack size (in UInt8)
	TArgc_t argc,						// argument count or for general use
	TArgv_t argv,						// argv pointer
	char *file,							// file name in which the code calls OSTASK_CreateWArg()
	UInt32 line							// line number
	);
#else // #ifdef HISTORY_LOGGING
Task_t OSTASK_CreateWArg(				// returns the newly-created task
	TEntryWArg_t entry,					// task function entry point
	TName_t  task_name,					// task name
	TPriority_t priority,				// task priority, bigger is higher
	TStackSize_t stack_size,			// task stack size (in UInt8)
	TArgc_t argc,						// argument count or for general use
	TArgv_t argv						// argv pointer
	);
#endif // #ifdef HISTORY_LOGGING

/**	
	Creates a task with arguments and put it into suspend state.
	@param entry		(in) Entry point for task.
	@param task_name	(in) Tasks Name.
	@param priority		(in) Tasks priority, bigger is higher.
	@param stack_size	(in) Initial size of tasks stack.
	@param argc			(in) Number of arguments in argument list.
	@param argv			(in) Argument List
	@return Task_t		A handle to a task object.
**/  

#ifdef HISTORY_LOGGING
#ifndef _OSTASK_C
#define	OSTASK_CreateSuspend(e, n, p, s, c, v) OSTASK_CreateSuspendDebug(e, n, p, s, c, v, __FILE__, __LINENUM__)
#endif

Task_t OSTASK_CreateSuspendDebug(		// returns the newly-created task
	TEntryWArg_t entry,					// task function entry point
	TName_t  task_name,					// task name
	TPriority_t priority,				// task priority, bigger is higher
	TStackSize_t stack_size,			// task stack size (in UInt8)
	TArgc_t argc,						// argument count or for general use
	TArgv_t argv,						// argv pointer
	char *file,							// file name in which the code calls OSTASK_CreateSuspend()
	UInt32 line							// line number
	);
#else // #ifdef HISTORY_LOGGING
Task_t OSTASK_CreateSuspend(			// returns the newly-created task
	TEntryWArg_t entry,					// task function entry point
	TName_t  task_name,					// task name
	TPriority_t priority,				// task priority, bigger is higher
	TStackSize_t stack_size,			// task stack size (in UInt8)
	TArgc_t argc,						// argument count or for general use
	TArgv_t argv						// argv pointer
	);
#endif // #ifdef HISTORY_LOGGING

/**
	Destroys a task.  Prior to destroying a task the resources the task has
	allocated should be released otherwise they can not be recovered.
	@param t (in) Handle to a task object.
**/

#ifdef HISTORY_LOGGING
#define	OSTASK_Destroy(t)	OSTASK_DestroyDebug(t, __FILE__, __LINENUM__)

void OSTASK_DestroyDebug(				// Terminate and destroy all the
	Task_t	t,							// task pointer
	char *file,							// file name in which the code calls OSTASK_Destroy()
	UInt32 line							// line number
	);
#else // #ifdef HISTORY_LOGGING
void OSTASK_Destroy(					// Terminate and destroy all the
	Task_t	t							// task pointer
	);
#endif // #ifdef HISTORY_LOGGING

/**
	Suspend the task for a specified number of timer ticks.
	@param timeout	(in) Number of ticks to suspend the task.
**/
#ifdef L1TEST
#else
#ifdef HISTORY_LOGGING
#define	OSTASK_Sleep(t)	OSTASK_SleepDebug(t, __FILE__, __LINENUM__)

void OSTASK_SleepDebug(					// suspend task, until timeout
	Ticks_t timeout,					// task sleep timeout
	char *file,							// file name in which the code calls OSTASK_Sleep()
	UInt32 line							// line number
	);
#else // #ifdef HISTORY_LOGGING
void OSTASK_Sleep(						// suspend task, until timeout
	Ticks_t timeout						// task sleep timeout
	);
#endif // #ifdef HISTORY_LOGGING
#endif

/**
	Change the priority of a task.
	@param t (in) Task Object.
	@param new_priority	(in) Task's new priority.
	@return Task's old priority.
**/
										// returns previous task priority
#ifdef HISTORY_LOGGING
#ifndef _OSTASK_C
#define	OSTASK_ChangePriority(t, p)	OSTASK_ChangePriorityDebug(t, p, __FILE__, __LINENUM__)
#endif

TPriority_t OSTASK_ChangePriorityDebug(	// change task priority
	Task_t	t,							// task pointer
	TPriority_t new_priority,			// new task priority
	char *file,							// file name in which the code calls OSTASK_ChangePriority()
	UInt32 line							// line number
	);
#else // #ifdef HISTORY_LOGGING
TPriority_t OSTASK_ChangePriority(		// change task priority
	Task_t	t,							// task pointer
	TPriority_t new_priority			// new task priority
	);
#endif // #ifdef HISTORY_LOGGING

/**
	Suspend the task until resumed.
	@param t	(in) Task Handle.
**/

#ifdef HISTORY_LOGGING
#define	OSTASK_Suspend(t)	OSTASK_SuspendDebug(t, __FILE__, __LINENUM__)

void OSTASK_SuspendDebug(				// suspend task until resumed;
	Task_t	t,							// task pointer
	char *file,							// file name in which the code calls OSTASK_Suspend()
	UInt32 line							// line number
	);
#else // #ifdef HISTORY_LOGGING
void OSTASK_Suspend(					// suspend task until resumed;
	Task_t	t							// task pointer
	);
#endif // #ifdef HISTORY_LOGGING

/**
	Resume a suspended task.
	@param t	(in) Task Handle.
**/

#ifdef HISTORY_LOGGING
#ifndef _OSTASK_C
#define	OSTASK_Resume(t)	OSTASK_ResumeDebug(t, __FILE__, __LINENUM__)
#endif

void OSTASK_ResumeDebug(				// resume suspended task;
	Task_t	t,							// task pointer
	char *file,							// file name in which the code calls OSTASK_Resume()
	UInt32 line							// line number
	);
#else // #ifdef HISTORY_LOGGING
void OSTASK_Resume(						// resume suspended task;
	Task_t	t							// task pointer
	);
#endif // #ifdef HISTORY_LOGGING

/**
	Get the current executing task.  If no task is currently exeucting NULL is returned.
	@return Task_t	Handle to current Task
**/

Task_t OSTASK_GetCurrentTask( void );	// get currently running task

/**
	Get the name associated with the passed task object.
	@param t	(in) Task Handle
	@param *p_name	(out) Pointer to store task name.
**/

OSStatus_t OSTASK_GetTaskName(			// get ASCII name of task
	Task_t t,							// task pointer
	UInt8 *p_name						// location to store the ASCII name
	);

/**
	Get the name pointer associated with the current task object.
**/

char* OSTASK_GetCurrentTaskNamePtr(	// get current task name pointer
	void
	);

/**
	Disable all preemption.  While disabled the scheduler will not pre-empt the currently
	executing task even if a higher priority task is ready to run.
**/

#ifdef HISTORY_LOGGING
#define	OSTASK_DisablePreemption()	OSTASK_DisablePreemptionDebug(__FILE__, __LINENUM__)

void OSTASK_DisablePreemptionDebug( char*, UInt32 );
#else // #ifdef HISTORY_LOGGING
void OSTASK_DisablePreemption( void );
#endif // #ifdef HISTORY_LOGGING

/**
	Enable all preemption.  When enabled the scheduler will always schedule the highest
	priority ready task to run.
**/

#ifdef HISTORY_LOGGING
#define	OSTASK_RestorePreemption()	OSTASK_RestorePreemptionDebug(__FILE__, __LINENUM__)

void OSTASK_RestorePreemptionDebug( char*, UInt32 );
#else // #ifdef HISTORY_LOGGING
void OSTASK_RestorePreemption( void );
#endif // #ifdef HISTORY_LOGGING

/**
	@cond internal
	Get a pointer to the current stack pointer.  
	@param	task	(in) Task handle
**/

UInt32* OSTASK_GetSP(		//Get the stack pointer for a given task
	Task_t task
	);

//CQ MobC00066446 Begin, Hui Luo, 2/22/09
/**
	Get current context: task, HISR, LISR, or RTOS kernel?  
**/

#define	OSTASK_CONTEXT_TASK		0
#define	OSTASK_CONTEXT_HISR		1
#define	OSTASK_CONTEXT_LISR		2
#define	OSTASK_CONTEXT_RTOS		3
#define	OSTASK_CONTEXT_ERROR	-1

int OSTASK_GetContext(void);
//CQ MobC00066446 End, Hui Luo, 2/22/09

Boolean OSTASK_IsValidTask(Task_t  task);
TPriority_t OSTASK_GetCurrentTaskPriority( void );


/**
	Check if a given TCB points to a valid task
	@param task	(in) Task Handle
	@return Boolean  Yes/No
**/
Boolean OSTASK_IsValidTask(Task_t task);

/**
	Check if a given TCB points to a valid HISR
	@param task	(in) Task Handle
	@return Boolean  Yes/No
**/
Boolean OSTASK_IsValidHISR(Task_t task);

/**
	Check if a given TCB points to a valid task or a HISR
	@param task	(in) Task Handle
	@return Boolean  Yes/No
**/
Boolean OSTASK_IsValidTaskOrHISR(Task_t task);

/**
	Get the task/HISR name from a given TCB
	@param task	(in) Task Handle
	@return char* pointer pointing to the task/HISR name
**/
char* OSTASK_GetNamePtr(Task_t task);

/**
	Get the TCB index so that the TCB can be quickly located from a TCB array.
	@param task	(in) Task Handle
	@return UInt32 Task index
**/
UInt32 OSTASK_GetProfilingIndex(Task_t task);

/**
	Set a TCB index in TCB so that the TCB can be quickly located from a TCB array.
	@param task	(in) Task Handle
	@param index(in) Task index
	@return UInt32 Task index
**/
void OSTASK_SetProfilingIndex(Task_t task, UInt32 index);

/**
	Check if a given task/HISR is in ready state
	@param task	(in) Task Handle
	@return Boolean  Yes/No
**/
Boolean OSTASK_IsTaskReady(Task_t task);

/**
	Get a task's priority from its TCB
	@param task	(in) Task Handle
	@return TPriority_t Task priority (the smaller, the higher)
**/
TPriority_t OSTASK_GetPriority(Task_t task);


/** @endcond */

#endif // #ifndef _RTOS_OSTASK_H_

/** @} */
