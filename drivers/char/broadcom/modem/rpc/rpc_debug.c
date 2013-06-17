/****************************************************************************
*
*   Copyright (c) 2007-2008 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license
*   agreement governing use of this software, this software is licensed to you
*   under the terms of the GNU General Public License version 2, available
*   at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
*   Notwithstanding the above, under no circumstances may you combine this
*   software in any way with any other Broadcom software provided under a license
*   other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/
/**
*
*   @file   rpc_debug.c
*
*   @brief  This file builds lookup table for client registered messages.
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
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <asm/system.h>
#include <linux/kthread.h>
#include <linux/proc_fs.h>	/* Necessary because we use proc fs */
#include <linux/seq_file.h>	/* for seq_file */

#include "mobcom_types.h"
#include "resultcode.h"
#include "taskmsgs.h"
#include "consts.h"

#include "xdr_porting_layer.h"
#include "xdr.h"
#include "ipcproperties.h"
#include "rpc_global.h"
#include "rpc_ipc.h"
#include "rpc_ipc_config.h"
#include "rpc_debug.h"
#include "mqueue.h"

#define MAX_CID 10

typedef struct {
	UInt8 cid;
	UInt32 status;
	struct timeval ts;
} RpcDbgPktCidStatus_t;

typedef struct {
	UInt32 pktHandle;
	UInt32 status;
	UInt8 context1;
	UInt8 itype;
	UInt16 context2;
	struct timeval ts;
	RpcDbgPktCidStatus_t cidList[MAX_CID];
} RpcDbgPktStatus_t;

#define MAX_DATA_STR 15
typedef struct {
	UInt8 type;
	UInt8 cid;
	UInt32 count;
	UInt32 cmd1;
	UInt32 cmd2;
	UInt32 pktHandle;
	struct timeval ts;
	char data[MAX_DATA_STR + 1];
} RpcDbgLogInfo_t;

#define RPC_DBG_GEN_MAX  1024
#define MAX_RPC_PACKETS (CFG_RPC_CMD_MAX_PACKETS + CFG_RPC_CMD_MAX_PACKETS2 + CFG_RPC_CMD_MAX_PACKETS3 + CFG_RPC_SERIALDATA_MAX_PACKETS + CFG_RPC_SERIALDATA_MAX_PACKETS2 + CFG_RPC_CSDDATA_MAX_PACKETS)

RpcDbgLogInfo_t *gRpcDbgGenInfoList;
RpcDbgPktStatus_t *gRpcDbgPktStatusList;

extern UInt32 recvRpcPkts;
extern UInt32 freeRpcPkts;
extern MsgQueueHandle_t rpcMQhandle;
extern MsgQueueHandle_t sysRpcMQhandle;

atomic_t gRpcDbgPktGenCurIndex = ATOMIC_INIT(-1);
atomic_t gRpcGlobalCount = ATOMIC_INIT(0);
/*atomic_t gRpcTotalRcvPkts = ATOMIC_INIT(0);*/
/*atomic_t gRpcTotalFreePkts = ATOMIC_INIT(0);*/
atomic_t gLastGoodIndex = ATOMIC_INIT(0);

void RpcDbgLogGenInfo(char *logStr, UInt8 cid, UInt32 pktHandle, UInt32 cmd1,
		      UInt32 cmd2)
{
	int curIndex = 0;

	if (!gRpcDbgGenInfoList)
		return;

	curIndex = atomic_add_return(1, &gRpcDbgPktGenCurIndex);

	if (curIndex >= RPC_DBG_GEN_MAX) {
		curIndex = 0;
		atomic_set(&gRpcDbgPktGenCurIndex, 0);
	}

	gRpcDbgGenInfoList[curIndex].type = 1;
	gRpcDbgGenInfoList[curIndex].count =
	atomic_add_return(1, &gRpcGlobalCount);
	gRpcDbgGenInfoList[curIndex].cid = cid;
	gRpcDbgGenInfoList[curIndex].cmd1 = cmd1;
	gRpcDbgGenInfoList[curIndex].cmd2 = cmd2;
	gRpcDbgGenInfoList[curIndex].pktHandle = pktHandle;
	do_gettimeofday(&(gRpcDbgGenInfoList[curIndex].ts));
	strncpy(gRpcDbgGenInfoList[curIndex].data, logStr, MAX_DATA_STR);

	/*The size is (MAX_DATA_STR + 1) */
	gRpcDbgGenInfoList[curIndex].data[MAX_DATA_STR] = '\0';
}

void RpcDbgUpdatePktStateEx(UInt32 pktHandle, UInt32 pktstatus, UInt8 cid,
			    UInt32 cidPktStatus, UInt8 context1,
			    UInt16 context2, UInt32 itype)
{
	int i, k;

	if (!gRpcDbgPktStatusList)
		return;

	/* cache read */
	i = atomic_read(&gLastGoodIndex);

	/* no hit? */
	if (gRpcDbgPktStatusList[i].pktHandle != pktHandle) {
		/* Look for existing */
		for (i = 0; i < MAX_RPC_PACKETS; i++) {
			if (gRpcDbgPktStatusList[i].pktHandle == pktHandle)
				break;
		}

		/* Look for unused */
		if (i == MAX_RPC_PACKETS) {
			for (i = 0; i < MAX_RPC_PACKETS; i++) {
				if (gRpcDbgPktStatusList[i].pktHandle == 0) {
					gRpcDbgPktStatusList[i].pktHandle =
					    pktHandle;
					break;
				}
			}
		}

	}
	/* Set Data */
	if (i < MAX_RPC_PACKETS) {
		if (gRpcDbgPktStatusList[i].pktHandle == pktHandle) {
			do_gettimeofday(&(gRpcDbgPktStatusList[i].ts));

			if (pktstatus != PKT_STATE_NA)
				gRpcDbgPktStatusList[i].status = pktstatus;

			if (cid > 0 && cidPktStatus != PKT_STATE_NA) {
				for (k = 0; k < MAX_CID; k++) {
					if (gRpcDbgPktStatusList[i].cidList[k].
					    cid == cid)
						break;
				}

				if (k == MAX_CID) {
					for (k = 0; k < MAX_CID; k++) {
						if (gRpcDbgPktStatusList[i].
						    cidList[k].cid == 0) {
							gRpcDbgPktStatusList[i].
							    cidList[k].cid =
							    cid;
							break;
						}
					}
				}

				if (k < MAX_CID) {
					gRpcDbgPktStatusList[i].cidList[k].
					    status = cidPktStatus;
					do_gettimeofday(&(gRpcDbgPktStatusList
							[i].cidList[k].ts));
				}
			}

			if (context1 || pktstatus == PKT_STATE_NEW)
				gRpcDbgPktStatusList[i].context1 = context1;
			if (context2 || pktstatus == PKT_STATE_NEW)
				gRpcDbgPktStatusList[i].context2 = context2;
			if (itype != 0xFF)
				gRpcDbgPktStatusList[i].itype = itype;
		}

		atomic_set(&gLastGoodIndex, i);
	}

}

char *GetStatusStr(UInt32 status)
{
	if (status == PKT_STATE_NA)
		return "N/A";
	else if (status == PKT_STATE_NEW)
		return "new";
	else if (status == PKT_STATE_RPC_POST)
		return "rpc post";
	else if (status == PKT_STATE_SYSRPC_POST)
		return "sysrpc post";
	else if (status == PKT_STATE_SYSRPC_PROCESS)
		return "sysrpc fetch";
	else if (status == PKT_STATE_RPC_PROCESS)
		return "rpc fetch";
	else if (status == PKT_STATE_PKT_FREE)
		return "free";
	else if (status == PKT_STATE_CID_DISPATCH)
		return "cid dispatch";
	else if (status == PKT_STATE_CID_FETCH)
		return "cid fetch";
	else if (status == PKT_STATE_CID_FREE)
		return "cid free";

	return "";
}

int RbcDbgDumpGenInfo(RpcOutputContext_t *c, int *offset, int maxlimit)
{
	int i, ret = 0, limit = 0;

	if (!offset || *offset >= MAX_RPC_PACKETS || *offset < 0 || !gRpcDbgGenInfoList)
		return 0;

	if (*offset == 0) {
		RpcDbgDumpStr(c,
		      "\n************** RPC GEN DUMP BEGIN ***********\n");
	}

	for (i = (*offset); i < RPC_DBG_GEN_MAX; i++) {
		*offset = i;

		if (maxlimit > 0 && (++limit > maxlimit))
			return -1;

		if (gRpcDbgGenInfoList[i].type == 0)
			break;

		ret = RpcDbgDumpStr(c, "[%d] TS:%u:%u %s cmd1:%d  \
					cmd2:%d pkt=%d\n",
				    (unsigned int)gRpcDbgGenInfoList
					[i].count,
				    (unsigned int)gRpcDbgGenInfoList
					[i].ts.tv_sec,
				    (unsigned int)gRpcDbgGenInfoList
					[i].ts.tv_usec,
				    gRpcDbgGenInfoList[i].data,
				    (int)gRpcDbgGenInfoList[i].cmd1,
				    (int)gRpcDbgGenInfoList[i].cmd2,
				    (int)gRpcDbgGenInfoList[i].pktHandle);

		if (ret != 0)
			return ret;
	}

	RpcDbgDumpStr(c, "\n************** RPC GEN DUMP END ***********\n");

	return 0;
}

int RpcDbgDumpPktState(RpcOutputContext_t *c, int *offset, int maxlimit)
{
	int i, k, ret = 0, limit = 0;

	if (!offset || *offset >= MAX_RPC_PACKETS || *offset < 0 || !gRpcDbgPktStatusList)
		return 0;

	if (*offset == 0) {
		RpcDbgDumpStr(c,
		      "\n************** RPC PKT State Begin ***********\n");
	}

	for (i = (*offset); i < MAX_RPC_PACKETS; i++) {
		*offset = i;

		if (maxlimit > 0 && (++limit > maxlimit))
			return -1;

		if (gRpcDbgPktStatusList[i].pktHandle != 0) {

			ret =
			    RpcDbgDumpStr(c,
					"pkt:%d TS:%u:%u status:%s itype:%d \
					c1=%d c2=0x%x\n",
					  (unsigned int)
					  gRpcDbgPktStatusList[i].pktHandle,
					  (unsigned int)
					  gRpcDbgPktStatusList[i].ts.tv_sec,
					  (unsigned int)
					  gRpcDbgPktStatusList[i].ts.tv_usec,
					  GetStatusStr(gRpcDbgPktStatusList[i].
						       status),
					  (int)gRpcDbgPktStatusList[i].itype,
					  (int)gRpcDbgPktStatusList[i].context1,
					  (int)gRpcDbgPktStatusList[i].context2);

			if (ret != 0)
				return ret;

			for (k = 0; k < MAX_CID; k++) {
				if (gRpcDbgPktStatusList[i].cidList[k].status !=
				    0) {
					ret =
					    RpcDbgDumpStr(c,
							  "\tcid:%d TS:%u:%u status:%s\n",
							  gRpcDbgPktStatusList
							  [i].cidList[k].cid,
							  (unsigned int)
							  gRpcDbgPktStatusList
							  [i].cidList[k].ts
								.tv_sec,
							  (unsigned int)
							  gRpcDbgPktStatusList
							  [i].cidList[k].ts
								.tv_usec,
							  GetStatusStr
							  (gRpcDbgPktStatusList
							   [i].cidList[k].
							   status));

					if (ret != 0)
						return ret;
				}
			}
		} else
			break;
	}

	RpcDbgDumpStr(c, "\n************** RPC PKT State End ***********\n");

	return 0;
}

int RpcDbgDumpKthread(RpcOutputContext_t *c, int option)
{
	if (option == 0 || option == 2)
		MsgQueueDebugList(&rpcMQhandle, c);
	if (option == 1 || option == 2)
		MsgQueueDebugList(&sysRpcMQhandle, c);
	return 0;
}

int RpcDbgDumpHdr(RpcOutputContext_t *c)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	RpcDbgDumpStr(c, "\nRcvCount=%d FreeCount=%d ts=%u:%u\n",
	(int)recvRpcPkts, (int)freeRpcPkts, tv.tv_sec, tv.tv_usec);
	return 0;
}

void RpcDbgInit(void)
{
	gRpcDbgGenInfoList = kmalloc((RPC_DBG_GEN_MAX + 1) * sizeof(RpcDbgLogInfo_t), GFP_ATOMIC);
	if (gRpcDbgGenInfoList)
		memset(gRpcDbgGenInfoList, 0, (RPC_DBG_GEN_MAX + 1) * sizeof(*gRpcDbgGenInfoList));

	gRpcDbgPktStatusList = kmalloc((MAX_RPC_PACKETS + 1) * sizeof(RpcDbgPktStatus_t), GFP_ATOMIC);
	if (gRpcDbgPktStatusList)
		memset(gRpcDbgPktStatusList, 0, (MAX_RPC_PACKETS + 1) * sizeof(*gRpcDbgPktStatusList));
}

int RpcDbgDumpStr(RpcOutputContext_t *c, char *fmt, ...)
{
	int n = 0;
	va_list ap;

	if (!c)
		return -1;

	va_start(ap, fmt);
	vsnprintf(c->buffer, MAX_BUF_LEN, fmt, ap);
	va_end(ap);

	if (c->type == 0)
		printk(c->buffer);
	else if (c->type == 1 && c->seq)
		n = seq_printf(c->seq, c->buffer);
	else if (c->type == 2)
		BCMLOG_LogCPCrashDumpString(c->buffer);

	return n;
}

int RpcDbgDumpRawStr(RpcOutputContext_t *c, char *str)
{
	int n = 0;

	if (!c)
		return -1;

	if (c->type == 0)
		printk(str);
	else if (c->type == 1 && c->seq)
		n = seq_printf(c->seq, str);
	else if (c->type == 2)
		BCMLOG_LogCPCrashDumpString(str);

	return n;
}

#define MAX_BUF_STR_LEN 1024
static char sCallStackBuffer[MAX_BUF_STR_LEN+1] = {0};
static pid_t sTid = 0;

/*
Just copy the buffer and DO NOT CALL any kernel system calls OR acquire spin locks
*/
int RpcDumpPrintkCb(const char *str)
{
	if (sTid == current->pid) {
		int remLen;
		remLen = MAX_BUF_STR_LEN - strlen(sCallStackBuffer);
		if (remLen > 0)
			strncat(sCallStackBuffer, str, remLen);
	}
	return 0;
}

static void dumpCallStackStr(RpcOutputContext_t *c, char* inStr)
{
	char *bstr = inStr;
	char *tstr = inStr;

	if (c->type != 2) {
		RpcDbgDumpRawStr(c, bstr);
		RpcDbgDumpRawStr(c, "\n");
		return;
	}

	while (*tstr != '\0') {
		if (*tstr == '\n') {
			*tstr = '\0';
			RpcDbgDumpRawStr(c, bstr);
			bstr = tstr+1;
		}
		tstr++;
	}
	RpcDbgDumpRawStr(c, "\n");
}

void RpcDumpTaskCallStack(RpcOutputContext_t *c, struct task_struct *t)
{
	if (c->type != 0) {   /* out to proc or crash dump */
		memset(sCallStackBuffer, 0, (MAX_BUF_STR_LEN+1));
		sTid = current->pid;
		BCMLOG_RegisterPrintkRedirectCbk(1, RpcDumpPrintkCb);

		sched_show_task(t);

		BCMLOG_RegisterPrintkRedirectCbk(0, NULL);
		dumpCallStackStr(c, sCallStackBuffer);
	} else
		sched_show_task(t);
}





