/****************************************************************************
*
*     Copyright (c) 2009 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license
*   agreement governing use of this software, this software is licensed to you
*   under the terms of the GNU General Public License version 2, available
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
*   Notwithstanding the above, under no circumstances may you combine this
*software in any way with any other Broadcom software provided under a license
*   other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/

/*#define AUDIO_MQUEUE_RPC_WAKELOCK*/

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
#if defined(CONFIG_HAS_WAKELOCK) && defined(AUDIO_MQUEUE_RPC_WAKELOCK)
#include <linux/wakelock.h>
#endif

#include "mobcom_types.h"
#include "resultcode.h"
#include "taskmsgs.h"
#include "consts.h"
/*#include "rpc_debug.h"*/
#include "audio_mqueue.h"
#include "bcmlog.h"
#include "audio_trace.h"

#define INVALID_HANDLE(a) (!(a) || !(a->valid))
static int Audio_MQueueKthreadFn(void *param);

int Audio_MsgQueueInit(Audio_MsgQueueHandle_t *mHandle,
		Audio_MsgQueueThreadFn_t fn,
		 char *name, unsigned int optionType,
		 void *optionData, char* wk_name)
{
	int ret = 0;
	if (!mHandle) {
		aTrace(LOG_AUDIO_DRIVER,
			"mq: Audio_MsgQueueInit has Invalid handle\r\n");
		return -1;
	}

	memset(mHandle, 0, sizeof(Audio_MsgQueueHandle_t));

	INIT_LIST_HEAD(&(mHandle->mList));
	mHandle->mAvailData = 0;
	spin_lock_init(&mHandle->mLock);
	init_waitqueue_head(&mHandle->mWaitQ);

	/*Spawn a kthread if required */
	if (fn) {
		mHandle->valid = 1;
		mHandle->mFn = fn;
		mHandle->mThread =
		    kthread_run(Audio_MQueueKthreadFn, (void *)mHandle, name);
		aTrace(LOG_AUDIO_DRIVER,
				"mq: Audio_MsgQueueInit mthread=%x\r\n",
				(int)mHandle->mThread);
		ret = (mHandle->mThread) ? 0 : -1;
	} else {
		mHandle->mThread = NULL;
		mHandle->mFn = 0;
		mHandle->valid = 1;
		ret = 0;
	}

#if defined(CONFIG_HAS_WAKELOCK) && defined(AUDIO_MQUEUE_RPC_WAKELOCK)
	wake_lock_init(&(mHandle->mq_wake_lock), WAKE_LOCK_SUSPEND, wk_name);
#endif
	strncpy(mHandle->name, name, MAX_NM_LEN);
	aTrace(LOG_AUDIO_DRIVER,
			"mq: Audio_MsgQueueInit mHandle=%x fn=%x nm=%s ret=%d\r\n",
			(int)mHandle, (int)fn, (name) ? name : "", ret);
	return ret;
}

int Audio_MsgQueueAdd(Audio_MsgQueueHandle_t *mHandle, void *data)
{
	Audio_MsgQueueElement_t *elem;

	if (INVALID_HANDLE(mHandle)) {
		aTrace(LOG_AUDIO_DRIVER,
				"mq: Audio_MsgQueueAdd has Invalid mHandle %x valid %d\r\n",
				(int)mHandle, (mHandle) ? mHandle->valid : -1);
		return -1;
	}

	elem = kmalloc(sizeof(Audio_MsgQueueElement_t),
			in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
	if (!elem) {
		aTrace(LOG_AUDIO_DRIVER,
			"mq: Audio_MsgQueueAdd kmalloc failed interrupt_context=%d\r\n",
			(int)in_interrupt());
		return -1;
	}
	elem->data = data;

	/*add to queue */
	spin_lock_bh(&mHandle->mLock);
	list_add_tail(&elem->mList, &mHandle->mList);
	mHandle->mAvailData = 1;
	spin_unlock_bh(&mHandle->mLock);

	aTrace(LOG_AUDIO_DRIVER,
			"mq: Audio_MsgQueueAdd mHandle=%x, data=%d\r\n",
		      (int)mHandle, (int)data);

	wake_up_interruptible(&mHandle->mWaitQ);

	return 0;
}

int Audio_MsgQueueIsEmpty(Audio_MsgQueueHandle_t *mHandle)
{
	int isEmpty = 1;

	if (INVALID_HANDLE(mHandle)) {
		aTrace(LOG_AUDIO_DRIVER,
				"mq: Audio_MsgQueueIsEmpty has Invalid mHandle\r\n");
		return -1;
	}
	spin_lock_bh(&mHandle->mLock);
	isEmpty = (Boolean) list_empty(&mHandle->mList);
	spin_unlock_bh(&mHandle->mLock);

	aTrace(LOG_AUDIO_DRIVER,
			"mq: Audio_MsgQueueIsEmpty mHandle=%x, isEmpty=%d\r\n",
			(int)mHandle, (int)isEmpty);

	return isEmpty;
}

int Audio_MsgQueueRemove(Audio_MsgQueueHandle_t *mHandle, void **outData)
{
	struct list_head *entry;
	Audio_MsgQueueElement_t *Item = NULL;
	void *data = NULL;
	int isEmpty = 1;

	if (INVALID_HANDLE(mHandle)) {
		aTrace(LOG_AUDIO_DRIVER,
				"mq: Audio_MsgQueueRemove has Invalid mHandle\r\n");
		return -1;
	}

	if (!outData) {
		aTrace(LOG_AUDIO_DRIVER,
				"mq: Audio_MsgQueueRemove Invalid param mHandle=%x data=%x\r\n",
				(int)mHandle, (int)outData);
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
			Item = list_entry(entry,
					Audio_MsgQueueElement_t,
					mList);
			data = Item->data;

			list_del(entry);
			spin_unlock_bh(&mHandle->mLock);

			kfree(Item);

			*outData = data;
			break;
		}
	}

	aTrace(LOG_AUDIO_DRIVER,
			"mq: Audio_MsgQueueRemove mHandle=%x, data=%d\r\n",
			(int)mHandle, (int)data);

	return 0;
}
#if 0
int Audio_MsgQueueDebugList(Audio_MsgQueueHandle_t *mHandle,
		RpcOutputContext_t *c)
{
	Audio_MsgQueueElement_t *Item = NULL;
	struct list_head *listptr, *pos;

	if (INVALID_HANDLE(mHandle)) {
		aTrace(LOG_AUDIO_DRIVER,
				"mq: Audio_MsgQueueDebugList has Invalid mHandle\r\n");
		return 0;
	}

	spin_lock_bh(&mHandle->mLock);

	RpcDbgDumpStr(c,  "\tkThread: %s tid:%d Rx:%d\r\n",
					mHandle->name,
					mHandle->mThread->pid,
					mHandle->mAvailData);

	RpcDumpTaskCallStack(c, mHandle->mThread);

	list_for_each_safe(listptr, pos, &mHandle->mList)
	{
		Item = list_entry(listptr, Audio_MsgQueueElement_t, mList);
		RpcDbgDumpStr(c,  "\tQUEUED pkt:%d\r\n",
					(int)Item->data);
	}

	spin_unlock_bh(&mHandle->mLock);

	return 0;
}
int Audio_MsgQueueCount(Audio_MsgQueueHandle_t *mHandle)
{
	int count = 0;
	struct list_head *pos;
	if (INVALID_HANDLE(mHandle)) {
		aTrace(LOG_AUDIO_DRIVER,
				"mq: Audio_MsgQueueDebugList has Invalid mHandle\r\n");
		return -1;
	}
	spin_lock_bh(&mHandle->mLock);
	list_for_each(pos, &mHandle->mList) count++;
	spin_unlock_bh(&mHandle->mLock);
	return count;
}

#endif
void *Audio_MsgQueueGet(Audio_MsgQueueHandle_t *mHandle)
{
	struct list_head *entry;
	Audio_MsgQueueElement_t *Item = NULL;
	void *data = NULL;
	int isEmpty = 1;
	if (INVALID_HANDLE(mHandle)) {
		aTrace(LOG_AUDIO_DRIVER,
				"mq: Audio_MsgQueueGet has Invalid mHandle\r\n");
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
	Item = list_entry(entry, Audio_MsgQueueElement_t, mList);
	data = Item->data;
	list_del(entry);
	spin_unlock_bh(&mHandle->mLock);
	kfree(Item);
	aTrace(LOG_AUDIO_DRIVER,
			"mq: Audio_MsgQueueGet mHandle=%x, data=%d\r\n",
			(int)mHandle, (int)data);
	return data;
}
static int Audio_MQueueKthreadFn(void *param)
{
	void *data;
	int ret = 0;
	Audio_MsgQueueHandle_t *mHandle = (Audio_MsgQueueHandle_t *) param;

	aTrace(LOG_AUDIO_DRIVER,
			"mq: Audio_MQueueKthreadFn mHandle=%x\r\n",
			(int)mHandle);

	set_user_nice(current, -16);

	while (ret == 0) {
		data = NULL;
		ret = Audio_MsgQueueRemove(mHandle, &data);

		aTrace(LOG_AUDIO_DRIVER,
				"mq: Audio_MQueueKthreadFn Dispatch mHandle=%x data=%d ret=%d\r\n",
				(int)mHandle, (int)data, ret);
		if (ret == 0 && data) {
#if defined(CONFIG_HAS_WAKELOCK) && defined(AUDIO_MQUEUE_RPC_WAKELOCK)
			wake_lock(&(mHandle->mq_wake_lock));
#endif
			/* Call actual handler */
			mHandle->mFn(mHandle, data);
#if defined(CONFIG_HAS_WAKELOCK) && defined(AUDIO_MQUEUE_RPC_WAKELOCK)
			wake_unlock(&(mHandle->mq_wake_lock));
#endif

		} else
			break;
	}
	aTrace(LOG_AUDIO_DRIVER,
			"mq: Audio_MQueueKthreadFn QUIT mHandle=%x\r\n",
			(int)mHandle);
	return 0;
}

int Audio_MsgQueueDeInit(Audio_MsgQueueHandle_t *mHandle)
{
	if (INVALID_HANDLE(mHandle)) {
		aTrace(LOG_AUDIO_DRIVER,
				"mq: Audio_MsgQueueDeInit has Invalid handle\r\n");
		return -1;
	}

	aTrace(LOG_AUDIO_DRIVER,
			"mq: Audio_MsgQueueDeInit mHandle=%x thread=%x\r\n",
			(int)mHandle, (int)mHandle->mThread);

	/*Send NULL data to exit kthread */
	if (mHandle->mThread)
		Audio_MsgQueueAdd(mHandle, NULL);

#if defined(CONFIG_HAS_WAKELOCK) && defined(AUDIO_MQUEUE_RPC_WAKELOCK)
	wake_lock_destroy(&(mHandle->mq_wake_lock));
#endif

	mHandle->valid = 0;
	return 0;
}
