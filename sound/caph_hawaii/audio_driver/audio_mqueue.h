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

#ifndef __AUDIO_MQUEUE_H
#define __AUDIO_MQUEUE_H

struct tag_Audio_MsgQueueHandle_t;
#define MAX_NM_LEN 64

typedef int (*Audio_MsgQueueThreadFn_t) (
		struct tag_Audio_MsgQueueHandle_t *mHandle,
				   void *data);

struct tag_Audio_MsgQueueHandle_t {
	struct list_head mList;
	spinlock_t mLock;
	wait_queue_head_t mWaitQ;
	int mAvailData;
	struct task_struct *mThread;
	Audio_MsgQueueThreadFn_t mFn;
	int valid;
	char name[MAX_NM_LEN+1];
#if defined(CONFIG_HAS_WAKELOCK) && defined(MQUEUE_RPC_WAKELOCK)
	struct wake_lock mq_wake_lock;
#endif
};
#define Audio_MsgQueueHandle_t struct tag_Audio_MsgQueueHandle_t

struct _Audio_MsgQueueElement_t {
	struct list_head mList;
	void *data;
};
#define Audio_MsgQueueElement_t struct _Audio_MsgQueueElement_t

int Audio_MsgQueueInit(Audio_MsgQueueHandle_t *mHandle,
		Audio_MsgQueueThreadFn_t fn, char *name,
		 unsigned int optionType, void *optionData, char* wk_name);
int Audio_MsgQueueAdd(Audio_MsgQueueHandle_t *mHandle, void *data);
int Audio_MsgQueueRemove(Audio_MsgQueueHandle_t *mHandle, void **outData);
int Audio_MsgQueueDeInit(Audio_MsgQueueHandle_t *mhandle);
int Audio_MsgQueueIsEmpty(Audio_MsgQueueHandle_t *mHandle);
#if 0
int Audio_MsgQueueDebugList(Audio_MsgQueueHandle_t *mHandle,
		RpcOutputContext_t *c);
#endif
void *Audio_MsgQueueGet(Audio_MsgQueueHandle_t *mHandle);
#if 0
int Audio_MsgQueueCount(Audio_MsgQueueHandle_t *mHandle);
#endif
#endif
