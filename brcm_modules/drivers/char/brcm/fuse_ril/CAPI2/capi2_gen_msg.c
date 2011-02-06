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

#include "capi2_gen_common.h"




bool_t xdr_CAPI2_MS_IsGSMRegistered_Rsp_t(void* xdrs, CAPI2_MS_IsGSMRegistered_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_IsGSMRegistered_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_MS_IsGPRSRegistered_Rsp_t(void* xdrs, CAPI2_MS_IsGPRSRegistered_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_IsGPRSRegistered_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_MS_GetGSMRegCause_Rsp_t(void* xdrs, CAPI2_MS_GetGSMRegCause_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetGSMRegCause_Rsp_t")

	 return xdr_NetworkCause_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_MS_GetGPRSRegCause_Rsp_t(void* xdrs, CAPI2_MS_GetGPRSRegCause_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetGPRSRegCause_Rsp_t")

	 return xdr_NetworkCause_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_MS_GetRegisteredLAC_Rsp_t(void* xdrs, CAPI2_MS_GetRegisteredLAC_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetRegisteredLAC_Rsp_t")

	 return _xdr_UInt16(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_MS_GetPlmnMCC_Rsp_t(void* xdrs, CAPI2_MS_GetPlmnMCC_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetPlmnMCC_Rsp_t")

	 return _xdr_UInt16(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_MS_GetPlmnMNC_Rsp_t(void* xdrs, CAPI2_MS_GetPlmnMNC_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetPlmnMNC_Rsp_t")

	 return _xdr_UInt8(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_SYS_GetMSPowerOnCause_Rsp_t(void* xdrs, CAPI2_SYS_GetMSPowerOnCause_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYS_GetMSPowerOnCause_Rsp_t")

	 return xdr_PowerOnCause_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_MS_IsGprsAllowed_Rsp_t(void* xdrs, CAPI2_MS_IsGprsAllowed_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_IsGprsAllowed_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_MS_GetCurrentRAT_Rsp_t(void* xdrs, CAPI2_MS_GetCurrentRAT_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetCurrentRAT_Rsp_t")

	 return _xdr_UInt8(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_MS_GetCurrentBand_Rsp_t(void* xdrs, CAPI2_MS_GetCurrentBand_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetCurrentBand_Rsp_t")

	 return _xdr_UInt8(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_MS_SetStartBand_Req_t(void* xdrs, CAPI2_MS_SetStartBand_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_SetStartBand_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->startBand,"startBand") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIM_UpdateSMSCapExceededFlag_Req_t(void* xdrs, CAPI2_SIM_UpdateSMSCapExceededFlag_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_UpdateSMSCapExceededFlag_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->cap_exceeded,"cap_exceeded") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYS_SelectBand_Req_t(void* xdrs, CAPI2_SYS_SelectBand_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYS_SelectBand_Req_t")

	if(
		xdr_BandSelect_t(xdrs, &rsp->bandSelect) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_SetSupportedRATandBand_Req_t(void* xdrs, CAPI2_MS_SetSupportedRATandBand_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_SetSupportedRATandBand_Req_t")

	if(
		xdr_RATSelect_t(xdrs, &rsp->RAT_cap) &&
		xdr_BandSelect_t(xdrs, &rsp->band_cap) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PLMN_GetCountryByMcc_Req_t(void* xdrs, CAPI2_PLMN_GetCountryByMcc_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PLMN_GetCountryByMcc_Req_t")

	if(
		_xdr_UInt16(xdrs, &rsp->mcc,"mcc") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PLMN_GetCountryByMcc_Rsp_t(void* xdrs, CAPI2_PLMN_GetCountryByMcc_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PLMN_GetCountryByMcc_Rsp_t")

	 return xdr_uchar_ptr_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_MS_GetPLMNEntryByIndex_Req_t(void* xdrs, CAPI2_MS_GetPLMNEntryByIndex_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetPLMNEntryByIndex_Req_t")

	if(
		_xdr_UInt16(xdrs, &rsp->index,"index") &&
		_xdr_Boolean(xdrs, &rsp->ucs2,"ucs2") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_GetPLMNEntryByIndex_Rsp_t(void* xdrs, CAPI2_MS_GetPLMNEntryByIndex_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetPLMNEntryByIndex_Rsp_t")

	 return xdr_MsPlmnInfo_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_MS_GetPLMNListSize_Rsp_t(void* xdrs, CAPI2_MS_GetPLMNListSize_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetPLMNListSize_Rsp_t")

	 return _xdr_UInt16(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_MS_GetPLMNByCode_Req_t(void* xdrs, CAPI2_MS_GetPLMNByCode_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetPLMNByCode_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->ucs2,"ucs2") &&
		_xdr_UInt16(xdrs, &rsp->plmn_mcc,"plmn_mcc") &&
		_xdr_UInt16(xdrs, &rsp->plmn_mnc,"plmn_mnc") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_GetPLMNByCode_Rsp_t(void* xdrs, CAPI2_MS_GetPLMNByCode_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetPLMNByCode_Rsp_t")

	 return xdr_MsPlmnInfo_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_MS_PlmnSelect_Req_t(void* xdrs, CAPI2_MS_PlmnSelect_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_PlmnSelect_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->ucs2,"ucs2") &&
		xdr_PlmnSelectMode_t(xdrs, &rsp->selectMode) &&
		xdr_PlmnSelectFormat_t(xdrs, &rsp->format) &&
		xdr_char_ptr_t(xdrs, &rsp->plmnValue) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_PlmnSelect_Rsp_t(void* xdrs, CAPI2_MS_PlmnSelect_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_PlmnSelect_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( UInt16 ), xdr_UInt16);
}

bool_t xdr_CAPI2_MS_GetPlmnMode_Rsp_t(void* xdrs, CAPI2_MS_GetPlmnMode_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetPlmnMode_Rsp_t")

	 return xdr_PlmnSelectMode_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_MS_SetPlmnMode_Req_t(void* xdrs, CAPI2_MS_SetPlmnMode_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_SetPlmnMode_Req_t")

	if(
		xdr_PlmnSelectMode_t(xdrs, &rsp->mode) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_GetPlmnFormat_Rsp_t(void* xdrs, CAPI2_MS_GetPlmnFormat_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetPlmnFormat_Rsp_t")

	 return xdr_PlmnSelectFormat_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_MS_SetPlmnFormat_Req_t(void* xdrs, CAPI2_MS_SetPlmnFormat_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_SetPlmnFormat_Req_t")

	if(
		xdr_PlmnSelectFormat_t(xdrs, &rsp->format) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_IsMatchedPLMN_Req_t(void* xdrs, CAPI2_MS_IsMatchedPLMN_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_IsMatchedPLMN_Req_t")

	if(
		_xdr_UInt16(xdrs, &rsp->net_mcc,"net_mcc") &&
		_xdr_UInt8(xdrs, &rsp->net_mnc,"net_mnc") &&
		_xdr_UInt16(xdrs, &rsp->mcc,"mcc") &&
		_xdr_UInt8(xdrs, &rsp->mnc,"mnc") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_IsMatchedPLMN_Rsp_t(void* xdrs, CAPI2_MS_IsMatchedPLMN_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_IsMatchedPLMN_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_MS_GetPLMNNameByCode_Req_t(void* xdrs, CAPI2_MS_GetPLMNNameByCode_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetPLMNNameByCode_Req_t")

	if(
		_xdr_UInt16(xdrs, &rsp->mcc,"mcc") &&
		_xdr_UInt8(xdrs, &rsp->mnc,"mnc") &&
		_xdr_UInt16(xdrs, &rsp->lac,"lac") &&
		_xdr_Boolean(xdrs, &rsp->ucs2,"ucs2") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_GetPLMNNameByCode_Rsp_t(void* xdrs, CAPI2_MS_GetPLMNNameByCode_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetPLMNNameByCode_Rsp_t")

	 return xdr_MsPlmnName_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SYS_IsResetCausedByAssert_Rsp_t(void* xdrs, CAPI2_SYS_IsResetCausedByAssert_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYS_IsResetCausedByAssert_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_SYS_GetSystemState_Rsp_t(void* xdrs, CAPI2_SYS_GetSystemState_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYS_GetSystemState_Rsp_t")

	 return xdr_SystemState_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SYS_SetSystemState_Req_t(void* xdrs, CAPI2_SYS_SetSystemState_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYS_SetSystemState_Req_t")

	if(
		xdr_SystemState_t(xdrs, &rsp->state) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYS_GetRxSignalInfo_Rsp_t(void* xdrs, CAPI2_SYS_GetRxSignalInfo_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYS_GetRxSignalInfo_Rsp_t")

	 return xdr_MsRxLevelData_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SYS_GetGSMRegistrationStatus_Rsp_t(void* xdrs, CAPI2_SYS_GetGSMRegistrationStatus_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYS_GetGSMRegistrationStatus_Rsp_t")

	 return xdr_RegisterStatus_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SYS_GetGPRSRegistrationStatus_Rsp_t(void* xdrs, CAPI2_SYS_GetGPRSRegistrationStatus_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYS_GetGPRSRegistrationStatus_Rsp_t")

	 return xdr_RegisterStatus_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SYS_IsRegisteredGSMOrGPRS_Rsp_t(void* xdrs, CAPI2_SYS_IsRegisteredGSMOrGPRS_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYS_IsRegisteredGSMOrGPRS_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_SYS_GetGSMRegistrationCause_Rsp_t(void* xdrs, CAPI2_SYS_GetGSMRegistrationCause_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYS_GetGSMRegistrationCause_Rsp_t")

	 return xdr_PCHRejectCause_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_MS_IsPlmnForbidden_Rsp_t(void* xdrs, CAPI2_MS_IsPlmnForbidden_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_IsPlmnForbidden_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_MS_IsRegisteredHomePLMN_Rsp_t(void* xdrs, CAPI2_MS_IsRegisteredHomePLMN_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_IsRegisteredHomePLMN_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_MS_SetPowerDownTimer_Req_t(void* xdrs, CAPI2_MS_SetPowerDownTimer_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_SetPowerDownTimer_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->powerDownTimer,"powerDownTimer") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIMLockIsLockOn_Req_t(void* xdrs, CAPI2_SIMLockIsLockOn_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIMLockIsLockOn_Req_t")

	if(
		xdr_SIMLockType_t(xdrs, &rsp->lockType) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIMLockIsLockOn_Rsp_t(void* xdrs, CAPI2_SIMLockIsLockOn_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIMLockIsLockOn_Rsp_t")

	 return xdr_SIM_LOCK_ON_RESULT_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIMLockCheckAllLocks_Req_t(void* xdrs, CAPI2_SIMLockCheckAllLocks_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIMLockCheckAllLocks_Req_t")

	if(
		xdr_uchar_ptr_t(xdrs, &rsp->imsi) &&
		xdr_uchar_ptr_t(xdrs, &rsp->gid1) &&
		xdr_uchar_ptr_t(xdrs, &rsp->gid2) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIMLockCheckAllLocks_Rsp_t(void* xdrs, CAPI2_SIMLockCheckAllLocks_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIMLockCheckAllLocks_Rsp_t")

	 return xdr_SIM_BOOLEAN_DATA_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIMLockUnlockSIM_Req_t(void* xdrs, CAPI2_SIMLockUnlockSIM_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIMLockUnlockSIM_Req_t")

	if(
		xdr_SIMLockType_t(xdrs, &rsp->lockType) &&
		xdr_uchar_ptr_t(xdrs, &rsp->key) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIMLockUnlockSIM_Rsp_t(void* xdrs, CAPI2_SIMLockUnlockSIM_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIMLockUnlockSIM_Rsp_t")

	 return xdr_SIM_LOCK_RESULT_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIMLockSetLock_Rsp_t(void* xdrs, CAPI2_SIMLockSetLock_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIMLockSetLock_Rsp_t")

	 return xdr_SIM_LOCK_RESULT_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIMLockGetCurrentClosedLock_Rsp_t(void* xdrs, CAPI2_SIMLockGetCurrentClosedLock_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIMLockGetCurrentClosedLock_Rsp_t")

	 return xdr_SIM_LOCK_TYPE_RESULT_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIMLockChangePasswordPHSIM_Req_t(void* xdrs, CAPI2_SIMLockChangePasswordPHSIM_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIMLockChangePasswordPHSIM_Req_t")

	if(
		xdr_uchar_ptr_t(xdrs, &rsp->old_pwd) &&
		xdr_uchar_ptr_t(xdrs, &rsp->new_pwd) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIMLockChangePasswordPHSIM_Rsp_t(void* xdrs, CAPI2_SIMLockChangePasswordPHSIM_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIMLockChangePasswordPHSIM_Rsp_t")

	 return xdr_SIM_LOCK_RESULT_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIMLockCheckPasswordPHSIM_Req_t(void* xdrs, CAPI2_SIMLockCheckPasswordPHSIM_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIMLockCheckPasswordPHSIM_Req_t")

	if(
		xdr_uchar_ptr_t(xdrs, &rsp->pwd) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIMLockCheckPasswordPHSIM_Rsp_t(void* xdrs, CAPI2_SIMLockCheckPasswordPHSIM_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIMLockCheckPasswordPHSIM_Rsp_t")

	 return xdr_SIM_BOOLEAN_DATA_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIMLockGetSignature_Rsp_t(void* xdrs, CAPI2_SIMLockGetSignature_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIMLockGetSignature_Rsp_t")

	 return xdr_SIM_LOCK_SIG_DATA_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIMLockGetImeiSecboot_Rsp_t(void* xdrs, CAPI2_SIMLockGetImeiSecboot_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIMLockGetImeiSecboot_Rsp_t")

	 return xdr_SIM_LOCK_IMEI_DATA_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_GetSmsParamRecNum_Rsp_t(void* xdrs, CAPI2_SIM_GetSmsParamRecNum_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_GetSmsParamRecNum_Rsp_t")

	 return xdr_SIM_INTEGER_DATA_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_MS_ConvertPLMNNameStr_Rsp_t(void* xdrs, CAPI2_MS_ConvertPLMNNameStr_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_ConvertPLMNNameStr_Rsp_t")

	 return xdr_uchar_ptr_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_GetSmsMemExceededFlag_Rsp_t(void* xdrs, CAPI2_SIM_GetSmsMemExceededFlag_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_GetSmsMemExceededFlag_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_SIM_IsPINRequired_Rsp_t(void* xdrs, CAPI2_SIM_IsPINRequired_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_IsPINRequired_Rsp_t")

	 return xdr_SIM_BOOLEAN_DATA_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_GetCardPhase_Rsp_t(void* xdrs, CAPI2_SIM_GetCardPhase_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_GetCardPhase_Rsp_t")

	 return xdr_SIM_PHASE_RESULT_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_GetSIMType_Rsp_t(void* xdrs, CAPI2_SIM_GetSIMType_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_GetSIMType_Rsp_t")

	 return xdr_SIM_TYPE_RESULT_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_GetPresentStatus_Rsp_t(void* xdrs, CAPI2_SIM_GetPresentStatus_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_GetPresentStatus_Rsp_t")

	 return xdr_SIM_PRESENT_RESULT_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_IsOperationRestricted_Rsp_t(void* xdrs, CAPI2_SIM_IsOperationRestricted_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_IsOperationRestricted_Rsp_t")

	 return xdr_SIM_BOOLEAN_DATA_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_IsPINBlocked_Req_t(void* xdrs, CAPI2_SIM_IsPINBlocked_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_IsPINBlocked_Req_t")

	if(
		xdr_CHV_t(xdrs, &rsp->chv) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIM_IsPINBlocked_Rsp_t(void* xdrs, CAPI2_SIM_IsPINBlocked_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_IsPINBlocked_Rsp_t")

	 return xdr_SIM_BOOLEAN_DATA_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_IsPUKBlocked_Req_t(void* xdrs, CAPI2_SIM_IsPUKBlocked_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_IsPUKBlocked_Req_t")

	if(
		xdr_CHV_t(xdrs, &rsp->chv) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIM_IsPUKBlocked_Rsp_t(void* xdrs, CAPI2_SIM_IsPUKBlocked_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_IsPUKBlocked_Rsp_t")

	 return xdr_SIM_BOOLEAN_DATA_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_IsInvalidSIM_Rsp_t(void* xdrs, CAPI2_SIM_IsInvalidSIM_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_IsInvalidSIM_Rsp_t")

	 return xdr_SIM_BOOLEAN_DATA_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_DetectSim_Rsp_t(void* xdrs, CAPI2_SIM_DetectSim_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_DetectSim_Rsp_t")

	 return xdr_SIM_BOOLEAN_DATA_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_GetRuimSuppFlag_Rsp_t(void* xdrs, CAPI2_SIM_GetRuimSuppFlag_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_GetRuimSuppFlag_Rsp_t")

	 return xdr_SIM_BOOLEAN_DATA_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_SendVerifyChvReq_Req_t(void* xdrs, CAPI2_SIM_SendVerifyChvReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendVerifyChvReq_Req_t")

	if(
		xdr_CHV_t(xdrs, &rsp->chv_select) &&
		xdr_CHVString_t(xdrs, &rsp->chv) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIM_SendVerifyChvReq_Rsp_t(void* xdrs, CAPI2_SIM_SendVerifyChvReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendVerifyChvReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_ACCESS_RESULT_t ), xdr_SIM_ACCESS_RESULT_t);
}

bool_t xdr_CAPI2_SIM_SendChangeChvReq_Req_t(void* xdrs, CAPI2_SIM_SendChangeChvReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendChangeChvReq_Req_t")

	if(
		xdr_CHV_t(xdrs, &rsp->chv_select) &&
		xdr_CHVString_t(xdrs, &rsp->old_chv) &&
		xdr_CHVString_t(xdrs, &rsp->new_chv) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIM_SendChangeChvReq_Rsp_t(void* xdrs, CAPI2_SIM_SendChangeChvReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendChangeChvReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_ACCESS_RESULT_t ), xdr_SIM_ACCESS_RESULT_t);
}

bool_t xdr_CAPI2_SIM_SendSetChv1OnOffReq_Req_t(void* xdrs, CAPI2_SIM_SendSetChv1OnOffReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendSetChv1OnOffReq_Req_t")

	if(
		xdr_CHVString_t(xdrs, &rsp->chv) &&
		_xdr_Boolean(xdrs, &rsp->enable_flag,"enable_flag") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIM_SendSetChv1OnOffReq_Rsp_t(void* xdrs, CAPI2_SIM_SendSetChv1OnOffReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendSetChv1OnOffReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_ACCESS_RESULT_t ), xdr_SIM_ACCESS_RESULT_t);
}

bool_t xdr_CAPI2_SIM_SendUnblockChvReq_Req_t(void* xdrs, CAPI2_SIM_SendUnblockChvReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendUnblockChvReq_Req_t")

	if(
		xdr_CHV_t(xdrs, &rsp->chv_select) &&
		xdr_PUKString_t(xdrs, &rsp->puk) &&
		xdr_CHVString_t(xdrs, &rsp->new_chv) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIM_SendUnblockChvReq_Rsp_t(void* xdrs, CAPI2_SIM_SendUnblockChvReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendUnblockChvReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_ACCESS_RESULT_t ), xdr_SIM_ACCESS_RESULT_t);
}

bool_t xdr_CAPI2_SIM_SendSetOperStateReq_Req_t(void* xdrs, CAPI2_SIM_SendSetOperStateReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendSetOperStateReq_Req_t")

	if(
		xdr_SIMOperState_t(xdrs, &rsp->oper_state) &&
		xdr_CHVString_t(xdrs, &rsp->chv2) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIM_SendSetOperStateReq_Rsp_t(void* xdrs, CAPI2_SIM_SendSetOperStateReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendSetOperStateReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_ACCESS_RESULT_t ), xdr_SIM_ACCESS_RESULT_t);
}

bool_t xdr_CAPI2_SIM_IsPbkAccessAllowed_Req_t(void* xdrs, CAPI2_SIM_IsPbkAccessAllowed_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_IsPbkAccessAllowed_Req_t")

	if(
		xdr_SIMPBK_ID_t(xdrs, &rsp->id) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIM_IsPbkAccessAllowed_Rsp_t(void* xdrs, CAPI2_SIM_IsPbkAccessAllowed_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_IsPbkAccessAllowed_Rsp_t")

	 return xdr_SIM_BOOLEAN_DATA_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_SendPbkInfoReq_Req_t(void* xdrs, CAPI2_SIM_SendPbkInfoReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendPbkInfoReq_Req_t")

	if(
		xdr_SIMPBK_ID_t(xdrs, &rsp->id) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIM_SendPbkInfoReq_Rsp_t(void* xdrs, CAPI2_SIM_SendPbkInfoReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendPbkInfoReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_PBK_INFO_t ), xdr_SIM_PBK_INFO_t);
}

bool_t xdr_CAPI2_SIM_SendReadAcmMaxReq_Rsp_t(void* xdrs, CAPI2_SIM_SendReadAcmMaxReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendReadAcmMaxReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_MAX_ACM_t ), xdr_SIM_MAX_ACM_t);
}

bool_t xdr_CAPI2_SIM_SendWriteAcmMaxReq_Req_t(void* xdrs, CAPI2_SIM_SendWriteAcmMaxReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendWriteAcmMaxReq_Req_t")

	if(
		xdr_CallMeterUnit_t(xdrs, &rsp->acm_max) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIM_SendWriteAcmMaxReq_Rsp_t(void* xdrs, CAPI2_SIM_SendWriteAcmMaxReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendWriteAcmMaxReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_ACCESS_RESULT_t ), xdr_SIM_ACCESS_RESULT_t);
}

bool_t xdr_CAPI2_SIM_SendReadAcmReq_Rsp_t(void* xdrs, CAPI2_SIM_SendReadAcmReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendReadAcmReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_ACM_VALUE_t ), xdr_SIM_ACM_VALUE_t);
}

bool_t xdr_CAPI2_SIM_SendWriteAcmReq_Req_t(void* xdrs, CAPI2_SIM_SendWriteAcmReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendWriteAcmReq_Req_t")

	if(
		xdr_CallMeterUnit_t(xdrs, &rsp->acm) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIM_SendWriteAcmReq_Rsp_t(void* xdrs, CAPI2_SIM_SendWriteAcmReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendWriteAcmReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_ACCESS_RESULT_t ), xdr_SIM_ACCESS_RESULT_t);
}

bool_t xdr_CAPI2_SIM_SendIncreaseAcmReq_Req_t(void* xdrs, CAPI2_SIM_SendIncreaseAcmReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendIncreaseAcmReq_Req_t")

	if(
		xdr_CallMeterUnit_t(xdrs, &rsp->acm) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIM_SendIncreaseAcmReq_Rsp_t(void* xdrs, CAPI2_SIM_SendIncreaseAcmReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendIncreaseAcmReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_ACCESS_RESULT_t ), xdr_SIM_ACCESS_RESULT_t);
}

bool_t xdr_CAPI2_SIM_SendReadSvcProvNameReq_Rsp_t(void* xdrs, CAPI2_SIM_SendReadSvcProvNameReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendReadSvcProvNameReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_SVC_PROV_NAME_t ), xdr_SIM_SVC_PROV_NAME_t);
}

bool_t xdr_CAPI2_SIM_SendReadPuctReq_Rsp_t(void* xdrs, CAPI2_SIM_SendReadPuctReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendReadPuctReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_PUCT_DATA_t ), xdr_SIM_PUCT_DATA_t);
}

bool_t xdr_CAPI2_SIM_GetServiceStatus_Req_t(void* xdrs, CAPI2_SIM_GetServiceStatus_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_GetServiceStatus_Req_t")

	if(
		xdr_SIMService_t(xdrs, &rsp->service) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIM_GetServiceStatus_Rsp_t(void* xdrs, CAPI2_SIM_GetServiceStatus_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_GetServiceStatus_Rsp_t")

	 return xdr_SIM_SERVICE_STATUS_RESULT_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_GetPinStatus_Rsp_t(void* xdrs, CAPI2_SIM_GetPinStatus_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_GetPinStatus_Rsp_t")

	 return xdr_SIM_PIN_STATUS_RESULT_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_IsPinOK_Rsp_t(void* xdrs, CAPI2_SIM_IsPinOK_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_IsPinOK_Rsp_t")

	 return xdr_SIM_BOOLEAN_DATA_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_GetIMSI_Rsp_t(void* xdrs, CAPI2_SIM_GetIMSI_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_GetIMSI_Rsp_t")

	 return xdr_SIM_IMSI_RESULT_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_GetGID1_Rsp_t(void* xdrs, CAPI2_SIM_GetGID1_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_GetGID1_Rsp_t")

	 return xdr_SIM_GID_RESULT_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_GetGID2_Rsp_t(void* xdrs, CAPI2_SIM_GetGID2_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_GetGID2_Rsp_t")

	 return xdr_SIM_GID_RESULT_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_GetHomePlmn_Rsp_t(void* xdrs, CAPI2_SIM_GetHomePlmn_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_GetHomePlmn_Rsp_t")

	 return xdr_SIM_HOME_PLMN_RESULT_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_GetCurrLockedSimlockType_Rsp_t(void* xdrs, CAPI2_SIM_GetCurrLockedSimlockType_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_GetCurrLockedSimlockType_Rsp_t")

	 return xdr_SIM_PIN_STATUS_RESULT_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_simmi_GetMasterFileId_Req_t(void* xdrs, CAPI2_simmi_GetMasterFileId_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_simmi_GetMasterFileId_Req_t")

	if(
		xdr_APDUFileID_t(xdrs, &rsp->file_id) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_simmi_GetMasterFileId_Rsp_t(void* xdrs, CAPI2_simmi_GetMasterFileId_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_simmi_GetMasterFileId_Rsp_t")

	 return xdr_SIM_APDU_FILEID_RESULT_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_SendOpenSocketReq_Rsp_t(void* xdrs, CAPI2_SIM_SendOpenSocketReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendOpenSocketReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_OPEN_SOCKET_RES_t ), xdr_SIM_OPEN_SOCKET_RES_t);
}

bool_t xdr_CAPI2_SIM_SendSelectAppiReq_Rsp_t(void* xdrs, CAPI2_SIM_SendSelectAppiReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendSelectAppiReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_SELECT_APPLI_RES_t ), xdr_SIM_SELECT_APPLI_RES_t);
}

bool_t xdr_CAPI2_SIM_SendDeactivateAppiReq_Rsp_t(void* xdrs, CAPI2_SIM_SendDeactivateAppiReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendDeactivateAppiReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_DEACTIVATE_APPLI_RES_t ), xdr_SIM_DEACTIVATE_APPLI_RES_t);
}

bool_t xdr_CAPI2_SIM_SendCloseSocketReq_Rsp_t(void* xdrs, CAPI2_SIM_SendCloseSocketReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendCloseSocketReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_CLOSE_SOCKET_RES_t ), xdr_SIM_CLOSE_SOCKET_RES_t);
}

bool_t xdr_CAPI2_SIM_GetAtrData_Rsp_t(void* xdrs, CAPI2_SIM_GetAtrData_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_GetAtrData_Rsp_t")

	 return xdr_SIM_ATR_DATA_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_SubmitDFileInfoReq_Rsp_t(void* xdrs, CAPI2_SIM_SubmitDFileInfoReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SubmitDFileInfoReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_DFILE_INFO_t ), xdr_SIM_DFILE_INFO_t);
}

bool_t xdr_CAPI2_SIM_SubmitEFileInfoReq_Rsp_t(void* xdrs, CAPI2_SIM_SubmitEFileInfoReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SubmitEFileInfoReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_EFILE_INFO_t ), xdr_SIM_EFILE_INFO_t);
}

bool_t xdr_CAPI2_SIM_SubmitWholeBinaryEFileReadReq_Rsp_t(void* xdrs, CAPI2_SIM_SubmitWholeBinaryEFileReadReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SubmitWholeBinaryEFileReadReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_EFILE_DATA_t ), xdr_SIM_EFILE_DATA_t);
}

bool_t xdr_CAPI2_SIM_SubmitBinaryEFileReadReq_Rsp_t(void* xdrs, CAPI2_SIM_SubmitBinaryEFileReadReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SubmitBinaryEFileReadReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_EFILE_DATA_t ), xdr_SIM_EFILE_DATA_t);
}

bool_t xdr_CAPI2_SIM_SubmitRecordEFileReadReq_Rsp_t(void* xdrs, CAPI2_SIM_SubmitRecordEFileReadReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SubmitRecordEFileReadReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_EFILE_DATA_t ), xdr_SIM_EFILE_DATA_t);
}

bool_t xdr_CAPI2_SIM_SubmitBinaryEFileUpdateReq_Rsp_t(void* xdrs, CAPI2_SIM_SubmitBinaryEFileUpdateReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SubmitBinaryEFileUpdateReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_EFILE_UPDATE_RESULT_t ), xdr_SIM_EFILE_UPDATE_RESULT_t);
}

bool_t xdr_CAPI2_SIM_SubmitLinearEFileUpdateReq_Rsp_t(void* xdrs, CAPI2_SIM_SubmitLinearEFileUpdateReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SubmitLinearEFileUpdateReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_EFILE_UPDATE_RESULT_t ), xdr_SIM_EFILE_UPDATE_RESULT_t);
}

bool_t xdr_CAPI2_SIM_SubmitSeekRecordReq_Rsp_t(void* xdrs, CAPI2_SIM_SubmitSeekRecordReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SubmitSeekRecordReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_SEEK_RECORD_DATA_t ), xdr_SIM_SEEK_RECORD_DATA_t);
}

bool_t xdr_CAPI2_SIM_SubmitCyclicEFileUpdateReq_Rsp_t(void* xdrs, CAPI2_SIM_SubmitCyclicEFileUpdateReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SubmitCyclicEFileUpdateReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_EFILE_UPDATE_RESULT_t ), xdr_SIM_EFILE_UPDATE_RESULT_t);
}

bool_t xdr_CAPI2_SIM_SendRemainingPinAttemptReq_Rsp_t(void* xdrs, CAPI2_SIM_SendRemainingPinAttemptReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendRemainingPinAttemptReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( PIN_ATTEMPT_RESULT_t ), xdr_PIN_ATTEMPT_RESULT_t);
}

bool_t xdr_CAPI2_SIM_IsCachedDataReady_Rsp_t(void* xdrs, CAPI2_SIM_IsCachedDataReady_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_IsCachedDataReady_Rsp_t")

	 return xdr_SIM_BOOLEAN_DATA_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_GetServiceCodeStatus_Req_t(void* xdrs, CAPI2_SIM_GetServiceCodeStatus_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_GetServiceCodeStatus_Req_t")

	if(
		xdr_SERVICE_CODE_STATUS_CPHS_t(xdrs, &rsp->service_code) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIM_GetServiceCodeStatus_Rsp_t(void* xdrs, CAPI2_SIM_GetServiceCodeStatus_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_GetServiceCodeStatus_Rsp_t")

	 return xdr_SIM_SERVICE_FLAG_STATUS_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_CheckCphsService_Req_t(void* xdrs, CAPI2_SIM_CheckCphsService_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_CheckCphsService_Req_t")

	if(
		xdr_CPHS_SST_ENTRY_t(xdrs, &rsp->sst_entry) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIM_CheckCphsService_Rsp_t(void* xdrs, CAPI2_SIM_CheckCphsService_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_CheckCphsService_Rsp_t")

	 return xdr_SIM_BOOLEAN_DATA_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_GetCphsPhase_Rsp_t(void* xdrs, CAPI2_SIM_GetCphsPhase_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_GetCphsPhase_Rsp_t")

	 return xdr_SIM_INTEGER_DATA_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_GetSmsSca_Req_t(void* xdrs, CAPI2_SIM_GetSmsSca_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_GetSmsSca_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->rec_no,"rec_no") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIM_GetSmsSca_Rsp_t(void* xdrs, CAPI2_SIM_GetSmsSca_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_GetSmsSca_Rsp_t")

	 return xdr_SIM_SCA_DATA_RESULT_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_GetIccid_Rsp_t(void* xdrs, CAPI2_SIM_GetIccid_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_GetIccid_Rsp_t")

	 return xdr_SIM_ICCID_STATUS_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_IsALSEnabled_Rsp_t(void* xdrs, CAPI2_SIM_IsALSEnabled_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_IsALSEnabled_Rsp_t")

	 return xdr_SIM_BOOLEAN_DATA_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_GetAlsDefaultLine_Rsp_t(void* xdrs, CAPI2_SIM_GetAlsDefaultLine_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_GetAlsDefaultLine_Rsp_t")

	 return xdr_SIM_INTEGER_DATA_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_SetAlsDefaultLine_Req_t(void* xdrs, CAPI2_SIM_SetAlsDefaultLine_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SetAlsDefaultLine_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->line,"line") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIM_GetCallForwardUnconditionalFlag_Rsp_t(void* xdrs, CAPI2_SIM_GetCallForwardUnconditionalFlag_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_GetCallForwardUnconditionalFlag_Rsp_t")

	 return xdr_SIM_CALL_FORWARD_UNCONDITIONAL_STATUS_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_GetApplicationType_Rsp_t(void* xdrs, CAPI2_SIM_GetApplicationType_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_GetApplicationType_Rsp_t")

	 return xdr_SIM_APPL_TYPE_RESULT_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_USIM_GetUst_Rsp_t(void* xdrs, CAPI2_USIM_GetUst_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USIM_GetUst_Rsp_t")

	 return xdr_USIM_UST_DATA_RSP_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_SendUpdatePrefListReq_Rsp_t(void* xdrs, CAPI2_SIM_SendUpdatePrefListReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendUpdatePrefListReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_ACCESS_RESULT_t ), xdr_SIM_ACCESS_RESULT_t);
}

bool_t xdr_CAPI2_SIM_SendWritePuctReq_Rsp_t(void* xdrs, CAPI2_SIM_SendWritePuctReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendWritePuctReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_ACCESS_RESULT_t ), xdr_SIM_ACCESS_RESULT_t);
}

bool_t xdr_CAPI2_SIM_SendGenericAccessReq_Rsp_t(void* xdrs, CAPI2_SIM_SendGenericAccessReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendGenericAccessReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_GENERIC_ACCESS_DATA_t ), xdr_SIM_GENERIC_ACCESS_DATA_t);
}

bool_t xdr_CAPI2_SIM_SubmitRestrictedAccessReq_Rsp_t(void* xdrs, CAPI2_SIM_SubmitRestrictedAccessReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SubmitRestrictedAccessReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_RESTRICTED_ACCESS_DATA_t ), xdr_SIM_RESTRICTED_ACCESS_DATA_t);
}

bool_t xdr_CAPI2_SIM_SendDetectionInd_Rsp_t(void* xdrs, CAPI2_SIM_SendDetectionInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendDetectionInd_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_DETECTION_t ), xdr_SIM_DETECTION_t);
}

bool_t xdr_CAPI2_MS_GsmRegStatusInd_Rsp_t(void* xdrs, CAPI2_MS_GsmRegStatusInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GsmRegStatusInd_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( MSRegInfo_t ), xdr_MSRegInfo_t);
}

bool_t xdr_CAPI2_MS_GprsRegStatusInd_Rsp_t(void* xdrs, CAPI2_MS_GprsRegStatusInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GprsRegStatusInd_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( MSRegInfo_t ), xdr_MSRegInfo_t);
}

bool_t xdr_CAPI2_MS_NetworkNameInd_Rsp_t(void* xdrs, CAPI2_MS_NetworkNameInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_NetworkNameInd_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( nitzNetworkName_t ), xdr_nitzNetworkName_t);
}

bool_t xdr_CAPI2_MS_RssiInd_Rsp_t(void* xdrs, CAPI2_MS_RssiInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_RssiInd_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( RxSignalInfo_t ), xdr_RxSignalInfo_t);
}

bool_t xdr_CAPI2_MS_SignalChangeInd_Rsp_t(void* xdrs, CAPI2_MS_SignalChangeInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_SignalChangeInd_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( RX_SIGNAL_INFO_CHG_t ), xdr_RX_SIGNAL_INFO_CHG_t);
}

bool_t xdr_CAPI2_MS_PlmnListInd_Rsp_t(void* xdrs, CAPI2_MS_PlmnListInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_PlmnListInd_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SEARCHED_PLMN_LIST_t ), xdr_SEARCHED_PLMN_LIST_t);
}

bool_t xdr_CAPI2_MS_TimeZoneInd_Rsp_t(void* xdrs, CAPI2_MS_TimeZoneInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_TimeZoneInd_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( TimeZoneDate_t ), xdr_TimeZoneDate_t);
}

bool_t xdr_CAPI2_ADCMGR_Start_Req_t(void* xdrs, CAPI2_ADCMGR_Start_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_ADCMGR_Start_Req_t")

	if(
		_xdr_UInt16(xdrs, &rsp->init_value,"init_value") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_ADCMGR_Start_Rsp_t(void* xdrs, CAPI2_ADCMGR_Start_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_ADCMGR_Start_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( ADC_StartRsp_t ), xdr_ADC_StartRsp_t);
}

bool_t xdr_CAPI2_AT_ProcessCmd_Req_t(void* xdrs, CAPI2_AT_ProcessCmd_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_AT_ProcessCmd_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->channel,"channel") &&
		xdr_uchar_ptr_t(xdrs, &rsp->cmdStr) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_AT_Response_Rsp_t(void* xdrs, CAPI2_AT_Response_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_AT_Response_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( AtResponse_t ), xdr_AtResponse_t);
}

bool_t xdr_CAPI2_MS_GetSystemRAT_Rsp_t(void* xdrs, CAPI2_MS_GetSystemRAT_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetSystemRAT_Rsp_t")

	 return xdr_RATSelect_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_MS_GetSupportedRAT_Rsp_t(void* xdrs, CAPI2_MS_GetSupportedRAT_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetSupportedRAT_Rsp_t")

	 return xdr_RATSelect_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_MS_GetSystemBand_Rsp_t(void* xdrs, CAPI2_MS_GetSystemBand_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetSystemBand_Rsp_t")

	 return xdr_BandSelect_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_MS_GetSupportedBand_Rsp_t(void* xdrs, CAPI2_MS_GetSupportedBand_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetSupportedBand_Rsp_t")

	 return xdr_BandSelect_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SYSPARM_GetMSClass_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetMSClass_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_GetMSClass_Rsp_t")

	 return _xdr_UInt16(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_AUDIO_GetMicrophoneGainSetting_Rsp_t(void* xdrs, CAPI2_AUDIO_GetMicrophoneGainSetting_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_AUDIO_GetMicrophoneGainSetting_Rsp_t")

	 return _xdr_UInt16(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_AUDIO_GetSpeakerVol_Rsp_t(void* xdrs, CAPI2_AUDIO_GetSpeakerVol_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_AUDIO_GetSpeakerVol_Rsp_t")

	 return _xdr_UInt16(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_AUDIO_SetSpeakerVol_Req_t(void* xdrs, CAPI2_AUDIO_SetSpeakerVol_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_AUDIO_SetSpeakerVol_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->vol,"vol") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_AUDIO_SetMicrophoneGain_Req_t(void* xdrs, CAPI2_AUDIO_SetMicrophoneGain_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_AUDIO_SetMicrophoneGain_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->gain,"gain") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYSPARM_GetManufacturerName_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetManufacturerName_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_GetManufacturerName_Rsp_t")

	 return xdr_uchar_ptr_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SYSPARM_GetModelName_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetModelName_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_GetModelName_Rsp_t")

	 return xdr_uchar_ptr_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SYSPARM_GetSWVersion_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetSWVersion_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_GetSWVersion_Rsp_t")

	 return xdr_uchar_ptr_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SYSPARM_GetEGPRSMSClass_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetEGPRSMSClass_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_GetEGPRSMSClass_Rsp_t")

	 return _xdr_UInt16(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_UTIL_ExtractImei_Rsp_t(void* xdrs, CAPI2_UTIL_ExtractImei_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_UTIL_ExtractImei_Rsp_t")

	 return xdr_MSImeiStr_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_MS_GetRegistrationInfo_Rsp_t(void* xdrs, CAPI2_MS_GetRegistrationInfo_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetRegistrationInfo_Rsp_t")

	 return xdr_MSRegStateInfo_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_SendNumOfPLMNEntryReq_Req_t(void* xdrs, CAPI2_SIM_SendNumOfPLMNEntryReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendNumOfPLMNEntryReq_Req_t")

	if(
		xdr_SIM_PLMN_FILE_t(xdrs, &rsp->plmn_file) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIM_SendNumOfPLMNEntryReq_Rsp_t(void* xdrs, CAPI2_SIM_SendNumOfPLMNEntryReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendNumOfPLMNEntryReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_PLMN_NUM_OF_ENTRY_t ), xdr_SIM_PLMN_NUM_OF_ENTRY_t);
}

bool_t xdr_CAPI2_SIM_SendReadPLMNEntryReq_Req_t(void* xdrs, CAPI2_SIM_SendReadPLMNEntryReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendReadPLMNEntryReq_Req_t")

	if(
		xdr_SIM_PLMN_FILE_t(xdrs, &rsp->plmn_file) &&
		_xdr_UInt16(xdrs, &rsp->start_index,"start_index") &&
		_xdr_UInt16(xdrs, &rsp->end_index,"end_index") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIM_SendReadPLMNEntryReq_Rsp_t(void* xdrs, CAPI2_SIM_SendReadPLMNEntryReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendReadPLMNEntryReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_PLMN_ENTRY_DATA_t ), xdr_SIM_PLMN_ENTRY_DATA_t);
}

bool_t xdr_CAPI2_SIM_SendWritePLMNEntryReq_Rsp_t(void* xdrs, CAPI2_SIM_SendWritePLMNEntryReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendWritePLMNEntryReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_PLMN_ENTRY_UPDATE_t ), xdr_SIM_PLMN_ENTRY_UPDATE_t);
}

bool_t xdr_CAPI2_SIM_SendWriteMulPLMNEntryReq_Rsp_t(void* xdrs, CAPI2_SIM_SendWriteMulPLMNEntryReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendWriteMulPLMNEntryReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_MUL_PLMN_ENTRY_UPDATE_t ), xdr_SIM_MUL_PLMN_ENTRY_UPDATE_t);
}

bool_t xdr_CAPI2_SYS_SetRegisteredEventMask_Rsp_t(void* xdrs, CAPI2_SYS_SetRegisteredEventMask_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYS_SetRegisteredEventMask_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_SYS_SetFilteredEventMask_Rsp_t(void* xdrs, CAPI2_SYS_SetFilteredEventMask_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYS_SetFilteredEventMask_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_SYS_SetRssiThreshold_Req_t(void* xdrs, CAPI2_SYS_SetRssiThreshold_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYS_SetRssiThreshold_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->gsm_rxlev_thresold,"gsm_rxlev_thresold") &&
		_xdr_UInt8(xdrs, &rsp->gsm_rxqual_thresold,"gsm_rxqual_thresold") &&
		_xdr_UInt8(xdrs, &rsp->umts_rscp_thresold,"umts_rscp_thresold") &&
		_xdr_UInt8(xdrs, &rsp->umts_ecio_thresold,"umts_ecio_thresold") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYS_GetBootLoaderVersion_Rsp_t(void* xdrs, CAPI2_SYS_GetBootLoaderVersion_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYS_GetBootLoaderVersion_Rsp_t")

	 return xdr_uchar_ptr_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SYS_GetDSFVersion_Rsp_t(void* xdrs, CAPI2_SYS_GetDSFVersion_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYS_GetDSFVersion_Rsp_t")

	 return xdr_uchar_ptr_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SYSPARM_GetChanMode_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetChanMode_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_GetChanMode_Rsp_t")

	 return _xdr_UInt16(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_SYSPARM_GetClassmark_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetClassmark_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_GetClassmark_Rsp_t")

	 return xdr_CAPI2_Class_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SYSPARM_GetIMEI_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetIMEI_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_GetIMEI_Rsp_t")

	 return xdr_CAPI2_SYSPARM_IMEI_PTR_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SYSPARM_GetSysparmIndPartFileVersion_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetSysparmIndPartFileVersion_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_GetSysparmIndPartFileVersion_Rsp_t")

	 return _xdr_UInt16(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_SYSPARM_SetDARPCfg_Req_t(void* xdrs, CAPI2_SYSPARM_SetDARPCfg_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_SetDARPCfg_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->darp_cfg,"darp_cfg") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYSPARM_SetEGPRSMSClass_Req_t(void* xdrs, CAPI2_SYSPARM_SetEGPRSMSClass_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_SetEGPRSMSClass_Req_t")

	if(
		_xdr_UInt16(xdrs, &rsp->egprs_class,"egprs_class") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYSPARM_SetGPRSMSClass_Req_t(void* xdrs, CAPI2_SYSPARM_SetGPRSMSClass_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_SetGPRSMSClass_Req_t")

	if(
		_xdr_UInt16(xdrs, &rsp->gprs_class,"gprs_class") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYS_SetMSPowerOnCause_Req_t(void* xdrs, CAPI2_SYS_SetMSPowerOnCause_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYS_SetMSPowerOnCause_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->reset_cause,"reset_cause") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_TIMEZONE_GetTZUpdateMode_Rsp_t(void* xdrs, CAPI2_TIMEZONE_GetTZUpdateMode_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_TIMEZONE_GetTZUpdateMode_Rsp_t")

	 return xdr_TimeZoneUpdateMode_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_TIMEZONE_SetTZUpdateMode_Req_t(void* xdrs, CAPI2_TIMEZONE_SetTZUpdateMode_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_TIMEZONE_SetTZUpdateMode_Req_t")

	if(
		xdr_TimeZoneUpdateMode_t(xdrs, &rsp->mode) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_TIMEZONE_UpdateRTC_Req_t(void* xdrs, CAPI2_TIMEZONE_UpdateRTC_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_TIMEZONE_UpdateRTC_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->updateFlag,"updateFlag") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PMU_IsSIMReady_RSP_Rsp_t(void* xdrs, CAPI2_PMU_IsSIMReady_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PMU_IsSIMReady_RSP_Rsp_t")
if(
		_xdr_Boolean(xdrs, &rsp->status,"status") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PMU_ActivateSIM_Req_t(void* xdrs, CAPI2_PMU_ActivateSIM_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PMU_ActivateSIM_Req_t")

	if(
		xdr_Int32(xdrs, &rsp->volt) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PMU_ActivateSIM_RSP_Rsp_t(void* xdrs, CAPI2_PMU_ActivateSIM_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PMU_ActivateSIM_RSP_Rsp_t")
if(
		_xdr_Boolean(xdrs, &rsp->status,"status") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PMU_DeactivateSIM_RSP_Rsp_t(void* xdrs, CAPI2_PMU_DeactivateSIM_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PMU_DeactivateSIM_RSP_Rsp_t")
if(
		_xdr_Boolean(xdrs, &rsp->status,"status") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_TestCmds_Req_t(void* xdrs, CAPI2_TestCmds_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_TestCmds_Req_t")

	if(
		xdr_UInt32(xdrs, &rsp->testId) &&
		xdr_UInt32(xdrs, &rsp->param1) &&
		xdr_UInt32(xdrs, &rsp->param2) &&
		xdr_uchar_ptr_t(xdrs, &rsp->buffer) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SATK_SendPlayToneRes_Req_t(void* xdrs, CAPI2_SATK_SendPlayToneRes_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SATK_SendPlayToneRes_Req_t")

	if(
		xdr_SATK_ResultCode_t(xdrs, &rsp->resultCode) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SATK_SendSetupCallRes_Req_t(void* xdrs, CAPI2_SATK_SendSetupCallRes_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SATK_SendSetupCallRes_Req_t")

	if(
		xdr_SATK_ResultCode_t(xdrs, &rsp->resultCode) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PBK_SetFdnCheck_Req_t(void* xdrs, CAPI2_PBK_SetFdnCheck_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PBK_SetFdnCheck_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->fdn_chk_on,"fdn_chk_on") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PBK_GetFdnCheck_Rsp_t(void* xdrs, CAPI2_PBK_GetFdnCheck_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PBK_GetFdnCheck_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_GPIO_Set_High_64Pin_Req_t(void* xdrs, CAPI2_GPIO_Set_High_64Pin_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_GPIO_Set_High_64Pin_Req_t")

	if(
		xdr_UInt32(xdrs, &rsp->gpio_pin) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_GPIO_Set_High_64Pin_RSP_Rsp_t(void* xdrs, CAPI2_GPIO_Set_High_64Pin_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_GPIO_Set_High_64Pin_RSP_Rsp_t")
if(
		_xdr_Boolean(xdrs, &rsp->status,"status") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_GPIO_Set_Low_64Pin_Req_t(void* xdrs, CAPI2_GPIO_Set_Low_64Pin_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_GPIO_Set_Low_64Pin_Req_t")

	if(
		xdr_UInt32(xdrs, &rsp->gpio_pin) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_GPIO_Set_Low_64Pin_RSP_Rsp_t(void* xdrs, CAPI2_GPIO_Set_Low_64Pin_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_GPIO_Set_Low_64Pin_RSP_Rsp_t")
if(
		_xdr_Boolean(xdrs, &rsp->status,"status") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PMU_StartCharging_RSP_Rsp_t(void* xdrs, CAPI2_PMU_StartCharging_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PMU_StartCharging_RSP_Rsp_t")
if(
		_xdr_Boolean(xdrs, &rsp->status,"status") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PMU_StopCharging_RSP_Rsp_t(void* xdrs, CAPI2_PMU_StopCharging_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PMU_StopCharging_RSP_Rsp_t")
if(
		_xdr_Boolean(xdrs, &rsp->status,"status") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PMU_Battery_Register_Req_t(void* xdrs, CAPI2_PMU_Battery_Register_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PMU_Battery_Register_Req_t")

	if(
		xdr_HAL_EM_BATTMGR_Event_en_t(xdrs, &rsp->event) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PMU_Battery_Register_Rsp_t(void* xdrs, CAPI2_PMU_Battery_Register_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PMU_Battery_Register_Rsp_t")

	 return xdr_HAL_EM_BATTMGR_Result_en_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_BattLevelInd_Rsp_t(void* xdrs, CAPI2_BattLevelInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_BattLevelInd_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( HAL_EM_BatteryLevel_t ), xdr_HAL_EM_BatteryLevel_t);
}

bool_t xdr_CAPI2_SMS_IsMeStorageEnabled_RSP_Rsp_t(void* xdrs, CAPI2_SMS_IsMeStorageEnabled_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_IsMeStorageEnabled_RSP_Rsp_t")
if(
		_xdr_Boolean(xdrs, &rsp->status,"status") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SMS_GetMaxMeCapacity_RSP_Rsp_t(void* xdrs, CAPI2_SMS_GetMaxMeCapacity_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_GetMaxMeCapacity_RSP_Rsp_t")
if(
		_xdr_UInt16(xdrs, &rsp->capacity,"capacity") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SMS_GetNextFreeSlot_RSP_Rsp_t(void* xdrs, CAPI2_SMS_GetNextFreeSlot_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_GetNextFreeSlot_RSP_Rsp_t")
if(
		_xdr_UInt16(xdrs, &rsp->free_slot,"free_slot") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SMS_SetMeSmsStatus_Req_t(void* xdrs, CAPI2_SMS_SetMeSmsStatus_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_SetMeSmsStatus_Req_t")

	if(
		_xdr_UInt16(xdrs, &rsp->slotNumber,"slotNumber") &&
		xdr_SIMSMSMesgStatus_t(xdrs, &rsp->status) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SMS_SetMeSmsStatus_RSP_Rsp_t(void* xdrs, CAPI2_SMS_SetMeSmsStatus_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_SetMeSmsStatus_RSP_Rsp_t")
if(
		_xdr_Boolean(xdrs, &rsp->flag,"flag") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SMS_GetMeSmsStatus_Req_t(void* xdrs, CAPI2_SMS_GetMeSmsStatus_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_GetMeSmsStatus_Req_t")

	if(
		_xdr_UInt16(xdrs, &rsp->slotNumber,"slotNumber") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SMS_GetMeSmsStatus_RSP_Rsp_t(void* xdrs, CAPI2_SMS_GetMeSmsStatus_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_GetMeSmsStatus_RSP_Rsp_t")
if(
		xdr_SIMSMSMesgStatus_t(xdrs, &rsp->status) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SMS_StoreSmsToMe_RSP_Rsp_t(void* xdrs, CAPI2_SMS_StoreSmsToMe_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_StoreSmsToMe_RSP_Rsp_t")
if(
		_xdr_Boolean(xdrs, &rsp->flag,"flag") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SMS_RetrieveSmsFromMe_Req_t(void* xdrs, CAPI2_SMS_RetrieveSmsFromMe_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_RetrieveSmsFromMe_Req_t")

	if(
		_xdr_UInt16(xdrs, &rsp->slotNumber,"slotNumber") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SMS_RetrieveSmsFromMe_RSP_Rsp_t(void* xdrs, CAPI2_SMS_RetrieveSmsFromMe_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_RetrieveSmsFromMe_RSP_Rsp_t")
if(
		_xdr_Boolean(xdrs, &rsp->flag,"flag") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SMS_RemoveSmsFromMe_Req_t(void* xdrs, CAPI2_SMS_RemoveSmsFromMe_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_RemoveSmsFromMe_Req_t")

	if(
		_xdr_UInt16(xdrs, &rsp->slotNumber,"slotNumber") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SMS_RemoveSmsFromMe_RSP_Rsp_t(void* xdrs, CAPI2_SMS_RemoveSmsFromMe_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_RemoveSmsFromMe_RSP_Rsp_t")
if(
		_xdr_Boolean(xdrs, &rsp->flag,"flag") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SMS_ConfigureMEStorage_Req_t(void* xdrs, CAPI2_SMS_ConfigureMEStorage_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_ConfigureMEStorage_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->flag,"flag") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_SetElement_Req_t(void* xdrs, CAPI2_MS_SetElement_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_SetElement_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->inElemData,sizeof( CAPI2_MS_Element_t ), xdr_CAPI2_MS_Element_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_GetElement_Req_t(void* xdrs, CAPI2_MS_GetElement_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetElement_Req_t")

	if(
		xdr_MS_Element_t(xdrs, &rsp->inElemType) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_GetElement_Rsp_t(void* xdrs, CAPI2_MS_GetElement_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetElement_Rsp_t")

	 return xdr_CAPI2_MS_Element_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_USIM_IsApplicationSupported_Req_t(void* xdrs, CAPI2_USIM_IsApplicationSupported_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USIM_IsApplicationSupported_Req_t")

	if(
		xdr_USIM_APPLICATION_TYPE(xdrs, &rsp->appli_type) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_USIM_IsApplicationSupported_Rsp_t(void* xdrs, CAPI2_USIM_IsApplicationSupported_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USIM_IsApplicationSupported_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_USIM_IsAllowedAPN_Req_t(void* xdrs, CAPI2_USIM_IsAllowedAPN_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USIM_IsAllowedAPN_Req_t")

	if(
		xdr_char_ptr_t(xdrs, &rsp->apn_name) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_USIM_IsAllowedAPN_Rsp_t(void* xdrs, CAPI2_USIM_IsAllowedAPN_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USIM_IsAllowedAPN_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_USIM_GetNumOfAPN_Rsp_t(void* xdrs, CAPI2_USIM_GetNumOfAPN_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USIM_GetNumOfAPN_Rsp_t")

	 return _xdr_UInt8(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_USIM_GetAPNEntry_Req_t(void* xdrs, CAPI2_USIM_GetAPNEntry_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USIM_GetAPNEntry_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->index,"index") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_USIM_GetAPNEntry_Rsp_t(void* xdrs, CAPI2_USIM_GetAPNEntry_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USIM_GetAPNEntry_Rsp_t")

	 return xdr_APN_NAME_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_USIM_IsEstServActivated_Req_t(void* xdrs, CAPI2_USIM_IsEstServActivated_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USIM_IsEstServActivated_Req_t")

	if(
		xdr_USIM_EST_SERVICE_t(xdrs, &rsp->est_serv) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_USIM_IsEstServActivated_Rsp_t(void* xdrs, CAPI2_USIM_IsEstServActivated_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USIM_IsEstServActivated_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_USIM_SendSetEstServReq_Req_t(void* xdrs, CAPI2_USIM_SendSetEstServReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USIM_SendSetEstServReq_Req_t")

	if(
		xdr_USIM_EST_SERVICE_t(xdrs, &rsp->est_serv) &&
		_xdr_Boolean(xdrs, &rsp->serv_on,"serv_on") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_USIM_SendSetEstServReq_Rsp_t(void* xdrs, CAPI2_USIM_SendSetEstServReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USIM_SendSetEstServReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( USIM_FILE_UPDATE_RSP_t ), xdr_USIM_FILE_UPDATE_RSP_t);
}

bool_t xdr_CAPI2_USIM_SendWriteAPNReq_Req_t(void* xdrs, CAPI2_USIM_SendWriteAPNReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USIM_SendWriteAPNReq_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->index,"index") &&
		xdr_char_ptr_t(xdrs, &rsp->apn_name) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_USIM_SendWriteAPNReq_Rsp_t(void* xdrs, CAPI2_USIM_SendWriteAPNReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USIM_SendWriteAPNReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( USIM_FILE_UPDATE_RSP_t ), xdr_USIM_FILE_UPDATE_RSP_t);
}

bool_t xdr_CAPI2_USIM_SendDeleteAllAPNReq_Rsp_t(void* xdrs, CAPI2_USIM_SendDeleteAllAPNReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USIM_SendDeleteAllAPNReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( USIM_FILE_UPDATE_RSP_t ), xdr_USIM_FILE_UPDATE_RSP_t);
}

bool_t xdr_CAPI2_USIM_GetRatModeSetting_Rsp_t(void* xdrs, CAPI2_USIM_GetRatModeSetting_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USIM_GetRatModeSetting_Rsp_t")

	 return xdr_USIM_RAT_MODE_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_PMU_ClientPowerDown_RSP_Rsp_t(void* xdrs, CAPI2_PMU_ClientPowerDown_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PMU_ClientPowerDown_RSP_Rsp_t")
if(
		_xdr_Boolean(xdrs, &rsp->status,"status") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PMU_GetPowerupCause_RSP_Rsp_t(void* xdrs, CAPI2_PMU_GetPowerupCause_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PMU_GetPowerupCause_RSP_Rsp_t")
if(
		xdr_PMU_PowerupId_t(xdrs, &rsp->powerupId) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_GetGPRSRegState_Rsp_t(void* xdrs, CAPI2_MS_GetGPRSRegState_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetGPRSRegState_Rsp_t")

	 return xdr_MSRegState_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_MS_GetGSMRegState_Rsp_t(void* xdrs, CAPI2_MS_GetGSMRegState_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetGSMRegState_Rsp_t")

	 return xdr_MSRegState_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_MS_GetRegisteredCellInfo_Rsp_t(void* xdrs, CAPI2_MS_GetRegisteredCellInfo_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetRegisteredCellInfo_Rsp_t")

	 return xdr_CellInfo_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_MS_GetStartBand_Rsp_t(void* xdrs, CAPI2_MS_GetStartBand_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetStartBand_Rsp_t")

	 return _xdr_UInt8(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_MS_SetMEPowerClass_Req_t(void* xdrs, CAPI2_MS_SetMEPowerClass_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_SetMEPowerClass_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->band,"band") &&
		_xdr_UInt8(xdrs, &rsp->pwrClass,"pwrClass") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_USIM_GetServiceStatus_Req_t(void* xdrs, CAPI2_USIM_GetServiceStatus_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USIM_GetServiceStatus_Req_t")

	if(
		xdr_SIMService_t(xdrs, &rsp->service) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_USIM_GetServiceStatus_Rsp_t(void* xdrs, CAPI2_USIM_GetServiceStatus_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USIM_GetServiceStatus_Rsp_t")

	 return xdr_SIMServiceStatus_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_IsAllowedAPN_Req_t(void* xdrs, CAPI2_SIM_IsAllowedAPN_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_IsAllowedAPN_Req_t")

	if(
		xdr_char_ptr_t(xdrs, &rsp->apn_name) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIM_IsAllowedAPN_Rsp_t(void* xdrs, CAPI2_SIM_IsAllowedAPN_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_IsAllowedAPN_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_SMS_GetSmsMaxCapacity_Req_t(void* xdrs, CAPI2_SMS_GetSmsMaxCapacity_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_GetSmsMaxCapacity_Req_t")

	if(
		xdr_SmsStorage_t(xdrs, &rsp->storageType) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SMS_GetSmsMaxCapacity_Rsp_t(void* xdrs, CAPI2_SMS_GetSmsMaxCapacity_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_GetSmsMaxCapacity_Rsp_t")

	 return _xdr_UInt16(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_SMS_RetrieveMaxCBChnlLength_Rsp_t(void* xdrs, CAPI2_SMS_RetrieveMaxCBChnlLength_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_RetrieveMaxCBChnlLength_Rsp_t")

	 return _xdr_UInt8(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_SIM_IsBdnOperationRestricted_Rsp_t(void* xdrs, CAPI2_SIM_IsBdnOperationRestricted_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_IsBdnOperationRestricted_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_SIM_SendPreferredPlmnUpdateInd_Req_t(void* xdrs, CAPI2_SIM_SendPreferredPlmnUpdateInd_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendPreferredPlmnUpdateInd_Req_t")

	if(
		xdr_SIM_PLMN_FILE_t(xdrs, &rsp->prefer_plmn_file) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIM_SendSetBdnReq_Req_t(void* xdrs, CAPI2_SIM_SendSetBdnReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendSetBdnReq_Req_t")

	if(
		xdr_SIMBdnOperState_t(xdrs, &rsp->oper_state) &&
		xdr_CHVString_t(xdrs, &rsp->chv2) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIM_SendSetBdnReq_Rsp_t(void* xdrs, CAPI2_SIM_SendSetBdnReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendSetBdnReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_ACCESS_RESULT_t ), xdr_SIM_ACCESS_RESULT_t);
}

bool_t xdr_CAPI2_SIM_PowerOnOffCard_Req_t(void* xdrs, CAPI2_SIM_PowerOnOffCard_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_PowerOnOffCard_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->power_on,"power_on") &&
		xdr_SIM_POWER_ON_MODE_t(xdrs, &rsp->mode) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIM_PowerOnOffCard_Rsp_t(void* xdrs, CAPI2_SIM_PowerOnOffCard_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_PowerOnOffCard_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_GENERIC_APDU_RES_INFO_t ), xdr_SIM_GENERIC_APDU_RES_INFO_t);
}

bool_t xdr_CAPI2_SIM_GetRawAtr_Rsp_t(void* xdrs, CAPI2_SIM_GetRawAtr_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_GetRawAtr_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_GENERIC_APDU_ATR_INFO_t ), xdr_SIM_GENERIC_APDU_ATR_INFO_t);
}

bool_t xdr_CAPI2_SIM_Set_Protocol_Req_t(void* xdrs, CAPI2_SIM_Set_Protocol_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_Set_Protocol_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->protocol,"protocol") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIM_Set_Protocol_Rsp_t(void* xdrs, CAPI2_SIM_Set_Protocol_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_Set_Protocol_Rsp_t")

	 return xdr_Result_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_Get_Protocol_Rsp_t(void* xdrs, CAPI2_SIM_Get_Protocol_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_Get_Protocol_Rsp_t")

	 return _xdr_UInt8(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_SIM_SendGenericApduCmd_Rsp_t(void* xdrs, CAPI2_SIM_SendGenericApduCmd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendGenericApduCmd_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_GENERIC_APDU_XFER_RSP_t ), xdr_SIM_GENERIC_APDU_XFER_RSP_t);
}

bool_t xdr_CAPI2_SIM_TerminateXferApdu_Rsp_t(void* xdrs, CAPI2_SIM_TerminateXferApdu_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_TerminateXferApdu_Rsp_t")

	 return xdr_Result_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_MS_SetPlmnSelectRat_Req_t(void* xdrs, CAPI2_MS_SetPlmnSelectRat_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_SetPlmnSelectRat_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->manual_rat,"manual_rat") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_IsDeRegisterInProgress_Rsp_t(void* xdrs, CAPI2_MS_IsDeRegisterInProgress_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_IsDeRegisterInProgress_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_MS_IsRegisterInProgress_Rsp_t(void* xdrs, CAPI2_MS_IsRegisterInProgress_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_IsRegisterInProgress_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_SOCKET_Open_Req_t(void* xdrs, CAPI2_SOCKET_Open_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_Open_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->domain,"domain") &&
		_xdr_UInt8(xdrs, &rsp->type,"type") &&
		_xdr_UInt8(xdrs, &rsp->protocol,"protocol") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_Open_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_Open_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_Open_RSP_Rsp_t")
if(
		xdr_Int32(xdrs, &rsp->descriptor) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_Bind_Req_t(void* xdrs, CAPI2_SOCKET_Bind_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_Bind_Req_t")

	if(
		xdr_Int32(xdrs, &rsp->descriptor) &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->addr,sizeof( sockaddr ), xdr_sockaddr) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_Bind_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_Bind_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_Bind_RSP_Rsp_t")
if(
		_xdr_Int8(xdrs, (char*)&rsp->status,"status") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_Listen_Req_t(void* xdrs, CAPI2_SOCKET_Listen_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_Listen_Req_t")

	if(
		xdr_Int32(xdrs, &rsp->descriptor) &&
		xdr_UInt32(xdrs, &rsp->backlog) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_Listen_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_Listen_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_Listen_RSP_Rsp_t")
if(
		_xdr_Int8(xdrs, (char*)&rsp->status,"status") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_Accept_Req_t(void* xdrs, CAPI2_SOCKET_Accept_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_Accept_Req_t")

	if(
		xdr_Int32(xdrs, &rsp->descriptor) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_Accept_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_Accept_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_Accept_RSP_Rsp_t")
if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->name,sizeof( sockaddr ), xdr_sockaddr) &&
		xdr_Int32(xdrs, &rsp->acceptDescriptor) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_Connect_Req_t(void* xdrs, CAPI2_SOCKET_Connect_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_Connect_Req_t")

	if(
		xdr_Int32(xdrs, &rsp->descriptor) &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->name,sizeof( sockaddr ), xdr_sockaddr) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_Connect_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_Connect_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_Connect_RSP_Rsp_t")
if(
		_xdr_Int8(xdrs, (char*)&rsp->status,"status") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_GetPeerName_Req_t(void* xdrs, CAPI2_SOCKET_GetPeerName_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_GetPeerName_Req_t")

	if(
		xdr_Int32(xdrs, &rsp->descriptor) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_GetPeerName_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_GetPeerName_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_GetPeerName_RSP_Rsp_t")
if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->peerName,sizeof( sockaddr ), xdr_sockaddr) &&
		_xdr_Int8(xdrs, (char*)&rsp->status,"status") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_GetSockName_Req_t(void* xdrs, CAPI2_SOCKET_GetSockName_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_GetSockName_Req_t")

	if(
		xdr_Int32(xdrs, &rsp->descriptor) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_GetSockName_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_GetSockName_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_GetSockName_RSP_Rsp_t")
if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->sockName,sizeof( sockaddr ), xdr_sockaddr) &&
		_xdr_Int8(xdrs, (char*)&rsp->status,"status") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_SetSockOpt_Req_t(void* xdrs, CAPI2_SOCKET_SetSockOpt_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_SetSockOpt_Req_t")

	if(
		xdr_Int32(xdrs, &rsp->descriptor) &&
		_xdr_UInt16(xdrs, &rsp->optname,"optname") &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->optval,sizeof( SockOptVal_t ), xdr_SockOptVal_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_SetSockOpt_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_SetSockOpt_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_SetSockOpt_RSP_Rsp_t")
if(
		_xdr_Int8(xdrs, (char*)&rsp->status,"status") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_GetSockOpt_Req_t(void* xdrs, CAPI2_SOCKET_GetSockOpt_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_GetSockOpt_Req_t")

	if(
		xdr_Int32(xdrs, &rsp->descriptor) &&
		_xdr_UInt16(xdrs, &rsp->optname,"optname") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_GetSockOpt_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_GetSockOpt_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_GetSockOpt_RSP_Rsp_t")
if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->optval,sizeof( SockOptVal_t ), xdr_SockOptVal_t) &&
		_xdr_Int8(xdrs, (char*)&rsp->status,"status") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_SignalInd_Req_t(void* xdrs, CAPI2_SOCKET_SignalInd_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_SignalInd_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->sockSignalInd,sizeof( SocketSignalInd_t ), xdr_SocketSignalInd_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_Send_Req_t(void* xdrs, CAPI2_SOCKET_Send_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_Send_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->sockSendReq,sizeof( SocketSendReq_t ), xdr_SocketSendReq_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_Send_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_Send_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_Send_RSP_Rsp_t")
if(
		xdr_Int32(xdrs, &rsp->bytesSent) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_SendTo_Req_t(void* xdrs, CAPI2_SOCKET_SendTo_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_SendTo_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->sockSendReq,sizeof( SocketSendReq_t ), xdr_SocketSendReq_t) &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->to,sizeof( sockaddr ), xdr_sockaddr) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_SendTo_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_SendTo_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_SendTo_RSP_Rsp_t")
if(
		xdr_Int32(xdrs, &rsp->bytesSent) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_Recv_Req_t(void* xdrs, CAPI2_SOCKET_Recv_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_Recv_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->sockRecvReq,sizeof( SocketRecvReq_t ), xdr_SocketRecvReq_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_Recv_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_Recv_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_Recv_RSP_Rsp_t")
if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->rsp,sizeof( SocketRecvRsp_t ), xdr_SocketRecvRsp_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_RecvFrom_Req_t(void* xdrs, CAPI2_SOCKET_RecvFrom_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_RecvFrom_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->sockRecvReq,sizeof( SocketRecvReq_t ), xdr_SocketRecvReq_t) &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->from,sizeof( sockaddr ), xdr_sockaddr) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_RecvFrom_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_RecvFrom_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_RecvFrom_RSP_Rsp_t")
if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->rsp,sizeof( SocketRecvRsp_t ), xdr_SocketRecvRsp_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_Close_Req_t(void* xdrs, CAPI2_SOCKET_Close_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_Close_Req_t")

	if(
		xdr_Int32(xdrs, &rsp->descriptor) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_Close_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_Close_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_Close_RSP_Rsp_t")
if(
		_xdr_Int8(xdrs, (char*)&rsp->status,"status") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_Shutdown_Req_t(void* xdrs, CAPI2_SOCKET_Shutdown_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_Shutdown_Req_t")

	if(
		xdr_Int32(xdrs, &rsp->descriptor) &&
		_xdr_UInt8(xdrs, &rsp->how,"how") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_Shutdown_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_Shutdown_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_Shutdown_RSP_Rsp_t")
if(
		_xdr_Int8(xdrs, (char*)&rsp->status,"status") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_Errno_Req_t(void* xdrs, CAPI2_SOCKET_Errno_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_Errno_Req_t")

	if(
		xdr_Int32(xdrs, &rsp->descriptor) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_Errno_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_Errno_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_Errno_RSP_Rsp_t")
if(
		xdr_Int32(xdrs, &rsp->error) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_SO2LONG_Req_t(void* xdrs, CAPI2_SOCKET_SO2LONG_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_SO2LONG_Req_t")

	if(
		xdr_Int32(xdrs, &rsp->socket) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_SO2LONG_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_SO2LONG_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_SO2LONG_RSP_Rsp_t")
if(
		xdr_Int32(xdrs, &rsp->descriptor) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_GetSocketSendBufferSpace_Req_t(void* xdrs, CAPI2_SOCKET_GetSocketSendBufferSpace_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_GetSocketSendBufferSpace_Req_t")

	if(
		xdr_Int32(xdrs, &rsp->bufferSpace) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_GetSocketSendBufferSpace_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_GetSocketSendBufferSpace_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_GetSocketSendBufferSpace_RSP_Rsp_t")
if(
		xdr_Int32(xdrs, &rsp->bufferSpace) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_ParseIPAddr_Req_t(void* xdrs, CAPI2_SOCKET_ParseIPAddr_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_ParseIPAddr_Req_t")

	if(
		xdr_char_ptr_t(xdrs, &rsp->ipString) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SOCKET_ParseIPAddr_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_ParseIPAddr_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SOCKET_ParseIPAddr_RSP_Rsp_t")
if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->ipAddress,sizeof( ip_addr ), xdr_ip_addr) &&
		xdr_unsigned(xdrs, &rsp->subnetBits) &&
		xdr_char_ptr_t(xdrs, &rsp->errorStr) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DC_SetupDataConnection_Req_t(void* xdrs, CAPI2_DC_SetupDataConnection_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DC_SetupDataConnection_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->inClientID,"inClientID") &&
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
		xdr_DC_ConnectionType_t(xdrs, &rsp->linkType) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DC_SetupDataConnection_RSP_Rsp_t(void* xdrs, CAPI2_DC_SetupDataConnection_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DC_SetupDataConnection_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->resultVal,"resultVal") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DC_SetupDataConnectionEx_Req_t(void* xdrs, CAPI2_DC_SetupDataConnectionEx_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DC_SetupDataConnectionEx_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->inClientID,"inClientID") &&
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
		xdr_DC_ConnectionType_t(xdrs, &rsp->linkType) &&
		xdr_uchar_ptr_t(xdrs, &rsp->apnCheck) &&
		xdr_uchar_ptr_t(xdrs, &rsp->actDCAcctId) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DC_SetupDataConnectionEx_RSP_Rsp_t(void* xdrs, CAPI2_DC_SetupDataConnectionEx_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DC_SetupDataConnectionEx_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->resultVal,"resultVal") &&
		_xdr_UInt8(xdrs, &rsp->actDCAcctId,"actDCAcctId") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DC_ReportCallStatusInd_Req_t(void* xdrs, CAPI2_DC_ReportCallStatusInd_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DC_ReportCallStatusInd_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->status,sizeof( DC_ReportCallStatus_t ), xdr_DC_ReportCallStatus_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DC_ShutdownDataConnection_Req_t(void* xdrs, CAPI2_DC_ShutdownDataConnection_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DC_ShutdownDataConnection_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->inClientID,"inClientID") &&
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DC_ShutdownDataConnection_RSP_Rsp_t(void* xdrs, CAPI2_DC_ShutdownDataConnection_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DC_ShutdownDataConnection_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->resultVal,"resultVal") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_IsAcctIDValid_Req_t(void* xdrs, CAPI2_DATA_IsAcctIDValid_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_IsAcctIDValid_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_IsAcctIDValid_RSP_Rsp_t(void* xdrs, CAPI2_DATA_IsAcctIDValid_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_IsAcctIDValid_RSP_Rsp_t")
if(
		_xdr_Boolean(xdrs, &rsp->isValid,"isValid") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_CreateGPRSDataAcct_Req_t(void* xdrs, CAPI2_DATA_CreateGPRSDataAcct_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_CreateGPRSDataAcct_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->pGprsSetting,sizeof( GPRSContext_t ), xdr_GPRSContext_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_CreateGPRSDataAcct_RSP_Rsp_t(void* xdrs, CAPI2_DATA_CreateGPRSDataAcct_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_CreateGPRSDataAcct_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->resultVal,"resultVal") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_CreateCSDDataAcct_Req_t(void* xdrs, CAPI2_DATA_CreateCSDDataAcct_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_CreateCSDDataAcct_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->pCsdSetting,sizeof( CSDContext_t ), xdr_CSDContext_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_CreateCSDDataAcct_RSP_Rsp_t(void* xdrs, CAPI2_DATA_CreateCSDDataAcct_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_CreateCSDDataAcct_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->resultVal,"resultVal") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_DeleteDataAcct_Req_t(void* xdrs, CAPI2_DATA_DeleteDataAcct_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_DeleteDataAcct_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_DeleteDataAcct_RSP_Rsp_t(void* xdrs, CAPI2_DATA_DeleteDataAcct_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_DeleteDataAcct_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->resultVal,"resultVal") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetUsername_Req_t(void* xdrs, CAPI2_DATA_SetUsername_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetUsername_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
		xdr_uchar_ptr_t(xdrs, &rsp->username) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetUsername_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetUsername_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetUsername_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->resultVal,"resultVal") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetUsername_Req_t(void* xdrs, CAPI2_DATA_GetUsername_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetUsername_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetUsername_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetUsername_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetUsername_RSP_Rsp_t")
if(
		xdr_uchar_ptr_t(xdrs, &rsp->username) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetPassword_Req_t(void* xdrs, CAPI2_DATA_SetPassword_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetPassword_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
		xdr_uchar_ptr_t(xdrs, &rsp->password) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetPassword_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetPassword_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetPassword_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->resultVal,"resultVal") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetPassword_Req_t(void* xdrs, CAPI2_DATA_GetPassword_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetPassword_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetPassword_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetPassword_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetPassword_RSP_Rsp_t")
if(
		xdr_uchar_ptr_t(xdrs, &rsp->password) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetStaticIPAddr_Req_t(void* xdrs, CAPI2_DATA_SetStaticIPAddr_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetStaticIPAddr_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
		xdr_uchar_ptr_t(xdrs, &rsp->staticIPAddr) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetStaticIPAddr_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetStaticIPAddr_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetStaticIPAddr_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->resultVal,"resultVal") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetStaticIPAddr_Req_t(void* xdrs, CAPI2_DATA_GetStaticIPAddr_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetStaticIPAddr_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetStaticIPAddr_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetStaticIPAddr_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetStaticIPAddr_RSP_Rsp_t")
if(
		xdr_uchar_ptr_t(xdrs, &rsp->staticIPAddr) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetPrimaryDnsAddr_Req_t(void* xdrs, CAPI2_DATA_SetPrimaryDnsAddr_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetPrimaryDnsAddr_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
		xdr_uchar_ptr_t(xdrs, &rsp->priDnsAddr) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetPrimaryDnsAddr_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetPrimaryDnsAddr_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetPrimaryDnsAddr_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->resultVal,"resultVal") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetPrimaryDnsAddr_Req_t(void* xdrs, CAPI2_DATA_GetPrimaryDnsAddr_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetPrimaryDnsAddr_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetPrimaryDnsAddr_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetPrimaryDnsAddr_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetPrimaryDnsAddr_RSP_Rsp_t")
if(
		xdr_uchar_ptr_t(xdrs, &rsp->priDnsAddr) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetSecondDnsAddr_Req_t(void* xdrs, CAPI2_DATA_SetSecondDnsAddr_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetSecondDnsAddr_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
		xdr_uchar_ptr_t(xdrs, &rsp->sndDnsAddr) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetSecondDnsAddr_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetSecondDnsAddr_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetSecondDnsAddr_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->resultVal,"resultVal") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetSecondDnsAddr_Req_t(void* xdrs, CAPI2_DATA_GetSecondDnsAddr_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetSecondDnsAddr_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetSecondDnsAddr_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetSecondDnsAddr_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetSecondDnsAddr_RSP_Rsp_t")
if(
		xdr_uchar_ptr_t(xdrs, &rsp->sndDnsAddr) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetDataCompression_Req_t(void* xdrs, CAPI2_DATA_SetDataCompression_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetDataCompression_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
		_xdr_Boolean(xdrs, &rsp->dataCompEnable,"dataCompEnable") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetDataCompression_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetDataCompression_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetDataCompression_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->resultVal,"resultVal") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetDataCompression_Req_t(void* xdrs, CAPI2_DATA_GetDataCompression_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetDataCompression_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetDataCompression_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetDataCompression_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetDataCompression_RSP_Rsp_t")
if(
		_xdr_Boolean(xdrs, &rsp->dataCompEnable,"dataCompEnable") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetAcctType_Req_t(void* xdrs, CAPI2_DATA_GetAcctType_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetAcctType_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetAcctType_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetAcctType_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetAcctType_RSP_Rsp_t")
if(
		xdr_DataAccountType_t(xdrs, &rsp->dataAcctType) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetEmptyAcctSlot_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetEmptyAcctSlot_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetEmptyAcctSlot_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->emptySlot,"emptySlot") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetCidFromDataAcctID_Req_t(void* xdrs, CAPI2_DATA_GetCidFromDataAcctID_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetCidFromDataAcctID_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetCidFromDataAcctID_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetCidFromDataAcctID_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetCidFromDataAcctID_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->contextID,"contextID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetDataAcctIDFromCid_Req_t(void* xdrs, CAPI2_DATA_GetDataAcctIDFromCid_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetDataAcctIDFromCid_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->contextID,"contextID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetDataAcctIDFromCid_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetDataAcctIDFromCid_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetDataAcctIDFromCid_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetPrimaryCidFromDataAcctID_Req_t(void* xdrs, CAPI2_DATA_GetPrimaryCidFromDataAcctID_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetPrimaryCidFromDataAcctID_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetPrimaryCidFromDataAcctID_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetPrimaryCidFromDataAcctID_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetPrimaryCidFromDataAcctID_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->priContextID,"priContextID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_IsSecondaryDataAcct_Req_t(void* xdrs, CAPI2_DATA_IsSecondaryDataAcct_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_IsSecondaryDataAcct_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_IsSecondaryDataAcct_RSP_Rsp_t(void* xdrs, CAPI2_DATA_IsSecondaryDataAcct_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_IsSecondaryDataAcct_RSP_Rsp_t")
if(
		_xdr_Boolean(xdrs, &rsp->isSndDataAcct,"isSndDataAcct") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetDataSentSize_Req_t(void* xdrs, CAPI2_DATA_GetDataSentSize_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetDataSentSize_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetDataSentSize_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetDataSentSize_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetDataSentSize_RSP_Rsp_t")
if(
		xdr_UInt32(xdrs, &rsp->dataSentSize) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetDataRcvSize_Req_t(void* xdrs, CAPI2_DATA_GetDataRcvSize_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetDataRcvSize_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetDataRcvSize_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetDataRcvSize_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetDataRcvSize_RSP_Rsp_t")
if(
		xdr_UInt32(xdrs, &rsp->dataRcvSize) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetGPRSPdpType_Req_t(void* xdrs, CAPI2_DATA_SetGPRSPdpType_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetGPRSPdpType_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
		xdr_uchar_ptr_t(xdrs, &rsp->pdpType) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetGPRSPdpType_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetGPRSPdpType_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetGPRSPdpType_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->resultVal,"resultVal") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetGPRSPdpType_Req_t(void* xdrs, CAPI2_DATA_GetGPRSPdpType_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetGPRSPdpType_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetGPRSPdpType_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetGPRSPdpType_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetGPRSPdpType_RSP_Rsp_t")
if(
		xdr_uchar_ptr_t(xdrs, &rsp->pdpType) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetGPRSApn_Req_t(void* xdrs, CAPI2_DATA_SetGPRSApn_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetGPRSApn_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
		xdr_uchar_ptr_t(xdrs, &rsp->apn) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetGPRSApn_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetGPRSApn_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetGPRSApn_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->resultVal,"resultVal") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetGPRSApn_Req_t(void* xdrs, CAPI2_DATA_GetGPRSApn_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetGPRSApn_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetGPRSApn_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetGPRSApn_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetGPRSApn_RSP_Rsp_t")
if(
		xdr_uchar_ptr_t(xdrs, &rsp->apn) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetAuthenMethod_Req_t(void* xdrs, CAPI2_DATA_SetAuthenMethod_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetAuthenMethod_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
		xdr_DataAuthenMethod_t(xdrs, &rsp->authenMethod) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetAuthenMethod_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetAuthenMethod_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetAuthenMethod_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->resultVal,"resultVal") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetAuthenMethod_Req_t(void* xdrs, CAPI2_DATA_GetAuthenMethod_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetAuthenMethod_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetAuthenMethod_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetAuthenMethod_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetAuthenMethod_RSP_Rsp_t")
if(
		xdr_DataAuthenMethod_t(xdrs, &rsp->authenMethod) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetGPRSHeaderCompression_Req_t(void* xdrs, CAPI2_DATA_SetGPRSHeaderCompression_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetGPRSHeaderCompression_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
		_xdr_Boolean(xdrs, &rsp->headerCompEnable,"headerCompEnable") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetGPRSHeaderCompression_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetGPRSHeaderCompression_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetGPRSHeaderCompression_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->resultVal,"resultVal") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetGPRSHeaderCompression_Req_t(void* xdrs, CAPI2_DATA_GetGPRSHeaderCompression_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetGPRSHeaderCompression_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetGPRSHeaderCompression_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetGPRSHeaderCompression_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetGPRSHeaderCompression_RSP_Rsp_t")
if(
		_xdr_Boolean(xdrs, &rsp->headerCompEnable,"headerCompEnable") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetGPRSQos_Req_t(void* xdrs, CAPI2_DATA_SetGPRSQos_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetGPRSQos_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
		xdr_PCHQosProfile_t(xdrs, &rsp->qos) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetGPRSQos_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetGPRSQos_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetGPRSQos_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->resultVal,"resultVal") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetGPRSQos_Req_t(void* xdrs, CAPI2_DATA_GetGPRSQos_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetGPRSQos_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetGPRSQos_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetGPRSQos_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetGPRSQos_RSP_Rsp_t")
if(
		xdr_PCHQosProfile_t(xdrs, &rsp->qos) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetAcctLock_Req_t(void* xdrs, CAPI2_DATA_SetAcctLock_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetAcctLock_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
		_xdr_Boolean(xdrs, &rsp->acctLock,"acctLock") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetAcctLock_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetAcctLock_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetAcctLock_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->resultVal,"resultVal") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetAcctLock_Req_t(void* xdrs, CAPI2_DATA_GetAcctLock_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetAcctLock_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetAcctLock_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetAcctLock_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetAcctLock_RSP_Rsp_t")
if(
		_xdr_Boolean(xdrs, &rsp->acctLock,"acctLock") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetGprsOnly_Req_t(void* xdrs, CAPI2_DATA_SetGprsOnly_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetGprsOnly_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
		_xdr_Boolean(xdrs, &rsp->gprsOnly,"gprsOnly") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetGprsOnly_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetGprsOnly_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetGprsOnly_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->resultVal,"resultVal") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetGprsOnly_Req_t(void* xdrs, CAPI2_DATA_GetGprsOnly_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetGprsOnly_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetGprsOnly_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetGprsOnly_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetGprsOnly_RSP_Rsp_t")
if(
		_xdr_Boolean(xdrs, &rsp->gprsOnly,"gprsOnly") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetGPRSTft_Req_t(void* xdrs, CAPI2_DATA_SetGPRSTft_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetGPRSTft_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->pTft,sizeof( PCHTrafficFlowTemplate_t ), xdr_PCHTrafficFlowTemplate_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetGPRSTft_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetGPRSTft_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetGPRSTft_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->resultVal,"resultVal") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetGPRSTft_Req_t(void* xdrs, CAPI2_DATA_GetGPRSTft_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetGPRSTft_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetGPRSTft_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetGPRSTft_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetGPRSTft_RSP_Rsp_t")
if(
		xdr_CAPI2_DATA_GetGPRSTft_Result_t(xdrs, &rsp->rsp) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetCSDDialNumber_Req_t(void* xdrs, CAPI2_DATA_SetCSDDialNumber_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetCSDDialNumber_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
		xdr_uchar_ptr_t(xdrs, &rsp->dialNumber) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetCSDDialNumber_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetCSDDialNumber_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetCSDDialNumber_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->resultVal,"resultVal") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetCSDDialNumber_Req_t(void* xdrs, CAPI2_DATA_GetCSDDialNumber_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetCSDDialNumber_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetCSDDialNumber_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetCSDDialNumber_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetCSDDialNumber_RSP_Rsp_t")
if(
		xdr_uchar_ptr_t(xdrs, &rsp->dialNumber) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetCSDDialType_Req_t(void* xdrs, CAPI2_DATA_SetCSDDialType_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetCSDDialType_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
		xdr_CSDDialType_t(xdrs, &rsp->csdDialType) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetCSDDialType_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetCSDDialType_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetCSDDialType_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->resultVal,"resultVal") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetCSDDialType_Req_t(void* xdrs, CAPI2_DATA_GetCSDDialType_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetCSDDialType_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetCSDDialType_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetCSDDialType_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetCSDDialType_RSP_Rsp_t")
if(
		xdr_CSDDialType_t(xdrs, &rsp->csdDialType) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetCSDBaudRate_Req_t(void* xdrs, CAPI2_DATA_SetCSDBaudRate_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetCSDBaudRate_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
		xdr_CSDBaudRate_t(xdrs, &rsp->csdBaudRate) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetCSDBaudRate_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetCSDBaudRate_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetCSDBaudRate_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->resultVal,"resultVal") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetCSDBaudRate_Req_t(void* xdrs, CAPI2_DATA_GetCSDBaudRate_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetCSDBaudRate_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetCSDBaudRate_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetCSDBaudRate_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetCSDBaudRate_RSP_Rsp_t")
if(
		xdr_CSDBaudRate_t(xdrs, &rsp->csdBaudRate) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetCSDSyncType_Req_t(void* xdrs, CAPI2_DATA_SetCSDSyncType_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetCSDSyncType_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
		xdr_CSDSyncType_t(xdrs, &rsp->csdSyncType) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetCSDSyncType_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetCSDSyncType_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetCSDSyncType_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->resultVal,"resultVal") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetCSDSyncType_Req_t(void* xdrs, CAPI2_DATA_GetCSDSyncType_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetCSDSyncType_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetCSDSyncType_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetCSDSyncType_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetCSDSyncType_RSP_Rsp_t")
if(
		xdr_CSDSyncType_t(xdrs, &rsp->csdSyncType) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetCSDErrorCorrection_Req_t(void* xdrs, CAPI2_DATA_SetCSDErrorCorrection_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetCSDErrorCorrection_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
		_xdr_Boolean(xdrs, &rsp->enable,"enable") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetCSDErrorCorrection_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetCSDErrorCorrection_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetCSDErrorCorrection_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->resultVal,"resultVal") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetCSDErrorCorrection_Req_t(void* xdrs, CAPI2_DATA_GetCSDErrorCorrection_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetCSDErrorCorrection_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetCSDErrorCorrection_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetCSDErrorCorrection_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetCSDErrorCorrection_RSP_Rsp_t")
if(
		_xdr_Boolean(xdrs, &rsp->enabled,"enabled") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetCSDErrCorrectionType_Req_t(void* xdrs, CAPI2_DATA_SetCSDErrCorrectionType_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetCSDErrCorrectionType_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
		xdr_CSDErrCorrectionType_t(xdrs, &rsp->errCorrectionType) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetCSDErrCorrectionType_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetCSDErrCorrectionType_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetCSDErrCorrectionType_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->resultVal,"resultVal") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetCSDErrCorrectionType_Req_t(void* xdrs, CAPI2_DATA_GetCSDErrCorrectionType_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetCSDErrCorrectionType_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetCSDErrCorrectionType_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetCSDErrCorrectionType_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetCSDErrCorrectionType_RSP_Rsp_t")
if(
		xdr_CSDErrCorrectionType_t(xdrs, &rsp->errCorrectionType) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetCSDDataCompType_Req_t(void* xdrs, CAPI2_DATA_SetCSDDataCompType_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetCSDDataCompType_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
		xdr_CSDDataCompType_t(xdrs, &rsp->dataCompType) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetCSDDataCompType_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetCSDDataCompType_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetCSDDataCompType_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->resultVal,"resultVal") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetCSDDataCompType_Req_t(void* xdrs, CAPI2_DATA_GetCSDDataCompType_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetCSDDataCompType_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetCSDDataCompType_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetCSDDataCompType_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetCSDDataCompType_RSP_Rsp_t")
if(
		xdr_CSDDataCompType_t(xdrs, &rsp->dataCompType) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetCSDConnElement_Req_t(void* xdrs, CAPI2_DATA_SetCSDConnElement_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetCSDConnElement_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
		xdr_CSDConnElement_t(xdrs, &rsp->connElement) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SetCSDConnElement_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetCSDConnElement_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SetCSDConnElement_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->resultVal,"resultVal") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetCSDConnElement_Req_t(void* xdrs, CAPI2_DATA_GetCSDConnElement_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetCSDConnElement_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->acctID,"acctID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_GetCSDConnElement_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetCSDConnElement_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_GetCSDConnElement_RSP_Rsp_t")
if(
		xdr_CSDConnElement_t(xdrs, &rsp->connElement) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_UpdateAccountToFileSystem_RSP_Rsp_t(void* xdrs, CAPI2_DATA_UpdateAccountToFileSystem_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_UpdateAccountToFileSystem_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->resultVal,"resultVal") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_resetDataSize_Req_t(void* xdrs, CAPI2_resetDataSize_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_resetDataSize_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_resetDataSize_RSP_Rsp_t(void* xdrs, CAPI2_resetDataSize_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_resetDataSize_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->resultVal,"resultVal") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_addDataSentSizebyCid_Req_t(void* xdrs, CAPI2_addDataSentSizebyCid_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_addDataSentSizebyCid_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
		xdr_UInt32(xdrs, &rsp->size) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_addDataSentSizebyCid_RSP_Rsp_t(void* xdrs, CAPI2_addDataSentSizebyCid_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_addDataSentSizebyCid_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->resultVal,"resultVal") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_addDataRcvSizebyCid_Req_t(void* xdrs, CAPI2_addDataRcvSizebyCid_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_addDataRcvSizebyCid_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
		xdr_UInt32(xdrs, &rsp->size) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_addDataRcvSizebyCid_RSP_Rsp_t(void* xdrs, CAPI2_addDataRcvSizebyCid_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_addDataRcvSizebyCid_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->resultVal,"resultVal") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PBK_SendUsimHdkReadReq_Rsp_t(void* xdrs, CAPI2_PBK_SendUsimHdkReadReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PBK_SendUsimHdkReadReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( USIM_PBK_READ_HDK_ENTRY_RSP_t ), xdr_USIM_PBK_READ_HDK_ENTRY_RSP_t);
}

bool_t xdr_CAPI2_PBK_SendUsimHdkUpdateReq_Req_t(void* xdrs, CAPI2_PBK_SendUsimHdkUpdateReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PBK_SendUsimHdkUpdateReq_Req_t")

	if(
		xdr_HDKString_t(xdrs, &rsp->hidden_key) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PBK_SendUsimHdkUpdateReq_Rsp_t(void* xdrs, CAPI2_PBK_SendUsimHdkUpdateReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PBK_SendUsimHdkUpdateReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( USIM_PBK_UPDATE_HDK_ENTRY_RSP_t ), xdr_USIM_PBK_UPDATE_HDK_ENTRY_RSP_t);
}

bool_t xdr_CAPI2_PBK_SendUsimAasReadReq_Req_t(void* xdrs, CAPI2_PBK_SendUsimAasReadReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PBK_SendUsimAasReadReq_Req_t")

	if(
		_xdr_UInt16(xdrs, &rsp->index,"index") &&
		xdr_PBK_Id_t(xdrs, &rsp->pbk_id) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PBK_SendUsimAasReadReq_Rsp_t(void* xdrs, CAPI2_PBK_SendUsimAasReadReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PBK_SendUsimAasReadReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( USIM_PBK_READ_ALPHA_ENTRY_RSP_t ), xdr_USIM_PBK_READ_ALPHA_ENTRY_RSP_t);
}

bool_t xdr_CAPI2_PBK_SendUsimAasUpdateReq_Req_t(void* xdrs, CAPI2_PBK_SendUsimAasUpdateReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PBK_SendUsimAasUpdateReq_Req_t")

	if(
		_xdr_UInt16(xdrs, &rsp->index,"index") &&
		xdr_PBK_Id_t(xdrs, &rsp->pbk_id) &&
		xdr_ALPHA_CODING_t(xdrs, &rsp->alpha_coding) &&
		_xdr_UInt8(xdrs, &rsp->alpha_size,"alpha_size") &&
		xdr_uchar_ptr_t(xdrs, &rsp->alpha) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PBK_SendUsimAasUpdateReq_Rsp_t(void* xdrs, CAPI2_PBK_SendUsimAasUpdateReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PBK_SendUsimAasUpdateReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( USIM_PBK_UPDATE_ALPHA_ENTRY_RSP_t ), xdr_USIM_PBK_UPDATE_ALPHA_ENTRY_RSP_t);
}

bool_t xdr_CAPI2_PBK_SendUsimGasReadReq_Req_t(void* xdrs, CAPI2_PBK_SendUsimGasReadReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PBK_SendUsimGasReadReq_Req_t")

	if(
		_xdr_UInt16(xdrs, &rsp->index,"index") &&
		xdr_PBK_Id_t(xdrs, &rsp->pbk_id) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PBK_SendUsimGasReadReq_Rsp_t(void* xdrs, CAPI2_PBK_SendUsimGasReadReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PBK_SendUsimGasReadReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( USIM_PBK_READ_ALPHA_ENTRY_RSP_t ), xdr_USIM_PBK_READ_ALPHA_ENTRY_RSP_t);
}

bool_t xdr_CAPI2_PBK_SendUsimGasUpdateReq_Req_t(void* xdrs, CAPI2_PBK_SendUsimGasUpdateReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PBK_SendUsimGasUpdateReq_Req_t")

	if(
		_xdr_UInt16(xdrs, &rsp->index,"index") &&
		xdr_PBK_Id_t(xdrs, &rsp->pbk_id) &&
		xdr_ALPHA_CODING_t(xdrs, &rsp->alpha_coding) &&
		_xdr_UInt8(xdrs, &rsp->alpha_size,"alpha_size") &&
		xdr_uchar_ptr_t(xdrs, &rsp->alpha) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PBK_SendUsimGasUpdateReq_Rsp_t(void* xdrs, CAPI2_PBK_SendUsimGasUpdateReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PBK_SendUsimGasUpdateReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( USIM_PBK_UPDATE_ALPHA_ENTRY_RSP_t ), xdr_USIM_PBK_UPDATE_ALPHA_ENTRY_RSP_t);
}

bool_t xdr_CAPI2_PBK_SendUsimAasInfoReq_Req_t(void* xdrs, CAPI2_PBK_SendUsimAasInfoReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PBK_SendUsimAasInfoReq_Req_t")

	if(
		xdr_PBK_Id_t(xdrs, &rsp->pbk_id) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PBK_SendUsimAasInfoReq_Rsp_t(void* xdrs, CAPI2_PBK_SendUsimAasInfoReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PBK_SendUsimAasInfoReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( USIM_PBK_ALPHA_INFO_RSP_t ), xdr_USIM_PBK_ALPHA_INFO_RSP_t);
}

bool_t xdr_CAPI2_PBK_SendUsimGasInfoReq_Req_t(void* xdrs, CAPI2_PBK_SendUsimGasInfoReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PBK_SendUsimGasInfoReq_Req_t")

	if(
		xdr_PBK_Id_t(xdrs, &rsp->pbk_id) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PBK_SendUsimGasInfoReq_Rsp_t(void* xdrs, CAPI2_PBK_SendUsimGasInfoReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PBK_SendUsimGasInfoReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( USIM_PBK_ALPHA_INFO_RSP_t ), xdr_USIM_PBK_ALPHA_INFO_RSP_t);
}

bool_t xdr_CAPI2_LCS_PosReqResultInd_Rsp_t(void* xdrs, CAPI2_LCS_PosReqResultInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_PosReqResultInd_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( LcsPosReqResult_t ), xdr_LcsPosReqResult_t);
}

bool_t xdr_CAPI2_LCS_PosInfoInd_Rsp_t(void* xdrs, CAPI2_LCS_PosInfoInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_PosInfoInd_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( LcsPosInfo_t ), xdr_LcsPosInfo_t);
}

bool_t xdr_CAPI2_LCS_PositionDetailInd_Rsp_t(void* xdrs, CAPI2_LCS_PositionDetailInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_PositionDetailInd_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( LcsPosDetail_t ), xdr_LcsPosDetail_t);
}

bool_t xdr_CAPI2_LCS_NmeaReadyInd_Rsp_t(void* xdrs, CAPI2_LCS_NmeaReadyInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_NmeaReadyInd_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( LcsNmeaData_t ), xdr_LcsNmeaData_t);
}

bool_t xdr_CAPI2_LCS_ServiceControl_Req_t(void* xdrs, CAPI2_LCS_ServiceControl_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_ServiceControl_Req_t")

	if(
		xdr_LcsServiceType_t(xdrs, &rsp->inServiceType) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LCS_ServiceControl_Rsp_t(void* xdrs, CAPI2_LCS_ServiceControl_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_ServiceControl_Rsp_t")

	 return xdr_LcsResult_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_LCS_PowerConfig_Req_t(void* xdrs, CAPI2_LCS_PowerConfig_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_PowerConfig_Req_t")

	if(
		xdr_LcsPowerState_t(xdrs, &rsp->inPowerSate) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LCS_PowerConfig_Rsp_t(void* xdrs, CAPI2_LCS_PowerConfig_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_PowerConfig_Rsp_t")

	 return xdr_LcsResult_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_LCS_ServiceQuery_Rsp_t(void* xdrs, CAPI2_LCS_ServiceQuery_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_ServiceQuery_Rsp_t")

	 return xdr_LcsServiceType_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_LCS_StartPosReqPeriodic_Rsp_t(void* xdrs, CAPI2_LCS_StartPosReqPeriodic_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_StartPosReqPeriodic_Rsp_t")

	 return xdr_LcsHandle_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_LCS_StartPosReqSingle_Rsp_t(void* xdrs, CAPI2_LCS_StartPosReqSingle_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_StartPosReqSingle_Rsp_t")

	 return xdr_LcsHandle_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_LCS_StopPosReq_Req_t(void* xdrs, CAPI2_LCS_StopPosReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_StopPosReq_Req_t")

	if(
		xdr_LcsHandle_t(xdrs, &rsp->inLcsHandle) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LCS_StopPosReq_Rsp_t(void* xdrs, CAPI2_LCS_StopPosReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_StopPosReq_Rsp_t")

	 return xdr_LcsResult_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_LCS_GetPosition_Rsp_t(void* xdrs, CAPI2_LCS_GetPosition_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_GetPosition_Rsp_t")

	 return xdr_LcsPosData_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_LCS_ConfigSet_Req_t(void* xdrs, CAPI2_LCS_ConfigSet_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_ConfigSet_Req_t")

	if(
		xdr_UInt32(xdrs, &rsp->configId) &&
		xdr_UInt32(xdrs, &rsp->value) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LCS_ConfigGet_Req_t(void* xdrs, CAPI2_LCS_ConfigGet_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_ConfigGet_Req_t")

	if(
		xdr_UInt32(xdrs, &rsp->configId) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LCS_ConfigGet_Rsp_t(void* xdrs, CAPI2_LCS_ConfigGet_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_ConfigGet_Rsp_t")

	 return xdr_UInt32(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_LCS_SuplVerificationRsp_Req_t(void* xdrs, CAPI2_LCS_SuplVerificationRsp_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_SuplVerificationRsp_Req_t")

	if(
		xdr_LcsSuplSessionHdl_t(xdrs, &rsp->inSessionHdl) &&
		_xdr_Boolean(xdrs, &rsp->inIsAllowed,"inIsAllowed") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LCS_SuplConnectRsp_Req_t(void* xdrs, CAPI2_LCS_SuplConnectRsp_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_SuplConnectRsp_Req_t")

	if(
		xdr_LcsSuplSessionHdl_t(xdrs, &rsp->inSessionHdl) &&
		xdr_LcsSuplConnectHdl_t(xdrs, &rsp->inConnectHdl) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LCS_SuplConnectRsp_Rsp_t(void* xdrs, CAPI2_LCS_SuplConnectRsp_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_SuplConnectRsp_Rsp_t")

	 return xdr_LcsResult_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_LCS_SuplDisconnected_Req_t(void* xdrs, CAPI2_LCS_SuplDisconnected_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_SuplDisconnected_Req_t")

	if(
		xdr_LcsSuplSessionHdl_t(xdrs, &rsp->inSessionHdl) &&
		xdr_LcsSuplConnectHdl_t(xdrs, &rsp->inConnectHdl) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LCS_SuplInitHmacReq_Rsp_t(void* xdrs, CAPI2_LCS_SuplInitHmacReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_SuplInitHmacReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( LcsSuplData_t ), xdr_LcsSuplData_t);
}

bool_t xdr_CAPI2_LCS_SuplConnectReq_Rsp_t(void* xdrs, CAPI2_LCS_SuplConnectReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_SuplConnectReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( LcsSuplConnection_t ), xdr_LcsSuplConnection_t);
}

bool_t xdr_CAPI2_LCS_SuplDisconnectReq_Rsp_t(void* xdrs, CAPI2_LCS_SuplDisconnectReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_SuplDisconnectReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( LcsSuplSessionInfo_t ), xdr_LcsSuplSessionInfo_t);
}

bool_t xdr_CAPI2_LCS_SuplNotificationInd_Rsp_t(void* xdrs, CAPI2_LCS_SuplNotificationInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_SuplNotificationInd_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( LcsSuplNotificationData_t ), xdr_LcsSuplNotificationData_t);
}

bool_t xdr_CAPI2_LCS_SuplWriteReq_Rsp_t(void* xdrs, CAPI2_LCS_SuplWriteReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_SuplWriteReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( LcsSuplCommData_t ), xdr_LcsSuplCommData_t);
}

bool_t xdr_CAPI2_LCS_CmdData_Req_t(void* xdrs, CAPI2_LCS_CmdData_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_CmdData_Req_t")

	if(
		xdr_CAPI2_LcsCmdData_t(xdrs, &rsp->inCmdData) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_GPIO_ConfigOutput_64Pin_Req_t(void* xdrs, CAPI2_GPIO_ConfigOutput_64Pin_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_GPIO_ConfigOutput_64Pin_Req_t")

	if(
		xdr_UInt32(xdrs, &rsp->pin) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_GPIO_ConfigOutput_64Pin_RSP_Rsp_t(void* xdrs, CAPI2_GPIO_ConfigOutput_64Pin_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_GPIO_ConfigOutput_64Pin_RSP_Rsp_t")
if(
		_xdr_Boolean(xdrs, &rsp->status,"status") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_GPS_Control_Req_t(void* xdrs, CAPI2_GPS_Control_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_GPS_Control_Req_t")

	if(
		xdr_UInt32(xdrs, &rsp->u32Cmnd) &&
		xdr_UInt32(xdrs, &rsp->u32Param0) &&
		xdr_UInt32(xdrs, &rsp->u32Param1) &&
		xdr_UInt32(xdrs, &rsp->u32Param2) &&
		xdr_UInt32(xdrs, &rsp->u32Param3) &&
		xdr_UInt32(xdrs, &rsp->u32Param4) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_GPS_Control_RSP_Rsp_t(void* xdrs, CAPI2_GPS_Control_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_GPS_Control_RSP_Rsp_t")
if(
		xdr_UInt32(xdrs, &rsp->u32Param) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_FFS_Control_Req_t(void* xdrs, CAPI2_FFS_Control_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_FFS_Control_Req_t")

	if(
		xdr_UInt32(xdrs, &rsp->cmd) &&
		xdr_UInt32(xdrs, &rsp->address) &&
		xdr_UInt32(xdrs, &rsp->offset) &&
		xdr_UInt32(xdrs, &rsp->size) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_FFS_Control_RSP_Rsp_t(void* xdrs, CAPI2_FFS_Control_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_FFS_Control_RSP_Rsp_t")
if(
		xdr_UInt32(xdrs, &rsp->param) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CP2AP_PedestalMode_Control_Req_t(void* xdrs, CAPI2_CP2AP_PedestalMode_Control_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CP2AP_PedestalMode_Control_Req_t")

	if(
		xdr_UInt32(xdrs, &rsp->enable) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CP2AP_PedestalMode_Control_RSP_Rsp_t(void* xdrs, CAPI2_CP2AP_PedestalMode_Control_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CP2AP_PedestalMode_Control_RSP_Rsp_t")
if(
		_xdr_Boolean(xdrs, &rsp->status,"status") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_AUDIO_ASIC_SetAudioMode_Req_t(void* xdrs, CAPI2_AUDIO_ASIC_SetAudioMode_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_AUDIO_ASIC_SetAudioMode_Req_t")

	if(
		xdr_AudioMode_t(xdrs, &rsp->mode) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SPEAKER_StartTone_Req_t(void* xdrs, CAPI2_SPEAKER_StartTone_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SPEAKER_StartTone_Req_t")

	if(
		xdr_SpeakerTone_t(xdrs, &rsp->tone) &&
		_xdr_UInt8(xdrs, &rsp->duration,"duration") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SPEAKER_StartGenericTone_Req_t(void* xdrs, CAPI2_SPEAKER_StartGenericTone_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SPEAKER_StartGenericTone_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->superimpose,"superimpose") &&
		_xdr_UInt16(xdrs, &rsp->tone_duration,"tone_duration") &&
		_xdr_UInt16(xdrs, &rsp->f1,"f1") &&
		_xdr_UInt16(xdrs, &rsp->f2,"f2") &&
		_xdr_UInt16(xdrs, &rsp->f3,"f3") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_AUDIO_Turn_EC_NS_OnOff_Req_t(void* xdrs, CAPI2_AUDIO_Turn_EC_NS_OnOff_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_AUDIO_Turn_EC_NS_OnOff_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->ec_on_off,"ec_on_off") &&
		_xdr_Boolean(xdrs, &rsp->ns_on_off,"ns_on_off") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_ECHO_SetDigitalTxGain_Req_t(void* xdrs, CAPI2_ECHO_SetDigitalTxGain_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_ECHO_SetDigitalTxGain_Req_t")

	if(
		xdr_int16_t(xdrs, &rsp->digital_gain_step) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_RIPCMDQ_Connect_Uplink_Req_t(void* xdrs, CAPI2_RIPCMDQ_Connect_Uplink_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_RIPCMDQ_Connect_Uplink_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->Uplink,"Uplink") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_RIPCMDQ_Connect_Downlink_Req_t(void* xdrs, CAPI2_RIPCMDQ_Connect_Downlink_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_RIPCMDQ_Connect_Downlink_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->Downlink,"Downlink") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_VOLUMECTRL_SetBasebandVolume_Req_t(void* xdrs, CAPI2_VOLUMECTRL_SetBasebandVolume_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_VOLUMECTRL_SetBasebandVolume_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->level,"level") &&
		_xdr_UInt8(xdrs, &rsp->chnl,"chnl") &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->audio_atten,sizeof( UInt16 ), xdr_UInt16) &&
		_xdr_UInt8(xdrs, &rsp->extid,"extid") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_MeasureReportInd_Rsp_t(void* xdrs, CAPI2_MS_MeasureReportInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_MeasureReportInd_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( MS_RxTestParam_t ), xdr_MS_RxTestParam_t);
}

bool_t xdr_CAPI2_DIAG_ApiMeasurmentReportReq_Req_t(void* xdrs, CAPI2_DIAG_ApiMeasurmentReportReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DIAG_ApiMeasurmentReportReq_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->inPeriodicReport,"inPeriodicReport") &&
		xdr_UInt32(xdrs, &rsp->inTimeInterval) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PMU_BattChargingNotification_Req_t(void* xdrs, CAPI2_PMU_BattChargingNotification_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PMU_BattChargingNotification_Req_t")

	if(
		xdr_HAL_EM_BATTMGR_Charger_t(xdrs, &rsp->chargeType) &&
		xdr_HAL_EM_BATTMGR_Charger_InOut_t(xdrs, &rsp->inOut) &&
		_xdr_UInt8(xdrs, &rsp->status,"status") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PATCH_GetRevision_Rsp_t(void* xdrs, CAPI2_PATCH_GetRevision_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PATCH_GetRevision_Rsp_t")

	 return xdr_CAPI2_Patch_Revision_Ptr_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_RTC_SetTime_Req_t(void* xdrs, CAPI2_RTC_SetTime_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_RTC_SetTime_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->inTime,sizeof( RTCTime_t ), xdr_RTCTime_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_RTC_SetTime_RSP_Rsp_t(void* xdrs, CAPI2_RTC_SetTime_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_RTC_SetTime_RSP_Rsp_t")
if(
		_xdr_Boolean(xdrs, &rsp->status,"status") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_RTC_SetDST_Req_t(void* xdrs, CAPI2_RTC_SetDST_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_RTC_SetDST_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->inDST,"inDST") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_RTC_SetDST_RSP_Rsp_t(void* xdrs, CAPI2_RTC_SetDST_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_RTC_SetDST_RSP_Rsp_t")
if(
		_xdr_Boolean(xdrs, &rsp->status,"status") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_RTC_SetTimeZone_Req_t(void* xdrs, CAPI2_RTC_SetTimeZone_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_RTC_SetTimeZone_Req_t")

	if(
		_xdr_Int8(xdrs, (char*)&rsp->inTimezone,"inTimezone") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_RTC_SetTimeZone_RSP_Rsp_t(void* xdrs, CAPI2_RTC_SetTimeZone_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_RTC_SetTimeZone_RSP_Rsp_t")
if(
		_xdr_Boolean(xdrs, &rsp->status,"status") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_RTC_GetTime_Req_t(void* xdrs, CAPI2_RTC_GetTime_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_RTC_GetTime_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->time,sizeof( RTCTime_t ), xdr_RTCTime_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_RTC_GetTime_RSP_Rsp_t(void* xdrs, CAPI2_RTC_GetTime_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_RTC_GetTime_RSP_Rsp_t")
if(
		xdr_RTCTime_t(xdrs, &rsp->time) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_RTC_GetTimeZone_RSP_Rsp_t(void* xdrs, CAPI2_RTC_GetTimeZone_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_RTC_GetTimeZone_RSP_Rsp_t")
if(
		_xdr_Int8(xdrs, (char*)&rsp->timeZone,"timeZone") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_RTC_GetDST_RSP_Rsp_t(void* xdrs, CAPI2_RTC_GetDST_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_RTC_GetDST_RSP_Rsp_t")
if(
		_xdr_UInt8(xdrs, &rsp->dst,"dst") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SMS_GetMeSmsBufferStatus_Req_t(void* xdrs, CAPI2_SMS_GetMeSmsBufferStatus_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_GetMeSmsBufferStatus_Req_t")

	if(
		_xdr_UInt16(xdrs, &rsp->cmd,"cmd") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SMS_GetMeSmsBufferStatus_RSP_Rsp_t(void* xdrs, CAPI2_SMS_GetMeSmsBufferStatus_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_GetMeSmsBufferStatus_RSP_Rsp_t")
if(
		xdr_UInt32(xdrs, &rsp->bfree) &&
		xdr_UInt32(xdrs, &rsp->bused) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SMS_GetRecordNumberOfReplaceSMS_Req_t(void* xdrs, CAPI2_SMS_GetRecordNumberOfReplaceSMS_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_GetRecordNumberOfReplaceSMS_Req_t")

	if(
		xdr_SmsStorage_t(xdrs, &rsp->storageType) &&
		_xdr_UInt8(xdrs, &rsp->tp_pid,"tp_pid") &&
		xdr_uchar_ptr_t(xdrs, &rsp->oaddress) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SMS_GetRecordNumberOfReplaceSMS_RSP_Rsp_t(void* xdrs, CAPI2_SMS_GetRecordNumberOfReplaceSMS_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_GetRecordNumberOfReplaceSMS_RSP_Rsp_t")
if(
		_xdr_UInt16(xdrs, &rsp->recordNumber,"recordNumber") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_FLASH_SaveImage_Req_t(void* xdrs, CAPI2_FLASH_SaveImage_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_FLASH_SaveImage_Req_t")

	if(
		xdr_UInt32(xdrs, &rsp->flash_addr) &&
		xdr_UInt32(xdrs, &rsp->length) &&
		xdr_UInt32(xdrs, &rsp->shared_mem_addr) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_FLASH_SaveImage_RSP_Rsp_t(void* xdrs, CAPI2_FLASH_SaveImage_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_FLASH_SaveImage_RSP_Rsp_t")
if(
		_xdr_Boolean(xdrs, &rsp->status,"status") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_AUDIO_GetSettings_Req_t(void* xdrs, CAPI2_AUDIO_GetSettings_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_AUDIO_GetSettings_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->audioMode,"audioMode") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_AUDIO_GetSettings_Rsp_t(void* xdrs, CAPI2_AUDIO_GetSettings_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_AUDIO_GetSettings_Rsp_t")

	 return xdr_Capi2AudioParams_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIMLOCK_GetStatus_Req_t(void* xdrs, CAPI2_SIMLOCK_GetStatus_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIMLOCK_GetStatus_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->sim_data,sizeof( SIMLOCK_SIM_DATA_t ), xdr_SIMLOCK_SIM_DATA_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIMLOCK_GetStatus_RSP_Rsp_t(void* xdrs, CAPI2_SIMLOCK_GetStatus_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIMLOCK_GetStatus_RSP_Rsp_t")
if(
		xdr_SIMLOCK_STATE_t(xdrs, &rsp->simlock_state) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIMLOCK_SetStatus_Req_t(void* xdrs, CAPI2_SIMLOCK_SetStatus_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIMLOCK_SetStatus_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->simlock_state,sizeof( SIMLOCK_STATE_t ), xdr_SIMLOCK_STATE_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYSPARM_GetActualLowVoltReading_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetActualLowVoltReading_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_GetActualLowVoltReading_Rsp_t")

	 return _xdr_UInt16(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_SYSPARM_GetActual4p2VoltReading_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetActual4p2VoltReading_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_GetActual4p2VoltReading_Rsp_t")

	 return _xdr_UInt16(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_program_equalizer_Req_t(void* xdrs, CAPI2_program_equalizer_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_program_equalizer_Req_t")

	if(
		xdr_Int32(xdrs, &rsp->equalizer) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_program_poly_equalizer_Req_t(void* xdrs, CAPI2_program_poly_equalizer_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_program_poly_equalizer_Req_t")

	if(
		xdr_Int32(xdrs, &rsp->equalizer) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_program_FIR_IIR_filter_Req_t(void* xdrs, CAPI2_program_FIR_IIR_filter_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_program_FIR_IIR_filter_Req_t")

	if(
		_xdr_UInt16(xdrs, &rsp->audio_mode,"audio_mode") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_program_poly_FIR_IIR_filter_Req_t(void* xdrs, CAPI2_program_poly_FIR_IIR_filter_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_program_poly_FIR_IIR_filter_Req_t")

	if(
		_xdr_UInt16(xdrs, &rsp->audio_mode,"audio_mode") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_audio_control_generic_Req_t(void* xdrs, CAPI2_audio_control_generic_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_audio_control_generic_Req_t")

	if(
		xdr_UInt32(xdrs, &rsp->param1) &&
		xdr_UInt32(xdrs, &rsp->param2) &&
		xdr_UInt32(xdrs, &rsp->param3) &&
		xdr_UInt32(xdrs, &rsp->param4) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_audio_control_generic_Rsp_t(void* xdrs, CAPI2_audio_control_generic_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_audio_control_generic_Rsp_t")

	 return xdr_UInt32(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_audio_control_dsp_Req_t(void* xdrs, CAPI2_audio_control_dsp_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_audio_control_dsp_Req_t")

	if(
		xdr_UInt32(xdrs, &rsp->param1) &&
		xdr_UInt32(xdrs, &rsp->param2) &&
		xdr_UInt32(xdrs, &rsp->param3) &&
		xdr_UInt32(xdrs, &rsp->param4) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_audio_control_dsp_Rsp_t(void* xdrs, CAPI2_audio_control_dsp_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_audio_control_dsp_Rsp_t")

	 return xdr_UInt32(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_MS_Ue3gStatusInd_Rsp_t(void* xdrs, CAPI2_MS_Ue3gStatusInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_Ue3gStatusInd_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( MSUe3gStatusInd_t ), xdr_MSUe3gStatusInd_t);
}

bool_t xdr_CAPI2_MS_CellInfoInd_Rsp_t(void* xdrs, CAPI2_MS_CellInfoInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_CellInfoInd_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( MSCellInfoInd_t ), xdr_MSCellInfoInd_t);
}

bool_t xdr_CAPI2_FFS_Read_Req_t(void* xdrs, CAPI2_FFS_Read_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_FFS_Read_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->ffsReadReq,sizeof( FFS_ReadReq_t ), xdr_FFS_ReadReq_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_FFS_Read_RSP_Rsp_t(void* xdrs, CAPI2_FFS_Read_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_FFS_Read_RSP_Rsp_t")
if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->ffsReadRsp,sizeof( FFS_Data_t ), xdr_FFS_Data_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DIAG_ApiCellLockReq_Req_t(void* xdrs, CAPI2_DIAG_ApiCellLockReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DIAG_ApiCellLockReq_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->cell_lockEnable,"cell_lockEnable") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DIAG_ApiCellLockStatus_Rsp_t(void* xdrs, CAPI2_DIAG_ApiCellLockStatus_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DIAG_ApiCellLockStatus_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_LCS_RegisterRrlpDataHandler_Req_t(void* xdrs, CAPI2_LCS_RegisterRrlpDataHandler_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_RegisterRrlpDataHandler_Req_t")

	if(
		xdr_LcsClientInfo_t(xdrs, &rsp->inRrlpHandler) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LCS_RrlpDataInd_Rsp_t(void* xdrs, CAPI2_LCS_RrlpDataInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_RrlpDataInd_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( LcsMsgData_t ), xdr_LcsMsgData_t);
}

bool_t xdr_CAPI2_LCS_RrlpReset_Rsp_t(void* xdrs, CAPI2_LCS_RrlpReset_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_RrlpReset_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( LcsClientInfo_t ), xdr_LcsClientInfo_t);
}

bool_t xdr_CAPI2_LCS_RegisterRrcDataHandler_Req_t(void* xdrs, CAPI2_LCS_RegisterRrcDataHandler_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_RegisterRrcDataHandler_Req_t")

	if(
		xdr_LcsClientInfo_t(xdrs, &rsp->inRrcHandler) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LCS_RrcAssistDataInd_Rsp_t(void* xdrs, CAPI2_LCS_RrcAssistDataInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_RrcAssistDataInd_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( LcsMsgData_t ), xdr_LcsMsgData_t);
}

bool_t xdr_CAPI2_LCS_RrcMeasCtrlInd_Rsp_t(void* xdrs, CAPI2_LCS_RrcMeasCtrlInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_RrcMeasCtrlInd_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( LcsRrcMeasurement_t ), xdr_LcsRrcMeasurement_t);
}

bool_t xdr_CAPI2_LCS_RrcSysInfoInd_Rsp_t(void* xdrs, CAPI2_LCS_RrcSysInfoInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_RrcSysInfoInd_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( LcsRrcBroadcastSysInfo_t ), xdr_LcsRrcBroadcastSysInfo_t);
}

bool_t xdr_CAPI2_LCS_RrcUeStateInd_Rsp_t(void* xdrs, CAPI2_LCS_RrcUeStateInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_RrcUeStateInd_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( LcsRrcUeState_t ), xdr_LcsRrcUeState_t);
}

bool_t xdr_CAPI2_LCS_RrcStopMeasInd_Rsp_t(void* xdrs, CAPI2_LCS_RrcStopMeasInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_RrcStopMeasInd_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( LcsClientInfo_t ), xdr_LcsClientInfo_t);
}

bool_t xdr_CAPI2_LCS_RrcReset_Rsp_t(void* xdrs, CAPI2_LCS_RrcReset_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_RrcReset_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( LcsClientInfo_t ), xdr_LcsClientInfo_t);
}

bool_t xdr_CAPI2_CC_IsThereEmergencyCall_Rsp_t(void* xdrs, CAPI2_CC_IsThereEmergencyCall_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CC_IsThereEmergencyCall_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_SYSPARM_GetBattLowThresh_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetBattLowThresh_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_GetBattLowThresh_Rsp_t")

	 return _xdr_UInt16(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_SYSPARM_GetDefault4p2VoltReading_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetDefault4p2VoltReading_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_GetDefault4p2VoltReading_Rsp_t")

	 return _xdr_UInt16(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_L1_bb_isLocked_Req_t(void* xdrs, CAPI2_L1_bb_isLocked_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_L1_bb_isLocked_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->watch,"watch") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_L1_bb_isLocked_Rsp_t(void* xdrs, CAPI2_L1_bb_isLocked_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_L1_bb_isLocked_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_ServingCellInfoInd_Rsp_t(void* xdrs, CAPI2_ServingCellInfoInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_ServingCellInfoInd_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( ServingCellInfo_t ), xdr_ServingCellInfo_t);
}

bool_t xdr_CAPI2_SMSPP_AppSpecificInd_Rsp_t(void* xdrs, CAPI2_SMSPP_AppSpecificInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMSPP_AppSpecificInd_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SmsAppSpecificData_t ), xdr_SmsAppSpecificData_t);
}

bool_t xdr_CAPI2_SYSPARM_GetGPIO_Value_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetGPIO_Value_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_GetGPIO_Value_Rsp_t")

	 return xdr_Capi2GpioValue_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SYS_EnableCellInfoMsg_Req_t(void* xdrs, CAPI2_SYS_EnableCellInfoMsg_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYS_EnableCellInfoMsg_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->inEnableCellInfoMsg,"inEnableCellInfoMsg") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYSPARM_SetHSDPAPHYCategory_Req_t(void* xdrs, CAPI2_SYSPARM_SetHSDPAPHYCategory_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_SetHSDPAPHYCategory_Req_t")

	if(
		xdr_UInt32(xdrs, &rsp->inHsdpaPhyCat) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYSPARM_GetHSDPAPHYCategory_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetHSDPAPHYCategory_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_GetHSDPAPHYCategory_Rsp_t")

	 return xdr_UInt32(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SIM_SendWriteSmsReq_Req_t(void* xdrs, CAPI2_SIM_SendWriteSmsReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendWriteSmsReq_Req_t")

	if(
		_xdr_UInt16(xdrs, &rsp->rec_no,"rec_no") &&
		_xdr_UInt8(xdrs, &rsp->tp_pid,"tp_pid") &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->p_sms_mesg,sizeof( SIMSMSMesg_t ), xdr_SIMSMSMesg_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIM_SendWriteSmsReq_Rsp_t(void* xdrs, CAPI2_SIM_SendWriteSmsReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendWriteSmsReq_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_SMS_UPDATE_RESULT_t ), xdr_SIM_SMS_UPDATE_RESULT_t);
}

bool_t xdr_CAPI2_BATTMGR_GetChargingStatus_Rsp_t(void* xdrs, CAPI2_BATTMGR_GetChargingStatus_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_BATTMGR_GetChargingStatus_Rsp_t")

	 return xdr_EM_BATTMGR_ChargingStatus_en_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_BATTMGR_GetPercentageLevel_Rsp_t(void* xdrs, CAPI2_BATTMGR_GetPercentageLevel_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_BATTMGR_GetPercentageLevel_Rsp_t")

	 return _xdr_UInt16(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_BATTMGR_IsBatteryPresent_Rsp_t(void* xdrs, CAPI2_BATTMGR_IsBatteryPresent_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_BATTMGR_IsBatteryPresent_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_BATTMGR_IsChargerPlugIn_Rsp_t(void* xdrs, CAPI2_BATTMGR_IsChargerPlugIn_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_BATTMGR_IsChargerPlugIn_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_SYSPARM_GetBattTable_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetBattTable_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_GetBattTable_Rsp_t")

	 return xdr_Batt_Level_Table_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_BATTMGR_GetLevel_Rsp_t(void* xdrs, CAPI2_BATTMGR_GetLevel_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_BATTMGR_GetLevel_Rsp_t")

	 return _xdr_UInt16(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_AT_ProcessCmdToAP_Req_t(void* xdrs, CAPI2_AT_ProcessCmdToAP_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_AT_ProcessCmdToAP_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->channel,"channel") &&
		xdr_uchar_ptr_t(xdrs, &rsp->cmdStr) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_ADCMGR_MultiChStart_Req_t(void* xdrs, CAPI2_ADCMGR_MultiChStart_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_ADCMGR_MultiChStart_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->ch,sizeof( CAPI2_ADC_ChannelReq_t ), xdr_CAPI2_ADC_ChannelReq_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_ADCMGR_MultiChStart_Rsp_t(void* xdrs, CAPI2_ADCMGR_MultiChStart_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_ADCMGR_MultiChStart_Rsp_t")

	 return xdr_CAPI2_ADC_ChannelRsp_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_InterTaskMsgToCP_Req_t(void* xdrs, CAPI2_InterTaskMsgToCP_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_InterTaskMsgToCP_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->inPtrMsg,sizeof( InterTaskMsg_t ), xdr_InterTaskMsg_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_InterTaskMsgToAP_Req_t(void* xdrs, CAPI2_InterTaskMsgToAP_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_InterTaskMsgToAP_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->inPtrMsg,sizeof( InterTaskMsg_t ), xdr_InterTaskMsg_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}
