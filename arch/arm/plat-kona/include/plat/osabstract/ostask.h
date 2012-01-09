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
*   @file   ostask.h
*
*   @brief  This file contains the public interface to the OSAL Task functionality.
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

/*
 * This file is taken from //depot/Sources/SystemIntegration/msp/soc/os/public/
 * on 6th Oct 2010
 */

/**
 * @addtogroup RTOSTaskGroup
 * @{
 */

#ifndef _OSTASK_H_
#define _OSTASK_H_

#include <plat/types.h>
#include <plat/osabstract/ostypes.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/delay.h>

#define  MAX_NU_TASK		200

//******************************************************************************
// Global Typedefs
//******************************************************************************

/**	
	Task priorities can be fine tuned by incrementing or decrementing the constants defined
	by TPriority_t.  Decrementing increases the priority and Incrementing decreases the priority.
	Tasks of lower priority can be preempted by tasks of higher priority.  Tasks of the same 
	priority are scheduled in a FIFO fashion.
**/
#ifdef FUSE_DUAL_PROCESSOR_ARCHITECTURE
typedef enum { 
	IDLE			= 250,	// only used by LOG task or any task with absolutely lowest priority
	LOWEST			= 60,	
	BELOW_NORMAL	= 37,
	NORMAL			= 30,
	ABOVE_NORMAL	= 20,
	HIGHEST			= 11
} TPriority_t;
#else
typedef enum { 
	IDLE			= 250,	// only used by LOG task or any task with absolutely lowest priority
	LOWEST			= 40,
	BELOW_NORMAL	= 30,
	NORMAL			= 20,
	ABOVE_NORMAL	= 10,
	HIGHEST			= 0
} TPriority_t;
#endif


#ifndef OSAL_USER_TASK_EXTENSION
#define OSAL_USER_TASK_EXTENSION    // Set to nothing if brcm_os_port.h not included
#endif

/** 8 characters + NULL */
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


static inline Task_t OSTASK_Create(					// returns the newly-created task
	TEntry_t entry,						// task function entry point
	TName_t task_name,					// task name
	TPriority_t priority,				// task priority, bigger is higher
	TStackSize_t stack_size				// task stack size (in UInt8)
	)
{
	struct task_struct *t;
	t = kthread_run((void *)entry, NULL, "%s", task_name);
	return IS_ERR(t) ? NULL : t;
}

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

static inline Task_t OSTASK_CreateWArg(				// returns the newly-created task
	TEntryWArg_t entry,					// task function entry point
	TName_t  task_name,					// task name
	TPriority_t priority,				// task priority, bigger is higher
	TStackSize_t stack_size,			// task stack size (in UInt8)
	TArgc_t argc,						// argument count or for general use
	TArgv_t argv						// argv pointer
	)
{
	return NULL;
}

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

static inline Task_t OSTASK_CreateSuspend(			// returns the newly-created task
	TEntryWArg_t entry,					// task function entry point
	TName_t  task_name,					// task name
	TPriority_t priority,				// task priority, bigger is higher
	TStackSize_t stack_size,			// task stack size (in UInt8)
	TArgc_t argc,						// argument count or for general use
	TArgv_t argv						// argv pointer
	)
{
	return NULL;
}

/**
	Destroys a task.  Prior to destroying a task the resources the task has
	allocated should be released otherwise they can not be recovered.
	@param t (in) Handle to a task object.
**/

static inline void OSTASK_Destroy(					// Terminate and destroy all the
	Task_t	t							// task pointer
	)
{
	BUG();
}

/**
	Terminatess a task.  
	@param t (in) Handle to a task object.
**/

static inline void OSTASK_Terminate(					// Terminate task
	Task_t	t							// task pointer
	)
{
	BUG();
}

/**
	Suspend the task for a specified number of timer ticks.
	@param timeout	(in) Number of ticks to suspend the task.
**/
static inline void OSTASK_Sleep(						// suspend task, until timeout
	Ticks_t timeout						// task sleep timeout
	)
{
	msleep(timeout);
}


/**
    Relinquishes the CPU to other tasks in the same priority.  Task is placed back on the
    ready queue immediately.
**/

static inline void OSTASK_Relinquish(void){}

/**
	Change the priority of a task.
	@param t (in) Task Object.
	@param new_priority	(in) Task's new priority.
	@return Task's old priority.
**/
										// returns previous task priority
static inline TPriority_t OSTASK_ChangePriority(		// change task priority
	Task_t	t,							// task pointer
	TPriority_t new_priority			// new task priority
	)
{
	return NORMAL;
}

/**
	Suspend the task until resumed.
	@param t	(in) Task Handle.
**/
static inline void OSTASK_Suspend(					// suspend task until resumed;
	Task_t	t							// task pointer
	){}

/**
	Resume a suspended task.
	@param t	(in) Task Handle.
**/

static inline void OSTASK_Resume(						// resume suspended task;
	Task_t	t							// task pointer
	)
{
}

/**
	Get the current executing task.  If no task is currently exeucting NULL is returned.
	@return Task_t	Handle to current Task
**/

static inline Task_t OSTASK_GetCurrentTask( void )	// get currently running task
{
	return NULL;
}

/**
	Get the name associated with the passed task object.
	@param t	(in) Task Handle
	@param *p_name	(out) Pointer to store task name.
**/

static inline OSStatus_t OSTASK_GetTaskName(			// get ASCII name of task
	Task_t t,							// task pointer
	UInt8 *p_name						// location to store the ASCII name
	)
{
	return OSSTATUS_SUCCESS;
}

/**
	Get the name pointer associated with the current task object.
**/

static inline char* OSTASK_GetCurrentTaskNamePtr(	// get current task name pointer
	void
	)
{
	return NULL;
}

/**
	Disable all preemption.  While disabled the scheduler will not pre-empt the currently
	executing task even if a higher priority task is ready to run.
**/

static inline void OSTASK_DisablePreemption( void ){}

/**
	Enable all preemption.  When enabled the scheduler will always schedule the highest
	priority ready task to run.
**/

static inline void OSTASK_RestorePreemption( void ){}

/**
	Get current context: task, HISR, LISR, or RTOS kernel?  
**/

#define	OSTASK_CONTEXT_TASK		0
#define	OSTASK_CONTEXT_HISR		1
#define	OSTASK_CONTEXT_LISR		2
#define	OSTASK_CONTEXT_RTOS		3
#define	OSTASK_CONTEXT_ERROR	-1

static inline int OSTASK_GetContext(void)
{
	return 0;
}

static inline TPriority_t OSTASK_GetCurrentTaskPriority( void )
{
	return NORMAL;
}


/**
	Check if a given TCB points to a valid task
	@param task	(in) Task Handle
	@return Boolean  Yes/No
**/
static inline Boolean OSTASK_IsValidTask(Task_t task)
{
	return TRUE;
}

/**
	Check if a given TCB points to a valid HISR
	@param task	(in) Task Handle
	@return Boolean  Yes/No
**/
static inline Boolean OSTASK_IsValidHISR(Task_t task)
{
	return TRUE;
}

/**
	Check if a given TCB points to a valid task or a HISR
	@param task	(in) Task Handle
	@return Boolean  Yes/No
**/
static inline Boolean OSTASK_IsValidTaskOrHISR(Task_t task)
{
	return TRUE;
}

/**
	Get the task/HISR name from a given TCB
	@param task	(in) Task Handle
	@return char* pointer pointing to the task/HISR name
**/
static inline char* OSTASK_GetNamePtr(Task_t task)
{
	return NULL;
}

/**
	Check if a given task/HISR is in ready state
	@param task	(in) Task Handle
	@return Boolean  Yes/No
**/
static inline Boolean OSTASK_IsTaskReady(Task_t task)
{
	return TRUE;
}

/**
	Check if a given task/HISR is in suspended state
	@param task	(in) Task Handle
	@return Boolean  Yes/No
**/

static inline Boolean OSTASK_IsTaskSuspended(Task_t task)
{
	return TRUE;
}

/**
	Get a task's priority from its TCB
	@param task	(in) Task Handle
	@return TPriority_t Task priority (the smaller, the higher)
**/
static inline TPriority_t OSTASK_GetPriority(Task_t task)
{
	return NORMAL;
}

/**
	Get a task's argc/argv pair
	@param task	(in) Task Handle
	@param argc (out) argc value when the task was created
	@param argv (out) argv value when the task was created
	@return OSStatus_t, if OSSTATUS_SUCCESS, argc/argv carry the inquired values.
**/
static inline OSStatus_t OSTASK_GetArgs(Task_t task, TArgc_t *argc, TArgv_t *argv)
{
	return OSSTATUS_SUCCESS;
}

/**
	Get a task's file pointer
	@param task	(in) Task Handle
	@return pointer to the file pointer.
**/
static inline void *OSTASK_GetFilePtr(Task_t task)
{
	return NULL;
}

/**
	Set a task's file pointer
	@param task	(in) Task Handle
	@param ptr pointer to the file pointer
	@return pointer to the file pointer.
**/
static inline void OSTASK_SetFilePtr(Task_t task, void *ptr){}

/**
	Get a task's stack usage
	@param task_name (in) Pointer to Task or HISR name
	@param usage_ptr (in) Pointer to stack usage
	@return pointer to the file pointer.
**/
static inline Boolean OSTASK_GetStackUsage(char *task_name, UInt32 *usage_ptr)
{
	return FALSE;
}

/**
	Get the reason why the task is suspended in string format 
	@param tcb  (in) task pointer of the task
**/
static inline const char *OSTASK_GetSuspendedReason(void *tcb)
{
	return NULL;
}

/**
	Get the current task's Thread Local Storage pointer. This is a pointer to some memory
	associated by the user to this task. If not yet set using OSTASK_TlsPtrSet or not called
	from a running task this function will return a NULL ptr.
	@return pointer to the Thread Local Storage.
**/
static inline void* OSTASK_TlsPtrGet(void)
{
	return NULL;
}

/**
	Set the current task's Thread Local Storage pointer. This is a pointer to some memory
	associated by the user to this task. If not called from a running task, this call will assert.
	@param ptr (in) Pointer to Thread Local Storage
**/
static inline void  OSTASK_TlsPtrSet(void *ptr){}


#endif // #ifndef _RTOS_OSTASK_H_

/** @} */
