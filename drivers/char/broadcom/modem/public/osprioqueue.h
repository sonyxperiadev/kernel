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
*   @file   osprioqueue.h
*
*   @brief  This file contains the interface to the Priority Message Queue functionality.
*
****************************************************************************/
/**

*   @defgroup   RTOSPRIOQUEUEGroup   Priority Message Queue Services
*   @ingroup    RTOSGroup
*
*	The Priority Queue API provides interfaces to the send and receive priority messages
	between tasks. 
*
****************************************************************************/

/**
 * @addtogroup RTOSPRIOQUEUEGroup
 * @{
 */

#ifndef _RTOS_OSPRIOQUEUE_H_
#define _RTOS_OSPRIOQUEUE_H_

//#include "mobcom_types.h"
#include "ostypes.h"
//#include "ossemaphore.h"
//#include "osinterrupt.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//#define OSPRIOQUEUE_DEBUG
#ifdef OSPRIOQUEUE_DEBUG
	extern void TRACE_Printf_Sio(const char *, ...);	///< The debug output function.

	#define OSPQ_DBG(_str_)  TRACE_Printf_Sio _str_		///< The debug output macro.
#else
	#define OSPQ_DBG(_str_)  {}							///< The debug output macro.
#endif

//******************************************************************************
// Global Macros
//******************************************************************************

#define MAX_NUM_PRIOQUEUES	10			///< max number of priorities supported by a set of priority queues (up to 256)

//******************************************************************************
// Global Typedefs
//******************************************************************************

///< Doubly-linked list for a priority queue
struct PrioQueueList
{
	void	*msg;						///< pointer to a message stored in a priority queue
	struct	PrioQueueList *prev;		///< pointer to the previous message in the queue
	struct	PrioQueueList *next;		///< pointer to the next message in the queue
};

typedef struct PrioQueueList *PrioQueueList_t;


/// Priority queue object returned by PRIOOSQUEUE_Create.
typedef struct
{
	UInt8					num_prio_queues;					///< number of priority queues
	struct PrioQueueList	prio_queue[MAX_NUM_PRIOQUEUES];		///< array of doubly-linked lists for the priority queues; 
																///<	lower index means higher priority
	Boolean					selected[MAX_NUM_PRIOQUEUES];		///< flags indicating which priority queues are selected; 
																///<	"selected" means a new message in the queue will trigger the task to run; 
																///<	"unselected means a new message to the queue will be queued but won't trigger the task
																///<	default "selected"
	UInt32					num_entries[MAX_NUM_PRIOQUEUES];	///< number of entries in a priority queue
	NU_EVENT_GROUP			events;								///< Nucleus event associated with the queues
	Semaphore_t				sema;								///< semaphore to protect the queues???
} PrioQueueStruct_t;

typedef PrioQueueStruct_t *PrioQueue_t;

#define Enqueue( list, elem )		\
  ( elem )->next = ( list )->next;	\
  ( elem )->prev = ( list );		\
  ( list )->next->prev = ( elem );	\
  ( list )->next = ( elem );

#define Dequeue( elem )						\
  ( elem )->next->prev = ( elem )->prev;	\
  ( elem )->prev->next = ( elem )->next;

//******************************************************************************
// Global Function Prototypes
//******************************************************************************

/**
	Create a set of priority queues. The created set of priority queues is to be associated with one single task.
	@param num_prio_queues	(in) Number of priority queues in this set.
	@return PrioQueue_t	Priority queue object associated with the created set of priority queues.
**/

PrioQueue_t OSPRIOQUEUE_Create(
	UInt8 num_prio_queues
	);

/**
	Destroy the passed priority queue object and release all associated resources.
	@param q	(in) Priority queue object to destroy.
**/

void OSPRIOQUEUE_Destroy(
	PrioQueue_t	q
	);

/**
	Pend on a set of "selected" priority queues to receive a message.  If the timeout is set to TICKS_FOREVER
	and no message is available in the Queue the pending task will block until a
	message is available.
	@param	q		(in)	Priority queue object to pend on.
	@param *prio	(out)	Address to store the priority of the received message.
	@param *msg		(out)	Address to store the received message.
	@param timeout	(in)	Timeout value
	@return OSSTATUS_t	Result of operation.
**/
	
OSStatus_t OSPRIOQUEUE_Pend(
	PrioQueue_t q,
	UInt8 *prio,
	void *msg,
	Ticks_t timeout
	);

/**
	Post a message to a priority queue. Since linked list is used, there is no predefined queue size.
	A message can be posted into the queue as long as there is enough memory allocated for the new entry
	to the queue. If the memory allocation fails, the caller will be suspended until the specific timeout value.
	@param	q		 (in)	Priority queue object to post message to.
	@param	prio 	 (in)	Priority of the message
	@param	*msg	 (in)	Pointer to message to post.
	@param  timeout	 (in)	Timeout value
	@return OSSTATUS_t		Result of operation.
**/

OSStatus_t OSPRIOQUEUE_Post(
	PrioQueue_t q,
	UInt8 prio,
	void *msg,
	Ticks_t timeout
	);

/**
	Post a message to the front of a priority queue. Since linked list is used, there is no predefined queue size.
	A message can be posted into the queue as long as there is enough memory allocated for the new entry
	to the queue. If the memory allocation fails, the caller will be suspended until the specific timeout value.
	@param	q		 (in)	Priority queue object to post message to.
	@param	prio	 (in)	Priority of the message
	@param	*msg	 (in)	Pointer to message to post.
	@param  timeout	 (in)	Timeout value
	@return OSSTATUS_t		Result of operation.
**/

OSStatus_t OSPRIOQUEUE_PostToFront(
	PrioQueue_t q,
	UInt8 prio,
	void *msg,
	Ticks_t timeout
	);

/**
	This function searches the specified priority queue to find the specified message. If the priority is specified
	as num_prio_queues then all priority queues will be searched.
	If found the message will be removed from the queue
	@param	q		 (in)	Priority queue object to search
	@param	prio	 (in)	Priority of the message
	@param	*msg	 (in)	Pointer to message to search for
	@return UInt32*			Pointer to the number of messages queued in the priority queue.
**/
OSStatus_t OSPRIOQUEUE_Remove(
	PrioQueue_t q,
	UInt8		prio,
	void		*msg
	);
/**
	Get number of messages currently queued in a priority queue.
	@param	q		(in)	Priority queue object.
	@param	prio	(in)	Priority of the message
	@return UInt32*			Pointer to the number of messages queued in the priority queue.
**/

UInt32 *OSPRIOQUEUE_GetCountAddr(
	PrioQueue_t q,
	UInt8 prio
	);

/**
	Select a priority queue so that when a message is posted to this queue, the associated task will be triggered to run.
	@param	q	 (in)	Priority queue object.
	@param	prio (in)	Priority queue to be selected.
**/

void OSPRIOQUEUE_Select(
	PrioQueue_t q,
	UInt8 prio
	);

/**
	Unselect a priority queue so that when a message is posted to this queue, the associated task will not be triggered to run.
	@param	q	 (in)	Priority queue object.
	@param	prio (in)	Priority queue to be selected.
**/

void OSPRIOQUEUE_Unselect(
	PrioQueue_t q,
	UInt8 prio
	);

/**
	Select all priority queues associated with the priority queue object.
	@param	q	 (in)	Priority queue object.
**/

void OSPRIOQUEUE_SelectAll(
	PrioQueue_t q
	);

/**
	Unselect all priority queues associated with the priority queue object.
	@param	q	 (in)	Priority queue object.
**/

void OSPRIOQUEUE_UnselectAll(
	PrioQueue_t q
	);


/** @} */

#ifdef	__cplusplus
}
#endif

#endif // #ifndef _RTOS_OSPRIOQUEUE_H_

