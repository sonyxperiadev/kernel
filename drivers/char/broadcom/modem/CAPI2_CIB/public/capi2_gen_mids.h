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

#ifdef DEFINE_CAPI2_GEN_MIDS
//MSG_GEN_REQ_START = 0x4000
	 /** 
	api is CAPI2_MS_IsGSMRegistered 
	**/
	MSG_MS_GSM_REGISTERED_REQ  = 0x4000,
	 /** 
	payload is ::Boolean 
	**/
	MSG_MS_GSM_REGISTERED_RSP  = 0x4001,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_MS_IsGPRSRegistered 
	**/
	MSG_MS_GPRS_REGISTERED_REQ  = 0x4002,
	 /** 
	payload is ::Boolean 
	**/
	MSG_MS_GPRS_REGISTERED_RSP  = 0x4003,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_MS_GetGSMRegCause 
	**/
	MSG_MS_GSM_CAUSE_REQ  = 0x4004,
	 /** 
	payload is ::NetworkCause_t 
	**/
	MSG_MS_GSM_CAUSE_RSP  = 0x4005,	///<Payload type {::NetworkCause_t}
	 /** 
	api is CAPI2_MS_GetGPRSRegCause 
	**/
	MSG_MS_GPRS_CAUSE_REQ  = 0x4006,
	 /** 
	payload is ::NetworkCause_t 
	**/
	MSG_MS_GPRS_CAUSE_RSP  = 0x4007,	///<Payload type {::NetworkCause_t}
	 /** 
	api is CAPI2_MS_GetRegisteredLAC 
	**/
	MSG_MS_REGISTERED_LAC_REQ  = 0x4008,
	 /** 
	payload is ::UInt16 
	**/
	MSG_MS_REGISTERED_LAC_RSP  = 0x4009,	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_MS_GetPlmnMCC 
	**/
	MSG_MS_GET_PLMN_MCC_REQ  = 0x400A,
	 /** 
	payload is ::UInt16 
	**/
	MSG_MS_GET_PLMN_MCC_RSP  = 0x400B,	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_MS_GetPlmnMNC 
	**/
	MSG_MS_GET_PLMN_MNC_REQ  = 0x400C,
	 /** 
	payload is ::UInt8 
	**/
	MSG_MS_GET_PLMN_MNC_RSP  = 0x400D,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_SYS_ProcessPowerDownReq 
	**/
	MSG_SYS_POWERDOWN_REQ  = 0x400E,
	 /** 
	payload is ::default_proc 
	**/
	MSG_SYS_POWERDOWN_RSP  = 0x400F,
	 /** 
	api is CAPI2_SYS_ProcessNoRfReq 
	**/
	MSG_SYS_POWERUP_NORF_REQ  = 0x4010,
	 /** 
	payload is ::default_proc 
	**/
	MSG_SYS_POWERUP_NORF_RSP  = 0x4011,
	 /** 
	api is CAPI2_SYS_ProcessPowerUpReq 
	**/
	MSG_SYS_POWERUP_REQ  = 0x4012,
	 /** 
	payload is ::default_proc 
	**/
	MSG_SYS_POWERUP_RSP  = 0x4013,
	 /** 
	api is CAPI2_MS_IsGprsAllowed 
	**/
	MSG_MS_GPRS_ALLOWED_REQ  = 0x4014,
	 /** 
	payload is ::Boolean 
	**/
	MSG_MS_GPRS_ALLOWED_RSP  = 0x4015,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_MS_GetCurrentRAT 
	**/
	MSG_MS_GET_CURRENT_RAT_REQ  = 0x4016,
	 /** 
	payload is ::UInt8 
	**/
	MSG_MS_GET_CURRENT_RAT_RSP  = 0x4017,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_MS_GetCurrentBand 
	**/
	MSG_MS_GET_CURRENT_BAND_REQ  = 0x4018,
	 /** 
	payload is ::UInt8 
	**/
	MSG_MS_GET_CURRENT_BAND_RSP  = 0x4019,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_SIM_UpdateSMSCapExceededFlag 
	**/
	MSG_SIM_UPDATE_SMSCAPEXC_FLAG_REQ  = 0x401A,	///<Payload type {::CAPI2_SIM_UpdateSMSCapExceededFlag_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SIM_UPDATE_SMSCAPEXC_FLAG_RSP  = 0x401B,
	 /** 
	api is CAPI2_SYS_SelectBand 
	**/
	MSG_MS_SELECT_BAND_REQ  = 0x401C,	///<Payload type {::CAPI2_SYS_SelectBand_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_MS_SELECT_BAND_RSP  = 0x401D,
	 /** 
	api is CAPI2_MS_SetSupportedRATandBand 
	**/
	MSG_MS_SET_RAT_BAND_REQ  = 0x401E,	///<Payload type {::CAPI2_MS_SetSupportedRATandBand_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_MS_SET_RAT_BAND_RSP  = 0x401F,
	 /** 
	api is CAPI2_PLMN_GetCountryByMcc 
	**/
	MSG_MS_GET_MCC_COUNTRY_REQ  = 0x4020,	///<Payload type {::CAPI2_PLMN_GetCountryByMcc_Req_t}
	 /** 
	payload is ::uchar_ptr_t 
	**/
	MSG_MS_GET_MCC_COUNTRY_RSP  = 0x4021,	///<Payload type {::uchar_ptr_t}
	 /** 
	api is CAPI2_MS_GetPLMNEntryByIndex 
	**/
	MSG_MS_PLMN_INFO_BY_CODE_REQ  = 0x4022,	///<Payload type {::CAPI2_MS_GetPLMNEntryByIndex_Req_t}
	 /** 
	payload is ::MsPlmnInfo_t 
	**/
	MSG_MS_PLMN_INFO_BY_CODE_RSP  = 0x4023,	///<Payload type {::MsPlmnInfo_t}
	 /** 
	api is CAPI2_MS_GetPLMNListSize 
	**/
	MSG_MS_PLMN_LIST_SIZE_REQ  = 0x4024,
	 /** 
	payload is ::UInt16 
	**/
	MSG_MS_PLMN_LIST_SIZE_RSP  = 0x4025,	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_MS_GetPLMNByCode 
	**/
	MSG_MS_PLMN_INFO_BY_INDEX_REQ  = 0x4026,	///<Payload type {::CAPI2_MS_GetPLMNByCode_Req_t}
	 /** 
	payload is ::MsPlmnInfo_t 
	**/
	MSG_MS_PLMN_INFO_BY_INDEX_RSP  = 0x4027,	///<Payload type {::MsPlmnInfo_t}
	 /** 
	api is CAPI2_MS_PlmnSelect 
	**/
	MSG_MS_PLMN_SELECT_REQ  = 0x4028,	///<Payload type {::CAPI2_MS_PlmnSelect_Req_t}
	 /** 
	api is CAPI2_MS_AbortPlmnSelect 
	**/
	MSG_MS_PLMN_ABORT_REQ  = 0x4029,
	 /** 
	payload is ::default_proc 
	**/
	MSG_MS_PLMN_ABORT_RSP  = 0x402A,
	 /** 
	api is CAPI2_MS_GetPlmnMode 
	**/
	MSG_MS_GET_PLMN_MODE_REQ  = 0x402B,
	 /** 
	payload is ::PlmnSelectMode_t 
	**/
	MSG_MS_GET_PLMN_MODE_RSP  = 0x402C,	///<Payload type {::PlmnSelectMode_t}
	 /** 
	api is CAPI2_MS_SetPlmnMode 
	**/
	MSG_MS_SET_PLMN_MODE_REQ  = 0x402D,	///<Payload type {::CAPI2_MS_SetPlmnMode_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_MS_SET_PLMN_MODE_RSP  = 0x402E,
	 /** 
	api is CAPI2_MS_GetPlmnFormat 
	**/
	MSG_MS_GET_PLMN_FORMAT_REQ  = 0x402F,
	 /** 
	payload is ::PlmnSelectFormat_t 
	**/
	MSG_MS_GET_PLMN_FORMAT_RSP  = 0x4030,	///<Payload type {::PlmnSelectFormat_t}
	 /** 
	api is CAPI2_MS_SetPlmnFormat 
	**/
	MSG_MS_SET_PLMN_FORMAT_REQ  = 0x4031,	///<Payload type {::CAPI2_MS_SetPlmnFormat_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_MS_SET_PLMN_FORMAT_RSP  = 0x4032,
	 /** 
	api is CAPI2_MS_IsMatchedPLMN 
	**/
	MSG_MS_MATCH_PLMN_REQ  = 0x4033,	///<Payload type {::CAPI2_MS_IsMatchedPLMN_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_MS_MATCH_PLMN_RSP  = 0x4034,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_MS_SearchAvailablePLMN 
	**/
	MSG_MS_SEARCH_PLMN_REQ  = 0x4035,
	 /** 
	api is CAPI2_MS_AbortSearchPLMN 
	**/
	MSG_MS_ABORT_PLMN_REQ  = 0x4036,
	 /** 
	payload is ::default_proc 
	**/
	MSG_MS_ABORT_PLMN_RSP  = 0x4037,
	 /** 
	api is CAPI2_MS_AutoSearchReq 
	**/
	MSG_MS_AUTO_SEARCH_REQ  = 0x4038,
	 /** 
	payload is ::default_proc 
	**/
	MSG_MS_AUTO_SEARCH_RSP  = 0x4039,
	 /** 
	api is CAPI2_MS_GetPLMNNameByCode 
	**/
	MSG_MS_PLMN_NAME_REQ  = 0x403A,	///<Payload type {::CAPI2_MS_GetPLMNNameByCode_Req_t}
	 /** 
	payload is ::MsPlmnName_t 
	**/
	MSG_MS_PLMN_NAME_RSP  = 0x403B,	///<Payload type {::MsPlmnName_t}
	 /** 
	api is CAPI2_SYS_GetSystemState 
	**/
	MSG_SYS_GET_SYSTEM_STATE_REQ  = 0x403C,
	 /** 
	payload is ::SystemState_t 
	**/
	MSG_SYS_GET_SYSTEM_STATE_RSP  = 0x403D,	///<Payload type {::SystemState_t}
	 /** 
	api is CAPI2_SYS_SetSystemState 
	**/
	MSG_SYS_SET_SYSTEM_STATE_REQ  = 0x403E,	///<Payload type {::CAPI2_SYS_SetSystemState_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SYS_SET_SYSTEM_STATE_RSP  = 0x403F,
	 /** 
	api is CAPI2_SYS_GetRxSignalInfo 
	**/
	MSG_SYS_GET_RX_LEVEL_REQ  = 0x4040,
	 /** 
	payload is ::MsRxLevelData_t 
	**/
	MSG_SYS_GET_RX_LEVEL_RSP  = 0x4041,	///<Payload type {::MsRxLevelData_t}
	 /** 
	api is CAPI2_SYS_GetGSMRegistrationStatus 
	**/
	MSG_SYS_GET_GSMREG_STATUS_REQ  = 0x4042,
	 /** 
	payload is ::RegisterStatus_t 
	**/
	MSG_SYS_GET_GSMREG_STATUS_RSP  = 0x4043,	///<Payload type {::RegisterStatus_t}
	 /** 
	api is CAPI2_SYS_GetGPRSRegistrationStatus 
	**/
	MSG_SYS_GET_GPRSREG_STATUS_REQ  = 0x4044,
	 /** 
	payload is ::RegisterStatus_t 
	**/
	MSG_SYS_GET_GPRSREG_STATUS_RSP  = 0x4045,	///<Payload type {::RegisterStatus_t}
	 /** 
	api is CAPI2_SYS_IsRegisteredGSMOrGPRS 
	**/
	MSG_SYS_GET_REG_STATUS_REQ  = 0x4046,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SYS_GET_REG_STATUS_RSP  = 0x4047,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SYS_GetGSMRegistrationCause 
	**/
	MSG_SYS_GET_GSMREG_CAUSE_REQ  = 0x4048,
	 /** 
	payload is ::PCHRejectCause_t 
	**/
	MSG_SYS_GET_GSMREG_CAUSE_RSP  = 0x4049,	///<Payload type {::PCHRejectCause_t}
	 /** 
	api is CAPI2_MS_IsPlmnForbidden 
	**/
	MSG_SYS_IS_PLMN_FORBIDDEN_REQ  = 0x404A,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SYS_IS_PLMN_FORBIDDEN_RSP  = 0x404B,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_MS_IsRegisteredHomePLMN 
	**/
	MSG_HOME_PLMN_REG_REQ  = 0x404C,
	 /** 
	payload is ::Boolean 
	**/
	MSG_HOME_PLMN_REG_RSP  = 0x404D,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_MS_SetPowerDownTimer 
	**/
	MSG_SET_POWER_DOWN_TIMER_REQ  = 0x404E,	///<Payload type {::CAPI2_MS_SetPowerDownTimer_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SET_POWER_DOWN_TIMER_RSP  = 0x404F,
	 /** 
	api is CAPI2_SIM_GetSmsParamRecNum 
	**/
	MSG_SIM_PARAM_REC_NUM_REQ  = 0x4050,
	 /** 
	payload is ::SIM_INTEGER_DATA_t 
	**/
	MSG_SIM_PARAM_REC_NUM_RSP  = 0x4051,	///<Payload type {::SIM_INTEGER_DATA_t}
	 /** 
	api is CAPI2_MS_ConvertPLMNNameStr 
	**/
	MSG_MS_CONVERT_PLMN_STRING_REQ  = 0x4052,	///<Payload type {::CAPI2_MS_ConvertPLMNNameStr_t}
	 /** 
	payload is ::uchar_ptr_t 
	**/
	MSG_MS_CONVERT_PLMN_STRING_RSP  = 0x4053,	///<Payload type {::uchar_ptr_t}
	 /** 
	api is CAPI2_SIM_GetSmsMemExceededFlag 
	**/
	MSG_SIM_GET_SMSMEMEXC_FLAG_REQ  = 0x4054,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_GET_SMSMEMEXC_FLAG_RSP  = 0x4055,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SIM_IsTestSIM 
	**/
	MSG_SIM_IS_TEST_SIM_REQ  = 0x4056,
	 /** 
	payload is ::SIM_BOOLEAN_DATA_t 
	**/
	MSG_SIM_IS_TEST_SIM_RSP  = 0x4057,	///<Payload type {::SIM_BOOLEAN_DATA_t}
	 /** 
	api is CAPI2_SIM_IsPINRequired 
	**/
	MSG_SIM_PIN_REQ_REQ  = 0x4058,
	 /** 
	payload is ::SIM_BOOLEAN_DATA_t 
	**/
	MSG_SIM_PIN_REQ_RSP  = 0x4059,	///<Payload type {::SIM_BOOLEAN_DATA_t}
	 /** 
	api is CAPI2_SIM_GetCardPhase 
	**/
	MSG_SIM_CARD_PHASE_REQ  = 0x405A,
	 /** 
	payload is ::SIM_PHASE_RESULT_t 
	**/
	MSG_SIM_CARD_PHASE_RSP  = 0x405B,	///<Payload type {::SIM_PHASE_RESULT_t}
	 /** 
	api is CAPI2_SIM_GetSIMType 
	**/
	MSG_SIM_TYPE_REQ  = 0x405C,
	 /** 
	payload is ::SIM_TYPE_RESULT_t 
	**/
	MSG_SIM_TYPE_RSP  = 0x405D,	///<Payload type {::SIM_TYPE_RESULT_t}
	 /** 
	api is CAPI2_SIM_GetPresentStatus 
	**/
	MSG_SIM_PRESENT_REQ  = 0x405E,
	 /** 
	payload is ::SIM_PRESENT_RESULT_t 
	**/
	MSG_SIM_PRESENT_RSP  = 0x405F,	///<Payload type {::SIM_PRESENT_RESULT_t}
	 /** 
	api is CAPI2_SIM_IsOperationRestricted 
	**/
	MSG_SIM_PIN_OPERATION_REQ  = 0x4060,
	 /** 
	payload is ::SIM_BOOLEAN_DATA_t 
	**/
	MSG_SIM_PIN_OPERATION_RSP  = 0x4061,	///<Payload type {::SIM_BOOLEAN_DATA_t}
	 /** 
	api is CAPI2_SIM_IsPINBlocked 
	**/
	MSG_SIM_PIN_BLOCK_REQ  = 0x4062,	///<Payload type {::CAPI2_SIM_IsPINBlocked_Req_t}
	 /** 
	payload is ::SIM_BOOLEAN_DATA_t 
	**/
	MSG_SIM_PIN_BLOCK_RSP  = 0x4063,	///<Payload type {::SIM_BOOLEAN_DATA_t}
	 /** 
	api is CAPI2_SIM_IsPUKBlocked 
	**/
	MSG_SIM_PUK_BLOCK_REQ  = 0x4064,	///<Payload type {::CAPI2_SIM_IsPUKBlocked_Req_t}
	 /** 
	payload is ::SIM_BOOLEAN_DATA_t 
	**/
	MSG_SIM_PUK_BLOCK_RSP  = 0x4065,	///<Payload type {::SIM_BOOLEAN_DATA_t}
	 /** 
	api is CAPI2_SIM_IsInvalidSIM 
	**/
	MSG_SIM_IS_INVALID_REQ  = 0x4066,
	 /** 
	payload is ::SIM_BOOLEAN_DATA_t 
	**/
	MSG_SIM_IS_INVALID_RSP  = 0x4067,	///<Payload type {::SIM_BOOLEAN_DATA_t}
	 /** 
	api is CAPI2_SIM_DetectSim 
	**/
	MSG_SIM_DETECT_REQ  = 0x4068,
	 /** 
	payload is ::SIM_BOOLEAN_DATA_t 
	**/
	MSG_SIM_DETECT_RSP  = 0x4069,	///<Payload type {::SIM_BOOLEAN_DATA_t}
	 /** 
	api is CAPI2_SIM_GetRuimSuppFlag 
	**/
	MSG_SIM_GET_RUIM_SUPP_FLAG_REQ  = 0x406A,
	 /** 
	payload is ::SIM_BOOLEAN_DATA_t 
	**/
	MSG_SIM_GET_RUIM_SUPP_FLAG_RSP  = 0x406B,	///<Payload type {::SIM_BOOLEAN_DATA_t}
	 /** 
	api is CAPI2_SIM_SendVerifyChvReq 
	**/
	MSG_SIM_VERIFY_CHV_REQ  = 0x406C,	///<Payload type {::CAPI2_SIM_SendVerifyChvReq_Req_t}
	 /** 
	api is CAPI2_SIM_SendChangeChvReq 
	**/
	MSG_SIM_CHANGE_CHV_REQ  = 0x406D,	///<Payload type {::CAPI2_SIM_SendChangeChvReq_Req_t}
	 /** 
	api is CAPI2_SIM_SendSetChv1OnOffReq 
	**/
	MSG_SIM_ENABLE_CHV_REQ  = 0x406E,	///<Payload type {::CAPI2_SIM_SendSetChv1OnOffReq_Req_t}
	 /** 
	api is CAPI2_SIM_SendUnblockChvReq 
	**/
	MSG_SIM_UNBLOCK_CHV_REQ  = 0x406F,	///<Payload type {::CAPI2_SIM_SendUnblockChvReq_Req_t}
	 /** 
	api is CAPI2_SIM_SendSetOperStateReq 
	**/
	MSG_SIM_SET_FDN_REQ  = 0x4070,	///<Payload type {::CAPI2_SIM_SendSetOperStateReq_Req_t}
	 /** 
	api is CAPI2_SIM_IsPbkAccessAllowed 
	**/
	MSG_SIM_IS_PBK_ALLOWED_REQ  = 0x4071,	///<Payload type {::CAPI2_SIM_IsPbkAccessAllowed_Req_t}
	 /** 
	payload is ::SIM_BOOLEAN_DATA_t 
	**/
	MSG_SIM_IS_PBK_ALLOWED_RSP  = 0x4072,	///<Payload type {::SIM_BOOLEAN_DATA_t}
	 /** 
	api is CAPI2_SIM_SendPbkInfoReq 
	**/
	MSG_SIM_PBK_INFO_REQ  = 0x4073,	///<Payload type {::CAPI2_SIM_SendPbkInfoReq_Req_t}
	 /** 
	api is CAPI2_SIM_SendReadAcmMaxReq 
	**/
	MSG_SIM_MAX_ACM_REQ  = 0x4074,
	 /** 
	api is CAPI2_SIM_SendWriteAcmMaxReq 
	**/
	MSG_SIM_ACM_MAX_UPDATE_REQ  = 0x4075,	///<Payload type {::CAPI2_SIM_SendWriteAcmMaxReq_Req_t}
	 /** 
	api is CAPI2_SIM_SendReadAcmReq 
	**/
	MSG_SIM_ACM_VALUE_REQ  = 0x4076,
	 /** 
	api is CAPI2_SIM_SendWriteAcmReq 
	**/
	MSG_SIM_ACM_UPDATE_REQ  = 0x4077,	///<Payload type {::CAPI2_SIM_SendWriteAcmReq_Req_t}
	 /** 
	api is CAPI2_SIM_SendIncreaseAcmReq 
	**/
	MSG_SIM_ACM_INCREASE_REQ  = 0x4078,	///<Payload type {::CAPI2_SIM_SendIncreaseAcmReq_Req_t}
	 /** 
	api is CAPI2_SIM_SendReadSvcProvNameReq 
	**/
	MSG_SIM_SVC_PROV_NAME_REQ  = 0x4079,
	 /** 
	api is CAPI2_SIM_SendReadPuctReq 
	**/
	MSG_SIM_PUCT_DATA_REQ  = 0x407A,
	 /** 
	api is CAPI2_SIM_GetServiceStatus 
	**/
	MSG_SIM_SERVICE_STATUS_REQ  = 0x407B,	///<Payload type {::CAPI2_SIM_GetServiceStatus_Req_t}
	 /** 
	payload is ::SIM_SERVICE_STATUS_RESULT_t 
	**/
	MSG_SIM_SERVICE_STATUS_RSP  = 0x407C,	///<Payload type {::SIM_SERVICE_STATUS_RESULT_t}
	 /** 
	api is CAPI2_SIM_GetPinStatus 
	**/
	MSG_SIM_PIN_STATUS_REQ  = 0x407D,
	 /** 
	payload is ::SIM_PIN_STATUS_RESULT_t 
	**/
	MSG_SIM_PIN_STATUS_RSP  = 0x407E,	///<Payload type {::SIM_PIN_STATUS_RESULT_t}
	 /** 
	api is CAPI2_SIM_IsPinOK 
	**/
	MSG_SIM_PIN_OK_STATUS_REQ  = 0x407F,
	 /** 
	payload is ::SIM_BOOLEAN_DATA_t 
	**/
	MSG_SIM_PIN_OK_STATUS_RSP  = 0x4080,	///<Payload type {::SIM_BOOLEAN_DATA_t}
	 /** 
	api is CAPI2_SIM_GetIMSI 
	**/
	MSG_SIM_IMSI_REQ  = 0x4081,
	 /** 
	payload is ::SIM_IMSI_RESULT_t 
	**/
	MSG_SIM_IMSI_RSP  = 0x4082,	///<Payload type {::SIM_IMSI_RESULT_t}
	 /** 
	api is CAPI2_SIM_GetGID1 
	**/
	MSG_SIM_GID_DIGIT_REQ  = 0x4083,
	 /** 
	payload is ::SIM_GID_RESULT_t 
	**/
	MSG_SIM_GID_DIGIT_RSP  = 0x4084,	///<Payload type {::SIM_GID_RESULT_t}
	 /** 
	api is CAPI2_SIM_GetGID2 
	**/
	MSG_SIM_GID_DIGIT2_REQ  = 0x4085,
	 /** 
	payload is ::SIM_GID_RESULT_t 
	**/
	MSG_SIM_GID_DIGIT2_RSP  = 0x4086,	///<Payload type {::SIM_GID_RESULT_t}
	 /** 
	api is CAPI2_SIM_GetHomePlmn 
	**/
	MSG_SIM_HOME_PLMN_REQ  = 0x4087,
	 /** 
	payload is ::SIM_HOME_PLMN_RESULT_t 
	**/
	MSG_SIM_HOME_PLMN_RSP  = 0x4088,	///<Payload type {::SIM_HOME_PLMN_RESULT_t}
	 /** 
	api is CAPI2_simmi_GetMasterFileId 
	**/
	MSG_SIM_APDU_FILEID_REQ  = 0x4089,	///<Payload type {::CAPI2_simmi_GetMasterFileId_Req_t}
	 /** 
	payload is ::SIM_APDU_FILEID_RESULT_t 
	**/
	MSG_SIM_APDU_FILEID_RSP  = 0x408A,	///<Payload type {::SIM_APDU_FILEID_RESULT_t}
	 /** 
	api is CAPI2_SIM_SendOpenSocketReq 
	**/
	MSG_SIM_OPEN_SOCKET_REQ  = 0x408B,
	 /** 
	api is CAPI2_SIM_SendSelectAppiReq 
	**/
	MSG_SIM_SELECT_APPLI_REQ  = 0x408C,	///<Payload type {::CAPI2_SimSelectAppli_t}
	 /** 
	api is CAPI2_SIM_SendDeactivateAppiReq 
	**/
	MSG_SIM_DEACTIVATE_APPLI_REQ  = 0x408D,	///<Payload type {::CAPI2_SimDeactivateAppli_t}
	 /** 
	api is CAPI2_SIM_SendCloseSocketReq 
	**/
	MSG_SIM_CLOSE_SOCKET_REQ  = 0x408E,	///<Payload type {::CAPI2_SimCloseSocket_t}
	 /** 
	api is CAPI2_SIM_GetAtrData 
	**/
	MSG_SIM_ATR_DATA_REQ  = 0x408F,
	 /** 
	payload is ::SIM_ATR_DATA_t 
	**/
	MSG_SIM_ATR_DATA_RSP  = 0x4090,	///<Payload type {::SIM_ATR_DATA_t}
	 /** 
	api is CAPI2_SIM_SubmitDFileInfoReq 
	**/
	MSG_SIM_DFILE_INFO_REQ  = 0x4091,	///<Payload type {::CAPI2_SimFileInfo_t}
	 /** 
	api is CAPI2_SIM_SubmitEFileInfoReq 
	**/
	MSG_SIM_EFILE_INFO_REQ  = 0x4092,	///<Payload type {::CAPI2_SimFileInfo_t}
	 /** 
	api is CAPI2_SIM_SubmitWholeBinaryEFileReadReq 
	**/
	MSG_SIM_WHOLE_EFILE_DATA_REQ  = 0x4093,	///<Payload type {::CAPI2_SimFileInfo_t}
	 /** 
	api is CAPI2_SIM_SubmitBinaryEFileReadReq 
	**/
	MSG_SIM_EFILE_DATA_REQ  = 0x4094,	///<Payload type {::CAPI2_SimFileInfo_t}
	 /** 
	api is CAPI2_SIM_SubmitRecordEFileReadReq 
	**/
	MSG_SIM_RECORD_EFILE_DATA_REQ  = 0x4095,	///<Payload type {::CAPI2_SimFileInfo_t}
	 /** 
	api is CAPI2_SIM_SubmitBinaryEFileUpdateReq 
	**/
	MSG_SIM_EFILE_UPDATE_REQ  = 0x4096,	///<Payload type {::CAPI2_SimFileInfo_t}
	 /** 
	api is CAPI2_SIM_SubmitLinearEFileUpdateReq 
	**/
	MSG_SIM_LINEAR_EFILE_UPDATE_REQ  = 0x4097,	///<Payload type {::CAPI2_SimFileInfo_t}
	 /** 
	api is CAPI2_SIM_SubmitSeekRecordReq 
	**/
	MSG_SIM_SEEK_RECORD_REQ  = 0x4098,	///<Payload type {::CAPI2_SimFileInfo_t}
	 /** 
	api is CAPI2_SIM_SubmitCyclicEFileUpdateReq 
	**/
	MSG_SIM_CYCLIC_EFILE_UPDATE_REQ  = 0x4099,	///<Payload type {::CAPI2_SimFileInfo_t}
	 /** 
	api is CAPI2_SIM_SendRemainingPinAttemptReq 
	**/
	MSG_SIM_PIN_ATTEMPT_REQ  = 0x409A,
	 /** 
	api is CAPI2_SIM_IsCachedDataReady 
	**/
	MSG_SIM_CACHE_DATA_READY_REQ  = 0x409B,
	 /** 
	payload is ::SIM_BOOLEAN_DATA_t 
	**/
	MSG_SIM_CACHE_DATA_READY_RSP  = 0x409C,	///<Payload type {::SIM_BOOLEAN_DATA_t}
	 /** 
	api is CAPI2_SIM_GetServiceCodeStatus 
	**/
	MSG_SIM_SERVICE_CODE_STATUS_REQ  = 0x409D,	///<Payload type {::CAPI2_SIM_GetServiceCodeStatus_Req_t}
	 /** 
	payload is ::SIM_SERVICE_FLAG_STATUS_t 
	**/
	MSG_SIM_SERVICE_CODE_STATUS_RSP  = 0x409E,	///<Payload type {::SIM_SERVICE_FLAG_STATUS_t}
	 /** 
	api is CAPI2_SIM_CheckCphsService 
	**/
	MSG_SIM_CHECK_CPHS_REQ  = 0x409F,	///<Payload type {::CAPI2_SIM_CheckCphsService_Req_t}
	 /** 
	payload is ::SIM_BOOLEAN_DATA_t 
	**/
	MSG_SIM_CHECK_CPHS_RSP  = 0x40A0,	///<Payload type {::SIM_BOOLEAN_DATA_t}
	 /** 
	api is CAPI2_SIM_GetCphsPhase 
	**/
	MSG_SIM_CPHS_PHASE_REQ  = 0x40A1,
	 /** 
	payload is ::SIM_INTEGER_DATA_t 
	**/
	MSG_SIM_CPHS_PHASE_RSP  = 0x40A2,	///<Payload type {::SIM_INTEGER_DATA_t}
	 /** 
	api is CAPI2_SIM_GetSmsSca 
	**/
	MSG_SIM_SMS_SCA_REQ  = 0x40A3,	///<Payload type {::CAPI2_SIM_GetSmsSca_Req_t}
	 /** 
	payload is ::SIM_SCA_DATA_RESULT_t 
	**/
	MSG_SIM_SMS_SCA_RSP  = 0x40A4,	///<Payload type {::SIM_SCA_DATA_RESULT_t}
	 /** 
	api is CAPI2_SIM_GetIccid 
	**/
	MSG_SIM_ICCID_PARAM_REQ  = 0x40A5,
	 /** 
	payload is ::SIM_ICCID_STATUS_t 
	**/
	MSG_SIM_ICCID_PARAM_RSP  = 0x40A6,	///<Payload type {::SIM_ICCID_STATUS_t}
	 /** 
	api is CAPI2_SIM_IsALSEnabled 
	**/
	MSG_SIM_ALS_STATUS_REQ  = 0x40A7,
	 /** 
	payload is ::SIM_BOOLEAN_DATA_t 
	**/
	MSG_SIM_ALS_STATUS_RSP  = 0x40A8,	///<Payload type {::SIM_BOOLEAN_DATA_t}
	 /** 
	api is CAPI2_SIM_GetAlsDefaultLine 
	**/
	MSG_SIM_ALS_DEFAULT_LINE_REQ  = 0x40A9,
	 /** 
	payload is ::SIM_INTEGER_DATA_t 
	**/
	MSG_SIM_ALS_DEFAULT_LINE_RSP  = 0x40AA,	///<Payload type {::SIM_INTEGER_DATA_t}
	 /** 
	api is CAPI2_SIM_SetAlsDefaultLine 
	**/
	MSG_SIM_SET_ALS_DEFAULT_REQ  = 0x40AB,	///<Payload type {::CAPI2_SIM_SetAlsDefaultLine_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SIM_SET_ALS_DEFAULT_RSP  = 0x40AC,
	 /** 
	api is CAPI2_SIM_GetCallForwardUnconditionalFlag 
	**/
	MSG_SIM_CALLFWD_COND_FLAG_REQ  = 0x40AD,
	 /** 
	payload is ::SIM_CALL_FORWARD_UNCONDITIONAL_STATUS_t 
	**/
	MSG_SIM_CALLFWD_COND_FLAG_RSP  = 0x40AE,	///<Payload type {::SIM_CALL_FORWARD_UNCONDITIONAL_STATUS_t}
	 /** 
	api is CAPI2_SIM_GetApplicationType 
	**/
	MSG_SIM_APP_TYPE_REQ  = 0x40AF,
	 /** 
	payload is ::SIM_APPL_TYPE_RESULT_t 
	**/
	MSG_SIM_APP_TYPE_RSP  = 0x40B0,	///<Payload type {::SIM_APPL_TYPE_RESULT_t}
	 /** 
	api is CAPI2_USIM_GetUst 
	**/
	MSG_USIM_UST_DATA_REQ  = 0x40B1,
	 /** 
	payload is ::USIM_UST_DATA_RSP_t 
	**/
	MSG_USIM_UST_DATA_RSP  = 0x40B2,	///<Payload type {::USIM_UST_DATA_RSP_t}
	 /** 
	api is CAPI2_SIM_SendWritePuctReq 
	**/
	MSG_SIM_PUCT_UPDATE_REQ  = 0x40B3,	///<Payload type {::CAPI2_SIM_SendWritePuctReq_t}
	 /** 
	api is CAPI2_SIM_SubmitRestrictedAccessReq 
	**/
	MSG_SIM_RESTRICTED_ACCESS_REQ  = 0x40B4,	///<Payload type {::CAPI2_SIM_SendRestrictedAccessReq_t}
	 /** 
	api is CAPI2_ADCMGR_Start 
	**/
	MSG_ADC_START_REQ  = 0x40B5,	///<Payload type {::CAPI2_ADCMGR_Start_Req_t}
	 /** 
	api is CAPI2_AT_ProcessCmd 
	**/
	MSG_CAPI2_AT_COMMAND_REQ  = 0x40B6,	///<Payload type {::CAPI2_AT_ProcessCmd_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_CAPI2_AT_COMMAND_RSP  = 0x40B7,
	 /** 
	api is CAPI2_MS_GetSystemRAT 
	**/
	MSG_MS_GET_SYSTEM_RAT_REQ  = 0x40B8,
	 /** 
	payload is ::RATSelect_t 
	**/
	MSG_MS_GET_SYSTEM_RAT_RSP  = 0x40B9,	///<Payload type {::RATSelect_t}
	 /** 
	api is CAPI2_MS_GetSupportedRAT 
	**/
	MSG_MS_GET_SUPPORTED_RAT_REQ  = 0x40BA,
	 /** 
	payload is ::RATSelect_t 
	**/
	MSG_MS_GET_SUPPORTED_RAT_RSP  = 0x40BB,	///<Payload type {::RATSelect_t}
	 /** 
	api is CAPI2_MS_GetSystemBand 
	**/
	MSG_MS_GET_SYSTEM_BAND_REQ  = 0x40BC,
	 /** 
	payload is ::BandSelect_t 
	**/
	MSG_MS_GET_SYSTEM_BAND_RSP  = 0x40BD,	///<Payload type {::BandSelect_t}
	 /** 
	api is CAPI2_MS_GetSupportedBand 
	**/
	MSG_MS_GET_SUPPORTED_BAND_REQ  = 0x40BE,
	 /** 
	payload is ::BandSelect_t 
	**/
	MSG_MS_GET_SUPPORTED_BAND_RSP  = 0x40BF,	///<Payload type {::BandSelect_t}
	 /** 
	api is CAPI2_SYSPARM_GetMSClass 
	**/
	MSG_SYSPARAM_GET_MSCLASS_REQ  = 0x40C0,
	 /** 
	payload is ::UInt16 
	**/
	MSG_SYSPARAM_GET_MSCLASS_RSP  = 0x40C1,	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_SYSPARM_GetManufacturerName 
	**/
	MSG_SYSPARAM_GET_MNF_NAME_REQ  = 0x40C2,
	 /** 
	payload is ::uchar_ptr_t 
	**/
	MSG_SYSPARAM_GET_MNF_NAME_RSP  = 0x40C3,	///<Payload type {::uchar_ptr_t}
	 /** 
	api is CAPI2_SYSPARM_GetModelName 
	**/
	MSG_SYSPARAM_GET_MODEL_NAME_REQ  = 0x40C4,
	 /** 
	payload is ::uchar_ptr_t 
	**/
	MSG_SYSPARAM_GET_MODEL_NAME_RSP  = 0x40C5,	///<Payload type {::uchar_ptr_t}
	 /** 
	api is CAPI2_SYSPARM_GetSWVersion 
	**/
	MSG_SYSPARAM_GET_SW_VERSION_REQ  = 0x40C6,
	 /** 
	payload is ::uchar_ptr_t 
	**/
	MSG_SYSPARAM_GET_SW_VERSION_RSP  = 0x40C7,	///<Payload type {::uchar_ptr_t}
	 /** 
	api is CAPI2_SYSPARM_GetEGPRSMSClass 
	**/
	MSG_SYSPARAM_GET_EGPRS_CLASS_REQ  = 0x40C8,
	 /** 
	payload is ::UInt16 
	**/
	MSG_SYSPARAM_GET_EGPRS_CLASS_RSP  = 0x40C9,	///<Payload type {::UInt16}
	 
	 /** 
	api is xxxxx 
	**/
	//available  = 0x40CA,
	 
	 /** 
	payload is ::MSImeiStr_t 
	**/
	MSG_SYSPARAM_GET_IMEI_STR_RSP  = 0x40CB,	///<Payload type {::MSImeiStr_t}
	 /** 
	api is CAPI2_MS_GetRegistrationInfo 
	**/
	MSG_MS_GET_REG_INFO_REQ  = 0x40CC,
	 /** 
	payload is ::MSRegStateInfo_t 
	**/
	MSG_MS_GET_REG_INFO_RSP  = 0x40CD,	///<Payload type {::MSRegStateInfo_t}
	 /** 
	api is CAPI2_SIM_SendNumOfPLMNEntryReq 
	**/
	MSG_SIM_PLMN_NUM_OF_ENTRY_REQ  = 0x40CE,	///<Payload type {::CAPI2_SIM_SendNumOfPLMNEntryReq_Req_t}
	 /** 
	api is CAPI2_SIM_SendReadPLMNEntryReq 
	**/
	MSG_SIM_PLMN_ENTRY_DATA_REQ  = 0x40CF,	///<Payload type {::CAPI2_SIM_SendReadPLMNEntryReq_Req_t}
	 /** 
	api is CAPI2_SIM_SendWriteMulPLMNEntryReq 
	**/
	MSG_SIM_MUL_PLMN_ENTRY_UPDATE_REQ  = 0x40D0,	///<Payload type {::CAPI2_SendWriteMulPLMNEntry_t}
	 /** 
	api is CAPI2_SYS_SetRegisteredEventMask 
	**/
	MSG_SYS_SET_REG_EVENT_MASK_REQ  = 0x40D1,	///<Payload type {::CAPI2_SYS_SetRegisteredEventMask_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SYS_SET_REG_EVENT_MASK_RSP  = 0x40D2,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SYS_SetFilteredEventMask 
	**/
	MSG_SYS_SET_REG_FILTER_MASK_REQ  = 0x40D3,	///<Payload type {::CAPI2_SYS_SetFilteredEventMask_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SYS_SET_REG_FILTER_MASK_RSP  = 0x40D4,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SYS_SetRssiThreshold 
	**/
	MSG_SYS_SET_RSSI_THRESHOLD_REQ  = 0x40D5,	///<Payload type {::CAPI2_SYS_SetRssiThreshold_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SYS_SET_RSSI_THRESHOLD_RSP  = 0x40D6,
	 /** 
	api is CAPI2_SYS_GetBootLoaderVersion 
	**/
	MSG_SYSPARAM_BOOTLOADER_VER_REQ  = 0x40D7,
	 /** 
	payload is ::uchar_ptr_t 
	**/
	MSG_SYSPARAM_BOOTLOADER_VER_RSP  = 0x40D8,	///<Payload type {::uchar_ptr_t}
	 /** 
	api is CAPI2_SYS_GetDSFVersion 
	**/
	MSG_SYSPARAM_DSF_VER_REQ  = 0x40D9,
	 /** 
	payload is ::uchar_ptr_t 
	**/
	MSG_SYSPARAM_DSF_VER_RSP  = 0x40DA,	///<Payload type {::uchar_ptr_t}
	 /** 
	api is CAPI2_SYSPARM_GetChanMode 
	**/
	MSG_SYSPARAM_GET_CHANNEL_MODE_REQ  = 0x40DB,
	 /** 
	payload is ::UInt16 
	**/
	MSG_SYSPARAM_GET_CHANNEL_MODE_RSP  = 0x40DC,	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_SYSPARM_GetClassmark 
	**/
	MSG_SYSPARAM_GET_CLASSMARK_REQ  = 0x40DD,
	 /** 
	payload is ::CAPI2_Class_t 
	**/
	MSG_SYSPARAM_GET_CLASSMARK_RSP  = 0x40DE,	///<Payload type {::CAPI2_Class_t}
	 /** 
	api is CAPI2_SYSPARM_GetSysparmIndPartFileVersion 
	**/
	MSG_SYSPARAM_GET_IND_FILE_VER_REQ  = 0x40DF,
	 /** 
	payload is ::UInt16 
	**/
	MSG_SYSPARAM_GET_IND_FILE_VER_RSP  = 0x40E0,	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_SYSPARM_SetDARPCfg 
	**/
	MSG_SYS_SET_DARP_CFG_REQ  = 0x40E1,	///<Payload type {::CAPI2_SYSPARM_SetDARPCfg_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SYS_SET_DARP_CFG_RSP  = 0x40E2,
	 /** 
	api is CAPI2_SYSPARM_SetEGPRSMSClass 
	**/
	MSG_SYSPARAM_SET_EGPRS_MSCLASS_REQ  = 0x40E3,	///<Payload type {::CAPI2_SYSPARM_SetEGPRSMSClass_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SYSPARAM_SET_EGPRS_MSCLASS_RSP  = 0x40E4,
	 /** 
	api is CAPI2_SYSPARM_SetGPRSMSClass 
	**/
	MSG_SYSPARAM_SET_GPRS_MSCLASS_REQ  = 0x40E5,	///<Payload type {::CAPI2_SYSPARM_SetGPRSMSClass_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SYSPARAM_SET_GPRS_MSCLASS_RSP  = 0x40E6,
	 /** 
	api is CAPI2_TIMEZONE_DeleteNetworkName 
	**/
	MSG_TIMEZONE_DELETE_NW_NAME_REQ  = 0x40E7,
	 /** 
	payload is ::default_proc 
	**/
	MSG_TIMEZONE_DELETE_NW_NAME_RSP  = 0x40E8,
	 /** 
	api is CAPI2_TIMEZONE_GetTZUpdateMode 
	**/
	MSG_TIMEZONE_GET_UPDATE_MODE_REQ  = 0x40E9,
	 /** 
	payload is ::TimeZoneUpdateMode_t 
	**/
	MSG_TIMEZONE_GET_UPDATE_MODE_RSP  = 0x40EA,	///<Payload type {::TimeZoneUpdateMode_t}
	 /** 
	api is CAPI2_TIMEZONE_SetTZUpdateMode 
	**/
	MSG_TIMEZONE_SET_UPDATE_MODE_REQ  = 0x40EB,	///<Payload type {::CAPI2_TIMEZONE_SetTZUpdateMode_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_TIMEZONE_SET_UPDATE_MODE_RSP  = 0x40EC,
	 /** 
	api is CAPI2_TIMEZONE_UpdateRTC 
	**/
	MSG_TIMEZONE_UPDATE_RTC_REQ  = 0x40ED,	///<Payload type {::CAPI2_TIMEZONE_UpdateRTC_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_TIMEZONE_UPDATE_RTC_RSP  = 0x40EE,
	 /** 
	api is CAPI2_TestCmds 
	**/
	MSG_CAPI2_TEST_REQ  = 0x40EF,	///<Payload type {::CAPI2_TestCmds_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_CAPI2_TEST_RSP  = 0x40F0,
	 /** 
	api is CAPI2_SATK_SendPlayToneRes 
	**/
	MSG_STK_SEND_PLAYTONE_RES_REQ  = 0x40F1,	///<Payload type {::CAPI2_SATK_SendPlayToneRes_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_STK_SEND_PLAYTONE_RES_RSP  = 0x40F2,
	 /** 
	api is CAPI2_SATK_SendSetupCallRes 
	**/
	MSG_STK_SETUP_CALL_RES_REQ  = 0x40F3,	///<Payload type {::CAPI2_SATK_SendSetupCallRes_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_STK_SETUP_CALL_RES_RSP  = 0x40F4,
	 /** 
	api is CAPI2_PBK_SetFdnCheck 
	**/
	MSG_PBK_SET_FDN_CHECK_REQ  = 0x40F5,	///<Payload type {::CAPI2_PBK_SetFdnCheck_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_PBK_SET_FDN_CHECK_RSP  = 0x40F6,
	 /** 
	api is CAPI2_PBK_GetFdnCheck 
	**/
	MSG_PBK_GET_FDN_CHECK_REQ  = 0x40F7,
	 /** 
	payload is ::Boolean 
	**/
	MSG_PBK_GET_FDN_CHECK_RSP  = 0x40F8,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PMU_Battery_Register 
	**/
	MSG_PMU_BATT_LEVEL_REGISTER_REQ  = 0x40F9,	///<Payload type {::CAPI2_PMU_Battery_Register_Req_t}
	 /** 
	payload is ::HAL_EM_BATTMGR_ErrorCode_en_t 
	**/
	MSG_PMU_BATT_LEVEL_REGISTER_RSP  = 0x40FA,	///<Payload type {::HAL_EM_BATTMGR_ErrorCode_en_t}
	 /** 
	payload is ::HAL_EM_BatteryLevel_t 
	**/
	MSG_PMU_BATT_LEVEL_IND  = 0x40FB,	///<Payload type {::HAL_EM_BatteryLevel_t}
	 /** 
	api is CAPI2_SMS_SendMemAvailInd 
	**/
	MSG_SMS_SEND_MEM_AVAL_IND_REQ  = 0x40FC,
	 /** 
	payload is ::default_proc 
	**/
	MSG_SMS_SEND_MEM_AVAL_IND_RSP  = 0x40FD,
	 /** 
	api is CAPI2_SMS_ConfigureMEStorage 
	**/
	MSG_SMS_CONFIGUREMESTORAGE_REQ  = 0x40FE,	///<Payload type {::CAPI2_SMS_ConfigureMEStorage_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SMS_CONFIGUREMESTORAGE_RSP  = 0x40FF,
	 /** 
	api is CAPI2_MS_SetElement 
	**/
	MSG_MS_SET_ELEMENT_REQ  = 0x4100,	///<Payload type {::CAPI2_MS_SetElement_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_MS_SET_ELEMENT_RSP  = 0x4101,
	 /** 
	api is CAPI2_MS_GetElement 
	**/
	MSG_MS_GET_ELEMENT_REQ  = 0x4102,	///<Payload type {::CAPI2_MS_GetElement_Req_t}
	 /** 
	payload is ::CAPI2_MS_Element_t 
	**/
	MSG_MS_GET_ELEMENT_RSP  = 0x4103,	///<Payload type {::CAPI2_MS_Element_t}
	 /** 
	api is CAPI2_USIM_IsApplicationSupported 
	**/
	MSG_USIM_IS_APP_SUPPORTED_REQ  = 0x4104,	///<Payload type {::CAPI2_USIM_IsApplicationSupported_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_USIM_IS_APP_SUPPORTED_RSP  = 0x4105,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_USIM_IsAllowedAPN 
	**/
	MSG_USIM_IS_APN_ALLOWED_REQ  = 0x4106,	///<Payload type {::CAPI2_USIM_IsAllowedAPN_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_USIM_IS_APN_ALLOWED_RSP  = 0x4107,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_USIM_GetNumOfAPN 
	**/
	MSG_USIM_GET_NUM_APN_REQ  = 0x4108,
	 /** 
	payload is ::UInt8 
	**/
	MSG_USIM_GET_NUM_APN_RSP  = 0x4109,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_USIM_GetAPNEntry 
	**/
	MSG_USIM_GET_APN_ENTRY_REQ  = 0x410A,	///<Payload type {::CAPI2_USIM_GetAPNEntry_Req_t}
	 /** 
	payload is ::APN_NAME_t 
	**/
	MSG_USIM_GET_APN_ENTRY_RSP  = 0x410B,	///<Payload type {::APN_NAME_t}
	 /** 
	api is CAPI2_USIM_IsEstServActivated 
	**/
	MSG_USIM_IS_EST_SERV_ACTIVATED_REQ  = 0x410C,	///<Payload type {::CAPI2_USIM_IsEstServActivated_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_USIM_IS_EST_SERV_ACTIVATED_RSP  = 0x410D,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_USIM_SendSetEstServReq 
	**/
	MSG_SIM_SET_EST_SERV_REQ  = 0x410E,	///<Payload type {::CAPI2_USIM_SendSetEstServReq_Req_t}
	 /** 
	api is CAPI2_USIM_SendWriteAPNReq 
	**/
	MSG_SIM_UPDATE_ONE_APN_REQ  = 0x410F,	///<Payload type {::CAPI2_USIM_SendWriteAPNReq_Req_t}
	 /** 
	api is CAPI2_USIM_SendDeleteAllAPNReq 
	**/
	MSG_SIM_DELETE_ALL_APN_REQ  = 0x4110,
	 /** 
	api is CAPI2_USIM_GetRatModeSetting 
	**/
	MSG_USIM_GET_RAT_MODE_REQ  = 0x4111,
	 /** 
	payload is ::USIM_RAT_MODE_t 
	**/
	MSG_USIM_GET_RAT_MODE_RSP  = 0x4112,	///<Payload type {::USIM_RAT_MODE_t}
	 /** 
	api is CAPI2_MS_GetGPRSRegState 
	**/
	MSG_MS_GET_GPRS_STATE_REQ  = 0x4113,
	 /** 
	payload is ::MSRegState_t 
	**/
	MSG_MS_GET_GPRS_STATE_RSP  = 0x4114,	///<Payload type {::MSRegState_t}
	 /** 
	api is CAPI2_MS_GetGSMRegState 
	**/
	MSG_MS_GET_GSM_STATE_REQ  = 0x4115,
	 /** 
	payload is ::MSRegState_t 
	**/
	MSG_MS_GET_GSM_STATE_RSP  = 0x4116,	///<Payload type {::MSRegState_t}
	 /** 
	api is CAPI2_MS_GetRegisteredCellInfo 
	**/
	MSG_MS_GET_CELL_INFO_REQ  = 0x4117,
	 /** 
	payload is ::CellInfo_t 
	**/
	MSG_MS_GET_CELL_INFO_RSP  = 0x4118,	///<Payload type {::CellInfo_t}
	 /** 
	api is CAPI2_MS_SetMEPowerClass 
	**/
	MSG_MS_SETMEPOWER_CLASS_REQ  = 0x4119,	///<Payload type {::CAPI2_MS_SetMEPowerClass_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_MS_SETMEPOWER_CLASS_RSP  = 0x411A,
	 /** 
	api is CAPI2_USIM_GetServiceStatus 
	**/
	MSG_USIM_GET_SERVICE_STATUS_REQ  = 0x411B,	///<Payload type {::CAPI2_USIM_GetServiceStatus_Req_t}
	 /** 
	payload is ::SIMServiceStatus_t 
	**/
	MSG_USIM_GET_SERVICE_STATUS_RSP  = 0x411C,	///<Payload type {::SIMServiceStatus_t}
	 /** 
	api is CAPI2_SIM_IsAllowedAPN 
	**/
	MSG_SIM_IS_ALLOWED_APN_REQ  = 0x411D,	///<Payload type {::CAPI2_SIM_IsAllowedAPN_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_IS_ALLOWED_APN_RSP  = 0x411E,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SMS_GetSmsMaxCapacity 
	**/
	MSG_SMS_GETSMSMAXCAPACITY_REQ  = 0x411F,	///<Payload type {::CAPI2_SMS_GetSmsMaxCapacity_Req_t}
	 /** 
	payload is ::UInt16 
	**/
	MSG_SMS_GETSMSMAXCAPACITY_RSP  = 0x4120,	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_SMS_RetrieveMaxCBChnlLength 
	**/
	MSG_SMS_RETRIEVEMAXCBCHNLLENGTH_REQ  = 0x4121,
	 /** 
	payload is ::UInt8 
	**/
	MSG_SMS_RETRIEVEMAXCBCHNLLENGTH_RSP  = 0x4122,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_SIM_IsBdnOperationRestricted 
	**/
	MSG_SIM_IS_BDN_RESTRICTED_REQ  = 0x4123,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_IS_BDN_RESTRICTED_RSP  = 0x4124,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SIM_SendPreferredPlmnUpdateInd 
	**/
	MSG_SIM_SEND_PLMN_UPDATE_IND_REQ  = 0x4125,	///<Payload type {::CAPI2_SIM_SendPreferredPlmnUpdateInd_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SIM_SEND_PLMN_UPDATE_IND_RSP  = 0x4126,
	 /** 
	api is CAPI2_SIMIO_DeactiveCard 
	**/
	MSG_SIMIO_DEACTIVATE_CARD_REQ  = 0x4127,
	 /** 
	payload is ::default_proc 
	**/
	MSG_SIMIO_DEACTIVATE_CARD_RSP  = 0x4128,
	 /** 
	api is CAPI2_SIM_SendSetBdnReq 
	**/
	MSG_SIM_SET_BDN_REQ  = 0x4129,	///<Payload type {::CAPI2_SIM_SendSetBdnReq_Req_t}
	 /** 
	api is CAPI2_SIM_PowerOnOffCard 
	**/
	MSG_SIM_POWER_ON_OFF_CARD_REQ  = 0x412A,	///<Payload type {::CAPI2_SIM_PowerOnOffCard_Req_t}
	 /** 
	api is CAPI2_SIM_GetRawAtr 
	**/
	MSG_SIM_GET_RAW_ATR_REQ  = 0x412B,
	 /** 
	api is CAPI2_SIM_Set_Protocol 
	**/
	MSG_SIM_SET_PROTOCOL_REQ  = 0x412C,	///<Payload type {::CAPI2_SIM_Set_Protocol_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SIM_SET_PROTOCOL_RSP  = 0x412D,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SIM_Get_Protocol 
	**/
	MSG_SIM_GET_PROTOCOL_REQ  = 0x412E,
	 /** 
	payload is ::UInt8 
	**/
	MSG_SIM_GET_PROTOCOL_RSP  = 0x412F,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_SIM_SendGenericApduCmd 
	**/
	MSG_SIM_SEND_GENERIC_APDU_CMD_REQ  = 0x4130,	///<Payload type {::CAPI2_SIM_SendGenericApduCmd_Req_t}
	 /** 
	api is CAPI2_SIM_TerminateXferApdu 
	**/
	MSG_SIM_TERMINATE_XFER_APDU_REQ  = 0x4131,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SIM_TERMINATE_XFER_APDU_RSP  = 0x4132,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SIM_GetSimInterface 
	**/
	MSG_SIM_GET_SIM_INTERFACE_REQ  = 0x4133,
	 /** 
	payload is ::SIM_SIM_INTERFACE_t 
	**/
	MSG_SIM_GET_SIM_INTERFACE_RSP  = 0x4134,	///<Payload type {::SIM_SIM_INTERFACE_t}
	 /** 
	api is CAPI2_MS_SetPlmnSelectRat 
	**/
	MSG_SET_PLMN_SELECT_RAT_REQ  = 0x4135,	///<Payload type {::CAPI2_MS_SetPlmnSelectRat_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SET_PLMN_SELECT_RAT_RSP  = 0x4136,
	 /** 
	api is CAPI2_MS_IsDeRegisterInProgress 
	**/
	MSG_IS_DEREGISTER_IN_PROGRESS_REQ  = 0x4137,
	 /** 
	payload is ::Boolean 
	**/
	MSG_IS_DEREGISTER_IN_PROGRESS_RSP  = 0x4138,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_MS_IsRegisterInProgress 
	**/
	MSG_IS_REGISTER_IN_PROGRESS_REQ  = 0x4139,
	 /** 
	payload is ::Boolean 
	**/
	MSG_IS_REGISTER_IN_PROGRESS_RSP  = 0x413A,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PBK_SendUsimHdkReadReq 
	**/
	MSG_READ_USIM_PBK_HDK_REQ  = 0x413B,
	 /** 
	api is CAPI2_PBK_SendUsimHdkUpdateReq 
	**/
	MSG_WRITE_USIM_PBK_HDK_REQ  = 0x413C,	///<Payload type {::CAPI2_PBK_SendUsimHdkUpdateReq_Req_t}
	 /** 
	api is CAPI2_PBK_SendUsimAasReadReq 
	**/
	MSG_READ_USIM_PBK_ALPHA_AAS_REQ  = 0x413D,	///<Payload type {::CAPI2_PBK_SendUsimAasReadReq_Req_t}
	 /** 
	api is CAPI2_PBK_SendUsimAasUpdateReq 
	**/
	MSG_WRITE_USIM_PBK_ALPHA_AAS_REQ  = 0x413E,	///<Payload type {::CAPI2_PBK_SendUsimAasUpdateReq_Req_t}
	 /** 
	api is CAPI2_PBK_SendUsimGasReadReq 
	**/
	MSG_READ_USIM_PBK_ALPHA_GAS_REQ  = 0x413F,	///<Payload type {::CAPI2_PBK_SendUsimGasReadReq_Req_t}
	 /** 
	api is CAPI2_PBK_SendUsimGasUpdateReq 
	**/
	MSG_WRITE_USIM_PBK_ALPHA_GAS_REQ  = 0x4140,	///<Payload type {::CAPI2_PBK_SendUsimGasUpdateReq_Req_t}
	 /** 
	api is CAPI2_PBK_SendUsimAasInfoReq 
	**/
	MSG_GET_USIM_PBK_ALPHA_INFO_AAS_REQ  = 0x4141,	///<Payload type {::CAPI2_PBK_SendUsimAasInfoReq_Req_t}
	 /** 
	api is CAPI2_PBK_SendUsimGasInfoReq 
	**/
	MSG_GET_USIM_PBK_ALPHA_INFO_GAS_REQ  = 0x4142,	///<Payload type {::CAPI2_PBK_SendUsimGasInfoReq_Req_t}
	 /** 
	api is CAPI2_DIAG_ApiMeasurmentReportReq 
	**/
	MSG_DIAG_MEASURE_REPORT_REQ  = 0x4143,	///<Payload type {::CAPI2_DIAG_ApiMeasurmentReportReq_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DIAG_MEASURE_REPORT_RSP  = 0x4144,
	 /** 
	api is CAPI2_PMU_BattChargingNotification 
	**/
	MSG_PMU_BATT_CHARGING_NOTIFICATION_REQ  = 0x4145,	///<Payload type {::CAPI2_PMU_BattChargingNotification_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_PMU_BATT_CHARGING_NOTIFICATION_RSP  = 0x4146,
	 /** 
	api is CAPI2_PATCH_GetRevision 
	**/
	MSG_PATCH_GET_REVISION_REQ  = 0x4147,
	 /** 
	payload is ::CAPI2_Patch_Revision_Ptr_t 
	**/
	MSG_PATCH_GET_REVISION_RSP  = 0x4148,	///<Payload type {::CAPI2_Patch_Revision_Ptr_t}
	 /** 
	api is CAPI2_MS_InitCallCfg 
	**/
	MSG_MS_INITCALLCFG_REQ  = 0x4149,
	 /** 
	payload is ::default_proc 
	**/
	MSG_MS_INITCALLCFG_RSP  = 0x414A,
	 /** 
	api is CAPI2_MS_InitFaxConfig 
	**/
	MSG_MS_INITFAXCFG_REQ  = 0x414B,
	 /** 
	payload is ::default_proc 
	**/
	MSG_MS_INITFAXCFG_RSP  = 0x414C,
	 /** 
	api is CAPI2_MS_InitVideoCallCfg 
	**/
	MSG_MS_INITVIDEOCALLCFG_REQ  = 0x414D,
	 /** 
	payload is ::default_proc 
	**/
	MSG_MS_INITVIDEOCALLCFG_RSP  = 0x414E,
	 /** 
	api is CAPI2_MS_InitCallCfgAmpF 
	**/
	MSG_MS_INITCALLCFGAMPF_REQ  = 0x414F,
	 /** 
	payload is ::default_proc 
	**/
	MSG_MS_INITCALLCFGAMPF_RSP  = 0x4150,
	 /** 
	api is CAPI2_SYSPARM_GetActualLowVoltReading 
	**/
	MSG_SYSPARM_GET_ACTUAL_LOW_VOLT_REQ  = 0x4151,
	 /** 
	payload is ::UInt16 
	**/
	MSG_SYSPARM_GET_ACTUAL_LOW_VOLT_RSP  = 0x4152,	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_SYSPARM_GetActual4p2VoltReading 
	**/
	MSG_SYSPARM_GET_ACTUAL_4P2_VOLT_REQ  = 0x4153,
	 /** 
	payload is ::UInt16 
	**/
	MSG_SYSPARM_GET_ACTUAL_4P2_VOLT_RSP  = 0x4154,	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_SMS_SendSMSCommandTxtReq 
	**/
	MSG_SMS_SEND_COMMAND_TXT_REQ  = 0x4155,	///<Payload type {::CAPI2_SMS_SendSMSCommandTxtReq_Req_t}
	 /** 
	api is CAPI2_SMS_SendSMSCommandPduReq 
	**/
	MSG_SMS_SEND_COMMAND_PDU_REQ  = 0x4156,	///<Payload type {::CAPI2_SMS_SendSMSCommandPduReq_Req_t}
	 /** 
	api is CAPI2_SMS_SendPDUAckToNetwork 
	**/
	MSG_SMS_SEND_ACKTONETWORK_PDU_REQ  = 0x4157,	///<Payload type {::CAPI2_SMS_SendPDUAckToNetwork_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SMS_SEND_ACKTONETWORK_PDU_RSP  = 0x4158,
	 /** 
	api is CAPI2_SMS_StartCellBroadcastWithChnlReq 
	**/
	MSG_SMS_CB_START_STOP_REQ  = 0x4159,	///<Payload type {::CAPI2_SMS_StartCellBroadcastWithChnlReq_Req_t}
	 /** 
	api is CAPI2_SMS_SetMoSmsTpMr 
	**/
	MSG_SMS_SET_TPMR_REQ  = 0x415A,	///<Payload type {::CAPI2_SMS_SetMoSmsTpMr_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SMS_SET_TPMR_RSP  = 0x415B,
	 /** 
	api is CAPI2_SIMLOCK_SetStatus 
	**/
	MSG_SIMLOCK_SET_STATUS_REQ  = 0x415C,	///<Payload type {::CAPI2_SIMLOCK_SetStatus_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SIMLOCK_SET_STATUS_RSP  = 0x415D,
	 /** 
	api is CAPI2_DIAG_ApiCellLockReq 
	**/
	MSG_DIAG_CELLLOCK_REQ  = 0x415E,	///<Payload type {::CAPI2_DIAG_ApiCellLockReq_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_DIAG_CELLLOCK_RSP  = 0x415F,
	 /** 
	api is CAPI2_DIAG_ApiCellLockStatus 
	**/
	MSG_DIAG_CELLLOCK_STATUS_REQ  = 0x4160,
	 /** 
	payload is ::Boolean 
	**/
	MSG_DIAG_CELLLOCK_STATUS_RSP  = 0x4161,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_MS_SetRuaReadyTimer 
	**/
	MSG_MS_SET_RUA_READY_TIMER_REQ  = 0x4162,	///<Payload type {::CAPI2_MS_SetRuaReadyTimer_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_MS_SET_RUA_READY_TIMER_RSP  = 0x4163,
	 /** 
	api is CAPI2_LCS_RegisterRrlpDataHandler 
	**/
	MSG_LCS_REG_RRLP_HDL_REQ  = 0x4164,	///<Payload type {::CAPI2_LCS_RegisterRrlpDataHandler_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_LCS_REG_RRLP_HDL_RSP  = 0x4165,
	 /** 
	api is CAPI2_LCS_SendRrlpDataToNetwork 
	**/
	MSG_LCS_SEND_RRLP_DATA_REQ  = 0x4166,	///<Payload type {::CAPI2_LCS_RrlpData_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_LCS_SEND_RRLP_DATA_RSP  = 0x4167,
	 /** 
	api is CAPI2_LCS_RegisterRrcDataHandler 
	**/
	MSG_LCS_REG_RRC_HDL_REQ  = 0x4168,	///<Payload type {::CAPI2_LCS_RegisterRrcDataHandler_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_LCS_REG_RRC_HDL_RSP  = 0x4169,
	 /** 
	api is CAPI2_LCS_RrcMeasurementReport 
	**/
	MSG_LCS_RRC_MEAS_REPORT_REQ  = 0x416A,	///<Payload type {::CAPI2_LCS_RrcMeasReport_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_LCS_RRC_MEAS_REPORT_RSP  = 0x416B,
	 /** 
	api is CAPI2_LCS_RrcMeasurementControlFailure 
	**/
	MSG_LCS_RRC_MEAS_FAILURE_REQ  = 0x416C,	///<Payload type {::CAPI2_LCS_RrcMeasFailure_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_LCS_RRC_MEAS_FAILURE_RSP  = 0x416D,
	 /** 
	api is CAPI2_LCS_RrcStatus 
	**/
	MSG_LCS_RRC_STATUS_REQ  = 0x416E,	///<Payload type {::CAPI2_LCS_RrcStatus_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_LCS_RRC_STATUS_RSP  = 0x416F,
	 /** 
	api is CAPI2_CC_IsThereEmergencyCall 
	**/
	MSG_CC_IS_THERE_EMERGENCY_CALL_REQ  = 0x4170,
	 /** 
	payload is ::Boolean 
	**/
	MSG_CC_IS_THERE_EMERGENCY_CALL_RSP  = 0x4171,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SYS_EnableCellInfoMsg 
	**/
	MSG_SYS_ENABLE_CELL_INFO_REQ  = 0x4172,	///<Payload type {::CAPI2_SYS_EnableCellInfoMsg_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SYS_ENABLE_CELL_INFO_RSP  = 0x4173,
	 /** 
	api is CAPI2_LCS_L1_bb_isLocked 
	**/
	MSG_L1_BB_ISLOCKED_REQ  = 0x4174,	///<Payload type {::CAPI2_LCS_L1_bb_isLocked_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_L1_BB_ISLOCKED_RSP  = 0x4175,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_DIALSTR_ParseGetCallType 
	**/
	MSG_UTIL_DIAL_STR_PARSE_REQ  = 0x4176,	///<Payload type {::CAPI2_DIALSTR_ParseGetCallType_Req_t}
	 /** 
	payload is ::CallType_t 
	**/
	MSG_UTIL_DIAL_STR_PARSE_RSP  = 0x4177,	///<Payload type {::CallType_t}
	 /** 
	api is CAPI2_LCS_FttSyncReq 
	**/
	MSG_LCS_FTT_SYNC_REQ  = 0x4178,	///<Payload type {::CAPI2_LCS_FttSyncReq_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_LCS_FTT_SYNC_RSP  = 0x4179,
	 /** 
	api is CAPI2_LCS_FttCalcDeltaTime 
	**/
	MSG_LCS_FTT_DELTA_TIME_REQ  = 0x417A,	///<Payload type {::CAPI2_LCS_FttCalcDeltaTime_Req_t}
	 /** 
	payload is ::UInt32 
	**/
	MSG_LCS_FTT_DELTA_TIME_RSP  = 0x417B,	///<Payload type {::UInt32}
	 /** 
	api is CAPI2_MS_ForcedReadyStateReq 
	**/
	MSG_MS_FORCEDREADYSTATE_REQ  = 0x417C,	///<Payload type {::CAPI2_MS_ForcedReadyStateReq_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_MS_FORCEDREADYSTATE_RSP  = 0x417D,
	 /** 
	api is CAPI2_SsApi_ResetSsAlsFlag 
	**/
	MSG_SS_RESETSSALSFLAG_REQ  = 0x417E,
	 /** 
	payload is ::default_proc 
	**/
	MSG_SS_RESETSSALSFLAG_RSP  = 0x417F,
	 /** 
	api is CAPI2_SIMLOCK_GetStatus 
	**/
	MSG_SIMLOCK_GET_STATUS_REQ  = 0x4180,	///<Payload type {::CAPI2_SIMLOCK_GetStatus_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SIMLOCK_GET_STATUS_RSP  = 0x4181,
	 /** 
	api is CAPI2_DIALSTR_IsValidString 
	**/
	MSG_DIALSTR_IS_VALID_REQ  = 0x4182,	///<Payload type {::CAPI2_DIALSTR_IsValidString_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_DIALSTR_IS_VALID_RSP  = 0x4183,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_UTIL_Cause2NetworkCause 
	**/
	MSG_UTIL_CONVERT_NTWK_CAUSE_REQ  = 0x4184,	///<Payload type {::CAPI2_UTIL_Cause2NetworkCause_Req_t}
	 /** 
	payload is ::NetworkCause_t 
	**/
	MSG_UTIL_CONVERT_NTWK_CAUSE_RSP  = 0x4185,	///<Payload type {::NetworkCause_t}
	 /** 
	api is CAPI2_UTIL_ErrCodeToNetCause 
	**/
	MSG_UTIL_CONVERT_ERRCODE_NTWK_CAUSE_REQ  = 0x4186,	///<Payload type {::CAPI2_UTIL_ErrCodeToNetCause_Req_t}
	 /** 
	payload is ::NetworkCause_t 
	**/
	MSG_UTIL_CONVERT_ERRCODE_NTWK_CAUSE_RSP  = 0x4187,	///<Payload type {::NetworkCause_t}
	 /** 
	api is CAPI2_IsGprsDialStr 
	**/
	MSG_ISGPRS_DIAL_STR_REQ  = 0x4188,	///<Payload type {::CAPI2_IsGprsDialStr_Req_t}
	 /** 
	payload is ::UInt8 
	**/
	MSG_ISGPRS_DIAL_STR_RSP  = 0x4189,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_UTIL_GetNumOffsetInSsStr 
	**/
	MSG_GET_NUM_SS_STR_REQ  = 0x418A,	///<Payload type {::CAPI2_UTIL_GetNumOffsetInSsStr_Req_t}
	 /** 
	payload is ::UInt8 
	**/
	MSG_GET_NUM_SS_STR_RSP  = 0x418B,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_IsPppLoopbackDialStr 
	**/
	MSG_DIALSTR_IS_PPPLOOPBACK_REQ  = 0x418C,	///<Payload type {::CAPI2_IsPppLoopbackDialStr_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_DIALSTR_IS_PPPLOOPBACK_RSP  = 0x418D,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SYS_GetRIPPROCVersion 
	**/
	MSG_SYS_GETRIPPROCVERSION_REQ  = 0x418E,
	 /** 
	payload is ::UInt8 
	**/
	MSG_SYS_GETRIPPROCVERSION_RSP  = 0x418F,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_SYSPARM_SetHSDPAPHYCategory 
	**/
	MSG_SYSPARM_SET_HSDPA_PHY_CAT_REQ  = 0x4190,	///<Payload type {::CAPI2_SYSPARM_SetHSDPAPHYCategory_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SYSPARM_SET_HSDPA_PHY_CAT_RSP  = 0x4191,
	 /** 
	api is CAPI2_SYSPARM_GetHSDPAPHYCategory 
	**/
	MSG_SYS_GET_HSDPA_CATEGORY_REQ  = 0x4192,
	 /** 
	payload is ::UInt32 
	**/
	MSG_SYS_GET_HSDPA_CATEGORY_RSP  = 0x4193,	///<Payload type {::UInt32}
	 /** 
	api is CAPI2_SmsApi_ConvertSmsMSMsgType 
	**/
	MSG_SMS_CONVERT_MSGTYPE_REQ  = 0x4194,	///<Payload type {::CAPI2_SmsApi_ConvertSmsMSMsgType_Req_t}
	 /** 
	payload is ::UInt8 
	**/
	MSG_SMS_CONVERT_MSGTYPE_RSP  = 0x4195,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_MS_GetPrefNetStatus 
	**/
	MSG_MS_GETPREFNETSTATUS_REQ  = 0x4196,
	 /** 
	payload is ::GANStatus_t 
	**/
	MSG_MS_GETPREFNETSTATUS_RSP  = 0x4197,	///<Payload type {::GANStatus_t}
	 /** 
	payload is ::uchar_ptr_t 
	**/
	MSG_SYS_TEST_MSG_IND_1  = 0x4198,	///<Payload type {::uchar_ptr_t}
	 /** 
	payload is ::uchar_ptr_t 
	**/
	MSG_SYS_TEST_MSG_IND_2  = 0x4199,	///<Payload type {::uchar_ptr_t}
	 /** 
	payload is ::uchar_ptr_t 
	**/
	MSG_SYS_TEST_MSG_IND_3  = 0x419A,	///<Payload type {::uchar_ptr_t}
	 /** 
	payload is ::uchar_ptr_t 
	**/
	MSG_SYS_TEST_MSG_IND_4  = 0x419B,	///<Payload type {::uchar_ptr_t}
	 /** 
	payload is ::uchar_ptr_t 
	**/
	MSG_SYS_TEST_MSG_IND_5  = 0x419C,	///<Payload type {::uchar_ptr_t}
	 /** 
	payload is ::uchar_ptr_t 
	**/
	MSG_SYS_TEST_MSG_IND_6  = 0x419D,	///<Payload type {::uchar_ptr_t}

	//MSG_GEN_REQ_END = 0x48FF

#endif

