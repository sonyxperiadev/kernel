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
*   @file   osqueue.h
*
*   @brief  This file contains the interface to the Message Queue functionality.
*
****************************************************************************/
/**

*   @defgroup   RTOSQUEUEGroup   Message Queue Services
*   @ingroup    RTOSGroup
*
*	The Queue API provides interfaces to the send and recieve messages
	between tasks. 
*
****************************************************************************/

/**
 * @addtogroup RTOSQUEUEGroup
 * @{
 */

#ifndef _RTOS_OSQUEUE_H_
#define _RTOS_OSQUEUE_H_

//#include "mobcom_types.h"
#include "ostypes.h"
//#include "ostask.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//******************************************************************************
// Global Macros
//******************************************************************************

#define	ENUM_QUEUE_HISTORY_CREATE			1
#define	ENUM_QUEUE_HISTORY_POST				2
#define	ENUM_QUEUE_HISTORY_POST_TO_FRONT	3
#define	ENUM_QUEUE_HISTORY_RETRIEVE			4
#define	ENUM_QUEUE_HISTORY_FLUSH			5
#define	ENUM_QUEUE_HISTORY_DESTROY			6

#define	OSQUEUE_DEBUG_ID					0x51554447	//QUDG

//******************************************************************************
// Global Typedefs
//******************************************************************************

/// Number of entries a Queue can hold.

typedef UInt32 QEntries_t;

/// Size of an entry within the Queue.

typedef UInt32 QMsgSize_t;

/// Queue Message.

typedef UInt8  QMsg_t;

typedef UInt32 QMem_t;

typedef struct
{
	UInt32				msg_size;
	UInt32				mem_size;
	UInt32*				qmem;
	UInt32				Id;
	
	UInt32				qcb[OS_QUEUE_SIZE];

	UInt32				debug_qcb_id;

	Task_t				create_task;
	char				*create_file;
	UInt32				create_line;
	UInt32				create_time;

	Task_t				post_task;
	char				*post_file;
	UInt32				post_line;
	UInt32				post_time;

	Task_t				pend_task;
	char				*pend_file;
	UInt32				pend_line;
	UInt32				pend_time;

	UInt32				low_watermark;	//MobC00062522, Hui Luo, 2/8/09
} QueueStruct_t;

//******************************************************************************
// Global Function Prototypes
//******************************************************************************

/**
	Create a queue.
	@param entries	(in) Number of entries in the Queue.
	@param msg_size	(in) Size of an entry in the Queue.
	@param mode		(in) Pending Method
	@return Queue_t	Queue object associated with created Queue.
**/

#ifdef HISTORY_LOGGING
#define	OSQUEUE_Create(entries, msg_size, mode)	OSQUEUE_CreateDebug(entries, msg_size, mode, __FILE__, __LINENUM__)
Queue_t OSQUEUE_CreateDebug(			// returns newly-created queue
	QEntries_t entries,					// number of entries
	QMsgSize_t msg_size,				// fixed-size message size (in QMsg_t)
	OSSuspend_t mode,					// pending method
	char *file,							// file that calls OSQUEUE_Creat()
	UInt32 line							// line number in above file
	);
#else // #ifdef HISTORY_LOGGING
Queue_t OSQUEUE_Create(					// returns newly-created queue
	QEntries_t entries,					// number of entries
	QMsgSize_t msg_size,				// fixed-size message size (in QMsg_t)
	OSSuspend_t mode					// pending method
	);
#endif // #ifdef HISTORY_LOGGING

/**
	Destroy the passed Queue and release all resources associated with the Queue.
	@param q	(in) Queue to destroy.
**/

#ifdef HISTORY_LOGGING
#define	OSQUEUE_Destroy(q)	OSQUEUE_DestroyDebug(q, __FILE__, __LINENUM__)
void OSQUEUE_DestroyDebug(				// Destroys all resources associated
	Queue_t	q,							// with the passed queue
	char *file,							// file that calls OSQUEUE_Destroy()
	UInt32 line							// line number in above file
	);
#else // #ifdef HISTORY_LOGGING
void OSQUEUE_Destroy(					// Destroys all resources associated
	Queue_t	q							// with the passed queue
	);
#endif // #ifdef HISTORY_LOGGING

/**
	Flush all entries in a Queue.
	@param	q	(in) Queue to flush.
**/

#ifdef HISTORY_LOGGING
#define	OSQUEUE_FlushQ(q)	OSQUEUE_FlushQDebug(q, __FILE__, __LINENUM__)
void OSQUEUE_FlushQDebug(				// clear queue
	Queue_t q,							// queue pointer
	char *file,							// file that calls OSQUEUE_FlushQ()
	UInt32 line							// line number in above file
	);
#else // #ifdef HISTORY_LOGGING
void OSQUEUE_FlushQ(					// clear queue
	Queue_t q							// queue pointer
	);
#endif // #ifdef HISTORY_LOGGING

/**
	Pend on a Queue to receive a message.  If the timeout is set to TICKS_FOREVEVER
	and no message is available in the Queue the pending task will block until a
	message is available.
	@param	q		(in)	Queue to pend on.
	@param *msg		(in)	Pointer to where to put message.
	@param timeout	(in)	Timeout value
	@return OSSTATUS_t	Result of operation.
**/

#ifdef HISTORY_LOGGING
#define	OSQUEUE_Pend(q, msg, timeout)	OSQUEUE_PendDebug(q, msg, timeout, __FILE__, __LINENUM__)
OSStatus_t OSQUEUE_PendDebug(			// get message from the queue
	Queue_t q,							// queue pointer
	QMsg_t *msg,						// pointer to message
	Ticks_t timeout,					// timeout to pend failure
	char *file,							// file that calls OSQUEUE_Pend()
	UInt32 line							// line number in above file
	);
#else // #ifdef HISTORY_LOGGING
OSStatus_t OSQUEUE_Pend(				// get message from the queue
	Queue_t q,							// queue pointer
	QMsg_t *msg,						// pointer to message
	Ticks_t timeout						// timeout to pend failure
	);
#endif // #ifdef HISTORY_LOGGING

/**
	Post a message to a Queue.  If the Queue is full and the timeout is set to TICKS_FOREVER
	the posting task will block until space is available on the queue.
	@param	q		(in)	Queue to post message to.
	@param	*msg	(in)	Pointer to message to post.
	@param	timeout	(in)	TICKS_NO_WAIT or TICKS_FOREVER
	@return OSSTATUS_t		Result of operation.
**/

#ifdef HISTORY_LOGGING
#define	OSQUEUE_Post(q, msg, timeout)	OSQUEUE_PostDebug(q, msg, timeout, __FILE__, __LINENUM__)
OSStatus_t OSQUEUE_PostDebug(			// put message to queue
	Queue_t q,							// queue pointer
	QMsg_t *msg,						// pointer to empty message
	Ticks_t timeout,					// TICKS_NO_WAIT or TICKS_FOREVER
	char *file,							// file that calls OSQUEUE_Post()
	UInt32 line							// line number in above file
	);
#else // #ifdef HISTORY_LOGGING
OSStatus_t OSQUEUE_Post(				// put message to queue
	Queue_t q,							// queue pointer
	QMsg_t *msg,						// pointer to empty message
	Ticks_t timeout						// TICKS_NO_WAIT or TICKS_FOREVER
	);
#endif // #ifdef HISTORY_LOGGING

/**
	Post a message to the front of a Queue.  The message becomes the first message in the
	Queue.
	@param	q		(in)	Queue to post message to.
	@param	*msg	(in)	Pointer to message to post.
	@param	timeout	(in)	TICKS_NO_WAIT or TICKS_FOREVER
	@return OSSTATUS_t		Result of operation.
**/

#ifdef HISTORY_LOGGING
#define	OSQUEUE_PostToFront(q, msg, timeout)	OSQUEUE_PostToFrontDebug(q, msg, timeout, __FILE__, __LINENUM__)
OSStatus_t OSQUEUE_PostToFrontDebug(	// put message to front of queue
	Queue_t q,							// queue pointer
	QMsg_t *msg,						// pointer to empty message
	Ticks_t timeout,					// TICKS_NO_WAIT or TICKS_FOREVER
	char *file,							// file that calls OSQUEUE_PostToFront()
	UInt32 line							// line number in above file
	);
#else // #ifdef HISTORY_LOGGING
OSStatus_t OSQUEUE_PostToFront(			// put message to front of queue
	Queue_t q,							// queue pointer
	QMsg_t *msg,						// pointer to empty message
	Ticks_t timeout						// TICKS_NO_WAIT or TICKS_FOREVER
	);
#endif // #ifdef HISTORY_LOGGING

/**
	Query if a Queue is Empty.
	@param q	(in)	Queue to query.
	@return Boolean	TRUE if Empty and FALSE otherwise.
**/

Boolean OSQUEUE_IsEmpty(				// Check if there are any messages in queue
	Queue_t q							// queue pointer
	);

/**
	Get name associated with the passed Queue.  
	@param qcb		(in)	Queue object.
	@param *p_name	(in)	Pointer to memory to store queue name.
**/
 
OSStatus_t OSQUEUE_GetQueueName(		// gets ASCII name of queue
	Queue_t qcb,						// Queue pointer
	Int8 *p_name						// location to storage the queue name
	);

/**
	@cond internal
	Get number of messages currently queued. 
	@param	q	(in)	Queue object.
	@return UInt32	Number of messages queue.
**/

UInt32 *OSQUEUE_GetCountAddr(			// Get number of messages currently queued. 
	Queue_t q
	);

/** @endcond */

/**
	Change the name of an existing queue.
	@param	queue	(in)	Queue object
	@param	name	(in)	New name of the queue
**/

void OSQUEUE_ChangeName(
	Queue_t queue,
	const char* name
	);

/**
	Query if a Task is idlely pending at its input Queue.
	@param t	(in)	Task to query.
	@param q	(in)	Task's input Queue.
	@return Boolean	TRUE if Idle and FALSE otherwise.
**/

Boolean OSQUEUE_IsTaskIdle(
	Task_t t,							// task (pointer)
	Queue_t q							// task's main queue (pointer)
	);

/**
	Get the name of a Queue.
	@param q		(in)	Queue object.
	@param p_name	(in)	Name storage (minimum 9 characters).
	@return OSSTATUS_t		Result of operation.
**/

OSStatus_t OSQUEUE_GetName(		// get ASCII name of Queue
	Queue_t q,					// Queue pointer
	UInt8 *p_name				// location to store the ASCII name
	);

/** @} */

#ifdef	__cplusplus
}
#endif

#endif

