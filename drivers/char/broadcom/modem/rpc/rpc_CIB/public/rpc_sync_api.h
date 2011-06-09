/****************************************************************************
*
*     Copyright (c) 2005 Broadcom Corporation
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
*   @file   rpc_sync_api.h
*
*   @brief  This file defines the types and prototypes for the RPC SYNC functions.
*
****************************************************************************/
/**

*   @defgroup   
*
*   @brief      This group defines the types and prototypes for the RPC SYNC fuctions.
*	
****************************************************************************/

#ifndef _RPC_SYNC_H_
#define _RPC_SYNC_H_

//******************************************************************************
//	 			include block
//******************************************************************************

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @addtogroup 
 * @{
 */


//**************************************************************************************
/**
	The callback to copy the payload data
	@return		Client ID 
**/
typedef Boolean (RPC_CopyPayLoad_t)( MsgType_t msgType, 
									 void* srcDataBuf, 
									 UInt32 destBufSize,
									 void* destDataBuf, 
									 UInt32* outDestDataSize, 
									 Result_t* outResult);


//**************************************************************************************
/**
	Initialize the RPC Sync Framework.  
	@param		timeout (in) Number of ticks to wait for emulator initialization
	@return		\n RESULT_OK for success, 
				\n RESULT_ERROR for failure, 
	@note
	This function will block until emulator is initialized or timeout duration has expired.
**/

//**************************************************************************************
/**
	Register new RPC Sync client.  
	@return		Client ID 
**/
Result_t RPC_SyncInitialize(void);

typedef struct
{
	RPC_CopyPayLoad_t* copyCb;
}RPC_SyncInitParams_t;

//**************************************************************************************
/**
	Register new RPC Sync client.  
	@return		Client ID 
**/
RPC_Handle_t RPC_SyncRegisterClient(RPC_InitParams_t*	initParams, RPC_SyncInitParams_t* syncInitParams);

void RPC_SyncDeregisterClient( UInt8 inClientId );

//**************************************************************************************
/**
	Prepare to issue a RPC request.
	@param		data (in) pointer to buffer to store response
	@param		size (in) size of response buffer
	@return		transaction ID for request

	@note
	If size is -1, this indicates that the response size is variable and buffer should
	be allocated by the RPC callback handler. The buffer will be freed by the caller 
	of RPC_SyncGetResponse

**/
UInt32 RPC_SyncCreateTID( void* data, Int32 size );

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
Result_t RPC_SyncWaitForResponse( UInt32 tid, UInt8 cid, RPC_ACK_Result_t* ack, MsgType_t* msgType, UInt32* dataSize );


//**************************************************************************************
/**
	Add ctx to tid. 
	@param		val (in) context value to be added 

**/
UInt32 RPC_SyncAddCbkToTid( UInt32 val);


void RPC_SyncDeleteCbkFromTid(UInt32 tid);

UInt32 RPC_SyncGetCbkFromTid(UInt32 tid);


UInt32 RPC_SyncSetTID(UInt32 tid, void* data, Int32 size );




//**************************************************************************************
/**
	Lock the response buffer associated with the current task. After calling this api, 
	response buffer will not be automatically released by RPC Sync Framework. Client apis
	must call RPC_SyncReleaseResponseBuffer() to release the response buffer. 
	
	Note: response buffer for a given rpc call must be released before another rpc
		  call is made from the same task
**/
void RPC_SyncLockResponseBuffer(void);

//**************************************************************************************
/**
	Release the response buffer associated with the current task.
**/
void RPC_SyncReleaseResponseBuffer(void);


// retrieve "main" client id
UInt8 RPC_SyncGetClientId(void);


/** @} */
#ifdef __cplusplus
}
#endif

#endif
