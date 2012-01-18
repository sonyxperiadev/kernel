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
*   software in any way with any other Broadcom software provided under a license 
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
#include <linux/broadcom/bcm_major.h>
#include <linux/broadcom/bcm_rpc.h>
#include <linux/broadcom/ipcinterface.h>
#include <linux/broadcom/ipcproperties.h>
#include <linux/kthread.h>

#include "mobcom_types.h"
#include "resultcode.h"
#include "taskmsgs.h"
#include "consts.h"
#include "xdr_porting_layer.h"
#include "xdr.h"
#include "rpc_ipc.h"
#include "rpc_api.h"
#include "rpc_internal_api.h"
#include "rpc_debug.h"
#include "rpc_sync_api.h"
#include "bcmlog.h"

#define SYSRPC_TRACE_TRACE_ON

#ifdef SYSRPC_TRACE_TRACE_ON
#define _DBG(a) a
#define SYSRPC_TRACE(fmt,args...) BCMLOG_Printf( BCMLOG_RPC_KERNEL_BASIC, fmt, ##args )
#else
#define SYSRPC_TRACE(str) {}
#endif

static int __init bcm_fuse_rpc_init_module(void);
static void __exit bcm_fuse_rpc_exit_module(void);

static struct class *rpc_class;
static struct task_struct *sysRpcThread = NULL;
static int gAvailData = 0;

typedef struct {
	struct list_head mList;
	void *data;
} SysRpcMsgInfo_t;

static spinlock_t mLock;
static wait_queue_head_t mWaitQ;

SysRpcMsgInfo_t gSysRpcMsgInfo = { LIST_HEAD_INIT(gSysRpcMsgInfo.mList), NULL };

/***************************************************************************/
/**
 *  Called by Linux power management system when AP enters and exits sleep.
 *  Api will notify CP of current AP sleep state, and CP will suspend
 *  unnecessary CP->AP notifications to avoid waking AP.
 *
 *  @param  inSleep (in)   TRUE if AP entering deep sleep, FALSE if exiting
 */
void BcmRpc_SetApSleep(bool inSleep)
{
	RPC_SetProperty(RPC_PROP_AP_IN_DEEPSLEEP, (inSleep ? 1 : 0));
}

void RPC_Assert(char *expr, char *file, int line, int value)
{
	printk("RPC Assert !!! (%s) file=%s line=%d val=%d \n", expr, file,
	       line, value);
}

static int rpc_open(struct inode *inode, struct file *filp)
{

	return 0;
}

static int rpc_release(struct inode *inode, struct file *filp)
{

	return 0;
}

int rpc_read(struct file *filp, char __user * buf, size_t size, loff_t * offset)
{
	int rc = 0;

	return rc;
}

int rpc_write(struct file *filp, const char __user * buf, size_t size,
	      loff_t * offset)
{
	int rc = 0;

	return rc;
}

static long rpc_ioctl(struct file *filp, unsigned int cmd, UInt32 arg)
{
	int rc = 0;
	return (rc);
}

static struct file_operations rpc_ops = {
	.owner = THIS_MODULE,
	.open = rpc_open,
	.read = rpc_read,
	.write = rpc_write,
	.unlocked_ioctl = rpc_ioctl,
	.mmap = NULL,
	.release = rpc_release,
};

/****************************************************************************
*
*  sysRpcKthreadFn(void);
*
*  kthread handler
*
***************************************************************************/
static int sysRpcKthreadFn(void *data)
{
	Boolean isEmpty;
	while (1) {
		spin_lock(&mLock);
		isEmpty = (Boolean) list_empty(&gSysRpcMsgInfo.mList);
		spin_unlock(&mLock);

		if (isEmpty) {
			gAvailData = 0;
			_DBG(SYSRPC_TRACE
			     ("sysrpc:before wait %x\n", (int)jiffies));
			wait_event_interruptible(mWaitQ, gAvailData);
			_DBG(SYSRPC_TRACE
			     ("sysrpc:after wait %x\n", (int)jiffies));
		} else {
			struct list_head *entry;
			SysRpcMsgInfo_t *Item = NULL;
			void *data;

			spin_lock(&mLock);
			entry = gSysRpcMsgInfo.mList.next;
			Item = list_entry(entry, SysRpcMsgInfo_t, mList);
			data = Item->data;

			list_del(entry);
			kfree(entry);
			spin_unlock(&mLock);

			_DBG(SYSRPC_TRACE
			     ("sysrpc: Handle event=%x\n", (int)data));

			RPC_HandleEvent(data);
		}
	}
	return 0;
}

/****************************************************************************
*
*  sysRpcHandlerCbk(void);
*
*  Sys RPC Handler Cbk
*
***************************************************************************/
static void sysRpcHandlerCbk(void *eventHandle)
{
	SysRpcMsgInfo_t *elem;

	elem = kmalloc(sizeof(SysRpcMsgInfo_t), GFP_KERNEL);
	if (!elem) {
		_DBG(SYSRPC_TRACE
		     ("sysrpc: sysRpcHandlerCbk Allocation error\n"));
		return;
	}
	elem->data = eventHandle;

	//add to queue
	spin_lock(&mLock);
	list_add_tail(&elem->mList, &gSysRpcMsgInfo.mList);
	spin_unlock(&mLock);

	_DBG(SYSRPC_TRACE("sysrpc: Post event=%x\n", (int)eventHandle));

	gAvailData = 1;
	wake_up_interruptible(&mWaitQ);
}

/****************************************************************************
*
*  RPC_Init(void);
*
*  Register RPC module.
*
***************************************************************************/
UInt32 RPC_Init(void)
{
	INIT_LIST_HEAD(&gSysRpcMsgInfo.mList);
	spin_lock_init(&mLock);
	init_waitqueue_head(&mWaitQ);

	sysRpcThread = kthread_run(sysRpcKthreadFn, NULL, "sysrpc_kthread");

	if (!sysRpcThread) {
		RPC_DEBUG(DBG_ERROR, "sysrpc: kthread_run fail...!\n");
		return 1;
	}

	if (RPC_SYS_Init(sysRpcHandlerCbk) == RESULT_ERROR) {
		RPC_DEBUG(DBG_ERROR, "RPC device init fail...!\n");
		return 1;
	}
	if (RPC_SYS_EndPointRegister(RPC_APPS) == RPC_RESULT_ERROR) {
		RPC_DEBUG(DBG_ERROR, "registering the RPC device fail...!\n");
		return 1;
	}
	// initialize the synchronous RPC interface
	if (RPC_SyncInitialize() != RESULT_OK) {
		RPC_DEBUG(DBG_ERROR, "RPC_SyncInitialize fail...!\n");
		return 1;
	}

	return (0);
}

/****************************************************************************
*
*  bcm_fuse_rpc_init_module(void);
*
*  Init module.
*
***************************************************************************/
static int __init bcm_fuse_rpc_init_module(void)
{
	int ret = 0;

	pr_info("RPC Support 1.00 (BUILD TIME " __DATE__ " " __TIME__ ")\n");

	if ((ret = register_chrdev(BCM_RPC_MAJOR, "bcm_rpc", &rpc_ops)) < 0) {
		RPC_DEBUG(DBG_ERROR,
			  "rpc: register_chrdev failed for major %d\n",
			  BCM_RPC_MAJOR);
		goto out;
	}

	rpc_class = class_create(THIS_MODULE, "bcm_rpc");
	if (IS_ERR(rpc_class)) {
		return PTR_ERR(rpc_class);
	}

	device_create(rpc_class, NULL, MKDEV(BCM_RPC_MAJOR, 0), NULL,
		      "bcm_rpc");

	RPC_DEBUG(DBG_INFO, "%s driver(major %d) installed.\n", "bcm_rpc",
		  BCM_RPC_MAJOR);

    /** Init RPC Driver */
	ret = RPC_Init();

	if (ret) {
		ret = -1;
		RPC_DEBUG(DBG_ERROR, "RPC_Init fail...!\n");
	}

      out:
	return ret;
}

/****************************************************************************
*
*  bcm_fuse_rpc_exit_module(void);
*
*  Exit module.
*
***************************************************************************/
static void __exit bcm_fuse_rpc_exit_module(void)
{

	return;
}

late_initcall(bcm_fuse_rpc_init_module);
