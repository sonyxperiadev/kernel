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


	MSG_GEN_REQ_START = 0x4000,

	 /** 
	capi2 api is CAPI2_MS_IsGSMRegistered 
	**/
	MSG_MS_GSM_REGISTERED_REQ  = 0x4001,
	 /** 
	payload is ::Boolean 
	**/
	MSG_MS_GSM_REGISTERED_RSP  = 0x4002,	///<Payload type {::Boolean}
	 /** 
	capi2 api is CAPI2_MS_IsGPRSRegistered 
	**/
	MSG_MS_GPRS_REGISTERED_REQ  = 0x4003,
	 /** 
	payload is ::Boolean 
	**/
	MSG_MS_GPRS_REGISTERED_RSP  = 0x4004,	///<Payload type {::Boolean}
	 /** 
	capi2 api is CAPI2_MS_GetGSMRegCause 
	**/
	MSG_MS_GSM_CAUSE_REQ  = 0x4005,
	 /** 
	payload is ::NetworkCause_t 
	**/
	MSG_MS_GSM_CAUSE_RSP  = 0x4006,	///<Payload type {::NetworkCause_t}
	 /** 
	capi2 api is CAPI2_MS_GetGPRSRegCause 
	**/
	MSG_MS_GPRS_CAUSE_REQ  = 0x4007,
	 /** 
	payload is ::NetworkCause_t 
	**/
	MSG_MS_GPRS_CAUSE_RSP  = 0x4008,	///<Payload type {::NetworkCause_t}
	 /** 
	capi2 api is CAPI2_MS_GetRegisteredLAC 
	**/
	MSG_MS_REGISTERED_LAC_REQ  = 0x4009,
	 /** 
	payload is ::UInt16 
	**/
	MSG_MS_REGISTERED_LAC_RSP  = 0x400A,	///<Payload type {::UInt16}
	 /** 
	capi2 api is CAPI2_MS_GetPlmnMCC 
	**/
	MSG_MS_GET_PLMN_MCC_REQ  = 0x400B,
	 /** 
	payload is ::UInt16 
	**/
	MSG_MS_GET_PLMN_MCC_RSP  = 0x400C,	///<Payload type {::UInt16}
	 /** 
	capi2 api is CAPI2_MS_GetPlmnMNC 
	**/
	MSG_MS_GET_PLMN_MNC_REQ  = 0x400D,
	 /** 
	payload is ::UInt8 
	**/
	MSG_MS_GET_PLMN_MNC_RSP  = 0x400E,	///<Payload type {::UInt8}
	 /** 
	capi2 api is CAPI2_SYS_ProcessPowerDownReq 
	**/
	MSG_SYS_POWERDOWN_REQ  = 0x400F,
	 /** 
	payload is ::default_proc 
	**/
	MSG_SYS_POWERDOWN_RSP  = 0x4010,
	 /** 
	capi2 api is CAPI2_SYS_ProcessNoRfReq 
	**/
	MSG_SYS_POWERUP_NORF_REQ  = 0x4011,
	 /** 
	payload is ::default_proc 
	**/
	MSG_SYS_POWERUP_NORF_RSP  = 0x4012,
	 /** 
	capi2 api is CAPI2_SYS_ProcessPowerUpReq 
	**/
	MSG_SYS_POWERUP_REQ  = 0x4013,
	 /** 
	payload is ::default_proc 
	**/
	MSG_SYS_POWERUP_RSP  = 0x4014,
	 /** 
	capi2 api is CAPI2_SYS_GetMSPowerOnCause 
	**/
	MSG_SYS_POWERON_CAUSE_REQ  = 0x4015,
	 /** 
	payload is ::PowerOnCause_t 
	**/
	MSG_SYS_POWERON_CAUSE_RSP  = 0x4016,	///<Payload type {::PowerOnCause_t}
	 /** 
	capi2 api is CAPI2_MS_IsGprsAllowed 
	**/
	MSG_MS_GPRS_ALLOWED_REQ  = 0x4017,
	 /** 
	payload is ::Boolean 
	**/
	MSG_MS_GPRS_ALLOWED_RSP  = 0x4018,	///<Payload type {::Boolean}
	 /** 
	capi2 api is CAPI2_MS_GetCurrentRAT 
	**/
	MSG_MS_GET_CURRENT_RAT_REQ  = 0x4019,
	 /** 
	payload is ::UInt8 
	**/
	MSG_MS_GET_CURRENT_RAT_RSP  = 0x401A,	///<Payload type {::UInt8}
	 /** 
	capi2 api is CAPI2_MS_GetCurrentBand 
	**/
	MSG_MS_GET_CURRENT_BAND_REQ  = 0x401B,
	 /** 
	payload is ::UInt8 
	**/
	MSG_MS_GET_CURRENT_BAND_RSP  = 0x401C,	///<Payload type {::UInt8}
	 /** 
	capi2 api is CAPI2_MS_SetStartBand 
	**/
	MSG_MS_START_BAND_REQ  = 0x401D,	///<Payload type {::CAPI2_MS_SetStartBand_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_MS_START_BAND_RSP  = 0x401E,
	 /** 
	capi2 api is CAPI2_SIM_UpdateSMSCapExceededFlag 
	**/
	MSG_SIM_UPDATE_SMSCAPEXC_FLAG_REQ  = 0x401F,	///<Payload type {::CAPI2_SIM_UpdateSMSCapExceededFlag_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SIM_UPDATE_SMSCAPEXC_FLAG_RSP  = 0x4020,
	 /** 
	capi2 api is CAPI2_SYS_SelectBand 
	**/
	MSG_MS_SELECT_BAND_REQ  = 0x4021,	///<Payload type {::CAPI2_SYS_SelectBand_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_MS_SELECT_BAND_RSP  = 0x4022,
	 /** 
	capi2 api is CAPI2_MS_SetSupportedRATandBand 
	**/
	MSG_MS_SET_RAT_BAND_REQ  = 0x4023,	///<Payload type {::CAPI2_MS_SetSupportedRATandBand_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_MS_SET_RAT_BAND_RSP  = 0x4024,
	 /** 
	capi2 api is CAPI2_PLMN_GetCountryByMcc 
	**/
	MSG_MS_GET_MCC_COUNTRY_REQ  = 0x4025,	///<Payload type {::CAPI2_PLMN_GetCountryByMcc_Req_t}
	 /** 
	payload is ::uchar_ptr_t 
	**/
	MSG_MS_GET_MCC_COUNTRY_RSP  = 0x4026,	///<Payload type {::uchar_ptr_t}
	 /** 
	capi2 api is CAPI2_MS_GetPLMNEntryByIndex 
	**/
	MSG_MS_PLMN_INFO_BY_CODE_REQ  = 0x4027,	///<Payload type {::CAPI2_MS_GetPLMNEntryByIndex_Req_t}
	 /** 
	payload is ::MsPlmnInfo_t 
	**/
	MSG_MS_PLMN_INFO_BY_CODE_RSP  = 0x4028,	///<Payload type {::MsPlmnInfo_t}
	 /** 
	capi2 api is CAPI2_MS_GetPLMNListSize 
	**/
	MSG_MS_PLMN_LIST_SIZE_REQ  = 0x4029,
	 /** 
	payload is ::UInt16 
	**/
	MSG_MS_PLMN_LIST_SIZE_RSP  = 0x402A,	///<Payload type {::UInt16}
	 /** 
	capi2 api is CAPI2_MS_GetPLMNByCode 
	**/
	MSG_MS_PLMN_INFO_BY_INDEX_REQ  = 0x402B,	///<Payload type {::CAPI2_MS_GetPLMNByCode_Req_t}
	 /** 
	payload is ::MsPlmnInfo_t 
	**/
	MSG_MS_PLMN_INFO_BY_INDEX_RSP  = 0x402C,	///<Payload type {::MsPlmnInfo_t}
	 /** 
	capi2 api is CAPI2_MS_PlmnSelect 
	**/
	MSG_MS_PLMN_SELECT_REQ  = 0x402D,	///<Payload type {::CAPI2_MS_PlmnSelect_Req_t}
	 /** 
	capi2 api is CAPI2_MS_AbortPlmnSelect 
	**/
	MSG_MS_PLMN_ABORT_REQ  = 0x402E,
	 /** 
	payload is ::default_proc 
	**/
	MSG_MS_PLMN_ABORT_RSP  = 0x402F,
	 /** 
	capi2 api is CAPI2_MS_GetPlmnMode 
	**/
	MSG_MS_GET_PLMN_MODE_REQ  = 0x4030,
	 /** 
	payload is ::PlmnSelectMode_t 
	**/
	MSG_MS_GET_PLMN_MODE_RSP  = 0x4031,	///<Payload type {::PlmnSelectMode_t}
	 /** 
	capi2 api is CAPI2_MS_SetPlmnMode 
	**/
	MSG_MS_SET_PLMN_MODE_REQ  = 0x4032,	///<Payload type {::CAPI2_MS_SetPlmnMode_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_MS_SET_PLMN_MODE_RSP  = 0x4033,
	 /** 
	capi2 api is CAPI2_MS_GetPlmnFormat 
	**/
	MSG_MS_GET_PLMN_FORMAT_REQ  = 0x4034,
	 /** 
	payload is ::PlmnSelectFormat_t 
	**/
	MSG_MS_GET_PLMN_FORMAT_RSP  = 0x4035,	///<Payload type {::PlmnSelectFormat_t}
	 /** 
	capi2 api is CAPI2_MS_SetPlmnFormat 
	**/
	MSG_MS_SET_PLMN_FORMAT_REQ  = 0x4036,	///<Payload type {::CAPI2_MS_SetPlmnFormat_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_MS_SET_PLMN_FORMAT_RSP  = 0x4037,
	 /** 
	capi2 api is CAPI2_MS_IsMatchedPLMN 
	**/
	MSG_MS_MATCH_PLMN_REQ  = 0x4038,	///<Payload type {::CAPI2_MS_IsMatchedPLMN_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_MS_MATCH_PLMN_RSP  = 0x4039,	///<Payload type {::Boolean}
	 /** 
	capi2 api is CAPI2_MS_SearchAvailablePLMN 
	**/
	MSG_MS_SEARCH_PLMN_REQ  = 0x403A,
	 /** 
	capi2 api is CAPI2_MS_AbortSearchPLMN 
	**/
	MSG_MS_ABORT_PLMN_REQ  = 0x403B,
	 /** 
	payload is ::default_proc 
	**/
	MSG_MS_ABORT_PLMN_RSP  = 0x403C,
	 /** 
	capi2 api is CAPI2_MS_AutoSearchReq 
	**/
	MSG_MS_AUTO_SEARCH_REQ  = 0x403D,
	 /** 
	payload is ::default_proc 
	**/
	MSG_MS_AUTO_SEARCH_RSP  = 0x403E,
	 /** 
	capi2 api is CAPI2_MS_GetPLMNNameByCode 
	**/
	MSG_MS_PLMN_NAME_REQ  = 0x403F,	///<Payload type {::CAPI2_MS_GetPLMNNameByCode_Req_t}
	 /** 
	payload is ::MsPlmnName_t 
	**/
	MSG_MS_PLMN_NAME_RSP  = 0x4040,	///<Payload type {::MsPlmnName_t}
	 /** 
	capi2 api is CAPI2_SYS_IsResetCausedByAssert 
	**/
	MSG_SYS_QUERY_RESET_CAUSE_REQ  = 0x4041,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SYS_QUERY_RESET_CAUSE_RSP  = 0x4042,	///<Payload type {::Boolean}
	 /** 
	capi2 api is CAPI2_SYS_GetSystemState 
	**/
	MSG_SYS_GET_SYSTEM_STATE_REQ  = 0x4043,
	 /** 
	payload is ::SystemState_t 
	**/
	MSG_SYS_GET_SYSTEM_STATE_RSP  = 0x4044,	///<Payload type {::SystemState_t}
	 /** 
	capi2 api is CAPI2_SYS_SetSystemState 
	**/
	MSG_SYS_SET_SYSTEM_STATE_REQ  = 0x4045,	///<Payload type {::CAPI2_SYS_SetSystemState_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SYS_SET_SYSTEM_STATE_RSP  = 0x4046,
	 /** 
	capi2 api is CAPI2_SYS_GetRxSignalInfo 
	**/
	MSG_SYS_GET_RX_LEVEL_REQ  = 0x4047,
	 /** 
	payload is ::MsRxLevelData_t 
	**/
	MSG_SYS_GET_RX_LEVEL_RSP  = 0x4048,	///<Payload type {::MsRxLevelData_t}
	 /** 
	capi2 api is CAPI2_SYS_GetGSMRegistrationStatus 
	**/
	MSG_SYS_GET_GSMREG_STATUS_REQ  = 0x4049,
	 /** 
	payload is ::RegisterStatus_t 
	**/
	MSG_SYS_GET_GSMREG_STATUS_RSP  = 0x404A,	///<Payload type {::RegisterStatus_t}
	 /** 
	capi2 api is CAPI2_SYS_GetGPRSRegistrationStatus 
	**/
	MSG_SYS_GET_GPRSREG_STATUS_REQ  = 0x404B,
	 /** 
	payload is ::RegisterStatus_t 
	**/
	MSG_SYS_GET_GPRSREG_STATUS_RSP  = 0x404C,	///<Payload type {::RegisterStatus_t}
	 /** 
	capi2 api is CAPI2_SYS_IsRegisteredGSMOrGPRS 
	**/
	MSG_SYS_GET_REG_STATUS_REQ  = 0x404D,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SYS_GET_REG_STATUS_RSP  = 0x404E,	///<Payload type {::Boolean}
	 /** 
	capi2 api is CAPI2_SYS_GetGSMRegistrationCause 
	**/
	MSG_SYS_GET_GSMREG_CAUSE_REQ  = 0x404F,
	 /** 
	payload is ::PCHRejectCause_t 
	**/
	MSG_SYS_GET_GSMREG_CAUSE_RSP  = 0x4050,	///<Payload type {::PCHRejectCause_t}
	 /** 
	capi2 api is CAPI2_MS_IsPlmnForbidden 
	**/
	MSG_SYS_IS_PLMN_FORBIDDEN_REQ  = 0x4051,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SYS_IS_PLMN_FORBIDDEN_RSP  = 0x4052,	///<Payload type {::Boolean}
	 /** 
	capi2 api is CAPI2_MS_IsRegisteredHomePLMN 
	**/
	MSG_HOME_PLMN_REG_REQ  = 0x4053,
	 /** 
	payload is ::Boolean 
	**/
	MSG_HOME_PLMN_REG_RSP  = 0x4054,	///<Payload type {::Boolean}
	 /** 
	capi2 api is CAPI2_MS_SetPowerDownTimer 
	**/
	MSG_SET_POWER_DOWN_TIMER_REQ  = 0x4055,	///<Payload type {::CAPI2_MS_SetPowerDownTimer_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SET_POWER_DOWN_TIMER_RSP  = 0x4056,
	 /** 
	capi2 api is CAPI2_SIMLockIsLockOn 
	**/
	MSG_SIM_LOCK_ON_REQ  = 0x4057,	///<Payload type {::CAPI2_SIMLockIsLockOn_Req_t}
	 /** 
	payload is ::SIM_LOCK_ON_RESULT_t 
	**/
	MSG_SIM_LOCK_ON_RSP  = 0x4058,	///<Payload type {::SIM_LOCK_ON_RESULT_t}
	 /** 
	capi2 api is CAPI2_SIMLockCheckAllLocks 
	**/
	MSG_SIM_LOCK_STATUS_REQ  = 0x4059,	///<Payload type {::CAPI2_SIMLockCheckAllLocks_Req_t}
	 /** 
	payload is ::SIM_BOOLEAN_DATA_t 
	**/
	MSG_SIM_LOCK_STATUS_RSP  = 0x405A,	///<Payload type {::SIM_BOOLEAN_DATA_t}
	 /** 
	capi2 api is CAPI2_SIMLockUnlockSIM 
	**/
	MSG_SIM_LOCK_UNLOCK_REQ  = 0x405B,	///<Payload type {::CAPI2_SIMLockUnlockSIM_Req_t}
	 /** 
	payload is ::SIM_LOCK_RESULT_t 
	**/
	MSG_SIM_LOCK_UNLOCK_RSP  = 0x405C,	///<Payload type {::SIM_LOCK_RESULT_t}
	 /** 
	capi2 api is CAPI2_SIMLockSetLock 
	**/
	MSG_SIM_LOCK_SET_LOCK_REQ  = 0x405D,	///<Payload type {::CAPI2_SIMLockSetLock_t}
	 /** 
	payload is ::SIM_LOCK_RESULT_t 
	**/
	MSG_SIM_LOCK_SET_LOCK_RSP  = 0x405E,	///<Payload type {::SIM_LOCK_RESULT_t}
	 /** 
	capi2 api is CAPI2_SIMLockGetCurrentClosedLock 
	**/
	MSG_SIM_LOCK_CLOSED_LOCK_REQ  = 0x405F,
	 /** 
	payload is ::SIM_LOCK_TYPE_RESULT_t 
	**/
	MSG_SIM_LOCK_CLOSED_LOCK_RSP  = 0x4060,	///<Payload type {::SIM_LOCK_TYPE_RESULT_t}
	 /** 
	capi2 api is CAPI2_SIMLockChangePasswordPHSIM 
	**/
	MSG_SIM_LOCK_CHANGE_PWD_REQ  = 0x4061,	///<Payload type {::CAPI2_SIMLockChangePasswordPHSIM_Req_t}
	 /** 
	payload is ::SIM_LOCK_RESULT_t 
	**/
	MSG_SIM_LOCK_CHANGE_PWD_RSP  = 0x4062,	///<Payload type {::SIM_LOCK_RESULT_t}
	 /** 
	capi2 api is CAPI2_SIMLockCheckPasswordPHSIM 
	**/
	MSG_SIM_LOCK_CHECK_PWD_PHSIM_REQ  = 0x4063,	///<Payload type {::CAPI2_SIMLockCheckPasswordPHSIM_Req_t}
	 /** 
	payload is ::SIM_BOOLEAN_DATA_t 
	**/
	MSG_SIM_LOCK_CHECK_PWD_PHSIM_RSP  = 0x4064,	///<Payload type {::SIM_BOOLEAN_DATA_t}
	 /** 
	capi2 api is CAPI2_SIMLockGetSignature 
	**/
	MSG_SIM_LOCK_LOCK_SIG_REQ  = 0x4065,
	 /** 
	payload is ::SIM_LOCK_SIG_DATA_t 
	**/
	MSG_SIM_LOCK_LOCK_SIG_RSP  = 0x4066,	///<Payload type {::SIM_LOCK_SIG_DATA_t}
	 /** 
	capi2 api is CAPI2_SIMLockGetImeiSecboot 
	**/
	MSG_SIM_LOCK_GET_IMEI_REQ  = 0x4067,
	 /** 
	payload is ::SIM_LOCK_IMEI_DATA_t 
	**/
	MSG_SIM_LOCK_GET_IMEI_RSP  = 0x4068,	///<Payload type {::SIM_LOCK_IMEI_DATA_t}
	 /** 
	capi2 api is CAPI2_SIM_GetSmsParamRecNum 
	**/
	MSG_SIM_PARAM_REC_NUM_REQ  = 0x4069,
	 /** 
	payload is ::SIM_INTEGER_DATA_t 
	**/
	MSG_SIM_PARAM_REC_NUM_RSP  = 0x406A,	///<Payload type {::SIM_INTEGER_DATA_t}
	 /** 
	capi2 api is CAPI2_MS_ConvertPLMNNameStr 
	**/
	MSG_MS_CONVERT_PLMN_STRING_REQ  = 0x406B,	///<Payload type {::CAPI2_MS_ConvertPLMNNameStr_t}
	 /** 
	payload is ::uchar_ptr_t 
	**/
	MSG_MS_CONVERT_PLMN_STRING_RSP  = 0x406C,	///<Payload type {::uchar_ptr_t}
	 /** 
	capi2 api is CAPI2_SIM_GetSmsMemExceededFlag 
	**/
	MSG_SIM_GET_SMSMEMEXC_FLAG_REQ  = 0x406D,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_GET_SMSMEMEXC_FLAG_RSP  = 0x406E,	///<Payload type {::Boolean}
	 /** 
	capi2 api is CAPI2_SIM_IsPINRequired 
	**/
	MSG_SIM_PIN_REQ_REQ  = 0x406F,
	 /** 
	payload is ::SIM_BOOLEAN_DATA_t 
	**/
	MSG_SIM_PIN_REQ_RSP  = 0x4070,	///<Payload type {::SIM_BOOLEAN_DATA_t}
	 /** 
	capi2 api is CAPI2_SIM_GetCardPhase 
	**/
	MSG_SIM_CARD_PHASE_REQ  = 0x4071,
	 /** 
	payload is ::SIM_PHASE_RESULT_t 
	**/
	MSG_SIM_CARD_PHASE_RSP  = 0x4072,	///<Payload type {::SIM_PHASE_RESULT_t}
	 /** 
	capi2 api is CAPI2_SIM_GetSIMType 
	**/
	MSG_SIM_TYPE_REQ  = 0x4073,
	 /** 
	payload is ::SIM_TYPE_RESULT_t 
	**/
	MSG_SIM_TYPE_RSP  = 0x4074,	///<Payload type {::SIM_TYPE_RESULT_t}
	 /** 
	capi2 api is CAPI2_SIM_GetPresentStatus 
	**/
	MSG_SIM_PRESENT_REQ  = 0x4075,
	 /** 
	payload is ::SIM_PRESENT_RESULT_t 
	**/
	MSG_SIM_PRESENT_RSP  = 0x4076,	///<Payload type {::SIM_PRESENT_RESULT_t}
	 /** 
	capi2 api is CAPI2_SIM_IsOperationRestricted 
	**/
	MSG_SIM_PIN_OPERATION_REQ  = 0x4077,
	 /** 
	payload is ::SIM_BOOLEAN_DATA_t 
	**/
	MSG_SIM_PIN_OPERATION_RSP  = 0x4078,	///<Payload type {::SIM_BOOLEAN_DATA_t}
	 /** 
	capi2 api is CAPI2_SIM_IsPINBlocked 
	**/
	MSG_SIM_PIN_BLOCK_REQ  = 0x4079,	///<Payload type {::CAPI2_SIM_IsPINBlocked_Req_t}
	 /** 
	payload is ::SIM_BOOLEAN_DATA_t 
	**/
	MSG_SIM_PIN_BLOCK_RSP  = 0x407A,	///<Payload type {::SIM_BOOLEAN_DATA_t}
	 /** 
	capi2 api is CAPI2_SIM_IsPUKBlocked 
	**/
	MSG_SIM_PUK_BLOCK_REQ  = 0x407B,	///<Payload type {::CAPI2_SIM_IsPUKBlocked_Req_t}
	 /** 
	payload is ::SIM_BOOLEAN_DATA_t 
	**/
	MSG_SIM_PUK_BLOCK_RSP  = 0x407C,	///<Payload type {::SIM_BOOLEAN_DATA_t}
	 /** 
	capi2 api is CAPI2_SIM_IsInvalidSIM 
	**/
	MSG_SIM_IS_INVALID_REQ  = 0x407D,
	 /** 
	payload is ::SIM_BOOLEAN_DATA_t 
	**/
	MSG_SIM_IS_INVALID_RSP  = 0x407E,	///<Payload type {::SIM_BOOLEAN_DATA_t}
	 /** 
	capi2 api is CAPI2_SIM_DetectSim 
	**/
	MSG_SIM_DETECT_REQ  = 0x407F,
	 /** 
	payload is ::SIM_BOOLEAN_DATA_t 
	**/
	MSG_SIM_DETECT_RSP  = 0x4080,	///<Payload type {::SIM_BOOLEAN_DATA_t}
	 /** 
	capi2 api is CAPI2_SIM_GetRuimSuppFlag 
	**/
	MSG_SIM_GET_RUIM_SUPP_FLAG_REQ  = 0x4081,
	 /** 
	payload is ::SIM_BOOLEAN_DATA_t 
	**/
	MSG_SIM_GET_RUIM_SUPP_FLAG_RSP  = 0x4082,	///<Payload type {::SIM_BOOLEAN_DATA_t}
	 /** 
	capi2 api is CAPI2_SIM_SendVerifyChvReq 
	**/
	MSG_SIM_VERIFY_CHV_REQ  = 0x4083,	///<Payload type {::CAPI2_SIM_SendVerifyChvReq_Req_t}
	 /** 
	capi2 api is CAPI2_SIM_SendChangeChvReq 
	**/
	MSG_SIM_CHANGE_CHV_REQ  = 0x4084,	///<Payload type {::CAPI2_SIM_SendChangeChvReq_Req_t}
	 /** 
	capi2 api is CAPI2_SIM_SendSetChv1OnOffReq 
	**/
	MSG_SIM_ENABLE_CHV_REQ  = 0x4085,	///<Payload type {::CAPI2_SIM_SendSetChv1OnOffReq_Req_t}
	 /** 
	capi2 api is CAPI2_SIM_SendUnblockChvReq 
	**/
	MSG_SIM_UNBLOCK_CHV_REQ  = 0x4086,	///<Payload type {::CAPI2_SIM_SendUnblockChvReq_Req_t}
	 /** 
	capi2 api is CAPI2_SIM_SendSetOperStateReq 
	**/
	MSG_SIM_SET_FDN_REQ  = 0x4087,	///<Payload type {::CAPI2_SIM_SendSetOperStateReq_Req_t}
	 /** 
	capi2 api is CAPI2_SIM_IsPbkAccessAllowed 
	**/
	MSG_SIM_IS_PBK_ALLOWED_REQ  = 0x4088,	///<Payload type {::CAPI2_SIM_IsPbkAccessAllowed_Req_t}
	 /** 
	payload is ::SIM_BOOLEAN_DATA_t 
	**/
	MSG_SIM_IS_PBK_ALLOWED_RSP  = 0x4089,	///<Payload type {::SIM_BOOLEAN_DATA_t}
	 /** 
	capi2 api is CAPI2_SIM_SendPbkInfoReq 
	**/
	MSG_SIM_PBK_INFO_REQ  = 0x408A,	///<Payload type {::CAPI2_SIM_SendPbkInfoReq_Req_t}
	 /** 
	capi2 api is CAPI2_SIM_SendReadAcmMaxReq 
	**/
	MSG_SIM_MAX_ACM_REQ  = 0x408B,
	 /** 
	capi2 api is CAPI2_SIM_SendWriteAcmMaxReq 
	**/
	MSG_SIM_ACM_MAX_UPDATE_REQ  = 0x408C,	///<Payload type {::CAPI2_SIM_SendWriteAcmMaxReq_Req_t}
	 /** 
	capi2 api is CAPI2_SIM_SendReadAcmReq 
	**/
	MSG_SIM_ACM_VALUE_REQ  = 0x408D,
	 /** 
	capi2 api is CAPI2_SIM_SendWriteAcmReq 
	**/
	MSG_SIM_ACM_UPDATE_REQ  = 0x408E,	///<Payload type {::CAPI2_SIM_SendWriteAcmReq_Req_t}
	 /** 
	capi2 api is CAPI2_SIM_SendIncreaseAcmReq 
	**/
	MSG_SIM_ACM_INCREASE_REQ  = 0x408F,	///<Payload type {::CAPI2_SIM_SendIncreaseAcmReq_Req_t}
	 /** 
	capi2 api is CAPI2_SIM_SendReadSvcProvNameReq 
	**/
	MSG_SIM_SVC_PROV_NAME_REQ  = 0x4090,
	 /** 
	capi2 api is CAPI2_SIM_SendReadPuctReq 
	**/
	MSG_SIM_PUCT_DATA_REQ  = 0x4091,
	 /** 
	capi2 api is CAPI2_SIM_GetServiceStatus 
	**/
	MSG_SIM_SERVICE_STATUS_REQ  = 0x4092,	///<Payload type {::CAPI2_SIM_GetServiceStatus_Req_t}
	 /** 
	payload is ::SIM_SERVICE_STATUS_RESULT_t 
	**/
	MSG_SIM_SERVICE_STATUS_RSP  = 0x4093,	///<Payload type {::SIM_SERVICE_STATUS_RESULT_t}
	 /** 
	capi2 api is CAPI2_SIM_SendGenericAccessEndReq 
	**/
	MSG_SIM_GENERIC_ACCESS_END_REQ  = 0x4094,
	 /** 
	payload is ::default_proc 
	**/
	MSG_SIM_GENERIC_ACCESS_END_RSP  = 0x4095,
	 /** 
	capi2 api is CAPI2_SIM_GetPinStatus 
	**/
	MSG_SIM_PIN_STATUS_REQ  = 0x4096,
	 /** 
	payload is ::SIM_PIN_STATUS_RESULT_t 
	**/
	MSG_SIM_PIN_STATUS_RSP  = 0x4097,	///<Payload type {::SIM_PIN_STATUS_RESULT_t}
	 /** 
	capi2 api is CAPI2_SIM_IsPinOK 
	**/
	MSG_SIM_PIN_OK_STATUS_REQ  = 0x4098,
	 /** 
	payload is ::SIM_BOOLEAN_DATA_t 
	**/
	MSG_SIM_PIN_OK_STATUS_RSP  = 0x4099,	///<Payload type {::SIM_BOOLEAN_DATA_t}
	 /** 
	capi2 api is CAPI2_SIM_GetIMSI 
	**/
	MSG_SIM_IMSI_REQ  = 0x409A,
	 /** 
	payload is ::SIM_IMSI_RESULT_t 
	**/
	MSG_SIM_IMSI_RSP  = 0x409B,	///<Payload type {::SIM_IMSI_RESULT_t}
	 /** 
	capi2 api is CAPI2_SIM_GetGID1 
	**/
	MSG_SIM_GID_DIGIT_REQ  = 0x409C,
	 /** 
	payload is ::SIM_GID_RESULT_t 
	**/
	MSG_SIM_GID_DIGIT_RSP  = 0x409D,	///<Payload type {::SIM_GID_RESULT_t}
	 /** 
	capi2 api is CAPI2_SIM_GetGID2 
	**/
	MSG_SIM_GID_DIGIT2_REQ  = 0x409E,
	 /** 
	payload is ::SIM_GID_RESULT_t 
	**/
	MSG_SIM_GID_DIGIT2_RSP  = 0x409F,	///<Payload type {::SIM_GID_RESULT_t}
	 /** 
	capi2 api is CAPI2_SIM_GetHomePlmn 
	**/
	MSG_SIM_HOME_PLMN_REQ  = 0x40A0,
	 /** 
	payload is ::SIM_HOME_PLMN_RESULT_t 
	**/
	MSG_SIM_HOME_PLMN_RSP  = 0x40A1,	///<Payload type {::SIM_HOME_PLMN_RESULT_t}
	 /** 
	capi2 api is CAPI2_SIM_GetCurrLockedSimlockType 
	**/
	MSG_SIM_PIN_LOCK_TYPE_REQ  = 0x40A2,
	 /** 
	payload is ::SIM_PIN_STATUS_RESULT_t 
	**/
	MSG_SIM_PIN_LOCK_TYPE_RSP  = 0x40A3,	///<Payload type {::SIM_PIN_STATUS_RESULT_t}
	 /** 
	capi2 api is CAPI2_simmi_GetMasterFileId 
	**/
	MSG_SIM_APDU_FILEID_REQ  = 0x40A4,	///<Payload type {::CAPI2_simmi_GetMasterFileId_Req_t}
	 /** 
	payload is ::SIM_APDU_FILEID_RESULT_t 
	**/
	MSG_SIM_APDU_FILEID_RSP  = 0x40A5,	///<Payload type {::SIM_APDU_FILEID_RESULT_t}
	 /** 
	capi2 api is CAPI2_SIM_SendOpenSocketReq 
	**/
	MSG_SIM_OPEN_SOCKET_REQ  = 0x40A6,
	 /** 
	capi2 api is CAPI2_SIM_SendSelectAppiReq 
	**/
	MSG_SIM_SELECT_APPLI_REQ  = 0x40A7,	///<Payload type {::CAPI2_SimSelectAppli_t}
	 /** 
	capi2 api is CAPI2_SIM_SendDeactivateAppiReq 
	**/
	MSG_SIM_DEACTIVATE_APPLI_REQ  = 0x40A8,	///<Payload type {::CAPI2_SimDeactivateAppli_t}
	 /** 
	capi2 api is CAPI2_SIM_SendCloseSocketReq 
	**/
	MSG_SIM_CLOSE_SOCKET_REQ  = 0x40A9,	///<Payload type {::CAPI2_SimCloseSocket_t}
	 /** 
	capi2 api is CAPI2_SIM_GetAtrData 
	**/
	MSG_SIM_ATR_DATA_REQ  = 0x40AA,
	 /** 
	payload is ::SIM_ATR_DATA_t 
	**/
	MSG_SIM_ATR_DATA_RSP  = 0x40AB,	///<Payload type {::SIM_ATR_DATA_t}
	 /** 
	capi2 api is CAPI2_SIM_SubmitDFileInfoReq 
	**/
	MSG_SIM_DFILE_INFO_REQ  = 0x40AC,	///<Payload type {::CAPI2_SimFileInfo_t}
	 /** 
	capi2 api is CAPI2_SIM_SubmitEFileInfoReq 
	**/
	MSG_SIM_EFILE_INFO_REQ  = 0x40AD,	///<Payload type {::CAPI2_SimFileInfo_t}
	 /** 
	capi2 api is CAPI2_SIM_SubmitWholeBinaryEFileReadReq 
	**/
	MSG_SIM_WHOLE_EFILE_DATA_REQ  = 0x40AE,	///<Payload type {::CAPI2_SimFileInfo_t}
	 /** 
	capi2 api is CAPI2_SIM_SubmitBinaryEFileReadReq 
	**/
	MSG_SIM_EFILE_DATA_REQ  = 0x40AF,	///<Payload type {::CAPI2_SimFileInfo_t}
	 /** 
	capi2 api is CAPI2_SIM_SubmitRecordEFileReadReq 
	**/
	MSG_SIM_RECORD_EFILE_DATA_REQ  = 0x40B0,	///<Payload type {::CAPI2_SimFileInfo_t}
	 /** 
	capi2 api is CAPI2_SIM_SubmitBinaryEFileUpdateReq 
	**/
	MSG_SIM_EFILE_UPDATE_REQ  = 0x40B1,	///<Payload type {::CAPI2_SimFileInfo_t}
	 /** 
	capi2 api is CAPI2_SIM_SubmitLinearEFileUpdateReq 
	**/
	MSG_SIM_LINEAR_EFILE_UPDATE_REQ  = 0x40B2,	///<Payload type {::CAPI2_SimFileInfo_t}
	 /** 
	capi2 api is CAPI2_SIM_SubmitSeekRecordReq 
	**/
	MSG_SIM_SEEK_RECORD_REQ  = 0x40B3,	///<Payload type {::CAPI2_SimFileInfo_t}
	 /** 
	capi2 api is CAPI2_SIM_SubmitCyclicEFileUpdateReq 
	**/
	MSG_SIM_CYCLIC_EFILE_UPDATE_REQ  = 0x40B4,	///<Payload type {::CAPI2_SimFileInfo_t}
	 /** 
	capi2 api is CAPI2_SIM_SendRemainingPinAttemptReq 
	**/
	MSG_SIM_PIN_ATTEMPT_REQ  = 0x40B5,
	 /** 
	capi2 api is CAPI2_SIM_IsCachedDataReady 
	**/
	MSG_SIM_CACHE_DATA_READY_REQ  = 0x40B6,
	 /** 
	payload is ::SIM_BOOLEAN_DATA_t 
	**/
	MSG_SIM_CACHE_DATA_READY_RSP  = 0x40B7,	///<Payload type {::SIM_BOOLEAN_DATA_t}
	 /** 
	capi2 api is CAPI2_SIM_GetServiceCodeStatus 
	**/
	MSG_SIM_SERVICE_CODE_STATUS_REQ  = 0x40B8,	///<Payload type {::CAPI2_SIM_GetServiceCodeStatus_Req_t}
	 /** 
	payload is ::SIM_SERVICE_FLAG_STATUS_t 
	**/
	MSG_SIM_SERVICE_CODE_STATUS_RSP  = 0x40B9,	///<Payload type {::SIM_SERVICE_FLAG_STATUS_t}
	 /** 
	capi2 api is CAPI2_SIM_CheckCphsService 
	**/
	MSG_SIM_CHECK_CPHS_REQ  = 0x40BA,	///<Payload type {::CAPI2_SIM_CheckCphsService_Req_t}
	 /** 
	payload is ::SIM_BOOLEAN_DATA_t 
	**/
	MSG_SIM_CHECK_CPHS_RSP  = 0x40BB,	///<Payload type {::SIM_BOOLEAN_DATA_t}
	 /** 
	capi2 api is CAPI2_SIM_GetCphsPhase 
	**/
	MSG_SIM_CPHS_PHASE_REQ  = 0x40BC,
	 /** 
	payload is ::SIM_INTEGER_DATA_t 
	**/
	MSG_SIM_CPHS_PHASE_RSP  = 0x40BD,	///<Payload type {::SIM_INTEGER_DATA_t}
	 /** 
	capi2 api is CAPI2_SIM_GetSmsSca 
	**/
	MSG_SIM_SMS_SCA_REQ  = 0x40BE,	///<Payload type {::CAPI2_SIM_GetSmsSca_Req_t}
	 /** 
	payload is ::SIM_SCA_DATA_RESULT_t 
	**/
	MSG_SIM_SMS_SCA_RSP  = 0x40BF,	///<Payload type {::SIM_SCA_DATA_RESULT_t}
	 /** 
	capi2 api is CAPI2_SIM_GetIccid 
	**/
	MSG_SIM_ICCID_PARAM_REQ  = 0x40C0,
	 /** 
	payload is ::SIM_ICCID_STATUS_t 
	**/
	MSG_SIM_ICCID_PARAM_RSP  = 0x40C1,	///<Payload type {::SIM_ICCID_STATUS_t}
	 /** 
	capi2 api is CAPI2_SIM_IsALSEnabled 
	**/
	MSG_SIM_ALS_STATUS_REQ  = 0x40C2,
	 /** 
	payload is ::SIM_BOOLEAN_DATA_t 
	**/
	MSG_SIM_ALS_STATUS_RSP  = 0x40C3,	///<Payload type {::SIM_BOOLEAN_DATA_t}
	 /** 
	capi2 api is CAPI2_SIM_GetAlsDefaultLine 
	**/
	MSG_SIM_ALS_DEFAULT_LINE_REQ  = 0x40C4,
	 /** 
	payload is ::SIM_INTEGER_DATA_t 
	**/
	MSG_SIM_ALS_DEFAULT_LINE_RSP  = 0x40C5,	///<Payload type {::SIM_INTEGER_DATA_t}
	 /** 
	capi2 api is CAPI2_SIM_SetAlsDefaultLine 
	**/
	MSG_SIM_SET_ALS_DEFAULT_REQ  = 0x40C6,	///<Payload type {::CAPI2_SIM_SetAlsDefaultLine_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SIM_SET_ALS_DEFAULT_RSP  = 0x40C7,
	 /** 
	capi2 api is CAPI2_SIM_GetCallForwardUnconditionalFlag 
	**/
	MSG_SIM_CALLFWD_COND_FLAG_REQ  = 0x40C8,
	 /** 
	payload is ::SIM_CALL_FORWARD_UNCONDITIONAL_STATUS_t 
	**/
	MSG_SIM_CALLFWD_COND_FLAG_RSP  = 0x40C9,	///<Payload type {::SIM_CALL_FORWARD_UNCONDITIONAL_STATUS_t}
	 /** 
	capi2 api is CAPI2_SIM_GetApplicationType 
	**/
	MSG_SIM_APP_TYPE_REQ  = 0x40CA,
	 /** 
	payload is ::SIM_APPL_TYPE_RESULT_t 
	**/
	MSG_SIM_APP_TYPE_RSP  = 0x40CB,	///<Payload type {::SIM_APPL_TYPE_RESULT_t}
	 /** 
	capi2 api is CAPI2_USIM_GetUst 
	**/
	MSG_USIM_UST_DATA_REQ  = 0x40CC,
	 /** 
	payload is ::USIM_UST_DATA_RSP_t 
	**/
	MSG_USIM_UST_DATA_RSP  = 0x40CD,	///<Payload type {::USIM_UST_DATA_RSP_t}
	 /** 
	capi2 api is CAPI2_SIM_SendUpdatePrefListReq 
	**/
	MSG_SIM_PLMN_WRITE_REQ  = 0x40CE,	///<Payload type {::CAPI2_SIM_SendUpdatePrefListReq_t}
	 /** 
	capi2 api is CAPI2_SIM_SendWritePuctReq 
	**/
	MSG_SIM_PUCT_UPDATE_REQ  = 0x40CF,	///<Payload type {::CAPI2_SIM_SendWritePuctReq_t}
	 /** 
	capi2 api is CAPI2_SIM_SendGenericAccessReq 
	**/
	MSG_SIM_GENERIC_ACCESS_REQ  = 0x40D0,	///<Payload type {::CAPI2_SIM_SendGenericAccessReq_t}
	 /** 
	capi2 api is CAPI2_SIM_SubmitRestrictedAccessReq 
	**/
	MSG_SIM_RESTRICTED_ACCESS_REQ  = 0x40D1,	///<Payload type {::CAPI2_SIM_SendRestrictedAccessReq_t}
	 /** 
	capi2 api is CAPI2_ADCMGR_Start 
	**/
	MSG_ADC_START_REQ  = 0x40D2,	///<Payload type {::CAPI2_ADCMGR_Start_Req_t}
	 /** 
	capi2 api is CAPI2_AT_ProcessCmd 
	**/
	MSG_CAPI2_AT_COMMAND_REQ  = 0x40D3,	///<Payload type {::CAPI2_AT_ProcessCmd_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_CAPI2_AT_COMMAND_RSP  = 0x40D4,
	 /** 
	capi2 api is CAPI2_MS_GetSystemRAT 
	**/
	MSG_MS_GET_SYSTEM_RAT_REQ  = 0x40D5,
	 /** 
	payload is ::RATSelect_t 
	**/
	MSG_MS_GET_SYSTEM_RAT_RSP  = 0x40D6,	///<Payload type {::RATSelect_t}
	 /** 
	capi2 api is CAPI2_MS_GetSupportedRAT 
	**/
	MSG_MS_GET_SUPPORTED_RAT_REQ  = 0x40D7,
	 /** 
	payload is ::RATSelect_t 
	**/
	MSG_MS_GET_SUPPORTED_RAT_RSP  = 0x40D8,	///<Payload type {::RATSelect_t}
	 /** 
	capi2 api is CAPI2_MS_GetSystemBand 
	**/
	MSG_MS_GET_SYSTEM_BAND_REQ  = 0x40D9,
	 /** 
	payload is ::BandSelect_t 
	**/
	MSG_MS_GET_SYSTEM_BAND_RSP  = 0x40DA,	///<Payload type {::BandSelect_t}
	 /** 
	capi2 api is CAPI2_MS_GetSupportedBand 
	**/
	MSG_MS_GET_SUPPORTED_BAND_REQ  = 0x40DB,
	 /** 
	payload is ::BandSelect_t 
	**/
	MSG_MS_GET_SUPPORTED_BAND_RSP  = 0x40DC,	///<Payload type {::BandSelect_t}
	 /** 
	capi2 api is CAPI2_SYSPARM_GetMSClass 
	**/
	MSG_SYSPARAM_GET_MSCLASS_REQ  = 0x40DD,
	 /** 
	payload is ::UInt16 
	**/
	MSG_SYSPARAM_GET_MSCLASS_RSP  = 0x40DE,	///<Payload type {::UInt16}
	 /** 
	capi2 api is CAPI2_AUDIO_GetMicrophoneGainSetting 
	**/
	MSG_AUDIO_GET_MIC_GAIN_REQ  = 0x40DF,
	 /** 
	payload is ::UInt16 
	**/
	MSG_AUDIO_GET_MIC_GAIN_RSP  = 0x40E0,	///<Payload type {::UInt16}
	 /** 
	capi2 api is CAPI2_AUDIO_GetSpeakerVol 
	**/
	MSG_AUDIO_GET_SPEAKER_VOL_REQ  = 0x40E1,
	 /** 
	payload is ::UInt16 
	**/
	MSG_AUDIO_GET_SPEAKER_VOL_RSP  = 0x40E2,	///<Payload type {::UInt16}
	 /** 
	capi2 api is CAPI2_AUDIO_SetSpeakerVol 
	**/
	MSG_AUDIO_SET_SPEAKER_VOL_REQ  = 0x40E3,	///<Payload type {::CAPI2_AUDIO_SetSpeakerVol_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_AUDIO_SET_SPEAKER_VOL_RSP  = 0x40E4,
	 /** 
	capi2 api is CAPI2_AUDIO_SetMicrophoneGain 
	**/
	MSG_AUDIO_SET_MIC_GAIN_REQ  = 0x40E5,	///<Payload type {::CAPI2_AUDIO_SetMicrophoneGain_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_AUDIO_SET_MIC_GAIN_RSP  = 0x40E6,
	 /** 
	capi2 api is CAPI2_SYSPARM_GetManufacturerName 
	**/
	MSG_SYSPARAM_GET_MNF_NAME_REQ  = 0x40E7,
	 /** 
	payload is ::uchar_ptr_t 
	**/
	MSG_SYSPARAM_GET_MNF_NAME_RSP  = 0x40E8,	///<Payload type {::uchar_ptr_t}
	 /** 
	capi2 api is CAPI2_SYSPARM_GetModelName 
	**/
	MSG_SYSPARAM_GET_MODEL_NAME_REQ  = 0x40E9,
	 /** 
	payload is ::uchar_ptr_t 
	**/
	MSG_SYSPARAM_GET_MODEL_NAME_RSP  = 0x40EA,	///<Payload type {::uchar_ptr_t}
	 /** 
	capi2 api is CAPI2_SYSPARM_GetSWVersion 
	**/
	MSG_SYSPARAM_GET_SW_VERSION_REQ  = 0x40EB,
	 /** 
	payload is ::uchar_ptr_t 
	**/
	MSG_SYSPARAM_GET_SW_VERSION_RSP  = 0x40EC,	///<Payload type {::uchar_ptr_t}
	 /** 
	capi2 api is CAPI2_SYSPARM_GetEGPRSMSClass 
	**/
	MSG_SYSPARAM_GET_EGPRS_CLASS_REQ  = 0x40ED,
	 /** 
	payload is ::UInt16 
	**/
	MSG_SYSPARAM_GET_EGPRS_CLASS_RSP  = 0x40EE,	///<Payload type {::UInt16}
	 /** 
	capi2 api is CAPI2_UTIL_ExtractImei 
	**/
	MSG_SYSPARAM_GET_IMEI_STR_REQ  = 0x40EF,
	 /** 
	payload is ::MSImeiStr_t 
	**/
	MSG_SYSPARAM_GET_IMEI_STR_RSP  = 0x40F0,	///<Payload type {::MSImeiStr_t}
	 /** 
	capi2 api is CAPI2_MS_GetRegistrationInfo 
	**/
	MSG_MS_GET_REG_INFO_REQ  = 0x40F1,
	 /** 
	payload is ::MSRegStateInfo_t 
	**/
	MSG_MS_GET_REG_INFO_RSP  = 0x40F2,	///<Payload type {::MSRegStateInfo_t}
	 /** 
	capi2 api is CAPI2_SIM_SendNumOfPLMNEntryReq 
	**/
	MSG_SIM_PLMN_NUM_OF_ENTRY_REQ  = 0x40F3,	///<Payload type {::CAPI2_SIM_SendNumOfPLMNEntryReq_Req_t}
	 /** 
	capi2 api is CAPI2_SIM_SendReadPLMNEntryReq 
	**/
	MSG_SIM_PLMN_ENTRY_DATA_REQ  = 0x40F4,	///<Payload type {::CAPI2_SIM_SendReadPLMNEntryReq_Req_t}
	 /** 
	capi2 api is CAPI2_SIM_SendWritePLMNEntryReq 
	**/
	MSG_SIM_PLMN_ENTRY_UPDATE_REQ  = 0x40F5,	///<Payload type {::CAPI2_SendWritePLMNEntry_t}
	 /** 
	capi2 api is CAPI2_SIM_SendWriteMulPLMNEntryReq 
	**/
	MSG_SIM_MUL_PLMN_ENTRY_UPDATE_REQ  = 0x40F6,	///<Payload type {::CAPI2_SendWriteMulPLMNEntry_t}
	 /** 
	capi2 api is CAPI2_SYS_SetRegisteredEventMask 
	**/
	MSG_SYS_SET_REG_EVENT_MASK_REQ  = 0x40F7,	///<Payload type {::CAPI2_SYS_SetRegisteredEventMask_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SYS_SET_REG_EVENT_MASK_RSP  = 0x40F8,	///<Payload type {::Boolean}
	 /** 
	capi2 api is CAPI2_SYS_SetFilteredEventMask 
	**/
	MSG_SYS_SET_REG_FILTER_MASK_REQ  = 0x40F9,	///<Payload type {::CAPI2_SYS_SetFilteredEventMask_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SYS_SET_REG_FILTER_MASK_RSP  = 0x40FA,	///<Payload type {::Boolean}
	 /** 
	capi2 api is CAPI2_SYS_SetRssiThreshold 
	**/
	MSG_SYS_SET_RSSI_THRESHOLD_REQ  = 0x40FB,	///<Payload type {::CAPI2_SYS_SetRssiThreshold_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SYS_SET_RSSI_THRESHOLD_RSP  = 0x40FC,
	 /** 
	capi2 api is CAPI2_SYS_GetBootLoaderVersion 
	**/
	MSG_SYSPARAM_BOOTLOADER_VER_REQ  = 0x40FD,
	 /** 
	payload is ::uchar_ptr_t 
	**/
	MSG_SYSPARAM_BOOTLOADER_VER_RSP  = 0x40FE,	///<Payload type {::uchar_ptr_t}
	 /** 
	capi2 api is CAPI2_SYS_GetDSFVersion 
	**/
	MSG_SYSPARAM_DSF_VER_REQ  = 0x40FF,
	 /** 
	payload is ::uchar_ptr_t 
	**/
	MSG_SYSPARAM_DSF_VER_RSP  = 0x4100,	///<Payload type {::uchar_ptr_t}
	 /** 
	capi2 api is CAPI2_SYSPARM_GetChanMode 
	**/
	MSG_SYSPARAM_GET_CHANNEL_MODE_REQ  = 0x4101,
	 /** 
	payload is ::UInt16 
	**/
	MSG_SYSPARAM_GET_CHANNEL_MODE_RSP  = 0x4102,	///<Payload type {::UInt16}
	 /** 
	capi2 api is CAPI2_SYSPARM_GetClassmark 
	**/
	MSG_SYSPARAM_GET_CLASSMARK_REQ  = 0x4103,
	 /** 
	payload is ::CAPI2_Class_t 
	**/
	MSG_SYSPARAM_GET_CLASSMARK_RSP  = 0x4104,	///<Payload type {::CAPI2_Class_t}
	 /** 
	capi2 api is CAPI2_SYSPARM_GetIMEI 
	**/
	MSG_SYSPARAM_GET_IMEI_REQ  = 0x4105,
	 /** 
	payload is ::CAPI2_SYSPARM_IMEI_PTR_t 
	**/
	MSG_SYSPARAM_GET_IMEI_RSP  = 0x4106,	///<Payload type {::CAPI2_SYSPARM_IMEI_PTR_t}
	 /** 
	capi2 api is CAPI2_SYSPARM_GetSysparmIndPartFileVersion 
	**/
	MSG_SYSPARAM_GET_IND_FILE_VER_REQ  = 0x4107,
	 /** 
	payload is ::UInt16 
	**/
	MSG_SYSPARAM_GET_IND_FILE_VER_RSP  = 0x4108,	///<Payload type {::UInt16}
	 /** 
	capi2 api is CAPI2_SYSPARM_SetDARPCfg 
	**/
	MSG_SYS_SET_DARP_CFG_REQ  = 0x4109,	///<Payload type {::CAPI2_SYSPARM_SetDARPCfg_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SYS_SET_DARP_CFG_RSP  = 0x410A,
	 /** 
	capi2 api is CAPI2_SYSPARM_SetEGPRSMSClass 
	**/
	MSG_SYSPARAM_SET_EGPRS_MSCLASS_REQ  = 0x410B,	///<Payload type {::CAPI2_SYSPARM_SetEGPRSMSClass_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SYSPARAM_SET_EGPRS_MSCLASS_RSP  = 0x410C,
	 /** 
	capi2 api is CAPI2_SYSPARM_SetGPRSMSClass 
	**/
	MSG_SYSPARAM_SET_GPRS_MSCLASS_REQ  = 0x410D,	///<Payload type {::CAPI2_SYSPARM_SetGPRSMSClass_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SYSPARAM_SET_GPRS_MSCLASS_RSP  = 0x410E,
	 /** 
	capi2 api is CAPI2_SYS_SetMSPowerOnCause 
	**/
	MSG_SYSPARAM_SET_POWERON_CAUSE_REQ  = 0x410F,	///<Payload type {::CAPI2_SYS_SetMSPowerOnCause_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SYSPARAM_SET_POWERON_CAUSE_RSP  = 0x4110,
	 /** 
	capi2 api is CAPI2_TIMEZONE_DeleteNetworkName 
	**/
	MSG_TIMEZONE_DELETE_NW_NAME_REQ  = 0x4111,
	 /** 
	payload is ::default_proc 
	**/
	MSG_TIMEZONE_DELETE_NW_NAME_RSP  = 0x4112,
	 /** 
	capi2 api is CAPI2_TIMEZONE_GetTZUpdateMode 
	**/
	MSG_TIMEZONE_GET_UPDATE_MODE_REQ  = 0x4113,
	 /** 
	payload is ::TimeZoneUpdateMode_t 
	**/
	MSG_TIMEZONE_GET_UPDATE_MODE_RSP  = 0x4114,	///<Payload type {::TimeZoneUpdateMode_t}
	 /** 
	capi2 api is CAPI2_TIMEZONE_SetTZUpdateMode 
	**/
	MSG_TIMEZONE_SET_UPDATE_MODE_REQ  = 0x4115,	///<Payload type {::CAPI2_TIMEZONE_SetTZUpdateMode_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_TIMEZONE_SET_UPDATE_MODE_RSP  = 0x4116,
	 /** 
	capi2 api is CAPI2_TIMEZONE_UpdateRTC 
	**/
	MSG_TIMEZONE_UPDATE_RTC_REQ  = 0x4117,	///<Payload type {::CAPI2_TIMEZONE_UpdateRTC_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_TIMEZONE_UPDATE_RTC_RSP  = 0x4118,
	 /** 
	capi2 api is CAPI2_PMU_IsSIMReady 
	**/
	MSG_PMU_IS_SIM_READY_REQ  = 0x4119,
	 /** 
	payload is ::Boolean 
	**/
	MSG_PMU_IS_SIM_READY_RSP  = 0x411A,	///<Payload type {::Boolean}
	 /** 
	capi2 api is CAPI2_PMU_ActivateSIM 
	**/
	MSG_PMU_ACTIVATE_SIM_REQ  = 0x411B,	///<Payload type {::CAPI2_PMU_ActivateSIM_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_PMU_ACTIVATE_SIM_RSP  = 0x411C,
	 /** 
	capi2 api is CAPI2_PMU_DeactivateSIM 
	**/
	MSG_PMU_DEACTIVATE_SIM_REQ  = 0x411D,
	 /** 
	payload is ::default_proc 
	**/
	MSG_PMU_DEACTIVATE_SIM_RSP  = 0x411E,
	 /** 
	capi2 api is CAPI2_TestCmds 
	**/
	MSG_CAPI2_TEST_REQ  = 0x411F,	///<Payload type {::CAPI2_TestCmds_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_CAPI2_TEST_RSP  = 0x4120,
	 /** 
	capi2 api is CAPI2_SATK_SendPlayToneRes 
	**/
	MSG_STK_SEND_PLAYTONE_RES_REQ  = 0x4121,	///<Payload type {::CAPI2_SATK_SendPlayToneRes_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_STK_SEND_PLAYTONE_RES_RSP  = 0x4122,
	 /** 
	capi2 api is CAPI2_SATK_SendSetupCallRes 
	**/
	MSG_STK_SETUP_CALL_RES_REQ  = 0x4123,	///<Payload type {::CAPI2_SATK_SendSetupCallRes_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_STK_SETUP_CALL_RES_RSP  = 0x4124,
	 /** 
	capi2 api is CAPI2_PBK_SetFdnCheck 
	**/
	MSG_PBK_SET_FDN_CHECK_REQ  = 0x4125,	///<Payload type {::CAPI2_PBK_SetFdnCheck_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_PBK_SET_FDN_CHECK_RSP  = 0x4126,
	 /** 
	capi2 api is CAPI2_PBK_GetFdnCheck 
	**/
	MSG_PBK_GET_FDN_CHECK_REQ  = 0x4127,
	 /** 
	payload is ::Boolean 
	**/
	MSG_PBK_GET_FDN_CHECK_RSP  = 0x4128,	///<Payload type {::Boolean}
	 /** 
	capi2 api is CAPI2_GPIO_Set_High_64Pin 
	**/
	MSG_GPIO_SET_HIGH_64PIN_REQ  = 0x4129,	///<Payload type {::CAPI2_GPIO_Set_High_64Pin_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_GPIO_SET_HIGH_64PIN_RSP  = 0x412A,
	 /** 
	capi2 api is CAPI2_GPIO_Set_Low_64Pin 
	**/
	MSG_GPIO_SET_LOW_64PIN_REQ  = 0x412B,	///<Payload type {::CAPI2_GPIO_Set_Low_64Pin_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_GPIO_SET_LOW_64PIN_RSP  = 0x412C,
	 /** 
	capi2 api is CAPI2_PMU_StartCharging 
	**/
	MSG_PMU_START_CHARGING_REQ  = 0x412D,
	 /** 
	payload is ::default_proc 
	**/
	MSG_PMU_START_CHARGING_RSP  = 0x412E,
	 /** 
	capi2 api is CAPI2_PMU_StopCharging 
	**/
	MSG_PMU_STOP_CHARGING_REQ  = 0x412F,
	 /** 
	payload is ::default_proc 
	**/
	MSG_PMU_STOP_CHARGING_RSP  = 0x4130,
	 /** 
	capi2 api is CAPI2_PMU_BattADCReq 
	**/
	MSG_PMU_BATT_LEVEL_REQ  = 0x4131,
	 /** 
	payload is ::default_proc 
	**/
	MSG_PMU_BATT_LEVEL_RSP  = 0x4132,
	 /** 
	capi2 api is CAPI2_PMU_Battery_Register 
	**/
	MSG_PMU_BATT_LEVEL_REGISTER_REQ  = 0x4133,	///<Payload type {::CAPI2_PMU_Battery_Register_Req_t}
	 /** 
	payload is ::HAL_EM_BATTMGR_Result_en_t 
	**/
	MSG_PMU_BATT_LEVEL_REGISTER_RSP  = 0x4134,	///<Payload type {::HAL_EM_BATTMGR_Result_en_t}
	 /** 
	payload is ::HAL_EM_BatteryLevel_t 
	**/
	MSG_PMU_BATT_LEVEL_IND  = 0x4135,	///<Payload type {::HAL_EM_BatteryLevel_t}
	 /** 
	capi2 api is CAPI2_SMS_IsMeStorageEnabled 
	**/
	MSG_SMS_ISMESTORAGEENABLED_REQ  = 0x4136,
	 /** 
	payload is ::default_proc 
	**/
	MSG_SMS_ISMESTORAGEENABLED_RSP  = 0x4137,
	 /** 
	capi2 api is CAPI2_SMS_SendMemAvailInd 
	**/
	MSG_SMS_SEND_MEM_AVAL_IND_REQ  = 0x4138,
	 /** 
	payload is ::default_proc 
	**/
	MSG_SMS_SEND_MEM_AVAL_IND_RSP  = 0x4139,
	 /** 
	capi2 api is CAPI2_SMS_GetMaxMeCapacity 
	**/
	MSG_SMS_GETMAXMECAPACITY_REQ  = 0x413A,
	 /** 
	payload is ::default_proc 
	**/
	MSG_SMS_GETMAXMECAPACITY_RSP  = 0x413B,
	 /** 
	capi2 api is CAPI2_SMS_GetNextFreeSlot 
	**/
	MSG_SMS_GETNEXTFREESLOT_REQ  = 0x413C,
	 /** 
	payload is ::default_proc 
	**/
	MSG_SMS_GETNEXTFREESLOT_RSP  = 0x413D,
	 /** 
	capi2 api is CAPI2_SMS_SetMeSmsStatus 
	**/
	MSG_SMS_SETMESMSSTATUS_REQ  = 0x413E,	///<Payload type {::CAPI2_SMS_SetMeSmsStatus_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SMS_SETMESMSSTATUS_RSP  = 0x413F,
	 /** 
	capi2 api is CAPI2_SMS_GetMeSmsStatus 
	**/
	MSG_SMS_GETMESMSSTATUS_REQ  = 0x4140,	///<Payload type {::CAPI2_SMS_GetMeSmsStatus_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SMS_GETMESMSSTATUS_RSP  = 0x4141,
	 /** 
	capi2 api is CAPI2_SMS_StoreSmsToMe 
	**/
	MSG_SMS_STORESMSTOME_REQ  = 0x4142,	///<Payload type {::CAPI2_SMS_StoreSmsToMe_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SMS_STORESMSTOME_RSP  = 0x4143,
	 /** 
	capi2 api is CAPI2_SMS_RetrieveSmsFromMe 
	**/
	MSG_SMS_RETRIEVESMSFROMME_REQ  = 0x4144,	///<Payload type {::CAPI2_SMS_RetrieveSmsFromMe_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SMS_RETRIEVESMSFROMME_RSP  = 0x4145,
	 /** 
	capi2 api is CAPI2_SMS_RemoveSmsFromMe 
	**/
	MSG_SMS_REMOVESMSFROMME_REQ  = 0x4146,	///<Payload type {::CAPI2_SMS_RemoveSmsFromMe_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SMS_REMOVESMSFROMME_RSP  = 0x4147,
	 /** 
	capi2 api is CAPI2_SMS_ConfigureMEStorage 
	**/
	MSG_SMS_CONFIGUREMESTORAGE_REQ  = 0x4148,	///<Payload type {::CAPI2_SMS_ConfigureMEStorage_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SMS_CONFIGUREMESTORAGE_RSP  = 0x4149,
	 /** 
	capi2 api is CAPI2_MS_SetElement 
	**/
	MSG_MS_SET_ELEMENT_REQ  = 0x414A,	///<Payload type {::CAPI2_MS_SetElement_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_MS_SET_ELEMENT_RSP  = 0x414B,
	 /** 
	capi2 api is CAPI2_MS_GetElement 
	**/
	MSG_MS_GET_ELEMENT_REQ  = 0x414C,	///<Payload type {::CAPI2_MS_GetElement_Req_t}
	 /** 
	payload is ::CAPI2_MS_Element_t 
	**/
	MSG_MS_GET_ELEMENT_RSP  = 0x414D,	///<Payload type {::CAPI2_MS_Element_t}
	 /** 
	capi2 api is CAPI2_USIM_IsApplicationSupported 
	**/
	MSG_USIM_IS_APP_SUPPORTED_REQ  = 0x414E,	///<Payload type {::CAPI2_USIM_IsApplicationSupported_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_USIM_IS_APP_SUPPORTED_RSP  = 0x414F,	///<Payload type {::Boolean}
	 /** 
	capi2 api is CAPI2_USIM_IsAllowedAPN 
	**/
	MSG_USIM_IS_APN_ALLOWED_REQ  = 0x4150,	///<Payload type {::CAPI2_USIM_IsAllowedAPN_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_USIM_IS_APN_ALLOWED_RSP  = 0x4151,	///<Payload type {::Boolean}
	 /** 
	capi2 api is CAPI2_USIM_GetNumOfAPN 
	**/
	MSG_USIM_GET_NUM_APN_REQ  = 0x4152,
	 /** 
	payload is ::UInt8 
	**/
	MSG_USIM_GET_NUM_APN_RSP  = 0x4153,	///<Payload type {::UInt8}
	 /** 
	capi2 api is CAPI2_USIM_GetAPNEntry 
	**/
	MSG_USIM_GET_APN_ENTRY_REQ  = 0x4154,	///<Payload type {::CAPI2_USIM_GetAPNEntry_Req_t}
	 /** 
	payload is ::APN_NAME_t 
	**/
	MSG_USIM_GET_APN_ENTRY_RSP  = 0x4155,	///<Payload type {::APN_NAME_t}
	 /** 
	capi2 api is CAPI2_USIM_IsEstServActivated 
	**/
	MSG_USIM_IS_EST_SERV_ACTIVATED_REQ  = 0x4156,	///<Payload type {::CAPI2_USIM_IsEstServActivated_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_USIM_IS_EST_SERV_ACTIVATED_RSP  = 0x4157,	///<Payload type {::Boolean}
	 /** 
	capi2 api is CAPI2_USIM_SendSetEstServReq 
	**/
	MSG_SIM_SET_EST_SERV_REQ  = 0x4158,	///<Payload type {::CAPI2_USIM_SendSetEstServReq_Req_t}
	 /** 
	capi2 api is CAPI2_USIM_SendWriteAPNReq 
	**/
	MSG_SIM_UPDATE_ONE_APN_REQ  = 0x4159,	///<Payload type {::CAPI2_USIM_SendWriteAPNReq_Req_t}
	 /** 
	capi2 api is CAPI2_USIM_SendDeleteAllAPNReq 
	**/
	MSG_SIM_DELETE_ALL_APN_REQ  = 0x415A,
	 /** 
	capi2 api is CAPI2_USIM_GetRatModeSetting 
	**/
	MSG_USIM_GET_RAT_MODE_REQ  = 0x415B,
	 /** 
	payload is ::USIM_RAT_MODE_t 
	**/
	MSG_USIM_GET_RAT_MODE_RSP  = 0x415C,	///<Payload type {::USIM_RAT_MODE_t}
	 /** 
	capi2 api is CAPI2_PMU_ClientPowerDown 
	**/
	MSG_PMU_CLIENT_POWER_DOWN_REQ  = 0x415D,
	 /** 
	payload is ::Boolean 
	**/
	MSG_PMU_CLIENT_POWER_DOWN_RSP  = 0x415E,	///<Payload type {::Boolean}
	 /** 
	capi2 api is CAPI2_PMU_GetPowerupCause 
	**/
	MSG_PMU_GET_POWERUP_CAUSE_REQ  = 0x415F,
	 /** 
	payload is ::PMU_PowerupId_t 
	**/
	MSG_PMU_GET_POWERUP_CAUSE_RSP  = 0x4160,	///<Payload type {::PMU_PowerupId_t}
	 /** 
	capi2 api is CAPI2_MS_GetGPRSRegState 
	**/
	MSG_MS_GET_GPRS_STATE_REQ  = 0x4161,
	 /** 
	payload is ::MSRegState_t 
	**/
	MSG_MS_GET_GPRS_STATE_RSP  = 0x4162,	///<Payload type {::MSRegState_t}
	 /** 
	capi2 api is CAPI2_MS_GetGSMRegState 
	**/
	MSG_MS_GET_GSM_STATE_REQ  = 0x4163,
	 /** 
	payload is ::MSRegState_t 
	**/
	MSG_MS_GET_GSM_STATE_RSP  = 0x4164,	///<Payload type {::MSRegState_t}
	 /** 
	capi2 api is CAPI2_MS_GetRegisteredCellInfo 
	**/
	MSG_MS_GET_CELL_INFO_REQ  = 0x4165,
	 /** 
	payload is ::CellInfo_t 
	**/
	MSG_MS_GET_CELL_INFO_RSP  = 0x4166,	///<Payload type {::CellInfo_t}
	 /** 
	capi2 api is CAPI2_MS_GetStartBand 
	**/
	MSG_MS_GET_START_BAND_REQ  = 0x4167,
	 /** 
	payload is ::UInt8 
	**/
	MSG_MS_GET_START_BAND_RSP  = 0x4168,	///<Payload type {::UInt8}
	 /** 
	capi2 api is CAPI2_MS_SetMEPowerClass 
	**/
	MSG_MS_SETMEPOWER_CLASS_REQ  = 0x4169,	///<Payload type {::CAPI2_MS_SetMEPowerClass_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_MS_SETMEPOWER_CLASS_RSP  = 0x416A,
	 /** 
	capi2 api is CAPI2_USIM_GetServiceStatus 
	**/
	MSG_USIM_GET_SERVICE_STATUS_REQ  = 0x416B,	///<Payload type {::CAPI2_USIM_GetServiceStatus_Req_t}
	 /** 
	payload is ::SIMServiceStatus_t 
	**/
	MSG_USIM_GET_SERVICE_STATUS_RSP  = 0x416C,	///<Payload type {::SIMServiceStatus_t}
	 /** 
	capi2 api is CAPI2_SIM_IsAllowedAPN 
	**/
	MSG_SIM_IS_ALLOWED_APN_REQ  = 0x416D,	///<Payload type {::CAPI2_SIM_IsAllowedAPN_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_IS_ALLOWED_APN_RSP  = 0x416E,	///<Payload type {::Boolean}
	 /** 
	capi2 api is CAPI2_SMS_GetSmsMaxCapacity 
	**/
	MSG_SMS_GETSMSMAXCAPACITY_REQ  = 0x416F,	///<Payload type {::CAPI2_SMS_GetSmsMaxCapacity_Req_t}
	 /** 
	payload is ::UInt16 
	**/
	MSG_SMS_GETSMSMAXCAPACITY_RSP  = 0x4170,	///<Payload type {::UInt16}
	 /** 
	capi2 api is CAPI2_SMS_RetrieveMaxCBChnlLength 
	**/
	MSG_SMS_RETRIEVEMAXCBCHNLLENGTH_REQ  = 0x4171,
	 /** 
	payload is ::UInt8 
	**/
	MSG_SMS_RETRIEVEMAXCBCHNLLENGTH_RSP  = 0x4172,	///<Payload type {::UInt8}
	 /** 
	capi2 api is CAPI2_SIM_IsBdnOperationRestricted 
	**/
	MSG_SIM_IS_BDN_RESTRICTED_REQ  = 0x4173,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_IS_BDN_RESTRICTED_RSP  = 0x4174,	///<Payload type {::Boolean}
	 /** 
	capi2 api is CAPI2_SIM_SendPreferredPlmnUpdateInd 
	**/
	MSG_SIM_SEND_PLMN_UPDATE_IND_REQ  = 0x4175,	///<Payload type {::CAPI2_SIM_SendPreferredPlmnUpdateInd_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SIM_SEND_PLMN_UPDATE_IND_RSP  = 0x4176,
	 /** 
	capi2 api is CAPI2_SIM_Deactive 
	**/
	MSG_SIM_DEACTIVATE_REQ  = 0x4177,
	 /** 
	payload is ::default_proc 
	**/
	MSG_SIM_DEACTIVATE_RSP  = 0x4178,
	 /** 
	capi2 api is CAPI2_SIM_SendSetBdnReq 
	**/
	MSG_SIM_SET_BDN_REQ  = 0x4179,	///<Payload type {::CAPI2_SIM_SendSetBdnReq_Req_t}
	 /** 
	capi2 api is CAPI2_SIM_PowerOnOffCard 
	**/
	MSG_SIM_POWER_ON_OFF_CARD_REQ  = 0x417A,	///<Payload type {::CAPI2_SIM_PowerOnOffCard_Req_t}
	 /** 
	capi2 api is CAPI2_SIM_GetRawAtr 
	**/
	MSG_SIM_GET_RAW_ATR_REQ  = 0x417B,
	 /** 
	capi2 api is CAPI2_SIM_Set_Protocol 
	**/
	MSG_SIM_SET_PROTOCOL_REQ  = 0x417C,	///<Payload type {::CAPI2_SIM_Set_Protocol_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SIM_SET_PROTOCOL_RSP  = 0x417D,	///<Payload type {::Result_t}
	 /** 
	capi2 api is CAPI2_SIM_Get_Protocol 
	**/
	MSG_SIM_GET_PROTOCOL_REQ  = 0x417E,
	 /** 
	payload is ::UInt8 
	**/
	MSG_SIM_GET_PROTOCOL_RSP  = 0x417F,	///<Payload type {::UInt8}
	 /** 
	capi2 api is CAPI2_SIM_SendGenericApduCmd 
	**/
	MSG_SIM_SEND_GENERIC_APDU_CMD_REQ  = 0x4180,	///<Payload type {::CAPI2_SIM_SendGenericApduCmd_Req_t}
	 /** 
	capi2 api is CAPI2_SIM_TerminateXferApdu 
	**/
	MSG_SIM_TERMINATE_XFER_APDU_REQ  = 0x4181,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SIM_TERMINATE_XFER_APDU_RSP  = 0x4182,	///<Payload type {::Result_t}
	 /** 
	capi2 api is CAPI2_MS_SetPlmnSelectRat 
	**/
	MSG_SET_PLMN_SELECT_RAT_REQ  = 0x4183,	///<Payload type {::CAPI2_MS_SetPlmnSelectRat_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SET_PLMN_SELECT_RAT_RSP  = 0x4184,
	 /** 
	capi2 api is CAPI2_MS_IsDeRegisterInProgress 
	**/
	MSG_IS_DEREGISTER_IN_PROGRESS_REQ  = 0x4185,
	 /** 
	payload is ::Boolean 
	**/
	MSG_IS_DEREGISTER_IN_PROGRESS_RSP  = 0x4186,	///<Payload type {::Boolean}
	 /** 
	capi2 api is CAPI2_MS_IsRegisterInProgress 
	**/
	MSG_IS_REGISTER_IN_PROGRESS_REQ  = 0x4187,
	 /** 
	payload is ::Boolean 
	**/
	MSG_IS_REGISTER_IN_PROGRESS_RSP  = 0x4188,	///<Payload type {::Boolean}
	 /** 
	capi2 api is CAPI2_SOCKET_Open 
	**/
	MSG_SOCKET_OPEN_REQ  = 0x4189,	///<Payload type {::CAPI2_SOCKET_Open_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SOCKET_OPEN_RSP  = 0x418A,
	 /** 
	capi2 api is CAPI2_SOCKET_Bind 
	**/
	MSG_SOCKET_BIND_REQ  = 0x418B,	///<Payload type {::CAPI2_SOCKET_Bind_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SOCKET_BIND_RSP  = 0x418C,
	 /** 
	capi2 api is CAPI2_SOCKET_Listen 
	**/
	MSG_SOCKET_LISTEN_REQ  = 0x418D,	///<Payload type {::CAPI2_SOCKET_Listen_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SOCKET_LISTEN_RSP  = 0x418E,
	 /** 
	capi2 api is CAPI2_SOCKET_Accept 
	**/
	MSG_SOCKET_ACCEPT_REQ  = 0x418F,	///<Payload type {::CAPI2_SOCKET_Accept_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SOCKET_ACCEPT_RSP  = 0x4190,
	 /** 
	capi2 api is CAPI2_SOCKET_Connect 
	**/
	MSG_SOCKET_CONNECT_REQ  = 0x4191,	///<Payload type {::CAPI2_SOCKET_Connect_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SOCKET_CONNECT_RSP  = 0x4192,
	 /** 
	capi2 api is CAPI2_SOCKET_GetPeerName 
	**/
	MSG_SOCKET_GETPEERNAME_REQ  = 0x4193,	///<Payload type {::CAPI2_SOCKET_GetPeerName_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SOCKET_GETPEERNAME_RSP  = 0x4194,
	 /** 
	capi2 api is CAPI2_SOCKET_GetSockName 
	**/
	MSG_SOCKET_GETSOCKNAME_REQ  = 0x4195,	///<Payload type {::CAPI2_SOCKET_GetSockName_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SOCKET_GETSOCKNAME_RSP  = 0x4196,
	 /** 
	capi2 api is CAPI2_SOCKET_SetSockOpt 
	**/
	MSG_SOCKET_SETSOCKOPT_REQ  = 0x4197,	///<Payload type {::CAPI2_SOCKET_SetSockOpt_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SOCKET_SETSOCKOPT_RSP  = 0x4198,
	 /** 
	capi2 api is CAPI2_SOCKET_GetSockOpt 
	**/
	MSG_SOCKET_GETSOCKOPT_REQ  = 0x4199,	///<Payload type {::CAPI2_SOCKET_GetSockOpt_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SOCKET_GETSOCKOPT_RSP  = 0x419A,
	 /** 
	capi2 api is CAPI2_SOCKET_SignalInd 
	**/
	MSG_CAPI2_SOCKET_SIGNAL_REQ  = 0x419B,	///<Payload type {::CAPI2_SOCKET_SignalInd_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_CAPI2_SOCKET_SIGNAL_RSP  = 0x419C,
	 /** 
	capi2 api is CAPI2_SOCKET_Send 
	**/
	MSG_SOCKET_SEND_REQ  = 0x419D,	///<Payload type {::CAPI2_SOCKET_Send_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SOCKET_SEND_RSP  = 0x419E,
	 /** 
	capi2 api is CAPI2_SOCKET_SendTo 
	**/
	MSG_SOCKET_SEND_TO_REQ  = 0x419F,	///<Payload type {::CAPI2_SOCKET_SendTo_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SOCKET_SEND_TO_RSP  = 0x41A0,
	 /** 
	capi2 api is CAPI2_SOCKET_Recv 
	**/
	MSG_SOCKET_RECV_REQ  = 0x41A1,	///<Payload type {::CAPI2_SOCKET_Recv_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SOCKET_RECV_RSP  = 0x41A2,
	 /** 
	capi2 api is CAPI2_SOCKET_RecvFrom 
	**/
	MSG_SOCKET_RECV_FROM_REQ  = 0x41A3,	///<Payload type {::CAPI2_SOCKET_RecvFrom_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SOCKET_RECV_FROM_RSP  = 0x41A4,
	 /** 
	capi2 api is CAPI2_SOCKET_Close 
	**/
	MSG_SOCKET_CLOSE_REQ  = 0x41A5,	///<Payload type {::CAPI2_SOCKET_Close_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SOCKET_CLOSE_RSP  = 0x41A6,
	 /** 
	capi2 api is CAPI2_SOCKET_Shutdown 
	**/
	MSG_SOCKET_SHUTDOWN_REQ  = 0x41A7,	///<Payload type {::CAPI2_SOCKET_Shutdown_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SOCKET_SHUTDOWN_RSP  = 0x41A8,
	 /** 
	capi2 api is CAPI2_SOCKET_Errno 
	**/
	MSG_SOCKET_ERRNO_REQ  = 0x41A9,	///<Payload type {::CAPI2_SOCKET_Errno_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SOCKET_ERRNO_RSP  = 0x41AA,
	 /** 
	capi2 api is CAPI2_SOCKET_SO2LONG 
	**/
	MSG_SOCKET_SO2LONG_REQ  = 0x41AB,	///<Payload type {::CAPI2_SOCKET_SO2LONG_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SOCKET_SO2LONG_RSP  = 0x41AC,
	 /** 
	capi2 api is CAPI2_SOCKET_GetSocketSendBufferSpace 
	**/
	MSG_SOCKET_GET_SOCKET_SEND_BUFFER_SPACE_REQ  = 0x41AD,	///<Payload type {::CAPI2_SOCKET_GetSocketSendBufferSpace_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SOCKET_GET_SOCKET_SEND_BUFFER_SPACE_RSP  = 0x41AE,
	 /** 
	capi2 api is CAPI2_SOCKET_ParseIPAddr 
	**/
	MSG_SOCKET_PARSE_IPAD_REQ  = 0x41AF,	///<Payload type {::CAPI2_SOCKET_ParseIPAddr_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SOCKET_PARSE_IPAD_RSP  = 0x41B0,
	 /** 
	capi2 api is CAPI2_DC_SetupDataConnection 
	**/
	MSG_SETUP_DATA_CONNECTION_REQ  = 0x41B1,	///<Payload type {::CAPI2_DC_SetupDataConnection_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SETUP_DATA_CONNECTION_RSP  = 0x41B2,
	 /** 
	capi2 api is CAPI2_DC_SetupDataConnectionEx 
	**/
	MSG_SETUP_DATA_CONNECTION_EX_REQ  = 0x41B3,	///<Payload type {::CAPI2_DC_SetupDataConnectionEx_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SETUP_DATA_CONNECTION_EX_RSP  = 0x41B4,
	 /** 
	capi2 api is CAPI2_DC_ReportCallStatusInd 
	**/
	MSG_DC_REPORT_CALL_STATUS_REQ  = 0x41B5,	///<Payload type {::CAPI2_DC_ReportCallStatusInd_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DC_REPORT_CALL_STATUS_RSP  = 0x41B6,
	 /** 
	capi2 api is CAPI2_DC_ShutdownDataConnection 
	**/
	MSG_DC_SHUTDOWN_DATA_CONNECTION_REQ  = 0x41B7,	///<Payload type {::CAPI2_DC_ShutdownDataConnection_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DC_SHUTDOWN_DATA_CONNECTION_RSP  = 0x41B8,
	 /** 
	capi2 api is CAPI2_DATA_IsAcctIDValid 
	**/
	MSG_DATA_IS_ACCT_ID_VALID_REQ  = 0x41B9,	///<Payload type {::CAPI2_DATA_IsAcctIDValid_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_IS_ACCT_ID_VALID_RSP  = 0x41BA,
	 /** 
	capi2 api is CAPI2_DATA_CreateGPRSDataAcct 
	**/
	MSG_DATA_CREATE_GPRS_ACCT_REQ  = 0x41BB,	///<Payload type {::CAPI2_DATA_CreateGPRSDataAcct_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_CREATE_GPRS_ACCT_RSP  = 0x41BC,
	 /** 
	capi2 api is CAPI2_DATA_CreateCSDDataAcct 
	**/
	MSG_DATA_CREATE_GSM_ACCT_REQ  = 0x41BD,	///<Payload type {::CAPI2_DATA_CreateCSDDataAcct_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_CREATE_GSM_ACCT_RSP  = 0x41BE,
	 /** 
	capi2 api is CAPI2_DATA_DeleteDataAcct 
	**/
	MSG_DATA_DELETE_ACCT_REQ  = 0x41BF,	///<Payload type {::CAPI2_DATA_DeleteDataAcct_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_DELETE_ACCT_RSP  = 0x41C0,
	 /** 
	capi2 api is CAPI2_DATA_SetUsername 
	**/
	MSG_DATA_SET_USERNAME_REQ  = 0x41C1,	///<Payload type {::CAPI2_DATA_SetUsername_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_SET_USERNAME_RSP  = 0x41C2,
	 /** 
	capi2 api is CAPI2_DATA_GetUsername 
	**/
	MSG_DATA_GET_USERNAME_REQ  = 0x41C3,	///<Payload type {::CAPI2_DATA_GetUsername_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_GET_USERNAME_RSP  = 0x41C4,
	 /** 
	capi2 api is CAPI2_DATA_SetPassword 
	**/
	MSG_DATA_SET_PASSWORD_REQ  = 0x41C5,	///<Payload type {::CAPI2_DATA_SetPassword_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_SET_PASSWORD_RSP  = 0x41C6,
	 /** 
	capi2 api is CAPI2_DATA_GetPassword 
	**/
	MSG_DATA_GET_PASSWORD_REQ  = 0x41C7,	///<Payload type {::CAPI2_DATA_GetPassword_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_GET_PASSWORD_RSP  = 0x41C8,
	 /** 
	capi2 api is CAPI2_DATA_SetStaticIPAddr 
	**/
	MSG_DATA_SET_STATIC_IP_REQ  = 0x41C9,	///<Payload type {::CAPI2_DATA_SetStaticIPAddr_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_SET_STATIC_IP_RSP  = 0x41CA,
	 /** 
	capi2 api is CAPI2_DATA_GetStaticIPAddr 
	**/
	MSG_DATA_GET_STATIC_IP_REQ  = 0x41CB,	///<Payload type {::CAPI2_DATA_GetStaticIPAddr_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_GET_STATIC_IP_RSP  = 0x41CC,
	 /** 
	capi2 api is CAPI2_DATA_SetPrimaryDnsAddr 
	**/
	MSG_DATA_SET_PRIMARY_DNS_ADDR_REQ  = 0x41CD,	///<Payload type {::CAPI2_DATA_SetPrimaryDnsAddr_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_SET_PRIMARY_DNS_ADDR_RSP  = 0x41CE,
	 /** 
	capi2 api is CAPI2_DATA_GetPrimaryDnsAddr 
	**/
	MSG_DATA_GET_PRIMARY_DNS_ADDR_REQ  = 0x41CF,	///<Payload type {::CAPI2_DATA_GetPrimaryDnsAddr_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_GET_PRIMARY_DNS_ADDR_RSP  = 0x41D0,
	 /** 
	capi2 api is CAPI2_DATA_SetSecondDnsAddr 
	**/
	MSG_DATA_SET_SECOND_DNS_ADDR_REQ  = 0x41D1,	///<Payload type {::CAPI2_DATA_SetSecondDnsAddr_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_SET_SECOND_DNS_ADDR_RSP  = 0x41D2,
	 /** 
	capi2 api is CAPI2_DATA_GetSecondDnsAddr 
	**/
	MSG_DATA_GET_SECOND_DNS_ADDR_REQ  = 0x41D3,	///<Payload type {::CAPI2_DATA_GetSecondDnsAddr_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_GET_SECOND_DNS_ADDR_RSP  = 0x41D4,
	 /** 
	capi2 api is CAPI2_DATA_SetDataCompression 
	**/
	MSG_DATA_SET_DATA_COMPRESSION_REQ  = 0x41D5,	///<Payload type {::CAPI2_DATA_SetDataCompression_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_SET_DATA_COMPRESSION_RSP  = 0x41D6,
	 /** 
	capi2 api is CAPI2_DATA_GetDataCompression 
	**/
	MSG_DATA_GET_DATA_COMPRESSION_REQ  = 0x41D7,	///<Payload type {::CAPI2_DATA_GetDataCompression_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_GET_DATA_COMPRESSION_RSP  = 0x41D8,
	 /** 
	capi2 api is CAPI2_DATA_GetAcctType 
	**/
	MSG_DATA_GET_ACCT_TYPE_REQ  = 0x41D9,	///<Payload type {::CAPI2_DATA_GetAcctType_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_GET_ACCT_TYPE_RSP  = 0x41DA,
	 /** 
	capi2 api is CAPI2_DATA_GetEmptyAcctSlot 
	**/
	MSG_DATA_GET_EMPTY_ACCT_SLOT_REQ  = 0x41DB,
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_GET_EMPTY_ACCT_SLOT_RSP  = 0x41DC,
	 /** 
	capi2 api is CAPI2_DATA_GetCidFromDataAcctID 
	**/
	MSG_DATA_GET_CID_FROM_ACCTID_REQ  = 0x41DD,	///<Payload type {::CAPI2_DATA_GetCidFromDataAcctID_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_GET_CID_FROM_ACCTID_RSP  = 0x41DE,
	 /** 
	capi2 api is CAPI2_DATA_GetDataAcctIDFromCid 
	**/
	MSG_CAPI2_DATA_GET_ACCTID_FROM_CID_REQ  = 0x41DF,	///<Payload type {::CAPI2_DATA_GetDataAcctIDFromCid_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_CAPI2_DATA_GET_ACCTID_FROM_CID_RSP  = 0x41E0,
	 /** 
	capi2 api is CAPI2_DATA_GetPrimaryCidFromDataAcctID 
	**/
	MSG_DATA_GET_PRI_FROM_ACCTID_REQ  = 0x41E1,	///<Payload type {::CAPI2_DATA_GetPrimaryCidFromDataAcctID_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_GET_PRI_FROM_ACCTID_RSP  = 0x41E2,
	 /** 
	capi2 api is CAPI2_DATA_IsSecondaryDataAcct 
	**/
	MSG_DATA_IS_SND_DATA_ACCT_REQ  = 0x41E3,	///<Payload type {::CAPI2_DATA_IsSecondaryDataAcct_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_IS_SND_DATA_ACCT_RSP  = 0x41E4,
	 /** 
	capi2 api is CAPI2_DATA_GetDataSentSize 
	**/
	MSG_DATA_GET_ACCTID_FROM_CID_REQ  = 0x41E5,	///<Payload type {::CAPI2_DATA_GetDataSentSize_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_GET_ACCTID_FROM_CID_RSP  = 0x41E6,
	 /** 
	capi2 api is CAPI2_DATA_GetDataRcvSize 
	**/
	MSG_DATA_GET_DATA_RCV_SIZE_REQ  = 0x41E7,	///<Payload type {::CAPI2_DATA_GetDataRcvSize_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_GET_DATA_RCV_SIZE_RSP  = 0x41E8,
	 /** 
	capi2 api is CAPI2_DATA_SetGPRSPdpType 
	**/
	MSG_DATA_SET_GPRS_PDP_TYPE_REQ  = 0x41E9,	///<Payload type {::CAPI2_DATA_SetGPRSPdpType_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_SET_GPRS_PDP_TYPE_RSP  = 0x41EA,
	 /** 
	capi2 api is CAPI2_DATA_GetGPRSPdpType 
	**/
	MSG_DATA_GET_GPRS_PDP_TYPE_REQ  = 0x41EB,	///<Payload type {::CAPI2_DATA_GetGPRSPdpType_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_GET_GPRS_PDP_TYPE_RSP  = 0x41EC,
	 /** 
	capi2 api is CAPI2_DATA_SetGPRSApn 
	**/
	MSG_DATA_SET_GPRS_APN_REQ  = 0x41ED,	///<Payload type {::CAPI2_DATA_SetGPRSApn_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_SET_GPRS_APN_RSP  = 0x41EE,
	 /** 
	capi2 api is CAPI2_DATA_GetGPRSApn 
	**/
	MSG_DATA_GET_GPRS_APN_REQ  = 0x41EF,	///<Payload type {::CAPI2_DATA_GetGPRSApn_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_GET_GPRS_APN_RSP  = 0x41F0,
	 /** 
	capi2 api is CAPI2_DATA_SetAuthenMethod 
	**/
	MSG_DATA_SET_AUTHEN_METHOD_REQ  = 0x41F1,	///<Payload type {::CAPI2_DATA_SetAuthenMethod_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_SET_AUTHEN_METHOD_RSP  = 0x41F2,
	 /** 
	capi2 api is CAPI2_DATA_GetAuthenMethod 
	**/
	MSG_DATA_GET_AUTHEN_METHOD_REQ  = 0x41F3,	///<Payload type {::CAPI2_DATA_GetAuthenMethod_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_GET_AUTHEN_METHOD_RSP  = 0x41F4,
	 /** 
	capi2 api is CAPI2_DATA_SetGPRSHeaderCompression 
	**/
	MSG_DATA_SET_GPRS_HEADER_COMPRESSION_REQ  = 0x41F5,	///<Payload type {::CAPI2_DATA_SetGPRSHeaderCompression_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_SET_GPRS_HEADER_COMPRESSION_RSP  = 0x41F6,
	 /** 
	capi2 api is CAPI2_DATA_GetGPRSHeaderCompression 
	**/
	MSG_DATA_GET_GPRS_HEADER_COMPRESSION_REQ  = 0x41F7,	///<Payload type {::CAPI2_DATA_GetGPRSHeaderCompression_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_GET_GPRS_HEADER_COMPRESSION_RSP  = 0x41F8,
	 /** 
	capi2 api is CAPI2_DATA_SetGPRSQos 
	**/
	MSG_DATA_SET_GPRS_QOS_REQ  = 0x41F9,	///<Payload type {::CAPI2_DATA_SetGPRSQos_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_SET_GPRS_QOS_RSP  = 0x41FA,
	 /** 
	capi2 api is CAPI2_DATA_GetGPRSQos 
	**/
	MSG_CAPI2_DATA_GET_GPRS_QOS_REQ  = 0x41FB,	///<Payload type {::CAPI2_DATA_GetGPRSQos_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_CAPI2_DATA_GET_GPRS_QOS_RSP  = 0x41FC,
	 /** 
	capi2 api is CAPI2_DATA_SetAcctLock 
	**/
	MSG_DATA_SET_ACCT_LOCK_REQ  = 0x41FD,	///<Payload type {::CAPI2_DATA_SetAcctLock_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_SET_ACCT_LOCK_RSP  = 0x41FE,
	 /** 
	capi2 api is CAPI2_DATA_GetAcctLock 
	**/
	MSG_DATA_GET_ACCT_LOCK_REQ  = 0x41FF,	///<Payload type {::CAPI2_DATA_GetAcctLock_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_GET_ACCT_LOCK_RSP  = 0x4200,
	 /** 
	capi2 api is CAPI2_DATA_SetGprsOnly 
	**/
	MSG_DATA_SET_GPRS_ONLY_REQ  = 0x4201,	///<Payload type {::CAPI2_DATA_SetGprsOnly_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_SET_GPRS_ONLY_RSP  = 0x4202,
	 /** 
	capi2 api is CAPI2_DATA_GetGprsOnly 
	**/
	MSG_DATA_GET_GPRS_ONLY_REQ  = 0x4203,	///<Payload type {::CAPI2_DATA_GetGprsOnly_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_GET_GPRS_ONLY_RSP  = 0x4204,
	 /** 
	capi2 api is CAPI2_DATA_SetGPRSTft 
	**/
	MSG_DATA_SET_GPRS_TFT_REQ  = 0x4205,	///<Payload type {::CAPI2_DATA_SetGPRSTft_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_SET_GPRS_TFT_RSP  = 0x4206,
	 /** 
	capi2 api is CAPI2_DATA_GetGPRSTft 
	**/
	MSG_DATA_GET_GPRS_TFT_REQ  = 0x4207,	///<Payload type {::CAPI2_DATA_GetGPRSTft_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_GET_GPRS_TFT_RSP  = 0x4208,
	 /** 
	capi2 api is CAPI2_DATA_SetCSDDialNumber 
	**/
	MSG_DATA_SET_CSD_DIAL_NUMBER_REQ  = 0x4209,	///<Payload type {::CAPI2_DATA_SetCSDDialNumber_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_SET_CSD_DIAL_NUMBER_RSP  = 0x420A,
	 /** 
	capi2 api is CAPI2_DATA_GetCSDDialNumber 
	**/
	MSG_DATA_GET_CSD_DIAL_NUMBER_REQ  = 0x420B,	///<Payload type {::CAPI2_DATA_GetCSDDialNumber_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_GET_CSD_DIAL_NUMBER_RSP  = 0x420C,
	 /** 
	capi2 api is CAPI2_DATA_SetCSDDialType 
	**/
	MSG_DATA_SET_CSD_DIAL_TYPE_REQ  = 0x420D,	///<Payload type {::CAPI2_DATA_SetCSDDialType_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_SET_CSD_DIAL_TYPE_RSP  = 0x420E,
	 /** 
	capi2 api is CAPI2_DATA_GetCSDDialType 
	**/
	MSG_DATA_GET_CSD_DIAL_TYPE_REQ  = 0x420F,	///<Payload type {::CAPI2_DATA_GetCSDDialType_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_GET_CSD_DIAL_TYPE_RSP  = 0x4210,
	 /** 
	capi2 api is CAPI2_DATA_SetCSDBaudRate 
	**/
	MSG_DATA_SET_CSD_BAUD_RATE_REQ  = 0x4211,	///<Payload type {::CAPI2_DATA_SetCSDBaudRate_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_SET_CSD_BAUD_RATE_RSP  = 0x4212,
	 /** 
	capi2 api is CAPI2_DATA_GetCSDBaudRate 
	**/
	MSG_DATA_GET_CSD_BAUD_RATE_REQ  = 0x4213,	///<Payload type {::CAPI2_DATA_GetCSDBaudRate_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_GET_CSD_BAUD_RATE_RSP  = 0x4214,
	 /** 
	capi2 api is CAPI2_DATA_SetCSDSyncType 
	**/
	MSG_DATA_SET_CSD_SYNC_TYPE_REQ  = 0x4215,	///<Payload type {::CAPI2_DATA_SetCSDSyncType_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_SET_CSD_SYNC_TYPE_RSP  = 0x4216,
	 /** 
	capi2 api is CAPI2_DATA_GetCSDSyncType 
	**/
	MSG_DATA_GET_CSD_SYNC_TYPE_REQ  = 0x4217,	///<Payload type {::CAPI2_DATA_GetCSDSyncType_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_GET_CSD_SYNC_TYPE_RSP  = 0x4218,
	 /** 
	capi2 api is CAPI2_DATA_SetCSDErrorCorrection 
	**/
	MSG_DATA_SET_CSD_ERROR_CORRECTION_REQ  = 0x4219,	///<Payload type {::CAPI2_DATA_SetCSDErrorCorrection_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_SET_CSD_ERROR_CORRECTION_RSP  = 0x421A,
	 /** 
	capi2 api is CAPI2_DATA_GetCSDErrorCorrection 
	**/
	MSG_DATA_GET_CSD_ERROR_CORRECTION_REQ  = 0x421B,	///<Payload type {::CAPI2_DATA_GetCSDErrorCorrection_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_GET_CSD_ERROR_CORRECTION_RSP  = 0x421C,
	 /** 
	capi2 api is CAPI2_DATA_SetCSDErrCorrectionType 
	**/
	MSG_DATA_SET_CSD_ERR_CORRECTION_TYPE_REQ  = 0x421D,	///<Payload type {::CAPI2_DATA_SetCSDErrCorrectionType_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_SET_CSD_ERR_CORRECTION_TYPE_RSP  = 0x421E,
	 /** 
	capi2 api is CAPI2_DATA_GetCSDErrCorrectionType 
	**/
	MSG_DATA_GET_CSD_ERR_CORRECTION_TYPE_REQ  = 0x421F,	///<Payload type {::CAPI2_DATA_GetCSDErrCorrectionType_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_GET_CSD_ERR_CORRECTION_TYPE_RSP  = 0x4220,
	 /** 
	capi2 api is CAPI2_DATA_SetCSDDataCompType 
	**/
	MSG_DATA_SET_CSD_DATA_COMP_TYPE_REQ  = 0x4221,	///<Payload type {::CAPI2_DATA_SetCSDDataCompType_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_SET_CSD_DATA_COMP_TYPE_RSP  = 0x4222,
	 /** 
	capi2 api is CAPI2_DATA_GetCSDDataCompType 
	**/
	MSG_DATA_GET_CSD_DATA_COMP_TYPE_REQ  = 0x4223,	///<Payload type {::CAPI2_DATA_GetCSDDataCompType_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_GET_CSD_DATA_COMP_TYPE_RSP  = 0x4224,
	 /** 
	capi2 api is CAPI2_DATA_SetCSDConnElement 
	**/
	MSG_DATA_SET_CSD_CONN_ELEMENT_REQ  = 0x4225,	///<Payload type {::CAPI2_DATA_SetCSDConnElement_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_SET_CSD_CONN_ELEMENT_RSP  = 0x4226,
	 /** 
	capi2 api is CAPI2_DATA_GetCSDConnElement 
	**/
	MSG_DATA_GET_CSD_CONN_ELEMENT_REQ  = 0x4227,	///<Payload type {::CAPI2_DATA_GetCSDConnElement_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_GET_CSD_CONN_ELEMENT_RSP  = 0x4228,
	 /** 
	capi2 api is CAPI2_DATA_UpdateAccountToFileSystem 
	**/
	MSG_DATA_UPDATE_ACCT_TO_FILE_SYSTEM_REQ  = 0x4229,
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_UPDATE_ACCT_TO_FILE_SYSTEM_RSP  = 0x422A,
	 /** 
	capi2 api is CAPI2_resetDataSize 
	**/
	MSG_DATA_RESET_DATA_SIZE_REQ  = 0x422B,	///<Payload type {::CAPI2_resetDataSize_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_RESET_DATA_SIZE_RSP  = 0x422C,
	 /** 
	capi2 api is CAPI2_addDataSentSizebyCid 
	**/
	MSG_DATA_ADD_DATA_SENT_SIZE_BY_CID_REQ  = 0x422D,	///<Payload type {::CAPI2_addDataSentSizebyCid_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_ADD_DATA_SENT_SIZE_BY_CID_RSP  = 0x422E,
	 /** 
	capi2 api is CAPI2_addDataRcvSizebyCid 
	**/
	MSG_DATA_ADD_DATA_RCV_SIZE_BY_CID_REQ  = 0x422F,	///<Payload type {::CAPI2_addDataRcvSizebyCid_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DATA_ADD_DATA_RCV_SIZE_BY_CID_RSP  = 0x4230,
	 /** 
	capi2 api is CAPI2_PBK_SendUsimHdkReadReq 
	**/
	MSG_READ_USIM_PBK_HDK_REQ  = 0x4231,
	 /** 
	capi2 api is CAPI2_PBK_SendUsimHdkUpdateReq 
	**/
	MSG_WRITE_USIM_PBK_HDK_REQ  = 0x4232,	///<Payload type {::CAPI2_PBK_SendUsimHdkUpdateReq_Req_t}
	 /** 
	capi2 api is CAPI2_PBK_SendUsimAasReadReq 
	**/
	MSG_READ_USIM_PBK_ALPHA_AAS_REQ  = 0x4233,	///<Payload type {::CAPI2_PBK_SendUsimAasReadReq_Req_t}
	 /** 
	capi2 api is CAPI2_PBK_SendUsimAasUpdateReq 
	**/
	MSG_WRITE_USIM_PBK_ALPHA_AAS_REQ  = 0x4234,	///<Payload type {::CAPI2_PBK_SendUsimAasUpdateReq_Req_t}
	 /** 
	capi2 api is CAPI2_PBK_SendUsimGasReadReq 
	**/
	MSG_READ_USIM_PBK_ALPHA_GAS_REQ  = 0x4235,	///<Payload type {::CAPI2_PBK_SendUsimGasReadReq_Req_t}
	 /** 
	capi2 api is CAPI2_PBK_SendUsimGasUpdateReq 
	**/
	MSG_WRITE_USIM_PBK_ALPHA_GAS_REQ  = 0x4236,	///<Payload type {::CAPI2_PBK_SendUsimGasUpdateReq_Req_t}
	 /** 
	capi2 api is CAPI2_PBK_SendUsimAasInfoReq 
	**/
	MSG_GET_USIM_PBK_ALPHA_INFO_AAS_REQ  = 0x4237,	///<Payload type {::CAPI2_PBK_SendUsimAasInfoReq_Req_t}
	 /** 
	capi2 api is CAPI2_PBK_SendUsimGasInfoReq 
	**/
	MSG_GET_USIM_PBK_ALPHA_INFO_GAS_REQ  = 0x4238,	///<Payload type {::CAPI2_PBK_SendUsimGasInfoReq_Req_t}
	 /** 
	capi2 api is CAPI2_LCS_ServiceControl 
	**/
	MSG_LCS_SERVICE_CONTROL_REQ  = 0x4239,	///<Payload type {::CAPI2_LCS_ServiceControl_Req_t}
	 /** 
	payload is ::LcsResult_t 
	**/
	MSG_LCS_SERVICE_CONTROL_RSP  = 0x423A,	///<Payload type {::LcsResult_t}
	 /** 
	capi2 api is CAPI2_LCS_PowerConfig 
	**/
	MSG_LCS_POWER_CONFIG_REQ  = 0x423B,	///<Payload type {::CAPI2_LCS_PowerConfig_Req_t}
	 /** 
	payload is ::LcsResult_t 
	**/
	MSG_LCS_POWER_CONFIG_RSP  = 0x423C,	///<Payload type {::LcsResult_t}
	 /** 
	capi2 api is CAPI2_LCS_ServiceQuery 
	**/
	MSG_LCS_SERVICE_QUERY_REQ  = 0x423D,
	 /** 
	payload is ::LcsServiceType_t 
	**/
	MSG_LCS_SERVICE_QUERY_RSP  = 0x423E,	///<Payload type {::LcsServiceType_t}
	 /** 
	capi2 api is CAPI2_LCS_RegisterSuplMsgHandler 
	**/
	MSG_LCS_REGISTER_SUPL_MSG_HANDLER_REQ  = 0x423F,	///<Payload type {::CAPI2_LCS_RegisterSuplMsgHandler_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_LCS_REGISTER_SUPL_MSG_HANDLER_RSP  = 0x4240,
	 /** 
	capi2 api is CAPI2_LCS_StartPosReqPeriodic 
	**/
	MSG_LCS_START_POS_REQ_PERIODIC_REQ  = 0x4241,	///<Payload type {::CAPI2_LCS_StartPosReqPeriodic_Req_t}
	 /** 
	payload is ::LcsHandle_t 
	**/
	MSG_LCS_START_POS_REQ_PERIODIC_RSP  = 0x4242,	///<Payload type {::LcsHandle_t}
	 /** 
	capi2 api is CAPI2_LCS_StartPosReqSingle 
	**/
	MSG_LCS_START_POS_REQ_SINGLE_REQ  = 0x4243,	///<Payload type {::CAPI2_LCS_StartPosReqSingle_Req_t}
	 /** 
	payload is ::LcsHandle_t 
	**/
	MSG_LCS_START_POS_REQ_SINGLE_RSP  = 0x4244,	///<Payload type {::LcsHandle_t}
	 /** 
	capi2 api is CAPI2_LCS_StopPosReq 
	**/
	MSG_LCS_STOP_POS_REQ  = 0x4245,	///<Payload type {::CAPI2_LCS_StopPosReq_Req_t}
	 /** 
	payload is ::LcsResult_t 
	**/
	MSG_LCS_STOP_POS_RSP  = 0x4246,	///<Payload type {::LcsResult_t}
	 /** 
	capi2 api is CAPI2_LCS_GetPosition 
	**/
	MSG_LCS_GET_POSITION_REQ  = 0x4247,
	 /** 
	payload is ::LcsPosData_t 
	**/
	MSG_LCS_GET_POSITION_RSP  = 0x4248,	///<Payload type {::LcsPosData_t}
	 /** 
	capi2 api is CAPI2_LCS_ConfigSet 
	**/
	MSG_LCS_CONFIG_SET_REQ  = 0x4249,	///<Payload type {::CAPI2_LCS_ConfigSet_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_LCS_CONFIG_SET_RSP  = 0x424A,
	 /** 
	capi2 api is CAPI2_LCS_ConfigGet 
	**/
	MSG_LCS_CONFIG_GET_REQ  = 0x424B,	///<Payload type {::CAPI2_LCS_ConfigGet_Req_t}
	 /** 
	payload is ::UInt32 
	**/
	MSG_LCS_CONFIG_GET_RSP  = 0x424C,	///<Payload type {::UInt32}
	 /** 
	capi2 api is CAPI2_LCS_SuplVerificationRsp 
	**/
	MSG_LCS_SUPL_VERIFICATION_RSP_REQ  = 0x424D,	///<Payload type {::CAPI2_LCS_SuplVerificationRsp_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_LCS_SUPL_VERIFICATION_RSP_RSP  = 0x424E,
	 /** 
	capi2 api is CAPI2_LCS_SuplConnectRsp 
	**/
	MSG_LCS_SUPL_CONNECT_RSP_REQ  = 0x424F,	///<Payload type {::CAPI2_LCS_SuplConnectRsp_Req_t}
	 /** 
	payload is ::LcsResult_t 
	**/
	MSG_LCS_SUPL_CONNECT_RSP_RSP  = 0x4250,	///<Payload type {::LcsResult_t}
	 /** 
	capi2 api is CAPI2_LCS_SuplDataAvailable 
	**/
	MSG_LCS_SUPL_DATA_AVAILABLE_REQ  = 0x4251,	///<Payload type {::LcsSuplCommData_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_LCS_SUPL_DATA_AVAILABLE_RSP  = 0x4252,
	 /** 
	capi2 api is CAPI2_LCS_SuplDisconnected 
	**/
	MSG_LCS_SUPL_DISCONNECTED_REQ  = 0x4253,	///<Payload type {::CAPI2_LCS_SuplDisconnected_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_LCS_SUPL_DISCONNECTED_RSP  = 0x4254,
	 /** 
	capi2 api is CAPI2_LCS_SuplInitHmacRsp 
	**/
	MSG_LCS_SUPL_INIT_HMAC_RSP_REQ  = 0x4255,	///<Payload type {::CAPI2_LCS_SuplInitHmacRsp_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_LCS_SUPL_INIT_HMAC_RSP_RSP  = 0x4256,
	 /** 
	capi2 api is CAPI2_LCS_CmdData 
	**/
	MSG_LCS_CMD_DATA_REQ  = 0x4257,	///<Payload type {::CAPI2_LCS_CmdData_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_LCS_CMD_DATA_RSP  = 0x4258,
	 /** 
	capi2 api is CAPI2_GPIO_ConfigOutput_64Pin 
	**/
	MSG_GPIO_CONFIG_OUTPUT_64PIN_REQ  = 0x4259,	///<Payload type {::CAPI2_GPIO_ConfigOutput_64Pin_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_GPIO_CONFIG_OUTPUT_64PIN_RSP  = 0x425A,
	 /** 
	capi2 api is CAPI2_GPS_Control 
	**/
	MSG_GPS_CONTROL_REQ  = 0x425B,	///<Payload type {::CAPI2_GPS_Control_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_GPS_CONTROL_RSP  = 0x425C,
	 /** 
	capi2 api is CAPI2_FFS_Control 
	**/
	MSG_FFS_CONTROL_REQ  = 0x425D,	///<Payload type {::CAPI2_FFS_Control_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_FFS_CONTROL_RSP  = 0x425E,
	 /** 
	capi2 api is CAPI2_CP2AP_PedestalMode_Control 
	**/
	MSG_CP2AP_PEDESTALMODE_CONTROL_REQ  = 0x425F,	///<Payload type {::CAPI2_CP2AP_PedestalMode_Control_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_CP2AP_PEDESTALMODE_CONTROL_RSP  = 0x4260,
	 /** 
	capi2 api is CAPI2_AUDIO_ASIC_SetAudioMode 
	**/
	MSG_AUDIO_ASIC_SetAudioMode_REQ  = 0x4261,	///<Payload type {::CAPI2_AUDIO_ASIC_SetAudioMode_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_AUDIO_ASIC_SetAudioMode_RSP  = 0x4262,
	 /** 
	capi2 api is CAPI2_SPEAKER_StartTone 
	**/
	MSG_SPEAKER_StartTone_REQ  = 0x4263,	///<Payload type {::CAPI2_SPEAKER_StartTone_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SPEAKER_StartTone_RSP  = 0x4264,
	 /** 
	capi2 api is CAPI2_SPEAKER_StartGenericTone 
	**/
	MSG_SPEAKER_StartGenericTone_REQ  = 0x4265,	///<Payload type {::CAPI2_SPEAKER_StartGenericTone_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SPEAKER_StartGenericTone_RSP  = 0x4266,
	 /** 
	capi2 api is CAPI2_SPEAKER_StopTone 
	**/
	MSG_SPEAKER_StopTone_REQ  = 0x4267,
	 /** 
	payload is ::default_proc 
	**/
	MSG_SPEAKER_StopTone_RSP  = 0x4268,
	 /** 
	capi2 api is CAPI2_AUDIO_Turn_EC_NS_OnOff 
	**/
	MSG_AUDIO_Turn_EC_NS_OnOff_REQ  = 0x4269,	///<Payload type {::CAPI2_AUDIO_Turn_EC_NS_OnOff_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_AUDIO_Turn_EC_NS_OnOff_RSP  = 0x426A,
	 /** 
	capi2 api is CAPI2_ECHO_SetDigitalTxGain 
	**/
	MSG_ECHO_SetDigitalTxGain_REQ  = 0x426B,	///<Payload type {::CAPI2_ECHO_SetDigitalTxGain_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_ECHO_SetDigitalTxGain_RSP  = 0x426C,
	 /** 
	capi2 api is CAPI2_RIPCMDQ_Connect_Uplink 
	**/
	MSG_RIPCMDQ_Connect_Uplink_REQ  = 0x426D,	///<Payload type {::CAPI2_RIPCMDQ_Connect_Uplink_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_RIPCMDQ_Connect_Uplink_RSP  = 0x426E,
	 /** 
	capi2 api is CAPI2_RIPCMDQ_Connect_Downlink 
	**/
	MSG_RIPCMDQ_Connect_Downlink_REQ  = 0x426F,	///<Payload type {::CAPI2_RIPCMDQ_Connect_Downlink_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_RIPCMDQ_Connect_Downlink_RSP  = 0x4270,
	 /** 
	capi2 api is CAPI2_VOLUMECTRL_SetBasebandVolume 
	**/
	MSG_VOLUMECTRL_SetBasebandVolume_REQ  = 0x4271,	///<Payload type {::CAPI2_VOLUMECTRL_SetBasebandVolume_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_VOLUMECTRL_SetBasebandVolume_RSP  = 0x4272,
	 /** 
	capi2 api is CAPI2_DIAG_ApiMeasurmentReportReq 
	**/
	MSG_DIAG_MEASURE_REPORT_REQ  = 0x4273,	///<Payload type {::CAPI2_DIAG_ApiMeasurmentReportReq_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DIAG_MEASURE_REPORT_RSP  = 0x4274,
	 /** 
	capi2 api is CAPI2_PMU_BattChargingNotification 
	**/
	MSG_PMU_BATT_CHARGING_NOTIFICATION_REQ  = 0x4275,	///<Payload type {::CAPI2_PMU_BattChargingNotification_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_PMU_BATT_CHARGING_NOTIFICATION_RSP  = 0x4276,
	 /** 
	capi2 api is CAPI2_PATCH_GetRevision 
	**/
	MSG_PATCH_GET_REVISION_REQ  = 0x4277,
	 /** 
	payload is ::CAPI2_Patch_Revision_Ptr_t 
	**/
	MSG_PATCH_GET_REVISION_RSP  = 0x4278,	///<Payload type {::CAPI2_Patch_Revision_Ptr_t}
	 /** 
	capi2 api is CAPI2_RTC_SetTime 
	**/
	MSG_RTC_SetTime_REQ  = 0x4279,	///<Payload type {::CAPI2_RTC_SetTime_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_RTC_SetTime_RSP  = 0x427A,
	 /** 
	capi2 api is CAPI2_RTC_SetDST 
	**/
	MSG_RTC_SetDST_REQ  = 0x427B,	///<Payload type {::CAPI2_RTC_SetDST_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_RTC_SetDST_RSP  = 0x427C,
	 /** 
	capi2 api is CAPI2_RTC_SetTimeZone 
	**/
	MSG_RTC_SetTimeZone_REQ  = 0x427D,	///<Payload type {::CAPI2_RTC_SetTimeZone_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_RTC_SetTimeZone_RSP  = 0x427E,
	 /** 
	capi2 api is CAPI2_MS_InitCallCfg 
	**/
	MSG_MS_INITCALLCFG_REQ  = 0x427F,
	 /** 
	payload is ::default_proc 
	**/
	MSG_MS_INITCALLCFG_RSP  = 0x4280,
	 /** 
	capi2 api is CAPI2_MS_InitFaxConfig 
	**/
	MSG_MS_INITFAXCFG_REQ  = 0x4281,
	 /** 
	payload is ::default_proc 
	**/
	MSG_MS_INITFAXCFG_RSP  = 0x4282,
	 /** 
	capi2 api is CAPI2_MS_InitVideoCallCfg 
	**/
	MSG_MS_INITVIDEOCALLCFG_REQ  = 0x4283,
	 /** 
	payload is ::default_proc 
	**/
	MSG_MS_INITVIDEOCALLCFG_RSP  = 0x4284,
	 /** 
	capi2 api is CAPI2_MS_InitCallCfgAmpF 
	**/
	MSG_MS_INITCALLCFGAMPF_REQ  = 0x4285,
	 /** 
	payload is ::default_proc 
	**/
	MSG_MS_INITCALLCFGAMPF_RSP  = 0x4286,
	 /** 
	capi2 api is CAPI2_RTC_GetTime 
	**/
	MSG_RTC_GetTime_REQ  = 0x4287,	///<Payload type {::CAPI2_RTC_GetTime_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_RTC_GetTime_RSP  = 0x4288,
	 /** 
	capi2 api is CAPI2_RTC_GetTimeZone 
	**/
	MSG_RTC_GetTimeZone_REQ  = 0x4289,
	 /** 
	payload is ::Int8 
	**/
	MSG_RTC_GetTimeZone_RSP  = 0x428A,	///<Payload type {::Int8}
	 /** 
	capi2 api is CAPI2_RTC_GetDST 
	**/
	MSG_RTC_GetDST_REQ  = 0x428B,
	 /** 
	payload is ::UInt8 
	**/
	MSG_RTC_GetDST_RSP  = 0x428C,	///<Payload type {::UInt8}
	 /** 
	capi2 api is CAPI2_SMS_GetMeSmsBufferStatus 
	**/
	MSG_SMS_GETMESMS_BUF_STATUS_REQ  = 0x428D,	///<Payload type {::CAPI2_SMS_GetMeSmsBufferStatus_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SMS_GETMESMS_BUF_STATUS_RSP  = 0x428E,
	 /** 
	capi2 api is CAPI2_SMS_GetRecordNumberOfReplaceSMS 
	**/
	MSG_SMS_GETRECORDNUMBEROFREPLACESMS_REQ  = 0x428F,	///<Payload type {::CAPI2_SMS_GetRecordNumberOfReplaceSMS_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SMS_GETRECORDNUMBEROFREPLACESMS_RSP  = 0x4290,
	 /** 
	capi2 api is CAPI2_PMU_BattFullInd 
	**/
	MSG_PMU_BATT_FULL_REQ  = 0x4291,
	 /** 
	payload is ::default_proc 
	**/
	MSG_PMU_BATT_FULL_RSP  = 0x4292,
	 /** 
	capi2 api is CAPI2_FLASH_SaveImage 
	**/
	MSG_FLASH_SAVEIMAGE_REQ  = 0x4293,	///<Payload type {::CAPI2_FLASH_SaveImage_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_FLASH_SAVEIMAGE_RSP  = 0x4294,
	 /** 
	capi2 api is CAPI2_AUDIO_GetSettings 
	**/
	MSG_AUDIO_GET_SETTINGS_REQ  = 0x4295,	///<Payload type {::CAPI2_AUDIO_GetSettings_Req_t}
	 /** 
	payload is ::Capi2AudioParams_t 
	**/
	MSG_AUDIO_GET_SETTINGS_RSP  = 0x4296,	///<Payload type {::Capi2AudioParams_t}
	 /** 
	capi2 api is CAPI2_SIMLOCK_GetStatus 
	**/
	MSG_SIMLOCK_GET_STATUS_REQ  = 0x4297,	///<Payload type {::CAPI2_SIMLOCK_GetStatus_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SIMLOCK_GET_STATUS_RSP  = 0x4298,
	 /** 
	capi2 api is CAPI2_SIMLOCK_SetStatus 
	**/
	MSG_SIMLOCK_SET_STATUS_REQ  = 0x4299,	///<Payload type {::CAPI2_SIMLOCK_SetStatus_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SIMLOCK_SET_STATUS_RSP  = 0x429A,
	 /** 
	capi2 api is CAPI2_SYSPARM_GetActualLowVoltReading 
	**/
	MSG_SYSPARM_GET_ACTUAL_LOW_VOLT_REQ  = 0x429B,
	 /** 
	payload is ::UInt16 
	**/
	MSG_SYSPARM_GET_ACTUAL_LOW_VOLT_RSP  = 0x429C,	///<Payload type {::UInt16}
	 /** 
	capi2 api is CAPI2_SYSPARM_GetActual4p2VoltReading 
	**/
	MSG_SYSPARM_GET_ACTUAL_4P2_VOLT_REQ  = 0x429D,
	 /** 
	payload is ::UInt16 
	**/
	MSG_SYSPARM_GET_ACTUAL_4P2_VOLT_RSP  = 0x429E,	///<Payload type {::UInt16}
	 /** 
	capi2 api is CAPI2_program_equalizer 
	**/
	MSG_PROG_EQU_TYPE_REQ  = 0x429F,	///<Payload type {::CAPI2_program_equalizer_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_PROG_EQU_TYPE_RSP  = 0x42A0,
	 /** 
	capi2 api is CAPI2_program_poly_equalizer 
	**/
	MSG_PROG_POLY_EQU_TYPE_REQ  = 0x42A1,	///<Payload type {::CAPI2_program_poly_equalizer_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_PROG_POLY_EQU_TYPE_RSP  = 0x42A2,
	 /** 
	capi2 api is CAPI2_program_FIR_IIR_filter 
	**/
	MSG_PROG_FIR_IIR_FILTER_REQ  = 0x42A3,	///<Payload type {::CAPI2_program_FIR_IIR_filter_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_PROG_FIR_IIR_FILTER_RSP  = 0x42A4,
	 /** 
	capi2 api is CAPI2_program_poly_FIR_IIR_filter 
	**/
	MSG_PROG_POLY_FIR_IIR_FILTER_REQ  = 0x42A5,	///<Payload type {::CAPI2_program_poly_FIR_IIR_filter_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_PROG_POLY_FIR_IIR_FILTER_RSP  = 0x42A6,
	 /** 
	capi2 api is CAPI2_audio_control_generic 
	**/
	MSG_AUDIO_CTRL_GENERIC_REQ  = 0x42A7,	///<Payload type {::CAPI2_audio_control_generic_Req_t}
	 /** 
	payload is ::UInt32 
	**/
	MSG_AUDIO_CTRL_GENERIC_RSP  = 0x42A8,	///<Payload type {::UInt32}
	 /** 
	capi2 api is CAPI2_audio_control_dsp 
	**/
	MSG_AUDIO_CTRL_DSP_REQ  = 0x42A9,	///<Payload type {::CAPI2_audio_control_dsp_Req_t}
	 /** 
	payload is ::UInt32 
	**/
	MSG_AUDIO_CTRL_DSP_RSP  = 0x42AA,	///<Payload type {::UInt32}
	 /** 
	capi2 api is CAPI2_FFS_Read 
	**/
	MSG_FFS_READ_REQ  = 0x42AB,	///<Payload type {::CAPI2_FFS_Read_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_FFS_READ_RSP  = 0x42AC,
	 /** 
	capi2 api is CAPI2_DIAG_ApiCellLockReq 
	**/
	MSG_DIAG_CELLLOCK_REQ  = 0x42AD,	///<Payload type {::CAPI2_DIAG_ApiCellLockReq_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DIAG_CELLLOCK_RSP  = 0x42AE,
	 /** 
	capi2 api is CAPI2_DIAG_ApiCellLockStatus 
	**/
	MSG_DIAG_CELLLOCK_STATUS_REQ  = 0x42AF,
	 /** 
	payload is ::Boolean 
	**/
	MSG_DIAG_CELLLOCK_STATUS_RSP  = 0x42B0,	///<Payload type {::Boolean}
	 /** 
	capi2 api is CAPI2_LCS_RegisterRrlpDataHandler 
	**/
	MSG_LCS_REG_RRLP_HDL_REQ  = 0x42B1,	///<Payload type {::CAPI2_LCS_RegisterRrlpDataHandler_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_LCS_REG_RRLP_HDL_RSP  = 0x42B2,
	 /** 
	capi2 api is CAPI2_LCS_SendRrlpDataToNetwork 
	**/
	MSG_LCS_SEND_RRLP_DATA_REQ  = 0x42B3,	///<Payload type {::CAPI2_LCS_RrlpData_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_LCS_SEND_RRLP_DATA_RSP  = 0x42B4,
	 /** 
	capi2 api is CAPI2_LCS_RegisterRrcDataHandler 
	**/
	MSG_LCS_REG_RRC_HDL_REQ  = 0x42B5,	///<Payload type {::CAPI2_LCS_RegisterRrcDataHandler_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_LCS_REG_RRC_HDL_RSP  = 0x42B6,
	 /** 
	capi2 api is CAPI2_LCS_RrcMeasurementReport 
	**/
	MSG_LCS_RRC_MEAS_REPORT_REQ  = 0x42B7,	///<Payload type {::CAPI2_LCS_RrcMeasReport_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_LCS_RRC_MEAS_REPORT_RSP  = 0x42B8,
	 /** 
	capi2 api is CAPI2_LCS_RrcMeasurementControlFailure 
	**/
	MSG_LCS_RRC_MEAS_FAILURE_REQ  = 0x42B9,	///<Payload type {::CAPI2_LCS_RrcMeasFailure_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_LCS_RRC_MEAS_FAILURE_RSP  = 0x42BA,
	 /** 
	capi2 api is CAPI2_LCS_RrcStatus 
	**/
	MSG_LCS_RRC_STATUS_REQ  = 0x42BB,	///<Payload type {::CAPI2_LCS_RrcStatus_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_LCS_RRC_STATUS_RSP  = 0x42BC,
	 /** 
	capi2 api is CAPI2_CC_IsThereEmergencyCall 
	**/
	MSG_CC_IS_THERE_EMERGENCY_CALL_REQ  = 0x42BD,
	 /** 
	payload is ::Boolean 
	**/
	MSG_CC_IS_THERE_EMERGENCY_CALL_RSP  = 0x42BE,	///<Payload type {::Boolean}
	 /** 
	capi2 api is CAPI2_SYSPARM_GetBattLowThresh 
	**/
	MSG_SYSPARM_GET_LOW_THRESHOLD_VOLT_REQ  = 0x42BF,
	 /** 
	payload is ::UInt16 
	**/
	MSG_SYSPARM_GET_LOW_THRESHOLD_VOLT_RSP  = 0x42C0,	///<Payload type {::UInt16}
	 /** 
	capi2 api is CAPI2_SYSPARM_GetDefault4p2VoltReading 
	**/
	MSG_SYSPARM_GET_DEFAULT_4P2_VOLT_REQ  = 0x42C1,
	 /** 
	payload is ::UInt16 
	**/
	MSG_SYSPARM_GET_DEFAULT_4P2_VOLT_RSP  = 0x42C2,	///<Payload type {::UInt16}
	 /** 
	capi2 api is CAPI2_L1_bb_isLocked 
	**/
	MSG_L1_BB_ISLOCKED_REQ  = 0x42C3,	///<Payload type {::CAPI2_L1_bb_isLocked_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_L1_BB_ISLOCKED_RSP  = 0x42C4,	///<Payload type {::Boolean}
	 /** 
	capi2 api is CAPI2_MS_ForcePsReleaseReq 
	**/
	MSG_MS_FORCE_PS_REL_REQ  = 0x42C5,
	 /** 
	payload is ::default_proc 
	**/
	MSG_MS_FORCE_PS_REL_RSP  = 0x42C6,
	 /** 
	capi2 api is CAPI2_SYS_ProcessNoRfOnlyChgReq 
	**/
	MSG_SYS_POWERUP_NORF_CHG_REQ  = 0x42C7,
	 /** 
	payload is ::default_proc 
	**/
	MSG_SYS_POWERUP_NORF_CHG_RSP  = 0x42C8,
	 /** 
	capi2 api is CAPI2_SYSPARM_GetGPIO_Value 
	**/
	MSG_SYSPARM_GET_GPIO_REQ  = 0x42C9,
	 /** 
	payload is ::Capi2GpioValue_t 
	**/
	MSG_SYSPARM_GET_GPIO_RSP  = 0x42CA,	///<Payload type {::Capi2GpioValue_t}
	 /** 
	capi2 api is CAPI2_SYS_EnableCellInfoMsg 
	**/
	MSG_SYS_ENABLE_CELL_INFO_REQ  = 0x42CB,	///<Payload type {::CAPI2_SYS_EnableCellInfoMsg_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SYS_ENABLE_CELL_INFO_RSP  = 0x42CC,
	 /** 
	capi2 api is CAPI2_SYSPARM_SetHSDPAPHYCategory 
	**/
	MSG_SYS_SET_HSDPA_CATEGORY_REQ  = 0x42CD,	///<Payload type {::CAPI2_SYSPARM_SetHSDPAPHYCategory_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SYS_SET_HSDPA_CATEGORY_RSP  = 0x42CE,
	 /** 
	capi2 api is CAPI2_SYSPARM_GetHSDPAPHYCategory 
	**/
	MSG_SYS_GET_HSDPA_CATEGORY_REQ  = 0x42CF,
	 /** 
	payload is ::UInt32 
	**/
	MSG_SYS_GET_HSDPA_CATEGORY_RSP  = 0x42D0,	///<Payload type {::UInt32}
	 /** 
	capi2 api is CAPI2_SMS_StartMultiSmsTransferReq 
	**/
	MSG_SIM_START_MULTI_SMS_XFR_REQ  = 0x42D1,
	 /** 
	payload is ::default_proc 
	**/
	MSG_SIM_START_MULTI_SMS_XFR_RSP  = 0x42D2,
	 /** 
	capi2 api is CAPI2_SMS_StopMultiSmsTransferReq 
	**/
	MSG_SIM_STOP_MULTI_SMS_XFR_REQ  = 0x42D3,
	 /** 
	payload is ::default_proc 
	**/
	MSG_SIM_STOP_MULTI_SMS_XFR_RSP  = 0x42D4,
	 /** 
	capi2 api is CAPI2_SIM_SendWriteSmsReq 
	**/
	MSG_SIM_SMS_WRITE_REQ  = 0x42D5,	///<Payload type {::CAPI2_SIM_SendWriteSmsReq_Req_t}
	 /** 
	capi2 api is CAPI2_BATTMGR_GetChargingStatus 
	**/
	MSG_BATTMGR_GETCHARGINGSTATUS_REQ  = 0x42D6,
	 /** 
	payload is ::EM_BATTMGR_ChargingStatus_en_t 
	**/
	MSG_BATTMGR_GETCHARGINGSTATUS_RSP  = 0x42D7,	///<Payload type {::EM_BATTMGR_ChargingStatus_en_t}
	 /** 
	capi2 api is CAPI2_BATTMGR_GetPercentageLevel 
	**/
	MSG_BATTMGR_GETPERCENTAGELEVEL_REQ  = 0x42D8,
	 /** 
	payload is ::UInt16 
	**/
	MSG_BATTMGR_GETPERCENTAGELEVEL_RSP  = 0x42D9,	///<Payload type {::UInt16}
	 /** 
	capi2 api is CAPI2_BATTMGR_IsBatteryPresent 
	**/
	MSG_BATTMGR_ISBATTERYPRESENT_REQ  = 0x42DA,
	 /** 
	payload is ::Boolean 
	**/
	MSG_BATTMGR_ISBATTERYPRESENT_RSP  = 0x42DB,	///<Payload type {::Boolean}
	 /** 
	capi2 api is CAPI2_BATTMGR_IsChargerPlugIn 
	**/
	MSG_BATTMGR_ISCHARGERPLUGIN_REQ  = 0x42DC,
	 /** 
	payload is ::Boolean 
	**/
	MSG_BATTMGR_ISCHARGERPLUGIN_RSP  = 0x42DD,	///<Payload type {::Boolean}
	 /** 
	capi2 api is CAPI2_SYSPARM_GetBattTable 
	**/
	MSG_SYSPARM_GETBATT_TABLE_REQ  = 0x42DE,
	 /** 
	payload is ::Batt_Level_Table_t 
	**/
	MSG_SYSPARM_GETBATT_TABLE_RSP  = 0x42DF,	///<Payload type {::Batt_Level_Table_t}
	 /** 
	capi2 api is CAPI2_BATTMGR_GetLevel 
	**/
	MSG_BATTMGR_GET_LEVEL_REQ  = 0x42E0,
	 /** 
	payload is ::UInt16 
	**/
	MSG_BATTMGR_GET_LEVEL_RSP  = 0x42E1,	///<Payload type {::UInt16}
	 /** 
	capi2 api is CAPI2_AT_ProcessCmdToAP 
	**/
	MSG_CAPI2_AT_COMMAND_TO_AP_REQ  = 0x42E2,	///<Payload type {::CAPI2_AT_ProcessCmdToAP_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_CAPI2_AT_COMMAND_TO_AP_RSP  = 0x42E3,
	 /** 
	capi2 api is CAPI2_SMS_WriteSMSPduToSIMRecordReq 
	**/
	MSG_SMS_WRITESMSPDUTO_SIMRECORD_REQ  = 0x42E4,	///<Payload type {::CAPI2_SMS_WriteSMSPduToSIMRecordReq_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SMS_WRITESMSPDUTO_SIMRECORD_RSP  = 0x42E5,
	 /** 
	capi2 api is CAPI2_ADCMGR_MultiChStart 
	**/
	MSG_ADC_MULTI_CH_START_REQ  = 0x42E6,	///<Payload type {::CAPI2_ADCMGR_MultiChStart_Req_t}
	 /** 
	payload is ::CAPI2_ADC_ChannelRsp_t 
	**/
	MSG_ADC_MULTI_CH_START_RSP  = 0x42E7,	///<Payload type {::CAPI2_ADC_ChannelRsp_t}
	 /** 
	capi2 api is CAPI2_InterTaskMsgToCP 
	**/
	MSG_INTERTASK_MSG_TO_CP_REQ  = 0x42E8,	///<Payload type {::CAPI2_InterTaskMsgToCP_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_INTERTASK_MSG_TO_CP_RSP  = 0x42E9,
	 /** 
	capi2 api is CAPI2_InterTaskMsgToAP 
	**/
	MSG_INTERTASK_MSG_TO_AP_REQ  = 0x42EA,	///<Payload type {::CAPI2_InterTaskMsgToAP_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_INTERTASK_MSG_TO_AP_RSP  = 0x42EB,

	MSG_GEN_REQ_END = MSG_GRP_CAPI2_GEN_F + 0xFF,

