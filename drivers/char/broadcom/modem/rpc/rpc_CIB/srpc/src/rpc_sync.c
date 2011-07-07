/****************************************************************************
*
*     Copyright (c) 2004 Broadcom Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           Broadcom Corporation
*           16215 Alton Parkway
*           P.O. Box 57013
*           Irvine, California 92619-7013
*
*     All information contained in this document is Broadcom Corporation
*     company private, proprietary, and trade secret.
*
****************************************************************************/
/**
*
*   @file   rpc_sync.c
*
*   @brief  This file implements the SYNC RPC functions.
*
****************************************************************************/


//******************************************************************************
//	 			include block
//******************************************************************************
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
#define OSHEAP_Alloc(x) kmalloc( x, GFP_KERNEL )
#define OSHEAP_Delete(x) kfree( x )
#define OSTASK_GetCurrentTask( ) sys_gettid( )
#define OSTASK_IsValidTask(x)  ( NULL != find_task_by_vpid(x) )
#define OSTASK_Sleep(x)  msleep(x) 
#endif



//--------------------------------------------------------------------
// Local Definitions
//--------------------------------------------------------------------
#define  MAX_TASKS_NUM						20   // number of concurrent tasks to support

#ifdef FPGA_VERSION												 
#define  RPC_SEMAPHORE_TIMEOUT			(TICKS_FOREVER) //FPGA very slow but should NOT be slower than 60 secs. Need further investigation.
#else
#define  RPC_SEMAPHORE_TIMEOUT			(TICKS_ONE_SECOND * 60) // 60 [second]
#endif

#define REMOTE_AP_NOT_RESPONDING (semaStatus==OSSTATUS_SUCCESS)
#define REMOTE_CP_NOT_RESPONDING (semaStatus==OSSTATUS_SUCCESS)

//---------------------------------------------------------------------
// Local variables
//---------------------------------------------------------------------
static UInt32 sCurrTID = 1;

// semaphore to manage access to array of task/req structs
static Semaphore_t	semaTaskReq = NULL;	

// always keep one client registered with RPC so we 
// can get responses to synchronous calls without having 
// a rpc client registered
static UInt8 sClientId = INVALID_CLIENT_ID;

// number of ticks to sleep while waiting for RPC to initialize
#define		RPC_INIT_SLEEP_TICKS	50

typedef struct
{
	UInt32 sig;
	UInt32   val;
}RPC_SyncContext_t;

// struct use to map request data to the task that originated the request
typedef struct {
	Task_t	task;				// task ID of originating task
	UInt32 tid;					// transaction ID of request
	Semaphore_t ackSema;		// semaphore used to wait/signal request ack
	RPC_ACK_Result_t ack;	// ack result from RPC
	Boolean isResultPending;		// pending result from ack
	Semaphore_t rspSema;		// semaphore used to wait/signal request response
	Result_t result;			// result code from response
	MsgType_t msgType;			// message type of response
	void* data;					// pointer to data buffer from response (may be null)
//vvvv	void* data;					// pointer to data buffer from response (may be null)
	Int32 dataLen;				// size of response data buffer
	UInt32 rspSize;				// size of actual data from response copied into data buffer
	Boolean releaseC2Buf;		// if TRUE, release response buffer immediately in response callback;
								// if FALSE, calling api must call RPC_SyncReleaseResponseBuffer() to release buffer memory
	void* dataBufHandle;		// handle to response data buffer
}TaskRequestMap_t; 

static TaskRequestMap_t sTaskRequestMap[MAX_TASKS_NUM];

// -------------------------------------------------------------------------------

// internal helper functions
// get task/response map struct for current task
static TaskRequestMap_t* GetMapForCurrentTask(void);
// get unused task/response map struct to be used with current task
static TaskRequestMap_t* GetNewMapForCurrentTask(void);
// get task/response map struct with the specified transaction ID
static TaskRequestMap_t* GetMapForTID( UInt32 tid );

void RPC_SyncHandleResponse(RPC_Msg_t* pMsg, 
							ResultDataBufHandle_t dataBufHandle, 
							UInt32 userContextData);
void RPC_SyncHandleAck( UInt32 tid, UInt8 clientID, RPC_ACK_Result_t ackResult, UInt32 ackData );

//**************************************************************************************
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

	// reset task map storage
	memset(&sTaskRequestMap, 0, sizeof(sTaskRequestMap));

	// create semaphore to manage access to array of task/req structs
	semaTaskReq = OSSEMAPHORE_Create( 1, OSSUSPEND_PRIORITY );
	assert(semaTaskReq);

	return result;
}

typedef struct
{
	RPC_InitParams_t clientParams; 
	RPC_InitParams_t SyncRpcParams; 
	RPC_SyncInitParams_t syncInitParams;
	UInt8 clientID;
}RPC_SyncParams_t;

//**************************************************************************************
/**
	Register new RPC client.  
	@return		Client ID 
**/
RPC_Handle_t RPC_SyncRegisterClient(RPC_InitParams_t*	initParams, RPC_SyncInitParams_t* syncInitParams)
{
	RPC_SyncParams_t* internalParam = (RPC_SyncParams_t*)OSHEAP_Alloc(sizeof(RPC_SyncParams_t));

	internalParam->clientParams = *initParams;
	internalParam->SyncRpcParams = *initParams;
	
	internalParam->SyncRpcParams.respCb = RPC_SyncHandleResponse;
	internalParam->SyncRpcParams.ackCb = RPC_SyncHandleAck;
	internalParam->SyncRpcParams.userData = (UInt32)internalParam;

	internalParam->syncInitParams = *syncInitParams;

	return RPC_SYS_RegisterClient(&internalParam->SyncRpcParams);
}

//**************************************************************************************
/**
	Deregister Async RPC client.  
	@param		inClientId (in) Client id of RPC client.
**/
void RPC_SyncDeregisterClient( UInt8 inClientId )
{
	//get user data
	//delete buffer
	//set user data
}

TaskRequestMap_t* RPC_SyncInitTaskMap()
{
	TaskRequestMap_t* taskMap = GetMapForCurrentTask();

	if ( !taskMap )
	{
		taskMap = GetNewMapForCurrentTask();
		assert(taskMap);
		// fill in struct (task field is already filled in by GetNewMapForCurrentTask() call)
		taskMap->ackSema = OSSEMAPHORE_Create( 1, OSSUSPEND_PRIORITY );
		assert(taskMap->ackSema);
		// start in signalled state
		OSSEMAPHORE_Obtain( taskMap->ackSema, TICKS_FOREVER );

		taskMap->rspSema = OSSEMAPHORE_Create( 1, OSSUSPEND_PRIORITY );
		assert(taskMap->rspSema);
		// start in signalled state
		OSSEMAPHORE_Obtain( taskMap->rspSema, TICKS_FOREVER );

	}

	taskMap->tid = 0;
	taskMap->ack = ACK_FAILED;
	taskMap->isResultPending = FALSE;
	taskMap->result = RESULT_ERROR;
	taskMap->data = NULL;
	taskMap->dataLen = 0;
	taskMap->rspSize = 0;
	taskMap->releaseC2Buf = TRUE;
	taskMap->dataBufHandle = NULL;

	return taskMap;
}

//**************************************************************************************
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
UInt32 RPC_SyncCreateTID( void* data, Int32 size )
{
	UInt32 tid;
	TaskRequestMap_t* taskMap = NULL;

	//Protect the tid with semaphore
	OSSEMAPHORE_Obtain( semaTaskReq, TICKS_FOREVER );
	tid = sCurrTID;
	if ( 0 == ++sCurrTID )
	{
		sCurrTID = 1;
	}
	OSSEMAPHORE_Release( semaTaskReq );

	taskMap = RPC_SyncInitTaskMap();

	taskMap->tid = tid;
	taskMap->data = data;
	taskMap->dataLen = size;

	return tid;
}


UInt32 RPC_SyncSetTID(UInt32 tid, void* data, Int32 size )
{
	TaskRequestMap_t* taskMap = NULL;


	taskMap = RPC_SyncInitTaskMap();
	taskMap->tid = tid;
	taskMap->data = data;
	taskMap->dataLen = size;

	return tid;
}

//**************************************************************************************
/**
	Retrieve the response from a RPC function call. Note that this includes the ack result as well.
	@param		tid (in) Transaction id for request.
	@param		cid (in) Client id for request
	@param		ack (out) Ack result for request
	@param		msgType (out) Message type of response.
	@param		dataSize (out) Actual data size copied to response data buffer
	
	@return Result code of response.
	
**/
Result_t RPC_SyncWaitForResponse( UInt32 tid, UInt8 cid, RPC_ACK_Result_t* ack, MsgType_t* msgType, UInt32* dataSize )
{
	OSStatus_t semaStatus;
	Result_t result = RESULT_OK;
	TaskRequestMap_t* taskMap = GetMapForCurrentTask();
	assert(taskMap);

	// wait to be signalled that request has been ack'd
	semaStatus = OSSEMAPHORE_Obtain( taskMap->ackSema, RPC_SEMAPHORE_TIMEOUT );
	//If you see this assert then likely remote processor does not free the buffers.
	//- Likely cause is CAPI2 Task on remote processor is stuck waiting on a Queue.
	//- or Watchdog issue where CAPI2 Task does not get chance to run.
	//- or Remote processor is completely DEAD
#ifdef FUSE_APPS_PROCESSOR
	xassert(REMOTE_CP_NOT_RESPONDING, semaStatus);
#else
	xassert(REMOTE_AP_NOT_RESPONDING, semaStatus);
#endif	
	
	*ack = taskMap->ack;

	// request ack'd by comm proc?

	if ( ACK_SUCCESS == taskMap->ack )
	{
		// synchronous response or error response from async api?
		if ( !taskMap->isResultPending )
		{
			// yes, so wait to be signalled that response is ready
			semaStatus = OSSEMAPHORE_Obtain( taskMap->rspSema, RPC_SEMAPHORE_TIMEOUT );
			//If you see this assert then likely remote processor does not free the buffers.
			//- Likely cause is CAPI2 Task on remote processor is stuck waiting on a Queue.
			//- or Watchdog issue where CAPI2 Task does not get chance to run.
			//- or Remote processor is completely DEAD
			#ifdef FUSE_APPS_PROCESSOR
				xassert(REMOTE_CP_NOT_RESPONDING, semaStatus);
			#else
				xassert(REMOTE_AP_NOT_RESPONDING, semaStatus);
			#endif	

			if ( msgType )
			{
				*msgType = taskMap->msgType;
			}

			if ( dataSize )
			{
				*dataSize = taskMap->rspSize;
			}
			
			result = taskMap->result;
		}
		else
		{
			
		}
	}
	else
	{
		xassert(FALSE, taskMap->ack);
	}

	_DBG_(RPC_TRACE("RPC_SyncWaitForResponse tid=%d cid=%d msg=%d ack=%d pend=%d sz=%d rs=%d task=0x%x \r\n", 
																						tid, cid, taskMap->msgType,
																						taskMap->ack, taskMap->isResultPending,
																						taskMap->rspSize, result,
																						(UInt32)OSTASK_GetCurrentTask() ));

	return result;
}

//**************************************************************************************
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
Result_t RPC_SyncWaitForResponseTimer( UInt32 tid, UInt8 cid, RPC_ACK_Result_t* ack, MsgType_t* msgType, UInt32* dataSize, UInt32 timeout)
{
	OSStatus_t semaStatus;
	Result_t result = RESULT_OK;
	TaskRequestMap_t* taskMap = GetMapForCurrentTask();
	assert(taskMap);

	// wait to be signalled that request has been ack'd
	semaStatus = OSSEMAPHORE_Obtain( taskMap->ackSema, (Ticks_t) timeout );
	//If you see this assert then likely remote processor does not free the buffers.
	//- Likely cause is CAPI2 Task on remote processor is stuck waiting on a Queue.
	//- or Watchdog issue where CAPI2 Task does not get chance to run.
	//- or Remote processor is completely DEAD

	if(semaStatus == OSSTATUS_TIMEOUT)
		return RESULT_TIMER_EXPIRED;
	else if (semaStatus != OSSTATUS_SUCCESS)
		return RESULT_ERROR;
	
	
	*ack = taskMap->ack;

	// request ack'd by comm proc?

	if ( ACK_SUCCESS == taskMap->ack )
	{
		// synchronous response or error response from async api?
		if ( !taskMap->isResultPending )
		{
			// yes, so wait to be signalled that response is ready
			semaStatus = OSSEMAPHORE_Obtain( taskMap->rspSema, (Ticks_t) timeout );
			//If you see this assert then likely remote processor does not free the buffers.
			//- Likely cause is CAPI2 Task on remote processor is stuck waiting on a Queue.
			//- or Watchdog issue where CAPI2 Task does not get chance to run.
			//- or Remote processor is completely DEAD
			if(semaStatus == OSSTATUS_TIMEOUT)
				return RESULT_TIMER_EXPIRED;
			else if (semaStatus != OSSTATUS_SUCCESS)
				return RESULT_ERROR;

			if ( msgType )
			{
				*msgType = taskMap->msgType;
			}

			if ( dataSize )
			{
				*dataSize = taskMap->rspSize;
			}
			
			result = taskMap->result;
		}
		else
		{
			
		}
	}
	else
	{
		return RESULT_ERROR;
	}

	_DBG_(RPC_TRACE("RPC_SyncWaitForResponseTimer tid=%d cid=%d msg=%d ack=%d pend=%d sz=%d rs=%d task=0x%x \r\n", 
																						tid, cid, taskMap->msgType,
																						taskMap->ack, taskMap->isResultPending,
																						taskMap->rspSize, result,
																						(UInt32)OSTASK_GetCurrentTask() ));

	return result;
}

//**************************************************************************************
/**
	Handle the ack result from the RPC function call.
	@param		tid (in) Transaction id for request.
	@param		ack (in) Ack result for request

**/
void RPC_SyncHandleAck( UInt32 tid, UInt8 clientID, RPC_ACK_Result_t ack, UInt32 ackData )
{
	OSStatus_t semaStatus;
	// find the map struct corresponding to this transaction ID
	TaskRequestMap_t* taskMap = NULL;
	Boolean isResultPending = (ackData)?TRUE:FALSE;
	
	if(tid == 0)
	{
		_DBG_(RPC_TRACE("RPC_SyncHandleAck Ignored tid %d ack %d pending %d\r\n", tid, ack, isResultPending ));
		return;
	}
	
	taskMap = GetMapForTID( tid );
//	assert(taskMap);

	if ( taskMap )
	{
		// record ack status
		taskMap->ack = ack;
		taskMap->isResultPending = isResultPending;
		
		_DBG_(RPC_TRACE("RPC_SyncHandleAck tid %d ack %d pending %d\r\n", tid, ack, isResultPending ));

		// since there won't be anybody waiting on it (ie. it will
		// be dispatched using the client ID/msg type)
		if ( (ACK_SUCCESS == ack) && isResultPending )
		{
			taskMap->tid = 0;
		}
		
		// notify calling task of receipt of ack
		semaStatus = OSSEMAPHORE_Release( taskMap->ackSema );
        if(semaStatus != OSSTATUS_SUCCESS)
        {
            _DBG_(RPC_TRACE("WARNING!!!! RPC_SyncHandleAck: Semaphore release failed\r\n"));
        }
	}
	else
	{
		_DBG_(RPC_TRACE("WARNING!!!! RPC_SyncHandleAck No Task map found tid %d ack %d pending %d\r\n", tid, ack, isResultPending ));
	}
}

//**************************************************************************************
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
void RPC_SyncHandleResponse(RPC_Msg_t* pMsg, 
							ResultDataBufHandle_t dataBufHandle, 
							UInt32 userContextData)
{
	RPC_SyncParams_t* internalParam;
	OSStatus_t semaStatus;
	UInt32 tid = pMsg->tid;
	UInt8 clientID = pMsg->clientID;
	MsgType_t msgType = pMsg->msgId;
	Result_t result = RESULT_OK;
	void* dataBuf = pMsg->dataBuf;
	UInt32 dataLength = pMsg->dataLen;

	TaskRequestMap_t* taskMap = NULL;
	// default is to always release response buffer automatically
	Boolean bReleaseResponseBuffer = TRUE;

	internalParam = (RPC_SyncParams_t*)(void*)userContextData;
	assert(internalParam != NULL);

	if ( 0 != tid )
	{
		taskMap = GetMapForTID( tid );
	}

	if(internalParam && internalParam->syncInitParams.copyCb)
		internalParam->syncInitParams.copyCb(msgType, dataBuf, (taskMap)?taskMap->dataLen:0, (taskMap)?taskMap->data:NULL, &dataLength,  &result);

	_DBG_(RPC_TRACE("RPC_SyncHandleResponse tid=%d cid=%d msg=%d res=%d tlen=%d tmap=%d len=%d \r\n", 
																	tid, clientID, msgType, result,
																	(taskMap)?taskMap->dataLen:0, 
																	(taskMap)?1:0,
																	dataLength));

	// did we find a pending request with this transaction id?
	if ( taskMap )
	{
		taskMap->result = result;
		taskMap->tid = 0;
		bReleaseResponseBuffer = taskMap->releaseC2Buf;
		taskMap->msgType = msgType;
		taskMap->rspSize = dataLength;

		xassert( !dataBuf || !(taskMap->data) || (taskMap->dataLen >= (Int32)dataLength),  msgType);

		// need to keep response buffer around for deep copy
		if( !bReleaseResponseBuffer )
		{
		   taskMap->dataBufHandle = dataBufHandle;
		}
		
		// notify calling task of receipt of response
		semaStatus = OSSEMAPHORE_Release( taskMap->rspSema );
		if(semaStatus != OSSTATUS_SUCCESS)
        {
            _DBG_(RPC_TRACE("WARNING!!!! RPC_SyncHandleResponse: Semaphore release failed\r\n"));
        }
	}
	else //Dispatch unsolicited notifications to the client
	{
		if(RESULT_OK != result)
		{
			_DBG_(RPC_TRACE("WARNING!!!! RPC_SyncHandleResponse tid %d cid %d result=%d noTaskMap \r\n", tid, clientID, result, dataLength ));
		}

		internalParam = (RPC_SyncParams_t*)(void*)userContextData;
		assert(internalParam && internalParam->clientParams.respCb);
		internalParam->clientParams.respCb( pMsg, dataBufHandle, internalParam->clientParams.userData);

		return; // The buffer will be freed by the client
	}

	if ( bReleaseResponseBuffer )
	{
		// release response buffer
		RPC_SYSFreeResultDataBuffer( dataBufHandle );
	}
}

//**************************************************************************************
/**
	Lock the response buffer associated with the current task. After calling this api, 
	response buffer will not be automatically released byRPC Sync Framework. Client apis
	must call RPC_SyncReleaseResponseBuffer() to release the response buffer. 
	
	Note: response buffer for a given call must be released before another rpc
		  call is made from the same task
**/
void RPC_SyncLockResponseBuffer(void)
{
	TaskRequestMap_t* taskMap = GetMapForCurrentTask();
	assert(taskMap);
	
	taskMap->releaseC2Buf = FALSE;
}

//**************************************************************************************
/**
	Release the response buffer associated with the current task.
**/
void RPC_SyncReleaseResponseBuffer(void)
{
	TaskRequestMap_t* taskMap = GetMapForCurrentTask();
	assert(taskMap);
	assert(taskMap->dataBufHandle);
	
	RPC_SYSFreeResultDataBuffer( taskMap->dataBufHandle );	
	taskMap->dataBufHandle = NULL;
}

void* RPC_SyncAllocFromHeap( UInt32 size )
{
	return OSHEAP_Alloc( size );
}

void RPC_SyncReleaseToHeap( void* inBuf )
{
	if ( inBuf )
		OSHEAP_Delete( inBuf );
}

UInt8 RPC_SyncGetClientId()
{
	return sClientId;
}

//
//  Internal helper functions
//

// get task/response map struct for current task
TaskRequestMap_t* GetMapForCurrentTask()
{
	TaskRequestMap_t* taskMap = NULL;
	Task_t currTask = OSTASK_GetCurrentTask();
	UInt8 i;

	for ( i=0; i < MAX_TASKS_NUM; i++ )
	{
		if ( currTask == (sTaskRequestMap+i)->task )
		{
			taskMap = (sTaskRequestMap+i);
			break;
		}
	}

	return taskMap;
}

// get unused task/response map struct to be used with current task
TaskRequestMap_t* GetNewMapForCurrentTask()
{
	TaskRequestMap_t* taskMap = NULL;
	UInt8 i;

	OSSEMAPHORE_Obtain( semaTaskReq, TICKS_FOREVER );
	
	for ( i=0; i < MAX_TASKS_NUM; i++ )
	{
		_DBG_(RPC_TRACE("GetNewMapForCurrentTask detail task %x index = %d\r\n", (UInt32)((sTaskRequestMap+i)->task), i ));

		if ( NULL == (sTaskRequestMap+i)->task )
		{
			(sTaskRequestMap+i)->task = OSTASK_GetCurrentTask();
			taskMap = (sTaskRequestMap+i);
			break;
		}
	}

	if (MAX_TASKS_NUM == i)
	{
		for ( i=0; i < MAX_TASKS_NUM; i++ )
		{
			_DBG_(RPC_TRACE("GetNewMapForCurrentTask 2nd loop task %x index = %d\r\n", (UInt32)((sTaskRequestMap+i)->task), i ));

			if  (FALSE == OSTASK_IsValidTask((sTaskRequestMap+i)->task)) 
			{
				(sTaskRequestMap+i)->task = OSTASK_GetCurrentTask();
				taskMap = (sTaskRequestMap+i);
				break;
			}

		}
	}

	OSSEMAPHORE_Release( semaTaskReq );

	_DBG_(RPC_TRACE("GetNewMapForCurrentTask task %x index= %d\r\n", (UInt32)OSTASK_GetCurrentTask(), i ));

	return taskMap;
}

// get task/response map struct with the specified transaction ID
TaskRequestMap_t* GetMapForTID( UInt32 tid )
{
	TaskRequestMap_t* taskMap = NULL;
	UInt8 i;

	for ( i=0; i < MAX_TASKS_NUM; i++ )
	{
		if ( tid == (sTaskRequestMap+i)->tid )
		{
			taskMap = (sTaskRequestMap+i);
			break;
		}
	}

	return taskMap;
}


//**************************************************************************************
/**
	Add ctx to tid. 
	@param		val (in) context value to be added 

**/
UInt32 RPC_SyncAddCbkToTid( UInt32 val)
{
	RPC_SyncContext_t *ctx;
	TaskRequestMap_t* taskMap = GetMapForCurrentTask();
	assert(taskMap);
	
	ctx = OSHEAP_Alloc(sizeof(RPC_SyncContext_t));
	assert(ctx != NULL);
	ctx->sig = 0xBABEFACE;
	ctx->val = val;
	
	_DBG_(RPC_TRACE("RPC_SyncAddCbkToTid oldTid=%d newTid=%d TaskID=%x\r\n", taskMap->tid, (UInt32)ctx,  taskMap->task));

	taskMap->tid = (UInt32)ctx;

	return (UInt32)ctx;
}

//**************************************************************************************
/**
	Get context from tid. 
	@param		tid (in) tid 

**/
UInt32 RPC_SyncGetCbkFromTid(UInt32 tid)
{
	UInt32 val = 0;
	RPC_SyncContext_t* ctx = (RPC_SyncContext_t*)tid;
	
	assert(ctx != NULL);

	_DBG_(RPC_TRACE("RPC_SyncGetCbkFromTid tid %d sig=%x \r\n", tid, ctx->sig ));

	if(ctx->sig == 0xBABEFACE && ctx->val != 0)
	{
		val = ctx->val;
	}
	else
	{
		OSTASK_Sleep(5000);
		_DBG_(RPC_TRACE("RPC_SyncGetCbkFromTid ERROR tid %d sig=%x \r\n", tid, ctx->sig ));
		xassert(0, tid);
	}
	return val;
}

//**************************************************************************************
/**
	Delete context from tid. 
	@param		tid (in) tid 

**/
void RPC_SyncDeleteCbkFromTid(UInt32 tid)
{
	RPC_SyncContext_t* ctx = (RPC_SyncContext_t*)tid;
	
	assert(ctx != NULL);

	_DBG_(RPC_TRACE("RPC_SyncDeleteCbkFromTid tid %d sig=%x \r\n", tid, ctx->sig ));

	if(ctx->sig == 0xBABEFACE && ctx->val != 0)
	{
		ctx->sig = 0;
		ctx->val = 0;
		OSHEAP_Delete(ctx);
	}
	else
	{
		OSTASK_Sleep(5000);
		_DBG_(RPC_TRACE("RPC_SyncDeleteCbkFromTid ERROR tid %d sig=%x \r\n", tid, ctx->sig ));
		xassert(0, tid);
	}
}

