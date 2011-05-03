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
****************************************************************************//****************************************************************************
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


#ifdef DEVELOPMENT_CAPI2_XDR_UNION_MAPPING

	{ MSG_MS_GSM_REGISTERED_REQ,_T("MSG_MS_GSM_REGISTERED_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_MS_GSM_REGISTERED_RSP,_T("MSG_MS_GSM_REGISTERED_RSP"), (xdrproc_t)xdr_CAPI2_MS_IsGSMRegistered_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_MS_GPRS_REGISTERED_REQ,_T("MSG_MS_GPRS_REGISTERED_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_MS_GPRS_REGISTERED_RSP,_T("MSG_MS_GPRS_REGISTERED_RSP"), (xdrproc_t)xdr_CAPI2_MS_IsGPRSRegistered_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_MS_GSM_CAUSE_REQ,_T("MSG_MS_GSM_CAUSE_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_MS_GSM_CAUSE_RSP,_T("MSG_MS_GSM_CAUSE_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetGSMRegCause_Rsp_t, sizeof( NetworkCause_t ), 0 },
	{ MSG_MS_GPRS_CAUSE_REQ,_T("MSG_MS_GPRS_CAUSE_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_MS_GPRS_CAUSE_RSP,_T("MSG_MS_GPRS_CAUSE_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetGPRSRegCause_Rsp_t, sizeof( NetworkCause_t ), 0 },
	{ MSG_MS_REGISTERED_LAC_REQ,_T("MSG_MS_REGISTERED_LAC_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_MS_REGISTERED_LAC_RSP,_T("MSG_MS_REGISTERED_LAC_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetRegisteredLAC_Rsp_t, sizeof( UInt16 ), 0 },
	{ MSG_MS_GET_PLMN_MCC_REQ,_T("MSG_MS_GET_PLMN_MCC_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_MS_GET_PLMN_MCC_RSP,_T("MSG_MS_GET_PLMN_MCC_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetPlmnMCC_Rsp_t, sizeof( UInt16 ), 0 },
	{ MSG_MS_GET_PLMN_MNC_REQ,_T("MSG_MS_GET_PLMN_MNC_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_MS_GET_PLMN_MNC_RSP,_T("MSG_MS_GET_PLMN_MNC_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetPlmnMNC_Rsp_t, sizeof( UInt8 ), 0 },
	{ MSG_SYS_POWERDOWN_REQ,_T("MSG_SYS_POWERDOWN_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SYS_POWERDOWN_RSP,_T("MSG_SYS_POWERDOWN_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SYS_POWERUP_NORF_REQ,_T("MSG_SYS_POWERUP_NORF_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SYS_POWERUP_NORF_RSP,_T("MSG_SYS_POWERUP_NORF_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SYS_NORF_CALIB_REQ,_T("MSG_SYS_NORF_CALIB_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SYS_NORF_CALIB_RSP,_T("MSG_SYS_NORF_CALIB_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SYS_POWERUP_REQ,_T("MSG_SYS_POWERUP_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SYS_POWERUP_RSP,_T("MSG_SYS_POWERUP_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_MS_GPRS_ALLOWED_REQ,_T("MSG_MS_GPRS_ALLOWED_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_MS_GPRS_ALLOWED_RSP,_T("MSG_MS_GPRS_ALLOWED_RSP"), (xdrproc_t)xdr_CAPI2_MS_IsGprsAllowed_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_MS_GET_CURRENT_RAT_REQ,_T("MSG_MS_GET_CURRENT_RAT_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_MS_GET_CURRENT_RAT_RSP,_T("MSG_MS_GET_CURRENT_RAT_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetCurrentRAT_Rsp_t, sizeof( UInt8 ), 0 },
	{ MSG_MS_GET_CURRENT_BAND_REQ,_T("MSG_MS_GET_CURRENT_BAND_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_MS_GET_CURRENT_BAND_RSP,_T("MSG_MS_GET_CURRENT_BAND_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetCurrentBand_Rsp_t, sizeof( UInt8 ), 0 },
	{ MSG_SIM_UPDATE_SMSCAPEXC_FLAG_REQ,_T("MSG_SIM_UPDATE_SMSCAPEXC_FLAG_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_UpdateSMSCapExceededFlag_Req_t, 0, 0},
	{ MSG_SIM_UPDATE_SMSCAPEXC_FLAG_RSP,_T("MSG_SIM_UPDATE_SMSCAPEXC_FLAG_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_MS_SELECT_BAND_REQ,_T("MSG_MS_SELECT_BAND_REQ"), (xdrproc_t) xdr_CAPI2_NetRegApi_SelectBand_Req_t, 0, 0},
	{ MSG_MS_SELECT_BAND_RSP,_T("MSG_MS_SELECT_BAND_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_MS_SET_RAT_BAND_REQ,_T("MSG_MS_SET_RAT_BAND_REQ"), (xdrproc_t) xdr_CAPI2_NetRegApi_SetSupportedRATandBand_Req_t, 0, 0},
	{ MSG_MS_SET_RAT_BAND_RSP,_T("MSG_MS_SET_RAT_BAND_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_MS_CELL_LOCK_REQ,_T("MSG_MS_CELL_LOCK_REQ"), (xdrproc_t) xdr_CAPI2_NetRegApi_CellLock_Req_t, 0, 0},
	{ MSG_MS_CELL_LOCK_RSP,_T("MSG_MS_CELL_LOCK_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_MS_GET_MCC_COUNTRY_REQ,_T("MSG_MS_GET_MCC_COUNTRY_REQ"), (xdrproc_t) xdr_CAPI2_PLMN_GetCountryByMcc_Req_t, 0, 0},
	{ MSG_MS_GET_MCC_COUNTRY_RSP,_T("MSG_MS_GET_MCC_COUNTRY_RSP"), (xdrproc_t)xdr_CAPI2_PLMN_GetCountryByMcc_Rsp_t, sizeof( char ), 0 },
	{ MSG_MS_PLMN_INFO_BY_CODE_REQ,_T("MSG_MS_PLMN_INFO_BY_CODE_REQ"), (xdrproc_t) xdr_CAPI2_MS_GetPLMNEntryByIndex_Req_t, 0, 0},
	{ MSG_MS_PLMN_INFO_BY_CODE_RSP,_T("MSG_MS_PLMN_INFO_BY_CODE_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetPLMNEntryByIndex_Rsp_t, sizeof( CAPI2_MS_GetPLMNEntryByIndex_Rsp_t ), 0 },
	{ MSG_MS_PLMN_LIST_SIZE_REQ,_T("MSG_MS_PLMN_LIST_SIZE_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_MS_PLMN_LIST_SIZE_RSP,_T("MSG_MS_PLMN_LIST_SIZE_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetPLMNListSize_Rsp_t, sizeof( UInt16 ), 0 },
	{ MSG_MS_PLMN_INFO_BY_INDEX_REQ,_T("MSG_MS_PLMN_INFO_BY_INDEX_REQ"), (xdrproc_t) xdr_CAPI2_MS_GetPLMNByCode_Req_t, 0, 0},
	{ MSG_MS_PLMN_INFO_BY_INDEX_RSP,_T("MSG_MS_PLMN_INFO_BY_INDEX_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetPLMNByCode_Rsp_t, sizeof( CAPI2_MS_GetPLMNByCode_Rsp_t ), 0 },
	{ MSG_PLMN_SELECT_REQ,_T("MSG_PLMN_SELECT_REQ"), (xdrproc_t) xdr_CAPI2_NetRegApi_PlmnSelect_Req_t, 0, 0},
	{ MSG_PLMN_SELECT_RSP,_T("MSG_PLMN_SELECT_RSP"), (xdrproc_t)xdr_CAPI2_NetRegApi_PlmnSelect_Rsp_t, sizeof( CAPI2_NetRegApi_PlmnSelect_Rsp_t ), 0 },
	{ MSG_MS_PLMN_ABORT_REQ,_T("MSG_MS_PLMN_ABORT_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_MS_PLMN_ABORT_RSP,_T("MSG_MS_PLMN_ABORT_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_MS_GET_PLMN_MODE_REQ,_T("MSG_MS_GET_PLMN_MODE_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_MS_GET_PLMN_MODE_RSP,_T("MSG_MS_GET_PLMN_MODE_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetPlmnMode_Rsp_t, sizeof( PlmnSelectMode_t ), 0 },
	{ MSG_MS_SET_PLMN_MODE_REQ,_T("MSG_MS_SET_PLMN_MODE_REQ"), (xdrproc_t) xdr_CAPI2_NetRegApi_SetPlmnMode_Req_t, 0, 0},
	{ MSG_MS_SET_PLMN_MODE_RSP,_T("MSG_MS_SET_PLMN_MODE_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_MS_GET_PLMN_FORMAT_REQ,_T("MSG_MS_GET_PLMN_FORMAT_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_MS_GET_PLMN_FORMAT_RSP,_T("MSG_MS_GET_PLMN_FORMAT_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetPlmnFormat_Rsp_t, sizeof( PlmnSelectFormat_t ), 0 },
	{ MSG_MS_SET_PLMN_FORMAT_REQ,_T("MSG_MS_SET_PLMN_FORMAT_REQ"), (xdrproc_t) xdr_CAPI2_MS_SetPlmnFormat_Req_t, 0, 0},
	{ MSG_MS_SET_PLMN_FORMAT_RSP,_T("MSG_MS_SET_PLMN_FORMAT_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_MS_MATCH_PLMN_REQ,_T("MSG_MS_MATCH_PLMN_REQ"), (xdrproc_t) xdr_CAPI2_MS_IsMatchedPLMN_Req_t, 0, 0},
	{ MSG_MS_MATCH_PLMN_RSP,_T("MSG_MS_MATCH_PLMN_RSP"), (xdrproc_t)xdr_CAPI2_MS_IsMatchedPLMN_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_MS_SEARCH_PLMN_REQ,_T("MSG_MS_SEARCH_PLMN_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_MS_SEARCH_PLMN_RSP,_T("MSG_MS_SEARCH_PLMN_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_MS_ABORT_PLMN_REQ,_T("MSG_MS_ABORT_PLMN_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_MS_ABORT_PLMN_RSP,_T("MSG_MS_ABORT_PLMN_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_MS_AUTO_SEARCH_REQ,_T("MSG_MS_AUTO_SEARCH_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_MS_AUTO_SEARCH_RSP,_T("MSG_MS_AUTO_SEARCH_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_MS_PLMN_NAME_REQ,_T("MSG_MS_PLMN_NAME_REQ"), (xdrproc_t) xdr_CAPI2_NetRegApi_GetPLMNNameByCode_Req_t, 0, 0},
	{ MSG_MS_PLMN_NAME_RSP,_T("MSG_MS_PLMN_NAME_RSP"), (xdrproc_t)xdr_CAPI2_NetRegApi_GetPLMNNameByCode_Rsp_t, sizeof( CAPI2_NetRegApi_GetPLMNNameByCode_Rsp_t ), 0 },
	{ MSG_SYS_GET_SYSTEM_STATE_REQ,_T("MSG_SYS_GET_SYSTEM_STATE_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SYS_GET_SYSTEM_STATE_RSP,_T("MSG_SYS_GET_SYSTEM_STATE_RSP"), (xdrproc_t)xdr_CAPI2_PhoneCtrlApi_GetSystemState_Rsp_t, sizeof( SystemState_t ), 0 },
	{ MSG_SYS_SET_SYSTEM_STATE_REQ,_T("MSG_SYS_SET_SYSTEM_STATE_REQ"), (xdrproc_t) xdr_CAPI2_PhoneCtrlApi_SetSystemState_Req_t, 0, 0},
	{ MSG_SYS_SET_SYSTEM_STATE_RSP,_T("MSG_SYS_SET_SYSTEM_STATE_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SYS_GET_RX_LEVEL_REQ,_T("MSG_SYS_GET_RX_LEVEL_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SYS_GET_RX_LEVEL_RSP,_T("MSG_SYS_GET_RX_LEVEL_RSP"), (xdrproc_t)xdr_CAPI2_PhoneCtrlApi_GetRxSignalInfo_Rsp_t, sizeof( CAPI2_PhoneCtrlApi_GetRxSignalInfo_Rsp_t ), 0 },
	{ MSG_SYS_GET_GSMREG_STATUS_REQ,_T("MSG_SYS_GET_GSMREG_STATUS_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SYS_GET_GSMREG_STATUS_RSP,_T("MSG_SYS_GET_GSMREG_STATUS_RSP"), (xdrproc_t)xdr_CAPI2_SYS_GetGSMRegistrationStatus_Rsp_t, sizeof( RegisterStatus_t ), 0 },
	{ MSG_SYS_GET_GPRSREG_STATUS_REQ,_T("MSG_SYS_GET_GPRSREG_STATUS_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SYS_GET_GPRSREG_STATUS_RSP,_T("MSG_SYS_GET_GPRSREG_STATUS_RSP"), (xdrproc_t)xdr_CAPI2_SYS_GetGPRSRegistrationStatus_Rsp_t, sizeof( RegisterStatus_t ), 0 },
	{ MSG_SYS_GET_REG_STATUS_REQ,_T("MSG_SYS_GET_REG_STATUS_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SYS_GET_REG_STATUS_RSP,_T("MSG_SYS_GET_REG_STATUS_RSP"), (xdrproc_t)xdr_CAPI2_SYS_IsRegisteredGSMOrGPRS_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_HOME_PLMN_REG_REQ,_T("MSG_HOME_PLMN_REG_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_HOME_PLMN_REG_RSP,_T("MSG_HOME_PLMN_REG_RSP"), (xdrproc_t)xdr_CAPI2_MS_IsRegisteredHomePLMN_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SET_POWER_DOWN_TIMER_REQ,_T("MSG_SET_POWER_DOWN_TIMER_REQ"), (xdrproc_t) xdr_CAPI2_PhoneCtrlApi_SetPowerDownTimer_Req_t, 0, 0},
	{ MSG_SET_POWER_DOWN_TIMER_RSP,_T("MSG_SET_POWER_DOWN_TIMER_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SIM_PARAM_REC_NUM_REQ,_T("MSG_SIM_PARAM_REC_NUM_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_PARAM_REC_NUM_RSP,_T("MSG_SIM_PARAM_REC_NUM_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_GetSmsParamRecNum_Rsp_t, sizeof( CAPI2_SimApi_GetSmsParamRecNum_Rsp_t ), 0 },
	{ MSG_SIM_GET_SMSMEMEXC_FLAG_REQ,_T("MSG_SIM_GET_SMSMEMEXC_FLAG_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_GET_SMSMEMEXC_FLAG_RSP,_T("MSG_SIM_GET_SMSMEMEXC_FLAG_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_GetSmsMemExceededFlag_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SIM_IS_TEST_SIM_REQ,_T("MSG_SIM_IS_TEST_SIM_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_IS_TEST_SIM_RSP,_T("MSG_SIM_IS_TEST_SIM_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_IsTestSIM_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SIM_PIN_REQ_REQ,_T("MSG_SIM_PIN_REQ_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_PIN_REQ_RSP,_T("MSG_SIM_PIN_REQ_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_IsPINRequired_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SIM_CARD_PHASE_REQ,_T("MSG_SIM_CARD_PHASE_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_CARD_PHASE_RSP,_T("MSG_SIM_CARD_PHASE_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_GetCardPhase_Rsp_t, sizeof( SIMPhase_t ), 0 },
	{ MSG_SIM_TYPE_REQ,_T("MSG_SIM_TYPE_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_TYPE_RSP,_T("MSG_SIM_TYPE_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_GetSIMType_Rsp_t, sizeof( SIMType_t ), 0 },
	{ MSG_SIM_PRESENT_REQ,_T("MSG_SIM_PRESENT_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_PRESENT_RSP,_T("MSG_SIM_PRESENT_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_GetPresentStatus_Rsp_t, sizeof( SIMPresent_t ), 0 },
	{ MSG_SIM_PIN_OPERATION_REQ,_T("MSG_SIM_PIN_OPERATION_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_PIN_OPERATION_RSP,_T("MSG_SIM_PIN_OPERATION_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_IsOperationRestricted_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SIM_PIN_BLOCK_REQ,_T("MSG_SIM_PIN_BLOCK_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_IsPINBlocked_Req_t, 0, 0},
	{ MSG_SIM_PIN_BLOCK_RSP,_T("MSG_SIM_PIN_BLOCK_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_IsPINBlocked_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SIM_PUK_BLOCK_REQ,_T("MSG_SIM_PUK_BLOCK_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_IsPUKBlocked_Req_t, 0, 0},
	{ MSG_SIM_PUK_BLOCK_RSP,_T("MSG_SIM_PUK_BLOCK_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_IsPUKBlocked_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SIM_IS_INVALID_REQ,_T("MSG_SIM_IS_INVALID_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_IS_INVALID_RSP,_T("MSG_SIM_IS_INVALID_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_IsInvalidSIM_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SIM_DETECT_REQ,_T("MSG_SIM_DETECT_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_DETECT_RSP,_T("MSG_SIM_DETECT_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_DetectSim_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SIM_GET_RUIM_SUPP_FLAG_REQ,_T("MSG_SIM_GET_RUIM_SUPP_FLAG_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_GET_RUIM_SUPP_FLAG_RSP,_T("MSG_SIM_GET_RUIM_SUPP_FLAG_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_GetRuimSuppFlag_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SIM_VERIFY_CHV_REQ,_T("MSG_SIM_VERIFY_CHV_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SendVerifyChvReq_Req_t, 0, 0},
	{ MSG_SIM_VERIFY_CHV_RSP,_T("MSG_SIM_VERIFY_CHV_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_SendVerifyChvReq_Rsp_t, sizeof( SimPinRsp_t ), 0 },
	{ MSG_SIM_CHANGE_CHV_REQ,_T("MSG_SIM_CHANGE_CHV_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SendChangeChvReq_Req_t, 0, 0},
	{ MSG_SIM_CHANGE_CHV_RSP,_T("MSG_SIM_CHANGE_CHV_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_SendChangeChvReq_Rsp_t, sizeof( SimPinRsp_t ), 0 },
	{ MSG_SIM_ENABLE_CHV_REQ,_T("MSG_SIM_ENABLE_CHV_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SendSetChv1OnOffReq_Req_t, 0, 0},
	{ MSG_SIM_ENABLE_CHV_RSP,_T("MSG_SIM_ENABLE_CHV_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_SendSetChv1OnOffReq_Rsp_t, sizeof( SimPinRsp_t ), 0 },
	{ MSG_SIM_UNBLOCK_CHV_REQ,_T("MSG_SIM_UNBLOCK_CHV_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SendUnblockChvReq_Req_t, 0, 0},
	{ MSG_SIM_UNBLOCK_CHV_RSP,_T("MSG_SIM_UNBLOCK_CHV_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_SendUnblockChvReq_Rsp_t, sizeof( SimPinRsp_t ), 0 },
	{ MSG_SIM_SET_FDN_REQ,_T("MSG_SIM_SET_FDN_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SendSetOperStateReq_Req_t, 0, 0},
	{ MSG_SIM_SET_FDN_RSP,_T("MSG_SIM_SET_FDN_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_SendSetOperStateReq_Rsp_t, sizeof( SIMAccess_t ), 0 },
	{ MSG_SIM_IS_PBK_ALLOWED_REQ,_T("MSG_SIM_IS_PBK_ALLOWED_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_IsPbkAccessAllowed_Req_t, 0, 0},
	{ MSG_SIM_IS_PBK_ALLOWED_RSP,_T("MSG_SIM_IS_PBK_ALLOWED_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_IsPbkAccessAllowed_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SIM_PBK_INFO_REQ,_T("MSG_SIM_PBK_INFO_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SendPbkInfoReq_Req_t, 0, 0},
	{ MSG_SIM_PBK_INFO_RSP,_T("MSG_SIM_PBK_INFO_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_SendPbkInfoReq_Rsp_t, sizeof( SIM_PBK_INFO_t ), 0 },
	{ MSG_SIM_MAX_ACM_REQ,_T("MSG_SIM_MAX_ACM_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_MAX_ACM_RSP,_T("MSG_SIM_MAX_ACM_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_SendReadAcmMaxReq_Rsp_t, sizeof( SIM_MAX_ACM_t ), 0 },
	{ MSG_SIM_ACM_MAX_UPDATE_REQ,_T("MSG_SIM_ACM_MAX_UPDATE_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SendWriteAcmMaxReq_Req_t, 0, 0},
	{ MSG_SIM_ACM_MAX_UPDATE_RSP,_T("MSG_SIM_ACM_MAX_UPDATE_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_SendWriteAcmMaxReq_Rsp_t, sizeof( SIMAccess_t ), 0 },
	{ MSG_SIM_ACM_VALUE_REQ,_T("MSG_SIM_ACM_VALUE_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_ACM_VALUE_RSP,_T("MSG_SIM_ACM_VALUE_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_SendReadAcmReq_Rsp_t, sizeof( SIM_ACM_VALUE_t ), 0 },
	{ MSG_SIM_ACM_UPDATE_REQ,_T("MSG_SIM_ACM_UPDATE_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SendWriteAcmReq_Req_t, 0, 0},
	{ MSG_SIM_ACM_UPDATE_RSP,_T("MSG_SIM_ACM_UPDATE_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_SendWriteAcmReq_Rsp_t, sizeof( SIMAccess_t ), 0 },
	{ MSG_SIM_ACM_INCREASE_REQ,_T("MSG_SIM_ACM_INCREASE_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SendIncreaseAcmReq_Req_t, 0, 0},
	{ MSG_SIM_ACM_INCREASE_RSP,_T("MSG_SIM_ACM_INCREASE_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_SendIncreaseAcmReq_Rsp_t, sizeof( SIMAccess_t ), 0 },
	{ MSG_SIM_SVC_PROV_NAME_REQ,_T("MSG_SIM_SVC_PROV_NAME_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_SVC_PROV_NAME_RSP,_T("MSG_SIM_SVC_PROV_NAME_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_SendReadSvcProvNameReq_Rsp_t, sizeof( SIM_SVC_PROV_NAME_t ), 0 },
	{ MSG_SIM_PUCT_DATA_REQ,_T("MSG_SIM_PUCT_DATA_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_PUCT_DATA_RSP,_T("MSG_SIM_PUCT_DATA_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_SendReadPuctReq_Rsp_t, sizeof( SIM_PUCT_DATA_t ), 0 },
	{ MSG_SIM_SERVICE_STATUS_REQ,_T("MSG_SIM_SERVICE_STATUS_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_GetServiceStatus_Req_t, 0, 0},
	{ MSG_SIM_SERVICE_STATUS_RSP,_T("MSG_SIM_SERVICE_STATUS_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_GetServiceStatus_Rsp_t, sizeof( SIMServiceStatus_t ), 0 },
	{ MSG_SIM_PIN_STATUS_REQ,_T("MSG_SIM_PIN_STATUS_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_PIN_STATUS_RSP,_T("MSG_SIM_PIN_STATUS_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_GetPinStatus_Rsp_t, sizeof( SIM_PIN_Status_t ), 0 },
	{ MSG_SIM_PIN_OK_STATUS_REQ,_T("MSG_SIM_PIN_OK_STATUS_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_PIN_OK_STATUS_RSP,_T("MSG_SIM_PIN_OK_STATUS_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_IsPinOK_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SIM_IMSI_REQ,_T("MSG_SIM_IMSI_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_IMSI_RSP,_T("MSG_SIM_IMSI_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_GetIMSI_Rsp_t, sizeof( IMSI_t ), 0 },
	{ MSG_SIM_GID_DIGIT_REQ,_T("MSG_SIM_GID_DIGIT_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_GID_DIGIT_RSP,_T("MSG_SIM_GID_DIGIT_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_GetGID1_Rsp_t, sizeof( GID_DIGIT_t ), 0 },
	{ MSG_SIM_GID_DIGIT2_REQ,_T("MSG_SIM_GID_DIGIT2_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_GID_DIGIT2_RSP,_T("MSG_SIM_GID_DIGIT2_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_GetGID2_Rsp_t, sizeof( GID_DIGIT_t ), 0 },
	{ MSG_SIM_HOME_PLMN_REQ,_T("MSG_SIM_HOME_PLMN_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_HOME_PLMN_RSP,_T("MSG_SIM_HOME_PLMN_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_GetHomePlmn_Rsp_t, sizeof( CAPI2_SimApi_GetHomePlmn_Rsp_t ), 0 },
	{ MSG_SIM_APDU_FILEID_REQ,_T("MSG_SIM_APDU_FILEID_REQ"), (xdrproc_t) xdr_CAPI2_simmiApi_GetMasterFileId_Req_t, 0, 0},
	{ MSG_SIM_APDU_FILEID_RSP,_T("MSG_SIM_APDU_FILEID_RSP"), (xdrproc_t)xdr_CAPI2_simmiApi_GetMasterFileId_Rsp_t, sizeof( APDUFileID_t ), 0 },
	{ MSG_SIM_OPEN_SOCKET_REQ,_T("MSG_SIM_OPEN_SOCKET_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_OPEN_SOCKET_RSP,_T("MSG_SIM_OPEN_SOCKET_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_SendOpenSocketReq_Rsp_t, sizeof( SIM_OPEN_SOCKET_RES_t ), 0 },
	{ MSG_SIM_SELECT_APPLI_REQ,_T("MSG_SIM_SELECT_APPLI_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SendSelectAppiReq_Req_t, 0, 0},
	{ MSG_SIM_SELECT_APPLI_RSP,_T("MSG_SIM_SELECT_APPLI_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_SendSelectAppiReq_Rsp_t, sizeof( SIM_SELECT_APPLI_RES_t ), 0 },
	{ MSG_SIM_DEACTIVATE_APPLI_REQ,_T("MSG_SIM_DEACTIVATE_APPLI_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SendDeactivateAppiReq_Req_t, 0, 0},
	{ MSG_SIM_DEACTIVATE_APPLI_RSP,_T("MSG_SIM_DEACTIVATE_APPLI_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_SendDeactivateAppiReq_Rsp_t, sizeof( SIM_DEACTIVATE_APPLI_RES_t ), 0 },
	{ MSG_SIM_CLOSE_SOCKET_REQ,_T("MSG_SIM_CLOSE_SOCKET_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SendCloseSocketReq_Req_t, 0, 0},
	{ MSG_SIM_CLOSE_SOCKET_RSP,_T("MSG_SIM_CLOSE_SOCKET_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_SendCloseSocketReq_Rsp_t, sizeof( SIM_CLOSE_SOCKET_RES_t ), 0 },
	{ MSG_SIM_ATR_DATA_REQ,_T("MSG_SIM_ATR_DATA_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_ATR_DATA_RSP,_T("MSG_SIM_ATR_DATA_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_GetAtrData_Rsp_t, sizeof( CAPI2_SimApi_GetAtrData_Rsp_t ), 0 },
	{ MSG_SIM_DFILE_INFO_REQ,_T("MSG_SIM_DFILE_INFO_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SubmitDFileInfoReq_Req_t, 0, 0},
	{ MSG_SIM_DFILE_INFO_RSP,_T("MSG_SIM_DFILE_INFO_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_SubmitDFileInfoReq_Rsp_t, sizeof( SIM_DFILE_INFO_t ), 0 },
	{ MSG_SIM_EFILE_INFO_REQ,_T("MSG_SIM_EFILE_INFO_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SubmitEFileInfoReq_Req_t, 0, 0},
	{ MSG_SIM_EFILE_INFO_RSP,_T("MSG_SIM_EFILE_INFO_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_SubmitEFileInfoReq_Rsp_t, sizeof( SIM_EFILE_INFO_t ), 0 },
	{ MSG_SIM_SEND_EFILE_INFO_REQ,_T("MSG_SIM_SEND_EFILE_INFO_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SendEFileInfoReq_Req_t, 0, 0},
	{ MSG_SIM_SEND_DFILE_INFO_REQ,_T("MSG_SIM_SEND_DFILE_INFO_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SendDFileInfoReq_Req_t, 0, 0},
	{ MSG_SIM_WHOLE_EFILE_DATA_REQ,_T("MSG_SIM_WHOLE_EFILE_DATA_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SubmitWholeBinaryEFileReadReq_Req_t, 0, 0},
	{ MSG_SIM_EFILE_DATA_RSP,_T("MSG_SIM_EFILE_DATA_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_SubmitWholeBinaryEFileReadReq_Rsp_t, sizeof( SIM_EFILE_DATA_t ), 0 },
	{ MSG_SIM_SEND_WHOLE_EFILE_DATA_REQ,_T("MSG_SIM_SEND_WHOLE_EFILE_DATA_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SendWholeBinaryEFileReadReq_Req_t, 0, 0},
	{ MSG_SIM_EFILE_DATA_REQ,_T("MSG_SIM_EFILE_DATA_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SubmitBinaryEFileReadReq_Req_t, 0, 0},
	{ MSG_SIM_SEND_EFILE_DATA_REQ,_T("MSG_SIM_SEND_EFILE_DATA_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SendBinaryEFileReadReq_Req_t, 0, 0},
	{ MSG_SIM_RECORD_EFILE_DATA_REQ,_T("MSG_SIM_RECORD_EFILE_DATA_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SubmitRecordEFileReadReq_Req_t, 0, 0},
	{ MSG_SIM_SEND_RECORD_EFILE_DATA_REQ,_T("MSG_SIM_SEND_RECORD_EFILE_DATA_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SendRecordEFileReadReq_Req_t, 0, 0},
	{ MSG_SIM_EFILE_UPDATE_REQ,_T("MSG_SIM_EFILE_UPDATE_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SubmitBinaryEFileUpdateReq_Req_t, 0, 0},
	{ MSG_SIM_EFILE_UPDATE_RSP,_T("MSG_SIM_EFILE_UPDATE_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_SubmitBinaryEFileUpdateReq_Rsp_t, sizeof( SIM_EFILE_UPDATE_RESULT_t ), 0 },
	{ MSG_SIM_SEND_EFILE_UPDATE_REQ,_T("MSG_SIM_SEND_EFILE_UPDATE_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SendBinaryEFileUpdateReq_Req_t, 0, 0},
	{ MSG_SIM_LINEAR_EFILE_UPDATE_REQ,_T("MSG_SIM_LINEAR_EFILE_UPDATE_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SubmitLinearEFileUpdateReq_Req_t, 0, 0},
	{ MSG_SIM_SEND_LINEAR_EFILE_UPDATE_REQ,_T("MSG_SIM_SEND_LINEAR_EFILE_UPDATE_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SendLinearEFileUpdateReq_Req_t, 0, 0},
	{ MSG_SIM_SEEK_RECORD_REQ,_T("MSG_SIM_SEEK_RECORD_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SubmitSeekRecordReq_Req_t, 0, 0},
	{ MSG_SIM_SEEK_REC_RSP,_T("MSG_SIM_SEEK_REC_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_SubmitSeekRecordReq_Rsp_t, sizeof( SIM_SEEK_RECORD_DATA_t ), 0 },
	{ MSG_SIM_SEND_SEEK_RECORD_REQ,_T("MSG_SIM_SEND_SEEK_RECORD_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SendSeekRecordReq_Req_t, 0, 0},
	{ MSG_SIM_CYCLIC_EFILE_UPDATE_REQ,_T("MSG_SIM_CYCLIC_EFILE_UPDATE_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SubmitCyclicEFileUpdateReq_Req_t, 0, 0},
	{ MSG_SIM_SEND_CYCLIC_EFILE_UPDATE_REQ,_T("MSG_SIM_SEND_CYCLIC_EFILE_UPDATE_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SendCyclicEFileUpdateReq_Req_t, 0, 0},
	{ MSG_SIM_PIN_ATTEMPT_REQ,_T("MSG_SIM_PIN_ATTEMPT_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_PIN_ATTEMPT_RSP,_T("MSG_SIM_PIN_ATTEMPT_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_SendRemainingPinAttemptReq_Rsp_t, sizeof( PIN_ATTEMPT_RESULT_t ), 0 },
	{ MSG_SIM_CACHE_DATA_READY_REQ,_T("MSG_SIM_CACHE_DATA_READY_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_CACHE_DATA_READY_RSP,_T("MSG_SIM_CACHE_DATA_READY_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_IsCachedDataReady_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SIM_SERVICE_CODE_STATUS_REQ,_T("MSG_SIM_SERVICE_CODE_STATUS_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_GetServiceCodeStatus_Req_t, 0, 0},
	{ MSG_SIM_SERVICE_CODE_STATUS_RSP,_T("MSG_SIM_SERVICE_CODE_STATUS_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_GetServiceCodeStatus_Rsp_t, sizeof( SERVICE_FLAG_STATUS_t ), 0 },
	{ MSG_SIM_CHECK_CPHS_REQ,_T("MSG_SIM_CHECK_CPHS_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_CheckCphsService_Req_t, 0, 0},
	{ MSG_SIM_CHECK_CPHS_RSP,_T("MSG_SIM_CHECK_CPHS_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_CheckCphsService_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SIM_CPHS_PHASE_REQ,_T("MSG_SIM_CPHS_PHASE_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_CPHS_PHASE_RSP,_T("MSG_SIM_CPHS_PHASE_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_GetCphsPhase_Rsp_t, sizeof( UInt8 ), 0 },
	{ MSG_SIM_SMS_SCA_REQ,_T("MSG_SIM_SMS_SCA_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_GetSmsSca_Req_t, 0, 0},
	{ MSG_SIM_SMS_SCA_RSP,_T("MSG_SIM_SMS_SCA_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_GetSmsSca_Rsp_t, sizeof( CAPI2_SimApi_GetSmsSca_Rsp_t ), 0 },
	{ MSG_SIM_ICCID_PARAM_REQ,_T("MSG_SIM_ICCID_PARAM_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_ICCID_PARAM_RSP,_T("MSG_SIM_ICCID_PARAM_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_GetIccid_Rsp_t, sizeof( CAPI2_SimApi_GetIccid_Rsp_t ), 0 },
	{ MSG_SIM_ALS_STATUS_REQ,_T("MSG_SIM_ALS_STATUS_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_ALS_STATUS_RSP,_T("MSG_SIM_ALS_STATUS_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_IsALSEnabled_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SIM_ALS_DEFAULT_LINE_REQ,_T("MSG_SIM_ALS_DEFAULT_LINE_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_ALS_DEFAULT_LINE_RSP,_T("MSG_SIM_ALS_DEFAULT_LINE_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_GetAlsDefaultLine_Rsp_t, sizeof( UInt8 ), 0 },
	{ MSG_SIM_SET_ALS_DEFAULT_REQ,_T("MSG_SIM_SET_ALS_DEFAULT_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SetAlsDefaultLine_Req_t, 0, 0},
	{ MSG_SIM_SET_ALS_DEFAULT_RSP,_T("MSG_SIM_SET_ALS_DEFAULT_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SIM_CALLFWD_COND_FLAG_REQ,_T("MSG_SIM_CALLFWD_COND_FLAG_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_CALLFWD_COND_FLAG_RSP,_T("MSG_SIM_CALLFWD_COND_FLAG_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_GetCallForwardUnconditionalFlag_Rsp_t, sizeof( CAPI2_SimApi_GetCallForwardUnconditionalFlag_Rsp_t ), 0 },
	{ MSG_SIM_APP_TYPE_REQ,_T("MSG_SIM_APP_TYPE_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_APP_TYPE_RSP,_T("MSG_SIM_APP_TYPE_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_GetApplicationType_Rsp_t, sizeof( SIM_APPL_TYPE_t ), 0 },
	{ MSG_SIM_PUCT_UPDATE_REQ,_T("MSG_SIM_PUCT_UPDATE_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SendWritePuctReq_Req_t, 0, 0},
	{ MSG_SIM_PUCT_UPDATE_RSP,_T("MSG_SIM_PUCT_UPDATE_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_SendWritePuctReq_Rsp_t, sizeof( SIMAccess_t ), 0 },
	{ MSG_SIM_RESTRICTED_ACCESS_REQ,_T("MSG_SIM_RESTRICTED_ACCESS_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SubmitRestrictedAccessReq_Req_t, 0, 0},
	{ MSG_SIM_RESTRICTED_ACCESS_RSP,_T("MSG_SIM_RESTRICTED_ACCESS_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_SubmitRestrictedAccessReq_Rsp_t, sizeof( SIM_RESTRICTED_ACCESS_DATA_t ), 0 },
	{ MSG_SIM_DETECTION_IND,_T("MSG_SIM_DETECTION_IND"), (xdrproc_t)xdr_CAPI2_SIM_SendDetectionInd_Rsp_t, sizeof( SIM_DETECTION_t ), 0 },
	{ MSG_REG_GSM_IND,_T("MSG_REG_GSM_IND"), (xdrproc_t)xdr_CAPI2_MS_GsmRegStatusInd_Rsp_t, sizeof( MSRegInfo_t ), 0 },
	{ MSG_REG_GPRS_IND,_T("MSG_REG_GPRS_IND"), (xdrproc_t)xdr_CAPI2_MS_GprsRegStatusInd_Rsp_t, sizeof( MSRegInfo_t ), 0 },
	{ MSG_NETWORK_NAME_IND,_T("MSG_NETWORK_NAME_IND"), (xdrproc_t)xdr_CAPI2_MS_NetworkNameInd_Rsp_t, sizeof( nitzNetworkName_t ), 0 },
	{ MSG_LSA_ID_IND,_T("MSG_LSA_ID_IND"), (xdrproc_t)xdr_CAPI2_MS_LsaInd_Rsp_t, sizeof( lsaIdentity_t ), 0 },
	{ MSG_RSSI_IND,_T("MSG_RSSI_IND"), (xdrproc_t)xdr_CAPI2_MS_RssiInd_Rsp_t, sizeof( RxSignalInfo_t ), 0 },
	{ MSG_RX_SIGNAL_INFO_CHG_IND,_T("MSG_RX_SIGNAL_INFO_CHG_IND"), (xdrproc_t)xdr_CAPI2_MS_SignalChangeInd_Rsp_t, sizeof( RX_SIGNAL_INFO_CHG_t ), 0 },
	{ MSG_PLMNLIST_IND,_T("MSG_PLMNLIST_IND"), (xdrproc_t)xdr_CAPI2_MS_PlmnListInd_Rsp_t, sizeof( SEARCHED_PLMN_LIST_t ), 3000 },
	{ MSG_DATE_TIMEZONE_IND,_T("MSG_DATE_TIMEZONE_IND"), (xdrproc_t)xdr_CAPI2_MS_TimeZoneInd_Rsp_t, sizeof( TimeZoneDate_t ), 0 },
	{ MSG_MS_GET_SYSTEM_RAT_REQ,_T("MSG_MS_GET_SYSTEM_RAT_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_MS_GET_SYSTEM_RAT_RSP,_T("MSG_MS_GET_SYSTEM_RAT_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetSystemRAT_Rsp_t, sizeof( RATSelect_t ), 0 },
	{ MSG_MS_GET_SUPPORTED_RAT_REQ,_T("MSG_MS_GET_SUPPORTED_RAT_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_MS_GET_SUPPORTED_RAT_RSP,_T("MSG_MS_GET_SUPPORTED_RAT_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetSupportedRAT_Rsp_t, sizeof( RATSelect_t ), 0 },
	{ MSG_MS_GET_SYSTEM_BAND_REQ,_T("MSG_MS_GET_SYSTEM_BAND_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_MS_GET_SYSTEM_BAND_RSP,_T("MSG_MS_GET_SYSTEM_BAND_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetSystemBand_Rsp_t, sizeof( BandSelect_t ), 0 },
	{ MSG_MS_GET_SUPPORTED_BAND_REQ,_T("MSG_MS_GET_SUPPORTED_BAND_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_MS_GET_SUPPORTED_BAND_RSP,_T("MSG_MS_GET_SUPPORTED_BAND_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetSupportedBand_Rsp_t, sizeof( BandSelect_t ), 0 },
	{ MSG_SYSPARAM_GET_MSCLASS_REQ,_T("MSG_SYSPARAM_GET_MSCLASS_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SYSPARAM_GET_MSCLASS_RSP,_T("MSG_SYSPARAM_GET_MSCLASS_RSP"), (xdrproc_t)xdr_CAPI2_SYSPARM_GetMSClass_Rsp_t, sizeof( UInt16 ), 0 },
	{ MSG_SYSPARAM_GET_MNF_NAME_REQ,_T("MSG_SYSPARAM_GET_MNF_NAME_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SYSPARAM_GET_MNF_NAME_RSP,_T("MSG_SYSPARAM_GET_MNF_NAME_RSP"), (xdrproc_t)xdr_CAPI2_SYSPARM_GetManufacturerName_Rsp_t, sizeof( uchar_ptr_t ), 0 },
	{ MSG_SYSPARAM_GET_MODEL_NAME_REQ,_T("MSG_SYSPARAM_GET_MODEL_NAME_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SYSPARAM_GET_MODEL_NAME_RSP,_T("MSG_SYSPARAM_GET_MODEL_NAME_RSP"), (xdrproc_t)xdr_CAPI2_SYSPARM_GetModelName_Rsp_t, sizeof( uchar_ptr_t ), 0 },
	{ MSG_SYSPARAM_GET_SW_VERSION_REQ,_T("MSG_SYSPARAM_GET_SW_VERSION_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SYSPARAM_GET_SW_VERSION_RSP,_T("MSG_SYSPARAM_GET_SW_VERSION_RSP"), (xdrproc_t)xdr_CAPI2_SYSPARM_GetSWVersion_Rsp_t, sizeof( uchar_ptr_t ), 0 },
	{ MSG_SYSPARAM_GET_EGPRS_CLASS_REQ,_T("MSG_SYSPARAM_GET_EGPRS_CLASS_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SYSPARAM_GET_EGPRS_CLASS_RSP,_T("MSG_SYSPARAM_GET_EGPRS_CLASS_RSP"), (xdrproc_t)xdr_CAPI2_SYSPARM_GetEGPRSMSClass_Rsp_t, sizeof( UInt16 ), 0 },
	{ MSG_SIM_PLMN_NUM_OF_ENTRY_REQ,_T("MSG_SIM_PLMN_NUM_OF_ENTRY_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SendNumOfPLMNEntryReq_Req_t, 0, 0},
	{ MSG_SIM_PLMN_NUM_OF_ENTRY_RSP,_T("MSG_SIM_PLMN_NUM_OF_ENTRY_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_SendNumOfPLMNEntryReq_Rsp_t, sizeof( SIM_PLMN_NUM_OF_ENTRY_t ), 0 },
	{ MSG_SIM_PLMN_ENTRY_DATA_REQ,_T("MSG_SIM_PLMN_ENTRY_DATA_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SendReadPLMNEntryReq_Req_t, 0, 3000},
	{ MSG_SIM_PLMN_ENTRY_DATA_RSP,_T("MSG_SIM_PLMN_ENTRY_DATA_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_SendReadPLMNEntryReq_Rsp_t, sizeof( SIM_PLMN_ENTRY_DATA_t ), 3000 },
	{ MSG_SIM_MUL_PLMN_ENTRY_UPDATE_REQ,_T("MSG_SIM_MUL_PLMN_ENTRY_UPDATE_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SendWriteMulPLMNEntryReq_Req_t, 0, 0},
	{ MSG_SIM_MUL_PLMN_ENTRY_UPDATE_RSP,_T("MSG_SIM_MUL_PLMN_ENTRY_UPDATE_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_SendWriteMulPLMNEntryReq_Rsp_t, sizeof( SIM_MUL_PLMN_ENTRY_UPDATE_t ), 0 },
	{ MSG_SYS_SET_REG_EVENT_MASK_REQ,_T("MSG_SYS_SET_REG_EVENT_MASK_REQ"), (xdrproc_t) xdr_CAPI2_SYS_SetRegisteredEventMask_Req_t, 0, 0},
	{ MSG_SYS_SET_REG_EVENT_MASK_RSP,_T("MSG_SYS_SET_REG_EVENT_MASK_RSP"), (xdrproc_t)xdr_CAPI2_SYS_SetRegisteredEventMask_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SYS_SET_REG_FILTER_MASK_REQ,_T("MSG_SYS_SET_REG_FILTER_MASK_REQ"), (xdrproc_t) xdr_CAPI2_SYS_SetFilteredEventMask_Req_t, 0, 0},
	{ MSG_SYS_SET_REG_FILTER_MASK_RSP,_T("MSG_SYS_SET_REG_FILTER_MASK_RSP"), (xdrproc_t)xdr_CAPI2_SYS_SetFilteredEventMask_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SYS_SET_RSSI_THRESHOLD_REQ,_T("MSG_SYS_SET_RSSI_THRESHOLD_REQ"), (xdrproc_t) xdr_CAPI2_PhoneCtrlApi_SetRssiThreshold_Req_t, 0, 0},
	{ MSG_SYS_SET_RSSI_THRESHOLD_RSP,_T("MSG_SYS_SET_RSSI_THRESHOLD_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SYSPARAM_GET_CHANNEL_MODE_REQ,_T("MSG_SYSPARAM_GET_CHANNEL_MODE_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SYSPARAM_GET_CHANNEL_MODE_RSP,_T("MSG_SYSPARAM_GET_CHANNEL_MODE_RSP"), (xdrproc_t)xdr_CAPI2_SYSPARM_GetChanMode_Rsp_t, sizeof( UInt16 ), 0 },
	{ MSG_SYSPARAM_GET_CLASSMARK_REQ,_T("MSG_SYSPARAM_GET_CLASSMARK_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SYSPARAM_GET_CLASSMARK_RSP,_T("MSG_SYSPARAM_GET_CLASSMARK_RSP"), (xdrproc_t)xdr_CAPI2_SYSPARM_GetClassmark_Rsp_t, sizeof( CAPI2_Class_t ), 0 },
	{ MSG_SYSPARAM_GET_IND_FILE_VER_REQ,_T("MSG_SYSPARAM_GET_IND_FILE_VER_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SYSPARAM_GET_IND_FILE_VER_RSP,_T("MSG_SYSPARAM_GET_IND_FILE_VER_RSP"), (xdrproc_t)xdr_CAPI2_SYSPARM_GetSysparmIndPartFileVersion_Rsp_t, sizeof( UInt16 ), 0 },
	{ MSG_SYS_SET_DARP_CFG_REQ,_T("MSG_SYS_SET_DARP_CFG_REQ"), (xdrproc_t) xdr_CAPI2_SYSPARM_SetDARPCfg_Req_t, 0, 0},
	{ MSG_SYS_SET_DARP_CFG_RSP,_T("MSG_SYS_SET_DARP_CFG_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SYSPARAM_SET_EGPRS_MSCLASS_REQ,_T("MSG_SYSPARAM_SET_EGPRS_MSCLASS_REQ"), (xdrproc_t) xdr_CAPI2_SYSPARM_SetEGPRSMSClass_Req_t, 0, 0},
	{ MSG_SYSPARAM_SET_EGPRS_MSCLASS_RSP,_T("MSG_SYSPARAM_SET_EGPRS_MSCLASS_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SYSPARAM_SET_GPRS_MSCLASS_REQ,_T("MSG_SYSPARAM_SET_GPRS_MSCLASS_REQ"), (xdrproc_t) xdr_CAPI2_SYSPARM_SetGPRSMSClass_Req_t, 0, 0},
	{ MSG_SYSPARAM_SET_GPRS_MSCLASS_RSP,_T("MSG_SYSPARAM_SET_GPRS_MSCLASS_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_TIMEZONE_DELETE_NW_NAME_REQ,_T("MSG_TIMEZONE_DELETE_NW_NAME_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_TIMEZONE_DELETE_NW_NAME_RSP,_T("MSG_TIMEZONE_DELETE_NW_NAME_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_CAPI2_TEST_REQ,_T("MSG_CAPI2_TEST_REQ"), (xdrproc_t) xdr_CAPI2_TestCmds_Req_t, 0, 0},
	{ MSG_CAPI2_TEST_RSP,_T("MSG_CAPI2_TEST_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_STK_SEND_PLAYTONE_RES_REQ,_T("MSG_STK_SEND_PLAYTONE_RES_REQ"), (xdrproc_t) xdr_CAPI2_SatkApi_SendPlayToneRes_Req_t, 0, 0},
	{ MSG_STK_SEND_PLAYTONE_RES_RSP,_T("MSG_STK_SEND_PLAYTONE_RES_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_STK_SETUP_CALL_RES_REQ,_T("MSG_STK_SETUP_CALL_RES_REQ"), (xdrproc_t) xdr_CAPI2_SATK_SendSetupCallRes_Req_t, 0, 0},
	{ MSG_STK_SETUP_CALL_RES_RSP,_T("MSG_STK_SETUP_CALL_RES_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_PBK_SET_FDN_CHECK_REQ,_T("MSG_PBK_SET_FDN_CHECK_REQ"), (xdrproc_t) xdr_CAPI2_PbkApi_SetFdnCheck_Req_t, 0, 0},
	{ MSG_PBK_SET_FDN_CHECK_RSP,_T("MSG_PBK_SET_FDN_CHECK_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_PBK_GET_FDN_CHECK_REQ,_T("MSG_PBK_GET_FDN_CHECK_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_PBK_GET_FDN_CHECK_RSP,_T("MSG_PBK_GET_FDN_CHECK_RSP"), (xdrproc_t)xdr_CAPI2_PbkApi_GetFdnCheck_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SMS_SEND_MEM_AVAL_IND_REQ,_T("MSG_SMS_SEND_MEM_AVAL_IND_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SMS_SEND_MEM_AVAL_IND_RSP,_T("MSG_SMS_SEND_MEM_AVAL_IND_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_CONFIGUREMESTORAGE_REQ,_T("MSG_SMS_CONFIGUREMESTORAGE_REQ"), (xdrproc_t) xdr_CAPI2_SMS_ConfigureMEStorage_Req_t, 0, 0},
	{ MSG_SMS_CONFIGUREMESTORAGE_RSP,_T("MSG_SMS_CONFIGUREMESTORAGE_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_MS_SET_ELEMENT_REQ,_T("MSG_MS_SET_ELEMENT_REQ"), (xdrproc_t) xdr_CAPI2_MsDbApi_SetElement_Req_t, 0, 0},
	{ MSG_MS_SET_ELEMENT_RSP,_T("MSG_MS_SET_ELEMENT_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_MS_GET_ELEMENT_REQ,_T("MSG_MS_GET_ELEMENT_REQ"), (xdrproc_t) xdr_CAPI2_MsDbApi_GetElement_Req_t, 0, 0},
	{ MSG_MS_GET_ELEMENT_RSP,_T("MSG_MS_GET_ELEMENT_RSP"), (xdrproc_t)xdr_CAPI2_MsDbApi_GetElement_Rsp_t, sizeof( CAPI2_MS_Element_t ), 0 },
	{ MSG_USIM_IS_APP_SUPPORTED_REQ,_T("MSG_USIM_IS_APP_SUPPORTED_REQ"), (xdrproc_t) xdr_CAPI2_USimApi_IsApplicationSupported_Req_t, 0, 0},
	{ MSG_USIM_IS_APP_SUPPORTED_RSP,_T("MSG_USIM_IS_APP_SUPPORTED_RSP"), (xdrproc_t)xdr_CAPI2_USimApi_IsApplicationSupported_Rsp_t, sizeof( CAPI2_USimApi_IsApplicationSupported_Rsp_t ), 0 },
	{ MSG_USIM_IS_APN_ALLOWED_REQ,_T("MSG_USIM_IS_APN_ALLOWED_REQ"), (xdrproc_t) xdr_CAPI2_USimApi_IsAllowedAPN_Req_t, 0, 0},
	{ MSG_USIM_IS_APN_ALLOWED_RSP,_T("MSG_USIM_IS_APN_ALLOWED_RSP"), (xdrproc_t)xdr_CAPI2_USimApi_IsAllowedAPN_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_USIM_GET_NUM_APN_REQ,_T("MSG_USIM_GET_NUM_APN_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_USIM_GET_NUM_APN_RSP,_T("MSG_USIM_GET_NUM_APN_RSP"), (xdrproc_t)xdr_CAPI2_USimApi_GetNumOfAPN_Rsp_t, sizeof( UInt8 ), 0 },
	{ MSG_USIM_GET_APN_ENTRY_REQ,_T("MSG_USIM_GET_APN_ENTRY_REQ"), (xdrproc_t) xdr_CAPI2_USimApi_GetAPNEntry_Req_t, 0, 0},
	{ MSG_USIM_GET_APN_ENTRY_RSP,_T("MSG_USIM_GET_APN_ENTRY_RSP"), (xdrproc_t)xdr_CAPI2_USimApi_GetAPNEntry_Rsp_t, sizeof( CAPI2_USimApi_GetAPNEntry_Rsp_t ), 0 },
	{ MSG_USIM_IS_EST_SERV_ACTIVATED_REQ,_T("MSG_USIM_IS_EST_SERV_ACTIVATED_REQ"), (xdrproc_t) xdr_CAPI2_USimApi_IsEstServActivated_Req_t, 0, 0},
	{ MSG_USIM_IS_EST_SERV_ACTIVATED_RSP,_T("MSG_USIM_IS_EST_SERV_ACTIVATED_RSP"), (xdrproc_t)xdr_CAPI2_USimApi_IsEstServActivated_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SIM_SET_EST_SERV_REQ,_T("MSG_SIM_SET_EST_SERV_REQ"), (xdrproc_t) xdr_CAPI2_USimApi_SendSetEstServReq_Req_t, 0, 0},
	{ MSG_SIM_SET_EST_SERV_RSP,_T("MSG_SIM_SET_EST_SERV_RSP"), (xdrproc_t)xdr_CAPI2_USimApi_SendSetEstServReq_Rsp_t, sizeof( USIM_FILE_UPDATE_RSP_t ), 0 },
	{ MSG_SIM_UPDATE_ONE_APN_REQ,_T("MSG_SIM_UPDATE_ONE_APN_REQ"), (xdrproc_t) xdr_CAPI2_USimApi_SendWriteAPNReq_Req_t, 0, 0},
	{ MSG_SIM_UPDATE_ONE_APN_RSP,_T("MSG_SIM_UPDATE_ONE_APN_RSP"), (xdrproc_t)xdr_CAPI2_USimApi_SendWriteAPNReq_Rsp_t, sizeof( USIM_FILE_UPDATE_RSP_t ), 0 },
	{ MSG_SIM_DELETE_ALL_APN_REQ,_T("MSG_SIM_DELETE_ALL_APN_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_DELETE_ALL_APN_RSP,_T("MSG_SIM_DELETE_ALL_APN_RSP"), (xdrproc_t)xdr_CAPI2_USimApi_SendDeleteAllAPNReq_Rsp_t, sizeof( USIM_FILE_UPDATE_RSP_t ), 0 },
	{ MSG_USIM_GET_RAT_MODE_REQ,_T("MSG_USIM_GET_RAT_MODE_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_USIM_GET_RAT_MODE_RSP,_T("MSG_USIM_GET_RAT_MODE_RSP"), (xdrproc_t)xdr_CAPI2_USimApi_GetRatModeSetting_Rsp_t, sizeof( USIM_RAT_MODE_t ), 0 },
	{ MSG_MS_GET_GPRS_STATE_REQ,_T("MSG_MS_GET_GPRS_STATE_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_MS_GET_GPRS_STATE_RSP,_T("MSG_MS_GET_GPRS_STATE_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetGPRSRegState_Rsp_t, sizeof( MSRegState_t ), 0 },
	{ MSG_MS_GET_GSM_STATE_REQ,_T("MSG_MS_GET_GSM_STATE_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_MS_GET_GSM_STATE_RSP,_T("MSG_MS_GET_GSM_STATE_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetGSMRegState_Rsp_t, sizeof( MSRegState_t ), 0 },
	{ MSG_MS_GET_CELL_INFO_REQ,_T("MSG_MS_GET_CELL_INFO_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_MS_GET_CELL_INFO_RSP,_T("MSG_MS_GET_CELL_INFO_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetRegisteredCellInfo_Rsp_t, sizeof( CellInfo_t ), 0 },
	{ MSG_MS_SETMEPOWER_CLASS_REQ,_T("MSG_MS_SETMEPOWER_CLASS_REQ"), (xdrproc_t) xdr_CAPI2_MS_SetMEPowerClass_Req_t, 0, 0},
	{ MSG_MS_SETMEPOWER_CLASS_RSP,_T("MSG_MS_SETMEPOWER_CLASS_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_USIM_GET_SERVICE_STATUS_REQ,_T("MSG_USIM_GET_SERVICE_STATUS_REQ"), (xdrproc_t) xdr_CAPI2_USimApi_GetServiceStatus_Req_t, 0, 0},
	{ MSG_USIM_GET_SERVICE_STATUS_RSP,_T("MSG_USIM_GET_SERVICE_STATUS_RSP"), (xdrproc_t)xdr_CAPI2_USimApi_GetServiceStatus_Rsp_t, sizeof( SIMServiceStatus_t ), 0 },
	{ MSG_SIM_IS_ALLOWED_APN_REQ,_T("MSG_SIM_IS_ALLOWED_APN_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_IsAllowedAPN_Req_t, 0, 0},
	{ MSG_SIM_IS_ALLOWED_APN_RSP,_T("MSG_SIM_IS_ALLOWED_APN_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_IsAllowedAPN_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SMS_GETSMSMAXCAPACITY_REQ,_T("MSG_SMS_GETSMSMAXCAPACITY_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_GetSmsMaxCapacity_Req_t, 0, 0},
	{ MSG_SMS_GETSMSMAXCAPACITY_RSP,_T("MSG_SMS_GETSMSMAXCAPACITY_RSP"), (xdrproc_t)xdr_CAPI2_SmsApi_GetSmsMaxCapacity_Rsp_t, sizeof( UInt16 ), 0 },
	{ MSG_SMS_RETRIEVEMAXCBCHNLLENGTH_REQ,_T("MSG_SMS_RETRIEVEMAXCBCHNLLENGTH_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SMS_RETRIEVEMAXCBCHNLLENGTH_RSP,_T("MSG_SMS_RETRIEVEMAXCBCHNLLENGTH_RSP"), (xdrproc_t)xdr_CAPI2_SmsApi_RetrieveMaxCBChnlLength_Rsp_t, sizeof( CAPI2_SmsApi_RetrieveMaxCBChnlLength_Rsp_t ), 0 },
	{ MSG_SIM_IS_BDN_RESTRICTED_REQ,_T("MSG_SIM_IS_BDN_RESTRICTED_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_IS_BDN_RESTRICTED_RSP,_T("MSG_SIM_IS_BDN_RESTRICTED_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_IsBdnOperationRestricted_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SIM_SEND_PLMN_UPDATE_IND_REQ,_T("MSG_SIM_SEND_PLMN_UPDATE_IND_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SendPreferredPlmnUpdateInd_Req_t, 0, 0},
	{ MSG_SIM_SEND_PLMN_UPDATE_IND_RSP,_T("MSG_SIM_SEND_PLMN_UPDATE_IND_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SIMIO_DEACTIVATE_CARD_REQ,_T("MSG_SIMIO_DEACTIVATE_CARD_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIMIO_DEACTIVATE_CARD_RSP,_T("MSG_SIMIO_DEACTIVATE_CARD_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SIM_SET_BDN_REQ,_T("MSG_SIM_SET_BDN_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SendSetBdnReq_Req_t, 0, 0},
	{ MSG_SIM_SET_BDN_RSP,_T("MSG_SIM_SET_BDN_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_SendSetBdnReq_Rsp_t, sizeof( SIMAccess_t ), 0 },
	{ MSG_SIM_POWER_ON_OFF_CARD_REQ,_T("MSG_SIM_POWER_ON_OFF_CARD_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_PowerOnOffCard_Req_t, 0, 0},
	{ MSG_SIM_POWER_ON_OFF_CARD_RSP,_T("MSG_SIM_POWER_ON_OFF_CARD_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_PowerOnOffCard_Rsp_t, sizeof( SIM_GENERIC_APDU_RES_INFO_t ), 0 },
	{ MSG_SIM_GET_RAW_ATR_REQ,_T("MSG_SIM_GET_RAW_ATR_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_GET_RAW_ATR_RSP,_T("MSG_SIM_GET_RAW_ATR_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_GetRawAtr_Rsp_t, sizeof( SIM_GENERIC_APDU_ATR_INFO_t ), 0 },
	{ MSG_SIM_SET_PROTOCOL_REQ,_T("MSG_SIM_SET_PROTOCOL_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_Set_Protocol_Req_t, 0, 0},
	{ MSG_SIM_SET_PROTOCOL_RSP,_T("MSG_SIM_SET_PROTOCOL_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SIM_GET_PROTOCOL_REQ,_T("MSG_SIM_GET_PROTOCOL_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_GET_PROTOCOL_RSP,_T("MSG_SIM_GET_PROTOCOL_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_Get_Protocol_Rsp_t, sizeof( UInt8 ), 0 },
	{ MSG_SIM_SEND_GENERIC_APDU_CMD_REQ,_T("MSG_SIM_SEND_GENERIC_APDU_CMD_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SendGenericApduCmd_Req_t, 0, 0},
	{ MSG_SIM_SEND_GENERIC_APDU_CMD_RSP,_T("MSG_SIM_SEND_GENERIC_APDU_CMD_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_SendGenericApduCmd_Rsp_t, sizeof( SIM_GENERIC_APDU_XFER_RSP_t ), 0 },
	{ MSG_SIM_TERMINATE_XFER_APDU_REQ,_T("MSG_SIM_TERMINATE_XFER_APDU_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_TERMINATE_XFER_APDU_RSP,_T("MSG_SIM_TERMINATE_XFER_APDU_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SIM_GET_SIM_INTERFACE_REQ,_T("MSG_SIM_GET_SIM_INTERFACE_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_GET_SIM_INTERFACE_RSP,_T("MSG_SIM_GET_SIM_INTERFACE_RSP"), (xdrproc_t)xdr_CAPI2_SIM_GetSimInterface_Rsp_t, sizeof( SIM_SIM_INTERFACE_t ), 0 },
	{ MSG_SET_PLMN_SELECT_RAT_REQ,_T("MSG_SET_PLMN_SELECT_RAT_REQ"), (xdrproc_t) xdr_CAPI2_NetRegApi_SetPlmnSelectRat_Req_t, 0, 0},
	{ MSG_SET_PLMN_SELECT_RAT_RSP,_T("MSG_SET_PLMN_SELECT_RAT_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_IS_DEREGISTER_IN_PROGRESS_REQ,_T("MSG_IS_DEREGISTER_IN_PROGRESS_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_IS_DEREGISTER_IN_PROGRESS_RSP,_T("MSG_IS_DEREGISTER_IN_PROGRESS_RSP"), (xdrproc_t)xdr_CAPI2_MS_IsDeRegisterInProgress_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_IS_REGISTER_IN_PROGRESS_REQ,_T("MSG_IS_REGISTER_IN_PROGRESS_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_IS_REGISTER_IN_PROGRESS_RSP,_T("MSG_IS_REGISTER_IN_PROGRESS_RSP"), (xdrproc_t)xdr_CAPI2_MS_IsRegisterInProgress_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_READ_USIM_PBK_HDK_REQ,_T("MSG_READ_USIM_PBK_HDK_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_READ_USIM_PBK_HDK_RSP,_T("MSG_READ_USIM_PBK_HDK_RSP"), (xdrproc_t)xdr_CAPI2_PbkApi_SendUsimHdkReadReq_Rsp_t, sizeof( USIM_PBK_READ_HDK_ENTRY_RSP_t ), 0 },
	{ MSG_WRITE_USIM_PBK_HDK_REQ,_T("MSG_WRITE_USIM_PBK_HDK_REQ"), (xdrproc_t) xdr_CAPI2_PbkApi_SendUsimHdkUpdateReq_Req_t, 0, 0},
	{ MSG_WRITE_USIM_PBK_HDK_RSP,_T("MSG_WRITE_USIM_PBK_HDK_RSP"), (xdrproc_t)xdr_CAPI2_PbkApi_SendUsimHdkUpdateReq_Rsp_t, sizeof( USIM_PBK_UPDATE_HDK_ENTRY_RSP_t ), 0 },
	{ MSG_READ_USIM_PBK_ALPHA_AAS_REQ,_T("MSG_READ_USIM_PBK_ALPHA_AAS_REQ"), (xdrproc_t) xdr_CAPI2_PbkApi_SendUsimAasReadReq_Req_t, 0, 0},
	{ MSG_READ_USIM_PBK_ALPHA_AAS_RSP,_T("MSG_READ_USIM_PBK_ALPHA_AAS_RSP"), (xdrproc_t)xdr_CAPI2_PbkApi_SendUsimAasReadReq_Rsp_t, sizeof( USIM_PBK_READ_ALPHA_ENTRY_RSP_t ), 0 },
	{ MSG_WRITE_USIM_PBK_ALPHA_AAS_REQ,_T("MSG_WRITE_USIM_PBK_ALPHA_AAS_REQ"), (xdrproc_t) xdr_CAPI2_PbkApi_SendUsimAasUpdateReq_Req_t, 0, 0},
	{ MSG_WRITE_USIM_PBK_ALPHA_AAS_RSP,_T("MSG_WRITE_USIM_PBK_ALPHA_AAS_RSP"), (xdrproc_t)xdr_CAPI2_PbkApi_SendUsimAasUpdateReq_Rsp_t, sizeof( USIM_PBK_UPDATE_ALPHA_ENTRY_RSP_t ), 0 },
	{ MSG_READ_USIM_PBK_ALPHA_GAS_REQ,_T("MSG_READ_USIM_PBK_ALPHA_GAS_REQ"), (xdrproc_t) xdr_CAPI2_PbkApi_SendUsimGasReadReq_Req_t, 0, 0},
	{ MSG_WRITE_USIM_PBK_ALPHA_GAS_REQ,_T("MSG_WRITE_USIM_PBK_ALPHA_GAS_REQ"), (xdrproc_t) xdr_CAPI2_PbkApi_SendUsimGasUpdateReq_Req_t, 0, 0},
	{ MSG_GET_USIM_PBK_ALPHA_INFO_AAS_REQ,_T("MSG_GET_USIM_PBK_ALPHA_INFO_AAS_REQ"), (xdrproc_t) xdr_CAPI2_PbkApi_SendUsimAasInfoReq_Req_t, 0, 0},
	{ MSG_GET_USIM_PBK_ALPHA_INFO_AAS_RSP,_T("MSG_GET_USIM_PBK_ALPHA_INFO_AAS_RSP"), (xdrproc_t)xdr_CAPI2_PbkApi_SendUsimAasInfoReq_Rsp_t, sizeof( USIM_PBK_ALPHA_INFO_RSP_t ), 0 },
	{ MSG_GET_USIM_PBK_ALPHA_INFO_GAS_REQ,_T("MSG_GET_USIM_PBK_ALPHA_INFO_GAS_REQ"), (xdrproc_t) xdr_CAPI2_PbkApi_SendUsimGasInfoReq_Req_t, 0, 0},
	{ MSG_GET_USIM_PBK_ALPHA_INFO_GAS_RSP,_T("MSG_GET_USIM_PBK_ALPHA_INFO_GAS_RSP"), (xdrproc_t)xdr_CAPI2_PbkApi_SendUsimGasInfoReq_Rsp_t, sizeof( USIM_PBK_ALPHA_INFO_RSP_t ), 0 },
	{ MSG_MEASURE_REPORT_PARAM_IND,_T("MSG_MEASURE_REPORT_PARAM_IND"), (xdrproc_t)xdr_CAPI2_MS_MeasureReportInd_Rsp_t, sizeof( MS_RxTestParam_t ), 4096 },
	{ MSG_DIAG_MEASURE_REPORT_REQ,_T("MSG_DIAG_MEASURE_REPORT_REQ"), (xdrproc_t) xdr_CAPI2_DiagApi_MeasurmentReportReq_Req_t, 0, 0},
	{ MSG_DIAG_MEASURE_REPORT_RSP,_T("MSG_DIAG_MEASURE_REPORT_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_MS_INITCALLCFG_REQ,_T("MSG_MS_INITCALLCFG_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_MS_INITCALLCFG_RSP,_T("MSG_MS_INITCALLCFG_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_MS_INITFAXCFG_REQ,_T("MSG_MS_INITFAXCFG_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_MS_INITFAXCFG_RSP,_T("MSG_MS_INITFAXCFG_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_MS_INITVIDEOCALLCFG_REQ,_T("MSG_MS_INITVIDEOCALLCFG_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_MS_INITVIDEOCALLCFG_RSP,_T("MSG_MS_INITVIDEOCALLCFG_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_MS_INITCALLCFGAMPF_REQ,_T("MSG_MS_INITCALLCFGAMPF_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_MS_INITCALLCFGAMPF_RSP,_T("MSG_MS_INITCALLCFGAMPF_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_MS_STATUS_IND,_T("MSG_MS_STATUS_IND"), (xdrproc_t)xdr_CAPI2_MS_StatusInd_Rsp_t, sizeof( MS_StatusIndication_t ), 0 },
	{ MSG_POWER_DOWN_CNF,_T("MSG_POWER_DOWN_CNF"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SYSPARM_GET_ACTUAL_LOW_VOLT_REQ,_T("MSG_SYSPARM_GET_ACTUAL_LOW_VOLT_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SYSPARM_GET_ACTUAL_LOW_VOLT_RSP,_T("MSG_SYSPARM_GET_ACTUAL_LOW_VOLT_RSP"), (xdrproc_t)xdr_CAPI2_SYSPARM_GetActualLowVoltReading_Rsp_t, sizeof( UInt16 ), 0 },
	{ MSG_SYSPARM_GET_ACTUAL_4P2_VOLT_REQ,_T("MSG_SYSPARM_GET_ACTUAL_4P2_VOLT_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SYSPARM_GET_ACTUAL_4P2_VOLT_RSP,_T("MSG_SYSPARM_GET_ACTUAL_4P2_VOLT_RSP"), (xdrproc_t)xdr_CAPI2_SYSPARM_GetActual4p2VoltReading_Rsp_t, sizeof( UInt16 ), 0 },
	{ MSG_SMS_SEND_COMMAND_TXT_REQ,_T("MSG_SMS_SEND_COMMAND_TXT_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_SendSMSCommandTxtReq_Req_t, 0, 0},
	{ MSG_SMS_SEND_COMMAND_TXT_RSP,_T("MSG_SMS_SEND_COMMAND_TXT_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_SEND_COMMAND_PDU_REQ,_T("MSG_SMS_SEND_COMMAND_PDU_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_SendSMSCommandPduReq_Req_t, 0, 0},
	{ MSG_SMS_SEND_COMMAND_PDU_RSP,_T("MSG_SMS_SEND_COMMAND_PDU_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_SEND_ACKTONETWORK_PDU_REQ,_T("MSG_SMS_SEND_ACKTONETWORK_PDU_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_SendPDUAckToNetwork_Req_t, 0, 0},
	{ MSG_SMS_SEND_ACKTONETWORK_PDU_RSP,_T("MSG_SMS_SEND_ACKTONETWORK_PDU_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_START_CB_WITHCHNL_REQ,_T("MSG_SMS_START_CB_WITHCHNL_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_StartCellBroadcastWithChnlReq_Req_t, 0, 0},
	{ MSG_SMS_START_CB_WITHCHNL_RSP,_T("MSG_SMS_START_CB_WITHCHNL_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_SET_TPMR_REQ,_T("MSG_SMS_SET_TPMR_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_SetMoSmsTpMr_Req_t, 0, 0},
	{ MSG_SMS_SET_TPMR_RSP,_T("MSG_SMS_SET_TPMR_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SIMLOCK_SET_STATUS_REQ,_T("MSG_SIMLOCK_SET_STATUS_REQ"), (xdrproc_t) xdr_CAPI2_SIMLOCKApi_SetStatus_Req_t, 0, 0},
	{ MSG_SIMLOCK_SET_STATUS_RSP,_T("MSG_SIMLOCK_SET_STATUS_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_UE_3G_STATUS_IND,_T("MSG_UE_3G_STATUS_IND"), (xdrproc_t)xdr_CAPI2_MS_Ue3gStatusInd_Rsp_t, sizeof( MSUe3gStatusInd_t ), 0 },
	{ MSG_DIAG_CELLLOCK_REQ,_T("MSG_DIAG_CELLLOCK_REQ"), (xdrproc_t) xdr_CAPI2_DIAG_ApiCellLockReq_Req_t, 0, 0},
	{ MSG_DIAG_CELLLOCK_RSP,_T("MSG_DIAG_CELLLOCK_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_DIAG_CELLLOCK_STATUS_REQ,_T("MSG_DIAG_CELLLOCK_STATUS_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_DIAG_CELLLOCK_STATUS_RSP,_T("MSG_DIAG_CELLLOCK_STATUS_RSP"), (xdrproc_t)xdr_CAPI2_DIAG_ApiCellLockStatus_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_MS_SET_RUA_READY_TIMER_REQ,_T("MSG_MS_SET_RUA_READY_TIMER_REQ"), (xdrproc_t) xdr_CAPI2_MS_SetRuaReadyTimer_Req_t, 0, 0},
	{ MSG_MS_SET_RUA_READY_TIMER_RSP,_T("MSG_MS_SET_RUA_READY_TIMER_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_LCS_RRLP_DATA_IND,_T("MSG_LCS_RRLP_DATA_IND"), (xdrproc_t)xdr_CAPI2_LCS_RrlpDataInd_Rsp_t, sizeof( LcsMsgData_t ), 0 },
	{ MSG_LCS_RRLP_RESET_POS_STORED_INFO_IND,_T("MSG_LCS_RRLP_RESET_POS_STORED_INFO_IND"), (xdrproc_t)xdr_CAPI2_LCS_RrlpReset_Rsp_t, sizeof( ClientInfo_t ), 0 },
	{ MSG_LCS_RRC_ASSISTANCE_DATA_IND,_T("MSG_LCS_RRC_ASSISTANCE_DATA_IND"), (xdrproc_t)xdr_CAPI2_LCS_RrcAssistDataInd_Rsp_t, sizeof( LcsMsgData_t ), 0 },
	{ MSG_LCS_RRC_MEASUREMENT_CTRL_IND,_T("MSG_LCS_RRC_MEASUREMENT_CTRL_IND"), (xdrproc_t)xdr_CAPI2_LCS_RrcMeasCtrlInd_Rsp_t, sizeof( LcsRrcMeasurement_t ), 0 },
	{ MSG_LCS_RRC_BROADCAST_SYS_INFO_IND,_T("MSG_LCS_RRC_BROADCAST_SYS_INFO_IND"), (xdrproc_t)xdr_CAPI2_LCS_RrcSysInfoInd_Rsp_t, sizeof( LcsRrcBroadcastSysInfo_t ), 0 },
	{ MSG_LCS_RRC_UE_STATE_IND,_T("MSG_LCS_RRC_UE_STATE_IND"), (xdrproc_t)xdr_CAPI2_LCS_RrcUeStateInd_Rsp_t, sizeof( LcsRrcUeState_t ), 0 },
	{ MSG_LCS_RRC_STOP_MEASUREMENT_IND,_T("MSG_LCS_RRC_STOP_MEASUREMENT_IND"), (xdrproc_t)xdr_CAPI2_LCS_RrcStopMeasInd_Rsp_t, sizeof( ClientInfo_t ), 0 },
	{ MSG_LCS_RRC_RESET_POS_STORED_INFO_IND,_T("MSG_LCS_RRC_RESET_POS_STORED_INFO_IND"), (xdrproc_t)xdr_CAPI2_LCS_RrcReset_Rsp_t, sizeof( ClientInfo_t ), 0 },
	{ MSG_CC_IS_THERE_EMERGENCY_CALL_REQ,_T("MSG_CC_IS_THERE_EMERGENCY_CALL_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_CC_IS_THERE_EMERGENCY_CALL_RSP,_T("MSG_CC_IS_THERE_EMERGENCY_CALL_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_IsThereEmergencyCall_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SERVING_CELL_INFO_IND,_T("MSG_SERVING_CELL_INFO_IND"), (xdrproc_t)xdr_CAPI2_ServingCellInfoInd_Rsp_t, sizeof( ServingCellInfo_t ), 10000 },
	{ MSG_SYS_ENABLE_CELL_INFO_REQ,_T("MSG_SYS_ENABLE_CELL_INFO_REQ"), (xdrproc_t) xdr_CAPI2_SYS_EnableCellInfoMsg_Req_t, 0, 0},
	{ MSG_SYS_ENABLE_CELL_INFO_RSP,_T("MSG_SYS_ENABLE_CELL_INFO_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_L1_BB_ISLOCKED_REQ,_T("MSG_L1_BB_ISLOCKED_REQ"), (xdrproc_t) xdr_CAPI2_LCS_L1_bb_isLocked_Req_t, 0, 0},
	{ MSG_L1_BB_ISLOCKED_RSP,_T("MSG_L1_BB_ISLOCKED_RSP"), (xdrproc_t)xdr_CAPI2_LCS_L1_bb_isLocked_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SMSPP_APP_SPECIFIC_SMS_IND,_T("MSG_SMSPP_APP_SPECIFIC_SMS_IND"), (xdrproc_t)xdr_CAPI2_SMSPP_AppSpecificInd_Rsp_t, sizeof( SmsAppSpecificData_t ), 0 },
	{ MSG_UTIL_DIAL_STR_PARSE_REQ,_T("MSG_UTIL_DIAL_STR_PARSE_REQ"), (xdrproc_t) xdr_CAPI2_DIALSTR_ParseGetCallType_Req_t, 0, 0},
	{ MSG_UTIL_DIAL_STR_PARSE_RSP,_T("MSG_UTIL_DIAL_STR_PARSE_RSP"), (xdrproc_t)xdr_CAPI2_DIALSTR_ParseGetCallType_Rsp_t, sizeof( CallType_t ), 0 },
	{ MSG_LCS_FTT_DELTA_TIME_REQ,_T("MSG_LCS_FTT_DELTA_TIME_REQ"), (xdrproc_t) xdr_CAPI2_LCS_FttCalcDeltaTime_Req_t, 0, 0},
	{ MSG_LCS_FTT_DELTA_TIME_RSP,_T("MSG_LCS_FTT_DELTA_TIME_RSP"), (xdrproc_t)xdr_CAPI2_LCS_FttCalcDeltaTime_Rsp_t, sizeof( UInt32 ), 0 },
	{ MSG_LCS_FTT_SYNC_RESULT_IND,_T("MSG_LCS_FTT_SYNC_RESULT_IND"), (xdrproc_t)xdr_CAPI2_LCS_SyncResult_Rsp_t, sizeof( LcsFttResult_t ), 0 },
	{ MSG_MS_FORCEDREADYSTATE_REQ,_T("MSG_MS_FORCEDREADYSTATE_REQ"), (xdrproc_t) xdr_CAPI2_NetRegApi_ForcedReadyStateReq_Req_t, 0, 0},
	{ MSG_MS_FORCEDREADYSTATE_RSP,_T("MSG_MS_FORCEDREADYSTATE_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SS_RESETSSALSFLAG_REQ,_T("MSG_SS_RESETSSALSFLAG_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SS_RESETSSALSFLAG_RSP,_T("MSG_SS_RESETSSALSFLAG_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SIMLOCK_GET_STATUS_REQ,_T("MSG_SIMLOCK_GET_STATUS_REQ"), (xdrproc_t) xdr_CAPI2_SIMLOCK_GetStatus_Req_t, 0, 0},
	{ MSG_SIMLOCK_GET_STATUS_RSP,_T("MSG_SIMLOCK_GET_STATUS_RSP"), (xdrproc_t)xdr_CAPI2_SIMLOCK_GetStatus_RSP_Rsp_t, sizeof( CAPI2_SIMLOCK_GetStatus_RSP_Rsp_t ), 0 },
	{ MSG_DIALSTR_IS_VALID_REQ,_T("MSG_DIALSTR_IS_VALID_REQ"), (xdrproc_t) xdr_CAPI2_DIALSTR_IsValidString_Req_t, 0, 0},
	{ MSG_DIALSTR_IS_VALID_RSP,_T("MSG_DIALSTR_IS_VALID_RSP"), (xdrproc_t)xdr_CAPI2_DIALSTR_IsValidString_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_UTIL_CONVERT_NTWK_CAUSE_REQ,_T("MSG_UTIL_CONVERT_NTWK_CAUSE_REQ"), (xdrproc_t) xdr_CAPI2_UTIL_Cause2NetworkCause_Req_t, 0, 0},
	{ MSG_UTIL_CONVERT_NTWK_CAUSE_RSP,_T("MSG_UTIL_CONVERT_NTWK_CAUSE_RSP"), (xdrproc_t)xdr_CAPI2_UTIL_Cause2NetworkCause_Rsp_t, sizeof( NetworkCause_t ), 0 },
	{ MSG_UTIL_CONVERT_ERRCODE_NTWK_CAUSE_REQ,_T("MSG_UTIL_CONVERT_ERRCODE_NTWK_CAUSE_REQ"), (xdrproc_t) xdr_CAPI2_UTIL_ErrCodeToNetCause_Req_t, 0, 0},
	{ MSG_UTIL_CONVERT_ERRCODE_NTWK_CAUSE_RSP,_T("MSG_UTIL_CONVERT_ERRCODE_NTWK_CAUSE_RSP"), (xdrproc_t)xdr_CAPI2_UTIL_ErrCodeToNetCause_Rsp_t, sizeof( NetworkCause_t ), 0 },
	{ MSG_ISGPRS_DIAL_STR_REQ,_T("MSG_ISGPRS_DIAL_STR_REQ"), (xdrproc_t) xdr_CAPI2_IsGprsDialStr_Req_t, 0, 0},
	{ MSG_ISGPRS_DIAL_STR_RSP,_T("MSG_ISGPRS_DIAL_STR_RSP"), (xdrproc_t)xdr_CAPI2_IsGprsDialStr_Rsp_t, sizeof( CAPI2_IsGprsDialStr_Rsp_t ), 0 },
	{ MSG_GET_NUM_SS_STR_REQ,_T("MSG_GET_NUM_SS_STR_REQ"), (xdrproc_t) xdr_CAPI2_UTIL_GetNumOffsetInSsStr_Req_t, 0, 0},
	{ MSG_GET_NUM_SS_STR_RSP,_T("MSG_GET_NUM_SS_STR_RSP"), (xdrproc_t)xdr_CAPI2_UTIL_GetNumOffsetInSsStr_Rsp_t, sizeof( UInt8 ), 0 },
	{ MSG_CIPHER_ALG_IND,_T("MSG_CIPHER_ALG_IND"), (xdrproc_t)xdr_CAPI2_CipherAlgInd_Rsp_t, sizeof( PhonectrlCipherAlg_t ), 0 },
	{ MSG_NW_MEAS_RESULT_IND,_T("MSG_NW_MEAS_RESULT_IND"), (xdrproc_t)xdr_CAPI2_NWMeasResultInd_Rsp_t, sizeof( Phonectrl_Nmr_t ), 0 },
	{ MSG_DIALSTR_IS_PPPLOOPBACK_REQ,_T("MSG_DIALSTR_IS_PPPLOOPBACK_REQ"), (xdrproc_t) xdr_CAPI2_IsPppLoopbackDialStr_Req_t, 0, 0},
	{ MSG_DIALSTR_IS_PPPLOOPBACK_RSP,_T("MSG_DIALSTR_IS_PPPLOOPBACK_RSP"), (xdrproc_t)xdr_CAPI2_IsPppLoopbackDialStr_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SYS_GETRIPPROCVERSION_REQ,_T("MSG_SYS_GETRIPPROCVERSION_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SYS_GETRIPPROCVERSION_RSP,_T("MSG_SYS_GETRIPPROCVERSION_RSP"), (xdrproc_t)xdr_CAPI2_SYS_GetRIPPROCVersion_Rsp_t, sizeof( UInt8 ), 0 },
	{ MSG_SYSPARM_SET_HSDPA_PHY_CAT_REQ,_T("MSG_SYSPARM_SET_HSDPA_PHY_CAT_REQ"), (xdrproc_t) xdr_CAPI2_SYSPARM_SetHSDPAPHYCategory_Req_t, 0, 0},
	{ MSG_SYSPARM_SET_HSDPA_PHY_CAT_RSP,_T("MSG_SYSPARM_SET_HSDPA_PHY_CAT_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SYS_GET_HSDPA_CATEGORY_REQ,_T("MSG_SYS_GET_HSDPA_CATEGORY_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SYS_GET_HSDPA_CATEGORY_RSP,_T("MSG_SYS_GET_HSDPA_CATEGORY_RSP"), (xdrproc_t)xdr_CAPI2_SYSPARM_GetHSDPAPHYCategory_Rsp_t, sizeof( UInt8 ), 0 },
	{ MSG_SMS_CONVERT_MSGTYPE_REQ,_T("MSG_SMS_CONVERT_MSGTYPE_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_ConvertSmsMSMsgType_Req_t, 0, 0},
	{ MSG_SMS_CONVERT_MSGTYPE_RSP,_T("MSG_SMS_CONVERT_MSGTYPE_RSP"), (xdrproc_t)xdr_CAPI2_SmsApi_ConvertSmsMSMsgType_Rsp_t, sizeof( UInt8 ), 0 },
	{ MSG_NET_UARFCN_DL_IND,_T("MSG_NET_UARFCN_DL_IND"), (xdrproc_t)xdr_CAPI2_NetUarfcnDlInd_Rsp_t, sizeof( UInt16 ), 0 },
	{ MSG_MS_GETPREFNETSTATUS_REQ,_T("MSG_MS_GETPREFNETSTATUS_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_MS_GETPREFNETSTATUS_RSP,_T("MSG_MS_GETPREFNETSTATUS_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetPrefNetStatus_Rsp_t, sizeof( GANStatus_t ), 0 },
	{ MSG_SYS_TEST_MSG_IND_1,_T("MSG_SYS_TEST_MSG_IND_1"), (xdrproc_t)xdr_SYS_TestSysMsgs1_Rsp_t, sizeof( uchar_ptr_t ), 0 },
	{ MSG_SYS_TEST_MSG_IND_2,_T("MSG_SYS_TEST_MSG_IND_2"), (xdrproc_t)xdr_SYS_TestSysMsgs2_Rsp_t, sizeof( uchar_ptr_t ), 0 },
	{ MSG_SYS_TEST_MSG_IND_3,_T("MSG_SYS_TEST_MSG_IND_3"), (xdrproc_t)xdr_SYS_TestSysMsgs3_Rsp_t, sizeof( uchar_ptr_t ), 0 },
	{ MSG_SYS_TEST_MSG_IND_4,_T("MSG_SYS_TEST_MSG_IND_4"), (xdrproc_t)xdr_SYS_TestSysMsgs4_Rsp_t, sizeof( uchar_ptr_t ), 0 },
	{ MSG_SYS_TEST_MSG_IND_5,_T("MSG_SYS_TEST_MSG_IND_5"), (xdrproc_t)xdr_SYS_TestSysMsgs5_Rsp_t, sizeof( uchar_ptr_t ), 0 },
	{ MSG_SYS_TEST_MSG_IND_6,_T("MSG_SYS_TEST_MSG_IND_6"), (xdrproc_t)xdr_SYS_TestSysMsgs6_Rsp_t, sizeof( uchar_ptr_t ), 0 },
	{ MSG_SYSPARM_SET_HSUPA_PHY_CAT_REQ,_T("MSG_SYSPARM_SET_HSUPA_PHY_CAT_REQ"), (xdrproc_t) xdr_CAPI2_SYSPARM_SetHSUPAPHYCategory_Req_t, 0, 0},
	{ MSG_SYSPARM_SET_HSUPA_PHY_CAT_RSP,_T("MSG_SYSPARM_SET_HSUPA_PHY_CAT_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SYSPARM_GET_HSUPA_PHY_CAT_REQ,_T("MSG_SYSPARM_GET_HSUPA_PHY_CAT_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SYSPARM_GET_HSUPA_PHY_CAT_RSP,_T("MSG_SYSPARM_GET_HSUPA_PHY_CAT_RSP"), (xdrproc_t)xdr_CAPI2_SYSPARM_GetHSUPAPHYCategory_Rsp_t, sizeof( UInt8 ), 0 },
	{ MSG_INTERTASK_MSG_TO_CP_REQ,_T("MSG_INTERTASK_MSG_TO_CP_REQ"), (xdrproc_t) xdr_CAPI2_InterTaskMsgToCP_Req_t, 0, 0},
	{ MSG_INTERTASK_MSG_TO_CP_RSP,_T("MSG_INTERTASK_MSG_TO_CP_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_INTERTASK_MSG_TO_AP_REQ,_T("MSG_INTERTASK_MSG_TO_AP_REQ"), (xdrproc_t) xdr_CAPI2_InterTaskMsgToAP_Req_t, 0, 0},
	{ MSG_CC_GETCURRENTCALLINDEX_REQ,_T("MSG_CC_GETCURRENTCALLINDEX_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_CC_GETCURRENTCALLINDEX_RSP,_T("MSG_CC_GETCURRENTCALLINDEX_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetCurrentCallIndex_Rsp_t, sizeof( UInt8 ), 0 },
	{ MSG_CC_GETNEXTACTIVECALLINDEX_REQ,_T("MSG_CC_GETNEXTACTIVECALLINDEX_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_CC_GETNEXTACTIVECALLINDEX_RSP,_T("MSG_CC_GETNEXTACTIVECALLINDEX_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetNextActiveCallIndex_Rsp_t, sizeof( UInt8 ), 0 },
	{ MSG_CC_GETNEXTHELDCALLINDEX_REQ,_T("MSG_CC_GETNEXTHELDCALLINDEX_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_CC_GETNEXTHELDCALLINDEX_RSP,_T("MSG_CC_GETNEXTHELDCALLINDEX_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetNextHeldCallIndex_Rsp_t, sizeof( UInt8 ), 0 },
	{ MSG_CC_GETNEXTWAITCALLINDEX_REQ,_T("MSG_CC_GETNEXTWAITCALLINDEX_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_CC_GETNEXTWAITCALLINDEX_RSP,_T("MSG_CC_GETNEXTWAITCALLINDEX_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetNextWaitCallIndex_Rsp_t, sizeof( UInt8 ), 0 },
	{ MSG_CC_GETMPTYCALLINDEX_REQ,_T("MSG_CC_GETMPTYCALLINDEX_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_CC_GETMPTYCALLINDEX_RSP,_T("MSG_CC_GETMPTYCALLINDEX_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetMPTYCallIndex_Rsp_t, sizeof( UInt8 ), 0 },
	{ MSG_CC_GETCALLSTATE_REQ,_T("MSG_CC_GETCALLSTATE_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_GetCallState_Req_t, 0, 0},
	{ MSG_CC_GETCALLSTATE_RSP,_T("MSG_CC_GETCALLSTATE_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetCallState_Rsp_t, sizeof( CCallState_t ), 0 },
	{ MSG_CC_GETCALLTYPE_REQ,_T("MSG_CC_GETCALLTYPE_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_GetCallType_Req_t, 0, 0},
	{ MSG_CC_GETCALLTYPE_RSP,_T("MSG_CC_GETCALLTYPE_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetCallType_Rsp_t, sizeof( CCallType_t ), 0 },
	{ MSG_CC_GETLASTCALLEXITCAUSE_REQ,_T("MSG_CC_GETLASTCALLEXITCAUSE_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_CC_GETLASTCALLEXITCAUSE_RSP,_T("MSG_CC_GETLASTCALLEXITCAUSE_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetLastCallExitCause_Rsp_t, sizeof( Cause_t ), 0 },
	{ MSG_CC_GETCALLNUMBER_REQ,_T("MSG_CC_GETCALLNUMBER_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_GetCallNumber_Req_t, 0, 0},
	{ MSG_CC_GETCALLNUMBER_RSP,_T("MSG_CC_GETCALLNUMBER_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetCallNumber_Rsp_t, sizeof( CAPI2_CcApi_GetCallNumber_Rsp_t ), 0 },
	{ MSG_CC_GETCALLINGINFO_REQ,_T("MSG_CC_GETCALLINGINFO_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_GetCallingInfo_Req_t, 0, 0},
	{ MSG_CC_GETCALLINGINFO_RSP,_T("MSG_CC_GETCALLINGINFO_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetCallingInfo_Rsp_t, sizeof( CAPI2_CcApi_GetCallingInfo_Rsp_t ), 0 },
	{ MSG_CC_GETALLCALLSTATES_REQ,_T("MSG_CC_GETALLCALLSTATES_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_CC_GETALLCALLSTATES_RSP,_T("MSG_CC_GETALLCALLSTATES_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetAllCallStates_Rsp_t, sizeof( CAPI2_CcApi_GetAllCallStates_Rsp_t ), 0 },
	{ MSG_CC_GETALLCALLINDEX_REQ,_T("MSG_CC_GETALLCALLINDEX_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_CC_GETALLCALLINDEX_RSP,_T("MSG_CC_GETALLCALLINDEX_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetAllCallIndex_Rsp_t, sizeof( CAPI2_CcApi_GetAllCallIndex_Rsp_t ), 0 },
	{ MSG_CC_GETALLHELDCALLINDEX_REQ,_T("MSG_CC_GETALLHELDCALLINDEX_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_CC_GETALLHELDCALLINDEX_RSP,_T("MSG_CC_GETALLHELDCALLINDEX_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetAllHeldCallIndex_Rsp_t, sizeof( CAPI2_CcApi_GetAllHeldCallIndex_Rsp_t ), 0 },
	{ MSG_CC_GETALLACTIVECALLINDEX_REQ,_T("MSG_CC_GETALLACTIVECALLINDEX_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_CC_GETALLACTIVECALLINDEX_RSP,_T("MSG_CC_GETALLACTIVECALLINDEX_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetAllActiveCallIndex_Rsp_t, sizeof( CAPI2_CcApi_GetAllActiveCallIndex_Rsp_t ), 0 },
	{ MSG_CC_GETALLMPTYCALLINDEX_REQ,_T("MSG_CC_GETALLMPTYCALLINDEX_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_CC_GETALLMPTYCALLINDEX_RSP,_T("MSG_CC_GETALLMPTYCALLINDEX_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetAllMPTYCallIndex_Rsp_t, sizeof( CAPI2_CcApi_GetAllMPTYCallIndex_Rsp_t ), 0 },
	{ MSG_CC_GETNUMOFMPTYCALLS_REQ,_T("MSG_CC_GETNUMOFMPTYCALLS_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_CC_GETNUMOFMPTYCALLS_RSP,_T("MSG_CC_GETNUMOFMPTYCALLS_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetNumOfMPTYCalls_Rsp_t, sizeof( UInt8 ), 0 },
	{ MSG_CC_GETNUMOFACTIVECALLS_REQ,_T("MSG_CC_GETNUMOFACTIVECALLS_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_CC_GETNUMOFACTIVECALLS_RSP,_T("MSG_CC_GETNUMOFACTIVECALLS_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetNumofActiveCalls_Rsp_t, sizeof( UInt8 ), 0 },
	{ MSG_CC_GETNUMOFHELDCALLS_REQ,_T("MSG_CC_GETNUMOFHELDCALLS_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_CC_GETNUMOFHELDCALLS_RSP,_T("MSG_CC_GETNUMOFHELDCALLS_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetNumofHeldCalls_Rsp_t, sizeof( UInt8 ), 0 },
	{ MSG_CC_ISTHEREWAITINGCALL_REQ,_T("MSG_CC_ISTHEREWAITINGCALL_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_CC_ISTHEREWAITINGCALL_RSP,_T("MSG_CC_ISTHEREWAITINGCALL_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_IsThereWaitingCall_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_CC_ISTHEREALERTINGCALL_REQ,_T("MSG_CC_ISTHEREALERTINGCALL_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_CC_ISTHEREALERTINGCALL_RSP,_T("MSG_CC_ISTHEREALERTINGCALL_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_IsThereAlertingCall_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_CC_GETCONNECTEDLINEID_REQ,_T("MSG_CC_GETCONNECTEDLINEID_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_GetConnectedLineID_Req_t, 0, 0},
	{ MSG_CC_GETCONNECTEDLINEID_RSP,_T("MSG_CC_GETCONNECTEDLINEID_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetConnectedLineID_Rsp_t, sizeof( UInt8 ), 0 },
	{ MSG_CC_GET_CALL_PRESENT_REQ,_T("MSG_CC_GET_CALL_PRESENT_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_GetCallPresent_Req_t, 0, 0},
	{ MSG_CC_GET_CALL_PRESENT_RSP,_T("MSG_CC_GET_CALL_PRESENT_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetCallPresent_Rsp_t, sizeof( CAPI2_CcApi_GetCallPresent_Rsp_t ), 0 },
	{ MSG_CC_GET_INDEX_STATE_REQ,_T("MSG_CC_GET_INDEX_STATE_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_GetCallIndexInThisState_Req_t, 0, 0},
	{ MSG_CC_GET_INDEX_STATE_RSP,_T("MSG_CC_GET_INDEX_STATE_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetCallIndexInThisState_Rsp_t, sizeof( CAPI2_CcApi_GetCallIndexInThisState_Rsp_t ), 0 },
	{ MSG_CC_ISMULTIPARTYCALL_REQ,_T("MSG_CC_ISMULTIPARTYCALL_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_IsMultiPartyCall_Req_t, 0, 0},
	{ MSG_CC_ISMULTIPARTYCALL_RSP,_T("MSG_CC_ISMULTIPARTYCALL_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_IsMultiPartyCall_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_CC_ISTHEREVOICECALL_REQ,_T("MSG_CC_ISTHEREVOICECALL_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_CC_ISTHEREVOICECALL_RSP,_T("MSG_CC_ISTHEREVOICECALL_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_IsThereVoiceCall_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_REQ,_T("MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_IsConnectedLineIDPresentAllowed_Req_t, 0, 0},
	{ MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_RSP,_T("MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_IsConnectedLineIDPresentAllowed_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_REQ,_T("MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_GetCurrentCallDurationInMilliSeconds_Req_t, 0, 0},
	{ MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_RSP,_T("MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetCurrentCallDurationInMilliSeconds_Rsp_t, sizeof( UInt32 ), 0 },
	{ MSG_CC_GETLASTCALLCCM_REQ,_T("MSG_CC_GETLASTCALLCCM_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_CC_GETLASTCALLCCM_RSP,_T("MSG_CC_GETLASTCALLCCM_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetLastCallCCM_Rsp_t, sizeof( UInt32 ), 0 },
	{ MSG_CC_GETLASTCALLDURATION_REQ,_T("MSG_CC_GETLASTCALLDURATION_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_CC_GETLASTCALLDURATION_RSP,_T("MSG_CC_GETLASTCALLDURATION_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetLastCallDuration_Rsp_t, sizeof( UInt32 ), 0 },
	{ MSG_CC_GETLASTDATACALLRXBYTES_REQ,_T("MSG_CC_GETLASTDATACALLRXBYTES_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_CC_GETLASTDATACALLRXBYTES_RSP,_T("MSG_CC_GETLASTDATACALLRXBYTES_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetLastDataCallRxBytes_Rsp_t, sizeof( UInt32 ), 0 },
	{ MSG_CC_GETLASTDATACALLTXBYTES_REQ,_T("MSG_CC_GETLASTDATACALLTXBYTES_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_CC_GETLASTDATACALLTXBYTES_RSP,_T("MSG_CC_GETLASTDATACALLTXBYTES_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetLastDataCallTxBytes_Rsp_t, sizeof( UInt32 ), 0 },
	{ MSG_CC_GETDATACALLINDEX_REQ,_T("MSG_CC_GETDATACALLINDEX_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_CC_GETDATACALLINDEX_RSP,_T("MSG_CC_GETDATACALLINDEX_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetDataCallIndex_Rsp_t, sizeof( UInt8 ), 0 },
	{ MSG_CC_GETCALLCLIENT_INFO_REQ,_T("MSG_CC_GETCALLCLIENT_INFO_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_GetCallClientInfo_Req_t, 0, 0},
	{ MSG_CC_GETCALLCLIENT_INFO_RSP,_T("MSG_CC_GETCALLCLIENT_INFO_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetCallClientInfo_Rsp_t, sizeof( CAPI2_CcApi_GetCallClientInfo_Rsp_t ), 0 },
	{ MSG_CC_GETCALLCLIENTID_REQ,_T("MSG_CC_GETCALLCLIENTID_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_GetCallClientID_Req_t, 0, 0},
	{ MSG_CC_GETCALLCLIENTID_RSP,_T("MSG_CC_GETCALLCLIENTID_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetCallClientID_Rsp_t, sizeof( UInt8 ), 0 },
	{ MSG_CC_GETTYPEADD_REQ,_T("MSG_CC_GETTYPEADD_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_GetTypeAdd_Req_t, 0, 0},
	{ MSG_CC_GETTYPEADD_RSP,_T("MSG_CC_GETTYPEADD_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetTypeAdd_Rsp_t, sizeof( UInt8 ), 0 },
	{ MSG_CC_SETVOICECALLAUTOREJECT_REQ,_T("MSG_CC_SETVOICECALLAUTOREJECT_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_SetVoiceCallAutoReject_Req_t, 0, 0},
	{ MSG_CC_SETVOICECALLAUTOREJECT_RSP,_T("MSG_CC_SETVOICECALLAUTOREJECT_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_CC_ISVOICECALLAUTOREJECT_REQ,_T("MSG_CC_ISVOICECALLAUTOREJECT_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_CC_ISVOICECALLAUTOREJECT_RSP,_T("MSG_CC_ISVOICECALLAUTOREJECT_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_IsVoiceCallAutoReject_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_CC_SETTTYCALL_REQ,_T("MSG_CC_SETTTYCALL_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_SetTTYCall_Req_t, 0, 0},
	{ MSG_CC_SETTTYCALL_RSP,_T("MSG_CC_SETTTYCALL_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_CC_ISTTYENABLE_REQ,_T("MSG_CC_ISTTYENABLE_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_CC_ISTTYENABLE_RSP,_T("MSG_CC_ISTTYENABLE_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_IsTTYEnable_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_CC_ISSIMORIGINEDCALL_REQ,_T("MSG_CC_ISSIMORIGINEDCALL_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_IsSimOriginedCall_Req_t, 0, 0},
	{ MSG_CC_ISSIMORIGINEDCALL_RSP,_T("MSG_CC_ISSIMORIGINEDCALL_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_IsSimOriginedCall_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_CC_SETVIDEOCALLPARAM_REQ,_T("MSG_CC_SETVIDEOCALLPARAM_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_SetVideoCallParam_Req_t, 0, 0},
	{ MSG_CC_SETVIDEOCALLPARAM_RSP,_T("MSG_CC_SETVIDEOCALLPARAM_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_CC_GETVIDEOCALLPARAM_REQ,_T("MSG_CC_GETVIDEOCALLPARAM_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_CC_GETVIDEOCALLPARAM_RSP,_T("MSG_CC_GETVIDEOCALLPARAM_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetVideoCallParam_Rsp_t, sizeof( VideoCallParam_t ), 0 },
	{ MSG_CC_GETCCM_REQ,_T("MSG_CC_GETCCM_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_GetCCM_Req_t, 0, 0},
	{ MSG_CC_GETCCM_RSP,_T("MSG_CC_GETCCM_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetCCM_Rsp_t, sizeof( CAPI2_CcApi_GetCCM_Rsp_t ), 0 },
	{ MSG_CCAPI_SENDDTMF_REQ,_T("MSG_CCAPI_SENDDTMF_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_SendDtmfTone_Req_t, 0, 0},
	{ MSG_CCAPI_SENDDTMF_RSP,_T("MSG_CCAPI_SENDDTMF_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_CCAPI_STOPDTMF_REQ,_T("MSG_CCAPI_STOPDTMF_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_StopDtmfTone_Req_t, 0, 0},
	{ MSG_CCAPI_STOPDTMF_RSP,_T("MSG_CCAPI_STOPDTMF_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_CCAPI_ABORTDTMF_REQ,_T("MSG_CCAPI_ABORTDTMF_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_AbortDtmf_Req_t, 0, 0},
	{ MSG_CCAPI_ABORTDTMF_RSP,_T("MSG_CCAPI_ABORTDTMF_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_CCAPI_SETDTMFTIMER_REQ,_T("MSG_CCAPI_SETDTMFTIMER_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_SetDtmfToneTimer_Req_t, 0, 0},
	{ MSG_CCAPI_SETDTMFTIMER_RSP,_T("MSG_CCAPI_SETDTMFTIMER_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_CCAPI_RESETDTMFTIMER_REQ,_T("MSG_CCAPI_RESETDTMFTIMER_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_ResetDtmfToneTimer_Req_t, 0, 0},
	{ MSG_CCAPI_RESETDTMFTIMER_RSP,_T("MSG_CCAPI_RESETDTMFTIMER_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_CCAPI_GETDTMFTIMER_REQ,_T("MSG_CCAPI_GETDTMFTIMER_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_GetDtmfToneTimer_Req_t, 0, 0},
	{ MSG_CCAPI_GETDTMFTIMER_RSP,_T("MSG_CCAPI_GETDTMFTIMER_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetDtmfToneTimer_Rsp_t, sizeof( Ticks_t ), 0 },
	{ MSG_CCAPI_GETTIFROMCALLINDEX_REQ,_T("MSG_CCAPI_GETTIFROMCALLINDEX_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_GetTiFromCallIndex_Req_t, 0, 0},
	{ MSG_CCAPI_GETTIFROMCALLINDEX_RSP,_T("MSG_CCAPI_GETTIFROMCALLINDEX_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetTiFromCallIndex_Rsp_t, sizeof( CAPI2_CcApi_GetTiFromCallIndex_Rsp_t ), 0 },
	{ MSG_CCAPI_IS_SUPPORTEDBC_REQ,_T("MSG_CCAPI_IS_SUPPORTEDBC_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_IsSupportedBC_Req_t, 0, 0},
	{ MSG_CCAPI_IS_SUPPORTEDBC_RSP,_T("MSG_CCAPI_IS_SUPPORTEDBC_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_IsSupportedBC_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_CCAPI_IS_BEARER_CAPABILITY_REQ,_T("MSG_CCAPI_IS_BEARER_CAPABILITY_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_GetBearerCapability_Req_t, 0, 0},
	{ MSG_CCAPI_IS_BEARER_CAPABILITY_RSP,_T("MSG_CCAPI_IS_BEARER_CAPABILITY_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetBearerCapability_Rsp_t, sizeof( CAPI2_CcApi_GetBearerCapability_Rsp_t ), 0 },
	{ MSG_SMS_SENDSMSSRVCENTERNUMBERUPDATE_REQ,_T("MSG_SMS_SENDSMSSRVCENTERNUMBERUPDATE_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_SendSMSSrvCenterNumberUpdateReq_Req_t, 0, 0},
	{ MSG_SMS_SENDSMSSRVCENTERNUMBERUPDATE_RSP,_T("MSG_SMS_SENDSMSSRVCENTERNUMBERUPDATE_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_GETSMSSRVCENTERNUMBER_REQ,_T("MSG_SMS_GETSMSSRVCENTERNUMBER_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_GetSMSrvCenterNumber_Req_t, 0, 0},
	{ MSG_SMS_GETSMSSRVCENTERNUMBER_RSP,_T("MSG_SMS_GETSMSSRVCENTERNUMBER_RSP"), (xdrproc_t)xdr_CAPI2_SmsApi_GetSMSrvCenterNumber_Rsp_t, sizeof( CAPI2_SmsApi_GetSMSrvCenterNumber_Rsp_t ), 0 },
	{ MSG_SMS_GETSIMSMSCAPACITYEXCEEDEDFLAG_REQ,_T("MSG_SMS_GETSIMSMSCAPACITYEXCEEDEDFLAG_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SMS_GETSIMSMSCAPACITYEXCEEDEDFLAG_RSP,_T("MSG_SMS_GETSIMSMSCAPACITYEXCEEDEDFLAG_RSP"), (xdrproc_t)xdr_CAPI2_SmsApi_GetSIMSMSCapacityExceededFlag_Rsp_t, sizeof( CAPI2_SmsApi_GetSIMSMSCapacityExceededFlag_Rsp_t ), 0 },
	{ MSG_SMS_ISSMSSERVICEAVAIL_REQ,_T("MSG_SMS_ISSMSSERVICEAVAIL_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SMS_ISSMSSERVICEAVAIL_RSP,_T("MSG_SMS_ISSMSSERVICEAVAIL_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_GETSMSSTOREDSTATE_REQ,_T("MSG_SMS_GETSMSSTOREDSTATE_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_GetSmsStoredState_Req_t, 0, 0},
	{ MSG_SMS_GETSMSSTOREDSTATE_RSP,_T("MSG_SMS_GETSMSSTOREDSTATE_RSP"), (xdrproc_t)xdr_CAPI2_SmsApi_GetSmsStoredState_Rsp_t, sizeof( CAPI2_SmsApi_GetSmsStoredState_Rsp_t ), 0 },
	{ MSG_SMS_WRITESMSPDU_REQ,_T("MSG_SMS_WRITESMSPDU_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_WriteSMSPduReq_Req_t, 0, 0},
	{ MSG_SMS_WRITE_RSP_IND,_T("MSG_SMS_WRITE_RSP_IND"), (xdrproc_t)xdr_CAPI2_SmsApi_WriteSMSPduReq_Rsp_t, sizeof( SIM_SMS_UPDATE_RESULT_t ), 0 },
	{ MSG_SMS_WRITESMSREQ_REQ,_T("MSG_SMS_WRITESMSREQ_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_WriteSMSReq_Req_t, 0, 0},
	{ MSG_SMS_SENDSMSREQ_REQ,_T("MSG_SMS_SENDSMSREQ_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_SendSMSReq_Req_t, 0, 0},
	{ MSG_SMS_SUBMIT_RSP,_T("MSG_SMS_SUBMIT_RSP"), (xdrproc_t)xdr_CAPI2_SmsApi_SendSMSReq_Rsp_t, sizeof( SmsSubmitRspMsg_t ), 0 },
	{ MSG_SMS_SENDSMSPDUREQ_REQ,_T("MSG_SMS_SENDSMSPDUREQ_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_SendSMSPduReq_Req_t, 0, 0},
	{ MSG_SMS_SENDSTOREDSMSREQ_REQ,_T("MSG_SMS_SENDSTOREDSMSREQ_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_SendStoredSmsReq_Req_t, 0, 0},
	{ MSG_SMS_WRITESMSPDUTOSIM_REQ,_T("MSG_SMS_WRITESMSPDUTOSIM_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_WriteSMSPduToSIMReq_Req_t, 0, 0},
	{ MSG_SMS_GETLASTTPMR_REQ,_T("MSG_SMS_GETLASTTPMR_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SMS_GETLASTTPMR_RSP,_T("MSG_SMS_GETLASTTPMR_RSP"), (xdrproc_t)xdr_CAPI2_SmsApi_GetLastTpMr_Rsp_t, sizeof( UInt8 ), 0 },
	{ MSG_SMS_GETSMSTXPARAMS_REQ,_T("MSG_SMS_GETSMSTXPARAMS_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SMS_GETSMSTXPARAMS_RSP,_T("MSG_SMS_GETSMSTXPARAMS_RSP"), (xdrproc_t)xdr_CAPI2_SmsApi_GetSmsTxParams_Rsp_t, sizeof( CAPI2_SmsApi_GetSmsTxParams_Rsp_t ), 0 },
	{ MSG_SMS_GETTXPARAMINTEXTMODE_REQ,_T("MSG_SMS_GETTXPARAMINTEXTMODE_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SMS_GETTXPARAMINTEXTMODE_RSP,_T("MSG_SMS_GETTXPARAMINTEXTMODE_RSP"), (xdrproc_t)xdr_CAPI2_SmsApi_GetTxParamInTextMode_Rsp_t, sizeof( CAPI2_SmsApi_GetTxParamInTextMode_Rsp_t ), 0 },
	{ MSG_SMS_SETSMSTXPARAMPROCID_REQ,_T("MSG_SMS_SETSMSTXPARAMPROCID_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_SetSmsTxParamProcId_Req_t, 0, 0},
	{ MSG_SMS_SETSMSTXPARAMPROCID_RSP,_T("MSG_SMS_SETSMSTXPARAMPROCID_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_SETSMSTXPARAMCODINGTYPE_REQ,_T("MSG_SMS_SETSMSTXPARAMCODINGTYPE_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_SetSmsTxParamCodingType_Req_t, 0, 0},
	{ MSG_SMS_SETSMSTXPARAMCODINGTYPE_RSP,_T("MSG_SMS_SETSMSTXPARAMCODINGTYPE_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_SETSMSTXPARAMVALIDPERIOD_REQ,_T("MSG_SMS_SETSMSTXPARAMVALIDPERIOD_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_SetSmsTxParamValidPeriod_Req_t, 0, 0},
	{ MSG_SMS_SETSMSTXPARAMVALIDPERIOD_RSP,_T("MSG_SMS_SETSMSTXPARAMVALIDPERIOD_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_SETSMSTXPARAMCOMPRESSION_REQ,_T("MSG_SMS_SETSMSTXPARAMCOMPRESSION_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_SetSmsTxParamCompression_Req_t, 0, 0},
	{ MSG_SMS_SETSMSTXPARAMCOMPRESSION_RSP,_T("MSG_SMS_SETSMSTXPARAMCOMPRESSION_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_SETSMSTXPARAMREPLYPATH_REQ,_T("MSG_SMS_SETSMSTXPARAMREPLYPATH_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_SetSmsTxParamReplyPath_Req_t, 0, 0},
	{ MSG_SMS_SETSMSTXPARAMREPLYPATH_RSP,_T("MSG_SMS_SETSMSTXPARAMREPLYPATH_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_SETSMSTXPARAMUSERDATAHDRIND_REQ,_T("MSG_SMS_SETSMSTXPARAMUSERDATAHDRIND_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_SetSmsTxParamUserDataHdrInd_Req_t, 0, 0},
	{ MSG_SMS_SETSMSTXPARAMUSERDATAHDRIND_RSP,_T("MSG_SMS_SETSMSTXPARAMUSERDATAHDRIND_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_SETSMSTXPARAMSTATUSRPTREQFLAG_REQ,_T("MSG_SMS_SETSMSTXPARAMSTATUSRPTREQFLAG_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_SetSmsTxParamStatusRptReqFlag_Req_t, 0, 0},
	{ MSG_SMS_SETSMSTXPARAMSTATUSRPTREQFLAG_RSP,_T("MSG_SMS_SETSMSTXPARAMSTATUSRPTREQFLAG_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_SETSMSTXPARAMREJDUPL_REQ,_T("MSG_SMS_SETSMSTXPARAMREJDUPL_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_SetSmsTxParamRejDupl_Req_t, 0, 0},
	{ MSG_SMS_SETSMSTXPARAMREJDUPL_RSP,_T("MSG_SMS_SETSMSTXPARAMREJDUPL_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_DELETESMSMSGBYINDEX_REQ,_T("MSG_SMS_DELETESMSMSGBYINDEX_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_DeleteSmsMsgByIndexReq_Req_t, 0, 0},
	{ MSG_SIM_SMS_STATUS_UPD_RSP,_T("MSG_SIM_SMS_STATUS_UPD_RSP"), (xdrproc_t)xdr_CAPI2_SmsApi_DeleteSmsMsgByIndexReq_Rsp_t, sizeof( SmsMsgDeleteResult_t ), 0 },
	{ MSG_SMS_READSMSMSG_REQ,_T("MSG_SMS_READSMSMSG_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_ReadSmsMsgReq_Req_t, 0, 0},
	{ MSG_SMS_LISTSMSMSG_REQ,_T("MSG_SMS_LISTSMSMSG_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_ListSmsMsgReq_Req_t, 0, 0},
	{ MSG_SMS_SETNEWMSGDISPLAYPREF_REQ,_T("MSG_SMS_SETNEWMSGDISPLAYPREF_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_SetNewMsgDisplayPref_Req_t, 0, 0},
	{ MSG_SMS_SETNEWMSGDISPLAYPREF_RSP,_T("MSG_SMS_SETNEWMSGDISPLAYPREF_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_GETNEWMSGDISPLAYPREF_REQ,_T("MSG_SMS_GETNEWMSGDISPLAYPREF_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_GetNewMsgDisplayPref_Req_t, 0, 0},
	{ MSG_SMS_GETNEWMSGDISPLAYPREF_RSP,_T("MSG_SMS_GETNEWMSGDISPLAYPREF_RSP"), (xdrproc_t)xdr_CAPI2_SmsApi_GetNewMsgDisplayPref_Rsp_t, sizeof( UInt8 ), 0 },
	{ MSG_SMS_SETSMSPREFSTORAGE_REQ,_T("MSG_SMS_SETSMSPREFSTORAGE_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_SetSMSPrefStorage_Req_t, 0, 0},
	{ MSG_SMS_SETSMSPREFSTORAGE_RSP,_T("MSG_SMS_SETSMSPREFSTORAGE_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_GETSMSPREFSTORAGE_REQ,_T("MSG_SMS_GETSMSPREFSTORAGE_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SMS_GETSMSPREFSTORAGE_RSP,_T("MSG_SMS_GETSMSPREFSTORAGE_RSP"), (xdrproc_t)xdr_CAPI2_SmsApi_GetSMSPrefStorage_Rsp_t, sizeof( SmsStorage_t ), 0 },
	{ MSG_SMS_GETSMSSTORAGESTATUS_REQ,_T("MSG_SMS_GETSMSSTORAGESTATUS_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_GetSMSStorageStatus_Req_t, 0, 0},
	{ MSG_SMS_GETSMSSTORAGESTATUS_RSP,_T("MSG_SMS_GETSMSSTORAGESTATUS_RSP"), (xdrproc_t)xdr_CAPI2_SmsApi_GetSMSStorageStatus_Rsp_t, sizeof( CAPI2_SmsApi_GetSMSStorageStatus_Rsp_t ), 0 },
	{ MSG_SMS_SAVESMSSERVICEPROFILE_REQ,_T("MSG_SMS_SAVESMSSERVICEPROFILE_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_SaveSmsServiceProfile_Req_t, 0, 0},
	{ MSG_SMS_SAVESMSSERVICEPROFILE_RSP,_T("MSG_SMS_SAVESMSSERVICEPROFILE_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_RESTORESMSSERVICEPROFILE_REQ,_T("MSG_SMS_RESTORESMSSERVICEPROFILE_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_RestoreSmsServiceProfile_Req_t, 0, 0},
	{ MSG_SMS_RESTORESMSSERVICEPROFILE_RSP,_T("MSG_SMS_RESTORESMSSERVICEPROFILE_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_SETCELLBROADCASTMSGTYPE_REQ,_T("MSG_SMS_SETCELLBROADCASTMSGTYPE_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_SetCellBroadcastMsgTypeReq_Req_t, 0, 0},
	{ MSG_SMS_CB_START_STOP_RSP,_T("MSG_SMS_CB_START_STOP_RSP"), (xdrproc_t)xdr_CAPI2_SmsApi_SetCellBroadcastMsgTypeReq_Rsp_t, sizeof( SmsCBMsgRspType_t ), 0 },
	{ MSG_SMS_CBALLOWALLCHNLREQ_REQ,_T("MSG_SMS_CBALLOWALLCHNLREQ_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_CBAllowAllChnlReq_Req_t, 0, 0},
	{ MSG_SMS_CBALLOWALLCHNLREQ_RSP,_T("MSG_SMS_CBALLOWALLCHNLREQ_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_ADDCELLBROADCASTCHNLREQ_REQ,_T("MSG_SMS_ADDCELLBROADCASTCHNLREQ_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_AddCellBroadcastChnlReq_Req_t, 0, 0},
	{ MSG_SMS_ADDCELLBROADCASTCHNLREQ_RSP,_T("MSG_SMS_ADDCELLBROADCASTCHNLREQ_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_REMOVECELLBROADCASTCHNLREQ_REQ,_T("MSG_SMS_REMOVECELLBROADCASTCHNLREQ_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_RemoveCellBroadcastChnlReq_Req_t, 0, 0},
	{ MSG_SMS_REMOVECELLBROADCASTCHNLREQ_RSP,_T("MSG_SMS_REMOVECELLBROADCASTCHNLREQ_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_REMOVEALLCBCHNLFROMSEARCHLIST_REQ,_T("MSG_SMS_REMOVEALLCBCHNLFROMSEARCHLIST_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SMS_REMOVEALLCBCHNLFROMSEARCHLIST_RSP,_T("MSG_SMS_REMOVEALLCBCHNLFROMSEARCHLIST_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_GETCBMI_REQ,_T("MSG_SMS_GETCBMI_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SMS_GETCBMI_RSP,_T("MSG_SMS_GETCBMI_RSP"), (xdrproc_t)xdr_CAPI2_SmsApi_GetCBMI_Rsp_t, sizeof( CAPI2_SmsApi_GetCBMI_Rsp_t ), 0 },
	{ MSG_SMS_GETCBLANGUAGE_REQ,_T("MSG_SMS_GETCBLANGUAGE_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SMS_GETCBLANGUAGE_RSP,_T("MSG_SMS_GETCBLANGUAGE_RSP"), (xdrproc_t)xdr_CAPI2_SmsApi_GetCbLanguage_Rsp_t, sizeof( CAPI2_SmsApi_GetCbLanguage_Rsp_t ), 0 },
	{ MSG_SMS_ADDCELLBROADCASTLANGREQ_REQ,_T("MSG_SMS_ADDCELLBROADCASTLANGREQ_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_AddCellBroadcastLangReq_Req_t, 0, 0},
	{ MSG_SMS_ADDCELLBROADCASTLANGREQ_RSP,_T("MSG_SMS_ADDCELLBROADCASTLANGREQ_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_REMOVECELLBROADCASTLANGREQ_REQ,_T("MSG_SMS_REMOVECELLBROADCASTLANGREQ_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_RemoveCellBroadcastLangReq_Req_t, 0, 0},
	{ MSG_SMS_REMOVECELLBROADCASTLANGREQ_RSP,_T("MSG_SMS_REMOVECELLBROADCASTLANGREQ_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_STARTRECEIVINGCELLBROADCAST_REQ,_T("MSG_SMS_STARTRECEIVINGCELLBROADCAST_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SMS_STOPRECEIVINGCELLBROADCAST_REQ,_T("MSG_SMS_STOPRECEIVINGCELLBROADCAST_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SMS_SETCBIGNOREDUPLFLAG_REQ,_T("MSG_SMS_SETCBIGNOREDUPLFLAG_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_SetCBIgnoreDuplFlag_Req_t, 0, 0},
	{ MSG_SMS_SETCBIGNOREDUPLFLAG_RSP,_T("MSG_SMS_SETCBIGNOREDUPLFLAG_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_GETCBIGNOREDUPLFLAG_REQ,_T("MSG_SMS_GETCBIGNOREDUPLFLAG_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SMS_GETCBIGNOREDUPLFLAG_RSP,_T("MSG_SMS_GETCBIGNOREDUPLFLAG_RSP"), (xdrproc_t)xdr_CAPI2_SmsApi_GetCBIgnoreDuplFlag_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SMS_SETVMINDONOFF_REQ,_T("MSG_SMS_SETVMINDONOFF_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_SetVMIndOnOff_Req_t, 0, 0},
	{ MSG_SMS_SETVMINDONOFF_RSP,_T("MSG_SMS_SETVMINDONOFF_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_ISVMINDENABLED_REQ,_T("MSG_SMS_ISVMINDENABLED_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SMS_ISVMINDENABLED_RSP,_T("MSG_SMS_ISVMINDENABLED_RSP"), (xdrproc_t)xdr_CAPI2_SmsApi_IsVMIndEnabled_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SMS_GETVMWAITINGSTATUS_REQ,_T("MSG_SMS_GETVMWAITINGSTATUS_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SMS_GETVMWAITINGSTATUS_RSP,_T("MSG_SMS_GETVMWAITINGSTATUS_RSP"), (xdrproc_t)xdr_CAPI2_SmsApi_GetVMWaitingStatus_Rsp_t, sizeof( CAPI2_SmsApi_GetVMWaitingStatus_Rsp_t ), 0 },
	{ MSG_SMS_GETNUMOFVMSCNUMBER_REQ,_T("MSG_SMS_GETNUMOFVMSCNUMBER_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SMS_GETNUMOFVMSCNUMBER_RSP,_T("MSG_SMS_GETNUMOFVMSCNUMBER_RSP"), (xdrproc_t)xdr_CAPI2_SmsApi_GetNumOfVmscNumber_Rsp_t, sizeof( CAPI2_SmsApi_GetNumOfVmscNumber_Rsp_t ), 0 },
	{ MSG_SMS_GETVMSCNUMBER_REQ,_T("MSG_SMS_GETVMSCNUMBER_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_GetVmscNumber_Req_t, 0, 0},
	{ MSG_SMS_GETVMSCNUMBER_RSP,_T("MSG_SMS_GETVMSCNUMBER_RSP"), (xdrproc_t)xdr_CAPI2_SmsApi_GetVmscNumber_Rsp_t, sizeof( CAPI2_SmsApi_GetVmscNumber_Rsp_t ), 0 },
	{ MSG_SMS_UPDATEVMSCNUMBERREQ_REQ,_T("MSG_SMS_UPDATEVMSCNUMBERREQ_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_UpdateVmscNumberReq_Req_t, 0, 0},
	{ MSG_SMS_UPDATEVMSCNUMBERREQ_RSP,_T("MSG_SMS_UPDATEVMSCNUMBERREQ_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_GETSMSBEARERPREFERENCE_REQ,_T("MSG_SMS_GETSMSBEARERPREFERENCE_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SMS_GETSMSBEARERPREFERENCE_RSP,_T("MSG_SMS_GETSMSBEARERPREFERENCE_RSP"), (xdrproc_t)xdr_CAPI2_SmsApi_GetSMSBearerPreference_Rsp_t, sizeof( SMS_BEARER_PREFERENCE_t ), 0 },
	{ MSG_SMS_SETSMSBEARERPREFERENCE_REQ,_T("MSG_SMS_SETSMSBEARERPREFERENCE_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_SetSMSBearerPreference_Req_t, 0, 0},
	{ MSG_SMS_SETSMSBEARERPREFERENCE_RSP,_T("MSG_SMS_SETSMSBEARERPREFERENCE_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_SETSMSREADSTATUSCHANGEMODE_REQ,_T("MSG_SMS_SETSMSREADSTATUSCHANGEMODE_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_SetSmsReadStatusChangeMode_Req_t, 0, 0},
	{ MSG_SMS_SETSMSREADSTATUSCHANGEMODE_RSP,_T("MSG_SMS_SETSMSREADSTATUSCHANGEMODE_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_GETSMSREADSTATUSCHANGEMODE_REQ,_T("MSG_SMS_GETSMSREADSTATUSCHANGEMODE_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SMS_GETSMSREADSTATUSCHANGEMODE_RSP,_T("MSG_SMS_GETSMSREADSTATUSCHANGEMODE_RSP"), (xdrproc_t)xdr_CAPI2_SmsApi_GetSmsReadStatusChangeMode_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SMS_CHANGESTATUSREQ_REQ,_T("MSG_SMS_CHANGESTATUSREQ_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_ChangeSmsStatusReq_Req_t, 0, 0},
	{ MSG_SMS_CHANGESTATUSREQ_RSP,_T("MSG_SMS_CHANGESTATUSREQ_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_SENDMESTOREDSTATUSIND_REQ,_T("MSG_SMS_SENDMESTOREDSTATUSIND_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_SendMEStoredStatusInd_Req_t, 0, 0},
	{ MSG_SMS_SENDMESTOREDSTATUSIND_RSP,_T("MSG_SMS_SENDMESTOREDSTATUSIND_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_SENDMERETRIEVESMSDATAIND_REQ,_T("MSG_SMS_SENDMERETRIEVESMSDATAIND_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_SendMERetrieveSmsDataInd_Req_t, 0, 0},
	{ MSG_SMS_SENDMERETRIEVESMSDATAIND_RSP,_T("MSG_SMS_SENDMERETRIEVESMSDATAIND_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_SENDMEREMOVEDSTATUSIND_REQ,_T("MSG_SMS_SENDMEREMOVEDSTATUSIND_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_SendMERemovedStatusInd_Req_t, 0, 0},
	{ MSG_SMS_SENDMEREMOVEDSTATUSIND_RSP,_T("MSG_SMS_SENDMEREMOVEDSTATUSIND_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_SETSMSSTOREDSTATE_REQ,_T("MSG_SMS_SETSMSSTOREDSTATE_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_SetSmsStoredState_Req_t, 0, 0},
	{ MSG_SMS_SETSMSSTOREDSTATE_RSP,_T("MSG_SMS_SETSMSSTOREDSTATE_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_ISCACHEDDATAREADY_REQ,_T("MSG_SMS_ISCACHEDDATAREADY_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SMS_ISCACHEDDATAREADY_RSP,_T("MSG_SMS_ISCACHEDDATAREADY_RSP"), (xdrproc_t)xdr_CAPI2_SmsApi_IsCachedDataReady_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SMS_GETENHANCEDVMINFOIEI_REQ,_T("MSG_SMS_GETENHANCEDVMINFOIEI_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SMS_GETENHANCEDVMINFOIEI_RSP,_T("MSG_SMS_GETENHANCEDVMINFOIEI_RSP"), (xdrproc_t)xdr_CAPI2_SmsApi_GetEnhancedVMInfoIEI_Rsp_t, sizeof( SmsEnhancedVMInd_t ), 0 },
	{ MSG_SMS_SETALLNEWMSGDISPLAYPREF_REQ,_T("MSG_SMS_SETALLNEWMSGDISPLAYPREF_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_SetAllNewMsgDisplayPref_Req_t, 0, 0},
	{ MSG_SMS_SETALLNEWMSGDISPLAYPREF_RSP,_T("MSG_SMS_SETALLNEWMSGDISPLAYPREF_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_ACKTONETWORK_REQ,_T("MSG_SMS_ACKTONETWORK_REQ"), (xdrproc_t) xdr_CAPI2_SmsApi_SendAckToNetwork_Req_t, 0, 0},
	{ MSG_SMS_ACKTONETWORK_RSP,_T("MSG_SMS_ACKTONETWORK_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_STARTMULTISMSTRANSFER_REQ,_T("MSG_SMS_STARTMULTISMSTRANSFER_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SMS_STARTMULTISMSTRANSFER_RSP,_T("MSG_SMS_STARTMULTISMSTRANSFER_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_STOPMULTISMSTRANSFER_REQ,_T("MSG_SMS_STOPMULTISMSTRANSFER_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SMS_STOPMULTISMSTRANSFER_RSP,_T("MSG_SMS_STOPMULTISMSTRANSFER_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_START_CELL_BROADCAST_REQ,_T("MSG_SMS_START_CELL_BROADCAST_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SMS_START_CELL_BROADCAST_RSP,_T("MSG_SMS_START_CELL_BROADCAST_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_SIMINIT_REQ,_T("MSG_SMS_SIMINIT_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SMS_SIMINIT_RSP,_T("MSG_SMS_SIMINIT_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMS_PDA_OVERFLOW_REQ,_T("MSG_SMS_PDA_OVERFLOW_REQ"), (xdrproc_t) xdr_CAPI2_SMS_SetPDAStorageOverFlowFlag_Req_t, 0, 0},
	{ MSG_SMS_PDA_OVERFLOW_RSP,_T("MSG_SMS_PDA_OVERFLOW_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_ISIM_SENDAUTHENAKAREQ_REQ,_T("MSG_ISIM_SENDAUTHENAKAREQ_REQ"), (xdrproc_t) xdr_CAPI2_ISimApi_SendAuthenAkaReq_Req_t, 0, 0},
	{ MSG_ISIM_AUTHEN_AKA_RSP,_T("MSG_ISIM_AUTHEN_AKA_RSP"), (xdrproc_t)xdr_CAPI2_ISimApi_SendAuthenAkaReq_Rsp_t, sizeof( ISIM_AUTHEN_AKA_RSP_t ), 0 },
	{ MSG_ISIM_ISISIMSUPPORTED_REQ,_T("MSG_ISIM_ISISIMSUPPORTED_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_ISIM_ISISIMSUPPORTED_RSP,_T("MSG_ISIM_ISISIMSUPPORTED_RSP"), (xdrproc_t)xdr_CAPI2_ISimApi_IsIsimSupported_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_ISIM_ISISIMACTIVATED_REQ,_T("MSG_ISIM_ISISIMACTIVATED_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_ISIM_ISISIMACTIVATED_RSP,_T("MSG_ISIM_ISISIMACTIVATED_RSP"), (xdrproc_t)xdr_CAPI2_ISimApi_IsIsimActivated_Rsp_t, sizeof( CAPI2_ISimApi_IsIsimActivated_Rsp_t ), 0 },
	{ MSG_ISIM_ACTIVATEISIMAPPLI_REQ,_T("MSG_ISIM_ACTIVATEISIMAPPLI_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_ISIM_ACTIVATE_RSP,_T("MSG_ISIM_ACTIVATE_RSP"), (xdrproc_t)xdr_CAPI2_ISimApi_ActivateIsimAppli_Rsp_t, sizeof( ISIM_ACTIVATE_RSP_t ), 0 },
	{ MSG_ISIM_SENDAUTHENHTTPREQ_REQ,_T("MSG_ISIM_SENDAUTHENHTTPREQ_REQ"), (xdrproc_t) xdr_CAPI2_ISimApi_SendAuthenHttpReq_Req_t, 0, 0},
	{ MSG_ISIM_AUTHEN_HTTP_RSP,_T("MSG_ISIM_AUTHEN_HTTP_RSP"), (xdrproc_t)xdr_CAPI2_ISimApi_SendAuthenHttpReq_Rsp_t, sizeof( ISIM_AUTHEN_HTTP_RSP_t ), 0 },
	{ MSG_ISIM_SENDAUTHENGBANAFREQ_REQ,_T("MSG_ISIM_SENDAUTHENGBANAFREQ_REQ"), (xdrproc_t) xdr_CAPI2_ISimApi_SendAuthenGbaNafReq_Req_t, 0, 0},
	{ MSG_ISIM_AUTHEN_GBA_NAF_RSP,_T("MSG_ISIM_AUTHEN_GBA_NAF_RSP"), (xdrproc_t)xdr_CAPI2_ISimApi_SendAuthenGbaNafReq_Rsp_t, sizeof( ISIM_AUTHEN_GBA_NAF_RSP_t ), 0 },
	{ MSG_ISIM_SENDAUTHENGBABOOTREQ_REQ,_T("MSG_ISIM_SENDAUTHENGBABOOTREQ_REQ"), (xdrproc_t) xdr_CAPI2_ISimApi_SendAuthenGbaBootReq_Req_t, 0, 0},
	{ MSG_ISIM_AUTHEN_GBA_BOOT_RSP,_T("MSG_ISIM_AUTHEN_GBA_BOOT_RSP"), (xdrproc_t)xdr_CAPI2_ISimApi_SendAuthenGbaBootReq_Rsp_t, sizeof( ISIM_AUTHEN_GBA_BOOT_RSP_t ), 0 },
	{ MSG_PBK_GETALPHA_REQ,_T("MSG_PBK_GETALPHA_REQ"), (xdrproc_t) xdr_CAPI2_PbkApi_GetAlpha_Req_t, 0, 0},
	{ MSG_PBK_GETALPHA_RSP,_T("MSG_PBK_GETALPHA_RSP"), (xdrproc_t)xdr_CAPI2_PbkApi_GetAlpha_Rsp_t, sizeof( CAPI2_PbkApi_GetAlpha_Rsp_t ), 0 },
	{ MSG_PBK_ISEMERGENCYCALLNUMBER_REQ,_T("MSG_PBK_ISEMERGENCYCALLNUMBER_REQ"), (xdrproc_t) xdr_CAPI2_PbkApi_IsEmergencyCallNumber_Req_t, 0, 0},
	{ MSG_PBK_ISEMERGENCYCALLNUMBER_RSP,_T("MSG_PBK_ISEMERGENCYCALLNUMBER_RSP"), (xdrproc_t)xdr_CAPI2_PbkApi_IsEmergencyCallNumber_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_REQ,_T("MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_REQ"), (xdrproc_t) xdr_CAPI2_PbkApi_IsPartialEmergencyCallNumber_Req_t, 0, 0},
	{ MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_RSP,_T("MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_RSP"), (xdrproc_t)xdr_CAPI2_PbkApi_IsPartialEmergencyCallNumber_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_PBK_SENDINFOREQ_REQ,_T("MSG_PBK_SENDINFOREQ_REQ"), (xdrproc_t) xdr_CAPI2_PbkApi_SendInfoReq_Req_t, 0, 0},
	{ MSG_GET_PBK_INFO_RSP,_T("MSG_GET_PBK_INFO_RSP"), (xdrproc_t)xdr_CAPI2_PbkApi_SendInfoReq_Rsp_t, sizeof( PBK_INFO_RSP_t ), 0 },
	{ MSG_SENDFINDALPHAMATCHMULTIPLEREQ_REQ,_T("MSG_SENDFINDALPHAMATCHMULTIPLEREQ_REQ"), (xdrproc_t) xdr_CAPI2_PbkApi_SendFindAlphaMatchMultipleReq_Req_t, 0, 0},
	{ MSG_PBK_ENTRY_DATA_RSP,_T("MSG_PBK_ENTRY_DATA_RSP"), (xdrproc_t)xdr_CAPI2_PbkApi_SendFindAlphaMatchMultipleReq_Rsp_t, sizeof( PBK_ENTRY_DATA_RSP_t ), 0 },
	{ MSG_SENDFINDALPHAMATCHONEREQ_REQ,_T("MSG_SENDFINDALPHAMATCHONEREQ_REQ"), (xdrproc_t) xdr_CAPI2_PbkApi_SendFindAlphaMatchOneReq_Req_t, 0, 0},
	{ MSG_PBK_ISREADY_REQ,_T("MSG_PBK_ISREADY_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_PBK_ISREADY_RSP,_T("MSG_PBK_ISREADY_RSP"), (xdrproc_t)xdr_CAPI2_PbkApi_IsReady_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SENDREADENTRYREQ_REQ,_T("MSG_SENDREADENTRYREQ_REQ"), (xdrproc_t) xdr_CAPI2_PbkApi_SendReadEntryReq_Req_t, 0, 0},
	{ MSG_PBK_SENDWRITEENTRYREQ_REQ,_T("MSG_PBK_SENDWRITEENTRYREQ_REQ"), (xdrproc_t) xdr_CAPI2_PbkApi_SendWriteEntryReq_Req_t, 0, 0},
	{ MSG_WRT_PBK_ENTRY_RSP,_T("MSG_WRT_PBK_ENTRY_RSP"), (xdrproc_t)xdr_CAPI2_PbkApi_SendWriteEntryReq_Rsp_t, sizeof( PBK_WRITE_ENTRY_RSP_t ), 0 },
	{ MSG_PBK_SENDUPDATEENTRYREQ,_T("MSG_PBK_SENDUPDATEENTRYREQ"), (xdrproc_t) xdr_CAPI2_PbkApi_SendUpdateEntryReq_Req_t, 0, 0},
	{ MSG_PBK_SENDISNUMDIALLABLEREQ,_T("MSG_PBK_SENDISNUMDIALLABLEREQ"), (xdrproc_t) xdr_CAPI2_PbkApi_SendIsNumDiallableReq_Req_t, 0, 0},
	{ MSG_CHK_PBK_DIALLED_NUM_RSP,_T("MSG_CHK_PBK_DIALLED_NUM_RSP"), (xdrproc_t)xdr_CAPI2_PbkApi_SendIsNumDiallableReq_Rsp_t, sizeof( PBK_CHK_NUM_DIALLABLE_RSP_t ), 0 },
	{ MSG_PBK_ISNUMDIALLABLE_REQ,_T("MSG_PBK_ISNUMDIALLABLE_REQ"), (xdrproc_t) xdr_CAPI2_PbkApi_IsNumDiallable_Req_t, 0, 0},
	{ MSG_PBK_ISNUMDIALLABLE_RSP,_T("MSG_PBK_ISNUMDIALLABLE_RSP"), (xdrproc_t)xdr_CAPI2_PbkApi_IsNumDiallable_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_PBK_ISNUMBARRED_REQ,_T("MSG_PBK_ISNUMBARRED_REQ"), (xdrproc_t) xdr_CAPI2_PbkApi_IsNumBarred_Req_t, 0, 0},
	{ MSG_PBK_ISNUMBARRED_RSP,_T("MSG_PBK_ISNUMBARRED_RSP"), (xdrproc_t)xdr_CAPI2_PbkApi_IsNumBarred_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_PBK_ISUSSDDIALLABLE_REQ,_T("MSG_PBK_ISUSSDDIALLABLE_REQ"), (xdrproc_t) xdr_CAPI2_PbkApi_IsUssdDiallable_Req_t, 0, 0},
	{ MSG_PBK_ISUSSDDIALLABLE_RSP,_T("MSG_PBK_ISUSSDDIALLABLE_RSP"), (xdrproc_t)xdr_CAPI2_PbkApi_IsUssdDiallable_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_PDP_SETPDPCONTEXT_REQ,_T("MSG_PDP_SETPDPCONTEXT_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_SetPDPContext_Req_t, 0, 0},
	{ MSG_PDP_SETPDPCONTEXT_RSP,_T("MSG_PDP_SETPDPCONTEXT_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_PDP_SETSECPDPCONTEXT_REQ,_T("MSG_PDP_SETSECPDPCONTEXT_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_SetSecPDPContext_Req_t, 0, 0},
	{ MSG_PDP_SETSECPDPCONTEXT_RSP,_T("MSG_PDP_SETSECPDPCONTEXT_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_PDP_GETGPRSQOS_REQ,_T("MSG_PDP_GETGPRSQOS_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_GetGPRSQoS_Req_t, 0, 0},
	{ MSG_PDP_GETGPRSQOS_RSP,_T("MSG_PDP_GETGPRSQOS_RSP"), (xdrproc_t)xdr_CAPI2_PdpApi_GetGPRSQoS_Rsp_t, sizeof( CAPI2_PdpApi_GetGPRSQoS_Rsp_t ), 0 },
	{ MSG_PDP_SETGPRSQOS_REQ,_T("MSG_PDP_SETGPRSQOS_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_SetGPRSQoS_Req_t, 0, 0},
	{ MSG_PDP_SETGPRSQOS_RSP,_T("MSG_PDP_SETGPRSQOS_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_PDP_GETGPRSMINQOS_REQ,_T("MSG_PDP_GETGPRSMINQOS_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_GetGPRSMinQoS_Req_t, 0, 0},
	{ MSG_PDP_GETGPRSMINQOS_RSP,_T("MSG_PDP_GETGPRSMINQOS_RSP"), (xdrproc_t)xdr_CAPI2_PdpApi_GetGPRSMinQoS_Rsp_t, sizeof( CAPI2_PdpApi_GetGPRSMinQoS_Rsp_t ), 0 },
	{ MSG_PDP_SETGPRSMINQOS_REQ,_T("MSG_PDP_SETGPRSMINQOS_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_SetGPRSMinQoS_Req_t, 0, 0},
	{ MSG_PDP_SETGPRSMINQOS_RSP,_T("MSG_PDP_SETGPRSMINQOS_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_MS_SENDCOMBINEDATTACHREQ_REQ,_T("MSG_MS_SENDCOMBINEDATTACHREQ_REQ"), (xdrproc_t) xdr_CAPI2_NetRegApi_SendCombinedAttachReq_Req_t, 0, 0},
	{ MSG_MS_SENDCOMBINEDATTACHREQ_RSP,_T("MSG_MS_SENDCOMBINEDATTACHREQ_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_MS_SENDDETACHREQ_REQ,_T("MSG_MS_SENDDETACHREQ_REQ"), (xdrproc_t) xdr_CAPI2_NetRegApi_SendDetachReq_Req_t, 0, 0},
	{ MSG_MS_SENDDETACHREQ_RSP,_T("MSG_MS_SENDDETACHREQ_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_MS_GETGPRSATTACHSTATUS_REQ,_T("MSG_MS_GETGPRSATTACHSTATUS_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_MS_GETGPRSATTACHSTATUS_RSP,_T("MSG_MS_GETGPRSATTACHSTATUS_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetGPRSAttachStatus_Rsp_t, sizeof( AttachState_t ), 0 },
	{ MSG_PDP_ISSECONDARYPDPDEFINED_REQ,_T("MSG_PDP_ISSECONDARYPDPDEFINED_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_IsSecondaryPdpDefined_Req_t, 0, 0},
	{ MSG_PDP_ISSECONDARYPDPDEFINED_RSP,_T("MSG_PDP_ISSECONDARYPDPDEFINED_RSP"), (xdrproc_t)xdr_CAPI2_PdpApi_IsSecondaryPdpDefined_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_PCHEX_SENDPDPACTIVATEREQ_REQ,_T("MSG_PCHEX_SENDPDPACTIVATEREQ_REQ"), (xdrproc_t) xdr_CAPI2_PchExApi_SendPDPActivateReq_Req_t, 0, 0},
	{ MSG_PDP_ACTIVATION_RSP,_T("MSG_PDP_ACTIVATION_RSP"), (xdrproc_t)xdr_CAPI2_PchExApi_SendPDPActivateReq_Rsp_t, sizeof( PDP_SendPDPActivateReq_Rsp_t ), 0 },
	{ MSG_PCHEX_SENDPDPMODIFYREQ_REQ,_T("MSG_PCHEX_SENDPDPMODIFYREQ_REQ"), (xdrproc_t) xdr_CAPI2_PchExApi_SendPDPModifyReq_Req_t, 0, 0},
	{ MSG_PDP_MODIFICATION_RSP,_T("MSG_PDP_MODIFICATION_RSP"), (xdrproc_t)xdr_CAPI2_PchExApi_SendPDPModifyReq_Rsp_t, sizeof( PDP_SendPDPModifyReq_Rsp_t ), 0 },
	{ MSG_PCHEX_SENDPDPDEACTIVATEREQ_REQ,_T("MSG_PCHEX_SENDPDPDEACTIVATEREQ_REQ"), (xdrproc_t) xdr_CAPI2_PchExApi_SendPDPDeactivateReq_Req_t, 0, 0},
	{ MSG_PDP_DEACTIVATION_RSP,_T("MSG_PDP_DEACTIVATION_RSP"), (xdrproc_t)xdr_CAPI2_PchExApi_SendPDPDeactivateReq_Rsp_t, sizeof( PDP_SendPDPDeactivateReq_Rsp_t ), 0 },
	{ MSG_PCHEX_SENDPDPACTIVATESECREQ_REQ,_T("MSG_PCHEX_SENDPDPACTIVATESECREQ_REQ"), (xdrproc_t) xdr_CAPI2_PchExApi_SendPDPActivateSecReq_Req_t, 0, 0},
	{ MSG_PDP_SEC_ACTIVATION_RSP,_T("MSG_PDP_SEC_ACTIVATION_RSP"), (xdrproc_t)xdr_CAPI2_PchExApi_SendPDPActivateSecReq_Rsp_t, sizeof( PDP_SendPDPActivateSecReq_Rsp_t ), 0 },
	{ MSG_PDP_GETGPRSACTIVATESTATUS_REQ,_T("MSG_PDP_GETGPRSACTIVATESTATUS_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_PDP_GETGPRSACTIVATESTATUS_RSP,_T("MSG_PDP_GETGPRSACTIVATESTATUS_RSP"), (xdrproc_t)xdr_CAPI2_PdpApi_GetGPRSActivateStatus_Rsp_t, sizeof( CAPI2_PdpApi_GetGPRSActivateStatus_Rsp_t ), 0 },
	{ MSG_PDP_SETMSCLASS_REQ,_T("MSG_PDP_SETMSCLASS_REQ"), (xdrproc_t) xdr_CAPI2_NetRegApi_SetMSClass_Req_t, 0, 0},
	{ MSG_PDP_SETMSCLASS_RSP,_T("MSG_PDP_SETMSCLASS_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_PDP_GETMSCLASS_REQ,_T("MSG_PDP_GETMSCLASS_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_PDP_GETMSCLASS_RSP,_T("MSG_PDP_GETMSCLASS_RSP"), (xdrproc_t)xdr_CAPI2_PDP_GetMSClass_Rsp_t, sizeof( MSClass_t ), 0 },
	{ MSG_PDP_GETUMTSTFT_REQ,_T("MSG_PDP_GETUMTSTFT_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_GetUMTSTft_Req_t, 0, 0},
	{ MSG_PDP_GETUMTSTFT_RSP,_T("MSG_PDP_GETUMTSTFT_RSP"), (xdrproc_t)xdr_CAPI2_PdpApi_GetUMTSTft_Rsp_t, sizeof( CAPI2_PdpApi_GetUMTSTft_Rsp_t ), 0 },
	{ MSG_PDP_SETUMTSTFT_REQ,_T("MSG_PDP_SETUMTSTFT_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_SetUMTSTft_Req_t, 0, 0},
	{ MSG_PDP_SETUMTSTFT_RSP,_T("MSG_PDP_SETUMTSTFT_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_PDP_DELETEUMTSTFT_REQ,_T("MSG_PDP_DELETEUMTSTFT_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_DeleteUMTSTft_Req_t, 0, 0},
	{ MSG_PDP_DELETEUMTSTFT_RSP,_T("MSG_PDP_DELETEUMTSTFT_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_PDP_DEACTIVATESNDCPCONNECTION_REQ,_T("MSG_PDP_DEACTIVATESNDCPCONNECTION_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_DeactivateSNDCPConnection_Req_t, 0, 0},
	{ MSG_PDP_DEACTIVATESNDCPCONNECTION_RSP,_T("MSG_PDP_DEACTIVATESNDCPCONNECTION_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_PDP_GETR99UMTSMINQOS_REQ,_T("MSG_PDP_GETR99UMTSMINQOS_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_GetR99UMTSMinQoS_Req_t, 0, 0},
	{ MSG_PDP_GETR99UMTSMINQOS_RSP,_T("MSG_PDP_GETR99UMTSMINQOS_RSP"), (xdrproc_t)xdr_CAPI2_PdpApi_GetR99UMTSMinQoS_Rsp_t, sizeof( CAPI2_PdpApi_GetR99UMTSMinQoS_Rsp_t ), 0 },
	{ MSG_PDP_GETR99UMTSQOS_REQ,_T("MSG_PDP_GETR99UMTSQOS_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_GetR99UMTSQoS_Req_t, 0, 0},
	{ MSG_PDP_GETR99UMTSQOS_RSP,_T("MSG_PDP_GETR99UMTSQOS_RSP"), (xdrproc_t)xdr_CAPI2_PdpApi_GetR99UMTSQoS_Rsp_t, sizeof( CAPI2_PdpApi_GetR99UMTSQoS_Rsp_t ), 0 },
	{ MSG_PDP_GETUMTSMINQOS_REQ,_T("MSG_PDP_GETUMTSMINQOS_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_GetUMTSMinQoS_Req_t, 0, 0},
	{ MSG_PDP_GETUMTSMINQOS_RSP,_T("MSG_PDP_GETUMTSMINQOS_RSP"), (xdrproc_t)xdr_CAPI2_PdpApi_GetUMTSMinQoS_Rsp_t, sizeof( CAPI2_PdpApi_GetUMTSMinQoS_Rsp_t ), 0 },
	{ MSG_PDP_GETUMTSQOS_REQ,_T("MSG_PDP_GETUMTSQOS_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_GetUMTSQoS_Req_t, 0, 0},
	{ MSG_PDP_GETUMTSQOS_RSP,_T("MSG_PDP_GETUMTSQOS_RSP"), (xdrproc_t)xdr_CAPI2_PdpApi_GetUMTSQoS_Rsp_t, sizeof( CAPI2_PdpApi_GetUMTSQoS_Rsp_t ), 0 },
	{ MSG_PDP_GETNEGQOS_REQ,_T("MSG_PDP_GETNEGQOS_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_GetNegQoS_Req_t, 0, 0},
	{ MSG_PDP_GETNEGQOS_RSP,_T("MSG_PDP_GETNEGQOS_RSP"), (xdrproc_t)xdr_CAPI2_PdpApi_GetNegQoS_Rsp_t, sizeof( CAPI2_PdpApi_GetNegQoS_Rsp_t ), 0 },
	{ MSG_PDP_SETR99UMTSMINQOS_REQ,_T("MSG_PDP_SETR99UMTSMINQOS_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_SetR99UMTSMinQoS_Req_t, 0, 0},
	{ MSG_PDP_SETR99UMTSMINQOS_RSP,_T("MSG_PDP_SETR99UMTSMINQOS_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_PDP_SETR99UMTSQOS_REQ,_T("MSG_PDP_SETR99UMTSQOS_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_SetR99UMTSQoS_Req_t, 0, 0},
	{ MSG_PDP_SETR99UMTSQOS_RSP,_T("MSG_PDP_SETR99UMTSQOS_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_PDP_SETUMTSMINQOS_REQ,_T("MSG_PDP_SETUMTSMINQOS_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_SetUMTSMinQoS_Req_t, 0, 0},
	{ MSG_PDP_SETUMTSMINQOS_RSP,_T("MSG_PDP_SETUMTSMINQOS_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_PDP_SETUMTSQOS_REQ,_T("MSG_PDP_SETUMTSQOS_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_SetUMTSQoS_Req_t, 0, 0},
	{ MSG_PDP_SETUMTSQOS_RSP,_T("MSG_PDP_SETUMTSQOS_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_PDP_GETNEGOTIATEDPARMS_REQ,_T("MSG_PDP_GETNEGOTIATEDPARMS_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_GetNegotiatedParms_Req_t, 0, 0},
	{ MSG_PDP_GETNEGOTIATEDPARMS_RSP,_T("MSG_PDP_GETNEGOTIATEDPARMS_RSP"), (xdrproc_t)xdr_CAPI2_PdpApi_GetNegotiatedParms_Rsp_t, sizeof( CAPI2_PdpApi_GetNegotiatedParms_Rsp_t ), 0 },
	{ MSG_MS_ISGPRSCALLACTIVE_REQ,_T("MSG_MS_ISGPRSCALLACTIVE_REQ"), (xdrproc_t) xdr_CAPI2_MS_IsGprsCallActive_Req_t, 0, 0},
	{ MSG_MS_ISGPRSCALLACTIVE_RSP,_T("MSG_MS_ISGPRSCALLACTIVE_RSP"), (xdrproc_t)xdr_CAPI2_MS_IsGprsCallActive_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_MS_SETCHANGPRSCALLACTIVE_REQ,_T("MSG_MS_SETCHANGPRSCALLACTIVE_REQ"), (xdrproc_t) xdr_CAPI2_MS_SetChanGprsCallActive_Req_t, 0, 0},
	{ MSG_MS_SETCHANGPRSCALLACTIVE_RSP,_T("MSG_MS_SETCHANGPRSCALLACTIVE_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_MS_SETCIDFORGPRSACTIVECHAN_REQ,_T("MSG_MS_SETCIDFORGPRSACTIVECHAN_REQ"), (xdrproc_t) xdr_CAPI2_MS_SetCidForGprsActiveChan_Req_t, 0, 0},
	{ MSG_MS_SETCIDFORGPRSACTIVECHAN_RSP,_T("MSG_MS_SETCIDFORGPRSACTIVECHAN_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_PDP_GETPPPMODEMCID_REQ,_T("MSG_PDP_GETPPPMODEMCID_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_PDP_GETPPPMODEMCID_RSP,_T("MSG_PDP_GETPPPMODEMCID_RSP"), (xdrproc_t)xdr_CAPI2_PdpApi_GetPPPModemCid_Rsp_t, sizeof( PCHCid_t ), 0 },
	{ MSG_MS_GETGPRSACTIVECHANFROMCID_REQ,_T("MSG_MS_GETGPRSACTIVECHANFROMCID_REQ"), (xdrproc_t) xdr_CAPI2_MS_GetGprsActiveChanFromCid_Req_t, 0, 0},
	{ MSG_MS_GETGPRSACTIVECHANFROMCID_RSP,_T("MSG_MS_GETGPRSACTIVECHANFROMCID_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetGprsActiveChanFromCid_Rsp_t, sizeof( UInt8 ), 0 },
	{ MSG_MS_GETCIDFROMGPRSACTIVECHAN_REQ,_T("MSG_MS_GETCIDFROMGPRSACTIVECHAN_REQ"), (xdrproc_t) xdr_CAPI2_MS_GetCidFromGprsActiveChan_Req_t, 0, 0},
	{ MSG_MS_GETCIDFROMGPRSACTIVECHAN_RSP,_T("MSG_MS_GETCIDFROMGPRSACTIVECHAN_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetCidFromGprsActiveChan_Rsp_t, sizeof( UInt8 ), 0 },
	{ MSG_PDP_GETPDPADDRESS_REQ,_T("MSG_PDP_GETPDPADDRESS_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_GetPDPAddress_Req_t, 0, 0},
	{ MSG_PDP_GETPDPADDRESS_RSP,_T("MSG_PDP_GETPDPADDRESS_RSP"), (xdrproc_t)xdr_CAPI2_PdpApi_GetPDPAddress_Rsp_t, sizeof( CAPI2_PdpApi_GetPDPAddress_Rsp_t ), 0 },
	{ MSG_PDP_SENDTBFDATA_REQ,_T("MSG_PDP_SENDTBFDATA_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_SendTBFData_Req_t, 0, 0},
	{ MSG_PDP_SENDTBFDATA_RSP,_T("MSG_PDP_SENDTBFDATA_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_PDP_TFTADDFILTER_REQ,_T("MSG_PDP_TFTADDFILTER_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_TftAddFilter_Req_t, 0, 0},
	{ MSG_PDP_TFTADDFILTER_RSP,_T("MSG_PDP_TFTADDFILTER_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_PDP_SETPCHCONTEXTSTATE_REQ,_T("MSG_PDP_SETPCHCONTEXTSTATE_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_SetPCHContextState_Req_t, 0, 0},
	{ MSG_PDP_SETPCHCONTEXTSTATE_RSP,_T("MSG_PDP_SETPCHCONTEXTSTATE_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_PDP_SETDEFAULTPDPCONTEXT_REQ,_T("MSG_PDP_SETDEFAULTPDPCONTEXT_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_SetDefaultPDPContext_Req_t, 0, 0},
	{ MSG_PDP_SETDEFAULTPDPCONTEXT_RSP,_T("MSG_PDP_SETDEFAULTPDPCONTEXT_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_PCHEX_READDECODEDPROTCONFIG_REQ,_T("MSG_PCHEX_READDECODEDPROTCONFIG_REQ"), (xdrproc_t) xdr_CAPI2_PchExApi_GetDecodedProtConfig_Req_t, 0, 0},
	{ MSG_PCHEX_READDECODEDPROTCONFIG_RSP,_T("MSG_PCHEX_READDECODEDPROTCONFIG_RSP"), (xdrproc_t)xdr_CAPI2_PchExApi_GetDecodedProtConfig_Rsp_t, sizeof( CAPI2_PchExApi_GetDecodedProtConfig_Rsp_t ), 0 },
	{ MSG_PCHEX_BUILDPROTCONFIGOPTIONS_REQ,_T("MSG_PCHEX_BUILDPROTCONFIGOPTIONS_REQ"), (xdrproc_t) xdr_CAPI2_PchExApi_BuildIpConfigOptions_Req_t, 0, 0},
	{ MSG_PCHEX_BUILDPROTCONFIGOPTIONS_RSP,_T("MSG_PCHEX_BUILDPROTCONFIGOPTIONS_RSP"), (xdrproc_t)xdr_CAPI2_PchExApi_BuildIpConfigOptions_Rsp_t, sizeof( CAPI2_PchExApi_BuildIpConfigOptions_Rsp_t ), 0 },
	{ MSG_PCHEX_BUILDPROTCONFIGOPTIONS2_REQ,_T("MSG_PCHEX_BUILDPROTCONFIGOPTIONS2_REQ"), (xdrproc_t) xdr_CAPI2_PchExApi_BuildIpConfigOptions2_Req_t, 0, 0},
	{ MSG_PCHEX_BUILDPROTCONFIGOPTIONS2_RSP,_T("MSG_PCHEX_BUILDPROTCONFIGOPTIONS2_RSP"), (xdrproc_t)xdr_CAPI2_PchExApi_BuildIpConfigOptions2_Rsp_t, sizeof( CAPI2_PchExApi_BuildIpConfigOptions2_Rsp_t ), 0 },
	{ MSG_PCHEX_BUILDPROTCONFIGOPTION_CHAP_TYPE_REQ,_T("MSG_PCHEX_BUILDPROTCONFIGOPTION_CHAP_TYPE_REQ"), (xdrproc_t) xdr_CAPI2_PchExApi_BuildIpConfigOptionsWithChapAuthType_Req_t, 0, 0},
	{ MSG_PCHEX_BUILDPROTCONFIGOPTION_CHAP_TYPE_RSP,_T("MSG_PCHEX_BUILDPROTCONFIGOPTION_CHAP_TYPE_RSP"), (xdrproc_t)xdr_CAPI2_PchExApi_BuildIpConfigOptionsWithChapAuthType_Rsp_t, sizeof( CAPI2_PchExApi_BuildIpConfigOptionsWithChapAuthType_Rsp_t ), 0 },
	{ MSG_PDP_GET_DEFAULT_QOS_REQ,_T("MSG_PDP_GET_DEFAULT_QOS_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_PDP_GET_DEFAULT_QOS_RSP,_T("MSG_PDP_GET_DEFAULT_QOS_RSP"), (xdrproc_t)xdr_CAPI2_PdpApi_GetDefaultQos_Rsp_t, sizeof( CAPI2_PdpApi_GetDefaultQos_Rsp_t ), 0 },
	{ MSG_PDP_ISCONTEXT_ACTIVE_REQ,_T("MSG_PDP_ISCONTEXT_ACTIVE_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_IsPDPContextActive_Req_t, 0, 0},
	{ MSG_PDP_ISCONTEXT_ACTIVE_RSP,_T("MSG_PDP_ISCONTEXT_ACTIVE_RSP"), (xdrproc_t)xdr_CAPI2_PdpApi_IsPDPContextActive_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_PDP_ACTIVATESNDCPCONNECTION_REQ,_T("MSG_PDP_ACTIVATESNDCPCONNECTION_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_ActivateSNDCPConnection_Req_t, 0, 0},
	{ MSG_PDP_ACTIVATE_SNDCP_RSP,_T("MSG_PDP_ACTIVATE_SNDCP_RSP"), (xdrproc_t)xdr_CAPI2_PdpApi_ActivateSNDCPConnection_Rsp_t, sizeof( PDP_DataState_t ), 0 },
	{ MSG_PDP_GETPDPCONTEXT_REQ,_T("MSG_PDP_GETPDPCONTEXT_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_GetPDPContext_Req_t, 0, 0},
	{ MSG_PDP_GETPDPCONTEXT_RSP,_T("MSG_PDP_GETPDPCONTEXT_RSP"), (xdrproc_t)xdr_CAPI2_PdpApi_GetPDPContext_Rsp_t, sizeof( PDPDefaultContext_t ), 0 },
	{ MSG_PDP_GETPDPCONTEXT_CID_LIST_REQ,_T("MSG_PDP_GETPDPCONTEXT_CID_LIST_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_PDP_GETPDPCONTEXT_CID_LIST_RSP,_T("MSG_PDP_GETPDPCONTEXT_CID_LIST_RSP"), (xdrproc_t)xdr_CAPI2_PdpApi_GetDefinedPDPContextCidList_Rsp_t, sizeof( CAPI2_PdpApi_GetDefinedPDPContextCidList_Rsp_t ), 0 },
	{ MSG_SYSPARAM_BOOTLOADER_VER_REQ,_T("MSG_SYSPARAM_BOOTLOADER_VER_REQ"), (xdrproc_t) xdr_CAPI2_SYS_GetBootLoaderVersion_Req_t, 0, 0},
	{ MSG_SYSPARAM_BOOTLOADER_VER_RSP,_T("MSG_SYSPARAM_BOOTLOADER_VER_RSP"), (xdrproc_t)xdr_CAPI2_SYS_GetBootLoaderVersion_Rsp_t, sizeof( CAPI2_SYS_GetBootLoaderVersion_Rsp_t ), 0 },
	{ MSG_SYSPARAM_DSF_VER_REQ,_T("MSG_SYSPARAM_DSF_VER_REQ"), (xdrproc_t) xdr_CAPI2_SYS_GetDSFVersion_Req_t, 0, 0},
	{ MSG_SYSPARAM_DSF_VER_RSP,_T("MSG_SYSPARAM_DSF_VER_RSP"), (xdrproc_t)xdr_CAPI2_SYS_GetDSFVersion_Rsp_t, sizeof( CAPI2_SYS_GetDSFVersion_Rsp_t ), 0 },
	{ MSG_USIM_UST_DATA_REQ,_T("MSG_USIM_UST_DATA_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_USIM_UST_DATA_RSP,_T("MSG_USIM_UST_DATA_RSP"), (xdrproc_t)xdr_CAPI2_USimApi_GetUstData_Rsp_t, sizeof( CAPI2_USimApi_GetUstData_Rsp_t ), 0 },
	{ MSG_PATCH_GET_REVISION_REQ,_T("MSG_PATCH_GET_REVISION_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_PATCH_GET_REVISION_RSP,_T("MSG_PATCH_GET_REVISION_RSP"), (xdrproc_t)xdr_CAPI2_PATCH_GetRevision_Rsp_t, sizeof( UInt8 ), 0 },
	{ MSG_SS_SENDCALLFORWARDREQ_REQ,_T("MSG_SS_SENDCALLFORWARDREQ_REQ"), (xdrproc_t) xdr_CAPI2_SS_SendCallForwardReq_Req_t, 0, 0},
	{ MSG_SS_CALL_FORWARD_STATUS_RSP,_T("MSG_SS_CALL_FORWARD_STATUS_RSP"), (xdrproc_t)xdr_CAPI2_SS_SendCallForwardReq_Rsp_t, sizeof( CallForwardStatus_t ), 0 },
	{ MSG_SS_QUERYCALLFORWARDSTATUS_REQ,_T("MSG_SS_QUERYCALLFORWARDSTATUS_REQ"), (xdrproc_t) xdr_CAPI2_SS_QueryCallForwardStatus_Req_t, 0, 0},
	{ MSG_SS_SENDCALLBARRINGREQ_REQ,_T("MSG_SS_SENDCALLBARRINGREQ_REQ"), (xdrproc_t) xdr_CAPI2_SS_SendCallBarringReq_Req_t, 0, 0},
	{ MSG_SS_CALL_BARRING_STATUS_RSP,_T("MSG_SS_CALL_BARRING_STATUS_RSP"), (xdrproc_t)xdr_CAPI2_SS_SendCallBarringReq_Rsp_t, sizeof( CallBarringStatus_t ), 0 },
	{ MSG_SS_QUERYCALLBARRINGSTATUS_REQ,_T("MSG_SS_QUERYCALLBARRINGSTATUS_REQ"), (xdrproc_t) xdr_CAPI2_SS_QueryCallBarringStatus_Req_t, 0, 0},
	{ MSG_SS_SENDCALLBARRINGPWDCHANGEREQ_REQ,_T("MSG_SS_SENDCALLBARRINGPWDCHANGEREQ_REQ"), (xdrproc_t) xdr_CAPI2_SS_SendCallBarringPWDChangeReq_Req_t, 0, 0},
	{ MSG_SS_CALL_BARRING_PWD_CHANGE_RSP,_T("MSG_SS_CALL_BARRING_PWD_CHANGE_RSP"), (xdrproc_t)xdr_CAPI2_SS_SendCallBarringPWDChangeReq_Rsp_t, sizeof( NetworkCause_t ), 0 },
	{ MSG_SS_SENDCALLWAITINGREQ_REQ,_T("MSG_SS_SENDCALLWAITINGREQ_REQ"), (xdrproc_t) xdr_CAPI2_SS_SendCallWaitingReq_Req_t, 0, 0},
	{ MSG_SS_CALL_WAITING_STATUS_RSP,_T("MSG_SS_CALL_WAITING_STATUS_RSP"), (xdrproc_t)xdr_CAPI2_SS_SendCallWaitingReq_Rsp_t, sizeof( SS_ActivationStatus_t ), 0 },
	{ MSG_SS_QUERYCALLWAITINGSTATUS_REQ,_T("MSG_SS_QUERYCALLWAITINGSTATUS_REQ"), (xdrproc_t) xdr_CAPI2_SS_QueryCallWaitingStatus_Req_t, 0, 0},
	{ MSG_SS_QUERYCALLINGLINEIDSTATUS_REQ,_T("MSG_SS_QUERYCALLINGLINEIDSTATUS_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SS_CALLING_LINE_ID_STATUS_RSP,_T("MSG_SS_CALLING_LINE_ID_STATUS_RSP"), (xdrproc_t)xdr_CAPI2_SS_QueryCallingLineIDStatus_Rsp_t, sizeof( SS_ProvisionStatus_t ), 0 },
	{ MSG_SS_QUERYCONNECTEDLINEIDSTATUS_REQ,_T("MSG_SS_QUERYCONNECTEDLINEIDSTATUS_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SS_CONNECTED_LINE_STATUS_RSP,_T("MSG_SS_CONNECTED_LINE_STATUS_RSP"), (xdrproc_t)xdr_CAPI2_SS_QueryConnectedLineIDStatus_Rsp_t, sizeof( SS_ProvisionStatus_t ), 0 },
	{ MSG_SS_QUERYCALLINGLINERESTRICTIONSTATUS_REQ,_T("MSG_SS_QUERYCALLINGLINERESTRICTIONSTATUS_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SS_CALLING_LINE_RESTRICTION_STATUS_RSP,_T("MSG_SS_CALLING_LINE_RESTRICTION_STATUS_RSP"), (xdrproc_t)xdr_CAPI2_SS_QueryCallingLineRestrictionStatus_Rsp_t, sizeof( SS_ProvisionStatus_t ), 0 },
	{ MSG_SS_QUERYCONNECTEDLINERESTRICTIONSTATUS_REQ,_T("MSG_SS_QUERYCONNECTEDLINERESTRICTIONSTATUS_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SS_CONNECTED_LINE_RESTRICTION_STATUS_RSP,_T("MSG_SS_CONNECTED_LINE_RESTRICTION_STATUS_RSP"), (xdrproc_t)xdr_CAPI2_SS_QueryConnectedLineRestrictionStatus_Rsp_t, sizeof( SS_ProvisionStatus_t ), 0 },
	{ MSG_SS_QUERYCALLINGNAMEPRESENTSTATUS_REQ,_T("MSG_SS_QUERYCALLINGNAMEPRESENTSTATUS_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SS_CALLING_NAME_PRESENT_STATUS_RSP,_T("MSG_SS_CALLING_NAME_PRESENT_STATUS_RSP"), (xdrproc_t)xdr_CAPI2_SS_QueryCallingNAmePresentStatus_Rsp_t, sizeof( SS_ProvisionStatus_t ), 0 },
	{ MSG_SS_SETCALLINGLINEIDSTATUS_REQ,_T("MSG_SS_SETCALLINGLINEIDSTATUS_REQ"), (xdrproc_t) xdr_CAPI2_SS_SetCallingLineIDStatus_Req_t, 0, 0},
	{ MSG_MS_LOCAL_ELEM_NOTIFY_IND,_T("MSG_MS_LOCAL_ELEM_NOTIFY_IND"), (xdrproc_t)xdr_CAPI2_SS_SetCallingLineIDStatus_Rsp_t, sizeof( MS_LocalElemNotifyInd_t ), 0 },
	{ MSG_SS_SETCALLINGLINERESTRICTIONSTATUS_REQ,_T("MSG_SS_SETCALLINGLINERESTRICTIONSTATUS_REQ"), (xdrproc_t) xdr_CAPI2_SS_SetCallingLineRestrictionStatus_Req_t, 0, 0},
	{ MSG_SS_SETCONNECTEDLINEIDSTATUS_REQ,_T("MSG_SS_SETCONNECTEDLINEIDSTATUS_REQ"), (xdrproc_t) xdr_CAPI2_SS_SetConnectedLineIDStatus_Req_t, 0, 0},
	{ MSG_SS_SETCONNECTEDLINERESTRICTIONSTATUS_REQ,_T("MSG_SS_SETCONNECTEDLINERESTRICTIONSTATUS_REQ"), (xdrproc_t) xdr_CAPI2_SS_SetConnectedLineRestrictionStatus_Req_t, 0, 0},
	{ MSG_SS_SENDUSSDCONNECTREQ_REQ,_T("MSG_SS_SENDUSSDCONNECTREQ_REQ"), (xdrproc_t) xdr_CAPI2_SS_SendUSSDConnectReq_Req_t, 0, 0},
	{ MSG_USSD_DATA_RSP,_T("MSG_USSD_DATA_RSP"), (xdrproc_t)xdr_CAPI2_SS_SendUSSDConnectReq_Rsp_t, sizeof( USSDataInfo_t ), 0 },
	{ MSG_SS_SENDUSSDDATA_REQ,_T("MSG_SS_SENDUSSDDATA_REQ"), (xdrproc_t) xdr_CAPI2_SS_SendUSSDData_Req_t, 0, 0},
	{ MSG_SSAPI_DIALSTRSRVREQ_REQ,_T("MSG_SSAPI_DIALSTRSRVREQ_REQ"), (xdrproc_t) xdr_CAPI2_SsApi_DialStrSrvReq_Req_t, 0, 0},
	{ MSG_SSAPI_DIALSTRSRVREQ_RSP,_T("MSG_SSAPI_DIALSTRSRVREQ_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SS_ENDUSSDCONNECTREQ_REQ,_T("MSG_SS_ENDUSSDCONNECTREQ_REQ"), (xdrproc_t) xdr_CAPI2_SS_EndUSSDConnectReq_Req_t, 0, 0},
	{ MSG_SS_ENDUSSDCONNECTREQ_RSP,_T("MSG_SS_ENDUSSDCONNECTREQ_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SSAPI_SSSRVREQ_REQ,_T("MSG_SSAPI_SSSRVREQ_REQ"), (xdrproc_t) xdr_CAPI2_SsApi_SsSrvReq_Req_t, 0, 0},
	{ MSG_SSAPI_SSSRVREQ_RSP,_T("MSG_SSAPI_SSSRVREQ_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SSAPI_USSDSRVREQ_REQ,_T("MSG_SSAPI_USSDSRVREQ_REQ"), (xdrproc_t) xdr_CAPI2_SsApi_UssdSrvReq_Req_t, 0, 0},
	{ MSG_SSAPI_USSDSRVREQ_RSP,_T("MSG_SSAPI_USSDSRVREQ_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SSAPI_USSDDATAREQ_REQ,_T("MSG_SSAPI_USSDDATAREQ_REQ"), (xdrproc_t) xdr_CAPI2_SsApi_UssdDataReq_Req_t, 0, 0},
	{ MSG_SSAPI_USSDDATAREQ_RSP,_T("MSG_SSAPI_USSDDATAREQ_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SSAPI_SSRELEASEREQ_REQ,_T("MSG_SSAPI_SSRELEASEREQ_REQ"), (xdrproc_t) xdr_CAPI2_SsApi_SsReleaseReq_Req_t, 0, 0},
	{ MSG_SSAPI_SSRELEASEREQ_RSP,_T("MSG_SSAPI_SSRELEASEREQ_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SSAPI_DATAREQ_REQ,_T("MSG_SSAPI_DATAREQ_REQ"), (xdrproc_t) xdr_CAPI2_SsApi_DataReq_Req_t, 0, 0},
	{ MSG_SSAPI_DATAREQ_RSP,_T("MSG_SSAPI_DATAREQ_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SSAPI_DISPATCH_REQ,_T("MSG_SSAPI_DISPATCH_REQ"), (xdrproc_t) xdr_CAPI2_SS_SsApiReqDispatcher_Req_t, 0, 0},
	{ MSG_SSAPI_DISPATCH_RSP,_T("MSG_SSAPI_DISPATCH_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SS_GET_STR_REQ,_T("MSG_SS_GET_STR_REQ"), (xdrproc_t) xdr_CAPI2_SS_GetStr_Req_t, 0, 0},
	{ MSG_SS_GET_STR_RSP,_T("MSG_SS_GET_STR_RSP"), (xdrproc_t)xdr_CAPI2_SS_GetStr_Rsp_t, sizeof( UInt8 ), 0 },
	{ MSG_SS_SETCLIENTID_REQ,_T("MSG_SS_SETCLIENTID_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SS_SETCLIENTID_RSP,_T("MSG_SS_SETCLIENTID_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SS_GETCLIENTID_REQ,_T("MSG_SS_GETCLIENTID_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SS_GETCLIENTID_RSP,_T("MSG_SS_GETCLIENTID_RSP"), (xdrproc_t)xdr_CAPI2_SS_GetClientID_Rsp_t, sizeof( UInt8 ), 0 },
	{ MSG_SS_RESETCLIENTID_REQ,_T("MSG_SS_RESETCLIENTID_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SS_RESETCLIENTID_RSP,_T("MSG_SS_RESETCLIENTID_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SATK_GETCACHEDROOTMENUPTR_REQ,_T("MSG_SATK_GETCACHEDROOTMENUPTR_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SATK_GETCACHEDROOTMENUPTR_RSP,_T("MSG_SATK_GETCACHEDROOTMENUPTR_RSP"), (xdrproc_t)xdr_CAPI2_SatkApi_GetCachedRootMenuPtr_Rsp_t, sizeof( SetupMenu_t ), 0 },
	{ MSG_SATK_SENDUSERACTIVITYEVENT_REQ,_T("MSG_SATK_SENDUSERACTIVITYEVENT_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SATK_SENDUSERACTIVITYEVENT_RSP,_T("MSG_SATK_SENDUSERACTIVITYEVENT_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SATK_SENDIDLESCREENAVAIEVENT_REQ,_T("MSG_SATK_SENDIDLESCREENAVAIEVENT_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SATK_SENDIDLESCREENAVAIEVENT_RSP,_T("MSG_SATK_SENDIDLESCREENAVAIEVENT_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SATK_SENDLANGSELECTEVENT_REQ,_T("MSG_SATK_SENDLANGSELECTEVENT_REQ"), (xdrproc_t) xdr_CAPI2_SatkApi_SendLangSelectEvent_Req_t, 0, 0},
	{ MSG_SATK_SENDLANGSELECTEVENT_RSP,_T("MSG_SATK_SENDLANGSELECTEVENT_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SATK_SENDBROWSERTERMEVENT_REQ,_T("MSG_SATK_SENDBROWSERTERMEVENT_REQ"), (xdrproc_t) xdr_CAPI2_SatkApi_SendBrowserTermEvent_Req_t, 0, 0},
	{ MSG_SATK_SENDBROWSERTERMEVENT_RSP,_T("MSG_SATK_SENDBROWSERTERMEVENT_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SATK_CMDRESP_REQ,_T("MSG_SATK_CMDRESP_REQ"), (xdrproc_t) xdr_CAPI2_SatkApi_CmdResp_Req_t, 0, 0},
	{ MSG_SATK_CMDRESP_RSP,_T("MSG_SATK_CMDRESP_RSP"), (xdrproc_t)xdr_CAPI2_SatkApi_CmdResp_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SATK_DATASERVCMDRESP_REQ,_T("MSG_SATK_DATASERVCMDRESP_REQ"), (xdrproc_t) xdr_CAPI2_SatkApi_DataServCmdResp_Req_t, 0, 0},
	{ MSG_SATK_DATASERVCMDRESP_RSP,_T("MSG_SATK_DATASERVCMDRESP_RSP"), (xdrproc_t)xdr_CAPI2_SatkApi_DataServCmdResp_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SATK_SENDDATASERVREQ_REQ,_T("MSG_SATK_SENDDATASERVREQ_REQ"), (xdrproc_t) xdr_CAPI2_SatkApi_SendDataServReq_Req_t, 0, 0},
	{ MSG_SATK_SENDDATASERVREQ_RSP,_T("MSG_SATK_SENDDATASERVREQ_RSP"), (xdrproc_t)xdr_CAPI2_SatkApi_SendDataServReq_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SATK_SENDTERMINALRSP_REQ,_T("MSG_SATK_SENDTERMINALRSP_REQ"), (xdrproc_t) xdr_CAPI2_SatkApi_SendTerminalRsp_Req_t, 0, 0},
	{ MSG_SATK_SENDTERMINALRSP_RSP,_T("MSG_SATK_SENDTERMINALRSP_RSP"), (xdrproc_t)xdr_CAPI2_SatkApi_SendTerminalRsp_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SATK_SETTERMPROFILE_REQ,_T("MSG_SATK_SETTERMPROFILE_REQ"), (xdrproc_t) xdr_CAPI2_SatkApi_SetTermProfile_Req_t, 0, 0},
	{ MSG_SATK_SETTERMPROFILE_RSP,_T("MSG_SATK_SETTERMPROFILE_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SATK_SEND_ENVELOPE_CMD_REQ,_T("MSG_SATK_SEND_ENVELOPE_CMD_REQ"), (xdrproc_t) xdr_CAPI2_SatkApi_SendEnvelopeCmdReq_Req_t, 0, 0},
	{ MSG_STK_ENVELOPE_RSP,_T("MSG_STK_ENVELOPE_RSP"), (xdrproc_t)xdr_CAPI2_SatkApi_SendEnvelopeCmdReq_Rsp_t, sizeof( EnvelopeRspData_t ), 0 },
	{ MSG_STK_TERMINAL_RESPONSE_REQ,_T("MSG_STK_TERMINAL_RESPONSE_REQ"), (xdrproc_t) xdr_CAPI2_SatkApi_SendTerminalRspReq_Req_t, 0, 0},
	{ MSG_STK_TERMINAL_RESPONSE_RSP,_T("MSG_STK_TERMINAL_RESPONSE_RSP"), (xdrproc_t)xdr_CAPI2_SatkApi_SendTerminalRspReq_Rsp_t, sizeof( TerminalResponseRspData_t ), 0 },
	{ MSG_STK_SEND_BROWSING_STATUS_EVT_REQ,_T("MSG_STK_SEND_BROWSING_STATUS_EVT_REQ"), (xdrproc_t) xdr_CAPI2_StkApi_SendBrowsingStatusEvent_Req_t, 0, 0},
	{ MSG_STK_SEND_BROWSING_STATUS_EVT_RSP,_T("MSG_STK_SEND_BROWSING_STATUS_EVT_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SATK_SEND_CC_SETUP_REQ,_T("MSG_SATK_SEND_CC_SETUP_REQ"), (xdrproc_t) xdr_CAPI2_SatkApi_SendCcSetupReq_Req_t, 0, 0},
	{ MSG_STK_CALL_CONTROL_SETUP_RSP,_T("MSG_STK_CALL_CONTROL_SETUP_RSP"), (xdrproc_t)xdr_CAPI2_SatkApi_Control_Setup_Rsp_Rsp_t, sizeof( StkCallControlSetupRsp_t ), 0 },
	{ MSG_STK_CALL_CONTROL_SS_RSP,_T("MSG_STK_CALL_CONTROL_SS_RSP"), (xdrproc_t)xdr_CAPI2_SatkApi_Control_SS_Rsp_Rsp_t, sizeof( StkCallControlSsRsp_t ), 0 },
	{ MSG_STK_CALL_CONTROL_USSD_RSP,_T("MSG_STK_CALL_CONTROL_USSD_RSP"), (xdrproc_t)xdr_CAPI2_SatkApi_Control_USSD_Rsp_Rsp_t, sizeof( StkCallControlUssdRsp_t ), 0 },
	{ MSG_SATK_SEND_CC_SS_REQ,_T("MSG_SATK_SEND_CC_SS_REQ"), (xdrproc_t) xdr_CAPI2_SatkApi_SendCcSsReq_Req_t, 0, 0},
	{ MSG_SATK_SEND_CC_SS_RSP,_T("MSG_SATK_SEND_CC_SS_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SATK_SEND_CC_USSD_REQ,_T("MSG_SATK_SEND_CC_USSD_REQ"), (xdrproc_t) xdr_CAPI2_SatkApi_SendCcUssdReq_Req_t, 0, 0},
	{ MSG_SATK_SEND_CC_USSD_RSP,_T("MSG_SATK_SEND_CC_USSD_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SATK_SEND_CC_SMS_REQ,_T("MSG_SATK_SEND_CC_SMS_REQ"), (xdrproc_t) xdr_CAPI2_SatkApi_SendCcSmsReq_Req_t, 0, 0},
	{ MSG_SATK_SEND_CC_SMS_RSP,_T("MSG_SATK_SEND_CC_SMS_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_STK_CALL_CONTROL_SMS_RSP,_T("MSG_STK_CALL_CONTROL_SMS_RSP"), (xdrproc_t)xdr_CAPI2_SatkApi_Control_SMS_Rsp_Rsp_t, sizeof( StkCallControlSmsRsp_t ), 0 },
	{ MSG_LCS_CPMOLRREQ_REQ,_T("MSG_LCS_CPMOLRREQ_REQ"), (xdrproc_t) xdr_CAPI2_LCS_CpMoLrReq_Req_t, 0, 0},
	{ MSG_MNSS_CLIENT_LCS_SRV_RSP,_T("MSG_MNSS_CLIENT_LCS_SRV_RSP"), (xdrproc_t)xdr_CAPI2_LCS_CpMoLrReq_Rsp_t, sizeof( LCS_SrvRsp_t ), 0 },
	{ MSG_LCS_CPMOLRABORT_REQ,_T("MSG_LCS_CPMOLRABORT_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_LCS_CPMOLRABORT_RSP,_T("MSG_LCS_CPMOLRABORT_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_LCS_CPMTLRVERIFICATIONRSP_REQ,_T("MSG_LCS_CPMTLRVERIFICATIONRSP_REQ"), (xdrproc_t) xdr_CAPI2_LCS_CpMtLrVerificationRsp_Req_t, 0, 0},
	{ MSG_LCS_CPMTLRVERIFICATIONRSP_RSP,_T("MSG_LCS_CPMTLRVERIFICATIONRSP_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_LCS_CPMTLRRSP_REQ,_T("MSG_LCS_CPMTLRRSP_REQ"), (xdrproc_t) xdr_CAPI2_LCS_CpMtLrRsp_Req_t, 0, 0},
	{ MSG_LCS_CPMTLRRSP_RSP,_T("MSG_LCS_CPMTLRRSP_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_LCS_CPLOCUPDATERSP_REQ,_T("MSG_LCS_CPLOCUPDATERSP_REQ"), (xdrproc_t) xdr_CAPI2_LCS_CpLocUpdateRsp_Req_t, 0, 0},
	{ MSG_LCS_CPLOCUPDATERSP_RSP,_T("MSG_LCS_CPLOCUPDATERSP_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_LCS_DECODEPOSESTIMATE_REQ,_T("MSG_LCS_DECODEPOSESTIMATE_REQ"), (xdrproc_t) xdr_CAPI2_LCS_DecodePosEstimate_Req_t, 0, 0},
	{ MSG_LCS_DECODEPOSESTIMATE_RSP,_T("MSG_LCS_DECODEPOSESTIMATE_RSP"), (xdrproc_t)xdr_CAPI2_LCS_DecodePosEstimate_Rsp_t, sizeof( CAPI2_LCS_DecodePosEstimate_Rsp_t ), 0 },
	{ MSG_LCS_ENCODEASSISTANCEREQ_REQ,_T("MSG_LCS_ENCODEASSISTANCEREQ_REQ"), (xdrproc_t) xdr_CAPI2_LCS_EncodeAssistanceReq_Req_t, 0, 0},
	{ MSG_LCS_ENCODEASSISTANCEREQ_RSP,_T("MSG_LCS_ENCODEASSISTANCEREQ_RSP"), (xdrproc_t)xdr_CAPI2_LCS_EncodeAssistanceReq_Rsp_t, sizeof( CAPI2_LCS_EncodeAssistanceReq_Rsp_t ), 0 },
	{ MSG_LCS_FTT_SYNC_REQ,_T("MSG_LCS_FTT_SYNC_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_LCS_FTT_SYNC_RSP,_T("MSG_LCS_FTT_SYNC_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_CC_MAKEVOICECALL_REQ,_T("MSG_CC_MAKEVOICECALL_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_MakeVoiceCall_Req_t, 0, 0},
	{ MSG_CC_MAKEDATACALL_REQ,_T("MSG_CC_MAKEDATACALL_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_MakeDataCall_Req_t, 0, 0},
	{ MSG_CC_MAKEFAXCALL_REQ,_T("MSG_CC_MAKEFAXCALL_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_MakeFaxCall_Req_t, 0, 0},
	{ MSG_CC_MAKEVIDEOCALL_REQ,_T("MSG_CC_MAKEVIDEOCALL_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_MakeVideoCall_Req_t, 0, 0},
	{ MSG_CC_ENDCALL_REQ,_T("MSG_CC_ENDCALL_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_EndCall_Req_t, 0, 0},
	{ MSG_CC_ENDALLCALLS_REQ,_T("MSG_CC_ENDALLCALLS_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_CCAPI_ENDCALL_IMMEDIATE_REQ,_T("MSG_CCAPI_ENDCALL_IMMEDIATE_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_EndCallImmediate_Req_t, 0, 0},
	{ MSG_CCAPI_ENDALLCALLS_IMMEDIATE_REQ,_T("MSG_CCAPI_ENDALLCALLS_IMMEDIATE_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_CC_ENDMPTYCALLS_REQ,_T("MSG_CC_ENDMPTYCALLS_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_CC_ENDHELDCALL_REQ,_T("MSG_CC_ENDHELDCALL_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_CC_ACCEPTVOICECALL_REQ,_T("MSG_CC_ACCEPTVOICECALL_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_AcceptVoiceCall_Req_t, 0, 0},
	{ MSG_CC_ACCEPTDATACALL_REQ,_T("MSG_CC_ACCEPTDATACALL_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_AcceptDataCall_Req_t, 0, 0},
	{ MSG_CC_ACCEPTWAITINGCALL_REQ,_T("MSG_CC_ACCEPTWAITINGCALL_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_CC_ACCEPTVIDEOCALL_REQ,_T("MSG_CC_ACCEPTVIDEOCALL_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_AcceptVideoCall_Req_t, 0, 0},
	{ MSG_CC_HOLDCURRENTCALL_REQ,_T("MSG_CC_HOLDCURRENTCALL_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_CC_HOLDCALL_REQ,_T("MSG_CC_HOLDCALL_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_HoldCall_Req_t, 0, 0},
	{ MSG_CC_RETRIEVENEXTHELDCALL_REQ,_T("MSG_CC_RETRIEVENEXTHELDCALL_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_CC_RETRIEVECALL_REQ,_T("MSG_CC_RETRIEVECALL_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_RetrieveCall_Req_t, 0, 0},
	{ MSG_CC_SWAPCALL_REQ,_T("MSG_CC_SWAPCALL_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_SwapCall_Req_t, 0, 0},
	{ MSG_CC_SPLITCALL_REQ,_T("MSG_CC_SPLITCALL_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_SplitCall_Req_t, 0, 0},
	{ MSG_CC_JOINCALL_REQ,_T("MSG_CC_JOINCALL_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_JoinCall_Req_t, 0, 0},
	{ MSG_CC_TRANSFERCALL_REQ,_T("MSG_CC_TRANSFERCALL_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_TransferCall_Req_t, 0, 0},
	{ MSG_SMS_READY_IND,_T("MSG_SMS_READY_IND"), (xdrproc_t)xdr_CAPI2_smsModuleReady_Rsp_t, sizeof( smsModuleReady_t ), 0 },
	{ MSG_SMSPP_DELIVER_IND,_T("MSG_SMSPP_DELIVER_IND"), (xdrproc_t)xdr_CAPI2_SmsSimMsgDel_Rsp_t, sizeof( SmsSimMsg_t ), 0 },
	{ MSG_SMSPP_OTA_IND,_T("MSG_SMSPP_OTA_IND"), (xdrproc_t)xdr_CAPI2_SmsSimMsgOta_Rsp_t, sizeof( SmsSimMsg_t ), 0 },
	{ MSG_SMSPP_REGULAR_PUSH_IND,_T("MSG_SMSPP_REGULAR_PUSH_IND"), (xdrproc_t)xdr_CAPI2_SmsSimMsgPush_Rsp_t, sizeof( SmsSimMsg_t ), 0 },
	{ MSG_SMSSR_STORED_IND,_T("MSG_SMSSR_STORED_IND"), (xdrproc_t)xdr_CAPI2_SmsStoredSmsStatus_Rsp_t, sizeof( SmsIncMsgStoredResult_t ), 0 },
	{ MSG_SMSCB_STORED_IND,_T("MSG_SMSCB_STORED_IND"), (xdrproc_t)xdr_CAPI2_SmsStoredSmsCb_Rsp_t, sizeof( SmsIncMsgStoredResult_t ), 0 },
	{ MSG_SMSPP_STORED_IND,_T("MSG_SMSPP_STORED_IND"), (xdrproc_t)xdr_CAPI2_SmsStoredSms_Rsp_t, sizeof( SmsIncMsgStoredResult_t ), 0 },
	{ MSG_SMSCB_DATA_IND,_T("MSG_SMSCB_DATA_IND"), (xdrproc_t)xdr_CAPI2_SmsCbData_Rsp_t, sizeof( SmsStoredSmsCb_t ), 0 },
	{ MSG_HOMEZONE_STATUS_IND,_T("MSG_HOMEZONE_STATUS_IND"), (xdrproc_t)xdr_CAPI2_HomezoneIndData_Rsp_t, sizeof( HomezoneIndData_t ), 0 },
	{ MSG_SATK_EVENT_DISPLAY_TEXT,_T("MSG_SATK_EVENT_DISPLAY_TEXT"), (xdrproc_t)xdr_CAPI2_SATK_EventDataDisp_Rsp_t, sizeof( SATK_EventData_t ), 0 },
	{ MSG_SATK_EVENT_GET_INKEY,_T("MSG_SATK_EVENT_GET_INKEY"), (xdrproc_t)xdr_CAPI2_SATK_EventDataGetInKey_Rsp_t, sizeof( SATK_EventData_t ), 0 },
	{ MSG_SATK_EVENT_GET_INPUT,_T("MSG_SATK_EVENT_GET_INPUT"), (xdrproc_t)xdr_CAPI2_SATK_EventDataGetInput_Rsp_t, sizeof( SATK_EventData_t ), 0 },
	{ MSG_SATK_EVENT_PLAY_TONE,_T("MSG_SATK_EVENT_PLAY_TONE"), (xdrproc_t)xdr_CAPI2_SATK_EventDataPlayTone_Rsp_t, sizeof( SATK_EventData_t ), 0 },
	{ MSG_SATK_EVENT_ACTIVATE,_T("MSG_SATK_EVENT_ACTIVATE"), (xdrproc_t)xdr_CAPI2_SATK_EventDataActivate_Rsp_t, sizeof( SATK_EventData_t ), 0 },
	{ MSG_SATK_EVENT_SELECT_ITEM,_T("MSG_SATK_EVENT_SELECT_ITEM"), (xdrproc_t)xdr_CAPI2_SATK_EventDataSelItem_Rsp_t, sizeof( SATK_EventData_t ), 0 },
	{ MSG_SATK_EVENT_SEND_SS,_T("MSG_SATK_EVENT_SEND_SS"), (xdrproc_t)xdr_CAPI2_SATK_EventDataSendSS_Rsp_t, sizeof( SATK_EventData_t ), 0 },
	{ MSG_SATK_EVENT_SEND_USSD,_T("MSG_SATK_EVENT_SEND_USSD"), (xdrproc_t)xdr_CAPI2_SATK_EventDataSendUSSD_Rsp_t, sizeof( SATK_EventData_t ), 0 },
	{ MSG_SATK_EVENT_SETUP_CALL,_T("MSG_SATK_EVENT_SETUP_CALL"), (xdrproc_t)xdr_CAPI2_SATK_EventDataSetupCall_Rsp_t, sizeof( SATK_EventData_t ), 0 },
	{ MSG_SATK_EVENT_SETUP_MENU,_T("MSG_SATK_EVENT_SETUP_MENU"), (xdrproc_t)xdr_CAPI2_SATK_EventDataSetupMenu_Rsp_t, sizeof( SATK_EventData_t ), 0 },
	{ MSG_SATK_EVENT_REFRESH,_T("MSG_SATK_EVENT_REFRESH"), (xdrproc_t)xdr_CAPI2_SATK_EventDataRefresh_Rsp_t, sizeof( SATK_EventData_t ), 0 },
	{ MSG_SIM_FATAL_ERROR_IND,_T("MSG_SIM_FATAL_ERROR_IND"), (xdrproc_t)xdr_CAPI2_SIM_FatalInd_Rsp_t, sizeof( SIM_FATAL_ERROR_t ), 0 },
	{ MSG_SIM_SIM_RECOVER_IND,_T("MSG_SIM_SIM_RECOVER_IND"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_STK_CC_SETUPFAIL_IND,_T("MSG_STK_CC_SETUPFAIL_IND"), (xdrproc_t)xdr_CAPI2_SATK_CallSetupFail_Rsp_t, sizeof( StkCallSetupFail_t ), 0 },
	{ MSG_STK_CC_DISPLAY_IND,_T("MSG_STK_CC_DISPLAY_IND"), (xdrproc_t)xdr_CAPI2_SATK_CallControlDisplay_Rsp_t, sizeof( StkCallControlDisplay_t ), 0 },
	{ MSG_SATK_EVENT_SEND_SHORT_MSG,_T("MSG_SATK_EVENT_SEND_SHORT_MSG"), (xdrproc_t)xdr_CAPI2_SATK_EventDataShortMsg_Rsp_t, sizeof( SATK_EventData_t ), 0 },
	{ MSG_SATK_EVENT_SEND_DTMF,_T("MSG_SATK_EVENT_SEND_DTMF"), (xdrproc_t)xdr_CAPI2_SATK_EventDataSendDtmf_Rsp_t, sizeof( SATK_EventData_t ), 0 },
	{ MSG_SATK_EVENT_LAUNCH_BROWSER,_T("MSG_SATK_EVENT_LAUNCH_BROWSER"), (xdrproc_t)xdr_CAPI2_SATK_EventData_Rsp_t, sizeof( SATK_EventData_t ), 0 },
	{ MSG_SATK_EVENT_IDLEMODE_TEXT,_T("MSG_SATK_EVENT_IDLEMODE_TEXT"), (xdrproc_t)xdr_CAPI2_SATK_EventDataLaunchBrowser_Rsp_t, sizeof( SATK_EventData_t ), 0 },
	{ MSG_SATK_EVENT_DATA_SERVICE_REQ,_T("MSG_SATK_EVENT_DATA_SERVICE_REQ"), (xdrproc_t)xdr_CAPI2_SATK_EventDataServiceReq_Rsp_t, sizeof( SATK_EventData_t ), 0 },
	{ MSG_STK_LANG_NOTIFICATION_IND,_T("MSG_STK_LANG_NOTIFICATION_IND"), (xdrproc_t)xdr_CAPI2_StkLangNotificationLangNotify_Rsp_t, sizeof( StkLangNotification_t ), 0 },
	{ MSG_STK_LOCAL_BATTERY_STATE_REQ,_T("MSG_STK_LOCAL_BATTERY_STATE_REQ"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SIM_CACHED_DATA_READY_IND,_T("MSG_SIM_CACHED_DATA_READY_IND"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SIM_MMI_SETUP_EVENT_IND,_T("MSG_SIM_MMI_SETUP_EVENT_IND"), (xdrproc_t)xdr_CAPI2_SimMmiSetupInd_Rsp_t, sizeof( UInt16 ), 0 },
	{ MSG_STK_RUN_AT_IND,_T("MSG_STK_RUN_AT_IND"), (xdrproc_t)xdr_CAPI2_SATK_EventDataRunATInd_Rsp_t, sizeof( SATK_EventData_t ), 0 },
	{ MSG_STK_RUN_AT_REQ,_T("MSG_STK_RUN_AT_REQ"), (xdrproc_t)xdr_CAPI2_StkRunAtReq_Rsp_t, sizeof( RunAT_Request ), 0 },
	{ MSG_SATK_EVENT_STK_SESSION_END,_T("MSG_SATK_EVENT_STK_SESSION_END"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SATK_EVENT_PROV_LOCAL_LANG,_T("MSG_SATK_EVENT_PROV_LOCAL_LANG"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SATK_EVENT_PROV_LOCAL_DATE,_T("MSG_SATK_EVENT_PROV_LOCAL_DATE"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_STK_SEND_IPAT_REQ,_T("MSG_STK_SEND_IPAT_REQ"), (xdrproc_t)xdr_CAPI2_StkRunIpAtReq_Rsp_t, sizeof( RunAT_Request ), 0 },
	{ MSG_STK_GET_CHANNEL_STATUS_REQ,_T("MSG_STK_GET_CHANNEL_STATUS_REQ"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_STK_MENU_SELECTION_RES,_T("MSG_STK_MENU_SELECTION_RES"), (xdrproc_t)xdr_CAPI2_StkMenuSelectionRes_Rsp_t, sizeof( STKMenuSelectionResCode_t ), 0 },
	{ MSG_STK_TERMINATE_DATA_SERV_REQ,_T("MSG_STK_TERMINATE_DATA_SERV_REQ"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_VOICECALL_CONNECTED_IND,_T("MSG_VOICECALL_CONNECTED_IND"), (xdrproc_t)xdr_CAPI2_VOICECALL_CONNECTED_IND_Rsp_t, sizeof( VoiceCallConnectMsg_t ), 0 },
	{ MSG_VOICECALL_PRECONNECT_IND,_T("MSG_VOICECALL_PRECONNECT_IND"), (xdrproc_t)xdr_CAPI2_VOICECALL_PRECONNECTED_IND_Rsp_t, sizeof( VoiceCallPreConnectMsg_t ), 0 },
	{ MSG_SS_CALL_NOTIFICATION,_T("MSG_SS_CALL_NOTIFICATION"), (xdrproc_t)xdr_CAPI2_SS_CALL_NOTIFICATION_Rsp_t, sizeof( SS_CallNotification_t ), 0 },
	{ MSG_MNCC_CLIENT_NOTIFY_SS_IND,_T("MSG_MNCC_CLIENT_NOTIFY_SS_IND"), (xdrproc_t)xdr_CAPI2_MNCC_CLIENT_NOTIFY_SS_IND_Rsp_t, sizeof( CC_NotifySsInd_t ), 0 },
	{ MSG_SS_NOTIFY_CLOSED_USER_GROUP,_T("MSG_SS_NOTIFY_CLOSED_USER_GROUP"), (xdrproc_t)xdr_CAPI2_SS_NOTIFY_CLOSED_USER_GROUP_Rsp_t, sizeof( SS_CallNotification_t ), 0 },
	{ MSG_SS_NOTIFY_EXTENDED_CALL_TRANSFER,_T("MSG_SS_NOTIFY_EXTENDED_CALL_TRANSFER"), (xdrproc_t)xdr_CAPI2_SS_NOTIFY_EXTENDED_CALL_TRANSFER_Rsp_t, sizeof( SS_CallNotification_t ), 0 },
	{ MSG_SS_NOTIFY_CALLING_NAME_PRESENT,_T("MSG_SS_NOTIFY_CALLING_NAME_PRESENT"), (xdrproc_t)xdr_CAPI2_SS_NOTIFY_CALLING_NAME_PRESENT_Rsp_t, sizeof( SS_CallNotification_t ), 0 },
	{ MSG_CALL_STATUS_IND,_T("MSG_CALL_STATUS_IND"), (xdrproc_t)xdr_CAPI2_CALL_STATUS_IND_Rsp_t, sizeof( CallStatusMsg_t ), 0 },
	{ MSG_VOICECALL_ACTION_RSP,_T("MSG_VOICECALL_ACTION_RSP"), (xdrproc_t)xdr_CAPI2_VOICECALL_ACTION_RSP_Rsp_t, sizeof( VoiceCallActionMsg_t ), 0 },
	{ MSG_VOICECALL_RELEASE_IND,_T("MSG_VOICECALL_RELEASE_IND"), (xdrproc_t)xdr_CAPI2_VOICECALL_RELEASE_IND_Rsp_t, sizeof( VoiceCallReleaseMsg_t ), 0 },
	{ MSG_VOICECALL_RELEASE_CNF,_T("MSG_VOICECALL_RELEASE_CNF"), (xdrproc_t)xdr_CAPI2_VOICECALL_RELEASE_CNF_Rsp_t, sizeof( VoiceCallReleaseMsg_t ), 0 },
	{ MSG_INCOMING_CALL_IND,_T("MSG_INCOMING_CALL_IND"), (xdrproc_t)xdr_CAPI2_INCOMING_CALL_IND_Rsp_t, sizeof( CallReceiveMsg_t ), 0 },
	{ MSG_VOICECALL_WAITING_IND,_T("MSG_VOICECALL_WAITING_IND"), (xdrproc_t)xdr_CAPI2_VOICECALL_WAITING_IND_Rsp_t, sizeof( VoiceCallWaitingMsg_t ), 0 },
	{ MSG_CALL_AOCSTATUS_IND,_T("MSG_CALL_AOCSTATUS_IND"), (xdrproc_t)xdr_CAPI2_CALL_AOCSTATUS_IND_Rsp_t, sizeof( CallAOCStatusMsg_t ), 0 },
	{ MSG_CALL_CCM_IND,_T("MSG_CALL_CCM_IND"), (xdrproc_t)xdr_CAPI2_CALL_CCM_IND_Rsp_t, sizeof( CallCCMMsg_t ), 0 },
	{ MSG_CALL_CONNECTEDLINEID_IND,_T("MSG_CALL_CONNECTEDLINEID_IND"), (xdrproc_t)xdr_CAPI2_CALL_CONNECTEDLINEID_IND_Rsp_t, sizeof( UInt8 ), 0 },
	{ MSG_DATACALL_STATUS_IND,_T("MSG_DATACALL_STATUS_IND"), (xdrproc_t)xdr_CAPI2_DATACALL_STATUS_IND_Rsp_t, sizeof( DataCallStatusMsg_t ), 0 },
	{ MSG_DATACALL_RELEASE_IND,_T("MSG_DATACALL_RELEASE_IND"), (xdrproc_t)xdr_CAPI2_DATACALL_RELEASE_IND_Rsp_t, sizeof( DataCallReleaseMsg_t ), 0 },
	{ MSG_DATACALL_ECDC_IND,_T("MSG_DATACALL_ECDC_IND"), (xdrproc_t)xdr_CAPI2_DATACALL_ECDC_IND_Rsp_t, sizeof( DataECDCLinkMsg_t ), 0 },
	{ MSG_DATACALL_CONNECTED_IND,_T("MSG_DATACALL_CONNECTED_IND"), (xdrproc_t)xdr_CAPI2_DATACALL_CONNECTED_IND_Rsp_t, sizeof( DataCallConnectMsg_t ), 0 },
	{ MSG_API_CLIENT_CMD_IND,_T("MSG_API_CLIENT_CMD_IND"), (xdrproc_t)xdr_CAPI2_API_CLIENT_CMD_IND_Rsp_t, sizeof( ApiClientCmdInd_t ), 0 },
	{ MSG_DTMF_STATUS_IND,_T("MSG_DTMF_STATUS_IND"), (xdrproc_t)xdr_CAPI2_DTMF_STATUS_IND_Rsp_t, sizeof( ApiDtmfStatus_t ), 0 },
	{ MSG_CALL_USER_INFORMATION,_T("MSG_CALL_USER_INFORMATION"), (xdrproc_t)xdr_CAPI2_USER_INFORMATION_Rsp_t, sizeof( SS_UserInfo_t ), 0 },
	{ MSG_CIPHER_IND,_T("MSG_CIPHER_IND"), (xdrproc_t)xdr_CAPI2_CcCipherInd_Rsp_t, sizeof( CcCipherInd_t ), 0 },
	{ MSG_GPRS_ACTIVATE_IND,_T("MSG_GPRS_ACTIVATE_IND"), (xdrproc_t)xdr_CAPI2_GPRS_ACTIVATE_IND_Rsp_t, sizeof( GPRSActInd_t ), 0 },
	{ MSG_PDP_DEACTIVATION_IND,_T("MSG_PDP_DEACTIVATION_IND"), (xdrproc_t)xdr_CAPI2_GPRS_DEACTIVATE_IND_Rsp_t, sizeof( GPRSDeactInd_t ), 0 },
	{ MSG_PDP_PPP_SENDCLOSE_IND,_T("MSG_PDP_PPP_SENDCLOSE_IND"), (xdrproc_t)xdr_CAPI2_PDP_DEACTIVATION_IND_Rsp_t, sizeof( GPRSDeactInd_t ), 0 },
	{ MSG_GPRS_MODIFY_IND,_T("MSG_GPRS_MODIFY_IND"), (xdrproc_t)xdr_CAPI2_GPRS_MODIFY_IND_Rsp_t, sizeof( Inter_ModifyContextInd_t ), 0 },
	{ MSG_GPRS_REACT_IND,_T("MSG_GPRS_REACT_IND"), (xdrproc_t)xdr_CAPI2_GPRS_REACT_IND_Rsp_t, sizeof( GPRSReActInd_t ), 0 },
	{ MSG_PDP_ACTIVATION_NWI_IND,_T("MSG_PDP_ACTIVATION_NWI_IND"), (xdrproc_t)xdr_CAPI2_PDP_ACTIVATION_NWI_IND_Rsp_t, sizeof( PDP_ActivateNWI_Ind_t ), 0 },
	{ MSG_DATA_SUSPEND_IND,_T("MSG_DATA_SUSPEND_IND"), (xdrproc_t)xdr_CAPI2_DATA_SUSPEND_IND_Rsp_t, sizeof( GPRSSuspendInd_t ), 0 },
	{ MSG_DATA_RESUME_IND,_T("MSG_DATA_RESUME_IND"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_MNSS_CLIENT_LCS_SRV_IND,_T("MSG_MNSS_CLIENT_LCS_SRV_IND"), (xdrproc_t)xdr_CAPI2_MNSS_CLIENT_LCS_SRV_IND_Rsp_t, sizeof( LCS_SrvInd_t ), 0 },
	{ MSG_MNSS_CLIENT_LCS_SRV_REL,_T("MSG_MNSS_CLIENT_LCS_SRV_REL"), (xdrproc_t)xdr_CAPI2_MNSS_CLIENT_LCS_SRV_REL_Rsp_t, sizeof( SS_SrvRel_t ), 0 },
	{ MSG_MNSS_CLIENT_SS_SRV_RSP,_T("MSG_MNSS_CLIENT_SS_SRV_RSP"), (xdrproc_t)xdr_CAPI2_MNSS_CLIENT_SS_SRV_RSP_Rsp_t, sizeof( SS_SrvRsp_t ), 0 },
	{ MSG_MNSS_CLIENT_SS_SRV_REL,_T("MSG_MNSS_CLIENT_SS_SRV_REL"), (xdrproc_t)xdr_CAPI2_MNSS_CLIENT_SS_SRV_REL_Rsp_t, sizeof( SS_SrvRel_t ), 0 },
	{ MSG_MNSS_CLIENT_SS_SRV_IND,_T("MSG_MNSS_CLIENT_SS_SRV_IND"), (xdrproc_t)xdr_CAPI2_MNSS_CLIENT_SS_SRV_IND_Rsp_t, sizeof( SS_SrvInd_t ), 0 },
	{ MSG_MNSS_SATK_SS_SRV_RSP,_T("MSG_MNSS_SATK_SS_SRV_RSP"), (xdrproc_t)xdr_CAPI2_MNSS_SATK_SS_SRV_RSP_Rsp_t, sizeof( STK_SsSrvRel_t ), 0 },
	{ MSG_VM_WAITING_IND,_T("MSG_VM_WAITING_IND"), (xdrproc_t)xdr_CAPI2_SmsVoicemailInd_Rsp_t, sizeof( SmsVoicemailInd_t ), 0 },
	{ MSG_SIM_SMS_DATA_RSP,_T("MSG_SIM_SMS_DATA_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SMS_DATA_RSP_Rsp_t, sizeof( SmsSimMsg_t ), 0 },
	{ MSG_CC_GETCNAPNAME_REQ,_T("MSG_CC_GETCNAPNAME_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_GetCNAPName_Req_t, 0, 0},
	{ MSG_CC_GETCNAPNAME_RSP,_T("MSG_CC_GETCNAPNAME_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetCNAPName_Rsp_t, sizeof( CNAP_NAME_t ), 0 },
	{ MSG_SYSPARM_GET_HSUPA_SUPPORTED_REQ,_T("MSG_SYSPARM_GET_HSUPA_SUPPORTED_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SYSPARM_GET_HSUPA_SUPPORTED_RSP,_T("MSG_SYSPARM_GET_HSUPA_SUPPORTED_RSP"), (xdrproc_t)xdr_CAPI2_SYSPARM_GetHSUPASupported_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SYSPARM_GET_HSDPA_SUPPORTED_REQ,_T("MSG_SYSPARM_GET_HSDPA_SUPPORTED_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SYSPARM_GET_HSDPA_SUPPORTED_RSP,_T("MSG_SYSPARM_GET_HSDPA_SUPPORTED_RSP"), (xdrproc_t)xdr_CAPI2_SYSPARM_GetHSDPASupported_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_RADIO_ACTIVITY_IND,_T("MSG_RADIO_ACTIVITY_IND"), (xdrproc_t)xdr_CAPI2_MSRadioActivityInd_Rsp_t, sizeof( MSRadioActivityInd_t ), 0 },
	{ MSG_MS_FORCE_PS_REL_REQ,_T("MSG_MS_FORCE_PS_REL_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_MS_FORCE_PS_REL_RSP,_T("MSG_MS_FORCE_PS_REL_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_CC_ISCURRENTSTATEMPTY_REQ,_T("MSG_CC_ISCURRENTSTATEMPTY_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_IsCurrentStateMpty_Req_t, 0, 0},
	{ MSG_CC_ISCURRENTSTATEMPTY_RSP,_T("MSG_CC_ISCURRENTSTATEMPTY_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_IsCurrentStateMpty_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_PDP_GETPCHCONTEXTSTATE_REQ,_T("MSG_PDP_GETPCHCONTEXTSTATE_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_GetPCHContextState_Req_t, 0, 0},
	{ MSG_PDP_GETPCHCONTEXTSTATE_RSP,_T("MSG_PDP_GETPCHCONTEXTSTATE_RSP"), (xdrproc_t)xdr_CAPI2_PdpApi_GetPCHContextState_Rsp_t, sizeof( PCHContextState_t ), 0 },
	{ MSG_PDP_GETPCHCONTEXT_EX_REQ,_T("MSG_PDP_GETPCHCONTEXT_EX_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_GetPDPContextEx_Req_t, 0, 0},
	{ MSG_PDP_GETPCHCONTEXT_EX_RSP,_T("MSG_PDP_GETPCHCONTEXT_EX_RSP"), (xdrproc_t)xdr_CAPI2_PdpApi_GetPDPContextEx_Rsp_t, sizeof( CAPI2_PdpApi_GetPDPContextEx_Rsp_t ), 0 },
	{ MSG_SIM_PIN_LOCK_TYPE_REQ,_T("MSG_SIM_PIN_LOCK_TYPE_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SIM_PIN_LOCK_TYPE_RSP,_T("MSG_SIM_PIN_LOCK_TYPE_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_GetCurrLockedSimlockType_Rsp_t, sizeof( SIM_PIN_Status_t ), 0 },
	{ MSG_PLMN_SELECT_CNF,_T("MSG_PLMN_SELECT_CNF"), (xdrproc_t)xdr_CAPI2_PLMN_SELECT_CNF_Rsp_t, sizeof( UInt16 ), 0 },
	{ MSG_SIM_SEND_APDU_REQ,_T("MSG_SIM_SEND_APDU_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SubmitSelectFileSendApduReq_Req_t, 0, 0},
	{ MSG_SIM_SEND_APDU_RSP,_T("MSG_SIM_SEND_APDU_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_SubmitSelectFileSendApduReq_Rsp_t, sizeof( SimApduRsp_t ), 0 },
	{ MSG_STK_TERMINAL_PROFILE_IND,_T("MSG_STK_TERMINAL_PROFILE_IND"), (xdrproc_t)xdr_CAPI2_SatkApi_SendTermProfileInd_Rsp_t, sizeof( StkTermProfileInd_t ), 0 },
	{ MSG_SIM_MUL_REC_DATA_REQ,_T("MSG_SIM_MUL_REC_DATA_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SubmitMulRecordEFileReq_Req_t, 0, 65000},
	{ MSG_SIM_MUL_REC_DATA_RSP,_T("MSG_SIM_MUL_REC_DATA_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_SubmitMulRecordEFileReq_Rsp_t, sizeof( SIM_MUL_REC_DATA_t ), 65000 },
	{ MSG_STK_UICC_CAT_IND,_T("MSG_STK_UICC_CAT_IND"), (xdrproc_t)xdr_CAPI2_SatkApi_SendUiccCatInd_Rsp_t, sizeof( StkUiccCatInd_t ), 0 },
	{ MSG_SIM_SELECT_APPLICATION_REQ,_T("MSG_SIM_SELECT_APPLICATION_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_SendSelectApplicationReq_Req_t, 0, 0},
	{ MSG_SIM_SELECT_APPLICATION_RSP,_T("MSG_SIM_SELECT_APPLICATION_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_SendSelectApplicationReq_Rsp_t, sizeof( SIM_SELECT_APPLICATION_RES_t ), 0 },
	{ MSG_SIM_PEROFRM_STEERING_OF_ROAMING_REQ,_T("MSG_SIM_PEROFRM_STEERING_OF_ROAMING_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_PerformSteeringOfRoaming_Req_t, 0, 0},
	{ MSG_SIM_PEROFRM_STEERING_OF_ROAMING_RSP,_T("MSG_SIM_PEROFRM_STEERING_OF_ROAMING_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_STK_PROACTIVE_CMD_IND,_T("MSG_STK_PROACTIVE_CMD_IND"), (xdrproc_t)xdr_CAPI2_SatkApi_SendProactiveCmdInd_Rsp_t, sizeof( ProactiveCmdData_t ), 0 },
	{ MSG_STK_PROACTIVE_CMD_FETCHING_ON_OFF_REQ,_T("MSG_STK_PROACTIVE_CMD_FETCHING_ON_OFF_REQ"), (xdrproc_t) xdr_CAPI2_SatkApi_SendProactiveCmdFetchingOnOffReq_Req_t, 0, 0},
	{ MSG_STK_PROACTIVE_CMD_FETCHING_ON_OFF_RSP,_T("MSG_STK_PROACTIVE_CMD_FETCHING_ON_OFF_RSP"), (xdrproc_t)xdr_CAPI2_SatkApi_SendProactiveCmdFetchingOnOffReq_Rsp_t, sizeof( StkProactiveCmdFetchingOnOffRsp_t ), 0 },
	{ MSG_SATK_SEND_EXT_PROACTIVE_CMD_REQ,_T("MSG_SATK_SEND_EXT_PROACTIVE_CMD_REQ"), (xdrproc_t) xdr_CAPI2_SatkApi_SendExtProactiveCmdReq_Req_t, 0, 0},
	{ MSG_SATK_SEND_EXT_PROACTIVE_CMD_RSP,_T("MSG_SATK_SEND_EXT_PROACTIVE_CMD_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SATK_SEND_TERMINAL_PROFILE_REQ,_T("MSG_SATK_SEND_TERMINAL_PROFILE_REQ"), (xdrproc_t) xdr_CAPI2_SatkApi_SendTerminalProfileReq_Req_t, 0, 0},
	{ MSG_SATK_SEND_TERMINAL_PROFILE_RSP,_T("MSG_SATK_SEND_TERMINAL_PROFILE_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_STK_POLLING_INTERVAL_REQ,_T("MSG_STK_POLLING_INTERVAL_REQ"), (xdrproc_t) xdr_CAPI2_SatkApi_SendPollingIntervalReq_Req_t, 0, 0},
	{ MSG_STK_POLLING_INTERVAL_RSP,_T("MSG_STK_POLLING_INTERVAL_RSP"), (xdrproc_t)xdr_CAPI2_SatkApi_SendPollingIntervalReq_Rsp_t, sizeof( StkPollingIntervalRsp_t ), 0 },
	{ MSG_STK_EXT_PROACTIVE_CMD_IND,_T("MSG_STK_EXT_PROACTIVE_CMD_IND"), (xdrproc_t)xdr_CAPI2_SatkApi_SendExtProactiveCmdInd_Rsp_t, sizeof( StkExtProactiveCmdInd_t ), 0 },
	{ MSG_PDP_SETPDPACTIVATIONCCFLAG_REQ,_T("MSG_PDP_SETPDPACTIVATIONCCFLAG_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_SetPDPActivationCallControlFlag_Req_t, 0, 0},
	{ MSG_PDP_SETPDPACTIVATIONCCFLAG_RSP,_T("MSG_PDP_SETPDPACTIVATIONCCFLAG_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_PDP_GETPDPACTIVATIONCCFLAG_REQ,_T("MSG_PDP_GETPDPACTIVATIONCCFLAG_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_PDP_GETPDPACTIVATIONCCFLAG_RSP,_T("MSG_PDP_GETPDPACTIVATIONCCFLAG_RSP"), (xdrproc_t)xdr_CAPI2_PdpApi_GetPDPActivationCallControlFlag_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_PDP_SENDPDPACTIVATIONPDU_REQ,_T("MSG_PDP_SENDPDPACTIVATIONPDU_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_SendPDPActivateReq_PDU_Req_t, 0, 0},
	{ MSG_PDP_ACTIVATION_PDU_RSP,_T("MSG_PDP_ACTIVATION_PDU_RSP"), (xdrproc_t)xdr_CAPI2_PdpApi_SendPDPActivateReq_PDU_Rsp_t, sizeof( PDP_SendPDPActivatePDUReq_Rsp_t ), 0 },
	{ MSG_PDP_REJECTNWIACTIVATION_REQ,_T("MSG_PDP_REJECTNWIACTIVATION_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_RejectNWIPDPActivation_Req_t, 0, 0},
	{ MSG_PDP_REJECTNWIACTIVATION_RSP,_T("MSG_PDP_REJECTNWIACTIVATION_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_PDP_SETBEARERCTRLMODE_REQ,_T("MSG_PDP_SETBEARERCTRLMODE_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_SetPDPBearerCtrlMode_Req_t, 0, 0},
	{ MSG_PDP_SETBEARERCTRLMODE_RSP,_T("MSG_PDP_SETBEARERCTRLMODE_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_PDP_GETBEARERCTRLMODE_REQ,_T("MSG_PDP_GETBEARERCTRLMODE_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_PDP_GETBEARERCTRLMODE_RSP,_T("MSG_PDP_GETBEARERCTRLMODE_RSP"), (xdrproc_t)xdr_CAPI2_PdpApi_GetPDPBearerCtrlMode_Rsp_t, sizeof( UInt8 ), 0 },
	{ MSG_PDP_REJECTSECNWIACTIVATION_REQ,_T("MSG_PDP_REJECTSECNWIACTIVATION_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_RejectSecNWIPDPActivation_Req_t, 0, 0},
	{ MSG_PDP_REJECTSECNWIACTIVATION_RSP,_T("MSG_PDP_REJECTSECNWIACTIVATION_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_PDP_SETPDPNWICONTROLFLAG_REQ,_T("MSG_PDP_SETPDPNWICONTROLFLAG_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_SetPDPNWIControlFlag_Req_t, 0, 0},
	{ MSG_PDP_SETPDPNWICONTROLFLAG_RSP,_T("MSG_PDP_SETPDPNWICONTROLFLAG_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_PDP_GETPDPNWICONTROLFLAG_REQ,_T("MSG_PDP_GETPDPNWICONTROLFLAG_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_PDP_GETPDPNWICONTROLFLAG_RSP,_T("MSG_PDP_GETPDPNWICONTROLFLAG_RSP"), (xdrproc_t)xdr_CAPI2_PdpApi_GetPDPNWIControlFlag_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_PDP_ACTIVATION_SEC_NWI_IND,_T("MSG_PDP_ACTIVATION_SEC_NWI_IND"), (xdrproc_t)xdr_CAPI2_PDP_ACTIVATION_SEC_NWI_IND_Rsp_t, sizeof( PDP_ActivateSecNWI_Ind_t ), 0 },
	{ MSG_PDP_CHECKUMTSTFT_REQ,_T("MSG_PDP_CHECKUMTSTFT_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_CheckUMTSTft_Req_t, 0, 0},
	{ MSG_PDP_CHECKUMTSTFT_RSP,_T("MSG_PDP_CHECKUMTSTFT_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_PDP_ISANYPDPCONTEXTACTIVE_REQ,_T("MSG_PDP_ISANYPDPCONTEXTACTIVE_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_PDP_ISANYPDPCONTEXTACTIVE_RSP,_T("MSG_PDP_ISANYPDPCONTEXTACTIVE_RSP"), (xdrproc_t)xdr_CAPI2_PdpApi_IsAnyPDPContextActive_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_PDP_ISANYPDPCONTEXTPENDING_REQ,_T("MSG_PDP_ISANYPDPCONTEXTPENDING_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_PDP_ISANYPDPCONTEXTPENDING_RSP,_T("MSG_PDP_ISANYPDPCONTEXTPENDING_RSP"), (xdrproc_t)xdr_CAPI2_PdpApi_IsAnyPDPContextActivePending_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_CCAPI_SET_ELEMENT_REQ,_T("MSG_CCAPI_SET_ELEMENT_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_SetElement_Req_t, 0, 0},
	{ MSG_CCAPI_SET_ELEMENT_RSP,_T("MSG_CCAPI_SET_ELEMENT_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_CCAPI_GET_ELEMENT_REQ,_T("MSG_CCAPI_GET_ELEMENT_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_GetElement_Req_t, 0, 0},
	{ MSG_CCAPI_GET_ELEMENT_RSP,_T("MSG_CCAPI_GET_ELEMENT_RSP"), (xdrproc_t)xdr_CAPI2_CcApi_GetElement_Rsp_t, sizeof( CAPI2_CcApi_GetElement_Rsp_t ), 0 },
	{ MSG_WL_PS_SET_FILTER_LIST_REQ,_T("MSG_WL_PS_SET_FILTER_LIST_REQ"), (xdrproc_t) xdr_CAPI2_WL_PsSetFilterList_Req_t, 0, 0},
	{ MSG_WL_PS_SET_FILTER_LIST_RSP,_T("MSG_WL_PS_SET_FILTER_LIST_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_PDP_GETPROTCONFIGOPTIONS_REQ,_T("MSG_PDP_GETPROTCONFIGOPTIONS_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_GetProtConfigOptions_Req_t, 0, 0},
	{ MSG_PDP_GETPROTCONFIGOPTIONS_RSP,_T("MSG_PDP_GETPROTCONFIGOPTIONS_RSP"), (xdrproc_t)xdr_CAPI2_PdpApi_GetProtConfigOptions_Rsp_t, sizeof( CAPI2_PdpApi_GetProtConfigOptions_Rsp_t ), 0 },
	{ MSG_PDP_SETPROTCONFIGOPTIONS_REQ,_T("MSG_PDP_SETPROTCONFIGOPTIONS_REQ"), (xdrproc_t) xdr_CAPI2_PdpApi_SetProtConfigOptions_Req_t, 0, 0},
	{ MSG_PDP_SETPROTCONFIGOPTIONS_RSP,_T("MSG_PDP_SETPROTCONFIGOPTIONS_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_LCS_RRLP_SEND_DATA_REQ,_T("MSG_LCS_RRLP_SEND_DATA_REQ"), (xdrproc_t) xdr_CAPI2_LcsApi_RrlpSendDataToNetwork_Req_t, 0, 0},
	{ MSG_LCS_RRLP_SEND_DATA_RSP,_T("MSG_LCS_RRLP_SEND_DATA_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_LCS_RRLP_REG_HDL_REQ,_T("MSG_LCS_RRLP_REG_HDL_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_LCS_RRLP_REG_HDL_RSP,_T("MSG_LCS_RRLP_REG_HDL_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_LCS_RRC_REG_HDL_REQ,_T("MSG_LCS_RRC_REG_HDL_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_LCS_RRC_REG_HDL_RSP,_T("MSG_LCS_RRC_REG_HDL_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_LCS_RRC_SEND_DL_DCCH_REQ,_T("MSG_LCS_RRC_SEND_DL_DCCH_REQ"), (xdrproc_t) xdr_CAPI2_LcsApi_RrcSendUlDcch_Req_t, 0, 0},
	{ MSG_LCS_RRC_SEND_DL_DCCH_RSP,_T("MSG_LCS_RRC_SEND_DL_DCCH_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_LCS_RRC_MEAS_CTRL_FAILURE_REQ,_T("MSG_LCS_RRC_MEAS_CTRL_FAILURE_REQ"), (xdrproc_t) xdr_CAPI2_LcsApi_RrcMeasCtrlFailure_Req_t, 0, 0},
	{ MSG_LCS_RRC_MEAS_CTRL_FAILURE_RSP,_T("MSG_LCS_RRC_MEAS_CTRL_FAILURE_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_LCS_RRC_STAT_REQ,_T("MSG_LCS_RRC_STAT_REQ"), (xdrproc_t) xdr_CAPI2_LcsApi_RrcStatus_Req_t, 0, 0},
	{ MSG_LCS_RRC_STAT_RSP,_T("MSG_LCS_RRC_STAT_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SIM_POWER_ON_OFF_SIM_REQ,_T("MSG_SIM_POWER_ON_OFF_SIM_REQ"), (xdrproc_t) xdr_CAPI2_SimApi_PowerOnOffSim_Req_t, 0, 0},
	{ MSG_SIM_POWER_ON_OFF_SIM_RSP,_T("MSG_SIM_POWER_ON_OFF_SIM_RSP"), (xdrproc_t)xdr_CAPI2_SimApi_PowerOnOffSim_Rsp_t, sizeof( SIMAccess_t ), 0 },
	{ MSG_SYS_SET_PAGING_STATUS_REQ,_T("MSG_SYS_SET_PAGING_STATUS_REQ"), (xdrproc_t) xdr_CAPI2_PhoneCtrlApi_SetPagingStatus_Req_t, 0, 0},
	{ MSG_SYS_SET_PAGING_STATUS_RSP,_T("MSG_SYS_SET_PAGING_STATUS_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_LCS_GET_GPS_CAP_REQ,_T("MSG_LCS_GET_GPS_CAP_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_LCS_GET_GPS_CAP_RSP,_T("MSG_LCS_GET_GPS_CAP_RSP"), (xdrproc_t)xdr_CAPI2_LcsApi_GetGpsCapabilities_Rsp_t, sizeof( CAPI2_LcsApi_GetGpsCapabilities_Rsp_t ), 0 },
	{ MSG_LCS_SET_GPS_CAP_REQ,_T("MSG_LCS_SET_GPS_CAP_REQ"), (xdrproc_t) xdr_CAPI2_LcsApi_SetGpsCapabilities_Req_t, 0, 0},
	{ MSG_LCS_SET_GPS_CAP_RSP,_T("MSG_LCS_SET_GPS_CAP_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_CCAPI_ABORTDTMF_TONE_REQ,_T("MSG_CCAPI_ABORTDTMF_TONE_REQ"), (xdrproc_t) xdr_CAPI2_CcApi_AbortDtmfTone_Req_t, 0, 0},
	{ MSG_CCAPI_ABORTDTMF_TONE_RSP,_T("MSG_CCAPI_ABORTDTMF_TONE_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_MS_SET_RAT_BAND_EX_REQ,_T("MSG_MS_SET_RAT_BAND_EX_REQ"), (xdrproc_t) xdr_CAPI2_NetRegApi_SetSupportedRATandBandEx_Req_t, 0, 0},
	{ MSG_MS_SET_RAT_BAND_EX_RSP,_T("MSG_MS_SET_RAT_BAND_EX_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_SMSSR_REPORT_IND,_T("MSG_SMSSR_REPORT_IND"), (xdrproc_t)xdr_CAPI2_SmsReportInd_Rsp_t, sizeof( SmsSimMsg_t ), 0 },
#endif //DEVELOPMENT_CAPI2_XDR_UNION_MAPPING



#ifdef DEVELOPMENT_CAPI2_XDR_UNION_DECLARE

	CAPI2_MS_IsGSMRegistered_Rsp_t   CAPI2_MS_IsGSMRegistered_Rsp;
	CAPI2_MS_IsGPRSRegistered_Rsp_t   CAPI2_MS_IsGPRSRegistered_Rsp;
	CAPI2_MS_GetGSMRegCause_Rsp_t   CAPI2_MS_GetGSMRegCause_Rsp;
	CAPI2_MS_GetGPRSRegCause_Rsp_t   CAPI2_MS_GetGPRSRegCause_Rsp;
	CAPI2_MS_GetRegisteredLAC_Rsp_t   CAPI2_MS_GetRegisteredLAC_Rsp;
	CAPI2_MS_GetPlmnMCC_Rsp_t   CAPI2_MS_GetPlmnMCC_Rsp;
	CAPI2_MS_GetPlmnMNC_Rsp_t   CAPI2_MS_GetPlmnMNC_Rsp;
	CAPI2_MS_IsGprsAllowed_Rsp_t   CAPI2_MS_IsGprsAllowed_Rsp;
	CAPI2_MS_GetCurrentRAT_Rsp_t   CAPI2_MS_GetCurrentRAT_Rsp;
	CAPI2_MS_GetCurrentBand_Rsp_t   CAPI2_MS_GetCurrentBand_Rsp;
	CAPI2_SimApi_UpdateSMSCapExceededFlag_Req_t   CAPI2_SimApi_UpdateSMSCapExceededFlag_Req;
	CAPI2_NetRegApi_SelectBand_Req_t   CAPI2_NetRegApi_SelectBand_Req;
	CAPI2_NetRegApi_SetSupportedRATandBand_Req_t   CAPI2_NetRegApi_SetSupportedRATandBand_Req;
	CAPI2_NetRegApi_CellLock_Req_t   CAPI2_NetRegApi_CellLock_Req;
	CAPI2_PLMN_GetCountryByMcc_Req_t   CAPI2_PLMN_GetCountryByMcc_Req;
	CAPI2_PLMN_GetCountryByMcc_Rsp_t   CAPI2_PLMN_GetCountryByMcc_Rsp;
	CAPI2_MS_GetPLMNEntryByIndex_Req_t   CAPI2_MS_GetPLMNEntryByIndex_Req;
	CAPI2_MS_GetPLMNEntryByIndex_Rsp_t   CAPI2_MS_GetPLMNEntryByIndex_Rsp;
	CAPI2_MS_GetPLMNListSize_Rsp_t   CAPI2_MS_GetPLMNListSize_Rsp;
	CAPI2_MS_GetPLMNByCode_Req_t   CAPI2_MS_GetPLMNByCode_Req;
	CAPI2_MS_GetPLMNByCode_Rsp_t   CAPI2_MS_GetPLMNByCode_Rsp;
	CAPI2_NetRegApi_PlmnSelect_Req_t   CAPI2_NetRegApi_PlmnSelect_Req;
	CAPI2_NetRegApi_PlmnSelect_Rsp_t   CAPI2_NetRegApi_PlmnSelect_Rsp;
	CAPI2_MS_GetPlmnMode_Rsp_t   CAPI2_MS_GetPlmnMode_Rsp;
	CAPI2_NetRegApi_SetPlmnMode_Req_t   CAPI2_NetRegApi_SetPlmnMode_Req;
	CAPI2_MS_GetPlmnFormat_Rsp_t   CAPI2_MS_GetPlmnFormat_Rsp;
	CAPI2_MS_SetPlmnFormat_Req_t   CAPI2_MS_SetPlmnFormat_Req;
	CAPI2_MS_IsMatchedPLMN_Req_t   CAPI2_MS_IsMatchedPLMN_Req;
	CAPI2_MS_IsMatchedPLMN_Rsp_t   CAPI2_MS_IsMatchedPLMN_Rsp;
	CAPI2_NetRegApi_GetPLMNNameByCode_Req_t   CAPI2_NetRegApi_GetPLMNNameByCode_Req;
	CAPI2_NetRegApi_GetPLMNNameByCode_Rsp_t   CAPI2_NetRegApi_GetPLMNNameByCode_Rsp;
	CAPI2_PhoneCtrlApi_GetSystemState_Rsp_t   CAPI2_PhoneCtrlApi_GetSystemState_Rsp;
	CAPI2_PhoneCtrlApi_SetSystemState_Req_t   CAPI2_PhoneCtrlApi_SetSystemState_Req;
	CAPI2_PhoneCtrlApi_GetRxSignalInfo_Rsp_t   CAPI2_PhoneCtrlApi_GetRxSignalInfo_Rsp;
	CAPI2_SYS_GetGSMRegistrationStatus_Rsp_t   CAPI2_SYS_GetGSMRegistrationStatus_Rsp;
	CAPI2_SYS_GetGPRSRegistrationStatus_Rsp_t   CAPI2_SYS_GetGPRSRegistrationStatus_Rsp;
	CAPI2_SYS_IsRegisteredGSMOrGPRS_Rsp_t   CAPI2_SYS_IsRegisteredGSMOrGPRS_Rsp;
	CAPI2_MS_IsRegisteredHomePLMN_Rsp_t   CAPI2_MS_IsRegisteredHomePLMN_Rsp;
	CAPI2_PhoneCtrlApi_SetPowerDownTimer_Req_t   CAPI2_PhoneCtrlApi_SetPowerDownTimer_Req;
	CAPI2_SimApi_GetSmsParamRecNum_Rsp_t   CAPI2_SimApi_GetSmsParamRecNum_Rsp;
	CAPI2_SimApi_GetSmsMemExceededFlag_Rsp_t   CAPI2_SimApi_GetSmsMemExceededFlag_Rsp;
	CAPI2_SimApi_IsTestSIM_Rsp_t   CAPI2_SimApi_IsTestSIM_Rsp;
	CAPI2_SimApi_IsPINRequired_Rsp_t   CAPI2_SimApi_IsPINRequired_Rsp;
	CAPI2_SimApi_GetCardPhase_Rsp_t   CAPI2_SimApi_GetCardPhase_Rsp;
	CAPI2_SimApi_GetSIMType_Rsp_t   CAPI2_SimApi_GetSIMType_Rsp;
	CAPI2_SimApi_GetPresentStatus_Rsp_t   CAPI2_SimApi_GetPresentStatus_Rsp;
	CAPI2_SimApi_IsOperationRestricted_Rsp_t   CAPI2_SimApi_IsOperationRestricted_Rsp;
	CAPI2_SimApi_IsPINBlocked_Req_t   CAPI2_SimApi_IsPINBlocked_Req;
	CAPI2_SimApi_IsPINBlocked_Rsp_t   CAPI2_SimApi_IsPINBlocked_Rsp;
	CAPI2_SimApi_IsPUKBlocked_Req_t   CAPI2_SimApi_IsPUKBlocked_Req;
	CAPI2_SimApi_IsPUKBlocked_Rsp_t   CAPI2_SimApi_IsPUKBlocked_Rsp;
	CAPI2_SimApi_IsInvalidSIM_Rsp_t   CAPI2_SimApi_IsInvalidSIM_Rsp;
	CAPI2_SimApi_DetectSim_Rsp_t   CAPI2_SimApi_DetectSim_Rsp;
	CAPI2_SimApi_GetRuimSuppFlag_Rsp_t   CAPI2_SimApi_GetRuimSuppFlag_Rsp;
	CAPI2_SimApi_SendVerifyChvReq_Req_t   CAPI2_SimApi_SendVerifyChvReq_Req;
	CAPI2_SimApi_SendVerifyChvReq_Rsp_t   CAPI2_SimApi_SendVerifyChvReq_Rsp;
	CAPI2_SimApi_SendChangeChvReq_Req_t   CAPI2_SimApi_SendChangeChvReq_Req;
	CAPI2_SimApi_SendChangeChvReq_Rsp_t   CAPI2_SimApi_SendChangeChvReq_Rsp;
	CAPI2_SimApi_SendSetChv1OnOffReq_Req_t   CAPI2_SimApi_SendSetChv1OnOffReq_Req;
	CAPI2_SimApi_SendSetChv1OnOffReq_Rsp_t   CAPI2_SimApi_SendSetChv1OnOffReq_Rsp;
	CAPI2_SimApi_SendUnblockChvReq_Req_t   CAPI2_SimApi_SendUnblockChvReq_Req;
	CAPI2_SimApi_SendUnblockChvReq_Rsp_t   CAPI2_SimApi_SendUnblockChvReq_Rsp;
	CAPI2_SimApi_SendSetOperStateReq_Req_t   CAPI2_SimApi_SendSetOperStateReq_Req;
	CAPI2_SimApi_SendSetOperStateReq_Rsp_t   CAPI2_SimApi_SendSetOperStateReq_Rsp;
	CAPI2_SimApi_IsPbkAccessAllowed_Req_t   CAPI2_SimApi_IsPbkAccessAllowed_Req;
	CAPI2_SimApi_IsPbkAccessAllowed_Rsp_t   CAPI2_SimApi_IsPbkAccessAllowed_Rsp;
	CAPI2_SimApi_SendPbkInfoReq_Req_t   CAPI2_SimApi_SendPbkInfoReq_Req;
	CAPI2_SimApi_SendPbkInfoReq_Rsp_t   CAPI2_SimApi_SendPbkInfoReq_Rsp;
	CAPI2_SimApi_SendReadAcmMaxReq_Rsp_t   CAPI2_SimApi_SendReadAcmMaxReq_Rsp;
	CAPI2_SimApi_SendWriteAcmMaxReq_Req_t   CAPI2_SimApi_SendWriteAcmMaxReq_Req;
	CAPI2_SimApi_SendWriteAcmMaxReq_Rsp_t   CAPI2_SimApi_SendWriteAcmMaxReq_Rsp;
	CAPI2_SimApi_SendReadAcmReq_Rsp_t   CAPI2_SimApi_SendReadAcmReq_Rsp;
	CAPI2_SimApi_SendWriteAcmReq_Req_t   CAPI2_SimApi_SendWriteAcmReq_Req;
	CAPI2_SimApi_SendWriteAcmReq_Rsp_t   CAPI2_SimApi_SendWriteAcmReq_Rsp;
	CAPI2_SimApi_SendIncreaseAcmReq_Req_t   CAPI2_SimApi_SendIncreaseAcmReq_Req;
	CAPI2_SimApi_SendIncreaseAcmReq_Rsp_t   CAPI2_SimApi_SendIncreaseAcmReq_Rsp;
	CAPI2_SimApi_SendReadSvcProvNameReq_Rsp_t   CAPI2_SimApi_SendReadSvcProvNameReq_Rsp;
	CAPI2_SimApi_SendReadPuctReq_Rsp_t   CAPI2_SimApi_SendReadPuctReq_Rsp;
	CAPI2_SimApi_GetServiceStatus_Req_t   CAPI2_SimApi_GetServiceStatus_Req;
	CAPI2_SimApi_GetServiceStatus_Rsp_t   CAPI2_SimApi_GetServiceStatus_Rsp;
	CAPI2_SimApi_GetPinStatus_Rsp_t   CAPI2_SimApi_GetPinStatus_Rsp;
	CAPI2_SimApi_IsPinOK_Rsp_t   CAPI2_SimApi_IsPinOK_Rsp;
	CAPI2_SimApi_GetIMSI_Rsp_t   CAPI2_SimApi_GetIMSI_Rsp;
	CAPI2_SimApi_GetGID1_Rsp_t   CAPI2_SimApi_GetGID1_Rsp;
	CAPI2_SimApi_GetGID2_Rsp_t   CAPI2_SimApi_GetGID2_Rsp;
	CAPI2_SimApi_GetHomePlmn_Rsp_t   CAPI2_SimApi_GetHomePlmn_Rsp;
	CAPI2_simmiApi_GetMasterFileId_Req_t   CAPI2_simmiApi_GetMasterFileId_Req;
	CAPI2_simmiApi_GetMasterFileId_Rsp_t   CAPI2_simmiApi_GetMasterFileId_Rsp;
	CAPI2_SimApi_SendOpenSocketReq_Rsp_t   CAPI2_SimApi_SendOpenSocketReq_Rsp;
	CAPI2_SimApi_SendSelectAppiReq_Req_t   CAPI2_SimApi_SendSelectAppiReq_Req;
	CAPI2_SimApi_SendSelectAppiReq_Rsp_t   CAPI2_SimApi_SendSelectAppiReq_Rsp;
	CAPI2_SimApi_SendDeactivateAppiReq_Req_t   CAPI2_SimApi_SendDeactivateAppiReq_Req;
	CAPI2_SimApi_SendDeactivateAppiReq_Rsp_t   CAPI2_SimApi_SendDeactivateAppiReq_Rsp;
	CAPI2_SimApi_SendCloseSocketReq_Req_t   CAPI2_SimApi_SendCloseSocketReq_Req;
	CAPI2_SimApi_SendCloseSocketReq_Rsp_t   CAPI2_SimApi_SendCloseSocketReq_Rsp;
	CAPI2_SimApi_GetAtrData_Rsp_t   CAPI2_SimApi_GetAtrData_Rsp;
	CAPI2_SimApi_SubmitDFileInfoReq_Req_t   CAPI2_SimApi_SubmitDFileInfoReq_Req;
	CAPI2_SimApi_SubmitDFileInfoReq_Rsp_t   CAPI2_SimApi_SubmitDFileInfoReq_Rsp;
	CAPI2_SimApi_SubmitEFileInfoReq_Req_t   CAPI2_SimApi_SubmitEFileInfoReq_Req;
	CAPI2_SimApi_SubmitEFileInfoReq_Rsp_t   CAPI2_SimApi_SubmitEFileInfoReq_Rsp;
	CAPI2_SimApi_SendEFileInfoReq_Req_t   CAPI2_SimApi_SendEFileInfoReq_Req;
	CAPI2_SimApi_SendDFileInfoReq_Req_t   CAPI2_SimApi_SendDFileInfoReq_Req;
	CAPI2_SimApi_SubmitWholeBinaryEFileReadReq_Req_t   CAPI2_SimApi_SubmitWholeBinaryEFileReadReq_Req;
	CAPI2_SimApi_SubmitWholeBinaryEFileReadReq_Rsp_t   CAPI2_SimApi_SubmitWholeBinaryEFileReadReq_Rsp;
	CAPI2_SimApi_SendWholeBinaryEFileReadReq_Req_t   CAPI2_SimApi_SendWholeBinaryEFileReadReq_Req;
	CAPI2_SimApi_SubmitBinaryEFileReadReq_Req_t   CAPI2_SimApi_SubmitBinaryEFileReadReq_Req;
	CAPI2_SimApi_SendBinaryEFileReadReq_Req_t   CAPI2_SimApi_SendBinaryEFileReadReq_Req;
	CAPI2_SimApi_SubmitRecordEFileReadReq_Req_t   CAPI2_SimApi_SubmitRecordEFileReadReq_Req;
	CAPI2_SimApi_SendRecordEFileReadReq_Req_t   CAPI2_SimApi_SendRecordEFileReadReq_Req;
	CAPI2_SimApi_SubmitBinaryEFileUpdateReq_Req_t   CAPI2_SimApi_SubmitBinaryEFileUpdateReq_Req;
	CAPI2_SimApi_SubmitBinaryEFileUpdateReq_Rsp_t   CAPI2_SimApi_SubmitBinaryEFileUpdateReq_Rsp;
	CAPI2_SimApi_SendBinaryEFileUpdateReq_Req_t   CAPI2_SimApi_SendBinaryEFileUpdateReq_Req;
	CAPI2_SimApi_SubmitLinearEFileUpdateReq_Req_t   CAPI2_SimApi_SubmitLinearEFileUpdateReq_Req;
	CAPI2_SimApi_SendLinearEFileUpdateReq_Req_t   CAPI2_SimApi_SendLinearEFileUpdateReq_Req;
	CAPI2_SimApi_SubmitSeekRecordReq_Req_t   CAPI2_SimApi_SubmitSeekRecordReq_Req;
	CAPI2_SimApi_SubmitSeekRecordReq_Rsp_t   CAPI2_SimApi_SubmitSeekRecordReq_Rsp;
	CAPI2_SimApi_SendSeekRecordReq_Req_t   CAPI2_SimApi_SendSeekRecordReq_Req;
	CAPI2_SimApi_SubmitCyclicEFileUpdateReq_Req_t   CAPI2_SimApi_SubmitCyclicEFileUpdateReq_Req;
	CAPI2_SimApi_SendCyclicEFileUpdateReq_Req_t   CAPI2_SimApi_SendCyclicEFileUpdateReq_Req;
	CAPI2_SimApi_SendRemainingPinAttemptReq_Rsp_t   CAPI2_SimApi_SendRemainingPinAttemptReq_Rsp;
	CAPI2_SimApi_IsCachedDataReady_Rsp_t   CAPI2_SimApi_IsCachedDataReady_Rsp;
	CAPI2_SimApi_GetServiceCodeStatus_Req_t   CAPI2_SimApi_GetServiceCodeStatus_Req;
	CAPI2_SimApi_GetServiceCodeStatus_Rsp_t   CAPI2_SimApi_GetServiceCodeStatus_Rsp;
	CAPI2_SimApi_CheckCphsService_Req_t   CAPI2_SimApi_CheckCphsService_Req;
	CAPI2_SimApi_CheckCphsService_Rsp_t   CAPI2_SimApi_CheckCphsService_Rsp;
	CAPI2_SimApi_GetCphsPhase_Rsp_t   CAPI2_SimApi_GetCphsPhase_Rsp;
	CAPI2_SimApi_GetSmsSca_Req_t   CAPI2_SimApi_GetSmsSca_Req;
	CAPI2_SimApi_GetSmsSca_Rsp_t   CAPI2_SimApi_GetSmsSca_Rsp;
	CAPI2_SimApi_GetIccid_Rsp_t   CAPI2_SimApi_GetIccid_Rsp;
	CAPI2_SimApi_IsALSEnabled_Rsp_t   CAPI2_SimApi_IsALSEnabled_Rsp;
	CAPI2_SimApi_GetAlsDefaultLine_Rsp_t   CAPI2_SimApi_GetAlsDefaultLine_Rsp;
	CAPI2_SimApi_SetAlsDefaultLine_Req_t   CAPI2_SimApi_SetAlsDefaultLine_Req;
	CAPI2_SimApi_GetCallForwardUnconditionalFlag_Rsp_t   CAPI2_SimApi_GetCallForwardUnconditionalFlag_Rsp;
	CAPI2_SimApi_GetApplicationType_Rsp_t   CAPI2_SimApi_GetApplicationType_Rsp;
	CAPI2_SimApi_SendWritePuctReq_Req_t   CAPI2_SimApi_SendWritePuctReq_Req;
	CAPI2_SimApi_SendWritePuctReq_Rsp_t   CAPI2_SimApi_SendWritePuctReq_Rsp;
	CAPI2_SimApi_SubmitRestrictedAccessReq_Req_t   CAPI2_SimApi_SubmitRestrictedAccessReq_Req;
	CAPI2_SimApi_SubmitRestrictedAccessReq_Rsp_t   CAPI2_SimApi_SubmitRestrictedAccessReq_Rsp;
	CAPI2_SIM_SendDetectionInd_Rsp_t   CAPI2_SIM_SendDetectionInd_Rsp;
	CAPI2_MS_GsmRegStatusInd_Rsp_t   CAPI2_MS_GsmRegStatusInd_Rsp;
	CAPI2_MS_GprsRegStatusInd_Rsp_t   CAPI2_MS_GprsRegStatusInd_Rsp;
	CAPI2_MS_NetworkNameInd_Rsp_t   CAPI2_MS_NetworkNameInd_Rsp;
	CAPI2_MS_LsaInd_Rsp_t   CAPI2_MS_LsaInd_Rsp;
	CAPI2_MS_RssiInd_Rsp_t   CAPI2_MS_RssiInd_Rsp;
	CAPI2_MS_SignalChangeInd_Rsp_t   CAPI2_MS_SignalChangeInd_Rsp;
	CAPI2_MS_PlmnListInd_Rsp_t   CAPI2_MS_PlmnListInd_Rsp;
	CAPI2_MS_TimeZoneInd_Rsp_t   CAPI2_MS_TimeZoneInd_Rsp;
	CAPI2_MS_GetSystemRAT_Rsp_t   CAPI2_MS_GetSystemRAT_Rsp;
	CAPI2_MS_GetSupportedRAT_Rsp_t   CAPI2_MS_GetSupportedRAT_Rsp;
	CAPI2_MS_GetSystemBand_Rsp_t   CAPI2_MS_GetSystemBand_Rsp;
	CAPI2_MS_GetSupportedBand_Rsp_t   CAPI2_MS_GetSupportedBand_Rsp;
	CAPI2_SYSPARM_GetMSClass_Rsp_t   CAPI2_SYSPARM_GetMSClass_Rsp;
	CAPI2_SYSPARM_GetManufacturerName_Rsp_t   CAPI2_SYSPARM_GetManufacturerName_Rsp;
	CAPI2_SYSPARM_GetModelName_Rsp_t   CAPI2_SYSPARM_GetModelName_Rsp;
	CAPI2_SYSPARM_GetSWVersion_Rsp_t   CAPI2_SYSPARM_GetSWVersion_Rsp;
	CAPI2_SYSPARM_GetEGPRSMSClass_Rsp_t   CAPI2_SYSPARM_GetEGPRSMSClass_Rsp;
	CAPI2_SimApi_SendNumOfPLMNEntryReq_Req_t   CAPI2_SimApi_SendNumOfPLMNEntryReq_Req;
	CAPI2_SimApi_SendNumOfPLMNEntryReq_Rsp_t   CAPI2_SimApi_SendNumOfPLMNEntryReq_Rsp;
	CAPI2_SimApi_SendReadPLMNEntryReq_Req_t   CAPI2_SimApi_SendReadPLMNEntryReq_Req;
	CAPI2_SimApi_SendReadPLMNEntryReq_Rsp_t   CAPI2_SimApi_SendReadPLMNEntryReq_Rsp;
	CAPI2_SimApi_SendWriteMulPLMNEntryReq_Req_t   CAPI2_SimApi_SendWriteMulPLMNEntryReq_Req;
	CAPI2_SimApi_SendWriteMulPLMNEntryReq_Rsp_t   CAPI2_SimApi_SendWriteMulPLMNEntryReq_Rsp;
	CAPI2_SYS_SetRegisteredEventMask_Req_t   CAPI2_SYS_SetRegisteredEventMask_Req;
	CAPI2_SYS_SetRegisteredEventMask_Rsp_t   CAPI2_SYS_SetRegisteredEventMask_Rsp;
	CAPI2_SYS_SetFilteredEventMask_Req_t   CAPI2_SYS_SetFilteredEventMask_Req;
	CAPI2_SYS_SetFilteredEventMask_Rsp_t   CAPI2_SYS_SetFilteredEventMask_Rsp;
	CAPI2_PhoneCtrlApi_SetRssiThreshold_Req_t   CAPI2_PhoneCtrlApi_SetRssiThreshold_Req;
	CAPI2_SYSPARM_GetChanMode_Rsp_t   CAPI2_SYSPARM_GetChanMode_Rsp;
	CAPI2_SYSPARM_GetClassmark_Rsp_t   CAPI2_SYSPARM_GetClassmark_Rsp;
	CAPI2_SYSPARM_GetSysparmIndPartFileVersion_Rsp_t   CAPI2_SYSPARM_GetSysparmIndPartFileVersion_Rsp;
	CAPI2_SYSPARM_SetDARPCfg_Req_t   CAPI2_SYSPARM_SetDARPCfg_Req;
	CAPI2_SYSPARM_SetEGPRSMSClass_Req_t   CAPI2_SYSPARM_SetEGPRSMSClass_Req;
	CAPI2_SYSPARM_SetGPRSMSClass_Req_t   CAPI2_SYSPARM_SetGPRSMSClass_Req;
	CAPI2_TestCmds_Req_t   CAPI2_TestCmds_Req;
	CAPI2_SatkApi_SendPlayToneRes_Req_t   CAPI2_SatkApi_SendPlayToneRes_Req;
	CAPI2_SATK_SendSetupCallRes_Req_t   CAPI2_SATK_SendSetupCallRes_Req;
	CAPI2_PbkApi_SetFdnCheck_Req_t   CAPI2_PbkApi_SetFdnCheck_Req;
	CAPI2_PbkApi_GetFdnCheck_Rsp_t   CAPI2_PbkApi_GetFdnCheck_Rsp;
	CAPI2_SMS_ConfigureMEStorage_Req_t   CAPI2_SMS_ConfigureMEStorage_Req;
	CAPI2_MsDbApi_SetElement_Req_t   CAPI2_MsDbApi_SetElement_Req;
	CAPI2_MsDbApi_GetElement_Req_t   CAPI2_MsDbApi_GetElement_Req;
	CAPI2_MsDbApi_GetElement_Rsp_t   CAPI2_MsDbApi_GetElement_Rsp;
	CAPI2_USimApi_IsApplicationSupported_Req_t   CAPI2_USimApi_IsApplicationSupported_Req;
	CAPI2_USimApi_IsApplicationSupported_Rsp_t   CAPI2_USimApi_IsApplicationSupported_Rsp;
	CAPI2_USimApi_IsAllowedAPN_Req_t   CAPI2_USimApi_IsAllowedAPN_Req;
	CAPI2_USimApi_IsAllowedAPN_Rsp_t   CAPI2_USimApi_IsAllowedAPN_Rsp;
	CAPI2_USimApi_GetNumOfAPN_Rsp_t   CAPI2_USimApi_GetNumOfAPN_Rsp;
	CAPI2_USimApi_GetAPNEntry_Req_t   CAPI2_USimApi_GetAPNEntry_Req;
	CAPI2_USimApi_GetAPNEntry_Rsp_t   CAPI2_USimApi_GetAPNEntry_Rsp;
	CAPI2_USimApi_IsEstServActivated_Req_t   CAPI2_USimApi_IsEstServActivated_Req;
	CAPI2_USimApi_IsEstServActivated_Rsp_t   CAPI2_USimApi_IsEstServActivated_Rsp;
	CAPI2_USimApi_SendSetEstServReq_Req_t   CAPI2_USimApi_SendSetEstServReq_Req;
	CAPI2_USimApi_SendSetEstServReq_Rsp_t   CAPI2_USimApi_SendSetEstServReq_Rsp;
	CAPI2_USimApi_SendWriteAPNReq_Req_t   CAPI2_USimApi_SendWriteAPNReq_Req;
	CAPI2_USimApi_SendWriteAPNReq_Rsp_t   CAPI2_USimApi_SendWriteAPNReq_Rsp;
	CAPI2_USimApi_SendDeleteAllAPNReq_Rsp_t   CAPI2_USimApi_SendDeleteAllAPNReq_Rsp;
	CAPI2_USimApi_GetRatModeSetting_Rsp_t   CAPI2_USimApi_GetRatModeSetting_Rsp;
	CAPI2_MS_GetGPRSRegState_Rsp_t   CAPI2_MS_GetGPRSRegState_Rsp;
	CAPI2_MS_GetGSMRegState_Rsp_t   CAPI2_MS_GetGSMRegState_Rsp;
	CAPI2_MS_GetRegisteredCellInfo_Rsp_t   CAPI2_MS_GetRegisteredCellInfo_Rsp;
	CAPI2_MS_SetMEPowerClass_Req_t   CAPI2_MS_SetMEPowerClass_Req;
	CAPI2_USimApi_GetServiceStatus_Req_t   CAPI2_USimApi_GetServiceStatus_Req;
	CAPI2_USimApi_GetServiceStatus_Rsp_t   CAPI2_USimApi_GetServiceStatus_Rsp;
	CAPI2_SimApi_IsAllowedAPN_Req_t   CAPI2_SimApi_IsAllowedAPN_Req;
	CAPI2_SimApi_IsAllowedAPN_Rsp_t   CAPI2_SimApi_IsAllowedAPN_Rsp;
	CAPI2_SmsApi_GetSmsMaxCapacity_Req_t   CAPI2_SmsApi_GetSmsMaxCapacity_Req;
	CAPI2_SmsApi_GetSmsMaxCapacity_Rsp_t   CAPI2_SmsApi_GetSmsMaxCapacity_Rsp;
	CAPI2_SmsApi_RetrieveMaxCBChnlLength_Rsp_t   CAPI2_SmsApi_RetrieveMaxCBChnlLength_Rsp;
	CAPI2_SimApi_IsBdnOperationRestricted_Rsp_t   CAPI2_SimApi_IsBdnOperationRestricted_Rsp;
	CAPI2_SimApi_SendPreferredPlmnUpdateInd_Req_t   CAPI2_SimApi_SendPreferredPlmnUpdateInd_Req;
	CAPI2_SimApi_SendSetBdnReq_Req_t   CAPI2_SimApi_SendSetBdnReq_Req;
	CAPI2_SimApi_SendSetBdnReq_Rsp_t   CAPI2_SimApi_SendSetBdnReq_Rsp;
	CAPI2_SimApi_PowerOnOffCard_Req_t   CAPI2_SimApi_PowerOnOffCard_Req;
	CAPI2_SimApi_PowerOnOffCard_Rsp_t   CAPI2_SimApi_PowerOnOffCard_Rsp;
	CAPI2_SimApi_GetRawAtr_Rsp_t   CAPI2_SimApi_GetRawAtr_Rsp;
	CAPI2_SimApi_Set_Protocol_Req_t   CAPI2_SimApi_Set_Protocol_Req;
	CAPI2_SimApi_Get_Protocol_Rsp_t   CAPI2_SimApi_Get_Protocol_Rsp;
	CAPI2_SimApi_SendGenericApduCmd_Req_t   CAPI2_SimApi_SendGenericApduCmd_Req;
	CAPI2_SimApi_SendGenericApduCmd_Rsp_t   CAPI2_SimApi_SendGenericApduCmd_Rsp;
	CAPI2_SIM_GetSimInterface_Rsp_t   CAPI2_SIM_GetSimInterface_Rsp;
	CAPI2_NetRegApi_SetPlmnSelectRat_Req_t   CAPI2_NetRegApi_SetPlmnSelectRat_Req;
	CAPI2_MS_IsDeRegisterInProgress_Rsp_t   CAPI2_MS_IsDeRegisterInProgress_Rsp;
	CAPI2_MS_IsRegisterInProgress_Rsp_t   CAPI2_MS_IsRegisterInProgress_Rsp;
	CAPI2_PbkApi_SendUsimHdkReadReq_Rsp_t   CAPI2_PbkApi_SendUsimHdkReadReq_Rsp;
	CAPI2_PbkApi_SendUsimHdkUpdateReq_Req_t   CAPI2_PbkApi_SendUsimHdkUpdateReq_Req;
	CAPI2_PbkApi_SendUsimHdkUpdateReq_Rsp_t   CAPI2_PbkApi_SendUsimHdkUpdateReq_Rsp;
	CAPI2_PbkApi_SendUsimAasReadReq_Req_t   CAPI2_PbkApi_SendUsimAasReadReq_Req;
	CAPI2_PbkApi_SendUsimAasReadReq_Rsp_t   CAPI2_PbkApi_SendUsimAasReadReq_Rsp;
	CAPI2_PbkApi_SendUsimAasUpdateReq_Req_t   CAPI2_PbkApi_SendUsimAasUpdateReq_Req;
	CAPI2_PbkApi_SendUsimAasUpdateReq_Rsp_t   CAPI2_PbkApi_SendUsimAasUpdateReq_Rsp;
	CAPI2_PbkApi_SendUsimGasReadReq_Req_t   CAPI2_PbkApi_SendUsimGasReadReq_Req;
	CAPI2_PbkApi_SendUsimGasUpdateReq_Req_t   CAPI2_PbkApi_SendUsimGasUpdateReq_Req;
	CAPI2_PbkApi_SendUsimAasInfoReq_Req_t   CAPI2_PbkApi_SendUsimAasInfoReq_Req;
	CAPI2_PbkApi_SendUsimAasInfoReq_Rsp_t   CAPI2_PbkApi_SendUsimAasInfoReq_Rsp;
	CAPI2_PbkApi_SendUsimGasInfoReq_Req_t   CAPI2_PbkApi_SendUsimGasInfoReq_Req;
	CAPI2_PbkApi_SendUsimGasInfoReq_Rsp_t   CAPI2_PbkApi_SendUsimGasInfoReq_Rsp;
	CAPI2_MS_MeasureReportInd_Rsp_t   CAPI2_MS_MeasureReportInd_Rsp;
	CAPI2_DiagApi_MeasurmentReportReq_Req_t   CAPI2_DiagApi_MeasurmentReportReq_Req;
	CAPI2_MS_StatusInd_Rsp_t   CAPI2_MS_StatusInd_Rsp;
	CAPI2_SYSPARM_GetActualLowVoltReading_Rsp_t   CAPI2_SYSPARM_GetActualLowVoltReading_Rsp;
	CAPI2_SYSPARM_GetActual4p2VoltReading_Rsp_t   CAPI2_SYSPARM_GetActual4p2VoltReading_Rsp;
	CAPI2_SmsApi_SendSMSCommandTxtReq_Req_t   CAPI2_SmsApi_SendSMSCommandTxtReq_Req;
	CAPI2_SmsApi_SendSMSCommandPduReq_Req_t   CAPI2_SmsApi_SendSMSCommandPduReq_Req;
	CAPI2_SmsApi_SendPDUAckToNetwork_Req_t   CAPI2_SmsApi_SendPDUAckToNetwork_Req;
	CAPI2_SmsApi_StartCellBroadcastWithChnlReq_Req_t   CAPI2_SmsApi_StartCellBroadcastWithChnlReq_Req;
	CAPI2_SmsApi_SetMoSmsTpMr_Req_t   CAPI2_SmsApi_SetMoSmsTpMr_Req;
	CAPI2_SIMLOCKApi_SetStatus_Req_t   CAPI2_SIMLOCKApi_SetStatus_Req;
	CAPI2_MS_Ue3gStatusInd_Rsp_t   CAPI2_MS_Ue3gStatusInd_Rsp;
	CAPI2_DIAG_ApiCellLockReq_Req_t   CAPI2_DIAG_ApiCellLockReq_Req;
	CAPI2_DIAG_ApiCellLockStatus_Rsp_t   CAPI2_DIAG_ApiCellLockStatus_Rsp;
	CAPI2_MS_SetRuaReadyTimer_Req_t   CAPI2_MS_SetRuaReadyTimer_Req;
	CAPI2_LCS_RrlpDataInd_Rsp_t   CAPI2_LCS_RrlpDataInd_Rsp;
	CAPI2_LCS_RrlpReset_Rsp_t   CAPI2_LCS_RrlpReset_Rsp;
	CAPI2_LCS_RrcAssistDataInd_Rsp_t   CAPI2_LCS_RrcAssistDataInd_Rsp;
	CAPI2_LCS_RrcMeasCtrlInd_Rsp_t   CAPI2_LCS_RrcMeasCtrlInd_Rsp;
	CAPI2_LCS_RrcSysInfoInd_Rsp_t   CAPI2_LCS_RrcSysInfoInd_Rsp;
	CAPI2_LCS_RrcUeStateInd_Rsp_t   CAPI2_LCS_RrcUeStateInd_Rsp;
	CAPI2_LCS_RrcStopMeasInd_Rsp_t   CAPI2_LCS_RrcStopMeasInd_Rsp;
	CAPI2_LCS_RrcReset_Rsp_t   CAPI2_LCS_RrcReset_Rsp;
	CAPI2_CcApi_IsThereEmergencyCall_Rsp_t   CAPI2_CcApi_IsThereEmergencyCall_Rsp;
	CAPI2_ServingCellInfoInd_Rsp_t   CAPI2_ServingCellInfoInd_Rsp;
	CAPI2_SYS_EnableCellInfoMsg_Req_t   CAPI2_SYS_EnableCellInfoMsg_Req;
	CAPI2_LCS_L1_bb_isLocked_Req_t   CAPI2_LCS_L1_bb_isLocked_Req;
	CAPI2_LCS_L1_bb_isLocked_Rsp_t   CAPI2_LCS_L1_bb_isLocked_Rsp;
	CAPI2_SMSPP_AppSpecificInd_Rsp_t   CAPI2_SMSPP_AppSpecificInd_Rsp;
	CAPI2_DIALSTR_ParseGetCallType_Req_t   CAPI2_DIALSTR_ParseGetCallType_Req;
	CAPI2_DIALSTR_ParseGetCallType_Rsp_t   CAPI2_DIALSTR_ParseGetCallType_Rsp;
	CAPI2_LCS_FttCalcDeltaTime_Req_t   CAPI2_LCS_FttCalcDeltaTime_Req;
	CAPI2_LCS_FttCalcDeltaTime_Rsp_t   CAPI2_LCS_FttCalcDeltaTime_Rsp;
	CAPI2_LCS_SyncResult_Rsp_t   CAPI2_LCS_SyncResult_Rsp;
	CAPI2_NetRegApi_ForcedReadyStateReq_Req_t   CAPI2_NetRegApi_ForcedReadyStateReq_Req;
	CAPI2_SIMLOCK_GetStatus_Req_t   CAPI2_SIMLOCK_GetStatus_Req;
	CAPI2_SIMLOCK_GetStatus_RSP_Rsp_t   CAPI2_SIMLOCK_GetStatus_RSP_Rsp;
	CAPI2_DIALSTR_IsValidString_Req_t   CAPI2_DIALSTR_IsValidString_Req;
	CAPI2_DIALSTR_IsValidString_Rsp_t   CAPI2_DIALSTR_IsValidString_Rsp;
	CAPI2_UTIL_Cause2NetworkCause_Req_t   CAPI2_UTIL_Cause2NetworkCause_Req;
	CAPI2_UTIL_Cause2NetworkCause_Rsp_t   CAPI2_UTIL_Cause2NetworkCause_Rsp;
	CAPI2_UTIL_ErrCodeToNetCause_Req_t   CAPI2_UTIL_ErrCodeToNetCause_Req;
	CAPI2_UTIL_ErrCodeToNetCause_Rsp_t   CAPI2_UTIL_ErrCodeToNetCause_Rsp;
	CAPI2_IsGprsDialStr_Req_t   CAPI2_IsGprsDialStr_Req;
	CAPI2_IsGprsDialStr_Rsp_t   CAPI2_IsGprsDialStr_Rsp;
	CAPI2_UTIL_GetNumOffsetInSsStr_Req_t   CAPI2_UTIL_GetNumOffsetInSsStr_Req;
	CAPI2_UTIL_GetNumOffsetInSsStr_Rsp_t   CAPI2_UTIL_GetNumOffsetInSsStr_Rsp;
	CAPI2_CipherAlgInd_Rsp_t   CAPI2_CipherAlgInd_Rsp;
	CAPI2_NWMeasResultInd_Rsp_t   CAPI2_NWMeasResultInd_Rsp;
	CAPI2_IsPppLoopbackDialStr_Req_t   CAPI2_IsPppLoopbackDialStr_Req;
	CAPI2_IsPppLoopbackDialStr_Rsp_t   CAPI2_IsPppLoopbackDialStr_Rsp;
	CAPI2_SYS_GetRIPPROCVersion_Rsp_t   CAPI2_SYS_GetRIPPROCVersion_Rsp;
	CAPI2_SYSPARM_SetHSDPAPHYCategory_Req_t   CAPI2_SYSPARM_SetHSDPAPHYCategory_Req;
	CAPI2_SYSPARM_GetHSDPAPHYCategory_Rsp_t   CAPI2_SYSPARM_GetHSDPAPHYCategory_Rsp;
	CAPI2_SmsApi_ConvertSmsMSMsgType_Req_t   CAPI2_SmsApi_ConvertSmsMSMsgType_Req;
	CAPI2_SmsApi_ConvertSmsMSMsgType_Rsp_t   CAPI2_SmsApi_ConvertSmsMSMsgType_Rsp;
	CAPI2_NetUarfcnDlInd_Rsp_t   CAPI2_NetUarfcnDlInd_Rsp;
	CAPI2_MS_GetPrefNetStatus_Rsp_t   CAPI2_MS_GetPrefNetStatus_Rsp;
	SYS_TestSysMsgs1_Rsp_t   SYS_TestSysMsgs1_Rsp;
	SYS_TestSysMsgs2_Rsp_t   SYS_TestSysMsgs2_Rsp;
	SYS_TestSysMsgs3_Rsp_t   SYS_TestSysMsgs3_Rsp;
	SYS_TestSysMsgs4_Rsp_t   SYS_TestSysMsgs4_Rsp;
	SYS_TestSysMsgs5_Rsp_t   SYS_TestSysMsgs5_Rsp;
	SYS_TestSysMsgs6_Rsp_t   SYS_TestSysMsgs6_Rsp;
	CAPI2_SYSPARM_SetHSUPAPHYCategory_Req_t   CAPI2_SYSPARM_SetHSUPAPHYCategory_Req;
	CAPI2_SYSPARM_GetHSUPAPHYCategory_Rsp_t   CAPI2_SYSPARM_GetHSUPAPHYCategory_Rsp;
	CAPI2_InterTaskMsgToCP_Req_t   CAPI2_InterTaskMsgToCP_Req;
	CAPI2_InterTaskMsgToAP_Req_t   CAPI2_InterTaskMsgToAP_Req;
	CAPI2_CcApi_GetCurrentCallIndex_Rsp_t   CAPI2_CcApi_GetCurrentCallIndex_Rsp;
	CAPI2_CcApi_GetNextActiveCallIndex_Rsp_t   CAPI2_CcApi_GetNextActiveCallIndex_Rsp;
	CAPI2_CcApi_GetNextHeldCallIndex_Rsp_t   CAPI2_CcApi_GetNextHeldCallIndex_Rsp;
	CAPI2_CcApi_GetNextWaitCallIndex_Rsp_t   CAPI2_CcApi_GetNextWaitCallIndex_Rsp;
	CAPI2_CcApi_GetMPTYCallIndex_Rsp_t   CAPI2_CcApi_GetMPTYCallIndex_Rsp;
	CAPI2_CcApi_GetCallState_Req_t   CAPI2_CcApi_GetCallState_Req;
	CAPI2_CcApi_GetCallState_Rsp_t   CAPI2_CcApi_GetCallState_Rsp;
	CAPI2_CcApi_GetCallType_Req_t   CAPI2_CcApi_GetCallType_Req;
	CAPI2_CcApi_GetCallType_Rsp_t   CAPI2_CcApi_GetCallType_Rsp;
	CAPI2_CcApi_GetLastCallExitCause_Rsp_t   CAPI2_CcApi_GetLastCallExitCause_Rsp;
	CAPI2_CcApi_GetCallNumber_Req_t   CAPI2_CcApi_GetCallNumber_Req;
	CAPI2_CcApi_GetCallNumber_Rsp_t   CAPI2_CcApi_GetCallNumber_Rsp;
	CAPI2_CcApi_GetCallingInfo_Req_t   CAPI2_CcApi_GetCallingInfo_Req;
	CAPI2_CcApi_GetCallingInfo_Rsp_t   CAPI2_CcApi_GetCallingInfo_Rsp;
	CAPI2_CcApi_GetAllCallStates_Rsp_t   CAPI2_CcApi_GetAllCallStates_Rsp;
	CAPI2_CcApi_GetAllCallIndex_Rsp_t   CAPI2_CcApi_GetAllCallIndex_Rsp;
	CAPI2_CcApi_GetAllHeldCallIndex_Rsp_t   CAPI2_CcApi_GetAllHeldCallIndex_Rsp;
	CAPI2_CcApi_GetAllActiveCallIndex_Rsp_t   CAPI2_CcApi_GetAllActiveCallIndex_Rsp;
	CAPI2_CcApi_GetAllMPTYCallIndex_Rsp_t   CAPI2_CcApi_GetAllMPTYCallIndex_Rsp;
	CAPI2_CcApi_GetNumOfMPTYCalls_Rsp_t   CAPI2_CcApi_GetNumOfMPTYCalls_Rsp;
	CAPI2_CcApi_GetNumofActiveCalls_Rsp_t   CAPI2_CcApi_GetNumofActiveCalls_Rsp;
	CAPI2_CcApi_GetNumofHeldCalls_Rsp_t   CAPI2_CcApi_GetNumofHeldCalls_Rsp;
	CAPI2_CcApi_IsThereWaitingCall_Rsp_t   CAPI2_CcApi_IsThereWaitingCall_Rsp;
	CAPI2_CcApi_IsThereAlertingCall_Rsp_t   CAPI2_CcApi_IsThereAlertingCall_Rsp;
	CAPI2_CcApi_GetConnectedLineID_Req_t   CAPI2_CcApi_GetConnectedLineID_Req;
	CAPI2_CcApi_GetConnectedLineID_Rsp_t   CAPI2_CcApi_GetConnectedLineID_Rsp;
	CAPI2_CcApi_GetCallPresent_Req_t   CAPI2_CcApi_GetCallPresent_Req;
	CAPI2_CcApi_GetCallPresent_Rsp_t   CAPI2_CcApi_GetCallPresent_Rsp;
	CAPI2_CcApi_GetCallIndexInThisState_Req_t   CAPI2_CcApi_GetCallIndexInThisState_Req;
	CAPI2_CcApi_GetCallIndexInThisState_Rsp_t   CAPI2_CcApi_GetCallIndexInThisState_Rsp;
	CAPI2_CcApi_IsMultiPartyCall_Req_t   CAPI2_CcApi_IsMultiPartyCall_Req;
	CAPI2_CcApi_IsMultiPartyCall_Rsp_t   CAPI2_CcApi_IsMultiPartyCall_Rsp;
	CAPI2_CcApi_IsThereVoiceCall_Rsp_t   CAPI2_CcApi_IsThereVoiceCall_Rsp;
	CAPI2_CcApi_IsConnectedLineIDPresentAllowed_Req_t   CAPI2_CcApi_IsConnectedLineIDPresentAllowed_Req;
	CAPI2_CcApi_IsConnectedLineIDPresentAllowed_Rsp_t   CAPI2_CcApi_IsConnectedLineIDPresentAllowed_Rsp;
	CAPI2_CcApi_GetCurrentCallDurationInMilliSeconds_Req_t   CAPI2_CcApi_GetCurrentCallDurationInMilliSeconds_Req;
	CAPI2_CcApi_GetCurrentCallDurationInMilliSeconds_Rsp_t   CAPI2_CcApi_GetCurrentCallDurationInMilliSeconds_Rsp;
	CAPI2_CcApi_GetLastCallCCM_Rsp_t   CAPI2_CcApi_GetLastCallCCM_Rsp;
	CAPI2_CcApi_GetLastCallDuration_Rsp_t   CAPI2_CcApi_GetLastCallDuration_Rsp;
	CAPI2_CcApi_GetLastDataCallRxBytes_Rsp_t   CAPI2_CcApi_GetLastDataCallRxBytes_Rsp;
	CAPI2_CcApi_GetLastDataCallTxBytes_Rsp_t   CAPI2_CcApi_GetLastDataCallTxBytes_Rsp;
	CAPI2_CcApi_GetDataCallIndex_Rsp_t   CAPI2_CcApi_GetDataCallIndex_Rsp;
	CAPI2_CcApi_GetCallClientInfo_Req_t   CAPI2_CcApi_GetCallClientInfo_Req;
	CAPI2_CcApi_GetCallClientInfo_Rsp_t   CAPI2_CcApi_GetCallClientInfo_Rsp;
	CAPI2_CcApi_GetCallClientID_Req_t   CAPI2_CcApi_GetCallClientID_Req;
	CAPI2_CcApi_GetCallClientID_Rsp_t   CAPI2_CcApi_GetCallClientID_Rsp;
	CAPI2_CcApi_GetTypeAdd_Req_t   CAPI2_CcApi_GetTypeAdd_Req;
	CAPI2_CcApi_GetTypeAdd_Rsp_t   CAPI2_CcApi_GetTypeAdd_Rsp;
	CAPI2_CcApi_SetVoiceCallAutoReject_Req_t   CAPI2_CcApi_SetVoiceCallAutoReject_Req;
	CAPI2_CcApi_IsVoiceCallAutoReject_Rsp_t   CAPI2_CcApi_IsVoiceCallAutoReject_Rsp;
	CAPI2_CcApi_SetTTYCall_Req_t   CAPI2_CcApi_SetTTYCall_Req;
	CAPI2_CcApi_IsTTYEnable_Rsp_t   CAPI2_CcApi_IsTTYEnable_Rsp;
	CAPI2_CcApi_IsSimOriginedCall_Req_t   CAPI2_CcApi_IsSimOriginedCall_Req;
	CAPI2_CcApi_IsSimOriginedCall_Rsp_t   CAPI2_CcApi_IsSimOriginedCall_Rsp;
	CAPI2_CcApi_SetVideoCallParam_Req_t   CAPI2_CcApi_SetVideoCallParam_Req;
	CAPI2_CcApi_GetVideoCallParam_Rsp_t   CAPI2_CcApi_GetVideoCallParam_Rsp;
	CAPI2_CcApi_GetCCM_Req_t   CAPI2_CcApi_GetCCM_Req;
	CAPI2_CcApi_GetCCM_Rsp_t   CAPI2_CcApi_GetCCM_Rsp;
	CAPI2_CcApi_SendDtmfTone_Req_t   CAPI2_CcApi_SendDtmfTone_Req;
	CAPI2_CcApi_StopDtmfTone_Req_t   CAPI2_CcApi_StopDtmfTone_Req;
	CAPI2_CcApi_AbortDtmf_Req_t   CAPI2_CcApi_AbortDtmf_Req;
	CAPI2_CcApi_SetDtmfToneTimer_Req_t   CAPI2_CcApi_SetDtmfToneTimer_Req;
	CAPI2_CcApi_ResetDtmfToneTimer_Req_t   CAPI2_CcApi_ResetDtmfToneTimer_Req;
	CAPI2_CcApi_GetDtmfToneTimer_Req_t   CAPI2_CcApi_GetDtmfToneTimer_Req;
	CAPI2_CcApi_GetDtmfToneTimer_Rsp_t   CAPI2_CcApi_GetDtmfToneTimer_Rsp;
	CAPI2_CcApi_GetTiFromCallIndex_Req_t   CAPI2_CcApi_GetTiFromCallIndex_Req;
	CAPI2_CcApi_GetTiFromCallIndex_Rsp_t   CAPI2_CcApi_GetTiFromCallIndex_Rsp;
	CAPI2_CcApi_IsSupportedBC_Req_t   CAPI2_CcApi_IsSupportedBC_Req;
	CAPI2_CcApi_IsSupportedBC_Rsp_t   CAPI2_CcApi_IsSupportedBC_Rsp;
	CAPI2_CcApi_GetBearerCapability_Req_t   CAPI2_CcApi_GetBearerCapability_Req;
	CAPI2_CcApi_GetBearerCapability_Rsp_t   CAPI2_CcApi_GetBearerCapability_Rsp;
	CAPI2_SmsApi_SendSMSSrvCenterNumberUpdateReq_Req_t   CAPI2_SmsApi_SendSMSSrvCenterNumberUpdateReq_Req;
	CAPI2_SmsApi_GetSMSrvCenterNumber_Req_t   CAPI2_SmsApi_GetSMSrvCenterNumber_Req;
	CAPI2_SmsApi_GetSMSrvCenterNumber_Rsp_t   CAPI2_SmsApi_GetSMSrvCenterNumber_Rsp;
	CAPI2_SmsApi_GetSIMSMSCapacityExceededFlag_Rsp_t   CAPI2_SmsApi_GetSIMSMSCapacityExceededFlag_Rsp;
	CAPI2_SmsApi_GetSmsStoredState_Req_t   CAPI2_SmsApi_GetSmsStoredState_Req;
	CAPI2_SmsApi_GetSmsStoredState_Rsp_t   CAPI2_SmsApi_GetSmsStoredState_Rsp;
	CAPI2_SmsApi_WriteSMSPduReq_Req_t   CAPI2_SmsApi_WriteSMSPduReq_Req;
	CAPI2_SmsApi_WriteSMSPduReq_Rsp_t   CAPI2_SmsApi_WriteSMSPduReq_Rsp;
	CAPI2_SmsApi_WriteSMSReq_Req_t   CAPI2_SmsApi_WriteSMSReq_Req;
	CAPI2_SmsApi_SendSMSReq_Req_t   CAPI2_SmsApi_SendSMSReq_Req;
	CAPI2_SmsApi_SendSMSReq_Rsp_t   CAPI2_SmsApi_SendSMSReq_Rsp;
	CAPI2_SmsApi_SendSMSPduReq_Req_t   CAPI2_SmsApi_SendSMSPduReq_Req;
	CAPI2_SmsApi_SendStoredSmsReq_Req_t   CAPI2_SmsApi_SendStoredSmsReq_Req;
	CAPI2_SmsApi_WriteSMSPduToSIMReq_Req_t   CAPI2_SmsApi_WriteSMSPduToSIMReq_Req;
	CAPI2_SmsApi_GetLastTpMr_Rsp_t   CAPI2_SmsApi_GetLastTpMr_Rsp;
	CAPI2_SmsApi_GetSmsTxParams_Rsp_t   CAPI2_SmsApi_GetSmsTxParams_Rsp;
	CAPI2_SmsApi_GetTxParamInTextMode_Rsp_t   CAPI2_SmsApi_GetTxParamInTextMode_Rsp;
	CAPI2_SmsApi_SetSmsTxParamProcId_Req_t   CAPI2_SmsApi_SetSmsTxParamProcId_Req;
	CAPI2_SmsApi_SetSmsTxParamCodingType_Req_t   CAPI2_SmsApi_SetSmsTxParamCodingType_Req;
	CAPI2_SmsApi_SetSmsTxParamValidPeriod_Req_t   CAPI2_SmsApi_SetSmsTxParamValidPeriod_Req;
	CAPI2_SmsApi_SetSmsTxParamCompression_Req_t   CAPI2_SmsApi_SetSmsTxParamCompression_Req;
	CAPI2_SmsApi_SetSmsTxParamReplyPath_Req_t   CAPI2_SmsApi_SetSmsTxParamReplyPath_Req;
	CAPI2_SmsApi_SetSmsTxParamUserDataHdrInd_Req_t   CAPI2_SmsApi_SetSmsTxParamUserDataHdrInd_Req;
	CAPI2_SmsApi_SetSmsTxParamStatusRptReqFlag_Req_t   CAPI2_SmsApi_SetSmsTxParamStatusRptReqFlag_Req;
	CAPI2_SmsApi_SetSmsTxParamRejDupl_Req_t   CAPI2_SmsApi_SetSmsTxParamRejDupl_Req;
	CAPI2_SmsApi_DeleteSmsMsgByIndexReq_Req_t   CAPI2_SmsApi_DeleteSmsMsgByIndexReq_Req;
	CAPI2_SmsApi_DeleteSmsMsgByIndexReq_Rsp_t   CAPI2_SmsApi_DeleteSmsMsgByIndexReq_Rsp;
	CAPI2_SmsApi_ReadSmsMsgReq_Req_t   CAPI2_SmsApi_ReadSmsMsgReq_Req;
	CAPI2_SmsApi_ListSmsMsgReq_Req_t   CAPI2_SmsApi_ListSmsMsgReq_Req;
	CAPI2_SmsApi_SetNewMsgDisplayPref_Req_t   CAPI2_SmsApi_SetNewMsgDisplayPref_Req;
	CAPI2_SmsApi_GetNewMsgDisplayPref_Req_t   CAPI2_SmsApi_GetNewMsgDisplayPref_Req;
	CAPI2_SmsApi_GetNewMsgDisplayPref_Rsp_t   CAPI2_SmsApi_GetNewMsgDisplayPref_Rsp;
	CAPI2_SmsApi_SetSMSPrefStorage_Req_t   CAPI2_SmsApi_SetSMSPrefStorage_Req;
	CAPI2_SmsApi_GetSMSPrefStorage_Rsp_t   CAPI2_SmsApi_GetSMSPrefStorage_Rsp;
	CAPI2_SmsApi_GetSMSStorageStatus_Req_t   CAPI2_SmsApi_GetSMSStorageStatus_Req;
	CAPI2_SmsApi_GetSMSStorageStatus_Rsp_t   CAPI2_SmsApi_GetSMSStorageStatus_Rsp;
	CAPI2_SmsApi_SaveSmsServiceProfile_Req_t   CAPI2_SmsApi_SaveSmsServiceProfile_Req;
	CAPI2_SmsApi_RestoreSmsServiceProfile_Req_t   CAPI2_SmsApi_RestoreSmsServiceProfile_Req;
	CAPI2_SmsApi_SetCellBroadcastMsgTypeReq_Req_t   CAPI2_SmsApi_SetCellBroadcastMsgTypeReq_Req;
	CAPI2_SmsApi_SetCellBroadcastMsgTypeReq_Rsp_t   CAPI2_SmsApi_SetCellBroadcastMsgTypeReq_Rsp;
	CAPI2_SmsApi_CBAllowAllChnlReq_Req_t   CAPI2_SmsApi_CBAllowAllChnlReq_Req;
	CAPI2_SmsApi_AddCellBroadcastChnlReq_Req_t   CAPI2_SmsApi_AddCellBroadcastChnlReq_Req;
	CAPI2_SmsApi_RemoveCellBroadcastChnlReq_Req_t   CAPI2_SmsApi_RemoveCellBroadcastChnlReq_Req;
	CAPI2_SmsApi_GetCBMI_Rsp_t   CAPI2_SmsApi_GetCBMI_Rsp;
	CAPI2_SmsApi_GetCbLanguage_Rsp_t   CAPI2_SmsApi_GetCbLanguage_Rsp;
	CAPI2_SmsApi_AddCellBroadcastLangReq_Req_t   CAPI2_SmsApi_AddCellBroadcastLangReq_Req;
	CAPI2_SmsApi_RemoveCellBroadcastLangReq_Req_t   CAPI2_SmsApi_RemoveCellBroadcastLangReq_Req;
	CAPI2_SmsApi_SetCBIgnoreDuplFlag_Req_t   CAPI2_SmsApi_SetCBIgnoreDuplFlag_Req;
	CAPI2_SmsApi_GetCBIgnoreDuplFlag_Rsp_t   CAPI2_SmsApi_GetCBIgnoreDuplFlag_Rsp;
	CAPI2_SmsApi_SetVMIndOnOff_Req_t   CAPI2_SmsApi_SetVMIndOnOff_Req;
	CAPI2_SmsApi_IsVMIndEnabled_Rsp_t   CAPI2_SmsApi_IsVMIndEnabled_Rsp;
	CAPI2_SmsApi_GetVMWaitingStatus_Rsp_t   CAPI2_SmsApi_GetVMWaitingStatus_Rsp;
	CAPI2_SmsApi_GetNumOfVmscNumber_Rsp_t   CAPI2_SmsApi_GetNumOfVmscNumber_Rsp;
	CAPI2_SmsApi_GetVmscNumber_Req_t   CAPI2_SmsApi_GetVmscNumber_Req;
	CAPI2_SmsApi_GetVmscNumber_Rsp_t   CAPI2_SmsApi_GetVmscNumber_Rsp;
	CAPI2_SmsApi_UpdateVmscNumberReq_Req_t   CAPI2_SmsApi_UpdateVmscNumberReq_Req;
	CAPI2_SmsApi_GetSMSBearerPreference_Rsp_t   CAPI2_SmsApi_GetSMSBearerPreference_Rsp;
	CAPI2_SmsApi_SetSMSBearerPreference_Req_t   CAPI2_SmsApi_SetSMSBearerPreference_Req;
	CAPI2_SmsApi_SetSmsReadStatusChangeMode_Req_t   CAPI2_SmsApi_SetSmsReadStatusChangeMode_Req;
	CAPI2_SmsApi_GetSmsReadStatusChangeMode_Rsp_t   CAPI2_SmsApi_GetSmsReadStatusChangeMode_Rsp;
	CAPI2_SmsApi_ChangeSmsStatusReq_Req_t   CAPI2_SmsApi_ChangeSmsStatusReq_Req;
	CAPI2_SmsApi_SendMEStoredStatusInd_Req_t   CAPI2_SmsApi_SendMEStoredStatusInd_Req;
	CAPI2_SmsApi_SendMERetrieveSmsDataInd_Req_t   CAPI2_SmsApi_SendMERetrieveSmsDataInd_Req;
	CAPI2_SmsApi_SendMERemovedStatusInd_Req_t   CAPI2_SmsApi_SendMERemovedStatusInd_Req;
	CAPI2_SmsApi_SetSmsStoredState_Req_t   CAPI2_SmsApi_SetSmsStoredState_Req;
	CAPI2_SmsApi_IsCachedDataReady_Rsp_t   CAPI2_SmsApi_IsCachedDataReady_Rsp;
	CAPI2_SmsApi_GetEnhancedVMInfoIEI_Rsp_t   CAPI2_SmsApi_GetEnhancedVMInfoIEI_Rsp;
	CAPI2_SmsApi_SetAllNewMsgDisplayPref_Req_t   CAPI2_SmsApi_SetAllNewMsgDisplayPref_Req;
	CAPI2_SmsApi_SendAckToNetwork_Req_t   CAPI2_SmsApi_SendAckToNetwork_Req;
	CAPI2_SMS_SetPDAStorageOverFlowFlag_Req_t   CAPI2_SMS_SetPDAStorageOverFlowFlag_Req;
	CAPI2_ISimApi_SendAuthenAkaReq_Req_t   CAPI2_ISimApi_SendAuthenAkaReq_Req;
	CAPI2_ISimApi_SendAuthenAkaReq_Rsp_t   CAPI2_ISimApi_SendAuthenAkaReq_Rsp;
	CAPI2_ISimApi_IsIsimSupported_Rsp_t   CAPI2_ISimApi_IsIsimSupported_Rsp;
	CAPI2_ISimApi_IsIsimActivated_Rsp_t   CAPI2_ISimApi_IsIsimActivated_Rsp;
	CAPI2_ISimApi_ActivateIsimAppli_Rsp_t   CAPI2_ISimApi_ActivateIsimAppli_Rsp;
	CAPI2_ISimApi_SendAuthenHttpReq_Req_t   CAPI2_ISimApi_SendAuthenHttpReq_Req;
	CAPI2_ISimApi_SendAuthenHttpReq_Rsp_t   CAPI2_ISimApi_SendAuthenHttpReq_Rsp;
	CAPI2_ISimApi_SendAuthenGbaNafReq_Req_t   CAPI2_ISimApi_SendAuthenGbaNafReq_Req;
	CAPI2_ISimApi_SendAuthenGbaNafReq_Rsp_t   CAPI2_ISimApi_SendAuthenGbaNafReq_Rsp;
	CAPI2_ISimApi_SendAuthenGbaBootReq_Req_t   CAPI2_ISimApi_SendAuthenGbaBootReq_Req;
	CAPI2_ISimApi_SendAuthenGbaBootReq_Rsp_t   CAPI2_ISimApi_SendAuthenGbaBootReq_Rsp;
	CAPI2_PbkApi_GetAlpha_Req_t   CAPI2_PbkApi_GetAlpha_Req;
	CAPI2_PbkApi_GetAlpha_Rsp_t   CAPI2_PbkApi_GetAlpha_Rsp;
	CAPI2_PbkApi_IsEmergencyCallNumber_Req_t   CAPI2_PbkApi_IsEmergencyCallNumber_Req;
	CAPI2_PbkApi_IsEmergencyCallNumber_Rsp_t   CAPI2_PbkApi_IsEmergencyCallNumber_Rsp;
	CAPI2_PbkApi_IsPartialEmergencyCallNumber_Req_t   CAPI2_PbkApi_IsPartialEmergencyCallNumber_Req;
	CAPI2_PbkApi_IsPartialEmergencyCallNumber_Rsp_t   CAPI2_PbkApi_IsPartialEmergencyCallNumber_Rsp;
	CAPI2_PbkApi_SendInfoReq_Req_t   CAPI2_PbkApi_SendInfoReq_Req;
	CAPI2_PbkApi_SendInfoReq_Rsp_t   CAPI2_PbkApi_SendInfoReq_Rsp;
	CAPI2_PbkApi_SendFindAlphaMatchMultipleReq_Req_t   CAPI2_PbkApi_SendFindAlphaMatchMultipleReq_Req;
	CAPI2_PbkApi_SendFindAlphaMatchMultipleReq_Rsp_t   CAPI2_PbkApi_SendFindAlphaMatchMultipleReq_Rsp;
	CAPI2_PbkApi_SendFindAlphaMatchOneReq_Req_t   CAPI2_PbkApi_SendFindAlphaMatchOneReq_Req;
	CAPI2_PbkApi_IsReady_Rsp_t   CAPI2_PbkApi_IsReady_Rsp;
	CAPI2_PbkApi_SendReadEntryReq_Req_t   CAPI2_PbkApi_SendReadEntryReq_Req;
	CAPI2_PbkApi_SendWriteEntryReq_Req_t   CAPI2_PbkApi_SendWriteEntryReq_Req;
	CAPI2_PbkApi_SendWriteEntryReq_Rsp_t   CAPI2_PbkApi_SendWriteEntryReq_Rsp;
	CAPI2_PbkApi_SendUpdateEntryReq_Req_t   CAPI2_PbkApi_SendUpdateEntryReq_Req;
	CAPI2_PbkApi_SendIsNumDiallableReq_Req_t   CAPI2_PbkApi_SendIsNumDiallableReq_Req;
	CAPI2_PbkApi_SendIsNumDiallableReq_Rsp_t   CAPI2_PbkApi_SendIsNumDiallableReq_Rsp;
	CAPI2_PbkApi_IsNumDiallable_Req_t   CAPI2_PbkApi_IsNumDiallable_Req;
	CAPI2_PbkApi_IsNumDiallable_Rsp_t   CAPI2_PbkApi_IsNumDiallable_Rsp;
	CAPI2_PbkApi_IsNumBarred_Req_t   CAPI2_PbkApi_IsNumBarred_Req;
	CAPI2_PbkApi_IsNumBarred_Rsp_t   CAPI2_PbkApi_IsNumBarred_Rsp;
	CAPI2_PbkApi_IsUssdDiallable_Req_t   CAPI2_PbkApi_IsUssdDiallable_Req;
	CAPI2_PbkApi_IsUssdDiallable_Rsp_t   CAPI2_PbkApi_IsUssdDiallable_Rsp;
	CAPI2_PdpApi_SetPDPContext_Req_t   CAPI2_PdpApi_SetPDPContext_Req;
	CAPI2_PdpApi_SetSecPDPContext_Req_t   CAPI2_PdpApi_SetSecPDPContext_Req;
	CAPI2_PdpApi_GetGPRSQoS_Req_t   CAPI2_PdpApi_GetGPRSQoS_Req;
	CAPI2_PdpApi_GetGPRSQoS_Rsp_t   CAPI2_PdpApi_GetGPRSQoS_Rsp;
	CAPI2_PdpApi_SetGPRSQoS_Req_t   CAPI2_PdpApi_SetGPRSQoS_Req;
	CAPI2_PdpApi_GetGPRSMinQoS_Req_t   CAPI2_PdpApi_GetGPRSMinQoS_Req;
	CAPI2_PdpApi_GetGPRSMinQoS_Rsp_t   CAPI2_PdpApi_GetGPRSMinQoS_Rsp;
	CAPI2_PdpApi_SetGPRSMinQoS_Req_t   CAPI2_PdpApi_SetGPRSMinQoS_Req;
	CAPI2_NetRegApi_SendCombinedAttachReq_Req_t   CAPI2_NetRegApi_SendCombinedAttachReq_Req;
	CAPI2_NetRegApi_SendDetachReq_Req_t   CAPI2_NetRegApi_SendDetachReq_Req;
	CAPI2_MS_GetGPRSAttachStatus_Rsp_t   CAPI2_MS_GetGPRSAttachStatus_Rsp;
	CAPI2_PdpApi_IsSecondaryPdpDefined_Req_t   CAPI2_PdpApi_IsSecondaryPdpDefined_Req;
	CAPI2_PdpApi_IsSecondaryPdpDefined_Rsp_t   CAPI2_PdpApi_IsSecondaryPdpDefined_Rsp;
	CAPI2_PchExApi_SendPDPActivateReq_Req_t   CAPI2_PchExApi_SendPDPActivateReq_Req;
	CAPI2_PchExApi_SendPDPActivateReq_Rsp_t   CAPI2_PchExApi_SendPDPActivateReq_Rsp;
	CAPI2_PchExApi_SendPDPModifyReq_Req_t   CAPI2_PchExApi_SendPDPModifyReq_Req;
	CAPI2_PchExApi_SendPDPModifyReq_Rsp_t   CAPI2_PchExApi_SendPDPModifyReq_Rsp;
	CAPI2_PchExApi_SendPDPDeactivateReq_Req_t   CAPI2_PchExApi_SendPDPDeactivateReq_Req;
	CAPI2_PchExApi_SendPDPDeactivateReq_Rsp_t   CAPI2_PchExApi_SendPDPDeactivateReq_Rsp;
	CAPI2_PchExApi_SendPDPActivateSecReq_Req_t   CAPI2_PchExApi_SendPDPActivateSecReq_Req;
	CAPI2_PchExApi_SendPDPActivateSecReq_Rsp_t   CAPI2_PchExApi_SendPDPActivateSecReq_Rsp;
	CAPI2_PdpApi_GetGPRSActivateStatus_Rsp_t   CAPI2_PdpApi_GetGPRSActivateStatus_Rsp;
	CAPI2_NetRegApi_SetMSClass_Req_t   CAPI2_NetRegApi_SetMSClass_Req;
	CAPI2_PDP_GetMSClass_Rsp_t   CAPI2_PDP_GetMSClass_Rsp;
	CAPI2_PdpApi_GetUMTSTft_Req_t   CAPI2_PdpApi_GetUMTSTft_Req;
	CAPI2_PdpApi_GetUMTSTft_Rsp_t   CAPI2_PdpApi_GetUMTSTft_Rsp;
	CAPI2_PdpApi_SetUMTSTft_Req_t   CAPI2_PdpApi_SetUMTSTft_Req;
	CAPI2_PdpApi_DeleteUMTSTft_Req_t   CAPI2_PdpApi_DeleteUMTSTft_Req;
	CAPI2_PdpApi_DeactivateSNDCPConnection_Req_t   CAPI2_PdpApi_DeactivateSNDCPConnection_Req;
	CAPI2_PdpApi_GetR99UMTSMinQoS_Req_t   CAPI2_PdpApi_GetR99UMTSMinQoS_Req;
	CAPI2_PdpApi_GetR99UMTSMinQoS_Rsp_t   CAPI2_PdpApi_GetR99UMTSMinQoS_Rsp;
	CAPI2_PdpApi_GetR99UMTSQoS_Req_t   CAPI2_PdpApi_GetR99UMTSQoS_Req;
	CAPI2_PdpApi_GetR99UMTSQoS_Rsp_t   CAPI2_PdpApi_GetR99UMTSQoS_Rsp;
	CAPI2_PdpApi_GetUMTSMinQoS_Req_t   CAPI2_PdpApi_GetUMTSMinQoS_Req;
	CAPI2_PdpApi_GetUMTSMinQoS_Rsp_t   CAPI2_PdpApi_GetUMTSMinQoS_Rsp;
	CAPI2_PdpApi_GetUMTSQoS_Req_t   CAPI2_PdpApi_GetUMTSQoS_Req;
	CAPI2_PdpApi_GetUMTSQoS_Rsp_t   CAPI2_PdpApi_GetUMTSQoS_Rsp;
	CAPI2_PdpApi_GetNegQoS_Req_t   CAPI2_PdpApi_GetNegQoS_Req;
	CAPI2_PdpApi_GetNegQoS_Rsp_t   CAPI2_PdpApi_GetNegQoS_Rsp;
	CAPI2_PdpApi_SetR99UMTSMinQoS_Req_t   CAPI2_PdpApi_SetR99UMTSMinQoS_Req;
	CAPI2_PdpApi_SetR99UMTSQoS_Req_t   CAPI2_PdpApi_SetR99UMTSQoS_Req;
	CAPI2_PdpApi_SetUMTSMinQoS_Req_t   CAPI2_PdpApi_SetUMTSMinQoS_Req;
	CAPI2_PdpApi_SetUMTSQoS_Req_t   CAPI2_PdpApi_SetUMTSQoS_Req;
	CAPI2_PdpApi_GetNegotiatedParms_Req_t   CAPI2_PdpApi_GetNegotiatedParms_Req;
	CAPI2_PdpApi_GetNegotiatedParms_Rsp_t   CAPI2_PdpApi_GetNegotiatedParms_Rsp;
	CAPI2_MS_IsGprsCallActive_Req_t   CAPI2_MS_IsGprsCallActive_Req;
	CAPI2_MS_IsGprsCallActive_Rsp_t   CAPI2_MS_IsGprsCallActive_Rsp;
	CAPI2_MS_SetChanGprsCallActive_Req_t   CAPI2_MS_SetChanGprsCallActive_Req;
	CAPI2_MS_SetCidForGprsActiveChan_Req_t   CAPI2_MS_SetCidForGprsActiveChan_Req;
	CAPI2_PdpApi_GetPPPModemCid_Rsp_t   CAPI2_PdpApi_GetPPPModemCid_Rsp;
	CAPI2_MS_GetGprsActiveChanFromCid_Req_t   CAPI2_MS_GetGprsActiveChanFromCid_Req;
	CAPI2_MS_GetGprsActiveChanFromCid_Rsp_t   CAPI2_MS_GetGprsActiveChanFromCid_Rsp;
	CAPI2_MS_GetCidFromGprsActiveChan_Req_t   CAPI2_MS_GetCidFromGprsActiveChan_Req;
	CAPI2_MS_GetCidFromGprsActiveChan_Rsp_t   CAPI2_MS_GetCidFromGprsActiveChan_Rsp;
	CAPI2_PdpApi_GetPDPAddress_Req_t   CAPI2_PdpApi_GetPDPAddress_Req;
	CAPI2_PdpApi_GetPDPAddress_Rsp_t   CAPI2_PdpApi_GetPDPAddress_Rsp;
	CAPI2_PdpApi_SendTBFData_Req_t   CAPI2_PdpApi_SendTBFData_Req;
	CAPI2_PdpApi_TftAddFilter_Req_t   CAPI2_PdpApi_TftAddFilter_Req;
	CAPI2_PdpApi_SetPCHContextState_Req_t   CAPI2_PdpApi_SetPCHContextState_Req;
	CAPI2_PdpApi_SetDefaultPDPContext_Req_t   CAPI2_PdpApi_SetDefaultPDPContext_Req;
	CAPI2_PchExApi_GetDecodedProtConfig_Req_t   CAPI2_PchExApi_GetDecodedProtConfig_Req;
	CAPI2_PchExApi_GetDecodedProtConfig_Rsp_t   CAPI2_PchExApi_GetDecodedProtConfig_Rsp;
	CAPI2_PchExApi_BuildIpConfigOptions_Req_t   CAPI2_PchExApi_BuildIpConfigOptions_Req;
	CAPI2_PchExApi_BuildIpConfigOptions_Rsp_t   CAPI2_PchExApi_BuildIpConfigOptions_Rsp;
	CAPI2_PchExApi_BuildIpConfigOptions2_Req_t   CAPI2_PchExApi_BuildIpConfigOptions2_Req;
	CAPI2_PchExApi_BuildIpConfigOptions2_Rsp_t   CAPI2_PchExApi_BuildIpConfigOptions2_Rsp;
	CAPI2_PchExApi_BuildIpConfigOptionsWithChapAuthType_Req_t   CAPI2_PchExApi_BuildIpConfigOptionsWithChapAuthType_Req;
	CAPI2_PchExApi_BuildIpConfigOptionsWithChapAuthType_Rsp_t   CAPI2_PchExApi_BuildIpConfigOptionsWithChapAuthType_Rsp;
	CAPI2_PdpApi_GetDefaultQos_Rsp_t   CAPI2_PdpApi_GetDefaultQos_Rsp;
	CAPI2_PdpApi_IsPDPContextActive_Req_t   CAPI2_PdpApi_IsPDPContextActive_Req;
	CAPI2_PdpApi_IsPDPContextActive_Rsp_t   CAPI2_PdpApi_IsPDPContextActive_Rsp;
	CAPI2_PdpApi_ActivateSNDCPConnection_Req_t   CAPI2_PdpApi_ActivateSNDCPConnection_Req;
	CAPI2_PdpApi_ActivateSNDCPConnection_Rsp_t   CAPI2_PdpApi_ActivateSNDCPConnection_Rsp;
	CAPI2_PdpApi_GetPDPContext_Req_t   CAPI2_PdpApi_GetPDPContext_Req;
	CAPI2_PdpApi_GetPDPContext_Rsp_t   CAPI2_PdpApi_GetPDPContext_Rsp;
	CAPI2_PdpApi_GetDefinedPDPContextCidList_Rsp_t   CAPI2_PdpApi_GetDefinedPDPContextCidList_Rsp;
	CAPI2_SYS_GetBootLoaderVersion_Req_t   CAPI2_SYS_GetBootLoaderVersion_Req;
	CAPI2_SYS_GetBootLoaderVersion_Rsp_t   CAPI2_SYS_GetBootLoaderVersion_Rsp;
	CAPI2_SYS_GetDSFVersion_Req_t   CAPI2_SYS_GetDSFVersion_Req;
	CAPI2_SYS_GetDSFVersion_Rsp_t   CAPI2_SYS_GetDSFVersion_Rsp;
	CAPI2_USimApi_GetUstData_Rsp_t   CAPI2_USimApi_GetUstData_Rsp;
	CAPI2_PATCH_GetRevision_Rsp_t   CAPI2_PATCH_GetRevision_Rsp;
	CAPI2_SS_SendCallForwardReq_Req_t   CAPI2_SS_SendCallForwardReq_Req;
	CAPI2_SS_SendCallForwardReq_Rsp_t   CAPI2_SS_SendCallForwardReq_Rsp;
	CAPI2_SS_QueryCallForwardStatus_Req_t   CAPI2_SS_QueryCallForwardStatus_Req;
	CAPI2_SS_SendCallBarringReq_Req_t   CAPI2_SS_SendCallBarringReq_Req;
	CAPI2_SS_SendCallBarringReq_Rsp_t   CAPI2_SS_SendCallBarringReq_Rsp;
	CAPI2_SS_QueryCallBarringStatus_Req_t   CAPI2_SS_QueryCallBarringStatus_Req;
	CAPI2_SS_SendCallBarringPWDChangeReq_Req_t   CAPI2_SS_SendCallBarringPWDChangeReq_Req;
	CAPI2_SS_SendCallBarringPWDChangeReq_Rsp_t   CAPI2_SS_SendCallBarringPWDChangeReq_Rsp;
	CAPI2_SS_SendCallWaitingReq_Req_t   CAPI2_SS_SendCallWaitingReq_Req;
	CAPI2_SS_SendCallWaitingReq_Rsp_t   CAPI2_SS_SendCallWaitingReq_Rsp;
	CAPI2_SS_QueryCallWaitingStatus_Req_t   CAPI2_SS_QueryCallWaitingStatus_Req;
	CAPI2_SS_QueryCallingLineIDStatus_Rsp_t   CAPI2_SS_QueryCallingLineIDStatus_Rsp;
	CAPI2_SS_QueryConnectedLineIDStatus_Rsp_t   CAPI2_SS_QueryConnectedLineIDStatus_Rsp;
	CAPI2_SS_QueryCallingLineRestrictionStatus_Rsp_t   CAPI2_SS_QueryCallingLineRestrictionStatus_Rsp;
	CAPI2_SS_QueryConnectedLineRestrictionStatus_Rsp_t   CAPI2_SS_QueryConnectedLineRestrictionStatus_Rsp;
	CAPI2_SS_QueryCallingNAmePresentStatus_Rsp_t   CAPI2_SS_QueryCallingNAmePresentStatus_Rsp;
	CAPI2_SS_SetCallingLineIDStatus_Req_t   CAPI2_SS_SetCallingLineIDStatus_Req;
	CAPI2_SS_SetCallingLineIDStatus_Rsp_t   CAPI2_SS_SetCallingLineIDStatus_Rsp;
	CAPI2_SS_SetCallingLineRestrictionStatus_Req_t   CAPI2_SS_SetCallingLineRestrictionStatus_Req;
	CAPI2_SS_SetConnectedLineIDStatus_Req_t   CAPI2_SS_SetConnectedLineIDStatus_Req;
	CAPI2_SS_SetConnectedLineRestrictionStatus_Req_t   CAPI2_SS_SetConnectedLineRestrictionStatus_Req;
	CAPI2_SS_SendUSSDConnectReq_Req_t   CAPI2_SS_SendUSSDConnectReq_Req;
	CAPI2_SS_SendUSSDConnectReq_Rsp_t   CAPI2_SS_SendUSSDConnectReq_Rsp;
	CAPI2_SS_SendUSSDData_Req_t   CAPI2_SS_SendUSSDData_Req;
	CAPI2_SsApi_DialStrSrvReq_Req_t   CAPI2_SsApi_DialStrSrvReq_Req;
	CAPI2_SS_EndUSSDConnectReq_Req_t   CAPI2_SS_EndUSSDConnectReq_Req;
	CAPI2_SsApi_SsSrvReq_Req_t   CAPI2_SsApi_SsSrvReq_Req;
	CAPI2_SsApi_UssdSrvReq_Req_t   CAPI2_SsApi_UssdSrvReq_Req;
	CAPI2_SsApi_UssdDataReq_Req_t   CAPI2_SsApi_UssdDataReq_Req;
	CAPI2_SsApi_SsReleaseReq_Req_t   CAPI2_SsApi_SsReleaseReq_Req;
	CAPI2_SsApi_DataReq_Req_t   CAPI2_SsApi_DataReq_Req;
	CAPI2_SS_SsApiReqDispatcher_Req_t   CAPI2_SS_SsApiReqDispatcher_Req;
	CAPI2_SS_GetStr_Req_t   CAPI2_SS_GetStr_Req;
	CAPI2_SS_GetStr_Rsp_t   CAPI2_SS_GetStr_Rsp;
	CAPI2_SS_GetClientID_Rsp_t   CAPI2_SS_GetClientID_Rsp;
	CAPI2_SatkApi_GetCachedRootMenuPtr_Rsp_t   CAPI2_SatkApi_GetCachedRootMenuPtr_Rsp;
	CAPI2_SatkApi_SendLangSelectEvent_Req_t   CAPI2_SatkApi_SendLangSelectEvent_Req;
	CAPI2_SatkApi_SendBrowserTermEvent_Req_t   CAPI2_SatkApi_SendBrowserTermEvent_Req;
	CAPI2_SatkApi_CmdResp_Req_t   CAPI2_SatkApi_CmdResp_Req;
	CAPI2_SatkApi_CmdResp_Rsp_t   CAPI2_SatkApi_CmdResp_Rsp;
	CAPI2_SatkApi_DataServCmdResp_Req_t   CAPI2_SatkApi_DataServCmdResp_Req;
	CAPI2_SatkApi_DataServCmdResp_Rsp_t   CAPI2_SatkApi_DataServCmdResp_Rsp;
	CAPI2_SatkApi_SendDataServReq_Req_t   CAPI2_SatkApi_SendDataServReq_Req;
	CAPI2_SatkApi_SendDataServReq_Rsp_t   CAPI2_SatkApi_SendDataServReq_Rsp;
	CAPI2_SatkApi_SendTerminalRsp_Req_t   CAPI2_SatkApi_SendTerminalRsp_Req;
	CAPI2_SatkApi_SendTerminalRsp_Rsp_t   CAPI2_SatkApi_SendTerminalRsp_Rsp;
	CAPI2_SatkApi_SetTermProfile_Req_t   CAPI2_SatkApi_SetTermProfile_Req;
	CAPI2_SatkApi_SendEnvelopeCmdReq_Req_t   CAPI2_SatkApi_SendEnvelopeCmdReq_Req;
	CAPI2_SatkApi_SendEnvelopeCmdReq_Rsp_t   CAPI2_SatkApi_SendEnvelopeCmdReq_Rsp;
	CAPI2_SatkApi_SendTerminalRspReq_Req_t   CAPI2_SatkApi_SendTerminalRspReq_Req;
	CAPI2_SatkApi_SendTerminalRspReq_Rsp_t   CAPI2_SatkApi_SendTerminalRspReq_Rsp;
	CAPI2_StkApi_SendBrowsingStatusEvent_Req_t   CAPI2_StkApi_SendBrowsingStatusEvent_Req;
	CAPI2_SatkApi_SendCcSetupReq_Req_t   CAPI2_SatkApi_SendCcSetupReq_Req;
	CAPI2_SatkApi_Control_Setup_Rsp_Rsp_t   CAPI2_SatkApi_Control_Setup_Rsp_Rsp;
	CAPI2_SatkApi_Control_SS_Rsp_Rsp_t   CAPI2_SatkApi_Control_SS_Rsp_Rsp;
	CAPI2_SatkApi_Control_USSD_Rsp_Rsp_t   CAPI2_SatkApi_Control_USSD_Rsp_Rsp;
	CAPI2_SatkApi_SendCcSsReq_Req_t   CAPI2_SatkApi_SendCcSsReq_Req;
	CAPI2_SatkApi_SendCcUssdReq_Req_t   CAPI2_SatkApi_SendCcUssdReq_Req;
	CAPI2_SatkApi_SendCcSmsReq_Req_t   CAPI2_SatkApi_SendCcSmsReq_Req;
	CAPI2_SatkApi_Control_SMS_Rsp_Rsp_t   CAPI2_SatkApi_Control_SMS_Rsp_Rsp;
	CAPI2_LCS_CpMoLrReq_Req_t   CAPI2_LCS_CpMoLrReq_Req;
	CAPI2_LCS_CpMoLrReq_Rsp_t   CAPI2_LCS_CpMoLrReq_Rsp;
	CAPI2_LCS_CpMtLrVerificationRsp_Req_t   CAPI2_LCS_CpMtLrVerificationRsp_Req;
	CAPI2_LCS_CpMtLrRsp_Req_t   CAPI2_LCS_CpMtLrRsp_Req;
	CAPI2_LCS_CpLocUpdateRsp_Req_t   CAPI2_LCS_CpLocUpdateRsp_Req;
	CAPI2_LCS_DecodePosEstimate_Req_t   CAPI2_LCS_DecodePosEstimate_Req;
	CAPI2_LCS_DecodePosEstimate_Rsp_t   CAPI2_LCS_DecodePosEstimate_Rsp;
	CAPI2_LCS_EncodeAssistanceReq_Req_t   CAPI2_LCS_EncodeAssistanceReq_Req;
	CAPI2_LCS_EncodeAssistanceReq_Rsp_t   CAPI2_LCS_EncodeAssistanceReq_Rsp;
	CAPI2_CcApi_MakeVoiceCall_Req_t   CAPI2_CcApi_MakeVoiceCall_Req;
	CAPI2_CcApi_MakeDataCall_Req_t   CAPI2_CcApi_MakeDataCall_Req;
	CAPI2_CcApi_MakeFaxCall_Req_t   CAPI2_CcApi_MakeFaxCall_Req;
	CAPI2_CcApi_MakeVideoCall_Req_t   CAPI2_CcApi_MakeVideoCall_Req;
	CAPI2_CcApi_EndCall_Req_t   CAPI2_CcApi_EndCall_Req;
	CAPI2_CcApi_EndCallImmediate_Req_t   CAPI2_CcApi_EndCallImmediate_Req;
	CAPI2_CcApi_AcceptVoiceCall_Req_t   CAPI2_CcApi_AcceptVoiceCall_Req;
	CAPI2_CcApi_AcceptDataCall_Req_t   CAPI2_CcApi_AcceptDataCall_Req;
	CAPI2_CcApi_AcceptVideoCall_Req_t   CAPI2_CcApi_AcceptVideoCall_Req;
	CAPI2_CcApi_HoldCall_Req_t   CAPI2_CcApi_HoldCall_Req;
	CAPI2_CcApi_RetrieveCall_Req_t   CAPI2_CcApi_RetrieveCall_Req;
	CAPI2_CcApi_SwapCall_Req_t   CAPI2_CcApi_SwapCall_Req;
	CAPI2_CcApi_SplitCall_Req_t   CAPI2_CcApi_SplitCall_Req;
	CAPI2_CcApi_JoinCall_Req_t   CAPI2_CcApi_JoinCall_Req;
	CAPI2_CcApi_TransferCall_Req_t   CAPI2_CcApi_TransferCall_Req;
	CAPI2_smsModuleReady_Rsp_t   CAPI2_smsModuleReady_Rsp;
	CAPI2_SmsSimMsgDel_Rsp_t   CAPI2_SmsSimMsgDel_Rsp;
	CAPI2_SmsSimMsgOta_Rsp_t   CAPI2_SmsSimMsgOta_Rsp;
	CAPI2_SmsSimMsgPush_Rsp_t   CAPI2_SmsSimMsgPush_Rsp;
	CAPI2_SmsStoredSmsStatus_Rsp_t   CAPI2_SmsStoredSmsStatus_Rsp;
	CAPI2_SmsStoredSmsCb_Rsp_t   CAPI2_SmsStoredSmsCb_Rsp;
	CAPI2_SmsStoredSms_Rsp_t   CAPI2_SmsStoredSms_Rsp;
	CAPI2_SmsCbData_Rsp_t   CAPI2_SmsCbData_Rsp;
	CAPI2_HomezoneIndData_Rsp_t   CAPI2_HomezoneIndData_Rsp;
	CAPI2_SATK_EventDataDisp_Rsp_t   CAPI2_SATK_EventDataDisp_Rsp;
	CAPI2_SATK_EventDataGetInKey_Rsp_t   CAPI2_SATK_EventDataGetInKey_Rsp;
	CAPI2_SATK_EventDataGetInput_Rsp_t   CAPI2_SATK_EventDataGetInput_Rsp;
	CAPI2_SATK_EventDataPlayTone_Rsp_t   CAPI2_SATK_EventDataPlayTone_Rsp;
	CAPI2_SATK_EventDataActivate_Rsp_t   CAPI2_SATK_EventDataActivate_Rsp;
	CAPI2_SATK_EventDataSelItem_Rsp_t   CAPI2_SATK_EventDataSelItem_Rsp;
	CAPI2_SATK_EventDataSendSS_Rsp_t   CAPI2_SATK_EventDataSendSS_Rsp;
	CAPI2_SATK_EventDataSendUSSD_Rsp_t   CAPI2_SATK_EventDataSendUSSD_Rsp;
	CAPI2_SATK_EventDataSetupCall_Rsp_t   CAPI2_SATK_EventDataSetupCall_Rsp;
	CAPI2_SATK_EventDataSetupMenu_Rsp_t   CAPI2_SATK_EventDataSetupMenu_Rsp;
	CAPI2_SATK_EventDataRefresh_Rsp_t   CAPI2_SATK_EventDataRefresh_Rsp;
	CAPI2_SIM_FatalInd_Rsp_t   CAPI2_SIM_FatalInd_Rsp;
	CAPI2_SATK_CallSetupFail_Rsp_t   CAPI2_SATK_CallSetupFail_Rsp;
	CAPI2_SATK_CallControlDisplay_Rsp_t   CAPI2_SATK_CallControlDisplay_Rsp;
	CAPI2_SATK_EventDataShortMsg_Rsp_t   CAPI2_SATK_EventDataShortMsg_Rsp;
	CAPI2_SATK_EventDataSendDtmf_Rsp_t   CAPI2_SATK_EventDataSendDtmf_Rsp;
	CAPI2_SATK_EventData_Rsp_t   CAPI2_SATK_EventData_Rsp;
	CAPI2_SATK_EventDataLaunchBrowser_Rsp_t   CAPI2_SATK_EventDataLaunchBrowser_Rsp;
	CAPI2_SATK_EventDataServiceReq_Rsp_t   CAPI2_SATK_EventDataServiceReq_Rsp;
	CAPI2_StkLangNotificationLangNotify_Rsp_t   CAPI2_StkLangNotificationLangNotify_Rsp;
	CAPI2_SimMmiSetupInd_Rsp_t   CAPI2_SimMmiSetupInd_Rsp;
	CAPI2_SATK_EventDataRunATInd_Rsp_t   CAPI2_SATK_EventDataRunATInd_Rsp;
	CAPI2_StkRunAtReq_Rsp_t   CAPI2_StkRunAtReq_Rsp;
	CAPI2_StkRunIpAtReq_Rsp_t   CAPI2_StkRunIpAtReq_Rsp;
	CAPI2_StkMenuSelectionRes_Rsp_t   CAPI2_StkMenuSelectionRes_Rsp;
	CAPI2_VOICECALL_CONNECTED_IND_Rsp_t   CAPI2_VOICECALL_CONNECTED_IND_Rsp;
	CAPI2_VOICECALL_PRECONNECTED_IND_Rsp_t   CAPI2_VOICECALL_PRECONNECTED_IND_Rsp;
	CAPI2_SS_CALL_NOTIFICATION_Rsp_t   CAPI2_SS_CALL_NOTIFICATION_Rsp;
	CAPI2_MNCC_CLIENT_NOTIFY_SS_IND_Rsp_t   CAPI2_MNCC_CLIENT_NOTIFY_SS_IND_Rsp;
	CAPI2_SS_NOTIFY_CLOSED_USER_GROUP_Rsp_t   CAPI2_SS_NOTIFY_CLOSED_USER_GROUP_Rsp;
	CAPI2_SS_NOTIFY_EXTENDED_CALL_TRANSFER_Rsp_t   CAPI2_SS_NOTIFY_EXTENDED_CALL_TRANSFER_Rsp;
	CAPI2_SS_NOTIFY_CALLING_NAME_PRESENT_Rsp_t   CAPI2_SS_NOTIFY_CALLING_NAME_PRESENT_Rsp;
	CAPI2_CALL_STATUS_IND_Rsp_t   CAPI2_CALL_STATUS_IND_Rsp;
	CAPI2_VOICECALL_ACTION_RSP_Rsp_t   CAPI2_VOICECALL_ACTION_RSP_Rsp;
	CAPI2_VOICECALL_RELEASE_IND_Rsp_t   CAPI2_VOICECALL_RELEASE_IND_Rsp;
	CAPI2_VOICECALL_RELEASE_CNF_Rsp_t   CAPI2_VOICECALL_RELEASE_CNF_Rsp;
	CAPI2_INCOMING_CALL_IND_Rsp_t   CAPI2_INCOMING_CALL_IND_Rsp;
	CAPI2_VOICECALL_WAITING_IND_Rsp_t   CAPI2_VOICECALL_WAITING_IND_Rsp;
	CAPI2_CALL_AOCSTATUS_IND_Rsp_t   CAPI2_CALL_AOCSTATUS_IND_Rsp;
	CAPI2_CALL_CCM_IND_Rsp_t   CAPI2_CALL_CCM_IND_Rsp;
	CAPI2_CALL_CONNECTEDLINEID_IND_Rsp_t   CAPI2_CALL_CONNECTEDLINEID_IND_Rsp;
	CAPI2_DATACALL_STATUS_IND_Rsp_t   CAPI2_DATACALL_STATUS_IND_Rsp;
	CAPI2_DATACALL_RELEASE_IND_Rsp_t   CAPI2_DATACALL_RELEASE_IND_Rsp;
	CAPI2_DATACALL_ECDC_IND_Rsp_t   CAPI2_DATACALL_ECDC_IND_Rsp;
	CAPI2_DATACALL_CONNECTED_IND_Rsp_t   CAPI2_DATACALL_CONNECTED_IND_Rsp;
	CAPI2_API_CLIENT_CMD_IND_Rsp_t   CAPI2_API_CLIENT_CMD_IND_Rsp;
	CAPI2_DTMF_STATUS_IND_Rsp_t   CAPI2_DTMF_STATUS_IND_Rsp;
	CAPI2_USER_INFORMATION_Rsp_t   CAPI2_USER_INFORMATION_Rsp;
	CAPI2_CcCipherInd_Rsp_t   CAPI2_CcCipherInd_Rsp;
	CAPI2_GPRS_ACTIVATE_IND_Rsp_t   CAPI2_GPRS_ACTIVATE_IND_Rsp;
	CAPI2_GPRS_DEACTIVATE_IND_Rsp_t   CAPI2_GPRS_DEACTIVATE_IND_Rsp;
	CAPI2_PDP_DEACTIVATION_IND_Rsp_t   CAPI2_PDP_DEACTIVATION_IND_Rsp;
	CAPI2_GPRS_MODIFY_IND_Rsp_t   CAPI2_GPRS_MODIFY_IND_Rsp;
	CAPI2_GPRS_REACT_IND_Rsp_t   CAPI2_GPRS_REACT_IND_Rsp;
	CAPI2_PDP_ACTIVATION_NWI_IND_Rsp_t   CAPI2_PDP_ACTIVATION_NWI_IND_Rsp;
	CAPI2_DATA_SUSPEND_IND_Rsp_t   CAPI2_DATA_SUSPEND_IND_Rsp;
	CAPI2_MNSS_CLIENT_LCS_SRV_IND_Rsp_t   CAPI2_MNSS_CLIENT_LCS_SRV_IND_Rsp;
	CAPI2_MNSS_CLIENT_LCS_SRV_REL_Rsp_t   CAPI2_MNSS_CLIENT_LCS_SRV_REL_Rsp;
	CAPI2_MNSS_CLIENT_SS_SRV_RSP_Rsp_t   CAPI2_MNSS_CLIENT_SS_SRV_RSP_Rsp;
	CAPI2_MNSS_CLIENT_SS_SRV_REL_Rsp_t   CAPI2_MNSS_CLIENT_SS_SRV_REL_Rsp;
	CAPI2_MNSS_CLIENT_SS_SRV_IND_Rsp_t   CAPI2_MNSS_CLIENT_SS_SRV_IND_Rsp;
	CAPI2_MNSS_SATK_SS_SRV_RSP_Rsp_t   CAPI2_MNSS_SATK_SS_SRV_RSP_Rsp;
	CAPI2_SmsVoicemailInd_Rsp_t   CAPI2_SmsVoicemailInd_Rsp;
	CAPI2_SIM_SMS_DATA_RSP_Rsp_t   CAPI2_SIM_SMS_DATA_RSP_Rsp;
	CAPI2_CcApi_GetCNAPName_Req_t   CAPI2_CcApi_GetCNAPName_Req;
	CAPI2_CcApi_GetCNAPName_Rsp_t   CAPI2_CcApi_GetCNAPName_Rsp;
	CAPI2_SYSPARM_GetHSUPASupported_Rsp_t   CAPI2_SYSPARM_GetHSUPASupported_Rsp;
	CAPI2_SYSPARM_GetHSDPASupported_Rsp_t   CAPI2_SYSPARM_GetHSDPASupported_Rsp;
	CAPI2_MSRadioActivityInd_Rsp_t   CAPI2_MSRadioActivityInd_Rsp;
	CAPI2_CcApi_IsCurrentStateMpty_Req_t   CAPI2_CcApi_IsCurrentStateMpty_Req;
	CAPI2_CcApi_IsCurrentStateMpty_Rsp_t   CAPI2_CcApi_IsCurrentStateMpty_Rsp;
	CAPI2_PdpApi_GetPCHContextState_Req_t   CAPI2_PdpApi_GetPCHContextState_Req;
	CAPI2_PdpApi_GetPCHContextState_Rsp_t   CAPI2_PdpApi_GetPCHContextState_Rsp;
	CAPI2_PdpApi_GetPDPContextEx_Req_t   CAPI2_PdpApi_GetPDPContextEx_Req;
	CAPI2_PdpApi_GetPDPContextEx_Rsp_t   CAPI2_PdpApi_GetPDPContextEx_Rsp;
	CAPI2_SimApi_GetCurrLockedSimlockType_Rsp_t   CAPI2_SimApi_GetCurrLockedSimlockType_Rsp;
	CAPI2_PLMN_SELECT_CNF_Rsp_t   CAPI2_PLMN_SELECT_CNF_Rsp;
	CAPI2_SimApi_SubmitSelectFileSendApduReq_Req_t   CAPI2_SimApi_SubmitSelectFileSendApduReq_Req;
	CAPI2_SimApi_SubmitSelectFileSendApduReq_Rsp_t   CAPI2_SimApi_SubmitSelectFileSendApduReq_Rsp;
	CAPI2_SatkApi_SendTermProfileInd_Rsp_t   CAPI2_SatkApi_SendTermProfileInd_Rsp;
	CAPI2_SimApi_SubmitMulRecordEFileReq_Req_t   CAPI2_SimApi_SubmitMulRecordEFileReq_Req;
	CAPI2_SimApi_SubmitMulRecordEFileReq_Rsp_t   CAPI2_SimApi_SubmitMulRecordEFileReq_Rsp;
	CAPI2_SatkApi_SendUiccCatInd_Rsp_t   CAPI2_SatkApi_SendUiccCatInd_Rsp;
	CAPI2_SimApi_SendSelectApplicationReq_Req_t   CAPI2_SimApi_SendSelectApplicationReq_Req;
	CAPI2_SimApi_SendSelectApplicationReq_Rsp_t   CAPI2_SimApi_SendSelectApplicationReq_Rsp;
	CAPI2_SimApi_PerformSteeringOfRoaming_Req_t   CAPI2_SimApi_PerformSteeringOfRoaming_Req;
	CAPI2_SatkApi_SendProactiveCmdInd_Rsp_t   CAPI2_SatkApi_SendProactiveCmdInd_Rsp;
	CAPI2_SatkApi_SendProactiveCmdFetchingOnOffReq_Req_t   CAPI2_SatkApi_SendProactiveCmdFetchingOnOffReq_Req;
	CAPI2_SatkApi_SendProactiveCmdFetchingOnOffReq_Rsp_t   CAPI2_SatkApi_SendProactiveCmdFetchingOnOffReq_Rsp;
	CAPI2_SatkApi_SendExtProactiveCmdReq_Req_t   CAPI2_SatkApi_SendExtProactiveCmdReq_Req;
	CAPI2_SatkApi_SendTerminalProfileReq_Req_t   CAPI2_SatkApi_SendTerminalProfileReq_Req;
	CAPI2_SatkApi_SendPollingIntervalReq_Req_t   CAPI2_SatkApi_SendPollingIntervalReq_Req;
	CAPI2_SatkApi_SendPollingIntervalReq_Rsp_t   CAPI2_SatkApi_SendPollingIntervalReq_Rsp;
	CAPI2_SatkApi_SendExtProactiveCmdInd_Rsp_t   CAPI2_SatkApi_SendExtProactiveCmdInd_Rsp;
	CAPI2_PdpApi_SetPDPActivationCallControlFlag_Req_t   CAPI2_PdpApi_SetPDPActivationCallControlFlag_Req;
	CAPI2_PdpApi_GetPDPActivationCallControlFlag_Rsp_t   CAPI2_PdpApi_GetPDPActivationCallControlFlag_Rsp;
	CAPI2_PdpApi_SendPDPActivateReq_PDU_Req_t   CAPI2_PdpApi_SendPDPActivateReq_PDU_Req;
	CAPI2_PdpApi_SendPDPActivateReq_PDU_Rsp_t   CAPI2_PdpApi_SendPDPActivateReq_PDU_Rsp;
	CAPI2_PdpApi_RejectNWIPDPActivation_Req_t   CAPI2_PdpApi_RejectNWIPDPActivation_Req;
	CAPI2_PdpApi_SetPDPBearerCtrlMode_Req_t   CAPI2_PdpApi_SetPDPBearerCtrlMode_Req;
	CAPI2_PdpApi_GetPDPBearerCtrlMode_Rsp_t   CAPI2_PdpApi_GetPDPBearerCtrlMode_Rsp;
	CAPI2_PdpApi_RejectSecNWIPDPActivation_Req_t   CAPI2_PdpApi_RejectSecNWIPDPActivation_Req;
	CAPI2_PdpApi_SetPDPNWIControlFlag_Req_t   CAPI2_PdpApi_SetPDPNWIControlFlag_Req;
	CAPI2_PdpApi_GetPDPNWIControlFlag_Rsp_t   CAPI2_PdpApi_GetPDPNWIControlFlag_Rsp;
	CAPI2_PDP_ACTIVATION_SEC_NWI_IND_Rsp_t   CAPI2_PDP_ACTIVATION_SEC_NWI_IND_Rsp;
	CAPI2_PdpApi_CheckUMTSTft_Req_t   CAPI2_PdpApi_CheckUMTSTft_Req;
	CAPI2_PdpApi_IsAnyPDPContextActive_Rsp_t   CAPI2_PdpApi_IsAnyPDPContextActive_Rsp;
	CAPI2_PdpApi_IsAnyPDPContextActivePending_Rsp_t   CAPI2_PdpApi_IsAnyPDPContextActivePending_Rsp;
	CAPI2_CcApi_SetElement_Req_t   CAPI2_CcApi_SetElement_Req;
	CAPI2_CcApi_GetElement_Req_t   CAPI2_CcApi_GetElement_Req;
	CAPI2_CcApi_GetElement_Rsp_t   CAPI2_CcApi_GetElement_Rsp;
	CAPI2_WL_PsSetFilterList_Req_t   CAPI2_WL_PsSetFilterList_Req;
	CAPI2_PdpApi_GetProtConfigOptions_Req_t   CAPI2_PdpApi_GetProtConfigOptions_Req;
	CAPI2_PdpApi_GetProtConfigOptions_Rsp_t   CAPI2_PdpApi_GetProtConfigOptions_Rsp;
	CAPI2_PdpApi_SetProtConfigOptions_Req_t   CAPI2_PdpApi_SetProtConfigOptions_Req;
	CAPI2_LcsApi_RrlpSendDataToNetwork_Req_t   CAPI2_LcsApi_RrlpSendDataToNetwork_Req;
	CAPI2_LcsApi_RrcSendUlDcch_Req_t   CAPI2_LcsApi_RrcSendUlDcch_Req;
	CAPI2_LcsApi_RrcMeasCtrlFailure_Req_t   CAPI2_LcsApi_RrcMeasCtrlFailure_Req;
	CAPI2_LcsApi_RrcStatus_Req_t   CAPI2_LcsApi_RrcStatus_Req;
	CAPI2_SimApi_PowerOnOffSim_Req_t   CAPI2_SimApi_PowerOnOffSim_Req;
	CAPI2_SimApi_PowerOnOffSim_Rsp_t   CAPI2_SimApi_PowerOnOffSim_Rsp;
	CAPI2_PhoneCtrlApi_SetPagingStatus_Req_t   CAPI2_PhoneCtrlApi_SetPagingStatus_Req;
	CAPI2_LcsApi_GetGpsCapabilities_Rsp_t   CAPI2_LcsApi_GetGpsCapabilities_Rsp;
	CAPI2_LcsApi_SetGpsCapabilities_Req_t   CAPI2_LcsApi_SetGpsCapabilities_Req;
	CAPI2_CcApi_AbortDtmfTone_Req_t   CAPI2_CcApi_AbortDtmfTone_Req;
	CAPI2_NetRegApi_SetSupportedRATandBandEx_Req_t   CAPI2_NetRegApi_SetSupportedRATandBandEx_Req;
	CAPI2_SmsReportInd_Rsp_t   CAPI2_SmsReportInd_Rsp;
#endif //DEVELOPMENT_CAPI2_XDR_UNION_DECLARE

