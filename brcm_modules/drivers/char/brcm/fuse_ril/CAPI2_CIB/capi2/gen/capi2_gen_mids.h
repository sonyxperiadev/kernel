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
//MSG_GEN_REQ_START = 0x0
	 /** 
	api is CAPI2_MS_IsGSMRegistered 
	**/
	MSG_MS_GSM_REGISTERED_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x0,
	 /** 
	payload is ::Boolean 
	**/
	MSG_MS_GSM_REGISTERED_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_MS_IsGPRSRegistered 
	**/
	MSG_MS_GPRS_REGISTERED_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2,
	 /** 
	payload is ::Boolean 
	**/
	MSG_MS_GPRS_REGISTERED_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x3,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_MS_GetGSMRegCause 
	**/
	MSG_MS_GSM_CAUSE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4,
	 /** 
	payload is ::NetworkCause_t 
	**/
	MSG_MS_GSM_CAUSE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x5,	///<Payload type {::NetworkCause_t}
	 /** 
	api is CAPI2_MS_GetGPRSRegCause 
	**/
	MSG_MS_GPRS_CAUSE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6,
	 /** 
	payload is ::NetworkCause_t 
	**/
	MSG_MS_GPRS_CAUSE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x7,	///<Payload type {::NetworkCause_t}
	 /** 
	api is CAPI2_MS_GetRegisteredLAC 
	**/
	MSG_MS_REGISTERED_LAC_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x8,
	 /** 
	payload is ::UInt16 
	**/
	MSG_MS_REGISTERED_LAC_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x9,	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_MS_GetPlmnMCC 
	**/
	MSG_MS_GET_PLMN_MCC_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xA,
	 /** 
	payload is ::UInt16 
	**/
	MSG_MS_GET_PLMN_MCC_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xB,	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_MS_GetPlmnMNC 
	**/
	MSG_MS_GET_PLMN_MNC_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xC,
	 /** 
	payload is ::UInt8 
	**/
	MSG_MS_GET_PLMN_MNC_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xD,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_PhoneCtrlApi_ProcessPowerDownReq 
	**/
	MSG_SYS_POWERDOWN_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xE,
	 /** 
	payload is ::void 
	**/
	MSG_SYS_POWERDOWN_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xF,
	 /** 
	api is CAPI2_PhoneCtrlApi_ProcessNoRfReq 
	**/
	MSG_SYS_POWERUP_NORF_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x10,
	 /** 
	payload is ::void 
	**/
	MSG_SYS_POWERUP_NORF_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x11,
	 /** 
	api is CAPI2_PhoneCtrlApi_ProcessNoRfToCalib 
	**/
	MSG_SYS_NORF_CALIB_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x12,
	 /** 
	payload is ::void 
	**/
	MSG_SYS_NORF_CALIB_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x13,
	 /** 
	api is CAPI2_PhoneCtrlApi_ProcessPowerUpReq 
	**/
	MSG_SYS_POWERUP_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x14,
	 /** 
	payload is ::void 
	**/
	MSG_SYS_POWERUP_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x15,
	 /** 
	api is CAPI2_MS_IsGprsAllowed 
	**/
	MSG_MS_GPRS_ALLOWED_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x16,
	 /** 
	payload is ::Boolean 
	**/
	MSG_MS_GPRS_ALLOWED_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x17,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_MS_GetCurrentRAT 
	**/
	MSG_MS_GET_CURRENT_RAT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x18,
	 /** 
	payload is ::UInt8 
	**/
	MSG_MS_GET_CURRENT_RAT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x19,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_MS_GetCurrentBand 
	**/
	MSG_MS_GET_CURRENT_BAND_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1A,
	 /** 
	payload is ::UInt8 
	**/
	MSG_MS_GET_CURRENT_BAND_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1B,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_SimApi_UpdateSMSCapExceededFlag 
	**/
	MSG_SIM_UPDATE_SMSCAPEXC_FLAG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1C,	///<Payload type {::CAPI2_SimApi_UpdateSMSCapExceededFlag_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_SIM_UPDATE_SMSCAPEXC_FLAG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1D,
	 /** 
	api is CAPI2_NetRegApi_SelectBand 
	**/
	MSG_MS_SELECT_BAND_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1E,	///<Payload type {::CAPI2_NetRegApi_SelectBand_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_MS_SELECT_BAND_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1F,
	 /** 
	api is CAPI2_NetRegApi_SetSupportedRATandBand 
	**/
	MSG_MS_SET_RAT_BAND_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x20,	///<Payload type {::CAPI2_NetRegApi_SetSupportedRATandBand_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_MS_SET_RAT_BAND_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x21,
	 /** 
	api is CAPI2_PLMN_GetCountryByMcc 
	**/
	MSG_MS_GET_MCC_COUNTRY_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x22,	///<Payload type {::CAPI2_PLMN_GetCountryByMcc_Req_t}
	 /** 
	payload is ::char 
	**/
	MSG_MS_GET_MCC_COUNTRY_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x23,	///<Payload type {::char}
	 /** 
	api is CAPI2_MS_GetPLMNEntryByIndex 
	**/
	MSG_MS_PLMN_INFO_BY_CODE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x24,	///<Payload type {::CAPI2_MS_GetPLMNEntryByIndex_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_MS_PLMN_INFO_BY_CODE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x25,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_MS_GetPLMNListSize 
	**/
	MSG_MS_PLMN_LIST_SIZE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x26,
	 /** 
	payload is ::UInt16 
	**/
	MSG_MS_PLMN_LIST_SIZE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x27,	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_MS_GetPLMNByCode 
	**/
	MSG_MS_PLMN_INFO_BY_INDEX_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x28,	///<Payload type {::CAPI2_MS_GetPLMNByCode_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_MS_PLMN_INFO_BY_INDEX_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x29,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_NetRegApi_PlmnSelect 
	**/
	MSG_PLMN_SELECT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2A,	///<Payload type {::CAPI2_NetRegApi_PlmnSelect_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PLMN_SELECT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2B,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_NetRegApi_AbortPlmnSelect 
	**/
	MSG_MS_PLMN_ABORT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2C,
	 /** 
	payload is ::Result_t 
	**/
	MSG_MS_PLMN_ABORT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2D,
	 /** 
	api is CAPI2_MS_GetPlmnMode 
	**/
	MSG_MS_GET_PLMN_MODE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2E,
	 /** 
	payload is ::PlmnSelectMode_t 
	**/
	MSG_MS_GET_PLMN_MODE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2F,	///<Payload type {::PlmnSelectMode_t}
	 /** 
	api is CAPI2_NetRegApi_SetPlmnMode 
	**/
	MSG_MS_SET_PLMN_MODE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x30,	///<Payload type {::CAPI2_NetRegApi_SetPlmnMode_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_MS_SET_PLMN_MODE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x31,
	 /** 
	api is CAPI2_MS_GetPlmnFormat 
	**/
	MSG_MS_GET_PLMN_FORMAT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x32,
	 /** 
	payload is ::PlmnSelectFormat_t 
	**/
	MSG_MS_GET_PLMN_FORMAT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x33,	///<Payload type {::PlmnSelectFormat_t}
	 /** 
	api is CAPI2_MS_SetPlmnFormat 
	**/
	MSG_MS_SET_PLMN_FORMAT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x34,	///<Payload type {::CAPI2_MS_SetPlmnFormat_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_MS_SET_PLMN_FORMAT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x35,
	 /** 
	api is CAPI2_MS_IsMatchedPLMN 
	**/
	MSG_MS_MATCH_PLMN_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x36,	///<Payload type {::CAPI2_MS_IsMatchedPLMN_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_MS_MATCH_PLMN_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x37,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_NetRegApi_SearchAvailablePLMN 
	**/
	MSG_MS_SEARCH_PLMN_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x38,
	 /** 
	api is CAPI2_NetRegApi_AbortSearchPLMN 
	**/
	MSG_MS_ABORT_PLMN_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x39,
	 /** 
	payload is ::void 
	**/
	MSG_MS_ABORT_PLMN_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x3A,
	 /** 
	api is CAPI2_NetRegApi_AutoSearchReq 
	**/
	MSG_MS_AUTO_SEARCH_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3B,
	 /** 
	payload is ::void 
	**/
	MSG_MS_AUTO_SEARCH_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x3C,
	 /** 
	api is CAPI2_NetRegApi_GetPLMNNameByCode 
	**/
	MSG_MS_PLMN_NAME_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3D,	///<Payload type {::CAPI2_NetRegApi_GetPLMNNameByCode_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_MS_PLMN_NAME_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x3E,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PhoneCtrlApi_GetSystemState 
	**/
	MSG_SYS_GET_SYSTEM_STATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3F,
	 /** 
	payload is ::SystemState_t 
	**/
	MSG_SYS_GET_SYSTEM_STATE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x40,	///<Payload type {::SystemState_t}
	 /** 
	api is CAPI2_PhoneCtrlApi_SetSystemState 
	**/
	MSG_SYS_SET_SYSTEM_STATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x41,	///<Payload type {::CAPI2_PhoneCtrlApi_SetSystemState_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SYS_SET_SYSTEM_STATE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x42,
	 /** 
	api is CAPI2_PhoneCtrlApi_GetRxSignalInfo 
	**/
	MSG_SYS_GET_RX_LEVEL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x43,
	 /** 
	payload is ::void 
	**/
	MSG_SYS_GET_RX_LEVEL_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x44,	///<Payload type {::void}
	 /** 
	api is CAPI2_SYS_GetGSMRegistrationStatus 
	**/
	MSG_SYS_GET_GSMREG_STATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x45,
	 /** 
	payload is ::RegisterStatus_t 
	**/
	MSG_SYS_GET_GSMREG_STATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x46,	///<Payload type {::RegisterStatus_t}
	 /** 
	api is CAPI2_SYS_GetGPRSRegistrationStatus 
	**/
	MSG_SYS_GET_GPRSREG_STATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x47,
	 /** 
	payload is ::RegisterStatus_t 
	**/
	MSG_SYS_GET_GPRSREG_STATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x48,	///<Payload type {::RegisterStatus_t}
	 /** 
	api is CAPI2_SYS_IsRegisteredGSMOrGPRS 
	**/
	MSG_SYS_GET_REG_STATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x49,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SYS_GET_REG_STATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x4A,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_MS_IsRegisteredHomePLMN 
	**/
	MSG_HOME_PLMN_REG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4B,
	 /** 
	payload is ::Boolean 
	**/
	MSG_HOME_PLMN_REG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x4C,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PhoneCtrlApi_SetPowerDownTimer 
	**/
	MSG_SET_POWER_DOWN_TIMER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4D,	///<Payload type {::CAPI2_PhoneCtrlApi_SetPowerDownTimer_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SET_POWER_DOWN_TIMER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x4E,
	 /** 
	api is CAPI2_SimApi_GetSmsParamRecNum 
	**/
	MSG_SIM_PARAM_REC_NUM_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4F,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SIM_PARAM_REC_NUM_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x50,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SimApi_GetSmsMemExceededFlag 
	**/
	MSG_SIM_GET_SMSMEMEXC_FLAG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x51,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_GET_SMSMEMEXC_FLAG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x52,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SimApi_IsTestSIM 
	**/
	MSG_SIM_IS_TEST_SIM_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x53,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_IS_TEST_SIM_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x54,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SimApi_IsPINRequired 
	**/
	MSG_SIM_PIN_REQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x55,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_PIN_REQ_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x56,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SimApi_GetCardPhase 
	**/
	MSG_SIM_CARD_PHASE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x57,
	 /** 
	payload is ::SIMPhase_t 
	**/
	MSG_SIM_CARD_PHASE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x58,	///<Payload type {::SIMPhase_t}
	 /** 
	api is CAPI2_SimApi_GetSIMType 
	**/
	MSG_SIM_TYPE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x59,
	 /** 
	payload is ::SIMType_t 
	**/
	MSG_SIM_TYPE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x5A,	///<Payload type {::SIMType_t}
	 /** 
	api is CAPI2_SimApi_GetPresentStatus 
	**/
	MSG_SIM_PRESENT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x5B,
	 /** 
	payload is ::SIMPresent_t 
	**/
	MSG_SIM_PRESENT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x5C,	///<Payload type {::SIMPresent_t}
	 /** 
	api is CAPI2_SimApi_IsOperationRestricted 
	**/
	MSG_SIM_PIN_OPERATION_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x5D,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_PIN_OPERATION_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x5E,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SimApi_IsPINBlocked 
	**/
	MSG_SIM_PIN_BLOCK_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x5F,	///<Payload type {::CAPI2_SimApi_IsPINBlocked_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_PIN_BLOCK_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x60,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SimApi_IsPUKBlocked 
	**/
	MSG_SIM_PUK_BLOCK_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x61,	///<Payload type {::CAPI2_SimApi_IsPUKBlocked_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_PUK_BLOCK_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x62,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SimApi_IsInvalidSIM 
	**/
	MSG_SIM_IS_INVALID_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x63,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_IS_INVALID_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x64,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SimApi_DetectSim 
	**/
	MSG_SIM_DETECT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x65,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_DETECT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x66,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SimApi_GetRuimSuppFlag 
	**/
	MSG_SIM_GET_RUIM_SUPP_FLAG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x67,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_GET_RUIM_SUPP_FLAG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x68,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SimApi_SendVerifyChvReq 
	**/
	MSG_SIM_VERIFY_CHV_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x69,	///<Payload type {::CAPI2_SimApi_SendVerifyChvReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendChangeChvReq 
	**/
	MSG_SIM_CHANGE_CHV_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6A,	///<Payload type {::CAPI2_SimApi_SendChangeChvReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendSetChv1OnOffReq 
	**/
	MSG_SIM_ENABLE_CHV_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6B,	///<Payload type {::CAPI2_SimApi_SendSetChv1OnOffReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendUnblockChvReq 
	**/
	MSG_SIM_UNBLOCK_CHV_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6C,	///<Payload type {::CAPI2_SimApi_SendUnblockChvReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendSetOperStateReq 
	**/
	MSG_SIM_SET_FDN_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6D,	///<Payload type {::CAPI2_SimApi_SendSetOperStateReq_Req_t}
	 /** 
	api is CAPI2_SimApi_IsPbkAccessAllowed 
	**/
	MSG_SIM_IS_PBK_ALLOWED_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6E,	///<Payload type {::CAPI2_SimApi_IsPbkAccessAllowed_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_IS_PBK_ALLOWED_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x6F,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SimApi_SendPbkInfoReq 
	**/
	MSG_SIM_PBK_INFO_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x70,	///<Payload type {::CAPI2_SimApi_SendPbkInfoReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendReadAcmMaxReq 
	**/
	MSG_SIM_MAX_ACM_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x71,
	 /** 
	api is CAPI2_SimApi_SendWriteAcmMaxReq 
	**/
	MSG_SIM_ACM_MAX_UPDATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x72,	///<Payload type {::CAPI2_SimApi_SendWriteAcmMaxReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendReadAcmReq 
	**/
	MSG_SIM_ACM_VALUE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x73,
	 /** 
	api is CAPI2_SimApi_SendWriteAcmReq 
	**/
	MSG_SIM_ACM_UPDATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x74,	///<Payload type {::CAPI2_SimApi_SendWriteAcmReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendIncreaseAcmReq 
	**/
	MSG_SIM_ACM_INCREASE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x75,	///<Payload type {::CAPI2_SimApi_SendIncreaseAcmReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendReadSvcProvNameReq 
	**/
	MSG_SIM_SVC_PROV_NAME_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x76,
	 /** 
	api is CAPI2_SimApi_SendReadPuctReq 
	**/
	MSG_SIM_PUCT_DATA_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x77,
	 /** 
	api is CAPI2_SimApi_GetServiceStatus 
	**/
	MSG_SIM_SERVICE_STATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x78,	///<Payload type {::CAPI2_SimApi_GetServiceStatus_Req_t}
	 /** 
	payload is ::SIMServiceStatus_t 
	**/
	MSG_SIM_SERVICE_STATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x79,	///<Payload type {::SIMServiceStatus_t}
	 /** 
	api is CAPI2_SimApi_GetPinStatus 
	**/
	MSG_SIM_PIN_STATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x7A,
	 /** 
	payload is ::SIM_PIN_Status_t 
	**/
	MSG_SIM_PIN_STATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x7B,	///<Payload type {::SIM_PIN_Status_t}
	 /** 
	api is CAPI2_SimApi_IsPinOK 
	**/
	MSG_SIM_PIN_OK_STATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x7C,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_PIN_OK_STATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x7D,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SimApi_GetIMSI 
	**/
	MSG_SIM_IMSI_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x7E,
	 /** 
	payload is ::IMSI_t 
	**/
	MSG_SIM_IMSI_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x7F,	///<Payload type {::IMSI_t}
	 /** 
	api is CAPI2_SimApi_GetGID1 
	**/
	MSG_SIM_GID_DIGIT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x80,
	 /** 
	payload is ::GID_DIGIT_t 
	**/
	MSG_SIM_GID_DIGIT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x81,	///<Payload type {::GID_DIGIT_t}
	 /** 
	api is CAPI2_SimApi_GetGID2 
	**/
	MSG_SIM_GID_DIGIT2_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x82,
	 /** 
	payload is ::GID_DIGIT_t 
	**/
	MSG_SIM_GID_DIGIT2_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x83,	///<Payload type {::GID_DIGIT_t}
	 /** 
	api is CAPI2_SimApi_GetHomePlmn 
	**/
	MSG_SIM_HOME_PLMN_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x84,
	 /** 
	payload is ::void 
	**/
	MSG_SIM_HOME_PLMN_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x85,	///<Payload type {::void}
	 /** 
	api is CAPI2_simmiApi_GetMasterFileId 
	**/
	MSG_SIM_APDU_FILEID_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x86,	///<Payload type {::CAPI2_simmiApi_GetMasterFileId_Req_t}
	 /** 
	payload is ::APDUFileID_t 
	**/
	MSG_SIM_APDU_FILEID_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x87,	///<Payload type {::APDUFileID_t}
	 /** 
	api is CAPI2_SimApi_SendOpenSocketReq 
	**/
	MSG_SIM_OPEN_SOCKET_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x88,
	 /** 
	api is CAPI2_SimApi_SendSelectAppiReq 
	**/
	MSG_SIM_SELECT_APPLI_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x89,	///<Payload type {::CAPI2_SimApi_SendSelectAppiReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendDeactivateAppiReq 
	**/
	MSG_SIM_DEACTIVATE_APPLI_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x8A,	///<Payload type {::CAPI2_SimApi_SendDeactivateAppiReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendCloseSocketReq 
	**/
	MSG_SIM_CLOSE_SOCKET_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x8B,	///<Payload type {::CAPI2_SimApi_SendCloseSocketReq_Req_t}
	 /** 
	api is CAPI2_SimApi_GetAtrData 
	**/
	MSG_SIM_ATR_DATA_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x8C,
	 /** 
	payload is ::SIMAccess_t 
	**/
	MSG_SIM_ATR_DATA_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x8D,	///<Payload type {::SIMAccess_t}
	 /** 
	api is CAPI2_SIM_SubmitDFileInfoReqOld 
	**/
	MSG_SIM_DFILE_INFO_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x8E,	///<Payload type {::CAPI2_SIM_SubmitDFileInfoReqOld_Req_t}
	 /** 
	api is CAPI2_SimApi_SubmitEFileInfoReq 
	**/
	MSG_SIM_EFILE_INFO_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x8F,	///<Payload type {::CAPI2_SimApi_SubmitEFileInfoReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendEFileInfoReq 
	**/
	MSG_SIM_SEND_EFILE_INFO_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x90,	///<Payload type {::CAPI2_SimApi_SendEFileInfoReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendDFileInfoReq 
	**/
	MSG_SIM_SEND_DFILE_INFO_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x91,	///<Payload type {::CAPI2_SimApi_SendDFileInfoReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SubmitWholeBinaryEFileReadReq 
	**/
	MSG_SIM_WHOLE_EFILE_DATA_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x92,	///<Payload type {::CAPI2_SimApi_SubmitWholeBinaryEFileReadReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendWholeBinaryEFileReadReq 
	**/
	MSG_SIM_SEND_WHOLE_EFILE_DATA_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x93,	///<Payload type {::CAPI2_SimApi_SendWholeBinaryEFileReadReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SubmitBinaryEFileReadReq 
	**/
	MSG_SIM_EFILE_DATA_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x94,	///<Payload type {::CAPI2_SimApi_SubmitBinaryEFileReadReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendBinaryEFileReadReq 
	**/
	MSG_SIM_SEND_EFILE_DATA_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x95,	///<Payload type {::CAPI2_SimApi_SendBinaryEFileReadReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SubmitRecordEFileReadReq 
	**/
	MSG_SIM_RECORD_EFILE_DATA_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x96,	///<Payload type {::CAPI2_SimApi_SubmitRecordEFileReadReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendRecordEFileReadReq 
	**/
	MSG_SIM_SEND_RECORD_EFILE_DATA_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x97,	///<Payload type {::CAPI2_SimApi_SendRecordEFileReadReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SubmitBinaryEFileUpdateReq 
	**/
	MSG_SIM_EFILE_UPDATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x98,	///<Payload type {::CAPI2_SimApi_SubmitBinaryEFileUpdateReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendBinaryEFileUpdateReq 
	**/
	MSG_SIM_SEND_EFILE_UPDATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x99,	///<Payload type {::CAPI2_SimApi_SendBinaryEFileUpdateReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SubmitLinearEFileUpdateReq 
	**/
	MSG_SIM_LINEAR_EFILE_UPDATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x9A,	///<Payload type {::CAPI2_SimApi_SubmitLinearEFileUpdateReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendLinearEFileUpdateReq 
	**/
	MSG_SIM_SEND_LINEAR_EFILE_UPDATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x9B,	///<Payload type {::CAPI2_SimApi_SendLinearEFileUpdateReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SubmitSeekRecordReq 
	**/
	MSG_SIM_SEEK_RECORD_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x9C,	///<Payload type {::CAPI2_SimApi_SubmitSeekRecordReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendSeekRecordReq 
	**/
	MSG_SIM_SEND_SEEK_RECORD_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x9D,	///<Payload type {::CAPI2_SimApi_SendSeekRecordReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SubmitCyclicEFileUpdateReq 
	**/
	MSG_SIM_CYCLIC_EFILE_UPDATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x9E,	///<Payload type {::CAPI2_SimApi_SubmitCyclicEFileUpdateReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendCyclicEFileUpdateReq 
	**/
	MSG_SIM_SEND_CYCLIC_EFILE_UPDATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x9F,	///<Payload type {::CAPI2_SimApi_SendCyclicEFileUpdateReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendRemainingPinAttemptReq 
	**/
	MSG_SIM_PIN_ATTEMPT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xA0,
	 /** 
	api is CAPI2_SimApi_IsCachedDataReady 
	**/
	MSG_SIM_CACHE_DATA_READY_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xA1,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_CACHE_DATA_READY_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xA2,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SimApi_GetServiceCodeStatus 
	**/
	MSG_SIM_SERVICE_CODE_STATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xA3,	///<Payload type {::CAPI2_SimApi_GetServiceCodeStatus_Req_t}
	 /** 
	payload is ::SERVICE_FLAG_STATUS_t 
	**/
	MSG_SIM_SERVICE_CODE_STATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xA4,	///<Payload type {::SERVICE_FLAG_STATUS_t}
	 /** 
	api is CAPI2_SimApi_CheckCphsService 
	**/
	MSG_SIM_CHECK_CPHS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xA5,	///<Payload type {::CAPI2_SimApi_CheckCphsService_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_CHECK_CPHS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xA6,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SimApi_GetCphsPhase 
	**/
	MSG_SIM_CPHS_PHASE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xA7,
	 /** 
	payload is ::UInt8 
	**/
	MSG_SIM_CPHS_PHASE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xA8,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_SimApi_GetSmsSca 
	**/
	MSG_SIM_SMS_SCA_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xA9,	///<Payload type {::CAPI2_SimApi_GetSmsSca_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SIM_SMS_SCA_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xAA,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SimApi_GetIccid 
	**/
	MSG_SIM_ICCID_PARAM_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xAB,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SIM_ICCID_PARAM_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xAC,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SimApi_IsALSEnabled 
	**/
	MSG_SIM_ALS_STATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xAD,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_ALS_STATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xAE,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SimApi_GetAlsDefaultLine 
	**/
	MSG_SIM_ALS_DEFAULT_LINE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xAF,
	 /** 
	payload is ::UInt8 
	**/
	MSG_SIM_ALS_DEFAULT_LINE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xB0,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_SimApi_SetAlsDefaultLine 
	**/
	MSG_SIM_SET_ALS_DEFAULT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xB1,	///<Payload type {::CAPI2_SimApi_SetAlsDefaultLine_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_SIM_SET_ALS_DEFAULT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xB2,
	 /** 
	api is CAPI2_SimApi_GetCallForwardUnconditionalFlag 
	**/
	MSG_SIM_CALLFWD_COND_FLAG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xB3,
	 /** 
	payload is ::SIM_CALL_FORWARD_UNCONDITIONAL_FLAG_t 
	**/
	MSG_SIM_CALLFWD_COND_FLAG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xB4,	///<Payload type {::SIM_CALL_FORWARD_UNCONDITIONAL_FLAG_t}
	 /** 
	api is CAPI2_SimApi_GetApplicationType 
	**/
	MSG_SIM_APP_TYPE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xB5,
	 /** 
	payload is ::SIM_APPL_TYPE_t 
	**/
	MSG_SIM_APP_TYPE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xB6,	///<Payload type {::SIM_APPL_TYPE_t}
	 /** 
	api is CAPI2_SimApi_SendWritePuctReq 
	**/
	MSG_SIM_PUCT_UPDATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xB7,	///<Payload type {::CAPI2_SimApi_SendWritePuctReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SubmitRestrictedAccessReq 
	**/
	MSG_SIM_RESTRICTED_ACCESS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xB8,	///<Payload type {::CAPI2_SimApi_SubmitRestrictedAccessReq_Req_t}
	 /** 
	api is CAPI2_ADCMGR_Start 
	**/
	MSG_ADC_START_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xB9,	///<Payload type {::CAPI2_ADCMGR_Start_Req_t}
	 /** 
	api is CAPI2_MS_GetSystemRAT 
	**/
	MSG_MS_GET_SYSTEM_RAT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xBA,
	 /** 
	payload is ::RATSelect_t 
	**/
	MSG_MS_GET_SYSTEM_RAT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xBB,	///<Payload type {::RATSelect_t}
	 /** 
	api is CAPI2_MS_GetSupportedRAT 
	**/
	MSG_MS_GET_SUPPORTED_RAT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xBC,
	 /** 
	payload is ::RATSelect_t 
	**/
	MSG_MS_GET_SUPPORTED_RAT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xBD,	///<Payload type {::RATSelect_t}
	 /** 
	api is CAPI2_MS_GetSystemBand 
	**/
	MSG_MS_GET_SYSTEM_BAND_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xBE,
	 /** 
	payload is ::BandSelect_t 
	**/
	MSG_MS_GET_SYSTEM_BAND_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xBF,	///<Payload type {::BandSelect_t}
	 /** 
	api is CAPI2_MS_GetSupportedBand 
	**/
	MSG_MS_GET_SUPPORTED_BAND_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xC0,
	 /** 
	payload is ::BandSelect_t 
	**/
	MSG_MS_GET_SUPPORTED_BAND_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xC1,	///<Payload type {::BandSelect_t}
	 /** 
	api is CAPI2_SYSPARM_GetMSClass 
	**/
	MSG_SYSPARAM_GET_MSCLASS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xC2,
	 /** 
	payload is ::UInt16 
	**/
	MSG_SYSPARAM_GET_MSCLASS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xC3,	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_SYSPARM_GetManufacturerName 
	**/
	MSG_SYSPARAM_GET_MNF_NAME_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xC4,
	 /** 
	payload is ::uchar_ptr_t 
	**/
	MSG_SYSPARAM_GET_MNF_NAME_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xC5,	///<Payload type {::uchar_ptr_t}
	 /** 
	api is CAPI2_SYSPARM_GetModelName 
	**/
	MSG_SYSPARAM_GET_MODEL_NAME_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xC6,
	 /** 
	payload is ::uchar_ptr_t 
	**/
	MSG_SYSPARAM_GET_MODEL_NAME_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xC7,	///<Payload type {::uchar_ptr_t}
	 /** 
	api is CAPI2_SYSPARM_GetSWVersion 
	**/
	MSG_SYSPARAM_GET_SW_VERSION_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xC8,
	 /** 
	payload is ::uchar_ptr_t 
	**/
	MSG_SYSPARAM_GET_SW_VERSION_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xC9,	///<Payload type {::uchar_ptr_t}
	 /** 
	api is CAPI2_SYSPARM_GetEGPRSMSClass 
	**/
	MSG_SYSPARAM_GET_EGPRS_CLASS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xCA,
	 /** 
	payload is ::UInt16 
	**/
	MSG_SYSPARAM_GET_EGPRS_CLASS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xCB,	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_SimApi_SendNumOfPLMNEntryReq 
	**/
	MSG_SIM_PLMN_NUM_OF_ENTRY_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xCC,	///<Payload type {::CAPI2_SimApi_SendNumOfPLMNEntryReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendReadPLMNEntryReq 
	**/
	MSG_SIM_PLMN_ENTRY_DATA_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xCD,	///<Payload type {::CAPI2_SimApi_SendReadPLMNEntryReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendWriteMulPLMNEntryReq 
	**/
	MSG_SIM_MUL_PLMN_ENTRY_UPDATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xCE,	///<Payload type {::CAPI2_SimApi_SendWriteMulPLMNEntryReq_Req_t}
	 /** 
	api is CAPI2_SYS_SetRegisteredEventMask 
	**/
	MSG_SYS_SET_REG_EVENT_MASK_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xCF,	///<Payload type {::CAPI2_SYS_SetRegisteredEventMask_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SYS_SET_REG_EVENT_MASK_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xD0,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SYS_SetFilteredEventMask 
	**/
	MSG_SYS_SET_REG_FILTER_MASK_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xD1,	///<Payload type {::CAPI2_SYS_SetFilteredEventMask_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SYS_SET_REG_FILTER_MASK_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xD2,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PhoneCtrlApi_SetRssiThreshold 
	**/
	MSG_SYS_SET_RSSI_THRESHOLD_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xD3,	///<Payload type {::CAPI2_PhoneCtrlApi_SetRssiThreshold_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_SYS_SET_RSSI_THRESHOLD_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xD4,
	 /** 
	api is CAPI2_SYSPARM_GetChanMode 
	**/
	MSG_SYSPARAM_GET_CHANNEL_MODE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xD5,
	 /** 
	payload is ::UInt16 
	**/
	MSG_SYSPARAM_GET_CHANNEL_MODE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xD6,	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_SYSPARM_GetClassmark 
	**/
	MSG_SYSPARAM_GET_CLASSMARK_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xD7,
	 /** 
	payload is ::CAPI2_Class_t 
	**/
	MSG_SYSPARAM_GET_CLASSMARK_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xD8,	///<Payload type {::CAPI2_Class_t}
	 /** 
	api is CAPI2_SYSPARM_GetSysparmIndPartFileVersion 
	**/
	MSG_SYSPARAM_GET_IND_FILE_VER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xD9,
	 /** 
	payload is ::UInt16 
	**/
	MSG_SYSPARAM_GET_IND_FILE_VER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xDA,	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_SYSPARM_SetDARPCfg 
	**/
	MSG_SYS_SET_DARP_CFG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xDB,	///<Payload type {::CAPI2_SYSPARM_SetDARPCfg_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_SYS_SET_DARP_CFG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xDC,
	 /** 
	api is CAPI2_SYSPARM_SetEGPRSMSClass 
	**/
	MSG_SYSPARAM_SET_EGPRS_MSCLASS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xDD,	///<Payload type {::CAPI2_SYSPARM_SetEGPRSMSClass_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_SYSPARAM_SET_EGPRS_MSCLASS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xDE,
	 /** 
	api is CAPI2_SYSPARM_SetGPRSMSClass 
	**/
	MSG_SYSPARAM_SET_GPRS_MSCLASS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xDF,	///<Payload type {::CAPI2_SYSPARM_SetGPRSMSClass_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_SYSPARAM_SET_GPRS_MSCLASS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xE0,
	 /** 
	api is CAPI2_NetRegApi_DeleteNetworkName 
	**/
	MSG_TIMEZONE_DELETE_NW_NAME_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xE1,
	 /** 
	payload is ::void 
	**/
	MSG_TIMEZONE_DELETE_NW_NAME_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xE2,
	 /** 
	api is CAPI2_TestCmds 
	**/
	MSG_CAPI2_TEST_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xE3,	///<Payload type {::CAPI2_TestCmds_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_CAPI2_TEST_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xE4,
	 /** 
	api is CAPI2_SatkApi_SendPlayToneRes 
	**/
	MSG_STK_SEND_PLAYTONE_RES_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xE5,	///<Payload type {::CAPI2_SatkApi_SendPlayToneRes_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_STK_SEND_PLAYTONE_RES_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xE6,
	 /** 
	api is CAPI2_SATK_SendSetupCallRes 
	**/
	MSG_STK_SETUP_CALL_RES_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xE7,	///<Payload type {::CAPI2_SATK_SendSetupCallRes_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_STK_SETUP_CALL_RES_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xE8,
	 /** 
	api is CAPI2_PbkApi_SetFdnCheck 
	**/
	MSG_PBK_SET_FDN_CHECK_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xE9,	///<Payload type {::CAPI2_PbkApi_SetFdnCheck_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_PBK_SET_FDN_CHECK_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xEA,
	 /** 
	api is CAPI2_PbkApi_GetFdnCheck 
	**/
	MSG_PBK_GET_FDN_CHECK_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xEB,
	 /** 
	payload is ::Boolean 
	**/
	MSG_PBK_GET_FDN_CHECK_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xEC,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PMU_Battery_Register 
	**/
	MSG_PMU_BATT_LEVEL_REGISTER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xED,	///<Payload type {::CAPI2_PMU_Battery_Register_Req_t}
	 /** 
	payload is ::HAL_EM_BATTMGR_ErrorCode_en_t 
	**/
	MSG_PMU_BATT_LEVEL_REGISTER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xEE,	///<Payload type {::HAL_EM_BATTMGR_ErrorCode_en_t}
	 /** 
	payload is ::HAL_EM_BatteryLevel_t 
	**/
	MSG_PMU_BATT_LEVEL_IND  = MSG_GRP_CAPI2_GEN_0 + 0xEF,	///<Payload type {::HAL_EM_BatteryLevel_t}
	 /** 
	api is CAPI2_SmsApi_SendMemAvailInd 
	**/
	MSG_SMS_SEND_MEM_AVAL_IND_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xF0,
	 /** 
	payload is ::void 
	**/
	MSG_SMS_SEND_MEM_AVAL_IND_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xF1,
	 /** 
	api is CAPI2_SMS_ConfigureMEStorage 
	**/
	MSG_SMS_CONFIGUREMESTORAGE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xF2,	///<Payload type {::CAPI2_SMS_ConfigureMEStorage_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_CONFIGUREMESTORAGE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xF3,
	 /** 
	api is CAPI2_MsDbApi_SetElement 
	**/
	MSG_MS_SET_ELEMENT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xF4,	///<Payload type {::CAPI2_MsDbApi_SetElement_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_MS_SET_ELEMENT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xF5,
	 /** 
	api is CAPI2_MsDbApi_GetElement 
	**/
	MSG_MS_GET_ELEMENT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xF6,	///<Payload type {::CAPI2_MsDbApi_GetElement_Req_t}
	 /** 
	payload is ::CAPI2_MS_Element_t 
	**/
	MSG_MS_GET_ELEMENT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xF7,	///<Payload type {::CAPI2_MS_Element_t}
	 /** 
	api is CAPI2_USimApi_IsApplicationSupported 
	**/
	MSG_USIM_IS_APP_SUPPORTED_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xF8,	///<Payload type {::CAPI2_USimApi_IsApplicationSupported_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_USIM_IS_APP_SUPPORTED_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xF9,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_USimApi_IsAllowedAPN 
	**/
	MSG_USIM_IS_APN_ALLOWED_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xFA,	///<Payload type {::CAPI2_USimApi_IsAllowedAPN_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_USIM_IS_APN_ALLOWED_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xFB,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_USimApi_GetNumOfAPN 
	**/
	MSG_USIM_GET_NUM_APN_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xFC,
	 /** 
	payload is ::UInt8 
	**/
	MSG_USIM_GET_NUM_APN_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xFD,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_USimApi_GetAPNEntry 
	**/
	MSG_USIM_GET_APN_ENTRY_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xFE,	///<Payload type {::CAPI2_USimApi_GetAPNEntry_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_USIM_GET_APN_ENTRY_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xFF,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_USimApi_IsEstServActivated 
	**/
	MSG_USIM_IS_EST_SERV_ACTIVATED_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x100,	///<Payload type {::CAPI2_USimApi_IsEstServActivated_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_USIM_IS_EST_SERV_ACTIVATED_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x101,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_USimApi_SendSetEstServReq 
	**/
	MSG_SIM_SET_EST_SERV_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x102,	///<Payload type {::CAPI2_USimApi_SendSetEstServReq_Req_t}
	 /** 
	api is CAPI2_USimApi_SendWriteAPNReq 
	**/
	MSG_SIM_UPDATE_ONE_APN_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x103,	///<Payload type {::CAPI2_USimApi_SendWriteAPNReq_Req_t}
	 /** 
	api is CAPI2_USimApi_SendDeleteAllAPNReq 
	**/
	MSG_SIM_DELETE_ALL_APN_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x104,
	 /** 
	api is CAPI2_USimApi_GetRatModeSetting 
	**/
	MSG_USIM_GET_RAT_MODE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x105,
	 /** 
	payload is ::USIM_RAT_MODE_t 
	**/
	MSG_USIM_GET_RAT_MODE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x106,	///<Payload type {::USIM_RAT_MODE_t}
	 /** 
	api is CAPI2_MS_GetGPRSRegState 
	**/
	MSG_MS_GET_GPRS_STATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x107,
	 /** 
	payload is ::MSRegState_t 
	**/
	MSG_MS_GET_GPRS_STATE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x108,	///<Payload type {::MSRegState_t}
	 /** 
	api is CAPI2_MS_GetGSMRegState 
	**/
	MSG_MS_GET_GSM_STATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x109,
	 /** 
	payload is ::MSRegState_t 
	**/
	MSG_MS_GET_GSM_STATE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x10A,	///<Payload type {::MSRegState_t}
	 /** 
	api is CAPI2_MS_GetRegisteredCellInfo 
	**/
	MSG_MS_GET_CELL_INFO_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x10B,
	 /** 
	payload is ::CellInfo_t 
	**/
	MSG_MS_GET_CELL_INFO_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x10C,	///<Payload type {::CellInfo_t}
	 /** 
	api is CAPI2_MS_SetMEPowerClass 
	**/
	MSG_MS_SETMEPOWER_CLASS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x10D,	///<Payload type {::CAPI2_MS_SetMEPowerClass_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_MS_SETMEPOWER_CLASS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x10E,
	 /** 
	api is CAPI2_USimApi_GetServiceStatus 
	**/
	MSG_USIM_GET_SERVICE_STATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x10F,	///<Payload type {::CAPI2_USimApi_GetServiceStatus_Req_t}
	 /** 
	payload is ::SIMServiceStatus_t 
	**/
	MSG_USIM_GET_SERVICE_STATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x110,	///<Payload type {::SIMServiceStatus_t}
	 /** 
	api is CAPI2_SimApi_IsAllowedAPN 
	**/
	MSG_SIM_IS_ALLOWED_APN_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x111,	///<Payload type {::CAPI2_SimApi_IsAllowedAPN_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_IS_ALLOWED_APN_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x112,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SmsApi_GetSmsMaxCapacity 
	**/
	MSG_SMS_GETSMSMAXCAPACITY_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x113,	///<Payload type {::CAPI2_SmsApi_GetSmsMaxCapacity_Req_t}
	 /** 
	payload is ::UInt16 
	**/
	MSG_SMS_GETSMSMAXCAPACITY_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x114,	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_SmsApi_RetrieveMaxCBChnlLength 
	**/
	MSG_SMS_RETRIEVEMAXCBCHNLLENGTH_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x115,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_RETRIEVEMAXCBCHNLLENGTH_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x116,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SimApi_IsBdnOperationRestricted 
	**/
	MSG_SIM_IS_BDN_RESTRICTED_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x117,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_IS_BDN_RESTRICTED_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x118,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SimApi_SendPreferredPlmnUpdateInd 
	**/
	MSG_SIM_SEND_PLMN_UPDATE_IND_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x119,	///<Payload type {::CAPI2_SimApi_SendPreferredPlmnUpdateInd_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_SIM_SEND_PLMN_UPDATE_IND_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x11A,
	 /** 
	api is CAPI2_SIMIO_DeactiveCard 
	**/
	MSG_SIMIO_DEACTIVATE_CARD_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x11B,
	 /** 
	payload is ::void 
	**/
	MSG_SIMIO_DEACTIVATE_CARD_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x11C,
	 /** 
	api is CAPI2_SimApi_SendSetBdnReq 
	**/
	MSG_SIM_SET_BDN_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x11D,	///<Payload type {::CAPI2_SimApi_SendSetBdnReq_Req_t}
	 /** 
	api is CAPI2_SimApi_PowerOnOffCard 
	**/
	MSG_SIM_POWER_ON_OFF_CARD_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x11E,	///<Payload type {::CAPI2_SimApi_PowerOnOffCard_Req_t}
	 /** 
	api is CAPI2_SimApi_GetRawAtr 
	**/
	MSG_SIM_GET_RAW_ATR_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x11F,
	 /** 
	api is CAPI2_SimApi_Set_Protocol 
	**/
	MSG_SIM_SET_PROTOCOL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x120,	///<Payload type {::CAPI2_SimApi_Set_Protocol_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SIM_SET_PROTOCOL_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x121,
	 /** 
	api is CAPI2_SimApi_Get_Protocol 
	**/
	MSG_SIM_GET_PROTOCOL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x122,
	 /** 
	payload is ::UInt8 
	**/
	MSG_SIM_GET_PROTOCOL_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x123,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_SimApi_SendGenericApduCmd 
	**/
	MSG_SIM_SEND_GENERIC_APDU_CMD_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x124,	///<Payload type {::CAPI2_SimApi_SendGenericApduCmd_Req_t}
	 /** 
	api is CAPI2_SimApi_TerminateXferApdu 
	**/
	MSG_SIM_TERMINATE_XFER_APDU_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x125,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SIM_TERMINATE_XFER_APDU_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x126,
	 /** 
	api is CAPI2_SIM_GetSimInterface 
	**/
	MSG_SIM_GET_SIM_INTERFACE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x127,
	 /** 
	payload is ::SIM_SIM_INTERFACE_t 
	**/
	MSG_SIM_GET_SIM_INTERFACE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x128,	///<Payload type {::SIM_SIM_INTERFACE_t}
	 /** 
	api is CAPI2_NetRegApi_SetPlmnSelectRat 
	**/
	MSG_SET_PLMN_SELECT_RAT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x129,	///<Payload type {::CAPI2_NetRegApi_SetPlmnSelectRat_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SET_PLMN_SELECT_RAT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x12A,
	 /** 
	api is CAPI2_MS_IsDeRegisterInProgress 
	**/
	MSG_IS_DEREGISTER_IN_PROGRESS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x12B,
	 /** 
	payload is ::Boolean 
	**/
	MSG_IS_DEREGISTER_IN_PROGRESS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x12C,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_MS_IsRegisterInProgress 
	**/
	MSG_IS_REGISTER_IN_PROGRESS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x12D,
	 /** 
	payload is ::Boolean 
	**/
	MSG_IS_REGISTER_IN_PROGRESS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x12E,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PbkApi_SendUsimHdkReadReq 
	**/
	MSG_READ_USIM_PBK_HDK_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x12F,
	 /** 
	api is CAPI2_PbkApi_SendUsimHdkUpdateReq 
	**/
	MSG_WRITE_USIM_PBK_HDK_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x130,	///<Payload type {::CAPI2_PbkApi_SendUsimHdkUpdateReq_Req_t}
	 /** 
	api is CAPI2_PbkApi_SendUsimAasReadReq 
	**/
	MSG_READ_USIM_PBK_ALPHA_AAS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x131,	///<Payload type {::CAPI2_PbkApi_SendUsimAasReadReq_Req_t}
	 /** 
	api is CAPI2_PbkApi_SendUsimAasUpdateReq 
	**/
	MSG_WRITE_USIM_PBK_ALPHA_AAS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x132,	///<Payload type {::CAPI2_PbkApi_SendUsimAasUpdateReq_Req_t}
	 /** 
	api is CAPI2_PbkApi_SendUsimGasReadReq 
	**/
	MSG_READ_USIM_PBK_ALPHA_GAS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x133,	///<Payload type {::CAPI2_PbkApi_SendUsimGasReadReq_Req_t}
	 /** 
	api is CAPI2_PbkApi_SendUsimGasUpdateReq 
	**/
	MSG_WRITE_USIM_PBK_ALPHA_GAS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x134,	///<Payload type {::CAPI2_PbkApi_SendUsimGasUpdateReq_Req_t}
	 /** 
	api is CAPI2_PbkApi_SendUsimAasInfoReq 
	**/
	MSG_GET_USIM_PBK_ALPHA_INFO_AAS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x135,	///<Payload type {::CAPI2_PbkApi_SendUsimAasInfoReq_Req_t}
	 /** 
	api is CAPI2_PbkApi_SendUsimGasInfoReq 
	**/
	MSG_GET_USIM_PBK_ALPHA_INFO_GAS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x136,	///<Payload type {::CAPI2_PbkApi_SendUsimGasInfoReq_Req_t}
	 /** 
	api is CAPI2_DiagApi_MeasurmentReportReq 
	**/
	MSG_DIAG_MEASURE_REPORT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x137,	///<Payload type {::CAPI2_DiagApi_MeasurmentReportReq_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_DIAG_MEASURE_REPORT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x138,
	 /** 
	api is CAPI2_PMU_BattChargingNotification 
	**/
	MSG_PMU_BATT_CHARGING_NOTIFICATION_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x139,	///<Payload type {::CAPI2_PMU_BattChargingNotification_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_PMU_BATT_CHARGING_NOTIFICATION_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x13A,
	 /** 
	api is CAPI2_MsDbApi_InitCallCfg 
	**/
	MSG_MS_INITCALLCFG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x13B,
	 /** 
	payload is ::Result_t 
	**/
	MSG_MS_INITCALLCFG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x13C,
	 /** 
	api is CAPI2_MS_InitFaxConfig 
	**/
	MSG_MS_INITFAXCFG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x13D,
	 /** 
	payload is ::Result_t 
	**/
	MSG_MS_INITFAXCFG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x13E,
	 /** 
	api is CAPI2_MS_InitVideoCallCfg 
	**/
	MSG_MS_INITVIDEOCALLCFG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x13F,
	 /** 
	payload is ::Result_t 
	**/
	MSG_MS_INITVIDEOCALLCFG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x140,
	 /** 
	api is CAPI2_MS_InitCallCfgAmpF 
	**/
	MSG_MS_INITCALLCFGAMPF_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x141,
	 /** 
	payload is ::Result_t 
	**/
	MSG_MS_INITCALLCFGAMPF_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x142,
	 /** 
	api is CAPI2_SYSPARM_GetActualLowVoltReading 
	**/
	MSG_SYSPARM_GET_ACTUAL_LOW_VOLT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x143,
	 /** 
	payload is ::UInt16 
	**/
	MSG_SYSPARM_GET_ACTUAL_LOW_VOLT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x144,	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_SYSPARM_GetActual4p2VoltReading 
	**/
	MSG_SYSPARM_GET_ACTUAL_4P2_VOLT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x145,
	 /** 
	payload is ::UInt16 
	**/
	MSG_SYSPARM_GET_ACTUAL_4P2_VOLT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x146,	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_SmsApi_SendSMSCommandTxtReq 
	**/
	MSG_SMS_SEND_COMMAND_TXT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x147,	///<Payload type {::CAPI2_SmsApi_SendSMSCommandTxtReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SEND_COMMAND_TXT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x148,
	 /** 
	api is CAPI2_SmsApi_SendSMSCommandPduReq 
	**/
	MSG_SMS_SEND_COMMAND_PDU_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x149,	///<Payload type {::CAPI2_SmsApi_SendSMSCommandPduReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SEND_COMMAND_PDU_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x14A,
	 /** 
	api is CAPI2_SmsApi_SendPDUAckToNetwork 
	**/
	MSG_SMS_SEND_ACKTONETWORK_PDU_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x14B,	///<Payload type {::CAPI2_SmsApi_SendPDUAckToNetwork_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_SMS_SEND_ACKTONETWORK_PDU_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x14C,
	 /** 
	api is CAPI2_SmsApi_StartCellBroadcastWithChnlReq 
	**/
	MSG_SMS_CB_START_STOP_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x14D,	///<Payload type {::CAPI2_SmsApi_StartCellBroadcastWithChnlReq_Req_t}
	 /** 
	api is CAPI2_SmsApi_SetMoSmsTpMr 
	**/
	MSG_SMS_SET_TPMR_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x14E,	///<Payload type {::CAPI2_SmsApi_SetMoSmsTpMr_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SET_TPMR_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x14F,
	 /** 
	api is CAPI2_SIMLOCKApi_SetStatus 
	**/
	MSG_SIMLOCK_SET_STATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x150,	///<Payload type {::CAPI2_SIMLOCKApi_SetStatus_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_SIMLOCK_SET_STATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x151,
	 /** 
	api is CAPI2_DIAG_ApiCellLockReq 
	**/
	MSG_DIAG_CELLLOCK_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x152,	///<Payload type {::CAPI2_DIAG_ApiCellLockReq_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_DIAG_CELLLOCK_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x153,
	 /** 
	api is CAPI2_DIAG_ApiCellLockStatus 
	**/
	MSG_DIAG_CELLLOCK_STATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x154,
	 /** 
	payload is ::Boolean 
	**/
	MSG_DIAG_CELLLOCK_STATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x155,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_MS_SetRuaReadyTimer 
	**/
	MSG_MS_SET_RUA_READY_TIMER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x156,	///<Payload type {::CAPI2_MS_SetRuaReadyTimer_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_MS_SET_RUA_READY_TIMER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x157,
	 /** 
	api is CAPI2_LCS_RegisterRrlpDataHandler 
	**/
	MSG_LCS_REG_RRLP_HDL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x158,	///<Payload type {::CAPI2_LCS_RegisterRrlpDataHandler_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_LCS_REG_RRLP_HDL_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x159,
	 /** 
	api is CAPI2_LCS_RegisterRrcDataHandler 
	**/
	MSG_LCS_REG_RRC_HDL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x15A,	///<Payload type {::CAPI2_LCS_RegisterRrcDataHandler_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_LCS_REG_RRC_HDL_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x15B,
	 /** 
	api is CAPI2_CcApi_IsThereEmergencyCall 
	**/
	MSG_CC_IS_THERE_EMERGENCY_CALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x15C,
	 /** 
	payload is ::Boolean 
	**/
	MSG_CC_IS_THERE_EMERGENCY_CALL_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x15D,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SYS_EnableCellInfoMsg 
	**/
	MSG_SYS_ENABLE_CELL_INFO_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x15E,	///<Payload type {::CAPI2_SYS_EnableCellInfoMsg_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_SYS_ENABLE_CELL_INFO_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x15F,
	 /** 
	api is CAPI2_LCS_L1_bb_isLocked 
	**/
	MSG_L1_BB_ISLOCKED_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x160,	///<Payload type {::CAPI2_LCS_L1_bb_isLocked_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_L1_BB_ISLOCKED_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x161,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_DIALSTR_ParseGetCallType 
	**/
	MSG_UTIL_DIAL_STR_PARSE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x162,	///<Payload type {::CAPI2_DIALSTR_ParseGetCallType_Req_t}
	 /** 
	payload is ::CallType_t 
	**/
	MSG_UTIL_DIAL_STR_PARSE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x163,	///<Payload type {::CallType_t}
	 /** 
	api is CAPI2_LCS_FttCalcDeltaTime 
	**/
	MSG_LCS_FTT_DELTA_TIME_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x164,	///<Payload type {::CAPI2_LCS_FttCalcDeltaTime_Req_t}
	 /** 
	payload is ::UInt32 
	**/
	MSG_LCS_FTT_DELTA_TIME_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x165,	///<Payload type {::UInt32}
	 /** 
	api is CAPI2_NetRegApi_ForcedReadyStateReq 
	**/
	MSG_MS_FORCEDREADYSTATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x166,	///<Payload type {::CAPI2_NetRegApi_ForcedReadyStateReq_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_MS_FORCEDREADYSTATE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x167,
	 /** 
	api is CAPI2_SsApi_ResetSsAlsFlag 
	**/
	MSG_SS_RESETSSALSFLAG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x168,
	 /** 
	payload is ::void 
	**/
	MSG_SS_RESETSSALSFLAG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x169,
	 /** 
	api is CAPI2_SIMLOCK_GetStatus 
	**/
	MSG_SIMLOCK_GET_STATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x16A,	///<Payload type {::CAPI2_SIMLOCK_GetStatus_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_SIMLOCK_GET_STATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x16B,
	 /** 
	api is CAPI2_DIALSTR_IsValidString 
	**/
	MSG_DIALSTR_IS_VALID_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x16C,	///<Payload type {::CAPI2_DIALSTR_IsValidString_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_DIALSTR_IS_VALID_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x16D,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_UTIL_Cause2NetworkCause 
	**/
	MSG_UTIL_CONVERT_NTWK_CAUSE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x16E,	///<Payload type {::CAPI2_UTIL_Cause2NetworkCause_Req_t}
	 /** 
	payload is ::NetworkCause_t 
	**/
	MSG_UTIL_CONVERT_NTWK_CAUSE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x16F,	///<Payload type {::NetworkCause_t}
	 /** 
	api is CAPI2_UTIL_ErrCodeToNetCause 
	**/
	MSG_UTIL_CONVERT_ERRCODE_NTWK_CAUSE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x170,	///<Payload type {::CAPI2_UTIL_ErrCodeToNetCause_Req_t}
	 /** 
	payload is ::NetworkCause_t 
	**/
	MSG_UTIL_CONVERT_ERRCODE_NTWK_CAUSE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x171,	///<Payload type {::NetworkCause_t}
	 /** 
	api is CAPI2_IsGprsDialStr 
	**/
	MSG_ISGPRS_DIAL_STR_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x172,	///<Payload type {::CAPI2_IsGprsDialStr_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_ISGPRS_DIAL_STR_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x173,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_UTIL_GetNumOffsetInSsStr 
	**/
	MSG_GET_NUM_SS_STR_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x174,	///<Payload type {::CAPI2_UTIL_GetNumOffsetInSsStr_Req_t}
	 /** 
	payload is ::UInt8 
	**/
	MSG_GET_NUM_SS_STR_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x175,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_IsPppLoopbackDialStr 
	**/
	MSG_DIALSTR_IS_PPPLOOPBACK_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x176,	///<Payload type {::CAPI2_IsPppLoopbackDialStr_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_DIALSTR_IS_PPPLOOPBACK_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x177,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SYS_GetRIPPROCVersion 
	**/
	MSG_SYS_GETRIPPROCVERSION_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x178,
	 /** 
	payload is ::UInt8 
	**/
	MSG_SYS_GETRIPPROCVERSION_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x179,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_SYSPARM_SetHSDPAPHYCategory 
	**/
	MSG_SYSPARM_SET_HSDPA_PHY_CAT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x17A,	///<Payload type {::CAPI2_SYSPARM_SetHSDPAPHYCategory_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_SYSPARM_SET_HSDPA_PHY_CAT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x17B,
	 /** 
	api is CAPI2_SYSPARM_GetHSDPAPHYCategory 
	**/
	MSG_SYS_GET_HSDPA_CATEGORY_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x17C,
	 /** 
	payload is ::UInt8 
	**/
	MSG_SYS_GET_HSDPA_CATEGORY_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x17D,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_SmsApi_ConvertSmsMSMsgType 
	**/
	MSG_SMS_CONVERT_MSGTYPE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x17E,	///<Payload type {::CAPI2_SmsApi_ConvertSmsMSMsgType_Req_t}
	 /** 
	payload is ::UInt8 
	**/
	MSG_SMS_CONVERT_MSGTYPE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x17F,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_MS_GetPrefNetStatus 
	**/
	MSG_MS_GETPREFNETSTATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x180,
	 /** 
	payload is ::GANStatus_t 
	**/
	MSG_MS_GETPREFNETSTATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x181,	///<Payload type {::GANStatus_t}
	 /** 
	payload is ::uchar_ptr_t 
	**/
	MSG_SYS_TEST_MSG_IND_1  = MSG_GRP_CAPI2_GEN_0 + 0x182,	///<Payload type {::uchar_ptr_t}
	 /** 
	payload is ::uchar_ptr_t 
	**/
	MSG_SYS_TEST_MSG_IND_2  = MSG_GRP_CAPI2_GEN_0 + 0x183,	///<Payload type {::uchar_ptr_t}
	 /** 
	payload is ::uchar_ptr_t 
	**/
	MSG_SYS_TEST_MSG_IND_3  = MSG_GRP_CAPI2_GEN_0 + 0x184,	///<Payload type {::uchar_ptr_t}
	 /** 
	payload is ::uchar_ptr_t 
	**/
	MSG_SYS_TEST_MSG_IND_4  = MSG_GRP_CAPI2_GEN_0 + 0x185,	///<Payload type {::uchar_ptr_t}
	 /** 
	payload is ::uchar_ptr_t 
	**/
	MSG_SYS_TEST_MSG_IND_5  = MSG_GRP_CAPI2_GEN_0 + 0x186,	///<Payload type {::uchar_ptr_t}
	 /** 
	payload is ::uchar_ptr_t 
	**/
	MSG_SYS_TEST_MSG_IND_6  = MSG_GRP_CAPI2_GEN_0 + 0x187,	///<Payload type {::uchar_ptr_t}
	 /** 
	api is CAPI2_InterTaskMsgToCP 
	**/
	MSG_INTERTASK_MSG_TO_CP_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x188,	///<Payload type {::CAPI2_InterTaskMsgToCP_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_INTERTASK_MSG_TO_CP_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x189,
	 /** 
	api is CAPI2_InterTaskMsgToAP 
	**/
	MSG_INTERTASK_MSG_TO_AP_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x18A,	///<Payload type {::CAPI2_InterTaskMsgToAP_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_INTERTASK_MSG_TO_AP_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x18B,
	 /** 
	api is CAPI2_CcApi_GetCurrentCallIndex 
	**/
	MSG_CC_GETCURRENTCALLINDEX_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x18C,
	 /** 
	payload is ::UInt8 
	**/
	MSG_CC_GETCURRENTCALLINDEX_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x18D,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_CcApi_GetNextActiveCallIndex 
	**/
	MSG_CC_GETNEXTACTIVECALLINDEX_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x18E,
	 /** 
	payload is ::UInt8 
	**/
	MSG_CC_GETNEXTACTIVECALLINDEX_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x18F,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_CcApi_GetNextHeldCallIndex 
	**/
	MSG_CC_GETNEXTHELDCALLINDEX_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x190,
	 /** 
	payload is ::UInt8 
	**/
	MSG_CC_GETNEXTHELDCALLINDEX_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x191,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_CcApi_GetNextWaitCallIndex 
	**/
	MSG_CC_GETNEXTWAITCALLINDEX_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x192,
	 /** 
	payload is ::UInt8 
	**/
	MSG_CC_GETNEXTWAITCALLINDEX_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x193,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_CcApi_GetMPTYCallIndex 
	**/
	MSG_CC_GETMPTYCALLINDEX_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x194,
	 /** 
	payload is ::UInt8 
	**/
	MSG_CC_GETMPTYCALLINDEX_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x195,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_CcApi_GetCallState 
	**/
	MSG_CC_GETCALLSTATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x196,	///<Payload type {::CAPI2_CcApi_GetCallState_Req_t}
	 /** 
	payload is ::CCallState_t 
	**/
	MSG_CC_GETCALLSTATE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x197,	///<Payload type {::CCallState_t}
	 /** 
	api is CAPI2_CcApi_GetCallType 
	**/
	MSG_CC_GETCALLTYPE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x198,	///<Payload type {::CAPI2_CcApi_GetCallType_Req_t}
	 /** 
	payload is ::CCallType_t 
	**/
	MSG_CC_GETCALLTYPE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x199,	///<Payload type {::CCallType_t}
	 /** 
	api is CAPI2_CcApi_GetLastCallExitCause 
	**/
	MSG_CC_GETLASTCALLEXITCAUSE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x19A,
	 /** 
	payload is ::Cause_t 
	**/
	MSG_CC_GETLASTCALLEXITCAUSE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x19B,	///<Payload type {::Cause_t}
	 /** 
	api is CAPI2_CcApi_GetCallNumber 
	**/
	MSG_CC_GETCALLNUMBER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x19C,	///<Payload type {::CAPI2_CcApi_GetCallNumber_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_CC_GETCALLNUMBER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x19D,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_CcApi_GetCallingInfo 
	**/
	MSG_CC_GETCALLINGINFO_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x19E,	///<Payload type {::CAPI2_CcApi_GetCallingInfo_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_CC_GETCALLINGINFO_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x19F,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_CcApi_GetAllCallStates 
	**/
	MSG_CC_GETALLCALLSTATES_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1A0,
	 /** 
	payload is ::Result_t 
	**/
	MSG_CC_GETALLCALLSTATES_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1A1,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_CcApi_GetAllCallIndex 
	**/
	MSG_CC_GETALLCALLINDEX_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1A2,
	 /** 
	payload is ::Result_t 
	**/
	MSG_CC_GETALLCALLINDEX_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1A3,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_CcApi_GetAllHeldCallIndex 
	**/
	MSG_CC_GETALLHELDCALLINDEX_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1A4,
	 /** 
	payload is ::Result_t 
	**/
	MSG_CC_GETALLHELDCALLINDEX_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1A5,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_CcApi_GetAllActiveCallIndex 
	**/
	MSG_CC_GETALLACTIVECALLINDEX_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1A6,
	 /** 
	payload is ::Result_t 
	**/
	MSG_CC_GETALLACTIVECALLINDEX_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1A7,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_CcApi_GetAllMPTYCallIndex 
	**/
	MSG_CC_GETALLMPTYCALLINDEX_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1A8,
	 /** 
	payload is ::Result_t 
	**/
	MSG_CC_GETALLMPTYCALLINDEX_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1A9,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_CcApi_GetNumOfMPTYCalls 
	**/
	MSG_CC_GETNUMOFMPTYCALLS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1AA,
	 /** 
	payload is ::UInt8 
	**/
	MSG_CC_GETNUMOFMPTYCALLS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1AB,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_CcApi_GetNumofActiveCalls 
	**/
	MSG_CC_GETNUMOFACTIVECALLS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1AC,
	 /** 
	payload is ::UInt8 
	**/
	MSG_CC_GETNUMOFACTIVECALLS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1AD,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_CcApi_GetNumofHeldCalls 
	**/
	MSG_CC_GETNUMOFHELDCALLS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1AE,
	 /** 
	payload is ::UInt8 
	**/
	MSG_CC_GETNUMOFHELDCALLS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1AF,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_CcApi_IsThereWaitingCall 
	**/
	MSG_CC_ISTHEREWAITINGCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1B0,
	 /** 
	payload is ::Boolean 
	**/
	MSG_CC_ISTHEREWAITINGCALL_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1B1,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_CcApi_IsThereAlertingCall 
	**/
	MSG_CC_ISTHEREALERTINGCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1B2,
	 /** 
	payload is ::Boolean 
	**/
	MSG_CC_ISTHEREALERTINGCALL_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1B3,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_CcApi_GetConnectedLineID 
	**/
	MSG_CC_GETCONNECTEDLINEID_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1B4,	///<Payload type {::CAPI2_CcApi_GetConnectedLineID_Req_t}
	 /** 
	payload is ::UInt8 
	**/
	MSG_CC_GETCONNECTEDLINEID_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1B5,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_CcApi_GetCallPresent 
	**/
	MSG_CC_GET_CALL_PRESENT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1B6,	///<Payload type {::CAPI2_CcApi_GetCallPresent_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_CC_GET_CALL_PRESENT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1B7,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_CcApi_GetCallIndexInThisState 
	**/
	MSG_CC_GET_INDEX_STATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1B8,	///<Payload type {::CAPI2_CcApi_GetCallIndexInThisState_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_CC_GET_INDEX_STATE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1B9,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_CcApi_IsMultiPartyCall 
	**/
	MSG_CC_ISMULTIPARTYCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1BA,	///<Payload type {::CAPI2_CcApi_IsMultiPartyCall_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_CC_ISMULTIPARTYCALL_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1BB,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_CcApi_IsThereVoiceCall 
	**/
	MSG_CC_ISTHEREVOICECALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1BC,
	 /** 
	payload is ::Boolean 
	**/
	MSG_CC_ISTHEREVOICECALL_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1BD,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_CcApi_IsConnectedLineIDPresentAllowed 
	**/
	MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1BE,	///<Payload type {::CAPI2_CcApi_IsConnectedLineIDPresentAllowed_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1BF,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_CcApi_GetCurrentCallDurationInMilliSeconds 
	**/
	MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1C0,	///<Payload type {::CAPI2_CcApi_GetCurrentCallDurationInMilliSeconds_Req_t}
	 /** 
	payload is ::UInt32 
	**/
	MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1C1,	///<Payload type {::UInt32}
	 /** 
	api is CAPI2_CcApi_GetLastCallCCM 
	**/
	MSG_CC_GETLASTCALLCCM_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1C2,
	 /** 
	payload is ::UInt32 
	**/
	MSG_CC_GETLASTCALLCCM_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1C3,	///<Payload type {::UInt32}
	 /** 
	api is CAPI2_CcApi_GetLastCallDuration 
	**/
	MSG_CC_GETLASTCALLDURATION_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1C4,
	 /** 
	payload is ::UInt32 
	**/
	MSG_CC_GETLASTCALLDURATION_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1C5,	///<Payload type {::UInt32}
	 /** 
	api is CAPI2_CcApi_GetLastDataCallRxBytes 
	**/
	MSG_CC_GETLASTDATACALLRXBYTES_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1C6,
	 /** 
	payload is ::UInt32 
	**/
	MSG_CC_GETLASTDATACALLRXBYTES_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1C7,	///<Payload type {::UInt32}
	 /** 
	api is CAPI2_CcApi_GetLastDataCallTxBytes 
	**/
	MSG_CC_GETLASTDATACALLTXBYTES_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1C8,
	 /** 
	payload is ::UInt32 
	**/
	MSG_CC_GETLASTDATACALLTXBYTES_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1C9,	///<Payload type {::UInt32}
	 /** 
	api is CAPI2_CcApi_GetDataCallIndex 
	**/
	MSG_CC_GETDATACALLINDEX_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1CA,
	 /** 
	payload is ::UInt8 
	**/
	MSG_CC_GETDATACALLINDEX_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1CB,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_CcApi_GetCallClientInfo 
	**/
	MSG_CC_GETCALLCLIENT_INFO_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1CC,	///<Payload type {::CAPI2_CcApi_GetCallClientInfo_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_CC_GETCALLCLIENT_INFO_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1CD,	///<Payload type {::void}
	 /** 
	api is CAPI2_CcApi_GetCallClientID 
	**/
	MSG_CC_GETCALLCLIENTID_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1CE,	///<Payload type {::CAPI2_CcApi_GetCallClientID_Req_t}
	 /** 
	payload is ::UInt8 
	**/
	MSG_CC_GETCALLCLIENTID_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1CF,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_CcApi_GetTypeAdd 
	**/
	MSG_CC_GETTYPEADD_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1D0,	///<Payload type {::CAPI2_CcApi_GetTypeAdd_Req_t}
	 /** 
	payload is ::UInt8 
	**/
	MSG_CC_GETTYPEADD_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1D1,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_CcApi_SetVoiceCallAutoReject 
	**/
	MSG_CC_SETVOICECALLAUTOREJECT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1D2,	///<Payload type {::CAPI2_CcApi_SetVoiceCallAutoReject_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_CC_SETVOICECALLAUTOREJECT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1D3,
	 /** 
	api is CAPI2_CcApi_IsVoiceCallAutoReject 
	**/
	MSG_CC_ISVOICECALLAUTOREJECT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1D4,
	 /** 
	payload is ::Boolean 
	**/
	MSG_CC_ISVOICECALLAUTOREJECT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1D5,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_CcApi_SetTTYCall 
	**/
	MSG_CC_SETTTYCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1D6,	///<Payload type {::CAPI2_CcApi_SetTTYCall_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_CC_SETTTYCALL_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1D7,
	 /** 
	api is CAPI2_CcApi_IsTTYEnable 
	**/
	MSG_CC_ISTTYENABLE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1D8,
	 /** 
	payload is ::Boolean 
	**/
	MSG_CC_ISTTYENABLE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1D9,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_CcApi_IsSimOriginedCall 
	**/
	MSG_CC_ISSIMORIGINEDCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1DA,	///<Payload type {::CAPI2_CcApi_IsSimOriginedCall_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_CC_ISSIMORIGINEDCALL_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1DB,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_CcApi_SetVideoCallParam 
	**/
	MSG_CC_SETVIDEOCALLPARAM_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1DC,	///<Payload type {::CAPI2_CcApi_SetVideoCallParam_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_CC_SETVIDEOCALLPARAM_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1DD,
	 /** 
	api is CAPI2_CcApi_GetVideoCallParam 
	**/
	MSG_CC_GETVIDEOCALLPARAM_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1DE,
	 /** 
	payload is ::VideoCallParam_t 
	**/
	MSG_CC_GETVIDEOCALLPARAM_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1DF,	///<Payload type {::VideoCallParam_t}
	 /** 
	api is CAPI2_CcApi_GetCCM 
	**/
	MSG_CC_GETCCM_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1E0,	///<Payload type {::CAPI2_CcApi_GetCCM_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_CC_GETCCM_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1E1,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_CcApi_SendDtmfTone 
	**/
	MSG_CCAPI_SENDDTMF_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1E2,	///<Payload type {::CAPI2_CcApi_SendDtmfTone_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_CCAPI_SENDDTMF_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1E3,
	 /** 
	api is CAPI2_CcApi_StopDtmfTone 
	**/
	MSG_CCAPI_STOPDTMF_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1E4,	///<Payload type {::CAPI2_CcApi_StopDtmfTone_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_CCAPI_STOPDTMF_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1E5,
	 /** 
	api is CAPI2_CcApi_AbortDtmf 
	**/
	MSG_CCAPI_ABORTDTMF_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1E6,	///<Payload type {::CAPI2_CcApi_AbortDtmf_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_CCAPI_ABORTDTMF_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1E7,
	 /** 
	api is CAPI2_CcApi_SetDtmfToneTimer 
	**/
	MSG_CCAPI_SETDTMFTIMER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1E8,	///<Payload type {::CAPI2_CcApi_SetDtmfToneTimer_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_CCAPI_SETDTMFTIMER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1E9,
	 /** 
	api is CAPI2_CcApi_ResetDtmfToneTimer 
	**/
	MSG_CCAPI_RESETDTMFTIMER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1EA,	///<Payload type {::CAPI2_CcApi_ResetDtmfToneTimer_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_CCAPI_RESETDTMFTIMER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1EB,
	 /** 
	api is CAPI2_CcApi_GetDtmfToneTimer 
	**/
	MSG_CCAPI_GETDTMFTIMER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1EC,	///<Payload type {::CAPI2_CcApi_GetDtmfToneTimer_Req_t}
	 /** 
	payload is ::Ticks_t 
	**/
	MSG_CCAPI_GETDTMFTIMER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1ED,	///<Payload type {::Ticks_t}
	 /** 
	api is CAPI2_CcApi_GetTiFromCallIndex 
	**/
	MSG_CCAPI_GETTIFROMCALLINDEX_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1EE,	///<Payload type {::CAPI2_CcApi_GetTiFromCallIndex_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_CCAPI_GETTIFROMCALLINDEX_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1EF,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_CcApi_IsSupportedBC 
	**/
	MSG_CCAPI_IS_SUPPORTEDBC_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1F0,	///<Payload type {::CAPI2_CcApi_IsSupportedBC_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_CCAPI_IS_SUPPORTEDBC_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1F1,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_CcApi_GetBearerCapability 
	**/
	MSG_CCAPI_IS_BEARER_CAPABILITY_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1F2,	///<Payload type {::CAPI2_CcApi_GetBearerCapability_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_CCAPI_IS_BEARER_CAPABILITY_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1F3,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SmsApi_SendSMSSrvCenterNumberUpdateReq 
	**/
	MSG_SMS_SENDSMSSRVCENTERNUMBERUPDATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1F4,	///<Payload type {::CAPI2_SmsApi_SendSMSSrvCenterNumberUpdateReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SENDSMSSRVCENTERNUMBERUPDATE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1F5,
	 /** 
	api is CAPI2_SmsApi_GetSMSrvCenterNumber 
	**/
	MSG_SMS_GETSMSSRVCENTERNUMBER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1F6,	///<Payload type {::CAPI2_SmsApi_GetSMSrvCenterNumber_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_GETSMSSRVCENTERNUMBER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1F7,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SmsApi_GetSIMSMSCapacityExceededFlag 
	**/
	MSG_SMS_GETSIMSMSCAPACITYEXCEEDEDFLAG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1F8,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_GETSIMSMSCAPACITYEXCEEDEDFLAG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1F9,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SmsApi_IsSmsServiceAvail 
	**/
	MSG_SMS_ISSMSSERVICEAVAIL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1FA,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_ISSMSSERVICEAVAIL_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1FB,
	 /** 
	api is CAPI2_SmsApi_GetSmsStoredState 
	**/
	MSG_SMS_GETSMSSTOREDSTATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1FC,	///<Payload type {::CAPI2_SmsApi_GetSmsStoredState_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_GETSMSSTOREDSTATE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1FD,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SmsApi_WriteSMSPduReq 
	**/
	MSG_SMS_WRITESMSPDU_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1FE,	///<Payload type {::CAPI2_SmsApi_WriteSMSPduReq_Req_t}
	 /** 
	api is CAPI2_SmsApi_WriteSMSReq 
	**/
	MSG_SMS_WRITESMSREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1FF,	///<Payload type {::CAPI2_SmsApi_WriteSMSReq_Req_t}
	 /** 
	api is CAPI2_SmsApi_SendSMSReq 
	**/
	MSG_SMS_SENDSMSREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x200,	///<Payload type {::CAPI2_SmsApi_SendSMSReq_Req_t}
	 /** 
	api is CAPI2_SmsApi_SendSMSPduReq 
	**/
	MSG_SMS_SENDSMSPDUREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x201,	///<Payload type {::CAPI2_SmsApi_SendSMSPduReq_Req_t}
	 /** 
	api is CAPI2_SmsApi_SendStoredSmsReq 
	**/
	MSG_SMS_SENDSTOREDSMSREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x202,	///<Payload type {::CAPI2_SmsApi_SendStoredSmsReq_Req_t}
	 /** 
	api is CAPI2_SmsApi_WriteSMSPduToSIMReq 
	**/
	MSG_SMS_WRITESMSPDUTOSIM_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x203,	///<Payload type {::CAPI2_SmsApi_WriteSMSPduToSIMReq_Req_t}
	 /** 
	api is CAPI2_SmsApi_GetLastTpMr 
	**/
	MSG_SMS_GETLASTTPMR_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x204,
	 /** 
	payload is ::UInt8 
	**/
	MSG_SMS_GETLASTTPMR_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x205,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_SmsApi_GetSmsTxParams 
	**/
	MSG_SMS_GETSMSTXPARAMS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x206,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_GETSMSTXPARAMS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x207,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SmsApi_GetTxParamInTextMode 
	**/
	MSG_SMS_GETTXPARAMINTEXTMODE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x208,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_GETTXPARAMINTEXTMODE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x209,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SmsApi_SetSmsTxParamProcId 
	**/
	MSG_SMS_SETSMSTXPARAMPROCID_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x20A,	///<Payload type {::CAPI2_SmsApi_SetSmsTxParamProcId_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SETSMSTXPARAMPROCID_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x20B,
	 /** 
	api is CAPI2_SmsApi_SetSmsTxParamCodingType 
	**/
	MSG_SMS_SETSMSTXPARAMCODINGTYPE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x20C,	///<Payload type {::CAPI2_SmsApi_SetSmsTxParamCodingType_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SETSMSTXPARAMCODINGTYPE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x20D,
	 /** 
	api is CAPI2_SmsApi_SetSmsTxParamValidPeriod 
	**/
	MSG_SMS_SETSMSTXPARAMVALIDPERIOD_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x20E,	///<Payload type {::CAPI2_SmsApi_SetSmsTxParamValidPeriod_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SETSMSTXPARAMVALIDPERIOD_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x20F,
	 /** 
	api is CAPI2_SmsApi_SetSmsTxParamCompression 
	**/
	MSG_SMS_SETSMSTXPARAMCOMPRESSION_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x210,	///<Payload type {::CAPI2_SmsApi_SetSmsTxParamCompression_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SETSMSTXPARAMCOMPRESSION_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x211,
	 /** 
	api is CAPI2_SmsApi_SetSmsTxParamReplyPath 
	**/
	MSG_SMS_SETSMSTXPARAMREPLYPATH_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x212,	///<Payload type {::CAPI2_SmsApi_SetSmsTxParamReplyPath_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SETSMSTXPARAMREPLYPATH_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x213,
	 /** 
	api is CAPI2_SmsApi_SetSmsTxParamUserDataHdrInd 
	**/
	MSG_SMS_SETSMSTXPARAMUSERDATAHDRIND_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x214,	///<Payload type {::CAPI2_SmsApi_SetSmsTxParamUserDataHdrInd_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SETSMSTXPARAMUSERDATAHDRIND_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x215,
	 /** 
	api is CAPI2_SmsApi_SetSmsTxParamStatusRptReqFlag 
	**/
	MSG_SMS_SETSMSTXPARAMSTATUSRPTREQFLAG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x216,	///<Payload type {::CAPI2_SmsApi_SetSmsTxParamStatusRptReqFlag_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SETSMSTXPARAMSTATUSRPTREQFLAG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x217,
	 /** 
	api is CAPI2_SmsApi_SetSmsTxParamRejDupl 
	**/
	MSG_SMS_SETSMSTXPARAMREJDUPL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x218,	///<Payload type {::CAPI2_SmsApi_SetSmsTxParamRejDupl_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SETSMSTXPARAMREJDUPL_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x219,
	 /** 
	api is CAPI2_SmsApi_DeleteSmsMsgByIndexReq 
	**/
	MSG_SMS_DELETESMSMSGBYINDEX_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x21A,	///<Payload type {::CAPI2_SmsApi_DeleteSmsMsgByIndexReq_Req_t}
	 /** 
	api is CAPI2_SmsApi_ReadSmsMsgReq 
	**/
	MSG_SMS_READSMSMSG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x21B,	///<Payload type {::CAPI2_SmsApi_ReadSmsMsgReq_Req_t}
	 /** 
	api is CAPI2_SmsApi_ListSmsMsgReq 
	**/
	MSG_SMS_LISTSMSMSG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x21C,	///<Payload type {::CAPI2_SmsApi_ListSmsMsgReq_Req_t}
	 /** 
	api is CAPI2_SmsApi_SetNewMsgDisplayPref 
	**/
	MSG_SMS_SETNEWMSGDISPLAYPREF_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x21D,	///<Payload type {::CAPI2_SmsApi_SetNewMsgDisplayPref_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SETNEWMSGDISPLAYPREF_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x21E,
	 /** 
	api is CAPI2_SmsApi_GetNewMsgDisplayPref 
	**/
	MSG_SMS_GETNEWMSGDISPLAYPREF_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x21F,	///<Payload type {::CAPI2_SmsApi_GetNewMsgDisplayPref_Req_t}
	 /** 
	payload is ::UInt8 
	**/
	MSG_SMS_GETNEWMSGDISPLAYPREF_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x220,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_SmsApi_SetSMSPrefStorage 
	**/
	MSG_SMS_SETSMSPREFSTORAGE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x221,	///<Payload type {::CAPI2_SmsApi_SetSMSPrefStorage_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SETSMSPREFSTORAGE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x222,
	 /** 
	api is CAPI2_SmsApi_GetSMSPrefStorage 
	**/
	MSG_SMS_GETSMSPREFSTORAGE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x223,
	 /** 
	payload is ::SmsStorage_t 
	**/
	MSG_SMS_GETSMSPREFSTORAGE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x224,	///<Payload type {::SmsStorage_t}
	 /** 
	api is CAPI2_SmsApi_GetSMSStorageStatus 
	**/
	MSG_SMS_GETSMSSTORAGESTATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x225,	///<Payload type {::CAPI2_SmsApi_GetSMSStorageStatus_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_GETSMSSTORAGESTATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x226,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SmsApi_SaveSmsServiceProfile 
	**/
	MSG_SMS_SAVESMSSERVICEPROFILE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x227,	///<Payload type {::CAPI2_SmsApi_SaveSmsServiceProfile_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SAVESMSSERVICEPROFILE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x228,
	 /** 
	api is CAPI2_SmsApi_RestoreSmsServiceProfile 
	**/
	MSG_SMS_RESTORESMSSERVICEPROFILE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x229,	///<Payload type {::CAPI2_SmsApi_RestoreSmsServiceProfile_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_RESTORESMSSERVICEPROFILE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x22A,
	 /** 
	api is CAPI2_SmsApi_SetCellBroadcastMsgTypeReq 
	**/
	MSG_SMS_SETCELLBROADCASTMSGTYPE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x22B,	///<Payload type {::CAPI2_SmsApi_SetCellBroadcastMsgTypeReq_Req_t}
	 /** 
	api is CAPI2_SmsApi_CBAllowAllChnlReq 
	**/
	MSG_SMS_CBALLOWALLCHNLREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x22C,	///<Payload type {::CAPI2_SmsApi_CBAllowAllChnlReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_CBALLOWALLCHNLREQ_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x22D,
	 /** 
	api is CAPI2_SmsApi_AddCellBroadcastChnlReq 
	**/
	MSG_SMS_ADDCELLBROADCASTCHNLREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x22E,	///<Payload type {::CAPI2_SmsApi_AddCellBroadcastChnlReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_ADDCELLBROADCASTCHNLREQ_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x22F,
	 /** 
	api is CAPI2_SmsApi_RemoveCellBroadcastChnlReq 
	**/
	MSG_SMS_REMOVECELLBROADCASTCHNLREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x230,	///<Payload type {::CAPI2_SmsApi_RemoveCellBroadcastChnlReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_REMOVECELLBROADCASTCHNLREQ_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x231,
	 /** 
	api is CAPI2_SmsApi_RemoveAllCBChnlFromSearchList 
	**/
	MSG_SMS_REMOVEALLCBCHNLFROMSEARCHLIST_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x232,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_REMOVEALLCBCHNLFROMSEARCHLIST_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x233,
	 /** 
	api is CAPI2_SmsApi_GetCBMI 
	**/
	MSG_SMS_GETCBMI_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x234,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_GETCBMI_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x235,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SmsApi_GetCbLanguage 
	**/
	MSG_SMS_GETCBLANGUAGE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x236,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_GETCBLANGUAGE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x237,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SmsApi_AddCellBroadcastLangReq 
	**/
	MSG_SMS_ADDCELLBROADCASTLANGREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x238,	///<Payload type {::CAPI2_SmsApi_AddCellBroadcastLangReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_ADDCELLBROADCASTLANGREQ_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x239,
	 /** 
	api is CAPI2_SmsApi_RemoveCellBroadcastLangReq 
	**/
	MSG_SMS_REMOVECELLBROADCASTLANGREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x23A,	///<Payload type {::CAPI2_SmsApi_RemoveCellBroadcastLangReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_REMOVECELLBROADCASTLANGREQ_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x23B,
	 /** 
	api is CAPI2_SmsApi_StartReceivingCellBroadcastReq 
	**/
	MSG_SMS_STARTRECEIVINGCELLBROADCAST_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x23C,
	 /** 
	api is CAPI2_SmsApi_StopReceivingCellBroadcastReq 
	**/
	MSG_SMS_STOPRECEIVINGCELLBROADCAST_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x23D,
	 /** 
	api is CAPI2_SmsApi_SetCBIgnoreDuplFlag 
	**/
	MSG_SMS_SETCBIGNOREDUPLFLAG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x23E,	///<Payload type {::CAPI2_SmsApi_SetCBIgnoreDuplFlag_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SETCBIGNOREDUPLFLAG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x23F,
	 /** 
	api is CAPI2_SmsApi_GetCBIgnoreDuplFlag 
	**/
	MSG_SMS_GETCBIGNOREDUPLFLAG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x240,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SMS_GETCBIGNOREDUPLFLAG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x241,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SmsApi_SetVMIndOnOff 
	**/
	MSG_SMS_SETVMINDONOFF_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x242,	///<Payload type {::CAPI2_SmsApi_SetVMIndOnOff_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SETVMINDONOFF_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x243,
	 /** 
	api is CAPI2_SmsApi_IsVMIndEnabled 
	**/
	MSG_SMS_ISVMINDENABLED_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x244,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SMS_ISVMINDENABLED_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x245,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SmsApi_GetVMWaitingStatus 
	**/
	MSG_SMS_GETVMWAITINGSTATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x246,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_GETVMWAITINGSTATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x247,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SmsApi_GetNumOfVmscNumber 
	**/
	MSG_SMS_GETNUMOFVMSCNUMBER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x248,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_GETNUMOFVMSCNUMBER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x249,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SmsApi_GetVmscNumber 
	**/
	MSG_SMS_GETVMSCNUMBER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x24A,	///<Payload type {::CAPI2_SmsApi_GetVmscNumber_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_GETVMSCNUMBER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x24B,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SmsApi_UpdateVmscNumberReq 
	**/
	MSG_SMS_UPDATEVMSCNUMBERREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x24C,	///<Payload type {::CAPI2_SmsApi_UpdateVmscNumberReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_UPDATEVMSCNUMBERREQ_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x24D,
	 /** 
	api is CAPI2_SmsApi_GetSMSBearerPreference 
	**/
	MSG_SMS_GETSMSBEARERPREFERENCE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x24E,
	 /** 
	payload is ::SMS_BEARER_PREFERENCE_t 
	**/
	MSG_SMS_GETSMSBEARERPREFERENCE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x24F,	///<Payload type {::SMS_BEARER_PREFERENCE_t}
	 /** 
	api is CAPI2_SmsApi_SetSMSBearerPreference 
	**/
	MSG_SMS_SETSMSBEARERPREFERENCE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x250,	///<Payload type {::CAPI2_SmsApi_SetSMSBearerPreference_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_SMS_SETSMSBEARERPREFERENCE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x251,
	 /** 
	api is CAPI2_SmsApi_SetSmsReadStatusChangeMode 
	**/
	MSG_SMS_SETSMSREADSTATUSCHANGEMODE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x252,	///<Payload type {::CAPI2_SmsApi_SetSmsReadStatusChangeMode_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_SMS_SETSMSREADSTATUSCHANGEMODE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x253,
	 /** 
	api is CAPI2_SmsApi_GetSmsReadStatusChangeMode 
	**/
	MSG_SMS_GETSMSREADSTATUSCHANGEMODE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x254,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SMS_GETSMSREADSTATUSCHANGEMODE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x255,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SmsApi_ChangeSmsStatusReq 
	**/
	MSG_SMS_CHANGESTATUSREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x256,	///<Payload type {::CAPI2_SmsApi_ChangeSmsStatusReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_CHANGESTATUSREQ_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x257,
	 /** 
	api is CAPI2_SmsApi_SendMEStoredStatusInd 
	**/
	MSG_SMS_SENDMESTOREDSTATUSIND_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x258,	///<Payload type {::CAPI2_SmsApi_SendMEStoredStatusInd_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SENDMESTOREDSTATUSIND_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x259,
	 /** 
	api is CAPI2_SmsApi_SendMERetrieveSmsDataInd 
	**/
	MSG_SMS_SENDMERETRIEVESMSDATAIND_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x25A,	///<Payload type {::CAPI2_SmsApi_SendMERetrieveSmsDataInd_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SENDMERETRIEVESMSDATAIND_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x25B,
	 /** 
	api is CAPI2_SmsApi_SendMERemovedStatusInd 
	**/
	MSG_SMS_SENDMEREMOVEDSTATUSIND_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x25C,	///<Payload type {::CAPI2_SmsApi_SendMERemovedStatusInd_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SENDMEREMOVEDSTATUSIND_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x25D,
	 /** 
	api is CAPI2_SmsApi_SetSmsStoredState 
	**/
	MSG_SMS_SETSMSSTOREDSTATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x25E,	///<Payload type {::CAPI2_SmsApi_SetSmsStoredState_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_SMS_SETSMSSTOREDSTATE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x25F,
	 /** 
	api is CAPI2_SmsApi_IsCachedDataReady 
	**/
	MSG_SMS_ISCACHEDDATAREADY_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x260,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SMS_ISCACHEDDATAREADY_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x261,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SmsApi_GetEnhancedVMInfoIEI 
	**/
	MSG_SMS_GETENHANCEDVMINFOIEI_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x262,
	 /** 
	payload is ::SmsEnhancedVMInd_t 
	**/
	MSG_SMS_GETENHANCEDVMINFOIEI_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x263,	///<Payload type {::SmsEnhancedVMInd_t}
	 /** 
	api is CAPI2_SmsApi_SetAllNewMsgDisplayPref 
	**/
	MSG_SMS_SETALLNEWMSGDISPLAYPREF_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x264,	///<Payload type {::CAPI2_SmsApi_SetAllNewMsgDisplayPref_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SETALLNEWMSGDISPLAYPREF_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x265,
	 /** 
	api is CAPI2_SmsApi_SendAckToNetwork 
	**/
	MSG_SMS_ACKTONETWORK_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x266,	///<Payload type {::CAPI2_SmsApi_SendAckToNetwork_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_SMS_ACKTONETWORK_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x267,
	 /** 
	api is CAPI2_SmsApi_StartMultiSmsTransferReq 
	**/
	MSG_SMS_STARTMULTISMSTRANSFER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x268,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_STARTMULTISMSTRANSFER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x269,
	 /** 
	api is CAPI2_SmsApi_StopMultiSmsTransferReq 
	**/
	MSG_SMS_STOPMULTISMSTRANSFER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x26A,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_STOPMULTISMSTRANSFER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x26B,
	 /** 
	api is CAPI2_SMS_StartCellBroadcastReq 
	**/
	MSG_SMS_START_CELL_BROADCAST_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x26C,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_START_CELL_BROADCAST_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x26D,
	 /** 
	api is CAPI2_SMS_SimInit 
	**/
	MSG_SMS_SIMINIT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x26E,
	 /** 
	payload is ::void 
	**/
	MSG_SMS_SIMINIT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x26F,
	 /** 
	api is CAPI2_SMS_SetPDAStorageOverFlowFlag 
	**/
	MSG_SMS_PDA_OVERFLOW_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x270,	///<Payload type {::CAPI2_SMS_SetPDAStorageOverFlowFlag_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_PDA_OVERFLOW_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x271,
	 /** 
	api is CAPI2_ISimApi_SendAuthenAkaReq 
	**/
	MSG_ISIM_SENDAUTHENAKAREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x272,	///<Payload type {::CAPI2_ISimApi_SendAuthenAkaReq_Req_t}
	 /** 
	api is CAPI2_ISimApi_IsIsimSupported 
	**/
	MSG_ISIM_ISISIMSUPPORTED_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x273,
	 /** 
	payload is ::Boolean 
	**/
	MSG_ISIM_ISISIMSUPPORTED_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x274,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_ISimApi_IsIsimActivated 
	**/
	MSG_ISIM_ISISIMACTIVATED_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x275,
	 /** 
	payload is ::Boolean 
	**/
	MSG_ISIM_ISISIMACTIVATED_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x276,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_ISimApi_ActivateIsimAppli 
	**/
	MSG_ISIM_ACTIVATEISIMAPPLI_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x277,
	 /** 
	api is CAPI2_ISimApi_SendAuthenHttpReq 
	**/
	MSG_ISIM_SENDAUTHENHTTPREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x278,	///<Payload type {::CAPI2_ISimApi_SendAuthenHttpReq_Req_t}
	 /** 
	api is CAPI2_ISimApi_SendAuthenGbaNafReq 
	**/
	MSG_ISIM_SENDAUTHENGBANAFREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x279,	///<Payload type {::CAPI2_ISimApi_SendAuthenGbaNafReq_Req_t}
	 /** 
	api is CAPI2_ISimApi_SendAuthenGbaBootReq 
	**/
	MSG_ISIM_SENDAUTHENGBABOOTREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x27A,	///<Payload type {::CAPI2_ISimApi_SendAuthenGbaBootReq_Req_t}
	 /** 
	api is CAPI2_PbkApi_GetAlpha 
	**/
	MSG_PBK_GETALPHA_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x27B,	///<Payload type {::CAPI2_PbkApi_GetAlpha_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_PBK_GETALPHA_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x27C,	///<Payload type {::void}
	 /** 
	api is CAPI2_PbkApi_IsEmergencyCallNumber 
	**/
	MSG_PBK_ISEMERGENCYCALLNUMBER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x27D,	///<Payload type {::CAPI2_PbkApi_IsEmergencyCallNumber_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_PBK_ISEMERGENCYCALLNUMBER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x27E,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PbkApi_IsPartialEmergencyCallNumber 
	**/
	MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x27F,	///<Payload type {::CAPI2_PbkApi_IsPartialEmergencyCallNumber_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x280,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PbkApi_SendInfoReq 
	**/
	MSG_PBK_SENDINFOREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x281,	///<Payload type {::CAPI2_PbkApi_SendInfoReq_Req_t}
	 /** 
	api is CAPI2_PbkApi_SendFindAlphaMatchMultipleReq 
	**/
	MSG_SENDFINDALPHAMATCHMULTIPLEREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x282,	///<Payload type {::CAPI2_PbkApi_SendFindAlphaMatchMultipleReq_Req_t}
	 /** 
	api is CAPI2_PbkApi_SendFindAlphaMatchOneReq 
	**/
	MSG_SENDFINDALPHAMATCHONEREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x283,	///<Payload type {::CAPI2_PbkApi_SendFindAlphaMatchOneReq_Req_t}
	 /** 
	api is CAPI2_PbkApi_IsReady 
	**/
	MSG_PBK_ISREADY_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x284,
	 /** 
	payload is ::Boolean 
	**/
	MSG_PBK_ISREADY_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x285,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PbkApi_SendReadEntryReq 
	**/
	MSG_SENDREADENTRYREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x286,	///<Payload type {::CAPI2_PbkApi_SendReadEntryReq_Req_t}
	 /** 
	api is CAPI2_PbkApi_SendWriteEntryReq 
	**/
	MSG_PBK_SENDWRITEENTRYREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x287,	///<Payload type {::CAPI2_PbkApi_SendWriteEntryReq_Req_t}
	 /** 
	api is CAPI2_PbkApi_SendUpdateEntryReq 
	**/
	MSG_PBK_SENDUPDATEENTRYREQ  = MSG_GRP_CAPI2_GEN_0 + 0x288,	///<Payload type {::CAPI2_PbkApi_SendUpdateEntryReq_Req_t}
	 /** 
	api is CAPI2_PbkApi_SendIsNumDiallableReq 
	**/
	MSG_PBK_SENDISNUMDIALLABLEREQ  = MSG_GRP_CAPI2_GEN_0 + 0x289,	///<Payload type {::CAPI2_PbkApi_SendIsNumDiallableReq_Req_t}
	 /** 
	api is CAPI2_PbkApi_IsNumDiallable 
	**/
	MSG_PBK_ISNUMDIALLABLE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x28A,	///<Payload type {::CAPI2_PbkApi_IsNumDiallable_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_PBK_ISNUMDIALLABLE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x28B,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PbkApi_IsNumBarred 
	**/
	MSG_PBK_ISNUMBARRED_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x28C,	///<Payload type {::CAPI2_PbkApi_IsNumBarred_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_PBK_ISNUMBARRED_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x28D,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PbkApi_IsUssdDiallable 
	**/
	MSG_PBK_ISUSSDDIALLABLE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x28E,	///<Payload type {::CAPI2_PbkApi_IsUssdDiallable_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_PBK_ISUSSDDIALLABLE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x28F,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PdpApi_SetPDPContext 
	**/
	MSG_PDP_SETPDPCONTEXT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x290,	///<Payload type {::CAPI2_PdpApi_SetPDPContext_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_SETPDPCONTEXT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x291,
	 /** 
	api is CAPI2_PdpApi_SetSecPDPContext 
	**/
	MSG_PDP_SETSECPDPCONTEXT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x292,	///<Payload type {::CAPI2_PdpApi_SetSecPDPContext_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_SETSECPDPCONTEXT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x293,
	 /** 
	api is CAPI2_PdpApi_GetGPRSQoS 
	**/
	MSG_PDP_GETGPRSQOS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x294,	///<Payload type {::CAPI2_PdpApi_GetGPRSQoS_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_GETGPRSQOS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x295,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_PdpApi_SetGPRSQoS 
	**/
	MSG_PDP_SETGPRSQOS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x296,	///<Payload type {::CAPI2_PdpApi_SetGPRSQoS_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_SETGPRSQOS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x297,
	 /** 
	api is CAPI2_PdpApi_GetGPRSMinQoS 
	**/
	MSG_PDP_GETGPRSMINQOS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x298,	///<Payload type {::CAPI2_PdpApi_GetGPRSMinQoS_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_GETGPRSMINQOS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x299,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_PdpApi_SetGPRSMinQoS 
	**/
	MSG_PDP_SETGPRSMINQOS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x29A,	///<Payload type {::CAPI2_PdpApi_SetGPRSMinQoS_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_SETGPRSMINQOS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x29B,
	 /** 
	api is CAPI2_NetRegApi_SendCombinedAttachReq 
	**/
	MSG_MS_SENDCOMBINEDATTACHREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x29C,	///<Payload type {::CAPI2_NetRegApi_SendCombinedAttachReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_MS_SENDCOMBINEDATTACHREQ_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x29D,
	 /** 
	api is CAPI2_NetRegApi_SendDetachReq 
	**/
	MSG_MS_SENDDETACHREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x29E,	///<Payload type {::CAPI2_NetRegApi_SendDetachReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_MS_SENDDETACHREQ_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x29F,
	 /** 
	api is CAPI2_MS_GetGPRSAttachStatus 
	**/
	MSG_MS_GETGPRSATTACHSTATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2A0,
	 /** 
	payload is ::AttachState_t 
	**/
	MSG_MS_GETGPRSATTACHSTATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2A1,	///<Payload type {::AttachState_t}
	 /** 
	api is CAPI2_PdpApi_IsSecondaryPdpDefined 
	**/
	MSG_PDP_ISSECONDARYPDPDEFINED_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2A2,	///<Payload type {::CAPI2_PdpApi_IsSecondaryPdpDefined_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_PDP_ISSECONDARYPDPDEFINED_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2A3,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PchExApi_SendPDPActivateReq 
	**/
	MSG_PCHEX_SENDPDPACTIVATEREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2A4,	///<Payload type {::CAPI2_PchExApi_SendPDPActivateReq_Req_t}
	 /** 
	api is CAPI2_PchExApi_SendPDPModifyReq 
	**/
	MSG_PCHEX_SENDPDPMODIFYREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2A5,	///<Payload type {::CAPI2_PchExApi_SendPDPModifyReq_Req_t}
	 /** 
	api is CAPI2_PchExApi_SendPDPDeactivateReq 
	**/
	MSG_PCHEX_SENDPDPDEACTIVATEREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2A6,	///<Payload type {::CAPI2_PchExApi_SendPDPDeactivateReq_Req_t}
	 /** 
	api is CAPI2_PchExApi_SendPDPActivateSecReq 
	**/
	MSG_PCHEX_SENDPDPACTIVATESECREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2A7,	///<Payload type {::CAPI2_PchExApi_SendPDPActivateSecReq_Req_t}
	 /** 
	api is CAPI2_PdpApi_GetGPRSActivateStatus 
	**/
	MSG_PDP_GETGPRSACTIVATESTATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2A8,
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_GETGPRSACTIVATESTATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2A9,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_NetRegApi_SetMSClass 
	**/
	MSG_PDP_SETMSCLASS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2AA,	///<Payload type {::CAPI2_NetRegApi_SetMSClass_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_SETMSCLASS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2AB,
	 /** 
	api is CAPI2_PDP_GetMSClass 
	**/
	MSG_PDP_GETMSCLASS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2AC,
	 /** 
	payload is ::MSClass_t 
	**/
	MSG_PDP_GETMSCLASS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2AD,	///<Payload type {::MSClass_t}
	 /** 
	api is CAPI2_PdpApi_GetUMTSTft 
	**/
	MSG_PDP_GETUMTSTFT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2AE,	///<Payload type {::CAPI2_PdpApi_GetUMTSTft_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_GETUMTSTFT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2AF,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_PdpApi_SetUMTSTft 
	**/
	MSG_PDP_SETUMTSTFT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2B0,	///<Payload type {::CAPI2_PdpApi_SetUMTSTft_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_SETUMTSTFT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2B1,
	 /** 
	api is CAPI2_PdpApi_DeleteUMTSTft 
	**/
	MSG_PDP_DELETEUMTSTFT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2B2,	///<Payload type {::CAPI2_PdpApi_DeleteUMTSTft_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_DELETEUMTSTFT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2B3,
	 /** 
	api is CAPI2_PdpApi_DeactivateSNDCPConnection 
	**/
	MSG_PDP_DEACTIVATESNDCPCONNECTION_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2B4,	///<Payload type {::CAPI2_PdpApi_DeactivateSNDCPConnection_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_DEACTIVATESNDCPCONNECTION_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2B5,
	 /** 
	api is CAPI2_PdpApi_GetR99UMTSMinQoS 
	**/
	MSG_PDP_GETR99UMTSMINQOS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2B6,	///<Payload type {::CAPI2_PdpApi_GetR99UMTSMinQoS_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_GETR99UMTSMINQOS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2B7,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_PdpApi_GetR99UMTSQoS 
	**/
	MSG_PDP_GETR99UMTSQOS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2B8,	///<Payload type {::CAPI2_PdpApi_GetR99UMTSQoS_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_GETR99UMTSQOS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2B9,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_PdpApi_GetUMTSMinQoS 
	**/
	MSG_PDP_GETUMTSMINQOS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2BA,	///<Payload type {::CAPI2_PdpApi_GetUMTSMinQoS_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_GETUMTSMINQOS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2BB,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_PdpApi_GetUMTSQoS 
	**/
	MSG_PDP_GETUMTSQOS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2BC,	///<Payload type {::CAPI2_PdpApi_GetUMTSQoS_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_GETUMTSQOS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2BD,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_PdpApi_GetNegQoS 
	**/
	MSG_PDP_GETNEGQOS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2BE,	///<Payload type {::CAPI2_PdpApi_GetNegQoS_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_GETNEGQOS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2BF,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_PdpApi_SetR99UMTSMinQoS 
	**/
	MSG_PDP_SETR99UMTSMINQOS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2C0,	///<Payload type {::CAPI2_PdpApi_SetR99UMTSMinQoS_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_SETR99UMTSMINQOS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2C1,
	 /** 
	api is CAPI2_PdpApi_SetR99UMTSQoS 
	**/
	MSG_PDP_SETR99UMTSQOS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2C2,	///<Payload type {::CAPI2_PdpApi_SetR99UMTSQoS_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_SETR99UMTSQOS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2C3,
	 /** 
	api is CAPI2_PdpApi_SetUMTSMinQoS 
	**/
	MSG_PDP_SETUMTSMINQOS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2C4,	///<Payload type {::CAPI2_PdpApi_SetUMTSMinQoS_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_SETUMTSMINQOS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2C5,
	 /** 
	api is CAPI2_PdpApi_SetUMTSQoS 
	**/
	MSG_PDP_SETUMTSQOS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2C6,	///<Payload type {::CAPI2_PdpApi_SetUMTSQoS_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_SETUMTSQOS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2C7,
	 /** 
	api is CAPI2_PdpApi_GetNegotiatedParms 
	**/
	MSG_PDP_GETNEGOTIATEDPARMS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2C8,	///<Payload type {::CAPI2_PdpApi_GetNegotiatedParms_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_GETNEGOTIATEDPARMS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2C9,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_MS_IsGprsCallActive 
	**/
	MSG_MS_ISGPRSCALLACTIVE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2CA,	///<Payload type {::CAPI2_MS_IsGprsCallActive_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_MS_ISGPRSCALLACTIVE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2CB,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_MS_SetChanGprsCallActive 
	**/
	MSG_MS_SETCHANGPRSCALLACTIVE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2CC,	///<Payload type {::CAPI2_MS_SetChanGprsCallActive_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_MS_SETCHANGPRSCALLACTIVE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2CD,
	 /** 
	api is CAPI2_MS_SetCidForGprsActiveChan 
	**/
	MSG_MS_SETCIDFORGPRSACTIVECHAN_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2CE,	///<Payload type {::CAPI2_MS_SetCidForGprsActiveChan_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_MS_SETCIDFORGPRSACTIVECHAN_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2CF,
	 /** 
	api is CAPI2_PdpApi_GetPPPModemCid 
	**/
	MSG_PDP_GETPPPMODEMCID_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2D0,
	 /** 
	payload is ::PCHCid_t 
	**/
	MSG_PDP_GETPPPMODEMCID_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2D1,	///<Payload type {::PCHCid_t}
	 /** 
	api is CAPI2_MS_GetGprsActiveChanFromCid 
	**/
	MSG_MS_GETGPRSACTIVECHANFROMCID_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2D2,	///<Payload type {::CAPI2_MS_GetGprsActiveChanFromCid_Req_t}
	 /** 
	payload is ::UInt8 
	**/
	MSG_MS_GETGPRSACTIVECHANFROMCID_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2D3,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_MS_GetCidFromGprsActiveChan 
	**/
	MSG_MS_GETCIDFROMGPRSACTIVECHAN_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2D4,	///<Payload type {::CAPI2_MS_GetCidFromGprsActiveChan_Req_t}
	 /** 
	payload is ::UInt8 
	**/
	MSG_MS_GETCIDFROMGPRSACTIVECHAN_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2D5,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_PdpApi_GetPDPAddress 
	**/
	MSG_PDP_GETPDPADDRESS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2D6,	///<Payload type {::CAPI2_PdpApi_GetPDPAddress_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_GETPDPADDRESS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2D7,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_PdpApi_SendTBFData 
	**/
	MSG_PDP_SENDTBFDATA_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2D8,	///<Payload type {::CAPI2_PdpApi_SendTBFData_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_SENDTBFDATA_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2D9,
	 /** 
	api is CAPI2_PdpApi_TftAddFilter 
	**/
	MSG_PDP_TFTADDFILTER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2DA,	///<Payload type {::CAPI2_PdpApi_TftAddFilter_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_TFTADDFILTER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2DB,
	 /** 
	api is CAPI2_PdpApi_SetPCHContextState 
	**/
	MSG_PDP_SETPCHCONTEXTSTATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2DC,	///<Payload type {::CAPI2_PdpApi_SetPCHContextState_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_SETPCHCONTEXTSTATE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2DD,
	 /** 
	api is CAPI2_PdpApi_SetDefaultPDPContext 
	**/
	MSG_PDP_SETDEFAULTPDPCONTEXT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2DE,	///<Payload type {::CAPI2_PdpApi_SetDefaultPDPContext_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_SETDEFAULTPDPCONTEXT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2DF,
	 /** 
	api is CAPI2_PCHEx_GetDecodedProtConfig 
	**/
	MSG_PCHEX_READDECODEDPROTCONFIG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2E0,	///<Payload type {::CAPI2_PCHEx_GetDecodedProtConfig_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PCHEX_READDECODEDPROTCONFIG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2E1,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_PchExApi_BuildIpConfigOptions 
	**/
	MSG_PCHEX_BUILDPROTCONFIGOPTIONS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2E2,	///<Payload type {::CAPI2_PchExApi_BuildIpConfigOptions_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_PCHEX_BUILDPROTCONFIGOPTIONS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2E3,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PCHEx_BuildIpConfigOptions2 
	**/
	MSG_PCHEX_BUILDPROTCONFIGOPTIONS2_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2E4,	///<Payload type {::CAPI2_PCHEx_BuildIpConfigOptions2_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_PCHEX_BUILDPROTCONFIGOPTIONS2_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2E5,	///<Payload type {::void}
	 /** 
	api is CAPI2_PchExApi_BuildIpConfigOptionsWithChapAuthType 
	**/
	MSG_PCHEX_BUILDPROTCONFIGOPTION_CHAP_TYPE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2E6,	///<Payload type {::CAPI2_PchExApi_BuildIpConfigOptionsWithChapAuthType_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_PCHEX_BUILDPROTCONFIGOPTION_CHAP_TYPE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2E7,	///<Payload type {::void}
	 /** 
	api is CAPI2_PdpApi_GetDefaultQos 
	**/
	MSG_PDP_GET_DEFAULT_QOS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2E8,
	 /** 
	payload is ::void 
	**/
	MSG_PDP_GET_DEFAULT_QOS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2E9,	///<Payload type {::void}
	 /** 
	api is CAPI2_PdpApi_IsPDPContextActive 
	**/
	MSG_PDP_ISCONTEXT_ACTIVE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2EA,	///<Payload type {::CAPI2_PdpApi_IsPDPContextActive_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_PDP_ISCONTEXT_ACTIVE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2EB,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PdpApi_ActivateSNDCPConnection 
	**/
	MSG_PDP_ACTIVATESNDCPCONNECTION_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2EC,	///<Payload type {::CAPI2_PdpApi_ActivateSNDCPConnection_Req_t}
	 /** 
	api is CAPI2_PdpApi_GetPDPContext 
	**/
	MSG_PDP_GETPDPCONTEXT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2ED,	///<Payload type {::CAPI2_PdpApi_GetPDPContext_Req_t}
	 /** 
	payload is ::PDPDefaultContext_t 
	**/
	MSG_PDP_GETPDPCONTEXT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2EE,	///<Payload type {::PDPDefaultContext_t}
	 /** 
	api is CAPI2_PdpApi_GetDefinedPDPContextCidList 
	**/
	MSG_PDP_GETPDPCONTEXT_CID_LIST_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2EF,
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_GETPDPCONTEXT_CID_LIST_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2F0,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_UTIL_ExtractImei 
	**/
	MSG_UTIL_GET_IMEI_STR_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2F1,
	 /** 
	payload is ::void 
	**/
	MSG_UTIL_GET_IMEI_STR_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2F2,	///<Payload type {::void}
	 /** 
	api is CAPI2_SYS_GetBootLoaderVersion 
	**/
	MSG_SYSPARAM_BOOTLOADER_VER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2F3,	///<Payload type {::CAPI2_SYS_GetBootLoaderVersion_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_SYSPARAM_BOOTLOADER_VER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2F4,	///<Payload type {::void}
	 /** 
	api is CAPI2_SYS_GetDSFVersion 
	**/
	MSG_SYSPARAM_DSF_VER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2F5,	///<Payload type {::CAPI2_SYS_GetDSFVersion_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_SYSPARAM_DSF_VER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2F6,	///<Payload type {::void}
	 /** 
	api is CAPI2_USimApi_GetUst 
	**/
	MSG_USIM_UST_DATA_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2F7,
	 /** 
	payload is ::UInt8 
	**/
	MSG_USIM_UST_DATA_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2F8,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_PATCH_GetRevision 
	**/
	MSG_PATCH_GET_REVISION_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2F9,
	 /** 
	payload is ::UInt8 
	**/
	MSG_PATCH_GET_REVISION_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2FA,	///<Payload type {::UInt8}

	MSG_GRP_CAPI2_SS_BEGIN = MSG_GRP_CAPI2_SS + 0,
	 /** 
	api is CAPI2_SS_SendCallForwardReq 
	**/
	MSG_SS_SENDCALLFORWARDREQ_REQ  = MSG_GRP_CAPI2_SS + 0x0,	///<Payload type {::CAPI2_SS_SendCallForwardReq_Req_t}
	 /** 
	api is CAPI2_SS_QueryCallForwardStatus 
	**/
	MSG_SS_QUERYCALLFORWARDSTATUS_REQ  = MSG_GRP_CAPI2_SS + 0x1,	///<Payload type {::CAPI2_SS_QueryCallForwardStatus_Req_t}
	 /** 
	api is CAPI2_SS_SendCallBarringReq 
	**/
	MSG_SS_SENDCALLBARRINGREQ_REQ  = MSG_GRP_CAPI2_SS + 0x2,	///<Payload type {::CAPI2_SS_SendCallBarringReq_Req_t}
	 /** 
	api is CAPI2_SS_QueryCallBarringStatus 
	**/
	MSG_SS_QUERYCALLBARRINGSTATUS_REQ  = MSG_GRP_CAPI2_SS + 0x3,	///<Payload type {::CAPI2_SS_QueryCallBarringStatus_Req_t}
	 /** 
	api is CAPI2_SS_SendCallBarringPWDChangeReq 
	**/
	MSG_SS_SENDCALLBARRINGPWDCHANGEREQ_REQ  = MSG_GRP_CAPI2_SS + 0x4,	///<Payload type {::CAPI2_SS_SendCallBarringPWDChangeReq_Req_t}
	 /** 
	api is CAPI2_SS_SendCallWaitingReq 
	**/
	MSG_SS_SENDCALLWAITINGREQ_REQ  = MSG_GRP_CAPI2_SS + 0x5,	///<Payload type {::CAPI2_SS_SendCallWaitingReq_Req_t}
	 /** 
	api is CAPI2_SS_QueryCallWaitingStatus 
	**/
	MSG_SS_QUERYCALLWAITINGSTATUS_REQ  = MSG_GRP_CAPI2_SS + 0x6,	///<Payload type {::CAPI2_SS_QueryCallWaitingStatus_Req_t}
	 /** 
	api is CAPI2_SS_QueryCallingLineIDStatus 
	**/
	MSG_SS_QUERYCALLINGLINEIDSTATUS_REQ  = MSG_GRP_CAPI2_SS + 0x7,
	 /** 
	api is CAPI2_SS_QueryConnectedLineIDStatus 
	**/
	MSG_SS_QUERYCONNECTEDLINEIDSTATUS_REQ  = MSG_GRP_CAPI2_SS + 0x8,
	 /** 
	api is CAPI2_SS_QueryCallingLineRestrictionStatus 
	**/
	MSG_SS_QUERYCALLINGLINERESTRICTIONSTATUS_REQ  = MSG_GRP_CAPI2_SS + 0x9,
	 /** 
	api is CAPI2_SS_QueryConnectedLineRestrictionStatus 
	**/
	MSG_SS_QUERYCONNECTEDLINERESTRICTIONSTATUS_REQ  = MSG_GRP_CAPI2_SS + 0xA,
	 /** 
	api is CAPI2_SS_QueryCallingNAmePresentStatus 
	**/
	MSG_SS_QUERYCALLINGNAMEPRESENTSTATUS_REQ  = MSG_GRP_CAPI2_SS + 0xB,
	 /** 
	api is CAPI2_SS_SetCallingLineIDStatus 
	**/
	MSG_SS_SETCALLINGLINEIDSTATUS_REQ  = MSG_GRP_CAPI2_SS + 0xC,	///<Payload type {::CAPI2_SS_SetCallingLineIDStatus_Req_t}
	 /** 
	api is CAPI2_SS_SetCallingLineRestrictionStatus 
	**/
	MSG_SS_SETCALLINGLINERESTRICTIONSTATUS_REQ  = MSG_GRP_CAPI2_SS + 0xD,	///<Payload type {::CAPI2_SS_SetCallingLineRestrictionStatus_Req_t}
	 /** 
	api is CAPI2_SS_SetConnectedLineIDStatus 
	**/
	MSG_SS_SETCONNECTEDLINEIDSTATUS_REQ  = MSG_GRP_CAPI2_SS + 0xE,	///<Payload type {::CAPI2_SS_SetConnectedLineIDStatus_Req_t}
	 /** 
	api is CAPI2_SS_SetConnectedLineRestrictionStatus 
	**/
	MSG_SS_SETCONNECTEDLINERESTRICTIONSTATUS_REQ  = MSG_GRP_CAPI2_SS + 0xF,	///<Payload type {::CAPI2_SS_SetConnectedLineRestrictionStatus_Req_t}
	 /** 
	api is CAPI2_SS_SendUSSDConnectReq 
	**/
	MSG_SS_SENDUSSDCONNECTREQ_REQ  = MSG_GRP_CAPI2_SS + 0x10,	///<Payload type {::CAPI2_SS_SendUSSDConnectReq_Req_t}
	 /** 
	api is CAPI2_SS_SendUSSDData 
	**/
	MSG_SS_SENDUSSDDATA_REQ  = MSG_GRP_CAPI2_SS + 0x11,	///<Payload type {::CAPI2_SS_SendUSSDData_Req_t}
	 /** 
	api is CAPI2_SsApi_DialStrSrvReq 
	**/
	MSG_SSAPI_DIALSTRSRVREQ_REQ  = MSG_GRP_CAPI2_SS + 0x12,	///<Payload type {::CAPI2_SsApi_DialStrSrvReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SSAPI_DIALSTRSRVREQ_RSP  = MSG_GRP_CAPI2_SS + 0x13,
	 /** 
	api is CAPI2_SS_EndUSSDConnectReq 
	**/
	MSG_SS_ENDUSSDCONNECTREQ_REQ  = MSG_GRP_CAPI2_SS + 0x14,	///<Payload type {::CAPI2_SS_EndUSSDConnectReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SS_ENDUSSDCONNECTREQ_RSP  = MSG_GRP_CAPI2_SS + 0x15,
	 /** 
	api is CAPI2_SsApi_SsSrvReq 
	**/
	MSG_SSAPI_SSSRVREQ_REQ  = MSG_GRP_CAPI2_SS + 0x16,	///<Payload type {::CAPI2_SsApi_SsSrvReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SSAPI_SSSRVREQ_RSP  = MSG_GRP_CAPI2_SS + 0x17,
	 /** 
	api is CAPI2_SsApi_UssdSrvReq 
	**/
	MSG_SSAPI_USSDSRVREQ_REQ  = MSG_GRP_CAPI2_SS + 0x18,	///<Payload type {::CAPI2_SsApi_UssdSrvReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SSAPI_USSDSRVREQ_RSP  = MSG_GRP_CAPI2_SS + 0x19,
	 /** 
	api is CAPI2_SsApi_UssdDataReq 
	**/
	MSG_SSAPI_USSDDATAREQ_REQ  = MSG_GRP_CAPI2_SS + 0x1A,	///<Payload type {::CAPI2_SsApi_UssdDataReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SSAPI_USSDDATAREQ_RSP  = MSG_GRP_CAPI2_SS + 0x1B,
	 /** 
	api is CAPI2_SsApi_SsReleaseReq 
	**/
	MSG_SSAPI_SSRELEASEREQ_REQ  = MSG_GRP_CAPI2_SS + 0x1C,	///<Payload type {::CAPI2_SsApi_SsReleaseReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SSAPI_SSRELEASEREQ_RSP  = MSG_GRP_CAPI2_SS + 0x1D,
	 /** 
	api is CAPI2_SsApi_DataReq 
	**/
	MSG_SSAPI_DATAREQ_REQ  = MSG_GRP_CAPI2_SS + 0x1E,	///<Payload type {::CAPI2_SsApi_DataReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SSAPI_DATAREQ_RSP  = MSG_GRP_CAPI2_SS + 0x1F,
	 /** 
	api is CAPI2_SS_SsApiReqDispatcher 
	**/
	MSG_SSAPI_DISPATCH_REQ  = MSG_GRP_CAPI2_SS + 0x20,	///<Payload type {::CAPI2_SS_SsApiReqDispatcher_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SSAPI_DISPATCH_RSP  = MSG_GRP_CAPI2_SS + 0x21,
	 /** 
	api is CAPI2_SS_GetStr 
	**/
	MSG_SS_GET_STR_REQ  = MSG_GRP_CAPI2_SS + 0x22,	///<Payload type {::CAPI2_SS_GetStr_Req_t}
	 /** 
	payload is ::UInt8 
	**/
	MSG_SS_GET_STR_RSP  = MSG_GRP_CAPI2_SS + 0x23,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_SS_SetClientID 
	**/
	MSG_SS_SETCLIENTID_REQ  = MSG_GRP_CAPI2_SS + 0x24,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SS_SETCLIENTID_RSP  = MSG_GRP_CAPI2_SS + 0x25,
	 /** 
	api is CAPI2_SS_GetClientID 
	**/
	MSG_SS_GETCLIENTID_REQ  = MSG_GRP_CAPI2_SS + 0x26,
	 /** 
	payload is ::UInt8 
	**/
	MSG_SS_GETCLIENTID_RSP  = MSG_GRP_CAPI2_SS + 0x27,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_SS_ResetClientID 
	**/
	MSG_SS_RESETCLIENTID_REQ  = MSG_GRP_CAPI2_SS + 0x28,
	 /** 
	payload is ::void 
	**/
	MSG_SS_RESETCLIENTID_RSP  = MSG_GRP_CAPI2_SS + 0x29,

	MSG_GRP_CAPI2_SS_END = MSG_GRP_CAPI2_SS + 0x2A,

	 /** 
	api is CAPI2_SatkApi_GetCachedRootMenuPtr 
	**/
	MSG_SATK_GETCACHEDROOTMENUPTR_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2FB,
	 /** 
	payload is ::SetupMenu_t 
	**/
	MSG_SATK_GETCACHEDROOTMENUPTR_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2FC,	///<Payload type {::SetupMenu_t}
	 /** 
	api is CAPI2_SatkApi_SendUserActivityEvent 
	**/
	MSG_SATK_SENDUSERACTIVITYEVENT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2FD,
	 /** 
	payload is ::void 
	**/
	MSG_SATK_SENDUSERACTIVITYEVENT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2FE,
	 /** 
	api is CAPI2_SatkApi_SendIdleScreenAvaiEvent 
	**/
	MSG_SATK_SENDIDLESCREENAVAIEVENT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2FF,
	 /** 
	payload is ::void 
	**/
	MSG_SATK_SENDIDLESCREENAVAIEVENT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x300,
	 /** 
	api is CAPI2_SatkApi_SendLangSelectEvent 
	**/
	MSG_SATK_SENDLANGSELECTEVENT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x301,	///<Payload type {::CAPI2_SatkApi_SendLangSelectEvent_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_SATK_SENDLANGSELECTEVENT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x302,
	 /** 
	api is CAPI2_SatkApi_SendBrowserTermEvent 
	**/
	MSG_SATK_SENDBROWSERTERMEVENT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x303,	///<Payload type {::CAPI2_SatkApi_SendBrowserTermEvent_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_SATK_SENDBROWSERTERMEVENT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x304,
	 /** 
	api is CAPI2_SatkApi_CmdResp 
	**/
	MSG_SATK_CMDRESP_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x305,	///<Payload type {::CAPI2_SatkApi_CmdResp_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SATK_CMDRESP_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x306,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SatkApi_DataServCmdResp 
	**/
	MSG_SATK_DATASERVCMDRESP_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x307,	///<Payload type {::CAPI2_SatkApi_DataServCmdResp_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SATK_DATASERVCMDRESP_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x308,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SatkApi_SendDataServReq 
	**/
	MSG_SATK_SENDDATASERVREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x309,	///<Payload type {::CAPI2_SatkApi_SendDataServReq_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SATK_SENDDATASERVREQ_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x30A,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SatkApi_SendTerminalRsp 
	**/
	MSG_SATK_SENDTERMINALRSP_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x30B,	///<Payload type {::CAPI2_SatkApi_SendTerminalRsp_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SATK_SENDTERMINALRSP_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x30C,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SatkApi_SetTermProfile 
	**/
	MSG_SATK_SETTERMPROFILE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x30D,	///<Payload type {::CAPI2_SatkApi_SetTermProfile_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_SATK_SETTERMPROFILE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x30E,
	 /** 
	api is CAPI2_SatkApi_SendEnvelopeCmdReq 
	**/
	MSG_SATK_SEND_ENVELOPE_CMD_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x30F,	///<Payload type {::CAPI2_SatkApi_SendEnvelopeCmdReq_Req_t}
	 /** 
	api is CAPI2_SatkApi_SendTerminalRspReq 
	**/
	MSG_STK_TERMINAL_RESPONSE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x310,	///<Payload type {::CAPI2_SatkApi_SendTerminalRspReq_Req_t}
	 /** 
	api is CAPI2_SATK_SendBrowsingStatusEvent 
	**/
	MSG_STK_SEND_BROWSING_STATUS_EVT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x311,	///<Payload type {::CAPI2_SATK_SendBrowsingStatusEvent_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_STK_SEND_BROWSING_STATUS_EVT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x312,
	 /** 
	api is CAPI2_SatkApi_SendCcSetupReq 
	**/
	MSG_SATK_SEND_CC_SETUP_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x313,	///<Payload type {::CAPI2_SatkApi_SendCcSetupReq_Req_t}
	 /** 
	api is CAPI2_SatkApi_SendCcSsReq 
	**/
	MSG_SATK_SEND_CC_SS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x314,	///<Payload type {::CAPI2_SatkApi_SendCcSsReq_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_SATK_SEND_CC_SS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x315,
	 /** 
	api is CAPI2_SatkApi_SendCcUssdReq 
	**/
	MSG_SATK_SEND_CC_USSD_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x316,	///<Payload type {::CAPI2_SatkApi_SendCcUssdReq_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_SATK_SEND_CC_USSD_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x317,
	 /** 
	api is CAPI2_SatkApi_SendCcSmsReq 
	**/
	MSG_SATK_SEND_CC_SMS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x318,	///<Payload type {::CAPI2_SatkApi_SendCcSmsReq_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_SATK_SEND_CC_SMS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x319,
	 /** 
	api is CAPI2_LCS_CpMoLrReq 
	**/
	MSG_LCS_CPMOLRREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x31A,	///<Payload type {::CAPI2_LCS_CpMoLrReq_Req_t}
	 /** 
	api is CAPI2_LCS_CpMoLrAbort 
	**/
	MSG_LCS_CPMOLRABORT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x31B,
	 /** 
	payload is ::Result_t 
	**/
	MSG_LCS_CPMOLRABORT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x31C,
	 /** 
	api is CAPI2_LCS_CpMtLrVerificationRsp 
	**/
	MSG_LCS_CPMTLRVERIFICATIONRSP_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x31D,	///<Payload type {::CAPI2_LCS_CpMtLrVerificationRsp_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_LCS_CPMTLRVERIFICATIONRSP_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x31E,
	 /** 
	api is CAPI2_LCS_CpMtLrRsp 
	**/
	MSG_LCS_CPMTLRRSP_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x31F,	///<Payload type {::CAPI2_LCS_CpMtLrRsp_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_LCS_CPMTLRRSP_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x320,
	 /** 
	api is CAPI2_LCS_CpLocUpdateRsp 
	**/
	MSG_LCS_CPLOCUPDATERSP_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x321,	///<Payload type {::CAPI2_LCS_CpLocUpdateRsp_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_LCS_CPLOCUPDATERSP_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x322,
	 /** 
	api is CAPI2_LCS_DecodePosEstimate 
	**/
	MSG_LCS_DECODEPOSESTIMATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x323,	///<Payload type {::CAPI2_LCS_DecodePosEstimate_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_LCS_DECODEPOSESTIMATE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x324,	///<Payload type {::void}
	 /** 
	api is CAPI2_LCS_EncodeAssistanceReq 
	**/
	MSG_LCS_ENCODEASSISTANCEREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x325,	///<Payload type {::CAPI2_LCS_EncodeAssistanceReq_Req_t}
	 /** 
	payload is ::int 
	**/
	MSG_LCS_ENCODEASSISTANCEREQ_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x326,	///<Payload type {::int}
	 /** 
	api is CAPI2_LCS_SendRrlpDataToNetwork 
	**/
	MSG_LCS_SEND_RRLP_DATA_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x327,	///<Payload type {::CAPI2_LCS_SendRrlpDataToNetwork_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_LCS_SEND_RRLP_DATA_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x328,
	 /** 
	api is CAPI2_LCS_RrcMeasurementReport 
	**/
	MSG_LCS_RRC_MEAS_REPORT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x329,	///<Payload type {::CAPI2_LCS_RrcMeasurementReport_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_LCS_RRC_MEAS_REPORT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x32A,
	 /** 
	api is CAPI2_LCS_RrcMeasurementControlFailure 
	**/
	MSG_LCS_RRC_MEAS_FAILURE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x32B,	///<Payload type {::CAPI2_LCS_RrcMeasurementControlFailure_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_LCS_RRC_MEAS_FAILURE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x32C,
	 /** 
	api is CAPI2_LCS_RrcStatus 
	**/
	MSG_LCS_RRC_STATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x32D,	///<Payload type {::CAPI2_LCS_RrcStatus_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_LCS_RRC_STATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x32E,
	 /** 
	api is CAPI2_LCS_FttSyncReq 
	**/
	MSG_LCS_FTT_SYNC_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x32F,
	 /** 
	payload is ::Result_t 
	**/
	MSG_LCS_FTT_SYNC_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x330,
	 /** 
	api is CAPI2_CcApi_MakeVoiceCall 
	**/
	MSG_CC_MAKEVOICECALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x331,	///<Payload type {::CAPI2_CcApi_MakeVoiceCall_Req_t}
	 /** 
	api is CAPI2_CcApi_MakeDataCall 
	**/
	MSG_CC_MAKEDATACALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x332,	///<Payload type {::CAPI2_CcApi_MakeDataCall_Req_t}
	 /** 
	api is CAPI2_CcApi_MakeFaxCall 
	**/
	MSG_CC_MAKEFAXCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x333,	///<Payload type {::CAPI2_CcApi_MakeFaxCall_Req_t}
	 /** 
	api is CAPI2_CcApi_MakeVideoCall 
	**/
	MSG_CC_MAKEVIDEOCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x334,	///<Payload type {::CAPI2_CcApi_MakeVideoCall_Req_t}
	 /** 
	api is CAPI2_CcApi_EndCall 
	**/
	MSG_CC_ENDCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x335,	///<Payload type {::CAPI2_CcApi_EndCall_Req_t}
	 /** 
	api is CAPI2_CcApi_EndAllCalls 
	**/
	MSG_CC_ENDALLCALLS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x336,
	 /** 
	api is CAPI2_CcApi_EndCallImmediate 
	**/
	MSG_CCAPI_ENDCALL_IMMEDIATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x337,	///<Payload type {::CAPI2_CcApi_EndCallImmediate_Req_t}
	 /** 
	api is CAPI2_CcApi_EndAllCallsImmediate 
	**/
	MSG_CCAPI_ENDALLCALLS_IMMEDIATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x338,
	 /** 
	api is CAPI2_CcApi_EndMPTYCalls 
	**/
	MSG_CC_ENDMPTYCALLS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x339,
	 /** 
	api is CAPI2_CcApi_EndHeldCall 
	**/
	MSG_CC_ENDHELDCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x33A,
	 /** 
	api is CAPI2_CcApi_AcceptVoiceCall 
	**/
	MSG_CC_ACCEPTVOICECALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x33B,	///<Payload type {::CAPI2_CcApi_AcceptVoiceCall_Req_t}
	 /** 
	api is CAPI2_CcApi_AcceptDataCall 
	**/
	MSG_CC_ACCEPTDATACALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x33C,	///<Payload type {::CAPI2_CcApi_AcceptDataCall_Req_t}
	 /** 
	api is CAPI2_CcApi_AcceptWaitingCall 
	**/
	MSG_CC_ACCEPTWAITINGCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x33D,
	 /** 
	api is CAPI2_CcApi_AcceptVideoCall 
	**/
	MSG_CC_ACCEPTVIDEOCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x33E,	///<Payload type {::CAPI2_CcApi_AcceptVideoCall_Req_t}
	 /** 
	api is CAPI2_CcApi_HoldCurrentCall 
	**/
	MSG_CC_HOLDCURRENTCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x33F,
	 /** 
	api is CAPI2_CcApi_HoldCall 
	**/
	MSG_CC_HOLDCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x340,	///<Payload type {::CAPI2_CcApi_HoldCall_Req_t}
	 /** 
	api is CAPI2_CcApi_RetrieveNextHeldCall 
	**/
	MSG_CC_RETRIEVENEXTHELDCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x341,
	 /** 
	api is CAPI2_CcApi_RetrieveCall 
	**/
	MSG_CC_RETRIEVECALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x342,	///<Payload type {::CAPI2_CcApi_RetrieveCall_Req_t}
	 /** 
	api is CAPI2_CcApi_SwapCall 
	**/
	MSG_CC_SWAPCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x343,	///<Payload type {::CAPI2_CcApi_SwapCall_Req_t}
	 /** 
	api is CAPI2_CcApi_SplitCall 
	**/
	MSG_CC_SPLITCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x344,	///<Payload type {::CAPI2_CcApi_SplitCall_Req_t}
	 /** 
	api is CAPI2_CcApi_JoinCall 
	**/
	MSG_CC_JOINCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x345,	///<Payload type {::CAPI2_CcApi_JoinCall_Req_t}
	 /** 
	api is CAPI2_CcApi_TransferCall 
	**/
	MSG_CC_TRANSFERCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x346,	///<Payload type {::CAPI2_CcApi_TransferCall_Req_t}
	 /** 
	api is CAPI2_CcApi_GetCNAPName 
	**/
	MSG_CC_GETCNAPNAME_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x347,	///<Payload type {::CAPI2_CcApi_GetCNAPName_Req_t}
	 /** 
	payload is ::CNAP_NAME_t 
	**/
	MSG_CC_GETCNAPNAME_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x348,	///<Payload type {::CNAP_NAME_t}
	 /** 
	api is CAPI2_SYSPARM_GetHSUPASupported 
	**/
	MSG_SYSPARM_GET_HSUPA_SUPPORTED_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x349,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SYSPARM_GET_HSUPA_SUPPORTED_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x34A,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SYSPARM_GetHSDPASupported 
	**/
	MSG_SYSPARM_GET_HSDPA_SUPPORTED_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x34B,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SYSPARM_GET_HSDPA_SUPPORTED_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x34C,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_NetRegApi_ForcePsReleaseReq 
	**/
	MSG_MS_FORCE_PS_REL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x34D,
	 /** 
	payload is ::void 
	**/
	MSG_MS_FORCE_PS_REL_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x34E,
	 /** 
	api is CAPI2_CcApi_IsCurrentStateMpty 
	**/
	MSG_CC_ISCURRENTSTATEMPTY_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x34F,	///<Payload type {::CAPI2_CcApi_IsCurrentStateMpty_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_CC_ISCURRENTSTATEMPTY_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x350,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PdpApi_GetPCHContextState 
	**/
	MSG_PDP_GETPCHCONTEXTSTATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x351,	///<Payload type {::CAPI2_PdpApi_GetPCHContextState_Req_t}
	 /** 
	payload is ::PCHContextState_t 
	**/
	MSG_PDP_GETPCHCONTEXTSTATE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x352,	///<Payload type {::PCHContextState_t}
	 /** 
	api is CcApi_PdpApi_GetPDPContextEx 
	**/
	MSG_PDP_GETPCHCONTEXT_EX_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x353,	///<Payload type {::CcApi_PdpApi_GetPDPContextEx_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_GETPCHCONTEXT_EX_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x354,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SimApi_GetCurrLockedSimlockType 
	**/
	MSG_SIM_PIN_LOCK_TYPE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x355,
	 /** 
	payload is ::SIM_PIN_Status_t 
	**/
	MSG_SIM_PIN_LOCK_TYPE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x356,	///<Payload type {::SIM_PIN_Status_t}
	 /** 
	api is CAPI2_SimApi_SubmitSelectFileSendApduReq 
	**/
	MSG_SIM_SEND_APDU_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x357,	///<Payload type {::CAPI2_SimApi_SubmitSelectFileSendApduReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendSelectApplicationReq 
	**/
	MSG_SIM_SELECT_APPLICATION_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x358,	///<Payload type {::CAPI2_SimApi_SendSelectApplicationReq_Req_t}
	 /** 
	api is CAPI2_SatkApi_SendProactiveCmdFetchingOnOffReq 
	**/
	MSG_STK_PROACTIVE_CMD_FETCHING_ON_OFF_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x359,	///<Payload type {::CAPI2_SatkApi_SendProactiveCmdFetchingOnOffReq_Req_t}
	 /** 
	api is CAPI2_SatkApi_SendExtProactiveCmdReq 
	**/
	MSG_SATK_SEND_EXT_PROACTIVE_CMD_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x35A,	///<Payload type {::CAPI2_SatkApi_SendExtProactiveCmdReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SATK_SEND_EXT_PROACTIVE_CMD_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x35B,
	 /** 
	api is CAPI2_SatkApi_SendTerminalProfileReq 
	**/
	MSG_SATK_SEND_TERMINAL_PROFILE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x35C,	///<Payload type {::CAPI2_SatkApi_SendTerminalProfileReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SATK_SEND_TERMINAL_PROFILE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x35D,
	 /** 
	api is CAPI2_SatkApi_SendPollingIntervalReq 
	**/
	MSG_STK_POLLING_INTERVAL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x35E,	///<Payload type {::CAPI2_SatkApi_SendPollingIntervalReq_Req_t}
	 /** 
	api is CAPI2_PdpApi_SetPDPActivationCallControlFlag 
	**/
	MSG_PDP_SETPDPACTIVATIONCCFLAG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x35F,	///<Payload type {::CAPI2_PdpApi_SetPDPActivationCallControlFlag_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_PDP_SETPDPACTIVATIONCCFLAG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x360,
	 /** 
	api is CAPI2_PdpApi_GetPDPActivationCallControlFlag 
	**/
	MSG_PDP_GETPDPACTIVATIONCCFLAG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x361,
	 /** 
	payload is ::Boolean 
	**/
	MSG_PDP_GETPDPACTIVATIONCCFLAG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x362,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PdpApi_SendPDPActivateReq_PDU 
	**/
	MSG_PDP_SENDPDPACTIVATIONPDU_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x363,	///<Payload type {::CAPI2_PdpApi_SendPDPActivateReq_PDU_Req_t}
	 /** 
	api is CAPI2_PdpApi_RejectNWIPDPActivation 
	**/
	MSG_PDP_REJECTNWIACTIVATION_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x364,	///<Payload type {::CAPI2_PdpApi_RejectNWIPDPActivation_Req_t}
	 /** 
	payload is ::void 
	**/
	MSG_PDP_REJECTNWIACTIVATION_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x365,

	//MSG_GEN_REQ_END = 0x48FF

#endif

