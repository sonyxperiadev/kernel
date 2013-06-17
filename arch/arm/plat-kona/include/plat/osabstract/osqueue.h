
/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*             @file     arch/arm/plat-kona/include/plat/osabstract/osqueue.h
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

#ifndef _OSQUEUE_H_
#define _OSQUEUE_H_

#include <plat/types.h>
#include <plat/osabstract/ostypes.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/completion.h>
#include <linux/hardirq.h>
#include <linux/semaphore.h>

#ifdef	__cplusplus
extern "C" {
#endif

//******************************************************************************
// Global Typedefs
//******************************************************************************

/// Number of entries a Queue can hold.

	typedef UInt32 QEntries_t;

/// Size of an entry within the Queue.

	typedef UInt32 QMsgSize_t;

/// Queue Message.

	typedef UInt8 QMsg_t;

	typedef UInt32 QMem_t;

/*   */
	struct QueueLLI_t {
		struct list_head head;
		UInt32 *msg_ptr;
	};

/*   */
	struct QueueStruct_t {
		UInt32 msg_size;
		UInt32 max_entries;
		struct mutex q_mutex;
		wait_queue_head_t msg_wait;
		struct list_head q_head;
	};

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

	static inline Queue_t OSQUEUE_Create(	// returns newly-created queue
						    QEntries_t entries,	// number of entries
						    QMsgSize_t msg_size,	// fixed-size message size (in QMsg_t)
						    OSSuspend_t mode	// pending method
	    ) {
		struct QueueStruct_t *new_queue =
		    kzalloc(sizeof(struct QueueStruct_t), GFP_KERNEL);
		if (new_queue) {
			new_queue->msg_size = msg_size;
			new_queue->max_entries = entries;
			mutex_init(&new_queue->q_mutex);
			init_waitqueue_head(&new_queue->msg_wait);
			INIT_LIST_HEAD(&new_queue->q_head);
		}
		return (Queue_t *) new_queue;
	}

/**
	Flush all entries in a Queue.
	@param	q	(in) Queue to flush.
**/

	static inline void OSQUEUE_FlushQ(	// clear queue
						 Queue_t q	// queue pointer
	    ) {
		struct QueueStruct_t *queue = (struct QueueStruct_t *)q;
		struct QueueLLI_t *tmpLLI;
		struct list_head *pos;
		list_for_each(pos, &queue->q_head) {
			tmpLLI = list_entry(pos, struct QueueLLI_t, head);
			if (tmpLLI->msg_ptr) {
				kfree(tmpLLI->msg_ptr);
				kfree(tmpLLI);
			}
			if (in_interrupt()) {
				if (!mutex_trylock(&queue->q_mutex))
					return;
			} else {
				if (mutex_lock_interruptible(&queue->q_mutex) !=
				    0) {
					return;
				}
			}
			list_del_init(&tmpLLI->head);
			mutex_unlock(&queue->q_mutex);
		}
	}

/**
	Destroy the passed Queue and release all resources associated with the Queue.
	@param q	(in) Queue to destroy.
**/

	static inline void OSQUEUE_Destroy(	// Destroys all resources associated
						  Queue_t q	// with the passed queue
	    ) {
		OSQUEUE_FlushQ(q);
		kfree(q);
	}

/**
	Pend on a Queue to receive a message.  If the timeout is set to TICKS_FOREVEVER
	and no message is available in the Queue the pending task will block until a
	message is available.
	@param	q		(in)	Queue to pend on.
	@param *msg		(in)	Pointer to where to put message.
	@param timeout	(in)	Timeout value
	@return OSSTATUS_t	Result of operation.
**/

	static inline OSStatus_t OSQUEUE_Pend(	// get message from the queue
						     Queue_t q,	// queue pointer
						     QMsg_t * msg,	// pointer to message
						     Ticks_t timeout	// timeout to pend failure
	    ) {
		OSStatus_t status = OSSTATUS_FAILURE;
		struct QueueStruct_t *queue = (struct QueueStruct_t *)q;
		struct QueueLLI_t *qlli;

		if (in_interrupt())
			return status;

		if (wait_event_interruptible(queue->msg_wait,
					     !list_empty(&queue->q_head)))
			return status;

		if (mutex_lock_interruptible(&queue->q_mutex) != 0)
			return status;

		qlli =
		    container_of(queue->q_head.next, struct QueueLLI_t, head);
		memcpy(msg, qlli->msg_ptr, queue->msg_size);
		list_del(queue->q_head.next);
		mutex_unlock(&queue->q_mutex);
		kfree(qlli->msg_ptr);
		kfree(qlli);

		return OSSTATUS_SUCCESS;
	}

/**
	Post a message to a Queue.  If the Queue is full and the timeout is set to TICKS_FOREVER
	the posting task will block until space is available on the queue.
	@param	q		(in)	Queue to post message to.
	@param	*msg	(in)	Pointer to message to post.
	@param	timeout	(in)	TICKS_NO_WAIT or TICKS_FOREVER
	@param post_to_tail	(in)	TRUE for normal queuing; FALSE to queue at the beginning of the queue
	@return OSSTATUS_t		Result of operation.
**/

	static inline OSStatus_t OSQUEUE_PostGeneric(	// internal method to post msg
							    Queue_t q,	// queue pointer
							    QMsg_t * msg,	// pointer to empty message
							    Ticks_t timeout,	// TICKS_NO_WAIT or TICKS_FOREVER
							    bool post_to_tail	// set to TRUE for normal queuing; FALSE to queue at the beginning of the queue
	    ) {
		struct QueueStruct_t *queue = (struct QueueStruct_t *)q;
		struct QueueLLI_t *tmp = (struct QueueLLI_t *)
		    kzalloc(sizeof(struct QueueLLI_t), GFP_KERNEL);
		if (!tmp)
			goto err_tmp;

		tmp->msg_ptr = (UInt32 *) kzalloc(queue->msg_size, GFP_KERNEL);
		if (!tmp->msg_ptr)
			goto err_msg;

		if (in_interrupt()) {
			if (!mutex_trylock(&queue->q_mutex))
				goto err;
		} else if (mutex_lock_interruptible(&queue->q_mutex)) {
			goto err;
		}

		if (post_to_tail)
			list_add_tail(&(tmp->head), &(queue->q_head));
		else
			list_add(&(tmp->head), &(queue->q_head));
		memcpy((QMsg_t *) tmp->msg_ptr, msg, queue->msg_size);
		mutex_unlock(&queue->q_mutex);
		wake_up(&queue->msg_wait);
		return OSSTATUS_SUCCESS;

	      err:
		kfree(tmp->msg_ptr);
	      err_msg:
		kfree(tmp);
	      err_tmp:
		return OSSTATUS_FAILURE;
	}

/**
	Post a message to a Queue.  If the Queue is full and the timeout is set to TICKS_FOREVER
	the posting task will block until space is available on the queue.
	@param	q		(in)	Queue to post message to.
	@param	*msg	(in)	Pointer to message to post.
	@param	timeout	(in)	TICKS_NO_WAIT or TICKS_FOREVER
	@return OSSTATUS_t		Result of operation.
**/

#define OSQUEUE_Post(q,msg,timeout) OSQUEUE_PostGeneric(q, msg, timeout, TRUE)

/**
	Post a message to the front of a Queue.  The message becomes the first message in the
	Queue.
	@param	q		(in)	Queue to post message to.
	@param	*msg	(in)	Pointer to message to post.
	@param	timeout	(in)	TICKS_NO_WAIT or TICKS_FOREVER
	@return OSSTATUS_t		Result of operation.
**/

#define OSQUEUE_Post_PostToFront(q,msg,timeout) OSQUEUE_PostGeneric(q, msg, timeout, FALSE)

/** @endcond */

/**
	Change the name of an existing queue.
	@param	queue	(in)	Queue object
	@param	name	(in)	New name of the queue
**/

	static inline void OSQUEUE_ChangeName(Queue_t queue, const char *name) {

	}

/** @} */

#ifdef	__cplusplus
}
#endif

#endif
