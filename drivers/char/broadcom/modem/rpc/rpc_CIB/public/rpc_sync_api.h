/******************************************************************************
*
* (c) 1999-2008 Broadcom Corporation
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
******************************************************************************/

/**
*
*   @file   rpc_sync_api.h
*
*   @brief  This file defines the types and prototypes for the RPC SYNC functions.
*
****************************************************************************/
/**

*   @defgroup	RPC_SyncApi   RPC SYNC API
*
*   @brief      This group defines the types and prototypes for the RPC SYNC fuctions.
*
\msc
	"AP Client", RPC, "CP Client";
	"AP Client"=>RPC [label="RPC_SyncInitialize()", URL="\ref RPC_SyncInitialize()"];
	"CP Client">>RPC [label="RPC_SyncInitialize()", URL="\ref RPC_SyncInitialize()"];
	"AP Client"=>RPC [label="RPC_SyncRegisterClient()", URL="\ref RPC_SyncRegisterClient()"];
	"CP Client">>RPC [label="RPC_SyncRegisterClient()", URL="\ref RPC_SyncRegisterClient()"];
	"AP Client"=>RPC [label="RPC_SyncCreateTID()", URL="\ref RPC_SyncCreateTID()"];
	"AP Client"=>RPC [label="RPC_SerializeReq()", URL="\ref RPC_SerializeReq()"];
	RPC rbox RPC [label="Serialize, send IPC Msg, Deser msg"];
	"AP Client":>RPC [label="RPC_SyncWaitForResponse()", URL="\ref RPC_SyncWaitForResponse()"];
	"AP Client"=>RPC [label="CBK: Notify Request"];
	"AP Client"=>"AP Client" [label="thread blocks"];
	"CP Client"=>RPC [label="RPC_SendAckForRequest()", URL="\ref RPC_SendAckForRequest()"];
	RPC rbox RPC [label="ARPC:Serialize, send IPC Msg, Deser msg"];
	RPC rbox RPC [label="SRPC:Raise ACK Sem"];
	"CP Client"=>RPC [label="RPC_SerializeRsp()", URL="\ref RPC_SerializeRsp()"];
	RPC rbox RPC [label="ARPC:Serialize, send IPC Msg, Deser msg"];
	RPC rbox RPC [label="SRPC:Copy Resp Data, Raise RSP Sem"];
	"AP Client"=>"AP Client" [label="thread unblocks"];
\endmsc
****************************************************************************/

#ifndef _RPC_SYNC_H_
#define _RPC_SYNC_H_

/*******************************************************************************
*                              include block
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @addtogroup RPC_SyncApi
 * @{
 */

/******************************************************************************/
/**
	Function to initialize the RPC Sync. This component provides framework for blocking RPC -
	communication between different processors.
	@return		\n RESULT_OK for success,
			\n RESULT_ERROR for failure,
**/
Result_t RPC_SyncInitialize(void);

/******************************************************************************/
/**
	The callback to copy the payload data. This allows deep copy of payload data.
	@param		msgType (in) Message Type
	@param		srcDataBuf (in) The source buffer
	@param		destBufSize (in) Destination buffer size
	@param		destDataBuf (in) Destination buffer
	@param		outDestDataSize (out) Size of data copied to Destination buffer
	@param		outResult (in) Copy Result_t
	@return	TRUE on success or FALSE
	@note
		The callback is called when we receive the response from remote processor
**/
typedef Boolean(RPC_CopyPayLoad_t) (MsgType_t msgType,
				    void *srcDataBuf,
				    UInt32 destBufSize,
				    void *destDataBuf,
				    UInt32 *outDestDataSize,
				    Result_t *outResult);

/**
	RPC Sync Init Params
**/
typedef struct {
	/* Pointer to callbak function for deep copy. */
	RPC_CopyPayLoad_t *copyCb;
} RPC_SyncInitParams_t;

/******************************************************************************/
/**
	Register new RPC Sync client.
	@param		initParams (in) The initialization params for low level RPC ( Aysnc )
	@param		syncInitParams (in) The initialization params for high level RPC ( Sync )
	@return		Non Zero RPC_Handle_t
	@note
	The RPC_SyncRegisterClient() calls RPC_SYS_RegisterClient() internally.
**/
RPC_Handle_t RPC_SyncRegisterClient(RPC_InitParams_t *initParams,
			    RPC_SyncInitParams_t *syncInitParams);

/******************************************************************************/
/**
	This function creates a unique TID for the calling client.
	@param		data (in) pointer to buffer to store response
	@param		size (in) size of response buffer
	@return		transaction ID for request

	@note
	If size is -1, this indicates that the response size is variable and buffer should
	be allocated by the RPC callback handler. The buffer will be freed by the caller
	of RPC_SyncGetResponse

**/
UInt32 RPC_SyncCreateTID(void *data, Int32 size);

/******************************************************************************/
/**
	This function block waits until we receive the response from remote processor.
	@param		tid (in) Transaction id for request.
	@param		cid (in) Client id for request
	@param		ack (out) Ack result for request
	@param		msgType (out) Message type of response.
	@param		dataSize (out) Actual data size copied to response data buffer

	@return Result code of response.
	@note
		Note that this includes the ack result as well.
**/
Result_t RPC_SyncWaitForResponse(UInt32 tid, UInt8 cid,
				 RPC_ACK_Result_t *ack,
				 MsgType_t *msgType,
				 UInt32 *dataSize);

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
				      MsgType_t *msgType,
				      UInt32 *dataSize,
				      UInt32 timeout);

/******************************************************************************/
/**
	The calling function can add callback ( or any data )  as TID.
	@param		val (in) Function pointer ( or any data ) to be used as part of TID

	@return Returns new TID which has associated val built into this TID.

	@note
		RPC Sync creates a TID from heap and client MUST call RPC_SyncDeleteCbkFromTid() later to avoid memory leak.
**/
UInt32 RPC_SyncAddCbkToTid(UInt32 val);

/******************************************************************************/
/**
	The calling function can retrieve callback from TID.
	@param		tid (in) Transaction ID

	@return Function pointer ( or any data ) that is part of tid

**/
UInt32 RPC_SyncGetCbkFromTid(UInt32 tid);

/******************************************************************************/
/**
	The calling function delete the callback ( or any data ) which is part of tid.
	Delete any memory associated with this.
	@param		tid (in) Transaction ID

	@return None

**/
void RPC_SyncDeleteCbkFromTid(UInt32 tid);

/******************************************************************************/
/**
	Lock the response buffer associated with the current task. After calling this api,
	response buffer will not be automatically released by RPC Sync Framework. Client apis
	must call RPC_SyncReleaseResponseBuffer() to release the response buffer.

	Note: response buffer for a given rpc call must be released before another rpc
		  call is made from the same task
**/
void RPC_SyncLockResponseBuffer(void);

/******************************************************************************/
/**
	Release the response buffer associated with the current task.
**/
void RPC_SyncReleaseResponseBuffer(void);
/** @} */

/** \cond  */

/* retrieve "main" client id */
UInt8 RPC_SyncGetClientId(void);

/*Obsolete */
UInt32 RPC_SyncSetTID(UInt32 tid, void *data, Int32 size);

void RPC_SyncDeregisterClient(UInt8 inClientId);

/** \endcond   */

#ifdef __cplusplus
}
#endif

#endif /* _RPC_SYNC_H_ */
