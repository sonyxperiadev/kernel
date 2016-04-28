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
#include <linux/timer.h>

#include "mobcom_types.h"
#include "resultcode.h"
#include "taskmsgs.h"
#include "consts.h"

#include "bcmlog.h"

#include <linux/wakelock.h>

#include "rpc_wakelock.h"
#include "xdr_porting_layer.h"
#include "xdr.h"
#include "ipcproperties.h"
#include "rpc_global.h"
#include "rpc_ipc.h"
#include "rpc_ipc_config.h"
#include "rpc_debug.h"

#if defined(CONFIG_HAS_WAKELOCK) && defined(ENABLE_RPC_WAKELOCK)

#define WK_TRACE_TRACE_ON

#ifdef WK_TRACE_TRACE_ON
#define _DBG(a) a
#define WK_TRACE(fmt, args...) \
		BCMLOG_Printf(BCMLOG_RPC_KERNEL_BASIC, fmt, ##args)
#else
#define _DBG(a)
#define WK_TRACE(str) {}
#endif

#define MAX_WAKE_TIMEOUT  (5*1000)

typedef struct {
	struct list_head mList;
	UInt32 elem;
} RpcPktElement_t;

static RpcPktElement_t pktHeadQ;
static spinlock_t mLock;
static struct wake_lock wklock;
static int isWakeLockEnabled;
struct timer_list ftimer;
static int refCount;

typedef struct {
	UInt32 wk_lock;
	UInt32 wk_rm;
	UInt32 wk_add;
	UInt32 wk_unlock;
	UInt32 wk_reset;
	UInt32 ts_lock;
	UInt32 ts_unlock;
	UInt32 ts_rm;
	UInt32 ts_add;
	UInt32 ts_reset;
	UInt32 ts_avg_unlock;
	UInt32 ts_max_unlock;
	UInt32 ts_min_unlock;
} RpcWakeLockStats_t;

static RpcWakeLockStats_t rpcLockStats;

static void rpc_timer_function(unsigned long data);

void rpc_wake_lock_init(void)
{
	INIT_LIST_HEAD(&pktHeadQ.mList);
	spin_lock_init(&mLock);
	wake_lock_init(&wklock, WAKE_LOCK_SUSPEND, "RpcWakeLock");

	init_timer(&ftimer);
	ftimer.function = rpc_timer_function;
	ftimer.data = 0;

	refCount = 0;
	isWakeLockEnabled = 0;
	memset(&rpcLockStats, 0, sizeof(rpcLockStats));
}

void rpc_wake_lock_add(UInt32 elem)
{
	int firstElem, ret = 0;
	RpcPktElement_t *pktElem;

	pktElem = kmalloc(sizeof(RpcPktElement_t), GFP_ATOMIC);
	if (!pktElem) {
		_DBG(WK_TRACE
		     ("w:rpc_wake_lock Allocation error elem=%d\n", elem));
		return;
	}

	spin_lock_bh(&mLock);
	firstElem = (UInt8)list_empty(&pktHeadQ.mList);

	pktElem->elem = elem;
	list_add_tail(&pktElem->mList, &pktHeadQ.mList);
	refCount++;
	rpcLockStats.wk_add++;
	rpcLockStats.ts_add = jiffies_to_msecs(jiffies);

	if (firstElem) {
		/*Acquire wake lock*/
		if (isWakeLockEnabled == 0) {
			wake_lock(&wklock);
			HISTORY_RPC_LOG("Wklock", 0, (int)elem, 0, refCount);

			isWakeLockEnabled = 1;
			rpcLockStats.wk_lock++;
			rpcLockStats.ts_lock = jiffies_to_msecs(jiffies);
			/*/Start timer*/
			ret =
			    mod_timer(&ftimer,
				      (jiffies +
				       msecs_to_jiffies(MAX_WAKE_TIMEOUT)));
			_DBG(WK_TRACE
			     ("w:rpc_wake_lock LOCK elem=%d ret=%d\n", elem,
			      ret));
		}

	} else
		_DBG(WK_TRACE
		     ("w:rpc_wake_lock ADD elem=%d refCount=%d\n", elem,
		      refCount));

	spin_unlock_bh(&mLock);
}

void rpc_wake_lock_remove(UInt32 elem)
{
	int isEmpty, found = 0, ret = 0;
	RpcPktElement_t *Item = NULL;
	struct list_head *listptr, *pos;

	spin_lock_bh(&mLock);
	isEmpty = (UInt8)list_empty(&pktHeadQ.mList);

	if (isEmpty) {
		spin_unlock_bh(&mLock);
		_DBG(WK_TRACE
		     ("w:rpc_wake_lock_remove List Empty elem=%d\n", elem));
		return;
	}

	list_for_each_safe(listptr, pos, &pktHeadQ.mList) {
		Item = list_entry(listptr, RpcPktElement_t, mList);
		if (Item->elem == elem) {
			list_del(listptr);
			_DBG(WK_TRACE("w:rpc_wake_lock FREE elem=%d\n", elem));
			kfree(Item);
			found = 1;
			refCount--;
			rpcLockStats.wk_rm++;
			rpcLockStats.ts_rm = jiffies_to_msecs(jiffies);
			break;
		}
	}

	if (!found) {
		spin_unlock_bh(&mLock);
		_DBG(WK_TRACE
		     ("w:rpc_wake_lock_remove Not Found elem=%d\n", elem));
		return;
	}

	isEmpty = (UInt8)list_empty(&pktHeadQ.mList);

	if (isEmpty) {
		spin_unlock_bh(&mLock);
		ret = del_timer_sync(&ftimer);
		spin_lock_bh(&mLock);
		_DBG(WK_TRACE
		     ("w:rpc_wake_lock UNLOCK elem=%d ret=%d\n", elem, isEmpty,
		      ret));

		if (isWakeLockEnabled == 1) {
			unsigned long t1;

			wake_unlock(&wklock);
			HISTORY_RPC_LOG("WkUnlock", 0, (int)elem, 0, refCount);
			isWakeLockEnabled = 0;
			refCount = 0;
			/* Stats */
			rpcLockStats.wk_unlock++;
			rpcLockStats.ts_unlock = jiffies_to_msecs(jiffies);
			t1 = rpcLockStats.ts_unlock - rpcLockStats.ts_lock;
			if (t1 > rpcLockStats.ts_max_unlock)
				rpcLockStats.ts_max_unlock = t1;
			if (t1 < rpcLockStats.ts_min_unlock
			    || rpcLockStats.ts_min_unlock == 0)
				rpcLockStats.ts_min_unlock = t1;
			rpcLockStats.ts_avg_unlock += t1;
		}
	}

	spin_unlock_bh(&mLock);

	return;
}

void rpc_wake_lock_reset(void)
{
	int isEmpty;
	RpcPktElement_t *Item = NULL;
	struct list_head *listptr, *pos;

	spin_lock_bh(&mLock);
	isEmpty = (UInt8)list_empty(&pktHeadQ.mList);

	_DBG(WK_TRACE
	     ("w:rpc_wake_lock RESET refCount=%d empty=%d\n", refCount,
	      isEmpty));

	if (isEmpty) {
		spin_unlock_bh(&mLock);
		return;
	}

	rpcLockStats.wk_reset++;
	rpcLockStats.ts_reset = jiffies_to_msecs(jiffies);

	list_for_each_safe(listptr, pos, &pktHeadQ.mList) {
		Item = list_entry(listptr, RpcPktElement_t, mList);
		list_del(listptr);
		_DBG(WK_TRACE("w:rpc_wake_lock RESET elem=%d\n", Item->elem));
		kfree(Item);
	}

	if (isWakeLockEnabled == 1) {
		wake_unlock(&wklock);
		isWakeLockEnabled = 0;
		rpcLockStats.wk_unlock++;
		rpcLockStats.ts_unlock = jiffies_to_msecs(jiffies);
		_DBG(WK_TRACE("w:rpc_wake_lock RESET UNLOCK\n"));
	}

	refCount = 0;

	spin_unlock_bh(&mLock);
}

void rpc_timer_function(unsigned long data)
{
	rpc_wake_lock_reset();
}

void RpcDbgDumpWakeLockStats(RpcOutputContext_t *c)
{
	int av;

	RpcDbgDumpStr(c, "===== RPC Wake Lock dump Start =====");
	RpcDbgDumpStr(c, "\nHDR: Lock=%d Unlock=%d State=%d ref=%d",
		      (int)rpcLockStats.wk_lock,
		      (int)rpcLockStats.wk_unlock, isWakeLockEnabled, refCount);

	RpcDbgDumpStr(c, "\nCOUNT: add=%d rm=%d reset=%d",
		      (int)rpcLockStats.wk_lock,
		      (int)rpcLockStats.wk_unlock, (int)rpcLockStats.wk_reset);

	RpcDbgDumpStr(c, "\nTS: add=%u lock=%u rm=%u unlock=%u reset=%u",
		      (int)rpcLockStats.ts_add,
		      (int)rpcLockStats.ts_lock,
		      (int)rpcLockStats.ts_rm,
		      (int)rpcLockStats.ts_unlock, (int)rpcLockStats.ts_reset);

	if (rpcLockStats.ts_unlock)
		av = rpcLockStats.ts_avg_unlock / rpcLockStats.ts_unlock;
	else
		av = 0;
	RpcDbgDumpStr(c, "\nTS STATS: min=%u max=%u avg=%u",
		      (int)rpcLockStats.ts_min_unlock,
		      (int)rpcLockStats.ts_max_unlock, av);

	RpcDbgDumpStr(c, "\n===== RPC Wake Lock dump End =====");
	return;
}

#endif
