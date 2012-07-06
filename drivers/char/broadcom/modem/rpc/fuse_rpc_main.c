/****************************************************************************
*
*     Copyright (c) 2009 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license
*   agreement governing use of this software, this software is licensed to you
*   under the terms of the GNU General Public License version 2, available
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
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
#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif
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
#include "mqueue.h"

#include "ipcinterface.h"

#define SYSRPC_TRACE_TRACE_ON

#ifdef SYSRPC_TRACE_TRACE_ON
#define _DBG(a) a
#define SYSRPC_TRACE(fmt , args...) \
	BCMLOG_Printf(BCMLOG_RPC_KERNEL_BASIC, fmt, ##args)
#else
#define SYSRPC_TRACE(str) {}
#endif

MsgQueueHandle_t sysRpcMQhandle;

static int __init bcm_fuse_rpc_init_module(void);
static void __exit bcm_fuse_rpc_exit_module(void);


/***************************************************************************/
/**
 *  Called by Linux power management system when AP enters and exits sleep.
 *  Api will notify CP of current AP sleep state, and CP will suspend
 *  unnecessary CP->AP notifications to avoid waking AP.
 *
 *  @param  inSleep (in)   TRUE if AP entering deep sleep, FALSE if exiting
 */
//extern void IPC_ApSleepModeSet(IPC_Boolean inSleep);

void BcmRpc_SetApSleep(bool inSleep)
{	
	RPC_SetProperty(RPC_PROP_AP_IN_DEEPSLEEP, (inSleep ? 1 : 0));

	IPC_ApSleepModeSet((IPC_Boolean)inSleep);
}

void RPC_Assert(char *expr, char *file, int line, int value)
{
	printk(KERN_CRIT "RPC Assert !!! (%s) file=%s line=%d val=%d\n",
				expr, file, line, value);
}

/****************************************************************************
*
*  sysRpcKthreadFn(void);
*
*  kthread handler
*
***************************************************************************/
static int sysRpcKthreadFn(MsgQueueHandle_t *mHandle, void *data)
{
	_DBG(SYSRPC_TRACE(
		"RPC_BufferDelivery PROCESS sysrpc mHandle=%x event=%d\n",
		(int)mHandle, (int)data));

	RpcDbgUpdatePktState((int)data, PKT_STATE_SYSRPC_PROCESS);

	RPC_HandleEvent(data);
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
	int ret;
	Int32 isReservedPkt = RPC_PACKET_IsReservedPkt(eventHandle);
	if (isReservedPkt) {
		if (!MsgQueueIsEmpty(&sysRpcMQhandle)) {
			void *data;
			data = MsgQueueGet(&sysRpcMQhandle);
			while (data) {
				RPC_PACKET_FreeBufferEx(data, 0);
				data = MsgQueueGet(&sysRpcMQhandle);
			}
		}
		RPC_PACKET_FreeBufferEx(eventHandle, 0);
		return;
	}
	_DBG(SYSRPC_TRACE(
		"RPC_BufferDelivery POST sysrpc h=%d\n\n", (int)eventHandle));
	ret = MsgQueueAdd(&sysRpcMQhandle, eventHandle);
	if (ret != 0) {
		_DBG(SYSRPC_TRACE(
			"sysrpc: sysRpcHandlerCbk POST event FAIL=%x\n\n",
			(int)eventHandle));
		RPC_PACKET_FreeBufferEx(eventHandle, 0);
	} else
		RpcDbgUpdatePktState((int)eventHandle, PKT_STATE_SYSRPC_POST);

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
	int ret;

	ret = MsgQueueInit(&sysRpcMQhandle, sysRpcKthreadFn,
				"SysRpcKThread", 0, NULL, "sysrpc_wake_lock");

	if (ret != 0) {
		RPC_DEBUG(DBG_ERROR, "sysrpc: MsgQueueInit fail...!\n");
		printk(KERN_CRIT "MsgQueueInit fail\n");
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
	/* initialize the synchronous RPC interface*/
	if (RPC_SyncInitialize() != RESULT_OK) {
		RPC_DEBUG(DBG_ERROR, "RPC_SyncInitialize fail...!\n");
		return 1;
	}

	return 0;
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

    /** Init RPC Driver */
	ret = RPC_Init();

	if (ret) {
		ret = -1;
		RPC_DEBUG(DBG_ERROR, "RPC_Init fail...!\n");
	}

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
	MsgQueueDeInit(&sysRpcMQhandle);
	return;
}

late_initcall(bcm_fuse_rpc_init_module);
