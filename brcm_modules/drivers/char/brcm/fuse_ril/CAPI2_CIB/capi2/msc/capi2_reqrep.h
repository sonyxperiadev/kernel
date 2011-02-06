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
*   @file   capi2_reqrep.h
*
*   @brief  This file define the request/respose structure for 
*	serialize/deserialize.
*
****************************************************************************/
#ifndef CAPI2_REQREP_H
#define CAPI2_REQREP_H

#ifndef UNDER_LINUX
#include "string.h"
#endif

#include "mobcom_types.h"
#include "resultcode.h"
#include "capi2_taskmsgs.h"

#include "rtc.h"
#include "netreg_def.h"

#include "ipcproperties.h"
#include "rpc_global.h"
#include "rpc_ipc.h"

#include "xdr_porting_layer.h"
#include "xdr.h"
#include "rpc_api.h"

#include "capi2_taskmsgs.h"
#include "capi2_resultcode.h"
#include "capi2_phonebk_msg.h"
#include "capi2_sms_msg.h"
#include "capi2_stk_msg.h"
#include "capi2_cc_msg.h"
#include "capi2_ss_msg.h"
#include "capi2_pch_msg.h"
#include "capi2_isim_msg.h"
#include "capi2_phonectrl_msg.h"
#include "capi2_cp_msg.h"
#include "capi2_ds_msg.h"
#include "capi2_sim_msg.h"
#include "capi2_lcs_cplane_msg.h"
#include "capi2_gen_msg.h"
#ifndef UNDER_LINUX
#include "capi2_comms_inc.h"
#endif

#define DEVELOPMENT_CAPI2_XDR_UNION_DECLARE

#define MAX_MSG_STREAM_SIZE	2048

/*********************** Temp place holder for CAPI2 Message Id's ********************************/
#define MSG_SYSPARM_SET_HSUPA_PHY_CAT_REQ  (MsgType_t)(MSG_GRP_CAPI2_GEN_A + 1)
#define MSG_SYSPARM_SET_HSUPA_PHY_CAT_RSP  (MsgType_t)(MSG_GRP_CAPI2_GEN_A + 2)
#define MSG_SYSPARM_GET_HSUPA_PHY_CAT_REQ  (MsgType_t)(MSG_GRP_CAPI2_GEN_A + 3)
#define MSG_SYSPARM_GET_HSUPA_PHY_CAT_RSP  (MsgType_t)(MSG_GRP_CAPI2_GEN_A + 4)

#define MSG_INTERTASK_MSG_TO_CP_REQ  (MsgType_t)(MSG_GRP_CAPI2_GEN_A + 5)
#define MSG_INTERTASK_MSG_TO_CP_RSP  (MsgType_t)(MSG_GRP_CAPI2_GEN_A + 6)
#define MSG_INTERTASK_MSG_TO_AP_REQ  (MsgType_t)(MSG_GRP_CAPI2_GEN_A + 7)
#define MSG_INTERTASK_MSG_TO_AP_RSP  (MsgType_t)(MSG_GRP_CAPI2_GEN_A + 8)

#define MSG_SYSPARM_GET_HSUPA_SUPPORTED_REQ  (MsgType_t)(MSG_GRP_CAPI2_GEN_A + 9)
#define MSG_SYSPARM_GET_HSUPA_SUPPORTED_RSP  (MsgType_t)(MSG_GRP_CAPI2_GEN_A + 10)
#define MSG_SYSPARM_GET_HSDPA_SUPPORTED_REQ  (MsgType_t)(MSG_GRP_CAPI2_GEN_A + 11)
#define MSG_SYSPARM_GET_HSDPA_SUPPORTED_RSP  (MsgType_t)(MSG_GRP_CAPI2_GEN_A + 12)
#define MSG_MS_FORCE_PS_REL_REQ					(MsgType_t)(MSG_GRP_CAPI2_GEN_A + 13)
#define MSG_MS_FORCE_PS_REL_RSP					(MsgType_t)(MSG_GRP_CAPI2_GEN_A + 14)
/*************************************************************************************************/

typedef struct
{
	Int32	descriptor;
	char*	msg;
	UInt16	msgLength;
	UInt16	flags;
	Boolean	finFlag;
} CAPI2_IPCPacketReq_t;

typedef struct
{
	RPC_Msg_t* pMsg;
	Result_t result;
	UInt32	dataBufLen;
	void*	pDataBuf;
	UInt32 userContextData;
}CAPI2_Msg_t;

typedef struct tag_CAPI2_ReqRep_t
{
	MsgType_t msgId;
	UInt8 clientID;
	ClientInfo_t clientInfo;
	Result_t result;
	MsgType_t respId;
	union {
		void*										void_ptr;
#include "capi2_gen_union_map.h"
	} req_rep_u;
} CAPI2_ReqRep_t;

#undef DEVELOPMENT_CAPI2_XDR_UNION_DECLARE



#include "capi2_global.h"

Result_t CAPI2_SerializeReqRsp(CAPI2_ReqRep_t* req_rsp, UInt32 tid, MsgType_t msgId, UInt8 clientID,  Result_t result, char** stream, UInt32* len);
Result_t CAPI2_SerializeRsp(CAPI2_ReqRep_t* req_rsp, UInt32 tid, MsgType_t msgId, UInt8 clientID,  Result_t result, char** stream, UInt32* len);
bool_t xdr_CAPI2_ReqRep_t(XDR *xdrs, CAPI2_ReqRep_t* req, xdrproc_t proc);
UInt8 capi2_reg_dummy(RPC_ResponseCallbackFunc_t *respCb);
void CAPI2_GetPayloadInfo(CAPI2_ReqRep_t* reqRep, MsgType_t msgId, void** ppBuf, UInt32* len);
RPC_XdrInfo_t* capi2GetXdrStruct(void);
UInt32 capi2GetXdrStructSize(void);
void CAPI2_DispatchRequestToServerInTask(RPC_Msg_t* pMsg, ResultDataBufHandle_t handle);
Result_t CAPI2_GenAppsMsgHnd(RPC_Msg_t* pMsg, CAPI2_ReqRep_t* reqRep);
void CAPI2_DispatchRequestToServer(RPC_Msg_t* pMsg, 
				  ResultDataBufHandle_t handle);
Result_t CAPI2_SerializeReqRspEx(CAPI2_ReqRep_t* req_rsp, MsgType_t msgId, ClientInfo_t *clientInfoPtr,  Result_t result, char** stream, UInt32* len);
Result_t CAPI2_SerializeRspEx(CAPI2_ReqRep_t* req_rsp, MsgType_t msgId, ClientInfo_t* clientInfo,  Result_t result, char** stream, UInt32* len);
Boolean Capi2CopyPayload( MsgType_t msgType, 
						 void* srcDataBuf, 
						 UInt32 destBufSize,
						 void* destDataBuf, 
						 UInt32* outDestDataSize, 
						 Result_t *outResult);



bool_t xdr_ClientInfo_t(XDR *xdrs, ClientInfo_t *args);

// **FIXME** MAG - don't change this during CP update integration; current CP
// sources have tid and clientID transposed, based on how the api is called from
// capi_gen_api_old.c (easier to change it once here than thousands of times in 
// capi_gen_api_old.c)
void CAPI2_InitClientInfo(ClientInfo_t *clientInfo, UInt32 tid, UInt8 clientID );

Boolean CAPI2_IsResultOK(Result_t ret);

#endif
