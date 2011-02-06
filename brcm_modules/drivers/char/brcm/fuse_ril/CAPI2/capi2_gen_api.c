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
#include "mobcom_types.h"

#include "resultcode.h"

#include "capi2_reqrep.h"




void CAPI2_MS_IsGSMRegistered(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GSM_REGISTERED_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GSM_REGISTERED_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_IsGPRSRegistered(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GPRS_REGISTERED_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GPRS_REGISTERED_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_GetGSMRegCause(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GSM_CAUSE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GSM_CAUSE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_GetGPRSRegCause(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GPRS_CAUSE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GPRS_CAUSE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_GetRegisteredLAC(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_REGISTERED_LAC_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_REGISTERED_LAC_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_GetPlmnMCC(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GET_PLMN_MCC_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GET_PLMN_MCC_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_GetPlmnMNC(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GET_PLMN_MNC_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GET_PLMN_MNC_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYS_ProcessPowerDownReq(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYS_POWERDOWN_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYS_POWERDOWN_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYS_ProcessNoRfReq(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYS_POWERUP_NORF_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYS_POWERUP_NORF_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYS_ProcessPowerUpReq(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYS_POWERUP_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYS_POWERUP_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYS_GetMSPowerOnCause(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYS_POWERON_CAUSE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYS_POWERON_CAUSE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_IsGprsAllowed(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GPRS_ALLOWED_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GPRS_ALLOWED_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_GetCurrentRAT(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GET_CURRENT_RAT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GET_CURRENT_RAT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_GetCurrentBand(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GET_CURRENT_BAND_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GET_CURRENT_BAND_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_SetStartBand(UInt32 tid, UInt8 clientID, UInt8 startBand)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_MS_SetStartBand_Req.startBand = startBand;
	req.respId = MSG_MS_START_BAND_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_START_BAND_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_UpdateSMSCapExceededFlag(UInt32 tid, UInt8 clientID, Boolean cap_exceeded)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIM_UpdateSMSCapExceededFlag_Req.cap_exceeded = cap_exceeded;
	req.respId = MSG_SIM_UPDATE_SMSCAPEXC_FLAG_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_UPDATE_SMSCAPEXC_FLAG_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYS_SelectBand(UInt32 tid, UInt8 clientID, BandSelect_t bandSelect)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SYS_SelectBand_Req.bandSelect = bandSelect;
	req.respId = MSG_MS_SELECT_BAND_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_SELECT_BAND_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_SetSupportedRATandBand(UInt32 tid, UInt8 clientID, RATSelect_t RAT_cap, BandSelect_t band_cap)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_MS_SetSupportedRATandBand_Req.RAT_cap = RAT_cap;
	req.req_rep_u.CAPI2_MS_SetSupportedRATandBand_Req.band_cap = band_cap;
	req.respId = MSG_MS_SET_RAT_BAND_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_SET_RAT_BAND_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_PLMN_GetCountryByMcc(UInt32 tid, UInt8 clientID, UInt16 mcc)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PLMN_GetCountryByMcc_Req.mcc = mcc;
	req.respId = MSG_MS_GET_MCC_COUNTRY_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GET_MCC_COUNTRY_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_GetPLMNEntryByIndex(UInt32 tid, UInt8 clientID, UInt16 index, Boolean ucs2)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_MS_GetPLMNEntryByIndex_Req.index = index;
	req.req_rep_u.CAPI2_MS_GetPLMNEntryByIndex_Req.ucs2 = ucs2;
	req.respId = MSG_MS_PLMN_INFO_BY_CODE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_PLMN_INFO_BY_CODE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_GetPLMNListSize(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_PLMN_LIST_SIZE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_PLMN_LIST_SIZE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_GetPLMNByCode(UInt32 tid, UInt8 clientID, Boolean ucs2, UInt16 plmn_mcc, UInt16 plmn_mnc)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_MS_GetPLMNByCode_Req.ucs2 = ucs2;
	req.req_rep_u.CAPI2_MS_GetPLMNByCode_Req.plmn_mcc = plmn_mcc;
	req.req_rep_u.CAPI2_MS_GetPLMNByCode_Req.plmn_mnc = plmn_mnc;
	req.respId = MSG_MS_PLMN_INFO_BY_INDEX_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_PLMN_INFO_BY_INDEX_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_PlmnSelect(UInt32 tid, UInt8 clientID, Boolean ucs2, PlmnSelectMode_t selectMode, PlmnSelectFormat_t format, char_ptr_t plmnValue)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_MS_PlmnSelect_Req.ucs2 = ucs2;
	req.req_rep_u.CAPI2_MS_PlmnSelect_Req.selectMode = selectMode;
	req.req_rep_u.CAPI2_MS_PlmnSelect_Req.format = format;
	req.req_rep_u.CAPI2_MS_PlmnSelect_Req.plmnValue = plmnValue;
	req.respId = MSG_MS_PLMN_SELECT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_PLMN_SELECT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_AbortPlmnSelect(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_PLMN_ABORT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_PLMN_ABORT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_GetPlmnMode(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GET_PLMN_MODE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GET_PLMN_MODE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_SetPlmnMode(UInt32 tid, UInt8 clientID, PlmnSelectMode_t mode)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_MS_SetPlmnMode_Req.mode = mode;
	req.respId = MSG_MS_SET_PLMN_MODE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_SET_PLMN_MODE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_GetPlmnFormat(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GET_PLMN_FORMAT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GET_PLMN_FORMAT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_SetPlmnFormat(UInt32 tid, UInt8 clientID, PlmnSelectFormat_t format)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_MS_SetPlmnFormat_Req.format = format;
	req.respId = MSG_MS_SET_PLMN_FORMAT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_SET_PLMN_FORMAT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_IsMatchedPLMN(UInt32 tid, UInt8 clientID, UInt16 net_mcc, UInt8 net_mnc, UInt16 mcc, UInt8 mnc)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_MS_IsMatchedPLMN_Req.net_mcc = net_mcc;
	req.req_rep_u.CAPI2_MS_IsMatchedPLMN_Req.net_mnc = net_mnc;
	req.req_rep_u.CAPI2_MS_IsMatchedPLMN_Req.mcc = mcc;
	req.req_rep_u.CAPI2_MS_IsMatchedPLMN_Req.mnc = mnc;
	req.respId = MSG_MS_MATCH_PLMN_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_MATCH_PLMN_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_SearchAvailablePLMN(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_SEARCH_PLMN_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_SEARCH_PLMN_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_AbortSearchPLMN(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_ABORT_PLMN_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_ABORT_PLMN_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_AutoSearchReq(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_AUTO_SEARCH_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_AUTO_SEARCH_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_GetPLMNNameByCode(UInt32 tid, UInt8 clientID, UInt16 mcc, UInt8 mnc, UInt16 lac, Boolean ucs2)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_MS_GetPLMNNameByCode_Req.mcc = mcc;
	req.req_rep_u.CAPI2_MS_GetPLMNNameByCode_Req.mnc = mnc;
	req.req_rep_u.CAPI2_MS_GetPLMNNameByCode_Req.lac = lac;
	req.req_rep_u.CAPI2_MS_GetPLMNNameByCode_Req.ucs2 = ucs2;
	req.respId = MSG_MS_PLMN_NAME_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_PLMN_NAME_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYS_IsResetCausedByAssert(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYS_QUERY_RESET_CAUSE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYS_QUERY_RESET_CAUSE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYS_GetSystemState(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYS_GET_SYSTEM_STATE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYS_GET_SYSTEM_STATE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYS_SetSystemState(UInt32 tid, UInt8 clientID, SystemState_t state)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SYS_SetSystemState_Req.state = state;
	req.respId = MSG_SYS_SET_SYSTEM_STATE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYS_SET_SYSTEM_STATE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYS_GetRxSignalInfo(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYS_GET_RX_LEVEL_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYS_GET_RX_LEVEL_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYS_GetGSMRegistrationStatus(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYS_GET_GSMREG_STATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYS_GET_GSMREG_STATUS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYS_GetGPRSRegistrationStatus(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYS_GET_GPRSREG_STATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYS_GET_GPRSREG_STATUS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYS_IsRegisteredGSMOrGPRS(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYS_GET_REG_STATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYS_GET_REG_STATUS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYS_GetGSMRegistrationCause(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYS_GET_GSMREG_CAUSE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYS_GET_GSMREG_CAUSE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_IsPlmnForbidden(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYS_IS_PLMN_FORBIDDEN_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYS_IS_PLMN_FORBIDDEN_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_IsRegisteredHomePLMN(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_HOME_PLMN_REG_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_HOME_PLMN_REG_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_SetPowerDownTimer(UInt32 tid, UInt8 clientID, UInt8 powerDownTimer)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_MS_SetPowerDownTimer_Req.powerDownTimer = powerDownTimer;
	req.respId = MSG_SET_POWER_DOWN_TIMER_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SET_POWER_DOWN_TIMER_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIMLockIsLockOn(UInt32 tid, UInt8 clientID, SIMLockType_t lockType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIMLockIsLockOn_Req.lockType = lockType;
	req.respId = MSG_SIM_LOCK_ON_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_LOCK_ON_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIMLockCheckAllLocks(UInt32 tid, UInt8 clientID, uchar_ptr_t imsi, uchar_ptr_t gid1, uchar_ptr_t gid2)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIMLockCheckAllLocks_Req.imsi = imsi;
	req.req_rep_u.CAPI2_SIMLockCheckAllLocks_Req.gid1 = gid1;
	req.req_rep_u.CAPI2_SIMLockCheckAllLocks_Req.gid2 = gid2;
	req.respId = MSG_SIM_LOCK_STATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_LOCK_STATUS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIMLockUnlockSIM(UInt32 tid, UInt8 clientID, SIMLockType_t lockType, uchar_ptr_t key)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIMLockUnlockSIM_Req.lockType = lockType;
	req.req_rep_u.CAPI2_SIMLockUnlockSIM_Req.key = key;
	req.respId = MSG_SIM_LOCK_UNLOCK_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_LOCK_UNLOCK_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIMLockGetCurrentClosedLock(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_LOCK_CLOSED_LOCK_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_LOCK_CLOSED_LOCK_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIMLockChangePasswordPHSIM(UInt32 tid, UInt8 clientID, uchar_ptr_t old_pwd, uchar_ptr_t new_pwd)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIMLockChangePasswordPHSIM_Req.old_pwd = old_pwd;
	req.req_rep_u.CAPI2_SIMLockChangePasswordPHSIM_Req.new_pwd = new_pwd;
	req.respId = MSG_SIM_LOCK_CHANGE_PWD_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_LOCK_CHANGE_PWD_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIMLockCheckPasswordPHSIM(UInt32 tid, UInt8 clientID, uchar_ptr_t pwd)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIMLockCheckPasswordPHSIM_Req.pwd = pwd;
	req.respId = MSG_SIM_LOCK_CHECK_PWD_PHSIM_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_LOCK_CHECK_PWD_PHSIM_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIMLockGetSignature(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_LOCK_LOCK_SIG_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_LOCK_LOCK_SIG_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIMLockGetImeiSecboot(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_LOCK_GET_IMEI_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_LOCK_GET_IMEI_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_GetSmsParamRecNum(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_PARAM_REC_NUM_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_PARAM_REC_NUM_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_GetSmsMemExceededFlag(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_GET_SMSMEMEXC_FLAG_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_GET_SMSMEMEXC_FLAG_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_IsPINRequired(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_PIN_REQ_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_PIN_REQ_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_GetCardPhase(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_CARD_PHASE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_CARD_PHASE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_GetSIMType(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_TYPE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_TYPE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_GetPresentStatus(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_PRESENT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_PRESENT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_IsOperationRestricted(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_PIN_OPERATION_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_PIN_OPERATION_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_IsPINBlocked(UInt32 tid, UInt8 clientID, CHV_t chv)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIM_IsPINBlocked_Req.chv = chv;
	req.respId = MSG_SIM_PIN_BLOCK_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_PIN_BLOCK_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_IsPUKBlocked(UInt32 tid, UInt8 clientID, CHV_t chv)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIM_IsPUKBlocked_Req.chv = chv;
	req.respId = MSG_SIM_PUK_BLOCK_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_PUK_BLOCK_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_IsInvalidSIM(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_IS_INVALID_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_IS_INVALID_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_DetectSim(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_DETECT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_DETECT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_GetRuimSuppFlag(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_GET_RUIM_SUPP_FLAG_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_GET_RUIM_SUPP_FLAG_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_SendVerifyChvReq(UInt32 tid, UInt8 clientID, CHV_t chv_select, CHVString_t chv)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIM_SendVerifyChvReq_Req.chv_select = chv_select;
	memcpy(&req.req_rep_u.CAPI2_SIM_SendVerifyChvReq_Req.chv , chv, sizeof(  CHVString_t ));
	req.respId = MSG_SIM_VERIFY_CHV_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_VERIFY_CHV_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_SendChangeChvReq(UInt32 tid, UInt8 clientID, CHV_t chv_select, CHVString_t old_chv, CHVString_t new_chv)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIM_SendChangeChvReq_Req.chv_select = chv_select;
	memcpy(&req.req_rep_u.CAPI2_SIM_SendChangeChvReq_Req.old_chv , old_chv, sizeof(  CHVString_t ));
	memcpy(&req.req_rep_u.CAPI2_SIM_SendChangeChvReq_Req.new_chv , new_chv, sizeof(  CHVString_t ));
	req.respId = MSG_SIM_CHANGE_CHV_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_CHANGE_CHV_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_SendSetChv1OnOffReq(UInt32 tid, UInt8 clientID, CHVString_t chv, Boolean enable_flag)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	memcpy(&req.req_rep_u.CAPI2_SIM_SendSetChv1OnOffReq_Req.chv , chv, sizeof(  CHVString_t ));
	req.req_rep_u.CAPI2_SIM_SendSetChv1OnOffReq_Req.enable_flag = enable_flag;
	req.respId = MSG_SIM_ENABLE_CHV_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_ENABLE_CHV_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_SendUnblockChvReq(UInt32 tid, UInt8 clientID, CHV_t chv_select, PUKString_t puk, CHVString_t new_chv)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIM_SendUnblockChvReq_Req.chv_select = chv_select;
	memcpy(&req.req_rep_u.CAPI2_SIM_SendUnblockChvReq_Req.puk , puk, sizeof(  PUKString_t ));
	memcpy(&req.req_rep_u.CAPI2_SIM_SendUnblockChvReq_Req.new_chv , new_chv, sizeof(  CHVString_t ));
	req.respId = MSG_SIM_UNBLOCK_CHV_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_UNBLOCK_CHV_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_SendSetOperStateReq(UInt32 tid, UInt8 clientID, SIMOperState_t oper_state, CHVString_t chv2)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIM_SendSetOperStateReq_Req.oper_state = oper_state;
	memcpy(&req.req_rep_u.CAPI2_SIM_SendSetOperStateReq_Req.chv2 , chv2, sizeof(  CHVString_t ));
	req.respId = MSG_SIM_SET_FDN_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_SET_FDN_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_IsPbkAccessAllowed(UInt32 tid, UInt8 clientID, SIMPBK_ID_t id)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIM_IsPbkAccessAllowed_Req.id = id;
	req.respId = MSG_SIM_IS_PBK_ALLOWED_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_IS_PBK_ALLOWED_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_SendPbkInfoReq(UInt32 tid, UInt8 clientID, SIMPBK_ID_t id)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIM_SendPbkInfoReq_Req.id = id;
	req.respId = MSG_SIM_PBK_INFO_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_PBK_INFO_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_SendReadAcmMaxReq(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_MAX_ACM_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_MAX_ACM_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_SendWriteAcmMaxReq(UInt32 tid, UInt8 clientID, CallMeterUnit_t acm_max)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIM_SendWriteAcmMaxReq_Req.acm_max = acm_max;
	req.respId = MSG_SIM_ACM_MAX_UPDATE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_ACM_MAX_UPDATE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_SendReadAcmReq(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_ACM_VALUE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_ACM_VALUE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_SendWriteAcmReq(UInt32 tid, UInt8 clientID, CallMeterUnit_t acm)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIM_SendWriteAcmReq_Req.acm = acm;
	req.respId = MSG_SIM_ACM_UPDATE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_ACM_UPDATE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_SendIncreaseAcmReq(UInt32 tid, UInt8 clientID, CallMeterUnit_t acm)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIM_SendIncreaseAcmReq_Req.acm = acm;
	req.respId = MSG_SIM_ACM_INCREASE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_ACM_INCREASE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_SendReadSvcProvNameReq(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_SVC_PROV_NAME_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_SVC_PROV_NAME_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_SendReadPuctReq(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_PUCT_DATA_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_PUCT_DATA_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_GetServiceStatus(UInt32 tid, UInt8 clientID, SIMService_t service)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIM_GetServiceStatus_Req.service = service;
	req.respId = MSG_SIM_SERVICE_STATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_SERVICE_STATUS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_SendGenericAccessEndReq(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_GENERIC_ACCESS_END_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_GENERIC_ACCESS_END_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_GetPinStatus(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_PIN_STATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_PIN_STATUS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_IsPinOK(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_PIN_OK_STATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_PIN_OK_STATUS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_GetIMSI(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_IMSI_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_IMSI_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_GetGID1(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_GID_DIGIT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_GID_DIGIT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_GetGID2(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_GID_DIGIT2_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_GID_DIGIT2_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_GetHomePlmn(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_HOME_PLMN_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_HOME_PLMN_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_GetCurrLockedSimlockType(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_PIN_LOCK_TYPE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_PIN_LOCK_TYPE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_simmi_GetMasterFileId(UInt32 tid, UInt8 clientID, APDUFileID_t file_id)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_simmi_GetMasterFileId_Req.file_id = file_id;
	req.respId = MSG_SIM_APDU_FILEID_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_APDU_FILEID_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_SendOpenSocketReq(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_OPEN_SOCKET_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_OPEN_SOCKET_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_GetAtrData(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_ATR_DATA_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_ATR_DATA_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_SendRemainingPinAttemptReq(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_PIN_ATTEMPT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_PIN_ATTEMPT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_IsCachedDataReady(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_CACHE_DATA_READY_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_CACHE_DATA_READY_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_GetServiceCodeStatus(UInt32 tid, UInt8 clientID, SERVICE_CODE_STATUS_CPHS_t service_code)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIM_GetServiceCodeStatus_Req.service_code = service_code;
	req.respId = MSG_SIM_SERVICE_CODE_STATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_SERVICE_CODE_STATUS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_CheckCphsService(UInt32 tid, UInt8 clientID, CPHS_SST_ENTRY_t sst_entry)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIM_CheckCphsService_Req.sst_entry = sst_entry;
	req.respId = MSG_SIM_CHECK_CPHS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_CHECK_CPHS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_GetCphsPhase(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_CPHS_PHASE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_CPHS_PHASE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_GetSmsSca(UInt32 tid, UInt8 clientID, UInt8 rec_no)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIM_GetSmsSca_Req.rec_no = rec_no;
	req.respId = MSG_SIM_SMS_SCA_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_SMS_SCA_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_GetIccid(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_ICCID_PARAM_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_ICCID_PARAM_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_IsALSEnabled(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_ALS_STATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_ALS_STATUS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_GetAlsDefaultLine(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_ALS_DEFAULT_LINE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_ALS_DEFAULT_LINE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_SetAlsDefaultLine(UInt32 tid, UInt8 clientID, UInt8 line)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIM_SetAlsDefaultLine_Req.line = line;
	req.respId = MSG_SIM_SET_ALS_DEFAULT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_SET_ALS_DEFAULT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_GetCallForwardUnconditionalFlag(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_CALLFWD_COND_FLAG_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_CALLFWD_COND_FLAG_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_GetApplicationType(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_APP_TYPE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_APP_TYPE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_USIM_GetUst(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_USIM_UST_DATA_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_USIM_UST_DATA_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_ADCMGR_Start(UInt32 tid, UInt8 clientID, UInt16 init_value)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_ADCMGR_Start_Req.init_value = init_value;
	req.respId = MSG_ADC_START_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_ADC_START_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_AT_ProcessCmd(UInt32 tid, UInt8 clientID, UInt8 channel, uchar_ptr_t cmdStr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_AT_ProcessCmd_Req.channel = channel;
	req.req_rep_u.CAPI2_AT_ProcessCmd_Req.cmdStr = cmdStr;
	req.respId = MSG_CAPI2_AT_COMMAND_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_CAPI2_AT_COMMAND_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_GetSystemRAT(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GET_SYSTEM_RAT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GET_SYSTEM_RAT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_GetSupportedRAT(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GET_SUPPORTED_RAT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GET_SUPPORTED_RAT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_GetSystemBand(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GET_SYSTEM_BAND_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GET_SYSTEM_BAND_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_GetSupportedBand(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GET_SUPPORTED_BAND_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GET_SUPPORTED_BAND_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYSPARM_GetMSClass(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYSPARAM_GET_MSCLASS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARAM_GET_MSCLASS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_AUDIO_GetMicrophoneGainSetting(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_AUDIO_GET_MIC_GAIN_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_AUDIO_GET_MIC_GAIN_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_AUDIO_GetSpeakerVol(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_AUDIO_GET_SPEAKER_VOL_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_AUDIO_GET_SPEAKER_VOL_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_AUDIO_SetSpeakerVol(UInt32 tid, UInt8 clientID, UInt8 vol)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_AUDIO_SetSpeakerVol_Req.vol = vol;
	req.respId = MSG_AUDIO_SET_SPEAKER_VOL_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_AUDIO_SET_SPEAKER_VOL_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_AUDIO_SetMicrophoneGain(UInt32 tid, UInt8 clientID, UInt8 gain)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_AUDIO_SetMicrophoneGain_Req.gain = gain;
	req.respId = MSG_AUDIO_SET_MIC_GAIN_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_AUDIO_SET_MIC_GAIN_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYSPARM_GetManufacturerName(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYSPARAM_GET_MNF_NAME_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARAM_GET_MNF_NAME_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYSPARM_GetModelName(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYSPARAM_GET_MODEL_NAME_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARAM_GET_MODEL_NAME_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYSPARM_GetSWVersion(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYSPARAM_GET_SW_VERSION_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARAM_GET_SW_VERSION_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYSPARM_GetEGPRSMSClass(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYSPARAM_GET_EGPRS_CLASS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARAM_GET_EGPRS_CLASS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_UTIL_ExtractImei(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYSPARAM_GET_IMEI_STR_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARAM_GET_IMEI_STR_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_GetRegistrationInfo(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GET_REG_INFO_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GET_REG_INFO_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_SendNumOfPLMNEntryReq(UInt32 tid, UInt8 clientID, SIM_PLMN_FILE_t plmn_file)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIM_SendNumOfPLMNEntryReq_Req.plmn_file = plmn_file;
	req.respId = MSG_SIM_PLMN_NUM_OF_ENTRY_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_PLMN_NUM_OF_ENTRY_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_SendReadPLMNEntryReq(UInt32 tid, UInt8 clientID, SIM_PLMN_FILE_t plmn_file, UInt16 start_index, UInt16 end_index)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIM_SendReadPLMNEntryReq_Req.plmn_file = plmn_file;
	req.req_rep_u.CAPI2_SIM_SendReadPLMNEntryReq_Req.start_index = start_index;
	req.req_rep_u.CAPI2_SIM_SendReadPLMNEntryReq_Req.end_index = end_index;
	req.respId = MSG_SIM_PLMN_ENTRY_DATA_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_PLMN_ENTRY_DATA_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYS_SetRegisteredEventMask(UInt32 tid, UInt8 clientID, UInt16 *maskList, UInt8 maskLen)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SYS_SetRegisteredEventMask_Req.maskList = maskList;
	req.req_rep_u.CAPI2_SYS_SetRegisteredEventMask_Req.maskLen = maskLen;
	req.respId = MSG_SYS_SET_REG_EVENT_MASK_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYS_SET_REG_EVENT_MASK_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYS_SetFilteredEventMask(UInt32 tid, UInt8 clientID, UInt16 *maskList, UInt8 maskLen, SysFilterEnable_t enableFlag)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SYS_SetFilteredEventMask_Req.maskList = maskList;
	req.req_rep_u.CAPI2_SYS_SetFilteredEventMask_Req.maskLen = maskLen;
	req.req_rep_u.CAPI2_SYS_SetFilteredEventMask_Req.enableFlag = enableFlag;
	req.respId = MSG_SYS_SET_REG_FILTER_MASK_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYS_SET_REG_FILTER_MASK_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYS_SetRssiThreshold(UInt32 tid, UInt8 clientID, UInt8 gsm_rxlev_thresold, UInt8 gsm_rxqual_thresold, UInt8 umts_rscp_thresold, UInt8 umts_ecio_thresold)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SYS_SetRssiThreshold_Req.gsm_rxlev_thresold = gsm_rxlev_thresold;
	req.req_rep_u.CAPI2_SYS_SetRssiThreshold_Req.gsm_rxqual_thresold = gsm_rxqual_thresold;
	req.req_rep_u.CAPI2_SYS_SetRssiThreshold_Req.umts_rscp_thresold = umts_rscp_thresold;
	req.req_rep_u.CAPI2_SYS_SetRssiThreshold_Req.umts_ecio_thresold = umts_ecio_thresold;
	req.respId = MSG_SYS_SET_RSSI_THRESHOLD_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYS_SET_RSSI_THRESHOLD_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYS_GetBootLoaderVersion(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYSPARAM_BOOTLOADER_VER_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARAM_BOOTLOADER_VER_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYS_GetDSFVersion(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYSPARAM_DSF_VER_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARAM_DSF_VER_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYSPARM_GetChanMode(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYSPARAM_GET_CHANNEL_MODE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARAM_GET_CHANNEL_MODE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYSPARM_GetClassmark(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYSPARAM_GET_CLASSMARK_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARAM_GET_CLASSMARK_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYSPARM_GetIMEI(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYSPARAM_GET_IMEI_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARAM_GET_IMEI_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYSPARM_GetSysparmIndPartFileVersion(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYSPARAM_GET_IND_FILE_VER_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARAM_GET_IND_FILE_VER_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYSPARM_SetDARPCfg(UInt32 tid, UInt8 clientID, UInt8 darp_cfg)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SYSPARM_SetDARPCfg_Req.darp_cfg = darp_cfg;
	req.respId = MSG_SYS_SET_DARP_CFG_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYS_SET_DARP_CFG_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYSPARM_SetEGPRSMSClass(UInt32 tid, UInt8 clientID, UInt16 egprs_class)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SYSPARM_SetEGPRSMSClass_Req.egprs_class = egprs_class;
	req.respId = MSG_SYSPARAM_SET_EGPRS_MSCLASS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARAM_SET_EGPRS_MSCLASS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYSPARM_SetGPRSMSClass(UInt32 tid, UInt8 clientID, UInt16 gprs_class)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SYSPARM_SetGPRSMSClass_Req.gprs_class = gprs_class;
	req.respId = MSG_SYSPARAM_SET_GPRS_MSCLASS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARAM_SET_GPRS_MSCLASS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYS_SetMSPowerOnCause(UInt32 tid, UInt8 clientID, UInt8 reset_cause)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SYS_SetMSPowerOnCause_Req.reset_cause = reset_cause;
	req.respId = MSG_SYSPARAM_SET_POWERON_CAUSE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARAM_SET_POWERON_CAUSE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_TIMEZONE_DeleteNetworkName(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_TIMEZONE_DELETE_NW_NAME_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_TIMEZONE_DELETE_NW_NAME_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_TIMEZONE_GetTZUpdateMode(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_TIMEZONE_GET_UPDATE_MODE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_TIMEZONE_GET_UPDATE_MODE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_TIMEZONE_SetTZUpdateMode(UInt32 tid, UInt8 clientID, TimeZoneUpdateMode_t mode)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_TIMEZONE_SetTZUpdateMode_Req.mode = mode;
	req.respId = MSG_TIMEZONE_SET_UPDATE_MODE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_TIMEZONE_SET_UPDATE_MODE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_TIMEZONE_UpdateRTC(UInt32 tid, UInt8 clientID, Boolean updateFlag)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_TIMEZONE_UpdateRTC_Req.updateFlag = updateFlag;
	req.respId = MSG_TIMEZONE_UPDATE_RTC_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_TIMEZONE_UPDATE_RTC_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_PMU_IsSIMReady_RSP(UInt32 tid, UInt8 clientID, Boolean status)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PMU_IsSIMReady_RSP_Rsp.status = status;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_PMU_IS_SIM_READY_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_PMU_ActivateSIM_RSP(UInt32 tid, UInt8 clientID, Boolean status)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PMU_ActivateSIM_RSP_Rsp.status = status;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_PMU_ACTIVATE_SIM_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_PMU_DeactivateSIM_RSP(UInt32 tid, UInt8 clientID, Boolean status)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PMU_DeactivateSIM_RSP_Rsp.status = status;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_PMU_DEACTIVATE_SIM_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_TestCmds(UInt32 tid, UInt8 clientID, UInt32 testId, UInt32 param1, UInt32 param2, uchar_ptr_t buffer)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_TestCmds_Req.testId = testId;
	req.req_rep_u.CAPI2_TestCmds_Req.param1 = param1;
	req.req_rep_u.CAPI2_TestCmds_Req.param2 = param2;
	req.req_rep_u.CAPI2_TestCmds_Req.buffer = buffer;
	req.respId = MSG_CAPI2_TEST_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_CAPI2_TEST_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SATK_SendPlayToneRes(UInt32 tid, UInt8 clientID, SATK_ResultCode_t resultCode)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SATK_SendPlayToneRes_Req.resultCode = resultCode;
	req.respId = MSG_STK_SEND_PLAYTONE_RES_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_STK_SEND_PLAYTONE_RES_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SATK_SendSetupCallRes(UInt32 tid, UInt8 clientID, SATK_ResultCode_t resultCode)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SATK_SendSetupCallRes_Req.resultCode = resultCode;
	req.respId = MSG_STK_SETUP_CALL_RES_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_STK_SETUP_CALL_RES_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_PBK_SetFdnCheck(UInt32 tid, UInt8 clientID, Boolean fdn_chk_on)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PBK_SetFdnCheck_Req.fdn_chk_on = fdn_chk_on;
	req.respId = MSG_PBK_SET_FDN_CHECK_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_PBK_SET_FDN_CHECK_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_PBK_GetFdnCheck(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_PBK_GET_FDN_CHECK_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_PBK_GET_FDN_CHECK_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_GPIO_Set_High_64Pin_RSP(UInt32 tid, UInt8 clientID, Boolean status)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_GPIO_Set_High_64Pin_RSP_Rsp.status = status;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_GPIO_SET_HIGH_64PIN_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_GPIO_Set_Low_64Pin_RSP(UInt32 tid, UInt8 clientID, Boolean status)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_GPIO_Set_Low_64Pin_RSP_Rsp.status = status;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_GPIO_SET_LOW_64PIN_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_PMU_StartCharging_RSP(UInt32 tid, UInt8 clientID, Boolean status)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PMU_StartCharging_RSP_Rsp.status = status;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_PMU_START_CHARGING_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_PMU_StopCharging_RSP(UInt32 tid, UInt8 clientID, Boolean status)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PMU_StopCharging_RSP_Rsp.status = status;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_PMU_STOP_CHARGING_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_PMU_BattADCReq(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_PMU_BATT_LEVEL_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_PMU_BATT_LEVEL_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_PMU_Battery_Register(UInt32 tid, UInt8 clientID, HAL_EM_BATTMGR_Event_en_t event)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PMU_Battery_Register_Req.event = event;
	req.respId = MSG_PMU_BATT_LEVEL_REGISTER_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_PMU_BATT_LEVEL_REGISTER_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SMS_SendMemAvailInd(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SMS_SEND_MEM_AVAL_IND_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_SEND_MEM_AVAL_IND_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SMS_IsMeStorageEnabled_RSP(UInt32 tid, UInt8 clientID, Boolean status)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SMS_IsMeStorageEnabled_RSP_Rsp.status = status;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_SMS_ISMESTORAGEENABLED_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SMS_GetMaxMeCapacity_RSP(UInt32 tid, UInt8 clientID, UInt16 capacity)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SMS_GetMaxMeCapacity_RSP_Rsp.capacity = capacity;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_SMS_GETMAXMECAPACITY_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SMS_GetNextFreeSlot_RSP(UInt32 tid, UInt8 clientID, UInt16 free_slot)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SMS_GetNextFreeSlot_RSP_Rsp.free_slot = free_slot;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_SMS_GETNEXTFREESLOT_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SMS_SetMeSmsStatus_RSP(UInt32 tid, UInt8 clientID, Boolean flag)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SMS_SetMeSmsStatus_RSP_Rsp.flag = flag;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_SMS_SETMESMSSTATUS_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SMS_GetMeSmsStatus_RSP(UInt32 tid, UInt8 clientID, SIMSMSMesgStatus_t status)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SMS_GetMeSmsStatus_RSP_Rsp.status = status;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_SMS_GETMESMSSTATUS_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SMS_StoreSmsToMe_RSP(UInt32 tid, UInt8 clientID, Boolean flag)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SMS_StoreSmsToMe_RSP_Rsp.flag = flag;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_SMS_STORESMSTOME_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SMS_RetrieveSmsFromMe_RSP(UInt32 tid, UInt8 clientID, Boolean flag)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SMS_RetrieveSmsFromMe_RSP_Rsp.flag = flag;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_SMS_RETRIEVESMSFROMME_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SMS_RemoveSmsFromMe_RSP(UInt32 tid, UInt8 clientID, Boolean flag)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SMS_RemoveSmsFromMe_RSP_Rsp.flag = flag;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_SMS_REMOVESMSFROMME_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SMS_ConfigureMEStorage(UInt32 tid, UInt8 clientID, Boolean flag)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SMS_ConfigureMEStorage_Req.flag = flag;
	req.respId = MSG_SMS_CONFIGUREMESTORAGE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_CONFIGUREMESTORAGE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_SetElement(UInt32 tid, UInt8 clientID, CAPI2_MS_Element_t *inElemData)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_MS_SetElement_Req.inElemData = inElemData;
	req.respId = MSG_MS_SET_ELEMENT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_SET_ELEMENT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_GetElement(UInt32 tid, UInt8 clientID, MS_Element_t inElemType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_MS_GetElement_Req.inElemType = inElemType;
	req.respId = MSG_MS_GET_ELEMENT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GET_ELEMENT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_USIM_IsApplicationSupported(UInt32 tid, UInt8 clientID, USIM_APPLICATION_TYPE appli_type)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_USIM_IsApplicationSupported_Req.appli_type = appli_type;
	req.respId = MSG_USIM_IS_APP_SUPPORTED_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_USIM_IS_APP_SUPPORTED_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_USIM_IsAllowedAPN(UInt32 tid, UInt8 clientID, char_ptr_t apn_name)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_USIM_IsAllowedAPN_Req.apn_name = apn_name;
	req.respId = MSG_USIM_IS_APN_ALLOWED_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_USIM_IS_APN_ALLOWED_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_USIM_GetNumOfAPN(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_USIM_GET_NUM_APN_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_USIM_GET_NUM_APN_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_USIM_GetAPNEntry(UInt32 tid, UInt8 clientID, UInt8 index)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_USIM_GetAPNEntry_Req.index = index;
	req.respId = MSG_USIM_GET_APN_ENTRY_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_USIM_GET_APN_ENTRY_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_USIM_IsEstServActivated(UInt32 tid, UInt8 clientID, USIM_EST_SERVICE_t est_serv)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_USIM_IsEstServActivated_Req.est_serv = est_serv;
	req.respId = MSG_USIM_IS_EST_SERV_ACTIVATED_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_USIM_IS_EST_SERV_ACTIVATED_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_USIM_SendSetEstServReq(UInt32 tid, UInt8 clientID, USIM_EST_SERVICE_t est_serv, Boolean serv_on)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_USIM_SendSetEstServReq_Req.est_serv = est_serv;
	req.req_rep_u.CAPI2_USIM_SendSetEstServReq_Req.serv_on = serv_on;
	req.respId = MSG_SIM_SET_EST_SERV_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_SET_EST_SERV_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_USIM_SendWriteAPNReq(UInt32 tid, UInt8 clientID, UInt8 index, char_ptr_t apn_name)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_USIM_SendWriteAPNReq_Req.index = index;
	req.req_rep_u.CAPI2_USIM_SendWriteAPNReq_Req.apn_name = apn_name;
	req.respId = MSG_SIM_UPDATE_ONE_APN_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_UPDATE_ONE_APN_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_USIM_SendDeleteAllAPNReq(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_DELETE_ALL_APN_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_DELETE_ALL_APN_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_USIM_GetRatModeSetting(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_USIM_GET_RAT_MODE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_USIM_GET_RAT_MODE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_PMU_ClientPowerDown_RSP(UInt32 tid, UInt8 clientID, Boolean status)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PMU_ClientPowerDown_RSP_Rsp.status = status;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_PMU_CLIENT_POWER_DOWN_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_PMU_GetPowerupCause_RSP(UInt32 tid, UInt8 clientID, PMU_PowerupId_t powerupId)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PMU_GetPowerupCause_RSP_Rsp.powerupId = powerupId;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_PMU_GET_POWERUP_CAUSE_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_MS_GetGPRSRegState(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GET_GPRS_STATE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GET_GPRS_STATE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_GetGSMRegState(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GET_GSM_STATE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GET_GSM_STATE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_GetRegisteredCellInfo(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GET_CELL_INFO_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GET_CELL_INFO_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_GetStartBand(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GET_START_BAND_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GET_START_BAND_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_SetMEPowerClass(UInt32 tid, UInt8 clientID, UInt8 band, UInt8 pwrClass)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_MS_SetMEPowerClass_Req.band = band;
	req.req_rep_u.CAPI2_MS_SetMEPowerClass_Req.pwrClass = pwrClass;
	req.respId = MSG_MS_SETMEPOWER_CLASS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_SETMEPOWER_CLASS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_USIM_GetServiceStatus(UInt32 tid, UInt8 clientID, SIMService_t service)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_USIM_GetServiceStatus_Req.service = service;
	req.respId = MSG_USIM_GET_SERVICE_STATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_USIM_GET_SERVICE_STATUS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_IsAllowedAPN(UInt32 tid, UInt8 clientID, char_ptr_t apn_name)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIM_IsAllowedAPN_Req.apn_name = apn_name;
	req.respId = MSG_SIM_IS_ALLOWED_APN_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_IS_ALLOWED_APN_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SMS_GetSmsMaxCapacity(UInt32 tid, UInt8 clientID, SmsStorage_t storageType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SMS_GetSmsMaxCapacity_Req.storageType = storageType;
	req.respId = MSG_SMS_GETSMSMAXCAPACITY_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_GETSMSMAXCAPACITY_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SMS_RetrieveMaxCBChnlLength(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SMS_RETRIEVEMAXCBCHNLLENGTH_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_RETRIEVEMAXCBCHNLLENGTH_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_IsBdnOperationRestricted(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_IS_BDN_RESTRICTED_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_IS_BDN_RESTRICTED_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_SendPreferredPlmnUpdateInd(UInt32 tid, UInt8 clientID, SIM_PLMN_FILE_t prefer_plmn_file)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIM_SendPreferredPlmnUpdateInd_Req.prefer_plmn_file = prefer_plmn_file;
	req.respId = MSG_SIM_SEND_PLMN_UPDATE_IND_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_SEND_PLMN_UPDATE_IND_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_Deactive(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_DEACTIVATE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_DEACTIVATE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_SendSetBdnReq(UInt32 tid, UInt8 clientID, SIMBdnOperState_t oper_state, CHVString_t chv2)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIM_SendSetBdnReq_Req.oper_state = oper_state;
	memcpy(&req.req_rep_u.CAPI2_SIM_SendSetBdnReq_Req.chv2 , chv2, sizeof(  CHVString_t ));
	req.respId = MSG_SIM_SET_BDN_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_SET_BDN_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_PowerOnOffCard(UInt32 tid, UInt8 clientID, Boolean power_on, SIM_POWER_ON_MODE_t mode)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIM_PowerOnOffCard_Req.power_on = power_on;
	req.req_rep_u.CAPI2_SIM_PowerOnOffCard_Req.mode = mode;
	req.respId = MSG_SIM_POWER_ON_OFF_CARD_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_POWER_ON_OFF_CARD_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_GetRawAtr(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_GET_RAW_ATR_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_GET_RAW_ATR_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_Set_Protocol(UInt32 tid, UInt8 clientID, UInt8 protocol)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIM_Set_Protocol_Req.protocol = protocol;
	req.respId = MSG_SIM_SET_PROTOCOL_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_SET_PROTOCOL_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_Get_Protocol(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_GET_PROTOCOL_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_GET_PROTOCOL_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_TerminateXferApdu(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_TERMINATE_XFER_APDU_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_TERMINATE_XFER_APDU_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_SetPlmnSelectRat(UInt32 tid, UInt8 clientID, UInt8 manual_rat)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_MS_SetPlmnSelectRat_Req.manual_rat = manual_rat;
	req.respId = MSG_SET_PLMN_SELECT_RAT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SET_PLMN_SELECT_RAT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_IsDeRegisterInProgress(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_IS_DEREGISTER_IN_PROGRESS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_IS_DEREGISTER_IN_PROGRESS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_IsRegisterInProgress(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_IS_REGISTER_IN_PROGRESS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_IS_REGISTER_IN_PROGRESS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SOCKET_Open_RSP(UInt32 tid, UInt8 clientID, Int32 descriptor)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_Open_RSP_Rsp.descriptor = descriptor;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_SOCKET_OPEN_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_Bind_RSP(UInt32 tid, UInt8 clientID, Int8 status)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_Bind_RSP_Rsp.status = status;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_SOCKET_BIND_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_Listen_RSP(UInt32 tid, UInt8 clientID, Int8 status)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_Listen_RSP_Rsp.status = status;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_SOCKET_LISTEN_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_Accept_RSP(UInt32 tid, UInt8 clientID, sockaddr *name, Int32 acceptDescriptor)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_Accept_RSP_Rsp.name = name;
	req.req_rep_u.CAPI2_SOCKET_Accept_RSP_Rsp.acceptDescriptor = acceptDescriptor;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_SOCKET_ACCEPT_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_Connect_RSP(UInt32 tid, UInt8 clientID, Int8 status)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_Connect_RSP_Rsp.status = status;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_SOCKET_CONNECT_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_GetPeerName_RSP(UInt32 tid, UInt8 clientID, sockaddr *peerName, Int8 status)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_GetPeerName_RSP_Rsp.peerName = peerName;
	req.req_rep_u.CAPI2_SOCKET_GetPeerName_RSP_Rsp.status = status;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_SOCKET_GETPEERNAME_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_GetSockName_RSP(UInt32 tid, UInt8 clientID, sockaddr *sockName, Int8 status)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_GetSockName_RSP_Rsp.sockName = sockName;
	req.req_rep_u.CAPI2_SOCKET_GetSockName_RSP_Rsp.status = status;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_SOCKET_GETSOCKNAME_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_SetSockOpt_RSP(UInt32 tid, UInt8 clientID, Int8 status)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_SetSockOpt_RSP_Rsp.status = status;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_SOCKET_SETSOCKOPT_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_GetSockOpt_RSP(UInt32 tid, UInt8 clientID, SockOptVal_t *optval, Int8 status)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_GetSockOpt_RSP_Rsp.optval = optval;
	req.req_rep_u.CAPI2_SOCKET_GetSockOpt_RSP_Rsp.status = status;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_SOCKET_GETSOCKOPT_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_SignalInd(UInt32 tid, UInt8 clientID, SocketSignalInd_t *sockSignalInd)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_SignalInd_Req.sockSignalInd = sockSignalInd;
	req.respId = MSG_CAPI2_SOCKET_SIGNAL_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_CAPI2_SOCKET_SIGNAL_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SOCKET_Send_RSP(UInt32 tid, UInt8 clientID, Int32 bytesSent)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_Send_RSP_Rsp.bytesSent = bytesSent;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_SOCKET_SEND_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_SendTo_RSP(UInt32 tid, UInt8 clientID, Int32 bytesSent)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_SendTo_RSP_Rsp.bytesSent = bytesSent;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_SOCKET_SEND_TO_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_Recv_RSP(UInt32 tid, UInt8 clientID, SocketRecvRsp_t *rsp)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_Recv_RSP_Rsp.rsp = rsp;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_SOCKET_RECV_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_RecvFrom_RSP(UInt32 tid, UInt8 clientID, SocketRecvRsp_t *rsp)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_RecvFrom_RSP_Rsp.rsp = rsp;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_SOCKET_RECV_FROM_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_Close_RSP(UInt32 tid, UInt8 clientID, Int8 status)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_Close_RSP_Rsp.status = status;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_SOCKET_CLOSE_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_Shutdown_RSP(UInt32 tid, UInt8 clientID, Int8 status)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_Shutdown_RSP_Rsp.status = status;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_SOCKET_SHUTDOWN_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_Errno_RSP(UInt32 tid, UInt8 clientID, Int32 error)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_Errno_RSP_Rsp.error = error;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_SOCKET_ERRNO_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_SO2LONG_RSP(UInt32 tid, UInt8 clientID, Int32 descriptor)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_SO2LONG_RSP_Rsp.descriptor = descriptor;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_SOCKET_SO2LONG_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_GetSocketSendBufferSpace_RSP(UInt32 tid, UInt8 clientID, Int32 bufferSpace)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_GetSocketSendBufferSpace_RSP_Rsp.bufferSpace = bufferSpace;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_SOCKET_GET_SOCKET_SEND_BUFFER_SPACE_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_ParseIPAddr_RSP(UInt32 tid, UInt8 clientID, ip_addr *ipAddress, unsigned subnetBits, char_ptr_t errorStr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_ParseIPAddr_RSP_Rsp.ipAddress = ipAddress;
	req.req_rep_u.CAPI2_SOCKET_ParseIPAddr_RSP_Rsp.subnetBits = subnetBits;
	req.req_rep_u.CAPI2_SOCKET_ParseIPAddr_RSP_Rsp.errorStr = errorStr;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_SOCKET_PARSE_IPAD_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DC_SetupDataConnection_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DC_SetupDataConnection_RSP_Rsp.resultVal = resultVal;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_SETUP_DATA_CONNECTION_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DC_SetupDataConnectionEx_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal, UInt8 actDCAcctId)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DC_SetupDataConnectionEx_RSP_Rsp.resultVal = resultVal;
	req.req_rep_u.CAPI2_DC_SetupDataConnectionEx_RSP_Rsp.actDCAcctId = actDCAcctId;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_SETUP_DATA_CONNECTION_EX_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DC_ReportCallStatusInd(UInt32 tid, UInt8 clientID, DC_ReportCallStatus_t *status)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DC_ReportCallStatusInd_Req.status = status;
	req.respId = MSG_DC_REPORT_CALL_STATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DC_REPORT_CALL_STATUS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_DC_ShutdownDataConnection_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DC_ShutdownDataConnection_RSP_Rsp.resultVal = resultVal;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DC_SHUTDOWN_DATA_CONNECTION_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_IsAcctIDValid_RSP(UInt32 tid, UInt8 clientID, Boolean isValid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_IsAcctIDValid_RSP_Rsp.isValid = isValid;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_IS_ACCT_ID_VALID_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_CreateGPRSDataAcct_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_CreateGPRSDataAcct_RSP_Rsp.resultVal = resultVal;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_CREATE_GPRS_ACCT_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_CreateCSDDataAcct_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_CreateCSDDataAcct_RSP_Rsp.resultVal = resultVal;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_CREATE_GSM_ACCT_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_DeleteDataAcct_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_DeleteDataAcct_RSP_Rsp.resultVal = resultVal;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_DELETE_ACCT_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetUsername_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetUsername_RSP_Rsp.resultVal = resultVal;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_SET_USERNAME_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetUsername_RSP(UInt32 tid, UInt8 clientID, uchar_ptr_t username)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetUsername_RSP_Rsp.username = username;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_GET_USERNAME_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetPassword_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetPassword_RSP_Rsp.resultVal = resultVal;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_SET_PASSWORD_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetPassword_RSP(UInt32 tid, UInt8 clientID, uchar_ptr_t password)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetPassword_RSP_Rsp.password = password;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_GET_PASSWORD_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetStaticIPAddr_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetStaticIPAddr_RSP_Rsp.resultVal = resultVal;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_SET_STATIC_IP_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetStaticIPAddr_RSP(UInt32 tid, UInt8 clientID, uchar_ptr_t staticIPAddr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetStaticIPAddr_RSP_Rsp.staticIPAddr = staticIPAddr;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_GET_STATIC_IP_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetPrimaryDnsAddr_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetPrimaryDnsAddr_RSP_Rsp.resultVal = resultVal;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_SET_PRIMARY_DNS_ADDR_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetPrimaryDnsAddr_RSP(UInt32 tid, UInt8 clientID, uchar_ptr_t priDnsAddr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetPrimaryDnsAddr_RSP_Rsp.priDnsAddr = priDnsAddr;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_GET_PRIMARY_DNS_ADDR_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetSecondDnsAddr_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetSecondDnsAddr_RSP_Rsp.resultVal = resultVal;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_SET_SECOND_DNS_ADDR_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetSecondDnsAddr_RSP(UInt32 tid, UInt8 clientID, uchar_ptr_t sndDnsAddr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetSecondDnsAddr_RSP_Rsp.sndDnsAddr = sndDnsAddr;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_GET_SECOND_DNS_ADDR_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetDataCompression_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetDataCompression_RSP_Rsp.resultVal = resultVal;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_SET_DATA_COMPRESSION_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetDataCompression_RSP(UInt32 tid, UInt8 clientID, Boolean dataCompEnable)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetDataCompression_RSP_Rsp.dataCompEnable = dataCompEnable;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_GET_DATA_COMPRESSION_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetAcctType_RSP(UInt32 tid, UInt8 clientID, DataAccountType_t dataAcctType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetAcctType_RSP_Rsp.dataAcctType = dataAcctType;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_GET_ACCT_TYPE_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetEmptyAcctSlot_RSP(UInt32 tid, UInt8 clientID, UInt8 emptySlot)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetEmptyAcctSlot_RSP_Rsp.emptySlot = emptySlot;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_GET_EMPTY_ACCT_SLOT_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetCidFromDataAcctID_RSP(UInt32 tid, UInt8 clientID, UInt8 contextID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetCidFromDataAcctID_RSP_Rsp.contextID = contextID;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_GET_CID_FROM_ACCTID_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetDataAcctIDFromCid_RSP(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetDataAcctIDFromCid_RSP_Rsp.acctID = acctID;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_CAPI2_DATA_GET_ACCTID_FROM_CID_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetPrimaryCidFromDataAcctID_RSP(UInt32 tid, UInt8 clientID, UInt8 priContextID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetPrimaryCidFromDataAcctID_RSP_Rsp.priContextID = priContextID;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_GET_PRI_FROM_ACCTID_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_IsSecondaryDataAcct_RSP(UInt32 tid, UInt8 clientID, Boolean isSndDataAcct)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_IsSecondaryDataAcct_RSP_Rsp.isSndDataAcct = isSndDataAcct;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_IS_SND_DATA_ACCT_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetDataSentSize_RSP(UInt32 tid, UInt8 clientID, UInt32 dataSentSize)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetDataSentSize_RSP_Rsp.dataSentSize = dataSentSize;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_GET_ACCTID_FROM_CID_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetDataRcvSize_RSP(UInt32 tid, UInt8 clientID, UInt32 dataRcvSize)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetDataRcvSize_RSP_Rsp.dataRcvSize = dataRcvSize;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_GET_DATA_RCV_SIZE_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetGPRSPdpType_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetGPRSPdpType_RSP_Rsp.resultVal = resultVal;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_SET_GPRS_PDP_TYPE_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetGPRSPdpType_RSP(UInt32 tid, UInt8 clientID, uchar_ptr_t pdpType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetGPRSPdpType_RSP_Rsp.pdpType = pdpType;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_GET_GPRS_PDP_TYPE_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetGPRSApn_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetGPRSApn_RSP_Rsp.resultVal = resultVal;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_SET_GPRS_APN_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetGPRSApn_RSP(UInt32 tid, UInt8 clientID, uchar_ptr_t apn)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetGPRSApn_RSP_Rsp.apn = apn;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_GET_GPRS_APN_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetAuthenMethod_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetAuthenMethod_RSP_Rsp.resultVal = resultVal;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_SET_AUTHEN_METHOD_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetAuthenMethod_RSP(UInt32 tid, UInt8 clientID, DataAuthenMethod_t authenMethod)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetAuthenMethod_RSP_Rsp.authenMethod = authenMethod;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_GET_AUTHEN_METHOD_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetGPRSHeaderCompression_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetGPRSHeaderCompression_RSP_Rsp.resultVal = resultVal;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_SET_GPRS_HEADER_COMPRESSION_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetGPRSHeaderCompression_RSP(UInt32 tid, UInt8 clientID, Boolean headerCompEnable)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetGPRSHeaderCompression_RSP_Rsp.headerCompEnable = headerCompEnable;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_GET_GPRS_HEADER_COMPRESSION_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetGPRSQos_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetGPRSQos_RSP_Rsp.resultVal = resultVal;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_SET_GPRS_QOS_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetGPRSQos_RSP(UInt32 tid, UInt8 clientID, PCHQosProfile_t qos)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetGPRSQos_RSP_Rsp.qos = qos;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_CAPI2_DATA_GET_GPRS_QOS_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetAcctLock_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetAcctLock_RSP_Rsp.resultVal = resultVal;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_SET_ACCT_LOCK_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetAcctLock_RSP(UInt32 tid, UInt8 clientID, Boolean acctLock)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetAcctLock_RSP_Rsp.acctLock = acctLock;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_GET_ACCT_LOCK_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetGprsOnly_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetGprsOnly_RSP_Rsp.resultVal = resultVal;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_SET_GPRS_ONLY_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetGprsOnly_RSP(UInt32 tid, UInt8 clientID, Boolean gprsOnly)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetGprsOnly_RSP_Rsp.gprsOnly = gprsOnly;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_GET_GPRS_ONLY_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetGPRSTft_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetGPRSTft_RSP_Rsp.resultVal = resultVal;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_SET_GPRS_TFT_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetGPRSTft_RSP(UInt32 tid, UInt8 clientID, CAPI2_DATA_GetGPRSTft_Result_t rsp)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetGPRSTft_RSP_Rsp.rsp = rsp;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_GET_GPRS_TFT_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetCSDDialNumber_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetCSDDialNumber_RSP_Rsp.resultVal = resultVal;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_SET_CSD_DIAL_NUMBER_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetCSDDialNumber_RSP(UInt32 tid, UInt8 clientID, uchar_ptr_t dialNumber)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetCSDDialNumber_RSP_Rsp.dialNumber = dialNumber;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_GET_CSD_DIAL_NUMBER_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetCSDDialType_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetCSDDialType_RSP_Rsp.resultVal = resultVal;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_SET_CSD_DIAL_TYPE_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetCSDDialType_RSP(UInt32 tid, UInt8 clientID, CSDDialType_t csdDialType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetCSDDialType_RSP_Rsp.csdDialType = csdDialType;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_GET_CSD_DIAL_TYPE_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetCSDBaudRate_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetCSDBaudRate_RSP_Rsp.resultVal = resultVal;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_SET_CSD_BAUD_RATE_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetCSDBaudRate_RSP(UInt32 tid, UInt8 clientID, CSDBaudRate_t csdBaudRate)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetCSDBaudRate_RSP_Rsp.csdBaudRate = csdBaudRate;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_GET_CSD_BAUD_RATE_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetCSDSyncType_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetCSDSyncType_RSP_Rsp.resultVal = resultVal;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_SET_CSD_SYNC_TYPE_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetCSDSyncType_RSP(UInt32 tid, UInt8 clientID, CSDSyncType_t csdSyncType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetCSDSyncType_RSP_Rsp.csdSyncType = csdSyncType;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_GET_CSD_SYNC_TYPE_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetCSDErrorCorrection_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetCSDErrorCorrection_RSP_Rsp.resultVal = resultVal;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_SET_CSD_ERROR_CORRECTION_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetCSDErrorCorrection_RSP(UInt32 tid, UInt8 clientID, Boolean enabled)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetCSDErrorCorrection_RSP_Rsp.enabled = enabled;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_GET_CSD_ERROR_CORRECTION_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetCSDErrCorrectionType_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetCSDErrCorrectionType_RSP_Rsp.resultVal = resultVal;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_SET_CSD_ERR_CORRECTION_TYPE_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetCSDErrCorrectionType_RSP(UInt32 tid, UInt8 clientID, CSDErrCorrectionType_t errCorrectionType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetCSDErrCorrectionType_RSP_Rsp.errCorrectionType = errCorrectionType;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_GET_CSD_ERR_CORRECTION_TYPE_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetCSDDataCompType_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetCSDDataCompType_RSP_Rsp.resultVal = resultVal;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_SET_CSD_DATA_COMP_TYPE_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetCSDDataCompType_RSP(UInt32 tid, UInt8 clientID, CSDDataCompType_t dataCompType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetCSDDataCompType_RSP_Rsp.dataCompType = dataCompType;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_GET_CSD_DATA_COMP_TYPE_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetCSDConnElement_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetCSDConnElement_RSP_Rsp.resultVal = resultVal;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_SET_CSD_CONN_ELEMENT_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetCSDConnElement_RSP(UInt32 tid, UInt8 clientID, CSDConnElement_t connElement)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetCSDConnElement_RSP_Rsp.connElement = connElement;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_GET_CSD_CONN_ELEMENT_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_UpdateAccountToFileSystem_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_UpdateAccountToFileSystem_RSP_Rsp.resultVal = resultVal;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_UPDATE_ACCT_TO_FILE_SYSTEM_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_resetDataSize_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_resetDataSize_RSP_Rsp.resultVal = resultVal;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_RESET_DATA_SIZE_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_addDataSentSizebyCid_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_addDataSentSizebyCid_RSP_Rsp.resultVal = resultVal;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_ADD_DATA_SENT_SIZE_BY_CID_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_addDataRcvSizebyCid_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_addDataRcvSizebyCid_RSP_Rsp.resultVal = resultVal;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_DATA_ADD_DATA_RCV_SIZE_BY_CID_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_PBK_SendUsimHdkReadReq(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_READ_USIM_PBK_HDK_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_READ_USIM_PBK_HDK_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_PBK_SendUsimHdkUpdateReq(UInt32 tid, UInt8 clientID, HDKString_t hidden_key)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	memcpy(&req.req_rep_u.CAPI2_PBK_SendUsimHdkUpdateReq_Req.hidden_key , hidden_key, sizeof(  HDKString_t ));
	req.respId = MSG_WRITE_USIM_PBK_HDK_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_WRITE_USIM_PBK_HDK_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_PBK_SendUsimAasReadReq(UInt32 tid, UInt8 clientID, UInt16 index, PBK_Id_t pbk_id)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PBK_SendUsimAasReadReq_Req.index = index;
	req.req_rep_u.CAPI2_PBK_SendUsimAasReadReq_Req.pbk_id = pbk_id;
	req.respId = MSG_READ_USIM_PBK_ALPHA_AAS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_READ_USIM_PBK_ALPHA_AAS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_PBK_SendUsimAasUpdateReq(UInt32 tid, UInt8 clientID, UInt16 index, PBK_Id_t pbk_id, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, uchar_ptr_t alpha)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PBK_SendUsimAasUpdateReq_Req.index = index;
	req.req_rep_u.CAPI2_PBK_SendUsimAasUpdateReq_Req.pbk_id = pbk_id;
	req.req_rep_u.CAPI2_PBK_SendUsimAasUpdateReq_Req.alpha_coding = alpha_coding;
	req.req_rep_u.CAPI2_PBK_SendUsimAasUpdateReq_Req.alpha_size = alpha_size;
	req.req_rep_u.CAPI2_PBK_SendUsimAasUpdateReq_Req.alpha = alpha;
	req.respId = MSG_WRITE_USIM_PBK_ALPHA_AAS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_WRITE_USIM_PBK_ALPHA_AAS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_PBK_SendUsimGasReadReq(UInt32 tid, UInt8 clientID, UInt16 index, PBK_Id_t pbk_id)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PBK_SendUsimGasReadReq_Req.index = index;
	req.req_rep_u.CAPI2_PBK_SendUsimGasReadReq_Req.pbk_id = pbk_id;
	req.respId = MSG_READ_USIM_PBK_ALPHA_GAS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_READ_USIM_PBK_ALPHA_GAS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_PBK_SendUsimGasUpdateReq(UInt32 tid, UInt8 clientID, UInt16 index, PBK_Id_t pbk_id, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, uchar_ptr_t alpha)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PBK_SendUsimGasUpdateReq_Req.index = index;
	req.req_rep_u.CAPI2_PBK_SendUsimGasUpdateReq_Req.pbk_id = pbk_id;
	req.req_rep_u.CAPI2_PBK_SendUsimGasUpdateReq_Req.alpha_coding = alpha_coding;
	req.req_rep_u.CAPI2_PBK_SendUsimGasUpdateReq_Req.alpha_size = alpha_size;
	req.req_rep_u.CAPI2_PBK_SendUsimGasUpdateReq_Req.alpha = alpha;
	req.respId = MSG_WRITE_USIM_PBK_ALPHA_GAS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_WRITE_USIM_PBK_ALPHA_GAS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_PBK_SendUsimAasInfoReq(UInt32 tid, UInt8 clientID, PBK_Id_t pbk_id)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PBK_SendUsimAasInfoReq_Req.pbk_id = pbk_id;
	req.respId = MSG_GET_USIM_PBK_ALPHA_INFO_AAS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_GET_USIM_PBK_ALPHA_INFO_AAS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_PBK_SendUsimGasInfoReq(UInt32 tid, UInt8 clientID, PBK_Id_t pbk_id)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PBK_SendUsimGasInfoReq_Req.pbk_id = pbk_id;
	req.respId = MSG_GET_USIM_PBK_ALPHA_INFO_GAS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_GET_USIM_PBK_ALPHA_INFO_GAS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_LCS_ServiceControl(UInt32 tid, UInt8 clientID, LcsServiceType_t inServiceType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_LCS_ServiceControl_Req.inServiceType = inServiceType;
	req.respId = MSG_LCS_SERVICE_CONTROL_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_LCS_SERVICE_CONTROL_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_LCS_PowerConfig(UInt32 tid, UInt8 clientID, LcsPowerState_t inPowerSate)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_LCS_PowerConfig_Req.inPowerSate = inPowerSate;
	req.respId = MSG_LCS_POWER_CONFIG_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_LCS_POWER_CONFIG_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_LCS_ServiceQuery(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_LCS_SERVICE_QUERY_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_LCS_SERVICE_QUERY_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_LCS_StopPosReq(UInt32 tid, UInt8 clientID, LcsHandle_t inLcsHandle)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_LCS_StopPosReq_Req.inLcsHandle = inLcsHandle;
	req.respId = MSG_LCS_STOP_POS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_LCS_STOP_POS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_LCS_GetPosition(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_LCS_GET_POSITION_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_LCS_GET_POSITION_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_LCS_ConfigSet(UInt32 tid, UInt8 clientID, UInt32 configId, UInt32 value)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_LCS_ConfigSet_Req.configId = configId;
	req.req_rep_u.CAPI2_LCS_ConfigSet_Req.value = value;
	req.respId = MSG_LCS_CONFIG_SET_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_LCS_CONFIG_SET_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_LCS_ConfigGet(UInt32 tid, UInt8 clientID, UInt32 configId)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_LCS_ConfigGet_Req.configId = configId;
	req.respId = MSG_LCS_CONFIG_GET_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_LCS_CONFIG_GET_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_LCS_SuplVerificationRsp(UInt32 tid, UInt8 clientID, LcsSuplSessionHdl_t inSessionHdl, Boolean inIsAllowed)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_LCS_SuplVerificationRsp_Req.inSessionHdl = inSessionHdl;
	req.req_rep_u.CAPI2_LCS_SuplVerificationRsp_Req.inIsAllowed = inIsAllowed;
	req.respId = MSG_LCS_SUPL_VERIFICATION_RSP_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_LCS_SUPL_VERIFICATION_RSP_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_LCS_SuplConnectRsp(UInt32 tid, UInt8 clientID, LcsSuplSessionHdl_t inSessionHdl, LcsSuplConnectHdl_t inConnectHdl)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_LCS_SuplConnectRsp_Req.inSessionHdl = inSessionHdl;
	req.req_rep_u.CAPI2_LCS_SuplConnectRsp_Req.inConnectHdl = inConnectHdl;
	req.respId = MSG_LCS_SUPL_CONNECT_RSP_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_LCS_SUPL_CONNECT_RSP_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_LCS_SuplDisconnected(UInt32 tid, UInt8 clientID, LcsSuplSessionHdl_t inSessionHdl, LcsSuplConnectHdl_t inConnectHdl)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_LCS_SuplDisconnected_Req.inSessionHdl = inSessionHdl;
	req.req_rep_u.CAPI2_LCS_SuplDisconnected_Req.inConnectHdl = inConnectHdl;
	req.respId = MSG_LCS_SUPL_DISCONNECTED_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_LCS_SUPL_DISCONNECTED_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_LCS_CmdData(UInt32 tid, UInt8 clientID, CAPI2_LcsCmdData_t inCmdData)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_LCS_CmdData_Req.inCmdData = inCmdData;
	req.respId = MSG_LCS_CMD_DATA_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_LCS_CMD_DATA_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_GPIO_ConfigOutput_64Pin_RSP(UInt32 tid, UInt8 clientID, Boolean status)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_GPIO_ConfigOutput_64Pin_RSP_Rsp.status = status;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_GPIO_CONFIG_OUTPUT_64PIN_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_GPS_Control_RSP(UInt32 tid, UInt8 clientID, UInt32 u32Param)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_GPS_Control_RSP_Rsp.u32Param = u32Param;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_GPS_CONTROL_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_FFS_Control_RSP(UInt32 tid, UInt8 clientID, UInt32 param)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_FFS_Control_RSP_Rsp.param = param;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_FFS_CONTROL_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_CP2AP_PedestalMode_Control_RSP(UInt32 tid, UInt8 clientID, Boolean status)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CP2AP_PedestalMode_Control_RSP_Rsp.status = status;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_CP2AP_PEDESTALMODE_CONTROL_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_AUDIO_ASIC_SetAudioMode(UInt32 tid, UInt8 clientID, AudioMode_t mode)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_AUDIO_ASIC_SetAudioMode_Req.mode = mode;
	req.respId = MSG_AUDIO_ASIC_SetAudioMode_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_AUDIO_ASIC_SetAudioMode_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SPEAKER_StartTone(UInt32 tid, UInt8 clientID, SpeakerTone_t tone, UInt8 duration)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SPEAKER_StartTone_Req.tone = tone;
	req.req_rep_u.CAPI2_SPEAKER_StartTone_Req.duration = duration;
	req.respId = MSG_SPEAKER_StartTone_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SPEAKER_StartTone_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SPEAKER_StartGenericTone(UInt32 tid, UInt8 clientID, Boolean superimpose, UInt16 tone_duration, UInt16 f1, UInt16 f2, UInt16 f3)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SPEAKER_StartGenericTone_Req.superimpose = superimpose;
	req.req_rep_u.CAPI2_SPEAKER_StartGenericTone_Req.tone_duration = tone_duration;
	req.req_rep_u.CAPI2_SPEAKER_StartGenericTone_Req.f1 = f1;
	req.req_rep_u.CAPI2_SPEAKER_StartGenericTone_Req.f2 = f2;
	req.req_rep_u.CAPI2_SPEAKER_StartGenericTone_Req.f3 = f3;
	req.respId = MSG_SPEAKER_StartGenericTone_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SPEAKER_StartGenericTone_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SPEAKER_StopTone(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SPEAKER_StopTone_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SPEAKER_StopTone_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_AUDIO_Turn_EC_NS_OnOff(UInt32 tid, UInt8 clientID, Boolean ec_on_off, Boolean ns_on_off)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_AUDIO_Turn_EC_NS_OnOff_Req.ec_on_off = ec_on_off;
	req.req_rep_u.CAPI2_AUDIO_Turn_EC_NS_OnOff_Req.ns_on_off = ns_on_off;
	req.respId = MSG_AUDIO_Turn_EC_NS_OnOff_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_AUDIO_Turn_EC_NS_OnOff_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_ECHO_SetDigitalTxGain(UInt32 tid, UInt8 clientID, int16_t digital_gain_step)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_ECHO_SetDigitalTxGain_Req.digital_gain_step = digital_gain_step;
	req.respId = MSG_ECHO_SetDigitalTxGain_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_ECHO_SetDigitalTxGain_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_RIPCMDQ_Connect_Uplink(UInt32 tid, UInt8 clientID, Boolean Uplink)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_RIPCMDQ_Connect_Uplink_Req.Uplink = Uplink;
	req.respId = MSG_RIPCMDQ_Connect_Uplink_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_RIPCMDQ_Connect_Uplink_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_RIPCMDQ_Connect_Downlink(UInt32 tid, UInt8 clientID, Boolean Downlink)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_RIPCMDQ_Connect_Downlink_Req.Downlink = Downlink;
	req.respId = MSG_RIPCMDQ_Connect_Downlink_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_RIPCMDQ_Connect_Downlink_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_VOLUMECTRL_SetBasebandVolume(UInt32 tid, UInt8 clientID, UInt8 level, UInt8 chnl, UInt16 *audio_atten, UInt8 extid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_VOLUMECTRL_SetBasebandVolume_Req.level = level;
	req.req_rep_u.CAPI2_VOLUMECTRL_SetBasebandVolume_Req.chnl = chnl;
	req.req_rep_u.CAPI2_VOLUMECTRL_SetBasebandVolume_Req.audio_atten = audio_atten;
	req.req_rep_u.CAPI2_VOLUMECTRL_SetBasebandVolume_Req.extid = extid;
	req.respId = MSG_VOLUMECTRL_SetBasebandVolume_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_VOLUMECTRL_SetBasebandVolume_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_DIAG_ApiMeasurmentReportReq(UInt32 tid, UInt8 clientID, Boolean inPeriodicReport, UInt32 inTimeInterval)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DIAG_ApiMeasurmentReportReq_Req.inPeriodicReport = inPeriodicReport;
	req.req_rep_u.CAPI2_DIAG_ApiMeasurmentReportReq_Req.inTimeInterval = inTimeInterval;
	req.respId = MSG_DIAG_MEASURE_REPORT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DIAG_MEASURE_REPORT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_PMU_BattChargingNotification(UInt32 tid, UInt8 clientID, HAL_EM_BATTMGR_Charger_t chargeType, HAL_EM_BATTMGR_Charger_InOut_t inOut, UInt8 status)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PMU_BattChargingNotification_Req.chargeType = chargeType;
	req.req_rep_u.CAPI2_PMU_BattChargingNotification_Req.inOut = inOut;
	req.req_rep_u.CAPI2_PMU_BattChargingNotification_Req.status = status;
	req.respId = MSG_PMU_BATT_CHARGING_NOTIFICATION_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_PMU_BATT_CHARGING_NOTIFICATION_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_PATCH_GetRevision(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_PATCH_GET_REVISION_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_PATCH_GET_REVISION_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_RTC_SetTime_RSP(UInt32 tid, UInt8 clientID, Boolean status)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_RTC_SetTime_RSP_Rsp.status = status;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_RTC_SetTime_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_RTC_SetDST_RSP(UInt32 tid, UInt8 clientID, Boolean status)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_RTC_SetDST_RSP_Rsp.status = status;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_RTC_SetDST_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_RTC_SetTimeZone_RSP(UInt32 tid, UInt8 clientID, Boolean status)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_RTC_SetTimeZone_RSP_Rsp.status = status;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_RTC_SetTimeZone_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_MS_InitCallCfg(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_INITCALLCFG_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_INITCALLCFG_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_InitFaxConfig(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_INITFAXCFG_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_INITFAXCFG_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_InitVideoCallCfg(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_INITVIDEOCALLCFG_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_INITVIDEOCALLCFG_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_InitCallCfgAmpF(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_INITCALLCFGAMPF_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_INITCALLCFGAMPF_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_RTC_GetTime_RSP(UInt32 tid, UInt8 clientID, RTCTime_t time)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_RTC_GetTime_RSP_Rsp.time = time;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_RTC_GetTime_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_RTC_GetTimeZone_RSP(UInt32 tid, UInt8 clientID, Int8 timeZone)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_RTC_GetTimeZone_RSP_Rsp.timeZone = timeZone;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_RTC_GetTimeZone_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_RTC_GetDST_RSP(UInt32 tid, UInt8 clientID, UInt8 dst)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_RTC_GetDST_RSP_Rsp.dst = dst;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_RTC_GetDST_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SMS_GetMeSmsBufferStatus_RSP(UInt32 tid, UInt8 clientID, UInt32 bfree, UInt32 bused)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SMS_GetMeSmsBufferStatus_RSP_Rsp.bfree = bfree;
	req.req_rep_u.CAPI2_SMS_GetMeSmsBufferStatus_RSP_Rsp.bused = bused;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_SMS_GETMESMS_BUF_STATUS_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SMS_GetRecordNumberOfReplaceSMS_RSP(UInt32 tid, UInt8 clientID, UInt16 recordNumber)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SMS_GetRecordNumberOfReplaceSMS_RSP_Rsp.recordNumber = recordNumber;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_SMS_GETRECORDNUMBEROFREPLACESMS_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_PMU_BattFullInd(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_PMU_BATT_FULL_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_PMU_BATT_FULL_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_FLASH_SaveImage_RSP(UInt32 tid, UInt8 clientID, Boolean status)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_FLASH_SaveImage_RSP_Rsp.status = status;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_FLASH_SAVEIMAGE_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_AUDIO_GetSettings(UInt32 tid, UInt8 clientID, UInt8 audioMode)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_AUDIO_GetSettings_Req.audioMode = audioMode;
	req.respId = MSG_AUDIO_GET_SETTINGS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_AUDIO_GET_SETTINGS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIMLOCK_GetStatus_RSP(UInt32 tid, UInt8 clientID, SIMLOCK_STATE_t simlock_state)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIMLOCK_GetStatus_RSP_Rsp.simlock_state = simlock_state;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_SIMLOCK_GET_STATUS_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SIMLOCK_SetStatus(UInt32 tid, UInt8 clientID, SIMLOCK_STATE_t *simlock_state)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIMLOCK_SetStatus_Req.simlock_state = simlock_state;
	req.respId = MSG_SIMLOCK_SET_STATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIMLOCK_SET_STATUS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYSPARM_GetActualLowVoltReading(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYSPARM_GET_ACTUAL_LOW_VOLT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARM_GET_ACTUAL_LOW_VOLT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYSPARM_GetActual4p2VoltReading(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYSPARM_GET_ACTUAL_4P2_VOLT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARM_GET_ACTUAL_4P2_VOLT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_program_equalizer(UInt32 tid, UInt8 clientID, Int32 equalizer)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_program_equalizer_Req.equalizer = equalizer;
	req.respId = MSG_PROG_EQU_TYPE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_PROG_EQU_TYPE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_program_poly_equalizer(UInt32 tid, UInt8 clientID, Int32 equalizer)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_program_poly_equalizer_Req.equalizer = equalizer;
	req.respId = MSG_PROG_POLY_EQU_TYPE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_PROG_POLY_EQU_TYPE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_program_FIR_IIR_filter(UInt32 tid, UInt8 clientID, UInt16 audio_mode)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_program_FIR_IIR_filter_Req.audio_mode = audio_mode;
	req.respId = MSG_PROG_FIR_IIR_FILTER_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_PROG_FIR_IIR_FILTER_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_program_poly_FIR_IIR_filter(UInt32 tid, UInt8 clientID, UInt16 audio_mode)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_program_poly_FIR_IIR_filter_Req.audio_mode = audio_mode;
	req.respId = MSG_PROG_POLY_FIR_IIR_FILTER_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_PROG_POLY_FIR_IIR_FILTER_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_audio_control_generic(UInt32 tid, UInt8 clientID, UInt32 param1, UInt32 param2, UInt32 param3, UInt32 param4)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_audio_control_generic_Req.param1 = param1;
	req.req_rep_u.CAPI2_audio_control_generic_Req.param2 = param2;
	req.req_rep_u.CAPI2_audio_control_generic_Req.param3 = param3;
	req.req_rep_u.CAPI2_audio_control_generic_Req.param4 = param4;
	req.respId = MSG_AUDIO_CTRL_GENERIC_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_AUDIO_CTRL_GENERIC_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_audio_control_dsp(UInt32 tid, UInt8 clientID, UInt32 param1, UInt32 param2, UInt32 param3, UInt32 param4)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_audio_control_dsp_Req.param1 = param1;
	req.req_rep_u.CAPI2_audio_control_dsp_Req.param2 = param2;
	req.req_rep_u.CAPI2_audio_control_dsp_Req.param3 = param3;
	req.req_rep_u.CAPI2_audio_control_dsp_Req.param4 = param4;
	req.respId = MSG_AUDIO_CTRL_DSP_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_AUDIO_CTRL_DSP_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_FFS_Read_RSP(UInt32 tid, UInt8 clientID, FFS_Data_t *ffsReadRsp)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_FFS_Read_RSP_Rsp.ffsReadRsp = ffsReadRsp;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_FFS_READ_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DIAG_ApiCellLockReq(UInt32 tid, UInt8 clientID, Boolean cell_lockEnable)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DIAG_ApiCellLockReq_Req.cell_lockEnable = cell_lockEnable;
	req.respId = MSG_DIAG_CELLLOCK_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DIAG_CELLLOCK_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_DIAG_ApiCellLockStatus(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_DIAG_CELLLOCK_STATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DIAG_CELLLOCK_STATUS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_LCS_RegisterRrlpDataHandler(UInt32 tid, UInt8 clientID, LcsClientInfo_t inRrlpHandler)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_LCS_RegisterRrlpDataHandler_Req.inRrlpHandler = inRrlpHandler;
	req.respId = MSG_LCS_REG_RRLP_HDL_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_LCS_REG_RRLP_HDL_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_LCS_RegisterRrcDataHandler(UInt32 tid, UInt8 clientID, LcsClientInfo_t inRrcHandler)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_LCS_RegisterRrcDataHandler_Req.inRrcHandler = inRrcHandler;
	req.respId = MSG_LCS_REG_RRC_HDL_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_LCS_REG_RRC_HDL_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_CC_IsThereEmergencyCall(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_CC_IS_THERE_EMERGENCY_CALL_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_CC_IS_THERE_EMERGENCY_CALL_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYSPARM_GetBattLowThresh(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYSPARM_GET_LOW_THRESHOLD_VOLT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARM_GET_LOW_THRESHOLD_VOLT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYSPARM_GetDefault4p2VoltReading(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYSPARM_GET_DEFAULT_4P2_VOLT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARM_GET_DEFAULT_4P2_VOLT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_L1_bb_isLocked(UInt32 tid, UInt8 clientID, Boolean watch)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_L1_bb_isLocked_Req.watch = watch;
	req.respId = MSG_L1_BB_ISLOCKED_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_L1_BB_ISLOCKED_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_MS_ForcePsReleaseReq(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_FORCE_PS_REL_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_FORCE_PS_REL_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYS_ProcessNoRfOnlyChgReq(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYS_POWERUP_NORF_CHG_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYS_POWERUP_NORF_CHG_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYSPARM_GetGPIO_Value(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYSPARM_GET_GPIO_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARM_GET_GPIO_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYS_EnableCellInfoMsg(UInt32 tid, UInt8 clientID, Boolean inEnableCellInfoMsg)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SYS_EnableCellInfoMsg_Req.inEnableCellInfoMsg = inEnableCellInfoMsg;
	req.respId = MSG_SYS_ENABLE_CELL_INFO_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYS_ENABLE_CELL_INFO_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYSPARM_SetHSDPAPHYCategory(UInt32 tid, UInt8 clientID, UInt32 inHsdpaPhyCat)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SYSPARM_SetHSDPAPHYCategory_Req.inHsdpaPhyCat = inHsdpaPhyCat;
	req.respId = MSG_SYS_SET_HSDPA_CATEGORY_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYS_SET_HSDPA_CATEGORY_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYSPARM_GetHSDPAPHYCategory(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYS_GET_HSDPA_CATEGORY_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYS_GET_HSDPA_CATEGORY_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SMS_StartMultiSmsTransferReq(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_START_MULTI_SMS_XFR_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_START_MULTI_SMS_XFR_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SMS_StopMultiSmsTransferReq(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_STOP_MULTI_SMS_XFR_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_STOP_MULTI_SMS_XFR_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_SendWriteSmsReq(UInt32 tid, UInt8 clientID, UInt16 rec_no, UInt8 tp_pid, SIMSMSMesg_t *p_sms_mesg)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIM_SendWriteSmsReq_Req.rec_no = rec_no;
	req.req_rep_u.CAPI2_SIM_SendWriteSmsReq_Req.tp_pid = tp_pid;
	req.req_rep_u.CAPI2_SIM_SendWriteSmsReq_Req.p_sms_mesg = p_sms_mesg;
	req.respId = MSG_SIM_SMS_WRITE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_SMS_WRITE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_BATTMGR_GetChargingStatus(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_BATTMGR_GETCHARGINGSTATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_BATTMGR_GETCHARGINGSTATUS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_BATTMGR_GetPercentageLevel(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_BATTMGR_GETPERCENTAGELEVEL_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_BATTMGR_GETPERCENTAGELEVEL_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_BATTMGR_IsBatteryPresent(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_BATTMGR_ISBATTERYPRESENT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_BATTMGR_ISBATTERYPRESENT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_BATTMGR_IsChargerPlugIn(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_BATTMGR_ISCHARGERPLUGIN_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_BATTMGR_ISCHARGERPLUGIN_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SYSPARM_GetBattTable(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYSPARM_GETBATT_TABLE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARM_GETBATT_TABLE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_BATTMGR_GetLevel(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_BATTMGR_GET_LEVEL_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_BATTMGR_GET_LEVEL_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SMS_WriteSMSPduToSIMRecordReq(UInt32 tid, UInt8 clientID, UInt8 length, UInt8 *inSmsPdu, SIMSMSMesgStatus_t smsState, UInt8 rec_no)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SMS_WriteSMSPduToSIMRecordReq_Req.length = length;
	req.req_rep_u.CAPI2_SMS_WriteSMSPduToSIMRecordReq_Req.inSmsPdu = inSmsPdu;
	req.req_rep_u.CAPI2_SMS_WriteSMSPduToSIMRecordReq_Req.smsState = smsState;
	req.req_rep_u.CAPI2_SMS_WriteSMSPduToSIMRecordReq_Req.rec_no = rec_no;
	req.respId = MSG_SMS_WRITESMSPDUTO_SIMRECORD_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_WRITESMSPDUTO_SIMRECORD_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_ADCMGR_MultiChStart(UInt32 tid, UInt8 clientID, CAPI2_ADC_ChannelReq_t *ch)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_ADCMGR_MultiChStart_Req.ch = ch;
	req.respId = MSG_ADC_MULTI_CH_START_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_ADC_MULTI_CH_START_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_InterTaskMsgToCP(UInt32 tid, UInt8 clientID, InterTaskMsg_t *inPtrMsg)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_InterTaskMsgToCP_Req.inPtrMsg = inPtrMsg;
	req.respId = MSG_INTERTASK_MSG_TO_CP_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_INTERTASK_MSG_TO_CP_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_InterTaskMsgToAP_RSP(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_INTERTASK_MSG_TO_AP_RSP, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}
