/****************************************************************************
*
*	Copyright (c) 2004 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license
*   agreement governing use of this software, this software is licensed to you
*   under the terms of the GNU General Public License version 2, available
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
*   Notwithstanding the above, under no circumstances may you combine this
*   software in any way with any other Broadcom software provided under a
*   license other than the GPL, without Broadcom's express prior written
*   consent.
*
****************************************************************************/

/**
*
*   @file   rpc_sync.c
*
*   @brief  This file implements the SYNC RPC functions.
*
****************************************************************************/

/*==============================================================================
*                              include block
*============================================================================*/
#ifdef WIN32
#define _WINSOCKAPI_
#endif

#include "mobcom_types.h"
#include "rpc_global.h"

#include "resultcode.h"
#include "taskmsgs.h"
#include "ipcinterface.h"
#include "ipcproperties.h"

#include "rpc_ipc.h"
#include "xdr_porting_layer.h"
#include "xdr.h"
#include "rpc_api.h"
#include "rpc_internal_api.h"
#include "rpc_sync_api.h"
#ifndef LINUX_RPC_KERNEL
#include "ostask.h"
#include "osheap.h"
#include "ossemaphore.h"
#include "xassert.h"
#else
#include <plat/types.h>
#include <plat/osabstract/ostypes.h>
#include <plat/osabstract/ossemaphore.h>
#include <linux/delay.h>
#include <linux/syscalls.h>
#define OSHEAP_Alloc(x) kmalloc(x, GFP_KERNEL)
#define OSHEAP_Delete(x) kfree(x)
#define OSTASK_GetCurrentTask() sys_gettid()
#define OSTASK_IsValidTask(x)  (NULL != find_task_by_vpid((pid_t) (x)))
#define OSTASK_Sleep(x)  msleep(x)
#endif

/*====================================================================
* Local Definitions
*===================================================================*/
/* number of concurrent tasks to support */
#define  MAX_TASKS_NUM						20

#ifdef FPGA_VERSION
/* FPGA very slow but should NOT be slower than 60 secs.
   Need further investigation. */
#define  RPC_SEMAPHORE_TIMEOUT			(TICKS_FOREVER)
#else
/* 60 [second] */
#define  RPC_SEMAPHORE_TIMEOUT			(TICKS_ONE_SECOND * 60)
#endif

#define REMOTE_AP_NOT_RESPONDING (semaStatus == OSSTATUS_SUCCESS)
#define REMOTE_CP_NOT_RESPONDING (semaStatus == OSSTATUS_SUCCESS)

/*---------------------------------------------------------------------
* Local variables
*--------------------------------------------------------------------*/
static UInt32 sCurrTID = 1;

/* semaphore to manage access to array of task/req structs */
static Semaphore_t semaTaskReq = NULL;

/**
	always keep one client registered with RPC so we
	can get responses to synchronous calls without having
	a rpc client registered
**/
static UInt8 sClientId = INVALID_CLIENT_ID;

/* number of ticks to sleep while waiting for RPC to initialize */
#define		RPC_INIT_SLEEP_TICKS	50

typedef enum {
	RPC_SYNC_STATE_WAITING_FOR_ACK,	/* waiting for ack */
	RPC_SYNC_STATE_WAITING_FOR_RSP,	/* waiting for response */
	RPC_SYNC_STATE_DONE			/* done with transaction */

} RPC_SyncState_t;

typedef struct {
	UInt32 sig;
	UInt32 val;
} RPC_SyncContext_t;

/* struct use to map request data to the task that originated the request */
typedef struct {
	/* task ID of originating task */
	Task_t task;

	/* transaction ID of request */
	UInt32 tid;

	/* state of transaction */
	RPC_SyncState_t state;

	/* semaphore used to wait/signal request ack */
	Semaphore_t ackSema;

	/* ack result from RPC */
	RPC_ACK_Result_t ack;

	/* pending result from ack */
	Boolean isResultPending;

	/* semaphore used to wait/signal request response */
	Semaphore_t rspSema;

	/* result code from response */
	Result_t result;

	/* message type of response */
	MsgType_t msgType;

	/* pointer to data buffer from response (may be null) */
	void *data;
	/* size of response data buffer */
	Int32 dataLen;

	/* size of actual data from response copied into data buffer */
	UInt32 rspSize;
	/**
	  if TRUE, release response buffer immediately in response callback;
	  if FALSE, calling api must call RPC_SyncReleaseResponseBuffer() to release buffer memory
	 */
	Boolean releaseC2Buf;

	/* handle to response data buffer */
	void *dataBufHandle;
} TaskRequestMap_t;

static TaskRequestMap_t sTaskRequestMap[MAX_TASKS_NUM];

/*----------------------------------------------------------------------------*/

/**
 internal helper functions
 get task/response map struct for current task
**/
static TaskRequestMap_t *GetMapForCurrentTask(void);

/* get unused task/response map struct to be used with current task */
static TaskRequestMap_t *GetNewMapForCurrentTask(void);

/* get task/response map struct with the specified transaction ID */
static TaskRequestMap_t *GetMapForTID(UInt32 tid);

void RPC_SyncHandleResponse(RPC_Msg_t *pMsg,
			    ResultDataBufHandle_t dataBufHandle,
			    UInt32 userContextData);

void RPC_SyncHandleAck(UInt32 tid, UInt8 clientID, RPC_ACK_Result_t ackResult,
		       UInt32 ackData);
static void RPC_SyncNotification(
	struct RpcNotificationEvent_t event, UInt8 clientID);

extern UInt32 RPC_GetUserData(UInt8 clientIndex);

/******************************************************************************/
/**
	Initialize the RPC Sync Framework.
	@param		timeout (in) Number of ticks to wait for emulator initialization
	@return		\n RESULT_OK for success,
			\n RESULT_ERROR for failure,
	@note
	This function will block until emulator is initialized or timeout duration has expired.
**/
Result_t RPC_SyncInitialize()
{
	Result_t result = RESULT_OK;

	/* reset task map storage */
	memset(&sTaskRequestMap, 0, sizeof(sTaskRequestMap));

	/* create semaphore to manage access to array of task/req structs */
	semaTaskReq = OSSEMAPHORE_Create(1, OSSUSPEND_PRIORITY);
	assert(semaTaskReq);

	return result;
}

typedef struct {
	RPC_InitParams_t clientParams;
	RPC_InitParams_t SyncRpcParams;
	RPC_SyncInitParams_t syncInitParams;
	UInt8 clientID;
} RPC_SyncParams_t;

/******************************************************************************/
/**
	Register new RPC client.
	@return		Client ID
**/
RPC_Handle_t RPC_SyncRegisterClient(RPC_InitParams_t *initParams,
				    RPC_SyncInitParams_t *syncInitParams)
{
	RPC_SyncParams_t *internalParam =
	    (RPC_SyncParams_t *) OSHEAP_Alloc(sizeof(RPC_SyncParams_t));

	if (!internalParam) {
		panic("RPC_SyncRegisterClient: OSHEAP_Alloc failed");
		return 0;
	}
	/* **FIXME** MAG - need special CP reset handler in here? Release all
	   semaphores for pending requests?
	*/
	internalParam->clientParams = *initParams;
	internalParam->SyncRpcParams = *initParams;

	internalParam->SyncRpcParams.respCb = RPC_SyncHandleResponse;
	internalParam->SyncRpcParams.ackCb = RPC_SyncHandleAck;
	internalParam->SyncRpcParams.userData = (UInt32) internalParam;
	internalParam->SyncRpcParams.rpcNtfFn = RPC_SyncNotification;

	internalParam->syncInitParams = *syncInitParams;

	return RPC_SYS_RegisterClient(&internalParam->SyncRpcParams);
}

/******************************************************************************/
/**
	Deregister Async RPC client.
	@param		inClientId (in) Client id of RPC client.
**/
void RPC_SyncDeregisterClient(UInt8 inClientId)
{
	/* get user data */

	/* delete buffer */

	/* set user data */
}

TaskRequestMap_t *RPC_SyncInitTaskMap(void)
{
	TaskRequestMap_t *taskMap = GetMapForCurrentTask();

	if (!taskMap) {
		taskMap = GetNewMapForCurrentTask();
		assert(taskMap);
		/* fill in struct (task field is already filled in by GetNewMapForCurrentTask() call) */
		taskMap->ackSema = OSSEMAPHORE_Create(1, OSSUSPEND_PRIORITY);
		assert(taskMap->ackSema);
		/* start in signalled state */
		OSSEMAPHORE_Obtain(taskMap->ackSema, TICKS_FOREVER);

		taskMap->rspSema = OSSEMAPHORE_Create(1, OSSUSPEND_PRIORITY);
		assert(taskMap->rspSema);
		/* start in signalled state */
		OSSEMAPHORE_Obtain(taskMap->rspSema, TICKS_FOREVER);

	}

	taskMap->tid = 0;
	taskMap->ack = ACK_FAILED;
	taskMap->state = RPC_SYNC_STATE_WAITING_FOR_ACK;
	taskMap->isResultPending = FALSE;
	taskMap->result = RESULT_ERROR;
	taskMap->data = NULL;
	taskMap->dataLen = 0;
	taskMap->rspSize = 0;
	taskMap->releaseC2Buf = TRUE;
	taskMap->dataBufHandle = NULL;

	return taskMap;
}

/******************************************************************************/
/**
	Prepare to issue a RPC request.
	@param		data (in) pointer to buffer to store response
	@param		size (in) size of response buffer
	@return		transaction ID for request

	@note
	If size is -1, this indicates that the response size is variable and buffer should
	be allocated by the RPC callback handler. The buffer will be freed by the caller
	of RPC_SyncWaitForResponse

**/
UInt32 RPC_SyncCreateTID(void *data, Int32 size)
{
	UInt32 tid;
	TaskRequestMap_t *taskMap = NULL;

	/*Protect the tid with semaphore */
	OSSEMAPHORE_Obtain(semaTaskReq, TICKS_FOREVER);
	tid = sCurrTID;

	if (0 == ++sCurrTID)
		sCurrTID = 1;

	OSSEMAPHORE_Release(semaTaskReq);

	taskMap = RPC_SyncInitTaskMap();

	taskMap->tid = tid;
	taskMap->data = data;
	taskMap->dataLen = size;

	return tid;
}

UInt32 RPC_SyncSetTID(UInt32 tid, void *data, Int32 size)
{
	TaskRequestMap_t *taskMap = NULL;

	taskMap = RPC_SyncInitTaskMap();
	taskMap->tid = tid;
	taskMap->data = data;
	taskMap->dataLen = size;

	return tid;
}

/******************************************************************************/
/**
	Retrieve the response from a RPC function call. Note that this includes the ack result as well.
	@param		tid (in) Transaction id for request.
	@param		cid (in) Client id for request
	@param		ack (out) Ack result for request
	@param		msgType (out) Message type of response.
	@param		dataSize (out) Actual data size copied to response data buffer

	@return Result code of response.

**/
Result_t RPC_SyncWaitForResponse(UInt32 tid, UInt8 cid, RPC_ACK_Result_t *ack,
				 MsgType_t *msgType, UInt32 *dataSize)
{
	OSStatus_t semaStatus;
	Result_t result = RESULT_OK;
	TaskRequestMap_t *taskMap = GetMapForCurrentTask();
	assert(taskMap);

	/* wait to be signalled that request has been ack'd */
	semaStatus =
	    OSSEMAPHORE_Obtain(taskMap->ackSema, RPC_SEMAPHORE_TIMEOUT);
	/*
	 If you see this assert then likely remote processor does not free the buffers.
	 - Likely cause is CAPI2 Task on remote processor is stuck waiting on a Queue.
	 - or Watchdog issue where CAPI2 Task does not get chance to run.
	 - or Remote processor is completely DEAD
	*/
#ifdef FUSE_APPS_PROCESSOR
	xassert(REMOTE_CP_NOT_RESPONDING, semaStatus);
#else
	xassert(REMOTE_AP_NOT_RESPONDING, semaStatus);
#endif

	*ack = taskMap->ack;

	/* request ack'd by comm proc? */

	if (ACK_SUCCESS == taskMap->ack) {
		/* synchronous response or error response from async api? */
		if (!taskMap->isResultPending) {
			/*
			  yes, so wait to be signalled that response is ready
			*/
			taskMap->state = RPC_SYNC_STATE_WAITING_FOR_RSP;
			semaStatus =
			    OSSEMAPHORE_Obtain(taskMap->rspSema,
					       RPC_SEMAPHORE_TIMEOUT);
			/*
			  If you see this assert then likely remote processor does not free the buffers.
			  - Likely cause is CAPI2 Task on remote processor is stuck waiting on a Queue.
			  - or Watchdog issue where CAPI2 Task does not get chance to run.
			  - or Remote processor is completely DEAD
			*/
#ifdef FUSE_APPS_PROCESSOR
			xassert(REMOTE_CP_NOT_RESPONDING, semaStatus);
#else
			xassert(REMOTE_AP_NOT_RESPONDING, semaStatus);
#endif

			if (msgType)
				*msgType = taskMap->msgType;

			if (dataSize)
				*dataSize = taskMap->rspSize;

			result = taskMap->result;
		} else {

		}
		taskMap->state = RPC_SYNC_STATE_DONE;
	} else if (ACK_CRITICAL_ERROR == taskMap->ack) {
		/* CP reset */
		taskMap->state = RPC_SYNC_STATE_DONE;
		result = RESULT_ERROR;
	} else {
		xassert(FALSE, taskMap->ack);
	}

	_DBG_(RPC_TRACE
		("RPC_SyncWaitForResponse tid=%d cid=%d msg=%d ack=%d\r\n",
		(int)tid, cid, taskMap->msgType, taskMap->ack));
	_DBG_(RPC_TRACE
		("RPC_SyncWaitForResponse pend=%d sz=%d rs=%d task=%p \r\n",
		taskMap->isResultPending, (int)taskMap->rspSize,
		result, (void *) OSTASK_GetCurrentTask()));

	return result;
}

/******************************************************************************/
/**
	Retrieve the response from a RPC function call. Note that this includes the ack result as well.
	@param		tid (in) Transaction id for request.
	@param		cid (in) Client id for request
	@param		ack (out) Ack result for request
	@param		msgType (out) Message type of response.
	@param		dataSize (out) Actual data size copied to response data buffer
	@param		timeout (in) timer value

	@return Result code of response.

**/
Result_t RPC_SyncWaitForResponseTimer(UInt32 tid, UInt8 cid,
				      RPC_ACK_Result_t *ack,
				      MsgType_t *msgType, UInt32 *dataSize,
				      UInt32 timeout)
{
	OSStatus_t semaStatus;
	Result_t result = RESULT_OK;
	TaskRequestMap_t *taskMap = GetMapForCurrentTask();
	assert(taskMap);

	/* wait to be signalled that request has been ack'd */
	semaStatus = OSSEMAPHORE_Obtain(taskMap->ackSema, (Ticks_t) timeout);
	/*
	 If you see this assert then likely remote processor does not free the buffers.
	 - Likely cause is CAPI2 Task on remote processor is stuck waiting on a Queue.
	 - or Watchdog issue where CAPI2 Task does not get chance to run.
	 - or Remote processor is completely DEAD
	*/

	if (semaStatus == OSSTATUS_TIMEOUT)
		return RESULT_TIMER_EXPIRED;
	else if (semaStatus != OSSTATUS_SUCCESS)
		return RESULT_ERROR;

	*ack = taskMap->ack;

	/* request ack'd by comm proc? */
	if (ACK_SUCCESS == taskMap->ack) {
		/* synchronous response or error response from async api? */
		if (!taskMap->isResultPending) {
			/*
			 yes, so wait to be signalled that response is ready
			*/
			taskMap->state = RPC_SYNC_STATE_WAITING_FOR_RSP;
			semaStatus =
			    OSSEMAPHORE_Obtain(taskMap->rspSema,
					       (Ticks_t) timeout);
			/*
			 If you see this assert then likely remote processor does not free the buffers.
			 - Likely cause is CAPI2 Task on remote processor is stuck waiting on a Queue.
			 - or Watchdog issue where CAPI2 Task does not get chance to run.
			 - or Remote processor is completely DEAD
			*/
			if (semaStatus == OSSTATUS_TIMEOUT)
				return RESULT_TIMER_EXPIRED;
			else if (semaStatus != OSSTATUS_SUCCESS)
				return RESULT_ERROR;

			if (msgType)
				*msgType = taskMap->msgType;

			if (dataSize)
				*dataSize = taskMap->rspSize;

			result = taskMap->result;
		} else {

		}
		taskMap->state = RPC_SYNC_STATE_DONE;
	} else {
		taskMap->state = RPC_SYNC_STATE_DONE;
		return RESULT_ERROR;
	}

	_DBG_(RPC_TRACE(
		"RPC_SyncWaitForResponseTimer tid=%d cid=%d msg=%d ack=%d\r\n",
		(int)tid, cid, taskMap->msgType, taskMap->ack));
	_DBG_(RPC_TRACE(
		"RPC_SyncWaitForResponseTimer pend=%d sz=%d rs=%d task=0x%p\r\n",
		taskMap->isResultPending, (int)taskMap->rspSize,
		result, (void *) OSTASK_GetCurrentTask()));

	return result;
}

/******************************************************************************/
/**
	Handle the ack result from the RPC function call.
	@param		tid (in) Transaction id for request.
	@param		ack (in) Ack result for request

**/
void RPC_SyncHandleAck(UInt32 tid, UInt8 clientID, RPC_ACK_Result_t ack,
		       UInt32 ackData)
{
	OSStatus_t semaStatus;
	/* find the map struct corresponding to this transaction ID */
	TaskRequestMap_t *taskMap = NULL;
	Boolean isResultPending = (ackData) ? TRUE : FALSE;

	if (tid == 0) {
		_DBG_(RPC_TRACE
		      ("RPC_SyncHandleAck Ignored tid %d ack %d pending %d\r\n",
		       (int)tid, ack, isResultPending));
		return;
	}

	taskMap = GetMapForTID(tid);
	/*assert(taskMap); */

	if (taskMap) {
		/* record ack status */
		taskMap->ack = ack;
		taskMap->isResultPending = isResultPending;

		_DBG_(RPC_TRACE
			("RPC_SyncHandleAck tid %d ack %d pending %d\r\n",
			(int)tid, ack, isResultPending));

		/**
		    since there won't be anybody waiting on it (ie. it will
		    be dispatched using the client ID/msg type)
		**/
		if ((ACK_SUCCESS == ack) && isResultPending)
			taskMap->tid = 0;

		/* notify calling task of receipt of ack */
		semaStatus = OSSEMAPHORE_Release(taskMap->ackSema);
		if (semaStatus != OSSTATUS_SUCCESS) {
			_DBG_(RPC_TRACE
				("WARNING!!!! RPC_SyncHandleAck: \
				Semaphore release failed\r\n"));
		}
	} else {
		_DBG_(RPC_TRACE
		      ("WARNING!!!! RPC_SyncHandleAck No Task map found \
			tid %d ack %d pending %d\r\n",
		       (int)tid, ack, isResultPending));
	}
}

/******************************************************************************/
/**
	Handle the response from a RPC function call.
	@param		tid (in) Transaction id for request.
	@param		cid (in) Client id for request
	@param		msgType (in) Message type of response.
	@param		result (in) Result code of response.
	@param		dataPtr (in) Pointer to response data buffer
	@param		dataSize (in) size of response data buffer
	@param		dataBufHandle (in) handle to response data buffer

	@return		\n TRUE if response handled,
			\n FALSE if response not handled.

**/
void RPC_SyncHandleResponse(RPC_Msg_t *pMsg,
			    ResultDataBufHandle_t dataBufHandle,
			    UInt32 userContextData)
{
	RPC_SyncParams_t *internalParam;
	OSStatus_t semaStatus;
	UInt32 tid = pMsg->tid;
	UInt8 clientID = pMsg->clientID;
	MsgType_t msgType = pMsg->msgId;
	Result_t result = RESULT_OK;
	void *dataBuf = pMsg->dataBuf;
	UInt32 dataLength = pMsg->dataLen;

	TaskRequestMap_t *taskMap = NULL;
	/* default is to always release response buffer automatically */
	Boolean bReleaseResponseBuffer = TRUE;

	internalParam = (RPC_SyncParams_t *) (void *)userContextData;
	assert(internalParam != NULL);

	if (0 != tid)
		taskMap = GetMapForTID(tid);

	if (internalParam && internalParam->syncInitParams.copyCb)
		internalParam->syncInitParams.copyCb(msgType, dataBuf,
						     (taskMap) ? taskMap->
						     dataLen : 0,
						     (taskMap) ? taskMap->
						     data : NULL, &dataLength,
						     &result);

	_DBG_(RPC_TRACE(
	      "RPC_SyncHandleResponse tid=%d cid=%d msg=%d res=%d\r\n",
	       (int)tid, clientID, msgType, result));
	_DBG_(RPC_TRACE
	      ("RPC_SyncHandleResponse tlen=%d tmap=%d len=%ld \r\n",
	       (taskMap) ? (int)taskMap->dataLen : 0,
	       (taskMap) ? 1 : 0, dataLength));

	/* did we find a pending request with this transaction id? */
	if (taskMap) {
		taskMap->result = result;
		taskMap->tid = 0;
		bReleaseResponseBuffer = taskMap->releaseC2Buf;
		taskMap->msgType = msgType;
		taskMap->rspSize = dataLength;

		xassert(!dataBuf || !(taskMap->data)
			|| (taskMap->dataLen >= (Int32) dataLength), msgType);

		/* need to keep response buffer around for deep copy */
		if (!bReleaseResponseBuffer)
			taskMap->dataBufHandle = dataBufHandle;

		/* notify calling task of receipt of response */
		semaStatus = OSSEMAPHORE_Release(taskMap->rspSema);
		if (semaStatus != OSSTATUS_SUCCESS) {
			_DBG_(RPC_TRACE
			      ("WARNING!!!! RPC_SyncHandleResponse: Semaphore release failed\r\n"));
		}
	} else {	/* Dispatch unsolicited notifications to the client */
		if (RESULT_OK != result) {
			_DBG_(RPC_TRACE
			      ("WARNING!!!! RPC_SyncHandleResponse tid %d cid %d result=%d noTaskMap \r\n",
			       (int)tid, clientID, result));
		}

		internalParam = (RPC_SyncParams_t *) (void *)userContextData;
		assert(internalParam && internalParam->clientParams.respCb);
		internalParam->clientParams.respCb(pMsg, dataBufHandle,
						   internalParam->clientParams.
						   userData);

		return;		/* The buffer will be freed by the client */
	}

	if (bReleaseResponseBuffer) {
		/* release response buffer */
		RPC_SYSFreeResultDataBuffer(dataBufHandle);
	}
}

/******************************************************************************/
/**
	Lock the response buffer associated with the current task. After calling this api,
	response buffer will not be automatically released byRPC Sync Framework. Client apis
	must call RPC_SyncReleaseResponseBuffer() to release the response buffer

	Note: response buffer for a given call must be released before another rpc
		  call is made from the same task
**/
void RPC_SyncLockResponseBuffer(void)
{
	TaskRequestMap_t *taskMap = GetMapForCurrentTask();
	assert(taskMap);

	taskMap->releaseC2Buf = FALSE;
}

/******************************************************************************/
/**
	Release the response buffer associated with the current task.
**/
void RPC_SyncReleaseResponseBuffer(void)
{
	TaskRequestMap_t *taskMap = GetMapForCurrentTask();
	assert(taskMap);
	assert(taskMap->dataBufHandle);

	RPC_SYSFreeResultDataBuffer(taskMap->dataBufHandle);
	taskMap->dataBufHandle = NULL;
}

void *RPC_SyncAllocFromHeap(UInt32 size)
{
	return OSHEAP_Alloc(size);
}

void RPC_SyncReleaseToHeap(void *inBuf)
{
	if (inBuf)
		OSHEAP_Delete(inBuf);
}

UInt8 RPC_SyncGetClientId()
{
	return sClientId;
}

/*=============================================================================
*  Internal helper functions
*============================================================================*/

/* get task/response map struct for current task */
TaskRequestMap_t *GetMapForCurrentTask(void)
{
	TaskRequestMap_t *taskMap = (TaskRequestMap_t *) NULL;
	Task_t currTask = (Task_t) OSTASK_GetCurrentTask();
	UInt8 i;

	for (i = 0; i < MAX_TASKS_NUM; i++) {
		if (currTask == (sTaskRequestMap + i)->task) {
			taskMap = (sTaskRequestMap + i);
			break;
		}
	}

	return taskMap;
}

/* get unused task/response map struct to be used with current task */
TaskRequestMap_t *GetNewMapForCurrentTask()
{
	TaskRequestMap_t *taskMap = NULL;
	UInt8 i;

	OSSEMAPHORE_Obtain(semaTaskReq, TICKS_FOREVER);

	for (i = 0; i < MAX_TASKS_NUM; i++) {
		_DBG_(RPC_TRACE
		      ("GetNewMapForCurrentTask detail task %lx index = %d\r\n",
		       (UInt32) ((sTaskRequestMap + i)->task), i));

		if (NULL == (sTaskRequestMap + i)->task) {
			(sTaskRequestMap + i)->task =
			    (Task_t) OSTASK_GetCurrentTask();
			taskMap = (sTaskRequestMap + i);
			break;
		}
	}

	if (MAX_TASKS_NUM == i) {
		for (i = 0; i < MAX_TASKS_NUM; i++) {
			_DBG_(RPC_TRACE
				("GetNewMapForCurrentTask 2nd loop task %lx index = %d\r\n",
				(UInt32) ((sTaskRequestMap + i)->task), i));

			if (FALSE ==
			    OSTASK_IsValidTask((sTaskRequestMap + i)->task)) {
				(sTaskRequestMap + i)->task =
				    (Task_t) OSTASK_GetCurrentTask();
				taskMap = (sTaskRequestMap + i);
				break;
			}

		}
	}

	OSSEMAPHORE_Release(semaTaskReq);

	_DBG_(RPC_TRACE
	      ("GetNewMapForCurrentTask task %lx index= %d\r\n",
	       (UInt32) OSTASK_GetCurrentTask(), i));

	return taskMap;
}

/* get task/response map struct with the specified transaction ID */
TaskRequestMap_t *GetMapForTID(UInt32 tid)
{
	TaskRequestMap_t *taskMap = NULL;
	UInt8 i;

	for (i = 0; i < MAX_TASKS_NUM; i++) {
		if (tid == (sTaskRequestMap + i)->tid) {
			taskMap = (sTaskRequestMap + i);
			break;
		}
	}

	return taskMap;
}

/******************************************************************************/
/**
	Add ctx to tid.
	@param		val (in) context value to be added

**/
UInt32 RPC_SyncAddCbkToTid(UInt32 val)
{
	RPC_SyncContext_t *ctx;
	TaskRequestMap_t *taskMap = GetMapForCurrentTask();
	assert(taskMap);

	ctx = OSHEAP_Alloc(sizeof(RPC_SyncContext_t));
	assert(ctx != NULL);
	ctx->sig = 0xBABEFACE;
	ctx->val = val;

	_DBG_(RPC_TRACE
	      ("RPC_SyncAddCbkToTid oldTid=%d newTid=%ld TaskID=%p\r\n",
	       (int)taskMap->tid, (UInt32) ctx, taskMap->task));

	taskMap->tid = (UInt32) ctx;

	return (UInt32) ctx;
}

/******************************************************************************/
/**
	Get context from tid.
	@param		tid (in) tid

**/
UInt32 RPC_SyncGetCbkFromTid(UInt32 tid)
{
	UInt32 val = 0;
	RPC_SyncContext_t *ctx = (RPC_SyncContext_t *) tid;

	assert(ctx != NULL);

	_DBG_(RPC_TRACE
		("RPC_SyncGetCbkFromTid tid %d sig=%lx \r\n",
		(int)tid, ctx->sig));

	if (ctx->sig == 0xBABEFACE && ctx->val != 0) {
		val = ctx->val;
	} else {
		_DBG_(RPC_TRACE
			("RPC_SyncGetCbkFromTid ERROR tid %d sig=%lx \r\n",
			(int)tid,
			ctx->sig));
		xassert(0, tid);
	}
	return val;
}

/******************************************************************************/
/**
	Delete context from tid.
	@param		tid (in) tid

**/
void RPC_SyncDeleteCbkFromTid(UInt32 tid)
{
	RPC_SyncContext_t *ctx = (RPC_SyncContext_t *) tid;

	assert(ctx != NULL);

	_DBG_(RPC_TRACE
	      ("RPC_SyncDeleteCbkFromTid tid %d sig=%lx \r\n",
		(int)tid, ctx->sig));

	if (ctx->sig == 0xBABEFACE && ctx->val != 0) {
		ctx->sig = 0;
		ctx->val = 0;
		OSHEAP_Delete(ctx);
	} else {
		_DBG_(RPC_TRACE
			("RPC_SyncDeleteCbkFromTid ERROR tid %d sig=%lx \r\n",
			(int)tid,
			ctx->sig));
		xassert(0, tid);
	}
}

/* cp silent reset callback for sync rpc layer */
static void RPC_SyncNotification(
	struct RpcNotificationEvent_t event, UInt8 clientID)
{
	TaskRequestMap_t *taskMap = (TaskRequestMap_t *) NULL;
	UInt8 i;
	RPC_SyncParams_t *internalParam;
	UInt8 clientIndex;

	_DBG_(RPC_TRACE("RPC_SyncNotification event %d param %d",
			(int) event.event, (int) event.param));

	switch (event.event) {
	case RPC_CPRESET_EVT:
		for (i = 0; i < MAX_TASKS_NUM; i++) {
			taskMap = (sTaskRequestMap + i);

			if (taskMap != NULL &&
				taskMap->task != NULL) {
				_DBG_(RPC_TRACE(
	      "RPC_SyncNotification i=%d tid=%d msg=%d ack=%d\r\n",
				i, (int)taskMap->tid, (int)taskMap->msgType,
				taskMap->ack));
				_DBG_(RPC_TRACE(
	      "RPC_SyncNotification pend=%d sz=%d rs=%d task=0x%p\r\n",
				taskMap->isResultPending,
				(int)taskMap->rspSize,
				(int)taskMap->result,
				(void *)taskMap->task));

			if (event.param == RPC_CPRESET_START) {
				switch (taskMap->state) {
				case RPC_SYNC_STATE_WAITING_FOR_ACK:
					_DBG_(RPC_TRACE(
		"RPC_SyncNotification waiting for ack\n"));
					taskMap->ack = ACK_CRITICAL_ERROR;
					taskMap->state = RPC_SYNC_STATE_DONE;
					if (taskMap->ackSema)
						OSSEMAPHORE_Release(
							taskMap->ackSema);
					_DBG_(RPC_TRACE(
		"RPC_SyncNotification after sema rel\n"));
					break;
				case RPC_SYNC_STATE_WAITING_FOR_RSP:
					_DBG_(RPC_TRACE(
		"RPC_SyncNotification waiting for rsp\n"));
					taskMap->result = RESULT_ERROR;
					taskMap->tid = 0;
					taskMap->state = RPC_SYNC_STATE_DONE;
					OSSEMAPHORE_Release(taskMap->rspSema);
					break;
				default:
					_DBG_(RPC_TRACE(
		"RPC_SyncNotification state=%d\n",
						taskMap->state));
					break;
				}
			} else {
			}
		}
		}

		clientIndex = RPC_SYS_GetClientHandle(clientID);
		internalParam = (RPC_SyncParams_t *)
		RPC_GetUserData(clientIndex);
		if (internalParam &&
			internalParam->clientParams.rpcNtfFn)
			internalParam->clientParams.rpcNtfFn(event, clientID);
		break;
	default:
		_DBG_(RPC_TRACE(
			"RPC_SyncNotification: Unsupported event %d\n",
			(int) event.event));
		break;

	}
}
