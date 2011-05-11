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


void GenGetPayloadInfo(void* dataBuf, MsgType_t msgType, void** ppBuf, UInt32* len)
{
	switch(msgType)
	{


	case MSG_MS_GSM_REGISTERED_RSP:
	{
		CAPI2_MS_IsGSMRegistered_Rsp_t* pVal = (CAPI2_MS_IsGSMRegistered_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_MS_GPRS_REGISTERED_RSP:
	{
		CAPI2_MS_IsGPRSRegistered_Rsp_t* pVal = (CAPI2_MS_IsGPRSRegistered_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_MS_GSM_CAUSE_RSP:
	{
		CAPI2_MS_GetGSMRegCause_Rsp_t* pVal = (CAPI2_MS_GetGSMRegCause_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_MS_GPRS_CAUSE_RSP:
	{
		CAPI2_MS_GetGPRSRegCause_Rsp_t* pVal = (CAPI2_MS_GetGPRSRegCause_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_MS_REGISTERED_LAC_RSP:
	{
		CAPI2_MS_GetRegisteredLAC_Rsp_t* pVal = (CAPI2_MS_GetRegisteredLAC_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_MS_GET_PLMN_MCC_RSP:
	{
		CAPI2_MS_GetPlmnMCC_Rsp_t* pVal = (CAPI2_MS_GetPlmnMCC_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_MS_GET_PLMN_MNC_RSP:
	{
		CAPI2_MS_GetPlmnMNC_Rsp_t* pVal = (CAPI2_MS_GetPlmnMNC_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SYS_POWERDOWN_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SYS_POWERUP_NORF_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SYS_NORF_CALIB_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SYS_POWERUP_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_MS_GPRS_ALLOWED_RSP:
	{
		CAPI2_MS_IsGprsAllowed_Rsp_t* pVal = (CAPI2_MS_IsGprsAllowed_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_MS_GET_CURRENT_RAT_RSP:
	{
		CAPI2_MS_GetCurrentRAT_Rsp_t* pVal = (CAPI2_MS_GetCurrentRAT_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_MS_GET_CURRENT_BAND_RSP:
	{
		CAPI2_MS_GetCurrentBand_Rsp_t* pVal = (CAPI2_MS_GetCurrentBand_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_UPDATE_SMSCAPEXC_FLAG_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_MS_SELECT_BAND_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_MS_SET_RAT_BAND_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_MS_CELL_LOCK_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_MS_GET_MCC_COUNTRY_RSP:
	{
		CAPI2_PLMN_GetCountryByMcc_Rsp_t* pVal = (CAPI2_PLMN_GetCountryByMcc_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_MS_PLMN_INFO_BY_CODE_RSP:
	{
		CAPI2_MS_GetPLMNEntryByIndex_Rsp_t* pVal = (CAPI2_MS_GetPLMNEntryByIndex_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_MS_PLMN_LIST_SIZE_RSP:
	{
		CAPI2_MS_GetPLMNListSize_Rsp_t* pVal = (CAPI2_MS_GetPLMNListSize_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_MS_PLMN_INFO_BY_INDEX_RSP:
	{
		CAPI2_MS_GetPLMNByCode_Rsp_t* pVal = (CAPI2_MS_GetPLMNByCode_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_PLMN_SELECT_RSP:
	{
		CAPI2_NetRegApi_PlmnSelect_Rsp_t* pVal = (CAPI2_NetRegApi_PlmnSelect_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_MS_PLMN_ABORT_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_MS_GET_PLMN_MODE_RSP:
	{
		CAPI2_MS_GetPlmnMode_Rsp_t* pVal = (CAPI2_MS_GetPlmnMode_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_MS_SET_PLMN_MODE_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_MS_GET_PLMN_FORMAT_RSP:
	{
		CAPI2_MS_GetPlmnFormat_Rsp_t* pVal = (CAPI2_MS_GetPlmnFormat_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_MS_SET_PLMN_FORMAT_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_MS_MATCH_PLMN_RSP:
	{
		CAPI2_MS_IsMatchedPLMN_Rsp_t* pVal = (CAPI2_MS_IsMatchedPLMN_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_MS_ABORT_PLMN_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_MS_AUTO_SEARCH_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_MS_PLMN_NAME_RSP:
	{
		CAPI2_NetRegApi_GetPLMNNameByCode_Rsp_t* pVal = (CAPI2_NetRegApi_GetPLMNNameByCode_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SYS_GET_SYSTEM_STATE_RSP:
	{
		CAPI2_PhoneCtrlApi_GetSystemState_Rsp_t* pVal = (CAPI2_PhoneCtrlApi_GetSystemState_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SYS_SET_SYSTEM_STATE_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SYS_GET_RX_LEVEL_RSP:
	{
		CAPI2_PhoneCtrlApi_GetRxSignalInfo_Rsp_t* pVal = (CAPI2_PhoneCtrlApi_GetRxSignalInfo_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SYS_GET_GSMREG_STATUS_RSP:
	{
		CAPI2_SYS_GetGSMRegistrationStatus_Rsp_t* pVal = (CAPI2_SYS_GetGSMRegistrationStatus_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SYS_GET_GPRSREG_STATUS_RSP:
	{
		CAPI2_SYS_GetGPRSRegistrationStatus_Rsp_t* pVal = (CAPI2_SYS_GetGPRSRegistrationStatus_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SYS_GET_REG_STATUS_RSP:
	{
		CAPI2_SYS_IsRegisteredGSMOrGPRS_Rsp_t* pVal = (CAPI2_SYS_IsRegisteredGSMOrGPRS_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_HOME_PLMN_REG_RSP:
	{
		CAPI2_MS_IsRegisteredHomePLMN_Rsp_t* pVal = (CAPI2_MS_IsRegisteredHomePLMN_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SET_POWER_DOWN_TIMER_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SIM_PARAM_REC_NUM_RSP:
	{
		CAPI2_SimApi_GetSmsParamRecNum_Rsp_t* pVal = (CAPI2_SimApi_GetSmsParamRecNum_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SIM_GET_SMSMEMEXC_FLAG_RSP:
	{
		CAPI2_SimApi_GetSmsMemExceededFlag_Rsp_t* pVal = (CAPI2_SimApi_GetSmsMemExceededFlag_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_IS_TEST_SIM_RSP:
	{
		CAPI2_SimApi_IsTestSIM_Rsp_t* pVal = (CAPI2_SimApi_IsTestSIM_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_PIN_REQ_RSP:
	{
		CAPI2_SimApi_IsPINRequired_Rsp_t* pVal = (CAPI2_SimApi_IsPINRequired_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_CARD_PHASE_RSP:
	{
		CAPI2_SimApi_GetCardPhase_Rsp_t* pVal = (CAPI2_SimApi_GetCardPhase_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_TYPE_RSP:
	{
		CAPI2_SimApi_GetSIMType_Rsp_t* pVal = (CAPI2_SimApi_GetSIMType_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_PRESENT_RSP:
	{
		CAPI2_SimApi_GetPresentStatus_Rsp_t* pVal = (CAPI2_SimApi_GetPresentStatus_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_PIN_OPERATION_RSP:
	{
		CAPI2_SimApi_IsOperationRestricted_Rsp_t* pVal = (CAPI2_SimApi_IsOperationRestricted_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_PIN_BLOCK_RSP:
	{
		CAPI2_SimApi_IsPINBlocked_Rsp_t* pVal = (CAPI2_SimApi_IsPINBlocked_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_PUK_BLOCK_RSP:
	{
		CAPI2_SimApi_IsPUKBlocked_Rsp_t* pVal = (CAPI2_SimApi_IsPUKBlocked_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_IS_INVALID_RSP:
	{
		CAPI2_SimApi_IsInvalidSIM_Rsp_t* pVal = (CAPI2_SimApi_IsInvalidSIM_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_DETECT_RSP:
	{
		CAPI2_SimApi_DetectSim_Rsp_t* pVal = (CAPI2_SimApi_DetectSim_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_GET_RUIM_SUPP_FLAG_RSP:
	{
		CAPI2_SimApi_GetRuimSuppFlag_Rsp_t* pVal = (CAPI2_SimApi_GetRuimSuppFlag_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_VERIFY_CHV_RSP:
	{
		CAPI2_SimApi_SendVerifyChvReq_Rsp_t* pVal = (CAPI2_SimApi_SendVerifyChvReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_CHANGE_CHV_RSP:
	{
		CAPI2_SimApi_SendChangeChvReq_Rsp_t* pVal = (CAPI2_SimApi_SendChangeChvReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_ENABLE_CHV_RSP:
	{
		CAPI2_SimApi_SendSetChv1OnOffReq_Rsp_t* pVal = (CAPI2_SimApi_SendSetChv1OnOffReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_UNBLOCK_CHV_RSP:
	{
		CAPI2_SimApi_SendUnblockChvReq_Rsp_t* pVal = (CAPI2_SimApi_SendUnblockChvReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_SET_FDN_RSP:
	{
		CAPI2_SimApi_SendSetOperStateReq_Rsp_t* pVal = (CAPI2_SimApi_SendSetOperStateReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_IS_PBK_ALLOWED_RSP:
	{
		CAPI2_SimApi_IsPbkAccessAllowed_Rsp_t* pVal = (CAPI2_SimApi_IsPbkAccessAllowed_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_PBK_INFO_RSP:
	{
		CAPI2_SimApi_SendPbkInfoReq_Rsp_t* pVal = (CAPI2_SimApi_SendPbkInfoReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_MAX_ACM_RSP:
	{
		CAPI2_SimApi_SendReadAcmMaxReq_Rsp_t* pVal = (CAPI2_SimApi_SendReadAcmMaxReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_ACM_MAX_UPDATE_RSP:
	{
		CAPI2_SimApi_SendWriteAcmMaxReq_Rsp_t* pVal = (CAPI2_SimApi_SendWriteAcmMaxReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_ACM_VALUE_RSP:
	{
		CAPI2_SimApi_SendReadAcmReq_Rsp_t* pVal = (CAPI2_SimApi_SendReadAcmReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_ACM_UPDATE_RSP:
	{
		CAPI2_SimApi_SendWriteAcmReq_Rsp_t* pVal = (CAPI2_SimApi_SendWriteAcmReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_ACM_INCREASE_RSP:
	{
		CAPI2_SimApi_SendIncreaseAcmReq_Rsp_t* pVal = (CAPI2_SimApi_SendIncreaseAcmReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_SVC_PROV_NAME_RSP:
	{
		CAPI2_SimApi_SendReadSvcProvNameReq_Rsp_t* pVal = (CAPI2_SimApi_SendReadSvcProvNameReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_PUCT_DATA_RSP:
	{
		CAPI2_SimApi_SendReadPuctReq_Rsp_t* pVal = (CAPI2_SimApi_SendReadPuctReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_SERVICE_STATUS_RSP:
	{
		CAPI2_SimApi_GetServiceStatus_Rsp_t* pVal = (CAPI2_SimApi_GetServiceStatus_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_PIN_STATUS_RSP:
	{
		CAPI2_SimApi_GetPinStatus_Rsp_t* pVal = (CAPI2_SimApi_GetPinStatus_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_PIN_OK_STATUS_RSP:
	{
		CAPI2_SimApi_IsPinOK_Rsp_t* pVal = (CAPI2_SimApi_IsPinOK_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_IMSI_RSP:
	{
		CAPI2_SimApi_GetIMSI_Rsp_t* pVal = (CAPI2_SimApi_GetIMSI_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_GID_DIGIT_RSP:
	{
		CAPI2_SimApi_GetGID1_Rsp_t* pVal = (CAPI2_SimApi_GetGID1_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_GID_DIGIT2_RSP:
	{
		CAPI2_SimApi_GetGID2_Rsp_t* pVal = (CAPI2_SimApi_GetGID2_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_HOME_PLMN_RSP:
	{
		CAPI2_SimApi_GetHomePlmn_Rsp_t* pVal = (CAPI2_SimApi_GetHomePlmn_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SIM_APDU_FILEID_RSP:
	{
		CAPI2_simmiApi_GetMasterFileId_Rsp_t* pVal = (CAPI2_simmiApi_GetMasterFileId_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_OPEN_SOCKET_RSP:
	{
		CAPI2_SimApi_SendOpenSocketReq_Rsp_t* pVal = (CAPI2_SimApi_SendOpenSocketReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_SELECT_APPLI_RSP:
	{
		CAPI2_SimApi_SendSelectAppiReq_Rsp_t* pVal = (CAPI2_SimApi_SendSelectAppiReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_DEACTIVATE_APPLI_RSP:
	{
		CAPI2_SimApi_SendDeactivateAppiReq_Rsp_t* pVal = (CAPI2_SimApi_SendDeactivateAppiReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_CLOSE_SOCKET_RSP:
	{
		CAPI2_SimApi_SendCloseSocketReq_Rsp_t* pVal = (CAPI2_SimApi_SendCloseSocketReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_ATR_DATA_RSP:
	{
		CAPI2_SimApi_GetAtrData_Rsp_t* pVal = (CAPI2_SimApi_GetAtrData_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SIM_DFILE_INFO_RSP:
	{
		CAPI2_SimApi_SubmitDFileInfoReq_Rsp_t* pVal = (CAPI2_SimApi_SubmitDFileInfoReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_EFILE_INFO_RSP:
	{
		CAPI2_SimApi_SubmitEFileInfoReq_Rsp_t* pVal = (CAPI2_SimApi_SubmitEFileInfoReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_EFILE_DATA_RSP:
	{
		CAPI2_SimApi_SubmitWholeBinaryEFileReadReq_Rsp_t* pVal = (CAPI2_SimApi_SubmitWholeBinaryEFileReadReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_EFILE_UPDATE_RSP:
	{
		CAPI2_SimApi_SubmitBinaryEFileUpdateReq_Rsp_t* pVal = (CAPI2_SimApi_SubmitBinaryEFileUpdateReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_SEEK_REC_RSP:
	{
		CAPI2_SimApi_SubmitSeekRecordReq_Rsp_t* pVal = (CAPI2_SimApi_SubmitSeekRecordReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_PIN_ATTEMPT_RSP:
	{
		CAPI2_SimApi_SendRemainingPinAttemptReq_Rsp_t* pVal = (CAPI2_SimApi_SendRemainingPinAttemptReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_CACHE_DATA_READY_RSP:
	{
		CAPI2_SimApi_IsCachedDataReady_Rsp_t* pVal = (CAPI2_SimApi_IsCachedDataReady_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_SERVICE_CODE_STATUS_RSP:
	{
		CAPI2_SimApi_GetServiceCodeStatus_Rsp_t* pVal = (CAPI2_SimApi_GetServiceCodeStatus_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_CHECK_CPHS_RSP:
	{
		CAPI2_SimApi_CheckCphsService_Rsp_t* pVal = (CAPI2_SimApi_CheckCphsService_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_CPHS_PHASE_RSP:
	{
		CAPI2_SimApi_GetCphsPhase_Rsp_t* pVal = (CAPI2_SimApi_GetCphsPhase_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_SMS_SCA_RSP:
	{
		CAPI2_SimApi_GetSmsSca_Rsp_t* pVal = (CAPI2_SimApi_GetSmsSca_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SIM_ICCID_PARAM_RSP:
	{
		CAPI2_SimApi_GetIccid_Rsp_t* pVal = (CAPI2_SimApi_GetIccid_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SIM_ALS_STATUS_RSP:
	{
		CAPI2_SimApi_IsALSEnabled_Rsp_t* pVal = (CAPI2_SimApi_IsALSEnabled_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_ALS_DEFAULT_LINE_RSP:
	{
		CAPI2_SimApi_GetAlsDefaultLine_Rsp_t* pVal = (CAPI2_SimApi_GetAlsDefaultLine_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_SET_ALS_DEFAULT_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SIM_CALLFWD_COND_FLAG_RSP:
	{
		CAPI2_SimApi_GetCallForwardUnconditionalFlag_Rsp_t* pVal = (CAPI2_SimApi_GetCallForwardUnconditionalFlag_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SIM_APP_TYPE_RSP:
	{
		CAPI2_SimApi_GetApplicationType_Rsp_t* pVal = (CAPI2_SimApi_GetApplicationType_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_PUCT_UPDATE_RSP:
	{
		CAPI2_SimApi_SendWritePuctReq_Rsp_t* pVal = (CAPI2_SimApi_SendWritePuctReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_RESTRICTED_ACCESS_RSP:
	{
		CAPI2_SimApi_SubmitRestrictedAccessReq_Rsp_t* pVal = (CAPI2_SimApi_SubmitRestrictedAccessReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_DETECTION_IND:
	{
		CAPI2_SIM_SendDetectionInd_Rsp_t* pVal = (CAPI2_SIM_SendDetectionInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_REG_GSM_IND:
	{
		CAPI2_MS_GsmRegStatusInd_Rsp_t* pVal = (CAPI2_MS_GsmRegStatusInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_REG_GPRS_IND:
	{
		CAPI2_MS_GprsRegStatusInd_Rsp_t* pVal = (CAPI2_MS_GprsRegStatusInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_NETWORK_NAME_IND:
	{
		CAPI2_MS_NetworkNameInd_Rsp_t* pVal = (CAPI2_MS_NetworkNameInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_LSA_ID_IND:
	{
		CAPI2_MS_LsaInd_Rsp_t* pVal = (CAPI2_MS_LsaInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_RSSI_IND:
	{
		CAPI2_MS_RssiInd_Rsp_t* pVal = (CAPI2_MS_RssiInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_RX_SIGNAL_INFO_CHG_IND:
	{
		CAPI2_MS_SignalChangeInd_Rsp_t* pVal = (CAPI2_MS_SignalChangeInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_PLMNLIST_IND:
	{
		CAPI2_MS_PlmnListInd_Rsp_t* pVal = (CAPI2_MS_PlmnListInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_DATE_TIMEZONE_IND:
	{
		CAPI2_MS_TimeZoneInd_Rsp_t* pVal = (CAPI2_MS_TimeZoneInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_MS_GET_SYSTEM_RAT_RSP:
	{
		CAPI2_MS_GetSystemRAT_Rsp_t* pVal = (CAPI2_MS_GetSystemRAT_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_MS_GET_SUPPORTED_RAT_RSP:
	{
		CAPI2_MS_GetSupportedRAT_Rsp_t* pVal = (CAPI2_MS_GetSupportedRAT_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_MS_GET_SYSTEM_BAND_RSP:
	{
		CAPI2_MS_GetSystemBand_Rsp_t* pVal = (CAPI2_MS_GetSystemBand_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_MS_GET_SUPPORTED_BAND_RSP:
	{
		CAPI2_MS_GetSupportedBand_Rsp_t* pVal = (CAPI2_MS_GetSupportedBand_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SYSPARAM_GET_MSCLASS_RSP:
	{
		CAPI2_SYSPARM_GetMSClass_Rsp_t* pVal = (CAPI2_SYSPARM_GetMSClass_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SYSPARAM_GET_MNF_NAME_RSP:
	{
		CAPI2_SYSPARM_GetManufacturerName_Rsp_t* pVal = (CAPI2_SYSPARM_GetManufacturerName_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SYSPARAM_GET_MODEL_NAME_RSP:
	{
		CAPI2_SYSPARM_GetModelName_Rsp_t* pVal = (CAPI2_SYSPARM_GetModelName_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SYSPARAM_GET_SW_VERSION_RSP:
	{
		CAPI2_SYSPARM_GetSWVersion_Rsp_t* pVal = (CAPI2_SYSPARM_GetSWVersion_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SYSPARAM_GET_EGPRS_CLASS_RSP:
	{
		CAPI2_SYSPARM_GetEGPRSMSClass_Rsp_t* pVal = (CAPI2_SYSPARM_GetEGPRSMSClass_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_PLMN_NUM_OF_ENTRY_RSP:
	{
		CAPI2_SimApi_SendNumOfPLMNEntryReq_Rsp_t* pVal = (CAPI2_SimApi_SendNumOfPLMNEntryReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_PLMN_ENTRY_DATA_RSP:
	{
		CAPI2_SimApi_SendReadPLMNEntryReq_Rsp_t* pVal = (CAPI2_SimApi_SendReadPLMNEntryReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_MUL_PLMN_ENTRY_UPDATE_RSP:
	{
		CAPI2_SimApi_SendWriteMulPLMNEntryReq_Rsp_t* pVal = (CAPI2_SimApi_SendWriteMulPLMNEntryReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SYS_SET_REG_EVENT_MASK_RSP:
	{
		CAPI2_SYS_SetRegisteredEventMask_Rsp_t* pVal = (CAPI2_SYS_SetRegisteredEventMask_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SYS_SET_REG_FILTER_MASK_RSP:
	{
		CAPI2_SYS_SetFilteredEventMask_Rsp_t* pVal = (CAPI2_SYS_SetFilteredEventMask_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SYS_SET_RSSI_THRESHOLD_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SYSPARAM_GET_CHANNEL_MODE_RSP:
	{
		CAPI2_SYSPARM_GetChanMode_Rsp_t* pVal = (CAPI2_SYSPARM_GetChanMode_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SYSPARAM_GET_CLASSMARK_RSP:
	{
		CAPI2_SYSPARM_GetClassmark_Rsp_t* pVal = (CAPI2_SYSPARM_GetClassmark_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SYSPARAM_GET_IND_FILE_VER_RSP:
	{
		CAPI2_SYSPARM_GetSysparmIndPartFileVersion_Rsp_t* pVal = (CAPI2_SYSPARM_GetSysparmIndPartFileVersion_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SYS_SET_DARP_CFG_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SYSPARAM_SET_EGPRS_MSCLASS_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SYSPARAM_SET_GPRS_MSCLASS_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_TIMEZONE_DELETE_NW_NAME_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_CAPI2_TEST_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_STK_SEND_PLAYTONE_RES_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_STK_SETUP_CALL_RES_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_PBK_SET_FDN_CHECK_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_PBK_GET_FDN_CHECK_RSP:
	{
		CAPI2_PbkApi_GetFdnCheck_Rsp_t* pVal = (CAPI2_PbkApi_GetFdnCheck_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SMS_SEND_MEM_AVAL_IND_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_CONFIGUREMESTORAGE_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_MS_SET_ELEMENT_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_MS_GET_ELEMENT_RSP:
	{
		CAPI2_MsDbApi_GetElement_Rsp_t* pVal = (CAPI2_MsDbApi_GetElement_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_USIM_IS_APP_SUPPORTED_RSP:
	{
		CAPI2_USimApi_IsApplicationSupported_Rsp_t* pVal = (CAPI2_USimApi_IsApplicationSupported_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_USIM_IS_APN_ALLOWED_RSP:
	{
		CAPI2_USimApi_IsAllowedAPN_Rsp_t* pVal = (CAPI2_USimApi_IsAllowedAPN_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_USIM_GET_NUM_APN_RSP:
	{
		CAPI2_USimApi_GetNumOfAPN_Rsp_t* pVal = (CAPI2_USimApi_GetNumOfAPN_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_USIM_GET_APN_ENTRY_RSP:
	{
		CAPI2_USimApi_GetAPNEntry_Rsp_t* pVal = (CAPI2_USimApi_GetAPNEntry_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_USIM_IS_EST_SERV_ACTIVATED_RSP:
	{
		CAPI2_USimApi_IsEstServActivated_Rsp_t* pVal = (CAPI2_USimApi_IsEstServActivated_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_SET_EST_SERV_RSP:
	{
		CAPI2_USimApi_SendSetEstServReq_Rsp_t* pVal = (CAPI2_USimApi_SendSetEstServReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_UPDATE_ONE_APN_RSP:
	{
		CAPI2_USimApi_SendWriteAPNReq_Rsp_t* pVal = (CAPI2_USimApi_SendWriteAPNReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_DELETE_ALL_APN_RSP:
	{
		CAPI2_USimApi_SendDeleteAllAPNReq_Rsp_t* pVal = (CAPI2_USimApi_SendDeleteAllAPNReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_USIM_GET_RAT_MODE_RSP:
	{
		CAPI2_USimApi_GetRatModeSetting_Rsp_t* pVal = (CAPI2_USimApi_GetRatModeSetting_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_MS_GET_GPRS_STATE_RSP:
	{
		CAPI2_MS_GetGPRSRegState_Rsp_t* pVal = (CAPI2_MS_GetGPRSRegState_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_MS_GET_GSM_STATE_RSP:
	{
		CAPI2_MS_GetGSMRegState_Rsp_t* pVal = (CAPI2_MS_GetGSMRegState_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_MS_GET_CELL_INFO_RSP:
	{
		CAPI2_MS_GetRegisteredCellInfo_Rsp_t* pVal = (CAPI2_MS_GetRegisteredCellInfo_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_MS_SETMEPOWER_CLASS_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_USIM_GET_SERVICE_STATUS_RSP:
	{
		CAPI2_USimApi_GetServiceStatus_Rsp_t* pVal = (CAPI2_USimApi_GetServiceStatus_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_IS_ALLOWED_APN_RSP:
	{
		CAPI2_SimApi_IsAllowedAPN_Rsp_t* pVal = (CAPI2_SimApi_IsAllowedAPN_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SMS_GETSMSMAXCAPACITY_RSP:
	{
		CAPI2_SmsApi_GetSmsMaxCapacity_Rsp_t* pVal = (CAPI2_SmsApi_GetSmsMaxCapacity_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SMS_RETRIEVEMAXCBCHNLLENGTH_RSP:
	{
		CAPI2_SmsApi_RetrieveMaxCBChnlLength_Rsp_t* pVal = (CAPI2_SmsApi_RetrieveMaxCBChnlLength_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SIM_IS_BDN_RESTRICTED_RSP:
	{
		CAPI2_SimApi_IsBdnOperationRestricted_Rsp_t* pVal = (CAPI2_SimApi_IsBdnOperationRestricted_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_SEND_PLMN_UPDATE_IND_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SIMIO_DEACTIVATE_CARD_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SIM_SET_BDN_RSP:
	{
		CAPI2_SimApi_SendSetBdnReq_Rsp_t* pVal = (CAPI2_SimApi_SendSetBdnReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_POWER_ON_OFF_CARD_RSP:
	{
		CAPI2_SimApi_PowerOnOffCard_Rsp_t* pVal = (CAPI2_SimApi_PowerOnOffCard_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_GET_RAW_ATR_RSP:
	{
		CAPI2_SimApi_GetRawAtr_Rsp_t* pVal = (CAPI2_SimApi_GetRawAtr_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_SET_PROTOCOL_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SIM_GET_PROTOCOL_RSP:
	{
		CAPI2_SimApi_Get_Protocol_Rsp_t* pVal = (CAPI2_SimApi_Get_Protocol_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_SEND_GENERIC_APDU_CMD_RSP:
	{
		CAPI2_SimApi_SendGenericApduCmd_Rsp_t* pVal = (CAPI2_SimApi_SendGenericApduCmd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_TERMINATE_XFER_APDU_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SIM_GET_SIM_INTERFACE_RSP:
	{
		CAPI2_SIM_GetSimInterface_Rsp_t* pVal = (CAPI2_SIM_GetSimInterface_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SET_PLMN_SELECT_RAT_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_IS_DEREGISTER_IN_PROGRESS_RSP:
	{
		CAPI2_MS_IsDeRegisterInProgress_Rsp_t* pVal = (CAPI2_MS_IsDeRegisterInProgress_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_IS_REGISTER_IN_PROGRESS_RSP:
	{
		CAPI2_MS_IsRegisterInProgress_Rsp_t* pVal = (CAPI2_MS_IsRegisterInProgress_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_READ_USIM_PBK_HDK_RSP:
	{
		CAPI2_PbkApi_SendUsimHdkReadReq_Rsp_t* pVal = (CAPI2_PbkApi_SendUsimHdkReadReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_WRITE_USIM_PBK_HDK_RSP:
	{
		CAPI2_PbkApi_SendUsimHdkUpdateReq_Rsp_t* pVal = (CAPI2_PbkApi_SendUsimHdkUpdateReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_READ_USIM_PBK_ALPHA_AAS_RSP:
	{
		CAPI2_PbkApi_SendUsimAasReadReq_Rsp_t* pVal = (CAPI2_PbkApi_SendUsimAasReadReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_WRITE_USIM_PBK_ALPHA_AAS_RSP:
	{
		CAPI2_PbkApi_SendUsimAasUpdateReq_Rsp_t* pVal = (CAPI2_PbkApi_SendUsimAasUpdateReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_GET_USIM_PBK_ALPHA_INFO_AAS_RSP:
	{
		CAPI2_PbkApi_SendUsimAasInfoReq_Rsp_t* pVal = (CAPI2_PbkApi_SendUsimAasInfoReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_MEASURE_REPORT_PARAM_IND:
	{
		CAPI2_MS_MeasureReportInd_Rsp_t* pVal = (CAPI2_MS_MeasureReportInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_DIAG_MEASURE_REPORT_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_MS_INITCALLCFG_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_MS_INITFAXCFG_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_MS_INITVIDEOCALLCFG_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_MS_INITCALLCFGAMPF_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_MS_STATUS_IND:
	{
		CAPI2_MS_StatusInd_Rsp_t* pVal = (CAPI2_MS_StatusInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_POWER_DOWN_CNF:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SYSPARM_GET_ACTUAL_LOW_VOLT_RSP:
	{
		CAPI2_SYSPARM_GetActualLowVoltReading_Rsp_t* pVal = (CAPI2_SYSPARM_GetActualLowVoltReading_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SYSPARM_GET_ACTUAL_4P2_VOLT_RSP:
	{
		CAPI2_SYSPARM_GetActual4p2VoltReading_Rsp_t* pVal = (CAPI2_SYSPARM_GetActual4p2VoltReading_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SMS_SEND_COMMAND_TXT_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_SEND_ACKTONETWORK_PDU_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_START_CB_WITHCHNL_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_SET_TPMR_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SIMLOCK_SET_STATUS_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_UE_3G_STATUS_IND:
	{
		CAPI2_MS_Ue3gStatusInd_Rsp_t* pVal = (CAPI2_MS_Ue3gStatusInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_DIAG_CELLLOCK_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_DIAG_CELLLOCK_STATUS_RSP:
	{
		CAPI2_DIAG_ApiCellLockStatus_Rsp_t* pVal = (CAPI2_DIAG_ApiCellLockStatus_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_MS_SET_RUA_READY_TIMER_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_LCS_RRLP_DATA_IND:
	{
		CAPI2_LCS_RrlpDataInd_Rsp_t* pVal = (CAPI2_LCS_RrlpDataInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_LCS_RRLP_RESET_POS_STORED_INFO_IND:
	{
		CAPI2_LCS_RrlpReset_Rsp_t* pVal = (CAPI2_LCS_RrlpReset_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_LCS_RRC_ASSISTANCE_DATA_IND:
	{
		CAPI2_LCS_RrcAssistDataInd_Rsp_t* pVal = (CAPI2_LCS_RrcAssistDataInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_LCS_RRC_MEASUREMENT_CTRL_IND:
	{
		CAPI2_LCS_RrcMeasCtrlInd_Rsp_t* pVal = (CAPI2_LCS_RrcMeasCtrlInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_LCS_RRC_BROADCAST_SYS_INFO_IND:
	{
		CAPI2_LCS_RrcSysInfoInd_Rsp_t* pVal = (CAPI2_LCS_RrcSysInfoInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_LCS_RRC_UE_STATE_IND:
	{
		CAPI2_LCS_RrcUeStateInd_Rsp_t* pVal = (CAPI2_LCS_RrcUeStateInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_LCS_RRC_STOP_MEASUREMENT_IND:
	{
		CAPI2_LCS_RrcStopMeasInd_Rsp_t* pVal = (CAPI2_LCS_RrcStopMeasInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_LCS_RRC_RESET_POS_STORED_INFO_IND:
	{
		CAPI2_LCS_RrcReset_Rsp_t* pVal = (CAPI2_LCS_RrcReset_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_CC_IS_THERE_EMERGENCY_CALL_RSP:
	{
		CAPI2_CcApi_IsThereEmergencyCall_Rsp_t* pVal = (CAPI2_CcApi_IsThereEmergencyCall_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SERVING_CELL_INFO_IND:
	{
		CAPI2_ServingCellInfoInd_Rsp_t* pVal = (CAPI2_ServingCellInfoInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SYS_ENABLE_CELL_INFO_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_L1_BB_ISLOCKED_RSP:
	{
		CAPI2_LCS_L1_bb_isLocked_Rsp_t* pVal = (CAPI2_LCS_L1_bb_isLocked_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SMSPP_APP_SPECIFIC_SMS_IND:
	{
		CAPI2_SMSPP_AppSpecificInd_Rsp_t* pVal = (CAPI2_SMSPP_AppSpecificInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_UTIL_DIAL_STR_PARSE_RSP:
	{
		CAPI2_DIALSTR_ParseGetCallType_Rsp_t* pVal = (CAPI2_DIALSTR_ParseGetCallType_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_LCS_FTT_DELTA_TIME_RSP:
	{
		CAPI2_LCS_FttCalcDeltaTime_Rsp_t* pVal = (CAPI2_LCS_FttCalcDeltaTime_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_LCS_FTT_SYNC_RESULT_IND:
	{
		CAPI2_LCS_SyncResult_Rsp_t* pVal = (CAPI2_LCS_SyncResult_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_MS_FORCEDREADYSTATE_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SS_RESETSSALSFLAG_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SIMLOCK_GET_STATUS_RSP:
	{
		CAPI2_SIMLOCK_GetStatus_RSP_Rsp_t* pVal = (CAPI2_SIMLOCK_GetStatus_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DIALSTR_IS_VALID_RSP:
	{
		CAPI2_DIALSTR_IsValidString_Rsp_t* pVal = (CAPI2_DIALSTR_IsValidString_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_UTIL_CONVERT_NTWK_CAUSE_RSP:
	{
		CAPI2_UTIL_Cause2NetworkCause_Rsp_t* pVal = (CAPI2_UTIL_Cause2NetworkCause_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_UTIL_CONVERT_ERRCODE_NTWK_CAUSE_RSP:
	{
		CAPI2_UTIL_ErrCodeToNetCause_Rsp_t* pVal = (CAPI2_UTIL_ErrCodeToNetCause_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_ISGPRS_DIAL_STR_RSP:
	{
		CAPI2_IsGprsDialStr_Rsp_t* pVal = (CAPI2_IsGprsDialStr_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_GET_NUM_SS_STR_RSP:
	{
		CAPI2_UTIL_GetNumOffsetInSsStr_Rsp_t* pVal = (CAPI2_UTIL_GetNumOffsetInSsStr_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_CIPHER_ALG_IND:
	{
		CAPI2_CipherAlgInd_Rsp_t* pVal = (CAPI2_CipherAlgInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_NW_MEAS_RESULT_IND:
	{
		CAPI2_NWMeasResultInd_Rsp_t* pVal = (CAPI2_NWMeasResultInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_DIALSTR_IS_PPPLOOPBACK_RSP:
	{
		CAPI2_IsPppLoopbackDialStr_Rsp_t* pVal = (CAPI2_IsPppLoopbackDialStr_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SYS_GETRIPPROCVERSION_RSP:
	{
		CAPI2_SYS_GetRIPPROCVersion_Rsp_t* pVal = (CAPI2_SYS_GetRIPPROCVersion_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SYSPARM_SET_HSDPA_PHY_CAT_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SYS_GET_HSDPA_CATEGORY_RSP:
	{
		CAPI2_SYSPARM_GetHSDPAPHYCategory_Rsp_t* pVal = (CAPI2_SYSPARM_GetHSDPAPHYCategory_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SMS_CONVERT_MSGTYPE_RSP:
	{
		CAPI2_SmsApi_ConvertSmsMSMsgType_Rsp_t* pVal = (CAPI2_SmsApi_ConvertSmsMSMsgType_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_NET_UARFCN_DL_IND:
	{
		CAPI2_NetUarfcnDlInd_Rsp_t* pVal = (CAPI2_NetUarfcnDlInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_MS_GETPREFNETSTATUS_RSP:
	{
		CAPI2_MS_GetPrefNetStatus_Rsp_t* pVal = (CAPI2_MS_GetPrefNetStatus_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SYS_TEST_MSG_IND_1:
	{
		SYS_TestSysMsgs1_Rsp_t* pVal = (SYS_TestSysMsgs1_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SYS_TEST_MSG_IND_2:
	{
		SYS_TestSysMsgs2_Rsp_t* pVal = (SYS_TestSysMsgs2_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SYS_TEST_MSG_IND_3:
	{
		SYS_TestSysMsgs3_Rsp_t* pVal = (SYS_TestSysMsgs3_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SYS_TEST_MSG_IND_4:
	{
		SYS_TestSysMsgs4_Rsp_t* pVal = (SYS_TestSysMsgs4_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SYS_TEST_MSG_IND_5:
	{
		SYS_TestSysMsgs5_Rsp_t* pVal = (SYS_TestSysMsgs5_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SYS_TEST_MSG_IND_6:
	{
		SYS_TestSysMsgs6_Rsp_t* pVal = (SYS_TestSysMsgs6_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SYSPARM_SET_HSUPA_PHY_CAT_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SYSPARM_GET_HSUPA_PHY_CAT_RSP:
	{
		CAPI2_SYSPARM_GetHSUPAPHYCategory_Rsp_t* pVal = (CAPI2_SYSPARM_GetHSUPAPHYCategory_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_INTERTASK_MSG_TO_CP_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_CC_GETCURRENTCALLINDEX_RSP:
	{
		CAPI2_CcApi_GetCurrentCallIndex_Rsp_t* pVal = (CAPI2_CcApi_GetCurrentCallIndex_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_CC_GETNEXTACTIVECALLINDEX_RSP:
	{
		CAPI2_CcApi_GetNextActiveCallIndex_Rsp_t* pVal = (CAPI2_CcApi_GetNextActiveCallIndex_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_CC_GETNEXTHELDCALLINDEX_RSP:
	{
		CAPI2_CcApi_GetNextHeldCallIndex_Rsp_t* pVal = (CAPI2_CcApi_GetNextHeldCallIndex_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_CC_GETNEXTWAITCALLINDEX_RSP:
	{
		CAPI2_CcApi_GetNextWaitCallIndex_Rsp_t* pVal = (CAPI2_CcApi_GetNextWaitCallIndex_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_CC_GETMPTYCALLINDEX_RSP:
	{
		CAPI2_CcApi_GetMPTYCallIndex_Rsp_t* pVal = (CAPI2_CcApi_GetMPTYCallIndex_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_CC_GETCALLSTATE_RSP:
	{
		CAPI2_CcApi_GetCallState_Rsp_t* pVal = (CAPI2_CcApi_GetCallState_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_CC_GETCALLTYPE_RSP:
	{
		CAPI2_CcApi_GetCallType_Rsp_t* pVal = (CAPI2_CcApi_GetCallType_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_CC_GETLASTCALLEXITCAUSE_RSP:
	{
		CAPI2_CcApi_GetLastCallExitCause_Rsp_t* pVal = (CAPI2_CcApi_GetLastCallExitCause_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_CC_GETCALLNUMBER_RSP:
	{
		CAPI2_CcApi_GetCallNumber_Rsp_t* pVal = (CAPI2_CcApi_GetCallNumber_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_CC_GETCALLINGINFO_RSP:
	{
		CAPI2_CcApi_GetCallingInfo_Rsp_t* pVal = (CAPI2_CcApi_GetCallingInfo_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_CC_GETALLCALLSTATES_RSP:
	{
		CAPI2_CcApi_GetAllCallStates_Rsp_t* pVal = (CAPI2_CcApi_GetAllCallStates_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_CC_GETALLCALLINDEX_RSP:
	{
		CAPI2_CcApi_GetAllCallIndex_Rsp_t* pVal = (CAPI2_CcApi_GetAllCallIndex_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_CC_GETALLHELDCALLINDEX_RSP:
	{
		CAPI2_CcApi_GetAllHeldCallIndex_Rsp_t* pVal = (CAPI2_CcApi_GetAllHeldCallIndex_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_CC_GETALLACTIVECALLINDEX_RSP:
	{
		CAPI2_CcApi_GetAllActiveCallIndex_Rsp_t* pVal = (CAPI2_CcApi_GetAllActiveCallIndex_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_CC_GETALLMPTYCALLINDEX_RSP:
	{
		CAPI2_CcApi_GetAllMPTYCallIndex_Rsp_t* pVal = (CAPI2_CcApi_GetAllMPTYCallIndex_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_CC_GETNUMOFMPTYCALLS_RSP:
	{
		CAPI2_CcApi_GetNumOfMPTYCalls_Rsp_t* pVal = (CAPI2_CcApi_GetNumOfMPTYCalls_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_CC_GETNUMOFACTIVECALLS_RSP:
	{
		CAPI2_CcApi_GetNumofActiveCalls_Rsp_t* pVal = (CAPI2_CcApi_GetNumofActiveCalls_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_CC_GETNUMOFHELDCALLS_RSP:
	{
		CAPI2_CcApi_GetNumofHeldCalls_Rsp_t* pVal = (CAPI2_CcApi_GetNumofHeldCalls_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_CC_ISTHEREWAITINGCALL_RSP:
	{
		CAPI2_CcApi_IsThereWaitingCall_Rsp_t* pVal = (CAPI2_CcApi_IsThereWaitingCall_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_CC_ISTHEREALERTINGCALL_RSP:
	{
		CAPI2_CcApi_IsThereAlertingCall_Rsp_t* pVal = (CAPI2_CcApi_IsThereAlertingCall_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_CC_GETCONNECTEDLINEID_RSP:
	{
		CAPI2_CcApi_GetConnectedLineID_Rsp_t* pVal = (CAPI2_CcApi_GetConnectedLineID_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_CC_GET_CALL_PRESENT_RSP:
	{
		CAPI2_CcApi_GetCallPresent_Rsp_t* pVal = (CAPI2_CcApi_GetCallPresent_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_CC_GET_INDEX_STATE_RSP:
	{
		CAPI2_CcApi_GetCallIndexInThisState_Rsp_t* pVal = (CAPI2_CcApi_GetCallIndexInThisState_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_CC_ISMULTIPARTYCALL_RSP:
	{
		CAPI2_CcApi_IsMultiPartyCall_Rsp_t* pVal = (CAPI2_CcApi_IsMultiPartyCall_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_CC_ISTHEREVOICECALL_RSP:
	{
		CAPI2_CcApi_IsThereVoiceCall_Rsp_t* pVal = (CAPI2_CcApi_IsThereVoiceCall_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_RSP:
	{
		CAPI2_CcApi_IsConnectedLineIDPresentAllowed_Rsp_t* pVal = (CAPI2_CcApi_IsConnectedLineIDPresentAllowed_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_RSP:
	{
		CAPI2_CcApi_GetCurrentCallDurationInMilliSeconds_Rsp_t* pVal = (CAPI2_CcApi_GetCurrentCallDurationInMilliSeconds_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_CC_GETLASTCALLCCM_RSP:
	{
		CAPI2_CcApi_GetLastCallCCM_Rsp_t* pVal = (CAPI2_CcApi_GetLastCallCCM_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_CC_GETLASTCALLDURATION_RSP:
	{
		CAPI2_CcApi_GetLastCallDuration_Rsp_t* pVal = (CAPI2_CcApi_GetLastCallDuration_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_CC_GETLASTDATACALLRXBYTES_RSP:
	{
		CAPI2_CcApi_GetLastDataCallRxBytes_Rsp_t* pVal = (CAPI2_CcApi_GetLastDataCallRxBytes_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_CC_GETLASTDATACALLTXBYTES_RSP:
	{
		CAPI2_CcApi_GetLastDataCallTxBytes_Rsp_t* pVal = (CAPI2_CcApi_GetLastDataCallTxBytes_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_CC_GETDATACALLINDEX_RSP:
	{
		CAPI2_CcApi_GetDataCallIndex_Rsp_t* pVal = (CAPI2_CcApi_GetDataCallIndex_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_CC_GETCALLCLIENT_INFO_RSP:
	{
		CAPI2_CcApi_GetCallClientInfo_Rsp_t* pVal = (CAPI2_CcApi_GetCallClientInfo_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_CC_GETCALLCLIENTID_RSP:
	{
		CAPI2_CcApi_GetCallClientID_Rsp_t* pVal = (CAPI2_CcApi_GetCallClientID_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_CC_GETTYPEADD_RSP:
	{
		CAPI2_CcApi_GetTypeAdd_Rsp_t* pVal = (CAPI2_CcApi_GetTypeAdd_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_CC_SETVOICECALLAUTOREJECT_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_CC_ISVOICECALLAUTOREJECT_RSP:
	{
		CAPI2_CcApi_IsVoiceCallAutoReject_Rsp_t* pVal = (CAPI2_CcApi_IsVoiceCallAutoReject_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_CC_SETTTYCALL_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_CC_ISTTYENABLE_RSP:
	{
		CAPI2_CcApi_IsTTYEnable_Rsp_t* pVal = (CAPI2_CcApi_IsTTYEnable_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_CC_ISSIMORIGINEDCALL_RSP:
	{
		CAPI2_CcApi_IsSimOriginedCall_Rsp_t* pVal = (CAPI2_CcApi_IsSimOriginedCall_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_CC_SETVIDEOCALLPARAM_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_CC_GETVIDEOCALLPARAM_RSP:
	{
		CAPI2_CcApi_GetVideoCallParam_Rsp_t* pVal = (CAPI2_CcApi_GetVideoCallParam_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_CC_GETCCM_RSP:
	{
		CAPI2_CcApi_GetCCM_Rsp_t* pVal = (CAPI2_CcApi_GetCCM_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_CCAPI_SENDDTMF_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_CCAPI_STOPDTMF_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_CCAPI_ABORTDTMF_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_CCAPI_SETDTMFTIMER_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_CCAPI_RESETDTMFTIMER_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_CCAPI_GETDTMFTIMER_RSP:
	{
		CAPI2_CcApi_GetDtmfToneTimer_Rsp_t* pVal = (CAPI2_CcApi_GetDtmfToneTimer_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_CCAPI_GETTIFROMCALLINDEX_RSP:
	{
		CAPI2_CcApi_GetTiFromCallIndex_Rsp_t* pVal = (CAPI2_CcApi_GetTiFromCallIndex_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_CCAPI_IS_SUPPORTEDBC_RSP:
	{
		CAPI2_CcApi_IsSupportedBC_Rsp_t* pVal = (CAPI2_CcApi_IsSupportedBC_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_CCAPI_IS_BEARER_CAPABILITY_RSP:
	{
		CAPI2_CcApi_GetBearerCapability_Rsp_t* pVal = (CAPI2_CcApi_GetBearerCapability_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SMS_SENDSMSSRVCENTERNUMBERUPDATE_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_GETSMSSRVCENTERNUMBER_RSP:
	{
		CAPI2_SmsApi_GetSMSrvCenterNumber_Rsp_t* pVal = (CAPI2_SmsApi_GetSMSrvCenterNumber_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SMS_GETSIMSMSCAPACITYEXCEEDEDFLAG_RSP:
	{
		CAPI2_SmsApi_GetSIMSMSCapacityExceededFlag_Rsp_t* pVal = (CAPI2_SmsApi_GetSIMSMSCapacityExceededFlag_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SMS_ISSMSSERVICEAVAIL_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_GETSMSSTOREDSTATE_RSP:
	{
		CAPI2_SmsApi_GetSmsStoredState_Rsp_t* pVal = (CAPI2_SmsApi_GetSmsStoredState_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SMS_WRITE_RSP_IND:
	{
		CAPI2_SmsApi_WriteSMSPduReq_Rsp_t* pVal = (CAPI2_SmsApi_WriteSMSPduReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SMS_SUBMIT_RSP:
	{
		CAPI2_SmsApi_SendSMSReq_Rsp_t* pVal = (CAPI2_SmsApi_SendSMSReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SMS_GETLASTTPMR_RSP:
	{
		CAPI2_SmsApi_GetLastTpMr_Rsp_t* pVal = (CAPI2_SmsApi_GetLastTpMr_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SMS_GETSMSTXPARAMS_RSP:
	{
		CAPI2_SmsApi_GetSmsTxParams_Rsp_t* pVal = (CAPI2_SmsApi_GetSmsTxParams_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SMS_GETTXPARAMINTEXTMODE_RSP:
	{
		CAPI2_SmsApi_GetTxParamInTextMode_Rsp_t* pVal = (CAPI2_SmsApi_GetTxParamInTextMode_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SMS_SETSMSTXPARAMPROCID_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_SETSMSTXPARAMCODINGTYPE_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_SETSMSTXPARAMVALIDPERIOD_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_SETSMSTXPARAMCOMPRESSION_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_SETSMSTXPARAMREPLYPATH_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_SETSMSTXPARAMUSERDATAHDRIND_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_SETSMSTXPARAMSTATUSRPTREQFLAG_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_SETSMSTXPARAMREJDUPL_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SIM_SMS_STATUS_UPD_RSP:
	{
		CAPI2_SmsApi_DeleteSmsMsgByIndexReq_Rsp_t* pVal = (CAPI2_SmsApi_DeleteSmsMsgByIndexReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SMS_SETNEWMSGDISPLAYPREF_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_GETNEWMSGDISPLAYPREF_RSP:
	{
		CAPI2_SmsApi_GetNewMsgDisplayPref_Rsp_t* pVal = (CAPI2_SmsApi_GetNewMsgDisplayPref_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SMS_SETSMSPREFSTORAGE_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_GETSMSPREFSTORAGE_RSP:
	{
		CAPI2_SmsApi_GetSMSPrefStorage_Rsp_t* pVal = (CAPI2_SmsApi_GetSMSPrefStorage_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SMS_GETSMSSTORAGESTATUS_RSP:
	{
		CAPI2_SmsApi_GetSMSStorageStatus_Rsp_t* pVal = (CAPI2_SmsApi_GetSMSStorageStatus_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SMS_SAVESMSSERVICEPROFILE_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_RESTORESMSSERVICEPROFILE_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_CB_START_STOP_RSP:
	{
		CAPI2_SmsApi_SetCellBroadcastMsgTypeReq_Rsp_t* pVal = (CAPI2_SmsApi_SetCellBroadcastMsgTypeReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SMS_CBALLOWALLCHNLREQ_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_ADDCELLBROADCASTCHNLREQ_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_REMOVECELLBROADCASTCHNLREQ_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_REMOVEALLCBCHNLFROMSEARCHLIST_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_GETCBMI_RSP:
	{
		CAPI2_SmsApi_GetCBMI_Rsp_t* pVal = (CAPI2_SmsApi_GetCBMI_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SMS_GETCBLANGUAGE_RSP:
	{
		CAPI2_SmsApi_GetCbLanguage_Rsp_t* pVal = (CAPI2_SmsApi_GetCbLanguage_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SMS_ADDCELLBROADCASTLANGREQ_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_REMOVECELLBROADCASTLANGREQ_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_SETCBIGNOREDUPLFLAG_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_GETCBIGNOREDUPLFLAG_RSP:
	{
		CAPI2_SmsApi_GetCBIgnoreDuplFlag_Rsp_t* pVal = (CAPI2_SmsApi_GetCBIgnoreDuplFlag_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SMS_SETVMINDONOFF_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_ISVMINDENABLED_RSP:
	{
		CAPI2_SmsApi_IsVMIndEnabled_Rsp_t* pVal = (CAPI2_SmsApi_IsVMIndEnabled_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SMS_GETVMWAITINGSTATUS_RSP:
	{
		CAPI2_SmsApi_GetVMWaitingStatus_Rsp_t* pVal = (CAPI2_SmsApi_GetVMWaitingStatus_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SMS_GETNUMOFVMSCNUMBER_RSP:
	{
		CAPI2_SmsApi_GetNumOfVmscNumber_Rsp_t* pVal = (CAPI2_SmsApi_GetNumOfVmscNumber_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SMS_GETVMSCNUMBER_RSP:
	{
		CAPI2_SmsApi_GetVmscNumber_Rsp_t* pVal = (CAPI2_SmsApi_GetVmscNumber_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SMS_UPDATEVMSCNUMBERREQ_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_GETSMSBEARERPREFERENCE_RSP:
	{
		CAPI2_SmsApi_GetSMSBearerPreference_Rsp_t* pVal = (CAPI2_SmsApi_GetSMSBearerPreference_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SMS_SETSMSBEARERPREFERENCE_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_SETSMSREADSTATUSCHANGEMODE_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_GETSMSREADSTATUSCHANGEMODE_RSP:
	{
		CAPI2_SmsApi_GetSmsReadStatusChangeMode_Rsp_t* pVal = (CAPI2_SmsApi_GetSmsReadStatusChangeMode_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SMS_CHANGESTATUSREQ_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_SENDMESTOREDSTATUSIND_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_SENDMERETRIEVESMSDATAIND_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_SENDMEREMOVEDSTATUSIND_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_SETSMSSTOREDSTATE_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_ISCACHEDDATAREADY_RSP:
	{
		CAPI2_SmsApi_IsCachedDataReady_Rsp_t* pVal = (CAPI2_SmsApi_IsCachedDataReady_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SMS_GETENHANCEDVMINFOIEI_RSP:
	{
		CAPI2_SmsApi_GetEnhancedVMInfoIEI_Rsp_t* pVal = (CAPI2_SmsApi_GetEnhancedVMInfoIEI_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SMS_SETALLNEWMSGDISPLAYPREF_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_ACKTONETWORK_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_STARTMULTISMSTRANSFER_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_STOPMULTISMSTRANSFER_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_START_CELL_BROADCAST_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_SIMINIT_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_PDA_OVERFLOW_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_ISIM_AUTHEN_AKA_RSP:
	{
		CAPI2_ISimApi_SendAuthenAkaReq_Rsp_t* pVal = (CAPI2_ISimApi_SendAuthenAkaReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_ISIM_ISISIMSUPPORTED_RSP:
	{
		CAPI2_ISimApi_IsIsimSupported_Rsp_t* pVal = (CAPI2_ISimApi_IsIsimSupported_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_ISIM_ISISIMACTIVATED_RSP:
	{
		CAPI2_ISimApi_IsIsimActivated_Rsp_t* pVal = (CAPI2_ISimApi_IsIsimActivated_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_ISIM_ACTIVATE_RSP:
	{
		CAPI2_ISimApi_ActivateIsimAppli_Rsp_t* pVal = (CAPI2_ISimApi_ActivateIsimAppli_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_ISIM_AUTHEN_HTTP_RSP:
	{
		CAPI2_ISimApi_SendAuthenHttpReq_Rsp_t* pVal = (CAPI2_ISimApi_SendAuthenHttpReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_ISIM_AUTHEN_GBA_NAF_RSP:
	{
		CAPI2_ISimApi_SendAuthenGbaNafReq_Rsp_t* pVal = (CAPI2_ISimApi_SendAuthenGbaNafReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_ISIM_AUTHEN_GBA_BOOT_RSP:
	{
		CAPI2_ISimApi_SendAuthenGbaBootReq_Rsp_t* pVal = (CAPI2_ISimApi_SendAuthenGbaBootReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_PBK_GETALPHA_RSP:
	{
		CAPI2_PbkApi_GetAlpha_Rsp_t* pVal = (CAPI2_PbkApi_GetAlpha_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_PBK_ISEMERGENCYCALLNUMBER_RSP:
	{
		CAPI2_PbkApi_IsEmergencyCallNumber_Rsp_t* pVal = (CAPI2_PbkApi_IsEmergencyCallNumber_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_RSP:
	{
		CAPI2_PbkApi_IsPartialEmergencyCallNumber_Rsp_t* pVal = (CAPI2_PbkApi_IsPartialEmergencyCallNumber_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_GET_PBK_INFO_RSP:
	{
		CAPI2_PbkApi_SendInfoReq_Rsp_t* pVal = (CAPI2_PbkApi_SendInfoReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_PBK_ENTRY_DATA_RSP:
	{
		CAPI2_PbkApi_SendFindAlphaMatchMultipleReq_Rsp_t* pVal = (CAPI2_PbkApi_SendFindAlphaMatchMultipleReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_PBK_ISREADY_RSP:
	{
		CAPI2_PbkApi_IsReady_Rsp_t* pVal = (CAPI2_PbkApi_IsReady_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_WRT_PBK_ENTRY_RSP:
	{
		CAPI2_PbkApi_SendWriteEntryReq_Rsp_t* pVal = (CAPI2_PbkApi_SendWriteEntryReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_CHK_PBK_DIALLED_NUM_RSP:
	{
		CAPI2_PbkApi_SendIsNumDiallableReq_Rsp_t* pVal = (CAPI2_PbkApi_SendIsNumDiallableReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_PBK_ISNUMDIALLABLE_RSP:
	{
		CAPI2_PbkApi_IsNumDiallable_Rsp_t* pVal = (CAPI2_PbkApi_IsNumDiallable_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_PBK_ISNUMBARRED_RSP:
	{
		CAPI2_PbkApi_IsNumBarred_Rsp_t* pVal = (CAPI2_PbkApi_IsNumBarred_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_PBK_ISUSSDDIALLABLE_RSP:
	{
		CAPI2_PbkApi_IsUssdDiallable_Rsp_t* pVal = (CAPI2_PbkApi_IsUssdDiallable_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_PDP_SETPDPCONTEXT_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_PDP_SETSECPDPCONTEXT_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_PDP_GETGPRSQOS_RSP:
	{
		CAPI2_PdpApi_GetGPRSQoS_Rsp_t* pVal = (CAPI2_PdpApi_GetGPRSQoS_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_PDP_SETGPRSQOS_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_PDP_GETGPRSMINQOS_RSP:
	{
		CAPI2_PdpApi_GetGPRSMinQoS_Rsp_t* pVal = (CAPI2_PdpApi_GetGPRSMinQoS_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_PDP_SETGPRSMINQOS_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_MS_SENDCOMBINEDATTACHREQ_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_MS_SENDDETACHREQ_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_MS_GETGPRSATTACHSTATUS_RSP:
	{
		CAPI2_MS_GetGPRSAttachStatus_Rsp_t* pVal = (CAPI2_MS_GetGPRSAttachStatus_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_PDP_ISSECONDARYPDPDEFINED_RSP:
	{
		CAPI2_PdpApi_IsSecondaryPdpDefined_Rsp_t* pVal = (CAPI2_PdpApi_IsSecondaryPdpDefined_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_PDP_ACTIVATION_RSP:
	{
		CAPI2_PchExApi_SendPDPActivateReq_Rsp_t* pVal = (CAPI2_PchExApi_SendPDPActivateReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_PDP_MODIFICATION_RSP:
	{
		CAPI2_PchExApi_SendPDPModifyReq_Rsp_t* pVal = (CAPI2_PchExApi_SendPDPModifyReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_PDP_DEACTIVATION_RSP:
	{
		CAPI2_PchExApi_SendPDPDeactivateReq_Rsp_t* pVal = (CAPI2_PchExApi_SendPDPDeactivateReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_PDP_SEC_ACTIVATION_RSP:
	{
		CAPI2_PchExApi_SendPDPActivateSecReq_Rsp_t* pVal = (CAPI2_PchExApi_SendPDPActivateSecReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_PDP_GETGPRSACTIVATESTATUS_RSP:
	{
		CAPI2_PdpApi_GetGPRSActivateStatus_Rsp_t* pVal = (CAPI2_PdpApi_GetGPRSActivateStatus_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_PDP_SETMSCLASS_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_PDP_GETMSCLASS_RSP:
	{
		CAPI2_PDP_GetMSClass_Rsp_t* pVal = (CAPI2_PDP_GetMSClass_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_PDP_GETUMTSTFT_RSP:
	{
		CAPI2_PdpApi_GetUMTSTft_Rsp_t* pVal = (CAPI2_PdpApi_GetUMTSTft_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_PDP_SETUMTSTFT_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_PDP_DELETEUMTSTFT_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_PDP_DEACTIVATESNDCPCONNECTION_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_PDP_GETR99UMTSMINQOS_RSP:
	{
		CAPI2_PdpApi_GetR99UMTSMinQoS_Rsp_t* pVal = (CAPI2_PdpApi_GetR99UMTSMinQoS_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_PDP_GETR99UMTSQOS_RSP:
	{
		CAPI2_PdpApi_GetR99UMTSQoS_Rsp_t* pVal = (CAPI2_PdpApi_GetR99UMTSQoS_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_PDP_GETUMTSMINQOS_RSP:
	{
		CAPI2_PdpApi_GetUMTSMinQoS_Rsp_t* pVal = (CAPI2_PdpApi_GetUMTSMinQoS_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_PDP_GETUMTSQOS_RSP:
	{
		CAPI2_PdpApi_GetUMTSQoS_Rsp_t* pVal = (CAPI2_PdpApi_GetUMTSQoS_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_PDP_GETNEGQOS_RSP:
	{
		CAPI2_PdpApi_GetNegQoS_Rsp_t* pVal = (CAPI2_PdpApi_GetNegQoS_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_PDP_SETR99UMTSMINQOS_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_PDP_SETR99UMTSQOS_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_PDP_SETUMTSMINQOS_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_PDP_SETUMTSQOS_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_PDP_GETNEGOTIATEDPARMS_RSP:
	{
		CAPI2_PdpApi_GetNegotiatedParms_Rsp_t* pVal = (CAPI2_PdpApi_GetNegotiatedParms_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_MS_ISGPRSCALLACTIVE_RSP:
	{
		CAPI2_MS_IsGprsCallActive_Rsp_t* pVal = (CAPI2_MS_IsGprsCallActive_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_MS_SETCHANGPRSCALLACTIVE_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_MS_SETCIDFORGPRSACTIVECHAN_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_PDP_GETPPPMODEMCID_RSP:
	{
		CAPI2_PdpApi_GetPPPModemCid_Rsp_t* pVal = (CAPI2_PdpApi_GetPPPModemCid_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_MS_GETGPRSACTIVECHANFROMCID_RSP:
	{
		CAPI2_MS_GetGprsActiveChanFromCid_Rsp_t* pVal = (CAPI2_MS_GetGprsActiveChanFromCid_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_MS_GETCIDFROMGPRSACTIVECHAN_RSP:
	{
		CAPI2_MS_GetCidFromGprsActiveChan_Rsp_t* pVal = (CAPI2_MS_GetCidFromGprsActiveChan_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_PDP_GETPDPADDRESS_RSP:
	{
		CAPI2_PdpApi_GetPDPAddress_Rsp_t* pVal = (CAPI2_PdpApi_GetPDPAddress_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_PDP_SENDTBFDATA_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_PDP_TFTADDFILTER_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_PDP_SETPCHCONTEXTSTATE_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_PDP_SETDEFAULTPDPCONTEXT_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_PCHEX_READDECODEDPROTCONFIG_RSP:
	{
		CAPI2_PchExApi_GetDecodedProtConfig_Rsp_t* pVal = (CAPI2_PchExApi_GetDecodedProtConfig_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_PCHEX_BUILDPROTCONFIGOPTIONS_RSP:
	{
		CAPI2_PchExApi_BuildIpConfigOptions_Rsp_t* pVal = (CAPI2_PchExApi_BuildIpConfigOptions_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_PCHEX_BUILDPROTCONFIGOPTIONS2_RSP:
	{
		CAPI2_PchExApi_BuildIpConfigOptions2_Rsp_t* pVal = (CAPI2_PchExApi_BuildIpConfigOptions2_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_PCHEX_BUILDPROTCONFIGOPTION_CHAP_TYPE_RSP:
	{
		CAPI2_PchExApi_BuildIpConfigOptionsWithChapAuthType_Rsp_t* pVal = (CAPI2_PchExApi_BuildIpConfigOptionsWithChapAuthType_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_PDP_GET_DEFAULT_QOS_RSP:
	{
		CAPI2_PdpApi_GetDefaultQos_Rsp_t* pVal = (CAPI2_PdpApi_GetDefaultQos_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_PDP_ISCONTEXT_ACTIVE_RSP:
	{
		CAPI2_PdpApi_IsPDPContextActive_Rsp_t* pVal = (CAPI2_PdpApi_IsPDPContextActive_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_PDP_ACTIVATE_SNDCP_RSP:
	{
		CAPI2_PdpApi_ActivateSNDCPConnection_Rsp_t* pVal = (CAPI2_PdpApi_ActivateSNDCPConnection_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_PDP_GETPDPCONTEXT_RSP:
	{
		CAPI2_PdpApi_GetPDPContext_Rsp_t* pVal = (CAPI2_PdpApi_GetPDPContext_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_PDP_GETPDPCONTEXT_CID_LIST_RSP:
	{
		CAPI2_PdpApi_GetDefinedPDPContextCidList_Rsp_t* pVal = (CAPI2_PdpApi_GetDefinedPDPContextCidList_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SYSPARAM_BOOTLOADER_VER_RSP:
	{
		CAPI2_SYS_GetBootLoaderVersion_Rsp_t* pVal = (CAPI2_SYS_GetBootLoaderVersion_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SYSPARAM_DSF_VER_RSP:
	{
		CAPI2_SYS_GetDSFVersion_Rsp_t* pVal = (CAPI2_SYS_GetDSFVersion_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_USIM_UST_DATA_RSP:
	{
		CAPI2_USimApi_GetUstData_Rsp_t* pVal = (CAPI2_USimApi_GetUstData_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_PATCH_GET_REVISION_RSP:
	{
		CAPI2_PATCH_GetRevision_Rsp_t* pVal = (CAPI2_PATCH_GetRevision_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SS_CALL_FORWARD_STATUS_RSP:
	{
		CAPI2_SS_SendCallForwardReq_Rsp_t* pVal = (CAPI2_SS_SendCallForwardReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SS_CALL_BARRING_STATUS_RSP:
	{
		CAPI2_SS_SendCallBarringReq_Rsp_t* pVal = (CAPI2_SS_SendCallBarringReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SS_CALL_BARRING_PWD_CHANGE_RSP:
	{
		CAPI2_SS_SendCallBarringPWDChangeReq_Rsp_t* pVal = (CAPI2_SS_SendCallBarringPWDChangeReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SS_CALL_WAITING_STATUS_RSP:
	{
		CAPI2_SS_SendCallWaitingReq_Rsp_t* pVal = (CAPI2_SS_SendCallWaitingReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SS_CALLING_LINE_ID_STATUS_RSP:
	{
		CAPI2_SS_QueryCallingLineIDStatus_Rsp_t* pVal = (CAPI2_SS_QueryCallingLineIDStatus_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SS_CONNECTED_LINE_STATUS_RSP:
	{
		CAPI2_SS_QueryConnectedLineIDStatus_Rsp_t* pVal = (CAPI2_SS_QueryConnectedLineIDStatus_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SS_CALLING_LINE_RESTRICTION_STATUS_RSP:
	{
		CAPI2_SS_QueryCallingLineRestrictionStatus_Rsp_t* pVal = (CAPI2_SS_QueryCallingLineRestrictionStatus_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SS_CONNECTED_LINE_RESTRICTION_STATUS_RSP:
	{
		CAPI2_SS_QueryConnectedLineRestrictionStatus_Rsp_t* pVal = (CAPI2_SS_QueryConnectedLineRestrictionStatus_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SS_CALLING_NAME_PRESENT_STATUS_RSP:
	{
		CAPI2_SS_QueryCallingNAmePresentStatus_Rsp_t* pVal = (CAPI2_SS_QueryCallingNAmePresentStatus_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_MS_LOCAL_ELEM_NOTIFY_IND:
	{
		CAPI2_SS_SetCallingLineIDStatus_Rsp_t* pVal = (CAPI2_SS_SetCallingLineIDStatus_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_USSD_DATA_RSP:
	{
		CAPI2_SS_SendUSSDConnectReq_Rsp_t* pVal = (CAPI2_SS_SendUSSDConnectReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SSAPI_DIALSTRSRVREQ_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SS_ENDUSSDCONNECTREQ_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SSAPI_SSSRVREQ_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SSAPI_USSDSRVREQ_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SSAPI_USSDDATAREQ_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SSAPI_SSRELEASEREQ_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SSAPI_DATAREQ_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SSAPI_DISPATCH_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SS_GET_STR_RSP:
	{
		CAPI2_SS_GetStr_Rsp_t* pVal = (CAPI2_SS_GetStr_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SS_SETCLIENTID_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SS_GETCLIENTID_RSP:
	{
		CAPI2_SS_GetClientID_Rsp_t* pVal = (CAPI2_SS_GetClientID_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SS_RESETCLIENTID_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SATK_GETCACHEDROOTMENUPTR_RSP:
	{
		CAPI2_SatkApi_GetCachedRootMenuPtr_Rsp_t* pVal = (CAPI2_SatkApi_GetCachedRootMenuPtr_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SATK_SENDUSERACTIVITYEVENT_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SATK_SENDIDLESCREENAVAIEVENT_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SATK_SENDLANGSELECTEVENT_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SATK_SENDBROWSERTERMEVENT_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SATK_CMDRESP_RSP:
	{
		CAPI2_SatkApi_CmdResp_Rsp_t* pVal = (CAPI2_SatkApi_CmdResp_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SATK_DATASERVCMDRESP_RSP:
	{
		CAPI2_SatkApi_DataServCmdResp_Rsp_t* pVal = (CAPI2_SatkApi_DataServCmdResp_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SATK_SENDDATASERVREQ_RSP:
	{
		CAPI2_SatkApi_SendDataServReq_Rsp_t* pVal = (CAPI2_SatkApi_SendDataServReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SATK_SENDTERMINALRSP_RSP:
	{
		CAPI2_SatkApi_SendTerminalRsp_Rsp_t* pVal = (CAPI2_SatkApi_SendTerminalRsp_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SATK_SETTERMPROFILE_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_STK_ENVELOPE_RSP:
	{
		CAPI2_SatkApi_SendEnvelopeCmdReq_Rsp_t* pVal = (CAPI2_SatkApi_SendEnvelopeCmdReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_STK_TERMINAL_RESPONSE_RSP:
	{
		CAPI2_SatkApi_SendTerminalRspReq_Rsp_t* pVal = (CAPI2_SatkApi_SendTerminalRspReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_STK_SEND_BROWSING_STATUS_EVT_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_STK_CALL_CONTROL_SETUP_RSP:
	{
		CAPI2_SatkApi_Control_Setup_Rsp_Rsp_t* pVal = (CAPI2_SatkApi_Control_Setup_Rsp_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_STK_CALL_CONTROL_SS_RSP:
	{
		CAPI2_SatkApi_Control_SS_Rsp_Rsp_t* pVal = (CAPI2_SatkApi_Control_SS_Rsp_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_STK_CALL_CONTROL_USSD_RSP:
	{
		CAPI2_SatkApi_Control_USSD_Rsp_Rsp_t* pVal = (CAPI2_SatkApi_Control_USSD_Rsp_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SATK_SEND_CC_SS_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SATK_SEND_CC_USSD_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SATK_SEND_CC_SMS_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_STK_CALL_CONTROL_SMS_RSP:
	{
		CAPI2_SatkApi_Control_SMS_Rsp_Rsp_t* pVal = (CAPI2_SatkApi_Control_SMS_Rsp_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_MNSS_CLIENT_LCS_SRV_RSP:
	{
		CAPI2_LCS_CpMoLrReq_Rsp_t* pVal = (CAPI2_LCS_CpMoLrReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_LCS_CPMOLRABORT_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_LCS_CPMTLRVERIFICATIONRSP_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_LCS_CPMTLRRSP_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_LCS_CPLOCUPDATERSP_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_LCS_DECODEPOSESTIMATE_RSP:
	{
		CAPI2_LCS_DecodePosEstimate_Rsp_t* pVal = (CAPI2_LCS_DecodePosEstimate_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_LCS_ENCODEASSISTANCEREQ_RSP:
	{
		CAPI2_LCS_EncodeAssistanceReq_Rsp_t* pVal = (CAPI2_LCS_EncodeAssistanceReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_LCS_FTT_SYNC_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMS_READY_IND:
	{
		CAPI2_smsModuleReady_Rsp_t* pVal = (CAPI2_smsModuleReady_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SMSPP_DELIVER_IND:
	{
		CAPI2_SmsSimMsgDel_Rsp_t* pVal = (CAPI2_SmsSimMsgDel_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SMSPP_OTA_IND:
	{
		CAPI2_SmsSimMsgOta_Rsp_t* pVal = (CAPI2_SmsSimMsgOta_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SMSPP_REGULAR_PUSH_IND:
	{
		CAPI2_SmsSimMsgPush_Rsp_t* pVal = (CAPI2_SmsSimMsgPush_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SMSSR_STORED_IND:
	{
		CAPI2_SmsStoredSmsStatus_Rsp_t* pVal = (CAPI2_SmsStoredSmsStatus_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SMSCB_STORED_IND:
	{
		CAPI2_SmsStoredSmsCb_Rsp_t* pVal = (CAPI2_SmsStoredSmsCb_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SMSPP_STORED_IND:
	{
		CAPI2_SmsStoredSms_Rsp_t* pVal = (CAPI2_SmsStoredSms_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SMSCB_DATA_IND:
	{
		CAPI2_SmsCbData_Rsp_t* pVal = (CAPI2_SmsCbData_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_HOMEZONE_STATUS_IND:
	{
		CAPI2_HomezoneIndData_Rsp_t* pVal = (CAPI2_HomezoneIndData_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SATK_EVENT_DISPLAY_TEXT:
	{
		CAPI2_SATK_EventDataDisp_Rsp_t* pVal = (CAPI2_SATK_EventDataDisp_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SATK_EVENT_GET_INKEY:
	{
		CAPI2_SATK_EventDataGetInKey_Rsp_t* pVal = (CAPI2_SATK_EventDataGetInKey_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SATK_EVENT_GET_INPUT:
	{
		CAPI2_SATK_EventDataGetInput_Rsp_t* pVal = (CAPI2_SATK_EventDataGetInput_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SATK_EVENT_PLAY_TONE:
	{
		CAPI2_SATK_EventDataPlayTone_Rsp_t* pVal = (CAPI2_SATK_EventDataPlayTone_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SATK_EVENT_ACTIVATE:
	{
		CAPI2_SATK_EventDataActivate_Rsp_t* pVal = (CAPI2_SATK_EventDataActivate_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SATK_EVENT_SELECT_ITEM:
	{
		CAPI2_SATK_EventDataSelItem_Rsp_t* pVal = (CAPI2_SATK_EventDataSelItem_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SATK_EVENT_SEND_SS:
	{
		CAPI2_SATK_EventDataSendSS_Rsp_t* pVal = (CAPI2_SATK_EventDataSendSS_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SATK_EVENT_SEND_USSD:
	{
		CAPI2_SATK_EventDataSendUSSD_Rsp_t* pVal = (CAPI2_SATK_EventDataSendUSSD_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SATK_EVENT_SETUP_CALL:
	{
		CAPI2_SATK_EventDataSetupCall_Rsp_t* pVal = (CAPI2_SATK_EventDataSetupCall_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SATK_EVENT_SETUP_MENU:
	{
		CAPI2_SATK_EventDataSetupMenu_Rsp_t* pVal = (CAPI2_SATK_EventDataSetupMenu_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SATK_EVENT_REFRESH:
	{
		CAPI2_SATK_EventDataRefresh_Rsp_t* pVal = (CAPI2_SATK_EventDataRefresh_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_FATAL_ERROR_IND:
	{
		CAPI2_SIM_FatalInd_Rsp_t* pVal = (CAPI2_SIM_FatalInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_SIM_RECOVER_IND:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_STK_CC_SETUPFAIL_IND:
	{
		CAPI2_SATK_CallSetupFail_Rsp_t* pVal = (CAPI2_SATK_CallSetupFail_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_STK_CC_DISPLAY_IND:
	{
		CAPI2_SATK_CallControlDisplay_Rsp_t* pVal = (CAPI2_SATK_CallControlDisplay_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SATK_EVENT_SEND_SHORT_MSG:
	{
		CAPI2_SATK_EventDataShortMsg_Rsp_t* pVal = (CAPI2_SATK_EventDataShortMsg_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SATK_EVENT_SEND_DTMF:
	{
		CAPI2_SATK_EventDataSendDtmf_Rsp_t* pVal = (CAPI2_SATK_EventDataSendDtmf_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SATK_EVENT_LAUNCH_BROWSER:
	{
		CAPI2_SATK_EventData_Rsp_t* pVal = (CAPI2_SATK_EventData_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SATK_EVENT_IDLEMODE_TEXT:
	{
		CAPI2_SATK_EventDataLaunchBrowser_Rsp_t* pVal = (CAPI2_SATK_EventDataLaunchBrowser_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SATK_EVENT_DATA_SERVICE_REQ:
	{
		CAPI2_SATK_EventDataServiceReq_Rsp_t* pVal = (CAPI2_SATK_EventDataServiceReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_STK_LANG_NOTIFICATION_IND:
	{
		CAPI2_StkLangNotificationLangNotify_Rsp_t* pVal = (CAPI2_StkLangNotificationLangNotify_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_STK_LOCAL_BATTERY_STATE_REQ:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SIM_CACHED_DATA_READY_IND:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SIM_MMI_SETUP_EVENT_IND:
	{
		CAPI2_SimMmiSetupInd_Rsp_t* pVal = (CAPI2_SimMmiSetupInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_STK_RUN_AT_IND:
	{
		CAPI2_SATK_EventDataRunATInd_Rsp_t* pVal = (CAPI2_SATK_EventDataRunATInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_STK_RUN_AT_REQ:
	{
		CAPI2_StkRunAtReq_Rsp_t* pVal = (CAPI2_StkRunAtReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SATK_EVENT_STK_SESSION_END:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SATK_EVENT_PROV_LOCAL_LANG:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SATK_EVENT_PROV_LOCAL_DATE:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_STK_SEND_IPAT_REQ:
	{
		CAPI2_StkRunIpAtReq_Rsp_t* pVal = (CAPI2_StkRunIpAtReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_STK_GET_CHANNEL_STATUS_REQ:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_STK_MENU_SELECTION_RES:
	{
		CAPI2_StkMenuSelectionRes_Rsp_t* pVal = (CAPI2_StkMenuSelectionRes_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_STK_TERMINATE_DATA_SERV_REQ:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_VOICECALL_CONNECTED_IND:
	{
		CAPI2_VOICECALL_CONNECTED_IND_Rsp_t* pVal = (CAPI2_VOICECALL_CONNECTED_IND_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_VOICECALL_PRECONNECT_IND:
	{
		CAPI2_VOICECALL_PRECONNECTED_IND_Rsp_t* pVal = (CAPI2_VOICECALL_PRECONNECTED_IND_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SS_CALL_NOTIFICATION:
	{
		CAPI2_SS_CALL_NOTIFICATION_Rsp_t* pVal = (CAPI2_SS_CALL_NOTIFICATION_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_MNCC_CLIENT_NOTIFY_SS_IND:
	{
		CAPI2_MNCC_CLIENT_NOTIFY_SS_IND_Rsp_t* pVal = (CAPI2_MNCC_CLIENT_NOTIFY_SS_IND_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SS_NOTIFY_CLOSED_USER_GROUP:
	{
		CAPI2_SS_NOTIFY_CLOSED_USER_GROUP_Rsp_t* pVal = (CAPI2_SS_NOTIFY_CLOSED_USER_GROUP_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SS_NOTIFY_EXTENDED_CALL_TRANSFER:
	{
		CAPI2_SS_NOTIFY_EXTENDED_CALL_TRANSFER_Rsp_t* pVal = (CAPI2_SS_NOTIFY_EXTENDED_CALL_TRANSFER_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SS_NOTIFY_CALLING_NAME_PRESENT:
	{
		CAPI2_SS_NOTIFY_CALLING_NAME_PRESENT_Rsp_t* pVal = (CAPI2_SS_NOTIFY_CALLING_NAME_PRESENT_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_CALL_STATUS_IND:
	{
		CAPI2_CALL_STATUS_IND_Rsp_t* pVal = (CAPI2_CALL_STATUS_IND_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_VOICECALL_ACTION_RSP:
	{
		CAPI2_VOICECALL_ACTION_RSP_Rsp_t* pVal = (CAPI2_VOICECALL_ACTION_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_VOICECALL_RELEASE_IND:
	{
		CAPI2_VOICECALL_RELEASE_IND_Rsp_t* pVal = (CAPI2_VOICECALL_RELEASE_IND_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_VOICECALL_RELEASE_CNF:
	{
		CAPI2_VOICECALL_RELEASE_CNF_Rsp_t* pVal = (CAPI2_VOICECALL_RELEASE_CNF_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_INCOMING_CALL_IND:
	{
		CAPI2_INCOMING_CALL_IND_Rsp_t* pVal = (CAPI2_INCOMING_CALL_IND_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_VOICECALL_WAITING_IND:
	{
		CAPI2_VOICECALL_WAITING_IND_Rsp_t* pVal = (CAPI2_VOICECALL_WAITING_IND_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_CALL_AOCSTATUS_IND:
	{
		CAPI2_CALL_AOCSTATUS_IND_Rsp_t* pVal = (CAPI2_CALL_AOCSTATUS_IND_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_CALL_CCM_IND:
	{
		CAPI2_CALL_CCM_IND_Rsp_t* pVal = (CAPI2_CALL_CCM_IND_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_CALL_CONNECTEDLINEID_IND:
	{
		CAPI2_CALL_CONNECTEDLINEID_IND_Rsp_t* pVal = (CAPI2_CALL_CONNECTEDLINEID_IND_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_DATACALL_STATUS_IND:
	{
		CAPI2_DATACALL_STATUS_IND_Rsp_t* pVal = (CAPI2_DATACALL_STATUS_IND_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_DATACALL_RELEASE_IND:
	{
		CAPI2_DATACALL_RELEASE_IND_Rsp_t* pVal = (CAPI2_DATACALL_RELEASE_IND_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_DATACALL_ECDC_IND:
	{
		CAPI2_DATACALL_ECDC_IND_Rsp_t* pVal = (CAPI2_DATACALL_ECDC_IND_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_DATACALL_CONNECTED_IND:
	{
		CAPI2_DATACALL_CONNECTED_IND_Rsp_t* pVal = (CAPI2_DATACALL_CONNECTED_IND_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_API_CLIENT_CMD_IND:
	{
		CAPI2_API_CLIENT_CMD_IND_Rsp_t* pVal = (CAPI2_API_CLIENT_CMD_IND_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_DTMF_STATUS_IND:
	{
		CAPI2_DTMF_STATUS_IND_Rsp_t* pVal = (CAPI2_DTMF_STATUS_IND_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_CALL_USER_INFORMATION:
	{
		CAPI2_USER_INFORMATION_Rsp_t* pVal = (CAPI2_USER_INFORMATION_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_CIPHER_IND:
	{
		CAPI2_CcCipherInd_Rsp_t* pVal = (CAPI2_CcCipherInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_GPRS_ACTIVATE_IND:
	{
		CAPI2_GPRS_ACTIVATE_IND_Rsp_t* pVal = (CAPI2_GPRS_ACTIVATE_IND_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_PDP_DEACTIVATION_IND:
	{
		CAPI2_GPRS_DEACTIVATE_IND_Rsp_t* pVal = (CAPI2_GPRS_DEACTIVATE_IND_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_PDP_PPP_SENDCLOSE_IND:
	{
		CAPI2_PDP_DEACTIVATION_IND_Rsp_t* pVal = (CAPI2_PDP_DEACTIVATION_IND_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_GPRS_MODIFY_IND:
	{
		CAPI2_GPRS_MODIFY_IND_Rsp_t* pVal = (CAPI2_GPRS_MODIFY_IND_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_GPRS_REACT_IND:
	{
		CAPI2_GPRS_REACT_IND_Rsp_t* pVal = (CAPI2_GPRS_REACT_IND_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_PDP_ACTIVATION_NWI_IND:
	{
		CAPI2_PDP_ACTIVATION_NWI_IND_Rsp_t* pVal = (CAPI2_PDP_ACTIVATION_NWI_IND_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_DATA_SUSPEND_IND:
	{
		CAPI2_DATA_SUSPEND_IND_Rsp_t* pVal = (CAPI2_DATA_SUSPEND_IND_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_DATA_RESUME_IND:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_MNSS_CLIENT_LCS_SRV_IND:
	{
		CAPI2_MNSS_CLIENT_LCS_SRV_IND_Rsp_t* pVal = (CAPI2_MNSS_CLIENT_LCS_SRV_IND_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_MNSS_CLIENT_LCS_SRV_REL:
	{
		CAPI2_MNSS_CLIENT_LCS_SRV_REL_Rsp_t* pVal = (CAPI2_MNSS_CLIENT_LCS_SRV_REL_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_MNSS_CLIENT_SS_SRV_RSP:
	{
		CAPI2_MNSS_CLIENT_SS_SRV_RSP_Rsp_t* pVal = (CAPI2_MNSS_CLIENT_SS_SRV_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_MNSS_CLIENT_SS_SRV_REL:
	{
		CAPI2_MNSS_CLIENT_SS_SRV_REL_Rsp_t* pVal = (CAPI2_MNSS_CLIENT_SS_SRV_REL_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_MNSS_CLIENT_SS_SRV_IND:
	{
		CAPI2_MNSS_CLIENT_SS_SRV_IND_Rsp_t* pVal = (CAPI2_MNSS_CLIENT_SS_SRV_IND_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_MNSS_SATK_SS_SRV_RSP:
	{
		CAPI2_MNSS_SATK_SS_SRV_RSP_Rsp_t* pVal = (CAPI2_MNSS_SATK_SS_SRV_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_VM_WAITING_IND:
	{
		CAPI2_SmsVoicemailInd_Rsp_t* pVal = (CAPI2_SmsVoicemailInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_SMS_DATA_RSP:
	{
		CAPI2_SIM_SMS_DATA_RSP_Rsp_t* pVal = (CAPI2_SIM_SMS_DATA_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_CC_GETCNAPNAME_RSP:
	{
		CAPI2_CcApi_GetCNAPName_Rsp_t* pVal = (CAPI2_CcApi_GetCNAPName_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SYSPARM_GET_HSUPA_SUPPORTED_RSP:
	{
		CAPI2_SYSPARM_GetHSUPASupported_Rsp_t* pVal = (CAPI2_SYSPARM_GetHSUPASupported_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SYSPARM_GET_HSDPA_SUPPORTED_RSP:
	{
		CAPI2_SYSPARM_GetHSDPASupported_Rsp_t* pVal = (CAPI2_SYSPARM_GetHSDPASupported_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_RADIO_ACTIVITY_IND:
	{
		CAPI2_MSRadioActivityInd_Rsp_t* pVal = (CAPI2_MSRadioActivityInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_MS_FORCE_PS_REL_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_CC_ISCURRENTSTATEMPTY_RSP:
	{
		CAPI2_CcApi_IsCurrentStateMpty_Rsp_t* pVal = (CAPI2_CcApi_IsCurrentStateMpty_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_PDP_GETPCHCONTEXTSTATE_RSP:
	{
		CAPI2_PdpApi_GetPCHContextState_Rsp_t* pVal = (CAPI2_PdpApi_GetPCHContextState_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_PDP_GETPCHCONTEXT_EX_RSP:
	{
		CAPI2_PdpApi_GetPDPContextEx_Rsp_t* pVal = (CAPI2_PdpApi_GetPDPContextEx_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SIM_PIN_LOCK_TYPE_RSP:
	{
		CAPI2_SimApi_GetCurrLockedSimlockType_Rsp_t* pVal = (CAPI2_SimApi_GetCurrLockedSimlockType_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_PLMN_SELECT_CNF:
	{
		CAPI2_PLMN_SELECT_CNF_Rsp_t* pVal = (CAPI2_PLMN_SELECT_CNF_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_SEND_APDU_RSP:
	{
		CAPI2_SimApi_SubmitSelectFileSendApduReq_Rsp_t* pVal = (CAPI2_SimApi_SubmitSelectFileSendApduReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_STK_TERMINAL_PROFILE_IND:
	{
		CAPI2_SatkApi_SendTermProfileInd_Rsp_t* pVal = (CAPI2_SatkApi_SendTermProfileInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_MUL_REC_DATA_RSP:
	{
		CAPI2_SimApi_SubmitMulRecordEFileReq_Rsp_t* pVal = (CAPI2_SimApi_SubmitMulRecordEFileReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_STK_UICC_CAT_IND:
	{
		CAPI2_SatkApi_SendUiccCatInd_Rsp_t* pVal = (CAPI2_SatkApi_SendUiccCatInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_SELECT_APPLICATION_RSP:
	{
		CAPI2_SimApi_SendSelectApplicationReq_Rsp_t* pVal = (CAPI2_SimApi_SendSelectApplicationReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_PEROFRM_STEERING_OF_ROAMING_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_STK_PROACTIVE_CMD_IND:
	{
		CAPI2_SatkApi_SendProactiveCmdInd_Rsp_t* pVal = (CAPI2_SatkApi_SendProactiveCmdInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_STK_PROACTIVE_CMD_FETCHING_ON_OFF_RSP:
	{
		CAPI2_SatkApi_SendProactiveCmdFetchingOnOffReq_Rsp_t* pVal = (CAPI2_SatkApi_SendProactiveCmdFetchingOnOffReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SATK_SEND_EXT_PROACTIVE_CMD_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SATK_SEND_TERMINAL_PROFILE_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_STK_POLLING_INTERVAL_RSP:
	{
		CAPI2_SatkApi_SendPollingIntervalReq_Rsp_t* pVal = (CAPI2_SatkApi_SendPollingIntervalReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_STK_EXT_PROACTIVE_CMD_IND:
	{
		CAPI2_SatkApi_SendExtProactiveCmdInd_Rsp_t* pVal = (CAPI2_SatkApi_SendExtProactiveCmdInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_PDP_SETPDPACTIVATIONCCFLAG_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_PDP_GETPDPACTIVATIONCCFLAG_RSP:
	{
		CAPI2_PdpApi_GetPDPActivationCallControlFlag_Rsp_t* pVal = (CAPI2_PdpApi_GetPDPActivationCallControlFlag_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_PDP_ACTIVATION_PDU_RSP:
	{
		CAPI2_PdpApi_SendPDPActivateReq_PDU_Rsp_t* pVal = (CAPI2_PdpApi_SendPDPActivateReq_PDU_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_PDP_REJECTNWIACTIVATION_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_PDP_SETBEARERCTRLMODE_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_PDP_GETBEARERCTRLMODE_RSP:
	{
		CAPI2_PdpApi_GetPDPBearerCtrlMode_Rsp_t* pVal = (CAPI2_PdpApi_GetPDPBearerCtrlMode_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_PDP_REJECTSECNWIACTIVATION_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_PDP_SETPDPNWICONTROLFLAG_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_PDP_GETPDPNWICONTROLFLAG_RSP:
	{
		CAPI2_PdpApi_GetPDPNWIControlFlag_Rsp_t* pVal = (CAPI2_PdpApi_GetPDPNWIControlFlag_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_PDP_ACTIVATION_SEC_NWI_IND:
	{
		CAPI2_PDP_ACTIVATION_SEC_NWI_IND_Rsp_t* pVal = (CAPI2_PDP_ACTIVATION_SEC_NWI_IND_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_PDP_CHECKUMTSTFT_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_PDP_ISANYPDPCONTEXTACTIVE_RSP:
	{
		CAPI2_PdpApi_IsAnyPDPContextActive_Rsp_t* pVal = (CAPI2_PdpApi_IsAnyPDPContextActive_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_PDP_ISANYPDPCONTEXTPENDING_RSP:
	{
		CAPI2_PdpApi_IsAnyPDPContextActivePending_Rsp_t* pVal = (CAPI2_PdpApi_IsAnyPDPContextActivePending_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_CCAPI_SET_ELEMENT_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_CCAPI_GET_ELEMENT_RSP:
	{
		CAPI2_CcApi_GetElement_Rsp_t* pVal = (CAPI2_CcApi_GetElement_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_WL_PS_SET_FILTER_LIST_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_PDP_GETPROTCONFIGOPTIONS_RSP:
	{
		CAPI2_PdpApi_GetProtConfigOptions_Rsp_t* pVal = (CAPI2_PdpApi_GetProtConfigOptions_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_PDP_SETPROTCONFIGOPTIONS_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_LCS_RRLP_SEND_DATA_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_LCS_RRLP_REG_HDL_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_LCS_RRC_REG_HDL_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_LCS_RRC_SEND_DL_DCCH_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_LCS_RRC_MEAS_CTRL_FAILURE_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_LCS_RRC_STAT_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SIM_POWER_ON_OFF_SIM_RSP:
	{
		CAPI2_SimApi_PowerOnOffSim_Rsp_t* pVal = (CAPI2_SimApi_PowerOnOffSim_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SYS_SET_PAGING_STATUS_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_LCS_GET_GPS_CAP_RSP:
	{
		CAPI2_LcsApi_GetGpsCapabilities_Rsp_t* pVal = (CAPI2_LcsApi_GetGpsCapabilities_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_LCS_SET_GPS_CAP_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_CCAPI_ABORTDTMF_TONE_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_MS_SET_RAT_BAND_EX_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_SMSSR_REPORT_IND:
	{
		CAPI2_SmsReportInd_Rsp_t* pVal = (CAPI2_SmsReportInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	default:
		break;
	}
	return;
}
