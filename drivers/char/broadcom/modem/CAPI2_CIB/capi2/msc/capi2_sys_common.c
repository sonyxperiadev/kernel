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
#include "taskmsgs.h"
#include "consts.h"

#include "xdr_porting_layer.h"
#include "xdr.h"

#include "common_defs.h"
#include "uelbs_api.h"
#include "ms_database_def.h"
#include "common_sim.h"
#include "sim_def.h"
#ifndef UNDER_LINUX
#include <string.h>
#endif
#include "assert.h"
#include "sysparm.h"
#include "engmode_api.h"
#include "sysparm.h"
///
#include "i2c_drv.h"
#include "ecdc.h"
#include "uelbs_api.h"
#include "common_sim.h"
#include "sim_def.h"
#include "stk_def.h"
#include "mti_trace.h"
#include "logapi.h"
#include "log.h"
#include "tones_def.h"
#include "phonebk_def.h"
#include "phonectrl_def.h"
#include "phonectrl_api.h"
#include "rtc.h"
#include "netreg_def.h"
#include "ms_database_old.h"
#include "ms_database_api.h"
#include "netreg_util_old.h"
#include "netreg_util.h"
#include "netreg_api.h"
#include "common_sim.h"
#include "sim_def.h"
#include "stk_def.h"
#include "ss_def.h"
#include "sim_api.h"
#include "phonectrl_def.h"
#include "isim_def.h"
#include "ostypes.h"
#include "pch_def.h"
#include "pchex_def.h"
#include "hal_em_battmgr.h"
#include "cc_def.h"
#include "rtem_events.h"
#include "rtc.h"
#include "engmode_def.h"
#include "sms_def.h"
#include "simlock_def.h"

#include "capi2_global.h"
#include "capi2_mstruct.h"
#include "capi2_cc_ds.h"
#include "capi2_cc_msg.h"
#include "capi2_msnu.h"
#include "ss_api_old.h"
#include "ss_lcs_def.h"
#include "capi2_ss_msg.h"
#include "capi2_cp_socket.h"
#include "capi2_cp_msg.h"
#include "capi2_pch_msg.h"
#include "capi2_sms_msg.h"
#include "capi2_phonectrl_api.h"
#include "capi2_phonectrl_msg.h"
#include "capi2_isim_msg.h"
#include "capi2_sim_msg.h"
#include "capi2_ss_msg.h"
#include "capi2_stk_ds.h"
#include "capi2_stk_msg.h"
#include "lcs_cplane_api.h"
#include "capi2_lcs_msg.h"
#include "lcs_cplane_rrc_api.h"
#include "lcs_cplane_shared_def.h"
#include "lcs_ftt_api.h"
#include "capi2_phonebk_msg.h"
#include "capi2_lcs_cplane_msg.h"

#define MAX_CC_GET_ELEM_SIZE  64
#include "capi2_gen_msg.h"

#include "ipcproperties.h"
#include "rpc_global.h"
#include "rpc_ipc.h"
#include "xdr_porting_layer.h"
#include "xdr.h"
#include "rpc_api.h"

#include "capi2_reqrep.h"//RPC_ResponseCallbackFunc_t *gRespCb = NULL;
#include "rpc_sync_api.h"

static Boolean isXdrTblRegistered = FALSE;

Boolean HandleIndicationMsg(RPC_Msg_t* pMsg, CAPI2_ReqRep_t* reqRep);


void HandleCapi2ReqMsg(RPC_Msg_t* pMsg, 
				  ResultDataBufHandle_t dataBufHandle, 
				  UInt32 userContextData)
{
	CAPI2_DispatchRequestToServer(pMsg, dataBufHandle);
}

void HandleCapi2RspMsg(RPC_Msg_t* pMsg, 
				  ResultDataBufHandle_t dataBufHandle, 
				  UInt32 userContextData)
{
	RPC_RequestCallbackFunc_t* pCbk = (RPC_RequestCallbackFunc_t*)userContextData;

	if(HandleIndicationMsg(pMsg, (CAPI2_ReqRep_t*)(pMsg->dataBuf)) )
	{
		RPC_SYSFreeResultDataBuffer(dataBufHandle);
	}
	else
	{
		pCbk(pMsg, dataBufHandle, 0);
	}
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
	params.userData = 0;
	params.xdrtbl = capi2GetXdrStruct();
	
	params.table_size = (!isXdrTblRegistered)?capi2GetXdrStructSize():0;

	if(!isXdrTblRegistered)
		isXdrTblRegistered = TRUE;

	handle = RPC_SYS_RegisterClient(&params);

	return RPC_SYS_GetClientID(handle);
}

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
	params.respCb = HandleCapi2RspMsg;
	params.table_size = capi2GetXdrStructSize();
	params.userData = (UInt32)respCb;
	params.xdrtbl = capi2GetXdrStruct();

	syncParams.copyCb = Capi2CopyPayload;
//	gRespCb = respCb;
	handle = RPC_SyncRegisterClient(&params,&syncParams);

	return RPC_SYS_GetClientID(handle);
}


Boolean CAPI2_SYS_DeregisterClient(UInt8 Id)
{
	RPC_SYS_DeregisterClient(RPC_SYS_GetClientHandle(Id));
	return TRUE;
}

void CAPI2_SYS_DeregisterClientEx(UInt8 clientID)
{
	if (clientID) { }  //fixes compiler warnings
}

