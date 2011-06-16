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
*   @file   osevent.h
*
*   @brief  This file contains the interface to the Event functionality.
*
****************************************************************************/
/**

*   @defgroup   RTOSEventGroup   Event Services
*   @ingroup    RTOSGroup
*
*
*	The Event API provides interfaces to create/destroy and control events.
*
****************************************************************************/
/**
 * @addtogroup RTOSEventGroup
 * @{
 */

#ifndef _RTOS_OSEVENT_H_
#define _RTOS_OSEVENT_H_

//#include "mobcom_types.h"
#include "ostypes.h"

//******************************************************************************
// Global Macros
//******************************************************************************

#define	ENUM_EVENT_HISTORY_CREATE			1
#define	ENUM_EVENT_HISTORY_BEFORE_SET		2
#define	ENUM_EVENT_HISTORY_AFTER_SET		3
#define	ENUM_EVENT_HISTORY_BEFORE_RETRIEVE	4
#define	ENUM_EVENT_HISTORY_AFTER_RETRIEVE	5
#define	ENUM_EVENT_HISTORY_DESTROY			6

#define	OSEVENT_DEBUG_ID					0x45564447	//EVDG

//******************************************************************************
// Global Typedefs
//******************************************************************************

#define EVENT_NAME_SIZE	9		//	8 characters + NULL

/// Task Name is a NULL terminated string, up to TASK_NAME_SIZE

typedef UInt8 *EName_t;


// We use the same values as the underlying RTOS, JPG
// For nucleus, see nucleus.h
#define OSEVENT_OPTION_OR 0
#define OSEVENT_OPTION_OR_CLEAR 1
#define OSEVENT_OPTION_AND 2
#define OSEVENT_OPTION_AND_CLEAR 3

typedef UInt32 EOption_t;

typedef struct
{
	UInt32				evb[OS_EVENT_GROUP_SIZE];

	UInt32				debug_evb_id;

	Task_t				create_task;
	char				*create_file;
	UInt32				create_line;
	UInt32				create_time;

	Task_t				set_task;
	char				*set_file;
	UInt32				set_line;
	UInt32				set_time;

	Task_t				retrieve_task;
	char				*retrieve_file;
	UInt32				retrieve_line;
	UInt32				retrieve_time;
} EventStruct_t;


//******************************************************************************
// Global Function Prototypes
//******************************************************************************

/**
	Create an event.
	@param event_name	(in)	Event Name.
	@return	Event_t	An event object.
**/

#ifdef HISTORY_LOGGING
#define	OSEVENT_Create(event_name)	OSEVENT_CreateDebug(event_name, __FILE__, __LINENUM__)
Event_t OSEVENT_CreateDebug(			// returns the newly-created event
	EName_t event_name,					// event name
	char *file,							// file that calls OSEVENT_Create()
	UInt32 line							// line number in above file
	);
#else //#ifdef HISTORY_LOGGING
Event_t OSEVENT_Create(					// returns the newly-created event
	EName_t event_name					// event name
	);
#endif // #ifdef HISTORY_LOGGING

/**
	Destroy an event.
	@param	e	(in)	Event object to destory.
**/

#ifdef HISTORY_LOGGING
#define	OSEVENT_Destroy(e)	OSEVENT_DestroyDebug(e, __FILE__, __LINENUM__)
void OSEVENT_DestroyDebug(
	Event_t e,							// event pointer
	char *file,							// file that calls OSEVENT_Destroy()
	UInt32 line							// line number in above file
	);
#else // #ifdef HISTORY_LOGGING
void OSEVENT_Destroy(
	Event_t e							// event pointer
	);
#endif // #ifdef HISTORY_LOGGING

/**
	Reset the event to the nonsignaled state.
	@param	e	(in)	Event Object.
	@return OSStatus_t	Status of operation.
**/

#ifdef HISTORY_LOGGING
#define	OSEVENT_Reset(e)	OSEVENT_ResetDebug(e, __FILE__, __LINENUM__)
OSStatus_t OSEVENT_ResetDebug(
	Event_t e,							// event pointer
	char *file,							// file that calls OSEVENT_Reset()
	UInt32 line							// line number in above file
	);
#else // #ifdef HISTORY_LOGGING
OSStatus_t OSEVENT_Reset(
	Event_t e							// event pointer
	);
#endif // #ifdef HISTORY_LOGGING

/**
	Set the event to the signaled state.
	@param	e	(in)	Event Object.
	@return OSStatus_t	Status of operation.
**/

#ifdef HISTORY_LOGGING
#define	OSEVENT_Set(e)	OSEVENT_SetDebug(e, __FILE__, __LINENUM__)
OSStatus_t OSEVENT_SetDebug(
	Event_t e,							// event pointer
	char *file,							// file that calls OSEVENT_Set()
	UInt32 line							// line number in above file
	);
#else // #ifdef HISTORY_LOGGING
OSStatus_t OSEVENT_Set(
	Event_t e							// event pointer
	);
#endif // #ifdef HISTORY_LOGGING

/**
	Obtain an event.  waiting until "timeout" before failing.
	@param e	(in)	Event Object.
	@param timeout	(in)	Ticks to wait for event.
	@return OSStatus_t	Status of operation.
**/

#ifdef HISTORY_LOGGING
#define	OSEVENT_Obtain(e, timeout)	OSEVENT_ObtainDebug(e, timeout, __FILE__, __LINENUM__)
OSStatus_t OSEVENT_ObtainDebug(			// wait to obtain a semaphore
	Event_t e,							// event pointer
	Ticks_t timeout,					// timeout to failed event obtain
	char *file,							// file that calls OSEVENT_Obtain()
	UInt32 line							// line number in above file
	);
#else // #ifdef HISTORY_LOGGING
OSStatus_t OSEVENT_Obtain(				// wait to obtain a semaphore
	Event_t e,							// event pointer
	Ticks_t timeout						// timeout to failed event obtain
	);
#endif // #ifdef HISTORY_LOGGING

/**
	Set the event to the signaled state.
	@param	e	(in)	Event Object.
	@param	em	(in)	Event bits to be set
	@param	op	(in)	Operation code
	@return OSStatus_t	Status of operation.
**/

#ifdef HISTORY_LOGGING
#define	OSEVENT_SetEvents(e, em, op)	OSEVENT_SetEventsDebug(e, em, op, __FILE__, __LINENUM__)
OSStatus_t OSEVENT_SetEventsDebug(
	Event_t e,							// event pointer
	UInt32 em,							// event mask bits
	EOption_t op,						// operation code
	char *file,							// file that calls OSEVENT_Set()
	UInt32 line							// line number in above file
	);
#else // #ifdef HISTORY_LOGGING
OSStatus_t OSEVENT_SetEvents(
	Event_t e,							// event pointer
	UInt32 em,							// event mask bits
	EOption_t op						// operation code
	);
#endif // #ifdef HISTORY_LOGGING

/**
	Obtain an event.  waiting until "timeout" before failing.
	@param	e	(in)	Event Object.
	@param	req	(in)	Requested event bits
	@param	op	(in)	Operation code
	@param	resp (out)	Returned event bits
	@param	timeout	(in)	Ticks to wait for event.
	@return OSStatus_t	Status of operation.
**/

#ifdef HISTORY_LOGGING
#define	OSEVENT_ObtainEvents(e, req, op, resp, timeout)	OSEVENT_ObtainEventsDebug(e, req, op, resp, timeout, __FILE__, __LINENUM__)
OSStatus_t OSEVENT_ObtainEventsDebug(	// wait to obtain a semaphore
	Event_t e,							// event pointer
	UInt32	req,						// requested event mask bits
	EOption_t	op,						// operation code
	UInt32	*resp,						// actual event mask bits
	Ticks_t timeout,					// timeout to failed event obtain
	char	*file,						// file that calls OSEVENT_Obtain()
	UInt32	line						// line number in above file
	);
#else // #ifdef HISTORY_LOGGING
OSStatus_t OSEVENT_ObtainEvents(		// wait to obtain a semaphore
	Event_t e,							// event pointer
	UInt32	req,						// requested event mask bits
	EOption_t	op,						// operation code
	UInt32	*resp,						// actual event mask bits
	Ticks_t timeout						// timeout to failed event obtain
	);
#endif // #ifdef HISTORY_LOGGING

/**
	Get the name of an event.
	@param	e		(in)	Event Object.
	@param	p_name	(in)	Name storage (minimum 9 characters)
	@return OSStatus_t		Status of operation.
**/

OSStatus_t OSEVENT_GetName(		// get ASCII name of event
	Event_t e,					// event pointer
	UInt8 *p_name				// location to store the ASCII name
	);

/** @} */

#endif  // _RTOS_OSEVENT_H_
