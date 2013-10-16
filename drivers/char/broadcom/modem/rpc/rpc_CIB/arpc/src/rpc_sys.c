/****************************************************************************
*
*     Copyright (c) 2007-2008 Broadcom Corporation
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
/**
*
*   @file   rpc_sys.c
*
*   @brief  This file implements RPC system api functions
*
****************************************************************************/
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
#include "rpc_ipc.h"

RPC_FlowControlCallbackFunc_t *stFlowControlCb = NULL;
RpcProcessorType_t gProcessorType = RPC_APPS;
RPC_EventCallbackFunc_t *stEventCb = NULL;
extern Boolean xdr_main_init(void);

static RPC_Result_t RPC_BufferDelivery(PACKET_InterfaceType_t interfaceType,
				       UInt8 index,
				       PACKET_BufHandle_t dataBufHandle);
UInt8 GetClientIndex(ResultDataBuffer_t *pDataBuf, Boolean *isUnsolicited);
void SYS_ReleaseClientID(unsigned char clientID);
static Boolean gRpcInit = FALSE;

static RPC_USER_LOCK_DECLARE(gRpcLock);
RPC_USER_LOCK_DECLARE(gRpcFreeLock);

static Boolean sCpResetting;

/******************************************************************************
*                              RPC Apps EP Register
******************************************************************************/
RPC_Result_t RPC_SYS_EndPointRegister(RpcProcessorType_t processorType)
{
	RPC_Result_t result = RPC_RESULT_OK;
	gProcessorType = processorType;

	result = RPC_IPC_EndPointInit(processorType);

	return result;
}

Result_t RPC_SYS_Init(RPC_EventCallbackFunc_t eventCb)
{
	RPC_Result_t res = RPC_RESULT_OK;
	stEventCb = eventCb;
	sCpResetting = FALSE;

	_DBG_(RPC_TRACE("RPC_SYS_Init gRpcInit=%d", gRpcInit));

	if (!gRpcInit) {
		gRpcInit = TRUE;

		res = RPC_IPC_Init(gProcessorType);

		RPC_USER_LOCK_INIT(gRpcLock);
		RPC_USER_LOCK_INIT(gRpcFreeLock);

	}
	/*xdr_main_init();*/

	return (res == RPC_RESULT_OK) ? RESULT_OK : RESULT_ERROR;
}

void RPC_HandleEvent(void *eventHandle)
{
	UInt8 clientId = 0;
	ResultDataBuffer_t *dataBuf;
	Result_t res = RESULT_OK;

	PACKET_BufHandle_t bufHandle = (PACKET_BufHandle_t) eventHandle;

	dataBuf = capi2_malloc(sizeof(ResultDataBuffer_t));

	if (dataBuf) {
		dataBuf->refCount = 1;
		res =
		    RPC_DeserializeMsg(RPC_PACKET_GetBufferData(bufHandle),
				       RPC_PACKET_GetBufferLength(bufHandle),
				       dataBuf);

		if (res == RESULT_OK) {
			Boolean isUnsolicited;
			UInt8 clientIndex =
			    GetClientIndex(dataBuf, &isUnsolicited);
			clientId = RPC_SYS_GetClientID(clientIndex);

			RPC_USER_LOCK(gRpcLock);

			RPC_DispatchMsg(dataBuf);

			RPC_USER_UNLOCK(gRpcLock);
		} else {
			capi2_free(dataBuf);
		}
	}

	RPC_PACKET_FreeBufferEx(bufHandle, clientId);
}

static RPC_Result_t RPC_BufferDelivery(PACKET_InterfaceType_t interfaceType,
				       UInt8 index,
				       PACKET_BufHandle_t dataBufHandle)
{
	if (interfaceType || index) {
	}			/*fixes compiler warnings */

	if (index == 0xCD)
		index = 0;

	if (RPC_IsRegisteredClient(index, dataBufHandle)) {
		RPC_PACKET_IncrementBufferRef(dataBufHandle, index);

		if (stEventCb)
			stEventCb((void *)dataBufHandle);
		else
			RPC_HandleEvent((void *)dataBufHandle);

		return RPC_RESULT_PENDING;
	}
	return RPC_RESULT_ERROR;
}

typedef struct {
	Boolean notifyUnsolicited;
	Boolean ackdCPReset;
} RPC_InitLocalParams_t;


Int8 gClientIndex = 0; /*Client Index zero is reserved */

#define MAX_RPC_CLIENTS 25
static RPC_InitParams_t gClientMap[MAX_RPC_CLIENTS];
static RPC_InitLocalParams_t gClientLocalMap[MAX_RPC_CLIENTS] = { {0} };
static UInt8 gClientIDMap[MAX_RPC_CLIENTS] = {0};

UInt8 gClientIDs[256] = {0};

/* cp silent reset callback for async rpc layer; called from rpc_ipc layer */
static void RPC_Handle_RPCNotification(
	struct RpcNotificationEvent_t event)
{
	int i;

	PACKET_InterfaceType_t interfaceType = event.ifType;

	switch (event.event) {
	case RPC_CPRESET_EVT:
		if ((sCpResetting && event.param == RPC_CPRESET_START) ||
	    (!sCpResetting && event.param == RPC_CPRESET_COMPLETE)) {
			/* already resetting, so just return */
			_DBG_(RPC_TRACE(
			"RPC_Handle_CPReset already processing %s",
						sCpResetting ?
						"RPC_CPRESET_START" :
						"RPC_CPRESET_COMPLETE"));
			return;
		}

		_DBG_(RPC_TRACE("RPC_Handle_CPReset event %s interface %d",
			event.param == RPC_CPRESET_START ?
					"RPC_CPRESET_START" :
					"RPC_CPRESET_COMPLETE",
					interfaceType));

		sCpResetting = (event.param == RPC_CPRESET_START);
		/* notify all clients for given interface */
		for (i = 1; i <= gClientIndex; i++)
			if (gClientMap[i].rpcNtfFn != NULL &&

			(PACKET_InterfaceType_t)gClientMap[i].iType ==
				interfaceType) {
				_DBG_(RPC_TRACE(
				"RPC_Handle_CPReset client:%d",
				gClientIDMap[i]));
				gClientLocalMap[i].ackdCPReset = FALSE;
			(gClientMap[i].rpcNtfFn) (event, gClientIDMap[i]);

		}

		_DBG_(RPC_TRACE(
			"RPC_Handle_CPReset done for interface %d",
			interfaceType));

		break;
	default:
		break;
	}
}

void RPC_AckCPReset(UInt8 clientID)
{
	UInt8 index = RPC_SYS_GetClientHandle(clientID);
	int i;
	Boolean bReady = TRUE;

	_DBG_(RPC_TRACE("RPC_AckCPReset client %d index %d\n",
					clientID, index));

	if (index <= gClientIndex)
		gClientLocalMap[index].ackdCPReset = TRUE;

	/* check if all for given PACKET_InterfaceType_t have ack'd;
	   if so, ack to rpc_ipc layer
	*/
	for (i = 1; i <= gClientIndex; i++)
		if (gClientMap[index].iType == gClientMap[i].iType &&
		    gClientMap[i].rpcNtfFn &&
				!gClientLocalMap[i].ackdCPReset) {
			/* at least one client for given
			   interface type has not yet ack'd
			   so we're not ready yet
			*/
			_DBG_(RPC_TRACE
				("RPC_AckCPReset fail index %d\n", i));
			bReady = FALSE;
			break;
		} else {
			_DBG_(RPC_TRACE("RPC_AckCPReset %d %d %d %d %d\n",
				i, index, gClientMap[index].iType,
				gClientMap[i].iType,
				gClientLocalMap[i].ackdCPReset));
		}

	if (bReady) {
		_DBG_(RPC_TRACE
		 ("RPC_AckCPReset calling RPC_PACKET_AckReadyForCPReset\n"));
		RPC_PACKET_AckReadyForCPReset(0,
			(PACKET_InterfaceType_t)gClientMap[index].iType);
	}
}

Boolean RPC_SYS_BindClientID(RPC_Handle_t handle, UInt8 userClientID)
{
	UInt8 index = (UInt8) handle;

	if (index < MAX_RPC_CLIENTS) {
		gClientIDs[userClientID] = index;
		return TRUE;
	}
	return FALSE;
}

UInt8 RPC_SYS_GetClientID(RPC_Handle_t handle)
{
	UInt8 index = (UInt8) handle;

	if (index <= gClientIndex)
		return gClientIDMap[index];

	return 0;
}

UInt8 RPC_SYS_GetClientHandle(UInt8 userClientID)
{
	return gClientIDs[userClientID];
}

Boolean RPC_SYS_isValidClientID(UInt8 userClientID)
{
	if (userClientID == 0 || userClientID == DEFAULT_CLIENT_ID
	    || gClientIDs[userClientID] > 0)
		return TRUE;

	return FALSE;
}

static int RPC_FindUnsedSlot(void)
{
	UInt8 i;

	for (i = 1; i < MAX_RPC_CLIENTS; i++) {
		if (gClientIDMap[i] == 0)
			return i;
	}
	return -1;
}

RPC_Handle_t RPC_SYS_RegisterClient(const RPC_InitParams_t *params)
{
	UInt8 userClientID;
	UInt8 clientIndex = 0;
	int index;

	_DBG_(RPC_TRACE("RPC_SYS_RegisterClient gRpcInit=%d tblSize=%d",
		gRpcInit, params->table_size));

	if (!gRpcInit)
		RPC_SYS_Init(NULL);

	RPC_USER_LOCK(gRpcLock);

	index = RPC_FindUnsedSlot();
	if (index == -1) {
		_DBG_(RPC_TRACE("RPC_SYS_RegisterClient ERROR Max clients reached"));

		RPC_USER_UNLOCK(gRpcLock);
		return (RPC_Handle_t)0;
	}

	gClientIndex++;

	clientIndex = index;

	gClientMap[clientIndex] = *params;

	userClientID = SYS_GenClientID();

	if (userClientID == 0) {
		_DBG_(RPC_TRACE("RPC_SYS_RegisterClient client ID allocation fail"));

		RPC_USER_UNLOCK(gRpcLock);
		return (RPC_Handle_t)0;
	}

	_DBG_(RPC_TRACE("RPC_SYS_RegisterClient index=%d userClientID=%d gClientIndex=%d", clientIndex, userClientID, gClientIndex));

	RPC_PACKET_RegisterDataInd(userClientID,
		(PACKET_InterfaceType_t)(gClientMap[clientIndex].iType),
		RPC_BufferDelivery, params->flowCb, RPC_Handle_RPCNotification);

	rpc_internal_xdr_init();
	rpc_register_xdr(clientIndex, params->xdrtbl, params->table_size);

	gClientIDs[userClientID] = clientIndex;
	gClientIDMap[clientIndex] = userClientID;
	gClientLocalMap[clientIndex].notifyUnsolicited = FALSE;
	gClientLocalMap[clientIndex].ackdCPReset = FALSE;

	RPC_USER_UNLOCK(gRpcLock);
	return (RPC_Handle_t)clientIndex;
}

Boolean RPC_SYS_LookupXdr(UInt8 clientIndex, UInt16 index,
			  XdrClientInfo_t *clientInfo)
{
	clientInfo->mainProc = NULL;
	clientInfo->xdrEntry = NULL;

	if (clientIndex <= gClientIndex
	    && index < gClientMap[clientIndex].table_size) {
		clientInfo->mainProc = gClientMap[clientIndex].mainProc;
		clientInfo->xdrEntry = &(gClientMap[clientIndex].xdrtbl[index]);
		clientInfo->maxDataBufSize =
		    gClientMap[clientIndex].maxDataBufSize;
		return TRUE;
	}

	return FALSE;
}

Boolean RPC_SYS_DeregisterClient(RPC_Handle_t handle)
{
	UInt8 index = (UInt8)handle;

	RPC_USER_LOCK(gRpcLock);

	_DBG_(RPC_TRACE("RPC_SYS_DeregisterClient index=%d userClientID=%d", index, gClientIDMap[index]));

	if (index < MAX_RPC_CLIENTS) {
		memset(&gClientMap[index], 0, sizeof(RPC_InitParams_t));
		if (index == gClientIndex)
			gClientIndex--;
		SYS_ReleaseClientID(gClientIDMap[index]);
		gClientIDMap[index] = 0;
		RPC_USER_UNLOCK(gRpcLock);
		return TRUE;
	}

	RPC_USER_UNLOCK(gRpcLock);
	return FALSE;
}

Boolean RPC_EnableUnsolicitedMsgs(RPC_Handle_t handle, Boolean bSet)
{
	UInt8 index = (UInt8)handle;

	RPC_USER_LOCK(gRpcLock);

	_DBG_(RPC_TRACE
	      ("RPC_EnableUnsolicitedMsgs handle=0x%x index=%d bSet=%d", handle,
	       index, bSet));
	if (index <= gClientIndex) {
		gClientLocalMap[index].notifyUnsolicited = bSet;
		RPC_USER_UNLOCK(gRpcLock);
		return TRUE;
	}

	RPC_USER_UNLOCK(gRpcLock);
	return FALSE;
}

Boolean RPC_RegisterUnsolicitedMsgs(RPC_Handle_t handle,
				const UInt16 *msgIds,
				UInt8 listSize)
{
	if (!msgIds || listSize == 0) {
	    _DBG_(RPC_TRACE
		("RPC_RegisterUnsolicitedMsgs FAIL handle=0x%x listSize=%d",
		handle, listSize));

		return FALSE;
	}

	_DBG_(RPC_TRACE
	      ("RPC_RegisterUnsolicitedMsgs handle=0x%x listSize=%d list=[%x:%x:%x:%x:%x]",
	       handle, listSize, msgIds[0], msgIds[1], msgIds[2], msgIds[3],
	       msgIds[4]));
	return rpc_lookup_set_mask(msgIds, listSize,
				   RPC_XDR_INFO_MASK_UNSOLICITED);
}

Boolean RPC_IsRegisteredClient(UInt8 channel, PACKET_BufHandle_t dataBufHandle)
{
	Boolean ret = FALSE;

	UInt8 xdrReqClient =
	    RPC_PACKET_GetContext(INTERFACE_CAPI2, dataBufHandle);
	UInt16 msgId = RPC_PACKET_GetContextEx(INTERFACE_CAPI2, dataBufHandle);
	Boolean isValid = RPC_IsValidMsg((MsgType_t) msgId);
	UInt8 clientHandle = RPC_SYS_GetClientHandle(channel);
	UInt8 clientAckHandle = RPC_SYS_GetClientHandle(xdrReqClient);

	if (msgId == MSG_CAPI2_ACK_RSP) {
		ret = (xdrReqClient > 0 && clientAckHandle > 0);
	}  else {
		if (channel > 0)
			ret = (clientHandle != 0) ? TRUE : FALSE;
		else
			ret = isValid;
	}

	if (ret)
		_DBG_(RPC_TRACE
		      ("RPC_IsRegisteredClient[ret=%d] ch=%d xdrClient=%d msgId=0x%x validMsg=%d validClient=%d validAckClient=%d",
		       ret, channel, xdrReqClient, msgId, isValid, clientHandle,
		       clientAckHandle));
	return ret;
}

UInt8 GetClientIndex(ResultDataBuffer_t *pDataBuf, Boolean *isUnsolicited)
{
	RPC_Msg_t *pMsg = &(pDataBuf->rsp.rootMsg);
	/*coverity[var_decl], entry will be initialized in the function rpc_fast_lookup() below*/
	RPC_InternalXdrInfo_t entry;
	bool_t ret = 0;
	UInt8 index;

	*isUnsolicited = FALSE;

	/*Check if message is unsolicited*/
	if (pDataBuf->rsp.rootMsg.msgId != MSG_CAPI2_ACK_RSP) {
		ret =
		    rpc_fast_lookup((UInt16) pDataBuf->rsp.rootMsg.msgId,
					&entry);
		if (ret) {
			*isUnsolicited =
			    (entry.
			     mask & RPC_XDR_INFO_MASK_UNSOLICITED) ? TRUE :
			    FALSE;

			_DBG_(RPC_TRACE_INFO
			      ("RPC GetClientIndex msgId=0x%x xdrClient=%d mask=%x unsol=%d",
			       pDataBuf->rsp.rootMsg.msgId, entry.clientIndex,
			       entry.mask, *isUnsolicited));
			/**isUnsolicited = FALSE; */
		}
	}
	/*Lookup index on registered clients */
	index = gClientIDs[pMsg->clientID];

	if (index > 0 && index < MAX_RPC_CLIENTS)
		return index;

	index = 0;

	if (pDataBuf->rsp.rootMsg.msgId == MSG_CAPI2_ACK_RSP) {
		index = pDataBuf->rsp.clientIndex;
	} else {
		/*Lookup clientIndex based on XDR table registered client*/
		if (ret && entry.clientIndex < MAX_RPC_CLIENTS)
			index = entry.clientIndex;
	}
	return index;
}

void RPC_DispatchMsg(ResultDataBuffer_t *pDataBuf)
{
	UInt8 clientIndex;
	Boolean isUnsolicited = FALSE;
	RPC_Msg_t *pMsg = &(pDataBuf->rsp.rootMsg);
	int i;

	pDataBuf->refCount = 1; /* set */

	/* Handle Ack first */
	if (pMsg->msgId == MSG_CAPI2_ACK_RSP) {
		RPC_Ack_t *ackRsp = (RPC_Ack_t *)pMsg->dataBuf;
		clientIndex = GetClientIndex(pDataBuf, &isUnsolicited);

		if (gClientMap[clientIndex].ackCb) {
			gClientMap[clientIndex].ackCb(pMsg->tid, pMsg->clientID,
						      ackRsp->ackResult,
						      ackRsp->ackUsrData);
		}
		RPC_SYSFreeResultDataBuffer((ResultDataBufHandle_t) pDataBuf);
	} else {
		clientIndex = GetClientIndex(pDataBuf, &isUnsolicited);

		if (pDataBuf->rsp.msgType == RPC_TYPE_REQUEST) {
			if (gClientMap[clientIndex].reqCb != NULL) {
				(gClientMap[clientIndex].reqCb) (pMsg,
								 (ResultDataBufHandle_t)
								 pDataBuf,
								 gClientMap
								 [clientIndex].
								 userData);
			}
		} else {	/* RPC_TYPE_RESPONSE */
			if (clientIndex > 0
			    && (pMsg->tid != 0 || pMsg->clientID != 0)
			    && !(isUnsolicited)) {
				_DBG_(RPC_TRACE_INFO
				      ("RPC_DispatchMsg Unicast msgId=0x%x cIndex=%d tid=%d cid=%d unsol=%d",
				       pMsg->msgId, clientIndex, pMsg->tid,
				       pMsg->clientID, isUnsolicited));
				if (gClientMap[clientIndex].respCb != NULL) {
					(gClientMap[clientIndex].respCb) (pMsg,
									  (ResultDataBufHandle_t)
									  pDataBuf,
									  gClientMap
									  [clientIndex].
									  userData);
				}
			} else if ((pMsg->tid == 0 && pMsg->clientID == 0) || isUnsolicited) {	/*unsolicited */
				UInt32 numClients = 0;
				_DBG_(RPC_TRACE_INFO
				      ("RPC_DispatchMsg Broadcast msgId=0x%x xdrClient=%d tid=%d cid=%d unsol=%d ",
				       pMsg->msgId, clientIndex, pMsg->tid,
				       pMsg->clientID, isUnsolicited));
				/*Set the ref count first */
				pDataBuf->refCount = 0;	/* reset */
				for (i = 1; i <= gClientIndex; i++) {
					if (gClientMap[i].respCb != NULL
					    && (gClientLocalMap[i].
						notifyUnsolicited
						|| clientIndex == i))
						pDataBuf->refCount++;
				}

				numClients = pDataBuf->refCount;
				/* Broadcast the message */
				for (i = 1; i <= gClientIndex; i++) {
					if (gClientMap[i].respCb != NULL
					    && (gClientLocalMap[i].
						notifyUnsolicited
						|| clientIndex == i)) {
						_DBG_(RPC_TRACE_INFO
						      ("RPC_DispatchMsg Broadcast Client ( Notify ) msgId=0x%x index=%d tid=%d cid=%d unsol=%d regUnsol=%d",
						       pMsg->msgId, i,
						       pMsg->tid,
						       pMsg->clientID,
						       isUnsolicited,
						       gClientLocalMap[i].
						       notifyUnsolicited));
						(gClientMap[i].respCb) (pMsg,
									(ResultDataBufHandle_t)
									pDataBuf,
									gClientMap
									[i].
									userData);
					} else {
						_DBG_(RPC_TRACE_INFO
						      ("RPC_DispatchMsg Broadcast Client ( Skip ) msgId=0x%x index=%d tid=%d cid=%d unsol=%d regUnsol=%d",
						       pMsg->msgId, i,
						       pMsg->tid,
						       pMsg->clientID,
						       isUnsolicited,
						       gClientLocalMap[i].
						       notifyUnsolicited));
					}

				}

				/* No clients handle this message? */
				if (numClients == 0) {
					_DBG_(RPC_TRACE_INFO
					      ("RPC_DispatchMsg Broadcast Ignored msgId=0x%x cIndex=%d tid=%d cid=%d unsol=%d regUnsol=%d",
					       pMsg->msgId, i, pMsg->tid,
					       pMsg->clientID, isUnsolicited,
					       gClientLocalMap[i].
					       notifyUnsolicited));

					pDataBuf->refCount = 1;
					RPC_SYSFreeResultDataBuffer((ResultDataBufHandle_t) pDataBuf);
				}
			} else {
				_DBG_(RPC_TRACE_INFO
				      ("RPC_DispatchMsg IGNORED msgId=0x%x cIndex=%d tid=%d cid=%d unsol=%d",
				       pMsg->msgId, clientIndex, pMsg->tid,
				       pMsg->clientID, isUnsolicited));
				RPC_SYSFreeResultDataBuffer((ResultDataBufHandle_t) pDataBuf);
			}
		}		/*RPC_TYPE_RESPONSE */
	}
}

PACKET_InterfaceType_t RPC_GetInterfaceType(UInt8 clientIndex)
{
	xassert(clientIndex < MAX_RPC_CLIENTS, clientIndex);

	/*coverity[overrun-local], clientIndex has checked in above xassert function, so it will not overrun of array gClientMap.*/
	return (PACKET_InterfaceType_t) (gClientMap[clientIndex].iType);
}

UInt32 RPC_GetUserData(UInt8 clientIndex)
{
	xassert(clientIndex < MAX_RPC_CLIENTS, clientIndex);

	/*coverity[overrun-local], clientIndex has checked in above xassert function, so it will not overrun of array gClientMap.*/
	return gClientMap[clientIndex].userData;
}
