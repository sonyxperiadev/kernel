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

#include "capi2_reqrep.h"
#include "capi2_lcs_cplane_api.h"

void CAPI2_LCS_SendRrlpDataToNetwork(UInt32 tid, UInt8 clientID, LcsClientInfo_t inRrlpHandler, const UInt8 *inData, UInt32 inDataLen)
{
    CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_LCS_RrlpData_Req.mHandler = inRrlpHandler;
	req.req_rep_u.CAPI2_LCS_RrlpData_Req.mData = inData;
	req.req_rep_u.CAPI2_LCS_RrlpData_Req.mDataLen = inDataLen;
	req.respId = MSG_LCS_SEND_RRLP_DATA_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_LCS_SEND_RRLP_DATA_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_LCS_RrcMeasurementReport(UInt32 tid, UInt8 clientID, 
										   LcsClientInfo_t inRrcHandler, 
										   UInt16 inMeasurementId, 
                                           UInt8* inMeasData, 
                                           UInt32 inMeasDataLen, 
                                           UInt8* inEventData,
                                           UInt32 inEventDataLen,
                                           Boolean inAddMeasResult)
{
    CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_LCS_RrcMeasReport_Req.mRrcHandler = inRrcHandler;
	req.req_rep_u.CAPI2_LCS_RrcMeasReport_Req.mMeasurementId = inMeasurementId;
	req.req_rep_u.CAPI2_LCS_RrcMeasReport_Req.mMeasData = inMeasData;
	req.req_rep_u.CAPI2_LCS_RrcMeasReport_Req.mMeasDataLen = inMeasDataLen;
	req.req_rep_u.CAPI2_LCS_RrcMeasReport_Req.mEventData = inEventData;
	req.req_rep_u.CAPI2_LCS_RrcMeasReport_Req.mEventDataLen = inEventDataLen;
	req.req_rep_u.CAPI2_LCS_RrcMeasReport_Req.mAddMeasResult = inAddMeasResult;

	req.respId = MSG_LCS_RRC_MEAS_REPORT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_LCS_RRC_MEAS_REPORT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_LCS_RrcMeasurementControlFailure(UInt32 tid, UInt8 clientID, 
												   LcsClientInfo_t inRrcHandler, 
												   UInt16 inTransactionId, 
												   LcsRrcMcFailure_t inMcFailure, 
												   UInt32 inErrorCode)
{
    CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_LCS_RrcMeasFailure_Req.mRrcHandler = inRrcHandler;
	req.req_rep_u.CAPI2_LCS_RrcMeasFailure_Req.mTransactionId = inTransactionId;
	req.req_rep_u.CAPI2_LCS_RrcMeasFailure_Req.mMcFailure = inMcFailure;
	req.req_rep_u.CAPI2_LCS_RrcMeasFailure_Req.mErrorCode = inErrorCode;

	req.respId = MSG_LCS_RRC_MEAS_FAILURE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_LCS_RRC_MEAS_FAILURE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_LCS_RrcStatus(UInt32 tid, UInt8 clientID, LcsClientInfo_t inRrcHandler, LcsRrcMcStatus_t inStatus)
{
    CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_LCS_RrcStatus_Req.mRrcHandler = inRrcHandler;
	req.req_rep_u.CAPI2_LCS_RrcStatus_Req.mStatus = inStatus;

	req.respId = MSG_LCS_RRC_STATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_LCS_RRC_STATUS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

/*
void CAPI2_LCS_RegisterSuplMsgHandler(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_LCS_RegisterSuplMsgHandler_Req.inClientID = clientID;
	req.msgType = CAPI2_TYPE_REQUEST;
	req.respId = MSG_LCS_REGISTER_SUPL_MSG_HANDLER_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_LCS_REGISTER_SUPL_MSG_HANDLER_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_LCS_StartPosReqPeriodic(UInt32 tid, UInt8 clientID, UInt32 inPosOutputMask, UInt32 inPosFixPeriod)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_LCS_StartPosReqPeriodic_Req.inClientID = clientID;
	req.req_rep_u.CAPI2_LCS_StartPosReqPeriodic_Req.inPosOutputMask = inPosOutputMask;
	req.req_rep_u.CAPI2_LCS_StartPosReqPeriodic_Req.inPosFixPeriod = inPosFixPeriod;
	req.msgType = CAPI2_TYPE_REQUEST;
	req.respId = MSG_LCS_START_POS_REQ_PERIODIC_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_LCS_START_POS_REQ_PERIODIC_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_LCS_StartPosReqSingle(UInt32 tid, UInt8 clientID, UInt32 inPosOutputMask, UInt32 inAccuracyInMeters, UInt32 inPosFixTimeOut)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	req.req_rep_u.CAPI2_LCS_StartPosReqSingle_Req.inClientID = clientID;
	req.req_rep_u.CAPI2_LCS_StartPosReqSingle_Req.inPosOutputMask = inPosOutputMask;
	req.req_rep_u.CAPI2_LCS_StartPosReqSingle_Req.inAccuracyInMeters = inAccuracyInMeters;
	req.req_rep_u.CAPI2_LCS_StartPosReqSingle_Req.inPosFixTimeOut = inPosFixTimeOut;
	req.msgType = CAPI2_TYPE_REQUEST;
	req.respId = MSG_LCS_START_POS_REQ_SINGLE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_LCS_START_POS_REQ_SINGLE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}
void CAPI2_LCS_SuplInitHmacRsp(UInt32 tid, UInt8 clientID, LcsSuplSessionHdl_t inSessionHdl, unsigned char *	inSuplInitData, UInt32 inSuplInitDataLen, SuplHmac_t inSuplHmac )
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_LCS_SuplInitHmacRsp_Req.sessionHdl = inSessionHdl;
	req.req_rep_u.CAPI2_LCS_SuplInitHmacRsp_Req.suplInitData = inSuplInitData;
	req.req_rep_u.CAPI2_LCS_SuplInitHmacRsp_Req.suplInitDataLen = inSuplInitDataLen;
	memcpy(req.req_rep_u.CAPI2_LCS_SuplInitHmacRsp_Req.suplHmac, inSuplHmac, sizeof(SuplHmac_t));
	req.msgType = CAPI2_TYPE_REQUEST;
	req.respId = MSG_LCS_SUPL_INIT_HMAC_RSP_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_LCS_SUPL_INIT_HMAC_RSP_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_LCS_SuplDataAvailable(UInt32 tid, UInt8 clientID, LcsSuplSessionHdl_t inSessionHdl, LcsSuplConnectHdl_t inConnectHdl, const UInt8 *inData, UInt32 inDataLen)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_LCS_SuplDataAvailable_Req.mSessionHdl = inSessionHdl;
	req.req_rep_u.CAPI2_LCS_SuplDataAvailable_Req.mConnectHdl = inConnectHdl;
	req.req_rep_u.CAPI2_LCS_SuplDataAvailable_Req.mData = (unsigned char*)inData;
	req.req_rep_u.CAPI2_LCS_SuplDataAvailable_Req.mDataLen = inDataLen;
	req.msgType = CAPI2_TYPE_REQUEST;
	req.respId = MSG_LCS_SUPL_DATA_AVAILABLE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_LCS_SUPL_DATA_AVAILABLE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}
*/
