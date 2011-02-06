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
*   @file   capi2_sys_common.c
*
*   @brief  This file implements the mapof CAPI2 response/event to the registered
*	 calback on the app side.
*
****************************************************************************/
#include "capi2_reqrep.h"
#include "capi2_taskmsgs.h"

#ifndef RPC_INCLUDED
#include "capi2_ipc.h"
#include "capi2_ipc_config.h"

CAPI2_ResponseCallbackFunc_t* responseCb = NULL;
CAPI2_AckCallbackFunc_t*		gAsyncCb = NULL;
CAPI2_FlowControlCallbackFunc_t*	stFlowControlCb = NULL;
Capi2ProcessorType_t	gProcessorType = CAPI2_APPS;
CAPI2_EventCallbackFunc_t* stEventCb = NULL;

static Result_t CAPI2_BufferDelivery(PACKET_InterfaceType_t interfaceType, UInt8 index, PACKET_BufHandle_t dataBufHandle);

#ifndef BYPASS_IPC


//******************************************************************************
//	 			CAPI2 Apps EP Register
//******************************************************************************
Result_t CAPI2_SYS_EndPointRegister(Capi2ProcessorType_t processorType)
{
	Result_t result;
	gProcessorType = processorType;
	
	result = CAPI2_IPC_EndPointInit(processorType);

	return result;
}


Result_t CAPI2_SYS_Init(CAPI2_EventCallbackFunc_t eventCb)
{
	stEventCb = eventCb;

	return CAPI2_IPC_Init(gProcessorType);
}


void CAPI2_HandleEvent(void* eventHandle)
{
	CAPI2_ReqRep_t* reqRep;
	ResultDataBuffer_t*  dataBuf;
	
	PACKET_BufHandle_t bufHandle = (PACKET_BufHandle_t)eventHandle;

	CAPI2_DeserializeReqRsp(CAPI2_PACKET_GetBufferData(bufHandle), CAPI2_PACKET_GetBufferLength(bufHandle), &reqRep, &dataBuf);

	if(reqRep)
		CAPI2_DispatchMsg(reqRep, dataBuf);

	CAPI2_PACKET_FreeBuffer(bufHandle);
}

static Result_t CAPI2_BufferDelivery(PACKET_InterfaceType_t interfaceType, UInt8 index, PACKET_BufHandle_t dataBufHandle)
{
	if (interfaceType || index) { }  //fixes compiler warnings

	if(stEventCb)
		stEventCb((void*)dataBufHandle);
	else
		CAPI2_HandleEvent((void*)dataBufHandle);

	return RESULT_PENDING;
}

#endif






UInt8 CAPI2_SYS_RegisterClient(CAPI2_ResponseCallbackFunc_t respCb, 
							   CAPI2_AckCallbackFunc_t		asyncCb,
							   CAPI2_FlowControlCallbackFunc_t	flowControlCb)
{
	responseCb = respCb;
	gAsyncCb = asyncCb;
	stFlowControlCb = flowControlCb;
#ifndef BYPASS_IPC		
	CAPI2_PACKET_RegisterDataInd(0, INTERFACE_CAPI2, CAPI2_BufferDelivery, stFlowControlCb);
#endif

	/* Fixme: Need to comeback and see how to map this on CAPI and if multiple are needed... */
	return 65;
}


Boolean CAPI2_SYS_DeregisterClient(UInt8 clientID)
{
	if (clientID) { }  //fixes compiler warnings
	//fixme, notify CP
	responseCb = NULL;
	gAsyncCb = NULL;
	stFlowControlCb = NULL;
	return TRUE;
}

#ifndef BYPASS_IPC
void CAPI2_Apps_EP_Register();
//fixme: Remove this function call from AppsIpcSupport.c file and use CAPI2_SYS_EndPointRegister instead
void CAPI2_Apps_EP_Register()
{
	CAPI2_SYS_EndPointRegister(CAPI2_APPS);
}
#endif

#else

#include "rpc_sync_api.h"

static Boolean isXdrTblRegistered = FALSE;

void HandleCapi2ReqMsg(RPC_Msg_t* pMsg, 
				  ResultDataBufHandle_t dataBufHandle, 
				  UInt32 userContextData)
{
	CAPI2_DispatchRequestToServer(pMsg, dataBufHandle);
}


UInt8 CAPI2_SYS_RegisterClient(RPC_RequestCallbackFunc_t reqCb,
								 RPC_ResponseCallbackFunc_t respCb, 
							   RPC_AckCallbackFunc_t		ackCb,
							   RPC_FlowControlCallbackFunc_t	flowControlCb)
{

	RPC_InitParams_t params;/*={HandleCapi2ReqMsg,respCb,ackCb,flowControlCb,INTERFACE_RPC_TELEPHONY,
							capi2GetXdrStruct(),capi2GetXdrStructSize(),
							0, 	xdr_CAPI2_ReqRep_t, sizeof(CAPI2_ReqRep_t)};*/

	params.ackCb = ackCb;
	params.flowCb = flowControlCb;
	params.iType = INTERFACE_RPC_TELEPHONY;
	params.mainProc = xdr_CAPI2_ReqRep_t;
	params.maxDataBufSize = sizeof(CAPI2_ReqRep_t);
	params.reqCb = HandleCapi2ReqMsg;
	params.respCb = respCb;
	params.table_size = (!isXdrTblRegistered)?capi2GetXdrStructSize():0;
	params.userData = 0;
	params.xdrtbl = capi2GetXdrStruct();


	if(!isXdrTblRegistered)
		isXdrTblRegistered = TRUE;

	return (UInt8)RPC_SYS_RegisterClient(&params);
}

UInt8 CAPI2_SYS_SyncRegisterClient(RPC_RequestCallbackFunc_t reqCb,
								 RPC_ResponseCallbackFunc_t respCb, 
							   RPC_AckCallbackFunc_t		ackCb,
							   RPC_FlowControlCallbackFunc_t	flowControlCb, 
							   RPC_SyncInitParams_t* syncInitParams)
{

	RPC_SyncInitParams_t syncParams;
	RPC_InitParams_t params;/*={HandleCapi2ReqMsg,respCb,ackCb,flowControlCb,INTERFACE_RPC_TELEPHONY,
							capi2GetXdrStruct(),capi2GetXdrStructSize(),
							0, 	xdr_CAPI2_ReqRep_t, sizeof(CAPI2_ReqRep_t)};*/
	params.ackCb = ackCb;
	params.flowCb = flowControlCb;
	params.iType = INTERFACE_RPC_TELEPHONY;
	params.mainProc = xdr_CAPI2_ReqRep_t;
	params.maxDataBufSize = sizeof(CAPI2_ReqRep_t);
	params.reqCb = HandleCapi2ReqMsg;
	params.respCb = respCb;
	params.table_size = capi2GetXdrStructSize();
	params.userData = 0;
	params.xdrtbl = capi2GetXdrStruct();

	syncParams.copyCb = Capi2CopyPayload;
//	gRespCb = respCb;

#if defined(UNDER_CE) || defined(UNDER_LINUX)
	return 0;
#else
	return (UInt8)RPC_SyncRegisterClient(&params,&syncParams);
#endif
}


Boolean CAPI2_SYS_DeregisterClient(UInt8 Id)
{
	RPC_SYS_DeregisterClient((RPC_Handle_t)Id);
	return TRUE;
}


#endif
