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

#ifndef __MQUEUE_H
#define __MQUEUE_H

struct tag_MsgQueueHandle_t;
#define MAX_NM_LEN 64

typedef int (*MsgQueueThreadFn_t) (struct tag_MsgQueueHandle_t *mHandle,
				   void *data);

typedef struct tag_MsgQueueHandle_t {
	struct list_head mList;
	spinlock_t mLock;
	wait_queue_head_t mWaitQ;
	int mAvailData;
	struct task_struct *mThread;
	MsgQueueThreadFn_t mFn;
	int valid;
	char name[MAX_NM_LEN+1];
#if defined(CONFIG_HAS_WAKELOCK) && defined(MQUEUE_RPC_WAKELOCK)
	struct wake_lock mq_wake_lock;
#endif
} MsgQueueHandle_t;

typedef struct {
	struct list_head mList;
	void *data;
} MsgQueueElement_t;

int MsgQueueInit(MsgQueueHandle_t *mHandle, MsgQueueThreadFn_t fn, char *name,
		 unsigned int optionType, void *optionData, char* wk_name);
int MsgQueueAdd(MsgQueueHandle_t *mHandle, void *data);
int MsgQueueRemove(MsgQueueHandle_t *mHandle, void **outData);
int MsgQueueDeInit(MsgQueueHandle_t *mhandle);
int MsgQueueIsEmpty(MsgQueueHandle_t *mHandle);
int MsgQueueDebugList(MsgQueueHandle_t *mHandle, RpcOutputContext_t *c);
void *MsgQueueGet(MsgQueueHandle_t *mHandle);
int MsgQueueCount(MsgQueueHandle_t *mHandle);
#endif
