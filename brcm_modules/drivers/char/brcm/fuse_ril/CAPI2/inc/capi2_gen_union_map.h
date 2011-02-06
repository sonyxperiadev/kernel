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


#ifndef RPC_INCLUDED
#define _D(a) (int)a 
#else
#define _D(a) a
#endif
#ifdef DEFINE_XDR_UNION_MAPPING

	{ _D(MSG_MS_GSM_REGISTERED_REQ),_T("MSG_MS_GSM_REGISTERED_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_GSM_REGISTERED_RSP),_T("MSG_MS_GSM_REGISTERED_RSP"), (xdrproc_t)xdr_CAPI2_MS_IsGSMRegistered_Rsp_t, REQRSP_MSG_EXT( sizeof( Boolean ), NULL, NULL, 0 )},
	{ _D(MSG_MS_GPRS_REGISTERED_REQ),_T("MSG_MS_GPRS_REGISTERED_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_GPRS_REGISTERED_RSP),_T("MSG_MS_GPRS_REGISTERED_RSP"), (xdrproc_t)xdr_CAPI2_MS_IsGPRSRegistered_Rsp_t, REQRSP_MSG_EXT( sizeof( Boolean ), NULL, NULL, 0 )},
	{ _D(MSG_MS_GSM_CAUSE_REQ),_T("MSG_MS_GSM_CAUSE_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_GSM_CAUSE_RSP),_T("MSG_MS_GSM_CAUSE_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetGSMRegCause_Rsp_t, REQRSP_MSG_EXT( sizeof( NetworkCause_t ), NULL, NULL, 0 )},
	{ _D(MSG_MS_GPRS_CAUSE_REQ),_T("MSG_MS_GPRS_CAUSE_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_GPRS_CAUSE_RSP),_T("MSG_MS_GPRS_CAUSE_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetGPRSRegCause_Rsp_t, REQRSP_MSG_EXT( sizeof( NetworkCause_t ), NULL, NULL, 0 )},
	{ _D(MSG_MS_REGISTERED_LAC_REQ),_T("MSG_MS_REGISTERED_LAC_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_REGISTERED_LAC_RSP),_T("MSG_MS_REGISTERED_LAC_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetRegisteredLAC_Rsp_t, REQRSP_MSG_EXT( sizeof( UInt16 ), NULL, NULL, 0 )},
	{ _D(MSG_MS_GET_PLMN_MCC_REQ),_T("MSG_MS_GET_PLMN_MCC_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_GET_PLMN_MCC_RSP),_T("MSG_MS_GET_PLMN_MCC_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetPlmnMCC_Rsp_t, REQRSP_MSG_EXT( sizeof( UInt16 ), NULL, NULL, 0 )},
	{ _D(MSG_MS_GET_PLMN_MNC_REQ),_T("MSG_MS_GET_PLMN_MNC_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_GET_PLMN_MNC_RSP),_T("MSG_MS_GET_PLMN_MNC_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetPlmnMNC_Rsp_t, REQRSP_MSG_EXT( sizeof( UInt8 ), NULL, NULL, 0 )},
	{ _D(MSG_SYS_POWERDOWN_REQ),_T("MSG_SYS_POWERDOWN_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYS_POWERDOWN_RSP),_T("MSG_SYS_POWERDOWN_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYS_POWERUP_NORF_REQ),_T("MSG_SYS_POWERUP_NORF_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYS_POWERUP_NORF_RSP),_T("MSG_SYS_POWERUP_NORF_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYS_POWERUP_REQ),_T("MSG_SYS_POWERUP_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYS_POWERUP_RSP),_T("MSG_SYS_POWERUP_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYS_POWERON_CAUSE_REQ),_T("MSG_SYS_POWERON_CAUSE_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYS_POWERON_CAUSE_RSP),_T("MSG_SYS_POWERON_CAUSE_RSP"), (xdrproc_t)xdr_CAPI2_SYS_GetMSPowerOnCause_Rsp_t, REQRSP_MSG_EXT( sizeof( PowerOnCause_t ), NULL, NULL, 0 )},
	{ _D(MSG_MS_GPRS_ALLOWED_REQ),_T("MSG_MS_GPRS_ALLOWED_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_GPRS_ALLOWED_RSP),_T("MSG_MS_GPRS_ALLOWED_RSP"), (xdrproc_t)xdr_CAPI2_MS_IsGprsAllowed_Rsp_t, REQRSP_MSG_EXT( sizeof( Boolean ), NULL, NULL, 0 )},
	{ _D(MSG_MS_GET_CURRENT_RAT_REQ),_T("MSG_MS_GET_CURRENT_RAT_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_GET_CURRENT_RAT_RSP),_T("MSG_MS_GET_CURRENT_RAT_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetCurrentRAT_Rsp_t, REQRSP_MSG_EXT( sizeof( UInt8 ), NULL, NULL, 0 )},
	{ _D(MSG_MS_GET_CURRENT_BAND_REQ),_T("MSG_MS_GET_CURRENT_BAND_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_GET_CURRENT_BAND_RSP),_T("MSG_MS_GET_CURRENT_BAND_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetCurrentBand_Rsp_t, REQRSP_MSG_EXT( sizeof( UInt8 ), NULL, NULL, 0 )},
	{ _D(MSG_MS_START_BAND_REQ),_T("MSG_MS_START_BAND_REQ"), (xdrproc_t) xdr_CAPI2_MS_SetStartBand_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_START_BAND_RSP),_T("MSG_MS_START_BAND_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_UPDATE_SMSCAPEXC_FLAG_REQ),_T("MSG_SIM_UPDATE_SMSCAPEXC_FLAG_REQ"), (xdrproc_t) xdr_CAPI2_SIM_UpdateSMSCapExceededFlag_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_UPDATE_SMSCAPEXC_FLAG_RSP),_T("MSG_SIM_UPDATE_SMSCAPEXC_FLAG_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_SELECT_BAND_REQ),_T("MSG_MS_SELECT_BAND_REQ"), (xdrproc_t) xdr_CAPI2_SYS_SelectBand_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_SELECT_BAND_RSP),_T("MSG_MS_SELECT_BAND_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_SET_RAT_BAND_REQ),_T("MSG_MS_SET_RAT_BAND_REQ"), (xdrproc_t) xdr_CAPI2_MS_SetSupportedRATandBand_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_SET_RAT_BAND_RSP),_T("MSG_MS_SET_RAT_BAND_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_GET_MCC_COUNTRY_REQ),_T("MSG_MS_GET_MCC_COUNTRY_REQ"), (xdrproc_t) xdr_CAPI2_PLMN_GetCountryByMcc_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_GET_MCC_COUNTRY_RSP),_T("MSG_MS_GET_MCC_COUNTRY_RSP"), (xdrproc_t)xdr_CAPI2_PLMN_GetCountryByMcc_Rsp_t, REQRSP_MSG_EXT( sizeof( uchar_ptr_t ), NULL, NULL, 0 )},
	{ _D(MSG_MS_PLMN_INFO_BY_CODE_REQ),_T("MSG_MS_PLMN_INFO_BY_CODE_REQ"), (xdrproc_t) xdr_CAPI2_MS_GetPLMNEntryByIndex_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_PLMN_INFO_BY_CODE_RSP),_T("MSG_MS_PLMN_INFO_BY_CODE_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetPLMNEntryByIndex_Rsp_t, REQRSP_MSG_EXT( sizeof( MsPlmnInfo_t ), NULL, NULL, 0 )},
	{ _D(MSG_MS_PLMN_LIST_SIZE_REQ),_T("MSG_MS_PLMN_LIST_SIZE_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_PLMN_LIST_SIZE_RSP),_T("MSG_MS_PLMN_LIST_SIZE_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetPLMNListSize_Rsp_t, REQRSP_MSG_EXT( sizeof( UInt16 ), NULL, NULL, 0 )},
	{ _D(MSG_MS_PLMN_INFO_BY_INDEX_REQ),_T("MSG_MS_PLMN_INFO_BY_INDEX_REQ"), (xdrproc_t) xdr_CAPI2_MS_GetPLMNByCode_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_PLMN_INFO_BY_INDEX_RSP),_T("MSG_MS_PLMN_INFO_BY_INDEX_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetPLMNByCode_Rsp_t, REQRSP_MSG_EXT( sizeof( MsPlmnInfo_t ), NULL, NULL, 0 )},
	{ _D(MSG_MS_PLMN_SELECT_REQ),_T("MSG_MS_PLMN_SELECT_REQ"), (xdrproc_t) xdr_CAPI2_MS_PlmnSelect_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_PLMN_SELECT_RSP),_T("MSG_MS_PLMN_SELECT_RSP"), (xdrproc_t)xdr_CAPI2_MS_PlmnSelect_Rsp_t, REQRSP_MSG_EXT( sizeof( UInt16 ), NULL, NULL, 0 )},
	{ _D(MSG_MS_PLMN_ABORT_REQ),_T("MSG_MS_PLMN_ABORT_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_PLMN_ABORT_RSP),_T("MSG_MS_PLMN_ABORT_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_GET_PLMN_MODE_REQ),_T("MSG_MS_GET_PLMN_MODE_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_GET_PLMN_MODE_RSP),_T("MSG_MS_GET_PLMN_MODE_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetPlmnMode_Rsp_t, REQRSP_MSG_EXT( sizeof( PlmnSelectMode_t ), NULL, NULL, 0 )},
	{ _D(MSG_MS_SET_PLMN_MODE_REQ),_T("MSG_MS_SET_PLMN_MODE_REQ"), (xdrproc_t) xdr_CAPI2_MS_SetPlmnMode_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_SET_PLMN_MODE_RSP),_T("MSG_MS_SET_PLMN_MODE_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_GET_PLMN_FORMAT_REQ),_T("MSG_MS_GET_PLMN_FORMAT_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_GET_PLMN_FORMAT_RSP),_T("MSG_MS_GET_PLMN_FORMAT_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetPlmnFormat_Rsp_t, REQRSP_MSG_EXT( sizeof( PlmnSelectFormat_t ), NULL, NULL, 0 )},
	{ _D(MSG_MS_SET_PLMN_FORMAT_REQ),_T("MSG_MS_SET_PLMN_FORMAT_REQ"), (xdrproc_t) xdr_CAPI2_MS_SetPlmnFormat_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_SET_PLMN_FORMAT_RSP),_T("MSG_MS_SET_PLMN_FORMAT_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_MATCH_PLMN_REQ),_T("MSG_MS_MATCH_PLMN_REQ"), (xdrproc_t) xdr_CAPI2_MS_IsMatchedPLMN_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_MATCH_PLMN_RSP),_T("MSG_MS_MATCH_PLMN_RSP"), (xdrproc_t)xdr_CAPI2_MS_IsMatchedPLMN_Rsp_t, REQRSP_MSG_EXT( sizeof( Boolean ), NULL, NULL, 0 )},
	{ _D(MSG_MS_SEARCH_PLMN_REQ),_T("MSG_MS_SEARCH_PLMN_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_SEARCH_PLMN_RSP),_T("MSG_MS_SEARCH_PLMN_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_ABORT_PLMN_REQ),_T("MSG_MS_ABORT_PLMN_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_ABORT_PLMN_RSP),_T("MSG_MS_ABORT_PLMN_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_AUTO_SEARCH_REQ),_T("MSG_MS_AUTO_SEARCH_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_AUTO_SEARCH_RSP),_T("MSG_MS_AUTO_SEARCH_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_PLMN_NAME_REQ),_T("MSG_MS_PLMN_NAME_REQ"), (xdrproc_t) xdr_CAPI2_MS_GetPLMNNameByCode_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_PLMN_NAME_RSP),_T("MSG_MS_PLMN_NAME_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetPLMNNameByCode_Rsp_t, REQRSP_MSG_EXT( sizeof( MsPlmnName_t ), NULL, NULL, 0 )},
	{ _D(MSG_SYS_QUERY_RESET_CAUSE_REQ),_T("MSG_SYS_QUERY_RESET_CAUSE_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYS_QUERY_RESET_CAUSE_RSP),_T("MSG_SYS_QUERY_RESET_CAUSE_RSP"), (xdrproc_t)xdr_CAPI2_SYS_IsResetCausedByAssert_Rsp_t, REQRSP_MSG_EXT( sizeof( Boolean ), NULL, NULL, 0 )},
	{ _D(MSG_SYS_GET_SYSTEM_STATE_REQ),_T("MSG_SYS_GET_SYSTEM_STATE_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYS_GET_SYSTEM_STATE_RSP),_T("MSG_SYS_GET_SYSTEM_STATE_RSP"), (xdrproc_t)xdr_CAPI2_SYS_GetSystemState_Rsp_t, REQRSP_MSG_EXT( sizeof( SystemState_t ), NULL, NULL, 0 )},
	{ _D(MSG_SYS_SET_SYSTEM_STATE_REQ),_T("MSG_SYS_SET_SYSTEM_STATE_REQ"), (xdrproc_t) xdr_CAPI2_SYS_SetSystemState_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYS_SET_SYSTEM_STATE_RSP),_T("MSG_SYS_SET_SYSTEM_STATE_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYS_GET_RX_LEVEL_REQ),_T("MSG_SYS_GET_RX_LEVEL_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYS_GET_RX_LEVEL_RSP),_T("MSG_SYS_GET_RX_LEVEL_RSP"), (xdrproc_t)xdr_CAPI2_SYS_GetRxSignalInfo_Rsp_t, REQRSP_MSG_EXT( sizeof( MsRxLevelData_t ), NULL, NULL, 0 )},
	{ _D(MSG_SYS_GET_GSMREG_STATUS_REQ),_T("MSG_SYS_GET_GSMREG_STATUS_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYS_GET_GSMREG_STATUS_RSP),_T("MSG_SYS_GET_GSMREG_STATUS_RSP"), (xdrproc_t)xdr_CAPI2_SYS_GetGSMRegistrationStatus_Rsp_t, REQRSP_MSG_EXT( sizeof( RegisterStatus_t ), NULL, NULL, 0 )},
	{ _D(MSG_SYS_GET_GPRSREG_STATUS_REQ),_T("MSG_SYS_GET_GPRSREG_STATUS_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYS_GET_GPRSREG_STATUS_RSP),_T("MSG_SYS_GET_GPRSREG_STATUS_RSP"), (xdrproc_t)xdr_CAPI2_SYS_GetGPRSRegistrationStatus_Rsp_t, REQRSP_MSG_EXT( sizeof( RegisterStatus_t ), NULL, NULL, 0 )},
	{ _D(MSG_SYS_GET_REG_STATUS_REQ),_T("MSG_SYS_GET_REG_STATUS_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYS_GET_REG_STATUS_RSP),_T("MSG_SYS_GET_REG_STATUS_RSP"), (xdrproc_t)xdr_CAPI2_SYS_IsRegisteredGSMOrGPRS_Rsp_t, REQRSP_MSG_EXT( sizeof( Boolean ), NULL, NULL, 0 )},
	{ _D(MSG_SYS_GET_GSMREG_CAUSE_REQ),_T("MSG_SYS_GET_GSMREG_CAUSE_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYS_GET_GSMREG_CAUSE_RSP),_T("MSG_SYS_GET_GSMREG_CAUSE_RSP"), (xdrproc_t)xdr_CAPI2_SYS_GetGSMRegistrationCause_Rsp_t, REQRSP_MSG_EXT( sizeof( PCHRejectCause_t ), NULL, NULL, 0 )},
	{ _D(MSG_SYS_IS_PLMN_FORBIDDEN_REQ),_T("MSG_SYS_IS_PLMN_FORBIDDEN_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYS_IS_PLMN_FORBIDDEN_RSP),_T("MSG_SYS_IS_PLMN_FORBIDDEN_RSP"), (xdrproc_t)xdr_CAPI2_MS_IsPlmnForbidden_Rsp_t, REQRSP_MSG_EXT( sizeof( Boolean ), NULL, NULL, 0 )},
	{ _D(MSG_HOME_PLMN_REG_REQ),_T("MSG_HOME_PLMN_REG_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_HOME_PLMN_REG_RSP),_T("MSG_HOME_PLMN_REG_RSP"), (xdrproc_t)xdr_CAPI2_MS_IsRegisteredHomePLMN_Rsp_t, REQRSP_MSG_EXT( sizeof( Boolean ), NULL, NULL, 0 )},
	{ _D(MSG_SET_POWER_DOWN_TIMER_REQ),_T("MSG_SET_POWER_DOWN_TIMER_REQ"), (xdrproc_t) xdr_CAPI2_MS_SetPowerDownTimer_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SET_POWER_DOWN_TIMER_RSP),_T("MSG_SET_POWER_DOWN_TIMER_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_LOCK_ON_REQ),_T("MSG_SIM_LOCK_ON_REQ"), (xdrproc_t) xdr_CAPI2_SIMLockIsLockOn_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_LOCK_ON_RSP),_T("MSG_SIM_LOCK_ON_RSP"), (xdrproc_t)xdr_CAPI2_SIMLockIsLockOn_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_LOCK_ON_RESULT_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_LOCK_STATUS_REQ),_T("MSG_SIM_LOCK_STATUS_REQ"), (xdrproc_t) xdr_CAPI2_SIMLockCheckAllLocks_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_LOCK_STATUS_RSP),_T("MSG_SIM_LOCK_STATUS_RSP"), (xdrproc_t)xdr_CAPI2_SIMLockCheckAllLocks_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_BOOLEAN_DATA_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_LOCK_UNLOCK_REQ),_T("MSG_SIM_LOCK_UNLOCK_REQ"), (xdrproc_t) xdr_CAPI2_SIMLockUnlockSIM_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_LOCK_UNLOCK_RSP),_T("MSG_SIM_LOCK_UNLOCK_RSP"), (xdrproc_t)xdr_CAPI2_SIMLockUnlockSIM_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_LOCK_RESULT_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_LOCK_SET_LOCK_REQ),_T("MSG_SIM_LOCK_SET_LOCK_REQ"), (xdrproc_t) xdr_CAPI2_SIMLockSetLock_t,REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_LOCK_SET_LOCK_RSP),_T("MSG_SIM_LOCK_SET_LOCK_RSP"), (xdrproc_t)xdr_CAPI2_SIMLockSetLock_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_LOCK_RESULT_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_LOCK_CLOSED_LOCK_REQ),_T("MSG_SIM_LOCK_CLOSED_LOCK_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_LOCK_CLOSED_LOCK_RSP),_T("MSG_SIM_LOCK_CLOSED_LOCK_RSP"), (xdrproc_t)xdr_CAPI2_SIMLockGetCurrentClosedLock_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_LOCK_TYPE_RESULT_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_LOCK_CHANGE_PWD_REQ),_T("MSG_SIM_LOCK_CHANGE_PWD_REQ"), (xdrproc_t) xdr_CAPI2_SIMLockChangePasswordPHSIM_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_LOCK_CHANGE_PWD_RSP),_T("MSG_SIM_LOCK_CHANGE_PWD_RSP"), (xdrproc_t)xdr_CAPI2_SIMLockChangePasswordPHSIM_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_LOCK_RESULT_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_LOCK_CHECK_PWD_PHSIM_REQ),_T("MSG_SIM_LOCK_CHECK_PWD_PHSIM_REQ"), (xdrproc_t) xdr_CAPI2_SIMLockCheckPasswordPHSIM_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_LOCK_CHECK_PWD_PHSIM_RSP),_T("MSG_SIM_LOCK_CHECK_PWD_PHSIM_RSP"), (xdrproc_t)xdr_CAPI2_SIMLockCheckPasswordPHSIM_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_BOOLEAN_DATA_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_LOCK_LOCK_SIG_REQ),_T("MSG_SIM_LOCK_LOCK_SIG_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_LOCK_LOCK_SIG_RSP),_T("MSG_SIM_LOCK_LOCK_SIG_RSP"), (xdrproc_t)xdr_CAPI2_SIMLockGetSignature_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_LOCK_SIG_DATA_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_LOCK_GET_IMEI_REQ),_T("MSG_SIM_LOCK_GET_IMEI_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_LOCK_GET_IMEI_RSP),_T("MSG_SIM_LOCK_GET_IMEI_RSP"), (xdrproc_t)xdr_CAPI2_SIMLockGetImeiSecboot_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_LOCK_IMEI_DATA_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_PARAM_REC_NUM_REQ),_T("MSG_SIM_PARAM_REC_NUM_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_PARAM_REC_NUM_RSP),_T("MSG_SIM_PARAM_REC_NUM_RSP"), (xdrproc_t)xdr_CAPI2_SIM_GetSmsParamRecNum_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_INTEGER_DATA_t ), NULL, NULL, 0 )},
	{ _D(MSG_MS_CONVERT_PLMN_STRING_REQ),_T("MSG_MS_CONVERT_PLMN_STRING_REQ"), (xdrproc_t) xdr_CAPI2_MS_ConvertPLMNNameStr_t,REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_CONVERT_PLMN_STRING_RSP),_T("MSG_MS_CONVERT_PLMN_STRING_RSP"), (xdrproc_t)xdr_CAPI2_MS_ConvertPLMNNameStr_Rsp_t, REQRSP_MSG_EXT( sizeof( uchar_ptr_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_GET_SMSMEMEXC_FLAG_REQ),_T("MSG_SIM_GET_SMSMEMEXC_FLAG_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_GET_SMSMEMEXC_FLAG_RSP),_T("MSG_SIM_GET_SMSMEMEXC_FLAG_RSP"), (xdrproc_t)xdr_CAPI2_SIM_GetSmsMemExceededFlag_Rsp_t, REQRSP_MSG_EXT( sizeof( Boolean ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_PIN_REQ_REQ),_T("MSG_SIM_PIN_REQ_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_PIN_REQ_RSP),_T("MSG_SIM_PIN_REQ_RSP"), (xdrproc_t)xdr_CAPI2_SIM_IsPINRequired_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_BOOLEAN_DATA_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_CARD_PHASE_REQ),_T("MSG_SIM_CARD_PHASE_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_CARD_PHASE_RSP),_T("MSG_SIM_CARD_PHASE_RSP"), (xdrproc_t)xdr_CAPI2_SIM_GetCardPhase_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_PHASE_RESULT_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_TYPE_REQ),_T("MSG_SIM_TYPE_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_TYPE_RSP),_T("MSG_SIM_TYPE_RSP"), (xdrproc_t)xdr_CAPI2_SIM_GetSIMType_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_TYPE_RESULT_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_PRESENT_REQ),_T("MSG_SIM_PRESENT_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_PRESENT_RSP),_T("MSG_SIM_PRESENT_RSP"), (xdrproc_t)xdr_CAPI2_SIM_GetPresentStatus_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_PRESENT_RESULT_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_PIN_OPERATION_REQ),_T("MSG_SIM_PIN_OPERATION_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_PIN_OPERATION_RSP),_T("MSG_SIM_PIN_OPERATION_RSP"), (xdrproc_t)xdr_CAPI2_SIM_IsOperationRestricted_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_BOOLEAN_DATA_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_PIN_BLOCK_REQ),_T("MSG_SIM_PIN_BLOCK_REQ"), (xdrproc_t) xdr_CAPI2_SIM_IsPINBlocked_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_PIN_BLOCK_RSP),_T("MSG_SIM_PIN_BLOCK_RSP"), (xdrproc_t)xdr_CAPI2_SIM_IsPINBlocked_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_BOOLEAN_DATA_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_PUK_BLOCK_REQ),_T("MSG_SIM_PUK_BLOCK_REQ"), (xdrproc_t) xdr_CAPI2_SIM_IsPUKBlocked_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_PUK_BLOCK_RSP),_T("MSG_SIM_PUK_BLOCK_RSP"), (xdrproc_t)xdr_CAPI2_SIM_IsPUKBlocked_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_BOOLEAN_DATA_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_IS_INVALID_REQ),_T("MSG_SIM_IS_INVALID_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_IS_INVALID_RSP),_T("MSG_SIM_IS_INVALID_RSP"), (xdrproc_t)xdr_CAPI2_SIM_IsInvalidSIM_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_BOOLEAN_DATA_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_DETECT_REQ),_T("MSG_SIM_DETECT_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_DETECT_RSP),_T("MSG_SIM_DETECT_RSP"), (xdrproc_t)xdr_CAPI2_SIM_DetectSim_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_BOOLEAN_DATA_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_GET_RUIM_SUPP_FLAG_REQ),_T("MSG_SIM_GET_RUIM_SUPP_FLAG_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_GET_RUIM_SUPP_FLAG_RSP),_T("MSG_SIM_GET_RUIM_SUPP_FLAG_RSP"), (xdrproc_t)xdr_CAPI2_SIM_GetRuimSuppFlag_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_BOOLEAN_DATA_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_VERIFY_CHV_REQ),_T("MSG_SIM_VERIFY_CHV_REQ"), (xdrproc_t) xdr_CAPI2_SIM_SendVerifyChvReq_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_VERIFY_CHV_RSP),_T("MSG_SIM_VERIFY_CHV_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SendVerifyChvReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_ACCESS_RESULT_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_CHANGE_CHV_REQ),_T("MSG_SIM_CHANGE_CHV_REQ"), (xdrproc_t) xdr_CAPI2_SIM_SendChangeChvReq_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_CHANGE_CHV_RSP),_T("MSG_SIM_CHANGE_CHV_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SendChangeChvReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_ACCESS_RESULT_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_ENABLE_CHV_REQ),_T("MSG_SIM_ENABLE_CHV_REQ"), (xdrproc_t) xdr_CAPI2_SIM_SendSetChv1OnOffReq_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_ENABLE_CHV_RSP),_T("MSG_SIM_ENABLE_CHV_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SendSetChv1OnOffReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_ACCESS_RESULT_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_UNBLOCK_CHV_REQ),_T("MSG_SIM_UNBLOCK_CHV_REQ"), (xdrproc_t) xdr_CAPI2_SIM_SendUnblockChvReq_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_UNBLOCK_CHV_RSP),_T("MSG_SIM_UNBLOCK_CHV_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SendUnblockChvReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_ACCESS_RESULT_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_SET_FDN_REQ),_T("MSG_SIM_SET_FDN_REQ"), (xdrproc_t) xdr_CAPI2_SIM_SendSetOperStateReq_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_SET_FDN_RSP),_T("MSG_SIM_SET_FDN_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SendSetOperStateReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_ACCESS_RESULT_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_IS_PBK_ALLOWED_REQ),_T("MSG_SIM_IS_PBK_ALLOWED_REQ"), (xdrproc_t) xdr_CAPI2_SIM_IsPbkAccessAllowed_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_IS_PBK_ALLOWED_RSP),_T("MSG_SIM_IS_PBK_ALLOWED_RSP"), (xdrproc_t)xdr_CAPI2_SIM_IsPbkAccessAllowed_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_BOOLEAN_DATA_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_PBK_INFO_REQ),_T("MSG_SIM_PBK_INFO_REQ"), (xdrproc_t) xdr_CAPI2_SIM_SendPbkInfoReq_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_PBK_INFO_RSP),_T("MSG_SIM_PBK_INFO_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SendPbkInfoReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_PBK_INFO_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_MAX_ACM_REQ),_T("MSG_SIM_MAX_ACM_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_MAX_ACM_RSP),_T("MSG_SIM_MAX_ACM_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SendReadAcmMaxReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_MAX_ACM_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_ACM_MAX_UPDATE_REQ),_T("MSG_SIM_ACM_MAX_UPDATE_REQ"), (xdrproc_t) xdr_CAPI2_SIM_SendWriteAcmMaxReq_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_ACM_MAX_UPDATE_RSP),_T("MSG_SIM_ACM_MAX_UPDATE_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SendWriteAcmMaxReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_ACCESS_RESULT_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_ACM_VALUE_REQ),_T("MSG_SIM_ACM_VALUE_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_ACM_VALUE_RSP),_T("MSG_SIM_ACM_VALUE_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SendReadAcmReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_ACM_VALUE_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_ACM_UPDATE_REQ),_T("MSG_SIM_ACM_UPDATE_REQ"), (xdrproc_t) xdr_CAPI2_SIM_SendWriteAcmReq_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_ACM_UPDATE_RSP),_T("MSG_SIM_ACM_UPDATE_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SendWriteAcmReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_ACCESS_RESULT_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_ACM_INCREASE_REQ),_T("MSG_SIM_ACM_INCREASE_REQ"), (xdrproc_t) xdr_CAPI2_SIM_SendIncreaseAcmReq_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_ACM_INCREASE_RSP),_T("MSG_SIM_ACM_INCREASE_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SendIncreaseAcmReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_ACCESS_RESULT_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_SVC_PROV_NAME_REQ),_T("MSG_SIM_SVC_PROV_NAME_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_SVC_PROV_NAME_RSP),_T("MSG_SIM_SVC_PROV_NAME_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SendReadSvcProvNameReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_SVC_PROV_NAME_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_PUCT_DATA_REQ),_T("MSG_SIM_PUCT_DATA_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_PUCT_DATA_RSP),_T("MSG_SIM_PUCT_DATA_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SendReadPuctReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_PUCT_DATA_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_SERVICE_STATUS_REQ),_T("MSG_SIM_SERVICE_STATUS_REQ"), (xdrproc_t) xdr_CAPI2_SIM_GetServiceStatus_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_SERVICE_STATUS_RSP),_T("MSG_SIM_SERVICE_STATUS_RSP"), (xdrproc_t)xdr_CAPI2_SIM_GetServiceStatus_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_SERVICE_STATUS_RESULT_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_GENERIC_ACCESS_END_REQ),_T("MSG_SIM_GENERIC_ACCESS_END_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_GENERIC_ACCESS_END_RSP),_T("MSG_SIM_GENERIC_ACCESS_END_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_PIN_STATUS_REQ),_T("MSG_SIM_PIN_STATUS_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_PIN_STATUS_RSP),_T("MSG_SIM_PIN_STATUS_RSP"), (xdrproc_t)xdr_CAPI2_SIM_GetPinStatus_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_PIN_STATUS_RESULT_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_PIN_OK_STATUS_REQ),_T("MSG_SIM_PIN_OK_STATUS_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_PIN_OK_STATUS_RSP),_T("MSG_SIM_PIN_OK_STATUS_RSP"), (xdrproc_t)xdr_CAPI2_SIM_IsPinOK_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_BOOLEAN_DATA_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_IMSI_REQ),_T("MSG_SIM_IMSI_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_IMSI_RSP),_T("MSG_SIM_IMSI_RSP"), (xdrproc_t)xdr_CAPI2_SIM_GetIMSI_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_IMSI_RESULT_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_GID_DIGIT_REQ),_T("MSG_SIM_GID_DIGIT_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_GID_DIGIT_RSP),_T("MSG_SIM_GID_DIGIT_RSP"), (xdrproc_t)xdr_CAPI2_SIM_GetGID1_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_GID_RESULT_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_GID_DIGIT2_REQ),_T("MSG_SIM_GID_DIGIT2_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_GID_DIGIT2_RSP),_T("MSG_SIM_GID_DIGIT2_RSP"), (xdrproc_t)xdr_CAPI2_SIM_GetGID2_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_GID_RESULT_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_HOME_PLMN_REQ),_T("MSG_SIM_HOME_PLMN_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_HOME_PLMN_RSP),_T("MSG_SIM_HOME_PLMN_RSP"), (xdrproc_t)xdr_CAPI2_SIM_GetHomePlmn_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_HOME_PLMN_RESULT_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_PIN_LOCK_TYPE_REQ),_T("MSG_SIM_PIN_LOCK_TYPE_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_PIN_LOCK_TYPE_RSP),_T("MSG_SIM_PIN_LOCK_TYPE_RSP"), (xdrproc_t)xdr_CAPI2_SIM_GetCurrLockedSimlockType_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_PIN_STATUS_RESULT_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_APDU_FILEID_REQ),_T("MSG_SIM_APDU_FILEID_REQ"), (xdrproc_t) xdr_CAPI2_simmi_GetMasterFileId_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_APDU_FILEID_RSP),_T("MSG_SIM_APDU_FILEID_RSP"), (xdrproc_t)xdr_CAPI2_simmi_GetMasterFileId_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_APDU_FILEID_RESULT_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_OPEN_SOCKET_REQ),_T("MSG_SIM_OPEN_SOCKET_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_OPEN_SOCKET_RSP),_T("MSG_SIM_OPEN_SOCKET_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SendOpenSocketReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_OPEN_SOCKET_RES_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_SELECT_APPLI_REQ),_T("MSG_SIM_SELECT_APPLI_REQ"), (xdrproc_t) xdr_CAPI2_SimSelectAppli_t,REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_SELECT_APPLI_RSP),_T("MSG_SIM_SELECT_APPLI_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SendSelectAppiReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_SELECT_APPLI_RES_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_DEACTIVATE_APPLI_REQ),_T("MSG_SIM_DEACTIVATE_APPLI_REQ"), (xdrproc_t) xdr_CAPI2_SimDeactivateAppli_t,REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_DEACTIVATE_APPLI_RSP),_T("MSG_SIM_DEACTIVATE_APPLI_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SendDeactivateAppiReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_DEACTIVATE_APPLI_RES_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_CLOSE_SOCKET_REQ),_T("MSG_SIM_CLOSE_SOCKET_REQ"), (xdrproc_t) xdr_CAPI2_SimCloseSocket_t,REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_CLOSE_SOCKET_RSP),_T("MSG_SIM_CLOSE_SOCKET_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SendCloseSocketReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_CLOSE_SOCKET_RES_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_ATR_DATA_REQ),_T("MSG_SIM_ATR_DATA_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_ATR_DATA_RSP),_T("MSG_SIM_ATR_DATA_RSP"), (xdrproc_t)xdr_CAPI2_SIM_GetAtrData_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_ATR_DATA_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_DFILE_INFO_REQ),_T("MSG_SIM_DFILE_INFO_REQ"), (xdrproc_t) xdr_CAPI2_SimFileInfo_t,REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_DFILE_INFO_RSP),_T("MSG_SIM_DFILE_INFO_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SubmitDFileInfoReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_DFILE_INFO_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_EFILE_INFO_REQ),_T("MSG_SIM_EFILE_INFO_REQ"), (xdrproc_t) xdr_CAPI2_SimFileInfo_t,REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_EFILE_INFO_RSP),_T("MSG_SIM_EFILE_INFO_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SubmitEFileInfoReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_EFILE_INFO_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_WHOLE_EFILE_DATA_REQ),_T("MSG_SIM_WHOLE_EFILE_DATA_REQ"), (xdrproc_t) xdr_CAPI2_SimFileInfo_t,REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_WHOLE_EFILE_DATA_RSP),_T("MSG_SIM_WHOLE_EFILE_DATA_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SubmitWholeBinaryEFileReadReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_EFILE_DATA_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_EFILE_DATA_REQ),_T("MSG_SIM_EFILE_DATA_REQ"), (xdrproc_t) xdr_CAPI2_SimFileInfo_t,REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_EFILE_DATA_RSP),_T("MSG_SIM_EFILE_DATA_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SubmitBinaryEFileReadReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_EFILE_DATA_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_RECORD_EFILE_DATA_REQ),_T("MSG_SIM_RECORD_EFILE_DATA_REQ"), (xdrproc_t) xdr_CAPI2_SimFileInfo_t,REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_RECORD_EFILE_DATA_RSP),_T("MSG_SIM_RECORD_EFILE_DATA_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SubmitRecordEFileReadReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_EFILE_DATA_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_EFILE_UPDATE_REQ),_T("MSG_SIM_EFILE_UPDATE_REQ"), (xdrproc_t) xdr_CAPI2_SimFileInfo_t,REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_EFILE_UPDATE_RSP),_T("MSG_SIM_EFILE_UPDATE_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SubmitBinaryEFileUpdateReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_EFILE_UPDATE_RESULT_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_LINEAR_EFILE_UPDATE_REQ),_T("MSG_SIM_LINEAR_EFILE_UPDATE_REQ"), (xdrproc_t) xdr_CAPI2_SimFileInfo_t,REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_LINEAR_EFILE_UPDATE_RSP),_T("MSG_SIM_LINEAR_EFILE_UPDATE_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SubmitLinearEFileUpdateReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_EFILE_UPDATE_RESULT_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_SEEK_RECORD_REQ),_T("MSG_SIM_SEEK_RECORD_REQ"), (xdrproc_t) xdr_CAPI2_SimFileInfo_t,REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_SEEK_RECORD_RSP),_T("MSG_SIM_SEEK_RECORD_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SubmitSeekRecordReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_SEEK_RECORD_DATA_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_CYCLIC_EFILE_UPDATE_REQ),_T("MSG_SIM_CYCLIC_EFILE_UPDATE_REQ"), (xdrproc_t) xdr_CAPI2_SimFileInfo_t,REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_CYCLIC_EFILE_UPDATE_RSP),_T("MSG_SIM_CYCLIC_EFILE_UPDATE_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SubmitCyclicEFileUpdateReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_EFILE_UPDATE_RESULT_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_PIN_ATTEMPT_REQ),_T("MSG_SIM_PIN_ATTEMPT_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_PIN_ATTEMPT_RSP),_T("MSG_SIM_PIN_ATTEMPT_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SendRemainingPinAttemptReq_Rsp_t, REQRSP_MSG_EXT( sizeof( PIN_ATTEMPT_RESULT_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_CACHE_DATA_READY_REQ),_T("MSG_SIM_CACHE_DATA_READY_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_CACHE_DATA_READY_RSP),_T("MSG_SIM_CACHE_DATA_READY_RSP"), (xdrproc_t)xdr_CAPI2_SIM_IsCachedDataReady_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_BOOLEAN_DATA_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_SERVICE_CODE_STATUS_REQ),_T("MSG_SIM_SERVICE_CODE_STATUS_REQ"), (xdrproc_t) xdr_CAPI2_SIM_GetServiceCodeStatus_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_SERVICE_CODE_STATUS_RSP),_T("MSG_SIM_SERVICE_CODE_STATUS_RSP"), (xdrproc_t)xdr_CAPI2_SIM_GetServiceCodeStatus_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_SERVICE_FLAG_STATUS_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_CHECK_CPHS_REQ),_T("MSG_SIM_CHECK_CPHS_REQ"), (xdrproc_t) xdr_CAPI2_SIM_CheckCphsService_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_CHECK_CPHS_RSP),_T("MSG_SIM_CHECK_CPHS_RSP"), (xdrproc_t)xdr_CAPI2_SIM_CheckCphsService_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_BOOLEAN_DATA_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_CPHS_PHASE_REQ),_T("MSG_SIM_CPHS_PHASE_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_CPHS_PHASE_RSP),_T("MSG_SIM_CPHS_PHASE_RSP"), (xdrproc_t)xdr_CAPI2_SIM_GetCphsPhase_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_INTEGER_DATA_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_SMS_SCA_REQ),_T("MSG_SIM_SMS_SCA_REQ"), (xdrproc_t) xdr_CAPI2_SIM_GetSmsSca_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_SMS_SCA_RSP),_T("MSG_SIM_SMS_SCA_RSP"), (xdrproc_t)xdr_CAPI2_SIM_GetSmsSca_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_SCA_DATA_RESULT_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_ICCID_PARAM_REQ),_T("MSG_SIM_ICCID_PARAM_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_ICCID_PARAM_RSP),_T("MSG_SIM_ICCID_PARAM_RSP"), (xdrproc_t)xdr_CAPI2_SIM_GetIccid_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_ICCID_STATUS_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_ALS_STATUS_REQ),_T("MSG_SIM_ALS_STATUS_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_ALS_STATUS_RSP),_T("MSG_SIM_ALS_STATUS_RSP"), (xdrproc_t)xdr_CAPI2_SIM_IsALSEnabled_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_BOOLEAN_DATA_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_ALS_DEFAULT_LINE_REQ),_T("MSG_SIM_ALS_DEFAULT_LINE_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_ALS_DEFAULT_LINE_RSP),_T("MSG_SIM_ALS_DEFAULT_LINE_RSP"), (xdrproc_t)xdr_CAPI2_SIM_GetAlsDefaultLine_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_INTEGER_DATA_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_SET_ALS_DEFAULT_REQ),_T("MSG_SIM_SET_ALS_DEFAULT_REQ"), (xdrproc_t) xdr_CAPI2_SIM_SetAlsDefaultLine_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_SET_ALS_DEFAULT_RSP),_T("MSG_SIM_SET_ALS_DEFAULT_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_CALLFWD_COND_FLAG_REQ),_T("MSG_SIM_CALLFWD_COND_FLAG_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_CALLFWD_COND_FLAG_RSP),_T("MSG_SIM_CALLFWD_COND_FLAG_RSP"), (xdrproc_t)xdr_CAPI2_SIM_GetCallForwardUnconditionalFlag_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_CALL_FORWARD_UNCONDITIONAL_STATUS_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_APP_TYPE_REQ),_T("MSG_SIM_APP_TYPE_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_APP_TYPE_RSP),_T("MSG_SIM_APP_TYPE_RSP"), (xdrproc_t)xdr_CAPI2_SIM_GetApplicationType_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_APPL_TYPE_RESULT_t ), NULL, NULL, 0 )},
	{ _D(MSG_USIM_UST_DATA_REQ),_T("MSG_USIM_UST_DATA_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_USIM_UST_DATA_RSP),_T("MSG_USIM_UST_DATA_RSP"), (xdrproc_t)xdr_CAPI2_USIM_GetUst_Rsp_t, REQRSP_MSG_EXT( sizeof( USIM_UST_DATA_RSP_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_PLMN_WRITE_REQ),_T("MSG_SIM_PLMN_WRITE_REQ"), (xdrproc_t) xdr_CAPI2_SIM_SendUpdatePrefListReq_t,REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_PLMN_WRITE_RSP),_T("MSG_SIM_PLMN_WRITE_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SendUpdatePrefListReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_ACCESS_RESULT_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_PUCT_UPDATE_REQ),_T("MSG_SIM_PUCT_UPDATE_REQ"), (xdrproc_t) xdr_CAPI2_SIM_SendWritePuctReq_t,REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_PUCT_UPDATE_RSP),_T("MSG_SIM_PUCT_UPDATE_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SendWritePuctReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_ACCESS_RESULT_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_GENERIC_ACCESS_REQ),_T("MSG_SIM_GENERIC_ACCESS_REQ"), (xdrproc_t) xdr_CAPI2_SIM_SendGenericAccessReq_t,REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_GENERIC_ACCESS_RSP),_T("MSG_SIM_GENERIC_ACCESS_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SendGenericAccessReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_GENERIC_ACCESS_DATA_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_RESTRICTED_ACCESS_REQ),_T("MSG_SIM_RESTRICTED_ACCESS_REQ"), (xdrproc_t) xdr_CAPI2_SIM_SendRestrictedAccessReq_t,REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_RESTRICTED_ACCESS_RSP),_T("MSG_SIM_RESTRICTED_ACCESS_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SubmitRestrictedAccessReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_RESTRICTED_ACCESS_DATA_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_DETECTION_IND),_T("MSG_SIM_DETECTION_IND"), (xdrproc_t)xdr_CAPI2_SIM_SendDetectionInd_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_DETECTION_t ), NULL, NULL, 0 )},
	{ _D(MSG_REG_GSM_IND),_T("MSG_REG_GSM_IND"), (xdrproc_t)xdr_CAPI2_MS_GsmRegStatusInd_Rsp_t, REQRSP_MSG_EXT( sizeof( MSRegInfo_t ), NULL, NULL, 0 )},
	{ _D(MSG_REG_GPRS_IND),_T("MSG_REG_GPRS_IND"), (xdrproc_t)xdr_CAPI2_MS_GprsRegStatusInd_Rsp_t, REQRSP_MSG_EXT( sizeof( MSRegInfo_t ), NULL, NULL, 0 )},
	{ _D(MSG_NETWORK_NAME_IND),_T("MSG_NETWORK_NAME_IND"), (xdrproc_t)xdr_CAPI2_MS_NetworkNameInd_Rsp_t, REQRSP_MSG_EXT( sizeof( nitzNetworkName_t ), NULL, NULL, 0 )},
	{ _D(MSG_RSSI_IND),_T("MSG_RSSI_IND"), (xdrproc_t)xdr_CAPI2_MS_RssiInd_Rsp_t, REQRSP_MSG_EXT( sizeof( RxSignalInfo_t ), NULL, NULL, 0 )},
	{ _D(MSG_RX_SIGNAL_INFO_CHG_IND),_T("MSG_RX_SIGNAL_INFO_CHG_IND"), (xdrproc_t)xdr_CAPI2_MS_SignalChangeInd_Rsp_t, REQRSP_MSG_EXT( sizeof( RX_SIGNAL_INFO_CHG_t ), NULL, NULL, 0 )},
	{ _D(MSG_PLMNLIST_IND),_T("MSG_PLMNLIST_IND"), (xdrproc_t)xdr_CAPI2_MS_PlmnListInd_Rsp_t, REQRSP_MSG_EXT( sizeof( SEARCHED_PLMN_LIST_t ), NULL, NULL, 3000 )},
	{ _D(MSG_DATE_TIMEZONE_IND),_T("MSG_DATE_TIMEZONE_IND"), (xdrproc_t)xdr_CAPI2_MS_TimeZoneInd_Rsp_t, REQRSP_MSG_EXT( sizeof( TimeZoneDate_t ), NULL, NULL, 0 )},
	{ _D(MSG_ADC_START_REQ),_T("MSG_ADC_START_REQ"), (xdrproc_t) xdr_CAPI2_ADCMGR_Start_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_ADC_START_RSP),_T("MSG_ADC_START_RSP"), (xdrproc_t)xdr_CAPI2_ADCMGR_Start_Rsp_t, REQRSP_MSG_EXT( sizeof( ADC_StartRsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_CAPI2_AT_COMMAND_REQ),_T("MSG_CAPI2_AT_COMMAND_REQ"), (xdrproc_t) xdr_CAPI2_AT_ProcessCmd_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_CAPI2_AT_COMMAND_RSP),_T("MSG_CAPI2_AT_COMMAND_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_CAPI2_AT_RESPONSE_IND),_T("MSG_CAPI2_AT_RESPONSE_IND"), (xdrproc_t)xdr_CAPI2_AT_Response_Rsp_t, REQRSP_MSG_EXT( sizeof( AtResponse_t ), NULL, NULL, 0 )},
	{ _D(MSG_MS_GET_SYSTEM_RAT_REQ),_T("MSG_MS_GET_SYSTEM_RAT_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_GET_SYSTEM_RAT_RSP),_T("MSG_MS_GET_SYSTEM_RAT_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetSystemRAT_Rsp_t, REQRSP_MSG_EXT( sizeof( RATSelect_t ), NULL, NULL, 0 )},
	{ _D(MSG_MS_GET_SUPPORTED_RAT_REQ),_T("MSG_MS_GET_SUPPORTED_RAT_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_GET_SUPPORTED_RAT_RSP),_T("MSG_MS_GET_SUPPORTED_RAT_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetSupportedRAT_Rsp_t, REQRSP_MSG_EXT( sizeof( RATSelect_t ), NULL, NULL, 0 )},
	{ _D(MSG_MS_GET_SYSTEM_BAND_REQ),_T("MSG_MS_GET_SYSTEM_BAND_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_GET_SYSTEM_BAND_RSP),_T("MSG_MS_GET_SYSTEM_BAND_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetSystemBand_Rsp_t, REQRSP_MSG_EXT( sizeof( BandSelect_t ), NULL, NULL, 0 )},
	{ _D(MSG_MS_GET_SUPPORTED_BAND_REQ),_T("MSG_MS_GET_SUPPORTED_BAND_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_GET_SUPPORTED_BAND_RSP),_T("MSG_MS_GET_SUPPORTED_BAND_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetSupportedBand_Rsp_t, REQRSP_MSG_EXT( sizeof( BandSelect_t ), NULL, NULL, 0 )},
	{ _D(MSG_SYSPARAM_GET_MSCLASS_REQ),_T("MSG_SYSPARAM_GET_MSCLASS_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYSPARAM_GET_MSCLASS_RSP),_T("MSG_SYSPARAM_GET_MSCLASS_RSP"), (xdrproc_t)xdr_CAPI2_SYSPARM_GetMSClass_Rsp_t, REQRSP_MSG_EXT( sizeof( UInt16 ), NULL, NULL, 0 )},
	{ _D(MSG_AUDIO_GET_MIC_GAIN_REQ),_T("MSG_AUDIO_GET_MIC_GAIN_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_AUDIO_GET_MIC_GAIN_RSP),_T("MSG_AUDIO_GET_MIC_GAIN_RSP"), (xdrproc_t)xdr_CAPI2_AUDIO_GetMicrophoneGainSetting_Rsp_t, REQRSP_MSG_EXT( sizeof( UInt16 ), NULL, NULL, 0 )},
	{ _D(MSG_AUDIO_GET_SPEAKER_VOL_REQ),_T("MSG_AUDIO_GET_SPEAKER_VOL_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_AUDIO_GET_SPEAKER_VOL_RSP),_T("MSG_AUDIO_GET_SPEAKER_VOL_RSP"), (xdrproc_t)xdr_CAPI2_AUDIO_GetSpeakerVol_Rsp_t, REQRSP_MSG_EXT( sizeof( UInt16 ), NULL, NULL, 0 )},
	{ _D(MSG_AUDIO_SET_SPEAKER_VOL_REQ),_T("MSG_AUDIO_SET_SPEAKER_VOL_REQ"), (xdrproc_t) xdr_CAPI2_AUDIO_SetSpeakerVol_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_AUDIO_SET_SPEAKER_VOL_RSP),_T("MSG_AUDIO_SET_SPEAKER_VOL_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_AUDIO_SET_MIC_GAIN_REQ),_T("MSG_AUDIO_SET_MIC_GAIN_REQ"), (xdrproc_t) xdr_CAPI2_AUDIO_SetMicrophoneGain_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_AUDIO_SET_MIC_GAIN_RSP),_T("MSG_AUDIO_SET_MIC_GAIN_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYSPARAM_GET_MNF_NAME_REQ),_T("MSG_SYSPARAM_GET_MNF_NAME_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYSPARAM_GET_MNF_NAME_RSP),_T("MSG_SYSPARAM_GET_MNF_NAME_RSP"), (xdrproc_t)xdr_CAPI2_SYSPARM_GetManufacturerName_Rsp_t, REQRSP_MSG_EXT( sizeof( uchar_ptr_t ), NULL, NULL, 0 )},
	{ _D(MSG_SYSPARAM_GET_MODEL_NAME_REQ),_T("MSG_SYSPARAM_GET_MODEL_NAME_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYSPARAM_GET_MODEL_NAME_RSP),_T("MSG_SYSPARAM_GET_MODEL_NAME_RSP"), (xdrproc_t)xdr_CAPI2_SYSPARM_GetModelName_Rsp_t, REQRSP_MSG_EXT( sizeof( uchar_ptr_t ), NULL, NULL, 0 )},
	{ _D(MSG_SYSPARAM_GET_SW_VERSION_REQ),_T("MSG_SYSPARAM_GET_SW_VERSION_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYSPARAM_GET_SW_VERSION_RSP),_T("MSG_SYSPARAM_GET_SW_VERSION_RSP"), (xdrproc_t)xdr_CAPI2_SYSPARM_GetSWVersion_Rsp_t, REQRSP_MSG_EXT( sizeof( uchar_ptr_t ), NULL, NULL, 0 )},
	{ _D(MSG_SYSPARAM_GET_EGPRS_CLASS_REQ),_T("MSG_SYSPARAM_GET_EGPRS_CLASS_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYSPARAM_GET_EGPRS_CLASS_RSP),_T("MSG_SYSPARAM_GET_EGPRS_CLASS_RSP"), (xdrproc_t)xdr_CAPI2_SYSPARM_GetEGPRSMSClass_Rsp_t, REQRSP_MSG_EXT( sizeof( UInt16 ), NULL, NULL, 0 )},
	{ _D(MSG_SYSPARAM_GET_IMEI_STR_REQ),_T("MSG_SYSPARAM_GET_IMEI_STR_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYSPARAM_GET_IMEI_STR_RSP),_T("MSG_SYSPARAM_GET_IMEI_STR_RSP"), (xdrproc_t)xdr_CAPI2_UTIL_ExtractImei_Rsp_t, REQRSP_MSG_EXT( sizeof( MSImeiStr_t ), NULL, NULL, 0 )},
	{ _D(MSG_MS_GET_REG_INFO_REQ),_T("MSG_MS_GET_REG_INFO_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_GET_REG_INFO_RSP),_T("MSG_MS_GET_REG_INFO_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetRegistrationInfo_Rsp_t, REQRSP_MSG_EXT( sizeof( MSRegStateInfo_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_PLMN_NUM_OF_ENTRY_REQ),_T("MSG_SIM_PLMN_NUM_OF_ENTRY_REQ"), (xdrproc_t) xdr_CAPI2_SIM_SendNumOfPLMNEntryReq_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_PLMN_NUM_OF_ENTRY_RSP),_T("MSG_SIM_PLMN_NUM_OF_ENTRY_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SendNumOfPLMNEntryReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_PLMN_NUM_OF_ENTRY_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_PLMN_ENTRY_DATA_REQ),_T("MSG_SIM_PLMN_ENTRY_DATA_REQ"), (xdrproc_t) xdr_CAPI2_SIM_SendReadPLMNEntryReq_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 3000 )},
	{ _D(MSG_SIM_PLMN_ENTRY_DATA_RSP),_T("MSG_SIM_PLMN_ENTRY_DATA_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SendReadPLMNEntryReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_PLMN_ENTRY_DATA_t ), NULL, NULL, 3000 )},
	{ _D(MSG_SIM_PLMN_ENTRY_UPDATE_REQ),_T("MSG_SIM_PLMN_ENTRY_UPDATE_REQ"), (xdrproc_t) xdr_CAPI2_SendWritePLMNEntry_t,REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_PLMN_ENTRY_UPDATE_RSP),_T("MSG_SIM_PLMN_ENTRY_UPDATE_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SendWritePLMNEntryReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_PLMN_ENTRY_UPDATE_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_MUL_PLMN_ENTRY_UPDATE_REQ),_T("MSG_SIM_MUL_PLMN_ENTRY_UPDATE_REQ"), (xdrproc_t) xdr_CAPI2_SendWriteMulPLMNEntry_t,REQRSP_MSG_EXT(0, NULL, NULL, 6000 )},
	{ _D(MSG_SIM_MUL_PLMN_ENTRY_UPDATE_RSP),_T("MSG_SIM_MUL_PLMN_ENTRY_UPDATE_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SendWriteMulPLMNEntryReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_MUL_PLMN_ENTRY_UPDATE_t ), NULL, NULL, 6000 )},
	{ _D(MSG_SYS_SET_REG_EVENT_MASK_REQ),_T("MSG_SYS_SET_REG_EVENT_MASK_REQ"), (xdrproc_t) xdr_CAPI2_SYS_SetRegisteredEventMask_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYS_SET_REG_EVENT_MASK_RSP),_T("MSG_SYS_SET_REG_EVENT_MASK_RSP"), (xdrproc_t)xdr_CAPI2_SYS_SetRegisteredEventMask_Rsp_t, REQRSP_MSG_EXT( sizeof( Boolean ), NULL, NULL, 0 )},
	{ _D(MSG_SYS_SET_REG_FILTER_MASK_REQ),_T("MSG_SYS_SET_REG_FILTER_MASK_REQ"), (xdrproc_t) xdr_CAPI2_SYS_SetFilteredEventMask_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYS_SET_REG_FILTER_MASK_RSP),_T("MSG_SYS_SET_REG_FILTER_MASK_RSP"), (xdrproc_t)xdr_CAPI2_SYS_SetFilteredEventMask_Rsp_t, REQRSP_MSG_EXT( sizeof( Boolean ), NULL, NULL, 0 )},
	{ _D(MSG_SYS_SET_RSSI_THRESHOLD_REQ),_T("MSG_SYS_SET_RSSI_THRESHOLD_REQ"), (xdrproc_t) xdr_CAPI2_SYS_SetRssiThreshold_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYS_SET_RSSI_THRESHOLD_RSP),_T("MSG_SYS_SET_RSSI_THRESHOLD_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYSPARAM_BOOTLOADER_VER_REQ),_T("MSG_SYSPARAM_BOOTLOADER_VER_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYSPARAM_BOOTLOADER_VER_RSP),_T("MSG_SYSPARAM_BOOTLOADER_VER_RSP"), (xdrproc_t)xdr_CAPI2_SYS_GetBootLoaderVersion_Rsp_t, REQRSP_MSG_EXT( sizeof( uchar_ptr_t ), NULL, NULL, 0 )},
	{ _D(MSG_SYSPARAM_DSF_VER_REQ),_T("MSG_SYSPARAM_DSF_VER_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYSPARAM_DSF_VER_RSP),_T("MSG_SYSPARAM_DSF_VER_RSP"), (xdrproc_t)xdr_CAPI2_SYS_GetDSFVersion_Rsp_t, REQRSP_MSG_EXT( sizeof( uchar_ptr_t ), NULL, NULL, 0 )},
	{ _D(MSG_SYSPARAM_GET_CHANNEL_MODE_REQ),_T("MSG_SYSPARAM_GET_CHANNEL_MODE_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYSPARAM_GET_CHANNEL_MODE_RSP),_T("MSG_SYSPARAM_GET_CHANNEL_MODE_RSP"), (xdrproc_t)xdr_CAPI2_SYSPARM_GetChanMode_Rsp_t, REQRSP_MSG_EXT( sizeof( UInt16 ), NULL, NULL, 0 )},
	{ _D(MSG_SYSPARAM_GET_CLASSMARK_REQ),_T("MSG_SYSPARAM_GET_CLASSMARK_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYSPARAM_GET_CLASSMARK_RSP),_T("MSG_SYSPARAM_GET_CLASSMARK_RSP"), (xdrproc_t)xdr_CAPI2_SYSPARM_GetClassmark_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_Class_t ), NULL, NULL, 0 )},
	{ _D(MSG_SYSPARAM_GET_IMEI_REQ),_T("MSG_SYSPARAM_GET_IMEI_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYSPARAM_GET_IMEI_RSP),_T("MSG_SYSPARAM_GET_IMEI_RSP"), (xdrproc_t)xdr_CAPI2_SYSPARM_GetIMEI_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_SYSPARM_IMEI_PTR_t ), NULL, NULL, 0 )},
	{ _D(MSG_SYSPARAM_GET_IND_FILE_VER_REQ),_T("MSG_SYSPARAM_GET_IND_FILE_VER_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYSPARAM_GET_IND_FILE_VER_RSP),_T("MSG_SYSPARAM_GET_IND_FILE_VER_RSP"), (xdrproc_t)xdr_CAPI2_SYSPARM_GetSysparmIndPartFileVersion_Rsp_t, REQRSP_MSG_EXT( sizeof( UInt16 ), NULL, NULL, 0 )},
	{ _D(MSG_SYS_SET_DARP_CFG_REQ),_T("MSG_SYS_SET_DARP_CFG_REQ"), (xdrproc_t) xdr_CAPI2_SYSPARM_SetDARPCfg_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYS_SET_DARP_CFG_RSP),_T("MSG_SYS_SET_DARP_CFG_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYSPARAM_SET_EGPRS_MSCLASS_REQ),_T("MSG_SYSPARAM_SET_EGPRS_MSCLASS_REQ"), (xdrproc_t) xdr_CAPI2_SYSPARM_SetEGPRSMSClass_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYSPARAM_SET_EGPRS_MSCLASS_RSP),_T("MSG_SYSPARAM_SET_EGPRS_MSCLASS_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYSPARAM_SET_GPRS_MSCLASS_REQ),_T("MSG_SYSPARAM_SET_GPRS_MSCLASS_REQ"), (xdrproc_t) xdr_CAPI2_SYSPARM_SetGPRSMSClass_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYSPARAM_SET_GPRS_MSCLASS_RSP),_T("MSG_SYSPARAM_SET_GPRS_MSCLASS_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYSPARAM_SET_POWERON_CAUSE_REQ),_T("MSG_SYSPARAM_SET_POWERON_CAUSE_REQ"), (xdrproc_t) xdr_CAPI2_SYS_SetMSPowerOnCause_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYSPARAM_SET_POWERON_CAUSE_RSP),_T("MSG_SYSPARAM_SET_POWERON_CAUSE_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_TIMEZONE_DELETE_NW_NAME_REQ),_T("MSG_TIMEZONE_DELETE_NW_NAME_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_TIMEZONE_DELETE_NW_NAME_RSP),_T("MSG_TIMEZONE_DELETE_NW_NAME_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_TIMEZONE_GET_UPDATE_MODE_REQ),_T("MSG_TIMEZONE_GET_UPDATE_MODE_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_TIMEZONE_GET_UPDATE_MODE_RSP),_T("MSG_TIMEZONE_GET_UPDATE_MODE_RSP"), (xdrproc_t)xdr_CAPI2_TIMEZONE_GetTZUpdateMode_Rsp_t, REQRSP_MSG_EXT( sizeof( TimeZoneUpdateMode_t ), NULL, NULL, 0 )},
	{ _D(MSG_TIMEZONE_SET_UPDATE_MODE_REQ),_T("MSG_TIMEZONE_SET_UPDATE_MODE_REQ"), (xdrproc_t) xdr_CAPI2_TIMEZONE_SetTZUpdateMode_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_TIMEZONE_SET_UPDATE_MODE_RSP),_T("MSG_TIMEZONE_SET_UPDATE_MODE_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_TIMEZONE_UPDATE_RTC_REQ),_T("MSG_TIMEZONE_UPDATE_RTC_REQ"), (xdrproc_t) xdr_CAPI2_TIMEZONE_UpdateRTC_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_TIMEZONE_UPDATE_RTC_RSP),_T("MSG_TIMEZONE_UPDATE_RTC_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_PMU_IS_SIM_READY_REQ),_T("MSG_PMU_IS_SIM_READY_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_PMU_IS_SIM_READY_RSP),_T("MSG_PMU_IS_SIM_READY_RSP"), (xdrproc_t)xdr_CAPI2_PMU_IsSIMReady_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_PMU_IsSIMReady_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_PMU_ACTIVATE_SIM_REQ),_T("MSG_PMU_ACTIVATE_SIM_REQ"), (xdrproc_t) xdr_CAPI2_PMU_ActivateSIM_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_PMU_ACTIVATE_SIM_RSP),_T("MSG_PMU_ACTIVATE_SIM_RSP"), (xdrproc_t)xdr_CAPI2_PMU_ActivateSIM_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_PMU_ActivateSIM_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_PMU_DEACTIVATE_SIM_REQ),_T("MSG_PMU_DEACTIVATE_SIM_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_PMU_DEACTIVATE_SIM_RSP),_T("MSG_PMU_DEACTIVATE_SIM_RSP"), (xdrproc_t)xdr_CAPI2_PMU_DeactivateSIM_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_PMU_DeactivateSIM_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_CAPI2_TEST_REQ),_T("MSG_CAPI2_TEST_REQ"), (xdrproc_t) xdr_CAPI2_TestCmds_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_CAPI2_TEST_RSP),_T("MSG_CAPI2_TEST_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_STK_SEND_PLAYTONE_RES_REQ),_T("MSG_STK_SEND_PLAYTONE_RES_REQ"), (xdrproc_t) xdr_CAPI2_SATK_SendPlayToneRes_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_STK_SEND_PLAYTONE_RES_RSP),_T("MSG_STK_SEND_PLAYTONE_RES_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_STK_SETUP_CALL_RES_REQ),_T("MSG_STK_SETUP_CALL_RES_REQ"), (xdrproc_t) xdr_CAPI2_SATK_SendSetupCallRes_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_STK_SETUP_CALL_RES_RSP),_T("MSG_STK_SETUP_CALL_RES_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_PBK_SET_FDN_CHECK_REQ),_T("MSG_PBK_SET_FDN_CHECK_REQ"), (xdrproc_t) xdr_CAPI2_PBK_SetFdnCheck_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_PBK_SET_FDN_CHECK_RSP),_T("MSG_PBK_SET_FDN_CHECK_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_PBK_GET_FDN_CHECK_REQ),_T("MSG_PBK_GET_FDN_CHECK_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_PBK_GET_FDN_CHECK_RSP),_T("MSG_PBK_GET_FDN_CHECK_RSP"), (xdrproc_t)xdr_CAPI2_PBK_GetFdnCheck_Rsp_t, REQRSP_MSG_EXT( sizeof( Boolean ), NULL, NULL, 0 )},
	{ _D(MSG_GPIO_SET_HIGH_64PIN_REQ),_T("MSG_GPIO_SET_HIGH_64PIN_REQ"), (xdrproc_t) xdr_CAPI2_GPIO_Set_High_64Pin_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_GPIO_SET_HIGH_64PIN_RSP),_T("MSG_GPIO_SET_HIGH_64PIN_RSP"), (xdrproc_t)xdr_CAPI2_GPIO_Set_High_64Pin_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_GPIO_Set_High_64Pin_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_GPIO_SET_LOW_64PIN_REQ),_T("MSG_GPIO_SET_LOW_64PIN_REQ"), (xdrproc_t) xdr_CAPI2_GPIO_Set_Low_64Pin_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_GPIO_SET_LOW_64PIN_RSP),_T("MSG_GPIO_SET_LOW_64PIN_RSP"), (xdrproc_t)xdr_CAPI2_GPIO_Set_Low_64Pin_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_GPIO_Set_Low_64Pin_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_PMU_START_CHARGING_REQ),_T("MSG_PMU_START_CHARGING_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_PMU_START_CHARGING_RSP),_T("MSG_PMU_START_CHARGING_RSP"), (xdrproc_t)xdr_CAPI2_PMU_StartCharging_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_PMU_StartCharging_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_PMU_STOP_CHARGING_REQ),_T("MSG_PMU_STOP_CHARGING_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_PMU_STOP_CHARGING_RSP),_T("MSG_PMU_STOP_CHARGING_RSP"), (xdrproc_t)xdr_CAPI2_PMU_StopCharging_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_PMU_StopCharging_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_PMU_BATT_LEVEL_REQ),_T("MSG_PMU_BATT_LEVEL_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_PMU_BATT_LEVEL_RSP),_T("MSG_PMU_BATT_LEVEL_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_PMU_BATT_LEVEL_REGISTER_REQ),_T("MSG_PMU_BATT_LEVEL_REGISTER_REQ"), (xdrproc_t) xdr_CAPI2_PMU_Battery_Register_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_PMU_BATT_LEVEL_REGISTER_RSP),_T("MSG_PMU_BATT_LEVEL_REGISTER_RSP"), (xdrproc_t)xdr_CAPI2_PMU_Battery_Register_Rsp_t, REQRSP_MSG_EXT( sizeof( HAL_EM_BATTMGR_Result_en_t ), NULL, NULL, 0 )},
	{ _D(MSG_PMU_BATT_LEVEL_IND),_T("MSG_PMU_BATT_LEVEL_IND"), (xdrproc_t)xdr_CAPI2_BattLevelInd_Rsp_t, REQRSP_MSG_EXT( sizeof( HAL_EM_BatteryLevel_t ), NULL, NULL, 0 )},
	{ _D(MSG_SMS_ISMESTORAGEENABLED_REQ),_T("MSG_SMS_ISMESTORAGEENABLED_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SMS_SEND_MEM_AVAL_IND_REQ),_T("MSG_SMS_SEND_MEM_AVAL_IND_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SMS_SEND_MEM_AVAL_IND_RSP),_T("MSG_SMS_SEND_MEM_AVAL_IND_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SMS_ISMESTORAGEENABLED_RSP),_T("MSG_SMS_ISMESTORAGEENABLED_RSP"), (xdrproc_t)xdr_CAPI2_SMS_IsMeStorageEnabled_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_SMS_IsMeStorageEnabled_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_SMS_GETMAXMECAPACITY_REQ),_T("MSG_SMS_GETMAXMECAPACITY_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SMS_GETMAXMECAPACITY_RSP),_T("MSG_SMS_GETMAXMECAPACITY_RSP"), (xdrproc_t)xdr_CAPI2_SMS_GetMaxMeCapacity_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_SMS_GetMaxMeCapacity_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_SMS_GETNEXTFREESLOT_REQ),_T("MSG_SMS_GETNEXTFREESLOT_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SMS_GETNEXTFREESLOT_RSP),_T("MSG_SMS_GETNEXTFREESLOT_RSP"), (xdrproc_t)xdr_CAPI2_SMS_GetNextFreeSlot_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_SMS_GetNextFreeSlot_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_SMS_SETMESMSSTATUS_REQ),_T("MSG_SMS_SETMESMSSTATUS_REQ"), (xdrproc_t) xdr_CAPI2_SMS_SetMeSmsStatus_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SMS_SETMESMSSTATUS_RSP),_T("MSG_SMS_SETMESMSSTATUS_RSP"), (xdrproc_t)xdr_CAPI2_SMS_SetMeSmsStatus_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_SMS_SetMeSmsStatus_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_SMS_GETMESMSSTATUS_REQ),_T("MSG_SMS_GETMESMSSTATUS_REQ"), (xdrproc_t) xdr_CAPI2_SMS_GetMeSmsStatus_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SMS_GETMESMSSTATUS_RSP),_T("MSG_SMS_GETMESMSSTATUS_RSP"), (xdrproc_t)xdr_CAPI2_SMS_GetMeSmsStatus_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_SMS_GetMeSmsStatus_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_SMS_STORESMSTOME_REQ),_T("MSG_SMS_STORESMSTOME_REQ"), (xdrproc_t) xdr_CAPI2_SMS_StoreSmsToMe_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SMS_STORESMSTOME_RSP),_T("MSG_SMS_STORESMSTOME_RSP"), (xdrproc_t)xdr_CAPI2_SMS_StoreSmsToMe_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_SMS_StoreSmsToMe_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_SMS_RETRIEVESMSFROMME_REQ),_T("MSG_SMS_RETRIEVESMSFROMME_REQ"), (xdrproc_t) xdr_CAPI2_SMS_RetrieveSmsFromMe_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SMS_RETRIEVESMSFROMME_RSP),_T("MSG_SMS_RETRIEVESMSFROMME_RSP"), (xdrproc_t)xdr_CAPI2_SMS_RetrieveSmsFromMe_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_SMS_RetrieveSmsFromMe_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_SMS_REMOVESMSFROMME_REQ),_T("MSG_SMS_REMOVESMSFROMME_REQ"), (xdrproc_t) xdr_CAPI2_SMS_RemoveSmsFromMe_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SMS_REMOVESMSFROMME_RSP),_T("MSG_SMS_REMOVESMSFROMME_RSP"), (xdrproc_t)xdr_CAPI2_SMS_RemoveSmsFromMe_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_SMS_RemoveSmsFromMe_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_SMS_CONFIGUREMESTORAGE_REQ),_T("MSG_SMS_CONFIGUREMESTORAGE_REQ"), (xdrproc_t) xdr_CAPI2_SMS_ConfigureMEStorage_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SMS_CONFIGUREMESTORAGE_RSP),_T("MSG_SMS_CONFIGUREMESTORAGE_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_SET_ELEMENT_REQ),_T("MSG_MS_SET_ELEMENT_REQ"), (xdrproc_t) xdr_CAPI2_MS_SetElement_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_SET_ELEMENT_RSP),_T("MSG_MS_SET_ELEMENT_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_GET_ELEMENT_REQ),_T("MSG_MS_GET_ELEMENT_REQ"), (xdrproc_t) xdr_CAPI2_MS_GetElement_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_GET_ELEMENT_RSP),_T("MSG_MS_GET_ELEMENT_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetElement_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_MS_Element_t ), NULL, NULL, 0 )},
	{ _D(MSG_USIM_IS_APP_SUPPORTED_REQ),_T("MSG_USIM_IS_APP_SUPPORTED_REQ"), (xdrproc_t) xdr_CAPI2_USIM_IsApplicationSupported_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_USIM_IS_APP_SUPPORTED_RSP),_T("MSG_USIM_IS_APP_SUPPORTED_RSP"), (xdrproc_t)xdr_CAPI2_USIM_IsApplicationSupported_Rsp_t, REQRSP_MSG_EXT( sizeof( Boolean ), NULL, NULL, 0 )},
	{ _D(MSG_USIM_IS_APN_ALLOWED_REQ),_T("MSG_USIM_IS_APN_ALLOWED_REQ"), (xdrproc_t) xdr_CAPI2_USIM_IsAllowedAPN_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_USIM_IS_APN_ALLOWED_RSP),_T("MSG_USIM_IS_APN_ALLOWED_RSP"), (xdrproc_t)xdr_CAPI2_USIM_IsAllowedAPN_Rsp_t, REQRSP_MSG_EXT( sizeof( Boolean ), NULL, NULL, 0 )},
	{ _D(MSG_USIM_GET_NUM_APN_REQ),_T("MSG_USIM_GET_NUM_APN_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_USIM_GET_NUM_APN_RSP),_T("MSG_USIM_GET_NUM_APN_RSP"), (xdrproc_t)xdr_CAPI2_USIM_GetNumOfAPN_Rsp_t, REQRSP_MSG_EXT( sizeof( UInt8 ), NULL, NULL, 0 )},
	{ _D(MSG_USIM_GET_APN_ENTRY_REQ),_T("MSG_USIM_GET_APN_ENTRY_REQ"), (xdrproc_t) xdr_CAPI2_USIM_GetAPNEntry_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_USIM_GET_APN_ENTRY_RSP),_T("MSG_USIM_GET_APN_ENTRY_RSP"), (xdrproc_t)xdr_CAPI2_USIM_GetAPNEntry_Rsp_t, REQRSP_MSG_EXT( sizeof( APN_NAME_t ), NULL, NULL, 0 )},
	{ _D(MSG_USIM_IS_EST_SERV_ACTIVATED_REQ),_T("MSG_USIM_IS_EST_SERV_ACTIVATED_REQ"), (xdrproc_t) xdr_CAPI2_USIM_IsEstServActivated_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_USIM_IS_EST_SERV_ACTIVATED_RSP),_T("MSG_USIM_IS_EST_SERV_ACTIVATED_RSP"), (xdrproc_t)xdr_CAPI2_USIM_IsEstServActivated_Rsp_t, REQRSP_MSG_EXT( sizeof( Boolean ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_SET_EST_SERV_REQ),_T("MSG_SIM_SET_EST_SERV_REQ"), (xdrproc_t) xdr_CAPI2_USIM_SendSetEstServReq_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_SET_EST_SERV_RSP),_T("MSG_SIM_SET_EST_SERV_RSP"), (xdrproc_t)xdr_CAPI2_USIM_SendSetEstServReq_Rsp_t, REQRSP_MSG_EXT( sizeof( USIM_FILE_UPDATE_RSP_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_UPDATE_ONE_APN_REQ),_T("MSG_SIM_UPDATE_ONE_APN_REQ"), (xdrproc_t) xdr_CAPI2_USIM_SendWriteAPNReq_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_UPDATE_ONE_APN_RSP),_T("MSG_SIM_UPDATE_ONE_APN_RSP"), (xdrproc_t)xdr_CAPI2_USIM_SendWriteAPNReq_Rsp_t, REQRSP_MSG_EXT( sizeof( USIM_FILE_UPDATE_RSP_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_DELETE_ALL_APN_REQ),_T("MSG_SIM_DELETE_ALL_APN_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_DELETE_ALL_APN_RSP),_T("MSG_SIM_DELETE_ALL_APN_RSP"), (xdrproc_t)xdr_CAPI2_USIM_SendDeleteAllAPNReq_Rsp_t, REQRSP_MSG_EXT( sizeof( USIM_FILE_UPDATE_RSP_t ), NULL, NULL, 0 )},
	{ _D(MSG_USIM_GET_RAT_MODE_REQ),_T("MSG_USIM_GET_RAT_MODE_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_USIM_GET_RAT_MODE_RSP),_T("MSG_USIM_GET_RAT_MODE_RSP"), (xdrproc_t)xdr_CAPI2_USIM_GetRatModeSetting_Rsp_t, REQRSP_MSG_EXT( sizeof( USIM_RAT_MODE_t ), NULL, NULL, 0 )},
	{ _D(MSG_PMU_CLIENT_POWER_DOWN_REQ),_T("MSG_PMU_CLIENT_POWER_DOWN_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_PMU_CLIENT_POWER_DOWN_RSP),_T("MSG_PMU_CLIENT_POWER_DOWN_RSP"), (xdrproc_t)xdr_CAPI2_PMU_ClientPowerDown_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_PMU_ClientPowerDown_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_PMU_GET_POWERUP_CAUSE_REQ),_T("MSG_PMU_GET_POWERUP_CAUSE_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_PMU_GET_POWERUP_CAUSE_RSP),_T("MSG_PMU_GET_POWERUP_CAUSE_RSP"), (xdrproc_t)xdr_CAPI2_PMU_GetPowerupCause_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_PMU_GetPowerupCause_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_MS_GET_GPRS_STATE_REQ),_T("MSG_MS_GET_GPRS_STATE_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_GET_GPRS_STATE_RSP),_T("MSG_MS_GET_GPRS_STATE_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetGPRSRegState_Rsp_t, REQRSP_MSG_EXT( sizeof( MSRegState_t ), NULL, NULL, 0 )},
	{ _D(MSG_MS_GET_GSM_STATE_REQ),_T("MSG_MS_GET_GSM_STATE_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_GET_GSM_STATE_RSP),_T("MSG_MS_GET_GSM_STATE_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetGSMRegState_Rsp_t, REQRSP_MSG_EXT( sizeof( MSRegState_t ), NULL, NULL, 0 )},
	{ _D(MSG_MS_GET_CELL_INFO_REQ),_T("MSG_MS_GET_CELL_INFO_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_GET_CELL_INFO_RSP),_T("MSG_MS_GET_CELL_INFO_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetRegisteredCellInfo_Rsp_t, REQRSP_MSG_EXT( sizeof( CellInfo_t ), NULL, NULL, 0 )},
	{ _D(MSG_MS_GET_START_BAND_REQ),_T("MSG_MS_GET_START_BAND_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_GET_START_BAND_RSP),_T("MSG_MS_GET_START_BAND_RSP"), (xdrproc_t)xdr_CAPI2_MS_GetStartBand_Rsp_t, REQRSP_MSG_EXT( sizeof( UInt8 ), NULL, NULL, 0 )},
	{ _D(MSG_MS_SETMEPOWER_CLASS_REQ),_T("MSG_MS_SETMEPOWER_CLASS_REQ"), (xdrproc_t) xdr_CAPI2_MS_SetMEPowerClass_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_SETMEPOWER_CLASS_RSP),_T("MSG_MS_SETMEPOWER_CLASS_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_USIM_GET_SERVICE_STATUS_REQ),_T("MSG_USIM_GET_SERVICE_STATUS_REQ"), (xdrproc_t) xdr_CAPI2_USIM_GetServiceStatus_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_USIM_GET_SERVICE_STATUS_RSP),_T("MSG_USIM_GET_SERVICE_STATUS_RSP"), (xdrproc_t)xdr_CAPI2_USIM_GetServiceStatus_Rsp_t, REQRSP_MSG_EXT( sizeof( SIMServiceStatus_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_IS_ALLOWED_APN_REQ),_T("MSG_SIM_IS_ALLOWED_APN_REQ"), (xdrproc_t) xdr_CAPI2_SIM_IsAllowedAPN_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_IS_ALLOWED_APN_RSP),_T("MSG_SIM_IS_ALLOWED_APN_RSP"), (xdrproc_t)xdr_CAPI2_SIM_IsAllowedAPN_Rsp_t, REQRSP_MSG_EXT( sizeof( Boolean ), NULL, NULL, 0 )},
	{ _D(MSG_SMS_GETSMSMAXCAPACITY_REQ),_T("MSG_SMS_GETSMSMAXCAPACITY_REQ"), (xdrproc_t) xdr_CAPI2_SMS_GetSmsMaxCapacity_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SMS_GETSMSMAXCAPACITY_RSP),_T("MSG_SMS_GETSMSMAXCAPACITY_RSP"), (xdrproc_t)xdr_CAPI2_SMS_GetSmsMaxCapacity_Rsp_t, REQRSP_MSG_EXT( sizeof( UInt16 ), NULL, NULL, 0 )},
	{ _D(MSG_SMS_RETRIEVEMAXCBCHNLLENGTH_REQ),_T("MSG_SMS_RETRIEVEMAXCBCHNLLENGTH_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SMS_RETRIEVEMAXCBCHNLLENGTH_RSP),_T("MSG_SMS_RETRIEVEMAXCBCHNLLENGTH_RSP"), (xdrproc_t)xdr_CAPI2_SMS_RetrieveMaxCBChnlLength_Rsp_t, REQRSP_MSG_EXT( sizeof( UInt8 ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_IS_BDN_RESTRICTED_REQ),_T("MSG_SIM_IS_BDN_RESTRICTED_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_IS_BDN_RESTRICTED_RSP),_T("MSG_SIM_IS_BDN_RESTRICTED_RSP"), (xdrproc_t)xdr_CAPI2_SIM_IsBdnOperationRestricted_Rsp_t, REQRSP_MSG_EXT( sizeof( Boolean ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_SEND_PLMN_UPDATE_IND_REQ),_T("MSG_SIM_SEND_PLMN_UPDATE_IND_REQ"), (xdrproc_t) xdr_CAPI2_SIM_SendPreferredPlmnUpdateInd_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_SEND_PLMN_UPDATE_IND_RSP),_T("MSG_SIM_SEND_PLMN_UPDATE_IND_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_DEACTIVATE_REQ),_T("MSG_SIM_DEACTIVATE_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_DEACTIVATE_RSP),_T("MSG_SIM_DEACTIVATE_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_SET_BDN_REQ),_T("MSG_SIM_SET_BDN_REQ"), (xdrproc_t) xdr_CAPI2_SIM_SendSetBdnReq_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_SET_BDN_RSP),_T("MSG_SIM_SET_BDN_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SendSetBdnReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_ACCESS_RESULT_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_POWER_ON_OFF_CARD_REQ),_T("MSG_SIM_POWER_ON_OFF_CARD_REQ"), (xdrproc_t) xdr_CAPI2_SIM_PowerOnOffCard_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_POWER_ON_OFF_CARD_RSP),_T("MSG_SIM_POWER_ON_OFF_CARD_RSP"), (xdrproc_t)xdr_CAPI2_SIM_PowerOnOffCard_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_GENERIC_APDU_RES_INFO_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_GET_RAW_ATR_REQ),_T("MSG_SIM_GET_RAW_ATR_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_GET_RAW_ATR_RSP),_T("MSG_SIM_GET_RAW_ATR_RSP"), (xdrproc_t)xdr_CAPI2_SIM_GetRawAtr_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_GENERIC_APDU_ATR_INFO_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_SET_PROTOCOL_REQ),_T("MSG_SIM_SET_PROTOCOL_REQ"), (xdrproc_t) xdr_CAPI2_SIM_Set_Protocol_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_SET_PROTOCOL_RSP),_T("MSG_SIM_SET_PROTOCOL_RSP"), (xdrproc_t)xdr_CAPI2_SIM_Set_Protocol_Rsp_t, REQRSP_MSG_EXT( sizeof( Result_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_GET_PROTOCOL_REQ),_T("MSG_SIM_GET_PROTOCOL_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_GET_PROTOCOL_RSP),_T("MSG_SIM_GET_PROTOCOL_RSP"), (xdrproc_t)xdr_CAPI2_SIM_Get_Protocol_Rsp_t, REQRSP_MSG_EXT( sizeof( UInt8 ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_SEND_GENERIC_APDU_CMD_REQ),_T("MSG_SIM_SEND_GENERIC_APDU_CMD_REQ"), (xdrproc_t) xdr_CAPI2_SIM_SendGenericApduCmd_Req_t,REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_SEND_GENERIC_APDU_CMD_RSP),_T("MSG_SIM_SEND_GENERIC_APDU_CMD_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SendGenericApduCmd_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_GENERIC_APDU_XFER_RSP_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_TERMINATE_XFER_APDU_REQ),_T("MSG_SIM_TERMINATE_XFER_APDU_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_TERMINATE_XFER_APDU_RSP),_T("MSG_SIM_TERMINATE_XFER_APDU_RSP"), (xdrproc_t)xdr_CAPI2_SIM_TerminateXferApdu_Rsp_t, REQRSP_MSG_EXT( sizeof( Result_t ), NULL, NULL, 0 )},
	{ _D(MSG_SET_PLMN_SELECT_RAT_REQ),_T("MSG_SET_PLMN_SELECT_RAT_REQ"), (xdrproc_t) xdr_CAPI2_MS_SetPlmnSelectRat_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SET_PLMN_SELECT_RAT_RSP),_T("MSG_SET_PLMN_SELECT_RAT_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_IS_DEREGISTER_IN_PROGRESS_REQ),_T("MSG_IS_DEREGISTER_IN_PROGRESS_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_IS_DEREGISTER_IN_PROGRESS_RSP),_T("MSG_IS_DEREGISTER_IN_PROGRESS_RSP"), (xdrproc_t)xdr_CAPI2_MS_IsDeRegisterInProgress_Rsp_t, REQRSP_MSG_EXT( sizeof( Boolean ), NULL, NULL, 0 )},
	{ _D(MSG_IS_REGISTER_IN_PROGRESS_REQ),_T("MSG_IS_REGISTER_IN_PROGRESS_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_IS_REGISTER_IN_PROGRESS_RSP),_T("MSG_IS_REGISTER_IN_PROGRESS_RSP"), (xdrproc_t)xdr_CAPI2_MS_IsRegisterInProgress_Rsp_t, REQRSP_MSG_EXT( sizeof( Boolean ), NULL, NULL, 0 )},
	{ _D(MSG_SOCKET_OPEN_REQ),_T("MSG_SOCKET_OPEN_REQ"), (xdrproc_t) xdr_CAPI2_SOCKET_Open_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SOCKET_OPEN_RSP),_T("MSG_SOCKET_OPEN_RSP"), (xdrproc_t)xdr_CAPI2_SOCKET_Open_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_SOCKET_Open_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_SOCKET_BIND_REQ),_T("MSG_SOCKET_BIND_REQ"), (xdrproc_t) xdr_CAPI2_SOCKET_Bind_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SOCKET_BIND_RSP),_T("MSG_SOCKET_BIND_RSP"), (xdrproc_t)xdr_CAPI2_SOCKET_Bind_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_SOCKET_Bind_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_SOCKET_LISTEN_REQ),_T("MSG_SOCKET_LISTEN_REQ"), (xdrproc_t) xdr_CAPI2_SOCKET_Listen_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SOCKET_LISTEN_RSP),_T("MSG_SOCKET_LISTEN_RSP"), (xdrproc_t)xdr_CAPI2_SOCKET_Listen_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_SOCKET_Listen_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_SOCKET_ACCEPT_REQ),_T("MSG_SOCKET_ACCEPT_REQ"), (xdrproc_t) xdr_CAPI2_SOCKET_Accept_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SOCKET_ACCEPT_RSP),_T("MSG_SOCKET_ACCEPT_RSP"), (xdrproc_t)xdr_CAPI2_SOCKET_Accept_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_SOCKET_Accept_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_SOCKET_CONNECT_REQ),_T("MSG_SOCKET_CONNECT_REQ"), (xdrproc_t) xdr_CAPI2_SOCKET_Connect_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SOCKET_CONNECT_RSP),_T("MSG_SOCKET_CONNECT_RSP"), (xdrproc_t)xdr_CAPI2_SOCKET_Connect_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_SOCKET_Connect_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_SOCKET_GETPEERNAME_REQ),_T("MSG_SOCKET_GETPEERNAME_REQ"), (xdrproc_t) xdr_CAPI2_SOCKET_GetPeerName_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SOCKET_GETPEERNAME_RSP),_T("MSG_SOCKET_GETPEERNAME_RSP"), (xdrproc_t)xdr_CAPI2_SOCKET_GetPeerName_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_SOCKET_GetPeerName_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_SOCKET_GETSOCKNAME_REQ),_T("MSG_SOCKET_GETSOCKNAME_REQ"), (xdrproc_t) xdr_CAPI2_SOCKET_GetSockName_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SOCKET_GETSOCKNAME_RSP),_T("MSG_SOCKET_GETSOCKNAME_RSP"), (xdrproc_t)xdr_CAPI2_SOCKET_GetSockName_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_SOCKET_GetSockName_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_SOCKET_SETSOCKOPT_REQ),_T("MSG_SOCKET_SETSOCKOPT_REQ"), (xdrproc_t) xdr_CAPI2_SOCKET_SetSockOpt_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SOCKET_SETSOCKOPT_RSP),_T("MSG_SOCKET_SETSOCKOPT_RSP"), (xdrproc_t)xdr_CAPI2_SOCKET_SetSockOpt_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_SOCKET_SetSockOpt_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_SOCKET_GETSOCKOPT_REQ),_T("MSG_SOCKET_GETSOCKOPT_REQ"), (xdrproc_t) xdr_CAPI2_SOCKET_GetSockOpt_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SOCKET_GETSOCKOPT_RSP),_T("MSG_SOCKET_GETSOCKOPT_RSP"), (xdrproc_t)xdr_CAPI2_SOCKET_GetSockOpt_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_SOCKET_GetSockOpt_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_CAPI2_SOCKET_SIGNAL_REQ),_T("MSG_CAPI2_SOCKET_SIGNAL_REQ"), (xdrproc_t) xdr_CAPI2_SOCKET_SignalInd_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_CAPI2_SOCKET_SIGNAL_RSP),_T("MSG_CAPI2_SOCKET_SIGNAL_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SOCKET_SEND_REQ),_T("MSG_SOCKET_SEND_REQ"), (xdrproc_t) xdr_CAPI2_SOCKET_Send_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 65536 )},
	{ _D(MSG_SOCKET_SEND_RSP),_T("MSG_SOCKET_SEND_RSP"), (xdrproc_t)xdr_CAPI2_SOCKET_Send_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_SOCKET_Send_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_SOCKET_SEND_TO_REQ),_T("MSG_SOCKET_SEND_TO_REQ"), (xdrproc_t) xdr_CAPI2_SOCKET_SendTo_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 65536 )},
	{ _D(MSG_SOCKET_SEND_TO_RSP),_T("MSG_SOCKET_SEND_TO_RSP"), (xdrproc_t)xdr_CAPI2_SOCKET_SendTo_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_SOCKET_SendTo_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_SOCKET_RECV_REQ),_T("MSG_SOCKET_RECV_REQ"), (xdrproc_t) xdr_CAPI2_SOCKET_Recv_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SOCKET_RECV_RSP),_T("MSG_SOCKET_RECV_RSP"), (xdrproc_t)xdr_CAPI2_SOCKET_Recv_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_SOCKET_Recv_RSP_Rsp_t ), NULL, NULL, 65536 )},
	{ _D(MSG_SOCKET_RECV_FROM_REQ),_T("MSG_SOCKET_RECV_FROM_REQ"), (xdrproc_t) xdr_CAPI2_SOCKET_RecvFrom_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SOCKET_RECV_FROM_RSP),_T("MSG_SOCKET_RECV_FROM_RSP"), (xdrproc_t)xdr_CAPI2_SOCKET_RecvFrom_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_SOCKET_RecvFrom_RSP_Rsp_t ), NULL, NULL, 65536 )},
	{ _D(MSG_SOCKET_CLOSE_REQ),_T("MSG_SOCKET_CLOSE_REQ"), (xdrproc_t) xdr_CAPI2_SOCKET_Close_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SOCKET_CLOSE_RSP),_T("MSG_SOCKET_CLOSE_RSP"), (xdrproc_t)xdr_CAPI2_SOCKET_Close_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_SOCKET_Close_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_SOCKET_SHUTDOWN_REQ),_T("MSG_SOCKET_SHUTDOWN_REQ"), (xdrproc_t) xdr_CAPI2_SOCKET_Shutdown_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SOCKET_SHUTDOWN_RSP),_T("MSG_SOCKET_SHUTDOWN_RSP"), (xdrproc_t)xdr_CAPI2_SOCKET_Shutdown_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_SOCKET_Shutdown_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_SOCKET_ERRNO_REQ),_T("MSG_SOCKET_ERRNO_REQ"), (xdrproc_t) xdr_CAPI2_SOCKET_Errno_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SOCKET_ERRNO_RSP),_T("MSG_SOCKET_ERRNO_RSP"), (xdrproc_t)xdr_CAPI2_SOCKET_Errno_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_SOCKET_Errno_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_SOCKET_SO2LONG_REQ),_T("MSG_SOCKET_SO2LONG_REQ"), (xdrproc_t) xdr_CAPI2_SOCKET_SO2LONG_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SOCKET_SO2LONG_RSP),_T("MSG_SOCKET_SO2LONG_RSP"), (xdrproc_t)xdr_CAPI2_SOCKET_SO2LONG_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_SOCKET_SO2LONG_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_SOCKET_GET_SOCKET_SEND_BUFFER_SPACE_REQ),_T("MSG_SOCKET_GET_SOCKET_SEND_BUFFER_SPACE_REQ"), (xdrproc_t) xdr_CAPI2_SOCKET_GetSocketSendBufferSpace_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SOCKET_GET_SOCKET_SEND_BUFFER_SPACE_RSP),_T("MSG_SOCKET_GET_SOCKET_SEND_BUFFER_SPACE_RSP"), (xdrproc_t)xdr_CAPI2_SOCKET_GetSocketSendBufferSpace_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_SOCKET_GetSocketSendBufferSpace_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_SOCKET_PARSE_IPAD_REQ),_T("MSG_SOCKET_PARSE_IPAD_REQ"), (xdrproc_t) xdr_CAPI2_SOCKET_ParseIPAddr_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SOCKET_PARSE_IPAD_RSP),_T("MSG_SOCKET_PARSE_IPAD_RSP"), (xdrproc_t)xdr_CAPI2_SOCKET_ParseIPAddr_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_SOCKET_ParseIPAddr_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_SETUP_DATA_CONNECTION_REQ),_T("MSG_SETUP_DATA_CONNECTION_REQ"), (xdrproc_t) xdr_CAPI2_DC_SetupDataConnection_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SETUP_DATA_CONNECTION_RSP),_T("MSG_SETUP_DATA_CONNECTION_RSP"), (xdrproc_t)xdr_CAPI2_DC_SetupDataConnection_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DC_SetupDataConnection_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_SETUP_DATA_CONNECTION_EX_REQ),_T("MSG_SETUP_DATA_CONNECTION_EX_REQ"), (xdrproc_t) xdr_CAPI2_DC_SetupDataConnectionEx_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SETUP_DATA_CONNECTION_EX_RSP),_T("MSG_SETUP_DATA_CONNECTION_EX_RSP"), (xdrproc_t)xdr_CAPI2_DC_SetupDataConnectionEx_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DC_SetupDataConnectionEx_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DC_REPORT_CALL_STATUS_REQ),_T("MSG_DC_REPORT_CALL_STATUS_REQ"), (xdrproc_t) xdr_CAPI2_DC_ReportCallStatusInd_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DC_REPORT_CALL_STATUS_RSP),_T("MSG_DC_REPORT_CALL_STATUS_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DC_SHUTDOWN_DATA_CONNECTION_REQ),_T("MSG_DC_SHUTDOWN_DATA_CONNECTION_REQ"), (xdrproc_t) xdr_CAPI2_DC_ShutdownDataConnection_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DC_SHUTDOWN_DATA_CONNECTION_RSP),_T("MSG_DC_SHUTDOWN_DATA_CONNECTION_RSP"), (xdrproc_t)xdr_CAPI2_DC_ShutdownDataConnection_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DC_ShutdownDataConnection_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_IS_ACCT_ID_VALID_REQ),_T("MSG_DATA_IS_ACCT_ID_VALID_REQ"), (xdrproc_t) xdr_CAPI2_DATA_IsAcctIDValid_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_IS_ACCT_ID_VALID_RSP),_T("MSG_DATA_IS_ACCT_ID_VALID_RSP"), (xdrproc_t)xdr_CAPI2_DATA_IsAcctIDValid_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_IsAcctIDValid_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_CREATE_GPRS_ACCT_REQ),_T("MSG_DATA_CREATE_GPRS_ACCT_REQ"), (xdrproc_t) xdr_CAPI2_DATA_CreateGPRSDataAcct_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_CREATE_GPRS_ACCT_RSP),_T("MSG_DATA_CREATE_GPRS_ACCT_RSP"), (xdrproc_t)xdr_CAPI2_DATA_CreateGPRSDataAcct_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_CreateGPRSDataAcct_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_CREATE_GSM_ACCT_REQ),_T("MSG_DATA_CREATE_GSM_ACCT_REQ"), (xdrproc_t) xdr_CAPI2_DATA_CreateCSDDataAcct_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_CREATE_GSM_ACCT_RSP),_T("MSG_DATA_CREATE_GSM_ACCT_RSP"), (xdrproc_t)xdr_CAPI2_DATA_CreateCSDDataAcct_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_CreateCSDDataAcct_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_DELETE_ACCT_REQ),_T("MSG_DATA_DELETE_ACCT_REQ"), (xdrproc_t) xdr_CAPI2_DATA_DeleteDataAcct_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_DELETE_ACCT_RSP),_T("MSG_DATA_DELETE_ACCT_RSP"), (xdrproc_t)xdr_CAPI2_DATA_DeleteDataAcct_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_DeleteDataAcct_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_USERNAME_REQ),_T("MSG_DATA_SET_USERNAME_REQ"), (xdrproc_t) xdr_CAPI2_DATA_SetUsername_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_USERNAME_RSP),_T("MSG_DATA_SET_USERNAME_RSP"), (xdrproc_t)xdr_CAPI2_DATA_SetUsername_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_SetUsername_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_USERNAME_REQ),_T("MSG_DATA_GET_USERNAME_REQ"), (xdrproc_t) xdr_CAPI2_DATA_GetUsername_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_USERNAME_RSP),_T("MSG_DATA_GET_USERNAME_RSP"), (xdrproc_t)xdr_CAPI2_DATA_GetUsername_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_GetUsername_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_PASSWORD_REQ),_T("MSG_DATA_SET_PASSWORD_REQ"), (xdrproc_t) xdr_CAPI2_DATA_SetPassword_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_PASSWORD_RSP),_T("MSG_DATA_SET_PASSWORD_RSP"), (xdrproc_t)xdr_CAPI2_DATA_SetPassword_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_SetPassword_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_PASSWORD_REQ),_T("MSG_DATA_GET_PASSWORD_REQ"), (xdrproc_t) xdr_CAPI2_DATA_GetPassword_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_PASSWORD_RSP),_T("MSG_DATA_GET_PASSWORD_RSP"), (xdrproc_t)xdr_CAPI2_DATA_GetPassword_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_GetPassword_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_STATIC_IP_REQ),_T("MSG_DATA_SET_STATIC_IP_REQ"), (xdrproc_t) xdr_CAPI2_DATA_SetStaticIPAddr_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_STATIC_IP_RSP),_T("MSG_DATA_SET_STATIC_IP_RSP"), (xdrproc_t)xdr_CAPI2_DATA_SetStaticIPAddr_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_SetStaticIPAddr_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_STATIC_IP_REQ),_T("MSG_DATA_GET_STATIC_IP_REQ"), (xdrproc_t) xdr_CAPI2_DATA_GetStaticIPAddr_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_STATIC_IP_RSP),_T("MSG_DATA_GET_STATIC_IP_RSP"), (xdrproc_t)xdr_CAPI2_DATA_GetStaticIPAddr_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_GetStaticIPAddr_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_PRIMARY_DNS_ADDR_REQ),_T("MSG_DATA_SET_PRIMARY_DNS_ADDR_REQ"), (xdrproc_t) xdr_CAPI2_DATA_SetPrimaryDnsAddr_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_PRIMARY_DNS_ADDR_RSP),_T("MSG_DATA_SET_PRIMARY_DNS_ADDR_RSP"), (xdrproc_t)xdr_CAPI2_DATA_SetPrimaryDnsAddr_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_SetPrimaryDnsAddr_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_PRIMARY_DNS_ADDR_REQ),_T("MSG_DATA_GET_PRIMARY_DNS_ADDR_REQ"), (xdrproc_t) xdr_CAPI2_DATA_GetPrimaryDnsAddr_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_PRIMARY_DNS_ADDR_RSP),_T("MSG_DATA_GET_PRIMARY_DNS_ADDR_RSP"), (xdrproc_t)xdr_CAPI2_DATA_GetPrimaryDnsAddr_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_GetPrimaryDnsAddr_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_SECOND_DNS_ADDR_REQ),_T("MSG_DATA_SET_SECOND_DNS_ADDR_REQ"), (xdrproc_t) xdr_CAPI2_DATA_SetSecondDnsAddr_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_SECOND_DNS_ADDR_RSP),_T("MSG_DATA_SET_SECOND_DNS_ADDR_RSP"), (xdrproc_t)xdr_CAPI2_DATA_SetSecondDnsAddr_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_SetSecondDnsAddr_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_SECOND_DNS_ADDR_REQ),_T("MSG_DATA_GET_SECOND_DNS_ADDR_REQ"), (xdrproc_t) xdr_CAPI2_DATA_GetSecondDnsAddr_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_SECOND_DNS_ADDR_RSP),_T("MSG_DATA_GET_SECOND_DNS_ADDR_RSP"), (xdrproc_t)xdr_CAPI2_DATA_GetSecondDnsAddr_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_GetSecondDnsAddr_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_DATA_COMPRESSION_REQ),_T("MSG_DATA_SET_DATA_COMPRESSION_REQ"), (xdrproc_t) xdr_CAPI2_DATA_SetDataCompression_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_DATA_COMPRESSION_RSP),_T("MSG_DATA_SET_DATA_COMPRESSION_RSP"), (xdrproc_t)xdr_CAPI2_DATA_SetDataCompression_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_SetDataCompression_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_DATA_COMPRESSION_REQ),_T("MSG_DATA_GET_DATA_COMPRESSION_REQ"), (xdrproc_t) xdr_CAPI2_DATA_GetDataCompression_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_DATA_COMPRESSION_RSP),_T("MSG_DATA_GET_DATA_COMPRESSION_RSP"), (xdrproc_t)xdr_CAPI2_DATA_GetDataCompression_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_GetDataCompression_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_ACCT_TYPE_REQ),_T("MSG_DATA_GET_ACCT_TYPE_REQ"), (xdrproc_t) xdr_CAPI2_DATA_GetAcctType_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_ACCT_TYPE_RSP),_T("MSG_DATA_GET_ACCT_TYPE_RSP"), (xdrproc_t)xdr_CAPI2_DATA_GetAcctType_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_GetAcctType_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_EMPTY_ACCT_SLOT_REQ),_T("MSG_DATA_GET_EMPTY_ACCT_SLOT_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_EMPTY_ACCT_SLOT_RSP),_T("MSG_DATA_GET_EMPTY_ACCT_SLOT_RSP"), (xdrproc_t)xdr_CAPI2_DATA_GetEmptyAcctSlot_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_GetEmptyAcctSlot_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_CID_FROM_ACCTID_REQ),_T("MSG_DATA_GET_CID_FROM_ACCTID_REQ"), (xdrproc_t) xdr_CAPI2_DATA_GetCidFromDataAcctID_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_CID_FROM_ACCTID_RSP),_T("MSG_DATA_GET_CID_FROM_ACCTID_RSP"), (xdrproc_t)xdr_CAPI2_DATA_GetCidFromDataAcctID_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_GetCidFromDataAcctID_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_CAPI2_DATA_GET_ACCTID_FROM_CID_REQ),_T("MSG_CAPI2_DATA_GET_ACCTID_FROM_CID_REQ"), (xdrproc_t) xdr_CAPI2_DATA_GetDataAcctIDFromCid_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_CAPI2_DATA_GET_ACCTID_FROM_CID_RSP),_T("MSG_CAPI2_DATA_GET_ACCTID_FROM_CID_RSP"), (xdrproc_t)xdr_CAPI2_DATA_GetDataAcctIDFromCid_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_GetDataAcctIDFromCid_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_PRI_FROM_ACCTID_REQ),_T("MSG_DATA_GET_PRI_FROM_ACCTID_REQ"), (xdrproc_t) xdr_CAPI2_DATA_GetPrimaryCidFromDataAcctID_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_PRI_FROM_ACCTID_RSP),_T("MSG_DATA_GET_PRI_FROM_ACCTID_RSP"), (xdrproc_t)xdr_CAPI2_DATA_GetPrimaryCidFromDataAcctID_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_GetPrimaryCidFromDataAcctID_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_IS_SND_DATA_ACCT_REQ),_T("MSG_DATA_IS_SND_DATA_ACCT_REQ"), (xdrproc_t) xdr_CAPI2_DATA_IsSecondaryDataAcct_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_IS_SND_DATA_ACCT_RSP),_T("MSG_DATA_IS_SND_DATA_ACCT_RSP"), (xdrproc_t)xdr_CAPI2_DATA_IsSecondaryDataAcct_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_IsSecondaryDataAcct_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_ACCTID_FROM_CID_REQ),_T("MSG_DATA_GET_ACCTID_FROM_CID_REQ"), (xdrproc_t) xdr_CAPI2_DATA_GetDataSentSize_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_ACCTID_FROM_CID_RSP),_T("MSG_DATA_GET_ACCTID_FROM_CID_RSP"), (xdrproc_t)xdr_CAPI2_DATA_GetDataSentSize_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_GetDataSentSize_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_DATA_RCV_SIZE_REQ),_T("MSG_DATA_GET_DATA_RCV_SIZE_REQ"), (xdrproc_t) xdr_CAPI2_DATA_GetDataRcvSize_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_DATA_RCV_SIZE_RSP),_T("MSG_DATA_GET_DATA_RCV_SIZE_RSP"), (xdrproc_t)xdr_CAPI2_DATA_GetDataRcvSize_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_GetDataRcvSize_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_GPRS_PDP_TYPE_REQ),_T("MSG_DATA_SET_GPRS_PDP_TYPE_REQ"), (xdrproc_t) xdr_CAPI2_DATA_SetGPRSPdpType_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_GPRS_PDP_TYPE_RSP),_T("MSG_DATA_SET_GPRS_PDP_TYPE_RSP"), (xdrproc_t)xdr_CAPI2_DATA_SetGPRSPdpType_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_SetGPRSPdpType_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_GPRS_PDP_TYPE_REQ),_T("MSG_DATA_GET_GPRS_PDP_TYPE_REQ"), (xdrproc_t) xdr_CAPI2_DATA_GetGPRSPdpType_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_GPRS_PDP_TYPE_RSP),_T("MSG_DATA_GET_GPRS_PDP_TYPE_RSP"), (xdrproc_t)xdr_CAPI2_DATA_GetGPRSPdpType_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_GetGPRSPdpType_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_GPRS_APN_REQ),_T("MSG_DATA_SET_GPRS_APN_REQ"), (xdrproc_t) xdr_CAPI2_DATA_SetGPRSApn_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_GPRS_APN_RSP),_T("MSG_DATA_SET_GPRS_APN_RSP"), (xdrproc_t)xdr_CAPI2_DATA_SetGPRSApn_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_SetGPRSApn_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_GPRS_APN_REQ),_T("MSG_DATA_GET_GPRS_APN_REQ"), (xdrproc_t) xdr_CAPI2_DATA_GetGPRSApn_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_GPRS_APN_RSP),_T("MSG_DATA_GET_GPRS_APN_RSP"), (xdrproc_t)xdr_CAPI2_DATA_GetGPRSApn_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_GetGPRSApn_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_AUTHEN_METHOD_REQ),_T("MSG_DATA_SET_AUTHEN_METHOD_REQ"), (xdrproc_t) xdr_CAPI2_DATA_SetAuthenMethod_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_AUTHEN_METHOD_RSP),_T("MSG_DATA_SET_AUTHEN_METHOD_RSP"), (xdrproc_t)xdr_CAPI2_DATA_SetAuthenMethod_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_SetAuthenMethod_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_AUTHEN_METHOD_REQ),_T("MSG_DATA_GET_AUTHEN_METHOD_REQ"), (xdrproc_t) xdr_CAPI2_DATA_GetAuthenMethod_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_AUTHEN_METHOD_RSP),_T("MSG_DATA_GET_AUTHEN_METHOD_RSP"), (xdrproc_t)xdr_CAPI2_DATA_GetAuthenMethod_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_GetAuthenMethod_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_GPRS_HEADER_COMPRESSION_REQ),_T("MSG_DATA_SET_GPRS_HEADER_COMPRESSION_REQ"), (xdrproc_t) xdr_CAPI2_DATA_SetGPRSHeaderCompression_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_GPRS_HEADER_COMPRESSION_RSP),_T("MSG_DATA_SET_GPRS_HEADER_COMPRESSION_RSP"), (xdrproc_t)xdr_CAPI2_DATA_SetGPRSHeaderCompression_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_SetGPRSHeaderCompression_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_GPRS_HEADER_COMPRESSION_REQ),_T("MSG_DATA_GET_GPRS_HEADER_COMPRESSION_REQ"), (xdrproc_t) xdr_CAPI2_DATA_GetGPRSHeaderCompression_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_GPRS_HEADER_COMPRESSION_RSP),_T("MSG_DATA_GET_GPRS_HEADER_COMPRESSION_RSP"), (xdrproc_t)xdr_CAPI2_DATA_GetGPRSHeaderCompression_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_GetGPRSHeaderCompression_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_GPRS_QOS_REQ),_T("MSG_DATA_SET_GPRS_QOS_REQ"), (xdrproc_t) xdr_CAPI2_DATA_SetGPRSQos_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_GPRS_QOS_RSP),_T("MSG_DATA_SET_GPRS_QOS_RSP"), (xdrproc_t)xdr_CAPI2_DATA_SetGPRSQos_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_SetGPRSQos_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_CAPI2_DATA_GET_GPRS_QOS_REQ),_T("MSG_CAPI2_DATA_GET_GPRS_QOS_REQ"), (xdrproc_t) xdr_CAPI2_DATA_GetGPRSQos_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_CAPI2_DATA_GET_GPRS_QOS_RSP),_T("MSG_CAPI2_DATA_GET_GPRS_QOS_RSP"), (xdrproc_t)xdr_CAPI2_DATA_GetGPRSQos_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_GetGPRSQos_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_ACCT_LOCK_REQ),_T("MSG_DATA_SET_ACCT_LOCK_REQ"), (xdrproc_t) xdr_CAPI2_DATA_SetAcctLock_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_ACCT_LOCK_RSP),_T("MSG_DATA_SET_ACCT_LOCK_RSP"), (xdrproc_t)xdr_CAPI2_DATA_SetAcctLock_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_SetAcctLock_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_ACCT_LOCK_REQ),_T("MSG_DATA_GET_ACCT_LOCK_REQ"), (xdrproc_t) xdr_CAPI2_DATA_GetAcctLock_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_ACCT_LOCK_RSP),_T("MSG_DATA_GET_ACCT_LOCK_RSP"), (xdrproc_t)xdr_CAPI2_DATA_GetAcctLock_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_GetAcctLock_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_GPRS_ONLY_REQ),_T("MSG_DATA_SET_GPRS_ONLY_REQ"), (xdrproc_t) xdr_CAPI2_DATA_SetGprsOnly_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_GPRS_ONLY_RSP),_T("MSG_DATA_SET_GPRS_ONLY_RSP"), (xdrproc_t)xdr_CAPI2_DATA_SetGprsOnly_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_SetGprsOnly_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_GPRS_ONLY_REQ),_T("MSG_DATA_GET_GPRS_ONLY_REQ"), (xdrproc_t) xdr_CAPI2_DATA_GetGprsOnly_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_GPRS_ONLY_RSP),_T("MSG_DATA_GET_GPRS_ONLY_RSP"), (xdrproc_t)xdr_CAPI2_DATA_GetGprsOnly_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_GetGprsOnly_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_GPRS_TFT_REQ),_T("MSG_DATA_SET_GPRS_TFT_REQ"), (xdrproc_t) xdr_CAPI2_DATA_SetGPRSTft_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_GPRS_TFT_RSP),_T("MSG_DATA_SET_GPRS_TFT_RSP"), (xdrproc_t)xdr_CAPI2_DATA_SetGPRSTft_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_SetGPRSTft_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_GPRS_TFT_REQ),_T("MSG_DATA_GET_GPRS_TFT_REQ"), (xdrproc_t) xdr_CAPI2_DATA_GetGPRSTft_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_GPRS_TFT_RSP),_T("MSG_DATA_GET_GPRS_TFT_RSP"), (xdrproc_t)xdr_CAPI2_DATA_GetGPRSTft_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_GetGPRSTft_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_CSD_DIAL_NUMBER_REQ),_T("MSG_DATA_SET_CSD_DIAL_NUMBER_REQ"), (xdrproc_t) xdr_CAPI2_DATA_SetCSDDialNumber_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_CSD_DIAL_NUMBER_RSP),_T("MSG_DATA_SET_CSD_DIAL_NUMBER_RSP"), (xdrproc_t)xdr_CAPI2_DATA_SetCSDDialNumber_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_SetCSDDialNumber_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_CSD_DIAL_NUMBER_REQ),_T("MSG_DATA_GET_CSD_DIAL_NUMBER_REQ"), (xdrproc_t) xdr_CAPI2_DATA_GetCSDDialNumber_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_CSD_DIAL_NUMBER_RSP),_T("MSG_DATA_GET_CSD_DIAL_NUMBER_RSP"), (xdrproc_t)xdr_CAPI2_DATA_GetCSDDialNumber_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_GetCSDDialNumber_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_CSD_DIAL_TYPE_REQ),_T("MSG_DATA_SET_CSD_DIAL_TYPE_REQ"), (xdrproc_t) xdr_CAPI2_DATA_SetCSDDialType_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_CSD_DIAL_TYPE_RSP),_T("MSG_DATA_SET_CSD_DIAL_TYPE_RSP"), (xdrproc_t)xdr_CAPI2_DATA_SetCSDDialType_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_SetCSDDialType_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_CSD_DIAL_TYPE_REQ),_T("MSG_DATA_GET_CSD_DIAL_TYPE_REQ"), (xdrproc_t) xdr_CAPI2_DATA_GetCSDDialType_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_CSD_DIAL_TYPE_RSP),_T("MSG_DATA_GET_CSD_DIAL_TYPE_RSP"), (xdrproc_t)xdr_CAPI2_DATA_GetCSDDialType_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_GetCSDDialType_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_CSD_BAUD_RATE_REQ),_T("MSG_DATA_SET_CSD_BAUD_RATE_REQ"), (xdrproc_t) xdr_CAPI2_DATA_SetCSDBaudRate_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_CSD_BAUD_RATE_RSP),_T("MSG_DATA_SET_CSD_BAUD_RATE_RSP"), (xdrproc_t)xdr_CAPI2_DATA_SetCSDBaudRate_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_SetCSDBaudRate_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_CSD_BAUD_RATE_REQ),_T("MSG_DATA_GET_CSD_BAUD_RATE_REQ"), (xdrproc_t) xdr_CAPI2_DATA_GetCSDBaudRate_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_CSD_BAUD_RATE_RSP),_T("MSG_DATA_GET_CSD_BAUD_RATE_RSP"), (xdrproc_t)xdr_CAPI2_DATA_GetCSDBaudRate_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_GetCSDBaudRate_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_CSD_SYNC_TYPE_REQ),_T("MSG_DATA_SET_CSD_SYNC_TYPE_REQ"), (xdrproc_t) xdr_CAPI2_DATA_SetCSDSyncType_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_CSD_SYNC_TYPE_RSP),_T("MSG_DATA_SET_CSD_SYNC_TYPE_RSP"), (xdrproc_t)xdr_CAPI2_DATA_SetCSDSyncType_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_SetCSDSyncType_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_CSD_SYNC_TYPE_REQ),_T("MSG_DATA_GET_CSD_SYNC_TYPE_REQ"), (xdrproc_t) xdr_CAPI2_DATA_GetCSDSyncType_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_CSD_SYNC_TYPE_RSP),_T("MSG_DATA_GET_CSD_SYNC_TYPE_RSP"), (xdrproc_t)xdr_CAPI2_DATA_GetCSDSyncType_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_GetCSDSyncType_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_CSD_ERROR_CORRECTION_REQ),_T("MSG_DATA_SET_CSD_ERROR_CORRECTION_REQ"), (xdrproc_t) xdr_CAPI2_DATA_SetCSDErrorCorrection_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_CSD_ERROR_CORRECTION_RSP),_T("MSG_DATA_SET_CSD_ERROR_CORRECTION_RSP"), (xdrproc_t)xdr_CAPI2_DATA_SetCSDErrorCorrection_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_SetCSDErrorCorrection_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_CSD_ERROR_CORRECTION_REQ),_T("MSG_DATA_GET_CSD_ERROR_CORRECTION_REQ"), (xdrproc_t) xdr_CAPI2_DATA_GetCSDErrorCorrection_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_CSD_ERROR_CORRECTION_RSP),_T("MSG_DATA_GET_CSD_ERROR_CORRECTION_RSP"), (xdrproc_t)xdr_CAPI2_DATA_GetCSDErrorCorrection_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_GetCSDErrorCorrection_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_CSD_ERR_CORRECTION_TYPE_REQ),_T("MSG_DATA_SET_CSD_ERR_CORRECTION_TYPE_REQ"), (xdrproc_t) xdr_CAPI2_DATA_SetCSDErrCorrectionType_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_CSD_ERR_CORRECTION_TYPE_RSP),_T("MSG_DATA_SET_CSD_ERR_CORRECTION_TYPE_RSP"), (xdrproc_t)xdr_CAPI2_DATA_SetCSDErrCorrectionType_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_SetCSDErrCorrectionType_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_CSD_ERR_CORRECTION_TYPE_REQ),_T("MSG_DATA_GET_CSD_ERR_CORRECTION_TYPE_REQ"), (xdrproc_t) xdr_CAPI2_DATA_GetCSDErrCorrectionType_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_CSD_ERR_CORRECTION_TYPE_RSP),_T("MSG_DATA_GET_CSD_ERR_CORRECTION_TYPE_RSP"), (xdrproc_t)xdr_CAPI2_DATA_GetCSDErrCorrectionType_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_GetCSDErrCorrectionType_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_CSD_DATA_COMP_TYPE_REQ),_T("MSG_DATA_SET_CSD_DATA_COMP_TYPE_REQ"), (xdrproc_t) xdr_CAPI2_DATA_SetCSDDataCompType_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_CSD_DATA_COMP_TYPE_RSP),_T("MSG_DATA_SET_CSD_DATA_COMP_TYPE_RSP"), (xdrproc_t)xdr_CAPI2_DATA_SetCSDDataCompType_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_SetCSDDataCompType_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_CSD_DATA_COMP_TYPE_REQ),_T("MSG_DATA_GET_CSD_DATA_COMP_TYPE_REQ"), (xdrproc_t) xdr_CAPI2_DATA_GetCSDDataCompType_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_CSD_DATA_COMP_TYPE_RSP),_T("MSG_DATA_GET_CSD_DATA_COMP_TYPE_RSP"), (xdrproc_t)xdr_CAPI2_DATA_GetCSDDataCompType_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_GetCSDDataCompType_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_CSD_CONN_ELEMENT_REQ),_T("MSG_DATA_SET_CSD_CONN_ELEMENT_REQ"), (xdrproc_t) xdr_CAPI2_DATA_SetCSDConnElement_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_SET_CSD_CONN_ELEMENT_RSP),_T("MSG_DATA_SET_CSD_CONN_ELEMENT_RSP"), (xdrproc_t)xdr_CAPI2_DATA_SetCSDConnElement_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_SetCSDConnElement_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_CSD_CONN_ELEMENT_REQ),_T("MSG_DATA_GET_CSD_CONN_ELEMENT_REQ"), (xdrproc_t) xdr_CAPI2_DATA_GetCSDConnElement_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_GET_CSD_CONN_ELEMENT_RSP),_T("MSG_DATA_GET_CSD_CONN_ELEMENT_RSP"), (xdrproc_t)xdr_CAPI2_DATA_GetCSDConnElement_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_GetCSDConnElement_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_UPDATE_ACCT_TO_FILE_SYSTEM_REQ),_T("MSG_DATA_UPDATE_ACCT_TO_FILE_SYSTEM_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_UPDATE_ACCT_TO_FILE_SYSTEM_RSP),_T("MSG_DATA_UPDATE_ACCT_TO_FILE_SYSTEM_RSP"), (xdrproc_t)xdr_CAPI2_DATA_UpdateAccountToFileSystem_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_DATA_UpdateAccountToFileSystem_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_RESET_DATA_SIZE_REQ),_T("MSG_DATA_RESET_DATA_SIZE_REQ"), (xdrproc_t) xdr_CAPI2_resetDataSize_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_RESET_DATA_SIZE_RSP),_T("MSG_DATA_RESET_DATA_SIZE_RSP"), (xdrproc_t)xdr_CAPI2_resetDataSize_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_resetDataSize_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_ADD_DATA_SENT_SIZE_BY_CID_REQ),_T("MSG_DATA_ADD_DATA_SENT_SIZE_BY_CID_REQ"), (xdrproc_t) xdr_CAPI2_addDataSentSizebyCid_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_ADD_DATA_SENT_SIZE_BY_CID_RSP),_T("MSG_DATA_ADD_DATA_SENT_SIZE_BY_CID_RSP"), (xdrproc_t)xdr_CAPI2_addDataSentSizebyCid_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_addDataSentSizebyCid_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_DATA_ADD_DATA_RCV_SIZE_BY_CID_REQ),_T("MSG_DATA_ADD_DATA_RCV_SIZE_BY_CID_REQ"), (xdrproc_t) xdr_CAPI2_addDataRcvSizebyCid_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DATA_ADD_DATA_RCV_SIZE_BY_CID_RSP),_T("MSG_DATA_ADD_DATA_RCV_SIZE_BY_CID_RSP"), (xdrproc_t)xdr_CAPI2_addDataRcvSizebyCid_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_addDataRcvSizebyCid_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_READ_USIM_PBK_HDK_REQ),_T("MSG_READ_USIM_PBK_HDK_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_READ_USIM_PBK_HDK_RSP),_T("MSG_READ_USIM_PBK_HDK_RSP"), (xdrproc_t)xdr_CAPI2_PBK_SendUsimHdkReadReq_Rsp_t, REQRSP_MSG_EXT( sizeof( USIM_PBK_READ_HDK_ENTRY_RSP_t ), NULL, NULL, 0 )},
	{ _D(MSG_WRITE_USIM_PBK_HDK_REQ),_T("MSG_WRITE_USIM_PBK_HDK_REQ"), (xdrproc_t) xdr_CAPI2_PBK_SendUsimHdkUpdateReq_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_WRITE_USIM_PBK_HDK_RSP),_T("MSG_WRITE_USIM_PBK_HDK_RSP"), (xdrproc_t)xdr_CAPI2_PBK_SendUsimHdkUpdateReq_Rsp_t, REQRSP_MSG_EXT( sizeof( USIM_PBK_UPDATE_HDK_ENTRY_RSP_t ), NULL, NULL, 0 )},
	{ _D(MSG_READ_USIM_PBK_ALPHA_AAS_REQ),_T("MSG_READ_USIM_PBK_ALPHA_AAS_REQ"), (xdrproc_t) xdr_CAPI2_PBK_SendUsimAasReadReq_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_READ_USIM_PBK_ALPHA_AAS_RSP),_T("MSG_READ_USIM_PBK_ALPHA_AAS_RSP"), (xdrproc_t)xdr_CAPI2_PBK_SendUsimAasReadReq_Rsp_t, REQRSP_MSG_EXT( sizeof( USIM_PBK_READ_ALPHA_ENTRY_RSP_t ), NULL, NULL, 0 )},
	{ _D(MSG_WRITE_USIM_PBK_ALPHA_AAS_REQ),_T("MSG_WRITE_USIM_PBK_ALPHA_AAS_REQ"), (xdrproc_t) xdr_CAPI2_PBK_SendUsimAasUpdateReq_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_WRITE_USIM_PBK_ALPHA_AAS_RSP),_T("MSG_WRITE_USIM_PBK_ALPHA_AAS_RSP"), (xdrproc_t)xdr_CAPI2_PBK_SendUsimAasUpdateReq_Rsp_t, REQRSP_MSG_EXT( sizeof( USIM_PBK_UPDATE_ALPHA_ENTRY_RSP_t ), NULL, NULL, 0 )},
	{ _D(MSG_READ_USIM_PBK_ALPHA_GAS_REQ),_T("MSG_READ_USIM_PBK_ALPHA_GAS_REQ"), (xdrproc_t) xdr_CAPI2_PBK_SendUsimGasReadReq_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_READ_USIM_PBK_ALPHA_GAS_RSP),_T("MSG_READ_USIM_PBK_ALPHA_GAS_RSP"), (xdrproc_t)xdr_CAPI2_PBK_SendUsimGasReadReq_Rsp_t, REQRSP_MSG_EXT( sizeof( USIM_PBK_READ_ALPHA_ENTRY_RSP_t ), NULL, NULL, 0 )},
	{ _D(MSG_WRITE_USIM_PBK_ALPHA_GAS_REQ),_T("MSG_WRITE_USIM_PBK_ALPHA_GAS_REQ"), (xdrproc_t) xdr_CAPI2_PBK_SendUsimGasUpdateReq_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_WRITE_USIM_PBK_ALPHA_GAS_RSP),_T("MSG_WRITE_USIM_PBK_ALPHA_GAS_RSP"), (xdrproc_t)xdr_CAPI2_PBK_SendUsimGasUpdateReq_Rsp_t, REQRSP_MSG_EXT( sizeof( USIM_PBK_UPDATE_ALPHA_ENTRY_RSP_t ), NULL, NULL, 0 )},
	{ _D(MSG_GET_USIM_PBK_ALPHA_INFO_AAS_REQ),_T("MSG_GET_USIM_PBK_ALPHA_INFO_AAS_REQ"), (xdrproc_t) xdr_CAPI2_PBK_SendUsimAasInfoReq_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_GET_USIM_PBK_ALPHA_INFO_AAS_RSP),_T("MSG_GET_USIM_PBK_ALPHA_INFO_AAS_RSP"), (xdrproc_t)xdr_CAPI2_PBK_SendUsimAasInfoReq_Rsp_t, REQRSP_MSG_EXT( sizeof( USIM_PBK_ALPHA_INFO_RSP_t ), NULL, NULL, 0 )},
	{ _D(MSG_GET_USIM_PBK_ALPHA_INFO_GAS_REQ),_T("MSG_GET_USIM_PBK_ALPHA_INFO_GAS_REQ"), (xdrproc_t) xdr_CAPI2_PBK_SendUsimGasInfoReq_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_GET_USIM_PBK_ALPHA_INFO_GAS_RSP),_T("MSG_GET_USIM_PBK_ALPHA_INFO_GAS_RSP"), (xdrproc_t)xdr_CAPI2_PBK_SendUsimGasInfoReq_Rsp_t, REQRSP_MSG_EXT( sizeof( USIM_PBK_ALPHA_INFO_RSP_t ), NULL, NULL, 0 )},
	{ _D(MSG_LCS_REQ_RESULT_IND),_T("MSG_LCS_REQ_RESULT_IND"), (xdrproc_t)xdr_CAPI2_LCS_PosReqResultInd_Rsp_t, REQRSP_MSG_EXT( sizeof( LcsPosReqResult_t ), NULL, NULL, 0 )},
	{ _D(MSG_LCS_POSITION_INFO_IND),_T("MSG_LCS_POSITION_INFO_IND"), (xdrproc_t)xdr_CAPI2_LCS_PosInfoInd_Rsp_t, REQRSP_MSG_EXT( sizeof( LcsPosInfo_t ), NULL, NULL, 0 )},
	{ _D(MSG_LCS_POSITION_DETAIL_IND),_T("MSG_LCS_POSITION_DETAIL_IND"), (xdrproc_t)xdr_CAPI2_LCS_PositionDetailInd_Rsp_t, REQRSP_MSG_EXT( sizeof( LcsPosDetail_t ), NULL, NULL, 0 )},
	{ _D(MSG_LCS_NMEA_READY_IND),_T("MSG_LCS_NMEA_READY_IND"), (xdrproc_t)xdr_CAPI2_LCS_NmeaReadyInd_Rsp_t, REQRSP_MSG_EXT( sizeof( LcsNmeaData_t ), NULL, NULL, 0 )},
	{ _D(MSG_LCS_SERVICE_CONTROL_REQ),_T("MSG_LCS_SERVICE_CONTROL_REQ"), (xdrproc_t) xdr_CAPI2_LCS_ServiceControl_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_LCS_SERVICE_CONTROL_RSP),_T("MSG_LCS_SERVICE_CONTROL_RSP"), (xdrproc_t)xdr_CAPI2_LCS_ServiceControl_Rsp_t, REQRSP_MSG_EXT( sizeof( LcsResult_t ), NULL, NULL, 0 )},
	{ _D(MSG_LCS_POWER_CONFIG_REQ),_T("MSG_LCS_POWER_CONFIG_REQ"), (xdrproc_t) xdr_CAPI2_LCS_PowerConfig_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_LCS_POWER_CONFIG_RSP),_T("MSG_LCS_POWER_CONFIG_RSP"), (xdrproc_t)xdr_CAPI2_LCS_PowerConfig_Rsp_t, REQRSP_MSG_EXT( sizeof( LcsResult_t ), NULL, NULL, 0 )},
	{ _D(MSG_LCS_SERVICE_QUERY_REQ),_T("MSG_LCS_SERVICE_QUERY_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_LCS_SERVICE_QUERY_RSP),_T("MSG_LCS_SERVICE_QUERY_RSP"), (xdrproc_t)xdr_CAPI2_LCS_ServiceQuery_Rsp_t, REQRSP_MSG_EXT( sizeof( LcsServiceType_t ), NULL, NULL, 0 )},
	{ _D(MSG_LCS_REGISTER_SUPL_MSG_HANDLER_REQ),_T("MSG_LCS_REGISTER_SUPL_MSG_HANDLER_REQ"), (xdrproc_t) xdr_CAPI2_LCS_RegisterSuplMsgHandler_Req_t,REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_LCS_REGISTER_SUPL_MSG_HANDLER_RSP),_T("MSG_LCS_REGISTER_SUPL_MSG_HANDLER_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_LCS_START_POS_REQ_PERIODIC_REQ),_T("MSG_LCS_START_POS_REQ_PERIODIC_REQ"), (xdrproc_t) xdr_CAPI2_LCS_StartPosReqPeriodic_Req_t,REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_LCS_START_POS_REQ_PERIODIC_RSP),_T("MSG_LCS_START_POS_REQ_PERIODIC_RSP"), (xdrproc_t)xdr_CAPI2_LCS_StartPosReqPeriodic_Rsp_t, REQRSP_MSG_EXT( sizeof( LcsHandle_t ), NULL, NULL, 0 )},
	{ _D(MSG_LCS_START_POS_REQ_SINGLE_REQ),_T("MSG_LCS_START_POS_REQ_SINGLE_REQ"), (xdrproc_t) xdr_CAPI2_LCS_StartPosReqSingle_Req_t,REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_LCS_START_POS_REQ_SINGLE_RSP),_T("MSG_LCS_START_POS_REQ_SINGLE_RSP"), (xdrproc_t)xdr_CAPI2_LCS_StartPosReqSingle_Rsp_t, REQRSP_MSG_EXT( sizeof( LcsHandle_t ), NULL, NULL, 0 )},
	{ _D(MSG_LCS_STOP_POS_REQ),_T("MSG_LCS_STOP_POS_REQ"), (xdrproc_t) xdr_CAPI2_LCS_StopPosReq_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_LCS_STOP_POS_RSP),_T("MSG_LCS_STOP_POS_RSP"), (xdrproc_t)xdr_CAPI2_LCS_StopPosReq_Rsp_t, REQRSP_MSG_EXT( sizeof( LcsResult_t ), NULL, NULL, 0 )},
	{ _D(MSG_LCS_GET_POSITION_REQ),_T("MSG_LCS_GET_POSITION_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_LCS_GET_POSITION_RSP),_T("MSG_LCS_GET_POSITION_RSP"), (xdrproc_t)xdr_CAPI2_LCS_GetPosition_Rsp_t, REQRSP_MSG_EXT( sizeof( LcsPosData_t ), NULL, NULL, 0 )},
	{ _D(MSG_LCS_CONFIG_SET_REQ),_T("MSG_LCS_CONFIG_SET_REQ"), (xdrproc_t) xdr_CAPI2_LCS_ConfigSet_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_LCS_CONFIG_SET_RSP),_T("MSG_LCS_CONFIG_SET_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_LCS_CONFIG_GET_REQ),_T("MSG_LCS_CONFIG_GET_REQ"), (xdrproc_t) xdr_CAPI2_LCS_ConfigGet_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_LCS_CONFIG_GET_RSP),_T("MSG_LCS_CONFIG_GET_RSP"), (xdrproc_t)xdr_CAPI2_LCS_ConfigGet_Rsp_t, REQRSP_MSG_EXT( sizeof( UInt32 ), NULL, NULL, 0 )},
	{ _D(MSG_LCS_SUPL_VERIFICATION_RSP_REQ),_T("MSG_LCS_SUPL_VERIFICATION_RSP_REQ"), (xdrproc_t) xdr_CAPI2_LCS_SuplVerificationRsp_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_LCS_SUPL_VERIFICATION_RSP_RSP),_T("MSG_LCS_SUPL_VERIFICATION_RSP_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_LCS_SUPL_CONNECT_RSP_REQ),_T("MSG_LCS_SUPL_CONNECT_RSP_REQ"), (xdrproc_t) xdr_CAPI2_LCS_SuplConnectRsp_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_LCS_SUPL_CONNECT_RSP_RSP),_T("MSG_LCS_SUPL_CONNECT_RSP_RSP"), (xdrproc_t)xdr_CAPI2_LCS_SuplConnectRsp_Rsp_t, REQRSP_MSG_EXT( sizeof( LcsResult_t ), NULL, NULL, 0 )},
	{ _D(MSG_LCS_SUPL_DATA_AVAILABLE_REQ),_T("MSG_LCS_SUPL_DATA_AVAILABLE_REQ"), (xdrproc_t) xdr_LcsSuplCommData_t,REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_LCS_SUPL_DATA_AVAILABLE_RSP),_T("MSG_LCS_SUPL_DATA_AVAILABLE_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_LCS_SUPL_DISCONNECTED_REQ),_T("MSG_LCS_SUPL_DISCONNECTED_REQ"), (xdrproc_t) xdr_CAPI2_LCS_SuplDisconnected_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_LCS_SUPL_DISCONNECTED_RSP),_T("MSG_LCS_SUPL_DISCONNECTED_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_LCS_SUPL_INIT_HMAC_RSP_REQ),_T("MSG_LCS_SUPL_INIT_HMAC_RSP_REQ"), (xdrproc_t) xdr_CAPI2_LCS_SuplInitHmacRsp_t,REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_LCS_SUPL_INIT_HMAC_RSP_RSP),_T("MSG_LCS_SUPL_INIT_HMAC_RSP_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_LCS_SUPL_INIT_HMAC_REQ_IND),_T("MSG_LCS_SUPL_INIT_HMAC_REQ_IND"), (xdrproc_t)xdr_CAPI2_LCS_SuplInitHmacReq_Rsp_t, REQRSP_MSG_EXT( sizeof( LcsSuplData_t ), NULL, NULL, 0 )},
	{ _D(MSG_LCS_SUPL_CONNECT_REQ_IND),_T("MSG_LCS_SUPL_CONNECT_REQ_IND"), (xdrproc_t)xdr_CAPI2_LCS_SuplConnectReq_Rsp_t, REQRSP_MSG_EXT( sizeof( LcsSuplConnection_t ), NULL, NULL, 0 )},
	{ _D(MSG_LCS_SUPL_DISCONNECT_REQ_IND),_T("MSG_LCS_SUPL_DISCONNECT_REQ_IND"), (xdrproc_t)xdr_CAPI2_LCS_SuplDisconnectReq_Rsp_t, REQRSP_MSG_EXT( sizeof( LcsSuplSessionInfo_t ), NULL, NULL, 0 )},
	{ _D(MSG_LCS_SUPL_NOTIFICATION_IND),_T("MSG_LCS_SUPL_NOTIFICATION_IND"), (xdrproc_t)xdr_CAPI2_LCS_SuplNotificationInd_Rsp_t, REQRSP_MSG_EXT( sizeof( LcsSuplNotificationData_t ), NULL, NULL, 0 )},
	{ _D(MSG_LCS_SUPL_WRITE_REQ_IND),_T("MSG_LCS_SUPL_WRITE_REQ_IND"), (xdrproc_t)xdr_CAPI2_LCS_SuplWriteReq_Rsp_t, REQRSP_MSG_EXT( sizeof( LcsSuplCommData_t ), NULL, NULL, 0 )},
	{ _D(MSG_LCS_CMD_DATA_REQ),_T("MSG_LCS_CMD_DATA_REQ"), (xdrproc_t) xdr_CAPI2_LCS_CmdData_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_LCS_CMD_DATA_RSP),_T("MSG_LCS_CMD_DATA_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_GPIO_CONFIG_OUTPUT_64PIN_REQ),_T("MSG_GPIO_CONFIG_OUTPUT_64PIN_REQ"), (xdrproc_t) xdr_CAPI2_GPIO_ConfigOutput_64Pin_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_GPIO_CONFIG_OUTPUT_64PIN_RSP),_T("MSG_GPIO_CONFIG_OUTPUT_64PIN_RSP"), (xdrproc_t)xdr_CAPI2_GPIO_ConfigOutput_64Pin_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_GPIO_ConfigOutput_64Pin_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_GPS_CONTROL_REQ),_T("MSG_GPS_CONTROL_REQ"), (xdrproc_t) xdr_CAPI2_GPS_Control_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_GPS_CONTROL_RSP),_T("MSG_GPS_CONTROL_RSP"), (xdrproc_t)xdr_CAPI2_GPS_Control_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_GPS_Control_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_FFS_CONTROL_REQ),_T("MSG_FFS_CONTROL_REQ"), (xdrproc_t) xdr_CAPI2_FFS_Control_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_FFS_CONTROL_RSP),_T("MSG_FFS_CONTROL_RSP"), (xdrproc_t)xdr_CAPI2_FFS_Control_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_FFS_Control_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_CP2AP_PEDESTALMODE_CONTROL_REQ),_T("MSG_CP2AP_PEDESTALMODE_CONTROL_REQ"), (xdrproc_t) xdr_CAPI2_CP2AP_PedestalMode_Control_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_CP2AP_PEDESTALMODE_CONTROL_RSP),_T("MSG_CP2AP_PEDESTALMODE_CONTROL_RSP"), (xdrproc_t)xdr_CAPI2_CP2AP_PedestalMode_Control_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_CP2AP_PedestalMode_Control_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_AUDIO_ASIC_SetAudioMode_REQ),_T("MSG_AUDIO_ASIC_SetAudioMode_REQ"), (xdrproc_t) xdr_CAPI2_AUDIO_ASIC_SetAudioMode_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_AUDIO_ASIC_SetAudioMode_RSP),_T("MSG_AUDIO_ASIC_SetAudioMode_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SPEAKER_StartTone_REQ),_T("MSG_SPEAKER_StartTone_REQ"), (xdrproc_t) xdr_CAPI2_SPEAKER_StartTone_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SPEAKER_StartTone_RSP),_T("MSG_SPEAKER_StartTone_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SPEAKER_StartGenericTone_REQ),_T("MSG_SPEAKER_StartGenericTone_REQ"), (xdrproc_t) xdr_CAPI2_SPEAKER_StartGenericTone_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SPEAKER_StartGenericTone_RSP),_T("MSG_SPEAKER_StartGenericTone_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SPEAKER_StopTone_REQ),_T("MSG_SPEAKER_StopTone_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SPEAKER_StopTone_RSP),_T("MSG_SPEAKER_StopTone_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_AUDIO_Turn_EC_NS_OnOff_REQ),_T("MSG_AUDIO_Turn_EC_NS_OnOff_REQ"), (xdrproc_t) xdr_CAPI2_AUDIO_Turn_EC_NS_OnOff_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_AUDIO_Turn_EC_NS_OnOff_RSP),_T("MSG_AUDIO_Turn_EC_NS_OnOff_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_ECHO_SetDigitalTxGain_REQ),_T("MSG_ECHO_SetDigitalTxGain_REQ"), (xdrproc_t) xdr_CAPI2_ECHO_SetDigitalTxGain_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_ECHO_SetDigitalTxGain_RSP),_T("MSG_ECHO_SetDigitalTxGain_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_RIPCMDQ_Connect_Uplink_REQ),_T("MSG_RIPCMDQ_Connect_Uplink_REQ"), (xdrproc_t) xdr_CAPI2_RIPCMDQ_Connect_Uplink_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_RIPCMDQ_Connect_Uplink_RSP),_T("MSG_RIPCMDQ_Connect_Uplink_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_RIPCMDQ_Connect_Downlink_REQ),_T("MSG_RIPCMDQ_Connect_Downlink_REQ"), (xdrproc_t) xdr_CAPI2_RIPCMDQ_Connect_Downlink_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_RIPCMDQ_Connect_Downlink_RSP),_T("MSG_RIPCMDQ_Connect_Downlink_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_VOLUMECTRL_SetBasebandVolume_REQ),_T("MSG_VOLUMECTRL_SetBasebandVolume_REQ"), (xdrproc_t) xdr_CAPI2_VOLUMECTRL_SetBasebandVolume_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_VOLUMECTRL_SetBasebandVolume_RSP),_T("MSG_VOLUMECTRL_SetBasebandVolume_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MEASURE_REPORT_PARAM_IND),_T("MSG_MEASURE_REPORT_PARAM_IND"), (xdrproc_t)xdr_CAPI2_MS_MeasureReportInd_Rsp_t, REQRSP_MSG_EXT( sizeof( MS_RxTestParam_t ), NULL, NULL, 10000 )},
	{ _D(MSG_DIAG_MEASURE_REPORT_REQ),_T("MSG_DIAG_MEASURE_REPORT_REQ"), (xdrproc_t) xdr_CAPI2_DIAG_ApiMeasurmentReportReq_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DIAG_MEASURE_REPORT_RSP),_T("MSG_DIAG_MEASURE_REPORT_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_PMU_BATT_CHARGING_NOTIFICATION_REQ),_T("MSG_PMU_BATT_CHARGING_NOTIFICATION_REQ"), (xdrproc_t) xdr_CAPI2_PMU_BattChargingNotification_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_PMU_BATT_CHARGING_NOTIFICATION_RSP),_T("MSG_PMU_BATT_CHARGING_NOTIFICATION_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_PATCH_GET_REVISION_REQ),_T("MSG_PATCH_GET_REVISION_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_PATCH_GET_REVISION_RSP),_T("MSG_PATCH_GET_REVISION_RSP"), (xdrproc_t)xdr_CAPI2_PATCH_GetRevision_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_Patch_Revision_Ptr_t ), NULL, NULL, 0 )},
	{ _D(MSG_RTC_SetTime_REQ),_T("MSG_RTC_SetTime_REQ"), (xdrproc_t) xdr_CAPI2_RTC_SetTime_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_RTC_SetTime_RSP),_T("MSG_RTC_SetTime_RSP"), (xdrproc_t)xdr_CAPI2_RTC_SetTime_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_RTC_SetTime_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_RTC_SetDST_REQ),_T("MSG_RTC_SetDST_REQ"), (xdrproc_t) xdr_CAPI2_RTC_SetDST_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_RTC_SetDST_RSP),_T("MSG_RTC_SetDST_RSP"), (xdrproc_t)xdr_CAPI2_RTC_SetDST_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_RTC_SetDST_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_RTC_SetTimeZone_REQ),_T("MSG_RTC_SetTimeZone_REQ"), (xdrproc_t) xdr_CAPI2_RTC_SetTimeZone_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_RTC_SetTimeZone_RSP),_T("MSG_RTC_SetTimeZone_RSP"), (xdrproc_t)xdr_CAPI2_RTC_SetTimeZone_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_RTC_SetTimeZone_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_MS_INITCALLCFG_REQ),_T("MSG_MS_INITCALLCFG_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_INITCALLCFG_RSP),_T("MSG_MS_INITCALLCFG_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_INITFAXCFG_REQ),_T("MSG_MS_INITFAXCFG_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_INITFAXCFG_RSP),_T("MSG_MS_INITFAXCFG_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_INITVIDEOCALLCFG_REQ),_T("MSG_MS_INITVIDEOCALLCFG_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_INITVIDEOCALLCFG_RSP),_T("MSG_MS_INITVIDEOCALLCFG_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_INITCALLCFGAMPF_REQ),_T("MSG_MS_INITCALLCFGAMPF_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_INITCALLCFGAMPF_RSP),_T("MSG_MS_INITCALLCFGAMPF_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_RTC_GetTime_REQ),_T("MSG_RTC_GetTime_REQ"), (xdrproc_t) xdr_CAPI2_RTC_GetTime_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_RTC_GetTime_RSP),_T("MSG_RTC_GetTime_RSP"), (xdrproc_t)xdr_CAPI2_RTC_GetTime_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_RTC_GetTime_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_RTC_GetTimeZone_REQ),_T("MSG_RTC_GetTimeZone_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_RTC_GetTimeZone_RSP),_T("MSG_RTC_GetTimeZone_RSP"), (xdrproc_t)xdr_CAPI2_RTC_GetTimeZone_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_RTC_GetTimeZone_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_RTC_GetDST_REQ),_T("MSG_RTC_GetDST_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_RTC_GetDST_RSP),_T("MSG_RTC_GetDST_RSP"), (xdrproc_t)xdr_CAPI2_RTC_GetDST_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_RTC_GetDST_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_SMS_GETMESMS_BUF_STATUS_REQ),_T("MSG_SMS_GETMESMS_BUF_STATUS_REQ"), (xdrproc_t) xdr_CAPI2_SMS_GetMeSmsBufferStatus_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SMS_GETMESMS_BUF_STATUS_RSP),_T("MSG_SMS_GETMESMS_BUF_STATUS_RSP"), (xdrproc_t)xdr_CAPI2_SMS_GetMeSmsBufferStatus_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_SMS_GetMeSmsBufferStatus_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_SMS_GETRECORDNUMBEROFREPLACESMS_REQ),_T("MSG_SMS_GETRECORDNUMBEROFREPLACESMS_REQ"), (xdrproc_t) xdr_CAPI2_SMS_GetRecordNumberOfReplaceSMS_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SMS_GETRECORDNUMBEROFREPLACESMS_RSP),_T("MSG_SMS_GETRECORDNUMBEROFREPLACESMS_RSP"), (xdrproc_t)xdr_CAPI2_SMS_GetRecordNumberOfReplaceSMS_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_SMS_GetRecordNumberOfReplaceSMS_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_POWER_DOWN_CNF),_T("MSG_POWER_DOWN_CNF"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_PMU_BATT_FULL_REQ),_T("MSG_PMU_BATT_FULL_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_PMU_BATT_FULL_RSP),_T("MSG_PMU_BATT_FULL_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_FLASH_SAVEIMAGE_REQ),_T("MSG_FLASH_SAVEIMAGE_REQ"), (xdrproc_t) xdr_CAPI2_FLASH_SaveImage_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_FLASH_SAVEIMAGE_RSP),_T("MSG_FLASH_SAVEIMAGE_RSP"), (xdrproc_t)xdr_CAPI2_FLASH_SaveImage_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_FLASH_SaveImage_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_AUDIO_GET_SETTINGS_REQ),_T("MSG_AUDIO_GET_SETTINGS_REQ"), (xdrproc_t) xdr_CAPI2_AUDIO_GetSettings_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_AUDIO_GET_SETTINGS_RSP),_T("MSG_AUDIO_GET_SETTINGS_RSP"), (xdrproc_t)xdr_CAPI2_AUDIO_GetSettings_Rsp_t, REQRSP_MSG_EXT( sizeof( Capi2AudioParams_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIMLOCK_GET_STATUS_REQ),_T("MSG_SIMLOCK_GET_STATUS_REQ"), (xdrproc_t) xdr_CAPI2_SIMLOCK_GetStatus_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIMLOCK_GET_STATUS_RSP),_T("MSG_SIMLOCK_GET_STATUS_RSP"), (xdrproc_t)xdr_CAPI2_SIMLOCK_GetStatus_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_SIMLOCK_GetStatus_RSP_Rsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_SIMLOCK_SET_STATUS_REQ),_T("MSG_SIMLOCK_SET_STATUS_REQ"), (xdrproc_t) xdr_CAPI2_SIMLOCK_SetStatus_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIMLOCK_SET_STATUS_RSP),_T("MSG_SIMLOCK_SET_STATUS_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYSPARM_GET_ACTUAL_LOW_VOLT_REQ),_T("MSG_SYSPARM_GET_ACTUAL_LOW_VOLT_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYSPARM_GET_ACTUAL_LOW_VOLT_RSP),_T("MSG_SYSPARM_GET_ACTUAL_LOW_VOLT_RSP"), (xdrproc_t)xdr_CAPI2_SYSPARM_GetActualLowVoltReading_Rsp_t, REQRSP_MSG_EXT( sizeof( UInt16 ), NULL, NULL, 0 )},
	{ _D(MSG_SYSPARM_GET_ACTUAL_4P2_VOLT_REQ),_T("MSG_SYSPARM_GET_ACTUAL_4P2_VOLT_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYSPARM_GET_ACTUAL_4P2_VOLT_RSP),_T("MSG_SYSPARM_GET_ACTUAL_4P2_VOLT_RSP"), (xdrproc_t)xdr_CAPI2_SYSPARM_GetActual4p2VoltReading_Rsp_t, REQRSP_MSG_EXT( sizeof( UInt16 ), NULL, NULL, 0 )},
	{ _D(MSG_PROG_EQU_TYPE_REQ),_T("MSG_PROG_EQU_TYPE_REQ"), (xdrproc_t) xdr_CAPI2_program_equalizer_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_PROG_EQU_TYPE_RSP),_T("MSG_PROG_EQU_TYPE_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_PROG_POLY_EQU_TYPE_REQ),_T("MSG_PROG_POLY_EQU_TYPE_REQ"), (xdrproc_t) xdr_CAPI2_program_poly_equalizer_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_PROG_POLY_EQU_TYPE_RSP),_T("MSG_PROG_POLY_EQU_TYPE_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_PROG_FIR_IIR_FILTER_REQ),_T("MSG_PROG_FIR_IIR_FILTER_REQ"), (xdrproc_t) xdr_CAPI2_program_FIR_IIR_filter_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_PROG_FIR_IIR_FILTER_RSP),_T("MSG_PROG_FIR_IIR_FILTER_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_PROG_POLY_FIR_IIR_FILTER_REQ),_T("MSG_PROG_POLY_FIR_IIR_FILTER_REQ"), (xdrproc_t) xdr_CAPI2_program_poly_FIR_IIR_filter_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_PROG_POLY_FIR_IIR_FILTER_RSP),_T("MSG_PROG_POLY_FIR_IIR_FILTER_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_AUDIO_CTRL_GENERIC_REQ),_T("MSG_AUDIO_CTRL_GENERIC_REQ"), (xdrproc_t) xdr_CAPI2_audio_control_generic_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_AUDIO_CTRL_GENERIC_RSP),_T("MSG_AUDIO_CTRL_GENERIC_RSP"), (xdrproc_t)xdr_CAPI2_audio_control_generic_Rsp_t, REQRSP_MSG_EXT( sizeof( UInt32 ), NULL, NULL, 0 )},
	{ _D(MSG_AUDIO_CTRL_DSP_REQ),_T("MSG_AUDIO_CTRL_DSP_REQ"), (xdrproc_t) xdr_CAPI2_audio_control_dsp_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_AUDIO_CTRL_DSP_RSP),_T("MSG_AUDIO_CTRL_DSP_RSP"), (xdrproc_t)xdr_CAPI2_audio_control_dsp_Rsp_t, REQRSP_MSG_EXT( sizeof( UInt32 ), NULL, NULL, 0 )},
	{ _D(MSG_UE_3G_STATUS_IND),_T("MSG_UE_3G_STATUS_IND"), (xdrproc_t)xdr_CAPI2_MS_Ue3gStatusInd_Rsp_t, REQRSP_MSG_EXT( sizeof( MSUe3gStatusInd_t ), NULL, NULL, 0 )},
	{ _D(MSG_CELL_INFO_IND),_T("MSG_CELL_INFO_IND"), (xdrproc_t)xdr_CAPI2_MS_CellInfoInd_Rsp_t, REQRSP_MSG_EXT( sizeof( MSCellInfoInd_t ), NULL, NULL, 0 )},
	{ _D(MSG_FFS_READ_REQ),_T("MSG_FFS_READ_REQ"), (xdrproc_t) xdr_CAPI2_FFS_Read_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_FFS_READ_RSP),_T("MSG_FFS_READ_RSP"), (xdrproc_t)xdr_CAPI2_FFS_Read_RSP_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_FFS_Read_RSP_Rsp_t ), NULL, NULL, 65536 )},
	{ _D(MSG_DIAG_CELLLOCK_REQ),_T("MSG_DIAG_CELLLOCK_REQ"), (xdrproc_t) xdr_CAPI2_DIAG_ApiCellLockReq_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DIAG_CELLLOCK_RSP),_T("MSG_DIAG_CELLLOCK_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DIAG_CELLLOCK_STATUS_REQ),_T("MSG_DIAG_CELLLOCK_STATUS_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_DIAG_CELLLOCK_STATUS_RSP),_T("MSG_DIAG_CELLLOCK_STATUS_RSP"), (xdrproc_t)xdr_CAPI2_DIAG_ApiCellLockStatus_Rsp_t, REQRSP_MSG_EXT( sizeof( Boolean ), NULL, NULL, 0 )},
	{ _D(MSG_LCS_REG_RRLP_HDL_REQ),_T("MSG_LCS_REG_RRLP_HDL_REQ"), (xdrproc_t) xdr_CAPI2_LCS_RegisterRrlpDataHandler_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_LCS_REG_RRLP_HDL_RSP),_T("MSG_LCS_REG_RRLP_HDL_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_LCS_SEND_RRLP_DATA_REQ),_T("MSG_LCS_SEND_RRLP_DATA_REQ"), (xdrproc_t) xdr_CAPI2_LCS_RrlpData_t,REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_LCS_SEND_RRLP_DATA_RSP),_T("MSG_LCS_SEND_RRLP_DATA_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_LCS_RRLP_DATA_IND),_T("MSG_LCS_RRLP_DATA_IND"), (xdrproc_t)xdr_CAPI2_LCS_RrlpDataInd_Rsp_t, REQRSP_MSG_EXT( sizeof( LcsMsgData_t ), NULL, NULL, 0 )},
	{ _D(MSG_LCS_RRLP_RESET_POS_STORED_INFO_IND),_T("MSG_LCS_RRLP_RESET_POS_STORED_INFO_IND"), (xdrproc_t)xdr_CAPI2_LCS_RrlpReset_Rsp_t, REQRSP_MSG_EXT( sizeof( LcsClientInfo_t ), NULL, NULL, 0 )},
	{ _D(MSG_LCS_REG_RRC_HDL_REQ),_T("MSG_LCS_REG_RRC_HDL_REQ"), (xdrproc_t) xdr_CAPI2_LCS_RegisterRrcDataHandler_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_LCS_REG_RRC_HDL_RSP),_T("MSG_LCS_REG_RRC_HDL_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_LCS_RRC_MEAS_REPORT_REQ),_T("MSG_LCS_RRC_MEAS_REPORT_REQ"), (xdrproc_t) xdr_CAPI2_LCS_RrcMeasReport_t,REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_LCS_RRC_MEAS_REPORT_RSP),_T("MSG_LCS_RRC_MEAS_REPORT_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_LCS_RRC_MEAS_FAILURE_REQ),_T("MSG_LCS_RRC_MEAS_FAILURE_REQ"), (xdrproc_t) xdr_CAPI2_LCS_RrcMeasFailure_t,REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_LCS_RRC_MEAS_FAILURE_RSP),_T("MSG_LCS_RRC_MEAS_FAILURE_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_LCS_RRC_STATUS_REQ),_T("MSG_LCS_RRC_STATUS_REQ"), (xdrproc_t) xdr_CAPI2_LCS_RrcStatus_t,REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_LCS_RRC_STATUS_RSP),_T("MSG_LCS_RRC_STATUS_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_LCS_RRC_ASSISTANCE_DATA_IND),_T("MSG_LCS_RRC_ASSISTANCE_DATA_IND"), (xdrproc_t)xdr_CAPI2_LCS_RrcAssistDataInd_Rsp_t, REQRSP_MSG_EXT( sizeof( LcsMsgData_t ), NULL, NULL, 0 )},
	{ _D(MSG_LCS_RRC_MEASUREMENT_CTRL_IND),_T("MSG_LCS_RRC_MEASUREMENT_CTRL_IND"), (xdrproc_t)xdr_CAPI2_LCS_RrcMeasCtrlInd_Rsp_t, REQRSP_MSG_EXT( sizeof( LcsRrcMeasurement_t ), NULL, NULL, 0 )},
	{ _D(MSG_LCS_RRC_BROADCAST_SYS_INFO_IND),_T("MSG_LCS_RRC_BROADCAST_SYS_INFO_IND"), (xdrproc_t)xdr_CAPI2_LCS_RrcSysInfoInd_Rsp_t, REQRSP_MSG_EXT( sizeof( LcsRrcBroadcastSysInfo_t ), NULL, NULL, 0 )},
	{ _D(MSG_LCS_RRC_UE_STATE_IND),_T("MSG_LCS_RRC_UE_STATE_IND"), (xdrproc_t)xdr_CAPI2_LCS_RrcUeStateInd_Rsp_t, REQRSP_MSG_EXT( sizeof( LcsRrcUeState_t ), NULL, NULL, 0 )},
	{ _D(MSG_LCS_RRC_STOP_MEASUREMENT_IND),_T("MSG_LCS_RRC_STOP_MEASUREMENT_IND"), (xdrproc_t)xdr_CAPI2_LCS_RrcStopMeasInd_Rsp_t, REQRSP_MSG_EXT( sizeof( LcsClientInfo_t ), NULL, NULL, 0 )},
	{ _D(MSG_LCS_RRC_RESET_POS_STORED_INFO_IND),_T("MSG_LCS_RRC_RESET_POS_STORED_INFO_IND"), (xdrproc_t)xdr_CAPI2_LCS_RrcReset_Rsp_t, REQRSP_MSG_EXT( sizeof( LcsClientInfo_t ), NULL, NULL, 0 )},
	{ _D(MSG_CC_IS_THERE_EMERGENCY_CALL_REQ),_T("MSG_CC_IS_THERE_EMERGENCY_CALL_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_CC_IS_THERE_EMERGENCY_CALL_RSP),_T("MSG_CC_IS_THERE_EMERGENCY_CALL_RSP"), (xdrproc_t)xdr_CAPI2_CC_IsThereEmergencyCall_Rsp_t, REQRSP_MSG_EXT( sizeof( Boolean ), NULL, NULL, 0 )},
	{ _D(MSG_SYSPARM_GET_LOW_THRESHOLD_VOLT_REQ),_T("MSG_SYSPARM_GET_LOW_THRESHOLD_VOLT_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYSPARM_GET_LOW_THRESHOLD_VOLT_RSP),_T("MSG_SYSPARM_GET_LOW_THRESHOLD_VOLT_RSP"), (xdrproc_t)xdr_CAPI2_SYSPARM_GetBattLowThresh_Rsp_t, REQRSP_MSG_EXT( sizeof( UInt16 ), NULL, NULL, 0 )},
	{ _D(MSG_SYSPARM_GET_DEFAULT_4P2_VOLT_REQ),_T("MSG_SYSPARM_GET_DEFAULT_4P2_VOLT_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYSPARM_GET_DEFAULT_4P2_VOLT_RSP),_T("MSG_SYSPARM_GET_DEFAULT_4P2_VOLT_RSP"), (xdrproc_t)xdr_CAPI2_SYSPARM_GetDefault4p2VoltReading_Rsp_t, REQRSP_MSG_EXT( sizeof( UInt16 ), NULL, NULL, 0 )},
	{ _D(MSG_L1_BB_ISLOCKED_REQ),_T("MSG_L1_BB_ISLOCKED_REQ"), (xdrproc_t) xdr_CAPI2_L1_bb_isLocked_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_L1_BB_ISLOCKED_RSP),_T("MSG_L1_BB_ISLOCKED_RSP"), (xdrproc_t)xdr_CAPI2_L1_bb_isLocked_Rsp_t, REQRSP_MSG_EXT( sizeof( Boolean ), NULL, NULL, 0 )},
	{ _D(MSG_SERVING_CELL_INFO_IND),_T("MSG_SERVING_CELL_INFO_IND"), (xdrproc_t)xdr_CAPI2_ServingCellInfoInd_Rsp_t, REQRSP_MSG_EXT( sizeof( ServingCellInfo_t ), NULL, NULL, 10000 )},
	{ _D(MSG_MS_FORCE_PS_REL_REQ),_T("MSG_MS_FORCE_PS_REL_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_MS_FORCE_PS_REL_RSP),_T("MSG_MS_FORCE_PS_REL_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SMSPP_APP_SPECIFIC_SMS_IND),_T("MSG_SMSPP_APP_SPECIFIC_SMS_IND"), (xdrproc_t)xdr_CAPI2_SMSPP_AppSpecificInd_Rsp_t, REQRSP_MSG_EXT( sizeof( SmsAppSpecificData_t ), NULL, NULL, 0 )},
	{ _D(MSG_SYS_POWERUP_NORF_CHG_REQ),_T("MSG_SYS_POWERUP_NORF_CHG_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYS_POWERUP_NORF_CHG_RSP),_T("MSG_SYS_POWERUP_NORF_CHG_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYSPARM_GET_GPIO_REQ),_T("MSG_SYSPARM_GET_GPIO_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYSPARM_GET_GPIO_RSP),_T("MSG_SYSPARM_GET_GPIO_RSP"), (xdrproc_t)xdr_CAPI2_SYSPARM_GetGPIO_Value_Rsp_t, REQRSP_MSG_EXT( sizeof( Capi2GpioValue_t ), NULL, NULL, 0 )},
	{ _D(MSG_SYS_ENABLE_CELL_INFO_REQ),_T("MSG_SYS_ENABLE_CELL_INFO_REQ"), (xdrproc_t) xdr_CAPI2_SYS_EnableCellInfoMsg_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYS_ENABLE_CELL_INFO_RSP),_T("MSG_SYS_ENABLE_CELL_INFO_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYS_SET_HSDPA_CATEGORY_REQ),_T("MSG_SYS_SET_HSDPA_CATEGORY_REQ"), (xdrproc_t) xdr_CAPI2_SYSPARM_SetHSDPAPHYCategory_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYS_SET_HSDPA_CATEGORY_RSP),_T("MSG_SYS_SET_HSDPA_CATEGORY_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYS_GET_HSDPA_CATEGORY_REQ),_T("MSG_SYS_GET_HSDPA_CATEGORY_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYS_GET_HSDPA_CATEGORY_RSP),_T("MSG_SYS_GET_HSDPA_CATEGORY_RSP"), (xdrproc_t)xdr_CAPI2_SYSPARM_GetHSDPAPHYCategory_Rsp_t, REQRSP_MSG_EXT( sizeof( UInt32 ), NULL, NULL, 0 )},
	{ _D(MSG_SIM_START_MULTI_SMS_XFR_REQ),_T("MSG_SIM_START_MULTI_SMS_XFR_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_START_MULTI_SMS_XFR_RSP),_T("MSG_SIM_START_MULTI_SMS_XFR_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_STOP_MULTI_SMS_XFR_REQ),_T("MSG_SIM_STOP_MULTI_SMS_XFR_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_STOP_MULTI_SMS_XFR_RSP),_T("MSG_SIM_STOP_MULTI_SMS_XFR_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_SMS_WRITE_REQ),_T("MSG_SIM_SMS_WRITE_REQ"), (xdrproc_t) xdr_CAPI2_SIM_SendWriteSmsReq_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SIM_SMS_WRITE_RSP),_T("MSG_SIM_SMS_WRITE_RSP"), (xdrproc_t)xdr_CAPI2_SIM_SendWriteSmsReq_Rsp_t, REQRSP_MSG_EXT( sizeof( SIM_SMS_UPDATE_RESULT_t ), NULL, NULL, 0 )},
	{ _D(MSG_BATTMGR_GETCHARGINGSTATUS_REQ),_T("MSG_BATTMGR_GETCHARGINGSTATUS_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_BATTMGR_GETCHARGINGSTATUS_RSP),_T("MSG_BATTMGR_GETCHARGINGSTATUS_RSP"), (xdrproc_t)xdr_CAPI2_BATTMGR_GetChargingStatus_Rsp_t, REQRSP_MSG_EXT( sizeof( EM_BATTMGR_ChargingStatus_en_t ), NULL, NULL, 0 )},
	{ _D(MSG_BATTMGR_GETPERCENTAGELEVEL_REQ),_T("MSG_BATTMGR_GETPERCENTAGELEVEL_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_BATTMGR_GETPERCENTAGELEVEL_RSP),_T("MSG_BATTMGR_GETPERCENTAGELEVEL_RSP"), (xdrproc_t)xdr_CAPI2_BATTMGR_GetPercentageLevel_Rsp_t, REQRSP_MSG_EXT( sizeof( UInt16 ), NULL, NULL, 0 )},
	{ _D(MSG_BATTMGR_ISBATTERYPRESENT_REQ),_T("MSG_BATTMGR_ISBATTERYPRESENT_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_BATTMGR_ISBATTERYPRESENT_RSP),_T("MSG_BATTMGR_ISBATTERYPRESENT_RSP"), (xdrproc_t)xdr_CAPI2_BATTMGR_IsBatteryPresent_Rsp_t, REQRSP_MSG_EXT( sizeof( Boolean ), NULL, NULL, 0 )},
	{ _D(MSG_BATTMGR_ISCHARGERPLUGIN_REQ),_T("MSG_BATTMGR_ISCHARGERPLUGIN_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_BATTMGR_ISCHARGERPLUGIN_RSP),_T("MSG_BATTMGR_ISCHARGERPLUGIN_RSP"), (xdrproc_t)xdr_CAPI2_BATTMGR_IsChargerPlugIn_Rsp_t, REQRSP_MSG_EXT( sizeof( Boolean ), NULL, NULL, 0 )},
	{ _D(MSG_SYSPARM_GETBATT_TABLE_REQ),_T("MSG_SYSPARM_GETBATT_TABLE_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SYSPARM_GETBATT_TABLE_RSP),_T("MSG_SYSPARM_GETBATT_TABLE_RSP"), (xdrproc_t)xdr_CAPI2_SYSPARM_GetBattTable_Rsp_t, REQRSP_MSG_EXT( sizeof( Batt_Level_Table_t ), NULL, NULL, 0 )},
	{ _D(MSG_BATTMGR_GET_LEVEL_REQ),_T("MSG_BATTMGR_GET_LEVEL_REQ"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_BATTMGR_GET_LEVEL_RSP),_T("MSG_BATTMGR_GET_LEVEL_RSP"), (xdrproc_t)xdr_CAPI2_BATTMGR_GetLevel_Rsp_t, REQRSP_MSG_EXT( sizeof( UInt16 ), NULL, NULL, 0 )},
	{ _D(MSG_CAPI2_AT_COMMAND_TO_AP_REQ),_T("MSG_CAPI2_AT_COMMAND_TO_AP_REQ"), (xdrproc_t) xdr_CAPI2_AT_ProcessCmdToAP_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SMS_WRITESMSPDUTO_SIMRECORD_REQ),_T("MSG_SMS_WRITESMSPDUTO_SIMRECORD_REQ"), (xdrproc_t) xdr_CAPI2_SMS_WriteSMSPduToSIMRecordReq_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_SMS_WRITESMSPDUTO_SIMRECORD_RSP),_T("MSG_SMS_WRITESMSPDUTO_SIMRECORD_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_ADC_MULTI_CH_START_REQ),_T("MSG_ADC_MULTI_CH_START_REQ"), (xdrproc_t) xdr_CAPI2_ADCMGR_MultiChStart_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_ADC_MULTI_CH_START_RSP),_T("MSG_ADC_MULTI_CH_START_RSP"), (xdrproc_t)xdr_CAPI2_ADCMGR_MultiChStart_Rsp_t, REQRSP_MSG_EXT( sizeof( CAPI2_ADC_ChannelRsp_t ), NULL, NULL, 0 )},
	{ _D(MSG_INTERTASK_MSG_TO_CP_REQ),_T("MSG_INTERTASK_MSG_TO_CP_REQ"), (xdrproc_t) xdr_CAPI2_InterTaskMsgToCP_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_INTERTASK_MSG_TO_CP_RSP),_T("MSG_INTERTASK_MSG_TO_CP_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_INTERTASK_MSG_TO_AP_REQ),_T("MSG_INTERTASK_MSG_TO_AP_REQ"), (xdrproc_t) xdr_CAPI2_InterTaskMsgToAP_Req_t, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
	{ _D(MSG_INTERTASK_MSG_TO_AP_RSP),_T("MSG_INTERTASK_MSG_TO_AP_RSP"), (xdrproc_t) xdr_default_proc, REQRSP_MSG_EXT(0, NULL, NULL, 0 )},
#endif //DEFINE_XDR_UNION_MAPPING



#ifdef DEFINE_XDR_UNION_DECLARE

	CAPI2_MS_IsGSMRegistered_Rsp_t   CAPI2_MS_IsGSMRegistered_Rsp;
	CAPI2_MS_IsGPRSRegistered_Rsp_t   CAPI2_MS_IsGPRSRegistered_Rsp;
	CAPI2_MS_GetGSMRegCause_Rsp_t   CAPI2_MS_GetGSMRegCause_Rsp;
	CAPI2_MS_GetGPRSRegCause_Rsp_t   CAPI2_MS_GetGPRSRegCause_Rsp;
	CAPI2_MS_GetRegisteredLAC_Rsp_t   CAPI2_MS_GetRegisteredLAC_Rsp;
	CAPI2_MS_GetPlmnMCC_Rsp_t   CAPI2_MS_GetPlmnMCC_Rsp;
	CAPI2_MS_GetPlmnMNC_Rsp_t   CAPI2_MS_GetPlmnMNC_Rsp;
	CAPI2_SYS_GetMSPowerOnCause_Rsp_t   CAPI2_SYS_GetMSPowerOnCause_Rsp;
	CAPI2_MS_IsGprsAllowed_Rsp_t   CAPI2_MS_IsGprsAllowed_Rsp;
	CAPI2_MS_GetCurrentRAT_Rsp_t   CAPI2_MS_GetCurrentRAT_Rsp;
	CAPI2_MS_GetCurrentBand_Rsp_t   CAPI2_MS_GetCurrentBand_Rsp;
	CAPI2_MS_SetStartBand_Req_t   CAPI2_MS_SetStartBand_Req;
	CAPI2_SIM_UpdateSMSCapExceededFlag_Req_t   CAPI2_SIM_UpdateSMSCapExceededFlag_Req;
	CAPI2_SYS_SelectBand_Req_t   CAPI2_SYS_SelectBand_Req;
	CAPI2_MS_SetSupportedRATandBand_Req_t   CAPI2_MS_SetSupportedRATandBand_Req;
	CAPI2_PLMN_GetCountryByMcc_Req_t   CAPI2_PLMN_GetCountryByMcc_Req;
	CAPI2_PLMN_GetCountryByMcc_Rsp_t   CAPI2_PLMN_GetCountryByMcc_Rsp;
	CAPI2_MS_GetPLMNEntryByIndex_Req_t   CAPI2_MS_GetPLMNEntryByIndex_Req;
	CAPI2_MS_GetPLMNEntryByIndex_Rsp_t   CAPI2_MS_GetPLMNEntryByIndex_Rsp;
	CAPI2_MS_GetPLMNListSize_Rsp_t   CAPI2_MS_GetPLMNListSize_Rsp;
	CAPI2_MS_GetPLMNByCode_Req_t   CAPI2_MS_GetPLMNByCode_Req;
	CAPI2_MS_GetPLMNByCode_Rsp_t   CAPI2_MS_GetPLMNByCode_Rsp;
	CAPI2_MS_PlmnSelect_Req_t   CAPI2_MS_PlmnSelect_Req;
	CAPI2_MS_PlmnSelect_Rsp_t   CAPI2_MS_PlmnSelect_Rsp;
	CAPI2_MS_GetPlmnMode_Rsp_t   CAPI2_MS_GetPlmnMode_Rsp;
	CAPI2_MS_SetPlmnMode_Req_t   CAPI2_MS_SetPlmnMode_Req;
	CAPI2_MS_GetPlmnFormat_Rsp_t   CAPI2_MS_GetPlmnFormat_Rsp;
	CAPI2_MS_SetPlmnFormat_Req_t   CAPI2_MS_SetPlmnFormat_Req;
	CAPI2_MS_IsMatchedPLMN_Req_t   CAPI2_MS_IsMatchedPLMN_Req;
	CAPI2_MS_IsMatchedPLMN_Rsp_t   CAPI2_MS_IsMatchedPLMN_Rsp;
	CAPI2_MS_GetPLMNNameByCode_Req_t   CAPI2_MS_GetPLMNNameByCode_Req;
	CAPI2_MS_GetPLMNNameByCode_Rsp_t   CAPI2_MS_GetPLMNNameByCode_Rsp;
	CAPI2_SYS_IsResetCausedByAssert_Rsp_t   CAPI2_SYS_IsResetCausedByAssert_Rsp;
	CAPI2_SYS_GetSystemState_Rsp_t   CAPI2_SYS_GetSystemState_Rsp;
	CAPI2_SYS_SetSystemState_Req_t   CAPI2_SYS_SetSystemState_Req;
	CAPI2_SYS_GetRxSignalInfo_Rsp_t   CAPI2_SYS_GetRxSignalInfo_Rsp;
	CAPI2_SYS_GetGSMRegistrationStatus_Rsp_t   CAPI2_SYS_GetGSMRegistrationStatus_Rsp;
	CAPI2_SYS_GetGPRSRegistrationStatus_Rsp_t   CAPI2_SYS_GetGPRSRegistrationStatus_Rsp;
	CAPI2_SYS_IsRegisteredGSMOrGPRS_Rsp_t   CAPI2_SYS_IsRegisteredGSMOrGPRS_Rsp;
	CAPI2_SYS_GetGSMRegistrationCause_Rsp_t   CAPI2_SYS_GetGSMRegistrationCause_Rsp;
	CAPI2_MS_IsPlmnForbidden_Rsp_t   CAPI2_MS_IsPlmnForbidden_Rsp;
	CAPI2_MS_IsRegisteredHomePLMN_Rsp_t   CAPI2_MS_IsRegisteredHomePLMN_Rsp;
	CAPI2_MS_SetPowerDownTimer_Req_t   CAPI2_MS_SetPowerDownTimer_Req;
	CAPI2_SIMLockIsLockOn_Req_t   CAPI2_SIMLockIsLockOn_Req;
	CAPI2_SIMLockIsLockOn_Rsp_t   CAPI2_SIMLockIsLockOn_Rsp;
	CAPI2_SIMLockCheckAllLocks_Req_t   CAPI2_SIMLockCheckAllLocks_Req;
	CAPI2_SIMLockCheckAllLocks_Rsp_t   CAPI2_SIMLockCheckAllLocks_Rsp;
	CAPI2_SIMLockUnlockSIM_Req_t   CAPI2_SIMLockUnlockSIM_Req;
	CAPI2_SIMLockUnlockSIM_Rsp_t   CAPI2_SIMLockUnlockSIM_Rsp;
	CAPI2_SIMLockSetLock_Rsp_t   CAPI2_SIMLockSetLock_Rsp;
	CAPI2_SIMLockGetCurrentClosedLock_Rsp_t   CAPI2_SIMLockGetCurrentClosedLock_Rsp;
	CAPI2_SIMLockChangePasswordPHSIM_Req_t   CAPI2_SIMLockChangePasswordPHSIM_Req;
	CAPI2_SIMLockChangePasswordPHSIM_Rsp_t   CAPI2_SIMLockChangePasswordPHSIM_Rsp;
	CAPI2_SIMLockCheckPasswordPHSIM_Req_t   CAPI2_SIMLockCheckPasswordPHSIM_Req;
	CAPI2_SIMLockCheckPasswordPHSIM_Rsp_t   CAPI2_SIMLockCheckPasswordPHSIM_Rsp;
	CAPI2_SIMLockGetSignature_Rsp_t   CAPI2_SIMLockGetSignature_Rsp;
	CAPI2_SIMLockGetImeiSecboot_Rsp_t   CAPI2_SIMLockGetImeiSecboot_Rsp;
	CAPI2_SIM_GetSmsParamRecNum_Rsp_t   CAPI2_SIM_GetSmsParamRecNum_Rsp;
	CAPI2_MS_ConvertPLMNNameStr_Rsp_t   CAPI2_MS_ConvertPLMNNameStr_Rsp;
	CAPI2_SIM_GetSmsMemExceededFlag_Rsp_t   CAPI2_SIM_GetSmsMemExceededFlag_Rsp;
	CAPI2_SIM_IsPINRequired_Rsp_t   CAPI2_SIM_IsPINRequired_Rsp;
	CAPI2_SIM_GetCardPhase_Rsp_t   CAPI2_SIM_GetCardPhase_Rsp;
	CAPI2_SIM_GetSIMType_Rsp_t   CAPI2_SIM_GetSIMType_Rsp;
	CAPI2_SIM_GetPresentStatus_Rsp_t   CAPI2_SIM_GetPresentStatus_Rsp;
	CAPI2_SIM_IsOperationRestricted_Rsp_t   CAPI2_SIM_IsOperationRestricted_Rsp;
	CAPI2_SIM_IsPINBlocked_Req_t   CAPI2_SIM_IsPINBlocked_Req;
	CAPI2_SIM_IsPINBlocked_Rsp_t   CAPI2_SIM_IsPINBlocked_Rsp;
	CAPI2_SIM_IsPUKBlocked_Req_t   CAPI2_SIM_IsPUKBlocked_Req;
	CAPI2_SIM_IsPUKBlocked_Rsp_t   CAPI2_SIM_IsPUKBlocked_Rsp;
	CAPI2_SIM_IsInvalidSIM_Rsp_t   CAPI2_SIM_IsInvalidSIM_Rsp;
	CAPI2_SIM_DetectSim_Rsp_t   CAPI2_SIM_DetectSim_Rsp;
	CAPI2_SIM_GetRuimSuppFlag_Rsp_t   CAPI2_SIM_GetRuimSuppFlag_Rsp;
	CAPI2_SIM_SendVerifyChvReq_Req_t   CAPI2_SIM_SendVerifyChvReq_Req;
	CAPI2_SIM_SendVerifyChvReq_Rsp_t   CAPI2_SIM_SendVerifyChvReq_Rsp;
	CAPI2_SIM_SendChangeChvReq_Req_t   CAPI2_SIM_SendChangeChvReq_Req;
	CAPI2_SIM_SendChangeChvReq_Rsp_t   CAPI2_SIM_SendChangeChvReq_Rsp;
	CAPI2_SIM_SendSetChv1OnOffReq_Req_t   CAPI2_SIM_SendSetChv1OnOffReq_Req;
	CAPI2_SIM_SendSetChv1OnOffReq_Rsp_t   CAPI2_SIM_SendSetChv1OnOffReq_Rsp;
	CAPI2_SIM_SendUnblockChvReq_Req_t   CAPI2_SIM_SendUnblockChvReq_Req;
	CAPI2_SIM_SendUnblockChvReq_Rsp_t   CAPI2_SIM_SendUnblockChvReq_Rsp;
	CAPI2_SIM_SendSetOperStateReq_Req_t   CAPI2_SIM_SendSetOperStateReq_Req;
	CAPI2_SIM_SendSetOperStateReq_Rsp_t   CAPI2_SIM_SendSetOperStateReq_Rsp;
	CAPI2_SIM_IsPbkAccessAllowed_Req_t   CAPI2_SIM_IsPbkAccessAllowed_Req;
	CAPI2_SIM_IsPbkAccessAllowed_Rsp_t   CAPI2_SIM_IsPbkAccessAllowed_Rsp;
	CAPI2_SIM_SendPbkInfoReq_Req_t   CAPI2_SIM_SendPbkInfoReq_Req;
	CAPI2_SIM_SendPbkInfoReq_Rsp_t   CAPI2_SIM_SendPbkInfoReq_Rsp;
	CAPI2_SIM_SendReadAcmMaxReq_Rsp_t   CAPI2_SIM_SendReadAcmMaxReq_Rsp;
	CAPI2_SIM_SendWriteAcmMaxReq_Req_t   CAPI2_SIM_SendWriteAcmMaxReq_Req;
	CAPI2_SIM_SendWriteAcmMaxReq_Rsp_t   CAPI2_SIM_SendWriteAcmMaxReq_Rsp;
	CAPI2_SIM_SendReadAcmReq_Rsp_t   CAPI2_SIM_SendReadAcmReq_Rsp;
	CAPI2_SIM_SendWriteAcmReq_Req_t   CAPI2_SIM_SendWriteAcmReq_Req;
	CAPI2_SIM_SendWriteAcmReq_Rsp_t   CAPI2_SIM_SendWriteAcmReq_Rsp;
	CAPI2_SIM_SendIncreaseAcmReq_Req_t   CAPI2_SIM_SendIncreaseAcmReq_Req;
	CAPI2_SIM_SendIncreaseAcmReq_Rsp_t   CAPI2_SIM_SendIncreaseAcmReq_Rsp;
	CAPI2_SIM_SendReadSvcProvNameReq_Rsp_t   CAPI2_SIM_SendReadSvcProvNameReq_Rsp;
	CAPI2_SIM_SendReadPuctReq_Rsp_t   CAPI2_SIM_SendReadPuctReq_Rsp;
	CAPI2_SIM_GetServiceStatus_Req_t   CAPI2_SIM_GetServiceStatus_Req;
	CAPI2_SIM_GetServiceStatus_Rsp_t   CAPI2_SIM_GetServiceStatus_Rsp;
	CAPI2_SIM_GetPinStatus_Rsp_t   CAPI2_SIM_GetPinStatus_Rsp;
	CAPI2_SIM_IsPinOK_Rsp_t   CAPI2_SIM_IsPinOK_Rsp;
	CAPI2_SIM_GetIMSI_Rsp_t   CAPI2_SIM_GetIMSI_Rsp;
	CAPI2_SIM_GetGID1_Rsp_t   CAPI2_SIM_GetGID1_Rsp;
	CAPI2_SIM_GetGID2_Rsp_t   CAPI2_SIM_GetGID2_Rsp;
	CAPI2_SIM_GetHomePlmn_Rsp_t   CAPI2_SIM_GetHomePlmn_Rsp;
	CAPI2_SIM_GetCurrLockedSimlockType_Rsp_t   CAPI2_SIM_GetCurrLockedSimlockType_Rsp;
	CAPI2_simmi_GetMasterFileId_Req_t   CAPI2_simmi_GetMasterFileId_Req;
	CAPI2_simmi_GetMasterFileId_Rsp_t   CAPI2_simmi_GetMasterFileId_Rsp;
	CAPI2_SIM_SendOpenSocketReq_Rsp_t   CAPI2_SIM_SendOpenSocketReq_Rsp;
	CAPI2_SIM_SendSelectAppiReq_Rsp_t   CAPI2_SIM_SendSelectAppiReq_Rsp;
	CAPI2_SIM_SendDeactivateAppiReq_Rsp_t   CAPI2_SIM_SendDeactivateAppiReq_Rsp;
	CAPI2_SIM_SendCloseSocketReq_Rsp_t   CAPI2_SIM_SendCloseSocketReq_Rsp;
	CAPI2_SIM_GetAtrData_Rsp_t   CAPI2_SIM_GetAtrData_Rsp;
	CAPI2_SIM_SubmitDFileInfoReq_Rsp_t   CAPI2_SIM_SubmitDFileInfoReq_Rsp;
	CAPI2_SIM_SubmitEFileInfoReq_Rsp_t   CAPI2_SIM_SubmitEFileInfoReq_Rsp;
	CAPI2_SIM_SubmitWholeBinaryEFileReadReq_Rsp_t   CAPI2_SIM_SubmitWholeBinaryEFileReadReq_Rsp;
	CAPI2_SIM_SubmitBinaryEFileReadReq_Rsp_t   CAPI2_SIM_SubmitBinaryEFileReadReq_Rsp;
	CAPI2_SIM_SubmitRecordEFileReadReq_Rsp_t   CAPI2_SIM_SubmitRecordEFileReadReq_Rsp;
	CAPI2_SIM_SubmitBinaryEFileUpdateReq_Rsp_t   CAPI2_SIM_SubmitBinaryEFileUpdateReq_Rsp;
	CAPI2_SIM_SubmitLinearEFileUpdateReq_Rsp_t   CAPI2_SIM_SubmitLinearEFileUpdateReq_Rsp;
	CAPI2_SIM_SubmitSeekRecordReq_Rsp_t   CAPI2_SIM_SubmitSeekRecordReq_Rsp;
	CAPI2_SIM_SubmitCyclicEFileUpdateReq_Rsp_t   CAPI2_SIM_SubmitCyclicEFileUpdateReq_Rsp;
	CAPI2_SIM_SendRemainingPinAttemptReq_Rsp_t   CAPI2_SIM_SendRemainingPinAttemptReq_Rsp;
	CAPI2_SIM_IsCachedDataReady_Rsp_t   CAPI2_SIM_IsCachedDataReady_Rsp;
	CAPI2_SIM_GetServiceCodeStatus_Req_t   CAPI2_SIM_GetServiceCodeStatus_Req;
	CAPI2_SIM_GetServiceCodeStatus_Rsp_t   CAPI2_SIM_GetServiceCodeStatus_Rsp;
	CAPI2_SIM_CheckCphsService_Req_t   CAPI2_SIM_CheckCphsService_Req;
	CAPI2_SIM_CheckCphsService_Rsp_t   CAPI2_SIM_CheckCphsService_Rsp;
	CAPI2_SIM_GetCphsPhase_Rsp_t   CAPI2_SIM_GetCphsPhase_Rsp;
	CAPI2_SIM_GetSmsSca_Req_t   CAPI2_SIM_GetSmsSca_Req;
	CAPI2_SIM_GetSmsSca_Rsp_t   CAPI2_SIM_GetSmsSca_Rsp;
	CAPI2_SIM_GetIccid_Rsp_t   CAPI2_SIM_GetIccid_Rsp;
	CAPI2_SIM_IsALSEnabled_Rsp_t   CAPI2_SIM_IsALSEnabled_Rsp;
	CAPI2_SIM_GetAlsDefaultLine_Rsp_t   CAPI2_SIM_GetAlsDefaultLine_Rsp;
	CAPI2_SIM_SetAlsDefaultLine_Req_t   CAPI2_SIM_SetAlsDefaultLine_Req;
	CAPI2_SIM_GetCallForwardUnconditionalFlag_Rsp_t   CAPI2_SIM_GetCallForwardUnconditionalFlag_Rsp;
	CAPI2_SIM_GetApplicationType_Rsp_t   CAPI2_SIM_GetApplicationType_Rsp;
	CAPI2_USIM_GetUst_Rsp_t   CAPI2_USIM_GetUst_Rsp;
	CAPI2_SIM_SendUpdatePrefListReq_Rsp_t   CAPI2_SIM_SendUpdatePrefListReq_Rsp;
	CAPI2_SIM_SendWritePuctReq_Rsp_t   CAPI2_SIM_SendWritePuctReq_Rsp;
	CAPI2_SIM_SendGenericAccessReq_Rsp_t   CAPI2_SIM_SendGenericAccessReq_Rsp;
	CAPI2_SIM_SubmitRestrictedAccessReq_Rsp_t   CAPI2_SIM_SubmitRestrictedAccessReq_Rsp;
	CAPI2_SIM_SendDetectionInd_Rsp_t   CAPI2_SIM_SendDetectionInd_Rsp;
	CAPI2_MS_GsmRegStatusInd_Rsp_t   CAPI2_MS_GsmRegStatusInd_Rsp;
	CAPI2_MS_GprsRegStatusInd_Rsp_t   CAPI2_MS_GprsRegStatusInd_Rsp;
	CAPI2_MS_NetworkNameInd_Rsp_t   CAPI2_MS_NetworkNameInd_Rsp;
	CAPI2_MS_RssiInd_Rsp_t   CAPI2_MS_RssiInd_Rsp;
	CAPI2_MS_SignalChangeInd_Rsp_t   CAPI2_MS_SignalChangeInd_Rsp;
	CAPI2_MS_PlmnListInd_Rsp_t   CAPI2_MS_PlmnListInd_Rsp;
	CAPI2_MS_TimeZoneInd_Rsp_t   CAPI2_MS_TimeZoneInd_Rsp;
	CAPI2_ADCMGR_Start_Req_t   CAPI2_ADCMGR_Start_Req;
	CAPI2_ADCMGR_Start_Rsp_t   CAPI2_ADCMGR_Start_Rsp;
	CAPI2_AT_ProcessCmd_Req_t   CAPI2_AT_ProcessCmd_Req;
	CAPI2_AT_Response_Rsp_t   CAPI2_AT_Response_Rsp;
	CAPI2_MS_GetSystemRAT_Rsp_t   CAPI2_MS_GetSystemRAT_Rsp;
	CAPI2_MS_GetSupportedRAT_Rsp_t   CAPI2_MS_GetSupportedRAT_Rsp;
	CAPI2_MS_GetSystemBand_Rsp_t   CAPI2_MS_GetSystemBand_Rsp;
	CAPI2_MS_GetSupportedBand_Rsp_t   CAPI2_MS_GetSupportedBand_Rsp;
	CAPI2_SYSPARM_GetMSClass_Rsp_t   CAPI2_SYSPARM_GetMSClass_Rsp;
	CAPI2_AUDIO_GetMicrophoneGainSetting_Rsp_t   CAPI2_AUDIO_GetMicrophoneGainSetting_Rsp;
	CAPI2_AUDIO_GetSpeakerVol_Rsp_t   CAPI2_AUDIO_GetSpeakerVol_Rsp;
	CAPI2_AUDIO_SetSpeakerVol_Req_t   CAPI2_AUDIO_SetSpeakerVol_Req;
	CAPI2_AUDIO_SetMicrophoneGain_Req_t   CAPI2_AUDIO_SetMicrophoneGain_Req;
	CAPI2_SYSPARM_GetManufacturerName_Rsp_t   CAPI2_SYSPARM_GetManufacturerName_Rsp;
	CAPI2_SYSPARM_GetModelName_Rsp_t   CAPI2_SYSPARM_GetModelName_Rsp;
	CAPI2_SYSPARM_GetSWVersion_Rsp_t   CAPI2_SYSPARM_GetSWVersion_Rsp;
	CAPI2_SYSPARM_GetEGPRSMSClass_Rsp_t   CAPI2_SYSPARM_GetEGPRSMSClass_Rsp;
	CAPI2_UTIL_ExtractImei_Rsp_t   CAPI2_UTIL_ExtractImei_Rsp;
	CAPI2_MS_GetRegistrationInfo_Rsp_t   CAPI2_MS_GetRegistrationInfo_Rsp;
	CAPI2_SIM_SendNumOfPLMNEntryReq_Req_t   CAPI2_SIM_SendNumOfPLMNEntryReq_Req;
	CAPI2_SIM_SendNumOfPLMNEntryReq_Rsp_t   CAPI2_SIM_SendNumOfPLMNEntryReq_Rsp;
	CAPI2_SIM_SendReadPLMNEntryReq_Req_t   CAPI2_SIM_SendReadPLMNEntryReq_Req;
	CAPI2_SIM_SendReadPLMNEntryReq_Rsp_t   CAPI2_SIM_SendReadPLMNEntryReq_Rsp;
	CAPI2_SIM_SendWritePLMNEntryReq_Rsp_t   CAPI2_SIM_SendWritePLMNEntryReq_Rsp;
	CAPI2_SIM_SendWriteMulPLMNEntryReq_Rsp_t   CAPI2_SIM_SendWriteMulPLMNEntryReq_Rsp;
	CAPI2_SYS_SetRegisteredEventMask_Req_t   CAPI2_SYS_SetRegisteredEventMask_Req;
	CAPI2_SYS_SetRegisteredEventMask_Rsp_t   CAPI2_SYS_SetRegisteredEventMask_Rsp;
	CAPI2_SYS_SetFilteredEventMask_Req_t   CAPI2_SYS_SetFilteredEventMask_Req;
	CAPI2_SYS_SetFilteredEventMask_Rsp_t   CAPI2_SYS_SetFilteredEventMask_Rsp;
	CAPI2_SYS_SetRssiThreshold_Req_t   CAPI2_SYS_SetRssiThreshold_Req;
	CAPI2_SYS_GetBootLoaderVersion_Rsp_t   CAPI2_SYS_GetBootLoaderVersion_Rsp;
	CAPI2_SYS_GetDSFVersion_Rsp_t   CAPI2_SYS_GetDSFVersion_Rsp;
	CAPI2_SYSPARM_GetChanMode_Rsp_t   CAPI2_SYSPARM_GetChanMode_Rsp;
	CAPI2_SYSPARM_GetClassmark_Rsp_t   CAPI2_SYSPARM_GetClassmark_Rsp;
	CAPI2_SYSPARM_GetIMEI_Rsp_t   CAPI2_SYSPARM_GetIMEI_Rsp;
	CAPI2_SYSPARM_GetSysparmIndPartFileVersion_Rsp_t   CAPI2_SYSPARM_GetSysparmIndPartFileVersion_Rsp;
	CAPI2_SYSPARM_SetDARPCfg_Req_t   CAPI2_SYSPARM_SetDARPCfg_Req;
	CAPI2_SYSPARM_SetEGPRSMSClass_Req_t   CAPI2_SYSPARM_SetEGPRSMSClass_Req;
	CAPI2_SYSPARM_SetGPRSMSClass_Req_t   CAPI2_SYSPARM_SetGPRSMSClass_Req;
	CAPI2_SYS_SetMSPowerOnCause_Req_t   CAPI2_SYS_SetMSPowerOnCause_Req;
	CAPI2_TIMEZONE_GetTZUpdateMode_Rsp_t   CAPI2_TIMEZONE_GetTZUpdateMode_Rsp;
	CAPI2_TIMEZONE_SetTZUpdateMode_Req_t   CAPI2_TIMEZONE_SetTZUpdateMode_Req;
	CAPI2_TIMEZONE_UpdateRTC_Req_t   CAPI2_TIMEZONE_UpdateRTC_Req;
	CAPI2_PMU_IsSIMReady_RSP_Rsp_t   CAPI2_PMU_IsSIMReady_RSP_Rsp;
	CAPI2_PMU_ActivateSIM_Req_t   CAPI2_PMU_ActivateSIM_Req;
	CAPI2_PMU_ActivateSIM_RSP_Rsp_t   CAPI2_PMU_ActivateSIM_RSP_Rsp;
	CAPI2_PMU_DeactivateSIM_RSP_Rsp_t   CAPI2_PMU_DeactivateSIM_RSP_Rsp;
	CAPI2_TestCmds_Req_t   CAPI2_TestCmds_Req;
	CAPI2_SATK_SendPlayToneRes_Req_t   CAPI2_SATK_SendPlayToneRes_Req;
	CAPI2_SATK_SendSetupCallRes_Req_t   CAPI2_SATK_SendSetupCallRes_Req;
	CAPI2_PBK_SetFdnCheck_Req_t   CAPI2_PBK_SetFdnCheck_Req;
	CAPI2_PBK_GetFdnCheck_Rsp_t   CAPI2_PBK_GetFdnCheck_Rsp;
	CAPI2_GPIO_Set_High_64Pin_Req_t   CAPI2_GPIO_Set_High_64Pin_Req;
	CAPI2_GPIO_Set_High_64Pin_RSP_Rsp_t   CAPI2_GPIO_Set_High_64Pin_RSP_Rsp;
	CAPI2_GPIO_Set_Low_64Pin_Req_t   CAPI2_GPIO_Set_Low_64Pin_Req;
	CAPI2_GPIO_Set_Low_64Pin_RSP_Rsp_t   CAPI2_GPIO_Set_Low_64Pin_RSP_Rsp;
	CAPI2_PMU_StartCharging_RSP_Rsp_t   CAPI2_PMU_StartCharging_RSP_Rsp;
	CAPI2_PMU_StopCharging_RSP_Rsp_t   CAPI2_PMU_StopCharging_RSP_Rsp;
	CAPI2_PMU_Battery_Register_Req_t   CAPI2_PMU_Battery_Register_Req;
	CAPI2_PMU_Battery_Register_Rsp_t   CAPI2_PMU_Battery_Register_Rsp;
	CAPI2_BattLevelInd_Rsp_t   CAPI2_BattLevelInd_Rsp;
	CAPI2_SMS_IsMeStorageEnabled_RSP_Rsp_t   CAPI2_SMS_IsMeStorageEnabled_RSP_Rsp;
	CAPI2_SMS_GetMaxMeCapacity_RSP_Rsp_t   CAPI2_SMS_GetMaxMeCapacity_RSP_Rsp;
	CAPI2_SMS_GetNextFreeSlot_RSP_Rsp_t   CAPI2_SMS_GetNextFreeSlot_RSP_Rsp;
	CAPI2_SMS_SetMeSmsStatus_Req_t   CAPI2_SMS_SetMeSmsStatus_Req;
	CAPI2_SMS_SetMeSmsStatus_RSP_Rsp_t   CAPI2_SMS_SetMeSmsStatus_RSP_Rsp;
	CAPI2_SMS_GetMeSmsStatus_Req_t   CAPI2_SMS_GetMeSmsStatus_Req;
	CAPI2_SMS_GetMeSmsStatus_RSP_Rsp_t   CAPI2_SMS_GetMeSmsStatus_RSP_Rsp;
	CAPI2_SMS_StoreSmsToMe_Req_t   CAPI2_SMS_StoreSmsToMe_Req;
	CAPI2_SMS_StoreSmsToMe_RSP_Rsp_t   CAPI2_SMS_StoreSmsToMe_RSP_Rsp;
	CAPI2_SMS_RetrieveSmsFromMe_Req_t   CAPI2_SMS_RetrieveSmsFromMe_Req;
	CAPI2_SMS_RetrieveSmsFromMe_RSP_Rsp_t   CAPI2_SMS_RetrieveSmsFromMe_RSP_Rsp;
	CAPI2_SMS_RemoveSmsFromMe_Req_t   CAPI2_SMS_RemoveSmsFromMe_Req;
	CAPI2_SMS_RemoveSmsFromMe_RSP_Rsp_t   CAPI2_SMS_RemoveSmsFromMe_RSP_Rsp;
	CAPI2_SMS_ConfigureMEStorage_Req_t   CAPI2_SMS_ConfigureMEStorage_Req;
	CAPI2_MS_SetElement_Req_t   CAPI2_MS_SetElement_Req;
	CAPI2_MS_GetElement_Req_t   CAPI2_MS_GetElement_Req;
	CAPI2_MS_GetElement_Rsp_t   CAPI2_MS_GetElement_Rsp;
	CAPI2_USIM_IsApplicationSupported_Req_t   CAPI2_USIM_IsApplicationSupported_Req;
	CAPI2_USIM_IsApplicationSupported_Rsp_t   CAPI2_USIM_IsApplicationSupported_Rsp;
	CAPI2_USIM_IsAllowedAPN_Req_t   CAPI2_USIM_IsAllowedAPN_Req;
	CAPI2_USIM_IsAllowedAPN_Rsp_t   CAPI2_USIM_IsAllowedAPN_Rsp;
	CAPI2_USIM_GetNumOfAPN_Rsp_t   CAPI2_USIM_GetNumOfAPN_Rsp;
	CAPI2_USIM_GetAPNEntry_Req_t   CAPI2_USIM_GetAPNEntry_Req;
	CAPI2_USIM_GetAPNEntry_Rsp_t   CAPI2_USIM_GetAPNEntry_Rsp;
	CAPI2_USIM_IsEstServActivated_Req_t   CAPI2_USIM_IsEstServActivated_Req;
	CAPI2_USIM_IsEstServActivated_Rsp_t   CAPI2_USIM_IsEstServActivated_Rsp;
	CAPI2_USIM_SendSetEstServReq_Req_t   CAPI2_USIM_SendSetEstServReq_Req;
	CAPI2_USIM_SendSetEstServReq_Rsp_t   CAPI2_USIM_SendSetEstServReq_Rsp;
	CAPI2_USIM_SendWriteAPNReq_Req_t   CAPI2_USIM_SendWriteAPNReq_Req;
	CAPI2_USIM_SendWriteAPNReq_Rsp_t   CAPI2_USIM_SendWriteAPNReq_Rsp;
	CAPI2_USIM_SendDeleteAllAPNReq_Rsp_t   CAPI2_USIM_SendDeleteAllAPNReq_Rsp;
	CAPI2_USIM_GetRatModeSetting_Rsp_t   CAPI2_USIM_GetRatModeSetting_Rsp;
	CAPI2_PMU_ClientPowerDown_RSP_Rsp_t   CAPI2_PMU_ClientPowerDown_RSP_Rsp;
	CAPI2_PMU_GetPowerupCause_RSP_Rsp_t   CAPI2_PMU_GetPowerupCause_RSP_Rsp;
	CAPI2_MS_GetGPRSRegState_Rsp_t   CAPI2_MS_GetGPRSRegState_Rsp;
	CAPI2_MS_GetGSMRegState_Rsp_t   CAPI2_MS_GetGSMRegState_Rsp;
	CAPI2_MS_GetRegisteredCellInfo_Rsp_t   CAPI2_MS_GetRegisteredCellInfo_Rsp;
	CAPI2_MS_GetStartBand_Rsp_t   CAPI2_MS_GetStartBand_Rsp;
	CAPI2_MS_SetMEPowerClass_Req_t   CAPI2_MS_SetMEPowerClass_Req;
	CAPI2_USIM_GetServiceStatus_Req_t   CAPI2_USIM_GetServiceStatus_Req;
	CAPI2_USIM_GetServiceStatus_Rsp_t   CAPI2_USIM_GetServiceStatus_Rsp;
	CAPI2_SIM_IsAllowedAPN_Req_t   CAPI2_SIM_IsAllowedAPN_Req;
	CAPI2_SIM_IsAllowedAPN_Rsp_t   CAPI2_SIM_IsAllowedAPN_Rsp;
	CAPI2_SMS_GetSmsMaxCapacity_Req_t   CAPI2_SMS_GetSmsMaxCapacity_Req;
	CAPI2_SMS_GetSmsMaxCapacity_Rsp_t   CAPI2_SMS_GetSmsMaxCapacity_Rsp;
	CAPI2_SMS_RetrieveMaxCBChnlLength_Rsp_t   CAPI2_SMS_RetrieveMaxCBChnlLength_Rsp;
	CAPI2_SIM_IsBdnOperationRestricted_Rsp_t   CAPI2_SIM_IsBdnOperationRestricted_Rsp;
	CAPI2_SIM_SendPreferredPlmnUpdateInd_Req_t   CAPI2_SIM_SendPreferredPlmnUpdateInd_Req;
	CAPI2_SIM_SendSetBdnReq_Req_t   CAPI2_SIM_SendSetBdnReq_Req;
	CAPI2_SIM_SendSetBdnReq_Rsp_t   CAPI2_SIM_SendSetBdnReq_Rsp;
	CAPI2_SIM_PowerOnOffCard_Req_t   CAPI2_SIM_PowerOnOffCard_Req;
	CAPI2_SIM_PowerOnOffCard_Rsp_t   CAPI2_SIM_PowerOnOffCard_Rsp;
	CAPI2_SIM_GetRawAtr_Rsp_t   CAPI2_SIM_GetRawAtr_Rsp;
	CAPI2_SIM_Set_Protocol_Req_t   CAPI2_SIM_Set_Protocol_Req;
	CAPI2_SIM_Set_Protocol_Rsp_t   CAPI2_SIM_Set_Protocol_Rsp;
	CAPI2_SIM_Get_Protocol_Rsp_t   CAPI2_SIM_Get_Protocol_Rsp;
	CAPI2_SIM_SendGenericApduCmd_Rsp_t   CAPI2_SIM_SendGenericApduCmd_Rsp;
	CAPI2_SIM_TerminateXferApdu_Rsp_t   CAPI2_SIM_TerminateXferApdu_Rsp;
	CAPI2_MS_SetPlmnSelectRat_Req_t   CAPI2_MS_SetPlmnSelectRat_Req;
	CAPI2_MS_IsDeRegisterInProgress_Rsp_t   CAPI2_MS_IsDeRegisterInProgress_Rsp;
	CAPI2_MS_IsRegisterInProgress_Rsp_t   CAPI2_MS_IsRegisterInProgress_Rsp;
	CAPI2_SOCKET_Open_Req_t   CAPI2_SOCKET_Open_Req;
	CAPI2_SOCKET_Open_RSP_Rsp_t   CAPI2_SOCKET_Open_RSP_Rsp;
	CAPI2_SOCKET_Bind_Req_t   CAPI2_SOCKET_Bind_Req;
	CAPI2_SOCKET_Bind_RSP_Rsp_t   CAPI2_SOCKET_Bind_RSP_Rsp;
	CAPI2_SOCKET_Listen_Req_t   CAPI2_SOCKET_Listen_Req;
	CAPI2_SOCKET_Listen_RSP_Rsp_t   CAPI2_SOCKET_Listen_RSP_Rsp;
	CAPI2_SOCKET_Accept_Req_t   CAPI2_SOCKET_Accept_Req;
	CAPI2_SOCKET_Accept_RSP_Rsp_t   CAPI2_SOCKET_Accept_RSP_Rsp;
	CAPI2_SOCKET_Connect_Req_t   CAPI2_SOCKET_Connect_Req;
	CAPI2_SOCKET_Connect_RSP_Rsp_t   CAPI2_SOCKET_Connect_RSP_Rsp;
	CAPI2_SOCKET_GetPeerName_Req_t   CAPI2_SOCKET_GetPeerName_Req;
	CAPI2_SOCKET_GetPeerName_RSP_Rsp_t   CAPI2_SOCKET_GetPeerName_RSP_Rsp;
	CAPI2_SOCKET_GetSockName_Req_t   CAPI2_SOCKET_GetSockName_Req;
	CAPI2_SOCKET_GetSockName_RSP_Rsp_t   CAPI2_SOCKET_GetSockName_RSP_Rsp;
	CAPI2_SOCKET_SetSockOpt_Req_t   CAPI2_SOCKET_SetSockOpt_Req;
	CAPI2_SOCKET_SetSockOpt_RSP_Rsp_t   CAPI2_SOCKET_SetSockOpt_RSP_Rsp;
	CAPI2_SOCKET_GetSockOpt_Req_t   CAPI2_SOCKET_GetSockOpt_Req;
	CAPI2_SOCKET_GetSockOpt_RSP_Rsp_t   CAPI2_SOCKET_GetSockOpt_RSP_Rsp;
	CAPI2_SOCKET_SignalInd_Req_t   CAPI2_SOCKET_SignalInd_Req;
	CAPI2_SOCKET_Send_Req_t   CAPI2_SOCKET_Send_Req;
	CAPI2_SOCKET_Send_RSP_Rsp_t   CAPI2_SOCKET_Send_RSP_Rsp;
	CAPI2_SOCKET_SendTo_Req_t   CAPI2_SOCKET_SendTo_Req;
	CAPI2_SOCKET_SendTo_RSP_Rsp_t   CAPI2_SOCKET_SendTo_RSP_Rsp;
	CAPI2_SOCKET_Recv_Req_t   CAPI2_SOCKET_Recv_Req;
	CAPI2_SOCKET_Recv_RSP_Rsp_t   CAPI2_SOCKET_Recv_RSP_Rsp;
	CAPI2_SOCKET_RecvFrom_Req_t   CAPI2_SOCKET_RecvFrom_Req;
	CAPI2_SOCKET_RecvFrom_RSP_Rsp_t   CAPI2_SOCKET_RecvFrom_RSP_Rsp;
	CAPI2_SOCKET_Close_Req_t   CAPI2_SOCKET_Close_Req;
	CAPI2_SOCKET_Close_RSP_Rsp_t   CAPI2_SOCKET_Close_RSP_Rsp;
	CAPI2_SOCKET_Shutdown_Req_t   CAPI2_SOCKET_Shutdown_Req;
	CAPI2_SOCKET_Shutdown_RSP_Rsp_t   CAPI2_SOCKET_Shutdown_RSP_Rsp;
	CAPI2_SOCKET_Errno_Req_t   CAPI2_SOCKET_Errno_Req;
	CAPI2_SOCKET_Errno_RSP_Rsp_t   CAPI2_SOCKET_Errno_RSP_Rsp;
	CAPI2_SOCKET_SO2LONG_Req_t   CAPI2_SOCKET_SO2LONG_Req;
	CAPI2_SOCKET_SO2LONG_RSP_Rsp_t   CAPI2_SOCKET_SO2LONG_RSP_Rsp;
	CAPI2_SOCKET_GetSocketSendBufferSpace_Req_t   CAPI2_SOCKET_GetSocketSendBufferSpace_Req;
	CAPI2_SOCKET_GetSocketSendBufferSpace_RSP_Rsp_t   CAPI2_SOCKET_GetSocketSendBufferSpace_RSP_Rsp;
	CAPI2_SOCKET_ParseIPAddr_Req_t   CAPI2_SOCKET_ParseIPAddr_Req;
	CAPI2_SOCKET_ParseIPAddr_RSP_Rsp_t   CAPI2_SOCKET_ParseIPAddr_RSP_Rsp;
	CAPI2_DC_SetupDataConnection_Req_t   CAPI2_DC_SetupDataConnection_Req;
	CAPI2_DC_SetupDataConnection_RSP_Rsp_t   CAPI2_DC_SetupDataConnection_RSP_Rsp;
	CAPI2_DC_SetupDataConnectionEx_Req_t   CAPI2_DC_SetupDataConnectionEx_Req;
	CAPI2_DC_SetupDataConnectionEx_RSP_Rsp_t   CAPI2_DC_SetupDataConnectionEx_RSP_Rsp;
	CAPI2_DC_ReportCallStatusInd_Req_t   CAPI2_DC_ReportCallStatusInd_Req;
	CAPI2_DC_ShutdownDataConnection_Req_t   CAPI2_DC_ShutdownDataConnection_Req;
	CAPI2_DC_ShutdownDataConnection_RSP_Rsp_t   CAPI2_DC_ShutdownDataConnection_RSP_Rsp;
	CAPI2_DATA_IsAcctIDValid_Req_t   CAPI2_DATA_IsAcctIDValid_Req;
	CAPI2_DATA_IsAcctIDValid_RSP_Rsp_t   CAPI2_DATA_IsAcctIDValid_RSP_Rsp;
	CAPI2_DATA_CreateGPRSDataAcct_Req_t   CAPI2_DATA_CreateGPRSDataAcct_Req;
	CAPI2_DATA_CreateGPRSDataAcct_RSP_Rsp_t   CAPI2_DATA_CreateGPRSDataAcct_RSP_Rsp;
	CAPI2_DATA_CreateCSDDataAcct_Req_t   CAPI2_DATA_CreateCSDDataAcct_Req;
	CAPI2_DATA_CreateCSDDataAcct_RSP_Rsp_t   CAPI2_DATA_CreateCSDDataAcct_RSP_Rsp;
	CAPI2_DATA_DeleteDataAcct_Req_t   CAPI2_DATA_DeleteDataAcct_Req;
	CAPI2_DATA_DeleteDataAcct_RSP_Rsp_t   CAPI2_DATA_DeleteDataAcct_RSP_Rsp;
	CAPI2_DATA_SetUsername_Req_t   CAPI2_DATA_SetUsername_Req;
	CAPI2_DATA_SetUsername_RSP_Rsp_t   CAPI2_DATA_SetUsername_RSP_Rsp;
	CAPI2_DATA_GetUsername_Req_t   CAPI2_DATA_GetUsername_Req;
	CAPI2_DATA_GetUsername_RSP_Rsp_t   CAPI2_DATA_GetUsername_RSP_Rsp;
	CAPI2_DATA_SetPassword_Req_t   CAPI2_DATA_SetPassword_Req;
	CAPI2_DATA_SetPassword_RSP_Rsp_t   CAPI2_DATA_SetPassword_RSP_Rsp;
	CAPI2_DATA_GetPassword_Req_t   CAPI2_DATA_GetPassword_Req;
	CAPI2_DATA_GetPassword_RSP_Rsp_t   CAPI2_DATA_GetPassword_RSP_Rsp;
	CAPI2_DATA_SetStaticIPAddr_Req_t   CAPI2_DATA_SetStaticIPAddr_Req;
	CAPI2_DATA_SetStaticIPAddr_RSP_Rsp_t   CAPI2_DATA_SetStaticIPAddr_RSP_Rsp;
	CAPI2_DATA_GetStaticIPAddr_Req_t   CAPI2_DATA_GetStaticIPAddr_Req;
	CAPI2_DATA_GetStaticIPAddr_RSP_Rsp_t   CAPI2_DATA_GetStaticIPAddr_RSP_Rsp;
	CAPI2_DATA_SetPrimaryDnsAddr_Req_t   CAPI2_DATA_SetPrimaryDnsAddr_Req;
	CAPI2_DATA_SetPrimaryDnsAddr_RSP_Rsp_t   CAPI2_DATA_SetPrimaryDnsAddr_RSP_Rsp;
	CAPI2_DATA_GetPrimaryDnsAddr_Req_t   CAPI2_DATA_GetPrimaryDnsAddr_Req;
	CAPI2_DATA_GetPrimaryDnsAddr_RSP_Rsp_t   CAPI2_DATA_GetPrimaryDnsAddr_RSP_Rsp;
	CAPI2_DATA_SetSecondDnsAddr_Req_t   CAPI2_DATA_SetSecondDnsAddr_Req;
	CAPI2_DATA_SetSecondDnsAddr_RSP_Rsp_t   CAPI2_DATA_SetSecondDnsAddr_RSP_Rsp;
	CAPI2_DATA_GetSecondDnsAddr_Req_t   CAPI2_DATA_GetSecondDnsAddr_Req;
	CAPI2_DATA_GetSecondDnsAddr_RSP_Rsp_t   CAPI2_DATA_GetSecondDnsAddr_RSP_Rsp;
	CAPI2_DATA_SetDataCompression_Req_t   CAPI2_DATA_SetDataCompression_Req;
	CAPI2_DATA_SetDataCompression_RSP_Rsp_t   CAPI2_DATA_SetDataCompression_RSP_Rsp;
	CAPI2_DATA_GetDataCompression_Req_t   CAPI2_DATA_GetDataCompression_Req;
	CAPI2_DATA_GetDataCompression_RSP_Rsp_t   CAPI2_DATA_GetDataCompression_RSP_Rsp;
	CAPI2_DATA_GetAcctType_Req_t   CAPI2_DATA_GetAcctType_Req;
	CAPI2_DATA_GetAcctType_RSP_Rsp_t   CAPI2_DATA_GetAcctType_RSP_Rsp;
	CAPI2_DATA_GetEmptyAcctSlot_RSP_Rsp_t   CAPI2_DATA_GetEmptyAcctSlot_RSP_Rsp;
	CAPI2_DATA_GetCidFromDataAcctID_Req_t   CAPI2_DATA_GetCidFromDataAcctID_Req;
	CAPI2_DATA_GetCidFromDataAcctID_RSP_Rsp_t   CAPI2_DATA_GetCidFromDataAcctID_RSP_Rsp;
	CAPI2_DATA_GetDataAcctIDFromCid_Req_t   CAPI2_DATA_GetDataAcctIDFromCid_Req;
	CAPI2_DATA_GetDataAcctIDFromCid_RSP_Rsp_t   CAPI2_DATA_GetDataAcctIDFromCid_RSP_Rsp;
	CAPI2_DATA_GetPrimaryCidFromDataAcctID_Req_t   CAPI2_DATA_GetPrimaryCidFromDataAcctID_Req;
	CAPI2_DATA_GetPrimaryCidFromDataAcctID_RSP_Rsp_t   CAPI2_DATA_GetPrimaryCidFromDataAcctID_RSP_Rsp;
	CAPI2_DATA_IsSecondaryDataAcct_Req_t   CAPI2_DATA_IsSecondaryDataAcct_Req;
	CAPI2_DATA_IsSecondaryDataAcct_RSP_Rsp_t   CAPI2_DATA_IsSecondaryDataAcct_RSP_Rsp;
	CAPI2_DATA_GetDataSentSize_Req_t   CAPI2_DATA_GetDataSentSize_Req;
	CAPI2_DATA_GetDataSentSize_RSP_Rsp_t   CAPI2_DATA_GetDataSentSize_RSP_Rsp;
	CAPI2_DATA_GetDataRcvSize_Req_t   CAPI2_DATA_GetDataRcvSize_Req;
	CAPI2_DATA_GetDataRcvSize_RSP_Rsp_t   CAPI2_DATA_GetDataRcvSize_RSP_Rsp;
	CAPI2_DATA_SetGPRSPdpType_Req_t   CAPI2_DATA_SetGPRSPdpType_Req;
	CAPI2_DATA_SetGPRSPdpType_RSP_Rsp_t   CAPI2_DATA_SetGPRSPdpType_RSP_Rsp;
	CAPI2_DATA_GetGPRSPdpType_Req_t   CAPI2_DATA_GetGPRSPdpType_Req;
	CAPI2_DATA_GetGPRSPdpType_RSP_Rsp_t   CAPI2_DATA_GetGPRSPdpType_RSP_Rsp;
	CAPI2_DATA_SetGPRSApn_Req_t   CAPI2_DATA_SetGPRSApn_Req;
	CAPI2_DATA_SetGPRSApn_RSP_Rsp_t   CAPI2_DATA_SetGPRSApn_RSP_Rsp;
	CAPI2_DATA_GetGPRSApn_Req_t   CAPI2_DATA_GetGPRSApn_Req;
	CAPI2_DATA_GetGPRSApn_RSP_Rsp_t   CAPI2_DATA_GetGPRSApn_RSP_Rsp;
	CAPI2_DATA_SetAuthenMethod_Req_t   CAPI2_DATA_SetAuthenMethod_Req;
	CAPI2_DATA_SetAuthenMethod_RSP_Rsp_t   CAPI2_DATA_SetAuthenMethod_RSP_Rsp;
	CAPI2_DATA_GetAuthenMethod_Req_t   CAPI2_DATA_GetAuthenMethod_Req;
	CAPI2_DATA_GetAuthenMethod_RSP_Rsp_t   CAPI2_DATA_GetAuthenMethod_RSP_Rsp;
	CAPI2_DATA_SetGPRSHeaderCompression_Req_t   CAPI2_DATA_SetGPRSHeaderCompression_Req;
	CAPI2_DATA_SetGPRSHeaderCompression_RSP_Rsp_t   CAPI2_DATA_SetGPRSHeaderCompression_RSP_Rsp;
	CAPI2_DATA_GetGPRSHeaderCompression_Req_t   CAPI2_DATA_GetGPRSHeaderCompression_Req;
	CAPI2_DATA_GetGPRSHeaderCompression_RSP_Rsp_t   CAPI2_DATA_GetGPRSHeaderCompression_RSP_Rsp;
	CAPI2_DATA_SetGPRSQos_Req_t   CAPI2_DATA_SetGPRSQos_Req;
	CAPI2_DATA_SetGPRSQos_RSP_Rsp_t   CAPI2_DATA_SetGPRSQos_RSP_Rsp;
	CAPI2_DATA_GetGPRSQos_Req_t   CAPI2_DATA_GetGPRSQos_Req;
	CAPI2_DATA_GetGPRSQos_RSP_Rsp_t   CAPI2_DATA_GetGPRSQos_RSP_Rsp;
	CAPI2_DATA_SetAcctLock_Req_t   CAPI2_DATA_SetAcctLock_Req;
	CAPI2_DATA_SetAcctLock_RSP_Rsp_t   CAPI2_DATA_SetAcctLock_RSP_Rsp;
	CAPI2_DATA_GetAcctLock_Req_t   CAPI2_DATA_GetAcctLock_Req;
	CAPI2_DATA_GetAcctLock_RSP_Rsp_t   CAPI2_DATA_GetAcctLock_RSP_Rsp;
	CAPI2_DATA_SetGprsOnly_Req_t   CAPI2_DATA_SetGprsOnly_Req;
	CAPI2_DATA_SetGprsOnly_RSP_Rsp_t   CAPI2_DATA_SetGprsOnly_RSP_Rsp;
	CAPI2_DATA_GetGprsOnly_Req_t   CAPI2_DATA_GetGprsOnly_Req;
	CAPI2_DATA_GetGprsOnly_RSP_Rsp_t   CAPI2_DATA_GetGprsOnly_RSP_Rsp;
	CAPI2_DATA_SetGPRSTft_Req_t   CAPI2_DATA_SetGPRSTft_Req;
	CAPI2_DATA_SetGPRSTft_RSP_Rsp_t   CAPI2_DATA_SetGPRSTft_RSP_Rsp;
	CAPI2_DATA_GetGPRSTft_Req_t   CAPI2_DATA_GetGPRSTft_Req;
	CAPI2_DATA_GetGPRSTft_RSP_Rsp_t   CAPI2_DATA_GetGPRSTft_RSP_Rsp;
	CAPI2_DATA_SetCSDDialNumber_Req_t   CAPI2_DATA_SetCSDDialNumber_Req;
	CAPI2_DATA_SetCSDDialNumber_RSP_Rsp_t   CAPI2_DATA_SetCSDDialNumber_RSP_Rsp;
	CAPI2_DATA_GetCSDDialNumber_Req_t   CAPI2_DATA_GetCSDDialNumber_Req;
	CAPI2_DATA_GetCSDDialNumber_RSP_Rsp_t   CAPI2_DATA_GetCSDDialNumber_RSP_Rsp;
	CAPI2_DATA_SetCSDDialType_Req_t   CAPI2_DATA_SetCSDDialType_Req;
	CAPI2_DATA_SetCSDDialType_RSP_Rsp_t   CAPI2_DATA_SetCSDDialType_RSP_Rsp;
	CAPI2_DATA_GetCSDDialType_Req_t   CAPI2_DATA_GetCSDDialType_Req;
	CAPI2_DATA_GetCSDDialType_RSP_Rsp_t   CAPI2_DATA_GetCSDDialType_RSP_Rsp;
	CAPI2_DATA_SetCSDBaudRate_Req_t   CAPI2_DATA_SetCSDBaudRate_Req;
	CAPI2_DATA_SetCSDBaudRate_RSP_Rsp_t   CAPI2_DATA_SetCSDBaudRate_RSP_Rsp;
	CAPI2_DATA_GetCSDBaudRate_Req_t   CAPI2_DATA_GetCSDBaudRate_Req;
	CAPI2_DATA_GetCSDBaudRate_RSP_Rsp_t   CAPI2_DATA_GetCSDBaudRate_RSP_Rsp;
	CAPI2_DATA_SetCSDSyncType_Req_t   CAPI2_DATA_SetCSDSyncType_Req;
	CAPI2_DATA_SetCSDSyncType_RSP_Rsp_t   CAPI2_DATA_SetCSDSyncType_RSP_Rsp;
	CAPI2_DATA_GetCSDSyncType_Req_t   CAPI2_DATA_GetCSDSyncType_Req;
	CAPI2_DATA_GetCSDSyncType_RSP_Rsp_t   CAPI2_DATA_GetCSDSyncType_RSP_Rsp;
	CAPI2_DATA_SetCSDErrorCorrection_Req_t   CAPI2_DATA_SetCSDErrorCorrection_Req;
	CAPI2_DATA_SetCSDErrorCorrection_RSP_Rsp_t   CAPI2_DATA_SetCSDErrorCorrection_RSP_Rsp;
	CAPI2_DATA_GetCSDErrorCorrection_Req_t   CAPI2_DATA_GetCSDErrorCorrection_Req;
	CAPI2_DATA_GetCSDErrorCorrection_RSP_Rsp_t   CAPI2_DATA_GetCSDErrorCorrection_RSP_Rsp;
	CAPI2_DATA_SetCSDErrCorrectionType_Req_t   CAPI2_DATA_SetCSDErrCorrectionType_Req;
	CAPI2_DATA_SetCSDErrCorrectionType_RSP_Rsp_t   CAPI2_DATA_SetCSDErrCorrectionType_RSP_Rsp;
	CAPI2_DATA_GetCSDErrCorrectionType_Req_t   CAPI2_DATA_GetCSDErrCorrectionType_Req;
	CAPI2_DATA_GetCSDErrCorrectionType_RSP_Rsp_t   CAPI2_DATA_GetCSDErrCorrectionType_RSP_Rsp;
	CAPI2_DATA_SetCSDDataCompType_Req_t   CAPI2_DATA_SetCSDDataCompType_Req;
	CAPI2_DATA_SetCSDDataCompType_RSP_Rsp_t   CAPI2_DATA_SetCSDDataCompType_RSP_Rsp;
	CAPI2_DATA_GetCSDDataCompType_Req_t   CAPI2_DATA_GetCSDDataCompType_Req;
	CAPI2_DATA_GetCSDDataCompType_RSP_Rsp_t   CAPI2_DATA_GetCSDDataCompType_RSP_Rsp;
	CAPI2_DATA_SetCSDConnElement_Req_t   CAPI2_DATA_SetCSDConnElement_Req;
	CAPI2_DATA_SetCSDConnElement_RSP_Rsp_t   CAPI2_DATA_SetCSDConnElement_RSP_Rsp;
	CAPI2_DATA_GetCSDConnElement_Req_t   CAPI2_DATA_GetCSDConnElement_Req;
	CAPI2_DATA_GetCSDConnElement_RSP_Rsp_t   CAPI2_DATA_GetCSDConnElement_RSP_Rsp;
	CAPI2_DATA_UpdateAccountToFileSystem_RSP_Rsp_t   CAPI2_DATA_UpdateAccountToFileSystem_RSP_Rsp;
	CAPI2_resetDataSize_Req_t   CAPI2_resetDataSize_Req;
	CAPI2_resetDataSize_RSP_Rsp_t   CAPI2_resetDataSize_RSP_Rsp;
	CAPI2_addDataSentSizebyCid_Req_t   CAPI2_addDataSentSizebyCid_Req;
	CAPI2_addDataSentSizebyCid_RSP_Rsp_t   CAPI2_addDataSentSizebyCid_RSP_Rsp;
	CAPI2_addDataRcvSizebyCid_Req_t   CAPI2_addDataRcvSizebyCid_Req;
	CAPI2_addDataRcvSizebyCid_RSP_Rsp_t   CAPI2_addDataRcvSizebyCid_RSP_Rsp;
	CAPI2_PBK_SendUsimHdkReadReq_Rsp_t   CAPI2_PBK_SendUsimHdkReadReq_Rsp;
	CAPI2_PBK_SendUsimHdkUpdateReq_Req_t   CAPI2_PBK_SendUsimHdkUpdateReq_Req;
	CAPI2_PBK_SendUsimHdkUpdateReq_Rsp_t   CAPI2_PBK_SendUsimHdkUpdateReq_Rsp;
	CAPI2_PBK_SendUsimAasReadReq_Req_t   CAPI2_PBK_SendUsimAasReadReq_Req;
	CAPI2_PBK_SendUsimAasReadReq_Rsp_t   CAPI2_PBK_SendUsimAasReadReq_Rsp;
	CAPI2_PBK_SendUsimAasUpdateReq_Req_t   CAPI2_PBK_SendUsimAasUpdateReq_Req;
	CAPI2_PBK_SendUsimAasUpdateReq_Rsp_t   CAPI2_PBK_SendUsimAasUpdateReq_Rsp;
	CAPI2_PBK_SendUsimGasReadReq_Req_t   CAPI2_PBK_SendUsimGasReadReq_Req;
	CAPI2_PBK_SendUsimGasReadReq_Rsp_t   CAPI2_PBK_SendUsimGasReadReq_Rsp;
	CAPI2_PBK_SendUsimGasUpdateReq_Req_t   CAPI2_PBK_SendUsimGasUpdateReq_Req;
	CAPI2_PBK_SendUsimGasUpdateReq_Rsp_t   CAPI2_PBK_SendUsimGasUpdateReq_Rsp;
	CAPI2_PBK_SendUsimAasInfoReq_Req_t   CAPI2_PBK_SendUsimAasInfoReq_Req;
	CAPI2_PBK_SendUsimAasInfoReq_Rsp_t   CAPI2_PBK_SendUsimAasInfoReq_Rsp;
	CAPI2_PBK_SendUsimGasInfoReq_Req_t   CAPI2_PBK_SendUsimGasInfoReq_Req;
	CAPI2_PBK_SendUsimGasInfoReq_Rsp_t   CAPI2_PBK_SendUsimGasInfoReq_Rsp;
	CAPI2_LCS_PosReqResultInd_Rsp_t   CAPI2_LCS_PosReqResultInd_Rsp;
	CAPI2_LCS_PosInfoInd_Rsp_t   CAPI2_LCS_PosInfoInd_Rsp;
	CAPI2_LCS_PositionDetailInd_Rsp_t   CAPI2_LCS_PositionDetailInd_Rsp;
	CAPI2_LCS_NmeaReadyInd_Rsp_t   CAPI2_LCS_NmeaReadyInd_Rsp;
	CAPI2_LCS_ServiceControl_Req_t   CAPI2_LCS_ServiceControl_Req;
	CAPI2_LCS_ServiceControl_Rsp_t   CAPI2_LCS_ServiceControl_Rsp;
	CAPI2_LCS_PowerConfig_Req_t   CAPI2_LCS_PowerConfig_Req;
	CAPI2_LCS_PowerConfig_Rsp_t   CAPI2_LCS_PowerConfig_Rsp;
	CAPI2_LCS_ServiceQuery_Rsp_t   CAPI2_LCS_ServiceQuery_Rsp;
	CAPI2_LCS_StartPosReqPeriodic_Rsp_t   CAPI2_LCS_StartPosReqPeriodic_Rsp;
	CAPI2_LCS_StartPosReqSingle_Rsp_t   CAPI2_LCS_StartPosReqSingle_Rsp;
	CAPI2_LCS_StopPosReq_Req_t   CAPI2_LCS_StopPosReq_Req;
	CAPI2_LCS_StopPosReq_Rsp_t   CAPI2_LCS_StopPosReq_Rsp;
	CAPI2_LCS_GetPosition_Rsp_t   CAPI2_LCS_GetPosition_Rsp;
	CAPI2_LCS_ConfigSet_Req_t   CAPI2_LCS_ConfigSet_Req;
	CAPI2_LCS_ConfigGet_Req_t   CAPI2_LCS_ConfigGet_Req;
	CAPI2_LCS_ConfigGet_Rsp_t   CAPI2_LCS_ConfigGet_Rsp;
	CAPI2_LCS_SuplVerificationRsp_Req_t   CAPI2_LCS_SuplVerificationRsp_Req;
	CAPI2_LCS_SuplConnectRsp_Req_t   CAPI2_LCS_SuplConnectRsp_Req;
	CAPI2_LCS_SuplConnectRsp_Rsp_t   CAPI2_LCS_SuplConnectRsp_Rsp;
	CAPI2_LCS_SuplDisconnected_Req_t   CAPI2_LCS_SuplDisconnected_Req;
	CAPI2_LCS_SuplInitHmacReq_Rsp_t   CAPI2_LCS_SuplInitHmacReq_Rsp;
	CAPI2_LCS_SuplConnectReq_Rsp_t   CAPI2_LCS_SuplConnectReq_Rsp;
	CAPI2_LCS_SuplDisconnectReq_Rsp_t   CAPI2_LCS_SuplDisconnectReq_Rsp;
	CAPI2_LCS_SuplNotificationInd_Rsp_t   CAPI2_LCS_SuplNotificationInd_Rsp;
	CAPI2_LCS_SuplWriteReq_Rsp_t   CAPI2_LCS_SuplWriteReq_Rsp;
	CAPI2_LCS_CmdData_Req_t   CAPI2_LCS_CmdData_Req;
	CAPI2_GPIO_ConfigOutput_64Pin_Req_t   CAPI2_GPIO_ConfigOutput_64Pin_Req;
	CAPI2_GPIO_ConfigOutput_64Pin_RSP_Rsp_t   CAPI2_GPIO_ConfigOutput_64Pin_RSP_Rsp;
	CAPI2_GPS_Control_Req_t   CAPI2_GPS_Control_Req;
	CAPI2_GPS_Control_RSP_Rsp_t   CAPI2_GPS_Control_RSP_Rsp;
	CAPI2_FFS_Control_Req_t   CAPI2_FFS_Control_Req;
	CAPI2_FFS_Control_RSP_Rsp_t   CAPI2_FFS_Control_RSP_Rsp;
	CAPI2_CP2AP_PedestalMode_Control_Req_t   CAPI2_CP2AP_PedestalMode_Control_Req;
	CAPI2_CP2AP_PedestalMode_Control_RSP_Rsp_t   CAPI2_CP2AP_PedestalMode_Control_RSP_Rsp;
	CAPI2_AUDIO_ASIC_SetAudioMode_Req_t   CAPI2_AUDIO_ASIC_SetAudioMode_Req;
	CAPI2_SPEAKER_StartTone_Req_t   CAPI2_SPEAKER_StartTone_Req;
	CAPI2_SPEAKER_StartGenericTone_Req_t   CAPI2_SPEAKER_StartGenericTone_Req;
	CAPI2_AUDIO_Turn_EC_NS_OnOff_Req_t   CAPI2_AUDIO_Turn_EC_NS_OnOff_Req;
	CAPI2_ECHO_SetDigitalTxGain_Req_t   CAPI2_ECHO_SetDigitalTxGain_Req;
	CAPI2_RIPCMDQ_Connect_Uplink_Req_t   CAPI2_RIPCMDQ_Connect_Uplink_Req;
	CAPI2_RIPCMDQ_Connect_Downlink_Req_t   CAPI2_RIPCMDQ_Connect_Downlink_Req;
	CAPI2_VOLUMECTRL_SetBasebandVolume_Req_t   CAPI2_VOLUMECTRL_SetBasebandVolume_Req;
	CAPI2_MS_MeasureReportInd_Rsp_t   CAPI2_MS_MeasureReportInd_Rsp;
	CAPI2_DIAG_ApiMeasurmentReportReq_Req_t   CAPI2_DIAG_ApiMeasurmentReportReq_Req;
	CAPI2_PMU_BattChargingNotification_Req_t   CAPI2_PMU_BattChargingNotification_Req;
	CAPI2_PATCH_GetRevision_Rsp_t   CAPI2_PATCH_GetRevision_Rsp;
	CAPI2_RTC_SetTime_Req_t   CAPI2_RTC_SetTime_Req;
	CAPI2_RTC_SetTime_RSP_Rsp_t   CAPI2_RTC_SetTime_RSP_Rsp;
	CAPI2_RTC_SetDST_Req_t   CAPI2_RTC_SetDST_Req;
	CAPI2_RTC_SetDST_RSP_Rsp_t   CAPI2_RTC_SetDST_RSP_Rsp;
	CAPI2_RTC_SetTimeZone_Req_t   CAPI2_RTC_SetTimeZone_Req;
	CAPI2_RTC_SetTimeZone_RSP_Rsp_t   CAPI2_RTC_SetTimeZone_RSP_Rsp;
	CAPI2_RTC_GetTime_Req_t   CAPI2_RTC_GetTime_Req;
	CAPI2_RTC_GetTime_RSP_Rsp_t   CAPI2_RTC_GetTime_RSP_Rsp;
	CAPI2_RTC_GetTimeZone_RSP_Rsp_t   CAPI2_RTC_GetTimeZone_RSP_Rsp;
	CAPI2_RTC_GetDST_RSP_Rsp_t   CAPI2_RTC_GetDST_RSP_Rsp;
	CAPI2_SMS_GetMeSmsBufferStatus_Req_t   CAPI2_SMS_GetMeSmsBufferStatus_Req;
	CAPI2_SMS_GetMeSmsBufferStatus_RSP_Rsp_t   CAPI2_SMS_GetMeSmsBufferStatus_RSP_Rsp;
	CAPI2_SMS_GetRecordNumberOfReplaceSMS_Req_t   CAPI2_SMS_GetRecordNumberOfReplaceSMS_Req;
	CAPI2_SMS_GetRecordNumberOfReplaceSMS_RSP_Rsp_t   CAPI2_SMS_GetRecordNumberOfReplaceSMS_RSP_Rsp;
	CAPI2_FLASH_SaveImage_Req_t   CAPI2_FLASH_SaveImage_Req;
	CAPI2_FLASH_SaveImage_RSP_Rsp_t   CAPI2_FLASH_SaveImage_RSP_Rsp;
	CAPI2_AUDIO_GetSettings_Req_t   CAPI2_AUDIO_GetSettings_Req;
	CAPI2_AUDIO_GetSettings_Rsp_t   CAPI2_AUDIO_GetSettings_Rsp;
	CAPI2_SIMLOCK_GetStatus_Req_t   CAPI2_SIMLOCK_GetStatus_Req;
	CAPI2_SIMLOCK_GetStatus_RSP_Rsp_t   CAPI2_SIMLOCK_GetStatus_RSP_Rsp;
	CAPI2_SIMLOCK_SetStatus_Req_t   CAPI2_SIMLOCK_SetStatus_Req;
	CAPI2_SYSPARM_GetActualLowVoltReading_Rsp_t   CAPI2_SYSPARM_GetActualLowVoltReading_Rsp;
	CAPI2_SYSPARM_GetActual4p2VoltReading_Rsp_t   CAPI2_SYSPARM_GetActual4p2VoltReading_Rsp;
	CAPI2_program_equalizer_Req_t   CAPI2_program_equalizer_Req;
	CAPI2_program_poly_equalizer_Req_t   CAPI2_program_poly_equalizer_Req;
	CAPI2_program_FIR_IIR_filter_Req_t   CAPI2_program_FIR_IIR_filter_Req;
	CAPI2_program_poly_FIR_IIR_filter_Req_t   CAPI2_program_poly_FIR_IIR_filter_Req;
	CAPI2_audio_control_generic_Req_t   CAPI2_audio_control_generic_Req;
	CAPI2_audio_control_generic_Rsp_t   CAPI2_audio_control_generic_Rsp;
	CAPI2_audio_control_dsp_Req_t   CAPI2_audio_control_dsp_Req;
	CAPI2_audio_control_dsp_Rsp_t   CAPI2_audio_control_dsp_Rsp;
	CAPI2_MS_Ue3gStatusInd_Rsp_t   CAPI2_MS_Ue3gStatusInd_Rsp;
	CAPI2_MS_CellInfoInd_Rsp_t   CAPI2_MS_CellInfoInd_Rsp;
	CAPI2_FFS_Read_Req_t   CAPI2_FFS_Read_Req;
	CAPI2_FFS_Read_RSP_Rsp_t   CAPI2_FFS_Read_RSP_Rsp;
	CAPI2_DIAG_ApiCellLockReq_Req_t   CAPI2_DIAG_ApiCellLockReq_Req;
	CAPI2_DIAG_ApiCellLockStatus_Rsp_t   CAPI2_DIAG_ApiCellLockStatus_Rsp;
	CAPI2_LCS_RegisterRrlpDataHandler_Req_t   CAPI2_LCS_RegisterRrlpDataHandler_Req;
	CAPI2_LCS_RrlpDataInd_Rsp_t   CAPI2_LCS_RrlpDataInd_Rsp;
	CAPI2_LCS_RrlpReset_Rsp_t   CAPI2_LCS_RrlpReset_Rsp;
	CAPI2_LCS_RegisterRrcDataHandler_Req_t   CAPI2_LCS_RegisterRrcDataHandler_Req;
	CAPI2_LCS_RrcAssistDataInd_Rsp_t   CAPI2_LCS_RrcAssistDataInd_Rsp;
	CAPI2_LCS_RrcMeasCtrlInd_Rsp_t   CAPI2_LCS_RrcMeasCtrlInd_Rsp;
	CAPI2_LCS_RrcSysInfoInd_Rsp_t   CAPI2_LCS_RrcSysInfoInd_Rsp;
	CAPI2_LCS_RrcUeStateInd_Rsp_t   CAPI2_LCS_RrcUeStateInd_Rsp;
	CAPI2_LCS_RrcStopMeasInd_Rsp_t   CAPI2_LCS_RrcStopMeasInd_Rsp;
	CAPI2_LCS_RrcReset_Rsp_t   CAPI2_LCS_RrcReset_Rsp;
	CAPI2_CC_IsThereEmergencyCall_Rsp_t   CAPI2_CC_IsThereEmergencyCall_Rsp;
	CAPI2_SYSPARM_GetBattLowThresh_Rsp_t   CAPI2_SYSPARM_GetBattLowThresh_Rsp;
	CAPI2_SYSPARM_GetDefault4p2VoltReading_Rsp_t   CAPI2_SYSPARM_GetDefault4p2VoltReading_Rsp;
	CAPI2_L1_bb_isLocked_Req_t   CAPI2_L1_bb_isLocked_Req;
	CAPI2_L1_bb_isLocked_Rsp_t   CAPI2_L1_bb_isLocked_Rsp;
	CAPI2_ServingCellInfoInd_Rsp_t   CAPI2_ServingCellInfoInd_Rsp;
	CAPI2_SMSPP_AppSpecificInd_Rsp_t   CAPI2_SMSPP_AppSpecificInd_Rsp;
	CAPI2_SYSPARM_GetGPIO_Value_Rsp_t   CAPI2_SYSPARM_GetGPIO_Value_Rsp;
	CAPI2_SYS_EnableCellInfoMsg_Req_t   CAPI2_SYS_EnableCellInfoMsg_Req;
	CAPI2_SYSPARM_SetHSDPAPHYCategory_Req_t   CAPI2_SYSPARM_SetHSDPAPHYCategory_Req;
	CAPI2_SYSPARM_GetHSDPAPHYCategory_Rsp_t   CAPI2_SYSPARM_GetHSDPAPHYCategory_Rsp;
	CAPI2_SIM_SendWriteSmsReq_Req_t   CAPI2_SIM_SendWriteSmsReq_Req;
	CAPI2_SIM_SendWriteSmsReq_Rsp_t   CAPI2_SIM_SendWriteSmsReq_Rsp;
	CAPI2_BATTMGR_GetChargingStatus_Rsp_t   CAPI2_BATTMGR_GetChargingStatus_Rsp;
	CAPI2_BATTMGR_GetPercentageLevel_Rsp_t   CAPI2_BATTMGR_GetPercentageLevel_Rsp;
	CAPI2_BATTMGR_IsBatteryPresent_Rsp_t   CAPI2_BATTMGR_IsBatteryPresent_Rsp;
	CAPI2_BATTMGR_IsChargerPlugIn_Rsp_t   CAPI2_BATTMGR_IsChargerPlugIn_Rsp;
	CAPI2_SYSPARM_GetBattTable_Rsp_t   CAPI2_SYSPARM_GetBattTable_Rsp;
	CAPI2_BATTMGR_GetLevel_Rsp_t   CAPI2_BATTMGR_GetLevel_Rsp;
	CAPI2_AT_ProcessCmdToAP_Req_t   CAPI2_AT_ProcessCmdToAP_Req;
	CAPI2_SMS_WriteSMSPduToSIMRecordReq_Req_t   CAPI2_SMS_WriteSMSPduToSIMRecordReq_Req;
	CAPI2_ADCMGR_MultiChStart_Req_t   CAPI2_ADCMGR_MultiChStart_Req;
	CAPI2_ADCMGR_MultiChStart_Rsp_t   CAPI2_ADCMGR_MultiChStart_Rsp;
	CAPI2_InterTaskMsgToCP_Req_t   CAPI2_InterTaskMsgToCP_Req;
	CAPI2_InterTaskMsgToAP_Req_t   CAPI2_InterTaskMsgToAP_Req;
#endif //DEFINE_XDR_UNION_DECLARE

