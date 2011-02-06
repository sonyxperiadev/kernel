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
#include "capi2_gen_msg.h"
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
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_SYS_POWERUP_NORF_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_SYS_POWERUP_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_SYS_POWERON_CAUSE_RSP:
	{
		CAPI2_SYS_GetMSPowerOnCause_Rsp_t* pVal = (CAPI2_SYS_GetMSPowerOnCause_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
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
	case MSG_MS_START_BAND_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_SIM_UPDATE_SMSCAPEXC_FLAG_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_MS_SELECT_BAND_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_MS_SET_RAT_BAND_RSP:
	{
		*ppBuf = NULL; *len=0;
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
		*ppBuf = (void*)&(pVal->val);
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
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_MS_PLMN_SELECT_RSP:
	{
		CAPI2_MS_PlmnSelect_Rsp_t* pVal = (CAPI2_MS_PlmnSelect_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_MS_PLMN_ABORT_RSP:
	{
		*ppBuf = NULL; *len=0;
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
		*ppBuf = NULL; *len=0;
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
		*ppBuf = NULL; *len=0;
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
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_MS_AUTO_SEARCH_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_MS_PLMN_NAME_RSP:
	{
		CAPI2_MS_GetPLMNNameByCode_Rsp_t* pVal = (CAPI2_MS_GetPLMNNameByCode_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SYS_QUERY_RESET_CAUSE_RSP:
	{
		CAPI2_SYS_IsResetCausedByAssert_Rsp_t* pVal = (CAPI2_SYS_IsResetCausedByAssert_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SYS_GET_SYSTEM_STATE_RSP:
	{
		CAPI2_SYS_GetSystemState_Rsp_t* pVal = (CAPI2_SYS_GetSystemState_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SYS_SET_SYSTEM_STATE_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_SYS_GET_RX_LEVEL_RSP:
	{
		CAPI2_SYS_GetRxSignalInfo_Rsp_t* pVal = (CAPI2_SYS_GetRxSignalInfo_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
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
	case MSG_SYS_GET_GSMREG_CAUSE_RSP:
	{
		CAPI2_SYS_GetGSMRegistrationCause_Rsp_t* pVal = (CAPI2_SYS_GetGSMRegistrationCause_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SYS_IS_PLMN_FORBIDDEN_RSP:
	{
		CAPI2_MS_IsPlmnForbidden_Rsp_t* pVal = (CAPI2_MS_IsPlmnForbidden_Rsp_t*)dataBuf;
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
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_SIM_LOCK_ON_RSP:
	{
		CAPI2_SIMLockIsLockOn_Rsp_t* pVal = (CAPI2_SIMLockIsLockOn_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_LOCK_STATUS_RSP:
	{
		CAPI2_SIMLockCheckAllLocks_Rsp_t* pVal = (CAPI2_SIMLockCheckAllLocks_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_LOCK_UNLOCK_RSP:
	{
		CAPI2_SIMLockUnlockSIM_Rsp_t* pVal = (CAPI2_SIMLockUnlockSIM_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_LOCK_SET_LOCK_RSP:
	{
		CAPI2_SIMLockSetLock_Rsp_t* pVal = (CAPI2_SIMLockSetLock_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_LOCK_CLOSED_LOCK_RSP:
	{
		CAPI2_SIMLockGetCurrentClosedLock_Rsp_t* pVal = (CAPI2_SIMLockGetCurrentClosedLock_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_LOCK_CHANGE_PWD_RSP:
	{
		CAPI2_SIMLockChangePasswordPHSIM_Rsp_t* pVal = (CAPI2_SIMLockChangePasswordPHSIM_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_LOCK_CHECK_PWD_PHSIM_RSP:
	{
		CAPI2_SIMLockCheckPasswordPHSIM_Rsp_t* pVal = (CAPI2_SIMLockCheckPasswordPHSIM_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_LOCK_LOCK_SIG_RSP:
	{
		CAPI2_SIMLockGetSignature_Rsp_t* pVal = (CAPI2_SIMLockGetSignature_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_LOCK_GET_IMEI_RSP:
	{
		CAPI2_SIMLockGetImeiSecboot_Rsp_t* pVal = (CAPI2_SIMLockGetImeiSecboot_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_PARAM_REC_NUM_RSP:
	{
		CAPI2_SIM_GetSmsParamRecNum_Rsp_t* pVal = (CAPI2_SIM_GetSmsParamRecNum_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_MS_CONVERT_PLMN_STRING_RSP:
	{
		CAPI2_MS_ConvertPLMNNameStr_Rsp_t* pVal = (CAPI2_MS_ConvertPLMNNameStr_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_GET_SMSMEMEXC_FLAG_RSP:
	{
		CAPI2_SIM_GetSmsMemExceededFlag_Rsp_t* pVal = (CAPI2_SIM_GetSmsMemExceededFlag_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_PIN_REQ_RSP:
	{
		CAPI2_SIM_IsPINRequired_Rsp_t* pVal = (CAPI2_SIM_IsPINRequired_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_CARD_PHASE_RSP:
	{
		CAPI2_SIM_GetCardPhase_Rsp_t* pVal = (CAPI2_SIM_GetCardPhase_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_TYPE_RSP:
	{
		CAPI2_SIM_GetSIMType_Rsp_t* pVal = (CAPI2_SIM_GetSIMType_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_PRESENT_RSP:
	{
		CAPI2_SIM_GetPresentStatus_Rsp_t* pVal = (CAPI2_SIM_GetPresentStatus_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_PIN_OPERATION_RSP:
	{
		CAPI2_SIM_IsOperationRestricted_Rsp_t* pVal = (CAPI2_SIM_IsOperationRestricted_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_PIN_BLOCK_RSP:
	{
		CAPI2_SIM_IsPINBlocked_Rsp_t* pVal = (CAPI2_SIM_IsPINBlocked_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_PUK_BLOCK_RSP:
	{
		CAPI2_SIM_IsPUKBlocked_Rsp_t* pVal = (CAPI2_SIM_IsPUKBlocked_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_IS_INVALID_RSP:
	{
		CAPI2_SIM_IsInvalidSIM_Rsp_t* pVal = (CAPI2_SIM_IsInvalidSIM_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_DETECT_RSP:
	{
		CAPI2_SIM_DetectSim_Rsp_t* pVal = (CAPI2_SIM_DetectSim_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_GET_RUIM_SUPP_FLAG_RSP:
	{
		CAPI2_SIM_GetRuimSuppFlag_Rsp_t* pVal = (CAPI2_SIM_GetRuimSuppFlag_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_VERIFY_CHV_RSP:
	{
		CAPI2_SIM_SendVerifyChvReq_Rsp_t* pVal = (CAPI2_SIM_SendVerifyChvReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_CHANGE_CHV_RSP:
	{
		CAPI2_SIM_SendChangeChvReq_Rsp_t* pVal = (CAPI2_SIM_SendChangeChvReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_ENABLE_CHV_RSP:
	{
		CAPI2_SIM_SendSetChv1OnOffReq_Rsp_t* pVal = (CAPI2_SIM_SendSetChv1OnOffReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_UNBLOCK_CHV_RSP:
	{
		CAPI2_SIM_SendUnblockChvReq_Rsp_t* pVal = (CAPI2_SIM_SendUnblockChvReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_SET_FDN_RSP:
	{
		CAPI2_SIM_SendSetOperStateReq_Rsp_t* pVal = (CAPI2_SIM_SendSetOperStateReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_IS_PBK_ALLOWED_RSP:
	{
		CAPI2_SIM_IsPbkAccessAllowed_Rsp_t* pVal = (CAPI2_SIM_IsPbkAccessAllowed_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_PBK_INFO_RSP:
	{
		CAPI2_SIM_SendPbkInfoReq_Rsp_t* pVal = (CAPI2_SIM_SendPbkInfoReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_MAX_ACM_RSP:
	{
		CAPI2_SIM_SendReadAcmMaxReq_Rsp_t* pVal = (CAPI2_SIM_SendReadAcmMaxReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_ACM_MAX_UPDATE_RSP:
	{
		CAPI2_SIM_SendWriteAcmMaxReq_Rsp_t* pVal = (CAPI2_SIM_SendWriteAcmMaxReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_ACM_VALUE_RSP:
	{
		CAPI2_SIM_SendReadAcmReq_Rsp_t* pVal = (CAPI2_SIM_SendReadAcmReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_ACM_UPDATE_RSP:
	{
		CAPI2_SIM_SendWriteAcmReq_Rsp_t* pVal = (CAPI2_SIM_SendWriteAcmReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_ACM_INCREASE_RSP:
	{
		CAPI2_SIM_SendIncreaseAcmReq_Rsp_t* pVal = (CAPI2_SIM_SendIncreaseAcmReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_SVC_PROV_NAME_RSP:
	{
		CAPI2_SIM_SendReadSvcProvNameReq_Rsp_t* pVal = (CAPI2_SIM_SendReadSvcProvNameReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_PUCT_DATA_RSP:
	{
		CAPI2_SIM_SendReadPuctReq_Rsp_t* pVal = (CAPI2_SIM_SendReadPuctReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_SERVICE_STATUS_RSP:
	{
		CAPI2_SIM_GetServiceStatus_Rsp_t* pVal = (CAPI2_SIM_GetServiceStatus_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_GENERIC_ACCESS_END_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_SIM_PIN_STATUS_RSP:
	{
		CAPI2_SIM_GetPinStatus_Rsp_t* pVal = (CAPI2_SIM_GetPinStatus_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_PIN_OK_STATUS_RSP:
	{
		CAPI2_SIM_IsPinOK_Rsp_t* pVal = (CAPI2_SIM_IsPinOK_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_IMSI_RSP:
	{
		CAPI2_SIM_GetIMSI_Rsp_t* pVal = (CAPI2_SIM_GetIMSI_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_GID_DIGIT_RSP:
	{
		CAPI2_SIM_GetGID1_Rsp_t* pVal = (CAPI2_SIM_GetGID1_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_GID_DIGIT2_RSP:
	{
		CAPI2_SIM_GetGID2_Rsp_t* pVal = (CAPI2_SIM_GetGID2_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_HOME_PLMN_RSP:
	{
		CAPI2_SIM_GetHomePlmn_Rsp_t* pVal = (CAPI2_SIM_GetHomePlmn_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_PIN_LOCK_TYPE_RSP:
	{
		CAPI2_SIM_GetCurrLockedSimlockType_Rsp_t* pVal = (CAPI2_SIM_GetCurrLockedSimlockType_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_APDU_FILEID_RSP:
	{
		CAPI2_simmi_GetMasterFileId_Rsp_t* pVal = (CAPI2_simmi_GetMasterFileId_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_OPEN_SOCKET_RSP:
	{
		CAPI2_SIM_SendOpenSocketReq_Rsp_t* pVal = (CAPI2_SIM_SendOpenSocketReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_SELECT_APPLI_RSP:
	{
		CAPI2_SIM_SendSelectAppiReq_Rsp_t* pVal = (CAPI2_SIM_SendSelectAppiReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_DEACTIVATE_APPLI_RSP:
	{
		CAPI2_SIM_SendDeactivateAppiReq_Rsp_t* pVal = (CAPI2_SIM_SendDeactivateAppiReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_CLOSE_SOCKET_RSP:
	{
		CAPI2_SIM_SendCloseSocketReq_Rsp_t* pVal = (CAPI2_SIM_SendCloseSocketReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_ATR_DATA_RSP:
	{
		CAPI2_SIM_GetAtrData_Rsp_t* pVal = (CAPI2_SIM_GetAtrData_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_DFILE_INFO_RSP:
	{
		CAPI2_SIM_SubmitDFileInfoReq_Rsp_t* pVal = (CAPI2_SIM_SubmitDFileInfoReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_EFILE_INFO_RSP:
	{
		CAPI2_SIM_SubmitEFileInfoReq_Rsp_t* pVal = (CAPI2_SIM_SubmitEFileInfoReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_EFILE_DATA_RSP:
	{
		CAPI2_SIM_SubmitBinaryEFileReadReq_Rsp_t* pVal = (CAPI2_SIM_SubmitBinaryEFileReadReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_EFILE_UPDATE_RSP:
	{
		CAPI2_SIM_SubmitBinaryEFileUpdateReq_Rsp_t* pVal = (CAPI2_SIM_SubmitBinaryEFileUpdateReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_SEEK_RECORD_RSP:
	{
		CAPI2_SIM_SubmitSeekRecordReq_Rsp_t* pVal = (CAPI2_SIM_SubmitSeekRecordReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_PIN_ATTEMPT_RSP:
	{
		CAPI2_SIM_SendRemainingPinAttemptReq_Rsp_t* pVal = (CAPI2_SIM_SendRemainingPinAttemptReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_CACHE_DATA_READY_RSP:
	{
		CAPI2_SIM_IsCachedDataReady_Rsp_t* pVal = (CAPI2_SIM_IsCachedDataReady_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_SERVICE_CODE_STATUS_RSP:
	{
		CAPI2_SIM_GetServiceCodeStatus_Rsp_t* pVal = (CAPI2_SIM_GetServiceCodeStatus_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_CHECK_CPHS_RSP:
	{
		CAPI2_SIM_CheckCphsService_Rsp_t* pVal = (CAPI2_SIM_CheckCphsService_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_CPHS_PHASE_RSP:
	{
		CAPI2_SIM_GetCphsPhase_Rsp_t* pVal = (CAPI2_SIM_GetCphsPhase_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_SMS_SCA_RSP:
	{
		CAPI2_SIM_GetSmsSca_Rsp_t* pVal = (CAPI2_SIM_GetSmsSca_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_ICCID_PARAM_RSP:
	{
		CAPI2_SIM_GetIccid_Rsp_t* pVal = (CAPI2_SIM_GetIccid_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_ALS_STATUS_RSP:
	{
		CAPI2_SIM_IsALSEnabled_Rsp_t* pVal = (CAPI2_SIM_IsALSEnabled_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_ALS_DEFAULT_LINE_RSP:
	{
		CAPI2_SIM_GetAlsDefaultLine_Rsp_t* pVal = (CAPI2_SIM_GetAlsDefaultLine_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_SET_ALS_DEFAULT_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_SIM_CALLFWD_COND_FLAG_RSP:
	{
		CAPI2_SIM_GetCallForwardUnconditionalFlag_Rsp_t* pVal = (CAPI2_SIM_GetCallForwardUnconditionalFlag_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_APP_TYPE_RSP:
	{
		CAPI2_SIM_GetApplicationType_Rsp_t* pVal = (CAPI2_SIM_GetApplicationType_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_USIM_UST_DATA_RSP:
	{
		CAPI2_USIM_GetUst_Rsp_t* pVal = (CAPI2_USIM_GetUst_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_PLMN_WRITE_RSP:
	{
		CAPI2_SIM_SendUpdatePrefListReq_Rsp_t* pVal = (CAPI2_SIM_SendUpdatePrefListReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_PUCT_UPDATE_RSP:
	{
		CAPI2_SIM_SendWritePuctReq_Rsp_t* pVal = (CAPI2_SIM_SendWritePuctReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_GENERIC_ACCESS_RSP:
	{
		CAPI2_SIM_SendGenericAccessReq_Rsp_t* pVal = (CAPI2_SIM_SendGenericAccessReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_RESTRICTED_ACCESS_RSP:
	{
		CAPI2_SIM_SubmitRestrictedAccessReq_Rsp_t* pVal = (CAPI2_SIM_SubmitRestrictedAccessReq_Rsp_t*)dataBuf;
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
	case MSG_ADC_START_RSP:
	{
		CAPI2_ADCMGR_Start_Rsp_t* pVal = (CAPI2_ADCMGR_Start_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_CAPI2_AT_COMMAND_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_CAPI2_AT_RESPONSE_IND:
	{
		CAPI2_AT_Response_Rsp_t* pVal = (CAPI2_AT_Response_Rsp_t*)dataBuf;
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
	case MSG_AUDIO_GET_MIC_GAIN_RSP:
	{
		CAPI2_AUDIO_GetMicrophoneGainSetting_Rsp_t* pVal = (CAPI2_AUDIO_GetMicrophoneGainSetting_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_AUDIO_GET_SPEAKER_VOL_RSP:
	{
		CAPI2_AUDIO_GetSpeakerVol_Rsp_t* pVal = (CAPI2_AUDIO_GetSpeakerVol_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_AUDIO_SET_SPEAKER_VOL_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_AUDIO_SET_MIC_GAIN_RSP:
	{
		*ppBuf = NULL; *len=0;
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
	case MSG_SYSPARAM_GET_IMEI_STR_RSP:
	{
		CAPI2_UTIL_ExtractImei_Rsp_t* pVal = (CAPI2_UTIL_ExtractImei_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_MS_GET_REG_INFO_RSP:
	{
		CAPI2_MS_GetRegistrationInfo_Rsp_t* pVal = (CAPI2_MS_GetRegistrationInfo_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_PLMN_NUM_OF_ENTRY_RSP:
	{
		CAPI2_SIM_SendNumOfPLMNEntryReq_Rsp_t* pVal = (CAPI2_SIM_SendNumOfPLMNEntryReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_PLMN_ENTRY_DATA_RSP:
	{
		CAPI2_SIM_SendReadPLMNEntryReq_Rsp_t* pVal = (CAPI2_SIM_SendReadPLMNEntryReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_PLMN_ENTRY_UPDATE_RSP:
	{
		CAPI2_SIM_SendWritePLMNEntryReq_Rsp_t* pVal = (CAPI2_SIM_SendWritePLMNEntryReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_MUL_PLMN_ENTRY_UPDATE_RSP:
	{
		CAPI2_SIM_SendWriteMulPLMNEntryReq_Rsp_t* pVal = (CAPI2_SIM_SendWriteMulPLMNEntryReq_Rsp_t*)dataBuf;
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
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_SYSPARAM_BOOTLOADER_VER_RSP:
	{
		CAPI2_SYS_GetBootLoaderVersion_Rsp_t* pVal = (CAPI2_SYS_GetBootLoaderVersion_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SYSPARAM_DSF_VER_RSP:
	{
		CAPI2_SYS_GetDSFVersion_Rsp_t* pVal = (CAPI2_SYS_GetDSFVersion_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
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
	case MSG_SYSPARAM_GET_IMEI_RSP:
	{
		CAPI2_SYSPARM_GetIMEI_Rsp_t* pVal = (CAPI2_SYSPARM_GetIMEI_Rsp_t*)dataBuf;
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
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_SYSPARAM_SET_EGPRS_MSCLASS_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_SYSPARAM_SET_GPRS_MSCLASS_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_SYSPARAM_SET_POWERON_CAUSE_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_TIMEZONE_DELETE_NW_NAME_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_TIMEZONE_GET_UPDATE_MODE_RSP:
	{
		CAPI2_TIMEZONE_GetTZUpdateMode_Rsp_t* pVal = (CAPI2_TIMEZONE_GetTZUpdateMode_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_TIMEZONE_SET_UPDATE_MODE_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_TIMEZONE_UPDATE_RTC_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_PMU_IS_SIM_READY_RSP:
	{
		CAPI2_PMU_IsSIMReady_RSP_Rsp_t* pVal = (CAPI2_PMU_IsSIMReady_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_PMU_ACTIVATE_SIM_RSP:
	{
		CAPI2_PMU_ActivateSIM_RSP_Rsp_t* pVal = (CAPI2_PMU_ActivateSIM_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_PMU_DEACTIVATE_SIM_RSP:
	{
		CAPI2_PMU_DeactivateSIM_RSP_Rsp_t* pVal = (CAPI2_PMU_DeactivateSIM_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_CAPI2_TEST_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_STK_SEND_PLAYTONE_RES_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_STK_SETUP_CALL_RES_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_PBK_SET_FDN_CHECK_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_PBK_GET_FDN_CHECK_RSP:
	{
		CAPI2_PBK_GetFdnCheck_Rsp_t* pVal = (CAPI2_PBK_GetFdnCheck_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_GPIO_SET_HIGH_64PIN_RSP:
	{
		CAPI2_GPIO_Set_High_64Pin_RSP_Rsp_t* pVal = (CAPI2_GPIO_Set_High_64Pin_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_GPIO_SET_LOW_64PIN_RSP:
	{
		CAPI2_GPIO_Set_Low_64Pin_RSP_Rsp_t* pVal = (CAPI2_GPIO_Set_Low_64Pin_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_PMU_START_CHARGING_RSP:
	{
		CAPI2_PMU_StartCharging_RSP_Rsp_t* pVal = (CAPI2_PMU_StartCharging_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_PMU_STOP_CHARGING_RSP:
	{
		CAPI2_PMU_StopCharging_RSP_Rsp_t* pVal = (CAPI2_PMU_StopCharging_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_PMU_BATT_LEVEL_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_PMU_BATT_LEVEL_REGISTER_RSP:
	{
		CAPI2_PMU_Battery_Register_Rsp_t* pVal = (CAPI2_PMU_Battery_Register_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_PMU_BATT_LEVEL_IND:
	{
		CAPI2_BattLevelInd_Rsp_t* pVal = (CAPI2_BattLevelInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SMS_SEND_MEM_AVAL_IND_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_SMS_ISMESTORAGEENABLED_RSP:
	{
		CAPI2_SMS_IsMeStorageEnabled_RSP_Rsp_t* pVal = (CAPI2_SMS_IsMeStorageEnabled_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SMS_GETMAXMECAPACITY_RSP:
	{
		CAPI2_SMS_GetMaxMeCapacity_RSP_Rsp_t* pVal = (CAPI2_SMS_GetMaxMeCapacity_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SMS_GETNEXTFREESLOT_RSP:
	{
		CAPI2_SMS_GetNextFreeSlot_RSP_Rsp_t* pVal = (CAPI2_SMS_GetNextFreeSlot_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SMS_SETMESMSSTATUS_RSP:
	{
		CAPI2_SMS_SetMeSmsStatus_RSP_Rsp_t* pVal = (CAPI2_SMS_SetMeSmsStatus_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SMS_GETMESMSSTATUS_RSP:
	{
		CAPI2_SMS_GetMeSmsStatus_RSP_Rsp_t* pVal = (CAPI2_SMS_GetMeSmsStatus_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SMS_STORESMSTOME_RSP:
	{
		CAPI2_SMS_StoreSmsToMe_RSP_Rsp_t* pVal = (CAPI2_SMS_StoreSmsToMe_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SMS_RETRIEVESMSFROMME_RSP:
	{
		CAPI2_SMS_RetrieveSmsFromMe_RSP_Rsp_t* pVal = (CAPI2_SMS_RetrieveSmsFromMe_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SMS_REMOVESMSFROMME_RSP:
	{
		CAPI2_SMS_RemoveSmsFromMe_RSP_Rsp_t* pVal = (CAPI2_SMS_RemoveSmsFromMe_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SMS_CONFIGUREMESTORAGE_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_MS_SET_ELEMENT_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_MS_GET_ELEMENT_RSP:
	{
		CAPI2_MS_GetElement_Rsp_t* pVal = (CAPI2_MS_GetElement_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_USIM_IS_APP_SUPPORTED_RSP:
	{
		CAPI2_USIM_IsApplicationSupported_Rsp_t* pVal = (CAPI2_USIM_IsApplicationSupported_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_USIM_IS_APN_ALLOWED_RSP:
	{
		CAPI2_USIM_IsAllowedAPN_Rsp_t* pVal = (CAPI2_USIM_IsAllowedAPN_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_USIM_GET_NUM_APN_RSP:
	{
		CAPI2_USIM_GetNumOfAPN_Rsp_t* pVal = (CAPI2_USIM_GetNumOfAPN_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_USIM_GET_APN_ENTRY_RSP:
	{
		CAPI2_USIM_GetAPNEntry_Rsp_t* pVal = (CAPI2_USIM_GetAPNEntry_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_USIM_IS_EST_SERV_ACTIVATED_RSP:
	{
		CAPI2_USIM_IsEstServActivated_Rsp_t* pVal = (CAPI2_USIM_IsEstServActivated_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_SET_EST_SERV_RSP:
	{
		CAPI2_USIM_SendSetEstServReq_Rsp_t* pVal = (CAPI2_USIM_SendSetEstServReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_UPDATE_ONE_APN_RSP:
	{
		CAPI2_USIM_SendWriteAPNReq_Rsp_t* pVal = (CAPI2_USIM_SendWriteAPNReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_DELETE_ALL_APN_RSP:
	{
		CAPI2_USIM_SendDeleteAllAPNReq_Rsp_t* pVal = (CAPI2_USIM_SendDeleteAllAPNReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_USIM_GET_RAT_MODE_RSP:
	{
		CAPI2_USIM_GetRatModeSetting_Rsp_t* pVal = (CAPI2_USIM_GetRatModeSetting_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_PMU_CLIENT_POWER_DOWN_RSP:
	{
		CAPI2_PMU_ClientPowerDown_RSP_Rsp_t* pVal = (CAPI2_PMU_ClientPowerDown_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_PMU_GET_POWERUP_CAUSE_RSP:
	{
		CAPI2_PMU_GetPowerupCause_RSP_Rsp_t* pVal = (CAPI2_PMU_GetPowerupCause_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
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
	case MSG_MS_GET_START_BAND_RSP:
	{
		CAPI2_MS_GetStartBand_Rsp_t* pVal = (CAPI2_MS_GetStartBand_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_MS_SETMEPOWER_CLASS_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_USIM_GET_SERVICE_STATUS_RSP:
	{
		CAPI2_USIM_GetServiceStatus_Rsp_t* pVal = (CAPI2_USIM_GetServiceStatus_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_IS_ALLOWED_APN_RSP:
	{
		CAPI2_SIM_IsAllowedAPN_Rsp_t* pVal = (CAPI2_SIM_IsAllowedAPN_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SMS_GETSMSMAXCAPACITY_RSP:
	{
		CAPI2_SMS_GetSmsMaxCapacity_Rsp_t* pVal = (CAPI2_SMS_GetSmsMaxCapacity_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SMS_RETRIEVEMAXCBCHNLLENGTH_RSP:
	{
		CAPI2_SMS_RetrieveMaxCBChnlLength_Rsp_t* pVal = (CAPI2_SMS_RetrieveMaxCBChnlLength_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_IS_BDN_RESTRICTED_RSP:
	{
		CAPI2_SIM_IsBdnOperationRestricted_Rsp_t* pVal = (CAPI2_SIM_IsBdnOperationRestricted_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_SEND_PLMN_UPDATE_IND_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_SIM_DEACTIVATE_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_SIM_SET_BDN_RSP:
	{
		CAPI2_SIM_SendSetBdnReq_Rsp_t* pVal = (CAPI2_SIM_SendSetBdnReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_POWER_ON_OFF_CARD_RSP:
	{
		CAPI2_SIM_PowerOnOffCard_Rsp_t* pVal = (CAPI2_SIM_PowerOnOffCard_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_GET_RAW_ATR_RSP:
	{
		CAPI2_SIM_GetRawAtr_Rsp_t* pVal = (CAPI2_SIM_GetRawAtr_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_SET_PROTOCOL_RSP:
	{
		CAPI2_SIM_Set_Protocol_Rsp_t* pVal = (CAPI2_SIM_Set_Protocol_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_GET_PROTOCOL_RSP:
	{
		CAPI2_SIM_Get_Protocol_Rsp_t* pVal = (CAPI2_SIM_Get_Protocol_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_SEND_GENERIC_APDU_CMD_RSP:
	{
		CAPI2_SIM_SendGenericApduCmd_Rsp_t* pVal = (CAPI2_SIM_SendGenericApduCmd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SIM_TERMINATE_XFER_APDU_RSP:
	{
		CAPI2_SIM_TerminateXferApdu_Rsp_t* pVal = (CAPI2_SIM_TerminateXferApdu_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SET_PLMN_SELECT_RAT_RSP:
	{
		*ppBuf = NULL; *len=0;
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
	case MSG_SOCKET_OPEN_RSP:
	{
		CAPI2_SOCKET_Open_RSP_Rsp_t* pVal = (CAPI2_SOCKET_Open_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SOCKET_BIND_RSP:
	{
		CAPI2_SOCKET_Bind_RSP_Rsp_t* pVal = (CAPI2_SOCKET_Bind_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SOCKET_LISTEN_RSP:
	{
		CAPI2_SOCKET_Listen_RSP_Rsp_t* pVal = (CAPI2_SOCKET_Listen_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SOCKET_ACCEPT_RSP:
	{
		CAPI2_SOCKET_Accept_RSP_Rsp_t* pVal = (CAPI2_SOCKET_Accept_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SOCKET_CONNECT_RSP:
	{
		CAPI2_SOCKET_Connect_RSP_Rsp_t* pVal = (CAPI2_SOCKET_Connect_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SOCKET_GETPEERNAME_RSP:
	{
		CAPI2_SOCKET_GetPeerName_RSP_Rsp_t* pVal = (CAPI2_SOCKET_GetPeerName_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SOCKET_GETSOCKNAME_RSP:
	{
		CAPI2_SOCKET_GetSockName_RSP_Rsp_t* pVal = (CAPI2_SOCKET_GetSockName_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SOCKET_SETSOCKOPT_RSP:
	{
		CAPI2_SOCKET_SetSockOpt_RSP_Rsp_t* pVal = (CAPI2_SOCKET_SetSockOpt_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SOCKET_GETSOCKOPT_RSP:
	{
		CAPI2_SOCKET_GetSockOpt_RSP_Rsp_t* pVal = (CAPI2_SOCKET_GetSockOpt_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_CAPI2_SOCKET_SIGNAL_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_SOCKET_SEND_RSP:
	{
		CAPI2_SOCKET_Send_RSP_Rsp_t* pVal = (CAPI2_SOCKET_Send_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SOCKET_SEND_TO_RSP:
	{
		CAPI2_SOCKET_SendTo_RSP_Rsp_t* pVal = (CAPI2_SOCKET_SendTo_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SOCKET_RECV_RSP:
	{
		CAPI2_SOCKET_Recv_RSP_Rsp_t* pVal = (CAPI2_SOCKET_Recv_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SOCKET_RECV_FROM_RSP:
	{
		CAPI2_SOCKET_RecvFrom_RSP_Rsp_t* pVal = (CAPI2_SOCKET_RecvFrom_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SOCKET_CLOSE_RSP:
	{
		CAPI2_SOCKET_Close_RSP_Rsp_t* pVal = (CAPI2_SOCKET_Close_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SOCKET_SHUTDOWN_RSP:
	{
		CAPI2_SOCKET_Shutdown_RSP_Rsp_t* pVal = (CAPI2_SOCKET_Shutdown_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SOCKET_ERRNO_RSP:
	{
		CAPI2_SOCKET_Errno_RSP_Rsp_t* pVal = (CAPI2_SOCKET_Errno_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SOCKET_SO2LONG_RSP:
	{
		CAPI2_SOCKET_SO2LONG_RSP_Rsp_t* pVal = (CAPI2_SOCKET_SO2LONG_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SOCKET_GET_SOCKET_SEND_BUFFER_SPACE_RSP:
	{
		CAPI2_SOCKET_GetSocketSendBufferSpace_RSP_Rsp_t* pVal = (CAPI2_SOCKET_GetSocketSendBufferSpace_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SOCKET_PARSE_IPAD_RSP:
	{
		CAPI2_SOCKET_ParseIPAddr_RSP_Rsp_t* pVal = (CAPI2_SOCKET_ParseIPAddr_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SETUP_DATA_CONNECTION_RSP:
	{
		CAPI2_DC_SetupDataConnection_RSP_Rsp_t* pVal = (CAPI2_DC_SetupDataConnection_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SETUP_DATA_CONNECTION_EX_RSP:
	{
		CAPI2_DC_SetupDataConnectionEx_RSP_Rsp_t* pVal = (CAPI2_DC_SetupDataConnectionEx_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DC_REPORT_CALL_STATUS_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_DC_SHUTDOWN_DATA_CONNECTION_RSP:
	{
		CAPI2_DC_ShutdownDataConnection_RSP_Rsp_t* pVal = (CAPI2_DC_ShutdownDataConnection_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_IS_ACCT_ID_VALID_RSP:
	{
		CAPI2_DATA_IsAcctIDValid_RSP_Rsp_t* pVal = (CAPI2_DATA_IsAcctIDValid_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_CREATE_GPRS_ACCT_RSP:
	{
		CAPI2_DATA_CreateGPRSDataAcct_RSP_Rsp_t* pVal = (CAPI2_DATA_CreateGPRSDataAcct_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_CREATE_GSM_ACCT_RSP:
	{
		CAPI2_DATA_CreateCSDDataAcct_RSP_Rsp_t* pVal = (CAPI2_DATA_CreateCSDDataAcct_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_DELETE_ACCT_RSP:
	{
		CAPI2_DATA_DeleteDataAcct_RSP_Rsp_t* pVal = (CAPI2_DATA_DeleteDataAcct_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_SET_USERNAME_RSP:
	{
		CAPI2_DATA_SetUsername_RSP_Rsp_t* pVal = (CAPI2_DATA_SetUsername_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_GET_USERNAME_RSP:
	{
		CAPI2_DATA_GetUsername_RSP_Rsp_t* pVal = (CAPI2_DATA_GetUsername_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_SET_PASSWORD_RSP:
	{
		CAPI2_DATA_SetPassword_RSP_Rsp_t* pVal = (CAPI2_DATA_SetPassword_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_GET_PASSWORD_RSP:
	{
		CAPI2_DATA_GetPassword_RSP_Rsp_t* pVal = (CAPI2_DATA_GetPassword_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_SET_STATIC_IP_RSP:
	{
		CAPI2_DATA_SetStaticIPAddr_RSP_Rsp_t* pVal = (CAPI2_DATA_SetStaticIPAddr_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_GET_STATIC_IP_RSP:
	{
		CAPI2_DATA_GetStaticIPAddr_RSP_Rsp_t* pVal = (CAPI2_DATA_GetStaticIPAddr_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_SET_PRIMARY_DNS_ADDR_RSP:
	{
		CAPI2_DATA_SetPrimaryDnsAddr_RSP_Rsp_t* pVal = (CAPI2_DATA_SetPrimaryDnsAddr_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_GET_PRIMARY_DNS_ADDR_RSP:
	{
		CAPI2_DATA_GetPrimaryDnsAddr_RSP_Rsp_t* pVal = (CAPI2_DATA_GetPrimaryDnsAddr_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_SET_SECOND_DNS_ADDR_RSP:
	{
		CAPI2_DATA_SetSecondDnsAddr_RSP_Rsp_t* pVal = (CAPI2_DATA_SetSecondDnsAddr_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_GET_SECOND_DNS_ADDR_RSP:
	{
		CAPI2_DATA_GetSecondDnsAddr_RSP_Rsp_t* pVal = (CAPI2_DATA_GetSecondDnsAddr_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_SET_DATA_COMPRESSION_RSP:
	{
		CAPI2_DATA_SetDataCompression_RSP_Rsp_t* pVal = (CAPI2_DATA_SetDataCompression_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_GET_DATA_COMPRESSION_RSP:
	{
		CAPI2_DATA_GetDataCompression_RSP_Rsp_t* pVal = (CAPI2_DATA_GetDataCompression_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_GET_ACCT_TYPE_RSP:
	{
		CAPI2_DATA_GetAcctType_RSP_Rsp_t* pVal = (CAPI2_DATA_GetAcctType_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_GET_EMPTY_ACCT_SLOT_RSP:
	{
		CAPI2_DATA_GetEmptyAcctSlot_RSP_Rsp_t* pVal = (CAPI2_DATA_GetEmptyAcctSlot_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_GET_CID_FROM_ACCTID_RSP:
	{
		CAPI2_DATA_GetCidFromDataAcctID_RSP_Rsp_t* pVal = (CAPI2_DATA_GetCidFromDataAcctID_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_CAPI2_DATA_GET_ACCTID_FROM_CID_RSP:
	{
		CAPI2_DATA_GetDataAcctIDFromCid_RSP_Rsp_t* pVal = (CAPI2_DATA_GetDataAcctIDFromCid_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_GET_PRI_FROM_ACCTID_RSP:
	{
		CAPI2_DATA_GetPrimaryCidFromDataAcctID_RSP_Rsp_t* pVal = (CAPI2_DATA_GetPrimaryCidFromDataAcctID_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_IS_SND_DATA_ACCT_RSP:
	{
		CAPI2_DATA_IsSecondaryDataAcct_RSP_Rsp_t* pVal = (CAPI2_DATA_IsSecondaryDataAcct_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_GET_ACCTID_FROM_CID_RSP:
	{
		CAPI2_DATA_GetDataSentSize_RSP_Rsp_t* pVal = (CAPI2_DATA_GetDataSentSize_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_GET_DATA_RCV_SIZE_RSP:
	{
		CAPI2_DATA_GetDataRcvSize_RSP_Rsp_t* pVal = (CAPI2_DATA_GetDataRcvSize_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_SET_GPRS_PDP_TYPE_RSP:
	{
		CAPI2_DATA_SetGPRSPdpType_RSP_Rsp_t* pVal = (CAPI2_DATA_SetGPRSPdpType_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_GET_GPRS_PDP_TYPE_RSP:
	{
		CAPI2_DATA_GetGPRSPdpType_RSP_Rsp_t* pVal = (CAPI2_DATA_GetGPRSPdpType_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_SET_GPRS_APN_RSP:
	{
		CAPI2_DATA_SetGPRSApn_RSP_Rsp_t* pVal = (CAPI2_DATA_SetGPRSApn_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_GET_GPRS_APN_RSP:
	{
		CAPI2_DATA_GetGPRSApn_RSP_Rsp_t* pVal = (CAPI2_DATA_GetGPRSApn_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_SET_AUTHEN_METHOD_RSP:
	{
		CAPI2_DATA_SetAuthenMethod_RSP_Rsp_t* pVal = (CAPI2_DATA_SetAuthenMethod_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_GET_AUTHEN_METHOD_RSP:
	{
		CAPI2_DATA_GetAuthenMethod_RSP_Rsp_t* pVal = (CAPI2_DATA_GetAuthenMethod_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_SET_GPRS_HEADER_COMPRESSION_RSP:
	{
		CAPI2_DATA_SetGPRSHeaderCompression_RSP_Rsp_t* pVal = (CAPI2_DATA_SetGPRSHeaderCompression_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_GET_GPRS_HEADER_COMPRESSION_RSP:
	{
		CAPI2_DATA_GetGPRSHeaderCompression_RSP_Rsp_t* pVal = (CAPI2_DATA_GetGPRSHeaderCompression_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_SET_GPRS_QOS_RSP:
	{
		CAPI2_DATA_SetGPRSQos_RSP_Rsp_t* pVal = (CAPI2_DATA_SetGPRSQos_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_CAPI2_DATA_GET_GPRS_QOS_RSP:
	{
		CAPI2_DATA_GetGPRSQos_RSP_Rsp_t* pVal = (CAPI2_DATA_GetGPRSQos_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_SET_ACCT_LOCK_RSP:
	{
		CAPI2_DATA_SetAcctLock_RSP_Rsp_t* pVal = (CAPI2_DATA_SetAcctLock_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_GET_ACCT_LOCK_RSP:
	{
		CAPI2_DATA_GetAcctLock_RSP_Rsp_t* pVal = (CAPI2_DATA_GetAcctLock_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_SET_GPRS_ONLY_RSP:
	{
		CAPI2_DATA_SetGprsOnly_RSP_Rsp_t* pVal = (CAPI2_DATA_SetGprsOnly_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_GET_GPRS_ONLY_RSP:
	{
		CAPI2_DATA_GetGprsOnly_RSP_Rsp_t* pVal = (CAPI2_DATA_GetGprsOnly_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_SET_GPRS_TFT_RSP:
	{
		CAPI2_DATA_SetGPRSTft_RSP_Rsp_t* pVal = (CAPI2_DATA_SetGPRSTft_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_GET_GPRS_TFT_RSP:
	{
		CAPI2_DATA_GetGPRSTft_RSP_Rsp_t* pVal = (CAPI2_DATA_GetGPRSTft_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_SET_CSD_DIAL_NUMBER_RSP:
	{
		CAPI2_DATA_SetCSDDialNumber_RSP_Rsp_t* pVal = (CAPI2_DATA_SetCSDDialNumber_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_GET_CSD_DIAL_NUMBER_RSP:
	{
		CAPI2_DATA_GetCSDDialNumber_RSP_Rsp_t* pVal = (CAPI2_DATA_GetCSDDialNumber_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_SET_CSD_DIAL_TYPE_RSP:
	{
		CAPI2_DATA_SetCSDDialType_RSP_Rsp_t* pVal = (CAPI2_DATA_SetCSDDialType_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_GET_CSD_DIAL_TYPE_RSP:
	{
		CAPI2_DATA_GetCSDDialType_RSP_Rsp_t* pVal = (CAPI2_DATA_GetCSDDialType_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_SET_CSD_BAUD_RATE_RSP:
	{
		CAPI2_DATA_SetCSDBaudRate_RSP_Rsp_t* pVal = (CAPI2_DATA_SetCSDBaudRate_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_GET_CSD_BAUD_RATE_RSP:
	{
		CAPI2_DATA_GetCSDBaudRate_RSP_Rsp_t* pVal = (CAPI2_DATA_GetCSDBaudRate_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_SET_CSD_SYNC_TYPE_RSP:
	{
		CAPI2_DATA_SetCSDSyncType_RSP_Rsp_t* pVal = (CAPI2_DATA_SetCSDSyncType_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_GET_CSD_SYNC_TYPE_RSP:
	{
		CAPI2_DATA_GetCSDSyncType_RSP_Rsp_t* pVal = (CAPI2_DATA_GetCSDSyncType_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_SET_CSD_ERROR_CORRECTION_RSP:
	{
		CAPI2_DATA_SetCSDErrorCorrection_RSP_Rsp_t* pVal = (CAPI2_DATA_SetCSDErrorCorrection_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_GET_CSD_ERROR_CORRECTION_RSP:
	{
		CAPI2_DATA_GetCSDErrorCorrection_RSP_Rsp_t* pVal = (CAPI2_DATA_GetCSDErrorCorrection_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_SET_CSD_ERR_CORRECTION_TYPE_RSP:
	{
		CAPI2_DATA_SetCSDErrCorrectionType_RSP_Rsp_t* pVal = (CAPI2_DATA_SetCSDErrCorrectionType_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_GET_CSD_ERR_CORRECTION_TYPE_RSP:
	{
		CAPI2_DATA_GetCSDErrCorrectionType_RSP_Rsp_t* pVal = (CAPI2_DATA_GetCSDErrCorrectionType_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_SET_CSD_DATA_COMP_TYPE_RSP:
	{
		CAPI2_DATA_SetCSDDataCompType_RSP_Rsp_t* pVal = (CAPI2_DATA_SetCSDDataCompType_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_GET_CSD_DATA_COMP_TYPE_RSP:
	{
		CAPI2_DATA_GetCSDDataCompType_RSP_Rsp_t* pVal = (CAPI2_DATA_GetCSDDataCompType_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_SET_CSD_CONN_ELEMENT_RSP:
	{
		CAPI2_DATA_SetCSDConnElement_RSP_Rsp_t* pVal = (CAPI2_DATA_SetCSDConnElement_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_GET_CSD_CONN_ELEMENT_RSP:
	{
		CAPI2_DATA_GetCSDConnElement_RSP_Rsp_t* pVal = (CAPI2_DATA_GetCSDConnElement_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_UPDATE_ACCT_TO_FILE_SYSTEM_RSP:
	{
		CAPI2_DATA_UpdateAccountToFileSystem_RSP_Rsp_t* pVal = (CAPI2_DATA_UpdateAccountToFileSystem_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_RESET_DATA_SIZE_RSP:
	{
		CAPI2_resetDataSize_RSP_Rsp_t* pVal = (CAPI2_resetDataSize_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_ADD_DATA_SENT_SIZE_BY_CID_RSP:
	{
		CAPI2_addDataSentSizebyCid_RSP_Rsp_t* pVal = (CAPI2_addDataSentSizebyCid_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DATA_ADD_DATA_RCV_SIZE_BY_CID_RSP:
	{
		CAPI2_addDataRcvSizebyCid_RSP_Rsp_t* pVal = (CAPI2_addDataRcvSizebyCid_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_READ_USIM_PBK_HDK_RSP:
	{
		CAPI2_PBK_SendUsimHdkReadReq_Rsp_t* pVal = (CAPI2_PBK_SendUsimHdkReadReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_WRITE_USIM_PBK_HDK_RSP:
	{
		CAPI2_PBK_SendUsimHdkUpdateReq_Rsp_t* pVal = (CAPI2_PBK_SendUsimHdkUpdateReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_READ_USIM_PBK_ALPHA_AAS_RSP:
	{
		CAPI2_PBK_SendUsimAasReadReq_Rsp_t* pVal = (CAPI2_PBK_SendUsimAasReadReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_WRITE_USIM_PBK_ALPHA_AAS_RSP:
	{
		CAPI2_PBK_SendUsimAasUpdateReq_Rsp_t* pVal = (CAPI2_PBK_SendUsimAasUpdateReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_GET_USIM_PBK_ALPHA_INFO_AAS_RSP:
	{
		CAPI2_PBK_SendUsimAasInfoReq_Rsp_t* pVal = (CAPI2_PBK_SendUsimAasInfoReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_LCS_REQ_RESULT_IND:
	{
		CAPI2_LCS_PosReqResultInd_Rsp_t* pVal = (CAPI2_LCS_PosReqResultInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_LCS_POSITION_INFO_IND:
	{
		CAPI2_LCS_PosInfoInd_Rsp_t* pVal = (CAPI2_LCS_PosInfoInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_LCS_POSITION_DETAIL_IND:
	{
		CAPI2_LCS_PositionDetailInd_Rsp_t* pVal = (CAPI2_LCS_PositionDetailInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_LCS_NMEA_READY_IND:
	{
		CAPI2_LCS_NmeaReadyInd_Rsp_t* pVal = (CAPI2_LCS_NmeaReadyInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_LCS_SERVICE_CONTROL_RSP:
	{
		CAPI2_LCS_ServiceControl_Rsp_t* pVal = (CAPI2_LCS_ServiceControl_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_LCS_POWER_CONFIG_RSP:
	{
		CAPI2_LCS_PowerConfig_Rsp_t* pVal = (CAPI2_LCS_PowerConfig_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_LCS_SERVICE_QUERY_RSP:
	{
		CAPI2_LCS_ServiceQuery_Rsp_t* pVal = (CAPI2_LCS_ServiceQuery_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_LCS_REGISTER_SUPL_MSG_HANDLER_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_LCS_START_POS_REQ_PERIODIC_RSP:
	{
		CAPI2_LCS_StartPosReqPeriodic_Rsp_t* pVal = (CAPI2_LCS_StartPosReqPeriodic_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_LCS_START_POS_REQ_SINGLE_RSP:
	{
		CAPI2_LCS_StartPosReqSingle_Rsp_t* pVal = (CAPI2_LCS_StartPosReqSingle_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_LCS_STOP_POS_RSP:
	{
		CAPI2_LCS_StopPosReq_Rsp_t* pVal = (CAPI2_LCS_StopPosReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_LCS_GET_POSITION_RSP:
	{
		CAPI2_LCS_GetPosition_Rsp_t* pVal = (CAPI2_LCS_GetPosition_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_LCS_CONFIG_SET_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_LCS_CONFIG_GET_RSP:
	{
		CAPI2_LCS_ConfigGet_Rsp_t* pVal = (CAPI2_LCS_ConfigGet_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_LCS_SUPL_VERIFICATION_RSP_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_LCS_SUPL_CONNECT_RSP_RSP:
	{
		CAPI2_LCS_SuplConnectRsp_Rsp_t* pVal = (CAPI2_LCS_SuplConnectRsp_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_LCS_SUPL_DATA_AVAILABLE_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_LCS_SUPL_DISCONNECTED_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_LCS_SUPL_INIT_HMAC_RSP_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_LCS_SUPL_INIT_HMAC_REQ_IND:
	{
		CAPI2_LCS_SuplInitHmacReq_Rsp_t* pVal = (CAPI2_LCS_SuplInitHmacReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_LCS_SUPL_CONNECT_REQ_IND:
	{
		CAPI2_LCS_SuplConnectReq_Rsp_t* pVal = (CAPI2_LCS_SuplConnectReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_LCS_SUPL_DISCONNECT_REQ_IND:
	{
		CAPI2_LCS_SuplDisconnectReq_Rsp_t* pVal = (CAPI2_LCS_SuplDisconnectReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_LCS_SUPL_NOTIFICATION_IND:
	{
		CAPI2_LCS_SuplNotificationInd_Rsp_t* pVal = (CAPI2_LCS_SuplNotificationInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_LCS_SUPL_WRITE_REQ_IND:
	{
		CAPI2_LCS_SuplWriteReq_Rsp_t* pVal = (CAPI2_LCS_SuplWriteReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_LCS_CMD_DATA_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_GPIO_CONFIG_OUTPUT_64PIN_RSP:
	{
		CAPI2_GPIO_ConfigOutput_64Pin_RSP_Rsp_t* pVal = (CAPI2_GPIO_ConfigOutput_64Pin_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_GPS_CONTROL_RSP:
	{
		CAPI2_GPS_Control_RSP_Rsp_t* pVal = (CAPI2_GPS_Control_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_FFS_CONTROL_RSP:
	{
		CAPI2_FFS_Control_RSP_Rsp_t* pVal = (CAPI2_FFS_Control_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_CP2AP_PEDESTALMODE_CONTROL_RSP:
	{
		CAPI2_CP2AP_PedestalMode_Control_RSP_Rsp_t* pVal = (CAPI2_CP2AP_PedestalMode_Control_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_AUDIO_ASIC_SetAudioMode_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_SPEAKER_StartTone_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_SPEAKER_StartGenericTone_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_SPEAKER_StopTone_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_AUDIO_Turn_EC_NS_OnOff_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_ECHO_SetDigitalTxGain_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_RIPCMDQ_Connect_Uplink_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_RIPCMDQ_Connect_Downlink_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_VOLUMECTRL_SetBasebandVolume_RSP:
	{
		*ppBuf = NULL; *len=0;
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
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_PMU_BATT_CHARGING_NOTIFICATION_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_PATCH_GET_REVISION_RSP:
	{
		CAPI2_PATCH_GetRevision_Rsp_t* pVal = (CAPI2_PATCH_GetRevision_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_RTC_SetTime_RSP:
	{
		CAPI2_RTC_SetTime_RSP_Rsp_t* pVal = (CAPI2_RTC_SetTime_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_RTC_SetDST_RSP:
	{
		CAPI2_RTC_SetDST_RSP_Rsp_t* pVal = (CAPI2_RTC_SetDST_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_RTC_SetTimeZone_RSP:
	{
		CAPI2_RTC_SetTimeZone_RSP_Rsp_t* pVal = (CAPI2_RTC_SetTimeZone_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_MS_INITCALLCFG_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_MS_INITFAXCFG_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_MS_INITVIDEOCALLCFG_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_MS_INITCALLCFGAMPF_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_RTC_GetTime_RSP:
	{
		CAPI2_RTC_GetTime_RSP_Rsp_t* pVal = (CAPI2_RTC_GetTime_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_RTC_GetTimeZone_RSP:
	{
		CAPI2_RTC_GetTimeZone_RSP_Rsp_t* pVal = (CAPI2_RTC_GetTimeZone_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_RTC_GetDST_RSP:
	{
		CAPI2_RTC_GetDST_RSP_Rsp_t* pVal = (CAPI2_RTC_GetDST_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
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
		CAPI2_SMS_GetRecordNumberOfReplaceSMS_RSP_Rsp_t* pVal = (CAPI2_SMS_GetRecordNumberOfReplaceSMS_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_POWER_DOWN_CNF:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_PMU_BATT_FULL_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_FLASH_SAVEIMAGE_RSP:
	{
		CAPI2_FLASH_SaveImage_RSP_Rsp_t* pVal = (CAPI2_FLASH_SaveImage_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_AUDIO_GET_SETTINGS_RSP:
	{
		CAPI2_AUDIO_GetSettings_Rsp_t* pVal = (CAPI2_AUDIO_GetSettings_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIMLOCK_GET_STATUS_RSP:
	{
		CAPI2_SIMLOCK_GetStatus_RSP_Rsp_t* pVal = (CAPI2_SIMLOCK_GetStatus_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_SIMLOCK_SET_STATUS_RSP:
	{
		*ppBuf = NULL; *len=0;
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
	case MSG_PROG_EQU_TYPE_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_PROG_POLY_EQU_TYPE_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_PROG_FIR_IIR_FILTER_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_PROG_POLY_FIR_IIR_FILTER_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_AUDIO_CTRL_GENERIC_RSP:
	{
		CAPI2_audio_control_generic_Rsp_t* pVal = (CAPI2_audio_control_generic_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_AUDIO_CTRL_DSP_RSP:
	{
		CAPI2_audio_control_dsp_Rsp_t* pVal = (CAPI2_audio_control_dsp_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_UE_3G_STATUS_IND:
	{
		CAPI2_MS_Ue3gStatusInd_Rsp_t* pVal = (CAPI2_MS_Ue3gStatusInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_CELL_INFO_IND:
	{
		CAPI2_MS_CellInfoInd_Rsp_t* pVal = (CAPI2_MS_CellInfoInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_FFS_READ_RSP:
	{
		CAPI2_FFS_Read_RSP_Rsp_t* pVal = (CAPI2_FFS_Read_RSP_Rsp_t*)dataBuf;
		*ppBuf = (void*)pVal;
		break;
	}
	case MSG_DIAG_CELLLOCK_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_DIAG_CELLLOCK_STATUS_RSP:
	{
		CAPI2_DIAG_ApiCellLockStatus_Rsp_t* pVal = (CAPI2_DIAG_ApiCellLockStatus_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_LCS_REG_RRLP_HDL_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_LCS_SEND_RRLP_DATA_RSP:
	{
		*ppBuf = NULL; *len=0;
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
	case MSG_LCS_REG_RRC_HDL_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_LCS_RRC_MEAS_REPORT_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_LCS_RRC_MEAS_FAILURE_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_LCS_RRC_STATUS_RSP:
	{
		*ppBuf = NULL; *len=0;
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
		CAPI2_CC_IsThereEmergencyCall_Rsp_t* pVal = (CAPI2_CC_IsThereEmergencyCall_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SYSPARM_GET_LOW_THRESHOLD_VOLT_RSP:
	{
		CAPI2_SYSPARM_GetBattLowThresh_Rsp_t* pVal = (CAPI2_SYSPARM_GetBattLowThresh_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SYSPARM_GET_DEFAULT_4P2_VOLT_RSP:
	{
		CAPI2_SYSPARM_GetDefault4p2VoltReading_Rsp_t* pVal = (CAPI2_SYSPARM_GetDefault4p2VoltReading_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_L1_BB_ISLOCKED_RSP:
	{
		CAPI2_L1_bb_isLocked_Rsp_t* pVal = (CAPI2_L1_bb_isLocked_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SERVING_CELL_INFO_IND:
	{
		CAPI2_ServingCellInfoInd_Rsp_t* pVal = (CAPI2_ServingCellInfoInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_MS_FORCE_PS_REL_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_SMSPP_APP_SPECIFIC_SMS_IND:
	{
		CAPI2_SMSPP_AppSpecificInd_Rsp_t* pVal = (CAPI2_SMSPP_AppSpecificInd_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SYS_POWERUP_NORF_CHG_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_SYSPARM_GET_GPIO_RSP:
	{
		CAPI2_SYSPARM_GetGPIO_Value_Rsp_t* pVal = (CAPI2_SYSPARM_GetGPIO_Value_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SYS_ENABLE_CELL_INFO_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_SYS_SET_HSDPA_CATEGORY_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_SYS_GET_HSDPA_CATEGORY_RSP:
	{
		CAPI2_SYSPARM_GetHSDPAPHYCategory_Rsp_t* pVal = (CAPI2_SYSPARM_GetHSDPAPHYCategory_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SIM_START_MULTI_SMS_XFR_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_SIM_STOP_MULTI_SMS_XFR_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_SIM_SMS_WRITE_RSP:
	{
		CAPI2_SIM_SendWriteSmsReq_Rsp_t* pVal = (CAPI2_SIM_SendWriteSmsReq_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_BATTMGR_GETCHARGINGSTATUS_RSP:
	{
		CAPI2_BATTMGR_GetChargingStatus_Rsp_t* pVal = (CAPI2_BATTMGR_GetChargingStatus_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_BATTMGR_GETPERCENTAGELEVEL_RSP:
	{
		CAPI2_BATTMGR_GetPercentageLevel_Rsp_t* pVal = (CAPI2_BATTMGR_GetPercentageLevel_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_BATTMGR_ISBATTERYPRESENT_RSP:
	{
		CAPI2_BATTMGR_IsBatteryPresent_Rsp_t* pVal = (CAPI2_BATTMGR_IsBatteryPresent_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_BATTMGR_ISCHARGERPLUGIN_RSP:
	{
		CAPI2_BATTMGR_IsChargerPlugIn_Rsp_t* pVal = (CAPI2_BATTMGR_IsChargerPlugIn_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SYSPARM_GETBATT_TABLE_RSP:
	{
		CAPI2_SYSPARM_GetBattTable_Rsp_t* pVal = (CAPI2_SYSPARM_GetBattTable_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_BATTMGR_GET_LEVEL_RSP:
	{
		CAPI2_BATTMGR_GetLevel_Rsp_t* pVal = (CAPI2_BATTMGR_GetLevel_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SMS_WRITESMSPDUTO_SIMRECORD_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_ADC_MULTI_CH_START_RSP:
	{
		CAPI2_ADCMGR_MultiChStart_Rsp_t* pVal = (CAPI2_ADCMGR_MultiChStart_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_INTERTASK_MSG_TO_CP_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	case MSG_INTERTASK_MSG_TO_AP_RSP:
	{
		*ppBuf = NULL; *len=0;
		break;
	}
	default:
		break;
	}
	return;
}
