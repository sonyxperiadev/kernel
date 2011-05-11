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


/***************************** Generated contents from ../gen/capi2_inc.txt file. ( Do not modify !!! ) Please checkout and modify ../gen/capi2_inc.txt to add any header files *************************/

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
#include "netreg_api_old.h"
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
#include "isim_def.h"
#include "pch_def.h"
#include "pchex_def.h"

#include "engmode_api.h"
#include "sysparm.h"
#include "ms_database_old.h"
#include "ms_database_api.h"
#include "ss_api.h"
#include "sms_api_atc.h"
#include "sms_api_old.h"
#include "sms_api.h"
#include "cc_api_old.h"
#include "sim_api_old.h"
#include "sim_api.h"
#include "phonebk_api_old.h"
#include "phonebk_api.h"
#include "phonectrl_api.h"
#include "isim_api_old.h"
#include "isim_api.h"

#include "util_api_old.h"
#include "util_api.h"
#include "dialstr_api.h"
#include "stk_api_old.h"
#include "stk_api.h"

#include "pch_api_old.h"
#include "pch_api.h"
#include "pchex_api.h"
#include "ss_api_old.h"
#include "lcs_cplane_rrlp_api.h"
#include "cc_api.h"
#include "netreg_api.h"
#include "lcs_ftt_api.h"
#include "lcs_cplane_rrc_api.h"
#include "lcs_cplane_shared_def.h"

#include "capi2_mstruct.h"
#include "capi2_sim_api.h"
#include "capi2_phonectrl_api.h"
#include "capi2_sms_api.h"
#include "capi2_cc_api.h"
#include "capi2_lcs_cplane_api.h"
#include "capi2_ss_api.h"
#include "capi2_phonebk_api.h"
#include "capi2_cmd_resp.h"
#include "capi2_phonectrl_api.h"
//#include "capi2_gen_api.h"	

#include "ipcproperties.h"
#include "rpc_global.h"
#include "rpc_ipc.h"
#include "xdr_porting_layer.h"
#include "xdr.h"
#include "rpc_api.h"

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

#include "capi2_reqrep.h"

#include "capi2_comm.h"
#include "capi2_gen_api.h"
#include "system_api.h"
#include "sim_api_atc.h"
#include "phonebk_api_atc.h"


#define _D(a) a

#ifdef WIN32
#define _A(a) AP_ ## a
#else
#define _A(a) a
#endif

/********************************** End Hdr file generated contents ************************************************************/






void CAPI2_MS_IsGSMRegistered(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GSM_REGISTERED_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GSM_REGISTERED_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_MS_IsGPRSRegistered(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GPRS_REGISTERED_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GPRS_REGISTERED_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_MS_GetGSMRegCause(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GSM_CAUSE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GSM_CAUSE_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_MS_GetGPRSRegCause(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GPRS_CAUSE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GPRS_CAUSE_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_MS_GetRegisteredLAC(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_REGISTERED_LAC_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_REGISTERED_LAC_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_MS_GetPlmnMCC(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GET_PLMN_MCC_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GET_PLMN_MCC_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_MS_GetPlmnMNC(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GET_PLMN_MNC_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GET_PLMN_MNC_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_PhoneCtrlApi_ProcessPowerDownReq(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYS_POWERDOWN_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SYS_POWERDOWN_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PhoneCtrlApi_ProcessNoRfReq(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYS_POWERUP_NORF_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SYS_POWERUP_NORF_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PhoneCtrlApi_ProcessNoRfToCalib(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYS_NORF_CALIB_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SYS_NORF_CALIB_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PhoneCtrlApi_ProcessPowerUpReq(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYS_POWERUP_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SYS_POWERUP_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_MS_IsGprsAllowed(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GPRS_ALLOWED_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GPRS_ALLOWED_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_MS_GetCurrentRAT(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GET_CURRENT_RAT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GET_CURRENT_RAT_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_MS_GetCurrentBand(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GET_CURRENT_BAND_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GET_CURRENT_BAND_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SimApi_UpdateSMSCapExceededFlag(ClientInfo_t* inClientInfoPtr, Boolean cap_exceeded)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_UpdateSMSCapExceededFlag_Req.cap_exceeded = cap_exceeded;
	req.respId = MSG_SIM_UPDATE_SMSCAPEXC_FLAG_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_UPDATE_SMSCAPEXC_FLAG_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_NetRegApi_SelectBand(ClientInfo_t* inClientInfoPtr, BandSelect_t bandSelect)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_NetRegApi_SelectBand_Req.bandSelect = bandSelect;
	req.respId = MSG_MS_SELECT_BAND_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_MS_SELECT_BAND_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_NetRegApi_SetSupportedRATandBand(ClientInfo_t* inClientInfoPtr, RATSelect_t RAT_cap, BandSelect_t band_cap)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_NetRegApi_SetSupportedRATandBand_Req.RAT_cap = RAT_cap;
	req.req_rep_u.CAPI2_NetRegApi_SetSupportedRATandBand_Req.band_cap = band_cap;
	req.respId = MSG_MS_SET_RAT_BAND_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_MS_SET_RAT_BAND_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_NetRegApi_CellLock(ClientInfo_t* inClientInfoPtr, Boolean Enable, BandSelect_t lockBand, UInt8 lockrat, UInt16 lockuarfcn, UInt16 lockpsc)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_NetRegApi_CellLock_Req.Enable = Enable;
	req.req_rep_u.CAPI2_NetRegApi_CellLock_Req.lockBand = lockBand;
	req.req_rep_u.CAPI2_NetRegApi_CellLock_Req.lockrat = lockrat;
	req.req_rep_u.CAPI2_NetRegApi_CellLock_Req.lockuarfcn = lockuarfcn;
	req.req_rep_u.CAPI2_NetRegApi_CellLock_Req.lockpsc = lockpsc;
	req.respId = MSG_MS_CELL_LOCK_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_MS_CELL_LOCK_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PLMN_GetCountryByMcc(UInt32 tid, UInt8 clientID, UInt16 mcc)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PLMN_GetCountryByMcc_Req.mcc = mcc;
	req.respId = MSG_MS_GET_MCC_COUNTRY_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GET_MCC_COUNTRY_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_MS_GetPLMNEntryByIndex(UInt32 tid, UInt8 clientID, UInt16 index, Boolean ucs2)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_MS_GetPLMNEntryByIndex_Req.index = index;
	req.req_rep_u.CAPI2_MS_GetPLMNEntryByIndex_Req.ucs2 = ucs2;
	req.respId = MSG_MS_PLMN_INFO_BY_CODE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_PLMN_INFO_BY_CODE_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_MS_GetPLMNListSize(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_PLMN_LIST_SIZE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_PLMN_LIST_SIZE_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_MS_GetPLMNByCode(UInt32 tid, UInt8 clientID, Boolean ucs2, UInt16 plmn_mcc, UInt16 plmn_mnc)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_MS_GetPLMNByCode_Req.ucs2 = ucs2;
	req.req_rep_u.CAPI2_MS_GetPLMNByCode_Req.plmn_mcc = plmn_mcc;
	req.req_rep_u.CAPI2_MS_GetPLMNByCode_Req.plmn_mnc = plmn_mnc;
	req.respId = MSG_MS_PLMN_INFO_BY_INDEX_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_PLMN_INFO_BY_INDEX_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_NetRegApi_PlmnSelect(ClientInfo_t* inClientInfoPtr, Boolean ucs2, PlmnSelectMode_t selectMode, PlmnSelectFormat_t format, char* plmnValue)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_NetRegApi_PlmnSelect_Req.ucs2 = ucs2;
	req.req_rep_u.CAPI2_NetRegApi_PlmnSelect_Req.selectMode = selectMode;
	req.req_rep_u.CAPI2_NetRegApi_PlmnSelect_Req.format = format;
	req.req_rep_u.CAPI2_NetRegApi_PlmnSelect_Req.plmnValue = plmnValue;
	req.respId = MSG_PLMN_SELECT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PLMN_SELECT_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_NetRegApi_AbortPlmnSelect(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_PLMN_ABORT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_MS_PLMN_ABORT_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_MS_GetPlmnMode(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GET_PLMN_MODE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GET_PLMN_MODE_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_NetRegApi_SetPlmnMode(ClientInfo_t* inClientInfoPtr, PlmnSelectMode_t mode)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_NetRegApi_SetPlmnMode_Req.mode = mode;
	req.respId = MSG_MS_SET_PLMN_MODE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_MS_SET_PLMN_MODE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_MS_GetPlmnFormat(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GET_PLMN_FORMAT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GET_PLMN_FORMAT_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_MS_SetPlmnFormat(UInt32 tid, UInt8 clientID, PlmnSelectFormat_t format)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_MS_SetPlmnFormat_Req.format = format;
	req.respId = MSG_MS_SET_PLMN_FORMAT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_SET_PLMN_FORMAT_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_MS_IsMatchedPLMN(UInt32 tid, UInt8 clientID, UInt16 net_mcc, UInt8 net_mnc, UInt16 mcc, UInt8 mnc)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_MS_IsMatchedPLMN_Req.net_mcc = net_mcc;
	req.req_rep_u.CAPI2_MS_IsMatchedPLMN_Req.net_mnc = net_mnc;
	req.req_rep_u.CAPI2_MS_IsMatchedPLMN_Req.mcc = mcc;
	req.req_rep_u.CAPI2_MS_IsMatchedPLMN_Req.mnc = mnc;
	req.respId = MSG_MS_MATCH_PLMN_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_MATCH_PLMN_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_NetRegApi_SearchAvailablePLMN(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_SEARCH_PLMN_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_MS_SEARCH_PLMN_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_NetRegApi_AbortSearchPLMN(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_ABORT_PLMN_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_MS_ABORT_PLMN_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_NetRegApi_AutoSearchReq(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_AUTO_SEARCH_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_MS_AUTO_SEARCH_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_NetRegApi_GetPLMNNameByCode(ClientInfo_t* inClientInfoPtr, UInt16 mcc, UInt8 mnc, UInt16 lac, Boolean ucs2)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_NetRegApi_GetPLMNNameByCode_Req.mcc = mcc;
	req.req_rep_u.CAPI2_NetRegApi_GetPLMNNameByCode_Req.mnc = mnc;
	req.req_rep_u.CAPI2_NetRegApi_GetPLMNNameByCode_Req.lac = lac;
	req.req_rep_u.CAPI2_NetRegApi_GetPLMNNameByCode_Req.ucs2 = ucs2;
	req.respId = MSG_MS_PLMN_NAME_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_MS_PLMN_NAME_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PhoneCtrlApi_GetSystemState(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYS_GET_SYSTEM_STATE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SYS_GET_SYSTEM_STATE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PhoneCtrlApi_SetSystemState(ClientInfo_t* inClientInfoPtr, SystemState_t state)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PhoneCtrlApi_SetSystemState_Req.state = state;
	req.respId = MSG_SYS_SET_SYSTEM_STATE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SYS_SET_SYSTEM_STATE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PhoneCtrlApi_GetRxSignalInfo(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYS_GET_RX_LEVEL_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SYS_GET_RX_LEVEL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SYS_GetGSMRegistrationStatus(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYS_GET_GSMREG_STATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYS_GET_GSMREG_STATUS_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SYS_GetGPRSRegistrationStatus(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYS_GET_GPRSREG_STATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYS_GET_GPRSREG_STATUS_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SYS_IsRegisteredGSMOrGPRS(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYS_GET_REG_STATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYS_GET_REG_STATUS_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_MS_IsRegisteredHomePLMN(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_HOME_PLMN_REG_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_HOME_PLMN_REG_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_PhoneCtrlApi_SetPowerDownTimer(ClientInfo_t* inClientInfoPtr, UInt8 powerDownTimer)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PhoneCtrlApi_SetPowerDownTimer_Req.powerDownTimer = powerDownTimer;
	req.respId = MSG_SET_POWER_DOWN_TIMER_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SET_POWER_DOWN_TIMER_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_GetSmsParamRecNum(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_PARAM_REC_NUM_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_PARAM_REC_NUM_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_GetSmsMemExceededFlag(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_GET_SMSMEMEXC_FLAG_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_GET_SMSMEMEXC_FLAG_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_IsTestSIM(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_IS_TEST_SIM_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_IS_TEST_SIM_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_IsPINRequired(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_PIN_REQ_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_PIN_REQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_GetCardPhase(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_CARD_PHASE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_CARD_PHASE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_GetSIMType(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_TYPE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_TYPE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_GetPresentStatus(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_PRESENT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_PRESENT_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_IsOperationRestricted(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_PIN_OPERATION_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_PIN_OPERATION_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_IsPINBlocked(ClientInfo_t* inClientInfoPtr, CHV_t chv)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_IsPINBlocked_Req.chv = chv;
	req.respId = MSG_SIM_PIN_BLOCK_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_PIN_BLOCK_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_IsPUKBlocked(ClientInfo_t* inClientInfoPtr, CHV_t chv)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_IsPUKBlocked_Req.chv = chv;
	req.respId = MSG_SIM_PUK_BLOCK_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_PUK_BLOCK_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_IsInvalidSIM(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_IS_INVALID_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_IS_INVALID_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_DetectSim(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_DETECT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_DETECT_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_GetRuimSuppFlag(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_GET_RUIM_SUPP_FLAG_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_GET_RUIM_SUPP_FLAG_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SendVerifyChvReq(ClientInfo_t* inClientInfoPtr, CHV_t chv_select, CHVString_t chv)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SendVerifyChvReq_Req.chv_select = chv_select;
	memcpy(&req.req_rep_u.CAPI2_SimApi_SendVerifyChvReq_Req.chv , chv, sizeof(  CHVString_t ));
	req.respId = MSG_SIM_VERIFY_CHV_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_VERIFY_CHV_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SendChangeChvReq(ClientInfo_t* inClientInfoPtr, CHV_t chv_select, CHVString_t old_chv, CHVString_t new_chv)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SendChangeChvReq_Req.chv_select = chv_select;
	memcpy(&req.req_rep_u.CAPI2_SimApi_SendChangeChvReq_Req.old_chv , old_chv, sizeof(  CHVString_t ));
	memcpy(&req.req_rep_u.CAPI2_SimApi_SendChangeChvReq_Req.new_chv , new_chv, sizeof(  CHVString_t ));
	req.respId = MSG_SIM_CHANGE_CHV_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_CHANGE_CHV_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SendSetChv1OnOffReq(ClientInfo_t* inClientInfoPtr, CHVString_t chv, Boolean enable_flag)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	memcpy(&req.req_rep_u.CAPI2_SimApi_SendSetChv1OnOffReq_Req.chv , chv, sizeof(  CHVString_t ));
	req.req_rep_u.CAPI2_SimApi_SendSetChv1OnOffReq_Req.enable_flag = enable_flag;
	req.respId = MSG_SIM_ENABLE_CHV_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_ENABLE_CHV_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SendUnblockChvReq(ClientInfo_t* inClientInfoPtr, CHV_t chv_select, PUKString_t puk, CHVString_t new_chv)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SendUnblockChvReq_Req.chv_select = chv_select;
	memcpy(&req.req_rep_u.CAPI2_SimApi_SendUnblockChvReq_Req.puk , puk, sizeof(  PUKString_t ));
	memcpy(&req.req_rep_u.CAPI2_SimApi_SendUnblockChvReq_Req.new_chv , new_chv, sizeof(  CHVString_t ));
	req.respId = MSG_SIM_UNBLOCK_CHV_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_UNBLOCK_CHV_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SendSetOperStateReq(ClientInfo_t* inClientInfoPtr, SIMOperState_t oper_state, CHVString_t chv2)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SendSetOperStateReq_Req.oper_state = oper_state;
	memcpy(&req.req_rep_u.CAPI2_SimApi_SendSetOperStateReq_Req.chv2 , chv2, sizeof(  CHVString_t ));
	req.respId = MSG_SIM_SET_FDN_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_SET_FDN_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_IsPbkAccessAllowed(ClientInfo_t* inClientInfoPtr, SIMPBK_ID_t id)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_IsPbkAccessAllowed_Req.id = id;
	req.respId = MSG_SIM_IS_PBK_ALLOWED_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_IS_PBK_ALLOWED_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SendPbkInfoReq(ClientInfo_t* inClientInfoPtr, SIMPBK_ID_t id)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SendPbkInfoReq_Req.id = id;
	req.respId = MSG_SIM_PBK_INFO_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_PBK_INFO_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SendReadAcmMaxReq(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_MAX_ACM_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_MAX_ACM_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SendWriteAcmMaxReq(ClientInfo_t* inClientInfoPtr, CallMeterUnit_t acm_max)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SendWriteAcmMaxReq_Req.acm_max = acm_max;
	req.respId = MSG_SIM_ACM_MAX_UPDATE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_ACM_MAX_UPDATE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SendReadAcmReq(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_ACM_VALUE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_ACM_VALUE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SendWriteAcmReq(ClientInfo_t* inClientInfoPtr, CallMeterUnit_t acm)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SendWriteAcmReq_Req.acm = acm;
	req.respId = MSG_SIM_ACM_UPDATE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_ACM_UPDATE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SendIncreaseAcmReq(ClientInfo_t* inClientInfoPtr, CallMeterUnit_t acm)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SendIncreaseAcmReq_Req.acm = acm;
	req.respId = MSG_SIM_ACM_INCREASE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_ACM_INCREASE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SendReadSvcProvNameReq(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_SVC_PROV_NAME_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_SVC_PROV_NAME_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SendReadPuctReq(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_PUCT_DATA_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_PUCT_DATA_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_GetServiceStatus(ClientInfo_t* inClientInfoPtr, SIMService_t service)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_GetServiceStatus_Req.service = service;
	req.respId = MSG_SIM_SERVICE_STATUS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_SERVICE_STATUS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_GetPinStatus(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_PIN_STATUS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_PIN_STATUS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_IsPinOK(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_PIN_OK_STATUS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_PIN_OK_STATUS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_GetIMSI(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_IMSI_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_IMSI_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_GetGID1(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_GID_DIGIT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_GID_DIGIT_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_GetGID2(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_GID_DIGIT2_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_GID_DIGIT2_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_GetHomePlmn(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_HOME_PLMN_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_HOME_PLMN_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_simmiApi_GetMasterFileId(ClientInfo_t* inClientInfoPtr, APDUFileID_t file_id)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_simmiApi_GetMasterFileId_Req.file_id = file_id;
	req.respId = MSG_SIM_APDU_FILEID_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_APDU_FILEID_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SendOpenSocketReq(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_OPEN_SOCKET_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_OPEN_SOCKET_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SendSelectAppiReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id, const UInt8 *aid_data, UInt8 aid_len)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SendSelectAppiReq_Req.socket_id = socket_id;
	req.req_rep_u.CAPI2_SimApi_SendSelectAppiReq_Req.aid_data = (UInt8*)aid_data;
	req.req_rep_u.CAPI2_SimApi_SendSelectAppiReq_Req.aid_len = aid_len;
	req.respId = MSG_SIM_SELECT_APPLI_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_SELECT_APPLI_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SendDeactivateAppiReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SendDeactivateAppiReq_Req.socket_id = socket_id;
	req.respId = MSG_SIM_DEACTIVATE_APPLI_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_DEACTIVATE_APPLI_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SendCloseSocketReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SendCloseSocketReq_Req.socket_id = socket_id;
	req.respId = MSG_SIM_CLOSE_SOCKET_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_CLOSE_SOCKET_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_GetAtrData(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_ATR_DATA_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_ATR_DATA_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SubmitDFileInfoReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id, APDUFileID_t dfile_id, UInt8 path_len, const UInt16 *select_path)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SubmitDFileInfoReq_Req.socket_id = socket_id;
	req.req_rep_u.CAPI2_SimApi_SubmitDFileInfoReq_Req.dfile_id = dfile_id;
	req.req_rep_u.CAPI2_SimApi_SubmitDFileInfoReq_Req.path_len = path_len;
	req.req_rep_u.CAPI2_SimApi_SubmitDFileInfoReq_Req.select_path = (UInt16*)select_path;
	req.respId = MSG_SIM_DFILE_INFO_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_DFILE_INFO_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SubmitEFileInfoReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 path_len, const UInt16 *select_path)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SubmitEFileInfoReq_Req.socket_id = socket_id;
	req.req_rep_u.CAPI2_SimApi_SubmitEFileInfoReq_Req.efile_id = efile_id;
	req.req_rep_u.CAPI2_SimApi_SubmitEFileInfoReq_Req.dfile_id = dfile_id;
	req.req_rep_u.CAPI2_SimApi_SubmitEFileInfoReq_Req.path_len = path_len;
	req.req_rep_u.CAPI2_SimApi_SubmitEFileInfoReq_Req.select_path = (UInt16*)select_path;
	req.respId = MSG_SIM_EFILE_INFO_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_EFILE_INFO_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SendEFileInfoReq(ClientInfo_t* inClientInfoPtr, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 path_len, const UInt16 *select_path)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SendEFileInfoReq_Req.efile_id = efile_id;
	req.req_rep_u.CAPI2_SimApi_SendEFileInfoReq_Req.dfile_id = dfile_id;
	req.req_rep_u.CAPI2_SimApi_SendEFileInfoReq_Req.path_len = path_len;
	req.req_rep_u.CAPI2_SimApi_SendEFileInfoReq_Req.select_path = (UInt16*)select_path;
	req.respId = MSG_SIM_EFILE_INFO_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_SEND_EFILE_INFO_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SendDFileInfoReq(ClientInfo_t* inClientInfoPtr, APDUFileID_t dfile_id, UInt8 path_len, const UInt16 *select_path)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SendDFileInfoReq_Req.dfile_id = dfile_id;
	req.req_rep_u.CAPI2_SimApi_SendDFileInfoReq_Req.path_len = path_len;
	req.req_rep_u.CAPI2_SimApi_SendDFileInfoReq_Req.select_path = (UInt16*)select_path;
	req.respId = MSG_SIM_DFILE_INFO_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_SEND_DFILE_INFO_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SubmitWholeBinaryEFileReadReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 path_len, const UInt16 *select_path)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SubmitWholeBinaryEFileReadReq_Req.socket_id = socket_id;
	req.req_rep_u.CAPI2_SimApi_SubmitWholeBinaryEFileReadReq_Req.efile_id = efile_id;
	req.req_rep_u.CAPI2_SimApi_SubmitWholeBinaryEFileReadReq_Req.dfile_id = dfile_id;
	req.req_rep_u.CAPI2_SimApi_SubmitWholeBinaryEFileReadReq_Req.path_len = path_len;
	req.req_rep_u.CAPI2_SimApi_SubmitWholeBinaryEFileReadReq_Req.select_path = (UInt16*)select_path;
	req.respId = MSG_SIM_EFILE_DATA_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_WHOLE_EFILE_DATA_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SendWholeBinaryEFileReadReq(ClientInfo_t* inClientInfoPtr, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 path_len, const UInt16 *select_path)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SendWholeBinaryEFileReadReq_Req.efile_id = efile_id;
	req.req_rep_u.CAPI2_SimApi_SendWholeBinaryEFileReadReq_Req.dfile_id = dfile_id;
	req.req_rep_u.CAPI2_SimApi_SendWholeBinaryEFileReadReq_Req.path_len = path_len;
	req.req_rep_u.CAPI2_SimApi_SendWholeBinaryEFileReadReq_Req.select_path = (UInt16*)select_path;
	req.respId = MSG_SIM_EFILE_DATA_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_SEND_WHOLE_EFILE_DATA_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SubmitBinaryEFileReadReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt16 offset, UInt16 length, UInt8 path_len, const UInt16 *select_path)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SubmitBinaryEFileReadReq_Req.socket_id = socket_id;
	req.req_rep_u.CAPI2_SimApi_SubmitBinaryEFileReadReq_Req.efile_id = efile_id;
	req.req_rep_u.CAPI2_SimApi_SubmitBinaryEFileReadReq_Req.dfile_id = dfile_id;
	req.req_rep_u.CAPI2_SimApi_SubmitBinaryEFileReadReq_Req.offset = offset;
	req.req_rep_u.CAPI2_SimApi_SubmitBinaryEFileReadReq_Req.length = length;
	req.req_rep_u.CAPI2_SimApi_SubmitBinaryEFileReadReq_Req.path_len = path_len;
	req.req_rep_u.CAPI2_SimApi_SubmitBinaryEFileReadReq_Req.select_path = (UInt16*)select_path;
	req.respId = MSG_SIM_EFILE_DATA_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_EFILE_DATA_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SendBinaryEFileReadReq(ClientInfo_t* inClientInfoPtr, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt16 offset, UInt16 length, UInt8 path_len, const UInt16 *select_path)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SendBinaryEFileReadReq_Req.efile_id = efile_id;
	req.req_rep_u.CAPI2_SimApi_SendBinaryEFileReadReq_Req.dfile_id = dfile_id;
	req.req_rep_u.CAPI2_SimApi_SendBinaryEFileReadReq_Req.offset = offset;
	req.req_rep_u.CAPI2_SimApi_SendBinaryEFileReadReq_Req.length = length;
	req.req_rep_u.CAPI2_SimApi_SendBinaryEFileReadReq_Req.path_len = path_len;
	req.req_rep_u.CAPI2_SimApi_SendBinaryEFileReadReq_Req.select_path = (UInt16*)select_path;
	req.respId = MSG_SIM_EFILE_DATA_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_SEND_EFILE_DATA_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SubmitRecordEFileReadReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 rec_no, UInt8 length, UInt8 path_len, const UInt16 *select_path)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SubmitRecordEFileReadReq_Req.socket_id = socket_id;
	req.req_rep_u.CAPI2_SimApi_SubmitRecordEFileReadReq_Req.efile_id = efile_id;
	req.req_rep_u.CAPI2_SimApi_SubmitRecordEFileReadReq_Req.dfile_id = dfile_id;
	req.req_rep_u.CAPI2_SimApi_SubmitRecordEFileReadReq_Req.rec_no = rec_no;
	req.req_rep_u.CAPI2_SimApi_SubmitRecordEFileReadReq_Req.length = length;
	req.req_rep_u.CAPI2_SimApi_SubmitRecordEFileReadReq_Req.path_len = path_len;
	req.req_rep_u.CAPI2_SimApi_SubmitRecordEFileReadReq_Req.select_path = (UInt16*)select_path;
	req.respId = MSG_SIM_EFILE_DATA_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_RECORD_EFILE_DATA_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SendRecordEFileReadReq(ClientInfo_t* inClientInfoPtr, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 rec_no, UInt8 length, UInt8 path_len, const UInt16 *select_path)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SendRecordEFileReadReq_Req.efile_id = efile_id;
	req.req_rep_u.CAPI2_SimApi_SendRecordEFileReadReq_Req.dfile_id = dfile_id;
	req.req_rep_u.CAPI2_SimApi_SendRecordEFileReadReq_Req.rec_no = rec_no;
	req.req_rep_u.CAPI2_SimApi_SendRecordEFileReadReq_Req.length = length;
	req.req_rep_u.CAPI2_SimApi_SendRecordEFileReadReq_Req.path_len = path_len;
	req.req_rep_u.CAPI2_SimApi_SendRecordEFileReadReq_Req.select_path = (UInt16*)select_path;
	req.respId = MSG_SIM_EFILE_DATA_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_SEND_RECORD_EFILE_DATA_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SubmitBinaryEFileUpdateReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt16 offset, const UInt8 *ptr, UInt16 length, UInt8 path_len, const UInt16 *select_path)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SubmitBinaryEFileUpdateReq_Req.socket_id = socket_id;
	req.req_rep_u.CAPI2_SimApi_SubmitBinaryEFileUpdateReq_Req.efile_id = efile_id;
	req.req_rep_u.CAPI2_SimApi_SubmitBinaryEFileUpdateReq_Req.dfile_id = dfile_id;
	req.req_rep_u.CAPI2_SimApi_SubmitBinaryEFileUpdateReq_Req.offset = offset;
	req.req_rep_u.CAPI2_SimApi_SubmitBinaryEFileUpdateReq_Req.ptr = (UInt8*)ptr;
	req.req_rep_u.CAPI2_SimApi_SubmitBinaryEFileUpdateReq_Req.length = length;
	req.req_rep_u.CAPI2_SimApi_SubmitBinaryEFileUpdateReq_Req.path_len = path_len;
	req.req_rep_u.CAPI2_SimApi_SubmitBinaryEFileUpdateReq_Req.select_path = (UInt16*)select_path;
	req.respId = MSG_SIM_EFILE_UPDATE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_EFILE_UPDATE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SendBinaryEFileUpdateReq(ClientInfo_t* inClientInfoPtr, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt16 offset, const UInt8 *ptr, UInt16 length, UInt8 path_len, const UInt16 *select_path)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SendBinaryEFileUpdateReq_Req.efile_id = efile_id;
	req.req_rep_u.CAPI2_SimApi_SendBinaryEFileUpdateReq_Req.dfile_id = dfile_id;
	req.req_rep_u.CAPI2_SimApi_SendBinaryEFileUpdateReq_Req.offset = offset;
	req.req_rep_u.CAPI2_SimApi_SendBinaryEFileUpdateReq_Req.ptr = (UInt8*)ptr;
	req.req_rep_u.CAPI2_SimApi_SendBinaryEFileUpdateReq_Req.length = length;
	req.req_rep_u.CAPI2_SimApi_SendBinaryEFileUpdateReq_Req.path_len = path_len;
	req.req_rep_u.CAPI2_SimApi_SendBinaryEFileUpdateReq_Req.select_path = (UInt16*)select_path;
	req.respId = MSG_SIM_EFILE_UPDATE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_SEND_EFILE_UPDATE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SubmitLinearEFileUpdateReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 rec_no, const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SubmitLinearEFileUpdateReq_Req.socket_id = socket_id;
	req.req_rep_u.CAPI2_SimApi_SubmitLinearEFileUpdateReq_Req.efile_id = efile_id;
	req.req_rep_u.CAPI2_SimApi_SubmitLinearEFileUpdateReq_Req.dfile_id = dfile_id;
	req.req_rep_u.CAPI2_SimApi_SubmitLinearEFileUpdateReq_Req.rec_no = rec_no;
	req.req_rep_u.CAPI2_SimApi_SubmitLinearEFileUpdateReq_Req.ptr = (UInt8*)ptr;
	req.req_rep_u.CAPI2_SimApi_SubmitLinearEFileUpdateReq_Req.length = length;
	req.req_rep_u.CAPI2_SimApi_SubmitLinearEFileUpdateReq_Req.path_len = path_len;
	req.req_rep_u.CAPI2_SimApi_SubmitLinearEFileUpdateReq_Req.select_path = (UInt16*)select_path;
	req.respId = MSG_SIM_EFILE_UPDATE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_LINEAR_EFILE_UPDATE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SendLinearEFileUpdateReq(ClientInfo_t* inClientInfoPtr, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 rec_no, const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SendLinearEFileUpdateReq_Req.efile_id = efile_id;
	req.req_rep_u.CAPI2_SimApi_SendLinearEFileUpdateReq_Req.dfile_id = dfile_id;
	req.req_rep_u.CAPI2_SimApi_SendLinearEFileUpdateReq_Req.rec_no = rec_no;
	req.req_rep_u.CAPI2_SimApi_SendLinearEFileUpdateReq_Req.ptr = (UInt8*)ptr;
	req.req_rep_u.CAPI2_SimApi_SendLinearEFileUpdateReq_Req.length = length;
	req.req_rep_u.CAPI2_SimApi_SendLinearEFileUpdateReq_Req.path_len = path_len;
	req.req_rep_u.CAPI2_SimApi_SendLinearEFileUpdateReq_Req.select_path = (UInt16*)select_path;
	req.respId = MSG_SIM_EFILE_UPDATE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_SEND_LINEAR_EFILE_UPDATE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SubmitSeekRecordReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SubmitSeekRecordReq_Req.socket_id = socket_id;
	req.req_rep_u.CAPI2_SimApi_SubmitSeekRecordReq_Req.efile_id = efile_id;
	req.req_rep_u.CAPI2_SimApi_SubmitSeekRecordReq_Req.dfile_id = dfile_id;
	req.req_rep_u.CAPI2_SimApi_SubmitSeekRecordReq_Req.ptr = (UInt8*)ptr;
	req.req_rep_u.CAPI2_SimApi_SubmitSeekRecordReq_Req.length = length;
	req.req_rep_u.CAPI2_SimApi_SubmitSeekRecordReq_Req.path_len = path_len;
	req.req_rep_u.CAPI2_SimApi_SubmitSeekRecordReq_Req.select_path = (UInt16*)select_path;
	req.respId = MSG_SIM_SEEK_REC_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_SEEK_RECORD_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SendSeekRecordReq(ClientInfo_t* inClientInfoPtr, APDUFileID_t efile_id, APDUFileID_t dfile_id, const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SendSeekRecordReq_Req.efile_id = efile_id;
	req.req_rep_u.CAPI2_SimApi_SendSeekRecordReq_Req.dfile_id = dfile_id;
	req.req_rep_u.CAPI2_SimApi_SendSeekRecordReq_Req.ptr = (UInt8*)ptr;
	req.req_rep_u.CAPI2_SimApi_SendSeekRecordReq_Req.length = length;
	req.req_rep_u.CAPI2_SimApi_SendSeekRecordReq_Req.path_len = path_len;
	req.req_rep_u.CAPI2_SimApi_SendSeekRecordReq_Req.select_path = (UInt16*)select_path;
	req.respId = MSG_SIM_SEEK_REC_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_SEND_SEEK_RECORD_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SubmitCyclicEFileUpdateReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SubmitCyclicEFileUpdateReq_Req.socket_id = socket_id;
	req.req_rep_u.CAPI2_SimApi_SubmitCyclicEFileUpdateReq_Req.efile_id = efile_id;
	req.req_rep_u.CAPI2_SimApi_SubmitCyclicEFileUpdateReq_Req.dfile_id = dfile_id;
	req.req_rep_u.CAPI2_SimApi_SubmitCyclicEFileUpdateReq_Req.ptr = (UInt8*)ptr;
	req.req_rep_u.CAPI2_SimApi_SubmitCyclicEFileUpdateReq_Req.length = length;
	req.req_rep_u.CAPI2_SimApi_SubmitCyclicEFileUpdateReq_Req.path_len = path_len;
	req.req_rep_u.CAPI2_SimApi_SubmitCyclicEFileUpdateReq_Req.select_path = (UInt16*)select_path;
	req.respId = MSG_SIM_EFILE_UPDATE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_CYCLIC_EFILE_UPDATE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SendCyclicEFileUpdateReq(ClientInfo_t* inClientInfoPtr, APDUFileID_t efile_id, APDUFileID_t dfile_id, const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SendCyclicEFileUpdateReq_Req.efile_id = efile_id;
	req.req_rep_u.CAPI2_SimApi_SendCyclicEFileUpdateReq_Req.dfile_id = dfile_id;
	req.req_rep_u.CAPI2_SimApi_SendCyclicEFileUpdateReq_Req.ptr = (UInt8*)ptr;
	req.req_rep_u.CAPI2_SimApi_SendCyclicEFileUpdateReq_Req.length = length;
	req.req_rep_u.CAPI2_SimApi_SendCyclicEFileUpdateReq_Req.path_len = path_len;
	req.req_rep_u.CAPI2_SimApi_SendCyclicEFileUpdateReq_Req.select_path = (UInt16*)select_path;
	req.respId = MSG_SIM_EFILE_UPDATE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_SEND_CYCLIC_EFILE_UPDATE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SendRemainingPinAttemptReq(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_PIN_ATTEMPT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_PIN_ATTEMPT_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_IsCachedDataReady(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_CACHE_DATA_READY_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_CACHE_DATA_READY_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_GetServiceCodeStatus(ClientInfo_t* inClientInfoPtr, SERVICE_CODE_STATUS_CPHS_t service_code)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_GetServiceCodeStatus_Req.service_code = service_code;
	req.respId = MSG_SIM_SERVICE_CODE_STATUS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_SERVICE_CODE_STATUS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_CheckCphsService(ClientInfo_t* inClientInfoPtr, CPHS_SST_ENTRY_t sst_entry)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_CheckCphsService_Req.sst_entry = sst_entry;
	req.respId = MSG_SIM_CHECK_CPHS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_CHECK_CPHS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_GetCphsPhase(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_CPHS_PHASE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_CPHS_PHASE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_GetSmsSca(ClientInfo_t* inClientInfoPtr, UInt8 rec_no)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_GetSmsSca_Req.rec_no = rec_no;
	req.respId = MSG_SIM_SMS_SCA_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_SMS_SCA_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_GetIccid(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_ICCID_PARAM_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_ICCID_PARAM_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_IsALSEnabled(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_ALS_STATUS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_ALS_STATUS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_GetAlsDefaultLine(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_ALS_DEFAULT_LINE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_ALS_DEFAULT_LINE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SetAlsDefaultLine(ClientInfo_t* inClientInfoPtr, UInt8 line)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SetAlsDefaultLine_Req.line = line;
	req.respId = MSG_SIM_SET_ALS_DEFAULT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_SET_ALS_DEFAULT_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_GetCallForwardUnconditionalFlag(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_CALLFWD_COND_FLAG_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_CALLFWD_COND_FLAG_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_GetApplicationType(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_APP_TYPE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_APP_TYPE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SendWritePuctReq(ClientInfo_t* inClientInfoPtr, CurrencyName_t currency, EPPU_t *eppu)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	memcpy(&req.req_rep_u.CAPI2_SimApi_SendWritePuctReq_Req.currency , currency, sizeof(  CurrencyName_t ));
	req.req_rep_u.CAPI2_SimApi_SendWritePuctReq_Req.eppu = eppu;
	req.respId = MSG_SIM_PUCT_UPDATE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_PUCT_UPDATE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SubmitRestrictedAccessReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id, APDUCmd_t command, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 p1, UInt8 p2, UInt8 p3, UInt8 path_len, const UInt16 *select_path, const UInt8 *data)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SubmitRestrictedAccessReq_Req.socket_id = socket_id;
	req.req_rep_u.CAPI2_SimApi_SubmitRestrictedAccessReq_Req.command = command;
	req.req_rep_u.CAPI2_SimApi_SubmitRestrictedAccessReq_Req.efile_id = efile_id;
	req.req_rep_u.CAPI2_SimApi_SubmitRestrictedAccessReq_Req.dfile_id = dfile_id;
	req.req_rep_u.CAPI2_SimApi_SubmitRestrictedAccessReq_Req.p1 = p1;
	req.req_rep_u.CAPI2_SimApi_SubmitRestrictedAccessReq_Req.p2 = p2;
	req.req_rep_u.CAPI2_SimApi_SubmitRestrictedAccessReq_Req.p3 = p3;
	req.req_rep_u.CAPI2_SimApi_SubmitRestrictedAccessReq_Req.path_len = path_len;
	req.req_rep_u.CAPI2_SimApi_SubmitRestrictedAccessReq_Req.select_path = (UInt16*)select_path;
	req.req_rep_u.CAPI2_SimApi_SubmitRestrictedAccessReq_Req.data = (UInt8*)data;
	req.respId = MSG_SIM_RESTRICTED_ACCESS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_RESTRICTED_ACCESS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_MS_GetSystemRAT(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GET_SYSTEM_RAT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GET_SYSTEM_RAT_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_MS_GetSupportedRAT(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GET_SUPPORTED_RAT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GET_SUPPORTED_RAT_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_MS_GetSystemBand(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GET_SYSTEM_BAND_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GET_SYSTEM_BAND_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_MS_GetSupportedBand(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GET_SUPPORTED_BAND_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GET_SUPPORTED_BAND_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SYSPARM_GetMSClass(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYSPARAM_GET_MSCLASS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARAM_GET_MSCLASS_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SYSPARM_GetManufacturerName(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYSPARAM_GET_MNF_NAME_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARAM_GET_MNF_NAME_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SYSPARM_GetModelName(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYSPARAM_GET_MODEL_NAME_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARAM_GET_MODEL_NAME_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SYSPARM_GetSWVersion(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYSPARAM_GET_SW_VERSION_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARAM_GET_SW_VERSION_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SYSPARM_GetEGPRSMSClass(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYSPARAM_GET_EGPRS_CLASS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARAM_GET_EGPRS_CLASS_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SimApi_SendNumOfPLMNEntryReq(ClientInfo_t* inClientInfoPtr, SIM_PLMN_FILE_t plmn_file)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SendNumOfPLMNEntryReq_Req.plmn_file = plmn_file;
	req.respId = MSG_SIM_PLMN_NUM_OF_ENTRY_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_PLMN_NUM_OF_ENTRY_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SendReadPLMNEntryReq(ClientInfo_t* inClientInfoPtr, SIM_PLMN_FILE_t plmn_file, UInt16 start_index, UInt16 end_index)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SendReadPLMNEntryReq_Req.plmn_file = plmn_file;
	req.req_rep_u.CAPI2_SimApi_SendReadPLMNEntryReq_Req.start_index = start_index;
	req.req_rep_u.CAPI2_SimApi_SendReadPLMNEntryReq_Req.end_index = end_index;
	req.respId = MSG_SIM_PLMN_ENTRY_DATA_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_PLMN_ENTRY_DATA_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SendWriteMulPLMNEntryReq(ClientInfo_t* inClientInfoPtr, SIM_PLMN_FILE_t plmn_file, UInt16 first_idx, UInt16 number_of_entry, SIM_MUL_PLMN_ENTRY_t *plmn_entry)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SendWriteMulPLMNEntryReq_Req.plmn_file = plmn_file;
	req.req_rep_u.CAPI2_SimApi_SendWriteMulPLMNEntryReq_Req.first_idx = first_idx;
	req.req_rep_u.CAPI2_SimApi_SendWriteMulPLMNEntryReq_Req.number_of_entry = number_of_entry;
	req.req_rep_u.CAPI2_SimApi_SendWriteMulPLMNEntryReq_Req.plmn_entry = plmn_entry;
	req.respId = MSG_SIM_MUL_PLMN_ENTRY_UPDATE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_MUL_PLMN_ENTRY_UPDATE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SYS_SetRegisteredEventMask(UInt32 tid, UInt8 clientID, const UInt16 *maskList, UInt8 maskLen)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SYS_SetRegisteredEventMask_Req.maskList = (UInt16*)maskList;
	req.req_rep_u.CAPI2_SYS_SetRegisteredEventMask_Req.maskLen = maskLen;
	req.respId = MSG_SYS_SET_REG_EVENT_MASK_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYS_SET_REG_EVENT_MASK_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SYS_SetFilteredEventMask(UInt32 tid, UInt8 clientID, const UInt16 *maskList, UInt8 maskLen, SysFilterEnable_t enableFlag)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SYS_SetFilteredEventMask_Req.maskList = (UInt16*)maskList;
	req.req_rep_u.CAPI2_SYS_SetFilteredEventMask_Req.maskLen = maskLen;
	req.req_rep_u.CAPI2_SYS_SetFilteredEventMask_Req.enableFlag = enableFlag;
	req.respId = MSG_SYS_SET_REG_FILTER_MASK_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYS_SET_REG_FILTER_MASK_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_PhoneCtrlApi_SetRssiThreshold(ClientInfo_t* inClientInfoPtr, UInt8 gsm_rxlev_thresold, UInt8 gsm_rxqual_thresold, UInt8 umts_rscp_thresold, UInt8 umts_ecio_thresold)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PhoneCtrlApi_SetRssiThreshold_Req.gsm_rxlev_thresold = gsm_rxlev_thresold;
	req.req_rep_u.CAPI2_PhoneCtrlApi_SetRssiThreshold_Req.gsm_rxqual_thresold = gsm_rxqual_thresold;
	req.req_rep_u.CAPI2_PhoneCtrlApi_SetRssiThreshold_Req.umts_rscp_thresold = umts_rscp_thresold;
	req.req_rep_u.CAPI2_PhoneCtrlApi_SetRssiThreshold_Req.umts_ecio_thresold = umts_ecio_thresold;
	req.respId = MSG_SYS_SET_RSSI_THRESHOLD_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SYS_SET_RSSI_THRESHOLD_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SYSPARM_GetChanMode(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYSPARAM_GET_CHANNEL_MODE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARAM_GET_CHANNEL_MODE_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SYSPARM_GetClassmark(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYSPARAM_GET_CLASSMARK_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARAM_GET_CLASSMARK_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SYSPARM_GetSysparmIndPartFileVersion(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYSPARAM_GET_IND_FILE_VER_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARAM_GET_IND_FILE_VER_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SYSPARM_SetDARPCfg(UInt32 tid, UInt8 clientID, UInt8 darp_cfg)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SYSPARM_SetDARPCfg_Req.darp_cfg = darp_cfg;
	req.respId = MSG_SYS_SET_DARP_CFG_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYS_SET_DARP_CFG_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SYSPARM_SetEGPRSMSClass(UInt32 tid, UInt8 clientID, UInt16 egprs_class)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SYSPARM_SetEGPRSMSClass_Req.egprs_class = egprs_class;
	req.respId = MSG_SYSPARAM_SET_EGPRS_MSCLASS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARAM_SET_EGPRS_MSCLASS_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SYSPARM_SetGPRSMSClass(UInt32 tid, UInt8 clientID, UInt16 gprs_class)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SYSPARM_SetGPRSMSClass_Req.gprs_class = gprs_class;
	req.respId = MSG_SYSPARAM_SET_GPRS_MSCLASS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARAM_SET_GPRS_MSCLASS_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_NetRegApi_DeleteNetworkName(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_TIMEZONE_DELETE_NW_NAME_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_TIMEZONE_DELETE_NW_NAME_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_TestCmds(UInt32 tid, UInt8 clientID, UInt32 testId, UInt32 param1, UInt32 param2, UInt8* buffer)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_TestCmds_Req.testId = testId;
	req.req_rep_u.CAPI2_TestCmds_Req.param1 = param1;
	req.req_rep_u.CAPI2_TestCmds_Req.param2 = param2;
	req.req_rep_u.CAPI2_TestCmds_Req.buffer = buffer;
	req.respId = MSG_CAPI2_TEST_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_CAPI2_TEST_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SatkApi_SendPlayToneRes(ClientInfo_t* inClientInfoPtr, SATK_ResultCode_t resultCode)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SatkApi_SendPlayToneRes_Req.resultCode = resultCode;
	req.respId = MSG_STK_SEND_PLAYTONE_RES_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_STK_SEND_PLAYTONE_RES_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SATK_SendSetupCallRes(UInt32 tid, UInt8 clientID, SATK_ResultCode_t result1, SATK_ResultCode2_t result2)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SATK_SendSetupCallRes_Req.result1 = result1;
	req.req_rep_u.CAPI2_SATK_SendSetupCallRes_Req.result2 = result2;
	req.respId = MSG_STK_SETUP_CALL_RES_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_STK_SETUP_CALL_RES_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_PbkApi_SetFdnCheck(ClientInfo_t* inClientInfoPtr, Boolean fdn_chk_on)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PbkApi_SetFdnCheck_Req.fdn_chk_on = fdn_chk_on;
	req.respId = MSG_PBK_SET_FDN_CHECK_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PBK_SET_FDN_CHECK_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PbkApi_GetFdnCheck(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_PBK_GET_FDN_CHECK_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PBK_GET_FDN_CHECK_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_SendMemAvailInd(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SMS_SEND_MEM_AVAL_IND_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_SEND_MEM_AVAL_IND_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SMS_ConfigureMEStorage(UInt32 tid, UInt8 clientID, Boolean flag)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SMS_ConfigureMEStorage_Req.flag = flag;
	req.respId = MSG_SMS_CONFIGUREMESTORAGE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_CONFIGUREMESTORAGE_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_MsDbApi_SetElement(ClientInfo_t* inClientInfoPtr, CAPI2_MS_Element_t *inElemData)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_MsDbApi_SetElement_Req.inElemData = inElemData;
	req.respId = MSG_MS_SET_ELEMENT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_MS_SET_ELEMENT_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_MsDbApi_GetElement(ClientInfo_t* inClientInfoPtr, MS_Element_t inElemType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_MsDbApi_GetElement_Req.inElemType = inElemType;
	req.respId = MSG_MS_GET_ELEMENT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_MS_GET_ELEMENT_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_USimApi_IsApplicationSupported(ClientInfo_t* inClientInfoPtr, USIM_APPLICATION_TYPE appli_type)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_USimApi_IsApplicationSupported_Req.appli_type = appli_type;
	req.respId = MSG_USIM_IS_APP_SUPPORTED_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_USIM_IS_APP_SUPPORTED_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_USimApi_IsAllowedAPN(ClientInfo_t* inClientInfoPtr, const char* apn_name)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_USimApi_IsAllowedAPN_Req.apn_name = (char_ptr_t)apn_name;
	req.respId = MSG_USIM_IS_APN_ALLOWED_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_USIM_IS_APN_ALLOWED_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_USimApi_GetNumOfAPN(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_USIM_GET_NUM_APN_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_USIM_GET_NUM_APN_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_USimApi_GetAPNEntry(ClientInfo_t* inClientInfoPtr, UInt8 index)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_USimApi_GetAPNEntry_Req.index = index;
	req.respId = MSG_USIM_GET_APN_ENTRY_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_USIM_GET_APN_ENTRY_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_USimApi_IsEstServActivated(ClientInfo_t* inClientInfoPtr, USIM_EST_SERVICE_t est_serv)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_USimApi_IsEstServActivated_Req.est_serv = est_serv;
	req.respId = MSG_USIM_IS_EST_SERV_ACTIVATED_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_USIM_IS_EST_SERV_ACTIVATED_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_USimApi_SendSetEstServReq(ClientInfo_t* inClientInfoPtr, USIM_EST_SERVICE_t est_serv, Boolean serv_on)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_USimApi_SendSetEstServReq_Req.est_serv = est_serv;
	req.req_rep_u.CAPI2_USimApi_SendSetEstServReq_Req.serv_on = serv_on;
	req.respId = MSG_SIM_SET_EST_SERV_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_SET_EST_SERV_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_USimApi_SendWriteAPNReq(ClientInfo_t* inClientInfoPtr, UInt8 index, const char* apn_name)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_USimApi_SendWriteAPNReq_Req.index = index;
	req.req_rep_u.CAPI2_USimApi_SendWriteAPNReq_Req.apn_name = (char_ptr_t)apn_name;
	req.respId = MSG_SIM_UPDATE_ONE_APN_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_UPDATE_ONE_APN_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_USimApi_SendDeleteAllAPNReq(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_DELETE_ALL_APN_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_DELETE_ALL_APN_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_USimApi_GetRatModeSetting(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_USIM_GET_RAT_MODE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_USIM_GET_RAT_MODE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_MS_GetGPRSRegState(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GET_GPRS_STATE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GET_GPRS_STATE_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_MS_GetGSMRegState(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GET_GSM_STATE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GET_GSM_STATE_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_MS_GetRegisteredCellInfo(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GET_CELL_INFO_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GET_CELL_INFO_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_MS_SetMEPowerClass(UInt32 tid, UInt8 clientID, UInt8 band, UInt8 pwrClass)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_MS_SetMEPowerClass_Req.band = band;
	req.req_rep_u.CAPI2_MS_SetMEPowerClass_Req.pwrClass = pwrClass;
	req.respId = MSG_MS_SETMEPOWER_CLASS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_SETMEPOWER_CLASS_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_USimApi_GetServiceStatus(ClientInfo_t* inClientInfoPtr, SIMService_t service)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_USimApi_GetServiceStatus_Req.service = service;
	req.respId = MSG_USIM_GET_SERVICE_STATUS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_USIM_GET_SERVICE_STATUS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_IsAllowedAPN(ClientInfo_t* inClientInfoPtr, const char* apn_name)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_IsAllowedAPN_Req.apn_name = (char_ptr_t)apn_name;
	req.respId = MSG_SIM_IS_ALLOWED_APN_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_IS_ALLOWED_APN_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_GetSmsMaxCapacity(ClientInfo_t* inClientInfoPtr, SmsStorage_t storageType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_GetSmsMaxCapacity_Req.storageType = storageType;
	req.respId = MSG_SMS_GETSMSMAXCAPACITY_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_GETSMSMAXCAPACITY_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_RetrieveMaxCBChnlLength(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SMS_RETRIEVEMAXCBCHNLLENGTH_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_RETRIEVEMAXCBCHNLLENGTH_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_IsBdnOperationRestricted(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_IS_BDN_RESTRICTED_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_IS_BDN_RESTRICTED_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SendPreferredPlmnUpdateInd(ClientInfo_t* inClientInfoPtr, SIM_PLMN_FILE_t prefer_plmn_file)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SendPreferredPlmnUpdateInd_Req.prefer_plmn_file = prefer_plmn_file;
	req.respId = MSG_SIM_SEND_PLMN_UPDATE_IND_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_SEND_PLMN_UPDATE_IND_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SIMIO_DeactiveCard(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIMIO_DEACTIVATE_CARD_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIMIO_DEACTIVATE_CARD_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SimApi_SendSetBdnReq(ClientInfo_t* inClientInfoPtr, SIMBdnOperState_t oper_state, CHVString_t chv2)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SendSetBdnReq_Req.oper_state = oper_state;
	memcpy(&req.req_rep_u.CAPI2_SimApi_SendSetBdnReq_Req.chv2 , chv2, sizeof(  CHVString_t ));
	req.respId = MSG_SIM_SET_BDN_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_SET_BDN_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_PowerOnOffCard(ClientInfo_t* inClientInfoPtr, Boolean power_on, SIM_POWER_ON_MODE_t mode)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_PowerOnOffCard_Req.power_on = power_on;
	req.req_rep_u.CAPI2_SimApi_PowerOnOffCard_Req.mode = mode;
	req.respId = MSG_SIM_POWER_ON_OFF_CARD_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_POWER_ON_OFF_CARD_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_GetRawAtr(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_GET_RAW_ATR_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_GET_RAW_ATR_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_Set_Protocol(ClientInfo_t* inClientInfoPtr, UInt8 protocol)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_Set_Protocol_Req.protocol = protocol;
	req.respId = MSG_SIM_SET_PROTOCOL_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_SET_PROTOCOL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_Get_Protocol(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_GET_PROTOCOL_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_GET_PROTOCOL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SendGenericApduCmd(ClientInfo_t* inClientInfoPtr, UInt8 *apdu, UInt16 len)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SendGenericApduCmd_Req.apdu = apdu;
	req.req_rep_u.CAPI2_SimApi_SendGenericApduCmd_Req.len = len;
	req.respId = MSG_SIM_SEND_GENERIC_APDU_CMD_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_SEND_GENERIC_APDU_CMD_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_TerminateXferApdu(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_TERMINATE_XFER_APDU_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_TERMINATE_XFER_APDU_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SIM_GetSimInterface(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_GET_SIM_INTERFACE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_GET_SIM_INTERFACE_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_NetRegApi_SetPlmnSelectRat(ClientInfo_t* inClientInfoPtr, UInt8 manual_rat)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_NetRegApi_SetPlmnSelectRat_Req.manual_rat = manual_rat;
	req.respId = MSG_SET_PLMN_SELECT_RAT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SET_PLMN_SELECT_RAT_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_MS_IsDeRegisterInProgress(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_IS_DEREGISTER_IN_PROGRESS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_IS_DEREGISTER_IN_PROGRESS_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_MS_IsRegisterInProgress(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_IS_REGISTER_IN_PROGRESS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_IS_REGISTER_IN_PROGRESS_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_PbkApi_SendUsimHdkReadReq(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_READ_USIM_PBK_HDK_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_READ_USIM_PBK_HDK_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PbkApi_SendUsimHdkUpdateReq(ClientInfo_t* inClientInfoPtr, const HDKString_t *hidden_key)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PbkApi_SendUsimHdkUpdateReq_Req.hidden_key = (HDKString_t*)hidden_key;
	req.respId = MSG_WRITE_USIM_PBK_HDK_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_WRITE_USIM_PBK_HDK_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PbkApi_SendUsimAasReadReq(ClientInfo_t* inClientInfoPtr, UInt16 index, PBK_Id_t pbk_id)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PbkApi_SendUsimAasReadReq_Req.index = index;
	req.req_rep_u.CAPI2_PbkApi_SendUsimAasReadReq_Req.pbk_id = pbk_id;
	req.respId = MSG_READ_USIM_PBK_ALPHA_AAS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_READ_USIM_PBK_ALPHA_AAS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PbkApi_SendUsimAasUpdateReq(ClientInfo_t* inClientInfoPtr, UInt16 index, PBK_Id_t pbk_id, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, const UInt8* alpha)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PbkApi_SendUsimAasUpdateReq_Req.index = index;
	req.req_rep_u.CAPI2_PbkApi_SendUsimAasUpdateReq_Req.pbk_id = pbk_id;
	req.req_rep_u.CAPI2_PbkApi_SendUsimAasUpdateReq_Req.alpha_coding = alpha_coding;
	req.req_rep_u.CAPI2_PbkApi_SendUsimAasUpdateReq_Req.alpha_size = alpha_size;
	req.req_rep_u.CAPI2_PbkApi_SendUsimAasUpdateReq_Req.alpha = (uchar_ptr_t)alpha;
	req.respId = MSG_WRITE_USIM_PBK_ALPHA_AAS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_WRITE_USIM_PBK_ALPHA_AAS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PbkApi_SendUsimGasReadReq(ClientInfo_t* inClientInfoPtr, UInt16 index, PBK_Id_t pbk_id)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PbkApi_SendUsimGasReadReq_Req.index = index;
	req.req_rep_u.CAPI2_PbkApi_SendUsimGasReadReq_Req.pbk_id = pbk_id;
	req.respId = MSG_READ_USIM_PBK_ALPHA_GAS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_READ_USIM_PBK_ALPHA_GAS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PbkApi_SendUsimGasUpdateReq(ClientInfo_t* inClientInfoPtr, UInt16 index, PBK_Id_t pbk_id, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, const UInt8* alpha)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PbkApi_SendUsimGasUpdateReq_Req.index = index;
	req.req_rep_u.CAPI2_PbkApi_SendUsimGasUpdateReq_Req.pbk_id = pbk_id;
	req.req_rep_u.CAPI2_PbkApi_SendUsimGasUpdateReq_Req.alpha_coding = alpha_coding;
	req.req_rep_u.CAPI2_PbkApi_SendUsimGasUpdateReq_Req.alpha_size = alpha_size;
	req.req_rep_u.CAPI2_PbkApi_SendUsimGasUpdateReq_Req.alpha = (uchar_ptr_t)alpha;
	req.respId = MSG_WRITE_USIM_PBK_ALPHA_GAS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_WRITE_USIM_PBK_ALPHA_GAS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PbkApi_SendUsimAasInfoReq(ClientInfo_t* inClientInfoPtr, PBK_Id_t pbk_id)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PbkApi_SendUsimAasInfoReq_Req.pbk_id = pbk_id;
	req.respId = MSG_GET_USIM_PBK_ALPHA_INFO_AAS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_GET_USIM_PBK_ALPHA_INFO_AAS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PbkApi_SendUsimGasInfoReq(ClientInfo_t* inClientInfoPtr, PBK_Id_t pbk_id)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PbkApi_SendUsimGasInfoReq_Req.pbk_id = pbk_id;
	req.respId = MSG_GET_USIM_PBK_ALPHA_INFO_GAS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_GET_USIM_PBK_ALPHA_INFO_GAS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_DiagApi_MeasurmentReportReq(ClientInfo_t* inClientInfoPtr, Boolean inPeriodicReport, UInt32 inTimeInterval)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DiagApi_MeasurmentReportReq_Req.inPeriodicReport = inPeriodicReport;
	req.req_rep_u.CAPI2_DiagApi_MeasurmentReportReq_Req.inTimeInterval = inTimeInterval;
	req.respId = MSG_DIAG_MEASURE_REPORT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_DIAG_MEASURE_REPORT_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_MsDbApi_InitCallCfg(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_INITCALLCFG_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_MS_INITCALLCFG_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_MS_InitFaxConfig(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_INITFAXCFG_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_INITFAXCFG_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_MS_InitVideoCallCfg(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_INITVIDEOCALLCFG_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_INITVIDEOCALLCFG_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_MS_InitCallCfgAmpF(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_INITCALLCFGAMPF_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_INITCALLCFGAMPF_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SYSPARM_GetActualLowVoltReading(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYSPARM_GET_ACTUAL_LOW_VOLT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARM_GET_ACTUAL_LOW_VOLT_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SYSPARM_GetActual4p2VoltReading(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYSPARM_GET_ACTUAL_4P2_VOLT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARM_GET_ACTUAL_4P2_VOLT_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SmsApi_SendSMSCommandTxtReq(ClientInfo_t* inClientInfoPtr, SmsCommand_t smsCmd, UInt8* inNum, UInt8* inCmdTxt, Sms_411Addr_t *sca)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_SendSMSCommandTxtReq_Req.smsCmd = smsCmd;
	req.req_rep_u.CAPI2_SmsApi_SendSMSCommandTxtReq_Req.inNum = inNum;
	req.req_rep_u.CAPI2_SmsApi_SendSMSCommandTxtReq_Req.inCmdTxt = inCmdTxt;
	req.req_rep_u.CAPI2_SmsApi_SendSMSCommandTxtReq_Req.sca = sca;
	req.respId = MSG_SMS_SEND_COMMAND_TXT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_SEND_COMMAND_TXT_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_SendSMSCommandPduReq(ClientInfo_t* inClientInfoPtr, UInt8 length, UInt8 *inSmsCmdPdu)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_SendSMSCommandPduReq_Req.length = length;
	req.req_rep_u.CAPI2_SmsApi_SendSMSCommandPduReq_Req.inSmsCmdPdu = inSmsCmdPdu;
	req.respId = MSG_SMS_SEND_COMMAND_PDU_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_SEND_COMMAND_PDU_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_SendPDUAckToNetwork(ClientInfo_t* inClientInfoPtr, SmsMti_t mti, UInt16 rp_cause, UInt8 *tpdu, UInt8 tpdu_len)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_SendPDUAckToNetwork_Req.mti = mti;
	req.req_rep_u.CAPI2_SmsApi_SendPDUAckToNetwork_Req.rp_cause = rp_cause;
	req.req_rep_u.CAPI2_SmsApi_SendPDUAckToNetwork_Req.tpdu = tpdu;
	req.req_rep_u.CAPI2_SmsApi_SendPDUAckToNetwork_Req.tpdu_len = tpdu_len;
	req.respId = MSG_SMS_SEND_ACKTONETWORK_PDU_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_SEND_ACKTONETWORK_PDU_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_StartCellBroadcastWithChnlReq(ClientInfo_t* inClientInfoPtr, char* cbmi, char* lang)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_StartCellBroadcastWithChnlReq_Req.cbmi = cbmi;
	req.req_rep_u.CAPI2_SmsApi_StartCellBroadcastWithChnlReq_Req.lang = lang;
	req.respId = MSG_SMS_START_CB_WITHCHNL_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_START_CB_WITHCHNL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_SetMoSmsTpMr(ClientInfo_t* inClientInfoPtr, UInt8* tpMr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_SetMoSmsTpMr_Req.tpMr = tpMr;
	req.respId = MSG_SMS_SET_TPMR_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_SET_TPMR_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SIMLOCKApi_SetStatus(ClientInfo_t* inClientInfoPtr, const SIMLOCK_STATE_t *simlock_state)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIMLOCKApi_SetStatus_Req.simlock_state = (SIMLOCK_STATE_t*)simlock_state;
	req.respId = MSG_SIMLOCK_SET_STATUS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIMLOCK_SET_STATUS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_DIAG_ApiCellLockReq(UInt32 tid, UInt8 clientID, Boolean cell_lockEnable)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DIAG_ApiCellLockReq_Req.cell_lockEnable = cell_lockEnable;
	req.respId = MSG_DIAG_CELLLOCK_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DIAG_CELLLOCK_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_DIAG_ApiCellLockStatus(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_DIAG_CELLLOCK_STATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DIAG_CELLLOCK_STATUS_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_MS_SetRuaReadyTimer(UInt32 tid, UInt8 clientID, UInt32 inRuaReadyTimer)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_MS_SetRuaReadyTimer_Req.inRuaReadyTimer = inRuaReadyTimer;
	req.respId = MSG_MS_SET_RUA_READY_TIMER_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_SET_RUA_READY_TIMER_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_CcApi_IsThereEmergencyCall(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_CC_IS_THERE_EMERGENCY_CALL_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_IS_THERE_EMERGENCY_CALL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SYS_EnableCellInfoMsg(UInt32 tid, UInt8 clientID, Boolean inEnableCellInfoMsg)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SYS_EnableCellInfoMsg_Req.inEnableCellInfoMsg = inEnableCellInfoMsg;
	req.respId = MSG_SYS_ENABLE_CELL_INFO_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYS_ENABLE_CELL_INFO_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_LCS_L1_bb_isLocked(UInt32 tid, UInt8 clientID, Boolean watch)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_LCS_L1_bb_isLocked_Req.watch = watch;
	req.respId = MSG_L1_BB_ISLOCKED_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_L1_BB_ISLOCKED_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_DIALSTR_ParseGetCallType(UInt32 tid, UInt8 clientID, const UInt8* dialStr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DIALSTR_ParseGetCallType_Req.dialStr = (uchar_ptr_t)dialStr;
	req.respId = MSG_UTIL_DIAL_STR_PARSE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_UTIL_DIAL_STR_PARSE_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_LCS_FttCalcDeltaTime(UInt32 tid, UInt8 clientID, const LcsFttParams_t *inT1, const LcsFttParams_t *inT2)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_LCS_FttCalcDeltaTime_Req.inT1 = (LcsFttParams_t*)inT1;
	req.req_rep_u.CAPI2_LCS_FttCalcDeltaTime_Req.inT2 = (LcsFttParams_t*)inT2;
	req.respId = MSG_LCS_FTT_DELTA_TIME_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_LCS_FTT_DELTA_TIME_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_NetRegApi_ForcedReadyStateReq(ClientInfo_t* inClientInfoPtr, Boolean forcedReadyState)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_NetRegApi_ForcedReadyStateReq_Req.forcedReadyState = forcedReadyState;
	req.respId = MSG_MS_FORCEDREADYSTATE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_MS_FORCEDREADYSTATE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SsApi_ResetSsAlsFlag(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SS_RESETSSALSFLAG_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_RESETSSALSFLAG_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SIMLOCK_GetStatus_RSP(UInt32 tid, UInt8 clientID, SIMLOCK_STATE_t simlock_state)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIMLOCK_GetStatus_RSP_Rsp.simlock_state = simlock_state;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_SIMLOCK_GET_STATUS_RSP, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_DIALSTR_IsValidString(UInt32 tid, UInt8 clientID, const UInt8* str)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DIALSTR_IsValidString_Req.str = (uchar_ptr_t)str;
	req.respId = MSG_DIALSTR_IS_VALID_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DIALSTR_IS_VALID_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_UTIL_Cause2NetworkCause(UInt32 tid, UInt8 clientID, Cause_t inCause)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_UTIL_Cause2NetworkCause_Req.inCause = inCause;
	req.respId = MSG_UTIL_CONVERT_NTWK_CAUSE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_UTIL_CONVERT_NTWK_CAUSE_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_UTIL_ErrCodeToNetCause(UInt32 tid, UInt8 clientID, SS_ErrorCode_t inErrCode)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_UTIL_ErrCodeToNetCause_Req.inErrCode = inErrCode;
	req.respId = MSG_UTIL_CONVERT_ERRCODE_NTWK_CAUSE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_UTIL_CONVERT_ERRCODE_NTWK_CAUSE_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_IsGprsDialStr(UInt32 tid, UInt8 clientID, const UInt8* inDialStr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_IsGprsDialStr_Req.inDialStr = (uchar_ptr_t)inDialStr;
	req.respId = MSG_ISGPRS_DIAL_STR_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_ISGPRS_DIAL_STR_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_UTIL_GetNumOffsetInSsStr(UInt32 tid, UInt8 clientID, const char* ssStr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_UTIL_GetNumOffsetInSsStr_Req.ssStr = (char_ptr_t)ssStr;
	req.respId = MSG_GET_NUM_SS_STR_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_GET_NUM_SS_STR_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_IsPppLoopbackDialStr(UInt32 tid, UInt8 clientID, const UInt8* str)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_IsPppLoopbackDialStr_Req.str = (uchar_ptr_t)str;
	req.respId = MSG_DIALSTR_IS_PPPLOOPBACK_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DIALSTR_IS_PPPLOOPBACK_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SYS_GetRIPPROCVersion(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYS_GETRIPPROCVERSION_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYS_GETRIPPROCVERSION_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SYSPARM_SetHSDPAPHYCategory(UInt32 tid, UInt8 clientID, int hsdpa_phy_cat)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SYSPARM_SetHSDPAPHYCategory_Req.hsdpa_phy_cat = hsdpa_phy_cat;
	req.respId = MSG_SYSPARM_SET_HSDPA_PHY_CAT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARM_SET_HSDPA_PHY_CAT_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SYSPARM_GetHSDPAPHYCategory(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYS_GET_HSDPA_CATEGORY_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYS_GET_HSDPA_CATEGORY_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SmsApi_ConvertSmsMSMsgType(UInt32 tid, UInt8 clientID, UInt8 ms_msg_type)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_ConvertSmsMSMsgType_Req.ms_msg_type = ms_msg_type;
	req.respId = MSG_SMS_CONVERT_MSGTYPE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_CONVERT_MSGTYPE_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_MS_GetPrefNetStatus(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GETPREFNETSTATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GETPREFNETSTATUS_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SYSPARM_SetHSUPAPHYCategory(UInt32 tid, UInt8 clientID, int hsdpa_phy_cat)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SYSPARM_SetHSUPAPHYCategory_Req.hsdpa_phy_cat = hsdpa_phy_cat;
	req.respId = MSG_SYSPARM_SET_HSUPA_PHY_CAT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARM_SET_HSUPA_PHY_CAT_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SYSPARM_GetHSUPAPHYCategory(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYSPARM_GET_HSUPA_PHY_CAT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARM_GET_HSUPA_PHY_CAT_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_InterTaskMsgToCP(UInt32 tid, UInt8 clientID, InterTaskMsg_t *inPtrMsg)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_InterTaskMsgToCP_Req.inPtrMsg = inPtrMsg;
	req.respId = MSG_INTERTASK_MSG_TO_CP_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_INTERTASK_MSG_TO_CP_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_InterTaskMsgToAP_RSP(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_INTERTASK_MSG_TO_AP_RSP, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_CcApi_GetCurrentCallIndex(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_CC_GETCURRENTCALLINDEX_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_GETCURRENTCALLINDEX_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetNextActiveCallIndex(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_CC_GETNEXTACTIVECALLINDEX_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_GETNEXTACTIVECALLINDEX_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetNextHeldCallIndex(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_CC_GETNEXTHELDCALLINDEX_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_GETNEXTHELDCALLINDEX_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetNextWaitCallIndex(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_CC_GETNEXTWAITCALLINDEX_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_GETNEXTWAITCALLINDEX_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetMPTYCallIndex(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_CC_GETMPTYCALLINDEX_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_GETMPTYCALLINDEX_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetCallState(ClientInfo_t* inClientInfoPtr, UInt8 callIndex)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_GetCallState_Req.callIndex = callIndex;
	req.respId = MSG_CC_GETCALLSTATE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_GETCALLSTATE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetCallType(ClientInfo_t* inClientInfoPtr, UInt8 callIndex)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_GetCallType_Req.callIndex = callIndex;
	req.respId = MSG_CC_GETCALLTYPE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_GETCALLTYPE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetLastCallExitCause(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_CC_GETLASTCALLEXITCAUSE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_GETLASTCALLEXITCAUSE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetCallNumber(ClientInfo_t* inClientInfoPtr, UInt8 callIndex)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_GetCallNumber_Req.callIndex = callIndex;
	req.respId = MSG_CC_GETCALLNUMBER_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_GETCALLNUMBER_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetCallingInfo(ClientInfo_t* inClientInfoPtr, UInt8 callIndex)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_GetCallingInfo_Req.callIndex = callIndex;
	req.respId = MSG_CC_GETCALLINGINFO_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_GETCALLINGINFO_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetAllCallStates(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_CC_GETALLCALLSTATES_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_GETALLCALLSTATES_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetAllCallIndex(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_CC_GETALLCALLINDEX_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_GETALLCALLINDEX_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetAllHeldCallIndex(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_CC_GETALLHELDCALLINDEX_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_GETALLHELDCALLINDEX_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetAllActiveCallIndex(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_CC_GETALLACTIVECALLINDEX_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_GETALLACTIVECALLINDEX_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetAllMPTYCallIndex(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_CC_GETALLMPTYCALLINDEX_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_GETALLMPTYCALLINDEX_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetNumOfMPTYCalls(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_CC_GETNUMOFMPTYCALLS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_GETNUMOFMPTYCALLS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetNumofActiveCalls(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_CC_GETNUMOFACTIVECALLS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_GETNUMOFACTIVECALLS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetNumofHeldCalls(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_CC_GETNUMOFHELDCALLS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_GETNUMOFHELDCALLS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_IsThereWaitingCall(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_CC_ISTHEREWAITINGCALL_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_ISTHEREWAITINGCALL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_IsThereAlertingCall(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_CC_ISTHEREALERTINGCALL_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_ISTHEREALERTINGCALL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetConnectedLineID(ClientInfo_t* inClientInfoPtr, UInt8 callIndex)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_GetConnectedLineID_Req.callIndex = callIndex;
	req.respId = MSG_CC_GETCONNECTEDLINEID_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_GETCONNECTEDLINEID_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetCallPresent(ClientInfo_t* inClientInfoPtr, UInt8 callIndex)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_GetCallPresent_Req.callIndex = callIndex;
	req.respId = MSG_CC_GET_CALL_PRESENT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_GET_CALL_PRESENT_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetCallIndexInThisState(ClientInfo_t* inClientInfoPtr, CCallState_t inCcCallState)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_GetCallIndexInThisState_Req.inCcCallState = inCcCallState;
	req.respId = MSG_CC_GET_INDEX_STATE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_GET_INDEX_STATE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_IsMultiPartyCall(ClientInfo_t* inClientInfoPtr, UInt8 callIndex)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_IsMultiPartyCall_Req.callIndex = callIndex;
	req.respId = MSG_CC_ISMULTIPARTYCALL_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_ISMULTIPARTYCALL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_IsThereVoiceCall(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_CC_ISTHEREVOICECALL_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_ISTHEREVOICECALL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_IsConnectedLineIDPresentAllowed(ClientInfo_t* inClientInfoPtr, UInt8 callIndex)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_IsConnectedLineIDPresentAllowed_Req.callIndex = callIndex;
	req.respId = MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetCurrentCallDurationInMilliSeconds(ClientInfo_t* inClientInfoPtr, UInt8 callIndex)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_GetCurrentCallDurationInMilliSeconds_Req.callIndex = callIndex;
	req.respId = MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetLastCallCCM(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_CC_GETLASTCALLCCM_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_GETLASTCALLCCM_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetLastCallDuration(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_CC_GETLASTCALLDURATION_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_GETLASTCALLDURATION_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetLastDataCallRxBytes(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_CC_GETLASTDATACALLRXBYTES_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_GETLASTDATACALLRXBYTES_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetLastDataCallTxBytes(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_CC_GETLASTDATACALLTXBYTES_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_GETLASTDATACALLTXBYTES_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetDataCallIndex(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_CC_GETDATACALLINDEX_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_GETDATACALLINDEX_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetCallClientInfo(UInt32 tid, UInt8 clientID, UInt8 callIndex)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_GetCallClientInfo_Req.callIndex = callIndex;
	req.respId = MSG_CC_GETCALLCLIENT_INFO_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_CC_GETCALLCLIENT_INFO_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_CcApi_GetCallClientID(ClientInfo_t* inClientInfoPtr, UInt8 callIndex)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_GetCallClientID_Req.callIndex = callIndex;
	req.respId = MSG_CC_GETCALLCLIENTID_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_GETCALLCLIENTID_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetTypeAdd(ClientInfo_t* inClientInfoPtr, UInt8 callIndex)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_GetTypeAdd_Req.callIndex = callIndex;
	req.respId = MSG_CC_GETTYPEADD_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_GETTYPEADD_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_SetVoiceCallAutoReject(ClientInfo_t* inClientInfoPtr, Boolean enableAutoRej)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_SetVoiceCallAutoReject_Req.enableAutoRej = enableAutoRej;
	req.respId = MSG_CC_SETVOICECALLAUTOREJECT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_SETVOICECALLAUTOREJECT_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_IsVoiceCallAutoReject(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_CC_ISVOICECALLAUTOREJECT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_ISVOICECALLAUTOREJECT_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_SetTTYCall(ClientInfo_t* inClientInfoPtr, Boolean enable)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_SetTTYCall_Req.enable = enable;
	req.respId = MSG_CC_SETTTYCALL_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_SETTTYCALL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_IsTTYEnable(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_CC_ISTTYENABLE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_ISTTYENABLE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_IsSimOriginedCall(ClientInfo_t* inClientInfoPtr, UInt8 callIndex)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_IsSimOriginedCall_Req.callIndex = callIndex;
	req.respId = MSG_CC_ISSIMORIGINEDCALL_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_ISSIMORIGINEDCALL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_SetVideoCallParam(ClientInfo_t* inClientInfoPtr, VideoCallParam_t param)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_SetVideoCallParam_Req.param = param;
	req.respId = MSG_CC_SETVIDEOCALLPARAM_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_SETVIDEOCALLPARAM_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetVideoCallParam(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_CC_GETVIDEOCALLPARAM_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_GETVIDEOCALLPARAM_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetCCM(ClientInfo_t* inClientInfoPtr, UInt8 callIndex)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_GetCCM_Req.callIndex = callIndex;
	req.respId = MSG_CC_GETCCM_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_GETCCM_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_SendDtmfTone(ClientInfo_t* inClientInfoPtr, ApiDtmf_t *inDtmfObjPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_SendDtmfTone_Req.inDtmfObjPtr = inDtmfObjPtr;
	req.respId = MSG_CCAPI_SENDDTMF_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CCAPI_SENDDTMF_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_StopDtmfTone(ClientInfo_t* inClientInfoPtr, ApiDtmf_t *inDtmfObjPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_StopDtmfTone_Req.inDtmfObjPtr = inDtmfObjPtr;
	req.respId = MSG_CCAPI_STOPDTMF_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CCAPI_STOPDTMF_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_AbortDtmf(UInt32 tid, UInt8 clientID, ApiDtmf_t *inDtmfObjPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_AbortDtmf_Req.inDtmfObjPtr = inDtmfObjPtr;
	req.respId = MSG_CCAPI_ABORTDTMF_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_CCAPI_ABORTDTMF_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_CcApi_SetDtmfToneTimer(ClientInfo_t* inClientInfoPtr, UInt8 inCallIndex, DtmfTimer_t inDtmfTimerType, Ticks_t inDtmfTimeInterval)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_SetDtmfToneTimer_Req.inCallIndex = inCallIndex;
	req.req_rep_u.CAPI2_CcApi_SetDtmfToneTimer_Req.inDtmfTimerType = inDtmfTimerType;
	req.req_rep_u.CAPI2_CcApi_SetDtmfToneTimer_Req.inDtmfTimeInterval = inDtmfTimeInterval;
	req.respId = MSG_CCAPI_SETDTMFTIMER_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CCAPI_SETDTMFTIMER_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_ResetDtmfToneTimer(ClientInfo_t* inClientInfoPtr, UInt8 inCallIndex, DtmfTimer_t inDtmfTimerType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_ResetDtmfToneTimer_Req.inCallIndex = inCallIndex;
	req.req_rep_u.CAPI2_CcApi_ResetDtmfToneTimer_Req.inDtmfTimerType = inDtmfTimerType;
	req.respId = MSG_CCAPI_RESETDTMFTIMER_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CCAPI_RESETDTMFTIMER_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetDtmfToneTimer(ClientInfo_t* inClientInfoPtr, UInt8 inCallIndex, DtmfTimer_t inDtmfTimerType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_GetDtmfToneTimer_Req.inCallIndex = inCallIndex;
	req.req_rep_u.CAPI2_CcApi_GetDtmfToneTimer_Req.inDtmfTimerType = inDtmfTimerType;
	req.respId = MSG_CCAPI_GETDTMFTIMER_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CCAPI_GETDTMFTIMER_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetTiFromCallIndex(ClientInfo_t* inClientInfoPtr, CallIndex_t inCallIndex)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_GetTiFromCallIndex_Req.inCallIndex = inCallIndex;
	req.respId = MSG_CCAPI_GETTIFROMCALLINDEX_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CCAPI_GETTIFROMCALLINDEX_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_IsSupportedBC(ClientInfo_t* inClientInfoPtr, BearerCapability_t *inBcPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_IsSupportedBC_Req.inBcPtr = inBcPtr;
	req.respId = MSG_CCAPI_IS_SUPPORTEDBC_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CCAPI_IS_SUPPORTEDBC_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetBearerCapability(ClientInfo_t* inClientInfoPtr, CallIndex_t inCallIndex)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_GetBearerCapability_Req.inCallIndex = inCallIndex;
	req.respId = MSG_CCAPI_IS_BEARER_CAPABILITY_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CCAPI_IS_BEARER_CAPABILITY_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_SendSMSSrvCenterNumberUpdateReq(ClientInfo_t* inClientInfoPtr, SmsAddress_t *sca, UInt8 rec_no)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_SendSMSSrvCenterNumberUpdateReq_Req.sca = sca;
	req.req_rep_u.CAPI2_SmsApi_SendSMSSrvCenterNumberUpdateReq_Req.rec_no = rec_no;
	req.respId = MSG_SMS_SENDSMSSRVCENTERNUMBERUPDATE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_SENDSMSSRVCENTERNUMBERUPDATE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_GetSMSrvCenterNumber(ClientInfo_t* inClientInfoPtr, UInt8 rec_no)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_GetSMSrvCenterNumber_Req.rec_no = rec_no;
	req.respId = MSG_SMS_GETSMSSRVCENTERNUMBER_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_GETSMSSRVCENTERNUMBER_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_GetSIMSMSCapacityExceededFlag(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SMS_GETSIMSMSCAPACITYEXCEEDEDFLAG_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_GETSIMSMSCAPACITYEXCEEDEDFLAG_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_IsSmsServiceAvail(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SMS_ISSMSSERVICEAVAIL_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_ISSMSSERVICEAVAIL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_GetSmsStoredState(ClientInfo_t* inClientInfoPtr, SmsStorage_t storageType, UInt16 index)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_GetSmsStoredState_Req.storageType = storageType;
	req.req_rep_u.CAPI2_SmsApi_GetSmsStoredState_Req.index = index;
	req.respId = MSG_SMS_GETSMSSTOREDSTATE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_GETSMSSTOREDSTATE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_WriteSMSPduReq(ClientInfo_t* inClientInfoPtr, UInt8 length, UInt8 *inSmsPdu, Sms_411Addr_t *sca, SmsStorage_t storageType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_WriteSMSPduReq_Req.length = length;
	req.req_rep_u.CAPI2_SmsApi_WriteSMSPduReq_Req.inSmsPdu = inSmsPdu;
	req.req_rep_u.CAPI2_SmsApi_WriteSMSPduReq_Req.sca = sca;
	req.req_rep_u.CAPI2_SmsApi_WriteSMSPduReq_Req.storageType = storageType;
	req.respId = MSG_SMS_WRITE_RSP_IND;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_WRITESMSPDU_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_WriteSMSReq(ClientInfo_t* inClientInfoPtr, UInt8* inNum, UInt8* inSMS, SmsTxParam_t *params, UInt8* inSca, SmsStorage_t storageType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_WriteSMSReq_Req.inNum = inNum;
	req.req_rep_u.CAPI2_SmsApi_WriteSMSReq_Req.inSMS = inSMS;
	req.req_rep_u.CAPI2_SmsApi_WriteSMSReq_Req.params = params;
	req.req_rep_u.CAPI2_SmsApi_WriteSMSReq_Req.inSca = inSca;
	req.req_rep_u.CAPI2_SmsApi_WriteSMSReq_Req.storageType = storageType;
	req.respId = MSG_SMS_WRITE_RSP_IND;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_WRITESMSREQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_SendSMSReq(ClientInfo_t* inClientInfoPtr, UInt8* inNum, UInt8* inSMS, SmsTxParam_t *params, UInt8* inSca)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_SendSMSReq_Req.inNum = inNum;
	req.req_rep_u.CAPI2_SmsApi_SendSMSReq_Req.inSMS = inSMS;
	req.req_rep_u.CAPI2_SmsApi_SendSMSReq_Req.params = params;
	req.req_rep_u.CAPI2_SmsApi_SendSMSReq_Req.inSca = inSca;
	req.respId = MSG_SMS_SUBMIT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_SENDSMSREQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_SendSMSPduReq(ClientInfo_t* inClientInfoPtr, UInt8 length, UInt8 *inSmsPdu, Sms_411Addr_t *sca)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_SendSMSPduReq_Req.length = length;
	req.req_rep_u.CAPI2_SmsApi_SendSMSPduReq_Req.inSmsPdu = inSmsPdu;
	req.req_rep_u.CAPI2_SmsApi_SendSMSPduReq_Req.sca = sca;
	req.respId = MSG_SMS_SUBMIT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_SENDSMSPDUREQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_SendStoredSmsReq(ClientInfo_t* inClientInfoPtr, SmsStorage_t storageType, UInt16 index)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_SendStoredSmsReq_Req.storageType = storageType;
	req.req_rep_u.CAPI2_SmsApi_SendStoredSmsReq_Req.index = index;
	req.respId = MSG_SMS_SUBMIT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_SENDSTOREDSMSREQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_WriteSMSPduToSIMReq(ClientInfo_t* inClientInfoPtr, UInt8 length, UInt8 *inSmsPdu, SIMSMSMesgStatus_t smsState)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_WriteSMSPduToSIMReq_Req.length = length;
	req.req_rep_u.CAPI2_SmsApi_WriteSMSPduToSIMReq_Req.inSmsPdu = inSmsPdu;
	req.req_rep_u.CAPI2_SmsApi_WriteSMSPduToSIMReq_Req.smsState = smsState;
	req.respId = MSG_SMS_WRITE_RSP_IND;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_WRITESMSPDUTOSIM_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_GetLastTpMr(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SMS_GETLASTTPMR_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_GETLASTTPMR_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_GetSmsTxParams(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SMS_GETSMSTXPARAMS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_GETSMSTXPARAMS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_GetTxParamInTextMode(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SMS_GETTXPARAMINTEXTMODE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_GETTXPARAMINTEXTMODE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_SetSmsTxParamProcId(ClientInfo_t* inClientInfoPtr, UInt8 pid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_SetSmsTxParamProcId_Req.pid = pid;
	req.respId = MSG_SMS_SETSMSTXPARAMPROCID_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_SETSMSTXPARAMPROCID_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_SetSmsTxParamCodingType(ClientInfo_t* inClientInfoPtr, SmsCodingType_t *codingType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_SetSmsTxParamCodingType_Req.codingType = codingType;
	req.respId = MSG_SMS_SETSMSTXPARAMCODINGTYPE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_SETSMSTXPARAMCODINGTYPE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_SetSmsTxParamValidPeriod(ClientInfo_t* inClientInfoPtr, UInt8 validatePeriod)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_SetSmsTxParamValidPeriod_Req.validatePeriod = validatePeriod;
	req.respId = MSG_SMS_SETSMSTXPARAMVALIDPERIOD_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_SETSMSTXPARAMVALIDPERIOD_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_SetSmsTxParamCompression(ClientInfo_t* inClientInfoPtr, Boolean compression)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_SetSmsTxParamCompression_Req.compression = compression;
	req.respId = MSG_SMS_SETSMSTXPARAMCOMPRESSION_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_SETSMSTXPARAMCOMPRESSION_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_SetSmsTxParamReplyPath(ClientInfo_t* inClientInfoPtr, Boolean replyPath)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_SetSmsTxParamReplyPath_Req.replyPath = replyPath;
	req.respId = MSG_SMS_SETSMSTXPARAMREPLYPATH_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_SETSMSTXPARAMREPLYPATH_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_SetSmsTxParamUserDataHdrInd(ClientInfo_t* inClientInfoPtr, Boolean udhi)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_SetSmsTxParamUserDataHdrInd_Req.udhi = udhi;
	req.respId = MSG_SMS_SETSMSTXPARAMUSERDATAHDRIND_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_SETSMSTXPARAMUSERDATAHDRIND_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_SetSmsTxParamStatusRptReqFlag(ClientInfo_t* inClientInfoPtr, Boolean srr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_SetSmsTxParamStatusRptReqFlag_Req.srr = srr;
	req.respId = MSG_SMS_SETSMSTXPARAMSTATUSRPTREQFLAG_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_SETSMSTXPARAMSTATUSRPTREQFLAG_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_SetSmsTxParamRejDupl(ClientInfo_t* inClientInfoPtr, Boolean rejDupl)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_SetSmsTxParamRejDupl_Req.rejDupl = rejDupl;
	req.respId = MSG_SMS_SETSMSTXPARAMREJDUPL_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_SETSMSTXPARAMREJDUPL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_DeleteSmsMsgByIndexReq(ClientInfo_t* inClientInfoPtr, SmsStorage_t storeType, UInt16 rec_no)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_DeleteSmsMsgByIndexReq_Req.storeType = storeType;
	req.req_rep_u.CAPI2_SmsApi_DeleteSmsMsgByIndexReq_Req.rec_no = rec_no;
	req.respId = MSG_SIM_SMS_STATUS_UPD_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_DELETESMSMSGBYINDEX_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_ReadSmsMsgReq(ClientInfo_t* inClientInfoPtr, SmsStorage_t storeType, UInt16 rec_no)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_ReadSmsMsgReq_Req.storeType = storeType;
	req.req_rep_u.CAPI2_SmsApi_ReadSmsMsgReq_Req.rec_no = rec_no;
	req.respId = MSG_SIM_SMS_DATA_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_READSMSMSG_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_ListSmsMsgReq(ClientInfo_t* inClientInfoPtr, SmsStorage_t storeType, SIMSMSMesgStatus_t msgBox)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_ListSmsMsgReq_Req.storeType = storeType;
	req.req_rep_u.CAPI2_SmsApi_ListSmsMsgReq_Req.msgBox = msgBox;
	req.respId = MSG_SIM_SMS_DATA_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_LISTSMSMSG_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_SetNewMsgDisplayPref(ClientInfo_t* inClientInfoPtr, NewMsgDisplayPref_t type, UInt8 mode)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_SetNewMsgDisplayPref_Req.type = type;
	req.req_rep_u.CAPI2_SmsApi_SetNewMsgDisplayPref_Req.mode = mode;
	req.respId = MSG_SMS_SETNEWMSGDISPLAYPREF_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_SETNEWMSGDISPLAYPREF_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_GetNewMsgDisplayPref(ClientInfo_t* inClientInfoPtr, NewMsgDisplayPref_t type)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_GetNewMsgDisplayPref_Req.type = type;
	req.respId = MSG_SMS_GETNEWMSGDISPLAYPREF_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_GETNEWMSGDISPLAYPREF_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_SetSMSPrefStorage(ClientInfo_t* inClientInfoPtr, SmsStorage_t storageType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_SetSMSPrefStorage_Req.storageType = storageType;
	req.respId = MSG_SMS_SETSMSPREFSTORAGE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_SETSMSPREFSTORAGE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_GetSMSPrefStorage(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SMS_GETSMSPREFSTORAGE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_GETSMSPREFSTORAGE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_GetSMSStorageStatus(ClientInfo_t* inClientInfoPtr, SmsStorage_t storageType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_GetSMSStorageStatus_Req.storageType = storageType;
	req.respId = MSG_SMS_GETSMSSTORAGESTATUS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_GETSMSSTORAGESTATUS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_SaveSmsServiceProfile(ClientInfo_t* inClientInfoPtr, UInt8 profileIndex)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_SaveSmsServiceProfile_Req.profileIndex = profileIndex;
	req.respId = MSG_SMS_SAVESMSSERVICEPROFILE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_SAVESMSSERVICEPROFILE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_RestoreSmsServiceProfile(ClientInfo_t* inClientInfoPtr, UInt8 profileIndex)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_RestoreSmsServiceProfile_Req.profileIndex = profileIndex;
	req.respId = MSG_SMS_RESTORESMSSERVICEPROFILE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_RESTORESMSSERVICEPROFILE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_SetCellBroadcastMsgTypeReq(ClientInfo_t* inClientInfoPtr, UInt8 mode, UInt8* chnlIDs, UInt8* codings)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_SetCellBroadcastMsgTypeReq_Req.mode = mode;
	req.req_rep_u.CAPI2_SmsApi_SetCellBroadcastMsgTypeReq_Req.chnlIDs = chnlIDs;
	req.req_rep_u.CAPI2_SmsApi_SetCellBroadcastMsgTypeReq_Req.codings = codings;
	req.respId = MSG_SMS_CB_START_STOP_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_SETCELLBROADCASTMSGTYPE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_CBAllowAllChnlReq(ClientInfo_t* inClientInfoPtr, Boolean suspend_filtering)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_CBAllowAllChnlReq_Req.suspend_filtering = suspend_filtering;
	req.respId = MSG_SMS_CBALLOWALLCHNLREQ_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_CBALLOWALLCHNLREQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_AddCellBroadcastChnlReq(ClientInfo_t* inClientInfoPtr, UInt8* newCBMI, UInt8 cbmiLen)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_AddCellBroadcastChnlReq_Req.newCBMI = newCBMI;
	req.req_rep_u.CAPI2_SmsApi_AddCellBroadcastChnlReq_Req.cbmiLen = cbmiLen;
	req.respId = MSG_SMS_ADDCELLBROADCASTCHNLREQ_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_ADDCELLBROADCASTCHNLREQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_RemoveCellBroadcastChnlReq(ClientInfo_t* inClientInfoPtr, UInt8* newCBMI)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_RemoveCellBroadcastChnlReq_Req.newCBMI = newCBMI;
	req.respId = MSG_SMS_REMOVECELLBROADCASTCHNLREQ_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_REMOVECELLBROADCASTCHNLREQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_RemoveAllCBChnlFromSearchList(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SMS_REMOVEALLCBCHNLFROMSEARCHLIST_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_REMOVEALLCBCHNLFROMSEARCHLIST_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_GetCBMI(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SMS_GETCBMI_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_GETCBMI_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_GetCbLanguage(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SMS_GETCBLANGUAGE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_GETCBLANGUAGE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_AddCellBroadcastLangReq(ClientInfo_t* inClientInfoPtr, UInt8 newCBLang)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_AddCellBroadcastLangReq_Req.newCBLang = newCBLang;
	req.respId = MSG_SMS_ADDCELLBROADCASTLANGREQ_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_ADDCELLBROADCASTLANGREQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_RemoveCellBroadcastLangReq(ClientInfo_t* inClientInfoPtr, UInt8* cbLang)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_RemoveCellBroadcastLangReq_Req.cbLang = cbLang;
	req.respId = MSG_SMS_REMOVECELLBROADCASTLANGREQ_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_REMOVECELLBROADCASTLANGREQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_StartReceivingCellBroadcastReq(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SMS_CB_START_STOP_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_STARTRECEIVINGCELLBROADCAST_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_StopReceivingCellBroadcastReq(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SMS_CB_START_STOP_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_STOPRECEIVINGCELLBROADCAST_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_SetCBIgnoreDuplFlag(ClientInfo_t* inClientInfoPtr, Boolean ignoreDupl)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_SetCBIgnoreDuplFlag_Req.ignoreDupl = ignoreDupl;
	req.respId = MSG_SMS_SETCBIGNOREDUPLFLAG_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_SETCBIGNOREDUPLFLAG_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_GetCBIgnoreDuplFlag(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SMS_GETCBIGNOREDUPLFLAG_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_GETCBIGNOREDUPLFLAG_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_SetVMIndOnOff(ClientInfo_t* inClientInfoPtr, Boolean on_off)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_SetVMIndOnOff_Req.on_off = on_off;
	req.respId = MSG_SMS_SETVMINDONOFF_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_SETVMINDONOFF_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_IsVMIndEnabled(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SMS_ISVMINDENABLED_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_ISVMINDENABLED_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_GetVMWaitingStatus(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SMS_GETVMWAITINGSTATUS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_GETVMWAITINGSTATUS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_GetNumOfVmscNumber(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SMS_GETNUMOFVMSCNUMBER_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_GETNUMOFVMSCNUMBER_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_GetVmscNumber(ClientInfo_t* inClientInfoPtr, SIM_MWI_TYPE_t vmsc_type)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_GetVmscNumber_Req.vmsc_type = vmsc_type;
	req.respId = MSG_SMS_GETVMSCNUMBER_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_GETVMSCNUMBER_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_UpdateVmscNumberReq(ClientInfo_t* inClientInfoPtr, SIM_MWI_TYPE_t vmsc_type, UInt8* vmscNum, UInt8 numType, UInt8 *alpha, UInt8 alphaCoding, UInt8 alphaLen)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_UpdateVmscNumberReq_Req.vmsc_type = vmsc_type;
	req.req_rep_u.CAPI2_SmsApi_UpdateVmscNumberReq_Req.vmscNum = vmscNum;
	req.req_rep_u.CAPI2_SmsApi_UpdateVmscNumberReq_Req.numType = numType;
	req.req_rep_u.CAPI2_SmsApi_UpdateVmscNumberReq_Req.alpha = alpha;
	req.req_rep_u.CAPI2_SmsApi_UpdateVmscNumberReq_Req.alphaCoding = alphaCoding;
	req.req_rep_u.CAPI2_SmsApi_UpdateVmscNumberReq_Req.alphaLen = alphaLen;
	req.respId = MSG_SMS_UPDATEVMSCNUMBERREQ_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_UPDATEVMSCNUMBERREQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_GetSMSBearerPreference(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SMS_GETSMSBEARERPREFERENCE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_GETSMSBEARERPREFERENCE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_SetSMSBearerPreference(ClientInfo_t* inClientInfoPtr, SMS_BEARER_PREFERENCE_t pref)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_SetSMSBearerPreference_Req.pref = pref;
	req.respId = MSG_SMS_SETSMSBEARERPREFERENCE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_SETSMSBEARERPREFERENCE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_SetSmsReadStatusChangeMode(ClientInfo_t* inClientInfoPtr, Boolean mode)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_SetSmsReadStatusChangeMode_Req.mode = mode;
	req.respId = MSG_SMS_SETSMSREADSTATUSCHANGEMODE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_SETSMSREADSTATUSCHANGEMODE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_GetSmsReadStatusChangeMode(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SMS_GETSMSREADSTATUSCHANGEMODE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_GETSMSREADSTATUSCHANGEMODE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_ChangeSmsStatusReq(ClientInfo_t* inClientInfoPtr, SmsStorage_t storageType, UInt16 index)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_ChangeSmsStatusReq_Req.storageType = storageType;
	req.req_rep_u.CAPI2_SmsApi_ChangeSmsStatusReq_Req.index = index;
	req.respId = MSG_SMS_CHANGESTATUSREQ_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_CHANGESTATUSREQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_SendMEStoredStatusInd(ClientInfo_t* inClientInfoPtr, MEAccess_t meResult, UInt16 slotNumber)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_SendMEStoredStatusInd_Req.meResult = meResult;
	req.req_rep_u.CAPI2_SmsApi_SendMEStoredStatusInd_Req.slotNumber = slotNumber;
	req.respId = MSG_SMS_SENDMESTOREDSTATUSIND_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_SENDMESTOREDSTATUSIND_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_SendMERetrieveSmsDataInd(ClientInfo_t* inClientInfoPtr, MEAccess_t meResult, UInt16 slotNumber, UInt8 *inSms, UInt16 inLen, SIMSMSMesgStatus_t status)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_SendMERetrieveSmsDataInd_Req.meResult = meResult;
	req.req_rep_u.CAPI2_SmsApi_SendMERetrieveSmsDataInd_Req.slotNumber = slotNumber;
	req.req_rep_u.CAPI2_SmsApi_SendMERetrieveSmsDataInd_Req.inSms = inSms;
	req.req_rep_u.CAPI2_SmsApi_SendMERetrieveSmsDataInd_Req.inLen = inLen;
	req.req_rep_u.CAPI2_SmsApi_SendMERetrieveSmsDataInd_Req.status = status;
	req.respId = MSG_SMS_SENDMERETRIEVESMSDATAIND_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_SENDMERETRIEVESMSDATAIND_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_SendMERemovedStatusInd(ClientInfo_t* inClientInfoPtr, MEAccess_t meResult, UInt16 slotNumber)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_SendMERemovedStatusInd_Req.meResult = meResult;
	req.req_rep_u.CAPI2_SmsApi_SendMERemovedStatusInd_Req.slotNumber = slotNumber;
	req.respId = MSG_SMS_SENDMEREMOVEDSTATUSIND_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_SENDMEREMOVEDSTATUSIND_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_SetSmsStoredState(ClientInfo_t* inClientInfoPtr, SmsStorage_t storageType, UInt16 index, SIMSMSMesgStatus_t status)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_SetSmsStoredState_Req.storageType = storageType;
	req.req_rep_u.CAPI2_SmsApi_SetSmsStoredState_Req.index = index;
	req.req_rep_u.CAPI2_SmsApi_SetSmsStoredState_Req.status = status;
	req.respId = MSG_SMS_SETSMSSTOREDSTATE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_SETSMSSTOREDSTATE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_IsCachedDataReady(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SMS_ISCACHEDDATAREADY_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_ISCACHEDDATAREADY_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_GetEnhancedVMInfoIEI(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SMS_GETENHANCEDVMINFOIEI_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_GETENHANCEDVMINFOIEI_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_SetAllNewMsgDisplayPref(ClientInfo_t* inClientInfoPtr, UInt8 *mode, UInt8 *mt, UInt8 *bm, UInt8 *ds, UInt8 *bfr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_SetAllNewMsgDisplayPref_Req.mode = mode;
	req.req_rep_u.CAPI2_SmsApi_SetAllNewMsgDisplayPref_Req.mt = mt;
	req.req_rep_u.CAPI2_SmsApi_SetAllNewMsgDisplayPref_Req.bm = bm;
	req.req_rep_u.CAPI2_SmsApi_SetAllNewMsgDisplayPref_Req.ds = ds;
	req.req_rep_u.CAPI2_SmsApi_SetAllNewMsgDisplayPref_Req.bfr = bfr;
	req.respId = MSG_SMS_SETALLNEWMSGDISPLAYPREF_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_SETALLNEWMSGDISPLAYPREF_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_SendAckToNetwork(ClientInfo_t* inClientInfoPtr, SmsMti_t mti, SmsAckNetworkType_t ackType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SmsApi_SendAckToNetwork_Req.mti = mti;
	req.req_rep_u.CAPI2_SmsApi_SendAckToNetwork_Req.ackType = ackType;
	req.respId = MSG_SMS_ACKTONETWORK_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_ACKTONETWORK_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_StartMultiSmsTransferReq(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SMS_STARTMULTISMSTRANSFER_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_STARTMULTISMSTRANSFER_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SmsApi_StopMultiSmsTransferReq(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SMS_STOPMULTISMSTRANSFER_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SMS_STOPMULTISMSTRANSFER_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SMS_StartCellBroadcastReq(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SMS_START_CELL_BROADCAST_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_START_CELL_BROADCAST_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SMS_SimInit(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SMS_SIMINIT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_SIMINIT_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SMS_SetPDAStorageOverFlowFlag(UInt32 tid, UInt8 clientID, Boolean flag)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SMS_SetPDAStorageOverFlowFlag_Req.flag = flag;
	req.respId = MSG_SMS_PDA_OVERFLOW_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_PDA_OVERFLOW_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_ISimApi_SendAuthenAkaReq(ClientInfo_t* inClientInfoPtr, const UInt8 *rand_data, UInt16 rand_len, const UInt8 *autn_data, UInt16 autn_len)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_ISimApi_SendAuthenAkaReq_Req.rand_data = (UInt8*)rand_data;
	req.req_rep_u.CAPI2_ISimApi_SendAuthenAkaReq_Req.rand_len = rand_len;
	req.req_rep_u.CAPI2_ISimApi_SendAuthenAkaReq_Req.autn_data = (UInt8*)autn_data;
	req.req_rep_u.CAPI2_ISimApi_SendAuthenAkaReq_Req.autn_len = autn_len;
	req.respId = MSG_ISIM_AUTHEN_AKA_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_ISIM_SENDAUTHENAKAREQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_ISimApi_IsIsimSupported(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_ISIM_ISISIMSUPPORTED_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_ISIM_ISISIMSUPPORTED_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_ISimApi_IsIsimActivated(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_ISIM_ISISIMACTIVATED_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_ISIM_ISISIMACTIVATED_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_ISimApi_ActivateIsimAppli(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_ISIM_ACTIVATE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_ISIM_ACTIVATEISIMAPPLI_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_ISimApi_SendAuthenHttpReq(ClientInfo_t* inClientInfoPtr, const UInt8 *realm_data, UInt16 realm_len, const UInt8 *nonce_data, UInt16 nonce_len, const UInt8 *cnonce_data, UInt16 cnonce_len)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_ISimApi_SendAuthenHttpReq_Req.realm_data = (UInt8*)realm_data;
	req.req_rep_u.CAPI2_ISimApi_SendAuthenHttpReq_Req.realm_len = realm_len;
	req.req_rep_u.CAPI2_ISimApi_SendAuthenHttpReq_Req.nonce_data = (UInt8*)nonce_data;
	req.req_rep_u.CAPI2_ISimApi_SendAuthenHttpReq_Req.nonce_len = nonce_len;
	req.req_rep_u.CAPI2_ISimApi_SendAuthenHttpReq_Req.cnonce_data = (UInt8*)cnonce_data;
	req.req_rep_u.CAPI2_ISimApi_SendAuthenHttpReq_Req.cnonce_len = cnonce_len;
	req.respId = MSG_ISIM_AUTHEN_HTTP_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_ISIM_SENDAUTHENHTTPREQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_ISimApi_SendAuthenGbaNafReq(ClientInfo_t* inClientInfoPtr, const UInt8 *naf_id_data, UInt16 naf_id_len)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_ISimApi_SendAuthenGbaNafReq_Req.naf_id_data = (UInt8*)naf_id_data;
	req.req_rep_u.CAPI2_ISimApi_SendAuthenGbaNafReq_Req.naf_id_len = naf_id_len;
	req.respId = MSG_ISIM_AUTHEN_GBA_NAF_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_ISIM_SENDAUTHENGBANAFREQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_ISimApi_SendAuthenGbaBootReq(ClientInfo_t* inClientInfoPtr, const UInt8 *rand_data, UInt16 rand_len, const UInt8 *autn_data, UInt16 autn_len)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_ISimApi_SendAuthenGbaBootReq_Req.rand_data = (UInt8*)rand_data;
	req.req_rep_u.CAPI2_ISimApi_SendAuthenGbaBootReq_Req.rand_len = rand_len;
	req.req_rep_u.CAPI2_ISimApi_SendAuthenGbaBootReq_Req.autn_data = (UInt8*)autn_data;
	req.req_rep_u.CAPI2_ISimApi_SendAuthenGbaBootReq_Req.autn_len = autn_len;
	req.respId = MSG_ISIM_AUTHEN_GBA_BOOT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_ISIM_SENDAUTHENGBABOOTREQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PbkApi_GetAlpha(ClientInfo_t* inClientInfoPtr, char* number)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PbkApi_GetAlpha_Req.number = number;
	req.respId = MSG_PBK_GETALPHA_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PBK_GETALPHA_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PbkApi_IsEmergencyCallNumber(ClientInfo_t* inClientInfoPtr, char* phone_number)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PbkApi_IsEmergencyCallNumber_Req.phone_number = phone_number;
	req.respId = MSG_PBK_ISEMERGENCYCALLNUMBER_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PBK_ISEMERGENCYCALLNUMBER_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PbkApi_IsPartialEmergencyCallNumber(ClientInfo_t* inClientInfoPtr, char* phone_number)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PbkApi_IsPartialEmergencyCallNumber_Req.phone_number = phone_number;
	req.respId = MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PbkApi_SendInfoReq(ClientInfo_t* inClientInfoPtr, PBK_Id_t pbk_id)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PbkApi_SendInfoReq_Req.pbk_id = pbk_id;
	req.respId = MSG_GET_PBK_INFO_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PBK_SENDINFOREQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PbkApi_SendFindAlphaMatchMultipleReq(ClientInfo_t* inClientInfoPtr, PBK_Id_t pbk_id, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PbkApi_SendFindAlphaMatchMultipleReq_Req.pbk_id = pbk_id;
	req.req_rep_u.CAPI2_PbkApi_SendFindAlphaMatchMultipleReq_Req.alpha_coding = alpha_coding;
	req.req_rep_u.CAPI2_PbkApi_SendFindAlphaMatchMultipleReq_Req.alpha_size = alpha_size;
	req.req_rep_u.CAPI2_PbkApi_SendFindAlphaMatchMultipleReq_Req.alpha = alpha;
	req.respId = MSG_PBK_ENTRY_DATA_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SENDFINDALPHAMATCHMULTIPLEREQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PbkApi_SendFindAlphaMatchOneReq(ClientInfo_t* inClientInfoPtr, UInt8 numOfPbk, PBK_Id_t *pbk_id, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PbkApi_SendFindAlphaMatchOneReq_Req.numOfPbk = numOfPbk;
	req.req_rep_u.CAPI2_PbkApi_SendFindAlphaMatchOneReq_Req.pbk_id = pbk_id;
	req.req_rep_u.CAPI2_PbkApi_SendFindAlphaMatchOneReq_Req.alpha_coding = alpha_coding;
	req.req_rep_u.CAPI2_PbkApi_SendFindAlphaMatchOneReq_Req.alpha_size = alpha_size;
	req.req_rep_u.CAPI2_PbkApi_SendFindAlphaMatchOneReq_Req.alpha = alpha;
	req.respId = MSG_PBK_ENTRY_DATA_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SENDFINDALPHAMATCHONEREQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PbkApi_IsReady(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_PBK_ISREADY_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PBK_ISREADY_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PbkApi_SendReadEntryReq(ClientInfo_t* inClientInfoPtr, PBK_Id_t pbk_id, UInt16 start_index, UInt16 end_index)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PbkApi_SendReadEntryReq_Req.pbk_id = pbk_id;
	req.req_rep_u.CAPI2_PbkApi_SendReadEntryReq_Req.start_index = start_index;
	req.req_rep_u.CAPI2_PbkApi_SendReadEntryReq_Req.end_index = end_index;
	req.respId = MSG_PBK_ENTRY_DATA_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SENDREADENTRYREQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PbkApi_SendWriteEntryReq(ClientInfo_t* inClientInfoPtr, PBK_Id_t pbk_id, Boolean special_fax_num, UInt16 index, UInt8 type_of_number, char* number, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PbkApi_SendWriteEntryReq_Req.pbk_id = pbk_id;
	req.req_rep_u.CAPI2_PbkApi_SendWriteEntryReq_Req.special_fax_num = special_fax_num;
	req.req_rep_u.CAPI2_PbkApi_SendWriteEntryReq_Req.index = index;
	req.req_rep_u.CAPI2_PbkApi_SendWriteEntryReq_Req.type_of_number = type_of_number;
	req.req_rep_u.CAPI2_PbkApi_SendWriteEntryReq_Req.number = number;
	req.req_rep_u.CAPI2_PbkApi_SendWriteEntryReq_Req.alpha_coding = alpha_coding;
	req.req_rep_u.CAPI2_PbkApi_SendWriteEntryReq_Req.alpha_size = alpha_size;
	req.req_rep_u.CAPI2_PbkApi_SendWriteEntryReq_Req.alpha = alpha;
	req.respId = MSG_WRT_PBK_ENTRY_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PBK_SENDWRITEENTRYREQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PbkApi_SendUpdateEntryReq(ClientInfo_t* inClientInfoPtr, PBK_Id_t pbk_id, Boolean special_fax_num, UInt16 index, UInt8 type_of_number, char* number, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha, const USIM_PBK_EXT_DATA_t *usim_adn_ext_data)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PbkApi_SendUpdateEntryReq_Req.pbk_id = pbk_id;
	req.req_rep_u.CAPI2_PbkApi_SendUpdateEntryReq_Req.special_fax_num = special_fax_num;
	req.req_rep_u.CAPI2_PbkApi_SendUpdateEntryReq_Req.index = index;
	req.req_rep_u.CAPI2_PbkApi_SendUpdateEntryReq_Req.type_of_number = type_of_number;
	req.req_rep_u.CAPI2_PbkApi_SendUpdateEntryReq_Req.number = number;
	req.req_rep_u.CAPI2_PbkApi_SendUpdateEntryReq_Req.alpha_coding = alpha_coding;
	req.req_rep_u.CAPI2_PbkApi_SendUpdateEntryReq_Req.alpha_size = alpha_size;
	req.req_rep_u.CAPI2_PbkApi_SendUpdateEntryReq_Req.alpha = alpha;
	req.req_rep_u.CAPI2_PbkApi_SendUpdateEntryReq_Req.usim_adn_ext_data = (USIM_PBK_EXT_DATA_t*)usim_adn_ext_data;
	req.respId = MSG_WRT_PBK_ENTRY_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PBK_SENDUPDATEENTRYREQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PbkApi_SendIsNumDiallableReq(ClientInfo_t* inClientInfoPtr, char* number)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PbkApi_SendIsNumDiallableReq_Req.number = number;
	req.respId = MSG_CHK_PBK_DIALLED_NUM_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PBK_SENDISNUMDIALLABLEREQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PbkApi_IsNumDiallable(ClientInfo_t* inClientInfoPtr, const char* number, Boolean is_voice_call)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PbkApi_IsNumDiallable_Req.number = (char_ptr_t)number;
	req.req_rep_u.CAPI2_PbkApi_IsNumDiallable_Req.is_voice_call = is_voice_call;
	req.respId = MSG_PBK_ISNUMDIALLABLE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PBK_ISNUMDIALLABLE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PbkApi_IsNumBarred(ClientInfo_t* inClientInfoPtr, const char* number, Boolean is_voice_call)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PbkApi_IsNumBarred_Req.number = (char_ptr_t)number;
	req.req_rep_u.CAPI2_PbkApi_IsNumBarred_Req.is_voice_call = is_voice_call;
	req.respId = MSG_PBK_ISNUMBARRED_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PBK_ISNUMBARRED_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PbkApi_IsUssdDiallable(ClientInfo_t* inClientInfoPtr, const UInt8 *data, UInt8 dcs, UInt8 dcsLen)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PbkApi_IsUssdDiallable_Req.data = (UInt8*)data;
	req.req_rep_u.CAPI2_PbkApi_IsUssdDiallable_Req.dcs = dcs;
	req.req_rep_u.CAPI2_PbkApi_IsUssdDiallable_Req.dcsLen = dcsLen;
	req.respId = MSG_PBK_ISUSSDDIALLABLE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PBK_ISUSSDDIALLABLE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_SetPDPContext(ClientInfo_t* inClientInfoPtr, UInt8 cid, UInt8 numParms, UInt8* pdpType, UInt8* apn, UInt8* pdpAddr, UInt8 dComp, UInt8 hComp)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_SetPDPContext_Req.cid = cid;
	req.req_rep_u.CAPI2_PdpApi_SetPDPContext_Req.numParms = numParms;
	req.req_rep_u.CAPI2_PdpApi_SetPDPContext_Req.pdpType = pdpType;
	req.req_rep_u.CAPI2_PdpApi_SetPDPContext_Req.apn = apn;
	req.req_rep_u.CAPI2_PdpApi_SetPDPContext_Req.pdpAddr = pdpAddr;
	req.req_rep_u.CAPI2_PdpApi_SetPDPContext_Req.dComp = dComp;
	req.req_rep_u.CAPI2_PdpApi_SetPDPContext_Req.hComp = hComp;
	req.respId = MSG_PDP_SETPDPCONTEXT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_SETPDPCONTEXT_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_SetSecPDPContext(ClientInfo_t* inClientInfoPtr, UInt8 cid, UInt8 numParms, UInt8 priCid, UInt8 dComp, UInt8 hComp)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_SetSecPDPContext_Req.cid = cid;
	req.req_rep_u.CAPI2_PdpApi_SetSecPDPContext_Req.numParms = numParms;
	req.req_rep_u.CAPI2_PdpApi_SetSecPDPContext_Req.priCid = priCid;
	req.req_rep_u.CAPI2_PdpApi_SetSecPDPContext_Req.dComp = dComp;
	req.req_rep_u.CAPI2_PdpApi_SetSecPDPContext_Req.hComp = hComp;
	req.respId = MSG_PDP_SETSECPDPCONTEXT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_SETSECPDPCONTEXT_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_GetGPRSQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_GetGPRSQoS_Req.cid = cid;
	req.respId = MSG_PDP_GETGPRSQOS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_GETGPRSQOS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_SetGPRSQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid, UInt8 NumPara, UInt8 prec, UInt8 delay, UInt8 rel, UInt8 peak, UInt8 mean)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_SetGPRSQoS_Req.cid = cid;
	req.req_rep_u.CAPI2_PdpApi_SetGPRSQoS_Req.NumPara = NumPara;
	req.req_rep_u.CAPI2_PdpApi_SetGPRSQoS_Req.prec = prec;
	req.req_rep_u.CAPI2_PdpApi_SetGPRSQoS_Req.delay = delay;
	req.req_rep_u.CAPI2_PdpApi_SetGPRSQoS_Req.rel = rel;
	req.req_rep_u.CAPI2_PdpApi_SetGPRSQoS_Req.peak = peak;
	req.req_rep_u.CAPI2_PdpApi_SetGPRSQoS_Req.mean = mean;
	req.respId = MSG_PDP_SETGPRSQOS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_SETGPRSQOS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_GetGPRSMinQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_GetGPRSMinQoS_Req.cid = cid;
	req.respId = MSG_PDP_GETGPRSMINQOS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_GETGPRSMINQOS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_SetGPRSMinQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid, UInt8 NumPara, UInt8 prec, UInt8 delay, UInt8 rel, UInt8 peak, UInt8 mean)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_SetGPRSMinQoS_Req.cid = cid;
	req.req_rep_u.CAPI2_PdpApi_SetGPRSMinQoS_Req.NumPara = NumPara;
	req.req_rep_u.CAPI2_PdpApi_SetGPRSMinQoS_Req.prec = prec;
	req.req_rep_u.CAPI2_PdpApi_SetGPRSMinQoS_Req.delay = delay;
	req.req_rep_u.CAPI2_PdpApi_SetGPRSMinQoS_Req.rel = rel;
	req.req_rep_u.CAPI2_PdpApi_SetGPRSMinQoS_Req.peak = peak;
	req.req_rep_u.CAPI2_PdpApi_SetGPRSMinQoS_Req.mean = mean;
	req.respId = MSG_PDP_SETGPRSMINQOS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_SETGPRSMINQOS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_NetRegApi_SendCombinedAttachReq(ClientInfo_t* inClientInfoPtr, Boolean SIMPresent, SIMType_t SIMType, RegType_t regType, PLMN_t plmn)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_NetRegApi_SendCombinedAttachReq_Req.SIMPresent = SIMPresent;
	req.req_rep_u.CAPI2_NetRegApi_SendCombinedAttachReq_Req.SIMType = SIMType;
	req.req_rep_u.CAPI2_NetRegApi_SendCombinedAttachReq_Req.regType = regType;
	req.req_rep_u.CAPI2_NetRegApi_SendCombinedAttachReq_Req.plmn = plmn;
	req.respId = MSG_MS_SENDCOMBINEDATTACHREQ_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_MS_SENDCOMBINEDATTACHREQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_NetRegApi_SendDetachReq(ClientInfo_t* inClientInfoPtr, DeRegCause_t cause, RegType_t regType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_NetRegApi_SendDetachReq_Req.cause = cause;
	req.req_rep_u.CAPI2_NetRegApi_SendDetachReq_Req.regType = regType;
	req.respId = MSG_MS_SENDDETACHREQ_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_MS_SENDDETACHREQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_MS_GetGPRSAttachStatus(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_GETGPRSATTACHSTATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GETGPRSATTACHSTATUS_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_PdpApi_IsSecondaryPdpDefined(ClientInfo_t* inClientInfoPtr, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_IsSecondaryPdpDefined_Req.cid = cid;
	req.respId = MSG_PDP_ISSECONDARYPDPDEFINED_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_ISSECONDARYPDPDEFINED_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PchExApi_SendPDPActivateReq(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHActivateReason_t reason, PCHProtConfig_t *protConfig)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PchExApi_SendPDPActivateReq_Req.cid = cid;
	req.req_rep_u.CAPI2_PchExApi_SendPDPActivateReq_Req.reason = reason;
	req.req_rep_u.CAPI2_PchExApi_SendPDPActivateReq_Req.protConfig = protConfig;
	req.respId = MSG_PDP_ACTIVATION_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PCHEX_SENDPDPACTIVATEREQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PchExApi_SendPDPModifyReq(ClientInfo_t* inClientInfoPtr, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PchExApi_SendPDPModifyReq_Req.cid = cid;
	req.respId = MSG_PDP_MODIFICATION_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PCHEX_SENDPDPMODIFYREQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PchExApi_SendPDPDeactivateReq(ClientInfo_t* inClientInfoPtr, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PchExApi_SendPDPDeactivateReq_Req.cid = cid;
	req.respId = MSG_PDP_DEACTIVATION_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PCHEX_SENDPDPDEACTIVATEREQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PchExApi_SendPDPActivateSecReq(ClientInfo_t* inClientInfoPtr, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PchExApi_SendPDPActivateSecReq_Req.cid = cid;
	req.respId = MSG_PDP_SEC_ACTIVATION_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PCHEX_SENDPDPACTIVATESECREQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_GetGPRSActivateStatus(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_PDP_GETGPRSACTIVATESTATUS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_GETGPRSACTIVATESTATUS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_NetRegApi_SetMSClass(ClientInfo_t* inClientInfoPtr, MSClass_t msClass)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_NetRegApi_SetMSClass_Req.msClass = msClass;
	req.respId = MSG_PDP_SETMSCLASS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_SETMSCLASS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PDP_GetMSClass(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_PDP_GETMSCLASS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_GETMSCLASS_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_PdpApi_GetUMTSTft(ClientInfo_t* inClientInfoPtr, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_GetUMTSTft_Req.cid = cid;
	req.respId = MSG_PDP_GETUMTSTFT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_GETUMTSTFT_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_SetUMTSTft(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHTrafficFlowTemplate_t *pTft_in)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_SetUMTSTft_Req.cid = cid;
	req.req_rep_u.CAPI2_PdpApi_SetUMTSTft_Req.pTft_in = pTft_in;
	req.respId = MSG_PDP_SETUMTSTFT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_SETUMTSTFT_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_DeleteUMTSTft(ClientInfo_t* inClientInfoPtr, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_DeleteUMTSTft_Req.cid = cid;
	req.respId = MSG_PDP_DELETEUMTSTFT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_DELETEUMTSTFT_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_DeactivateSNDCPConnection(ClientInfo_t* inClientInfoPtr, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_DeactivateSNDCPConnection_Req.cid = cid;
	req.respId = MSG_PDP_DEACTIVATESNDCPCONNECTION_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_DEACTIVATESNDCPCONNECTION_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_GetR99UMTSMinQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_GetR99UMTSMinQoS_Req.cid = cid;
	req.respId = MSG_PDP_GETR99UMTSMINQOS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_GETR99UMTSMINQOS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_GetR99UMTSQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_GetR99UMTSQoS_Req.cid = cid;
	req.respId = MSG_PDP_GETR99UMTSQOS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_GETR99UMTSQOS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_GetUMTSMinQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_GetUMTSMinQoS_Req.cid = cid;
	req.respId = MSG_PDP_GETUMTSMINQOS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_GETUMTSMINQOS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_GetUMTSQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_GetUMTSQoS_Req.cid = cid;
	req.respId = MSG_PDP_GETUMTSQOS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_GETUMTSQOS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_GetNegQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_GetNegQoS_Req.cid = cid;
	req.respId = MSG_PDP_GETNEGQOS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_GETNEGQOS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_SetR99UMTSMinQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHR99QosProfile_t *pR99MinQos)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_SetR99UMTSMinQoS_Req.cid = cid;
	req.req_rep_u.CAPI2_PdpApi_SetR99UMTSMinQoS_Req.pR99MinQos = pR99MinQos;
	req.respId = MSG_PDP_SETR99UMTSMINQOS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_SETR99UMTSMINQOS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_SetR99UMTSQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHR99QosProfile_t *pR99Qos)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_SetR99UMTSQoS_Req.cid = cid;
	req.req_rep_u.CAPI2_PdpApi_SetR99UMTSQoS_Req.pR99Qos = pR99Qos;
	req.respId = MSG_PDP_SETR99UMTSQOS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_SETR99UMTSQOS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_SetUMTSMinQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHUMTSQosProfile_t *pMinQos)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_SetUMTSMinQoS_Req.cid = cid;
	req.req_rep_u.CAPI2_PdpApi_SetUMTSMinQoS_Req.pMinQos = pMinQos;
	req.respId = MSG_PDP_SETUMTSMINQOS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_SETUMTSMINQOS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_SetUMTSQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHUMTSQosProfile_t *pQos)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_SetUMTSQoS_Req.cid = cid;
	req.req_rep_u.CAPI2_PdpApi_SetUMTSQoS_Req.pQos = pQos;
	req.respId = MSG_PDP_SETUMTSQOS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_SETUMTSQOS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_GetNegotiatedParms(ClientInfo_t* inClientInfoPtr, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_GetNegotiatedParms_Req.cid = cid;
	req.respId = MSG_PDP_GETNEGOTIATEDPARMS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_GETNEGOTIATEDPARMS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_MS_IsGprsCallActive(UInt32 tid, UInt8 clientID, UInt8 chan)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_MS_IsGprsCallActive_Req.chan = chan;
	req.respId = MSG_MS_ISGPRSCALLACTIVE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_ISGPRSCALLACTIVE_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_MS_SetChanGprsCallActive(UInt32 tid, UInt8 clientID, UInt8 chan, Boolean active)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_MS_SetChanGprsCallActive_Req.chan = chan;
	req.req_rep_u.CAPI2_MS_SetChanGprsCallActive_Req.active = active;
	req.respId = MSG_MS_SETCHANGPRSCALLACTIVE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_SETCHANGPRSCALLACTIVE_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_MS_SetCidForGprsActiveChan(UInt32 tid, UInt8 clientID, UInt8 chan, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_MS_SetCidForGprsActiveChan_Req.chan = chan;
	req.req_rep_u.CAPI2_MS_SetCidForGprsActiveChan_Req.cid = cid;
	req.respId = MSG_MS_SETCIDFORGPRSACTIVECHAN_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_SETCIDFORGPRSACTIVECHAN_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_PdpApi_GetPPPModemCid(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_PDP_GETPPPMODEMCID_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_GETPPPMODEMCID_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_MS_GetGprsActiveChanFromCid(UInt32 tid, UInt8 clientID, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_MS_GetGprsActiveChanFromCid_Req.cid = cid;
	req.respId = MSG_MS_GETGPRSACTIVECHANFROMCID_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GETGPRSACTIVECHANFROMCID_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_MS_GetCidFromGprsActiveChan(UInt32 tid, UInt8 clientID, UInt8 chan)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_MS_GetCidFromGprsActiveChan_Req.chan = chan;
	req.respId = MSG_MS_GETCIDFROMGPRSACTIVECHAN_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GETCIDFROMGPRSACTIVECHAN_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_PdpApi_GetPDPAddress(ClientInfo_t* inClientInfoPtr, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_GetPDPAddress_Req.cid = cid;
	req.respId = MSG_PDP_GETPDPADDRESS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_GETPDPADDRESS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_SendTBFData(ClientInfo_t* inClientInfoPtr, UInt8 cid, UInt32 numberBytes)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_SendTBFData_Req.cid = cid;
	req.req_rep_u.CAPI2_PdpApi_SendTBFData_Req.numberBytes = numberBytes;
	req.respId = MSG_PDP_SENDTBFDATA_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_SENDTBFDATA_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_TftAddFilter(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHPacketFilter_T *pPktFilter)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_TftAddFilter_Req.cid = cid;
	req.req_rep_u.CAPI2_PdpApi_TftAddFilter_Req.pPktFilter = pPktFilter;
	req.respId = MSG_PDP_TFTADDFILTER_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_TFTADDFILTER_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_SetPCHContextState(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHContextState_t contextState)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_SetPCHContextState_Req.cid = cid;
	req.req_rep_u.CAPI2_PdpApi_SetPCHContextState_Req.contextState = contextState;
	req.respId = MSG_PDP_SETPCHCONTEXTSTATE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_SETPCHCONTEXTSTATE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_SetDefaultPDPContext(ClientInfo_t* inClientInfoPtr, UInt8 cid, PDPDefaultContext_t *pDefaultContext)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_SetDefaultPDPContext_Req.cid = cid;
	req.req_rep_u.CAPI2_PdpApi_SetDefaultPDPContext_Req.pDefaultContext = pDefaultContext;
	req.respId = MSG_PDP_SETDEFAULTPDPCONTEXT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_SETDEFAULTPDPCONTEXT_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PchExApi_GetDecodedProtConfig(ClientInfo_t* inClientInfoPtr, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PchExApi_GetDecodedProtConfig_Req.cid = cid;
	req.respId = MSG_PCHEX_READDECODEDPROTCONFIG_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PCHEX_READDECODEDPROTCONFIG_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PchExApi_BuildIpConfigOptions(ClientInfo_t* inClientInfoPtr, char* username, char* password, IPConfigAuthType_t authType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PchExApi_BuildIpConfigOptions_Req.username = username;
	req.req_rep_u.CAPI2_PchExApi_BuildIpConfigOptions_Req.password = password;
	req.req_rep_u.CAPI2_PchExApi_BuildIpConfigOptions_Req.authType = authType;
	req.respId = MSG_PCHEX_BUILDPROTCONFIGOPTIONS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PCHEX_BUILDPROTCONFIGOPTIONS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PchExApi_BuildIpConfigOptions2(ClientInfo_t* inClientInfoPtr, IPConfigAuthType_t authType, CHAP_ChallengeOptions_t *cc, CHAP_ResponseOptions_t *cr, PAP_CnfgOptions_t *po)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PchExApi_BuildIpConfigOptions2_Req.authType = authType;
	req.req_rep_u.CAPI2_PchExApi_BuildIpConfigOptions2_Req.cc = cc;
	req.req_rep_u.CAPI2_PchExApi_BuildIpConfigOptions2_Req.cr = cr;
	req.req_rep_u.CAPI2_PchExApi_BuildIpConfigOptions2_Req.po = po;
	req.respId = MSG_PCHEX_BUILDPROTCONFIGOPTIONS2_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PCHEX_BUILDPROTCONFIGOPTIONS2_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PchExApi_BuildIpConfigOptionsWithChapAuthType(ClientInfo_t* inClientInfoPtr, PCHEx_ChapAuthType_t *params)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PchExApi_BuildIpConfigOptionsWithChapAuthType_Req.params = params;
	req.respId = MSG_PCHEX_BUILDPROTCONFIGOPTION_CHAP_TYPE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PCHEX_BUILDPROTCONFIGOPTION_CHAP_TYPE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_GetDefaultQos(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_PDP_GET_DEFAULT_QOS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_GET_DEFAULT_QOS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_IsPDPContextActive(ClientInfo_t* inClientInfoPtr, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_IsPDPContextActive_Req.cid = cid;
	req.respId = MSG_PDP_ISCONTEXT_ACTIVE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_ISCONTEXT_ACTIVE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_ActivateSNDCPConnection(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHL2P_t l2p)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_ActivateSNDCPConnection_Req.cid = cid;
	memcpy(&req.req_rep_u.CAPI2_PdpApi_ActivateSNDCPConnection_Req.l2p , l2p, sizeof(  PCHL2P_t ));
	req.respId = MSG_PDP_ACTIVATE_SNDCP_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_ACTIVATESNDCPCONNECTION_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_GetPDPContext(ClientInfo_t* inClientInfoPtr, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_GetPDPContext_Req.cid = cid;
	req.respId = MSG_PDP_GETPDPCONTEXT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_GETPDPCONTEXT_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_GetDefinedPDPContextCidList(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_PDP_GETPDPCONTEXT_CID_LIST_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_GETPDPCONTEXT_CID_LIST_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SYS_GetBootLoaderVersion(UInt32 tid, UInt8 clientID, UInt8 BootLoaderVersion_sz)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SYS_GetBootLoaderVersion_Req.BootLoaderVersion_sz = BootLoaderVersion_sz;
	req.respId = MSG_SYSPARAM_BOOTLOADER_VER_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARAM_BOOTLOADER_VER_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SYS_GetDSFVersion(UInt32 tid, UInt8 clientID, UInt8 DSFVersion_sz)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SYS_GetDSFVersion_Req.DSFVersion_sz = DSFVersion_sz;
	req.respId = MSG_SYSPARAM_DSF_VER_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARAM_DSF_VER_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_USimApi_GetUstData(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_USIM_UST_DATA_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_USIM_UST_DATA_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PATCH_GetRevision(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_PATCH_GET_REVISION_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_PATCH_GET_REVISION_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SS_SendCallForwardReq(UInt32 tid, UInt8 clientID, SS_Mode_t mode, SS_CallFwdReason_t reason, SS_SvcCls_t svcCls, UInt8 waitToFwdSec, UInt8* number)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SS_SendCallForwardReq_Req.mode = mode;
	req.req_rep_u.CAPI2_SS_SendCallForwardReq_Req.reason = reason;
	req.req_rep_u.CAPI2_SS_SendCallForwardReq_Req.svcCls = svcCls;
	req.req_rep_u.CAPI2_SS_SendCallForwardReq_Req.waitToFwdSec = waitToFwdSec;
	req.req_rep_u.CAPI2_SS_SendCallForwardReq_Req.number = number;
	req.respId = MSG_SS_CALL_FORWARD_STATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_SENDCALLFORWARDREQ_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SS_QueryCallForwardStatus(UInt32 tid, UInt8 clientID, SS_CallFwdReason_t reason, SS_SvcCls_t svcCls)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SS_QueryCallForwardStatus_Req.reason = reason;
	req.req_rep_u.CAPI2_SS_QueryCallForwardStatus_Req.svcCls = svcCls;
	req.respId = MSG_SS_CALL_FORWARD_STATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_QUERYCALLFORWARDSTATUS_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SS_SendCallBarringReq(UInt32 tid, UInt8 clientID, SS_Mode_t mode, SS_CallBarType_t callBarType, SS_SvcCls_t svcCls, UInt8 *password)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SS_SendCallBarringReq_Req.mode = mode;
	req.req_rep_u.CAPI2_SS_SendCallBarringReq_Req.callBarType = callBarType;
	req.req_rep_u.CAPI2_SS_SendCallBarringReq_Req.svcCls = svcCls;
	req.req_rep_u.CAPI2_SS_SendCallBarringReq_Req.password = password;
	req.respId = MSG_SS_CALL_BARRING_STATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_SENDCALLBARRINGREQ_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SS_QueryCallBarringStatus(UInt32 tid, UInt8 clientID, SS_CallBarType_t callBarType, SS_SvcCls_t svcCls)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SS_QueryCallBarringStatus_Req.callBarType = callBarType;
	req.req_rep_u.CAPI2_SS_QueryCallBarringStatus_Req.svcCls = svcCls;
	req.respId = MSG_SS_CALL_BARRING_STATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_QUERYCALLBARRINGSTATUS_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SS_SendCallBarringPWDChangeReq(UInt32 tid, UInt8 clientID, SS_CallBarType_t callBarType, UInt8 *oldPwd, UInt8 *newPwd, UInt8 *reNewPwd)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SS_SendCallBarringPWDChangeReq_Req.callBarType = callBarType;
	req.req_rep_u.CAPI2_SS_SendCallBarringPWDChangeReq_Req.oldPwd = oldPwd;
	req.req_rep_u.CAPI2_SS_SendCallBarringPWDChangeReq_Req.newPwd = newPwd;
	req.req_rep_u.CAPI2_SS_SendCallBarringPWDChangeReq_Req.reNewPwd = reNewPwd;
	req.respId = MSG_SS_CALL_BARRING_PWD_CHANGE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_SENDCALLBARRINGPWDCHANGEREQ_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SS_SendCallWaitingReq(UInt32 tid, UInt8 clientID, SS_Mode_t mode, SS_SvcCls_t svcCls)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SS_SendCallWaitingReq_Req.mode = mode;
	req.req_rep_u.CAPI2_SS_SendCallWaitingReq_Req.svcCls = svcCls;
	req.respId = MSG_SS_CALL_WAITING_STATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_SENDCALLWAITINGREQ_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SS_QueryCallWaitingStatus(UInt32 tid, UInt8 clientID, SS_SvcCls_t svcCls)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SS_QueryCallWaitingStatus_Req.svcCls = svcCls;
	req.respId = MSG_SS_CALL_WAITING_STATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_QUERYCALLWAITINGSTATUS_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SS_QueryCallingLineIDStatus(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SS_CALLING_LINE_ID_STATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_QUERYCALLINGLINEIDSTATUS_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SS_QueryConnectedLineIDStatus(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SS_CONNECTED_LINE_STATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_QUERYCONNECTEDLINEIDSTATUS_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SS_QueryCallingLineRestrictionStatus(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SS_CALLING_LINE_RESTRICTION_STATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_QUERYCALLINGLINERESTRICTIONSTATUS_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SS_QueryConnectedLineRestrictionStatus(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SS_CONNECTED_LINE_RESTRICTION_STATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_QUERYCONNECTEDLINERESTRICTIONSTATUS_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SS_QueryCallingNAmePresentStatus(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SS_CALLING_NAME_PRESENT_STATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_QUERYCALLINGNAMEPRESENTSTATUS_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SS_SetCallingLineIDStatus(UInt32 tid, UInt8 clientID, Boolean enable)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SS_SetCallingLineIDStatus_Req.enable = enable;
	req.respId = MSG_MS_LOCAL_ELEM_NOTIFY_IND;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_SETCALLINGLINEIDSTATUS_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SS_SetCallingLineRestrictionStatus(UInt32 tid, UInt8 clientID, CLIRMode_t clir_mode)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SS_SetCallingLineRestrictionStatus_Req.clir_mode = clir_mode;
	req.respId = MSG_MS_LOCAL_ELEM_NOTIFY_IND;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_SETCALLINGLINERESTRICTIONSTATUS_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SS_SetConnectedLineIDStatus(UInt32 tid, UInt8 clientID, Boolean enable)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SS_SetConnectedLineIDStatus_Req.enable = enable;
	req.respId = MSG_MS_LOCAL_ELEM_NOTIFY_IND;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_SETCONNECTEDLINEIDSTATUS_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SS_SetConnectedLineRestrictionStatus(UInt32 tid, UInt8 clientID, Boolean enable)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SS_SetConnectedLineRestrictionStatus_Req.enable = enable;
	req.respId = MSG_MS_LOCAL_ELEM_NOTIFY_IND;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_SETCONNECTEDLINERESTRICTIONSTATUS_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SS_SendUSSDConnectReq(UInt32 tid, UInt8 clientID, USSDString_t *ussd)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SS_SendUSSDConnectReq_Req.ussd = ussd;
	req.respId = MSG_USSD_DATA_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_SENDUSSDCONNECTREQ_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SS_SendUSSDData(UInt32 tid, UInt8 clientID, CallIndex_t ussd_id, UInt8 dcs, UInt8 dlen, UInt8 *ussdString)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SS_SendUSSDData_Req.ussd_id = ussd_id;
	req.req_rep_u.CAPI2_SS_SendUSSDData_Req.dcs = dcs;
	req.req_rep_u.CAPI2_SS_SendUSSDData_Req.dlen = dlen;
	req.req_rep_u.CAPI2_SS_SendUSSDData_Req.ussdString = ussdString;
	req.respId = MSG_USSD_DATA_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_SENDUSSDDATA_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SsApi_DialStrSrvReq(ClientInfo_t* inClientInfoPtr, SsApi_DialStrSrvReq_t *inDialStrSrvReqPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SsApi_DialStrSrvReq_Req.inDialStrSrvReqPtr = inDialStrSrvReqPtr;
	req.respId = MSG_SSAPI_DIALSTRSRVREQ_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SSAPI_DIALSTRSRVREQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SS_EndUSSDConnectReq(UInt32 tid, UInt8 clientID, CallIndex_t ussd_id)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SS_EndUSSDConnectReq_Req.ussd_id = ussd_id;
	req.respId = MSG_SS_ENDUSSDCONNECTREQ_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_ENDUSSDCONNECTREQ_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SsApi_SsSrvReq(ClientInfo_t* inClientInfoPtr, SsApi_SrvReq_t *inApiSrvReqPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SsApi_SsSrvReq_Req.inApiSrvReqPtr = inApiSrvReqPtr;
	req.respId = MSG_SSAPI_SSSRVREQ_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SSAPI_SSSRVREQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SsApi_UssdSrvReq(ClientInfo_t* inClientInfoPtr, SsApi_UssdSrvReq_t *inUssdSrvReqPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SsApi_UssdSrvReq_Req.inUssdSrvReqPtr = inUssdSrvReqPtr;
	req.respId = MSG_SSAPI_USSDSRVREQ_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SSAPI_USSDSRVREQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SsApi_UssdDataReq(ClientInfo_t* inClientInfoPtr, SsApi_UssdDataReq_t *inUssdDataReqPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SsApi_UssdDataReq_Req.inUssdDataReqPtr = inUssdDataReqPtr;
	req.respId = MSG_SSAPI_USSDDATAREQ_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SSAPI_USSDDATAREQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SsApi_SsReleaseReq(ClientInfo_t* inClientInfoPtr, SsApi_SrvReq_t *inApiSrvReqPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SsApi_SsReleaseReq_Req.inApiSrvReqPtr = inApiSrvReqPtr;
	req.respId = MSG_SSAPI_SSRELEASEREQ_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SSAPI_SSRELEASEREQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SsApi_DataReq(ClientInfo_t* inClientInfoPtr, SsApi_DataReq_t *inDataReqPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SsApi_DataReq_Req.inDataReqPtr = inDataReqPtr;
	req.respId = MSG_SSAPI_DATAREQ_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SSAPI_DATAREQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SS_SsApiReqDispatcher(UInt32 tid, UInt8 clientID, SS_SsApiReq_t *inSsApiReqPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SS_SsApiReqDispatcher_Req.inSsApiReqPtr = inSsApiReqPtr;
	req.respId = MSG_SSAPI_DISPATCH_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SSAPI_DISPATCH_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SS_GetStr(UInt32 tid, UInt8 clientID, SS_ConstString_t strName)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SS_GetStr_Req.strName = strName;
	req.respId = MSG_SS_GET_STR_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_GET_STR_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SS_SetClientID(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SS_SETCLIENTID_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_SETCLIENTID_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SS_GetClientID(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SS_GETCLIENTID_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_GETCLIENTID_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SS_ResetClientID(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SS_RESETCLIENTID_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_RESETCLIENTID_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SatkApi_GetCachedRootMenuPtr(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SATK_GETCACHEDROOTMENUPTR_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SATK_GETCACHEDROOTMENUPTR_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SatkApi_SendUserActivityEvent(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SATK_SENDUSERACTIVITYEVENT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SATK_SENDUSERACTIVITYEVENT_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SatkApi_SendIdleScreenAvaiEvent(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SATK_SENDIDLESCREENAVAIEVENT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SATK_SENDIDLESCREENAVAIEVENT_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SatkApi_SendLangSelectEvent(ClientInfo_t* inClientInfoPtr, UInt16 language)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SatkApi_SendLangSelectEvent_Req.language = language;
	req.respId = MSG_SATK_SENDLANGSELECTEVENT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SATK_SENDLANGSELECTEVENT_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SatkApi_SendBrowserTermEvent(ClientInfo_t* inClientInfoPtr, Boolean user_term)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SatkApi_SendBrowserTermEvent_Req.user_term = user_term;
	req.respId = MSG_SATK_SENDBROWSERTERMEVENT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SATK_SENDBROWSERTERMEVENT_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SatkApi_CmdResp(ClientInfo_t* inClientInfoPtr, SATK_EVENTS_t toEvent, UInt8 result1, UInt8 result2, SATKString_t *inText, UInt8 menuID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SatkApi_CmdResp_Req.toEvent = toEvent;
	req.req_rep_u.CAPI2_SatkApi_CmdResp_Req.result1 = result1;
	req.req_rep_u.CAPI2_SatkApi_CmdResp_Req.result2 = result2;
	req.req_rep_u.CAPI2_SatkApi_CmdResp_Req.inText = inText;
	req.req_rep_u.CAPI2_SatkApi_CmdResp_Req.menuID = menuID;
	req.respId = MSG_SATK_CMDRESP_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SATK_CMDRESP_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SatkApi_DataServCmdResp(ClientInfo_t* inClientInfoPtr, const StkCmdRespond_t *response)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SatkApi_DataServCmdResp_Req.response = (StkCmdRespond_t*)response;
	req.respId = MSG_SATK_DATASERVCMDRESP_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SATK_DATASERVCMDRESP_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SatkApi_SendDataServReq(ClientInfo_t* inClientInfoPtr, const StkDataServReq_t *dsReq)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SatkApi_SendDataServReq_Req.dsReq = (StkDataServReq_t*)dsReq;
	req.respId = MSG_SATK_SENDDATASERVREQ_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SATK_SENDDATASERVREQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SatkApi_SendTerminalRsp(ClientInfo_t* inClientInfoPtr, SATK_EVENTS_t toEvent, UInt8 result1, UInt16 result2, SATKString_t *inText, UInt8 menuID, UInt8 *ssFacIePtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SatkApi_SendTerminalRsp_Req.toEvent = toEvent;
	req.req_rep_u.CAPI2_SatkApi_SendTerminalRsp_Req.result1 = result1;
	req.req_rep_u.CAPI2_SatkApi_SendTerminalRsp_Req.result2 = result2;
	req.req_rep_u.CAPI2_SatkApi_SendTerminalRsp_Req.inText = inText;
	req.req_rep_u.CAPI2_SatkApi_SendTerminalRsp_Req.menuID = menuID;
	req.req_rep_u.CAPI2_SatkApi_SendTerminalRsp_Req.ssFacIePtr = ssFacIePtr;
	req.respId = MSG_SATK_SENDTERMINALRSP_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SATK_SENDTERMINALRSP_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SatkApi_SetTermProfile(ClientInfo_t* inClientInfoPtr, const UInt8 *data, UInt8 length)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SatkApi_SetTermProfile_Req.data = (UInt8*)data;
	req.req_rep_u.CAPI2_SatkApi_SetTermProfile_Req.length = length;
	req.respId = MSG_SATK_SETTERMPROFILE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SATK_SETTERMPROFILE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SatkApi_SendEnvelopeCmdReq(ClientInfo_t* inClientInfoPtr, const UInt8 *data, UInt8 data_len)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SatkApi_SendEnvelopeCmdReq_Req.data = (UInt8*)data;
	req.req_rep_u.CAPI2_SatkApi_SendEnvelopeCmdReq_Req.data_len = data_len;
	req.respId = MSG_STK_ENVELOPE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SATK_SEND_ENVELOPE_CMD_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SatkApi_SendTerminalRspReq(ClientInfo_t* inClientInfoPtr, const UInt8 *data, UInt8 data_len)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SatkApi_SendTerminalRspReq_Req.data = (UInt8*)data;
	req.req_rep_u.CAPI2_SatkApi_SendTerminalRspReq_Req.data_len = data_len;
	req.respId = MSG_STK_TERMINAL_RESPONSE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_STK_TERMINAL_RESPONSE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_StkApi_SendBrowsingStatusEvent(ClientInfo_t* inClientInfoPtr, UInt8 *status, UInt8 data_len)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_StkApi_SendBrowsingStatusEvent_Req.status = status;
	req.req_rep_u.CAPI2_StkApi_SendBrowsingStatusEvent_Req.data_len = data_len;
	req.respId = MSG_STK_SEND_BROWSING_STATUS_EVT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_STK_SEND_BROWSING_STATUS_EVT_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SatkApi_SendCcSetupReq(ClientInfo_t* inClientInfoPtr, TypeOfNumber_t ton, NumberPlanId_t npi, char *number, BearerCapability_t *bc1, Subaddress_t *subaddr_data, BearerCapability_t *bc2, UInt8 bc_repeat_ind, Boolean simtk_orig)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SatkApi_SendCcSetupReq_Req.ton = ton;
	req.req_rep_u.CAPI2_SatkApi_SendCcSetupReq_Req.npi = npi;
	req.req_rep_u.CAPI2_SatkApi_SendCcSetupReq_Req.number = number;
	req.req_rep_u.CAPI2_SatkApi_SendCcSetupReq_Req.bc1 = bc1;
	req.req_rep_u.CAPI2_SatkApi_SendCcSetupReq_Req.subaddr_data = subaddr_data;
	req.req_rep_u.CAPI2_SatkApi_SendCcSetupReq_Req.bc2 = bc2;
	req.req_rep_u.CAPI2_SatkApi_SendCcSetupReq_Req.bc_repeat_ind = bc_repeat_ind;
	req.req_rep_u.CAPI2_SatkApi_SendCcSetupReq_Req.simtk_orig = simtk_orig;
	req.respId = MSG_STK_CALL_CONTROL_SETUP_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SATK_SEND_CC_SETUP_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SatkApi_SendCcSsReq(ClientInfo_t* inClientInfoPtr, UInt8 ton_npi, UInt8 ss_len, const UInt8 *ss_data)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SatkApi_SendCcSsReq_Req.ton_npi = ton_npi;
	req.req_rep_u.CAPI2_SatkApi_SendCcSsReq_Req.ss_len = ss_len;
	req.req_rep_u.CAPI2_SatkApi_SendCcSsReq_Req.ss_data = (UInt8*)ss_data;
	req.respId = MSG_SATK_SEND_CC_SS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SATK_SEND_CC_SS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SatkApi_SendCcUssdReq(ClientInfo_t* inClientInfoPtr, UInt8 ussd_dcs, UInt8 ussd_len, const UInt8 *ussd_data)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SatkApi_SendCcUssdReq_Req.ussd_dcs = ussd_dcs;
	req.req_rep_u.CAPI2_SatkApi_SendCcUssdReq_Req.ussd_len = ussd_len;
	req.req_rep_u.CAPI2_SatkApi_SendCcUssdReq_Req.ussd_data = (UInt8*)ussd_data;
	req.respId = MSG_SATK_SEND_CC_USSD_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SATK_SEND_CC_USSD_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SatkApi_SendCcSmsReq(ClientInfo_t* inClientInfoPtr, UInt8 sca_toa, UInt8 sca_number_len, UInt8 *sca_number, UInt8 dest_toa, UInt8 dest_number_len, UInt8 *dest_number, Boolean simtk_orig)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SatkApi_SendCcSmsReq_Req.sca_toa = sca_toa;
	req.req_rep_u.CAPI2_SatkApi_SendCcSmsReq_Req.sca_number_len = sca_number_len;
	req.req_rep_u.CAPI2_SatkApi_SendCcSmsReq_Req.sca_number = sca_number;
	req.req_rep_u.CAPI2_SatkApi_SendCcSmsReq_Req.dest_toa = dest_toa;
	req.req_rep_u.CAPI2_SatkApi_SendCcSmsReq_Req.dest_number_len = dest_number_len;
	req.req_rep_u.CAPI2_SatkApi_SendCcSmsReq_Req.dest_number = dest_number;
	req.req_rep_u.CAPI2_SatkApi_SendCcSmsReq_Req.simtk_orig = simtk_orig;
	req.respId = MSG_SATK_SEND_CC_SMS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SATK_SEND_CC_SMS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_LCS_CpMoLrReq(ClientInfo_t* inClientInfoPtr, const LcsCpMoLrReq_t *inCpMoLrReq)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_LCS_CpMoLrReq_Req.inCpMoLrReq = (LcsCpMoLrReq_t*)inCpMoLrReq;
	req.respId = MSG_MNSS_CLIENT_LCS_SRV_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_LCS_CPMOLRREQ_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_LCS_CpMoLrAbort(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_LCS_CPMOLRABORT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_LCS_CPMOLRABORT_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_LCS_CpMtLrVerificationRsp(ClientInfo_t* inClientInfoPtr, LCS_VerifRsp_t inVerificationRsp)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_LCS_CpMtLrVerificationRsp_Req.inVerificationRsp = inVerificationRsp;
	req.respId = MSG_LCS_CPMTLRVERIFICATIONRSP_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_LCS_CPMTLRVERIFICATIONRSP_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_LCS_CpMtLrRsp(ClientInfo_t* inClientInfoPtr, SS_Operation_t inOperation, Boolean inIsAccepted)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_LCS_CpMtLrRsp_Req.inOperation = inOperation;
	req.req_rep_u.CAPI2_LCS_CpMtLrRsp_Req.inIsAccepted = inIsAccepted;
	req.respId = MSG_LCS_CPMTLRRSP_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_LCS_CPMTLRRSP_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_LCS_CpLocUpdateRsp(ClientInfo_t* inClientInfoPtr, const LCS_TermCause_t *inTerminationCause)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_LCS_CpLocUpdateRsp_Req.inTerminationCause = (LCS_TermCause_t*)inTerminationCause;
	req.respId = MSG_LCS_CPLOCUPDATERSP_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_LCS_CPLOCUPDATERSP_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_LCS_DecodePosEstimate(UInt32 tid, UInt8 clientID, const LCS_LocEstimate_t *inLocEstData)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_LCS_DecodePosEstimate_Req.inLocEstData = (LCS_LocEstimate_t*)inLocEstData;
	req.respId = MSG_LCS_DECODEPOSESTIMATE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_LCS_DECODEPOSESTIMATE_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_LCS_EncodeAssistanceReq(UInt32 tid, UInt8 clientID, const LcsAssistanceReq_t *inAssistReq)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_LCS_EncodeAssistanceReq_Req.inAssistReq = (LcsAssistanceReq_t*)inAssistReq;
	req.respId = MSG_LCS_ENCODEASSISTANCEREQ_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_LCS_ENCODEASSISTANCEREQ_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_LCS_FttSyncReq(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_LCS_FTT_SYNC_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_LCS_FTT_SYNC_REQ, inClientInfoPtr, result, &stream, &u32len);
}

//coverity[pass_by_value]
void CAPI2_CcApi_MakeVoiceCall(ClientInfo_t* inClientInfoPtr, UInt8* callNum, VoiceCallParam_t voiceCallParam)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_MakeVoiceCall_Req.callNum = callNum;
	req.req_rep_u.CAPI2_CcApi_MakeVoiceCall_Req.voiceCallParam = voiceCallParam;
	req.respId = MSG_CAPI2_UNDETERMINED_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_MAKEVOICECALL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_MakeDataCall(ClientInfo_t* inClientInfoPtr, UInt8* callNum)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_MakeDataCall_Req.callNum = callNum;
	req.respId = MSG_CAPI2_UNDETERMINED_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_MAKEDATACALL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_MakeFaxCall(ClientInfo_t* inClientInfoPtr, UInt8* callNum)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_MakeFaxCall_Req.callNum = callNum;
	req.respId = MSG_CAPI2_UNDETERMINED_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_MAKEFAXCALL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_MakeVideoCall(ClientInfo_t* inClientInfoPtr, UInt8* callNum)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_MakeVideoCall_Req.callNum = callNum;
	req.respId = MSG_CAPI2_UNDETERMINED_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_MAKEVIDEOCALL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_EndCall(ClientInfo_t* inClientInfoPtr, UInt8 callIdx)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_EndCall_Req.callIdx = callIdx;
	req.respId = MSG_CAPI2_UNDETERMINED_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_ENDCALL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_EndAllCalls(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_CAPI2_UNDETERMINED_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_ENDALLCALLS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_EndCallImmediate(ClientInfo_t* inClientInfoPtr, UInt8 callIdx)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_EndCallImmediate_Req.callIdx = callIdx;
	req.respId = MSG_CAPI2_UNDETERMINED_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CCAPI_ENDCALL_IMMEDIATE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_EndAllCallsImmediate(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_CAPI2_UNDETERMINED_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CCAPI_ENDALLCALLS_IMMEDIATE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_EndMPTYCalls(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_CAPI2_UNDETERMINED_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_ENDMPTYCALLS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_EndHeldCall(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_CAPI2_UNDETERMINED_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_ENDHELDCALL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_AcceptVoiceCall(ClientInfo_t* inClientInfoPtr, UInt8 callIndex)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_AcceptVoiceCall_Req.callIndex = callIndex;
	req.respId = MSG_CAPI2_UNDETERMINED_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_ACCEPTVOICECALL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_AcceptDataCall(ClientInfo_t* inClientInfoPtr, UInt8 callIndex)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_AcceptDataCall_Req.callIndex = callIndex;
	req.respId = MSG_CAPI2_UNDETERMINED_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_ACCEPTDATACALL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_AcceptWaitingCall(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_CAPI2_UNDETERMINED_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_ACCEPTWAITINGCALL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_AcceptVideoCall(ClientInfo_t* inClientInfoPtr, UInt8 callIndex)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_AcceptVideoCall_Req.callIndex = callIndex;
	req.respId = MSG_CAPI2_UNDETERMINED_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_ACCEPTVIDEOCALL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_HoldCurrentCall(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_VOICECALL_ACTION_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_HOLDCURRENTCALL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_HoldCall(ClientInfo_t* inClientInfoPtr, UInt8 callIndex)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_HoldCall_Req.callIndex = callIndex;
	req.respId = MSG_VOICECALL_ACTION_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_HOLDCALL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_RetrieveNextHeldCall(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_VOICECALL_ACTION_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_RETRIEVENEXTHELDCALL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_RetrieveCall(ClientInfo_t* inClientInfoPtr, UInt8 callIndex)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_RetrieveCall_Req.callIndex = callIndex;
	req.respId = MSG_VOICECALL_ACTION_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_RETRIEVECALL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_SwapCall(ClientInfo_t* inClientInfoPtr, UInt8 callIndex)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_SwapCall_Req.callIndex = callIndex;
	req.respId = MSG_VOICECALL_ACTION_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_SWAPCALL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_SplitCall(ClientInfo_t* inClientInfoPtr, UInt8 callIndex)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_SplitCall_Req.callIndex = callIndex;
	req.respId = MSG_VOICECALL_ACTION_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_SPLITCALL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_JoinCall(ClientInfo_t* inClientInfoPtr, UInt8 callIndex)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_JoinCall_Req.callIndex = callIndex;
	req.respId = MSG_VOICECALL_ACTION_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_JOINCALL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_TransferCall(ClientInfo_t* inClientInfoPtr, UInt8 callIndex)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_TransferCall_Req.callIndex = callIndex;
	req.respId = MSG_VOICECALL_ACTION_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_TRANSFERCALL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetCNAPName(ClientInfo_t* inClientInfoPtr, UInt8 callIndex)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_GetCNAPName_Req.callIndex = callIndex;
	req.respId = MSG_CC_GETCNAPNAME_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_GETCNAPNAME_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SYSPARM_GetHSUPASupported(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYSPARM_GET_HSUPA_SUPPORTED_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARM_GET_HSUPA_SUPPORTED_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_SYSPARM_GetHSDPASupported(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SYSPARM_GET_HSDPA_SUPPORTED_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SYSPARM_GET_HSDPA_SUPPORTED_REQ, (UInt8)clientID, result, &stream, &u32len);
}

void CAPI2_NetRegApi_ForcePsReleaseReq(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_FORCE_PS_REL_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_MS_FORCE_PS_REL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_IsCurrentStateMpty(ClientInfo_t* inClientInfoPtr, UInt8 callIndex)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_IsCurrentStateMpty_Req.callIndex = callIndex;
	req.respId = MSG_CC_ISCURRENTSTATEMPTY_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CC_ISCURRENTSTATEMPTY_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_GetPCHContextState(ClientInfo_t* inClientInfoPtr, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_GetPCHContextState_Req.cid = cid;
	req.respId = MSG_PDP_GETPCHCONTEXTSTATE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_GETPCHCONTEXTSTATE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_GetPDPContextEx(ClientInfo_t* inClientInfoPtr, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_GetPDPContextEx_Req.cid = cid;
	req.respId = MSG_PDP_GETPCHCONTEXT_EX_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_GETPCHCONTEXT_EX_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_GetCurrLockedSimlockType(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_PIN_LOCK_TYPE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_PIN_LOCK_TYPE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SubmitSelectFileSendApduReq(ClientInfo_t* inClientInfoPtr, UInt8 socketId, APDUFileID_t dfileId, APDUFileID_t efileId, UInt8 pathLen, const UInt16 *pPath, UInt16 apduLen, const UInt8 *pApdu)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SubmitSelectFileSendApduReq_Req.socketId = socketId;
	req.req_rep_u.CAPI2_SimApi_SubmitSelectFileSendApduReq_Req.dfileId = dfileId;
	req.req_rep_u.CAPI2_SimApi_SubmitSelectFileSendApduReq_Req.efileId = efileId;
	req.req_rep_u.CAPI2_SimApi_SubmitSelectFileSendApduReq_Req.pathLen = pathLen;
	req.req_rep_u.CAPI2_SimApi_SubmitSelectFileSendApduReq_Req.pPath = (UInt16*)pPath;
	req.req_rep_u.CAPI2_SimApi_SubmitSelectFileSendApduReq_Req.apduLen = apduLen;
	req.req_rep_u.CAPI2_SimApi_SubmitSelectFileSendApduReq_Req.pApdu = (UInt8*)pApdu;
	req.respId = MSG_SIM_SEND_APDU_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_SEND_APDU_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SubmitMulRecordEFileReq(ClientInfo_t* inClientInfoPtr, UInt8 socketId, APDUFileID_t efileId, APDUFileID_t dfileId, UInt8 firstRecNum, UInt8 numOfRec, UInt8 recLen, UInt8 pathLen, const UInt16 *pSelectPath, Boolean optimizationFlag1, Boolean optimizationFlag2)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SubmitMulRecordEFileReq_Req.socketId = socketId;
	req.req_rep_u.CAPI2_SimApi_SubmitMulRecordEFileReq_Req.efileId = efileId;
	req.req_rep_u.CAPI2_SimApi_SubmitMulRecordEFileReq_Req.dfileId = dfileId;
	req.req_rep_u.CAPI2_SimApi_SubmitMulRecordEFileReq_Req.firstRecNum = firstRecNum;
	req.req_rep_u.CAPI2_SimApi_SubmitMulRecordEFileReq_Req.numOfRec = numOfRec;
	req.req_rep_u.CAPI2_SimApi_SubmitMulRecordEFileReq_Req.recLen = recLen;
	req.req_rep_u.CAPI2_SimApi_SubmitMulRecordEFileReq_Req.pathLen = pathLen;
	req.req_rep_u.CAPI2_SimApi_SubmitMulRecordEFileReq_Req.pSelectPath = (UInt16*)pSelectPath;
	req.req_rep_u.CAPI2_SimApi_SubmitMulRecordEFileReq_Req.optimizationFlag1 = optimizationFlag1;
	req.req_rep_u.CAPI2_SimApi_SubmitMulRecordEFileReq_Req.optimizationFlag2 = optimizationFlag2;
	req.respId = MSG_SIM_MUL_REC_DATA_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_MUL_REC_DATA_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_SendSelectApplicationReq(ClientInfo_t* inClientInfoPtr, UInt8 socketId, const UInt8 *aid_data, UInt8 aid_len, SIM_APP_OCCURRENCE_t app_occur)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_SendSelectApplicationReq_Req.socketId = socketId;
	req.req_rep_u.CAPI2_SimApi_SendSelectApplicationReq_Req.aid_data = (UInt8*)aid_data;
	req.req_rep_u.CAPI2_SimApi_SendSelectApplicationReq_Req.aid_len = aid_len;
	req.req_rep_u.CAPI2_SimApi_SendSelectApplicationReq_Req.app_occur = app_occur;
	req.respId = MSG_SIM_SELECT_APPLICATION_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_SELECT_APPLICATION_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_PerformSteeringOfRoaming(ClientInfo_t* inClientInfoPtr, SIM_MUL_PLMN_ENTRY_t *inPlmnListPtr, UInt8 inListLen, UInt8 inRefreshType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_PerformSteeringOfRoaming_Req.inPlmnListPtr = inPlmnListPtr;
	req.req_rep_u.CAPI2_SimApi_PerformSteeringOfRoaming_Req.inListLen = inListLen;
	req.req_rep_u.CAPI2_SimApi_PerformSteeringOfRoaming_Req.inRefreshType = inRefreshType;
	req.respId = MSG_SIM_PEROFRM_STEERING_OF_ROAMING_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_PEROFRM_STEERING_OF_ROAMING_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SatkApi_SendProactiveCmdFetchingOnOffReq(ClientInfo_t* inClientInfoPtr, Boolean enableCmdFetching)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SatkApi_SendProactiveCmdFetchingOnOffReq_Req.enableCmdFetching = enableCmdFetching;
	req.respId = MSG_STK_PROACTIVE_CMD_FETCHING_ON_OFF_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_STK_PROACTIVE_CMD_FETCHING_ON_OFF_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SatkApi_SendExtProactiveCmdReq(ClientInfo_t* inClientInfoPtr, UInt8 dataLen, const UInt8 *ptrData)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SatkApi_SendExtProactiveCmdReq_Req.dataLen = dataLen;
	req.req_rep_u.CAPI2_SatkApi_SendExtProactiveCmdReq_Req.ptrData = (UInt8*)ptrData;
	req.respId = MSG_SATK_SEND_EXT_PROACTIVE_CMD_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SATK_SEND_EXT_PROACTIVE_CMD_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SatkApi_SendTerminalProfileReq(ClientInfo_t* inClientInfoPtr, UInt8 dataLen, const UInt8 *ptrData)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SatkApi_SendTerminalProfileReq_Req.dataLen = dataLen;
	req.req_rep_u.CAPI2_SatkApi_SendTerminalProfileReq_Req.ptrData = (UInt8*)ptrData;
	req.respId = MSG_SATK_SEND_TERMINAL_PROFILE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SATK_SEND_TERMINAL_PROFILE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SatkApi_SendPollingIntervalReq(ClientInfo_t* inClientInfoPtr, UInt16 pollingInterval)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SatkApi_SendPollingIntervalReq_Req.pollingInterval = pollingInterval;
	req.respId = MSG_STK_POLLING_INTERVAL_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_STK_POLLING_INTERVAL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_SetPDPActivationCallControlFlag(ClientInfo_t* inClientInfoPtr, Boolean flag)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_SetPDPActivationCallControlFlag_Req.flag = flag;
	req.respId = MSG_PDP_SETPDPACTIVATIONCCFLAG_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_SETPDPACTIVATIONCCFLAG_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_GetPDPActivationCallControlFlag(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_PDP_GETPDPACTIVATIONCCFLAG_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_GETPDPACTIVATIONCCFLAG_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_SendPDPActivateReq_PDU(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHPDPActivateCallControlResult_t callControlResult, PCHPDPActivatePDU_t *pdu)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_SendPDPActivateReq_PDU_Req.cid = cid;
	req.req_rep_u.CAPI2_PdpApi_SendPDPActivateReq_PDU_Req.callControlResult = callControlResult;
	req.req_rep_u.CAPI2_PdpApi_SendPDPActivateReq_PDU_Req.pdu = pdu;
	req.respId = MSG_PDP_ACTIVATION_PDU_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_SENDPDPACTIVATIONPDU_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_RejectNWIPDPActivation(ClientInfo_t* inClientInfoPtr, PCHPDPAddress_t inPdpAddress, PCHRejectCause_t inCause, PCHAPN_t inApn)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	memcpy(&req.req_rep_u.CAPI2_PdpApi_RejectNWIPDPActivation_Req.inPdpAddress , inPdpAddress, sizeof(  PCHPDPAddress_t ));
	req.req_rep_u.CAPI2_PdpApi_RejectNWIPDPActivation_Req.inCause = inCause;
	memcpy(&req.req_rep_u.CAPI2_PdpApi_RejectNWIPDPActivation_Req.inApn , inApn, sizeof(  PCHAPN_t ));
	req.respId = MSG_PDP_REJECTNWIACTIVATION_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_REJECTNWIACTIVATION_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_SetPDPBearerCtrlMode(ClientInfo_t* inClientInfoPtr, UInt8 inMode)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_SetPDPBearerCtrlMode_Req.inMode = inMode;
	req.respId = MSG_PDP_SETBEARERCTRLMODE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_SETBEARERCTRLMODE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_GetPDPBearerCtrlMode(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_PDP_GETBEARERCTRLMODE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_GETBEARERCTRLMODE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_RejectSecNWIPDPActivation(ClientInfo_t* inClientInfoPtr, PDP_ActivateSecNWI_Ind_t *pActivateSecInd, PCHRejectCause_t inCause)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_RejectSecNWIPDPActivation_Req.pActivateSecInd = pActivateSecInd;
	req.req_rep_u.CAPI2_PdpApi_RejectSecNWIPDPActivation_Req.inCause = inCause;
	req.respId = MSG_PDP_REJECTSECNWIACTIVATION_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_REJECTSECNWIACTIVATION_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_SetPDPNWIControlFlag(ClientInfo_t* inClientInfoPtr, Boolean inFlag)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_SetPDPNWIControlFlag_Req.inFlag = inFlag;
	req.respId = MSG_PDP_SETPDPNWICONTROLFLAG_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_SETPDPNWICONTROLFLAG_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_GetPDPNWIControlFlag(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_PDP_GETPDPNWICONTROLFLAG_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_GETPDPNWICONTROLFLAG_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_CheckUMTSTft(ClientInfo_t* inClientInfoPtr, PCHTrafficFlowTemplate_t *inTftPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_CheckUMTSTft_Req.inTftPtr = inTftPtr;
	req.respId = MSG_PDP_CHECKUMTSTFT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_CHECKUMTSTFT_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_IsAnyPDPContextActive(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_PDP_ISANYPDPCONTEXTACTIVE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_ISANYPDPCONTEXTACTIVE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_IsAnyPDPContextActivePending(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_PDP_ISANYPDPCONTEXTPENDING_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_ISANYPDPCONTEXTPENDING_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_SetElement(ClientInfo_t* inClientInfoPtr, CcApi_Element_t inElement, void *inCompareObjPtr, void *outElementPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_SetElement_Req.inElement = inElement;
	req.req_rep_u.CAPI2_CcApi_SetElement_Req.inCompareObjPtr = inCompareObjPtr;
	req.req_rep_u.CAPI2_CcApi_SetElement_Req.outElementPtr = outElementPtr;
	req.respId = MSG_CCAPI_SET_ELEMENT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CCAPI_SET_ELEMENT_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_GetElement(ClientInfo_t* inClientInfoPtr, CcApi_Element_t inElement, void *inCompareObjPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_GetElement_Req.inElement = inElement;
	req.req_rep_u.CAPI2_CcApi_GetElement_Req.inCompareObjPtr = inCompareObjPtr;
	req.respId = MSG_CCAPI_GET_ELEMENT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CCAPI_GET_ELEMENT_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_WL_PsSetFilterList(ClientInfo_t* inClientInfoPtr, UInt8 cid, WL_SocketFilterList_t *inDataPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_WL_PsSetFilterList_Req.cid = cid;
	req.req_rep_u.CAPI2_WL_PsSetFilterList_Req.inDataPtr = inDataPtr;
	req.respId = MSG_WL_PS_SET_FILTER_LIST_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_WL_PS_SET_FILTER_LIST_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_GetProtConfigOptions(ClientInfo_t* inClientInfoPtr, UInt8 inCid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_GetProtConfigOptions_Req.inCid = inCid;
	req.respId = MSG_PDP_GETPROTCONFIGOPTIONS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_GETPROTCONFIGOPTIONS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PdpApi_SetProtConfigOptions(ClientInfo_t* inClientInfoPtr, UInt8 inCid, PCHProtConfig_t *inProtConfigPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PdpApi_SetProtConfigOptions_Req.inCid = inCid;
	req.req_rep_u.CAPI2_PdpApi_SetProtConfigOptions_Req.inProtConfigPtr = inProtConfigPtr;
	req.respId = MSG_PDP_SETPROTCONFIGOPTIONS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_PDP_SETPROTCONFIGOPTIONS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_LcsApi_RrlpSendDataToNetwork(ClientInfo_t* inClientInfoPtr, const UInt8 *inData, UInt32 inDataLen)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_LcsApi_RrlpSendDataToNetwork_Req.inData = (UInt8*)inData;
	req.req_rep_u.CAPI2_LcsApi_RrlpSendDataToNetwork_Req.inDataLen = inDataLen;
	req.respId = MSG_LCS_RRLP_SEND_DATA_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_LCS_RRLP_SEND_DATA_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_LcsApi_RrlpRegisterDataHandler(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_LCS_RRLP_REG_HDL_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_LCS_RRLP_REG_HDL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_LcsApi_RrcRegisterDataHandler(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_LCS_RRC_REG_HDL_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_LCS_RRC_REG_HDL_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_LcsApi_RrcSendUlDcch(ClientInfo_t* inClientInfoPtr, UInt8 *inData, UInt32 inDataLen)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_LcsApi_RrcSendUlDcch_Req.inData = inData;
	req.req_rep_u.CAPI2_LcsApi_RrcSendUlDcch_Req.inDataLen = inDataLen;
	req.respId = MSG_LCS_RRC_SEND_DL_DCCH_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_LCS_RRC_SEND_DL_DCCH_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_LcsApi_RrcMeasCtrlFailure(ClientInfo_t* inClientInfoPtr, UInt16 inTransactionId, LcsRrcMcFailure_t inMcFailure, UInt32 inErrorCode)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_LcsApi_RrcMeasCtrlFailure_Req.inTransactionId = inTransactionId;
	req.req_rep_u.CAPI2_LcsApi_RrcMeasCtrlFailure_Req.inMcFailure = inMcFailure;
	req.req_rep_u.CAPI2_LcsApi_RrcMeasCtrlFailure_Req.inErrorCode = inErrorCode;
	req.respId = MSG_LCS_RRC_MEAS_CTRL_FAILURE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_LCS_RRC_MEAS_CTRL_FAILURE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_LcsApi_RrcStatus(ClientInfo_t* inClientInfoPtr, LcsRrcMcStatus_t inStatus)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_LcsApi_RrcStatus_Req.inStatus = inStatus;
	req.respId = MSG_LCS_RRC_STAT_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_LCS_RRC_STAT_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_SimApi_PowerOnOffSim(ClientInfo_t* inClientInfoPtr, Boolean power_on, SIM_POWER_ON_MODE_t mode)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SimApi_PowerOnOffSim_Req.power_on = power_on;
	req.req_rep_u.CAPI2_SimApi_PowerOnOffSim_Req.mode = mode;
	req.respId = MSG_SIM_POWER_ON_OFF_SIM_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SIM_POWER_ON_OFF_SIM_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_PhoneCtrlApi_SetPagingStatus(ClientInfo_t* inClientInfoPtr, UInt8 status)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PhoneCtrlApi_SetPagingStatus_Req.status = status;
	req.respId = MSG_SYS_SET_PAGING_STATUS_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_SYS_SET_PAGING_STATUS_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_LcsApi_GetGpsCapabilities(ClientInfo_t* inClientInfoPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_LCS_GET_GPS_CAP_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_LCS_GET_GPS_CAP_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_LcsApi_SetGpsCapabilities(ClientInfo_t* inClientInfoPtr, UInt16 inCapMask)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_LcsApi_SetGpsCapabilities_Req.inCapMask = inCapMask;
	req.respId = MSG_LCS_SET_GPS_CAP_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_LCS_SET_GPS_CAP_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_CcApi_AbortDtmfTone(ClientInfo_t* inClientInfoPtr, ApiDtmf_t *inDtmfObjPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CcApi_AbortDtmfTone_Req.inDtmfObjPtr = inDtmfObjPtr;
	req.respId = MSG_CCAPI_ABORTDTMF_TONE_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_CCAPI_ABORTDTMF_TONE_REQ, inClientInfoPtr, result, &stream, &u32len);
}

void CAPI2_NetRegApi_SetSupportedRATandBandEx(ClientInfo_t* inClientInfoPtr, RATSelect_t RAT_cap, BandSelect_t band_cap, RATSelect_t RAT_cap2, BandSelect_t band_cap2)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 u32len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_NetRegApi_SetSupportedRATandBandEx_Req.RAT_cap = RAT_cap;
	req.req_rep_u.CAPI2_NetRegApi_SetSupportedRATandBandEx_Req.band_cap = band_cap;
	req.req_rep_u.CAPI2_NetRegApi_SetSupportedRATandBandEx_Req.RAT_cap2 = RAT_cap2;
	req.req_rep_u.CAPI2_NetRegApi_SetSupportedRATandBandEx_Req.band_cap2 = band_cap2;
	req.respId = MSG_MS_SET_RAT_BAND_EX_RSP;
	CAPI2_SerializeReqRspEx(&req, MSG_MS_SET_RAT_BAND_EX_REQ, inClientInfoPtr, result, &stream, &u32len);
}
