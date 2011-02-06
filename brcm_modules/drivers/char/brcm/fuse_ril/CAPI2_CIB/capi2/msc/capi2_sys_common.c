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
#include "mobcom_types.h"
#include "resultcode.h"
#include "common_defs.h"
#include "uelbs_api.h"
#include "ms_database_def.h"
#include "capi2_reqrep.h"
#include "capi2_taskmsgs.h"
#include "rpc_sync_api.h"

//RPC_ResponseCallbackFunc_t *gRespCb = NULL;


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

	RPC_Handle_t handle;
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

//	gRespCb = respCb;

	handle = RPC_SYS_RegisterClient(&params);

	return RPC_SYS_GetClientID(handle);
}


// **FIXME** MAG - should be able to enable now that sync RPC is working on ANdroid....
#ifndef UNDER_LINUX
UInt8 CAPI2_SYS_SyncRegisterClient(RPC_RequestCallbackFunc_t reqCb,
								 RPC_ResponseCallbackFunc_t respCb, 
							   RPC_AckCallbackFunc_t		ackCb,
							   RPC_FlowControlCallbackFunc_t	flowControlCb, 
							   RPC_SyncInitParams_t* syncInitParams)
{

	RPC_Handle_t handle;
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
	handle = RPC_SyncRegisterClient(&params,&syncParams);

	return RPC_SYS_GetClientID(handle);
}
#endif // UNDER_LINUX

Boolean CAPI2_SYS_DeregisterClient(UInt8 Id)
{
	RPC_SYS_DeregisterClient(RPC_SYS_GetClientHandle(Id));
	return TRUE;
}

void CAPI2_SYS_DeregisterClientEx(UInt8 clientID)
{
	if (clientID) { }  //fixes compiler warnings
}

