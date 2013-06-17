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

#ifndef _BCM_RPC_DEBUG_H
#define _BCM_RPC_DEBUG_H

#define DBG_ERROR   0x01
#define DBG_INFO    0x02
#define DBG_TRACE   0x04
#define DBG_TRACE2  0x08
#define DBG_DATA    0x10
#define DBG_DATA2   0x20

/*#define DBG_DEFAULT_LEVEL (DBG_ERROR)*/
/*#define DBG_DEFAULT_LEVEL (DBG_ERROR|DBG_INFO|DBG_TRACE|DBG_TRACE2)*/
#define DBG_DEFAULT_LEVEL (DBG_ERROR|DBG_INFO)

/*static int logLevel = DBG_DEFAULT_LEVEL;*/

#define RPC_DEBUG(level, fmt, args...) printk("%s:: " fmt, __FUNCTION__, ##args)


extern void RpcDbgLogGenInfo(char *logStr, UInt8 cid,
				UInt32 pktHandle, UInt32 cmd1, UInt32 cmd2);

#define HISTORY_RPC_LOG(logStr , cid, pktHandle, cmd1, cmd2) \
	do {\
		RpcDbgLogGenInfo(logStr, cid, pktHandle, cmd1, cmd2);\
	} while (0)

#define HISTORY_RPC_LOG_CID(logStr, cid) \
		 HISTORY_RPC_LOG(logStr, cid, 0, 0,  0)
#define HISTORY_RPC_LOG_PKT(logStr, cid, pktHandle) \
		HISTORY_RPC_LOG(logStr , cid, pktHandle,  0, 0)


#define PKT_STATE_NA                     0
#define PKT_STATE_NEW                    1
#define PKT_STATE_RPC_POST               2
#define PKT_STATE_SYSRPC_POST            3
#define PKT_STATE_SYSRPC_PROCESS         4
#define PKT_STATE_RPC_PROCESS            5
#define PKT_STATE_PKT_FREE               6

#define PKT_STATE_CID_DISPATCH           7
#define PKT_STATE_CID_FETCH              8
#define PKT_STATE_CID_FREE               9


void RpcDbgUpdatePktStateEx(UInt32 pktHandle,
				UInt32 pktstatus, UInt8 cid,
				UInt32 cidPktStatus, UInt8 context1,
				UInt16 context2, UInt32 itype);

#define RpcDbgUpdatePktState(a, b) \
		RpcDbgUpdatePktStateEx(a, b, 0, 0, 0, 0, 0xFF)

void RpcDbgInit(void);

#define MAX_BUF_LEN 255

typedef struct {
	int type; /* 0 is printk, 1 is seq_file, 2 is bcm log, 3 is crash dump*/
	const Boolean protected;
	struct seq_file *seq;
	char buffer[MAX_BUF_LEN];
/*	int len;*/
} RpcOutputContext_t;

int RpcDbgDumpHdr(RpcOutputContext_t *c);
int RpcDbgDumpPktState(RpcOutputContext_t *c,  int *offset, int maxlimit);
int RbcDbgDumpGenInfo(RpcOutputContext_t *c,  int *offset, int maxlimit);
int RpcDbgDumpStr(RpcOutputContext_t *c, char *fmt, ...);
int RpcDbgDumpHistoryLogging(int type, int level);

#if defined(CONFIG_HAS_WAKELOCK) && defined(ENABLE_RPC_WAKELOCK)
void RpcDbgDumpWakeLockStats(RpcOutputContext_t *c);
#else
static inline void RpcDbgDumpWakeLockStats(RpcOutputContext_t *c) {};
#endif

void RpcDumpTaskCallStack(RpcOutputContext_t *c, struct task_struct *t);
int RpcDbgDumpKthread(RpcOutputContext_t *c, int option);
#endif /* _BCM_RPC_DEBUG_H */
