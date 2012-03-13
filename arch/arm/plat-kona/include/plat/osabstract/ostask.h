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
	IDLE = 250,		// only used by LOG task or any task with absolutely lowest priority
	LOWEST = 60,
	BELOW_NORMAL = 37,
	NORMAL = 30,
	ABOVE_NORMAL = 20,
	HIGHEST = 11
} TPriority_t;
#else
typedef enum {
	IDLE = 250,		// only used by LOG task or any task with absolutely lowest priority
	LOWEST = 40,
	BELOW_NORMAL = 30,
	NORMAL = 20,
	ABOVE_NORMAL = 10,
	HIGHEST = 0
} TPriority_t;
#endif

#ifndef OSAL_USER_TASK_EXTENSION
#define OSAL_USER_TASK_EXTENSION	// Set to nothing if brcm_os_port.h not included
#endif

/** 8 characters + NULL */
#define TASK_NAME_SIZE	9	//      8 characters + NULL

/// Task Name is a NULL terminated string, up to TASK_NAME_SIZE

typedef UInt8 *TName_t;

/// Identifies how many arguments are in the TArgv_t list.

typedef UInt32 TArgc_t;

/// Argument list passed to task entry point.

typedef void *TArgv_t;

/// Task entry point which has parameters.

typedef void (*TEntry_t) (void);

/// Task entry point which has no-parameters.

typedef void (*TEntryWArg_t) (TArgc_t, TArgv_t);

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

static inline Task_t OSTASK_Create(	// returns the newly-created task
					  TEntry_t entry,	// task function entry point
					  TName_t task_name,	// task name
					  TPriority_t priority,	// task priority, bigger is higher
					  TStackSize_t stack_size	// task stack size (in UInt8)
    )
{
	struct task_struct *t;
	t = kthread_run((void *)entry, NULL, "%s", task_name);
	return IS_ERR(t) ? NULL : t;
}

/**
	Destroys a task.  Prior to destroying a task the resources the task has
	allocated should be released otherwise they can not be recovered.
	@param t (in) Handle to a task object.
**/

static inline void OSTASK_Destroy(	// Terminate and destroy all the
					 Task_t t	// task pointer
    )
{
	kthread_stop((struct task_struct *)t);
}

/**
	Suspend the task for a specified number of timer ticks.
	@param timeout	(in) Number of ticks to suspend the task.
**/
static inline void OSTASK_Sleep(	// suspend task, until timeout
				       Ticks_t timeout	// task sleep timeout
    )
{
	unsigned long timeout_val = (unsigned long)timeout;

	while (timeout_val)
		timeout_val = schedule_timeout_uninterruptible(timeout_val);
}

#endif // #ifndef _RTOS_OSTASK_H_

/** @} */
