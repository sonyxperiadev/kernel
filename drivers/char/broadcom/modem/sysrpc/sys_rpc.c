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
#define UNDEF_SYS_GEN_MIDS
#define DEFINE_SYS_GEN_MIDS_NEW

#include "mobcom_types.h"
#include "resultcode.h"
#include "taskmsgs.h"

#include <linux/broadcom/ipcproperties.h>
#include "rpc_global.h"
#include "rpc_ipc.h"

#include "xdr_porting_layer.h"
#include "xdr.h"
#include "rpc_api.h"
#include "rpc_sync_api.h"

#include "xdr.h"

#include "sys_api.h"
#include "sys_common_rpc.h"
#include "sys_gen_rpc.h"
#include "sys_rpc.h"
#include "mobcom_types.h"
#include "config.h"
#include "bcmlog.h"

#ifdef DEVELOPMENT_SYSRPC_WIN_UNIT_TEST
#define _D(a) _ ## a
#else
#define _D(a) a
#endif

/********************** REGISTER CBK HANDLERS *******************************************/

static void HandleSysReqMsg(RPC_Msg_t * pMsg,
			    ResultDataBufHandle_t dataBufHandle,
			    UInt32 userContextData)
{
	SYS_ReqRep_t *req = (SYS_ReqRep_t *) pMsg->dataBuf;

	RPC_SendAckForRequest(dataBufHandle, 0);

	if (pMsg->msgId == MSG_AT_COMMAND_REQ) {

	} else {
		SYS_GenCommsMsgHnd(pMsg, req);
	}

	RPC_SYSFreeResultDataBuffer(dataBufHandle);
}

static void HandleSysEventRspCb(RPC_Msg_t * pMsg,
				ResultDataBufHandle_t dataBufHandle,
				UInt32 userContextData)
{
	//SYS_ReqRep_t* rsp;
	// SYS_TRACE( "HandleSysEventRspCb msg=0x%x clientID=%d \n", pMsg->msgId, 0);

	RPC_SYSFreeResultDataBuffer(dataBufHandle);
}

static void SYS_GetPayloadInfo(SYS_ReqRep_t * reqRep, MsgType_t msgId,
			       void **ppBuf, UInt32 * len)
{
	*ppBuf = NULL;
	*len = 0;
	switch (msgId) {
	default:
		{
			SYS_GenGetPayloadInfo((void *)&reqRep->req_rep_u, msgId,
					      ppBuf, len);
			*len = RPC_GetMsgPayloadSize(msgId);
		}
		break;
	}
}

static Boolean SysCopyPayload(MsgType_t msgType,
			      void *srcDataBuf,
			      UInt32 destBufSize,
			      void *destDataBuf,
			      UInt32 * outDestDataSize, Result_t * outResult)
{
	UInt32 len;
	SYS_ReqRep_t *req;
	void *dataBuf;
	SYS_ReqRep_t *reqRep = (SYS_ReqRep_t *) srcDataBuf;

	xassert(srcDataBuf != NULL, 0);
	SYS_GetPayloadInfo(reqRep, msgType, &dataBuf, &len);

	req = (SYS_ReqRep_t *) srcDataBuf;
	*outResult = req->result;
	*outDestDataSize = len;

	if (destDataBuf && dataBuf && len <= destBufSize) {
		memcpy(destDataBuf, dataBuf, len);
		return TRUE;
	}
	return FALSE;
}

void SYS_InitRpc(void)
{
	static int first_time = 1;
	//UInt8 ret = 0;

	if (first_time) {
		RPC_InitParams_t params = { 0 };
		RPC_SyncInitParams_t syncParams;

		params.iType = INTERFACE_RPC_DEFAULT;
		params.respCb = HandleSysEventRspCb;
		params.reqCb = HandleSysReqMsg;
		params.mainProc = (xdrproc_t) xdr_SYS_ReqRep_t;
		sysGetXdrStruct(&(params.xdrtbl), &(params.table_size));
		params.maxDataBufSize = sizeof(SYS_ReqRep_t);

		syncParams.copyCb = SysCopyPayload;
		RPC_SyncRegisterClient(&params, &syncParams);

		BCMLOG_EnableLogId(BCMLOG_RPC_KERNEL_BASIC, 1);

		first_time = 0;

//              SYS_TRACE( "SYS_InitRpc \n");
	}
}

Result_t Send_SYS_RspForRequest(RPC_Msg_t * req, MsgType_t msgType,
				SYS_ReqRep_t * payload)
{
	RPC_Msg_t rsp;

	rsp.msgId = msgType;
	rsp.tid = req->tid;
	rsp.clientID = req->clientID;
	rsp.dataBuf = (void *)payload;
	rsp.dataLen = 0;

	return RPC_SerializeRsp(&rsp);
}

Result_t SYS_SendRsp(UInt32 tid, UInt8 clientId, MsgType_t msgType,
		     void *payload)
{
	RPC_Msg_t rsp;

	rsp.msgId = msgType;
	rsp.tid = tid;
	rsp.clientID = clientId;
	rsp.dataBuf = (void *)payload;
	rsp.dataLen = 0;

	return RPC_SerializeRsp(&rsp);
}

Result_t Handle_CAPI2_CPPS_Control(RPC_Msg_t * pReqMsg, UInt32 cmd,
				   UInt32 address, UInt32 offset, UInt32 size)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;

	memset(&data, 0, sizeof(SYS_ReqRep_t));
	data.req_rep_u.CAPI2_CPPS_Control_Rsp.val = (UInt32) 3;	//CPPS_CONTROL_NO_FFS_SUPPORT_ON_AP

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_CPPS_CONTROL_RSP, &data);
	return result;
}

Result_t Handle_CAPI2_FLASH_SaveImage(RPC_Msg_t * pReqMsg, UInt32 flash_addr,
				      UInt32 length, UInt32 shared_mem_addr)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;

	memset(&data, 0, sizeof(SYS_ReqRep_t));
	data.req_rep_u.CAPI2_FLASH_SaveImage_Rsp.val = FALSE;	//(Boolean)FlashSaveData(flash_addr,length,(UInt8*)shared_mem_addr);

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_FLASH_SAVEIMAGE_RSP, &data);
	return result;
}
