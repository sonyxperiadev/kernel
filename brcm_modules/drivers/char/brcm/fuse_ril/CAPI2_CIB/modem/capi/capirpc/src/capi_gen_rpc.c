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
/****************************************************************************
*																			
*     WARNING!!!! Generated File ( Do NOT Modify !!!! )					
*																			
****************************************************************************/

#include "capi_common_rpc.h"
#include "capi_gen_rpc.h"
#include "capi_rpc.h"


//***************** < 4 > **********************






#if defined(FUSE_COMMS_PROCESSOR) 

void CAPI2_SMS_IsMeStorageEnabled(UInt32 tid, UInt8 clientID)
{
	CAPI_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(CAPI_ReqRep_t));
	
	req.respId = MSG_SMS_ISMESTORAGEENABLED_RSP;
	msg.msgId = MSG_SMS_ISMESTORAGEENABLED_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

void CAPI2_SMS_GetMaxMeCapacity(UInt32 tid, UInt8 clientID)
{
	CAPI_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(CAPI_ReqRep_t));
	
	req.respId = MSG_SMS_GETMAXMECAPACITY_RSP;
	msg.msgId = MSG_SMS_GETMAXMECAPACITY_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

void CAPI2_SMS_GetNextFreeSlot(UInt32 tid, UInt8 clientID)
{
	CAPI_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(CAPI_ReqRep_t));
	
	req.respId = MSG_SMS_GETNEXTFREESLOT_RSP;
	msg.msgId = MSG_SMS_GETNEXTFREESLOT_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

void CAPI2_SMS_SetMeSmsStatus(UInt32 tid, UInt8 clientID, UInt16 slotNumber, SIMSMSMesgStatus_t status)
{
	CAPI_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(CAPI_ReqRep_t));
	
	req.req_rep_u.CAPI2_SMS_SetMeSmsStatus_Req.slotNumber = slotNumber;
	req.req_rep_u.CAPI2_SMS_SetMeSmsStatus_Req.status = status;
	req.respId = MSG_SMS_SETMESMSSTATUS_RSP;
	msg.msgId = MSG_SMS_SETMESMSSTATUS_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

void CAPI2_SMS_GetMeSmsStatus(UInt32 tid, UInt8 clientID, UInt16 slotNumber)
{
	CAPI_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(CAPI_ReqRep_t));
	
	req.req_rep_u.CAPI2_SMS_GetMeSmsStatus_Req.slotNumber = slotNumber;
	req.respId = MSG_SMS_GETMESMSSTATUS_RSP;
	msg.msgId = MSG_SMS_GETMESMSSTATUS_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}
#endif


void CAPI2_SMS_StoreSmsToMe(UInt32 tid, UInt8 clientID, UInt8 *inSms, UInt16 inLength, SIMSMSMesgStatus_t status, UInt16 slotNumber)
{
	CAPI_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(CAPI_ReqRep_t));
	
	req.req_rep_u.CAPI2_SMS_StoreSmsToMe_Req.inSms = inSms;
	req.req_rep_u.CAPI2_SMS_StoreSmsToMe_Req.inLength = inLength;
	req.req_rep_u.CAPI2_SMS_StoreSmsToMe_Req.status = status;
	req.req_rep_u.CAPI2_SMS_StoreSmsToMe_Req.slotNumber = slotNumber;
	req.respId = MSG_SMS_STORESMSTOME_RSP;
	msg.msgId = MSG_SMS_STORESMSTOME_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

#if defined(FUSE_COMMS_PROCESSOR) 

void CAPI2_SMS_RetrieveSmsFromMe(UInt32 tid, UInt8 clientID, UInt16 slotNumber)
{
	CAPI_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(CAPI_ReqRep_t));
	
	req.req_rep_u.CAPI2_SMS_RetrieveSmsFromMe_Req.slotNumber = slotNumber;
	req.respId = MSG_SMS_RETRIEVESMSFROMME_RSP;
	msg.msgId = MSG_SMS_RETRIEVESMSFROMME_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

void CAPI2_SMS_RemoveSmsFromMe(UInt32 tid, UInt8 clientID, UInt16 slotNumber)
{
	CAPI_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(CAPI_ReqRep_t));
	
	req.req_rep_u.CAPI2_SMS_RemoveSmsFromMe_Req.slotNumber = slotNumber;
	req.respId = MSG_SMS_REMOVESMSFROMME_RSP;
	msg.msgId = MSG_SMS_REMOVESMSFROMME_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

void CAPI2_SMS_IsSmsReplSupported(UInt32 tid, UInt8 clientID)
{
	CAPI_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(CAPI_ReqRep_t));
	
	req.respId = MSG_SMS_ISSMSREPLACE_SUPPORTED_RSP;
	msg.msgId = MSG_SMS_ISSMSREPLACE_SUPPORTED_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

void CAPI2_SMS_GetMeSmsBufferStatus(UInt32 tid, UInt8 clientID, UInt16 cmd)
{
	CAPI_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(CAPI_ReqRep_t));
	
	req.req_rep_u.CAPI2_SMS_GetMeSmsBufferStatus_Req.cmd = cmd;
	req.respId = MSG_SMS_GETMESMS_BUF_STATUS_RSP;
	msg.msgId = MSG_SMS_GETMESMS_BUF_STATUS_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

void CAPI2_SMS_GetMeSmsBufferStatus_RSP(UInt32 tid, UInt8 clientID, UInt16 bfree, UInt16 bused)
{
	CAPI_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(CAPI_ReqRep_t));
	
	req.req_rep_u.CAPI2_SMS_GetMeSmsBufferStatus_RSP_Rsp.bfree = bfree;
	req.req_rep_u.CAPI2_SMS_GetMeSmsBufferStatus_RSP_Rsp.bused = bused;
	req.respId = (MsgType_t)0;
	msg.msgId = MSG_SMS_GETMESMS_BUF_STATUS_RSP;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeRsp(&msg);
}

void CAPI2_SMS_GetRecordNumberOfReplaceSMS(UInt32 tid, UInt8 clientID, SmsStorage_t storageType, UInt8 tp_pid, uchar_ptr_t oaddress)
{
	CAPI_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(CAPI_ReqRep_t));
	
	req.req_rep_u.CAPI2_SMS_GetRecordNumberOfReplaceSMS_Req.storageType = storageType;
	req.req_rep_u.CAPI2_SMS_GetRecordNumberOfReplaceSMS_Req.tp_pid = tp_pid;
	req.req_rep_u.CAPI2_SMS_GetRecordNumberOfReplaceSMS_Req.oaddress = oaddress;
	req.respId = MSG_SMS_GETRECORDNUMBEROFREPLACESMS_RSP;
	msg.msgId = MSG_SMS_GETRECORDNUMBEROFREPLACESMS_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}
#endif


//***************** < 5 > **********************






bool_t xdr_CAPI2_SMS_IsMeStorageEnabled_Rsp_t(void* xdrs, CAPI2_SMS_IsMeStorageEnabled_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_IsMeStorageEnabled_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_SMS_GetMaxMeCapacity_Rsp_t(void* xdrs, CAPI2_SMS_GetMaxMeCapacity_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_GetMaxMeCapacity_Rsp_t")

	 return _xdr_UInt16(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_SMS_GetNextFreeSlot_Rsp_t(void* xdrs, CAPI2_SMS_GetNextFreeSlot_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_GetNextFreeSlot_Rsp_t")

	 return _xdr_UInt16(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_SMS_SetMeSmsStatus_Req_t(void* xdrs, CAPI2_SMS_SetMeSmsStatus_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_SetMeSmsStatus_Req_t")

	if(
		_xdr_UInt16(xdrs, &rsp->slotNumber,"slotNumber") &&
		xdr_SIMSMSMesgStatus_t(xdrs, &rsp->status) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SMS_SetMeSmsStatus_Rsp_t(void* xdrs, CAPI2_SMS_SetMeSmsStatus_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_SetMeSmsStatus_Rsp_t")

	 return xdr_Result_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SMS_GetMeSmsStatus_Req_t(void* xdrs, CAPI2_SMS_GetMeSmsStatus_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_GetMeSmsStatus_Req_t")

	if(
		_xdr_UInt16(xdrs, &rsp->slotNumber,"slotNumber") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SMS_GetMeSmsStatus_Rsp_t(void* xdrs, CAPI2_SMS_GetMeSmsStatus_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_GetMeSmsStatus_Rsp_t")

	 return xdr_SIMSMSMesgStatus_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SMS_StoreSmsToMe_Rsp_t(void* xdrs, CAPI2_SMS_StoreSmsToMe_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_StoreSmsToMe_Rsp_t")

	 return xdr_Result_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SMS_RetrieveSmsFromMe_Req_t(void* xdrs, CAPI2_SMS_RetrieveSmsFromMe_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_RetrieveSmsFromMe_Req_t")

	if(
		_xdr_UInt16(xdrs, &rsp->slotNumber,"slotNumber") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SMS_RetrieveSmsFromMe_Rsp_t(void* xdrs, CAPI2_SMS_RetrieveSmsFromMe_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_RetrieveSmsFromMe_Rsp_t")

	 return xdr_Result_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SMS_RemoveSmsFromMe_Req_t(void* xdrs, CAPI2_SMS_RemoveSmsFromMe_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_RemoveSmsFromMe_Req_t")

	if(
		_xdr_UInt16(xdrs, &rsp->slotNumber,"slotNumber") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SMS_RemoveSmsFromMe_Rsp_t(void* xdrs, CAPI2_SMS_RemoveSmsFromMe_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_RemoveSmsFromMe_Rsp_t")

	 return xdr_Result_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SMS_IsSmsReplSupported_Rsp_t(void* xdrs, CAPI2_SMS_IsSmsReplSupported_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_IsSmsReplSupported_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_SMS_GetMeSmsBufferStatus_Req_t(void* xdrs, CAPI2_SMS_GetMeSmsBufferStatus_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_GetMeSmsBufferStatus_Req_t")

	if(
		_xdr_UInt16(xdrs, &rsp->cmd,"cmd") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SMS_GetMeSmsBufferStatus_RSP_Rsp_t(void* xdrs, CAPI2_SMS_GetMeSmsBufferStatus_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_GetMeSmsBufferStatus_RSP_Rsp_t")
if(
		_xdr_UInt16(xdrs, &rsp->bfree,"bfree") &&
		_xdr_UInt16(xdrs, &rsp->bused,"bused") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SMS_GetRecordNumberOfReplaceSMS_Req_t(void* xdrs, CAPI2_SMS_GetRecordNumberOfReplaceSMS_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_GetRecordNumberOfReplaceSMS_Req_t")

	if(
		xdr_SmsStorage_t(xdrs, &rsp->storageType) &&
		_xdr_UInt8(xdrs, &rsp->tp_pid,"tp_pid") &&
		xdr_uchar_ptr_t(xdrs, &rsp->oaddress) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SMS_GetRecordNumberOfReplaceSMS_Rsp_t(void* xdrs, CAPI2_SMS_GetRecordNumberOfReplaceSMS_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_GetRecordNumberOfReplaceSMS_Rsp_t")

	 return _xdr_UInt16(xdrs, &rsp->val,"val");
}

//***************** < 9 > **********************





// **FIXME** MAG - I know this is generated code and not to be modified, but for 
// now I'm modifying it just for Athena bringup. This should NOT be checked in to the 
// "real" branch (maybe just for bringup); final fix has to be sorted out with Naveen - 
// maybe just always add these "UNDER_LINUX" wrappers around generated AP handler functions?
#ifndef UNDER_LINUX

#if defined(FUSE_APPS_PROCESSOR) 

Result_t Handle_CAPI2_SMS_IsMeStorageEnabled(RPC_Msg_t* pReqMsg)
{
	Result_t result = RESULT_OK;
	CAPI_ReqRep_t data;

	memset(&data, 0, sizeof(CAPI_ReqRep_t));
	data.req_rep_u.CAPI2_SMS_IsMeStorageEnabled_Rsp.val = (Boolean)SMS_IsMeStorageEnabled();

	data.result = result;
	Send_CAPI_RspForRequest(pReqMsg, MSG_SMS_ISMESTORAGEENABLED_RSP, &data);
	return result;
}

Result_t Handle_CAPI2_SMS_GetMaxMeCapacity(RPC_Msg_t* pReqMsg)
{
	Result_t result = RESULT_OK;
	CAPI_ReqRep_t data;

	memset(&data, 0, sizeof(CAPI_ReqRep_t));
	data.req_rep_u.CAPI2_SMS_GetMaxMeCapacity_Rsp.val = (UInt16)SMS_GetMaxMeCapacity();

	data.result = result;
	Send_CAPI_RspForRequest(pReqMsg, MSG_SMS_GETMAXMECAPACITY_RSP, &data);
	return result;
}

Result_t Handle_CAPI2_SMS_GetNextFreeSlot(RPC_Msg_t* pReqMsg)
{
	Result_t result = RESULT_OK;
	CAPI_ReqRep_t data;

	memset(&data, 0, sizeof(CAPI_ReqRep_t));
	data.req_rep_u.CAPI2_SMS_GetNextFreeSlot_Rsp.val = (UInt16)SMS_GetNextFreeSlot();

	data.result = result;
	Send_CAPI_RspForRequest(pReqMsg, MSG_SMS_GETNEXTFREESLOT_RSP, &data);
	return result;
}

Result_t Handle_CAPI2_SMS_SetMeSmsStatus(RPC_Msg_t* pReqMsg, UInt16 slotNumber, SIMSMSMesgStatus_t status)
{
	Result_t result = RESULT_OK;
	CAPI_ReqRep_t data;

	memset(&data, 0, sizeof(CAPI_ReqRep_t));
	data.req_rep_u.CAPI2_SMS_SetMeSmsStatus_Rsp.val = (Result_t)SMS_SetMeSmsStatus(slotNumber,status);

	data.result = result;
	Send_CAPI_RspForRequest(pReqMsg, MSG_SMS_SETMESMSSTATUS_RSP, &data);
	return result;
}
#endif


Result_t Handle_CAPI2_SMS_StoreSmsToMe(RPC_Msg_t* pReqMsg, UInt8 *inSms, UInt16 inLength, SIMSMSMesgStatus_t status, UInt16 slotNumber)
{
	Result_t result = RESULT_OK;
	CAPI_ReqRep_t data;

	memset(&data, 0, sizeof(CAPI_ReqRep_t));
	data.req_rep_u.CAPI2_SMS_StoreSmsToMe_Rsp.val = (Result_t)SMS_StoreSmsToMe(inSms,inLength,status,slotNumber);

	data.result = result;
	Send_CAPI_RspForRequest(pReqMsg, MSG_SMS_STORESMSTOME_RSP, &data);
	return result;
}

#if defined(FUSE_APPS_PROCESSOR) 

Result_t Handle_CAPI2_SMS_RetrieveSmsFromMe(RPC_Msg_t* pReqMsg, UInt16 slotNumber)
{
	Result_t result = RESULT_OK;
	CAPI_ReqRep_t data;

	memset(&data, 0, sizeof(CAPI_ReqRep_t));
	data.req_rep_u.CAPI2_SMS_RetrieveSmsFromMe_Rsp.val = (Result_t)SMS_RetrieveSmsFromMe(slotNumber);

	data.result = result;
	Send_CAPI_RspForRequest(pReqMsg, MSG_SMS_RETRIEVESMSFROMME_RSP, &data);
	return result;
}

Result_t Handle_CAPI2_SMS_RemoveSmsFromMe(RPC_Msg_t* pReqMsg, UInt16 slotNumber)
{
	Result_t result = RESULT_OK;
	CAPI_ReqRep_t data;

	memset(&data, 0, sizeof(CAPI_ReqRep_t));
	data.req_rep_u.CAPI2_SMS_RemoveSmsFromMe_Rsp.val = (Result_t)SMS_RemoveSmsFromMe(slotNumber);

	data.result = result;
	Send_CAPI_RspForRequest(pReqMsg, MSG_SMS_REMOVESMSFROMME_RSP, &data);
	return result;
}

Result_t Handle_CAPI2_SMS_IsSmsReplSupported(RPC_Msg_t* pReqMsg)
{
	Result_t result = RESULT_OK;
	CAPI_ReqRep_t data;

	memset(&data, 0, sizeof(CAPI_ReqRep_t));
	data.req_rep_u.CAPI2_SMS_IsSmsReplSupported_Rsp.val = (Boolean)SMS_IsSmsReplSupported();

	data.result = result;
	Send_CAPI_RspForRequest(pReqMsg, MSG_SMS_ISSMSREPLACE_SUPPORTED_RSP, &data);
	return result;
}

Result_t Handle_CAPI2_SMS_GetRecordNumberOfReplaceSMS(RPC_Msg_t* pReqMsg, SmsStorage_t storageType, UInt8 tp_pid, uchar_ptr_t oaddress)
{
	Result_t result = RESULT_OK;
	CAPI_ReqRep_t data;

	memset(&data, 0, sizeof(CAPI_ReqRep_t));
	data.req_rep_u.CAPI2_SMS_GetRecordNumberOfReplaceSMS_Rsp.val = (UInt16)SMS_GetRecordNumberOfReplaceSMS(storageType,tp_pid,oaddress);

	data.result = result;
	Send_CAPI_RspForRequest(pReqMsg, MSG_SMS_GETRECORDNUMBEROFREPLACESMS_RSP, &data);
	return result;
}
#endif
#endif // UNDER_LINUX


//***************** < 10 > **********************



Result_t CAPI_GenCommsMsgHnd(RPC_Msg_t* pReqMsg, CAPI_ReqRep_t* req)
{
	Result_t result = RESULT_OK;
	switch(pReqMsg->msgId)
	{



#if defined(FUSE_APPS_PROCESSOR) 
	case MSG_SMS_ISMESTORAGEENABLED_REQ:
		result = Handle_CAPI2_SMS_IsMeStorageEnabled(pReqMsg);
		break;
	case MSG_SMS_GETMAXMECAPACITY_REQ:
		result = Handle_CAPI2_SMS_GetMaxMeCapacity(pReqMsg);
		break;
	case MSG_SMS_GETNEXTFREESLOT_REQ:
		result = Handle_CAPI2_SMS_GetNextFreeSlot(pReqMsg);
		break;
	case MSG_SMS_SETMESMSSTATUS_REQ:
		result = Handle_CAPI2_SMS_SetMeSmsStatus(pReqMsg,req->req_rep_u.CAPI2_SMS_SetMeSmsStatus_Req.slotNumber,req->req_rep_u.CAPI2_SMS_SetMeSmsStatus_Req.status);
		break;
	case MSG_SMS_GETMESMSSTATUS_REQ:
		result = Handle_CAPI2_SMS_GetMeSmsStatus(pReqMsg,req->req_rep_u.CAPI2_SMS_GetMeSmsStatus_Req.slotNumber);
		break;
#endif

	case MSG_SMS_STORESMSTOME_REQ:
		result = Handle_CAPI2_SMS_StoreSmsToMe(pReqMsg,req->req_rep_u.CAPI2_SMS_StoreSmsToMe_Req.inSms,req->req_rep_u.CAPI2_SMS_StoreSmsToMe_Req.inLength,req->req_rep_u.CAPI2_SMS_StoreSmsToMe_Req.status,req->req_rep_u.CAPI2_SMS_StoreSmsToMe_Req.slotNumber);
		break;

#if defined(FUSE_APPS_PROCESSOR) 
	case MSG_SMS_RETRIEVESMSFROMME_REQ:
		result = Handle_CAPI2_SMS_RetrieveSmsFromMe(pReqMsg,req->req_rep_u.CAPI2_SMS_RetrieveSmsFromMe_Req.slotNumber);
		break;
	case MSG_SMS_REMOVESMSFROMME_REQ:
		result = Handle_CAPI2_SMS_RemoveSmsFromMe(pReqMsg,req->req_rep_u.CAPI2_SMS_RemoveSmsFromMe_Req.slotNumber);
		break;
	case MSG_SMS_ISSMSREPLACE_SUPPORTED_REQ:
		result = Handle_CAPI2_SMS_IsSmsReplSupported(pReqMsg);
		break;
	case MSG_SMS_GETMESMS_BUF_STATUS_REQ:
		result = Handle_CAPI2_SMS_GetMeSmsBufferStatus(pReqMsg,req->req_rep_u.CAPI2_SMS_GetMeSmsBufferStatus_Req.cmd);
		break;
	case MSG_SMS_GETRECORDNUMBEROFREPLACESMS_REQ:
		result = Handle_CAPI2_SMS_GetRecordNumberOfReplaceSMS(pReqMsg,req->req_rep_u.CAPI2_SMS_GetRecordNumberOfReplaceSMS_Req.storageType,req->req_rep_u.CAPI2_SMS_GetRecordNumberOfReplaceSMS_Req.tp_pid,req->req_rep_u.CAPI2_SMS_GetRecordNumberOfReplaceSMS_Req.oaddress);
		break;
#endif

	default:
		break;
	}
	return result;
}

//***************** < 11 > **********************


void CAPI_GenGetPayloadInfo(void* dataBuf, MsgType_t msgType, void** ppBuf, UInt32* len)
{
	switch(msgType)
	{


	case MSG_SMS_ISMESTORAGEENABLED_RSP:
	{
		CAPI2_SMS_IsMeStorageEnabled_Rsp_t* pVal = (CAPI2_SMS_IsMeStorageEnabled_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SMS_GETMAXMECAPACITY_RSP:
	{
		CAPI2_SMS_GetMaxMeCapacity_Rsp_t* pVal = (CAPI2_SMS_GetMaxMeCapacity_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SMS_GETNEXTFREESLOT_RSP:
	{
		CAPI2_SMS_GetNextFreeSlot_Rsp_t* pVal = (CAPI2_SMS_GetNextFreeSlot_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SMS_SETMESMSSTATUS_RSP:
	{
		CAPI2_SMS_SetMeSmsStatus_Rsp_t* pVal = (CAPI2_SMS_SetMeSmsStatus_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SMS_GETMESMSSTATUS_RSP:
	{
		CAPI2_SMS_GetMeSmsStatus_Rsp_t* pVal = (CAPI2_SMS_GetMeSmsStatus_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SMS_STORESMSTOME_RSP:
	{
		CAPI2_SMS_StoreSmsToMe_Rsp_t* pVal = (CAPI2_SMS_StoreSmsToMe_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SMS_RETRIEVESMSFROMME_RSP:
	{
		CAPI2_SMS_RetrieveSmsFromMe_Rsp_t* pVal = (CAPI2_SMS_RetrieveSmsFromMe_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SMS_REMOVESMSFROMME_RSP:
	{
		CAPI2_SMS_RemoveSmsFromMe_Rsp_t* pVal = (CAPI2_SMS_RemoveSmsFromMe_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SMS_ISSMSREPLACE_SUPPORTED_RSP:
	{
		CAPI2_SMS_IsSmsReplSupported_Rsp_t* pVal = (CAPI2_SMS_IsSmsReplSupported_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SMS_GETMESMS_BUF_STATUS_RSP:
	{
		CAPI2_SMS_GetMeSmsBufferStatus_RSP_Rsp_t* pVal = (CAPI2_SMS_GetMeSmsBufferStatus_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SMS_GETRECORDNUMBEROFREPLACESMS_RSP:
	{
		CAPI2_SMS_GetRecordNumberOfReplaceSMS_Rsp_t* pVal = (CAPI2_SMS_GetRecordNumberOfReplaceSMS_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	default:xassert(0,msgType);
		break;
	}
	return;
}

//***************** < 13 > **********************






//***************** < 14 > **********************




//***************** < 15 > **********************






//***************** < 17 > **********************



#ifdef DEVELOPMENT_MODEM_CAPI_WIN_UNIT_TEST 
#define _D(a) _ ## a 
#else 
#define _D(a) a
#endif


