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
*   @file   capi2_reqrep.c
*
*   @brief  This file define the request/respose structure for 
*	serialize/deserialize.
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

#include "capi2_reqrep.h"
#include "capi2_common_xdr.h"



#define DEVELOPMENT_CAPI2_XDR_UNION_MAPPING
#define _T(a) a
XDR_ENUM_FUNC(SimNumber_t)

static void CAPI2_InitDialogId(ClientInfo_t* clientInfo);

//#define TABLE_SIZE	(sizeof(CAPI2_ReqRep_dscrm) / sizeof(xdr_discrim))
#define REQRSP_MSG_EXT(unsize,capi2_proc,ptr_proc,maxMsgSize)	unsize, maxMsgSize


static const RPC_XdrInfo_t CAPI2_ReqRep_dscrm[] = {
	
	/* Add phonebook message serialize/deserialize routine map */
	{ MSG_PBK_READY_IND,_T("MSG_PBK_READY_IND"), (xdrproc_t)xdr_default_proc, REQRSP_MSG_EXT(0,NULL,NULL,0) },
	
#include "capi2_gen_union_map.h"
	
	/* Add other modules message to serialize/deserialize routine map */
	{ (MsgType_t)__dontcare__, "",NULL_xdrproc_t, REQRSP_MSG_EXT(0,NULL,NULL,0) } 

};

RPC_XdrInfo_t* capi2GetXdrStruct(void)
{
	return (RPC_XdrInfo_t*)CAPI2_ReqRep_dscrm;
}

UInt32 capi2GetXdrStructSize(void)
{
	return (sizeof(CAPI2_ReqRep_dscrm)/sizeof(RPC_XdrInfo_t));
}

bool_t xdr_CAPI2_ReqRep_t(XDR *xdrs, CAPI2_ReqRep_t* req, xdrproc_t proc)
{
	if( xdr_u_char(xdrs, &req->clientID) &&
		XDR_ENUM(xdrs, &req->msgId, MsgType_t) &&
		XDR_ENUM(xdrs, &req->respId, MsgType_t) &&
		XDR_ENUM(xdrs, &req->result, Result_t) &&
		xdr_ClientInfo_t(xdrs, &req->clientInfo) 
		)
	{

#ifdef CAPI2_CLIENTINFO_COMPLETE //Remove after RPC component support the extended interface
			if(xdrs->x_basiclogbuffer)
			{
				UInt32 val = (req->clientInfo.dialogId & 0x7FFFFFFF);

				snprintf(xdrs->x_basiclogbuffer, MAX_XDR_BASIC_LOG, "{cid:%d sim:%d dlg:%d ref:%d rz:%d} result %d",
					req->clientInfo.clientId, req->clientInfo.simId, val, req->clientInfo.clientRef, req->clientInfo.reserved, req->result);
			}
#endif
			return proc(xdrs, &(req->req_rep_u));
	}
	
	return(FALSE);
}



Result_t CAPI2_SerializeReqRspEx(CAPI2_ReqRep_t* req_rsp, MsgType_t msgId, ClientInfo_t* clientInfo,  Result_t result, char** stream, UInt32* len)
{
	RPC_Msg_t msg;
	
	CAPI2_InitDialogId(clientInfo);

	req_rsp->clientID = clientInfo->clientId;
	req_rsp->msgId = msgId;
	req_rsp->result = result;
	req_rsp->clientInfo = *clientInfo;

	msg.msgId = msgId;
	msg.tid = clientInfo->reserved;
	msg.clientID = clientInfo->clientId;
	msg.dataBuf = (void*)req_rsp;
	msg.dataLen = 0;
	
	return RPC_SerializeReq(&msg);

}

Result_t CAPI2_SerializeRspEx(CAPI2_ReqRep_t* req_rsp, MsgType_t msgId, ClientInfo_t* clientInfo,  Result_t result, char** stream, UInt32* len)
{
	RPC_Msg_t msg;
	
	req_rsp->clientID = clientInfo->clientId;
	req_rsp->msgId = msgId;
	req_rsp->result = result;
	req_rsp->clientInfo = *clientInfo;

	msg.msgId = msgId;
	msg.tid = clientInfo->reserved;
	msg.clientID = clientInfo->clientId;
	msg.dataBuf = (void*)req_rsp;
	msg.dataLen = 0;
	
	return RPC_SerializeRsp(&msg);

}


Result_t CAPI2_SerializeRsp(CAPI2_ReqRep_t* req_rsp, UInt32 tid, MsgType_t msgId, UInt8 clientID,  Result_t result, char** stream, UInt32* len)
{
	RPC_Msg_t msg;
	
	msg.msgId = msgId;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)req_rsp;
	msg.dataLen = 0;
	
	return RPC_SerializeRsp(&msg);
}




bool_t xdr_ClientInfo_t(XDR *xdrs, ClientInfo_t *args)
{
	XDR_LOG(xdrs,"ClientInfo_t ")
	if( 
		_xdr_u_char(xdrs, &args->clientId,"clientId") && 
		_xdr_u_long(xdrs, &args->clientRef,"refId") &&
		_xdr_u_long(xdrs, &args->dialogId,"dialogId") &&
		XDR_ENUM(xdrs, &args->simId,SimNumber_t) &&
		_xdr_u_long(xdrs, &args->reserved,"reserved") 
	  )
		return(TRUE);
	else
		return(FALSE);
}

Result_t CAPI2_SerializeReqRsp(CAPI2_ReqRep_t* req_rsp, UInt32 tid, MsgType_t msgId, UInt8 clientID,  Result_t result, char** stream, UInt32* len)
{

	ClientInfo_t clientInfo={0};
	
	clientInfo.clientId = clientID;
	clientInfo.clientRef = tid;
	clientInfo.reserved = tid;


	return CAPI2_SerializeReqRspEx(req_rsp,  msgId, &clientInfo,  result, stream, len);

}

static UInt32 stDialogID = 0;

static void CAPI2_InitDialogId(ClientInfo_t* clientInfo)
{
   UInt32 curID;

   if(clientInfo->dialogId == 0)
   {
	   curID = ++stDialogID;

	   curID |= 0x80000000;

	   clientInfo->dialogId = curID;
   }
   else if( !((clientInfo->dialogId) & 0x80000000) || (clientInfo->dialogId & (~0x80000000)) > stDialogID )
   {
		_DBG_(CAPI2_TRACE("CAPI2_SerializeReqRsp Warning ( Invalid dialogID %x)  current=%x\r\n", clientInfo->dialogId, stDialogID));
   }
}


void CAPI2_InitClientInfo(ClientInfo_t *clientInfo, UInt32 tid, UInt8 clientID)
{
	memset(clientInfo,0, sizeof(ClientInfo_t));
	clientInfo->clientId = clientID;
	clientInfo->clientRef = tid;
	clientInfo->reserved = tid;
}

Boolean CAPI2_IsResultOK(Result_t ret)
{
	if(ret == CC_MAKE_CALL_SUCCESS ||
		ret == CC_END_CALL_SUCCESS ||
		ret == CC_ACCEPT_CALL_SUCCESS ||
		ret == CC_HOLD_CALL_SUCCESS ||
		ret == CC_RESUME_CALL_SUCCESS ||
		ret == CC_SWAP_CALL_SUCCESS ||
		ret == CC_SPLIT_CALL_SUCCESS ||
		ret == CC_JOIN_CALL_SUCCESS ||
		ret == CC_TRANS_CALL_SUCCESS ||
		ret == CC_OPERATION_SUCCESS ||
		ret == RESULT_DONE ||
		ret == RESULT_OK)
	{
		return TRUE;
	}

	return FALSE;
}

