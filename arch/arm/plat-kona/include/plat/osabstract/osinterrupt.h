/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*             @file     arch/arm/plat-kona/include/plat/osabstract/osinterrupt.h
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

#ifndef _RTOS_OSINTERRUPT_H_
#define _RTOS_OSINTERRUPT_H_

#include <plat/types.h>
#include <plat/osabstract/ostypes.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/delay.h>

/**
 * @addtogroup RTOSInterruptGroup
 * @{
 */

//******************************************************************************
// Global Macros
//******************************************************************************

//******************************************************************************
// Global Typedefs
//******************************************************************************

/// Interrupt object returned from the call to OSINTERRUPT_Create.

typedef void *Interrupt_t;

/// Entry point for an HISR is a function with no parameters and no return value.

typedef void (*IEntry_t) (void);

/// Priority associated with HISR.  Higher priroity HISRs may preempt lower priority
/// HISRs.

typedef enum {
	IPRIORITY_HIGH,
	IPRIORITY_MIDDLE,
	IPRIORITY_LOW
} IPriority_t;

/// HISR Stack Size

typedef UInt16 IStackSize_t;

/// HISR Name

typedef UInt8 *IName_t;

//******************************************************************************
// Global Function Prototypes
//******************************************************************************

/**
	Create an HISR and make it known to the RTOS.
	@param entry		(in) Entry point for HISR
	@param name			(in) Name associated with HISR.  Only 8 characters are significant
	@param priority		(in) interrupt priority
	@param stack_size	(in) Size of stack used by HISR.
	@return Interrupt_t	Handle to HISR.
**/

static inline Interrupt_t OSINTERRUPT_Create(	// return an interrupt pointer
						    IEntry_t entry,	// interrupt function entry point
						    IName_t name,	// ASCII name
						    IPriority_t priority,	// interrupt priority
						    IStackSize_t stack_size	// interrupt stack size (in UInt8)
    )
{
	struct tasklet_struct *tasklet =
	    kzalloc(sizeof(struct tasklet_struct), GFP_KERNEL);

	// DECLARE_TASKLET_DISABLED(name, func, data) struct tasklet_struct name = { NULL, 0, ATOMIC_INIT(1), func, data }
	// struct tasklet_struct
	// {
	//      struct tasklet_struct *next;
	//      unsigned long state;
	//      atomic_t count;
	//      void (*func)(unsigned long);
	//      unsigned long data;
	// };
	tasklet->next = NULL;
	tasklet->state = 0;
	tasklet->count.counter = 1;
	tasklet->func = (void *)entry;
	tasklet->data = 0;
	tasklet_enable(tasklet);

	return (Interrupt_t) tasklet;
}

/**
	Destroy an HISR.
	@param t	Handle to HISR.
**/

static inline void OSINTERRUPT_Destroy(Interrupt_t t)
{
	struct tasklet_struct *tasklet = (struct tasklet_struct *)t;

	tasklet_kill(tasklet);
}

/**
	Trigger an HISR.  This is typically called from an LISR to trigger the
	processing associated with the HISR.
	@param i	(in) Handle associated with HISR.
	@return OSTATUS_t	Status 
**/

static inline OSStatus_t OSINTERRUPT_Trigger(	// process interrupt
						    Interrupt_t i	// interrupt task pointer
    )
{
	struct tasklet_struct *tasklet = (struct tasklet_struct *)i;

	tasklet_schedule(tasklet);

	return OSSTATUS_SUCCESS;
}

#endif
