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
*   @file   osinterrupt.h
*
*   @brief  This file contains the interface to the Interrupt functionality.
*
****************************************************************************/
/**

*   @defgroup   RTOSInterruptGroup   Interrupt Services
*   @ingroup    RTOSGroup
*
*	The Interrupt API provides interfaces to make an Interrupt Service
	routine known to the RTOS.  Interrupt service routines fall into
	two categories.  There are Low Level Interrupt Service Routines (LISR)
	and High Level Interrupt Service Routines (HISR).
	
	An LISR is created and made known to the platform using the Interrupt
	Controller API that is described in the Platform Application Programming
	Interface Document.  Typically all a LISR will do is "trigger" an HISR
	using the OSINTERRUPT_Trigger API described here.
	
	When an LISR "triggers" an HISR the HISR will pre-empt any other HISRs
	of lower priority or the current task that is running.  The HISR has
	its own stack and can make RTOS calls that do not cause the system 
	to block.
*
****************************************************************************/


#ifndef _RTOS_OSINTERRUPT_H_
#define _RTOS_OSINTERRUPT_H_

//#include "mobcom_types.h"
#include "ostypes.h"

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

typedef void (*IEntry_t)( void );

/// Priority associated with HISR.  Higher priroity HISRs may preempt lower priority
/// HISRs.

typedef enum
{
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

#ifdef HISTORY_LOGGING
#define	OSINTERRUPT_Create(e, n, p, s)	OSINTERRUPT_CreateDebug(e, n, p, s, __FILE__, __LINENUM__)

Interrupt_t OSINTERRUPT_CreateDebug(	// return an interrupt pointer
	IEntry_t entry,						// interrupt function entry point
	IName_t		name,					// ASCII name
	IPriority_t priority,				// interrupt priority
	IStackSize_t stack_size,			// interrupt stack size (in UInt8)
	char *file,							// name of file in which OSINTERRUPT_Create() is called
	UInt32 line							// line number
	);
#else // #ifdef HISTORY_LOGGING
Interrupt_t OSINTERRUPT_Create(			// return an interrupt pointer
	IEntry_t entry,						// interrupt function entry point
	IName_t		name,					// ASCII name
	IPriority_t priority,				// interrupt priority
	IStackSize_t stack_size				// interrupt stack size (in UInt8)
	);
#endif // #ifdef HISTORY_LOGGING

/**
	Destroy an HISR.
	@param t	Handle to HISR.
**/
#ifdef HISTORY_LOGGING
#define	OSINTERRUPT_Destroy(t)	OSINTERRUPT_DestroyDebug(t, __FILE__, __LINENUM__)

void OSINTERRUPT_DestroyDebug(
	Interrupt_t t,						// pointer to HC_HCB of HISR to be destroyed
	char *file,							// name of file in which OSINTERRUPT_Create() is called
	UInt32 line							// line number
	);
#else // #ifdef HISTORY_LOGGING
void OSINTERRUPT_Destroy(
	Interrupt_t t
	);
#endif // #ifdef HISTORY_LOGGING

/**
	Trigger an HISR.  This is typically called from an LISR to trigger the
	processing associated with the HISR.
	@param i	(in) Handle associated with HISR.
	@return OSTATUS_t	Status 
**/

OSStatus_t OSINTERRUPT_Trigger(			// process interrupt
	Interrupt_t i						// interrupt task pointer
	);

#if 0
// OSINTERRUPT_EnableAll() has been misused to pair with OSINTERRUPT_DisableAll(). Remove it.
// The correct pair should be mask = OSINTERRUPT_DisableAll() and OSINTERRUPT_Restore(mask).
// Hui Luo, 12/3/07
/**
	Enable all interrupts.
**/

UInt32 OSINTERRUPT_EnableAll( void );		// enables all known interrupts
#endif

/**
	Disable all interrupts.
**/

#ifdef HISTORY_LOGGING
#define	OSINTERRUPT_DisableAll()	OSINTERRUPT_DisableAllDebug(__FILE__, __LINENUM__)

UInt32 OSINTERRUPT_DisableAllDebug(		// disables all interrupts
	char *file,							// name of file in which OSINTERRUPT_Create() is called
	UInt32 line							// line number
	);
#else // #ifdef HISTORY_LOGGING
UInt32 OSINTERRUPT_DisableAll(				// disables all interrupts
	void
	);
#endif // #ifdef HISTORY_LOGGING

/**
	Restore previous interrupt mask.
**/

#ifdef HISTORY_LOGGING
#define	OSINTERRUPT_Restore(m)	OSINTERRUPT_RestoreDebug(m, __FILE__, __LINENUM__)

UInt32 OSINTERRUPT_RestoreDebug(		// restore previous interrupt
	UInt32 mask,						// mask returned from OSINTERRUPT_DisableAll()
	char *file,							// name of file in which OSINTERRUPT_Create() is called
	UInt32 line							// line number
	);
#else // #ifdef HISTORY_LOGGING
UInt32 OSINTERRUPT_Restore(				// restore previous interrupt
	UInt32 mask
	);
#endif // #ifdef HISTORY_LOGGING

/**
	Get the name of an HISR
	@param i		(in)	Handle associated with HISR.
	@param p_name	(in)	Name storage (minimum 9 characters).
	@return OSStatus_t		Status of operation.
**/

OSStatus_t OSINTERRUPT_GetName(		// get ASCII name of interrupt
	Interrupt_t i,					// interrupt pointer
	UInt8 *p_name					// location to store the ASCII name
	);

/** @} */

#endif
