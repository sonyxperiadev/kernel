/****************************************************************************
*
*   Copyright (c) 2009 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license
*   agreement governing use of this software, this software is licensed to you
*   under the terms of the GNU General Public License version 2, available
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
*   Notwithstanding the above, under no circumstances may you combine this
*   software in any way with any other Broadcom software provided under a license
*   other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/

/*#define MQUEUE_RPC_WAKELOCK*/

#include <linux/sched.h>
#include <linux/kernel.h>	/* printk() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/delay.h>	/* udelay */
#include <linux/slab.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/timer.h>
#include <linux/poll.h>

#include <linux/unistd.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <asm/system.h>
#include <linux/kthread.h>
#if defined(CONFIG_HAS_WAKELOCK) && defined(MQUEUE_RPC_WAKELOCK)
#include <linux/wakelock.h>
#endif

#include "mobcom_types.h"
#include "resultcode.h"
#include "taskmsgs.h"
#include "consts.h"
#include "rpc_debug.h"
#include "mqueue.h"
#include "bcmlog.h"

/*#define MQ_TRACE_TRACE_ON*/

#ifdef MQ_TRACE_TRACE_ON
#define _DBG(a) a
#define MQ_TRACE(fmt, args...) \
		BCMLOG_Printf(BCMLOG_RPC_KERNEL_BASIC, fmt, ##args)
#else
#define _DBG(a)
#define MQ_TRACE(str) {}
#endif

#define INVALID_HANDLE(a) (!(a) || !(a->valid))
static int MQueueKthreadFn(void *param);

int MsgQueueInit(MsgQueueHandle_t *mHandle, MsgQueueThreadFn_t fn,
		 char *name, unsigned int optionType, void *optionData, char* wk_name)
{
	int ret = 0;
	if (!mHandle) {
		_DBG(MQ_TRACE("mq: MsgQueueInit has Invalid handle\n"));
		return -1;
	}

	memset(mHandle, 0, sizeof(MsgQueueHandle_t));

	INIT_LIST_HEAD(&(mHandle->mList));
	mHandle->mAvailData = 0;
	spin_lock_init(&mHandle->mLock);
	init_waitqueue_head(&mHandle->mWaitQ);

	/*Spawn a kthread if required */
	if (fn) {
		mHandle->valid = 1;
		mHandle->mFn = fn;
		mHandle->mThread =
		    kthread_run(MQueueKthreadFn, (void *)mHandle, name);
		_DBG(MQ_TRACE
		     ("mq: MsgQueueInit mthread=%x\n", (int)mHandle->mThread));
		ret = (mHandle->mThread) ? 0 : -1;
	} else {
		mHandle->mThread = NULL;
		mHandle->mFn = 0;
		mHandle->valid = 1;
		ret = 0;
	}

#if defined(CONFIG_HAS_WAKELOCK) && defined(MQUEUE_RPC_WAKELOCK)
	wake_lock_init(&(mHandle->mq_wake_lock), WAKE_LOCK_SUSPEND, wk_name);
#endif
	strncpy(mHandle->name, name, MAX_NM_LEN);
	_DBG(MQ_TRACE("mq: MsgQueueInit mHandle=%x fn=%x nm=%s ret=%d\n",
		      (int)mHandle, (int)fn, (name) ? name : "", ret));
	return ret;
}

int MsgQueueAdd(MsgQueueHandle_t *mHandle, void *data)
{
	MsgQueueElement_t *elem;

	if (INVALID_HANDLE(mHandle)) {
		_DBG(MQ_TRACE
		     ("mq: MsgQueueAdd has Invalid mHandle %x valid %d\n",
		      (int)mHandle, (mHandle) ? mHandle->valid : -1));
		return -1;
	}

	elem = kmalloc(sizeof(MsgQueueElement_t),
			in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
	if (!elem) {
		_DBG(MQ_TRACE
		     ("mq: MsgQueueAdd kmalloc failed interrupt_context=%d\n",
		      (int)in_interrupt()));
		return -1;
	}
	elem->data = data;

	/*add to queue */
	spin_lock_bh(&mHandle->mLock);
	list_add_tail(&elem->mList, &mHandle->mList);
	mHandle->mAvailData = 1;
	spin_unlock_bh(&mHandle->mLock);

	_DBG(MQ_TRACE("mq: MsgQueueAdd mHandle=%x, data=%d\n",
		      (int)mHandle, (int)data));

	wake_up_interruptible(&mHandle->mWaitQ);

	return 0;
}

int MsgQueueIsEmpty(MsgQueueHandle_t *mHandle)
{
	int isEmpty = 1;

	if (INVALID_HANDLE(mHandle)) {
		_DBG(MQ_TRACE("mq: MsgQueueIsEmpty has Invalid mHandle\n"));
		return -1;
	}
	spin_lock_bh(&mHandle->mLock);
	isEmpty = (Boolean) list_empty(&mHandle->mList);
	spin_unlock_bh(&mHandle->mLock);

	_DBG(MQ_TRACE("mq: MsgQueueIsEmpty mHandle=%x, isEmpty=%d\n",
		      (int)mHandle, (int)isEmpty));

	return isEmpty;
}

int MsgQueueRemove(MsgQueueHandle_t *mHandle, void **outData)
{
	struct list_head *entry;
	MsgQueueElement_t *Item = NULL;
	void *data = NULL;
	int isEmpty = 1;

	if (INVALID_HANDLE(mHandle)) {
		_DBG(MQ_TRACE("mq: MsgQueueRemove has Invalid mHandle\n"));
		return -1;
	}

	if (!outData) {
		_DBG(MQ_TRACE
		("mq: MsgQueueRemove Invalid param mHandle=%x data=%x\n",
		(int)mHandle, (int)outData));
		return -1;
	}

	while (1) {
		spin_lock_bh(&mHandle->mLock);
		isEmpty = (Boolean)list_empty(&mHandle->mList);

		if (isEmpty) {
			mHandle->mAvailData = 0;
			spin_unlock_bh(&mHandle->mLock);
			wait_event_interruptible(mHandle->mWaitQ,
						 mHandle->mAvailData);
		} else {
			entry = mHandle->mList.next;
			Item = list_entry(entry, MsgQueueElement_t, mList);
			data = Item->data;

			list_del(entry);
			spin_unlock_bh(&mHandle->mLock);

			kfree(Item);

			*outData = data;
			break;
		}
	}

	_DBG(MQ_TRACE("mq: MsgQueueRemove mHandle=%x, data=%d\n",
		      (int)mHandle, (int)data));

	return 0;
}

int MsgQueueDebugList(MsgQueueHandle_t *mHandle, RpcOutputContext_t *c)
{
	MsgQueueElement_t *Item = NULL;
	struct list_head *listptr, *pos;

	if (INVALID_HANDLE(mHandle)) {
		_DBG(MQ_TRACE("mq: MsgQueueDebugList has Invalid mHandle\n"));
		return 0;
	}

	spin_lock_bh(&mHandle->mLock);

	RpcDbgDumpStr(c,  "\tkThread: %s tid:%d Rx:%d\n",
					mHandle->name, mHandle->mThread->pid, mHandle->mAvailData);

	RpcDumpTaskCallStack(c, mHandle->mThread);

	list_for_each_safe(listptr, pos, &mHandle->mList)
	{
		Item = list_entry(listptr, MsgQueueElement_t, mList);
		RpcDbgDumpStr(c,  "\tQUEUED pkt:%d\n",
					(int)Item->data);
	}

	spin_unlock_bh(&mHandle->mLock);

	return 0;
}

int MsgQueueCount(MsgQueueHandle_t *mHandle)
{
	int count = 0;
	struct list_head *pos;
	if (INVALID_HANDLE(mHandle)) {
		_DBG(MQ_TRACE("mq: MsgQueueDebugList has Invalid mHandle\n"));
		return -1;
	}

	spin_lock_bh(&mHandle->mLock);
	list_for_each(pos, &mHandle->mList) count++;
	spin_unlock_bh(&mHandle->mLock);
	return count;
}

void *MsgQueueGet(MsgQueueHandle_t *mHandle)
{
	struct list_head *entry;
	MsgQueueElement_t *Item = NULL;
	void *data = NULL;
	int isEmpty = 1;
	if (INVALID_HANDLE(mHandle)) {
		_DBG(MQ_TRACE("mq: MsgQueueGet has Invalid mHandle\n"));
		return NULL;
	}

	spin_lock_bh(&mHandle->mLock);
	isEmpty = (Boolean)list_empty(&mHandle->mList);
	if (isEmpty) {
		mHandle->mAvailData = 0;
		spin_unlock_bh(&mHandle->mLock);
		return NULL;
	}

	entry = mHandle->mList.next;
	Item = list_entry(entry, MsgQueueElement_t, mList);
	data = Item->data;
	list_del(entry);
	spin_unlock_bh(&mHandle->mLock);
	kfree(Item);
	_DBG(MQ_TRACE("mq: MsgQueueGet mHandle=%x, data=%d\n",
		      (int)mHandle, (int)data));
	return data;
}
static int MQueueKthreadFn(void *param)
{
	void *data;
	int ret = 0;
	MsgQueueHandle_t *mHandle = (MsgQueueHandle_t *) param;

	_DBG(MQ_TRACE("mq: MQueueKthreadFn mHandle=%x\n", (int)mHandle));

	set_user_nice(current, -16);

	while (ret == 0) {
		data = NULL;
		ret = MsgQueueRemove(mHandle, &data);

		_DBG(MQ_TRACE("mq: MQueueKthreadFn Dispatch \
				mHandle=%x data=%d ret=%d\n",
				(int)mHandle, (int)data, ret));
		if (ret == 0 && data) {
#if defined(CONFIG_HAS_WAKELOCK) && defined(MQUEUE_RPC_WAKELOCK)
			wake_lock(&(mHandle->mq_wake_lock));
#endif
			/* Call actual handler */
			mHandle->mFn(mHandle, data);
#if defined(CONFIG_HAS_WAKELOCK) && defined(MQUEUE_RPC_WAKELOCK)
			wake_unlock(&(mHandle->mq_wake_lock));
#endif

		} else
			break;
	}
	_DBG(MQ_TRACE("mq: MQueueKthreadFn QUIT mHandle=%x\n", (int)mHandle));
	return 0;
}

int MsgQueueDeInit(MsgQueueHandle_t *mHandle)
{
	if (INVALID_HANDLE(mHandle)) {
		_DBG(MQ_TRACE("mq: MsgQueueDeInit has Invalid handle\n"));
		return -1;
	}

	_DBG(MQ_TRACE("mq: MsgQueueDeInit mHandle=%x thread=%x\n",
		      (int)mHandle, (int)mHandle->mThread));

	/*Send NULL data to exit kthread */
	if (mHandle->mThread)
		MsgQueueAdd(mHandle, NULL);

#if defined(CONFIG_HAS_WAKELOCK) && defined(MQUEUE_RPC_WAKELOCK)
	wake_lock_destroy(&(mHandle->mq_wake_lock));
#endif

	mHandle->valid = 0;
	return 0;
}
