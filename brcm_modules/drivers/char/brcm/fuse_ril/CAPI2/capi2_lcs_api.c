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
#include "capi2_lcs_api.h"

void CAPI2_LCS_RegisterSuplMsgHandler(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_LCS_RegisterSuplMsgHandler_Req.inClientID = clientID;
//	req.msgType = CAPI2_TYPE_REQUEST;
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
//	req.msgType = CAPI2_TYPE_REQUEST;
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
//	req.msgType = CAPI2_TYPE_REQUEST;
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
//	req.msgType = CAPI2_TYPE_REQUEST;
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
//	req.msgType = CAPI2_TYPE_REQUEST;
	req.respId = MSG_LCS_SUPL_DATA_AVAILABLE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_LCS_SUPL_DATA_AVAILABLE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_LCS_CpMoLrReq(UInt32 tid, UInt8 clientID, ClientInfo_t *inClientInfoPtr, const LcsCpMoLrReq_t *inCpMoLrReq)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_LCS_CpMoLrReq_Req.inClientInfoPtr = inClientInfoPtr;
	req.req_rep_u.CAPI2_LCS_CpMoLrReq_Req.inCpMoLrReq = inCpMoLrReq;

	req.respId = MSG_MNSS_CLIENT_LCS_SRV_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_LCS_CPMOLRREQ_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

void CAPI2_LCS_CpMoLrAbort(UInt32 tid, UInt8 clientID, ClientInfo_t *inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_LCS_CpMoLrAbort_Req.inClientInfoPtr = inClientInfoPtr;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_LCS_CPMOLRABORT_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

void CAPI2_LCS_CpMtLrVerificationRsp(UInt32 tid, UInt8 clientID, ClientInfo_t *inClientInfoPtr, LCS_VerifRsp_t inVerificationRsp)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_LCS_CpMtLrVerificationRsp_Req.inClientInfoPtr = inClientInfoPtr;
	req.req_rep_u.CAPI2_LCS_CpMtLrVerificationRsp_Req.inVerificationRsp = inVerificationRsp;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_LCS_CPMTLRVERIFICATIONRSP_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

void CAPI2_LCS_CpMtLrRsp(UInt32 tid, UInt8 clientID, ClientInfo_t *inClientInfoPtr, SS_Operation_t inOperation, Boolean inIsAccepted)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_LCS_CpMtLrRsp_Req.inClientInfoPtr = inClientInfoPtr;
	req.req_rep_u.CAPI2_LCS_CpMtLrRsp_Req.inOperation = inOperation;
	req.req_rep_u.CAPI2_LCS_CpMtLrRsp_Req.inIsAccepted = inIsAccepted;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_LCS_CPMTLRRSP_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

void CAPI2_LCS_CpLocUpdateRsp(UInt32 tid, UInt8 clientID, ClientInfo_t *inClientInfoPtr, const LCS_TermCause_t *inTerminationCause)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_LCS_CpLocUpdateRsp_Req.inClientInfoPtr = inClientInfoPtr;
	req.req_rep_u.CAPI2_LCS_CpLocUpdateRsp_Req.inTerminationCause = inTerminationCause;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_LCS_CPLOCUPDATERSP_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

void CAPI2_LCS_DecodePosEstimate(UInt32 tid, UInt8 clientID, const LCS_LocEstimate_t *inLocEstData)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_LCS_DecodePosEstimate_Req.inLocEstData = inLocEstData;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_LCS_DECODEPOSESTIMATE_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

void CAPI2_LCS_EncodeAssistanceReq(UInt32 tid, UInt8 clientID, const LcsAssistanceReq_t *inAssistReq)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_LCS_EncodeAssistanceReq_Req.inAssistReq = inAssistReq;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_LCS_ENCODEASSISTANCEREQ_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

