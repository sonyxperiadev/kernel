/****************************************************************************
*
*     Copyright (c) 2007-2008 Broadcom Corporation
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
/**
*
*   @file   rpc_sys.c
*
*   @brief  This file implements RPC system api functions
*
****************************************************************************/
#include "rpc_internal_api.h"
#include "rpc_ipc.h"

RPC_FlowControlCallbackFunc_t*	stFlowControlCb = NULL;
RpcProcessorType_t	gProcessorType = RPC_APPS;
RPC_EventCallbackFunc_t* stEventCb = NULL;
extern Boolean xdr_main_init(void);

static RPC_Result_t RPC_BufferDelivery(PACKET_InterfaceType_t interfaceType, UInt8 index, PACKET_BufHandle_t dataBufHandle);



//******************************************************************************
//	 			RPC Apps EP Register
//******************************************************************************
RPC_Result_t RPC_SYS_EndPointRegister(RpcProcessorType_t processorType)
{
	Result_t result = 0;
	gProcessorType = processorType;
	
	result = RPC_IPC_EndPointInit(processorType);

	return result;
}


Result_t RPC_SYS_Init(RPC_EventCallbackFunc_t eventCb)
{
	Result_t res = 0;
	stEventCb = eventCb;

	res = RPC_IPC_Init(gProcessorType);
	
	//xdr_main_init();

	return res;
}


void RPC_HandleEvent(void* eventHandle)
{
	ResultDataBuffer_t*  dataBuf;
	Result_t res = 0;
	
	PACKET_BufHandle_t bufHandle = (PACKET_BufHandle_t)eventHandle;

	dataBuf = capi2_malloc(sizeof(ResultDataBuffer_t));

	if(dataBuf)
	{
		dataBuf->refCount = 1;
		res = RPC_DeserializeMsg(RPC_PACKET_GetBufferData(bufHandle), RPC_PACKET_GetBufferLength(bufHandle),dataBuf);

		if(res == RESULT_OK)
		{
			RPC_DispatchMsg(dataBuf);
		}
	}

	RPC_PACKET_FreeBuffer(bufHandle);
}

static RPC_Result_t RPC_BufferDelivery(PACKET_InterfaceType_t interfaceType, UInt8 index, PACKET_BufHandle_t dataBufHandle)
{
	if (interfaceType || index) { }  //fixes compiler warnings

	if(stEventCb)
		stEventCb((void*)dataBufHandle);
	else
		RPC_HandleEvent((void*)dataBufHandle);

	return RPC_RESULT_PENDING;
}





Int8 gClientIndex = 0;//Client Index zero is reserved


#define MAX_RPC_CLIENTS 25
static RPC_InitParams_t gClientMap[MAX_RPC_CLIENTS];

UInt8 gClientIDs[255]={0};

Boolean RPC_SYS_BindClientID(RPC_Handle_t handle, UInt8 userClientID) 
{
	UInt8 index = (UInt8)handle;

	if(index < MAX_RPC_CLIENTS)
	{
		gClientIDs[userClientID] = index;
		return TRUE;
	}
	return FALSE;
}

RPC_Handle_t RPC_SYS_RegisterClient(const RPC_InitParams_t *params) 
{
	UInt8 clientIndex = 0;

	gClientIndex++;

	clientIndex = gClientIndex;

	if(clientIndex >= MAX_RPC_CLIENTS)
		return 0;

	gClientMap[clientIndex] = *params;

	RPC_PACKET_RegisterDataInd(0, gClientMap[clientIndex].iType, RPC_BufferDelivery, params->flowCb);

	rpc_internal_xdr_init();
	rpc_register_xdr(clientIndex, params->xdrtbl, params->table_size);

	return (RPC_Handle_t)clientIndex;
}

Boolean RPC_SYS_LookupXdr(UInt8 clientIndex, UInt16 index, XdrClientInfo_t* clientInfo)
{
	clientInfo->mainProc = NULL;
	clientInfo->xdrEntry = NULL;

	if(clientIndex <= gClientIndex && index < gClientMap[clientIndex].table_size)
	{
		clientInfo->mainProc = gClientMap[clientIndex].mainProc;
		clientInfo->xdrEntry = &(gClientMap[clientIndex].xdrtbl[index]);
		clientInfo->maxDataBufSize = gClientMap[clientIndex].maxDataBufSize;
		return TRUE;
	}

	return FALSE;
}




Boolean RPC_SYS_DeregisterClient(RPC_Handle_t handle)
{
	UInt8 index = (UInt8)handle;

	if(index < MAX_RPC_CLIENTS)
	{
		memset(&gClientMap[index],0,sizeof(RPC_InitParams_t));
		if(index == gClientIndex)
			gClientIndex--;
		return TRUE;
	}
	return FALSE;
}

UInt8 GetClientIndex(ResultDataBuffer_t* pDataBuf, Boolean* isUnsolicited)
{
	RPC_Msg_t* pMsg = &(pDataBuf->rsp.rootMsg);
	RPC_InternalXdrInfo_t entry = {0};
	bool_t ret;
	UInt8 index;
	
	*isUnsolicited = FALSE;
	
	//Check if message is unsolicited
	if(pDataBuf->rsp.rootMsg.msgId != MSG_CAPI2_ACK_RSP)
	{
		ret = rpc_fast_lookup((UInt16)pDataBuf->rsp.rootMsg.msgId, &entry);
		if(ret)
		{
			*isUnsolicited = (entry.xdrInfo && entry.xdrInfo->unsolicited)?TRUE:FALSE;
		}
	}

	//Lookup index on registered clients
	index = gClientIDs[pMsg->clientID];
	
	if(index > 0 && index < MAX_RPC_CLIENTS)
		return index;
	
	index = 0;
	
	if(pDataBuf->rsp.rootMsg.msgId == MSG_CAPI2_ACK_RSP)
	{
		index = pDataBuf->rsp.clientIndex;
	}
	else
	{
		//Lookup clientIndex based on XDR table registered client
		if(ret && entry.clientIndex < MAX_RPC_CLIENTS)
		{
			index = entry.clientIndex;
		}
	}
	return index;
}

void RPC_DispatchMsg(ResultDataBuffer_t* pDataBuf)
{
	UInt8 clientIndex;
	Boolean bFree = TRUE;
	Boolean isUnsolicited = FALSE;
	RPC_Msg_t* pMsg = &(pDataBuf->rsp.rootMsg);
	int i;
	
	pDataBuf->refCount = 1;//set
	
	//Handle Ack first
	if(pMsg->msgId == MSG_CAPI2_ACK_RSP)
	{
		RPC_Ack_t	*ackRsp = (RPC_Ack_t*)pMsg->dataBuf;
		clientIndex = GetClientIndex(pDataBuf, &isUnsolicited);
		
		if(gClientMap[clientIndex].ackCb)
		{
			gClientMap[clientIndex].ackCb(pMsg->tid, pMsg->clientID, ackRsp->ackResult, ackRsp->ackUsrData);
			RPC_SYSFreeResultDataBuffer((ResultDataBufHandle_t)pDataBuf);
		}
	}
	else
	{
		clientIndex = GetClientIndex(pDataBuf, &isUnsolicited);
		
		if(pDataBuf->rsp.msgType == RPC_TYPE_REQUEST)
		{
			if(gClientMap[clientIndex].reqCb != NULL)
			{
				(gClientMap[clientIndex].reqCb)(pMsg, (ResultDataBufHandle_t)pDataBuf, gClientMap[clientIndex].userData);
			}
		}
		else //RPC_TYPE_RESPONSE
		{
			if(clientIndex > 0 && (pMsg->tid != 0 || pMsg->clientID != 0) && !(isUnsolicited))
			{
				_DBG_(RPC_TRACE("RPC_DispatchMsg Unicast msgId=0x%x cIndex=%d tid=%d cid=%d unsol=%d", pMsg->msgId, clientIndex, pMsg->tid, pMsg->clientID, isUnsolicited));
				if(gClientMap[clientIndex].respCb != NULL)
				{
					bFree = FALSE;
					(gClientMap[clientIndex].respCb)(pMsg, (ResultDataBufHandle_t)pDataBuf,gClientMap[clientIndex].userData);
				}
			}
			else if((pMsg->tid == 0 && pMsg->clientID == 0) || isUnsolicited)//unsolicited
			{
				_DBG_(RPC_TRACE("RPC_DispatchMsg Broadcast msgId=0x%x cIndex=%d tid=%d cid=%d unsol=%d", pMsg->msgId, clientIndex, pMsg->tid, pMsg->clientID, isUnsolicited));
				//Set the ref count first
				pDataBuf->refCount = 0;//reset
				for(i=1;i<=gClientIndex;i++)
				{
					if(gClientMap[i].respCb != NULL)
						pDataBuf->refCount++;
				}
				
				//Broadcast the message
				for(i=1;i<=gClientIndex;i++)
				{
					if(gClientMap[i].respCb != NULL)
					{
						(gClientMap[i].respCb)(pMsg, (ResultDataBufHandle_t)pDataBuf,gClientMap[i].userData);
					}
				}
			}
			else
			{
				_DBG_(RPC_TRACE("RPC_DispatchMsg IGNORED msgId=0x%x cIndex=%d tid=%d cid=%d unsol=%d", pMsg->msgId, clientIndex, pMsg->tid, pMsg->clientID, isUnsolicited));
				RPC_SYSFreeResultDataBuffer((ResultDataBufHandle_t)pDataBuf);
			}
		}//RPC_TYPE_RESPONSE
	}
}

PACKET_InterfaceType_t RPC_GetInterfaceType(UInt8 clientIndex)
{
	if(clientIndex >= MAX_RPC_CLIENTS)
	{
		return 0;
	}
	return gClientMap[clientIndex].iType;
}

UInt32 RPC_GetUserData(UInt8 clientIndex)
{
	if(clientIndex >= MAX_RPC_CLIENTS)
	{
		return 0;
	}
	return gClientMap[clientIndex].userData;
}
