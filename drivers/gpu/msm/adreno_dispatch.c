/* Copyright (c) 2013-2015, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/jiffies.h>
#include <linux/err.h>

#include "kgsl.h"
#include "kgsl_cffdump.h"
#include "kgsl_sharedmem.h"
#include "adreno.h"
#include "adreno_ringbuffer.h"
#include "adreno_trace.h"
#include "kgsl_sharedmem.h"

#define CMDQUEUE_NEXT(_i, _s) (((_i) + 1) % (_s))

/* Number of commands that can be queued in a context before it sleeps */
static unsigned int _context_cmdqueue_size = 50;

/* Number of milliseconds to wait for the context queue to clear */
static unsigned int _context_queue_wait = 10000;

/* Number of command batches sent at a time from a single context */
static unsigned int _context_cmdbatch_burst = 5;

/*
 * GFT throttle parameters. If GFT recovered more than
 * X times in Y ms invalidate the context and do not attempt recovery.
 * X -> _fault_throttle_burst
 * Y -> _fault_throttle_time
 */
static unsigned int _fault_throttle_time = 3000;
static unsigned int _fault_throttle_burst = 3;

/*
 * Maximum ringbuffer inflight for the single submitting context case - this
 * should be sufficiently high to keep the GPU loaded
 */
static unsigned int _dispatcher_q_inflight_hi = 15;

/*
 * Minimum inflight for the multiple context case - this should sufficiently low
 * to allow for lower latency context switching
 */
static unsigned int _dispatcher_q_inflight_lo = 4;

/* Command batch timeout (in milliseconds) */
static unsigned int _cmdbatch_timeout = 2000;

/* Interval for reading and comparing fault detection registers */
static unsigned int _fault_timer_interval = 200;

static int adreno_dispatch_process_cmdqueue(struct adreno_device *adreno_dev,
				struct adreno_dispatcher_cmdqueue *dispatch_q,
				int long_ib_detect);

/**
 * _track_context - Add a context ID to the list of recently seen contexts
 * for the command queue
 * @cmdqueue: cmdqueue to add the context to
 * @id: ID of the context to add
 *
 * This function is called when a new item is added to a context - this tracks
 * the number of active contexts seen in the last 500ms for the command queue
 */
static void _track_context(struct adreno_dispatcher_cmdqueue *cmdqueue,
		unsigned int id)
{
	struct adreno_context_list *list = cmdqueue->active_contexts;
	int oldest = -1, empty = -1;
	unsigned long age = 0;
	int i, count = 0;
	bool updated = false;

	for (i = 0; i < ACTIVE_CONTEXT_LIST_MAX; i++) {

		/* If the new ID matches the slot update the expire time */
		if (list[i].id == id) {
			list[i].jiffies = jiffies + msecs_to_jiffies(500);
			updated = true;
			count++;
			continue;
		}

		/* Remember and skip empty slots */
		if ((list[i].id == 0) ||
			time_after(jiffies, list[i].jiffies)) {
			empty = i;
			continue;
		}

		count++;

		/* Remember the oldest active entry */
		if (oldest == -1 || time_before(list[i].jiffies, age)) {
			age = list[i].jiffies;
			oldest = i;
		}
	}

	if (updated == false) {
		int pos = (empty != -1) ? empty : oldest;

		list[pos].jiffies = jiffies + msecs_to_jiffies(500);
		list[pos].id = id;
		count++;
	}

	cmdqueue->active_context_count = count;
}

/*
 *  If only one context has queued in the last 500 millseconds increase
 *  inflight to a high number to load up the GPU. If multiple contexts
 *  have queued drop the inflight for better context switch latency.
 *  If no contexts have queued what are you even doing here?
 */

static inline int
_cmdqueue_inflight(struct adreno_dispatcher_cmdqueue *cmdqueue)
{
	return (cmdqueue->active_context_count > 1)
		? _dispatcher_q_inflight_lo : _dispatcher_q_inflight_hi;
}

/**
 * fault_detect_read() - Read the set of fault detect registers
 * @device: Pointer to the KGSL device struct
 *
 * Read the set of fault detect registers and store them in the local array.
 * This is for the initial values that are compared later with
 * fault_detect_read_compare. Also store the initial timestamp of each rb
 * to compare the timestamps with.
 */
static void fault_detect_read(struct kgsl_device *device)
{
	int i;
	struct adreno_device *adreno_dev = ADRENO_DEVICE(device);

	for (i = 0; i < adreno_dev->num_ringbuffers; i++) {
		struct adreno_ringbuffer *rb = &(adreno_dev->ringbuffers[i]);
		adreno_rb_readtimestamp(device, rb,
			KGSL_TIMESTAMP_RETIRED, &(rb->fault_detect_ts));
	}

	for (i = 0; i < adreno_ft_regs_num; i++) {
		if (adreno_ft_regs[i] == 0)
			continue;
		kgsl_regread(device, adreno_ft_regs[i], &adreno_ft_regs_val[i]);
	}
}

/*
 * Check to see if the device is idle and that the global timestamp is up to
 * date
 */
static inline bool _isidle(struct kgsl_device *device)
{
	struct adreno_device *adreno_dev = ADRENO_DEVICE(device);
	unsigned int i;
	unsigned int reg_rbbm_status;

	if (!kgsl_state_is_awake(device))
		goto ret;

	adreno_readreg(adreno_dev, ADRENO_REG_RBBM_STATUS,
			&reg_rbbm_status);

	/*
	 * Check if gpu is busy by checking bits in RBBM_STATUS register
	 * which indicate gpu activity
	 */
	if (reg_rbbm_status & ADRENO_RBBM_STATUS_BUSY_MASK)
		return false;
ret:
	for (i = 0; i < adreno_ft_regs_num; i++)
		adreno_ft_regs_val[i] = 0;
	return true;
}

/**
 * fault_detect_read_compare() - Read the fault detect registers and compare
 * them to the current value
 * @device: Pointer to the KGSL device struct
 *
 * Read the set of fault detect registers and compare them to the current set
 * of registers.  Return 1 if any of the register values changed. Also, compare
 * if the current RB's timstamp has changed or not.
 */
static int fault_detect_read_compare(struct kgsl_device *device)
{
	struct adreno_device *adreno_dev = ADRENO_DEVICE(device);
	struct adreno_ringbuffer *rb = ADRENO_CURRENT_RINGBUFFER(adreno_dev);
	int i, ret = 0;
	unsigned int ts;

	/* Check to see if the device is idle - if so report no hang */
	if (_isidle(device) == true)
		ret = 1;

	for (i = 0; i < adreno_ft_regs_num; i++) {
		unsigned int val;

		if (adreno_ft_regs[i] == 0)
			continue;
		kgsl_regread(device, adreno_ft_regs[i], &val);
		if (val != adreno_ft_regs_val[i])
			ret = 1;
		adreno_ft_regs_val[i] = val;
	}

	if (!adreno_rb_readtimestamp(device, adreno_dev->cur_rb,
				KGSL_TIMESTAMP_RETIRED, &ts)) {
		if (ts != rb->fault_detect_ts)
			ret = 1;

		rb->fault_detect_ts = ts;
	}

	return ret;
}

/**
 * _retire_marker() - Retire a marker command batch without sending it to the
 * hardware
 * @cmdbatch: Pointer to the cmdbatch to retire
 *
 * In some cases marker commands can be retired by the software without going to
 * the GPU.  In those cases, update the memstore from the CPU, kick off the
 * event engine to handle expired events and destroy the command batch.
 */
static void _retire_marker(struct kgsl_cmdbatch *cmdbatch)
{
	struct kgsl_context *context = cmdbatch->context;
	struct kgsl_device *device = context->device;

	/*
	 * Write the start and end timestamp to the memstore to keep the
	 * accounting sane
	 */
	kgsl_sharedmem_writel(device, &device->memstore,
		KGSL_MEMSTORE_OFFSET(context->id, soptimestamp),
		cmdbatch->timestamp);

	kgsl_sharedmem_writel(device, &device->memstore,
		KGSL_MEMSTORE_OFFSET(context->id, eoptimestamp),
		cmdbatch->timestamp);


	/* Retire pending GPU events for the object */
	kgsl_process_event_group(device, &context->events);

	trace_adreno_cmdbatch_retired(cmdbatch, -1, 0, 0);
	kgsl_cmdbatch_destroy(cmdbatch);
}

static int _check_context_queue(struct adreno_context *drawctxt)
{
	int ret;

	spin_lock(&drawctxt->lock);

	/*
	 * Wake up if there is room in the context or if the whole thing got
	 * invalidated while we were asleep
	 */

	if (kgsl_context_invalid(&drawctxt->base))
		ret = 1;
	else
		ret = drawctxt->queued < _context_cmdqueue_size ? 1 : 0;

	spin_unlock(&drawctxt->lock);

	return ret;
}

/*
 * return true if this is a marker command and the dependent timestamp has
 * retired
 */
static bool _marker_expired(struct kgsl_cmdbatch *cmdbatch)
{
	return (cmdbatch->flags & KGSL_CMDBATCH_MARKER) &&
		kgsl_check_timestamp(cmdbatch->device, cmdbatch->context,
			cmdbatch->marker_timestamp);
}

static inline void _pop_cmdbatch(struct adreno_context *drawctxt)
{
	drawctxt->cmdqueue_head = CMDQUEUE_NEXT(drawctxt->cmdqueue_head,
		ADRENO_CONTEXT_CMDQUEUE_SIZE);
	drawctxt->queued--;
}

static struct kgsl_cmdbatch *_get_cmdbatch(struct adreno_context *drawctxt)
{
	struct kgsl_cmdbatch *cmdbatch = NULL;
	bool pending = false;

	if (drawctxt->cmdqueue_head == drawctxt->cmdqueue_tail)
		return NULL;

	cmdbatch = drawctxt->cmdqueue[drawctxt->cmdqueue_head];

	/* Check to see if this is a marker we can skip over */
	if (cmdbatch->flags & KGSL_CMDBATCH_MARKER) {
		if (_marker_expired(cmdbatch)) {
			_pop_cmdbatch(drawctxt);
			_retire_marker(cmdbatch);

			/* Get the next thing in the queue */
			return _get_cmdbatch(drawctxt);
		}

		/*
		 * If the marker isn't expired but the SKIP bit is set
		 * then there are real commands following this one in
		 * the queue.  This means that we need to dispatch the
		 * command so that we can keep the timestamp accounting
		 * correct.  If skip isn't set then we block this queue
		 * until the dependent timestamp expires
		 */

		if (!test_bit(CMDBATCH_FLAG_SKIP, &cmdbatch->priv))
			pending = true;
	}

	/*
	 * We may have cmdbatch timer running, which also uses same lock,
	 * take a lock with software interrupt disabled (bh) to avoid
	 * spin lock recursion.
	 */
	spin_lock_bh(&cmdbatch->lock);
	if (!list_empty(&cmdbatch->synclist))
		pending = true;
	spin_unlock_bh(&cmdbatch->lock);

	/*
	 * If changes are pending and the canary timer hasn't been
	 * started yet, start it
	 */
	if (pending) {
		/*
		 * If syncpoints are pending start the canary timer if
		 * it hasn't already been started
		 */
		if (!cmdbatch->timeout_jiffies) {
			cmdbatch->timeout_jiffies = jiffies + 5 * HZ;
			mod_timer(&cmdbatch->timer, cmdbatch->timeout_jiffies);
		}

		return ERR_PTR(-EAGAIN);
	}

	/*
	 * Otherwise, delete the timer to make sure it is good
	 * and dead before queuing the buffer
	 */
	del_timer_sync(&cmdbatch->timer);

	_pop_cmdbatch(drawctxt);
	return cmdbatch;
}

/**
 * adreno_dispatcher_get_cmdbatch() - Get a new command from a context queue
 * @drawctxt: Pointer to the adreno draw context
 *
 * Dequeue a new command batch from the context list
 */
static struct kgsl_cmdbatch *adreno_dispatcher_get_cmdbatch(
		struct adreno_context *drawctxt)
{
	struct kgsl_cmdbatch *cmdbatch;

	spin_lock(&drawctxt->lock);
	cmdbatch = _get_cmdbatch(drawctxt);
	spin_unlock(&drawctxt->lock);

	return cmdbatch;
}

/**
 * adreno_dispatcher_requeue_cmdbatch() - Put a command back on the context
 * queue
 * @drawctxt: Pointer to the adreno draw context
 * @cmdbatch: Pointer to the KGSL cmdbatch to requeue
 *
 * Failure to submit a command to the ringbuffer isn't the fault of the command
 * being submitted so if a failure happens, push it back on the head of the the
 * context queue to be reconsidered again unless the context got detached.
 */
static inline int adreno_dispatcher_requeue_cmdbatch(
		struct adreno_context *drawctxt, struct kgsl_cmdbatch *cmdbatch)
{
	unsigned int prev;
	spin_lock(&drawctxt->lock);

	if (kgsl_context_detached(&drawctxt->base) ||
		kgsl_context_invalid(&drawctxt->base)) {
		spin_unlock(&drawctxt->lock);
		/* get rid of this cmdbatch since the context is bad */
		kgsl_cmdbatch_destroy(cmdbatch);
		return -EINVAL;
	}

	prev = drawctxt->cmdqueue_head == 0 ?
		(ADRENO_CONTEXT_CMDQUEUE_SIZE - 1) :
		(drawctxt->cmdqueue_head - 1);

	/*
	 * The maximum queue size always needs to be one less then the size of
	 * the ringbuffer queue so there is "room" to put the cmdbatch back in
	 */

	BUG_ON(prev == drawctxt->cmdqueue_tail);

	drawctxt->cmdqueue[prev] = cmdbatch;
	drawctxt->queued++;

	/* Reset the command queue head to reflect the newly requeued change */
	drawctxt->cmdqueue_head = prev;
	spin_unlock(&drawctxt->lock);
	return 0;
}

/**
 * dispatcher_queue_context() - Queue a context in the dispatcher pending list
 * @dispatcher: Pointer to the adreno dispatcher struct
 * @drawctxt: Pointer to the adreno draw context
 *
 * Add a context to the dispatcher pending list.
 */
static void  dispatcher_queue_context(struct adreno_device *adreno_dev,
		struct adreno_context *drawctxt)
{
	struct adreno_dispatcher *dispatcher = &adreno_dev->dispatcher;

	/* Refuse to queue a detached context */
	if (kgsl_context_detached(&drawctxt->base))
		return;

	spin_lock(&dispatcher->plist_lock);

	if (plist_node_empty(&drawctxt->pending)) {
		/* Get a reference to the context while it sits on the list */
		if (_kgsl_context_get(&drawctxt->base)) {
			trace_dispatch_queue_context(drawctxt);
			plist_add(&drawctxt->pending, &dispatcher->pending);
		}
	}

	spin_unlock(&dispatcher->plist_lock);
}

/**
 * sendcmd() - Send a command batch to the GPU hardware
 * @dispatcher: Pointer to the adreno dispatcher struct
 * @cmdbatch: Pointer to the KGSL cmdbatch being sent
 *
 * Send a KGSL command batch to the GPU hardware
 */
static int sendcmd(struct adreno_device *adreno_dev,
	struct kgsl_cmdbatch *cmdbatch)
{
	struct kgsl_device *device = &adreno_dev->dev;
	struct adreno_dispatcher *dispatcher = &adreno_dev->dispatcher;
	struct adreno_dispatcher_cmdqueue *dispatch_q =
				ADRENO_CMDBATCH_DISPATCH_CMDQUEUE(cmdbatch);
	struct adreno_submit_time time;
	uint64_t secs = 0;
	unsigned long nsecs = 0;
	int ret;

	mutex_lock(&device->mutex);
	if (adreno_gpu_halt(adreno_dev) != 0) {
		mutex_unlock(&device->mutex);
		return -EBUSY;
	}

	dispatcher->inflight++;
	dispatch_q->inflight++;

	if (dispatcher->inflight == 1 &&
			!test_bit(ADRENO_DISPATCHER_POWER, &dispatcher->priv)) {
		/* Time to make the donuts.  Turn on the GPU */
		ret = kgsl_active_count_get(device);
		if (ret) {
			dispatcher->inflight--;
			mutex_unlock(&device->mutex);
			return ret;
		}

		set_bit(ADRENO_DISPATCHER_POWER, &dispatcher->priv);
	}

	if (test_bit(ADRENO_DEVICE_CMDBATCH_PROFILE, &adreno_dev->priv)) {
		set_bit(CMDBATCH_FLAG_PROFILE, &cmdbatch->priv);
		cmdbatch->profile_index = adreno_dev->cmdbatch_profile_index;
		adreno_dev->cmdbatch_profile_index =
			(adreno_dev->cmdbatch_profile_index + 1) %
			ADRENO_CMDBATCH_PROFILE_COUNT;
	}

	ret = adreno_ringbuffer_submitcmd(adreno_dev, cmdbatch, &time);

	/*
	 * On the first command, if the submission was successful, then read the
	 * fault registers.  If it failed then turn off the GPU. Sad face.
	 */

	if (dispatcher->inflight == 1) {
		if (ret == 0) {
			fault_detect_read(device);

			if (!test_and_set_bit(ADRENO_DISPATCHER_ACTIVE,
				&dispatcher->priv))
				INIT_COMPLETION(dispatcher->idle_gate);
		} else {
			kgsl_active_count_put(device);
			clear_bit(ADRENO_DISPATCHER_POWER, &dispatcher->priv);
		}
	}

	mutex_unlock(&device->mutex);

	if (ret) {
		dispatcher->inflight--;
		dispatch_q->inflight--;
		KGSL_DRV_ERR(device,
			"Unable to submit command to the ringbuffer %d\n", ret);
		return ret;
	}

	secs = time.ktime;
	nsecs = do_div(secs, 1000000000);

	trace_adreno_cmdbatch_submitted(cmdbatch, (int) dispatcher->inflight,
		time.ticks, (unsigned long) secs, nsecs / 1000);

	cmdbatch->submit_ticks = time.ticks;

	dispatch_q->cmd_q[dispatch_q->tail] = cmdbatch;
	dispatch_q->tail = (dispatch_q->tail + 1) %
		ADRENO_DISPATCH_CMDQUEUE_SIZE;

	/*
	 * If this is the first command in the pipe then the GPU will
	 * immediately start executing it so we can start the expiry timeout on
	 * the command batch here.  Subsequent command batches will have their
	 * timer started when the previous command batch is retired
	 */
	if (dispatcher->inflight == 1) {
		/*
		 * set the timer if the cmdbatch was submitted to current
		 * active RB else this timer will need to be set when the
		 * RB becomes active
		 */
		if (&(adreno_dev->cur_rb->dispatch_q) == dispatch_q) {
			cmdbatch->expires = jiffies +
				msecs_to_jiffies(_cmdbatch_timeout);
			mod_timer(&dispatcher->timer, cmdbatch->expires);
		}

		/* Start the fault detection timer */
		if (adreno_dev->fast_hang_detect)
			mod_timer(&dispatcher->fault_timer,
				jiffies +
				msecs_to_jiffies(_fault_timer_interval));
	}

	return 0;
}

/**
 * dispatcher_context_sendcmds() - Send commands from a context to the GPU
 * @adreno_dev: Pointer to the adreno device struct
 * @drawctxt: Pointer to the adreno context to dispatch commands from
 *
 * Dequeue and send a burst of commands from the specified context to the GPU
 * Returns postive if the context needs to be put back on the pending queue
 * 0 if the context is empty or detached and negative on error
 */
static int dispatcher_context_sendcmds(struct adreno_device *adreno_dev,
		struct adreno_context *drawctxt)
{
	struct adreno_dispatcher_cmdqueue *dispatch_q =
					&(drawctxt->rb->dispatch_q);
	int count = 0;
	int ret = 0;
	int inflight = _cmdqueue_inflight(dispatch_q);
	unsigned int timestamp;

	if (dispatch_q->inflight >= inflight)
		return -EBUSY;

	/*
	 * Each context can send a specific number of command batches per cycle
	 */
	while ((count < _context_cmdbatch_burst) &&
		(dispatch_q->inflight < inflight)) {
		struct kgsl_cmdbatch *cmdbatch;

		if (adreno_gpu_fault(adreno_dev) != 0)
			break;

		cmdbatch = adreno_dispatcher_get_cmdbatch(drawctxt);

		/*
		 * adreno_context_get_cmdbatch returns -EAGAIN if the current
		 * cmdbatch has pending sync points so no more to do here.
		 * When the sync points are satisfied then the context will get
		 * reqeueued
		 */

		if (IS_ERR_OR_NULL(cmdbatch)) {
			if (IS_ERR(cmdbatch))
				ret = PTR_ERR(cmdbatch);
			break;
		}

		/*
		 * If this is a synchronization submission then there are no
		 * commands to submit.  Discard it and get the next item from
		 * the queue.  Decrement count so this packet doesn't count
		 * against the burst for the context
		 */

		if (cmdbatch->flags & KGSL_CMDBATCH_SYNC) {
			kgsl_cmdbatch_destroy(cmdbatch);
			continue;
		}

		timestamp = cmdbatch->timestamp;

		ret = sendcmd(adreno_dev, cmdbatch);

		/*
		 * There are various reasons why we can't submit a command (no
		 * memory for the commands, full ringbuffer, etc) but none of
		 * these are actually the current command's fault.  Requeue it
		 * back on the context and let it come back around again if
		 * conditions improve
		 */
		if (ret) {
			if (adreno_dispatcher_requeue_cmdbatch(drawctxt,
				cmdbatch))
				ret = -EINVAL;
			break;
		}

		drawctxt->submitted_timestamp = timestamp;

		count++;
	}

	/*
	 * Wake up any snoozing threads if we have consumed any real commands
	 * or marker commands and we have room in the context queue.
	 */

	if (_check_context_queue(drawctxt))
		wake_up_all(&drawctxt->wq);

	if (!ret)
		ret = count;

	/* Return error or the number of commands queued */
	return ret;
}

/**
 * _adreno_dispatcher_issuecmds() - Issue commmands from pending contexts
 * @adreno_dev: Pointer to the adreno device struct
 *
 * Issue as many commands as possible (up to inflight) from the pending contexts
 * This function assumes the dispatcher mutex has been locked.
 */
static int _adreno_dispatcher_issuecmds(struct adreno_device *adreno_dev)
{
	struct adreno_dispatcher *dispatcher = &adreno_dev->dispatcher;
	struct adreno_context *drawctxt, *next;
	struct plist_head requeue, busy_list;
	int ret;

	/* Leave early if the dispatcher isn't in a happy state */
	if (adreno_gpu_fault(adreno_dev) != 0)
			return 0;

	plist_head_init(&requeue);
	plist_head_init(&busy_list);

	/* Try to fill the ringbuffers as much as possible */
	while (1) {

		/* Stop doing things if the dispatcher is paused or faulted */
		if (adreno_gpu_fault(adreno_dev) != 0)
			break;

		if (0 != adreno_gpu_halt(adreno_dev))
			break;

		spin_lock(&dispatcher->plist_lock);

		if (plist_head_empty(&dispatcher->pending)) {
			spin_unlock(&dispatcher->plist_lock);
			break;
		}

		/* Get the next entry on the list */
		drawctxt = plist_first_entry(&dispatcher->pending,
			struct adreno_context, pending);

		plist_del(&drawctxt->pending, &dispatcher->pending);

		spin_unlock(&dispatcher->plist_lock);

		if (kgsl_context_detached(&drawctxt->base) ||
			kgsl_context_invalid(&drawctxt->base)) {
			kgsl_context_put(&drawctxt->base);
			continue;
		}

		ret = dispatcher_context_sendcmds(adreno_dev, drawctxt);

		if (ret != 0) {
			spin_lock(&dispatcher->plist_lock);

			/*
			 * Check to seen if the context had been requeued while
			 * we were processing it (probably by another thread
			 * pushing commands). If it has then do a put to make
			 * sure the reference counting stays accurate.
			 * If the dispatch_q is full then put it on the
			 * busy list so it gets first preference when space
			 * becomes available.
			 * Otherwise put it on the requeue list since it may
			 * have more commands.
			 */

			if (!plist_node_empty(&drawctxt->pending)) {
				plist_del(&drawctxt->pending,
						&dispatcher->pending);
				kgsl_context_put(&drawctxt->base);
			}

			if (ret == -EBUSY)
				/* Inflight queue is full */
				plist_add(&drawctxt->pending, &busy_list);
			else
				plist_add(&drawctxt->pending, &requeue);

			spin_unlock(&dispatcher->plist_lock);
		} else {
			/*
			 * If the context doesn't need be requeued put back the
			 * refcount
			 */

			kgsl_context_put(&drawctxt->base);
		}
	}

	spin_lock(&dispatcher->plist_lock);

	/* Put the contexts that couldn't submit back on the pending list */
	plist_for_each_entry_safe(drawctxt, next, &busy_list, pending) {
		plist_del(&drawctxt->pending, &busy_list);
		plist_add(&drawctxt->pending, &dispatcher->pending);
	}

	/* Now put the contexts that need to be requeued back on the list */
	plist_for_each_entry_safe(drawctxt, next, &requeue, pending) {
		plist_del(&drawctxt->pending, &requeue);
		plist_add(&drawctxt->pending, &dispatcher->pending);
	}

	spin_unlock(&dispatcher->plist_lock);

	return 0;
}

/**
 * adreno_dispatcher_issuecmds() - Issue commmands from pending contexts
 * @adreno_dev: Pointer to the adreno device struct
 *
 * Lock the dispatcher and call _adreno_dispatcher_issueibcmds
 */
static int adreno_dispatcher_issuecmds(struct adreno_device *adreno_dev)
{
	struct adreno_dispatcher *dispatcher = &adreno_dev->dispatcher;
	int ret;

	/* If the dispatcher is busy then schedule the work for later */
	if (!mutex_trylock(&dispatcher->mutex)) {
		adreno_dispatcher_schedule(&adreno_dev->dev);
		return 0;
	}

	ret = _adreno_dispatcher_issuecmds(adreno_dev);
	mutex_unlock(&dispatcher->mutex);

	return ret;
}

/**
 * get_timestamp() - Return the next timestamp for the context
 * @drawctxt - Pointer to an adreno draw context struct
 * @cmdbatch - Pointer to a command batch
 * @timestamp - Pointer to a timestamp value possibly passed from the user
 *
 * Assign a timestamp based on the settings of the draw context and the command
 * batch.
 */
static int get_timestamp(struct adreno_context *drawctxt,
		struct kgsl_cmdbatch *cmdbatch, unsigned int *timestamp)
{
	/* Synchronization commands don't get a timestamp */
	if (cmdbatch->flags & KGSL_CMDBATCH_SYNC) {
		*timestamp = 0;
		return 0;
	}

	if (drawctxt->base.flags & KGSL_CONTEXT_USER_GENERATED_TS) {
		/*
		 * User specified timestamps need to be greater than the last
		 * issued timestamp in the context
		 */
		if (timestamp_cmp(drawctxt->timestamp, *timestamp) >= 0)
			return -ERANGE;

		drawctxt->timestamp = *timestamp;
	} else
		drawctxt->timestamp++;

	*timestamp = drawctxt->timestamp;
	return 0;
}

/**
 * adreno_dispactcher_queue_cmd() - Queue a new command in the context
 * @adreno_dev: Pointer to the adreno device struct
 * @drawctxt: Pointer to the adreno draw context
 * @cmdbatch: Pointer to the command batch being submitted
 * @timestamp: Pointer to the requested timestamp
 *
 * Queue a command in the context - if there isn't any room in the queue, then
 * block until there is
 */
int adreno_dispatcher_queue_cmd(struct adreno_device *adreno_dev,
		struct adreno_context *drawctxt, struct kgsl_cmdbatch *cmdbatch,
		uint32_t *timestamp)
{
	struct adreno_dispatcher_cmdqueue *dispatch_q =
				ADRENO_CMDBATCH_DISPATCH_CMDQUEUE(cmdbatch);
	int ret;

	spin_lock(&drawctxt->lock);

	if (kgsl_context_detached(&drawctxt->base)) {
		spin_unlock(&drawctxt->lock);
		return -EINVAL;
	}

	/*
	 * Force the preamble for this submission only - this is usually
	 * requested by the dispatcher as part of fault recovery
	 */

	if (test_and_clear_bit(ADRENO_CONTEXT_FORCE_PREAMBLE,
				&drawctxt->base.priv))
		set_bit(CMDBATCH_FLAG_FORCE_PREAMBLE, &cmdbatch->priv);

	/*
	 * Force the premable if set from userspace in the context or cmdbatch
	 * flags
	 */

	if ((drawctxt->base.flags & KGSL_CONTEXT_CTX_SWITCH) ||
		(cmdbatch->flags & KGSL_CMDBATCH_CTX_SWITCH))
		set_bit(CMDBATCH_FLAG_FORCE_PREAMBLE, &cmdbatch->priv);

	/* Skip this cmdbatch commands if IFH_NOP is enabled */
	if (drawctxt->base.flags & KGSL_CONTEXT_IFH_NOP)
		set_bit(CMDBATCH_FLAG_SKIP, &cmdbatch->priv);

	/*
	 * If we are waiting for the end of frame and it hasn't appeared yet,
	 * then mark the command batch as skipped.  It will still progress
	 * through the pipeline but it won't actually send any commands
	 */

	if (test_bit(ADRENO_CONTEXT_SKIP_EOF, &drawctxt->base.priv)) {
		set_bit(CMDBATCH_FLAG_SKIP, &cmdbatch->priv);

		/*
		 * If this command batch represents the EOF then clear the way
		 * for the dispatcher to continue submitting
		 */

		if (cmdbatch->flags & KGSL_CMDBATCH_END_OF_FRAME) {
			clear_bit(ADRENO_CONTEXT_SKIP_EOF,
				  &drawctxt->base.priv);

			/*
			 * Force the preamble on the next command to ensure that
			 * the state is correct
			 */
			set_bit(ADRENO_CONTEXT_FORCE_PREAMBLE,
				&drawctxt->base.priv);
		}
	}

	/* Wait for room in the context queue */

	while (drawctxt->queued >= _context_cmdqueue_size) {
		trace_adreno_drawctxt_sleep(drawctxt);
		spin_unlock(&drawctxt->lock);

		ret = wait_event_interruptible_timeout(drawctxt->wq,
			_check_context_queue(drawctxt),
			msecs_to_jiffies(_context_queue_wait));

		spin_lock(&drawctxt->lock);
		trace_adreno_drawctxt_wake(drawctxt);

		if (ret <= 0) {
			spin_unlock(&drawctxt->lock);
			return (ret == 0) ? -ETIMEDOUT : (int) ret;
		}
	}
	/*
	 * Account for the possiblity that the context got invalidated
	 * while we were sleeping
	 */

	if (kgsl_context_invalid(&drawctxt->base)) {
		spin_unlock(&drawctxt->lock);
		return -EDEADLK;
	}
	if (kgsl_context_detached(&drawctxt->base)) {
		spin_unlock(&drawctxt->lock);
		return -EINVAL;
	}

	ret = get_timestamp(drawctxt, cmdbatch, timestamp);
	if (ret) {
		spin_unlock(&drawctxt->lock);
		return ret;
	}

	cmdbatch->timestamp = *timestamp;

	if (cmdbatch->flags & KGSL_CMDBATCH_MARKER) {

		/*
		 * See if we can fastpath this thing - if nothing is queued
		 * and nothing is inflight retire without bothering the GPU
		 */

		if (!drawctxt->queued && kgsl_check_timestamp(cmdbatch->device,
			cmdbatch->context, drawctxt->queued_timestamp)) {
			trace_adreno_cmdbatch_queued(cmdbatch,
				drawctxt->queued);

			_retire_marker(cmdbatch);
			spin_unlock(&drawctxt->lock);
			return 0;
		}

		/*
		 * Remember the last queued timestamp - the marker will block
		 * until that timestamp is expired (unless another command
		 * comes along and forces the marker to execute)
		 */

		cmdbatch->marker_timestamp = drawctxt->queued_timestamp;
	}

	/* SYNC commands have timestamp 0 and will get optimized out anyway */
	if (!(cmdbatch->flags & KGSL_CONTEXT_SYNC))
		drawctxt->queued_timestamp = *timestamp;

	/*
	 * Set the fault tolerance policy for the command batch - assuming the
	 * context hasn't disabled FT use the current device policy
	 */

	if (drawctxt->base.flags & KGSL_CONTEXT_NO_FAULT_TOLERANCE)
		set_bit(KGSL_FT_DISABLE, &cmdbatch->fault_policy);
	else
		cmdbatch->fault_policy = adreno_dev->ft_policy;

	/* Put the command into the queue */
	drawctxt->cmdqueue[drawctxt->cmdqueue_tail] = cmdbatch;
	drawctxt->cmdqueue_tail = (drawctxt->cmdqueue_tail + 1) %
		ADRENO_CONTEXT_CMDQUEUE_SIZE;

	/*
	 * If this is a real command then we need to force any markers queued
	 * before it to dispatch to keep time linear - set the skip bit so
	 * the commands get NOPed.
	 */

	if (!(cmdbatch->flags & KGSL_CMDBATCH_MARKER)) {
		unsigned int i = drawctxt->cmdqueue_head;

		while (i != drawctxt->cmdqueue_tail) {
			if (drawctxt->cmdqueue[i]->flags & KGSL_CMDBATCH_MARKER)
				set_bit(CMDBATCH_FLAG_SKIP,
					&drawctxt->cmdqueue[i]->priv);

			i = CMDQUEUE_NEXT(i, ADRENO_CONTEXT_CMDQUEUE_SIZE);
		}
	}

	drawctxt->queued++;
	trace_adreno_cmdbatch_queued(cmdbatch, drawctxt->queued);

	_track_context(dispatch_q, drawctxt->base.id);

	spin_unlock(&drawctxt->lock);

	/* Add the context to the dispatcher pending list */
	dispatcher_queue_context(adreno_dev, drawctxt);

	/*
	 * Only issue commands if inflight is less than burst -this prevents us
	 * from sitting around waiting for the mutex on a busy system - the work
	 * loop will schedule it for us. Inflight is mutex protected but the
	 * worse that can happen is that it will go to 0 after we check and if
	 * it goes to 0 it is because the work loop decremented it and the work
	 * queue will try to schedule new commands anyway.
	 */

	if (dispatch_q->inflight < _context_cmdbatch_burst)
		adreno_dispatcher_issuecmds(adreno_dev);

	return 0;
}

static int _mark_context(int id, void *ptr, void *data)
{
	unsigned int guilty = *((unsigned int *) data);
	struct kgsl_context *context = ptr;

	/*
	 * If the context is guilty mark it as such.  Otherwise mark it as
	 * innocent if it had not already been marked as guilty.  If id is
	 * passed as 0 then mark EVERYBODY guilty (recovery failed)
	 */

	if (guilty == 0 || guilty == context->id)
		context->reset_status =
			KGSL_CTX_STAT_GUILTY_CONTEXT_RESET_EXT;
	else if (context->reset_status !=
		KGSL_CTX_STAT_GUILTY_CONTEXT_RESET_EXT)
		context->reset_status =
			KGSL_CTX_STAT_INNOCENT_CONTEXT_RESET_EXT;

	return 0;
}

/**
 * mark_guilty_context() - Mark the given context as guilty (failed recovery)
 * @device: Pointer to a KGSL device structure
 * @id: Context ID of the guilty context (or 0 to mark all as guilty)
 *
 * Mark the given (or all) context(s) as guilty (failed recovery)
 */
static void mark_guilty_context(struct kgsl_device *device, unsigned int id)
{
	/* Mark the status for all the contexts in the device */

	read_lock(&device->context_lock);
	idr_for_each(&device->context_idr, _mark_context, &id);
	read_unlock(&device->context_lock);
}

/*
 * If an IB inside of the command batch has a gpuaddr that matches the base
 * passed in then zero the size which effectively skips it when it is submitted
 * in the ringbuffer.
 */
static void cmdbatch_skip_ib(struct kgsl_cmdbatch *cmdbatch,
				unsigned int base)
{
	struct kgsl_memobj_node *ib;

	list_for_each_entry(ib, &cmdbatch->cmdlist, node) {
		if (ib->gpuaddr == base) {
			ib->priv |= MEMOBJ_SKIP;
			if (base)
				return;
		}
	}
}

static void cmdbatch_skip_cmd(struct kgsl_cmdbatch *cmdbatch,
	struct kgsl_cmdbatch **replay, int count)
{
	struct adreno_context *drawctxt = ADRENO_CONTEXT(cmdbatch->context);
	int i;

	/*
	 * SKIPCMD policy: next IB issued for this context is tentative
	 * if it fails we assume that GFT failed and if it succeeds
	 * we mark GFT as a success.
	 *
	 * Find next commandbatch for the faulting context
	 * If commandbatch is found
	 * a) store the current commandbatch fault_policy in context's next
	 *    commandbatch fault_policy
	 * b) force preamble for next commandbatch
	 */
	for (i = 1; i < count; i++) {
		if (replay[i]->context->id == cmdbatch->context->id) {
			replay[i]->fault_policy = replay[0]->fault_policy;
			set_bit(CMDBATCH_FLAG_FORCE_PREAMBLE, &replay[i]->priv);
			set_bit(KGSL_FT_SKIPCMD, &replay[i]->fault_recovery);
			break;
		}
	}

	/*
	 * If we did not find the next cmd then
	 * a) set a flag for next command issued in this context
	 * b) store the fault_policy, this fault_policy becomes the policy of
	 *    next command issued in this context
	 */
	if ((i == count) && drawctxt) {
		set_bit(ADRENO_CONTEXT_SKIP_CMD, &drawctxt->base.priv);
		drawctxt->fault_policy = replay[0]->fault_policy;
	}

	/* set the flags to skip this cmdbatch */
	set_bit(CMDBATCH_FLAG_SKIP, &cmdbatch->priv);
	cmdbatch->fault_recovery = 0;
}

static void cmdbatch_skip_frame(struct kgsl_cmdbatch *cmdbatch,
	struct kgsl_cmdbatch **replay, int count)
{
	struct adreno_context *drawctxt = ADRENO_CONTEXT(cmdbatch->context);
	int skip = 1;
	int i;

	for (i = 0; i < count; i++) {

		/*
		 * Only operate on command batches that belong to the
		 * faulting context
		 */

		if (replay[i]->context->id != cmdbatch->context->id)
			continue;

		/*
		 * Skip all the command batches in this context until
		 * the EOF flag is seen.  If the EOF flag is seen then
		 * force the preamble for the next command.
		 */

		if (skip) {
			set_bit(CMDBATCH_FLAG_SKIP, &replay[i]->priv);

			if (replay[i]->flags & KGSL_CMDBATCH_END_OF_FRAME)
				skip = 0;
		} else {
			set_bit(CMDBATCH_FLAG_FORCE_PREAMBLE, &replay[i]->priv);
			return;
		}
	}

	/*
	 * If the EOF flag hasn't been seen yet then set the flag in the
	 * drawctxt to keep looking for it
	 */

	if (skip && drawctxt)
		set_bit(ADRENO_CONTEXT_SKIP_EOF, &drawctxt->base.priv);

	/*
	 * If we did see the EOF flag then force the preamble on for the
	 * next command issued on this context
	 */

	if (!skip && drawctxt)
		set_bit(ADRENO_CONTEXT_FORCE_PREAMBLE, &drawctxt->base.priv);
}

static void remove_invalidated_cmdbatches(struct kgsl_device *device,
		struct kgsl_cmdbatch **replay, int count)
{
	int i;

	for (i = 0; i < count; i++) {
		struct kgsl_cmdbatch *cmd = replay[i];
		if (cmd == NULL)
			continue;

		if (kgsl_context_detached(cmd->context) ||
			kgsl_context_invalid(cmd->context)) {
			replay[i] = NULL;

			mutex_lock(&device->mutex);
			kgsl_cancel_events_timestamp(device,
				&cmd->context->events, cmd->timestamp);
			mutex_unlock(&device->mutex);

			kgsl_cmdbatch_destroy(cmd);
		}
	}
}

static char _pidname[TASK_COMM_LEN];

static inline const char *_kgsl_context_comm(struct kgsl_context *context)
{
	if (context && context->proc_priv)
		strlcpy(_pidname, context->proc_priv->comm, sizeof(_pidname));
	else
		snprintf(_pidname, TASK_COMM_LEN, "unknown");

	return _pidname;
}

#define pr_fault(_d, _c, fmt, args...) \
		dev_err((_d)->dev, "%s[%d]: " fmt, \
		_kgsl_context_comm((_c)->context), \
		(_c)->context->proc_priv->pid, ##args)


static void adreno_fault_header(struct kgsl_device *device,
	struct kgsl_cmdbatch *cmdbatch)
{
	struct adreno_device *adreno_dev = ADRENO_DEVICE(device);
	unsigned int status, base, rptr, wptr, ib1base, ib2base, ib1sz, ib2sz;

	kgsl_regread(device,
			adreno_getreg(adreno_dev, ADRENO_REG_RBBM_STATUS),
			&status);
	kgsl_regread(device,
		adreno_getreg(adreno_dev, ADRENO_REG_CP_RB_BASE),
		&base);
	kgsl_regread(device,
		adreno_getreg(adreno_dev, ADRENO_REG_CP_RB_RPTR),
		&rptr);
	kgsl_regread(device,
		adreno_getreg(adreno_dev, ADRENO_REG_CP_RB_WPTR),
		&wptr);
	kgsl_regread(device,
		adreno_getreg(adreno_dev, ADRENO_REG_CP_IB1_BASE),
		&ib1base);
	kgsl_regread(device,
		adreno_getreg(adreno_dev, ADRENO_REG_CP_IB1_BUFSZ),
		&ib1sz);
	kgsl_regread(device,
		adreno_getreg(adreno_dev, ADRENO_REG_CP_IB2_BASE),
		&ib2base);
	kgsl_regread(device,
		adreno_getreg(adreno_dev, ADRENO_REG_CP_IB2_BUFSZ),
		&ib2sz);

	trace_adreno_gpu_fault(cmdbatch->context->id, cmdbatch->timestamp,
		status, rptr, wptr, ib1base, ib1sz, ib2base, ib2sz);

	pr_fault(device, cmdbatch,
		"gpu fault ctx %d ts %d status %8.8X rb %4.4x/%4.4x ib1 %8.8x/%4.4x ib2 %8.8x/%4.4x\n",
		cmdbatch->context->id, cmdbatch->timestamp, status,
		rptr, wptr, ib1base, ib1sz, ib2base, ib2sz);
}

void adreno_fault_skipcmd_detached(struct kgsl_device *device,
				 struct adreno_context *drawctxt,
				 struct kgsl_cmdbatch *cmdbatch)
{
	if (test_bit(ADRENO_CONTEXT_SKIP_CMD, &drawctxt->base.priv) &&
			kgsl_context_detached(&drawctxt->base)) {
		pr_fault(device, cmdbatch, "gpu %s ctx %d\n",
			 "detached", cmdbatch->context->id);
		clear_bit(ADRENO_CONTEXT_SKIP_CMD, &drawctxt->base.priv);
	}
}

/**
 * process_cmdbatch_fault() - Process a cmdbatch for fault policies
 * @device: Device on which the cmdbatch caused a fault
 * @replay: List of cmdbatches that are to be replayed on the device. The
 * faulting cmdbatch is the first command in the replay list and the remaining
 * cmdbatches in the list are commands that were submitted to the same queue
 * as the faulting one.
 * @count: Number of cmdbatches in replay
 * @base: The IB1 base at the time of fault
 * @fault: The fault type
 */
void process_cmdbatch_fault(struct kgsl_device *device,
		struct kgsl_cmdbatch **replay, int count,
		unsigned int base,
		int fault)
{
	struct kgsl_cmdbatch *cmdbatch = replay[0];
	int i;
	char *state = "failed";

	/*
	 * If GFT recovered more than X times in Y ms invalidate the context
	 * and do not attempt recovery.
	 * Example: X==3 and Y==3000 ms, GPU hung at 500ms, 1700ms, 25000ms and
	 * 3000ms for the same context, we will not try FT and invalidate the
	 * context @3000ms because context triggered GFT more than 3 times in
	 * last 3 seconds. If a context caused recoverable GPU hangs
	 * where 1st and 4th gpu hang are more than 3 seconds apart we
	 * won't disable GFT and invalidate the context.
	 */
	if (test_bit(KGSL_FT_THROTTLE, &cmdbatch->fault_policy)) {
		if (time_after(jiffies, (cmdbatch->context->fault_time
				+ msecs_to_jiffies(_fault_throttle_time)))) {
			cmdbatch->context->fault_time = jiffies;
			cmdbatch->context->fault_count = 1;
		} else {
			cmdbatch->context->fault_count++;
			if (cmdbatch->context->fault_count >
					_fault_throttle_burst) {
				set_bit(KGSL_FT_DISABLE,
						&cmdbatch->fault_policy);
				pr_fault(device, cmdbatch,
					 "gpu fault threshold exceeded %d faults in %d msecs\n",
					 _fault_throttle_burst,
					 _fault_throttle_time);
			}
		}
	}

	/*
	 * If FT is disabled for this cmdbatch invalidate immediately
	 */

	if (test_bit(KGSL_FT_DISABLE, &cmdbatch->fault_policy) ||
		test_bit(KGSL_FT_TEMP_DISABLE, &cmdbatch->fault_policy)) {
		state = "skipped";
		bitmap_zero(&cmdbatch->fault_policy, BITS_PER_LONG);
	}

	/* If the context is detached do not run FT on context */
	if (kgsl_context_detached(cmdbatch->context)) {
		state = "detached";
		bitmap_zero(&cmdbatch->fault_policy, BITS_PER_LONG);
	}

	/*
	 * Set a flag so we don't print another PM dump if the cmdbatch fails
	 * again on replay
	 */

	set_bit(KGSL_FT_SKIP_PMDUMP, &cmdbatch->fault_policy);

	/*
	 * A hardware fault generally means something was deterministically
	 * wrong with the command batch - no point in trying to replay it
	 * Clear the replay bit and move on to the next policy level
	 */

	if (fault & ADRENO_HARD_FAULT)
		clear_bit(KGSL_FT_REPLAY, &(cmdbatch->fault_policy));

	/*
	 * A timeout fault means the IB timed out - clear the policy and
	 * invalidate - this will clear the FT_SKIP_PMDUMP bit but that is okay
	 * because we won't see this cmdbatch again
	 */

	if (fault & ADRENO_TIMEOUT_FAULT)
		bitmap_zero(&cmdbatch->fault_policy, BITS_PER_LONG);

	/*
	 * If the context had a GPU page fault then it is likely it would fault
	 * again if replayed
	 */

	if (test_bit(KGSL_CONTEXT_PRIV_PAGEFAULT,
		     &cmdbatch->context->priv)) {
		/* we'll need to resume the mmu later... */
		clear_bit(KGSL_FT_REPLAY, &cmdbatch->fault_policy);
		clear_bit(KGSL_CONTEXT_PRIV_PAGEFAULT,
			  &cmdbatch->context->priv);
	}

	/*
	 * Execute the fault tolerance policy. Each command batch stores the
	 * current fault policy that was set when it was queued.
	 * As the options are tried in descending priority
	 * (REPLAY -> SKIPIBS -> SKIPFRAME -> NOTHING) the bits are cleared
	 * from the cmdbatch policy so the next thing can be tried if the
	 * change comes around again
	 */

	/* Replay the hanging command batch again */
	if (test_and_clear_bit(KGSL_FT_REPLAY, &cmdbatch->fault_policy)) {
		trace_adreno_cmdbatch_recovery(cmdbatch, BIT(KGSL_FT_REPLAY));
		set_bit(KGSL_FT_REPLAY, &cmdbatch->fault_recovery);
		return;
	}

	/*
	 * Skip the last IB1 that was played but replay everything else.
	 * Note that the last IB1 might not be in the "hung" command batch
	 * because the CP may have caused a page-fault while it was prefetching
	 * the next IB1/IB2. walk all outstanding commands and zap the
	 * supposedly bad IB1 where ever it lurks.
	 */

	if (test_and_clear_bit(KGSL_FT_SKIPIB, &cmdbatch->fault_policy)) {
		trace_adreno_cmdbatch_recovery(cmdbatch, BIT(KGSL_FT_SKIPIB));
		set_bit(KGSL_FT_SKIPIB, &cmdbatch->fault_recovery);

		for (i = 0; i < count; i++) {
			if (replay[i] != NULL &&
				replay[i]->context->id == cmdbatch->context->id)
				cmdbatch_skip_ib(replay[i], base);
		}

		return;
	}

	/* Skip the faulted command batch submission */
	if (test_and_clear_bit(KGSL_FT_SKIPCMD, &cmdbatch->fault_policy)) {
		trace_adreno_cmdbatch_recovery(cmdbatch, BIT(KGSL_FT_SKIPCMD));

		/* Skip faulting command batch */
		cmdbatch_skip_cmd(cmdbatch, replay, count);

		return;
	}

	if (test_and_clear_bit(KGSL_FT_SKIPFRAME, &cmdbatch->fault_policy)) {
		trace_adreno_cmdbatch_recovery(cmdbatch,
			BIT(KGSL_FT_SKIPFRAME));
		set_bit(KGSL_FT_SKIPFRAME, &cmdbatch->fault_recovery);

		/*
		 * Skip all the pending command batches for this context until
		 * the EOF frame is seen
		 */
		cmdbatch_skip_frame(cmdbatch, replay, count);
		return;
	}

	/* If we get here then all the policies failed */

	pr_fault(device, cmdbatch, "gpu %s ctx %d ts %d\n",
		state, cmdbatch->context->id, cmdbatch->timestamp);

	/* Mark the context as failed */
	mark_guilty_context(device, cmdbatch->context->id);

	/* Invalidate the context */
	adreno_drawctxt_invalidate(device, cmdbatch->context);
}

/**
 * recover_dispatch_q() - Recover all commands in a dispatch queue by
 * resubmitting the commands
 * @device: Device on which recovery is performed
 * @dispatch_q: The command queue to recover
 * @fault: Faults caused by the command in the dispatch q
 * @base: The IB1 base during the fault
 */
static void recover_dispatch_q(struct kgsl_device *device,
		struct adreno_dispatcher_cmdqueue *dispatch_q,
		int fault,
		unsigned int base)
{
	struct adreno_device *adreno_dev = ADRENO_DEVICE(device);
	struct kgsl_cmdbatch **replay = NULL;
	unsigned int ptr;
	int first = 0;
	int count = 0;
	int i;

	/* Allocate memory to store the inflight commands */
	replay = kzalloc(sizeof(*replay) * dispatch_q->inflight, GFP_KERNEL);

	if (replay == NULL) {
		unsigned int ptr = dispatch_q->head;

		/* Recovery failed - mark everybody on this q guilty */
		while (ptr != dispatch_q->tail) {
			struct kgsl_context *context =
				dispatch_q->cmd_q[ptr]->context;

			mark_guilty_context(device, context->id);
			adreno_drawctxt_invalidate(device, context);
			kgsl_cmdbatch_destroy(dispatch_q->cmd_q[ptr]);

			ptr = CMDQUEUE_NEXT(ptr, ADRENO_DISPATCH_CMDQUEUE_SIZE);
		}

		/*
		 * Set the replay count to zero - this will ensure that the
		 * hardware gets reset but nothing else gets played
		 */

		count = 0;
		goto replay;
	}

	/* Copy the inflight command batches into the temporary storage */
	ptr = dispatch_q->head;

	while (ptr != dispatch_q->tail) {
		replay[count++] = dispatch_q->cmd_q[ptr];
		ptr = CMDQUEUE_NEXT(ptr, ADRENO_DISPATCH_CMDQUEUE_SIZE);
	}

	if (fault && count)
		process_cmdbatch_fault(device, replay,
					count, base, fault);
replay:
	dispatch_q->inflight = 0;
	dispatch_q->head = dispatch_q->tail = 0;
	/* Remove any pending command batches that have been invalidated */
	remove_invalidated_cmdbatches(device, replay, count);

	/* Replay the pending command buffers */
	for (i = 0; i < count; i++) {

		int ret;

		if (replay[i] == NULL)
			continue;

		/*
		 * Force the preamble on the first command (if applicable) to
		 * avoid any strange stage issues
		 */

		if (first == 0) {
			set_bit(CMDBATCH_FLAG_FORCE_PREAMBLE, &replay[i]->priv);
			first = 1;
		}

		/*
		 * Force each command batch to wait for idle - this avoids weird
		 * CP parse issues
		 */

		set_bit(CMDBATCH_FLAG_WFI, &replay[i]->priv);

		ret = sendcmd(adreno_dev, replay[i]);

		/*
		 * If sending the command fails, then try to recover by
		 * invalidating the context
		 */

		if (ret) {
			pr_fault(device, replay[i],
				"gpu reset failed ctx %d ts %d\n",
				replay[i]->context->id, replay[i]->timestamp);

			/* Mark this context as guilty (failed recovery) */
			mark_guilty_context(device, replay[i]->context->id);

			adreno_drawctxt_invalidate(device, replay[i]->context);
			remove_invalidated_cmdbatches(device, &replay[i],
				count - i);
		}
	}

	/* Clear the fault bit */
	clear_bit(ADRENO_DEVICE_FAULT, &adreno_dev->priv);

	kfree(replay);
}

static int dispatcher_do_fault(struct kgsl_device *device)
{
	struct adreno_device *adreno_dev = ADRENO_DEVICE(device);
	struct adreno_dispatcher *dispatcher = &adreno_dev->dispatcher;
	struct adreno_dispatcher_cmdqueue *dispatch_q = NULL, *dispatch_q_temp;
	struct adreno_ringbuffer *rb;
	unsigned int reg, base;
	struct kgsl_cmdbatch *cmdbatch = NULL;
	int ret, i;
	int fault;
	int halt;

	fault = atomic_xchg(&dispatcher->fault, 0);
	if (fault == 0)
		return 0;
	/*
	 * Return early if no command inflight - can happen on
	 * false hang detects
	 */
	if (dispatcher->inflight == 0) {
		KGSL_DRV_WARN(device,
		"dispatcher_do_fault with 0 inflight commands\n");
		/*
		 * For certain faults like h/w fault the interrupts are
		 * turned off, re-enable here
		 */
		mutex_lock(&device->mutex);
		if (device->state == KGSL_STATE_AWARE)
			ret = kgsl_pwrctrl_change_state(device,
				KGSL_STATE_ACTIVE);
		else
			ret = 0;
		mutex_unlock(&device->mutex);
		return ret;
	}

	/* Turn off all the timers */
	del_timer_sync(&dispatcher->timer);
	del_timer_sync(&dispatcher->fault_timer);

	mutex_lock(&device->mutex);

	/* hang opcode */
	kgsl_cffdump_hang(device);

	adreno_readreg(adreno_dev, ADRENO_REG_CP_RB_BASE, &base);

	/*
	 * If the fault was due to a timeout then stop the CP to ensure we don't
	 * get activity while we are trying to dump the state of the system
	 */

	if (fault & ADRENO_TIMEOUT_FAULT) {
		adreno_readreg(adreno_dev, ADRENO_REG_CP_ME_CNTL, &reg);
		reg |= (1 << 27) | (1 << 28);
		adreno_writereg(adreno_dev, ADRENO_REG_CP_ME_CNTL, reg);
	}
	/*
	 * retire cmdbatches from all the dispatch_q's before starting recovery
	 */
	FOR_EACH_RINGBUFFER(adreno_dev, rb, i) {
		adreno_dispatch_process_cmdqueue(adreno_dev,
			&(rb->dispatch_q), 0);
		/* Select the active dispatch_q */
		if (base == rb->buffer_desc.gpuaddr)
			dispatch_q = &(rb->dispatch_q);
	}

	if (dispatch_q && (dispatch_q->tail != dispatch_q->head)) {
		cmdbatch = dispatch_q->cmd_q[dispatch_q->head];
		trace_adreno_cmdbatch_fault(cmdbatch, fault);
	}

	/* Set pagefault if it occurred */
	kgsl_mmu_set_pagefault(&device->mmu);

	adreno_readreg(adreno_dev, ADRENO_REG_CP_IB1_BASE, &base);

	/*
	 * Dump the snapshot information if this is the first
	 * detected fault for the oldest active command batch
	 */

	if (cmdbatch &&
		!test_bit(KGSL_FT_SKIP_PMDUMP, &cmdbatch->fault_policy)) {
		adreno_fault_header(device, cmdbatch);
		kgsl_device_snapshot(device, cmdbatch->context);
	}

	/* Reset the dispatcher queue */
	dispatcher->inflight = 0;

	/* Reset the GPU and make sure halt is not set during recovery */
	halt = adreno_gpu_halt(adreno_dev);
	adreno_clear_gpu_halt(adreno_dev);
	ret = adreno_reset(device);
	mutex_unlock(&device->mutex);
	/* if any other fault got in until reset then ignore */
	atomic_set(&dispatcher->fault, 0);

	/* If adreno_reset() fails then what hope do we have for the future? */
	BUG_ON(ret);

	/* recover all the dispatch_q's starting with the one that hung */
	if (dispatch_q)
		recover_dispatch_q(device, dispatch_q, fault, base);
	FOR_EACH_RINGBUFFER(adreno_dev, rb, i) {
		dispatch_q_temp = &(rb->dispatch_q);
		if (dispatch_q_temp != dispatch_q)
			recover_dispatch_q(device, dispatch_q_temp, 0, base);
	}

	atomic_add(halt, &adreno_dev->halt);

	return 1;
}

static inline int cmdbatch_consumed(struct kgsl_cmdbatch *cmdbatch,
		unsigned int consumed, unsigned int retired)
{
	return ((timestamp_cmp(cmdbatch->timestamp, consumed) >= 0) &&
		(timestamp_cmp(retired, cmdbatch->timestamp) < 0));
}

static void _print_recovery(struct kgsl_device *device,
		struct kgsl_cmdbatch *cmdbatch)
{
	static struct {
		unsigned int mask;
		const char *str;
	} flags[] = { ADRENO_FT_TYPES };

	int i, nr = find_first_bit(&cmdbatch->fault_recovery, BITS_PER_LONG);
	char *result = "unknown";

	for (i = 0; i < ARRAY_SIZE(flags); i++) {
		if (flags[i].mask == BIT(nr)) {
			result = (char *) flags[i].str;
			break;
		}
	}

	pr_fault(device, cmdbatch,
		"gpu %s ctx %d ts %d policy %lX\n",
		result, cmdbatch->context->id, cmdbatch->timestamp,
		cmdbatch->fault_recovery);
}

static void cmdbatch_profile_ticks(struct adreno_device *adreno_dev,
	struct kgsl_cmdbatch *cmdbatch, uint64_t *start, uint64_t *retire)
{
	void *ptr = adreno_dev->cmdbatch_profile_buffer.hostptr;
	struct adreno_cmdbatch_profile_entry *entry;

	entry = (struct adreno_cmdbatch_profile_entry *)
		(ptr + (cmdbatch->profile_index * sizeof(*entry)));

	rmb();
	*start = entry->started;
	*retire = entry->retired;
}

static int adreno_dispatch_process_cmdqueue(struct adreno_device *adreno_dev,
				struct adreno_dispatcher_cmdqueue *dispatch_q,
				int long_ib_detect)
{
	struct kgsl_device *device = &(adreno_dev->dev);
	struct adreno_dispatcher *dispatcher = &(adreno_dev->dispatcher);
	uint64_t start_ticks = 0, retire_ticks = 0;

	struct adreno_dispatcher_cmdqueue *active_q =
			&(adreno_dev->cur_rb->dispatch_q);
	int count = 0;

	while (dispatch_q->head != dispatch_q->tail) {
		struct kgsl_cmdbatch *cmdbatch =
			dispatch_q->cmd_q[dispatch_q->head];
		struct adreno_context *drawctxt;
		BUG_ON(cmdbatch == NULL);

		drawctxt = ADRENO_CONTEXT(cmdbatch->context);

		/*
		 * First try to expire the timestamp. This happens if the
		 * context is valid and the timestamp expired normally or if the
		 * context was destroyed before the command batch was finished
		 * in the GPU.  Either way retire the command batch advance the
		 * pointers and continue processing the queue
		 */

		if (kgsl_check_timestamp(device, cmdbatch->context,
			cmdbatch->timestamp)) {

			/*
			 * If the cmdbatch in question had faulted announce its
			 * successful completion to the world
			 */

			if (cmdbatch->fault_recovery != 0) {
				/* Mark the context as faulted and recovered */
				set_bit(ADRENO_CONTEXT_FAULT,
					&cmdbatch->context->priv);

				_print_recovery(device, cmdbatch);
			}

			/* Reduce the number of inflight command batches */
			dispatcher->inflight--;
			dispatch_q->inflight--;

			/*
			 * If kernel profiling is enabled get the submit and
			 * retired ticks from the buffer
			 */

			if (test_bit(CMDBATCH_FLAG_PROFILE, &cmdbatch->priv))
				cmdbatch_profile_ticks(adreno_dev, cmdbatch,
					&start_ticks, &retire_ticks);

			trace_adreno_cmdbatch_retired(cmdbatch,
				(int) dispatcher->inflight, start_ticks,
				retire_ticks);

			/* Record the delta between submit and retire ticks */
			drawctxt->submit_retire_ticks[drawctxt->ticks_index] =
				retire_ticks - cmdbatch->submit_ticks;

			drawctxt->ticks_index = (drawctxt->ticks_index + 1)
				% SUBMIT_RETIRE_TICKS_SIZE;

			/* Zero the old entry*/
			dispatch_q->cmd_q[dispatch_q->head] = NULL;

			/* Advance the buffer head */
			dispatch_q->head = CMDQUEUE_NEXT(dispatch_q->head,
				ADRENO_DISPATCH_CMDQUEUE_SIZE);

			/* Destroy the retired command batch */
			kgsl_cmdbatch_destroy(cmdbatch);

			/* Update the expire time for the next command batch */

			if (dispatch_q->inflight > 0 &&
				dispatch_q == active_q) {
				cmdbatch =
					dispatch_q->cmd_q[dispatch_q->head];
				cmdbatch->expires = jiffies +
					msecs_to_jiffies(_cmdbatch_timeout);
			}

			count++;
			continue;
		}
		/*
		 * Break here if fault detection is disabled for the context or
		 * if the long running IB detection is disaled device wide or
		 * if the dispatch q is not active
		 * Long running command buffers will be allowed to run to
		 * completion - but badly behaving command buffers (infinite
		 * shaders etc) can end up running forever.
		 */

		if (!long_ib_detect ||
			drawctxt->base.flags & KGSL_CONTEXT_NO_FAULT_TOLERANCE
			|| dispatch_q != active_q)
			break;

		/*
		 * The last line of defense is to check if the command batch has
		 * timed out. If we get this far but the timeout hasn't expired
		 * yet then the GPU is still ticking away
		 */

		if (time_is_after_jiffies(cmdbatch->expires))
			break;

		/* Boom goes the dynamite */

		pr_fault(device, cmdbatch,
			"gpu timeout ctx %d ts %d\n",
			cmdbatch->context->id, cmdbatch->timestamp);

		adreno_set_gpu_fault(adreno_dev, ADRENO_TIMEOUT_FAULT);
		break;
	}
	return count;
}

/**
 * adreno_dispatcher_work() - Master work handler for the dispatcher
 * @work: Pointer to the work struct for the current work queue
 *
 * Process expired commands and send new ones.
 */
static void adreno_dispatcher_work(struct work_struct *work)
{
	struct adreno_dispatcher *dispatcher =
		container_of(work, struct adreno_dispatcher, work);
	struct adreno_device *adreno_dev =
		container_of(dispatcher, struct adreno_device, dispatcher);
	struct kgsl_device *device = &adreno_dev->dev;
	int count;

	mutex_lock(&dispatcher->mutex);

	/* process the active q*/
	count = adreno_dispatch_process_cmdqueue(adreno_dev,
		&(adreno_dev->cur_rb->dispatch_q),
		adreno_dev->long_ib_detect);

	/* Check if gpu fault occurred */
	if (dispatcher_do_fault(device))
		goto done;

	/*
	 * If inflight went to 0, queue back up the event processor to catch
	 * stragglers
	 */
	if (dispatcher->inflight == 0 && count)
		queue_work(device->work_queue, &device->event_work);

	/* Try to dispatch new commands */
	_adreno_dispatcher_issuecmds(adreno_dev);

done:
	/* Either update the timer for the next command batch or disable it */
	if (dispatcher->inflight) {
		struct kgsl_cmdbatch *cmdbatch =
			adreno_dev->cur_rb->dispatch_q.cmd_q[
				adreno_dev->cur_rb->dispatch_q.head];
		if (cmdbatch)
			/* Update the timeout timer for the next cmdbatch */
			mod_timer(&dispatcher->timer, cmdbatch->expires);

		/* There are still things in flight - update the idle counts */
		mutex_lock(&device->mutex);
		kgsl_pwrscale_update(device);
		mod_timer(&device->idle_timer, jiffies +
				device->pwrctrl.interval_timeout);
		mutex_unlock(&device->mutex);
	} else {
		/* There is nothing left in the pipeline.  Shut 'er down boys */
		mutex_lock(&device->mutex);

		if (test_and_clear_bit(ADRENO_DISPATCHER_ACTIVE,
			&dispatcher->priv))
			complete_all(&dispatcher->idle_gate);

		/*
		 * Stop the fault timer before decrementing the active count to
		 * avoid reading the hardware registers while we are trying to
		 * turn clocks off
		 */
		del_timer_sync(&dispatcher->fault_timer);

		if (test_bit(ADRENO_DISPATCHER_POWER, &dispatcher->priv)) {
			kgsl_active_count_put(device);
			clear_bit(ADRENO_DISPATCHER_POWER, &dispatcher->priv);
		}

		mutex_unlock(&device->mutex);
	}

	mutex_unlock(&dispatcher->mutex);
}

void adreno_dispatcher_schedule(struct kgsl_device *device)
{
	struct adreno_device *adreno_dev = ADRENO_DEVICE(device);
	struct adreno_dispatcher *dispatcher = &adreno_dev->dispatcher;

	queue_work(device->work_queue, &dispatcher->work);
}

/**
 * adreno_dispatcher_queue_context() - schedule a drawctxt in the dispatcher
 * device: pointer to the KGSL device
 * drawctxt: pointer to the drawctxt to schedule
 *
 * Put a draw context on the dispatcher pending queue and schedule the
 * dispatcher. This is used to reschedule changes that might have been blocked
 * for sync points or other concerns
 */
void adreno_dispatcher_queue_context(struct kgsl_device *device,
	struct adreno_context *drawctxt)
{
	struct adreno_device *adreno_dev = ADRENO_DEVICE(device);

	dispatcher_queue_context(adreno_dev, drawctxt);
	adreno_dispatcher_schedule(device);
}

/*
 * This is called on a regular basis while command batches are inflight.  Fault
 * detection registers are read and compared to the existing values - if they
 * changed then the GPU is still running.  If they are the same between
 * subsequent calls then the GPU may have faulted
 */

static void adreno_dispatcher_fault_timer(unsigned long data)
{
	struct adreno_device *adreno_dev = (struct adreno_device *) data;
	struct kgsl_device *device = &adreno_dev->dev;
	struct adreno_dispatcher *dispatcher = &adreno_dev->dispatcher;

	/* Leave if the user decided to turn off fast hang detection */
	if (adreno_dev->fast_hang_detect == 0)
		return;

	if (adreno_gpu_fault(adreno_dev)) {
		adreno_dispatcher_schedule(device);
		return;
	}

	/*
	 * Read the fault registers - if it returns 0 then they haven't changed
	 * so mark the dispatcher as faulted and schedule the work loop.
	 */

	if (!fault_detect_read_compare(device)) {
		adreno_set_gpu_fault(adreno_dev, ADRENO_SOFT_FAULT);
		adreno_dispatcher_schedule(device);
	} else {
		mod_timer(&dispatcher->fault_timer,
			jiffies + msecs_to_jiffies(_fault_timer_interval));
	}
}

/*
 * This is called when the timer expires - it either means the GPU is hung or
 * the IB is taking too long to execute
 */
static void adreno_dispatcher_timer(unsigned long data)
{
	struct adreno_device *adreno_dev = (struct adreno_device *) data;
	struct kgsl_device *device = &adreno_dev->dev;

	adreno_dispatcher_schedule(device);
}
/**
 * adreno_dispatcher_irq_fault() - Trigger a fault in the dispatcher
 * @device: Pointer to the KGSL device
 *
 * Called from an interrupt context this will trigger a fault in the
 * dispatcher for the oldest pending command batch
 */
void adreno_dispatcher_irq_fault(struct kgsl_device *device)
{
	adreno_set_gpu_fault(ADRENO_DEVICE(device), ADRENO_HARD_FAULT);
	adreno_dispatcher_schedule(device);
}

/**
 * adreno_dispatcher_start() - activate the dispatcher
 * @adreno_dev: pointer to the adreno device structure
 *
 */
void adreno_dispatcher_start(struct kgsl_device *device)
{
	complete_all(&device->cmdbatch_gate);

	/* Schedule the work loop to get things going */
	adreno_dispatcher_schedule(device);
}

/**
 * adreno_dispatcher_stop() - stop the dispatcher
 * @adreno_dev: pointer to the adreno device structure
 *
 * Stop the dispatcher and close all the timers
 */
void adreno_dispatcher_stop(struct adreno_device *adreno_dev)
{
	struct adreno_dispatcher *dispatcher = &adreno_dev->dispatcher;

	del_timer_sync(&dispatcher->timer);
	del_timer_sync(&dispatcher->fault_timer);
}

/**
 * adreno_dispatcher_close() - close the dispatcher
 * @adreno_dev: pointer to the adreno device structure
 *
 * Close the dispatcher and free all the oustanding commands and memory
 */
void adreno_dispatcher_close(struct adreno_device *adreno_dev)
{
	struct adreno_dispatcher *dispatcher = &adreno_dev->dispatcher;
	int i;
	struct adreno_ringbuffer *rb;

	mutex_lock(&dispatcher->mutex);
	del_timer_sync(&dispatcher->timer);
	del_timer_sync(&dispatcher->fault_timer);

	FOR_EACH_RINGBUFFER(adreno_dev, rb, i) {
		struct adreno_dispatcher_cmdqueue *dispatch_q =
			&(rb->dispatch_q);
		while (dispatch_q->head != dispatch_q->tail) {
			kgsl_cmdbatch_destroy(
				dispatch_q->cmd_q[dispatch_q->head]);
			dispatch_q->head = (dispatch_q->head + 1)
				% ADRENO_DISPATCH_CMDQUEUE_SIZE;
		}
	}

	mutex_unlock(&dispatcher->mutex);

	kobject_put(&dispatcher->kobj);
}

struct dispatcher_attribute {
	struct attribute attr;
	ssize_t (*show)(struct adreno_dispatcher *,
			struct dispatcher_attribute *, char *);
	ssize_t (*store)(struct adreno_dispatcher *,
			struct dispatcher_attribute *, const char *buf,
			size_t count);
	unsigned int max;
	unsigned int *value;
};

#define DISPATCHER_UINT_ATTR(_name, _mode, _max, _value) \
	struct dispatcher_attribute dispatcher_attr_##_name =  { \
		.attr = { .name = __stringify(_name), .mode = _mode }, \
		.show = _show_uint, \
		.store = _store_uint, \
		.max = _max, \
		.value = &(_value), \
	}

#define to_dispatcher_attr(_a) \
	container_of((_a), struct dispatcher_attribute, attr)
#define to_dispatcher(k) container_of(k, struct adreno_dispatcher, kobj)

static ssize_t _store_uint(struct adreno_dispatcher *dispatcher,
		struct dispatcher_attribute *attr,
		const char *buf, size_t size)
{
	unsigned int val = 0;
	int ret;

	ret = kgsl_sysfs_store(buf, &val);
	if (ret)
		return ret;

	if (!val || (attr->max && (val > attr->max)))
		return -EINVAL;

	*((unsigned int *) attr->value) = val;
	return size;
}

static ssize_t _show_uint(struct adreno_dispatcher *dispatcher,
		struct dispatcher_attribute *attr,
		char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%u\n",
		*((unsigned int *) attr->value));
}

static DISPATCHER_UINT_ATTR(inflight, 0644, ADRENO_DISPATCH_CMDQUEUE_SIZE,
	_dispatcher_q_inflight_hi);

static DISPATCHER_UINT_ATTR(inflight_low_latency, 0644,
	ADRENO_DISPATCH_CMDQUEUE_SIZE, _dispatcher_q_inflight_lo);
/*
 * Our code that "puts back" a command from the context is much cleaner
 * if we are sure that there will always be enough room in the
 * ringbuffer so restrict the maximum size of the context queue to
 * ADRENO_CONTEXT_CMDQUEUE_SIZE - 1
 */
static DISPATCHER_UINT_ATTR(context_cmdqueue_size, 0644,
	ADRENO_CONTEXT_CMDQUEUE_SIZE - 1, _context_cmdqueue_size);
static DISPATCHER_UINT_ATTR(context_burst_count, 0644, 0,
	_context_cmdbatch_burst);
static DISPATCHER_UINT_ATTR(cmdbatch_timeout, 0644, 0, _cmdbatch_timeout);
static DISPATCHER_UINT_ATTR(context_queue_wait, 0644, 0, _context_queue_wait);
static DISPATCHER_UINT_ATTR(fault_detect_interval, 0644, 0,
	_fault_timer_interval);
static DISPATCHER_UINT_ATTR(fault_throttle_time, 0644, 0,
	_fault_throttle_time);
static DISPATCHER_UINT_ATTR(fault_throttle_burst, 0644, 0,
	_fault_throttle_burst);

static struct attribute *dispatcher_attrs[] = {
	&dispatcher_attr_inflight.attr,
	&dispatcher_attr_inflight_low_latency.attr,
	&dispatcher_attr_context_cmdqueue_size.attr,
	&dispatcher_attr_context_burst_count.attr,
	&dispatcher_attr_cmdbatch_timeout.attr,
	&dispatcher_attr_context_queue_wait.attr,
	&dispatcher_attr_fault_detect_interval.attr,
	&dispatcher_attr_fault_throttle_time.attr,
	&dispatcher_attr_fault_throttle_burst.attr,
	NULL,
};

static ssize_t dispatcher_sysfs_show(struct kobject *kobj,
				   struct attribute *attr, char *buf)
{
	struct adreno_dispatcher *dispatcher = to_dispatcher(kobj);
	struct dispatcher_attribute *pattr = to_dispatcher_attr(attr);
	ssize_t ret = -EIO;

	if (pattr->show)
		ret = pattr->show(dispatcher, pattr, buf);

	return ret;
}

static ssize_t dispatcher_sysfs_store(struct kobject *kobj,
				    struct attribute *attr,
				    const char *buf, size_t count)
{
	struct adreno_dispatcher *dispatcher = to_dispatcher(kobj);
	struct dispatcher_attribute *pattr = to_dispatcher_attr(attr);
	ssize_t ret = -EIO;

	if (pattr->store)
		ret = pattr->store(dispatcher, pattr, buf, count);

	return ret;
}

static void dispatcher_sysfs_release(struct kobject *kobj)
{
}

static const struct sysfs_ops dispatcher_sysfs_ops = {
	.show = dispatcher_sysfs_show,
	.store = dispatcher_sysfs_store
};

static struct kobj_type ktype_dispatcher = {
	.sysfs_ops = &dispatcher_sysfs_ops,
	.default_attrs = dispatcher_attrs,
	.release = dispatcher_sysfs_release
};

/**
 * adreno_dispatcher_init() - Initialize the dispatcher
 * @adreno_dev: pointer to the adreno device structure
 *
 * Initialize the dispatcher
 */
int adreno_dispatcher_init(struct adreno_device *adreno_dev)
{
	struct kgsl_device *device = &adreno_dev->dev;
	struct adreno_dispatcher *dispatcher = &adreno_dev->dispatcher;
	int ret;

	memset(dispatcher, 0, sizeof(*dispatcher));

	mutex_init(&dispatcher->mutex);

	setup_timer(&dispatcher->timer, adreno_dispatcher_timer,
		(unsigned long) adreno_dev);

	setup_timer(&dispatcher->fault_timer, adreno_dispatcher_fault_timer,
		(unsigned long) adreno_dev);

	INIT_WORK(&dispatcher->work, adreno_dispatcher_work);

	init_completion(&dispatcher->idle_gate);
	complete_all(&dispatcher->idle_gate);

	plist_head_init(&dispatcher->pending);
	spin_lock_init(&dispatcher->plist_lock);

	ret = kobject_init_and_add(&dispatcher->kobj, &ktype_dispatcher,
		&device->dev->kobj, "dispatch");

	return ret;
}

/*
 * adreno_dispatcher_idle() - Wait for dispatcher to idle
 * @adreno_dev: Adreno device whose dispatcher needs to idle
 *
 * Signal dispatcher to stop sending more commands and complete
 * the commands that have already been submitted. This function
 * should not be called when dispatcher mutex is held.
 */
int adreno_dispatcher_idle(struct adreno_device *adreno_dev)
{
	struct kgsl_device *device = &adreno_dev->dev;
	struct adreno_dispatcher *dispatcher = &adreno_dev->dispatcher;
	int ret;

	BUG_ON(!mutex_is_locked(&device->mutex));
	if (!test_bit(ADRENO_DEVICE_STARTED, &adreno_dev->priv))
		return 0;

	/*
	 * Ensure that this function is not called when dispatcher
	 * mutex is held and device is started
	 */
	if (mutex_is_locked(&dispatcher->mutex) &&
		dispatcher->mutex.owner == current)
		BUG_ON(1);

	adreno_get_gpu_halt(adreno_dev);

	mutex_unlock(&device->mutex);

	ret = wait_for_completion_timeout(&dispatcher->idle_gate,
			msecs_to_jiffies(ADRENO_IDLE_TIMEOUT));
	if (ret == 0) {
		ret = -ETIMEDOUT;
		WARN(1, "Dispatcher halt timeout ");
	} else if (ret < 0) {
		KGSL_DRV_ERR(device, "Dispatcher halt failed %d\n", ret);
	} else {
		ret = 0;
	}

	mutex_lock(&device->mutex);
	adreno_put_gpu_halt(adreno_dev);
	/*
	 * requeue dispatcher work to resubmit pending commands
	 * that may have been blocked due to this idling request
	 */
	adreno_dispatcher_schedule(device);
	return ret;
}
