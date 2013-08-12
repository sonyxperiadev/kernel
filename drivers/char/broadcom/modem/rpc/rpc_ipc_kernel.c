/*******************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

/****************************************************************************/
/**
*
* @file   rpc_ipc_kernel.c
*
* @brief This driver is as Braodcom Rpc server in which user space clients
*        can send RPC messages from/to Application processor to Modem processor.
*
*
****************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>

#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/list.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/unistd.h>
#include <linux/errno.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <asm/pgtable.h>
#include <linux/io.h>
#include <linux/proc_fs.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>	/* Necessary because we use proc fs */
#include <linux/seq_file.h>	/* for seq_file */

#include <linux/broadcom/mobcom_types.h>
#include <linux/broadcom/bcm_major.h>
#include <linux/broadcom/ipc_sharedmemory.h>
#include <linux/broadcom/ipcinterface.h>

#include <mach/comms/platform_mconfig.h>

#include "resultcode.h"
#include "taskmsgs.h"
#include "consts.h"
#include "bcmlog.h"

#include "rpc_ipc.h"
#include "rpc_client_msgs.h"
#include "rpc_wakelock.h"
#include "rpc_debug.h"

#include <linux/broadcom/rpc_ipc_kernel.h>
#include <linux/time.h>

MODULE_LICENSE("GPL");

/******************************************/
extern void kRpcDebugPrintf(char *fmt, ...);
extern Boolean RPC_SetProperty(UInt32 type, UInt32 value);
extern Boolean RPC_GetProperty(UInt32 type, UInt32 *value);

#define RPC_TRACE_TRACE_ON
#ifdef RPC_TRACE_TRACE_ON
#define _DBG(a) a
/*#define RPC_TRACE(fmt,args...) printk (fmt, ##args)*/
#define RPC_TRACE(fmt, args...) \
	BCMLOG_Printf(BCMLOG_RPC_KERNEL_BASIC, fmt, ##args)
#else
#define _DBG(a)
#define RPC_TRACE(str) {}
#endif

#define CAL_RPC_CLIENTID 2
#define CAL_RPC_DEVID	 9	/* SERIAL_DEVICE_RPC_0 -
		Same as dft ATC RPC ch */

#ifdef USE_RPC_MUTEX

struct mutex gRpcLock;
#define DEFINE_RPC_LOCK mutex_init(&gRpcLock)
#define RPC_READ_LOCK mutex_lock(&gRpcLock)
#define RPC_READ_UNLOCK mutex_unlock(&gRpcLock)

#define RPC_WRITE_LOCK mutex_lock(&gRpcLock)
#define RPC_WRITE_UNLOCK mutex_unlock(&gRpcLock)

#else

struct rw_semaphore gRpcLock;
#define DEFINE_RPC_LOCK init_rwsem(&gRpcLock)

#if 0
#define RPC_READ_LOCK \
	do { \
	    down_read(&gRpcLock); \
	    _DBG(RPC_TRACE("[RPC_LOCK] Read Lock pid=%d", current->pid)); \
	} while (0)

#define RPC_READ_UNLOCK \
	do { \
	    up_read(&gRpcLock); \
	    _DBG(RPC_TRACE("[RPC_LOCK] Read UnLock pid=%d", current->pid)); \
	} while (0)

#define RPC_WRITE_LOCK \
	do { \
	    down_write(&gRpcLock); \
	    _DBG(RPC_TRACE("[RPC_LOCK] Write Lock pid=%d", current->pid)); \
	} while (0)

#define RPC_WRITE_UNLOCK \
	do { \
	    up_write(&gRpcLock);\
	    _DBG(RPC_TRACE("[RPC_LOCK] Write UnLock pid=%d", current->pid)); \
	} while (0)
#else

#define RPC_READ_LOCK		down_read(&gRpcLock)
#define RPC_READ_UNLOCK		up_read(&gRpcLock)

#define RPC_WRITE_LOCK		down_write(&gRpcLock)
#define RPC_WRITE_UNLOCK	up_write(&gRpcLock)

#endif

#endif

/*#define USE_INTERNAL_BUFFER*/

/**
 *  module data
 */
typedef struct {
	struct class *mDriverClass;	/* driver class */
	/* resp related */
} RPCIpc_Module_t;

typedef struct {
	struct list_head mList;
	RpcCbkType_t type;
	PACKET_InterfaceType_t interfaceType;
	UInt8 channel;
	PACKET_BufHandle_t dataBufHandle;
	RPC_FlowCtrlEvent_t event;
	struct RpcNotificationEvent_t rpcNotificationEvent;
} RpcCbkElement_t;

typedef struct {
	struct list_head mList;
	PACKET_BufHandle_t dataBufHandle;
} RpcPktkElement_t;

typedef struct {
	struct timeval ts;
	UInt8 state;
	UInt8 clientId;
	int notresponding;
	rpc_pkt_reg_ind_t info;
	rpc_pkt_reg_ind_ex_t infoEx;
	struct file *filep;
	int availData;
	pid_t	pid;
	pid_t	tid;
	char pidName[TASK_COMM_LEN];
	int IsMsgsRegistered;
	RpcCbkElement_t mQ;
	spinlock_t mLock;
	wait_queue_head_t mWaitQ;
	RpcPktkElement_t pktQ;
	int ackdCPReset;
} RpcClientInfo_t;

RpcClientInfo_t *gRpcClientList[0xFF] = { 0 };
static int gNumActiveClients = 0;
static int gEnableKprint = 0;
static int gCPResetting = 0;
/**
 *  module status
 */
static RPCIpc_Module_t sModule = { 0 };

/**
 *  private data for each session, right now, only support one client, so not needed for now.
 *  can be modified if need to support multiple clients later on.
 */
typedef struct {
	/* user file handle (for open, close and ioctl calls) */
	struct file *mUserfile;

	/* log enable/disable status bits */
	int ep;

	UInt8 clientId;
} RpcIpc_PrivData_t;

/* local function protos */

/* forward declarations used in 'struct file_operations' */

static int rpcipc_open(struct inode *inode, struct file *file);
static int rpcipc_release(struct inode *inode, struct file *file);
ssize_t rpcipc_read(struct file *filep, char __user * buf, size_t len,
		    loff_t *off);
ssize_t rpcipc_write(struct file *filep, const char __user * buf, size_t len,
		     loff_t *off);
static long rpcipc_ioctl(struct file *filp, unsigned int cmd, UInt32 arg);
static unsigned int rpcipc_poll(struct file *filp, poll_table * wait);
static int rpcipc_mmap(struct file *file, struct vm_area_struct *vma);

static void rpcipc_vma_open(struct vm_area_struct *vma);
static void rpcipc_vma_close(struct vm_area_struct *vma);
struct page *rpcipc_vma_nopage(struct vm_area_struct *vma,
			       unsigned long address, int *type);

/*****************************************************************/
static void RPC_ServerNotification(
	struct RpcNotificationEvent_t event);
RPC_Result_t RPC_ServerRxCbk(PACKET_InterfaceType_t interfaceType,
			     UInt8 channel, PACKET_BufHandle_t dataBufHandle);
static long handle_pkt_rx_buffer_ioc(struct file *filp, unsigned int cmd,
				     UInt32 param);
static long handle_pkt_read_buffer_ioc(struct file *filp, unsigned int cmd,
				       UInt32 param);
static long handle_pkt_get_buffer_info_ioc(struct file *filp, unsigned int cmd,
					   UInt32 param);
static long handle_pkt_send_buffer_ioc(struct file *filp, unsigned int cmd,
				       UInt32 param);
static long handle_pkt_alloc_buffer_ioc(struct file *filp, unsigned int cmd,
					UInt32 param);
static long handle_pkt_free_buffer_ioc(struct file *filp, unsigned int cmd,
				       UInt32 param);
static long handle_pkt_cmd_ioc(struct file *filp, unsigned int cmd,
			       UInt32 param);
static long handle_pkt_register_data_ind_ioc(struct file *filp,
					     unsigned int cmd, UInt32 param);
static long handle_test_cmd_ioc(struct file *filp, unsigned int cmd,
				UInt32 param);
static long handle_pkt_poll_ioc(struct file *filp, unsigned int cmd,
				UInt32 param);
static void RpcListCleanup(UInt8 clientId);
static int free_client_queued_pkts(UInt8 clientId);

static long handle_pkt_deregister_data_ind_ioc(struct file *filp,
					     unsigned int cmd, UInt32 param);

static long handle_pkt_register_data_ind_ex_ioc(struct file *filp,
					     unsigned int cmd, UInt32 param);
static long handle_pkt_poll_ex_ioc(struct file *filp, unsigned int cmd,
					UInt32 param);
static long handle_pkt_alloc_buffer_ptr_ioc(struct file *filp, unsigned int cmd,
					UInt32 param);
static long handle_pkt_send_buffer_ex_ioc(struct file *filp, unsigned int cmd,
				       UInt32 param);
static long handle_pkt_reg_msgs_ioc(struct file *filp, unsigned int cmd,
				       UInt32 param);
static long handle_pkt_ack_cp_reset_ioc(struct file *filp, unsigned int cmd,
				       UInt32 param);
static long free_client_unfreed_pkts(UInt8 clientId);
static int RpcIsTaskValid(pid_t tid);

/*****************************************************************/
extern Boolean is_CP_running(void);
extern UInt32 RPC_GetMaxPktSize(PACKET_InterfaceType_t interfaceType,
				UInt32 size);
extern void KRIL_SysRpc_Init(void);
extern unsigned char SYS_GenClientID(void);
extern void SYS_ReleaseClientID(unsigned char clientID);

/*****************************************************************/

/**
 *  file ops
 */
static struct file_operations rpc_ops = {
	.owner = THIS_MODULE,
	.open = rpcipc_open,
	.read = rpcipc_read,
	.write = rpcipc_write,
	.unlocked_ioctl = rpcipc_ioctl,
	.poll = rpcipc_poll,
	.mmap = rpcipc_mmap,
	.release = rpcipc_release,
};

static struct vm_operations_struct rpcipc_vm_ops = {
	.open = rpcipc_vma_open,
	.close = rpcipc_vma_close,
	/*.nopage = rpcipc_vma_nopage,*/
};

/*==========================File operations=============================*/
static int gEp = 0;

static int rpcipc_open(struct inode *inode, struct file *file)
{
	int ret = -1;
	static int sysrpc_initialized = 0;
	RpcIpc_PrivData_t *priv;

	_DBG(RPC_TRACE("rpcipc_open begin file=%x\n", (int)file));

	RPC_WRITE_LOCK;

	priv = kmalloc(sizeof(RpcIpc_PrivData_t), GFP_KERNEL);

	if (!priv) {
		_DBG(RPC_TRACE
		     ("rpcipc_open mem allocation fail file=%x\n", (int)file));
		ret = -ENOMEM;
	} else {
		priv->mUserfile = file;
		priv->ep = ++gEp;
		priv->clientId = 0;
		file->private_data = priv;

		HISTORY_RPC_LOG("open", 0, 0, (int)file, (int)current->tgid);

		if (is_CP_running()) {
			_DBG(RPC_TRACE
			     ("rpcipc_open success file=%x sysrpc_initialized=%d\n",
			      (int)file, sysrpc_initialized));

			if (!sysrpc_initialized) {
				sysrpc_initialized = 1;
				KRIL_SysRpc_Init();
			}
			ret = 0;
		} else {
			_DBG(RPC_TRACE
			     ("rpcipc_open FAIL CP is NOT running file=%x\n",
			      (int)file));
			ret = -ENODEV;
			if (priv)
				kfree(priv);
			file->private_data = NULL;
		}
	}

	RPC_WRITE_UNLOCK;

	return ret;

}

static int rpcipc_release(struct inode *inode, struct file *file)
{
	unsigned char k = 0;
	RpcIpc_PrivData_t *priv = file->private_data;

	RPC_WRITE_LOCK;

	BUG_ON(!priv);

	HISTORY_RPC_LOG("release", 0, 0, (int)file, (int)current->tgid);

	for (k = 0; k < 0xFF; k++) {
		RpcClientInfo_t *cInfo;
		cInfo = gRpcClientList[k];

		if (cInfo && (cInfo->filep == file)) {
			free_client_unfreed_pkts(cInfo->clientId);
			RpcListCleanup(cInfo->clientId);
			SYS_ReleaseClientID(cInfo->clientId);
		}
	}


	_DBG(RPC_TRACE("rpcipc_release ok\n"));

	if (priv)
		kfree(priv);

	RPC_WRITE_UNLOCK;

	return 0;
}

ssize_t rpcipc_read(struct file *filep, char __user *buf, size_t len,
		    loff_t *off)
{
	return -EFAULT;
}

ssize_t rpcipc_write(struct file *filep, const char __user *buf, size_t len,
		     loff_t *off)
{
	return -EFAULT;
}

static long rpcipc_ioctl(struct file *filp, unsigned int cmd, UInt32 arg)
{
	long retVal = 0;

	if (!is_CP_running()) {
		_DBG(RPC_TRACE("rpcipc_ioctl: Error - CP is not running\n"));
		return -ENODEV;
	}

	if (_IOC_TYPE(cmd) != RPC_SERVER_IOC_MAGIC
	    || _IOC_NR(cmd) >= RPC_SERVER_IOC_MAXNR) {
		_DBG(RPC_TRACE("rpcipc_ioctl ERROR cmd=0x%x\n", cmd));
		return -ENOIOCTLCMD;
	}

	switch (cmd) {
	case RPC_PKT_REGISTER_DATA_IND_IOC:
		{
			RPC_WRITE_LOCK;
			retVal =
			    handle_pkt_register_data_ind_ioc(filp, cmd, arg);
			RPC_WRITE_UNLOCK;
			break;
		}
	case RPC_PKT_DEREGISTER_DATA_IND_IOC:
		{
			RPC_WRITE_LOCK;
			retVal =
			    handle_pkt_deregister_data_ind_ioc(filp, cmd, arg);
			RPC_WRITE_UNLOCK;
			break;
		}
	case RPC_RX_BUFFER_IOC:
		{
			RPC_READ_LOCK;
			retVal = handle_pkt_rx_buffer_ioc(filp, cmd, arg);
			RPC_READ_UNLOCK;
			break;
		}
	case RPC_READ_BUFFER_IOC:
		{
			RPC_READ_LOCK;
			retVal = handle_pkt_read_buffer_ioc(filp, cmd, arg);
			RPC_READ_UNLOCK;
			break;
		}
	case RPC_BUFF_INFO_IOC:
		{
			RPC_READ_LOCK;
			retVal = handle_pkt_get_buffer_info_ioc(filp, cmd, arg);
			RPC_READ_UNLOCK;
			break;
		}
	case RPC_SEND_BUFFER_IOC:
		{
			RPC_READ_LOCK;
			retVal = handle_pkt_send_buffer_ioc(filp, cmd, arg);
			RPC_READ_UNLOCK;
			break;
		}
	case RPC_PKT_ALLOC_BUFFER_IOC:
		{
			RPC_READ_LOCK;
			retVal = handle_pkt_alloc_buffer_ioc(filp, cmd, arg);
			RPC_READ_UNLOCK;
			break;
		}
	case RPC_PKT_FREE_BUFFER_IOC:
		{
			RPC_READ_LOCK;
			retVal = handle_pkt_free_buffer_ioc(filp, cmd, arg);
			RPC_READ_UNLOCK;
			break;
		}
	case RPC_PKT_CMD_IOC:
		{
			RPC_READ_LOCK;
			retVal = handle_pkt_cmd_ioc(filp, cmd, arg);
			RPC_READ_UNLOCK;
			break;
		}
	case RPC_TEST_CMD_IOC:
		{
			RPC_READ_LOCK;
			retVal = handle_test_cmd_ioc(filp, cmd, arg);
			RPC_READ_UNLOCK;
			break;
		}
	case RPC_PKT_POLL_IOC:
		{
			RPC_READ_LOCK;
			retVal = handle_pkt_poll_ioc(filp, cmd, arg);
			RPC_READ_UNLOCK;
			break;
		}
	case RPC_PKT_REGISTER_DATA_IND_EX_IOC:
		{
			RPC_WRITE_LOCK;
			retVal =
			    handle_pkt_register_data_ind_ex_ioc(filp, cmd, arg);
			RPC_WRITE_UNLOCK;
			break;
		}
	case RPC_PKT_POLL_EX_IOC:
		{
			RPC_READ_LOCK;
			retVal = handle_pkt_poll_ex_ioc(filp, cmd, arg);
			RPC_READ_UNLOCK;
			break;
		}
	case RPC_PKT_ALLOC_BUFFER_PTR_IOC:
		{
			RPC_READ_LOCK;
			retVal = handle_pkt_alloc_buffer_ptr_ioc(filp, cmd, arg);
			RPC_READ_UNLOCK;
			break;
		}
	case RPC_TX_BUFFER_IOC:
		{
			RPC_READ_LOCK;
			retVal = handle_pkt_send_buffer_ex_ioc(filp, cmd, arg);
			RPC_READ_UNLOCK;
			break;
		}
	case RPC_REG_MSGS_IOC:
		{
			RPC_READ_LOCK;
			retVal = handle_pkt_reg_msgs_ioc(filp, cmd, arg);
			RPC_READ_UNLOCK;
			break;
		}
	case RPC_PKT_ACK_CP_RESET_IOC:
		{
			RPC_WRITE_LOCK;
			retVal = handle_pkt_ack_cp_reset_ioc(filp, cmd, arg);
			RPC_WRITE_UNLOCK;
			break;
		}
	default:
		retVal = -ENOIOCTLCMD;
		_DBG(RPC_TRACE
		     ("rpcipc_ioctl ERROR unhandled cmd=0x%x tst=0x%x\n", cmd,
		      RPC_RX_BUFFER_IOC));
		break;
	}

	return retVal;
}

RPC_Result_t RPC_ServerDispatchNotificationMsg(
	PACKET_InterfaceType_t interfaceType,
	UInt8 clientId,
	struct RpcNotificationEvent_t eventType)
{
	RpcCbkElement_t *elem;
	RpcClientInfo_t *cInfo;

	cInfo = gRpcClientList[clientId];

	if (!cInfo) {
		_DBG(RPC_TRACE(
"k:RPC_ServerDispatchNotificationMsg invalid clientID = %d\n",
		      clientId));
		return RPC_RESULT_ERROR;
	}

	if (clientId == 0) {
		_DBG(RPC_TRACE(
"k:RPC_ServerDispatchNotificationMsg Error !!!\n"));
		return RPC_RESULT_ERROR;
	}

	elem = kmalloc(sizeof(RpcCbkElement_t), in_interrupt() ?
			GFP_ATOMIC : GFP_KERNEL);
	if (!elem) {
		_DBG(RPC_TRACE(
"k:RPC_ServerDispatchNotificationMsg Allocation error\n"));
		return RPC_RESULT_ERROR;
	}
	memset(elem, 0, sizeof(RpcCbkElement_t));
	elem->type = RPC_SERVER_CBK_CP_RESET;
	elem->interfaceType = interfaceType;
	elem->channel = 0;
	elem->dataBufHandle = 0;
	elem->event = 0;
	elem->rpcNotificationEvent = eventType;

	_DBG(RPC_TRACE(
"RPC_ServerDispatchNotificationMsg: add to queue for client %d\n",
		clientId));

	/* add to queue */
	spin_lock_bh(&cInfo->mLock);
	list_add_tail(&elem->mList, &cInfo->mQ.mList);
	spin_unlock_bh(&cInfo->mLock);

	cInfo->availData = 1;
	wake_up_interruptible(&cInfo->mWaitQ);
	_DBG(RPC_TRACE(
"RPC_ServerDispatchNotificationMsg: done if:%d client:%d\n",
		interfaceType, clientId));

	return RPC_RESULT_OK;
}

void RPC_ServerRecoverUnfreedMsgsForClient(UInt8 clientId)
{
	RpcClientInfo_t *cInfo;
	struct list_head *pos;
	int queuedMsgs, unfreedMsgs;

	cInfo = gRpcClientList[clientId];

	if (!cInfo) {
		_DBG(RPC_TRACE
		     ("k:RPC_RPC_ServerRecoverUnfreedMsgsForClient \
			invalid clientID = %d \n", clientId));
		return;
	}

	spin_lock_bh(&cInfo->mLock);

	queuedMsgs = 0;
	unfreedMsgs = 0;
	list_for_each(pos, &cInfo->mQ.mList)queuedMsgs++;
	list_for_each(pos, &cInfo->pktQ.mList)unfreedMsgs++;
	cInfo->notresponding = (queuedMsgs > 0 || unfreedMsgs > 0) ? 1 : 0;

	spin_unlock_bh(&cInfo->mLock);

	if (queuedMsgs > 0)
		free_client_queued_pkts(cInfo->clientId);

	/* Do not free unfreedMsgs ( dequeued ) as they client may still
	using it. Just free queued packets*/
/*	if(unfreedMsgs > 0)
		free_client_unfreed_pkts(cInfo->clientId);*/

	if (queuedMsgs > 0 || unfreedMsgs > 0) {
		_DBG(RPC_TRACE("k:RPC_ServerRecoverUnfreedMsgsForClient  \
		FORCE Free Msgs tid %d cid %d, pid %d name %s qp=%d up=%d\n",
		cInfo->tid, clientId, cInfo->pid, cInfo->pidName,
		queuedMsgs, unfreedMsgs));
	} else {
		_DBG(RPC_TRACE("k:RPC_RPC_ServerRecoverUnfreedMsgsForClient \
			clientID = %d OK\n", clientId));
	}
}


void RPC_ServerRecoverUnfreedMsgs(PACKET_InterfaceType_t interfaceType)
{
	int k;
	for (k = 0; k < 0xFF; k++) {
		if (gRpcClientList[k] &&
		(gRpcClientList[k]->info.interfaceType == interfaceType &&
		gRpcClientList[k]->notresponding == 0))
			RPC_ServerRecoverUnfreedMsgsForClient(k);
	}
}


RPC_Result_t RPC_ServerDispatchMsg(PACKET_InterfaceType_t interfaceType,
				   UInt8 clientId, UInt8 channel,
				   PACKET_BufHandle_t dataBufHandle,
				   UInt16 msgId,
				   Boolean broadcastMsg)
{
	RpcCbkElement_t *elem;
	RpcClientInfo_t *cInfo;

	cInfo = gRpcClientList[clientId];

	if (!cInfo) {
		_DBG(RPC_TRACE
		    ("k:RPC_ServerDispatchMsg invalid clientID = %d msgId=%x\n",
		    clientId, (int)msgId));
		return RPC_RESULT_ERROR;
	}

	if (clientId == 0) {
		_DBG(RPC_TRACE
		     ("k:RPC_ServerDispatchMsg Error !!!  clientID zero\n"));
		return RPC_RESULT_ERROR;
	}

	if (cInfo->IsMsgsRegistered && broadcastMsg) {
		int ret;

		ret = rpc_is_registered_msg(msgId, clientId);
		if (ret != 1) {
			_DBG(RPC_TRACE
			     ("k:RPC_ServerDispatchMsg SKIP h=%d cid=%d msgId=%x ret=%d\n",
			      (int)dataBufHandle, clientId, (int)msgId, ret));
			return RPC_RESULT_ERROR;
		}
	}

	/*Find out if the process of client is dead*/
	if (cInfo->tid && RpcIsTaskValid(cInfo->tid) == -ESRCH) {
		HISTORY_RPC_LOG("zombie", clientId, (int)dataBufHandle,
			(int)cInfo->tid, (int)cInfo->pid);

		_DBG(RPC_TRACE("k:RPC_ServerDispatchMsg ERROR(Invalid tid %d) \
			 cid %d, pid %d name %s\n",
			 cInfo->tid, clientId, cInfo->pid, cInfo->pidName));
		/*Switch to write lock and back to read lock*/
		RPC_READ_UNLOCK;

		RPC_WRITE_LOCK;
		free_client_unfreed_pkts(cInfo->clientId);
		RpcListCleanup(cInfo->clientId);
		/*SYS_ReleaseClientID(cInfo->clientId);*/
		RPC_WRITE_UNLOCK;

		RPC_READ_LOCK;

		return RPC_RESULT_ERROR;
	}

	if (cInfo->notresponding) {
		int empty;
		spin_lock_bh(&cInfo->mLock);
		empty = list_empty(&cInfo->mQ.mList);
		spin_unlock_bh(&cInfo->mLock);
		if (!empty) {
			_DBG(RPC_TRACE("k:RPC_ServerDispatchMsg ERROR(\
			 NOT RESPONDING tid %d) cid %d, pid %d name %s\n",
			cInfo->tid, clientId, cInfo->pid, cInfo->pidName));
			return RPC_RESULT_ERROR;
		}
	}
	elem = kmalloc(sizeof(RpcCbkElement_t), in_interrupt() ?
			GFP_ATOMIC : GFP_KERNEL);
	if (!elem) {
		_DBG(RPC_TRACE("k:RPC_ServerDispatchMsg Allocation error\n"));
		return RPC_RESULT_ERROR;
	}
	memset(elem, 0, sizeof(RpcCbkElement_t));
	elem->type = RPC_SERVER_CBK_RX_DATA;
	elem->interfaceType = interfaceType;
	elem->channel = channel;
	elem->dataBufHandle = dataBufHandle;
	elem->event = 0;
	RPC_PACKET_IncrementBufferRef(dataBufHandle, clientId);

	_DBG(RPC_TRACE
	     ("k:RPC_ServerDispatchMsg cInfo=%x h=%d cid=%d elem=%x msgId=%x b=%d nr=%d\n",
	      (int)cInfo, (int)dataBufHandle, clientId, (int)elem,
		(int)msgId, broadcastMsg, cInfo->notresponding));

	/* add to queue */
	RpcDbgUpdatePktStateEx((int)dataBufHandle, PKT_STATE_NA, clientId,
			PKT_STATE_CID_DISPATCH,  0, (int)msgId, (UInt8)interfaceType);
	HISTORY_RPC_LOG("Dispatch", clientId, (int)dataBufHandle,
							interfaceType, msgId);
	spin_lock_bh(&cInfo->mLock);
	list_add_tail(&elem->mList, &cInfo->mQ.mList);
	cInfo->availData = 1;
	spin_unlock_bh(&cInfo->mLock);

	wake_up_interruptible(&cInfo->mWaitQ);

	return RPC_RESULT_PENDING;
}

RPC_Result_t RPC_ServerRxCbk(PACKET_InterfaceType_t interfaceType,
			     UInt8 channel, PACKET_BufHandle_t dataBufHandle)
{
	RpcClientInfo_t *cInfo;
	UInt8 clientId, k;
	UInt16 msgId = 0;
	RPC_Result_t ret = RPC_RESULT_ERROR;
	int bFound = 0;

	RPC_READ_LOCK;

	if (RPC_PACKET_IsReservedPkt(dataBufHandle)) {
		RPC_ServerRecoverUnfreedMsgs(interfaceType);
		RPC_READ_UNLOCK;
		return RPC_RESULT_OK;
	}
	if (interfaceType == INTERFACE_CAPI2) {
		if (channel == 0xCD)
			channel = 0;

		/* All response from CP will pass clientID in channel */
		clientId = channel;
	}
	/*For non-telephony clients, match by interface type */
	else {
		clientId = 0;
		/* Check for CALD client */
		if ((channel == CAL_RPC_DEVID) &&
		    (interfaceType == INTERFACE_SERIAL) &&
		    gRpcClientList[CAL_RPC_CLIENTID]) {
			clientId = CAL_RPC_CLIENTID;
			bFound = 1;
		} else {
			for (k = 0; k < 0xFF; k++) {
				RpcClientInfo_t *cInfo;
				cInfo = gRpcClientList[k];
				if (cInfo
				    && (cInfo->info.interfaceType ==
					interfaceType)) {
					clientId = k;
					bFound = 1;
					break;
				}
		}
		}
	}

	_DBG(RPC_TRACE
	     ("k:RPC_ServerRxCbk if=%d pkt=%d ch=%d cid=%d found=%d numClients=%d\n",
	      (int)interfaceType, (int)dataBufHandle, channel, (int)clientId,
	      (int)bFound, gNumActiveClients));

	/* Did not find client with registered interface type */
	if (interfaceType != INTERFACE_CAPI2 && !bFound)
		ret = RPC_RESULT_ERROR;
#if 0
	/* Drop DRX message for now */
	else if (interfaceType == INTERFACE_CAPI2 && channel == 201)
		ret = RPC_RESULT_ERROR;
#endif
	else {
		cInfo = gRpcClientList[clientId];

		msgId = RPC_PACKET_GetContextEx(interfaceType, dataBufHandle);

		if (clientId != 0 && cInfo) {
			ret =
			    RPC_ServerDispatchMsg(interfaceType, clientId,
						  channel, dataBufHandle,
						  msgId, 0);
		} else if (gNumActiveClients == 0) {
			ret = RPC_RESULT_OK;
		} else {
			Boolean bSent = 0;
			RPC_Result_t ret2 = RPC_RESULT_ERROR;

			/* lock the buffer */
			RPC_PACKET_IncrementBufferRef(dataBufHandle, 0);
			for (k = 0; k < 0xFF; k++) {
				if (gRpcClientList[k]
				    && (gRpcClientList[k]->info.interfaceType ==
					INTERFACE_CAPI2)) {
					ret2 =
					    RPC_ServerDispatchMsg(interfaceType,
								  k, channel,
								  dataBufHandle,
								  msgId, 1);
					if (ret2 == RPC_RESULT_PENDING)
						bSent = 1;
				}
			}
			/* unlock buffer */
			RPC_PACKET_FreeBufferEx(dataBufHandle, 0);
			ret = RPC_RESULT_PENDING;
		}
	}

	RPC_READ_UNLOCK;

	_DBG(RPC_TRACE("k:RPC_ServerRxCbk ret=%d\n", ret));

	return ret;
}

/* handle notifcations of CP reset from rpc_ipc layer */
void RPC_ServerNotification(struct RpcNotificationEvent_t event)
{
	int k;
	PACKET_InterfaceType_t interfaceType = event.ifType;

	switch (event.event) {
	case RPC_CPRESET_EVT:
		_DBG(RPC_TRACE(
	"RPC_ServerNotification: event %d interface %d\n",
		       (int) event.event, interfaceType));

	/* if we're already in process of resetting, just return */
	if ((gCPResetting && (event.param == RPC_CPRESET_START)) ||
	    (!gCPResetting && (event.param == RPC_CPRESET_COMPLETE))) {
		_DBG(RPC_TRACE
			("RPC_ServerNotification already handling event\n"));
		return;
	}

	RPC_READ_LOCK;

	/* start of CP reset process; notify all user space clients */
	gCPResetting = (event.param == RPC_CPRESET_START);

	for (k = 0; k < 0xFF; k++)
		if (gRpcClientList[k]) {
			_DBG(RPC_TRACE
			 ("RPC_ServerNotification: client %d\n", k));
			if (event.param == RPC_CPRESET_START)
				gRpcClientList[k]->ackdCPReset = 0;
			event.ifType = gRpcClientList[k]->info.interfaceType;
			RPC_ServerDispatchNotificationMsg(
				event.ifType,
				k, event);
		}

	_DBG(RPC_TRACE("RPC_ServerNotification: done\n"));
	RPC_READ_UNLOCK;
		break;
	default:
		_DBG(RPC_TRACE(
		"RPC_ServerNotification: Unsupported event %d\n",
		(int) event.event));
		break;
	}
}

static unsigned int rpcipc_poll(struct file *filp, poll_table * wait)
{
	UInt32 mask = 0;
	RpcClientInfo_t *cInfo;
	RpcIpc_PrivData_t *priv = filp->private_data;

	if (!is_CP_running()) {
	    _DBG(RPC_TRACE("rpcipc_poll: Error - CP is not running\n"));
	    return POLLERR;
	}

	BUG_ON(!priv);

	cInfo = gRpcClientList[priv->clientId];

	if (!cInfo) {
	    _DBG(RPC_TRACE
		("k:rpcipc_poll invalid clientID %d\n", priv->clientId));
	    return POLLERR;
	}
	/*
	_DBG(RPC_TRACE("k:rpcipc_poll() start client=%d\n", priv->clientId));
	*/

	/* if data exist already, just return */
	spin_lock_bh(&cInfo->mLock);
	if (!list_empty(&cInfo->mQ.mList)) {
	    spin_unlock_bh(&cInfo->mLock);
	    mask |= (POLLIN | POLLRDNORM);
	    _DBG(RPC_TRACE("k:rpcipc_poll() precheck list not empty\n"));
	    return mask;
	}
	do_gettimeofday(&(cInfo->ts));
	cInfo->state = 1;

	spin_unlock_bh(&cInfo->mLock);

	/*wait till data is ready */
	/*_DBG(RPC_TRACE("k:rpcipc_poll() begin wait %x\n", (int)jiffies));*/
	poll_wait(filp, &cInfo->mWaitQ, wait);
	/*_DBG(RPC_TRACE("k:rpcipc_poll() end wait %x\n", (int)jiffies));*/

	spin_lock_bh(&cInfo->mLock);
	if (!list_empty(&cInfo->mQ.mList))
		mask |= (POLLIN | POLLRDNORM);

	_DBG(RPC_TRACE("rpcipc_poll: mask = %x\n", (int)mask));

	cInfo->state = 2;
	do_gettimeofday(&(cInfo->ts));
	cInfo->availData = 0;

	spin_unlock_bh(&cInfo->mLock);

	_DBG(RPC_TRACE("rpcipc_poll: mask = %x\n", (int)mask));


	/*_DBG(RPC_TRACE("k:rpcipc_poll() mask=%x avail=%d\n", (int)mask, (int)(mask & (POLLIN | POLLRDNORM))?1:0 )); */

	return mask;
}

static long handle_pkt_poll_ioc(struct file *filp, unsigned int cmd,
				UInt32 param)
{
	int copyRc;
	RpcClientInfo_t *cInfo;
	rpc_pkt_avail_t ioc_param = { 0 };

	if (copy_from_user
	    (&ioc_param, (rpc_pkt_avail_t *) param,
	     sizeof(rpc_pkt_avail_t)) != 0) {
		_DBG(RPC_TRACE
		     ("k:handle_pkt_poll_ioc - copy_from_user() had error\n"));
		return -EFAULT;
	}

	if (ioc_param.clientId >= ARRAY_SIZE(gRpcClientList))
		return -EINVAL;

	cInfo = gRpcClientList[ioc_param.clientId];
	if (!cInfo) {
		_DBG(RPC_TRACE
		     ("k:handle_pkt_poll_ioc invalid clientID %d\n",
		      ioc_param.clientId));
		return -EINVAL;
	}

	spin_lock_bh(&cInfo->mLock);
	ioc_param.isEmpty = (Boolean) list_empty(&cInfo->mQ.mList);

	if (ioc_param.isEmpty)
		cInfo->availData = 0;

	do_gettimeofday(&(cInfo->ts));
	cInfo->state = 1;
	cInfo->tid = current->pid;

	spin_unlock_bh(&cInfo->mLock);

	if (ioc_param.waitTime > 0 && ioc_param.isEmpty) {


		RPC_READ_UNLOCK;
		wait_event_interruptible_timeout(cInfo->mWaitQ,
						cInfo->availData,
						 msecs_to_jiffies(ioc_param.
								  waitTime));
		RPC_READ_LOCK;

		/* Re-evaluate cInfo after wait */
		cInfo = gRpcClientList[ioc_param.clientId];
		if (!cInfo) {
			_DBG(RPC_TRACE
			 ("k:pkt_poll_ioc invalid clientID after WAIT %d\n",
			      ioc_param.clientId));
			return -EINVAL;
		}


	    spin_lock_bh(&cInfo->mLock);
		ioc_param.isEmpty = (Boolean) list_empty(&cInfo->mQ.mList);
		cInfo->state = 2;
		do_gettimeofday(&(cInfo->ts));
	    spin_unlock_bh(&cInfo->mLock);
	}

	_DBG(RPC_TRACE
	    ("k:handle_pkt_poll_ioc clientId=%d empty=%d\n",
	    (int)ioc_param.clientId, (int)ioc_param.isEmpty));

	copyRc = copy_to_user((rpc_pkt_avail_t *) param, &ioc_param,
			sizeof(rpc_pkt_avail_t));
	if (copyRc != 0) {
		_DBG(RPC_TRACE
			("k:handle_pkt_poll_ioc - copy_to_user() FAILS! \
				RC=%d\n", copyRc));
		_DBG(RPC_TRACE
			("  src addr=%x, dest addr=%x, size=%d\n",
				(int)&ioc_param, (int)param, sizeof(rpc_pkt_avail_t)));
		_DBG(RPC_TRACE
			("  clientId:%d, isEmpty:%d, waitTime:%d\n",
				(int)ioc_param.clientId,
				(int)ioc_param.isEmpty, (int)ioc_param.waitTime));
		return -EFAULT;
	}

	return 0;
}

static long handle_pkt_rx_buffer_ioc(struct file *filp, unsigned int cmd,
				     UInt32 param)
{
	rpc_pkt_rx_buf_t ioc_param = { 0 };
	struct list_head *entry;
	RpcCbkElement_t *Item = NULL;
	RpcClientInfo_t *cInfo;
	RpcPktkElement_t *pktElem;

	if (copy_from_user
	    (&ioc_param, (rpc_pkt_rx_buf_t *) param,
	     sizeof(rpc_pkt_rx_buf_t)) != 0) {
		_DBG(RPC_TRACE
		     ("k:handle_pkt_rx_buffer_ioc - copy_from_user() had error\n"));
		return -EFAULT;
	}

	_DBG(RPC_TRACE
	     ("k:handle_pkt_rx_buffer_ioc client=%d\n", ioc_param.clientId));

	if (ioc_param.clientId >= ARRAY_SIZE(gRpcClientList))
		return -EINVAL;

	cInfo = gRpcClientList[ioc_param.clientId];

	if (!cInfo) {
		_DBG(RPC_TRACE
		     ("k:handle_pkt_rx_buffer_ioc invalid clientID %d\n",
		      ioc_param.clientId));
		return -EINVAL;
	}

	pktElem = kmalloc(sizeof(RpcPktkElement_t), GFP_KERNEL);
	if (!pktElem) {
		_DBG(RPC_TRACE("k:handle_pkt_rx_buffer_ioc Allocation error cid=%d\n", ioc_param.clientId));
		return -EFAULT;
	}

	/* Get one resp from the queue */
	spin_lock_bh(&cInfo->mLock);
	if (list_empty(&cInfo->mQ.mList)) {
		spin_unlock_bh(&cInfo->mLock);
		_DBG(RPC_TRACE("k:handle_pkt_rx_buffer_ioc Q empty\n"));
		kfree(pktElem);
		return -EAGAIN;
	}

	/* Remove from Rx List */
	entry = cInfo->mQ.mList.next;
	Item = list_entry(entry, RpcCbkElement_t, mList);
	list_del(entry);

	/* Add to free List */
	pktElem->dataBufHandle = Item->dataBufHandle;
	list_add_tail(&pktElem->mList, &cInfo->pktQ.mList);

	cInfo->state = 2;
	do_gettimeofday(&(cInfo->ts));
	cInfo->notresponding = 0;

	spin_unlock_bh(&cInfo->mLock);

	HISTORY_RPC_LOG_PKT("rxBuf", ioc_param.clientId, (int)Item->dataBufHandle);

	RpcDbgUpdatePktStateEx((int)Item->dataBufHandle, PKT_STATE_NA, ioc_param.clientId, PKT_STATE_CID_FETCH,  0, 0, 0xFF);

	ioc_param.type = Item->type;
	ioc_param.interfaceType = Item->interfaceType;
	ioc_param.channel = Item->channel;
	ioc_param.dataBufHandle = Item->dataBufHandle;
	ioc_param.event = Item->event;
	ioc_param.dataIndFunc = cInfo->info.dataIndFunc;
	ioc_param.flowIndFunc = cInfo->info.flowIndFunc;
	ioc_param.len = RPC_PACKET_GetBufferLength(Item->dataBufHandle);

	_DBG(RPC_TRACE
	     ("k:handle_pkt_rx_buffer_ioc item=%x len=%d pkt=%x\n", (int)Item,
	      (int)ioc_param.len, (int)Item->dataBufHandle));

	kfree(entry);


	if (copy_to_user
	    ((rpc_pkt_rx_buf_t *) param, &ioc_param,
	     sizeof(rpc_pkt_rx_buf_t)) != 0) {
		_DBG(RPC_TRACE
		     ("k:handle_pkt_rx_buffer_ioc - copy_to_user() had error\n"));
		return -EFAULT;
	}

	return 0;
}

static long handle_pkt_read_buffer_ioc(struct file *filp, unsigned int cmd,
				       UInt32 param)
{
	rpc_pkt_user_buf_t ioc_param = { 0 };
	RpcClientInfo_t *cInfo;
	UInt8 *buffer;

	if (param == 0) {
		_DBG(RPC_TRACE("k:%s param is invalid address\n", __func__));
		return -EFAULT;
	}
	if (copy_from_user
	    (&ioc_param, (rpc_pkt_user_buf_t *) param,
	     sizeof(rpc_pkt_user_buf_t)) != 0) {
		_DBG(RPC_TRACE
		     ("k:handle_pkt_read_buffer_ioc - copy_from_user() had error\n"));
		return -EFAULT;
	}

	cInfo = gRpcClientList[ioc_param.clientId];

	if (!cInfo) {
		_DBG(RPC_TRACE
		     ("k:handle_pkt_read_buffer_ioc invalid clientID %d\n",
		      ioc_param.clientId));
		return -EINVAL;
	}

	_DBG(RPC_TRACE
	     ("k:handle_pkt_read_buffer_ioc client=%d pkt=%x len=%d\n",
	      ioc_param.clientId, (int)ioc_param.dataBufHandle,
	      (int)ioc_param.userBufLen));

	buffer = RPC_PACKET_GetBufferData(ioc_param.dataBufHandle);

	HISTORY_RPC_LOG_PKT("GetBufPtr", ioc_param.clientId,
					(int)ioc_param.dataBufHandle);
	if (ioc_param.userBufLen == 0) {
				_DBG(RPC_TRACE
				("k:%s - invalid userBufLen = %d\n",
				__func__, ioc_param.userBufLen));
				return -EFAULT;
	}
	if (copy_to_user(ioc_param.userBuf, buffer, ioc_param.userBufLen) != 0) {
		_DBG(RPC_TRACE
		     ("k:handle_pkt_read_buffer_ioc - copy_to_user() had error\n"));
		return -EFAULT;
	}

	return 0;
}

static long handle_pkt_get_buffer_info_ioc(struct file *filp, unsigned int cmd,
					   UInt32 param)
{
	rpc_pkt_buf_info_t ioc_param = { 0 };
	UInt8 *buffer;

	if (copy_from_user
	    (&ioc_param, (rpc_pkt_buf_info_t *) param,
	     sizeof(rpc_pkt_buf_info_t)) != 0) {
		_DBG(RPC_TRACE
		     ("k:handle_pkt_get_buffer_info_ioc - copy_from_user() had error\n"));
		return -EFAULT;
	}

	buffer = RPC_PACKET_GetBufferData(ioc_param.dataBufHandle);
	ioc_param.len = RPC_PACKET_GetBufferLength(ioc_param.dataBufHandle);
	ioc_param.kernelPtr = buffer;
	ioc_param.kernelBasePtr = IPC_SmAddress(0);
	ioc_param.offset = IPC_SmOffset(buffer);

	HISTORY_RPC_LOG_PKT("GetBufInfo", 0, (int)ioc_param.dataBufHandle);

	_DBG(RPC_TRACE
	     ("k:handle_pkt_get_buffer_info_ioc pkt=%x ptr=%x offset=%d base=%x len=%d\n",
	      (int)ioc_param.dataBufHandle, (int)ioc_param.kernelPtr,
	      (int)ioc_param.offset, (int)ioc_param.kernelBasePtr,
	      (int)ioc_param.len));

	if (copy_to_user
	    ((rpc_pkt_buf_info_t *) param, &ioc_param,
	     sizeof(rpc_pkt_buf_info_t)) != 0) {
		_DBG(RPC_TRACE
		     ("k:handle_pkt_get_buffer_info_ioc - copy_to_user() had error\n"));
		return -EFAULT;
	}

	return 0;
}

static long handle_pkt_send_buffer_ioc(struct file *filp, unsigned int cmd,
				       UInt32 param)
{
	rpc_pkt_user_buf_t ioc_param = { 0 };
	/*RpcClientInfo_t *cInfo;*/

	if (copy_from_user
	    (&ioc_param, (rpc_pkt_user_buf_t *) param,
	     sizeof(rpc_pkt_user_buf_t)) != 0) {
		_DBG(RPC_TRACE
		     ("k:handle_pkt_send_buffer_ioc - copy_from_user() had error\n"));
		return -EFAULT;
	}

	_DBG(RPC_TRACE
	     ("k:handle_pkt_send_buffer_ioc client=%d pkt=%x len=%d\n",
	      (int)ioc_param.clientId, (int)ioc_param.dataBufHandle,
	      (int)ioc_param.userBufLen));

#ifdef USE_INTERNAL_BUFFER
	{
		UInt8 *buffer =
		    RPC_PACKET_GetBufferData(ioc_param.dataBufHandle);
		UInt8 *srcBuf = (UInt8 *) ioc_param.userBuf;
		UInt32 bufLen =
		    RPC_PACKET_GetBufferLength(ioc_param.dataBufHandle);

		if (copy_from_user(buffer, ioc_param.userBuf, bufLen) != 0) {
			_DBG(RPC_TRACE
			     ("k:handle_pkt_send_buffer_ioc - buf copy_from_user() had error\n"));
			return -EFAULT;
		}
		_DBG(RPC_TRACE
		     ("k:handle_pkt_send_buffer_ioc len=%d dest[%x:%x:%x:%x:%x], src[%x:%x:%x:%x:%x]\n",
		      bufLen, buffer[0], buffer[1], buffer[2], buffer[3],
		      buffer[4], srcBuf[0], srcBuf[1], srcBuf[2], srcBuf[3],
		      srcBuf[4]));
	}
#endif
	/*fixme: Handle return value?*/
	{
		RPC_READ_UNLOCK;
		RPC_PACKET_SendData(ioc_param.clientId, ioc_param.interfaceType,
				    ioc_param.channel, ioc_param.dataBufHandle);
		RPC_READ_LOCK;

		HISTORY_RPC_LOG_PKT("Send", ioc_param.clientId, (int)ioc_param.dataBufHandle);
	}

	return 0;
}

static long handle_pkt_alloc_buffer_ioc(struct file *filp, unsigned int cmd,
					UInt32 param)
{
	rpc_pkt_alloc_buf_t ioc_param = { 0 };

	if (param == 0) {
		_DBG(RPC_TRACE("k:%s param is invalid address\n", __func__));
		return -EFAULT;
	}
	if (copy_from_user
	    (&ioc_param, (rpc_pkt_alloc_buf_t *) param,
	     sizeof(rpc_pkt_alloc_buf_t)) != 0) {
		_DBG(RPC_TRACE
		     ("k:handle_pkt_alloc_buffer_ioc - copy_from_user() had error\n"));
		return -EFAULT;
	}

	if (ioc_param.interfaceType >= INTERFACE_TOTAL ||
				ioc_param.interfaceType < INTERFACE_START) {
			_DBG(RPC_TRACE("k:%s invalid interfaceType=%d\n"
				, __func__, ioc_param.interfaceType));
			return -EINVAL;
	}
	RPC_READ_UNLOCK;
	ioc_param.pktBufHandle =
	    RPC_PACKET_AllocateBufferEx(ioc_param.interfaceType,
					ioc_param.requiredSize,
					ioc_param.channel, ioc_param.waitTime);
	RPC_READ_LOCK;

	HISTORY_RPC_LOG_PKT("Alloc", 0, (int)ioc_param.pktBufHandle);

	_DBG(RPC_TRACE
	     ("k:handle_pkt_alloc_buffer_ioc pkt=%x len=%d\n",
	      (int)ioc_param.pktBufHandle, (int)ioc_param.requiredSize));

	if (copy_to_user
	    ((rpc_pkt_alloc_buf_t *) param, &ioc_param,
	     sizeof(rpc_pkt_alloc_buf_t)) != 0) {
		_DBG(RPC_TRACE
		     ("k:handle_pkt_alloc_buffer_ioc - copy_to_user() had error\n"));
		return -EFAULT;
	}

	return 0;
}

static long free_client_pkt(struct file *filp, UInt8 clientId, PACKET_BufHandle_t pktBufHandle)
{
	RpcClientInfo_t *cInfo;
	RpcPktkElement_t *Item = NULL;
	struct list_head *listptr, *pos;
	RPC_Result_t res;
	int ret = -1, k = 0;

	cInfo = gRpcClientList[clientId];

	if (!cInfo) {
		for (k = 0; k < 0xFF; k++) {
			RpcClientInfo_t *tempcInfo;
			tempcInfo = gRpcClientList[k];

			if (tempcInfo && (tempcInfo->filep == filp)) {
				cInfo = tempcInfo;
				break;
			}
		}
	}

	if (!cInfo) {
		_DBG(RPC_TRACE
		     ("k:free_client_pkt Error Invalid Client: filep %d clientID %d pkt %d\n",
		      (int)filp, (int)clientId, (int)pktBufHandle));
		return -EINVAL;
	}

	spin_lock_bh(&cInfo->mLock);
	list_for_each_safe(listptr, pos, &cInfo->pktQ.mList)
	{
		Item = list_entry(listptr, RpcPktkElement_t, mList);
		if (Item->dataBufHandle == pktBufHandle) {
			list_del(listptr);
			_DBG(RPC_TRACE
			     ("k:free_client_pkt FREE clientID %d pkt %d\n", (int)clientId,
			      (int)pktBufHandle));
			res = RPC_PACKET_FreeBufferEx(pktBufHandle, clientId);
			ret = (res == RPC_RESULT_OK) ? 0 : -EFAULT;
			kfree(Item);
			break;
		}
	}
	spin_unlock_bh(&cInfo->mLock);
	if (ret != 0)
		_DBG(RPC_TRACE
		     ("k:free_client_pkt ERROR Pkt Not Found filep %d clientID %d pkt %d\n",
		      (int)filp, (int)clientId, (int)pktBufHandle));

	return ret;
}

static long free_client_unfreed_pkts(UInt8 clientId)
{
	RpcClientInfo_t *cInfo;
	RpcPktkElement_t *Item = NULL;
	struct list_head *listptr, *pos;
	RPC_Result_t res;
	int ret = -1;

	cInfo = gRpcClientList[clientId];

	if (!cInfo)
		return -1;

	spin_lock_bh(&cInfo->mLock);
	list_for_each_safe(listptr, pos, &cInfo->pktQ.mList)
	{
		Item = list_entry(listptr, RpcPktkElement_t, mList);
		list_del(listptr);
		res = RPC_PACKET_FreeBufferEx(Item->dataBufHandle, clientId);
		_DBG(RPC_TRACE
		     ("k:free_client_unfreed_pkts FREE clientID %d pkt %d res %d\n", (int)clientId,
		      (int)Item->dataBufHandle, (int)res));
		kfree(Item);
		ret = (res == RPC_RESULT_OK) ? 0 : -EFAULT;
	}
	spin_unlock_bh(&cInfo->mLock);

	return ret;
}

static long handle_pkt_free_buffer_ioc(struct file *filp, unsigned int cmd,
				       UInt32 param)
{
	int ret = -1;
	rpc_pkt_free_buf_t ioc_param = { 0 };
	if (param == 0) {
		_DBG(RPC_TRACE("k:%s param is invalid address\n", __func__));
		return -EFAULT;
	}
	if (copy_from_user
	    (&ioc_param, (rpc_pkt_free_buf_t *) param,
	     sizeof(rpc_pkt_free_buf_t)) != 0) {
		_DBG(RPC_TRACE
		     ("k:handle_pkt_free_buffer_ioc - copy_from_user() had error\n"));
		return -EFAULT;
	}

	if (ioc_param.dataBufHandle == NULL) {
		_DBG(RPC_TRACE("k:%s dataBufHandle is invalid pinter\n",
		__func__));
		return -EFAULT;
	}
	ret = free_client_pkt(filp, ioc_param.clientId, ioc_param.dataBufHandle);

	return ret;
}

static long handle_pkt_cmd_ioc(struct file *filp, unsigned int cmd,
			       UInt32 param)
{
	rpc_pkt_cmd_t ioc_param = { 0 };

	if (copy_from_user
	    (&ioc_param, (rpc_pkt_cmd_t *) param, sizeof(rpc_pkt_cmd_t)) != 0) {
		_DBG(RPC_TRACE
		     ("k:handle_pkt_cmd_ioc - copy_from_user() had error\n"));
		return -EFAULT;
	}

	ioc_param.outParam = 0;
	ioc_param.result = 0;

	if (ioc_param.type == RPC_PROXY_INFO_GET_NUM_BUFFER) {
		ioc_param.outParam =
		    RPC_PACKET_Get_Num_FreeBuffers(ioc_param.interfaceType,
						   ioc_param.input1);
	} else if (ioc_param.type == RPC_PROXY_INFO_SET_BUFFER_LEN) {
		RPC_PACKET_SetBufferLength(ioc_param.dataBufHandle,
					   ioc_param.input1);
	} else if (ioc_param.type == RPC_PROXY_INFO_GET_PROPERTY) {
		ioc_param.result =
		    RPC_GetProperty(ioc_param.input1, &(ioc_param.outParam));
	} else if (ioc_param.type == RPC_PROXY_INFO_SET_PROPERTY) {
		ioc_param.result =
		    RPC_SetProperty(ioc_param.input1, ioc_param.input2);
	} else if (ioc_param.type == RPC_PROXY_INFO_GET_CONTEXT) {
		ioc_param.outParam =
		    RPC_PACKET_GetContext(ioc_param.interfaceType,
					  ioc_param.dataBufHandle);
	} else if (ioc_param.type == RPC_PROXY_INFO_SET_CONTEXT) {
		ioc_param.result =
		    RPC_PACKET_SetContext(ioc_param.interfaceType,
					  ioc_param.dataBufHandle,
					  ioc_param.input1);
	} else if (ioc_param.type == RPC_PROXY_INFO_GET_CONTEXT_EX) {
		ioc_param.outParam =
		    RPC_PACKET_GetContextEx(ioc_param.interfaceType,
					    ioc_param.dataBufHandle);
	} else if (ioc_param.type == RPC_PROXY_INFO_SET_CONTEXT_EX) {
		ioc_param.result =
		    RPC_PACKET_SetContextEx(ioc_param.interfaceType,
					    ioc_param.dataBufHandle,
					    ioc_param.input1);
	} else if (ioc_param.type == RPC_PROXY_INFO_GET_MAX_PKT_SIZE) {
		ioc_param.outParam =
		    RPC_GetMaxPktSize(ioc_param.interfaceType,
				      ioc_param.input1);
	} else if (ioc_param.type == RPC_PROXY_INFO_GET_CID) {
		ioc_param.outParam = (unsigned char)SYS_GenClientID();
	} else if (ioc_param.type == RPC_PROXY_INFO_RELEASE_CID) {
		SYS_ReleaseClientID((UInt8) ioc_param.input1);
	} else if (ioc_param.type == RPC_PROXY_INFO_GET_MAX_IPC_SIZE) {
		ioc_param.outParam = IPC_SIZE;
	} else if (ioc_param.type == RPC_PROXY_GET_PERSISTENT_OFFSET) {
		IPC_PersistentDataStore_t thePersistentData;
		IPC_GetPersistentData(&thePersistentData);
		ioc_param.outParam = IPC_SmOffset(thePersistentData.DataPtr);
		ioc_param.result = thePersistentData.DataLength;
	} else if (ioc_param.type == RPC_PROXY_WAKEUP_USER_THREAD) {
		RpcClientInfo_t *cInfo;
		UInt8 cid = (UInt8) ioc_param.input1;
		cInfo = gRpcClientList[cid];

		if (cInfo) {
			spin_lock_bh(&cInfo->mLock);
			cInfo->availData = 1;
			spin_unlock_bh(&cInfo->mLock);
			wake_up_interruptible(&cInfo->mWaitQ);
			ioc_param.outParam = 1;
		} else {
			_DBG(RPC_TRACE
			     ("k:RPC_PROXY_WAKEUP_USER_THREAD invalid clientID %d\n",
			      cid));
			ioc_param.outParam = 0;
		}
	} else {
		_DBG(RPC_TRACE
		     ("k:handle_pkt_cmd_ioc ERROR: Invalid option=%d\n",
		      ioc_param.type));
		return -1;
	}

	HISTORY_RPC_LOG("Cmd", 0, (int)ioc_param.dataBufHandle, ioc_param.input1, ioc_param.outParam);

	_DBG(RPC_TRACE
	     ("k:handle_pkt_cmd_ioc cmd=%d itype=%x handle=%x i1=%x i2=%x res=%x out=%x\n",
	      (int)ioc_param.type, (int)ioc_param.interfaceType,
	      (int)ioc_param.dataBufHandle, (int)ioc_param.input1,
	      (int)ioc_param.input2, (int)ioc_param.result,
	      (int)ioc_param.outParam));
	if (copy_to_user
	    ((rpc_pkt_cmd_t *) param, &ioc_param, sizeof(rpc_pkt_cmd_t)) != 0) {
		_DBG(RPC_TRACE
		     ("k:handle_pkt_cmd_ioc - copy_to_user() had error\n"));
		return -EFAULT;
	}

	return 0;
}

static long handle_pkt_register_data_ind_ioc(struct file *filp,
					     unsigned int cmd, UInt32 param)
{
	RpcClientInfo_t *cInfo;
	UInt8 clientId = 0;
	rpc_pkt_reg_ind_t ioc_param = { 0 };
	RpcIpc_PrivData_t *priv = filp->private_data;

	BUG_ON(!priv);

	if (copy_from_user
	    (&ioc_param, (rpc_pkt_reg_ind_t *) param,
	     sizeof(rpc_pkt_reg_ind_t)) != 0) {
		_DBG(RPC_TRACE
		     ("k:handle_pkt_register_data_ind_ioc - copy_from_user() had error\n"));
		return -EFAULT;
	}

	clientId = ioc_param.rpcClientID;
	priv->clientId = clientId;

	if (clientId == 0) {
		_DBG(RPC_TRACE
		     ("handle_pkt_register_data_ind_ioc - clientId is zero\n"));
		return -EINVAL;
	}

	if (gRpcClientList[clientId]) {
		_DBG(RPC_TRACE
		     ("handle_pkt_register_data_ind_ioc - clientId exist\n"));
		return -EBUSY;
	}

	cInfo =
	    (RpcClientInfo_t *)kmalloc(sizeof(RpcClientInfo_t), GFP_KERNEL);

	if (!cInfo) {
		_DBG(RPC_TRACE
		     ("handle_pkt_register_data_ind_ioc - kmalloc fail\n"));
		return -ENOMEM;
	}

	memset(cInfo, 0, sizeof(RpcClientInfo_t));

	gRpcClientList[clientId] = cInfo;
	gNumActiveClients++;

	_DBG(RPC_TRACE
	     ("k:handle_pkt_register_data_ind_ioc client=%d numclients=%d\n",
	      ioc_param.rpcClientID, gNumActiveClients));

	/*fill up*/
	cInfo->info = ioc_param;
	cInfo->infoEx.rpcClientID = cInfo->info.rpcClientID;
	cInfo->infoEx.interfaceType = cInfo->info.interfaceType;
	cInfo->infoEx.dataIndFuncEx = NULL;
	cInfo->infoEx.flowIndFunc = cInfo->info.flowIndFunc;
	cInfo->infoEx.rpcNotificationFunc = cInfo->info.rpcNotificationFunc;

	cInfo->clientId = clientId;
	cInfo->filep = filp;

	cInfo->pid = current->tgid;
	cInfo->tid = 0;
	strncpy(cInfo->pidName, current->comm, TASK_COMM_LEN);

	cInfo->ackdCPReset = 0;

	/* Init module  queue */
	INIT_LIST_HEAD(&cInfo->mQ.mList);
	INIT_LIST_HEAD(&cInfo->pktQ.mList);
	spin_lock_init(&cInfo->mLock);
	init_waitqueue_head(&cInfo->mWaitQ);
	cInfo->availData = 0;

	HISTORY_RPC_LOG("Register", clientId, 0, current->tgid, cInfo->info.interfaceType);

	RPC_PACKET_RegisterFilterCbk(ioc_param.rpcClientID,
				ioc_param.interfaceType,
				RPC_ServerRxCbk,
				RPC_ServerNotification);

	return 0;
}

static long handle_pkt_deregister_data_ind_ioc(struct file *filp,
					     unsigned int cmd, UInt32 param)
{
	RpcClientInfo_t *cInfo;
	UInt8 clientId = 0;
	rpc_pkt_dereg_ind_t ioc_param = { 0 };
	RpcIpc_PrivData_t *priv = filp->private_data;

	BUG_ON(!priv);

	if (copy_from_user
	    (&ioc_param, (rpc_pkt_dereg_ind_t *) param,
	     sizeof(rpc_pkt_dereg_ind_t)) != 0) {
		_DBG(RPC_TRACE
		     ("k:handle_pkt_deregister_data_ind_ioc - copy_from_user() had error\n"));
		return -EFAULT;
	}

	clientId = ioc_param.rpcClientID;

	if (clientId == 0) {
		_DBG(RPC_TRACE
		     ("handle_pkt_deregister_data_ind_ioc - clientId is zero\n"));
		return -EINVAL;
	}

	cInfo = gRpcClientList[clientId];

	if (!cInfo) {
		_DBG(RPC_TRACE
		     ("handle_pkt_deregister_data_ind_ioc - Invalid clientId\n"));
		return -EINVAL;
	}

	RpcListCleanup(cInfo->clientId);
	SYS_ReleaseClientID(cInfo->clientId);

	HISTORY_RPC_LOG("DeRegister", cInfo->clientId, 0, current->tgid, cInfo->info.interfaceType);

	_DBG(RPC_TRACE
	     ("k:handle_pkt_deregister_data_ind_ioc client=%d numclients=%d\n",
	      ioc_param.rpcClientID, gNumActiveClients));

	return 0;
}

static long handle_pkt_register_data_ind_ex_ioc(struct file *filp,
					     unsigned int cmd, UInt32 param)
{
	RpcClientInfo_t *cInfo;
	UInt8 clientId = 0;
	rpc_pkt_reg_ind_ex_t ioc_param_ex = { 0 };
	RpcIpc_PrivData_t *priv = filp->private_data;
	int ret;

	BUG_ON(!priv);

	ret = copy_from_user
	    (&ioc_param_ex, (rpc_pkt_reg_ind_ex_t *) param,
	     sizeof(rpc_pkt_reg_ind_ex_t));

	clientId = ioc_param_ex.rpcClientID;

	if (ret != 0) {
		_DBG(RPC_TRACE
		     ("k:handle_pkt_register_data_ind_ex_ioc - copy_from_user() had error\n"));
		return -EFAULT;
	}

	priv->clientId = clientId;

	if (clientId == 0) {
		_DBG(RPC_TRACE
		     ("handle_pkt_register_data_ind_ex_ioc - clientId is zero\n"));
		return -EINVAL;
	}

	if (gRpcClientList[clientId]) {
		_DBG(RPC_TRACE
		     ("handle_pkt_register_data_ind_ex_ioc - clientId exist\n"));
		return -EBUSY;
	}

	cInfo =
	    (RpcClientInfo_t *)kmalloc(sizeof(RpcClientInfo_t), GFP_KERNEL);

	if (!cInfo) {
		_DBG(RPC_TRACE
		     ("handle_pkt_register_data_ind_ex_ioc - kmalloc fail\n"));
		return -ENOMEM;
	}

	memset(cInfo, 0, sizeof(RpcClientInfo_t));

	gRpcClientList[clientId] = cInfo;
	gNumActiveClients++;

	_DBG(RPC_TRACE
	     ("k:handle_pkt_register_data_ind_ex_ioc client=%d numclients=%d\n",
	      clientId, gNumActiveClients));

	/* fill up */
	cInfo->infoEx = ioc_param_ex;
	cInfo->info.rpcClientID = cInfo->infoEx.rpcClientID;
	cInfo->info.interfaceType = cInfo->infoEx.interfaceType;
	cInfo->info.dataIndFunc = NULL;
	cInfo->info.flowIndFunc = cInfo->infoEx.flowIndFunc;
	cInfo->info.rpcNotificationFunc = cInfo->infoEx.rpcNotificationFunc;

	cInfo->clientId = clientId;
	cInfo->filep = filp;

	cInfo->pid = current->tgid;
	cInfo->tid = 0;
	strncpy(cInfo->pidName, current->comm, TASK_COMM_LEN);

	cInfo->ackdCPReset = 0;
	/* Init module  queue */
	INIT_LIST_HEAD(&cInfo->mQ.mList);
	INIT_LIST_HEAD(&cInfo->pktQ.mList);
	spin_lock_init(&cInfo->mLock);
	init_waitqueue_head(&cInfo->mWaitQ);
	cInfo->availData = 0;

	HISTORY_RPC_LOG("RegisterEx", clientId, 0, current->tgid, cInfo->info.interfaceType);

	RPC_PACKET_RegisterFilterCbk(ioc_param_ex.rpcClientID,
				ioc_param_ex.interfaceType,
				RPC_ServerRxCbk,
				RPC_ServerNotification);

	return 0;
}

static long handle_pkt_poll_ex_ioc(struct file *filp, unsigned int cmd,
					UInt32 param)
{
	RpcClientInfo_t *cInfo;
	struct list_head *entry;
	RpcCbkElement_t *Item = NULL;
	rpc_pkt_rx_buf_ex_t ioc_param;
	unsigned int tsBefore = 0, tsnow = 0;
	RpcPktkElement_t *pktElem = NULL;
	struct timeval ts;
	do_gettimeofday(&ts);
	tsBefore = ts.tv_usec;
	/* coverity[ -tainted_data_argument : arg-0 ]*/
	if (copy_from_user
	    (&ioc_param, (rpc_pkt_rx_buf_ex_t *) param,
	     sizeof(rpc_pkt_rx_buf_ex_t)) != 0) {
		_DBG(RPC_TRACE
		     ("k:handle_pkt_poll_ex_ioc - copy_from_user() had error\n"));
		return -EFAULT;
	}

	if (ioc_param.clientId >= ARRAY_SIZE(gRpcClientList))
			return -EINVAL;

	cInfo = gRpcClientList[ioc_param.clientId];
	if (!cInfo) {
		_DBG(RPC_TRACE
		     ("k:handle_pkt_poll_ex_ioc invalid clientID %d\n",
		      ioc_param.clientId));
		return -EINVAL;
	}

	spin_lock_bh(&cInfo->mLock);
	ioc_param.isEmpty = (Boolean) list_empty(&cInfo->mQ.mList);
	if (ioc_param.isEmpty)
		cInfo->availData = 0;
	cInfo->state = 1;
	do_gettimeofday(&(cInfo->ts));
	cInfo->tid = current->pid;
	spin_unlock_bh(&cInfo->mLock);

	if (ioc_param.waitTime > 0 && ioc_param.isEmpty) {

		RPC_READ_UNLOCK;

		wait_event_interruptible_timeout(cInfo->mWaitQ,
						cInfo->availData,
						 msecs_to_jiffies(ioc_param.
								  waitTime));
		RPC_READ_LOCK;

		/* Re-evaluate cInfo after wait */
		cInfo = gRpcClientList[ioc_param.clientId];
		if (!cInfo) {
			_DBG(RPC_TRACE
			     ("k:handle_pkt_poll_ex_ioc invalid clientID after WAIT %d\n",
			      ioc_param.clientId));
			return -EINVAL;
		}

	    spin_lock_bh(&cInfo->mLock);
		ioc_param.isEmpty = (UInt8) list_empty(&cInfo->mQ.mList);
		cInfo->state = 2;
		do_gettimeofday(&(cInfo->ts));
	    spin_unlock_bh(&cInfo->mLock);
	}

	if (!(ioc_param.isEmpty)) {
		pktElem = kmalloc(sizeof(RpcPktkElement_t), GFP_KERNEL);
		if (!pktElem) {
			_DBG(RPC_TRACE("k:handle_pkt_poll_ex_ioc Allocation error cid=%d\n", ioc_param.clientId));
			return -EFAULT;
		}

		spin_lock_bh(&cInfo->mLock);
		entry = cInfo->mQ.mList.next;
		Item = list_entry(entry, RpcCbkElement_t, mList);
		list_del(entry);
		/* Add to free List */
		if (Item->dataBufHandle) {
			pktElem->dataBufHandle = Item->dataBufHandle;
			list_add_tail(&pktElem->mList, &cInfo->pktQ.mList);
		}
		cInfo->notresponding = 0;

		spin_unlock_bh(&cInfo->mLock);

		ioc_param.isEmpty = 0;
		ioc_param.type = Item->type;
		ioc_param.event = Item->event;
		ioc_param.rpcNotificationEvent = Item->rpcNotificationEvent;
		ioc_param.dataIndFuncEx = cInfo->infoEx.dataIndFuncEx;
		ioc_param.dataIndFunc = cInfo->info.dataIndFunc;
		ioc_param.flowIndFunc = (cInfo->infoEx.flowIndFunc) ?
			cInfo->infoEx.flowIndFunc :
			cInfo->info.flowIndFunc;
		ioc_param.rpcNotificationFunc =
			(cInfo->infoEx.rpcNotificationFunc) ?
			cInfo->infoEx.rpcNotificationFunc :
			cInfo->info.rpcNotificationFunc;
		ioc_param.bufInfo.interfaceType = Item->interfaceType;
		ioc_param.bufInfo.channel = Item->channel;
		ioc_param.bufInfo.dataBufHandle = Item->dataBufHandle;
		if (Item->dataBufHandle) {
			ioc_param.bufInfo.bufferLen =
				RPC_PACKET_GetBufferLength(
					Item->dataBufHandle);
			ioc_param.bufInfo.buffer =
				RPC_PACKET_GetBufferData(Item->dataBufHandle);
			ioc_param.bufInfo.context1 =
				RPC_PACKET_GetContext(Item->interfaceType,
					Item->dataBufHandle);
			ioc_param.bufInfo.context2 =
				RPC_PACKET_GetContextEx(Item->interfaceType,
					Item->dataBufHandle);

			ioc_param.offset = IPC_SmOffset(
			ioc_param.bufInfo.buffer);

			RpcDbgUpdatePktStateEx((int)Item->dataBufHandle,
					PKT_STATE_NA, ioc_param.clientId,
					PKT_STATE_CID_FETCH,  0, 0, 0xFF);
			HISTORY_RPC_LOG_PKT("rxBufEx", ioc_param.clientId,
				(int)Item->dataBufHandle);
		} else {
			kfree(pktElem);
		}
		do_gettimeofday(&(cInfo->ts));
		tsnow = cInfo->ts.tv_usec;
		_DBG(RPC_TRACE("k:handle_pkt_poll_ex_ioc cid=%d item=%x \
				len=%d pkt=%d wait=%d\n",
			ioc_param.clientId, (int)Item,
			(int)ioc_param.bufInfo.bufferLen,
			(int)Item->dataBufHandle,
			tsnow - tsBefore));

		kfree(entry);

	} else {
		do_gettimeofday(&(cInfo->ts));
		tsnow = cInfo->ts.tv_usec;
		_DBG(RPC_TRACE
		    ("k:handle_pkt_poll_ex_ioc EMPTY cid=%d wait=%d\n",
		     (int)ioc_param.clientId, tsnow - tsBefore));
	}

	if (copy_to_user
	    ((rpc_pkt_rx_buf_ex_t *) param, &ioc_param,
	     sizeof(rpc_pkt_rx_buf_ex_t)) != 0) {
		_DBG(RPC_TRACE
		     ("k:handle_pkt_poll_ex_ioc - copy_to_user() had error\n"));
		return -EFAULT;
	}

	return 0;
}

static long handle_pkt_alloc_buffer_ptr_ioc(struct file *filp, unsigned int cmd,
					UInt32 param)
{
	rpc_pkt_alloc_buf_ptr_t ioc_param = { 0 };

	if (param == 0) {
		_DBG(RPC_TRACE("k:%s param is invalid address\n", __func__));
		return -EFAULT;
	}
	/* coverity[ -tainted_data_argument : arg-0 ] */
	if (copy_from_user
	    (&ioc_param, (rpc_pkt_alloc_buf_ptr_t *) param,
	     sizeof(rpc_pkt_alloc_buf_ptr_t)) != 0) {
		_DBG(RPC_TRACE
		     ("k:handle_pkt_alloc_buffer_ptr_ioc - copy_from_user() had error\n"));
		return -EFAULT;
	}

	if (ioc_param.interfaceType >= INTERFACE_TOTAL ||
				ioc_param.interfaceType < INTERFACE_START) {
		_DBG(RPC_TRACE("k:%s invalid interfaceType=%d\n"
				, __func__, ioc_param.interfaceType));
		return -EINVAL;
	}
	RPC_READ_UNLOCK;
	ioc_param.pktBufHandle =
	    RPC_PACKET_AllocateBufferEx(ioc_param.interfaceType,
					ioc_param.requiredSize,
					ioc_param.channel, ioc_param.waitTime);
	RPC_READ_LOCK;

	if (ioc_param.pktBufHandle) {
		void *ptr;
		ptr = RPC_PACKET_GetBufferData(ioc_param.pktBufHandle);
		ioc_param.offset = IPC_SmOffset(ptr);
		ioc_param.allocatedSize = RPC_GetMaxPktSize(ioc_param.interfaceType,
				      ioc_param.requiredSize);
	}

	_DBG(RPC_TRACE
	     ("k:handle_pkt_alloc_buffer_ptr_ioc pkt=%x len=%d\n",
	      (int)ioc_param.pktBufHandle, (int)ioc_param.requiredSize));

	HISTORY_RPC_LOG("Alloc", 0, (int)ioc_param.pktBufHandle, ioc_param.offset, ioc_param.allocatedSize);

	if (copy_to_user
	    ((rpc_pkt_alloc_buf_ptr_t *) param, &ioc_param,
	     sizeof(rpc_pkt_alloc_buf_ptr_t)) != 0) {
		_DBG(RPC_TRACE
		     ("k:handle_pkt_alloc_buffer_ptr_ioc - copy_to_user() had error\n"));
		return -EFAULT;
	}

	return 0;
}

static long handle_pkt_send_buffer_ex_ioc(struct file *filp, unsigned int cmd,
				       UInt32 param)
{
	rpc_pkt_tx_buf_t ioc_param;

	if (copy_from_user
	    (&ioc_param, (rpc_pkt_tx_buf_t *) param,
	     sizeof(rpc_pkt_tx_buf_t)) != 0) {
		_DBG(RPC_TRACE
		     ("k:handle_pkt_send_buffer_ex_ioc - copy_from_user() had error\n"));
		return -EFAULT;
	}

	_DBG(RPC_TRACE
	     ("k:handle_pkt_send_buffer_ex_ioc client=%d pkt=%x len=%d\n",
	      (int)ioc_param.txBuf.clientId, (int)ioc_param.txBuf.dataBufHandle,
	      (int)ioc_param.txBuf.bufferLen));

#ifdef USE_INTERNAL_BUFFER
	{
		UInt8 *buffer =
		    RPC_PACKET_GetBufferData(ioc_param.txBuf.dataBufHandle);
		UInt8 *srcBuf = (UInt8 *) ioc_param.txBuf.buffer;
		UInt32 bufLen =
		    RPC_PACKET_GetBufferLength(ioc_param.txBuf.dataBufHandle);

		if (copy_from_user(buffer, ioc_param.txBuf.buffer, ioc_param.txBuf.bufferLen) != 0) {
			_DBG(RPC_TRACE
			     ("k:handle_pkt_send_buffer_ex_ioc - buf copy_from_user() had error\n"));
			return -EFAULT;
		}
		_DBG(RPC_TRACE
		     ("k:handle_pkt_send_buffer_ex_ioc len=%d dest[%x:%x:%x:%x:%x], src[%x:%x:%x:%x:%x]\n",
		      bufLen, buffer[0], buffer[1], buffer[2], buffer[3],
		      buffer[4], srcBuf[0], srcBuf[1], srcBuf[2], srcBuf[3],
		      srcBuf[4]));
	}
#endif
	/*fixme: Handle return value?*/
	{
		RPC_PACKET_SetBufferLength(ioc_param.txBuf.dataBufHandle, ioc_param.txBuf.bufferLen);

		RPC_PACKET_SetContext(ioc_param.txBuf.interfaceType, ioc_param.txBuf.dataBufHandle, ioc_param.txBuf.context1);
		RPC_PACKET_SetContextEx(ioc_param.txBuf.interfaceType, ioc_param.txBuf.dataBufHandle, ioc_param.txBuf.context2);

		RPC_READ_UNLOCK;
		RPC_PACKET_SendData(ioc_param.txBuf.clientId, ioc_param.txBuf.interfaceType,
				    ioc_param.txBuf.channel, ioc_param.txBuf.dataBufHandle);
		RPC_READ_LOCK;

		HISTORY_RPC_LOG("SendEx", ioc_param.txBuf.clientId, (int)ioc_param.txBuf.dataBufHandle, ioc_param.txBuf.interfaceType, ioc_param.txBuf.context2);

	}

	return 0;
}

static long handle_pkt_reg_msgs_ioc(struct file *filp, unsigned int cmd,
				       UInt32 param)
{
	RpcPktRegMsgIds_t ioc_param = { 0 };
	UInt8 cid = 0;
	RpcClientInfo_t *cInfo;
	int readSize;
	Boolean ret;

	if (copy_from_user
	    (&ioc_param, (RpcPktRegMsgIds_t *) param,
	     8) != 0) {
		_DBG(RPC_TRACE
		     ("k:handle_pkt_reg_msgs_ioc - copy_from_user() had error\n"));
		return -EFAULT;
	}

	if (ioc_param.msgsSize == 0)
		return -EINVAL;

	cid = (UInt8)(ioc_param.clientId);

	cInfo = gRpcClientList[cid];

	if (!cInfo) {
		_DBG(RPC_TRACE
		     ("k:handle_pkt_reg_msgs_ioc invalid clientID %d\n",
		      cid));
		return -EINVAL;
	}

	readSize = (offsetof(RpcPktRegMsgIds_t, msgs) + (ioc_param.msgsSize * 2));

	if (copy_from_user
	    (&ioc_param, (RpcPktRegMsgIds_t *) param,
	     readSize) != 0) {
		_DBG(RPC_TRACE
		     ("k:handle_pkt_reg_msgs_ioc - copy_from_user() had error\n"));
		return -EFAULT;
	}

	ret = rpc_register_client_msgs(cid, ioc_param.msgs,
					ioc_param.msgsSize);

	cInfo->IsMsgsRegistered = 1;

	HISTORY_RPC_LOG("RegMsgs", cid, 0, readSize, ioc_param.msgsSize);

	_DBG(RPC_TRACE
	     ("k:handle_pkt_reg_msgs_ioc cid=%d offset=%d sz=%d readsz=%d ret=%d\n",
		cid,
		offsetof(RpcPktRegMsgIds_t, msgs),
		(int)ioc_param.msgsSize, readSize, ret));

	return (!ret) ? -EINVAL : 0;
}

static long handle_pkt_ack_cp_reset_ioc(struct file *filp, unsigned int cmd,
				       UInt32 param)
{
	rpc_pkt_cp_reset_ack_t ioc_param;

	if (param == 0) {
		_DBG(RPC_TRACE("k:%s param is invalid address\n", __func__));
		return -EFAULT;
	}
	if (copy_from_user
	    (&ioc_param, (int *) param,
	     sizeof(rpc_pkt_cp_reset_ack_t)) != 0) {
		_DBG(RPC_TRACE("k:handle_pkt_ack_cp_reset_ioc - \
			 copy_from_user() had error\n"));
		return -EFAULT;
	}
	
	_DBG(RPC_TRACE("handle_pkt_ack_cp_reset_ioc: %d %d\n",
				ioc_param.rpcClientID,
				ioc_param.interfaceType));

	if (INTERFACE_CAPI2 != ioc_param.interfaceType) {
		_DBG(RPC_TRACE("handle_pkt_ack_cp_reset_ioc: ack ifce %d\n",
			ioc_param.interfaceType));
		if (ioc_param.interfaceType >= INTERFACE_TOTAL ||
				ioc_param.interfaceType < INTERFACE_START) {
			_DBG(RPC_TRACE("k:%s invalid interfaceType=%d\n"
					, __func__, ioc_param.interfaceType));
			return -EINVAL;
		}
		RPC_PACKET_FilterAckReadyForCPReset(ioc_param.rpcClientID,
						ioc_param.interfaceType);
	} else {
		UInt8 cid = 0;
		RpcClientInfo_t *cInfo;
		int k;
		int bComplete = 1;

		_DBG(RPC_TRACE("handle_pkt_ack_cp_reset_ioc: capi2 client\n"));

		/* for capi2 interface, track ack'd status in client info
		   and only call back to rpc_ipc after all clients have ack'd
		*/
		cid = (UInt8)(ioc_param.rpcClientID);

		cInfo = gRpcClientList[cid];

		if (!cInfo) {
			_DBG(RPC_TRACE
			("k:handle_pkt_ack_cp_reset_ioc invalid clientID %d\n",
			cid));
			return -EINVAL;
		}

		cInfo->ackdCPReset = 1;

		for (k = 0; k < 0xFF; k++) {
			cInfo = gRpcClientList[k];
			if (cInfo &&
			  (cInfo->info.interfaceType == INTERFACE_CAPI2) &&
			  cInfo->info.rpcNotificationFunc &&
			  !cInfo->ackdCPReset) {
				_DBG(RPC_TRACE
				("k:handle_pkt_ack_cp_reset_ioc not ackd %d\n",
				k));
				bComplete = 0;
				break;
			}
		}

		/* if all capi2 clients have ack'd, info rpc_ipc layer */
		if (bComplete) {
			_DBG(RPC_TRACE
			("k:handle_pkt_ack_cp_reset_ioc all ack'd\n"));
			_DBG(RPC_TRACE("     calling rpc_ipc\n"));
			RPC_PACKET_FilterAckReadyForCPReset(
						ioc_param.rpcClientID,
						ioc_param.interfaceType);
		}
	}

	return 0;
}

void __iomem *rpcipc_shmem;

static long handle_test_cmd_ioc(struct file *filp, unsigned int cmd,
				UInt32 param)
{
	rpc_pkt_test_cmd_t ioc_param = { 0 };

	if (copy_from_user
	    (&ioc_param, (rpc_pkt_test_cmd_t *) param,
	     sizeof(rpc_pkt_test_cmd_t)) != 0) {
		_DBG(RPC_TRACE
		     ("k:handle_test_cmd_ioc - copy_from_user() had error\n"));
		return -EFAULT;
	}

	_DBG(RPC_TRACE
	     ("k:handle_test_cmd_ioc cmd1=%x cmd2=%x cmd3=%x\n", ioc_param.cmd1,
	      ioc_param.cmd2, ioc_param.cmd3));

	if (ioc_param.cmd1 == 1) {
		rpcipc_shmem = ioremap_nocache(IPC_BASE, IPC_SIZE);
		_DBG(RPC_TRACE
		     ("k:handle_test_cmd_ioc ioremap_nocache() mem=%x\n",
		      (int)rpcipc_shmem));

	} else if (ioc_param.cmd1 == 2) {
		if (rpcipc_shmem)
			iounmap(rpcipc_shmem);
	} else if (ioc_param.cmd1 == 3) {
		UInt32 val1;
		IPC_SmControl_T *IPC_cb_ptr = (IPC_SmControl_T *) rpcipc_shmem;

		val1 = IPC_cb_ptr->Properties[RPC_PROP_AP_IN_DEEPSLEEP];

		_DBG(RPC_TRACE
		     ("K:dumpIpcData: init_cp=%x init_cp=%x conf=%x val1=%x\n",
		      (int)IPC_cb_ptr->Initialised[0],
		      (int)IPC_cb_ptr->Initialised[1], (int)IPC_SmConfigured,
		      (int)val1));
	} else if (ioc_param.cmd1 == 4) {
		gEnableKprint = ioc_param.cmd2;
		printk("RPC kernel log = %d\n", gEnableKprint);
	}

	if (copy_to_user
	    ((rpc_pkt_test_cmd_t *) param, &ioc_param,
	     sizeof(rpc_pkt_test_cmd_t)) != 0) {
		_DBG(RPC_TRACE
		     ("k:handle_test_cmd_ioc - copy_to_user() had error\n"));
		return -EFAULT;
	}

	return 0;
}

/****************************************************************************/
/****************************************************************************/

static int free_client_queued_pkts(UInt8 clientId)
{
	struct list_head *listptr, *pos;
	RpcClientInfo_t *cInfo;
	RpcCbkElement_t *Item = NULL;
	int num = 0;

	cInfo = gRpcClientList[clientId];
	if (!cInfo)
		return 0;

	spin_lock_bh(&cInfo->mLock);
	list_for_each_safe(listptr, pos, &cInfo->mQ.mList) {
		Item = list_entry(listptr, RpcCbkElement_t, mList);
		list_del(listptr);
		spin_unlock_bh(&cInfo->mLock);

		RPC_PACKET_FreeBufferEx(Item->dataBufHandle, clientId);

		_DBG(RPC_TRACE
		     ("k:free_client_queued_pkts index=%d item=%x\n", clientId,
		      (int)Item));
		kfree(Item);
		num++;
		spin_lock_bh(&cInfo->mLock);
	}
	spin_unlock_bh(&cInfo->mLock);

	return num;
}

static void RpcListCleanup(UInt8 clientId)
{
	free_client_queued_pkts(clientId);
	gRpcClientList[clientId] = NULL;
	gNumActiveClients--;

}

static void RpcServerCleanup(void)
{
	int k;

	for (k = 0; k < 0xFF; k++)
		RpcListCleanup(k);
}

/* mmap() requirements for ipcdev.
 * - Process should always map this region from the start.
 * - It should always map exactly 1MB of region at once,
 *   expansions (e.g. mremap() )will not be allowed.
 */
static int rpcipc_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned long off = vma->vm_pgoff << PAGE_SHIFT;
	/*  Should be the start PFN, and not the phys addres */
	unsigned long phys_start = (IPC_BASE + off) >> PAGE_SHIFT;

	_DBG(RPC_TRACE
	     ("rpcipc_mmap base=%x size=%x pageoff=%x off=%x shift=%x\n",
	      (int)IPC_BASE, (int)IPC_SIZE, (int)vma->vm_pgoff, (int)off,
	      (int)PAGE_SHIFT));
	_DBG(RPC_TRACE
	     ("rpcipc_mmap phyStart=%x vm_start=%x vm_end=%x size=%x prot=%x\n",
	      (int)phys_start, (int)vma->vm_start, (int)vma->vm_end,
	      (int)(vma->vm_end - vma->vm_start), (int)vma->vm_page_prot));

	/* Make it mandatory for the process to map this region from the
	 * start.
	 */
	if (off) {
		_DBG(RPC_TRACE("k:rpcipc_mmap Invalid offset=%x\n", (int)off));
		return -EINVAL;
	}

	if ((vma->vm_end - vma->vm_start) != (IPC_SIZE)) {
		_DBG(RPC_TRACE
		     ("k:rpcipc_mmap Invalid size=%x req=%x\n",
		      (int)(vma->vm_end - vma->vm_start), (int)IPC_SIZE));
		return -EINVAL;	/* Cant map, dont have enough phys mem */
	}

	/* This has to be uncached acess, for CP to see the changes */
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	if (io_remap_pfn_range(vma, vma->vm_start, phys_start,
			       (vma->vm_end - vma->vm_start),
			       vma->vm_page_prot)) {
		_DBG(RPC_TRACE
		     ("k:rpcipc_mmap io_remap_pfn_range FAIL!!! size=%x req=%x\n",
		      (int)(vma->vm_end - vma->vm_start), (int)IPC_SIZE));
		return -EAGAIN;
	}
	/* Do this to setup the nopage method, and avoid
	 * extension of this vma.
	 */
	vma->vm_ops = &rpcipc_vm_ops;
	return 0;
}

static void rpcipc_vma_open(struct vm_area_struct *vma)
{
	/* Nothing here, this is called mostly on a fork() call
	 * by the process.
	 */
	_DBG(RPC_TRACE("rpcipc_vma_open()\n"));
}

static void rpcipc_vma_close(struct vm_area_struct *vma)
{
	/* Nothing here */
	_DBG(RPC_TRACE("rpcipc_vma_close()\n"));
}

struct page *rpcipc_vma_nopage(struct vm_area_struct *vma,
			       unsigned long address, int *type)
{
	_DBG(RPC_TRACE("rpcipc_vma_nopage()\n"));
	/* No more mmapping */
	/*return NOPAGE_SIGBUS;*/
	return NULL;
}


/****************************************************************************
*                      LOG PROC FILE
****************************************************************************/

int RpcIsTaskValid(pid_t inTid)
{
	struct task_struct *task;
	rcu_read_lock();
	task = find_task_by_vpid(inTid);
	rcu_read_unlock();
	return (!task) ? -ESRCH : 0;
}

void RpcDumpTaskState(RpcOutputContext_t *c, pid_t inTid, pid_t inPid)
{
	struct task_struct *t;

	rcu_read_lock();
	t = find_task_by_vpid(inTid);
	rcu_read_unlock();

	if (t) {
		RpcDbgDumpStr(c, "\t%s pid:%d tid:%d state:%d flag:0x%x\n",
			      t->comm, inPid, inTid, (int)t->state, (int)t->flags);

		RpcDumpTaskCallStack(c, t);
	} else
		RpcDbgDumpStr(c, "\tError!!!(Process not found) pid:%d tid:%d\n",
						inPid, inTid);
}

int RpcDbgListClientMsgs(RpcOutputContext_t *c)
{
	UInt8 clientId;
	RpcClientInfo_t *cInfo;
	RpcPktkElement_t *pktItem = NULL;
	RpcCbkElement_t *cbkItem = NULL;
	struct list_head *listptr, *pos;
	int k;


	for (k = 0; k < 0xFF; k++) {
		clientId = k;
		cInfo = gRpcClientList[clientId];

		if (!cInfo)
			continue;

		RpcDbgDumpStr(c,
				"%s cid:%d state:%s ts:%u:%u RxData:%d \
				Active:%d if:%d\n",
				cInfo->pidName,
				(int)clientId,
				(cInfo->state == 1) ? "Wait" : "Active",
				(unsigned int)cInfo->ts.tv_sec,
				(unsigned int)cInfo->ts.tv_usec,
				(int)cInfo->availData,
				(int)cInfo->notresponding,
				(int)cInfo->info.interfaceType);

		RpcDumpTaskState(c, cInfo->tid, cInfo->pid);

		if (c->protected)
			spin_lock(&cInfo->mLock);
		else
			spin_lock_bh(&cInfo->mLock);

		list_for_each_safe(listptr, pos, &cInfo->pktQ.mList)
		{
			pktItem = list_entry(listptr, RpcPktkElement_t, mList);

			RpcDbgDumpStr(c,  "\tcid:%d NOT FREED pkt:%d\n",
						(int)clientId,
						(int)pktItem->dataBufHandle);
		}

		list_for_each_safe(listptr, pos, &cInfo->mQ.mList)
		{
			cbkItem = list_entry(listptr, RpcCbkElement_t, mList);

			RpcDbgDumpStr(c, "\tcid:%d QUEUED pkt:%d\n",
						(int)clientId,
						(int)cbkItem->dataBufHandle);
		}

		if (c->protected)
			spin_unlock(&cInfo->mLock);
		else
			spin_unlock_bh(&cInfo->mLock);

	}
	return 0;
}

/****************************************************************************
*                      LOG SEQ PROC FILE
****************************************************************************/


typedef struct {
	UInt32 counter;
	int offset;
	int eof;
	RpcOutputContext_t out;
} RpcLogContext_t;

static void *log_seq_start(struct seq_file *s, loff_t *pos)
{
	static RpcLogContext_t context = {0, 0, 0, { 1, FALSE, NULL, {0} } };

	context.out.type = 1;
	context.out.seq = s;



	/* beginning a new sequence ? */
	if (context.eof == 0) {
		/* yes => return a non null value to begin the sequence */
		return &context;
	} else {
		/* no => it's the end of the sequence,
		   return end to stop reading */
		context.offset = 0;
		context.counter = 0;
		*pos = 0;
		context.eof = 0;
		return NULL;
	}
}

static void *log_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	RpcLogContext_t *context = (RpcLogContext_t *)v;

	(*pos)++;


	if (context->eof || *pos > 10000) {
		context->eof = 1;
		return NULL;
	}

	return (void *)context;
}

static void log_seq_stop(struct seq_file *s, void *v)
{
}

static int log_seq_show(struct seq_file *s, void *v)
{
	int ret = 0;
	RpcLogContext_t *context = (RpcLogContext_t *)v;

	if (context->counter == 0)
	    RpcDbgDumpHdr(&(context->out));
	else if (context->counter == 1)
	    RpcDbgListClientMsgs(&(context->out));
	else if (context->counter == 2)
	    RpcDbgDumpKthread(&(context->out), 0);
	else if (context->counter == 3)
	    RpcDbgDumpKthread(&(context->out), 1);
	else if (context->counter == 4)
	    RpcDbgDumpWakeLockStats(&(context->out));
	else if (context->counter == 5)
	    ret = RpcDbgDumpPktState(&(context->out), &(context->offset), 10);
	else if (context->counter == 6)
	    ret = RbcDbgDumpGenInfo(&(context->out), &(context->offset), 10);

	if (ret == 0) {
		if (context->counter >= 6) {
			RpcDbgDumpHdr(&(context->out));
			context->eof = 1;
		}

		context->offset = 0;
		(context->counter)++;
	}
	return 0;
}



static struct seq_operations log_seq_ops = {
	.start = log_seq_start,
	.next  = log_seq_next,
	.stop  = log_seq_stop,
	.show  = log_seq_show
};

static int my_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &log_seq_ops);
}

static struct file_operations my_file_ops = {
	.owner   = THIS_MODULE,
	.open    = my_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release
};

int log_proc_init(void)
{
	struct proc_dir_entry *myproc = proc_create_data("bcmrpclog", 0667, 0,
					 &my_file_ops, NULL);

	if (myproc == NULL)
		return -ENOMEM;

	return 0;
}

void log_proc_cleanup(void)
{
	remove_proc_entry("bcmrpclog", NULL);
}

int RpcDbgDumpHistoryLogging(int type, int level)
{
	RpcOutputContext_t outContext = {0, TRUE, NULL, {0} };
	int offset = 0;

	outContext.type = type;

	RpcDbgDumpStr(&outContext,
			"===== RPC memory dump Begin ( %d )=====", type);

	RpcDbgDumpHdr(&outContext);
	RpcDbgListClientMsgs(&outContext);
	RpcDbgDumpWakeLockStats(&outContext);
	RpcDbgDumpKthread(&outContext, 2);

	if (level > 0) {
		RpcDbgDumpPktState(&outContext, &offset, 0);
		RbcDbgDumpGenInfo(&outContext, &offset, 0);
		RpcDbgDumpHdr(&outContext);
	}

	RpcDbgDumpStr(&outContext, "===== RPC memory dump End =====");

	return 0;
}

static int major;
/**************************************************************************/
/**
 *  Called by Linux I/O system to initialize module.
 *  @return int     0 if success, -1 if error
 *  @note
 *      API is defined by module_init macro
 */
static int __init rpcipc_ModuleInit(void)
{
	int k;
	struct device *drvdata;
	_DBG(RPC_TRACE("enter rpcipc_ModuleInit()\n"));

	for (k = 0; k < 0xFF; k++)
		gRpcClientList[k] = NULL;

	gNumActiveClients = 0;

	/* drive driver process: */
	if ((major = register_chrdev(0, BCM_KERNEL_RPC_NAME, &rpc_ops)) < 0) {
		_DBG(RPC_TRACE("register_chrdev failed\n"));
		return major;
	}

	sModule.mDriverClass = class_create(THIS_MODULE, BCM_KERNEL_RPC_NAME);
	if (IS_ERR(sModule.mDriverClass)) {
		_DBG(RPC_TRACE("driver class_create failed\n"));
		unregister_chrdev(major, BCM_KERNEL_RPC_NAME);
		return PTR_ERR(sModule.mDriverClass);
	}

	drvdata =
	    device_create(sModule.mDriverClass, NULL, MKDEV(major, 0), NULL,
			  BCM_KERNEL_RPC_NAME);
	if (IS_ERR(drvdata)) {
		_DBG(RPC_TRACE("device_create_drvdata failed\n"));
		unregister_chrdev(major, BCM_KERNEL_RPC_NAME);
		class_destroy(sModule.mDriverClass);
		return PTR_ERR(drvdata);
	}

	log_proc_init();

	RpcDbgInit();

	DEFINE_RPC_LOCK;

	_DBG(RPC_TRACE("exit rpcipc_ModuleInit()\n"));
	return 0;
}

/****************************************************************************/
/**
 *  Called by Linux I/O system to exit module.
 *  @return int     0 if success, -1 if error
 *  @note
 *      API is defined by module_exit macro
 **/
static void __exit rpcipc_ModuleExit(void)
{
	_DBG(RPC_TRACE("rpcipc_ModuleExit()\n"));

	log_proc_cleanup();

	RpcServerCleanup();

	device_destroy(sModule.mDriverClass, MKDEV(major, 0));
	class_destroy(sModule.mDriverClass);
	unregister_chrdev(major, BCM_KERNEL_RPC_NAME);

}

/**
 *  export module init and export functions
 **/
module_init(rpcipc_ModuleInit);
module_exit(rpcipc_ModuleExit);
