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


/***************************** Generated contents from ../gen/capi2_msg_inc.txt file. ( Do not modify !!! ) Please checkout and modify ../gen/capi2_msg_inc.txt to add any header files *************************/

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
#include "capi2_common_xdr.h"


/********************************** End Hdr file generated contents ************************************************************/






static bool_t xdr_copy_len(u_int* destLen, u_int srcLen)
{
	*destLen = srcLen;
	return TRUE;
}


bool_t xdr_CAPI2_MS_IsGSMRegistered_Rsp_t(void* xdrs, CAPI2_MS_IsGSMRegistered_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_IsGSMRegistered_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_IsGPRSRegistered_Rsp_t(void* xdrs, CAPI2_MS_IsGPRSRegistered_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_IsGPRSRegistered_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_GetGSMRegCause_Rsp_t(void* xdrs, CAPI2_MS_GetGSMRegCause_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetGSMRegCause_Rsp_t")

	if(
		 xdr_NetworkCause_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_GetGPRSRegCause_Rsp_t(void* xdrs, CAPI2_MS_GetGPRSRegCause_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetGPRSRegCause_Rsp_t")

	if(
		 xdr_NetworkCause_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_GetRegisteredLAC_Rsp_t(void* xdrs, CAPI2_MS_GetRegisteredLAC_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetRegisteredLAC_Rsp_t")

	if(
		 _xdr_UInt16(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_GetPlmnMCC_Rsp_t(void* xdrs, CAPI2_MS_GetPlmnMCC_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetPlmnMCC_Rsp_t")

	if(
		 _xdr_UInt16(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_GetPlmnMNC_Rsp_t(void* xdrs, CAPI2_MS_GetPlmnMNC_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetPlmnMNC_Rsp_t")

	if(
		 _xdr_UInt8(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_IsGprsAllowed_Rsp_t(void* xdrs, CAPI2_MS_IsGprsAllowed_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_IsGprsAllowed_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_GetCurrentRAT_Rsp_t(void* xdrs, CAPI2_MS_GetCurrentRAT_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetCurrentRAT_Rsp_t")

	if(
		 _xdr_UInt8(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_GetCurrentBand_Rsp_t(void* xdrs, CAPI2_MS_GetCurrentBand_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetCurrentBand_Rsp_t")

	if(
		 _xdr_UInt8(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_UpdateSMSCapExceededFlag_Req_t(void* xdrs, CAPI2_SimApi_UpdateSMSCapExceededFlag_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_UpdateSMSCapExceededFlag_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->cap_exceeded,"cap_exceeded") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_NetRegApi_SelectBand_Req_t(void* xdrs, CAPI2_NetRegApi_SelectBand_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_NetRegApi_SelectBand_Req_t")

	if(
		xdr_BandSelect_t(xdrs, &rsp->bandSelect) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_NetRegApi_SetSupportedRATandBand_Req_t(void* xdrs, CAPI2_NetRegApi_SetSupportedRATandBand_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_NetRegApi_SetSupportedRATandBand_Req_t")

	if(
		xdr_RATSelect_t(xdrs, &rsp->RAT_cap) &&
		xdr_BandSelect_t(xdrs, &rsp->band_cap) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_NetRegApi_CellLock_Req_t(void* xdrs, CAPI2_NetRegApi_CellLock_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_NetRegApi_CellLock_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->Enable,"Enable") &&
		xdr_BandSelect_t(xdrs, &rsp->lockBand) &&
		_xdr_UInt8(xdrs, &rsp->lockrat,"lockrat") &&
		_xdr_UInt16(xdrs, &rsp->lockuarfcn,"lockuarfcn") &&
		_xdr_UInt16(xdrs, &rsp->lockpsc,"lockpsc") &&
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
	u_int len;

	XDR_LOG(xdrs,"CAPI2_PLMN_GetCountryByMcc_Rsp_t")

	if(
		xdr_copy_len(&len, 10 ) && xdr_bytes(xdrs, (char **)(void*)&rsp->val, &len , ~0) &&
	1)
		return TRUE;
	else
		return FALSE;
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

	if(
		_xdr_UInt16(xdrs, &rsp->plmn_mcc,"plmn_mcc") &&
		_xdr_UInt16(xdrs, &rsp->plmn_mnc,"plmn_mnc") &&
		xdr_PLMN_NAME_t(xdrs, &rsp->long_name) &&
		xdr_PLMN_NAME_t(xdrs, &rsp->short_name) &&
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_GetPLMNListSize_Rsp_t(void* xdrs, CAPI2_MS_GetPLMNListSize_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetPLMNListSize_Rsp_t")

	if(
		 _xdr_UInt16(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
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

	if(
		xdr_PLMN_NAME_t(xdrs, &rsp->long_name) &&
		xdr_PLMN_NAME_t(xdrs, &rsp->short_name) &&
		xdr_char_ptr_t(xdrs, &rsp->country_name) &&
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_NetRegApi_PlmnSelect_Req_t(void* xdrs, CAPI2_NetRegApi_PlmnSelect_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_NetRegApi_PlmnSelect_Req_t")

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

bool_t xdr_CAPI2_NetRegApi_PlmnSelect_Rsp_t(void* xdrs, CAPI2_NetRegApi_PlmnSelect_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_NetRegApi_PlmnSelect_Rsp_t")

	if(
		_xdr_Boolean(xdrs, &rsp->netReqSent,"netReqSent") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_GetPlmnMode_Rsp_t(void* xdrs, CAPI2_MS_GetPlmnMode_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetPlmnMode_Rsp_t")

	if(
		 xdr_PlmnSelectMode_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_NetRegApi_SetPlmnMode_Req_t(void* xdrs, CAPI2_NetRegApi_SetPlmnMode_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_NetRegApi_SetPlmnMode_Req_t")

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

	if(
		 xdr_PlmnSelectFormat_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
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

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_NetRegApi_GetPLMNNameByCode_Req_t(void* xdrs, CAPI2_NetRegApi_GetPLMNNameByCode_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_NetRegApi_GetPLMNNameByCode_Req_t")

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

bool_t xdr_CAPI2_NetRegApi_GetPLMNNameByCode_Rsp_t(void* xdrs, CAPI2_NetRegApi_GetPLMNNameByCode_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_NetRegApi_GetPLMNNameByCode_Rsp_t")

	if(
		xdr_PLMN_NAME_t(xdrs, &rsp->long_name) &&
		xdr_PLMN_NAME_t(xdrs, &rsp->short_name) &&
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PhoneCtrlApi_GetSystemState_Rsp_t(void* xdrs, CAPI2_PhoneCtrlApi_GetSystemState_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PhoneCtrlApi_GetSystemState_Rsp_t")

	if(
		 xdr_SystemState_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PhoneCtrlApi_SetSystemState_Req_t(void* xdrs, CAPI2_PhoneCtrlApi_SetSystemState_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PhoneCtrlApi_SetSystemState_Req_t")

	if(
		xdr_SystemState_t(xdrs, &rsp->state) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PhoneCtrlApi_GetRxSignalInfo_Rsp_t(void* xdrs, CAPI2_PhoneCtrlApi_GetRxSignalInfo_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PhoneCtrlApi_GetRxSignalInfo_Rsp_t")

	if(
		_xdr_UInt8(xdrs, &rsp->RxLev,"RxLev") &&
		_xdr_UInt8(xdrs, &rsp->RxQual,"RxQual") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYS_GetGSMRegistrationStatus_Rsp_t(void* xdrs, CAPI2_SYS_GetGSMRegistrationStatus_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYS_GetGSMRegistrationStatus_Rsp_t")

	if(
		 xdr_RegisterStatus_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYS_GetGPRSRegistrationStatus_Rsp_t(void* xdrs, CAPI2_SYS_GetGPRSRegistrationStatus_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYS_GetGPRSRegistrationStatus_Rsp_t")

	if(
		 xdr_RegisterStatus_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYS_IsRegisteredGSMOrGPRS_Rsp_t(void* xdrs, CAPI2_SYS_IsRegisteredGSMOrGPRS_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYS_IsRegisteredGSMOrGPRS_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_IsRegisteredHomePLMN_Rsp_t(void* xdrs, CAPI2_MS_IsRegisteredHomePLMN_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_IsRegisteredHomePLMN_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PhoneCtrlApi_SetPowerDownTimer_Req_t(void* xdrs, CAPI2_PhoneCtrlApi_SetPowerDownTimer_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PhoneCtrlApi_SetPowerDownTimer_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->powerDownTimer,"powerDownTimer") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_GetSmsParamRecNum_Rsp_t(void* xdrs, CAPI2_SimApi_GetSmsParamRecNum_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_GetSmsParamRecNum_Rsp_t")

	if(
		_xdr_UInt8(xdrs, &rsp->sms_param_rec_no,"sms_param_rec_no") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_GetSmsMemExceededFlag_Rsp_t(void* xdrs, CAPI2_SimApi_GetSmsMemExceededFlag_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_GetSmsMemExceededFlag_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_IsTestSIM_Rsp_t(void* xdrs, CAPI2_SimApi_IsTestSIM_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_IsTestSIM_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_IsPINRequired_Rsp_t(void* xdrs, CAPI2_SimApi_IsPINRequired_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_IsPINRequired_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_GetCardPhase_Rsp_t(void* xdrs, CAPI2_SimApi_GetCardPhase_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_GetCardPhase_Rsp_t")

	if(
		 xdr_SIMPhase_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_GetSIMType_Rsp_t(void* xdrs, CAPI2_SimApi_GetSIMType_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_GetSIMType_Rsp_t")

	if(
		 xdr_SIMType_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_GetPresentStatus_Rsp_t(void* xdrs, CAPI2_SimApi_GetPresentStatus_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_GetPresentStatus_Rsp_t")

	if(
		 xdr_SIMPresent_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_IsOperationRestricted_Rsp_t(void* xdrs, CAPI2_SimApi_IsOperationRestricted_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_IsOperationRestricted_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_IsPINBlocked_Req_t(void* xdrs, CAPI2_SimApi_IsPINBlocked_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_IsPINBlocked_Req_t")

	if(
		xdr_CHV_t(xdrs, &rsp->chv) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_IsPINBlocked_Rsp_t(void* xdrs, CAPI2_SimApi_IsPINBlocked_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_IsPINBlocked_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_IsPUKBlocked_Req_t(void* xdrs, CAPI2_SimApi_IsPUKBlocked_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_IsPUKBlocked_Req_t")

	if(
		xdr_CHV_t(xdrs, &rsp->chv) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_IsPUKBlocked_Rsp_t(void* xdrs, CAPI2_SimApi_IsPUKBlocked_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_IsPUKBlocked_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_IsInvalidSIM_Rsp_t(void* xdrs, CAPI2_SimApi_IsInvalidSIM_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_IsInvalidSIM_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_DetectSim_Rsp_t(void* xdrs, CAPI2_SimApi_DetectSim_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_DetectSim_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_GetRuimSuppFlag_Rsp_t(void* xdrs, CAPI2_SimApi_GetRuimSuppFlag_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_GetRuimSuppFlag_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendVerifyChvReq_Req_t(void* xdrs, CAPI2_SimApi_SendVerifyChvReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendVerifyChvReq_Req_t")

	if(
		xdr_CHV_t(xdrs, &rsp->chv_select) &&
		xdr_CHVString_t(xdrs, &rsp->chv) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendVerifyChvReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendVerifyChvReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendVerifyChvReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SimPinRsp_t ), xdr_SimPinRsp_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendChangeChvReq_Req_t(void* xdrs, CAPI2_SimApi_SendChangeChvReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendChangeChvReq_Req_t")

	if(
		xdr_CHV_t(xdrs, &rsp->chv_select) &&
		xdr_CHVString_t(xdrs, &rsp->old_chv) &&
		xdr_CHVString_t(xdrs, &rsp->new_chv) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendChangeChvReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendChangeChvReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendChangeChvReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SimPinRsp_t ), xdr_SimPinRsp_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendSetChv1OnOffReq_Req_t(void* xdrs, CAPI2_SimApi_SendSetChv1OnOffReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendSetChv1OnOffReq_Req_t")

	if(
		xdr_CHVString_t(xdrs, &rsp->chv) &&
		_xdr_Boolean(xdrs, &rsp->enable_flag,"enable_flag") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendSetChv1OnOffReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendSetChv1OnOffReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendSetChv1OnOffReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SimPinRsp_t ), xdr_SimPinRsp_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendUnblockChvReq_Req_t(void* xdrs, CAPI2_SimApi_SendUnblockChvReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendUnblockChvReq_Req_t")

	if(
		xdr_CHV_t(xdrs, &rsp->chv_select) &&
		xdr_PUKString_t(xdrs, &rsp->puk) &&
		xdr_CHVString_t(xdrs, &rsp->new_chv) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendUnblockChvReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendUnblockChvReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendUnblockChvReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SimPinRsp_t ), xdr_SimPinRsp_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendSetOperStateReq_Req_t(void* xdrs, CAPI2_SimApi_SendSetOperStateReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendSetOperStateReq_Req_t")

	if(
		xdr_SIMOperState_t(xdrs, &rsp->oper_state) &&
		xdr_CHVString_t(xdrs, &rsp->chv2) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendSetOperStateReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendSetOperStateReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendSetOperStateReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIMAccess_t ), xdr_SIMAccess_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_IsPbkAccessAllowed_Req_t(void* xdrs, CAPI2_SimApi_IsPbkAccessAllowed_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_IsPbkAccessAllowed_Req_t")

	if(
		xdr_SIMPBK_ID_t(xdrs, &rsp->id) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_IsPbkAccessAllowed_Rsp_t(void* xdrs, CAPI2_SimApi_IsPbkAccessAllowed_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_IsPbkAccessAllowed_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendPbkInfoReq_Req_t(void* xdrs, CAPI2_SimApi_SendPbkInfoReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendPbkInfoReq_Req_t")

	if(
		xdr_SIMPBK_ID_t(xdrs, &rsp->id) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendPbkInfoReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendPbkInfoReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendPbkInfoReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_PBK_INFO_t ), xdr_SIM_PBK_INFO_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendReadAcmMaxReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendReadAcmMaxReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendReadAcmMaxReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_MAX_ACM_t ), xdr_SIM_MAX_ACM_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendWriteAcmMaxReq_Req_t(void* xdrs, CAPI2_SimApi_SendWriteAcmMaxReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendWriteAcmMaxReq_Req_t")

	if(
		xdr_CallMeterUnit_t(xdrs, &rsp->acm_max) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendWriteAcmMaxReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendWriteAcmMaxReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendWriteAcmMaxReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIMAccess_t ), xdr_SIMAccess_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendReadAcmReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendReadAcmReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendReadAcmReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_ACM_VALUE_t ), xdr_SIM_ACM_VALUE_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendWriteAcmReq_Req_t(void* xdrs, CAPI2_SimApi_SendWriteAcmReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendWriteAcmReq_Req_t")

	if(
		xdr_CallMeterUnit_t(xdrs, &rsp->acm) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendWriteAcmReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendWriteAcmReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendWriteAcmReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIMAccess_t ), xdr_SIMAccess_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendIncreaseAcmReq_Req_t(void* xdrs, CAPI2_SimApi_SendIncreaseAcmReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendIncreaseAcmReq_Req_t")

	if(
		xdr_CallMeterUnit_t(xdrs, &rsp->acm) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendIncreaseAcmReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendIncreaseAcmReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendIncreaseAcmReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIMAccess_t ), xdr_SIMAccess_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendReadSvcProvNameReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendReadSvcProvNameReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendReadSvcProvNameReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_SVC_PROV_NAME_t ), xdr_SIM_SVC_PROV_NAME_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendReadPuctReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendReadPuctReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendReadPuctReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_PUCT_DATA_t ), xdr_SIM_PUCT_DATA_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_GetServiceStatus_Req_t(void* xdrs, CAPI2_SimApi_GetServiceStatus_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_GetServiceStatus_Req_t")

	if(
		xdr_SIMService_t(xdrs, &rsp->service) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_GetServiceStatus_Rsp_t(void* xdrs, CAPI2_SimApi_GetServiceStatus_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_GetServiceStatus_Rsp_t")

	if(
		 xdr_SIMServiceStatus_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_GetPinStatus_Rsp_t(void* xdrs, CAPI2_SimApi_GetPinStatus_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_GetPinStatus_Rsp_t")

	if(
		 xdr_SIM_PIN_Status_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_IsPinOK_Rsp_t(void* xdrs, CAPI2_SimApi_IsPinOK_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_IsPinOK_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_GetIMSI_Rsp_t(void* xdrs, CAPI2_SimApi_GetIMSI_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_GetIMSI_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( IMSI_t ), xdr_IMSI_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_GetGID1_Rsp_t(void* xdrs, CAPI2_SimApi_GetGID1_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_GetGID1_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( GID_DIGIT_t ), xdr_GID_DIGIT_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_GetGID2_Rsp_t(void* xdrs, CAPI2_SimApi_GetGID2_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_GetGID2_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( GID_DIGIT_t ), xdr_GID_DIGIT_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_GetHomePlmn_Rsp_t(void* xdrs, CAPI2_SimApi_GetHomePlmn_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_GetHomePlmn_Rsp_t")

	if(
		xdr_PLMNId_t(xdrs, &rsp->home_plmn) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_simmiApi_GetMasterFileId_Req_t(void* xdrs, CAPI2_simmiApi_GetMasterFileId_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_simmiApi_GetMasterFileId_Req_t")

	if(
		xdr_APDUFileID_t(xdrs, &rsp->file_id) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_simmiApi_GetMasterFileId_Rsp_t(void* xdrs, CAPI2_simmiApi_GetMasterFileId_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_simmiApi_GetMasterFileId_Rsp_t")

	if(
		 xdr_APDUFileID_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendOpenSocketReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendOpenSocketReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendOpenSocketReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_OPEN_SOCKET_RES_t ), xdr_SIM_OPEN_SOCKET_RES_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendSelectAppiReq_Req_t(void* xdrs, CAPI2_SimApi_SendSelectAppiReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SimApi_SendSelectAppiReq_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->socket_id,"socket_id") &&
		xdr_UInt8(xdrs, &rsp->aid_len) &&
		xdr_copy_len(&len, (u_int)(rsp->aid_len)) && xdr_bytes(xdrs, (char **)(void*)&rsp->aid_data, &len, 0xFFFF) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendSelectAppiReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendSelectAppiReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendSelectAppiReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_SELECT_APPLI_RES_t ), xdr_SIM_SELECT_APPLI_RES_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendDeactivateAppiReq_Req_t(void* xdrs, CAPI2_SimApi_SendDeactivateAppiReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendDeactivateAppiReq_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->socket_id,"socket_id") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendDeactivateAppiReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendDeactivateAppiReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendDeactivateAppiReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_DEACTIVATE_APPLI_RES_t ), xdr_SIM_DEACTIVATE_APPLI_RES_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendCloseSocketReq_Req_t(void* xdrs, CAPI2_SimApi_SendCloseSocketReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendCloseSocketReq_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->socket_id,"socket_id") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendCloseSocketReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendCloseSocketReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendCloseSocketReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_CLOSE_SOCKET_RES_t ), xdr_SIM_CLOSE_SOCKET_RES_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_GetAtrData_Rsp_t(void* xdrs, CAPI2_SimApi_GetAtrData_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_GetAtrData_Rsp_t")

	if(
		xdr_APDU_t(xdrs, &rsp->atr_data) &&
		 xdr_SIMAccess_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SubmitDFileInfoReq_Req_t(void* xdrs, CAPI2_SimApi_SubmitDFileInfoReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SimApi_SubmitDFileInfoReq_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->socket_id,"socket_id") &&
		xdr_APDUFileID_t(xdrs, &rsp->dfile_id) &&
		_xdr_UInt8(xdrs, &rsp->path_len,"path_len") &&
		xdr_copy_len(&len, (u_int)(rsp->path_len)) && xdr_array(xdrs, (char **)(void*)&rsp->select_path, &len, ~0, sizeof( UInt16 ),(xdrproc_t) xdr_UInt16) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SubmitDFileInfoReq_Rsp_t(void* xdrs, CAPI2_SimApi_SubmitDFileInfoReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SubmitDFileInfoReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_DFILE_INFO_t ), xdr_SIM_DFILE_INFO_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SubmitEFileInfoReq_Req_t(void* xdrs, CAPI2_SimApi_SubmitEFileInfoReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SimApi_SubmitEFileInfoReq_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->socket_id,"socket_id") &&
		xdr_APDUFileID_t(xdrs, &rsp->efile_id) &&
		xdr_APDUFileID_t(xdrs, &rsp->dfile_id) &&
		_xdr_UInt8(xdrs, &rsp->path_len,"path_len") &&
		xdr_copy_len(&len, (u_int)(rsp->path_len)) && xdr_array(xdrs, (char **)(void*)&rsp->select_path, &len, ~0, sizeof( UInt16 ),(xdrproc_t) xdr_UInt16) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SubmitEFileInfoReq_Rsp_t(void* xdrs, CAPI2_SimApi_SubmitEFileInfoReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SubmitEFileInfoReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_EFILE_INFO_t ), xdr_SIM_EFILE_INFO_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendEFileInfoReq_Req_t(void* xdrs, CAPI2_SimApi_SendEFileInfoReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SimApi_SendEFileInfoReq_Req_t")

	if(
		xdr_APDUFileID_t(xdrs, &rsp->efile_id) &&
		xdr_APDUFileID_t(xdrs, &rsp->dfile_id) &&
		_xdr_UInt8(xdrs, &rsp->path_len,"path_len") &&
		xdr_copy_len(&len, (u_int)(rsp->path_len)) && xdr_array(xdrs, (char **)(void*)&rsp->select_path, &len, ~0, sizeof( UInt16 ),(xdrproc_t) xdr_UInt16) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendDFileInfoReq_Req_t(void* xdrs, CAPI2_SimApi_SendDFileInfoReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SimApi_SendDFileInfoReq_Req_t")

	if(
		xdr_APDUFileID_t(xdrs, &rsp->dfile_id) &&
		_xdr_UInt8(xdrs, &rsp->path_len,"path_len") &&
		xdr_copy_len(&len, (u_int)(rsp->path_len)) && xdr_array(xdrs, (char **)(void*)&rsp->select_path, &len, ~0, sizeof( UInt16 ),(xdrproc_t) xdr_UInt16) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SubmitWholeBinaryEFileReadReq_Req_t(void* xdrs, CAPI2_SimApi_SubmitWholeBinaryEFileReadReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SimApi_SubmitWholeBinaryEFileReadReq_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->socket_id,"socket_id") &&
		xdr_APDUFileID_t(xdrs, &rsp->efile_id) &&
		xdr_APDUFileID_t(xdrs, &rsp->dfile_id) &&
		_xdr_UInt8(xdrs, &rsp->path_len,"path_len") &&
		xdr_copy_len(&len, (u_int)(rsp->path_len)) && xdr_array(xdrs, (char **)(void*)&rsp->select_path, &len, ~0, sizeof( UInt16 ),(xdrproc_t) xdr_UInt16) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SubmitWholeBinaryEFileReadReq_Rsp_t(void* xdrs, CAPI2_SimApi_SubmitWholeBinaryEFileReadReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SubmitWholeBinaryEFileReadReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_EFILE_DATA_t ), xdr_SIM_EFILE_DATA_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendWholeBinaryEFileReadReq_Req_t(void* xdrs, CAPI2_SimApi_SendWholeBinaryEFileReadReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SimApi_SendWholeBinaryEFileReadReq_Req_t")

	if(
		xdr_APDUFileID_t(xdrs, &rsp->efile_id) &&
		xdr_APDUFileID_t(xdrs, &rsp->dfile_id) &&
		_xdr_UInt8(xdrs, &rsp->path_len,"path_len") &&
		xdr_copy_len(&len, (u_int)(rsp->path_len)) && xdr_array(xdrs, (char **)(void*)&rsp->select_path, &len, ~0, sizeof( UInt16 ),(xdrproc_t) xdr_UInt16) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SubmitBinaryEFileReadReq_Req_t(void* xdrs, CAPI2_SimApi_SubmitBinaryEFileReadReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SimApi_SubmitBinaryEFileReadReq_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->socket_id,"socket_id") &&
		xdr_APDUFileID_t(xdrs, &rsp->efile_id) &&
		xdr_APDUFileID_t(xdrs, &rsp->dfile_id) &&
		_xdr_UInt16(xdrs, &rsp->offset,"offset") &&
		_xdr_UInt16(xdrs, &rsp->length,"length") &&
		_xdr_UInt8(xdrs, &rsp->path_len,"path_len") &&
		xdr_copy_len(&len, (u_int)(rsp->path_len)) && xdr_array(xdrs, (char **)(void*)&rsp->select_path, &len, ~0, sizeof( UInt16 ),(xdrproc_t) xdr_UInt16) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendBinaryEFileReadReq_Req_t(void* xdrs, CAPI2_SimApi_SendBinaryEFileReadReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SimApi_SendBinaryEFileReadReq_Req_t")

	if(
		xdr_APDUFileID_t(xdrs, &rsp->efile_id) &&
		xdr_APDUFileID_t(xdrs, &rsp->dfile_id) &&
		_xdr_UInt16(xdrs, &rsp->offset,"offset") &&
		_xdr_UInt16(xdrs, &rsp->length,"length") &&
		_xdr_UInt8(xdrs, &rsp->path_len,"path_len") &&
		xdr_copy_len(&len, (u_int)(rsp->path_len)) && xdr_array(xdrs, (char **)(void*)&rsp->select_path, &len, ~0, sizeof( UInt16 ),(xdrproc_t) xdr_UInt16) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SubmitRecordEFileReadReq_Req_t(void* xdrs, CAPI2_SimApi_SubmitRecordEFileReadReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SimApi_SubmitRecordEFileReadReq_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->socket_id,"socket_id") &&
		xdr_APDUFileID_t(xdrs, &rsp->efile_id) &&
		xdr_APDUFileID_t(xdrs, &rsp->dfile_id) &&
		_xdr_UInt8(xdrs, &rsp->rec_no,"rec_no") &&
		_xdr_UInt8(xdrs, &rsp->length,"length") &&
		_xdr_UInt8(xdrs, &rsp->path_len,"path_len") &&
		xdr_copy_len(&len, (u_int)(rsp->path_len)) && xdr_array(xdrs, (char **)(void*)&rsp->select_path, &len, ~0, sizeof( UInt16 ),(xdrproc_t) xdr_UInt16) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendRecordEFileReadReq_Req_t(void* xdrs, CAPI2_SimApi_SendRecordEFileReadReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SimApi_SendRecordEFileReadReq_Req_t")

	if(
		xdr_APDUFileID_t(xdrs, &rsp->efile_id) &&
		xdr_APDUFileID_t(xdrs, &rsp->dfile_id) &&
		_xdr_UInt8(xdrs, &rsp->rec_no,"rec_no") &&
		_xdr_UInt8(xdrs, &rsp->length,"length") &&
		_xdr_UInt8(xdrs, &rsp->path_len,"path_len") &&
		xdr_copy_len(&len, (u_int)(rsp->path_len)) && xdr_array(xdrs, (char **)(void*)&rsp->select_path, &len, ~0, sizeof( UInt16 ),(xdrproc_t) xdr_UInt16) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SubmitBinaryEFileUpdateReq_Req_t(void* xdrs, CAPI2_SimApi_SubmitBinaryEFileUpdateReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SimApi_SubmitBinaryEFileUpdateReq_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->socket_id,"socket_id") &&
		xdr_APDUFileID_t(xdrs, &rsp->efile_id) &&
		xdr_APDUFileID_t(xdrs, &rsp->dfile_id) &&
		_xdr_UInt16(xdrs, &rsp->offset,"offset") &&
		xdr_UInt16(xdrs, &rsp->length) &&
		xdr_copy_len(&len, (u_int)(rsp->length)) && xdr_bytes(xdrs, (char **)(void*)&rsp->ptr, &len, 0xFFFF) &&
		_xdr_UInt8(xdrs, &rsp->path_len,"path_len") &&
		xdr_copy_len(&len, (u_int)(rsp->path_len)) && xdr_array(xdrs, (char **)(void*)&rsp->select_path, &len, ~0, sizeof( UInt16 ),(xdrproc_t) xdr_UInt16) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SubmitBinaryEFileUpdateReq_Rsp_t(void* xdrs, CAPI2_SimApi_SubmitBinaryEFileUpdateReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SubmitBinaryEFileUpdateReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_EFILE_UPDATE_RESULT_t ), xdr_SIM_EFILE_UPDATE_RESULT_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendBinaryEFileUpdateReq_Req_t(void* xdrs, CAPI2_SimApi_SendBinaryEFileUpdateReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SimApi_SendBinaryEFileUpdateReq_Req_t")

	if(
		xdr_APDUFileID_t(xdrs, &rsp->efile_id) &&
		xdr_APDUFileID_t(xdrs, &rsp->dfile_id) &&
		_xdr_UInt16(xdrs, &rsp->offset,"offset") &&
		xdr_UInt16(xdrs, &rsp->length) &&
		xdr_copy_len(&len, (u_int)(rsp->length)) && xdr_bytes(xdrs, (char **)(void*)&rsp->ptr, &len, 0xFFFF) &&
		_xdr_UInt8(xdrs, &rsp->path_len,"path_len") &&
		xdr_copy_len(&len, (u_int)(rsp->path_len)) && xdr_array(xdrs, (char **)(void*)&rsp->select_path, &len, ~0, sizeof( UInt16 ),(xdrproc_t) xdr_UInt16) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SubmitLinearEFileUpdateReq_Req_t(void* xdrs, CAPI2_SimApi_SubmitLinearEFileUpdateReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SimApi_SubmitLinearEFileUpdateReq_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->socket_id,"socket_id") &&
		xdr_APDUFileID_t(xdrs, &rsp->efile_id) &&
		xdr_APDUFileID_t(xdrs, &rsp->dfile_id) &&
		_xdr_UInt8(xdrs, &rsp->rec_no,"rec_no") &&
		xdr_UInt8(xdrs, &rsp->length) &&
		xdr_copy_len(&len, (u_int)(rsp->length)) && xdr_bytes(xdrs, (char **)(void*)&rsp->ptr, &len, 0xFFFF) &&
		_xdr_UInt8(xdrs, &rsp->path_len,"path_len") &&
		xdr_copy_len(&len, (u_int)(rsp->path_len)) && xdr_array(xdrs, (char **)(void*)&rsp->select_path, &len, ~0, sizeof( UInt16 ),(xdrproc_t) xdr_UInt16) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendLinearEFileUpdateReq_Req_t(void* xdrs, CAPI2_SimApi_SendLinearEFileUpdateReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SimApi_SendLinearEFileUpdateReq_Req_t")

	if(
		xdr_APDUFileID_t(xdrs, &rsp->efile_id) &&
		xdr_APDUFileID_t(xdrs, &rsp->dfile_id) &&
		_xdr_UInt8(xdrs, &rsp->rec_no,"rec_no") &&
		xdr_UInt8(xdrs, &rsp->length) &&
		xdr_copy_len(&len, (u_int)(rsp->length)) && xdr_bytes(xdrs, (char **)(void*)&rsp->ptr, &len, 0xFFFF) &&
		_xdr_UInt8(xdrs, &rsp->path_len,"path_len") &&
		xdr_copy_len(&len, (u_int)(rsp->path_len)) && xdr_array(xdrs, (char **)(void*)&rsp->select_path, &len, ~0, sizeof( UInt16 ),(xdrproc_t) xdr_UInt16) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SubmitSeekRecordReq_Req_t(void* xdrs, CAPI2_SimApi_SubmitSeekRecordReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SimApi_SubmitSeekRecordReq_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->socket_id,"socket_id") &&
		xdr_APDUFileID_t(xdrs, &rsp->efile_id) &&
		xdr_APDUFileID_t(xdrs, &rsp->dfile_id) &&
		xdr_UInt8(xdrs, &rsp->length) &&
		xdr_copy_len(&len, (u_int)(rsp->length)) && xdr_bytes(xdrs, (char **)(void*)&rsp->ptr, &len, 0xFFFF) &&
		_xdr_UInt8(xdrs, &rsp->path_len,"path_len") &&
		xdr_copy_len(&len, (u_int)(rsp->path_len)) && xdr_array(xdrs, (char **)(void*)&rsp->select_path, &len, ~0, sizeof( UInt16 ),(xdrproc_t) xdr_UInt16) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SubmitSeekRecordReq_Rsp_t(void* xdrs, CAPI2_SimApi_SubmitSeekRecordReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SubmitSeekRecordReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_SEEK_RECORD_DATA_t ), xdr_SIM_SEEK_RECORD_DATA_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendSeekRecordReq_Req_t(void* xdrs, CAPI2_SimApi_SendSeekRecordReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SimApi_SendSeekRecordReq_Req_t")

	if(
		xdr_APDUFileID_t(xdrs, &rsp->efile_id) &&
		xdr_APDUFileID_t(xdrs, &rsp->dfile_id) &&
		xdr_UInt8(xdrs, &rsp->length) &&
		xdr_copy_len(&len, (u_int)(rsp->length)) && xdr_bytes(xdrs, (char **)(void*)&rsp->ptr, &len, 0xFFFF) &&
		_xdr_UInt8(xdrs, &rsp->path_len,"path_len") &&
		xdr_copy_len(&len, (u_int)(rsp->path_len)) && xdr_array(xdrs, (char **)(void*)&rsp->select_path, &len, ~0, sizeof( UInt16 ),(xdrproc_t) xdr_UInt16) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SubmitCyclicEFileUpdateReq_Req_t(void* xdrs, CAPI2_SimApi_SubmitCyclicEFileUpdateReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SimApi_SubmitCyclicEFileUpdateReq_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->socket_id,"socket_id") &&
		xdr_APDUFileID_t(xdrs, &rsp->efile_id) &&
		xdr_APDUFileID_t(xdrs, &rsp->dfile_id) &&
		xdr_UInt8(xdrs, &rsp->length) &&
		xdr_copy_len(&len, (u_int)(rsp->length)) && xdr_bytes(xdrs, (char **)(void*)&rsp->ptr, &len, 0xFFFF) &&
		_xdr_UInt8(xdrs, &rsp->path_len,"path_len") &&
		xdr_copy_len(&len, (u_int)(rsp->path_len)) && xdr_array(xdrs, (char **)(void*)&rsp->select_path, &len, ~0, sizeof( UInt16 ),(xdrproc_t) xdr_UInt16) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendCyclicEFileUpdateReq_Req_t(void* xdrs, CAPI2_SimApi_SendCyclicEFileUpdateReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SimApi_SendCyclicEFileUpdateReq_Req_t")

	if(
		xdr_APDUFileID_t(xdrs, &rsp->efile_id) &&
		xdr_APDUFileID_t(xdrs, &rsp->dfile_id) &&
		xdr_UInt8(xdrs, &rsp->length) &&
		xdr_copy_len(&len, (u_int)(rsp->length)) && xdr_bytes(xdrs, (char **)(void*)&rsp->ptr, &len, 0xFFFF) &&
		_xdr_UInt8(xdrs, &rsp->path_len,"path_len") &&
		xdr_copy_len(&len, (u_int)(rsp->path_len)) && xdr_array(xdrs, (char **)(void*)&rsp->select_path, &len, ~0, sizeof( UInt16 ),(xdrproc_t) xdr_UInt16) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendRemainingPinAttemptReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendRemainingPinAttemptReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendRemainingPinAttemptReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( PIN_ATTEMPT_RESULT_t ), xdr_PIN_ATTEMPT_RESULT_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_IsCachedDataReady_Rsp_t(void* xdrs, CAPI2_SimApi_IsCachedDataReady_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_IsCachedDataReady_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_GetServiceCodeStatus_Req_t(void* xdrs, CAPI2_SimApi_GetServiceCodeStatus_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_GetServiceCodeStatus_Req_t")

	if(
		xdr_SERVICE_CODE_STATUS_CPHS_t(xdrs, &rsp->service_code) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_GetServiceCodeStatus_Rsp_t(void* xdrs, CAPI2_SimApi_GetServiceCodeStatus_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_GetServiceCodeStatus_Rsp_t")

	if(
		 xdr_SERVICE_FLAG_STATUS_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_CheckCphsService_Req_t(void* xdrs, CAPI2_SimApi_CheckCphsService_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_CheckCphsService_Req_t")

	if(
		xdr_CPHS_SST_ENTRY_t(xdrs, &rsp->sst_entry) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_CheckCphsService_Rsp_t(void* xdrs, CAPI2_SimApi_CheckCphsService_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_CheckCphsService_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_GetCphsPhase_Rsp_t(void* xdrs, CAPI2_SimApi_GetCphsPhase_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_GetCphsPhase_Rsp_t")

	if(
		 _xdr_UInt8(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_GetSmsSca_Req_t(void* xdrs, CAPI2_SimApi_GetSmsSca_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_GetSmsSca_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->rec_no,"rec_no") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_GetSmsSca_Rsp_t(void* xdrs, CAPI2_SimApi_GetSmsSca_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_GetSmsSca_Rsp_t")

	if(
		xdr_SIM_SCA_DATA_t(xdrs, &rsp->sca_data) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_GetIccid_Rsp_t(void* xdrs, CAPI2_SimApi_GetIccid_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_GetIccid_Rsp_t")

	if(
		xdr_ICCID_ASCII_t(xdrs, &rsp->icc_parm) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_IsALSEnabled_Rsp_t(void* xdrs, CAPI2_SimApi_IsALSEnabled_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_IsALSEnabled_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_GetAlsDefaultLine_Rsp_t(void* xdrs, CAPI2_SimApi_GetAlsDefaultLine_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_GetAlsDefaultLine_Rsp_t")

	if(
		 _xdr_UInt8(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SetAlsDefaultLine_Req_t(void* xdrs, CAPI2_SimApi_SetAlsDefaultLine_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SetAlsDefaultLine_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->line,"line") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_GetCallForwardUnconditionalFlag_Rsp_t(void* xdrs, CAPI2_SimApi_GetCallForwardUnconditionalFlag_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_GetCallForwardUnconditionalFlag_Rsp_t")

	if(
		_xdr_Boolean(xdrs, &rsp->sim_file_exist,"sim_file_exist") &&
		 xdr_SIM_CALL_FORWARD_UNCONDITIONAL_FLAG_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_GetApplicationType_Rsp_t(void* xdrs, CAPI2_SimApi_GetApplicationType_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_GetApplicationType_Rsp_t")

	if(
		 xdr_SIM_APPL_TYPE_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendWritePuctReq_Req_t(void* xdrs, CAPI2_SimApi_SendWritePuctReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendWritePuctReq_Req_t")

	if(
		xdr_CurrencyName_t(xdrs, &rsp->currency) &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->eppu,sizeof( EPPU_t ), xdr_EPPU_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendWritePuctReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendWritePuctReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendWritePuctReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIMAccess_t ), xdr_SIMAccess_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SubmitRestrictedAccessReq_Req_t(void* xdrs, CAPI2_SimApi_SubmitRestrictedAccessReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SimApi_SubmitRestrictedAccessReq_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->socket_id,"socket_id") &&
		xdr_APDUCmd_t(xdrs, &rsp->command) &&
		xdr_APDUFileID_t(xdrs, &rsp->efile_id) &&
		xdr_APDUFileID_t(xdrs, &rsp->dfile_id) &&
		_xdr_UInt8(xdrs, &rsp->p1,"p1") &&
		_xdr_UInt8(xdrs, &rsp->p2,"p2") &&
		_xdr_UInt8(xdrs, &rsp->p3,"p3") &&
		_xdr_UInt8(xdrs, &rsp->path_len,"path_len") &&
		xdr_copy_len(&len, (u_int)(rsp->path_len)) && xdr_array(xdrs, (char **)(void*)&rsp->select_path, &len, ~0, sizeof( UInt16 ),(xdrproc_t) xdr_UInt16) &&
		xdr_copy_len(&len, (u_int)(rsp->p3)) && xdr_bytes(xdrs, (char **)(void*)&rsp->data, &len, 0xFFFF) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SubmitRestrictedAccessReq_Rsp_t(void* xdrs, CAPI2_SimApi_SubmitRestrictedAccessReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SubmitRestrictedAccessReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_RESTRICTED_ACCESS_DATA_t ), xdr_SIM_RESTRICTED_ACCESS_DATA_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIM_SendDetectionInd_Rsp_t(void* xdrs, CAPI2_SIM_SendDetectionInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendDetectionInd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_DETECTION_t ), xdr_SIM_DETECTION_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_GsmRegStatusInd_Rsp_t(void* xdrs, CAPI2_MS_GsmRegStatusInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GsmRegStatusInd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( MSRegInfo_t ), xdr_MSRegInfo_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_GprsRegStatusInd_Rsp_t(void* xdrs, CAPI2_MS_GprsRegStatusInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GprsRegStatusInd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( MSRegInfo_t ), xdr_MSRegInfo_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_NetworkNameInd_Rsp_t(void* xdrs, CAPI2_MS_NetworkNameInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_NetworkNameInd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( nitzNetworkName_t ), xdr_nitzNetworkName_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_LsaInd_Rsp_t(void* xdrs, CAPI2_MS_LsaInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_LsaInd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( lsaIdentity_t ), xdr_lsaIdentity_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_RssiInd_Rsp_t(void* xdrs, CAPI2_MS_RssiInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_RssiInd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( RxSignalInfo_t ), xdr_RxSignalInfo_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_SignalChangeInd_Rsp_t(void* xdrs, CAPI2_MS_SignalChangeInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_SignalChangeInd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( RX_SIGNAL_INFO_CHG_t ), xdr_RX_SIGNAL_INFO_CHG_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_PlmnListInd_Rsp_t(void* xdrs, CAPI2_MS_PlmnListInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_PlmnListInd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SEARCHED_PLMN_LIST_t ), xdr_SEARCHED_PLMN_LIST_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_TimeZoneInd_Rsp_t(void* xdrs, CAPI2_MS_TimeZoneInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_TimeZoneInd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( TimeZoneDate_t ), xdr_TimeZoneDate_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_GetSystemRAT_Rsp_t(void* xdrs, CAPI2_MS_GetSystemRAT_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetSystemRAT_Rsp_t")

	if(
		 xdr_RATSelect_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_GetSupportedRAT_Rsp_t(void* xdrs, CAPI2_MS_GetSupportedRAT_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetSupportedRAT_Rsp_t")

	if(
		 xdr_RATSelect_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_GetSystemBand_Rsp_t(void* xdrs, CAPI2_MS_GetSystemBand_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetSystemBand_Rsp_t")

	if(
		 xdr_BandSelect_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_GetSupportedBand_Rsp_t(void* xdrs, CAPI2_MS_GetSupportedBand_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetSupportedBand_Rsp_t")

	if(
		 xdr_BandSelect_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYSPARM_GetMSClass_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetMSClass_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_GetMSClass_Rsp_t")

	if(
		 _xdr_UInt16(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYSPARM_GetManufacturerName_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetManufacturerName_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_GetManufacturerName_Rsp_t")

	if(
		 xdr_uchar_ptr_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYSPARM_GetModelName_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetModelName_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_GetModelName_Rsp_t")

	if(
		 xdr_uchar_ptr_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYSPARM_GetSWVersion_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetSWVersion_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_GetSWVersion_Rsp_t")

	if(
		 xdr_uchar_ptr_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYSPARM_GetEGPRSMSClass_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetEGPRSMSClass_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_GetEGPRSMSClass_Rsp_t")

	if(
		 _xdr_UInt16(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendNumOfPLMNEntryReq_Req_t(void* xdrs, CAPI2_SimApi_SendNumOfPLMNEntryReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendNumOfPLMNEntryReq_Req_t")

	if(
		xdr_SIM_PLMN_FILE_t(xdrs, &rsp->plmn_file) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendNumOfPLMNEntryReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendNumOfPLMNEntryReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendNumOfPLMNEntryReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_PLMN_NUM_OF_ENTRY_t ), xdr_SIM_PLMN_NUM_OF_ENTRY_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendReadPLMNEntryReq_Req_t(void* xdrs, CAPI2_SimApi_SendReadPLMNEntryReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendReadPLMNEntryReq_Req_t")

	if(
		xdr_SIM_PLMN_FILE_t(xdrs, &rsp->plmn_file) &&
		_xdr_UInt16(xdrs, &rsp->start_index,"start_index") &&
		_xdr_UInt16(xdrs, &rsp->end_index,"end_index") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendReadPLMNEntryReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendReadPLMNEntryReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendReadPLMNEntryReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_PLMN_ENTRY_DATA_t ), xdr_SIM_PLMN_ENTRY_DATA_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendWriteMulPLMNEntryReq_Req_t(void* xdrs, CAPI2_SimApi_SendWriteMulPLMNEntryReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SimApi_SendWriteMulPLMNEntryReq_Req_t")

	if(
		xdr_SIM_PLMN_FILE_t(xdrs, &rsp->plmn_file) &&
		_xdr_UInt16(xdrs, &rsp->first_idx,"first_idx") &&
		_xdr_UInt16(xdrs, &rsp->number_of_entry,"number_of_entry") &&
		xdr_copy_len(&len, (u_int)(rsp->number_of_entry)) && xdr_bytes(xdrs, (char **)(void*)&rsp->plmn_entry, &len, 0xFFFF) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendWriteMulPLMNEntryReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendWriteMulPLMNEntryReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendWriteMulPLMNEntryReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_MUL_PLMN_ENTRY_UPDATE_t ), xdr_SIM_MUL_PLMN_ENTRY_UPDATE_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYS_SetRegisteredEventMask_Req_t(void* xdrs, CAPI2_SYS_SetRegisteredEventMask_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SYS_SetRegisteredEventMask_Req_t")

	if(
		xdr_UInt8(xdrs, &rsp->maskLen) &&
		xdr_copy_len(&len, (u_int)(rsp->maskLen)) && xdr_array(xdrs, (char **)(void*)&rsp->maskList, &len, ~0, sizeof( UInt16 ),(xdrproc_t) xdr_UInt16) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYS_SetRegisteredEventMask_Rsp_t(void* xdrs, CAPI2_SYS_SetRegisteredEventMask_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYS_SetRegisteredEventMask_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYS_SetFilteredEventMask_Req_t(void* xdrs, CAPI2_SYS_SetFilteredEventMask_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SYS_SetFilteredEventMask_Req_t")

	if(
		xdr_UInt8(xdrs, &rsp->maskLen) &&
		xdr_copy_len(&len, (u_int)(rsp->maskLen)) && xdr_array(xdrs, (char **)(void*)&rsp->maskList, &len, ~0, sizeof( UInt16 ),(xdrproc_t) xdr_UInt16) &&
		xdr_SysFilterEnable_t(xdrs, &rsp->enableFlag) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYS_SetFilteredEventMask_Rsp_t(void* xdrs, CAPI2_SYS_SetFilteredEventMask_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYS_SetFilteredEventMask_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PhoneCtrlApi_SetRssiThreshold_Req_t(void* xdrs, CAPI2_PhoneCtrlApi_SetRssiThreshold_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PhoneCtrlApi_SetRssiThreshold_Req_t")

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

bool_t xdr_CAPI2_SYSPARM_GetChanMode_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetChanMode_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_GetChanMode_Rsp_t")

	if(
		 _xdr_UInt16(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYSPARM_GetClassmark_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetClassmark_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_GetClassmark_Rsp_t")

	if(
		 xdr_CAPI2_Class_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYSPARM_GetSysparmIndPartFileVersion_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetSysparmIndPartFileVersion_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_GetSysparmIndPartFileVersion_Rsp_t")

	if(
		 _xdr_UInt16(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
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

bool_t xdr_CAPI2_SatkApi_SendPlayToneRes_Req_t(void* xdrs, CAPI2_SatkApi_SendPlayToneRes_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SatkApi_SendPlayToneRes_Req_t")

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
		xdr_SATK_ResultCode_t(xdrs, &rsp->result1) &&
		xdr_SATK_ResultCode2_t(xdrs, &rsp->result2) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_SetFdnCheck_Req_t(void* xdrs, CAPI2_PbkApi_SetFdnCheck_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PbkApi_SetFdnCheck_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->fdn_chk_on,"fdn_chk_on") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_GetFdnCheck_Rsp_t(void* xdrs, CAPI2_PbkApi_GetFdnCheck_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PbkApi_GetFdnCheck_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
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

bool_t xdr_CAPI2_MsDbApi_SetElement_Req_t(void* xdrs, CAPI2_MsDbApi_SetElement_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MsDbApi_SetElement_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->inElemData,sizeof( CAPI2_MS_Element_t ), xdr_CAPI2_MS_Element_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MsDbApi_GetElement_Req_t(void* xdrs, CAPI2_MsDbApi_GetElement_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MsDbApi_GetElement_Req_t")

	if(
		xdr_MS_Element_t(xdrs, &rsp->inElemType) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MsDbApi_GetElement_Rsp_t(void* xdrs, CAPI2_MsDbApi_GetElement_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MsDbApi_GetElement_Rsp_t")

	if(
		 xdr_CAPI2_MS_Element_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_USimApi_IsApplicationSupported_Req_t(void* xdrs, CAPI2_USimApi_IsApplicationSupported_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USimApi_IsApplicationSupported_Req_t")

	if(
		xdr_USIM_APPLICATION_TYPE(xdrs, &rsp->appli_type) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_USimApi_IsApplicationSupported_Rsp_t(void* xdrs, CAPI2_USimApi_IsApplicationSupported_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USimApi_IsApplicationSupported_Rsp_t")

	if(
		xdr_USIM_AID_DATA_t(xdrs, &rsp->pOutAidData) &&
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_USimApi_IsAllowedAPN_Req_t(void* xdrs, CAPI2_USimApi_IsAllowedAPN_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USimApi_IsAllowedAPN_Req_t")

	if(
		xdr_char_ptr_t(xdrs, &rsp->apn_name) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_USimApi_IsAllowedAPN_Rsp_t(void* xdrs, CAPI2_USimApi_IsAllowedAPN_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USimApi_IsAllowedAPN_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_USimApi_GetNumOfAPN_Rsp_t(void* xdrs, CAPI2_USimApi_GetNumOfAPN_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USimApi_GetNumOfAPN_Rsp_t")

	if(
		 _xdr_UInt8(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_USimApi_GetAPNEntry_Req_t(void* xdrs, CAPI2_USimApi_GetAPNEntry_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USimApi_GetAPNEntry_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->index,"index") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_USimApi_GetAPNEntry_Rsp_t(void* xdrs, CAPI2_USimApi_GetAPNEntry_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USimApi_GetAPNEntry_Rsp_t")

	if(
		xdr_APN_NAME_t(xdrs, &rsp->apn_name) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_USimApi_IsEstServActivated_Req_t(void* xdrs, CAPI2_USimApi_IsEstServActivated_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USimApi_IsEstServActivated_Req_t")

	if(
		xdr_USIM_EST_SERVICE_t(xdrs, &rsp->est_serv) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_USimApi_IsEstServActivated_Rsp_t(void* xdrs, CAPI2_USimApi_IsEstServActivated_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USimApi_IsEstServActivated_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_USimApi_SendSetEstServReq_Req_t(void* xdrs, CAPI2_USimApi_SendSetEstServReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USimApi_SendSetEstServReq_Req_t")

	if(
		xdr_USIM_EST_SERVICE_t(xdrs, &rsp->est_serv) &&
		_xdr_Boolean(xdrs, &rsp->serv_on,"serv_on") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_USimApi_SendSetEstServReq_Rsp_t(void* xdrs, CAPI2_USimApi_SendSetEstServReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USimApi_SendSetEstServReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( USIM_FILE_UPDATE_RSP_t ), xdr_USIM_FILE_UPDATE_RSP_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_USimApi_SendWriteAPNReq_Req_t(void* xdrs, CAPI2_USimApi_SendWriteAPNReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USimApi_SendWriteAPNReq_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->index,"index") &&
		xdr_char_ptr_t(xdrs, &rsp->apn_name) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_USimApi_SendWriteAPNReq_Rsp_t(void* xdrs, CAPI2_USimApi_SendWriteAPNReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USimApi_SendWriteAPNReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( USIM_FILE_UPDATE_RSP_t ), xdr_USIM_FILE_UPDATE_RSP_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_USimApi_SendDeleteAllAPNReq_Rsp_t(void* xdrs, CAPI2_USimApi_SendDeleteAllAPNReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USimApi_SendDeleteAllAPNReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( USIM_FILE_UPDATE_RSP_t ), xdr_USIM_FILE_UPDATE_RSP_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_USimApi_GetRatModeSetting_Rsp_t(void* xdrs, CAPI2_USimApi_GetRatModeSetting_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USimApi_GetRatModeSetting_Rsp_t")

	if(
		 xdr_USIM_RAT_MODE_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_GetGPRSRegState_Rsp_t(void* xdrs, CAPI2_MS_GetGPRSRegState_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetGPRSRegState_Rsp_t")

	if(
		 xdr_MSRegState_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_GetGSMRegState_Rsp_t(void* xdrs, CAPI2_MS_GetGSMRegState_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetGSMRegState_Rsp_t")

	if(
		 xdr_MSRegState_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_GetRegisteredCellInfo_Rsp_t(void* xdrs, CAPI2_MS_GetRegisteredCellInfo_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetRegisteredCellInfo_Rsp_t")

	if(
		 xdr_CellInfo_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
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

bool_t xdr_CAPI2_USimApi_GetServiceStatus_Req_t(void* xdrs, CAPI2_USimApi_GetServiceStatus_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USimApi_GetServiceStatus_Req_t")

	if(
		xdr_SIMService_t(xdrs, &rsp->service) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_USimApi_GetServiceStatus_Rsp_t(void* xdrs, CAPI2_USimApi_GetServiceStatus_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USimApi_GetServiceStatus_Rsp_t")

	if(
		 xdr_SIMServiceStatus_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_IsAllowedAPN_Req_t(void* xdrs, CAPI2_SimApi_IsAllowedAPN_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_IsAllowedAPN_Req_t")

	if(
		xdr_char_ptr_t(xdrs, &rsp->apn_name) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_IsAllowedAPN_Rsp_t(void* xdrs, CAPI2_SimApi_IsAllowedAPN_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_IsAllowedAPN_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_GetSmsMaxCapacity_Req_t(void* xdrs, CAPI2_SmsApi_GetSmsMaxCapacity_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_GetSmsMaxCapacity_Req_t")

	if(
		xdr_SmsStorage_t(xdrs, &rsp->storageType) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_GetSmsMaxCapacity_Rsp_t(void* xdrs, CAPI2_SmsApi_GetSmsMaxCapacity_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_GetSmsMaxCapacity_Rsp_t")

	if(
		 _xdr_UInt16(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_RetrieveMaxCBChnlLength_Rsp_t(void* xdrs, CAPI2_SmsApi_RetrieveMaxCBChnlLength_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_RetrieveMaxCBChnlLength_Rsp_t")

	if(
		_xdr_UInt8(xdrs, &rsp->length,"length") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_IsBdnOperationRestricted_Rsp_t(void* xdrs, CAPI2_SimApi_IsBdnOperationRestricted_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_IsBdnOperationRestricted_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendPreferredPlmnUpdateInd_Req_t(void* xdrs, CAPI2_SimApi_SendPreferredPlmnUpdateInd_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendPreferredPlmnUpdateInd_Req_t")

	if(
		xdr_SIM_PLMN_FILE_t(xdrs, &rsp->prefer_plmn_file) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendSetBdnReq_Req_t(void* xdrs, CAPI2_SimApi_SendSetBdnReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendSetBdnReq_Req_t")

	if(
		xdr_SIMBdnOperState_t(xdrs, &rsp->oper_state) &&
		xdr_CHVString_t(xdrs, &rsp->chv2) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendSetBdnReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendSetBdnReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendSetBdnReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIMAccess_t ), xdr_SIMAccess_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_PowerOnOffCard_Req_t(void* xdrs, CAPI2_SimApi_PowerOnOffCard_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_PowerOnOffCard_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->power_on,"power_on") &&
		xdr_SIM_POWER_ON_MODE_t(xdrs, &rsp->mode) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_PowerOnOffCard_Rsp_t(void* xdrs, CAPI2_SimApi_PowerOnOffCard_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_PowerOnOffCard_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_GENERIC_APDU_RES_INFO_t ), xdr_SIM_GENERIC_APDU_RES_INFO_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_GetRawAtr_Rsp_t(void* xdrs, CAPI2_SimApi_GetRawAtr_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_GetRawAtr_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_GENERIC_APDU_ATR_INFO_t ), xdr_SIM_GENERIC_APDU_ATR_INFO_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_Set_Protocol_Req_t(void* xdrs, CAPI2_SimApi_Set_Protocol_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_Set_Protocol_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->protocol,"protocol") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_Get_Protocol_Rsp_t(void* xdrs, CAPI2_SimApi_Get_Protocol_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_Get_Protocol_Rsp_t")

	if(
		 _xdr_UInt8(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendGenericApduCmd_Req_t(void* xdrs, CAPI2_SimApi_SendGenericApduCmd_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SimApi_SendGenericApduCmd_Req_t")

	if(
		xdr_UInt16(xdrs, &rsp->len) &&
		xdr_copy_len(&len, (u_int)(rsp->len)) && xdr_bytes(xdrs, (char **)(void*)&rsp->apdu, &len, 0xFFFF) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendGenericApduCmd_Rsp_t(void* xdrs, CAPI2_SimApi_SendGenericApduCmd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendGenericApduCmd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_GENERIC_APDU_XFER_RSP_t ), xdr_SIM_GENERIC_APDU_XFER_RSP_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIM_GetSimInterface_Rsp_t(void* xdrs, CAPI2_SIM_GetSimInterface_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_GetSimInterface_Rsp_t")

	if(
		 xdr_SIM_SIM_INTERFACE_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_NetRegApi_SetPlmnSelectRat_Req_t(void* xdrs, CAPI2_NetRegApi_SetPlmnSelectRat_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_NetRegApi_SetPlmnSelectRat_Req_t")

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

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_IsRegisterInProgress_Rsp_t(void* xdrs, CAPI2_MS_IsRegisterInProgress_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_IsRegisterInProgress_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_SendUsimHdkReadReq_Rsp_t(void* xdrs, CAPI2_PbkApi_SendUsimHdkReadReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PbkApi_SendUsimHdkReadReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( USIM_PBK_READ_HDK_ENTRY_RSP_t ), xdr_USIM_PBK_READ_HDK_ENTRY_RSP_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_SendUsimHdkUpdateReq_Req_t(void* xdrs, CAPI2_PbkApi_SendUsimHdkUpdateReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PbkApi_SendUsimHdkUpdateReq_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->hidden_key,sizeof( HDKString_t ), xdr_HDKString_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_SendUsimHdkUpdateReq_Rsp_t(void* xdrs, CAPI2_PbkApi_SendUsimHdkUpdateReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PbkApi_SendUsimHdkUpdateReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( USIM_PBK_UPDATE_HDK_ENTRY_RSP_t ), xdr_USIM_PBK_UPDATE_HDK_ENTRY_RSP_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_SendUsimAasReadReq_Req_t(void* xdrs, CAPI2_PbkApi_SendUsimAasReadReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PbkApi_SendUsimAasReadReq_Req_t")

	if(
		_xdr_UInt16(xdrs, &rsp->index,"index") &&
		xdr_PBK_Id_t(xdrs, &rsp->pbk_id) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_SendUsimAasReadReq_Rsp_t(void* xdrs, CAPI2_PbkApi_SendUsimAasReadReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PbkApi_SendUsimAasReadReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( USIM_PBK_READ_ALPHA_ENTRY_RSP_t ), xdr_USIM_PBK_READ_ALPHA_ENTRY_RSP_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_SendUsimAasUpdateReq_Req_t(void* xdrs, CAPI2_PbkApi_SendUsimAasUpdateReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PbkApi_SendUsimAasUpdateReq_Req_t")

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

bool_t xdr_CAPI2_PbkApi_SendUsimAasUpdateReq_Rsp_t(void* xdrs, CAPI2_PbkApi_SendUsimAasUpdateReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PbkApi_SendUsimAasUpdateReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( USIM_PBK_UPDATE_ALPHA_ENTRY_RSP_t ), xdr_USIM_PBK_UPDATE_ALPHA_ENTRY_RSP_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_SendUsimGasReadReq_Req_t(void* xdrs, CAPI2_PbkApi_SendUsimGasReadReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PbkApi_SendUsimGasReadReq_Req_t")

	if(
		_xdr_UInt16(xdrs, &rsp->index,"index") &&
		xdr_PBK_Id_t(xdrs, &rsp->pbk_id) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_SendUsimGasUpdateReq_Req_t(void* xdrs, CAPI2_PbkApi_SendUsimGasUpdateReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PbkApi_SendUsimGasUpdateReq_Req_t")

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

bool_t xdr_CAPI2_PbkApi_SendUsimAasInfoReq_Req_t(void* xdrs, CAPI2_PbkApi_SendUsimAasInfoReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PbkApi_SendUsimAasInfoReq_Req_t")

	if(
		xdr_PBK_Id_t(xdrs, &rsp->pbk_id) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_SendUsimAasInfoReq_Rsp_t(void* xdrs, CAPI2_PbkApi_SendUsimAasInfoReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PbkApi_SendUsimAasInfoReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( USIM_PBK_ALPHA_INFO_RSP_t ), xdr_USIM_PBK_ALPHA_INFO_RSP_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_SendUsimGasInfoReq_Req_t(void* xdrs, CAPI2_PbkApi_SendUsimGasInfoReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PbkApi_SendUsimGasInfoReq_Req_t")

	if(
		xdr_PBK_Id_t(xdrs, &rsp->pbk_id) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_SendUsimGasInfoReq_Rsp_t(void* xdrs, CAPI2_PbkApi_SendUsimGasInfoReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PbkApi_SendUsimGasInfoReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( USIM_PBK_ALPHA_INFO_RSP_t ), xdr_USIM_PBK_ALPHA_INFO_RSP_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_MeasureReportInd_Rsp_t(void* xdrs, CAPI2_MS_MeasureReportInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_MeasureReportInd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( MS_RxTestParam_t ), xdr_MS_RxTestParam_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DiagApi_MeasurmentReportReq_Req_t(void* xdrs, CAPI2_DiagApi_MeasurmentReportReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DiagApi_MeasurmentReportReq_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->inPeriodicReport,"inPeriodicReport") &&
		xdr_UInt32(xdrs, &rsp->inTimeInterval) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_StatusInd_Rsp_t(void* xdrs, CAPI2_MS_StatusInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_StatusInd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( MS_StatusIndication_t ), xdr_MS_StatusIndication_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYSPARM_GetActualLowVoltReading_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetActualLowVoltReading_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_GetActualLowVoltReading_Rsp_t")

	if(
		 _xdr_UInt16(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYSPARM_GetActual4p2VoltReading_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetActual4p2VoltReading_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_GetActual4p2VoltReading_Rsp_t")

	if(
		 _xdr_UInt16(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_SendSMSCommandTxtReq_Req_t(void* xdrs, CAPI2_SmsApi_SendSMSCommandTxtReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_SendSMSCommandTxtReq_Req_t")

	if(
		xdr_SmsCommand_t(xdrs, &rsp->smsCmd) &&
		xdr_uchar_ptr_t(xdrs, &rsp->inNum) &&
		xdr_uchar_ptr_t(xdrs, &rsp->inCmdTxt) &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->sca,sizeof( Sms_411Addr_t ), xdr_Sms_411Addr_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_SendSMSCommandPduReq_Req_t(void* xdrs, CAPI2_SmsApi_SendSMSCommandPduReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SmsApi_SendSMSCommandPduReq_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->length,"length") &&
		xdr_copy_len(&len, (u_int)(rsp->length)) && xdr_bytes(xdrs, (char **)(void*)&rsp->inSmsCmdPdu, &len, 0xFFFF) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_SendPDUAckToNetwork_Req_t(void* xdrs, CAPI2_SmsApi_SendPDUAckToNetwork_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SmsApi_SendPDUAckToNetwork_Req_t")

	if(
		xdr_SmsMti_t(xdrs, &rsp->mti) &&
		_xdr_UInt16(xdrs, &rsp->rp_cause,"rp_cause") &&
		xdr_UInt8(xdrs, &rsp->tpdu_len) &&
		xdr_copy_len(&len, (u_int)(rsp->tpdu_len)) && xdr_bytes(xdrs, (char **)(void*)&rsp->tpdu, &len, 0xFFFF) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_StartCellBroadcastWithChnlReq_Req_t(void* xdrs, CAPI2_SmsApi_StartCellBroadcastWithChnlReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_StartCellBroadcastWithChnlReq_Req_t")

	if(
		xdr_char_ptr_t(xdrs, &rsp->cbmi) &&
		xdr_char_ptr_t(xdrs, &rsp->lang) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_SetMoSmsTpMr_Req_t(void* xdrs, CAPI2_SmsApi_SetMoSmsTpMr_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_SetMoSmsTpMr_Req_t")

	if(
		xdr_uchar_ptr_t(xdrs, &rsp->tpMr) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIMLOCKApi_SetStatus_Req_t(void* xdrs, CAPI2_SIMLOCKApi_SetStatus_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIMLOCKApi_SetStatus_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->simlock_state,sizeof( SIMLOCK_STATE_t ), xdr_SIMLOCK_STATE_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_Ue3gStatusInd_Rsp_t(void* xdrs, CAPI2_MS_Ue3gStatusInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_Ue3gStatusInd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( MSUe3gStatusInd_t ), xdr_MSUe3gStatusInd_t) &&
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

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_SetRuaReadyTimer_Req_t(void* xdrs, CAPI2_MS_SetRuaReadyTimer_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_SetRuaReadyTimer_Req_t")

	if(
		xdr_UInt32(xdrs, &rsp->inRuaReadyTimer) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LCS_RrlpDataInd_Rsp_t(void* xdrs, CAPI2_LCS_RrlpDataInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_RrlpDataInd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( LcsMsgData_t ), xdr_LcsMsgData_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LCS_RrlpReset_Rsp_t(void* xdrs, CAPI2_LCS_RrlpReset_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_RrlpReset_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( ClientInfo_t ), xdr_ClientInfo_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LCS_RrcAssistDataInd_Rsp_t(void* xdrs, CAPI2_LCS_RrcAssistDataInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_RrcAssistDataInd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( LcsMsgData_t ), xdr_LcsMsgData_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LCS_RrcMeasCtrlInd_Rsp_t(void* xdrs, CAPI2_LCS_RrcMeasCtrlInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_RrcMeasCtrlInd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( LcsRrcMeasurement_t ), xdr_LcsRrcMeasurement_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LCS_RrcSysInfoInd_Rsp_t(void* xdrs, CAPI2_LCS_RrcSysInfoInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_RrcSysInfoInd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( LcsRrcBroadcastSysInfo_t ), xdr_LcsRrcBroadcastSysInfo_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LCS_RrcUeStateInd_Rsp_t(void* xdrs, CAPI2_LCS_RrcUeStateInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_RrcUeStateInd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( LcsRrcUeState_t ), xdr_LcsRrcUeState_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LCS_RrcStopMeasInd_Rsp_t(void* xdrs, CAPI2_LCS_RrcStopMeasInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_RrcStopMeasInd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( ClientInfo_t ), xdr_ClientInfo_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LCS_RrcReset_Rsp_t(void* xdrs, CAPI2_LCS_RrcReset_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_RrcReset_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( ClientInfo_t ), xdr_ClientInfo_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_IsThereEmergencyCall_Rsp_t(void* xdrs, CAPI2_CcApi_IsThereEmergencyCall_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_IsThereEmergencyCall_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_ServingCellInfoInd_Rsp_t(void* xdrs, CAPI2_ServingCellInfoInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_ServingCellInfoInd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( ServingCellInfo_t ), xdr_ServingCellInfo_t) &&
	1)
		return TRUE;
	else
		return FALSE;
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

bool_t xdr_CAPI2_LCS_L1_bb_isLocked_Req_t(void* xdrs, CAPI2_LCS_L1_bb_isLocked_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_L1_bb_isLocked_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->watch,"watch") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LCS_L1_bb_isLocked_Rsp_t(void* xdrs, CAPI2_LCS_L1_bb_isLocked_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_L1_bb_isLocked_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SMSPP_AppSpecificInd_Rsp_t(void* xdrs, CAPI2_SMSPP_AppSpecificInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMSPP_AppSpecificInd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SmsAppSpecificData_t ), xdr_SmsAppSpecificData_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DIALSTR_ParseGetCallType_Req_t(void* xdrs, CAPI2_DIALSTR_ParseGetCallType_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DIALSTR_ParseGetCallType_Req_t")

	if(
		xdr_uchar_ptr_t(xdrs, &rsp->dialStr) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DIALSTR_ParseGetCallType_Rsp_t(void* xdrs, CAPI2_DIALSTR_ParseGetCallType_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DIALSTR_ParseGetCallType_Rsp_t")

	if(
		 xdr_CallType_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LCS_FttCalcDeltaTime_Req_t(void* xdrs, CAPI2_LCS_FttCalcDeltaTime_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_FttCalcDeltaTime_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->inT1,sizeof( LcsFttParams_t ), xdr_LcsFttParams_t) &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->inT2,sizeof( LcsFttParams_t ), xdr_LcsFttParams_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LCS_FttCalcDeltaTime_Rsp_t(void* xdrs, CAPI2_LCS_FttCalcDeltaTime_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_FttCalcDeltaTime_Rsp_t")

	if(
		 xdr_UInt32(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LCS_SyncResult_Rsp_t(void* xdrs, CAPI2_LCS_SyncResult_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_SyncResult_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( LcsFttResult_t ), xdr_LcsFttResult_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_NetRegApi_ForcedReadyStateReq_Req_t(void* xdrs, CAPI2_NetRegApi_ForcedReadyStateReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_NetRegApi_ForcedReadyStateReq_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->forcedReadyState,"forcedReadyState") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIMLOCK_GetStatus_Req_t(void* xdrs, CAPI2_SIMLOCK_GetStatus_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIMLOCK_GetStatus_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->sim_data,sizeof( SIMLOCK_SIM_DATA_t ), xdr_SIMLOCK_SIM_DATA_t) &&
		_xdr_Boolean(xdrs, &rsp->is_testsim,"is_testsim") &&
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

bool_t xdr_CAPI2_DIALSTR_IsValidString_Req_t(void* xdrs, CAPI2_DIALSTR_IsValidString_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DIALSTR_IsValidString_Req_t")

	if(
		xdr_uchar_ptr_t(xdrs, &rsp->str) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DIALSTR_IsValidString_Rsp_t(void* xdrs, CAPI2_DIALSTR_IsValidString_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DIALSTR_IsValidString_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_UTIL_Cause2NetworkCause_Req_t(void* xdrs, CAPI2_UTIL_Cause2NetworkCause_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_UTIL_Cause2NetworkCause_Req_t")

	if(
		xdr_Cause_t(xdrs, &rsp->inCause) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_UTIL_Cause2NetworkCause_Rsp_t(void* xdrs, CAPI2_UTIL_Cause2NetworkCause_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_UTIL_Cause2NetworkCause_Rsp_t")

	if(
		 xdr_NetworkCause_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_UTIL_ErrCodeToNetCause_Req_t(void* xdrs, CAPI2_UTIL_ErrCodeToNetCause_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_UTIL_ErrCodeToNetCause_Req_t")

	if(
		xdr_SS_ErrorCode_t(xdrs, &rsp->inErrCode) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_UTIL_ErrCodeToNetCause_Rsp_t(void* xdrs, CAPI2_UTIL_ErrCodeToNetCause_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_UTIL_ErrCodeToNetCause_Rsp_t")

	if(
		 xdr_NetworkCause_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_IsGprsDialStr_Req_t(void* xdrs, CAPI2_IsGprsDialStr_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_IsGprsDialStr_Req_t")

	if(
		xdr_uchar_ptr_t(xdrs, &rsp->inDialStr) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_IsGprsDialStr_Rsp_t(void* xdrs, CAPI2_IsGprsDialStr_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_IsGprsDialStr_Rsp_t")

	if(
		_xdr_UInt8(xdrs, &rsp->gprsContextID,"gprsContextID") &&
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_UTIL_GetNumOffsetInSsStr_Req_t(void* xdrs, CAPI2_UTIL_GetNumOffsetInSsStr_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_UTIL_GetNumOffsetInSsStr_Req_t")

	if(
		xdr_char_ptr_t(xdrs, &rsp->ssStr) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_UTIL_GetNumOffsetInSsStr_Rsp_t(void* xdrs, CAPI2_UTIL_GetNumOffsetInSsStr_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_UTIL_GetNumOffsetInSsStr_Rsp_t")

	if(
		 _xdr_UInt8(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CipherAlgInd_Rsp_t(void* xdrs, CAPI2_CipherAlgInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CipherAlgInd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( PhonectrlCipherAlg_t ), xdr_PhonectrlCipherAlg_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_NWMeasResultInd_Rsp_t(void* xdrs, CAPI2_NWMeasResultInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_NWMeasResultInd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( Phonectrl_Nmr_t ), xdr_Phonectrl_Nmr_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_IsPppLoopbackDialStr_Req_t(void* xdrs, CAPI2_IsPppLoopbackDialStr_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_IsPppLoopbackDialStr_Req_t")

	if(
		xdr_uchar_ptr_t(xdrs, &rsp->str) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_IsPppLoopbackDialStr_Rsp_t(void* xdrs, CAPI2_IsPppLoopbackDialStr_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_IsPppLoopbackDialStr_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYS_GetRIPPROCVersion_Rsp_t(void* xdrs, CAPI2_SYS_GetRIPPROCVersion_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYS_GetRIPPROCVersion_Rsp_t")

	if(
		 _xdr_UInt8(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYSPARM_SetHSDPAPHYCategory_Req_t(void* xdrs, CAPI2_SYSPARM_SetHSDPAPHYCategory_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_SetHSDPAPHYCategory_Req_t")

	if(
		xdr_int(xdrs, &rsp->hsdpa_phy_cat) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYSPARM_GetHSDPAPHYCategory_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetHSDPAPHYCategory_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_GetHSDPAPHYCategory_Rsp_t")

	if(
		 _xdr_UInt8(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_ConvertSmsMSMsgType_Req_t(void* xdrs, CAPI2_SmsApi_ConvertSmsMSMsgType_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_ConvertSmsMSMsgType_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->ms_msg_type,"ms_msg_type") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_ConvertSmsMSMsgType_Rsp_t(void* xdrs, CAPI2_SmsApi_ConvertSmsMSMsgType_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_ConvertSmsMSMsgType_Rsp_t")

	if(
		 _xdr_UInt8(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_NetUarfcnDlInd_Rsp_t(void* xdrs, CAPI2_NetUarfcnDlInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_NetUarfcnDlInd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( UInt16 ), xdr_UInt16) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_GetPrefNetStatus_Rsp_t(void* xdrs, CAPI2_MS_GetPrefNetStatus_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetPrefNetStatus_Rsp_t")

	if(
		 xdr_GANStatus_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_SYS_TestSysMsgs1_Rsp_t(void* xdrs, SYS_TestSysMsgs1_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"SYS_TestSysMsgs1_Rsp_t")

	if(
		 xdr_uchar_ptr_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_SYS_TestSysMsgs2_Rsp_t(void* xdrs, SYS_TestSysMsgs2_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"SYS_TestSysMsgs2_Rsp_t")

	if(
		 xdr_uchar_ptr_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_SYS_TestSysMsgs3_Rsp_t(void* xdrs, SYS_TestSysMsgs3_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"SYS_TestSysMsgs3_Rsp_t")

	if(
		 xdr_uchar_ptr_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_SYS_TestSysMsgs4_Rsp_t(void* xdrs, SYS_TestSysMsgs4_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"SYS_TestSysMsgs4_Rsp_t")

	if(
		 xdr_uchar_ptr_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_SYS_TestSysMsgs5_Rsp_t(void* xdrs, SYS_TestSysMsgs5_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"SYS_TestSysMsgs5_Rsp_t")

	if(
		 xdr_uchar_ptr_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_SYS_TestSysMsgs6_Rsp_t(void* xdrs, SYS_TestSysMsgs6_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"SYS_TestSysMsgs6_Rsp_t")

	if(
		 xdr_uchar_ptr_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYSPARM_SetHSUPAPHYCategory_Req_t(void* xdrs, CAPI2_SYSPARM_SetHSUPAPHYCategory_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_SetHSUPAPHYCategory_Req_t")

	if(
		xdr_int(xdrs, &rsp->hsdpa_phy_cat) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYSPARM_GetHSUPAPHYCategory_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetHSUPAPHYCategory_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_GetHSUPAPHYCategory_Rsp_t")

	if(
		 _xdr_UInt8(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
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

bool_t xdr_CAPI2_CcApi_GetCurrentCallIndex_Rsp_t(void* xdrs, CAPI2_CcApi_GetCurrentCallIndex_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetCurrentCallIndex_Rsp_t")

	if(
		 _xdr_UInt8(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetNextActiveCallIndex_Rsp_t(void* xdrs, CAPI2_CcApi_GetNextActiveCallIndex_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetNextActiveCallIndex_Rsp_t")

	if(
		 _xdr_UInt8(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetNextHeldCallIndex_Rsp_t(void* xdrs, CAPI2_CcApi_GetNextHeldCallIndex_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetNextHeldCallIndex_Rsp_t")

	if(
		 _xdr_UInt8(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetNextWaitCallIndex_Rsp_t(void* xdrs, CAPI2_CcApi_GetNextWaitCallIndex_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetNextWaitCallIndex_Rsp_t")

	if(
		 _xdr_UInt8(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetMPTYCallIndex_Rsp_t(void* xdrs, CAPI2_CcApi_GetMPTYCallIndex_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetMPTYCallIndex_Rsp_t")

	if(
		 _xdr_UInt8(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetCallState_Req_t(void* xdrs, CAPI2_CcApi_GetCallState_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetCallState_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->callIndex,"callIndex") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetCallState_Rsp_t(void* xdrs, CAPI2_CcApi_GetCallState_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetCallState_Rsp_t")

	if(
		 xdr_CCallState_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetCallType_Req_t(void* xdrs, CAPI2_CcApi_GetCallType_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetCallType_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->callIndex,"callIndex") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetCallType_Rsp_t(void* xdrs, CAPI2_CcApi_GetCallType_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetCallType_Rsp_t")

	if(
		 xdr_CCallType_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetLastCallExitCause_Rsp_t(void* xdrs, CAPI2_CcApi_GetLastCallExitCause_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetLastCallExitCause_Rsp_t")

	if(
		 xdr_Cause_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetCallNumber_Req_t(void* xdrs, CAPI2_CcApi_GetCallNumber_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetCallNumber_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->callIndex,"callIndex") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetCallNumber_Rsp_t(void* xdrs, CAPI2_CcApi_GetCallNumber_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetCallNumber_Rsp_t")

	if(
		xdr_opaque(xdrs, (caddr_t)(void*)rsp->phNum,PHONE_NUMBER_LEN ) &&
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetCallingInfo_Req_t(void* xdrs, CAPI2_CcApi_GetCallingInfo_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetCallingInfo_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->callIndex,"callIndex") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetCallingInfo_Rsp_t(void* xdrs, CAPI2_CcApi_GetCallingInfo_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetCallingInfo_Rsp_t")

	if(
		xdr_CallingInfo_t(xdrs, &rsp->callingInfoPtr) &&
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetAllCallStates_Rsp_t(void* xdrs, CAPI2_CcApi_GetAllCallStates_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetAllCallStates_Rsp_t")

	if(
		xdr_CCallStateList_t(xdrs, &rsp->stateList) &&
		_xdr_UInt8(xdrs, &rsp->listSz,"listSz") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetAllCallIndex_Rsp_t(void* xdrs, CAPI2_CcApi_GetAllCallIndex_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetAllCallIndex_Rsp_t")

	if(
		xdr_CCallIndexList_t(xdrs, &rsp->indexList) &&
		_xdr_UInt8(xdrs, &rsp->listSz,"listSz") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetAllHeldCallIndex_Rsp_t(void* xdrs, CAPI2_CcApi_GetAllHeldCallIndex_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetAllHeldCallIndex_Rsp_t")

	if(
		xdr_CCallIndexList_t(xdrs, &rsp->indexList) &&
		_xdr_UInt8(xdrs, &rsp->listSz,"listSz") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetAllActiveCallIndex_Rsp_t(void* xdrs, CAPI2_CcApi_GetAllActiveCallIndex_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetAllActiveCallIndex_Rsp_t")

	if(
		xdr_CCallIndexList_t(xdrs, &rsp->indexList) &&
		_xdr_UInt8(xdrs, &rsp->listSz,"listSz") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetAllMPTYCallIndex_Rsp_t(void* xdrs, CAPI2_CcApi_GetAllMPTYCallIndex_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetAllMPTYCallIndex_Rsp_t")

	if(
		xdr_CCallIndexList_t(xdrs, &rsp->indexList) &&
		_xdr_UInt8(xdrs, &rsp->listSz,"listSz") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetNumOfMPTYCalls_Rsp_t(void* xdrs, CAPI2_CcApi_GetNumOfMPTYCalls_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetNumOfMPTYCalls_Rsp_t")

	if(
		 _xdr_UInt8(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetNumofActiveCalls_Rsp_t(void* xdrs, CAPI2_CcApi_GetNumofActiveCalls_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetNumofActiveCalls_Rsp_t")

	if(
		 _xdr_UInt8(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetNumofHeldCalls_Rsp_t(void* xdrs, CAPI2_CcApi_GetNumofHeldCalls_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetNumofHeldCalls_Rsp_t")

	if(
		 _xdr_UInt8(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_IsThereWaitingCall_Rsp_t(void* xdrs, CAPI2_CcApi_IsThereWaitingCall_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_IsThereWaitingCall_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_IsThereAlertingCall_Rsp_t(void* xdrs, CAPI2_CcApi_IsThereAlertingCall_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_IsThereAlertingCall_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetConnectedLineID_Req_t(void* xdrs, CAPI2_CcApi_GetConnectedLineID_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetConnectedLineID_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->callIndex,"callIndex") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetConnectedLineID_Rsp_t(void* xdrs, CAPI2_CcApi_GetConnectedLineID_Rsp_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_CcApi_GetConnectedLineID_Rsp_t")

	if(
		xdr_copy_len(&len, MAX_DIGITS+2 ) && xdr_bytes(xdrs, (char **)(void*)&rsp->val, &len , ~0) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetCallPresent_Req_t(void* xdrs, CAPI2_CcApi_GetCallPresent_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetCallPresent_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->callIndex,"callIndex") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetCallPresent_Rsp_t(void* xdrs, CAPI2_CcApi_GetCallPresent_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetCallPresent_Rsp_t")

	if(
		xdr_PresentationInd_t(xdrs, &rsp->inPresentPtr) &&
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetCallIndexInThisState_Req_t(void* xdrs, CAPI2_CcApi_GetCallIndexInThisState_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetCallIndexInThisState_Req_t")

	if(
		xdr_CCallState_t(xdrs, &rsp->inCcCallState) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetCallIndexInThisState_Rsp_t(void* xdrs, CAPI2_CcApi_GetCallIndexInThisState_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetCallIndexInThisState_Rsp_t")

	if(
		_xdr_UInt8(xdrs, &rsp->inCallIndexPtr,"inCallIndexPtr") &&
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_IsMultiPartyCall_Req_t(void* xdrs, CAPI2_CcApi_IsMultiPartyCall_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_IsMultiPartyCall_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->callIndex,"callIndex") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_IsMultiPartyCall_Rsp_t(void* xdrs, CAPI2_CcApi_IsMultiPartyCall_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_IsMultiPartyCall_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_IsThereVoiceCall_Rsp_t(void* xdrs, CAPI2_CcApi_IsThereVoiceCall_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_IsThereVoiceCall_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_IsConnectedLineIDPresentAllowed_Req_t(void* xdrs, CAPI2_CcApi_IsConnectedLineIDPresentAllowed_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_IsConnectedLineIDPresentAllowed_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->callIndex,"callIndex") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_IsConnectedLineIDPresentAllowed_Rsp_t(void* xdrs, CAPI2_CcApi_IsConnectedLineIDPresentAllowed_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_IsConnectedLineIDPresentAllowed_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetCurrentCallDurationInMilliSeconds_Req_t(void* xdrs, CAPI2_CcApi_GetCurrentCallDurationInMilliSeconds_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetCurrentCallDurationInMilliSeconds_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->callIndex,"callIndex") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetCurrentCallDurationInMilliSeconds_Rsp_t(void* xdrs, CAPI2_CcApi_GetCurrentCallDurationInMilliSeconds_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetCurrentCallDurationInMilliSeconds_Rsp_t")

	if(
		 xdr_UInt32(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetLastCallCCM_Rsp_t(void* xdrs, CAPI2_CcApi_GetLastCallCCM_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetLastCallCCM_Rsp_t")

	if(
		 xdr_UInt32(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetLastCallDuration_Rsp_t(void* xdrs, CAPI2_CcApi_GetLastCallDuration_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetLastCallDuration_Rsp_t")

	if(
		 xdr_UInt32(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetLastDataCallRxBytes_Rsp_t(void* xdrs, CAPI2_CcApi_GetLastDataCallRxBytes_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetLastDataCallRxBytes_Rsp_t")

	if(
		 xdr_UInt32(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetLastDataCallTxBytes_Rsp_t(void* xdrs, CAPI2_CcApi_GetLastDataCallTxBytes_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetLastDataCallTxBytes_Rsp_t")

	if(
		 xdr_UInt32(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetDataCallIndex_Rsp_t(void* xdrs, CAPI2_CcApi_GetDataCallIndex_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetDataCallIndex_Rsp_t")

	if(
		 _xdr_UInt8(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetCallClientInfo_Req_t(void* xdrs, CAPI2_CcApi_GetCallClientInfo_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetCallClientInfo_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->callIndex,"callIndex") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetCallClientInfo_Rsp_t(void* xdrs, CAPI2_CcApi_GetCallClientInfo_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetCallClientInfo_Rsp_t")

	if(
		xdr_ClientInfo_t(xdrs, &rsp->inClientInfoPtr) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetCallClientID_Req_t(void* xdrs, CAPI2_CcApi_GetCallClientID_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetCallClientID_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->callIndex,"callIndex") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetCallClientID_Rsp_t(void* xdrs, CAPI2_CcApi_GetCallClientID_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetCallClientID_Rsp_t")

	if(
		 _xdr_UInt8(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetTypeAdd_Req_t(void* xdrs, CAPI2_CcApi_GetTypeAdd_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetTypeAdd_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->callIndex,"callIndex") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetTypeAdd_Rsp_t(void* xdrs, CAPI2_CcApi_GetTypeAdd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetTypeAdd_Rsp_t")

	if(
		 _xdr_UInt8(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_SetVoiceCallAutoReject_Req_t(void* xdrs, CAPI2_CcApi_SetVoiceCallAutoReject_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_SetVoiceCallAutoReject_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->enableAutoRej,"enableAutoRej") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_IsVoiceCallAutoReject_Rsp_t(void* xdrs, CAPI2_CcApi_IsVoiceCallAutoReject_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_IsVoiceCallAutoReject_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_SetTTYCall_Req_t(void* xdrs, CAPI2_CcApi_SetTTYCall_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_SetTTYCall_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->enable,"enable") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_IsTTYEnable_Rsp_t(void* xdrs, CAPI2_CcApi_IsTTYEnable_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_IsTTYEnable_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_IsSimOriginedCall_Req_t(void* xdrs, CAPI2_CcApi_IsSimOriginedCall_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_IsSimOriginedCall_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->callIndex,"callIndex") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_IsSimOriginedCall_Rsp_t(void* xdrs, CAPI2_CcApi_IsSimOriginedCall_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_IsSimOriginedCall_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_SetVideoCallParam_Req_t(void* xdrs, CAPI2_CcApi_SetVideoCallParam_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_SetVideoCallParam_Req_t")

	if(
		xdr_VideoCallParam_t(xdrs, &rsp->param) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetVideoCallParam_Rsp_t(void* xdrs, CAPI2_CcApi_GetVideoCallParam_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetVideoCallParam_Rsp_t")

	if(
		 xdr_VideoCallParam_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetCCM_Req_t(void* xdrs, CAPI2_CcApi_GetCCM_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetCCM_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->callIndex,"callIndex") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetCCM_Rsp_t(void* xdrs, CAPI2_CcApi_GetCCM_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetCCM_Rsp_t")

	if(
		xdr_CallMeterUnit_t(xdrs, &rsp->ccm) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_SendDtmfTone_Req_t(void* xdrs, CAPI2_CcApi_SendDtmfTone_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_SendDtmfTone_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->inDtmfObjPtr,sizeof( ApiDtmf_t ), xdr_ApiDtmf_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_StopDtmfTone_Req_t(void* xdrs, CAPI2_CcApi_StopDtmfTone_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_StopDtmfTone_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->inDtmfObjPtr,sizeof( ApiDtmf_t ), xdr_ApiDtmf_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_AbortDtmf_Req_t(void* xdrs, CAPI2_CcApi_AbortDtmf_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_AbortDtmf_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->inDtmfObjPtr,sizeof( ApiDtmf_t ), xdr_ApiDtmf_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_SetDtmfToneTimer_Req_t(void* xdrs, CAPI2_CcApi_SetDtmfToneTimer_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_SetDtmfToneTimer_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->inCallIndex,"inCallIndex") &&
		xdr_DtmfTimer_t(xdrs, &rsp->inDtmfTimerType) &&
		xdr_Ticks_t(xdrs, &rsp->inDtmfTimeInterval) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_ResetDtmfToneTimer_Req_t(void* xdrs, CAPI2_CcApi_ResetDtmfToneTimer_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_ResetDtmfToneTimer_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->inCallIndex,"inCallIndex") &&
		xdr_DtmfTimer_t(xdrs, &rsp->inDtmfTimerType) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetDtmfToneTimer_Req_t(void* xdrs, CAPI2_CcApi_GetDtmfToneTimer_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetDtmfToneTimer_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->inCallIndex,"inCallIndex") &&
		xdr_DtmfTimer_t(xdrs, &rsp->inDtmfTimerType) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetDtmfToneTimer_Rsp_t(void* xdrs, CAPI2_CcApi_GetDtmfToneTimer_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetDtmfToneTimer_Rsp_t")

	if(
		 xdr_Ticks_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetTiFromCallIndex_Req_t(void* xdrs, CAPI2_CcApi_GetTiFromCallIndex_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetTiFromCallIndex_Req_t")

	if(
		xdr_CallIndex_t(xdrs, &rsp->inCallIndex) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetTiFromCallIndex_Rsp_t(void* xdrs, CAPI2_CcApi_GetTiFromCallIndex_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetTiFromCallIndex_Rsp_t")

	if(
		_xdr_UInt8(xdrs, &rsp->ti,"ti") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_IsSupportedBC_Req_t(void* xdrs, CAPI2_CcApi_IsSupportedBC_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_IsSupportedBC_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->inBcPtr,sizeof( BearerCapability_t ), xdr_BearerCapability_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_IsSupportedBC_Rsp_t(void* xdrs, CAPI2_CcApi_IsSupportedBC_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_IsSupportedBC_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetBearerCapability_Req_t(void* xdrs, CAPI2_CcApi_GetBearerCapability_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetBearerCapability_Req_t")

	if(
		xdr_CallIndex_t(xdrs, &rsp->inCallIndex) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetBearerCapability_Rsp_t(void* xdrs, CAPI2_CcApi_GetBearerCapability_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetBearerCapability_Rsp_t")

	if(
		xdr_CC_BearerCap_t(xdrs, &rsp->outBcPtr) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_SendSMSSrvCenterNumberUpdateReq_Req_t(void* xdrs, CAPI2_SmsApi_SendSMSSrvCenterNumberUpdateReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_SendSMSSrvCenterNumberUpdateReq_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->sca,sizeof( SmsAddress_t ), xdr_SmsAddress_t) &&
		_xdr_UInt8(xdrs, &rsp->rec_no,"rec_no") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_GetSMSrvCenterNumber_Req_t(void* xdrs, CAPI2_SmsApi_GetSMSrvCenterNumber_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_GetSMSrvCenterNumber_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->rec_no,"rec_no") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_GetSMSrvCenterNumber_Rsp_t(void* xdrs, CAPI2_SmsApi_GetSMSrvCenterNumber_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_GetSMSrvCenterNumber_Rsp_t")

	if(
		xdr_SmsAddress_t(xdrs, &rsp->sca) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_GetSIMSMSCapacityExceededFlag_Rsp_t(void* xdrs, CAPI2_SmsApi_GetSIMSMSCapacityExceededFlag_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_GetSIMSMSCapacityExceededFlag_Rsp_t")

	if(
		_xdr_Boolean(xdrs, &rsp->flag,"flag") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_GetSmsStoredState_Req_t(void* xdrs, CAPI2_SmsApi_GetSmsStoredState_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_GetSmsStoredState_Req_t")

	if(
		xdr_SmsStorage_t(xdrs, &rsp->storageType) &&
		_xdr_UInt16(xdrs, &rsp->index,"index") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_GetSmsStoredState_Rsp_t(void* xdrs, CAPI2_SmsApi_GetSmsStoredState_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_GetSmsStoredState_Rsp_t")

	if(
		xdr_SIMSMSMesgStatus_t(xdrs, &rsp->status) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_WriteSMSPduReq_Req_t(void* xdrs, CAPI2_SmsApi_WriteSMSPduReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SmsApi_WriteSMSPduReq_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->length,"length") &&
		xdr_copy_len(&len, (u_int)(rsp->length)) && xdr_bytes(xdrs, (char **)(void*)&rsp->inSmsPdu, &len, 0xFFFF) &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->sca,sizeof( Sms_411Addr_t ), xdr_Sms_411Addr_t) &&
		xdr_SmsStorage_t(xdrs, &rsp->storageType) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_WriteSMSPduReq_Rsp_t(void* xdrs, CAPI2_SmsApi_WriteSMSPduReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_WriteSMSPduReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_SMS_UPDATE_RESULT_t ), xdr_SIM_SMS_UPDATE_RESULT_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_WriteSMSReq_Req_t(void* xdrs, CAPI2_SmsApi_WriteSMSReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_WriteSMSReq_Req_t")

	if(
		xdr_uchar_ptr_t(xdrs, &rsp->inNum) &&
		xdr_uchar_ptr_t(xdrs, &rsp->inSMS) &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->params,sizeof( SmsTxParam_t ), xdr_SmsTxParam_t) &&
		xdr_uchar_ptr_t(xdrs, &rsp->inSca) &&
		xdr_SmsStorage_t(xdrs, &rsp->storageType) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_SendSMSReq_Req_t(void* xdrs, CAPI2_SmsApi_SendSMSReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_SendSMSReq_Req_t")

	if(
		xdr_uchar_ptr_t(xdrs, &rsp->inNum) &&
		xdr_uchar_ptr_t(xdrs, &rsp->inSMS) &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->params,sizeof( SmsTxParam_t ), xdr_SmsTxParam_t) &&
		xdr_uchar_ptr_t(xdrs, &rsp->inSca) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_SendSMSReq_Rsp_t(void* xdrs, CAPI2_SmsApi_SendSMSReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_SendSMSReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SmsSubmitRspMsg_t ), xdr_SmsSubmitRspMsg_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_SendSMSPduReq_Req_t(void* xdrs, CAPI2_SmsApi_SendSMSPduReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SmsApi_SendSMSPduReq_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->length,"length") &&
		xdr_copy_len(&len, (u_int)(rsp->length)) && xdr_bytes(xdrs, (char **)(void*)&rsp->inSmsPdu, &len, 0xFFFF) &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->sca,sizeof( Sms_411Addr_t ), xdr_Sms_411Addr_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_SendStoredSmsReq_Req_t(void* xdrs, CAPI2_SmsApi_SendStoredSmsReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_SendStoredSmsReq_Req_t")

	if(
		xdr_SmsStorage_t(xdrs, &rsp->storageType) &&
		_xdr_UInt16(xdrs, &rsp->index,"index") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_WriteSMSPduToSIMReq_Req_t(void* xdrs, CAPI2_SmsApi_WriteSMSPduToSIMReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SmsApi_WriteSMSPduToSIMReq_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->length,"length") &&
		xdr_copy_len(&len, (u_int)(rsp->length)) && xdr_bytes(xdrs, (char **)(void*)&rsp->inSmsPdu, &len, 0xFFFF) &&
		xdr_SIMSMSMesgStatus_t(xdrs, &rsp->smsState) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_GetLastTpMr_Rsp_t(void* xdrs, CAPI2_SmsApi_GetLastTpMr_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_GetLastTpMr_Rsp_t")

	if(
		 _xdr_UInt8(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_GetSmsTxParams_Rsp_t(void* xdrs, CAPI2_SmsApi_GetSmsTxParams_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_GetSmsTxParams_Rsp_t")

	if(
		xdr_SmsTxParam_t(xdrs, &rsp->params) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_GetTxParamInTextMode_Rsp_t(void* xdrs, CAPI2_SmsApi_GetTxParamInTextMode_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_GetTxParamInTextMode_Rsp_t")

	if(
		xdr_SmsTxTextModeParms_t(xdrs, &rsp->smsParms) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_SetSmsTxParamProcId_Req_t(void* xdrs, CAPI2_SmsApi_SetSmsTxParamProcId_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_SetSmsTxParamProcId_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->pid,"pid") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_SetSmsTxParamCodingType_Req_t(void* xdrs, CAPI2_SmsApi_SetSmsTxParamCodingType_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_SetSmsTxParamCodingType_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->codingType,sizeof( SmsCodingType_t ), xdr_SmsCodingType_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_SetSmsTxParamValidPeriod_Req_t(void* xdrs, CAPI2_SmsApi_SetSmsTxParamValidPeriod_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_SetSmsTxParamValidPeriod_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->validatePeriod,"validatePeriod") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_SetSmsTxParamCompression_Req_t(void* xdrs, CAPI2_SmsApi_SetSmsTxParamCompression_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_SetSmsTxParamCompression_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->compression,"compression") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_SetSmsTxParamReplyPath_Req_t(void* xdrs, CAPI2_SmsApi_SetSmsTxParamReplyPath_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_SetSmsTxParamReplyPath_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->replyPath,"replyPath") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_SetSmsTxParamUserDataHdrInd_Req_t(void* xdrs, CAPI2_SmsApi_SetSmsTxParamUserDataHdrInd_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_SetSmsTxParamUserDataHdrInd_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->udhi,"udhi") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_SetSmsTxParamStatusRptReqFlag_Req_t(void* xdrs, CAPI2_SmsApi_SetSmsTxParamStatusRptReqFlag_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_SetSmsTxParamStatusRptReqFlag_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->srr,"srr") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_SetSmsTxParamRejDupl_Req_t(void* xdrs, CAPI2_SmsApi_SetSmsTxParamRejDupl_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_SetSmsTxParamRejDupl_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->rejDupl,"rejDupl") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_DeleteSmsMsgByIndexReq_Req_t(void* xdrs, CAPI2_SmsApi_DeleteSmsMsgByIndexReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_DeleteSmsMsgByIndexReq_Req_t")

	if(
		xdr_SmsStorage_t(xdrs, &rsp->storeType) &&
		_xdr_UInt16(xdrs, &rsp->rec_no,"rec_no") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_DeleteSmsMsgByIndexReq_Rsp_t(void* xdrs, CAPI2_SmsApi_DeleteSmsMsgByIndexReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_DeleteSmsMsgByIndexReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SmsMsgDeleteResult_t ), xdr_SmsMsgDeleteResult_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_ReadSmsMsgReq_Req_t(void* xdrs, CAPI2_SmsApi_ReadSmsMsgReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_ReadSmsMsgReq_Req_t")

	if(
		xdr_SmsStorage_t(xdrs, &rsp->storeType) &&
		_xdr_UInt16(xdrs, &rsp->rec_no,"rec_no") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_ListSmsMsgReq_Req_t(void* xdrs, CAPI2_SmsApi_ListSmsMsgReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_ListSmsMsgReq_Req_t")

	if(
		xdr_SmsStorage_t(xdrs, &rsp->storeType) &&
		xdr_SIMSMSMesgStatus_t(xdrs, &rsp->msgBox) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_SetNewMsgDisplayPref_Req_t(void* xdrs, CAPI2_SmsApi_SetNewMsgDisplayPref_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_SetNewMsgDisplayPref_Req_t")

	if(
		xdr_NewMsgDisplayPref_t(xdrs, &rsp->type) &&
		_xdr_UInt8(xdrs, &rsp->mode,"mode") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_GetNewMsgDisplayPref_Req_t(void* xdrs, CAPI2_SmsApi_GetNewMsgDisplayPref_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_GetNewMsgDisplayPref_Req_t")

	if(
		xdr_NewMsgDisplayPref_t(xdrs, &rsp->type) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_GetNewMsgDisplayPref_Rsp_t(void* xdrs, CAPI2_SmsApi_GetNewMsgDisplayPref_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_GetNewMsgDisplayPref_Rsp_t")

	if(
		 _xdr_UInt8(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_SetSMSPrefStorage_Req_t(void* xdrs, CAPI2_SmsApi_SetSMSPrefStorage_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_SetSMSPrefStorage_Req_t")

	if(
		xdr_SmsStorage_t(xdrs, &rsp->storageType) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_GetSMSPrefStorage_Rsp_t(void* xdrs, CAPI2_SmsApi_GetSMSPrefStorage_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_GetSMSPrefStorage_Rsp_t")

	if(
		 xdr_SmsStorage_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_GetSMSStorageStatus_Req_t(void* xdrs, CAPI2_SmsApi_GetSMSStorageStatus_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_GetSMSStorageStatus_Req_t")

	if(
		xdr_SmsStorage_t(xdrs, &rsp->storageType) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_GetSMSStorageStatus_Rsp_t(void* xdrs, CAPI2_SmsApi_GetSMSStorageStatus_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_GetSMSStorageStatus_Rsp_t")

	if(
		_xdr_UInt16(xdrs, &rsp->NbFree,"NbFree") &&
		_xdr_UInt16(xdrs, &rsp->NbUsed,"NbUsed") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_SaveSmsServiceProfile_Req_t(void* xdrs, CAPI2_SmsApi_SaveSmsServiceProfile_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_SaveSmsServiceProfile_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->profileIndex,"profileIndex") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_RestoreSmsServiceProfile_Req_t(void* xdrs, CAPI2_SmsApi_RestoreSmsServiceProfile_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_RestoreSmsServiceProfile_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->profileIndex,"profileIndex") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_SetCellBroadcastMsgTypeReq_Req_t(void* xdrs, CAPI2_SmsApi_SetCellBroadcastMsgTypeReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_SetCellBroadcastMsgTypeReq_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->mode,"mode") &&
		xdr_uchar_ptr_t(xdrs, &rsp->chnlIDs) &&
		xdr_uchar_ptr_t(xdrs, &rsp->codings) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_SetCellBroadcastMsgTypeReq_Rsp_t(void* xdrs, CAPI2_SmsApi_SetCellBroadcastMsgTypeReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_SetCellBroadcastMsgTypeReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SmsCBMsgRspType_t ), xdr_SmsCBMsgRspType_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_CBAllowAllChnlReq_Req_t(void* xdrs, CAPI2_SmsApi_CBAllowAllChnlReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_CBAllowAllChnlReq_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->suspend_filtering,"suspend_filtering") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_AddCellBroadcastChnlReq_Req_t(void* xdrs, CAPI2_SmsApi_AddCellBroadcastChnlReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_AddCellBroadcastChnlReq_Req_t")

	if(
		xdr_uchar_ptr_t(xdrs, &rsp->newCBMI) &&
		_xdr_UInt8(xdrs, &rsp->cbmiLen,"cbmiLen") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_RemoveCellBroadcastChnlReq_Req_t(void* xdrs, CAPI2_SmsApi_RemoveCellBroadcastChnlReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_RemoveCellBroadcastChnlReq_Req_t")

	if(
		xdr_uchar_ptr_t(xdrs, &rsp->newCBMI) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_GetCBMI_Rsp_t(void* xdrs, CAPI2_SmsApi_GetCBMI_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_GetCBMI_Rsp_t")

	if(
		xdr_SMS_CB_MSG_IDS_t(xdrs, &rsp->mids) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_GetCbLanguage_Rsp_t(void* xdrs, CAPI2_SmsApi_GetCbLanguage_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_GetCbLanguage_Rsp_t")

	if(
		xdr_MS_T_MN_CB_LANGUAGES(xdrs, &rsp->langs) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_AddCellBroadcastLangReq_Req_t(void* xdrs, CAPI2_SmsApi_AddCellBroadcastLangReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_AddCellBroadcastLangReq_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->newCBLang,"newCBLang") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_RemoveCellBroadcastLangReq_Req_t(void* xdrs, CAPI2_SmsApi_RemoveCellBroadcastLangReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_RemoveCellBroadcastLangReq_Req_t")

	if(
		xdr_uchar_ptr_t(xdrs, &rsp->cbLang) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_SetCBIgnoreDuplFlag_Req_t(void* xdrs, CAPI2_SmsApi_SetCBIgnoreDuplFlag_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_SetCBIgnoreDuplFlag_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->ignoreDupl,"ignoreDupl") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_GetCBIgnoreDuplFlag_Rsp_t(void* xdrs, CAPI2_SmsApi_GetCBIgnoreDuplFlag_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_GetCBIgnoreDuplFlag_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_SetVMIndOnOff_Req_t(void* xdrs, CAPI2_SmsApi_SetVMIndOnOff_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_SetVMIndOnOff_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->on_off,"on_off") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_IsVMIndEnabled_Rsp_t(void* xdrs, CAPI2_SmsApi_IsVMIndEnabled_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_IsVMIndEnabled_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_GetVMWaitingStatus_Rsp_t(void* xdrs, CAPI2_SmsApi_GetVMWaitingStatus_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_GetVMWaitingStatus_Rsp_t")

	if(
		xdr_SmsVoicemailInd_t(xdrs, &rsp->vmInd) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_GetNumOfVmscNumber_Rsp_t(void* xdrs, CAPI2_SmsApi_GetNumOfVmscNumber_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_GetNumOfVmscNumber_Rsp_t")

	if(
		_xdr_UInt8(xdrs, &rsp->num,"num") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_GetVmscNumber_Req_t(void* xdrs, CAPI2_SmsApi_GetVmscNumber_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_GetVmscNumber_Req_t")

	if(
		xdr_SIM_MWI_TYPE_t(xdrs, &rsp->vmsc_type) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_GetVmscNumber_Rsp_t(void* xdrs, CAPI2_SmsApi_GetVmscNumber_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_GetVmscNumber_Rsp_t")

	if(
		xdr_SmsAddress_t(xdrs, &rsp->vmsc) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_UpdateVmscNumberReq_Req_t(void* xdrs, CAPI2_SmsApi_UpdateVmscNumberReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SmsApi_UpdateVmscNumberReq_Req_t")

	if(
		xdr_SIM_MWI_TYPE_t(xdrs, &rsp->vmsc_type) &&
		xdr_uchar_ptr_t(xdrs, &rsp->vmscNum) &&
		_xdr_UInt8(xdrs, &rsp->numType,"numType") &&
		xdr_UInt8(xdrs, &rsp->alphaLen) &&
		xdr_copy_len(&len, (u_int)(rsp->alphaLen)) && xdr_bytes(xdrs, (char **)(void*)&rsp->alpha, &len, 0xFFFF) &&
		_xdr_UInt8(xdrs, &rsp->alphaCoding,"alphaCoding") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_GetSMSBearerPreference_Rsp_t(void* xdrs, CAPI2_SmsApi_GetSMSBearerPreference_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_GetSMSBearerPreference_Rsp_t")

	if(
		 xdr_SMS_BEARER_PREFERENCE_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_SetSMSBearerPreference_Req_t(void* xdrs, CAPI2_SmsApi_SetSMSBearerPreference_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_SetSMSBearerPreference_Req_t")

	if(
		xdr_SMS_BEARER_PREFERENCE_t(xdrs, &rsp->pref) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_SetSmsReadStatusChangeMode_Req_t(void* xdrs, CAPI2_SmsApi_SetSmsReadStatusChangeMode_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_SetSmsReadStatusChangeMode_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->mode,"mode") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_GetSmsReadStatusChangeMode_Rsp_t(void* xdrs, CAPI2_SmsApi_GetSmsReadStatusChangeMode_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_GetSmsReadStatusChangeMode_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_ChangeSmsStatusReq_Req_t(void* xdrs, CAPI2_SmsApi_ChangeSmsStatusReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_ChangeSmsStatusReq_Req_t")

	if(
		xdr_SmsStorage_t(xdrs, &rsp->storageType) &&
		_xdr_UInt16(xdrs, &rsp->index,"index") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_SendMEStoredStatusInd_Req_t(void* xdrs, CAPI2_SmsApi_SendMEStoredStatusInd_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_SendMEStoredStatusInd_Req_t")

	if(
		xdr_MEAccess_t(xdrs, &rsp->meResult) &&
		_xdr_UInt16(xdrs, &rsp->slotNumber,"slotNumber") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_SendMERetrieveSmsDataInd_Req_t(void* xdrs, CAPI2_SmsApi_SendMERetrieveSmsDataInd_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SmsApi_SendMERetrieveSmsDataInd_Req_t")

	if(
		xdr_MEAccess_t(xdrs, &rsp->meResult) &&
		_xdr_UInt16(xdrs, &rsp->slotNumber,"slotNumber") &&
		xdr_UInt16(xdrs, &rsp->inLen) &&
		xdr_copy_len(&len, (u_int)(rsp->inLen)) && xdr_bytes(xdrs, (char **)(void*)&rsp->inSms, &len, 0xFFFF) &&
		xdr_SIMSMSMesgStatus_t(xdrs, &rsp->status) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_SendMERemovedStatusInd_Req_t(void* xdrs, CAPI2_SmsApi_SendMERemovedStatusInd_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_SendMERemovedStatusInd_Req_t")

	if(
		xdr_MEAccess_t(xdrs, &rsp->meResult) &&
		_xdr_UInt16(xdrs, &rsp->slotNumber,"slotNumber") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_SetSmsStoredState_Req_t(void* xdrs, CAPI2_SmsApi_SetSmsStoredState_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_SetSmsStoredState_Req_t")

	if(
		xdr_SmsStorage_t(xdrs, &rsp->storageType) &&
		_xdr_UInt16(xdrs, &rsp->index,"index") &&
		xdr_SIMSMSMesgStatus_t(xdrs, &rsp->status) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_IsCachedDataReady_Rsp_t(void* xdrs, CAPI2_SmsApi_IsCachedDataReady_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_IsCachedDataReady_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_GetEnhancedVMInfoIEI_Rsp_t(void* xdrs, CAPI2_SmsApi_GetEnhancedVMInfoIEI_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_GetEnhancedVMInfoIEI_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SmsEnhancedVMInd_t ), xdr_SmsEnhancedVMInd_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_SetAllNewMsgDisplayPref_Req_t(void* xdrs, CAPI2_SmsApi_SetAllNewMsgDisplayPref_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_SetAllNewMsgDisplayPref_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->mode,sizeof( UInt8 ), xdr_UInt8) &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->mt,sizeof( UInt8 ), xdr_UInt8) &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->bm,sizeof( UInt8 ), xdr_UInt8) &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->ds,sizeof( UInt8 ), xdr_UInt8) &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->bfr,sizeof( UInt8 ), xdr_UInt8) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsApi_SendAckToNetwork_Req_t(void* xdrs, CAPI2_SmsApi_SendAckToNetwork_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsApi_SendAckToNetwork_Req_t")

	if(
		xdr_SmsMti_t(xdrs, &rsp->mti) &&
		xdr_SmsAckNetworkType_t(xdrs, &rsp->ackType) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SMS_SetPDAStorageOverFlowFlag_Req_t(void* xdrs, CAPI2_SMS_SetPDAStorageOverFlowFlag_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_SetPDAStorageOverFlowFlag_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->flag,"flag") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_ISimApi_SendAuthenAkaReq_Req_t(void* xdrs, CAPI2_ISimApi_SendAuthenAkaReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_ISimApi_SendAuthenAkaReq_Req_t")

	if(
		xdr_UInt16(xdrs, &rsp->rand_len) &&
		xdr_copy_len(&len, (u_int)(rsp->rand_len)) && xdr_bytes(xdrs, (char **)(void*)&rsp->rand_data, &len, 0xFFFF) &&
		xdr_UInt16(xdrs, &rsp->autn_len) &&
		xdr_copy_len(&len, (u_int)(rsp->autn_len)) && xdr_bytes(xdrs, (char **)(void*)&rsp->autn_data, &len, 0xFFFF) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_ISimApi_SendAuthenAkaReq_Rsp_t(void* xdrs, CAPI2_ISimApi_SendAuthenAkaReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_ISimApi_SendAuthenAkaReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( ISIM_AUTHEN_AKA_RSP_t ), xdr_ISIM_AUTHEN_AKA_RSP_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_ISimApi_IsIsimSupported_Rsp_t(void* xdrs, CAPI2_ISimApi_IsIsimSupported_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_ISimApi_IsIsimSupported_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_ISimApi_IsIsimActivated_Rsp_t(void* xdrs, CAPI2_ISimApi_IsIsimActivated_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_ISimApi_IsIsimActivated_Rsp_t")

	if(
		_xdr_UInt8(xdrs, &rsp->socket_id,"socket_id") &&
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_ISimApi_ActivateIsimAppli_Rsp_t(void* xdrs, CAPI2_ISimApi_ActivateIsimAppli_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_ISimApi_ActivateIsimAppli_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( ISIM_ACTIVATE_RSP_t ), xdr_ISIM_ACTIVATE_RSP_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_ISimApi_SendAuthenHttpReq_Req_t(void* xdrs, CAPI2_ISimApi_SendAuthenHttpReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_ISimApi_SendAuthenHttpReq_Req_t")

	if(
		xdr_UInt16(xdrs, &rsp->realm_len) &&
		xdr_copy_len(&len, (u_int)(rsp->realm_len)) && xdr_bytes(xdrs, (char **)(void*)&rsp->realm_data, &len, 0xFFFF) &&
		xdr_UInt16(xdrs, &rsp->nonce_len) &&
		xdr_copy_len(&len, (u_int)(rsp->nonce_len)) && xdr_bytes(xdrs, (char **)(void*)&rsp->nonce_data, &len, 0xFFFF) &&
		xdr_UInt16(xdrs, &rsp->cnonce_len) &&
		xdr_copy_len(&len, (u_int)(rsp->cnonce_len)) && xdr_bytes(xdrs, (char **)(void*)&rsp->cnonce_data, &len, 0xFFFF) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_ISimApi_SendAuthenHttpReq_Rsp_t(void* xdrs, CAPI2_ISimApi_SendAuthenHttpReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_ISimApi_SendAuthenHttpReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( ISIM_AUTHEN_HTTP_RSP_t ), xdr_ISIM_AUTHEN_HTTP_RSP_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_ISimApi_SendAuthenGbaNafReq_Req_t(void* xdrs, CAPI2_ISimApi_SendAuthenGbaNafReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_ISimApi_SendAuthenGbaNafReq_Req_t")

	if(
		xdr_UInt16(xdrs, &rsp->naf_id_len) &&
		xdr_copy_len(&len, (u_int)(rsp->naf_id_len)) && xdr_bytes(xdrs, (char **)(void*)&rsp->naf_id_data, &len, 0xFFFF) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_ISimApi_SendAuthenGbaNafReq_Rsp_t(void* xdrs, CAPI2_ISimApi_SendAuthenGbaNafReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_ISimApi_SendAuthenGbaNafReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( ISIM_AUTHEN_GBA_NAF_RSP_t ), xdr_ISIM_AUTHEN_GBA_NAF_RSP_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_ISimApi_SendAuthenGbaBootReq_Req_t(void* xdrs, CAPI2_ISimApi_SendAuthenGbaBootReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_ISimApi_SendAuthenGbaBootReq_Req_t")

	if(
		xdr_UInt16(xdrs, &rsp->rand_len) &&
		xdr_copy_len(&len, (u_int)(rsp->rand_len)) && xdr_bytes(xdrs, (char **)(void*)&rsp->rand_data, &len, 0xFFFF) &&
		xdr_UInt16(xdrs, &rsp->autn_len) &&
		xdr_copy_len(&len, (u_int)(rsp->autn_len)) && xdr_bytes(xdrs, (char **)(void*)&rsp->autn_data, &len, 0xFFFF) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_ISimApi_SendAuthenGbaBootReq_Rsp_t(void* xdrs, CAPI2_ISimApi_SendAuthenGbaBootReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_ISimApi_SendAuthenGbaBootReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( ISIM_AUTHEN_GBA_BOOT_RSP_t ), xdr_ISIM_AUTHEN_GBA_BOOT_RSP_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_GetAlpha_Req_t(void* xdrs, CAPI2_PbkApi_GetAlpha_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PbkApi_GetAlpha_Req_t")

	if(
		xdr_char_ptr_t(xdrs, &rsp->number) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_GetAlpha_Rsp_t(void* xdrs, CAPI2_PbkApi_GetAlpha_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PbkApi_GetAlpha_Rsp_t")

	if(
		xdr_PBK_API_Name_t(xdrs, &rsp->alpha) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_IsEmergencyCallNumber_Req_t(void* xdrs, CAPI2_PbkApi_IsEmergencyCallNumber_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PbkApi_IsEmergencyCallNumber_Req_t")

	if(
		xdr_char_ptr_t(xdrs, &rsp->phone_number) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_IsEmergencyCallNumber_Rsp_t(void* xdrs, CAPI2_PbkApi_IsEmergencyCallNumber_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PbkApi_IsEmergencyCallNumber_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_IsPartialEmergencyCallNumber_Req_t(void* xdrs, CAPI2_PbkApi_IsPartialEmergencyCallNumber_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PbkApi_IsPartialEmergencyCallNumber_Req_t")

	if(
		xdr_char_ptr_t(xdrs, &rsp->phone_number) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_IsPartialEmergencyCallNumber_Rsp_t(void* xdrs, CAPI2_PbkApi_IsPartialEmergencyCallNumber_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PbkApi_IsPartialEmergencyCallNumber_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_SendInfoReq_Req_t(void* xdrs, CAPI2_PbkApi_SendInfoReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PbkApi_SendInfoReq_Req_t")

	if(
		xdr_PBK_Id_t(xdrs, &rsp->pbk_id) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_SendInfoReq_Rsp_t(void* xdrs, CAPI2_PbkApi_SendInfoReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PbkApi_SendInfoReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( PBK_INFO_RSP_t ), xdr_PBK_INFO_RSP_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_SendFindAlphaMatchMultipleReq_Req_t(void* xdrs, CAPI2_PbkApi_SendFindAlphaMatchMultipleReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_PbkApi_SendFindAlphaMatchMultipleReq_Req_t")

	if(
		xdr_PBK_Id_t(xdrs, &rsp->pbk_id) &&
		xdr_ALPHA_CODING_t(xdrs, &rsp->alpha_coding) &&
		_xdr_UInt8(xdrs, &rsp->alpha_size,"alpha_size") &&
		xdr_copy_len(&len, (u_int)(rsp->alpha_size)) && xdr_bytes(xdrs, (char **)(void*)&rsp->alpha, &len, 0xFFFF) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_SendFindAlphaMatchMultipleReq_Rsp_t(void* xdrs, CAPI2_PbkApi_SendFindAlphaMatchMultipleReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PbkApi_SendFindAlphaMatchMultipleReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( PBK_ENTRY_DATA_RSP_t ), xdr_PBK_ENTRY_DATA_RSP_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_SendFindAlphaMatchOneReq_Req_t(void* xdrs, CAPI2_PbkApi_SendFindAlphaMatchOneReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_PbkApi_SendFindAlphaMatchOneReq_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->numOfPbk,"numOfPbk") &&
		xdr_copy_len(&len, (u_int)(rsp->numOfPbk)) && xdr_bytes(xdrs, (char **)(void*)&rsp->pbk_id, &len, 0xFFFF) &&
		xdr_ALPHA_CODING_t(xdrs, &rsp->alpha_coding) &&
		_xdr_UInt8(xdrs, &rsp->alpha_size,"alpha_size") &&
		xdr_copy_len(&len, (u_int)(rsp->alpha_size)) && xdr_bytes(xdrs, (char **)(void*)&rsp->alpha, &len, 0xFFFF) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_IsReady_Rsp_t(void* xdrs, CAPI2_PbkApi_IsReady_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PbkApi_IsReady_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_SendReadEntryReq_Req_t(void* xdrs, CAPI2_PbkApi_SendReadEntryReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PbkApi_SendReadEntryReq_Req_t")

	if(
		xdr_PBK_Id_t(xdrs, &rsp->pbk_id) &&
		_xdr_UInt16(xdrs, &rsp->start_index,"start_index") &&
		_xdr_UInt16(xdrs, &rsp->end_index,"end_index") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_SendWriteEntryReq_Req_t(void* xdrs, CAPI2_PbkApi_SendWriteEntryReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_PbkApi_SendWriteEntryReq_Req_t")

	if(
		xdr_PBK_Id_t(xdrs, &rsp->pbk_id) &&
		_xdr_Boolean(xdrs, &rsp->special_fax_num,"special_fax_num") &&
		_xdr_UInt16(xdrs, &rsp->index,"index") &&
		_xdr_UInt8(xdrs, &rsp->type_of_number,"type_of_number") &&
		xdr_char_ptr_t(xdrs, &rsp->number) &&
		xdr_ALPHA_CODING_t(xdrs, &rsp->alpha_coding) &&
		_xdr_UInt8(xdrs, &rsp->alpha_size,"alpha_size") &&
		xdr_copy_len(&len, (u_int)(rsp->alpha_size)) && xdr_bytes(xdrs, (char **)(void*)&rsp->alpha, &len, 0xFFFF) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_SendWriteEntryReq_Rsp_t(void* xdrs, CAPI2_PbkApi_SendWriteEntryReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PbkApi_SendWriteEntryReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( PBK_WRITE_ENTRY_RSP_t ), xdr_PBK_WRITE_ENTRY_RSP_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_SendUpdateEntryReq_Req_t(void* xdrs, CAPI2_PbkApi_SendUpdateEntryReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_PbkApi_SendUpdateEntryReq_Req_t")

	if(
		xdr_PBK_Id_t(xdrs, &rsp->pbk_id) &&
		_xdr_Boolean(xdrs, &rsp->special_fax_num,"special_fax_num") &&
		_xdr_UInt16(xdrs, &rsp->index,"index") &&
		_xdr_UInt8(xdrs, &rsp->type_of_number,"type_of_number") &&
		xdr_char_ptr_t(xdrs, &rsp->number) &&
		xdr_ALPHA_CODING_t(xdrs, &rsp->alpha_coding) &&
		_xdr_UInt8(xdrs, &rsp->alpha_size,"alpha_size") &&
		xdr_copy_len(&len, (u_int)(rsp->alpha_size)) && xdr_bytes(xdrs, (char **)(void*)&rsp->alpha, &len, 0xFFFF) &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->usim_adn_ext_data,sizeof( USIM_PBK_EXT_DATA_t ), xdr_USIM_PBK_EXT_DATA_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_SendIsNumDiallableReq_Req_t(void* xdrs, CAPI2_PbkApi_SendIsNumDiallableReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PbkApi_SendIsNumDiallableReq_Req_t")

	if(
		xdr_char_ptr_t(xdrs, &rsp->number) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_SendIsNumDiallableReq_Rsp_t(void* xdrs, CAPI2_PbkApi_SendIsNumDiallableReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PbkApi_SendIsNumDiallableReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( PBK_CHK_NUM_DIALLABLE_RSP_t ), xdr_PBK_CHK_NUM_DIALLABLE_RSP_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_IsNumDiallable_Req_t(void* xdrs, CAPI2_PbkApi_IsNumDiallable_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PbkApi_IsNumDiallable_Req_t")

	if(
		xdr_char_ptr_t(xdrs, &rsp->number) &&
		_xdr_Boolean(xdrs, &rsp->is_voice_call,"is_voice_call") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_IsNumDiallable_Rsp_t(void* xdrs, CAPI2_PbkApi_IsNumDiallable_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PbkApi_IsNumDiallable_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_IsNumBarred_Req_t(void* xdrs, CAPI2_PbkApi_IsNumBarred_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PbkApi_IsNumBarred_Req_t")

	if(
		xdr_char_ptr_t(xdrs, &rsp->number) &&
		_xdr_Boolean(xdrs, &rsp->is_voice_call,"is_voice_call") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_IsNumBarred_Rsp_t(void* xdrs, CAPI2_PbkApi_IsNumBarred_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PbkApi_IsNumBarred_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_IsUssdDiallable_Req_t(void* xdrs, CAPI2_PbkApi_IsUssdDiallable_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_PbkApi_IsUssdDiallable_Req_t")

	if(
		xdr_UInt8(xdrs, &rsp->dcsLen) &&
		xdr_copy_len(&len, (u_int)(rsp->dcsLen)) && xdr_bytes(xdrs, (char **)(void*)&rsp->data, &len, 0xFFFF) &&
		_xdr_UInt8(xdrs, &rsp->dcs,"dcs") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PbkApi_IsUssdDiallable_Rsp_t(void* xdrs, CAPI2_PbkApi_IsUssdDiallable_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PbkApi_IsUssdDiallable_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_SetPDPContext_Req_t(void* xdrs, CAPI2_PdpApi_SetPDPContext_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_SetPDPContext_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
		_xdr_UInt8(xdrs, &rsp->numParms,"numParms") &&
		xdr_uchar_ptr_t(xdrs, &rsp->pdpType) &&
		xdr_uchar_ptr_t(xdrs, &rsp->apn) &&
		xdr_uchar_ptr_t(xdrs, &rsp->pdpAddr) &&
		_xdr_UInt8(xdrs, &rsp->dComp,"dComp") &&
		_xdr_UInt8(xdrs, &rsp->hComp,"hComp") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_SetSecPDPContext_Req_t(void* xdrs, CAPI2_PdpApi_SetSecPDPContext_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_SetSecPDPContext_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
		_xdr_UInt8(xdrs, &rsp->numParms,"numParms") &&
		_xdr_UInt8(xdrs, &rsp->priCid,"priCid") &&
		_xdr_UInt8(xdrs, &rsp->dComp,"dComp") &&
		_xdr_UInt8(xdrs, &rsp->hComp,"hComp") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetGPRSQoS_Req_t(void* xdrs, CAPI2_PdpApi_GetGPRSQoS_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetGPRSQoS_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetGPRSQoS_Rsp_t(void* xdrs, CAPI2_PdpApi_GetGPRSQoS_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetGPRSQoS_Rsp_t")

	if(
		_xdr_UInt8(xdrs, &rsp->prec,"prec") &&
		_xdr_UInt8(xdrs, &rsp->delay,"delay") &&
		_xdr_UInt8(xdrs, &rsp->rel,"rel") &&
		_xdr_UInt8(xdrs, &rsp->peak,"peak") &&
		_xdr_UInt8(xdrs, &rsp->mean,"mean") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_SetGPRSQoS_Req_t(void* xdrs, CAPI2_PdpApi_SetGPRSQoS_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_SetGPRSQoS_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
		_xdr_UInt8(xdrs, &rsp->NumPara,"NumPara") &&
		_xdr_UInt8(xdrs, &rsp->prec,"prec") &&
		_xdr_UInt8(xdrs, &rsp->delay,"delay") &&
		_xdr_UInt8(xdrs, &rsp->rel,"rel") &&
		_xdr_UInt8(xdrs, &rsp->peak,"peak") &&
		_xdr_UInt8(xdrs, &rsp->mean,"mean") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetGPRSMinQoS_Req_t(void* xdrs, CAPI2_PdpApi_GetGPRSMinQoS_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetGPRSMinQoS_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetGPRSMinQoS_Rsp_t(void* xdrs, CAPI2_PdpApi_GetGPRSMinQoS_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetGPRSMinQoS_Rsp_t")

	if(
		_xdr_UInt8(xdrs, &rsp->prec,"prec") &&
		_xdr_UInt8(xdrs, &rsp->delay,"delay") &&
		_xdr_UInt8(xdrs, &rsp->rel,"rel") &&
		_xdr_UInt8(xdrs, &rsp->peak,"peak") &&
		_xdr_UInt8(xdrs, &rsp->mean,"mean") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_SetGPRSMinQoS_Req_t(void* xdrs, CAPI2_PdpApi_SetGPRSMinQoS_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_SetGPRSMinQoS_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
		_xdr_UInt8(xdrs, &rsp->NumPara,"NumPara") &&
		_xdr_UInt8(xdrs, &rsp->prec,"prec") &&
		_xdr_UInt8(xdrs, &rsp->delay,"delay") &&
		_xdr_UInt8(xdrs, &rsp->rel,"rel") &&
		_xdr_UInt8(xdrs, &rsp->peak,"peak") &&
		_xdr_UInt8(xdrs, &rsp->mean,"mean") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_NetRegApi_SendCombinedAttachReq_Req_t(void* xdrs, CAPI2_NetRegApi_SendCombinedAttachReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_NetRegApi_SendCombinedAttachReq_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->SIMPresent,"SIMPresent") &&
		xdr_SIMType_t(xdrs, &rsp->SIMType) &&
		xdr_RegType_t(xdrs, &rsp->regType) &&
		xdr_PLMN_t(xdrs, &rsp->plmn) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_NetRegApi_SendDetachReq_Req_t(void* xdrs, CAPI2_NetRegApi_SendDetachReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_NetRegApi_SendDetachReq_Req_t")

	if(
		xdr_DeRegCause_t(xdrs, &rsp->cause) &&
		xdr_RegType_t(xdrs, &rsp->regType) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_GetGPRSAttachStatus_Rsp_t(void* xdrs, CAPI2_MS_GetGPRSAttachStatus_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetGPRSAttachStatus_Rsp_t")

	if(
		 xdr_AttachState_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_IsSecondaryPdpDefined_Req_t(void* xdrs, CAPI2_PdpApi_IsSecondaryPdpDefined_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_IsSecondaryPdpDefined_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_IsSecondaryPdpDefined_Rsp_t(void* xdrs, CAPI2_PdpApi_IsSecondaryPdpDefined_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_IsSecondaryPdpDefined_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PchExApi_SendPDPActivateReq_Req_t(void* xdrs, CAPI2_PchExApi_SendPDPActivateReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PchExApi_SendPDPActivateReq_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
		xdr_PCHActivateReason_t(xdrs, &rsp->reason) &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->protConfig,sizeof( PCHProtConfig_t ), xdr_PCHProtConfig_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PchExApi_SendPDPActivateReq_Rsp_t(void* xdrs, CAPI2_PchExApi_SendPDPActivateReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PchExApi_SendPDPActivateReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( PDP_SendPDPActivateReq_Rsp_t ), xdr_PDP_SendPDPActivateReq_Rsp_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PchExApi_SendPDPModifyReq_Req_t(void* xdrs, CAPI2_PchExApi_SendPDPModifyReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PchExApi_SendPDPModifyReq_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PchExApi_SendPDPModifyReq_Rsp_t(void* xdrs, CAPI2_PchExApi_SendPDPModifyReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PchExApi_SendPDPModifyReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( PDP_SendPDPModifyReq_Rsp_t ), xdr_PDP_SendPDPModifyReq_Rsp_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PchExApi_SendPDPDeactivateReq_Req_t(void* xdrs, CAPI2_PchExApi_SendPDPDeactivateReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PchExApi_SendPDPDeactivateReq_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PchExApi_SendPDPDeactivateReq_Rsp_t(void* xdrs, CAPI2_PchExApi_SendPDPDeactivateReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PchExApi_SendPDPDeactivateReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( PDP_SendPDPDeactivateReq_Rsp_t ), xdr_PDP_SendPDPDeactivateReq_Rsp_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PchExApi_SendPDPActivateSecReq_Req_t(void* xdrs, CAPI2_PchExApi_SendPDPActivateSecReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PchExApi_SendPDPActivateSecReq_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PchExApi_SendPDPActivateSecReq_Rsp_t(void* xdrs, CAPI2_PchExApi_SendPDPActivateSecReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PchExApi_SendPDPActivateSecReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( PDP_SendPDPActivateSecReq_Rsp_t ), xdr_PDP_SendPDPActivateSecReq_Rsp_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetGPRSActivateStatus_Rsp_t(void* xdrs, CAPI2_PdpApi_GetGPRSActivateStatus_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetGPRSActivateStatus_Rsp_t")

	if(
		_xdr_UInt8(xdrs, &rsp->numActiveCid,"numActiveCid") &&
		xdr_vector(xdrs, (caddr_t)(void*)rsp->outCidActivate,MAX_PDP_CONTEXTS, sizeof(GPRSActivate_t ), xdr_GPRSActivate_t ) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_NetRegApi_SetMSClass_Req_t(void* xdrs, CAPI2_NetRegApi_SetMSClass_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_NetRegApi_SetMSClass_Req_t")

	if(
		xdr_MSClass_t(xdrs, &rsp->msClass) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PDP_GetMSClass_Rsp_t(void* xdrs, CAPI2_PDP_GetMSClass_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PDP_GetMSClass_Rsp_t")

	if(
		 xdr_MSClass_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetUMTSTft_Req_t(void* xdrs, CAPI2_PdpApi_GetUMTSTft_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetUMTSTft_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetUMTSTft_Rsp_t(void* xdrs, CAPI2_PdpApi_GetUMTSTft_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetUMTSTft_Rsp_t")

	if(
		xdr_PCHTrafficFlowTemplate_t(xdrs, &rsp->pUMTSTft) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_SetUMTSTft_Req_t(void* xdrs, CAPI2_PdpApi_SetUMTSTft_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_SetUMTSTft_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->pTft_in,sizeof( PCHTrafficFlowTemplate_t ), xdr_PCHTrafficFlowTemplate_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_DeleteUMTSTft_Req_t(void* xdrs, CAPI2_PdpApi_DeleteUMTSTft_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_DeleteUMTSTft_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_DeactivateSNDCPConnection_Req_t(void* xdrs, CAPI2_PdpApi_DeactivateSNDCPConnection_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_DeactivateSNDCPConnection_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetR99UMTSMinQoS_Req_t(void* xdrs, CAPI2_PdpApi_GetR99UMTSMinQoS_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetR99UMTSMinQoS_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetR99UMTSMinQoS_Rsp_t(void* xdrs, CAPI2_PdpApi_GetR99UMTSMinQoS_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetR99UMTSMinQoS_Rsp_t")

	if(
		xdr_PCHR99QosProfile_t(xdrs, &rsp->pR99Qos) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetR99UMTSQoS_Req_t(void* xdrs, CAPI2_PdpApi_GetR99UMTSQoS_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetR99UMTSQoS_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetR99UMTSQoS_Rsp_t(void* xdrs, CAPI2_PdpApi_GetR99UMTSQoS_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetR99UMTSQoS_Rsp_t")

	if(
		xdr_PCHR99QosProfile_t(xdrs, &rsp->pR99Qos) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetUMTSMinQoS_Req_t(void* xdrs, CAPI2_PdpApi_GetUMTSMinQoS_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetUMTSMinQoS_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetUMTSMinQoS_Rsp_t(void* xdrs, CAPI2_PdpApi_GetUMTSMinQoS_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetUMTSMinQoS_Rsp_t")

	if(
		xdr_PCHUMTSQosProfile_t(xdrs, &rsp->pDstQos) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetUMTSQoS_Req_t(void* xdrs, CAPI2_PdpApi_GetUMTSQoS_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetUMTSQoS_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetUMTSQoS_Rsp_t(void* xdrs, CAPI2_PdpApi_GetUMTSQoS_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetUMTSQoS_Rsp_t")

	if(
		xdr_PCHUMTSQosProfile_t(xdrs, &rsp->pDstQos) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetNegQoS_Req_t(void* xdrs, CAPI2_PdpApi_GetNegQoS_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetNegQoS_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetNegQoS_Rsp_t(void* xdrs, CAPI2_PdpApi_GetNegQoS_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetNegQoS_Rsp_t")

	if(
		xdr_PCHR99QosProfile_t(xdrs, &rsp->pR99Qos) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_SetR99UMTSMinQoS_Req_t(void* xdrs, CAPI2_PdpApi_SetR99UMTSMinQoS_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_SetR99UMTSMinQoS_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->pR99MinQos,sizeof( PCHR99QosProfile_t ), xdr_PCHR99QosProfile_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_SetR99UMTSQoS_Req_t(void* xdrs, CAPI2_PdpApi_SetR99UMTSQoS_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_SetR99UMTSQoS_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->pR99Qos,sizeof( PCHR99QosProfile_t ), xdr_PCHR99QosProfile_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_SetUMTSMinQoS_Req_t(void* xdrs, CAPI2_PdpApi_SetUMTSMinQoS_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_SetUMTSMinQoS_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->pMinQos,sizeof( PCHUMTSQosProfile_t ), xdr_PCHUMTSQosProfile_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_SetUMTSQoS_Req_t(void* xdrs, CAPI2_PdpApi_SetUMTSQoS_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_SetUMTSQoS_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->pQos,sizeof( PCHUMTSQosProfile_t ), xdr_PCHUMTSQosProfile_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetNegotiatedParms_Req_t(void* xdrs, CAPI2_PdpApi_GetNegotiatedParms_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetNegotiatedParms_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetNegotiatedParms_Rsp_t(void* xdrs, CAPI2_PdpApi_GetNegotiatedParms_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetNegotiatedParms_Rsp_t")

	if(
		xdr_PCHNegotiatedParms_t(xdrs, &rsp->pParms) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_IsGprsCallActive_Req_t(void* xdrs, CAPI2_MS_IsGprsCallActive_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_IsGprsCallActive_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->chan,"chan") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_IsGprsCallActive_Rsp_t(void* xdrs, CAPI2_MS_IsGprsCallActive_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_IsGprsCallActive_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_SetChanGprsCallActive_Req_t(void* xdrs, CAPI2_MS_SetChanGprsCallActive_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_SetChanGprsCallActive_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->chan,"chan") &&
		_xdr_Boolean(xdrs, &rsp->active,"active") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_SetCidForGprsActiveChan_Req_t(void* xdrs, CAPI2_MS_SetCidForGprsActiveChan_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_SetCidForGprsActiveChan_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->chan,"chan") &&
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetPPPModemCid_Rsp_t(void* xdrs, CAPI2_PdpApi_GetPPPModemCid_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetPPPModemCid_Rsp_t")

	if(
		 xdr_PCHCid_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_GetGprsActiveChanFromCid_Req_t(void* xdrs, CAPI2_MS_GetGprsActiveChanFromCid_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetGprsActiveChanFromCid_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_GetGprsActiveChanFromCid_Rsp_t(void* xdrs, CAPI2_MS_GetGprsActiveChanFromCid_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetGprsActiveChanFromCid_Rsp_t")

	if(
		 _xdr_UInt8(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_GetCidFromGprsActiveChan_Req_t(void* xdrs, CAPI2_MS_GetCidFromGprsActiveChan_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetCidFromGprsActiveChan_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->chan,"chan") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MS_GetCidFromGprsActiveChan_Rsp_t(void* xdrs, CAPI2_MS_GetCidFromGprsActiveChan_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetCidFromGprsActiveChan_Rsp_t")

	if(
		 _xdr_UInt8(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetPDPAddress_Req_t(void* xdrs, CAPI2_PdpApi_GetPDPAddress_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetPDPAddress_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetPDPAddress_Rsp_t(void* xdrs, CAPI2_PdpApi_GetPDPAddress_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetPDPAddress_Rsp_t")

	if(
		xdr_opaque(xdrs, (caddr_t)(void*)rsp->pdpAddr,PCH_PDP_ADDR_LEN ) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_SendTBFData_Req_t(void* xdrs, CAPI2_PdpApi_SendTBFData_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_SendTBFData_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
		xdr_UInt32(xdrs, &rsp->numberBytes) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_TftAddFilter_Req_t(void* xdrs, CAPI2_PdpApi_TftAddFilter_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_TftAddFilter_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->pPktFilter,sizeof( PCHPacketFilter_T ), xdr_PCHPacketFilter_T) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_SetPCHContextState_Req_t(void* xdrs, CAPI2_PdpApi_SetPCHContextState_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_SetPCHContextState_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
		xdr_PCHContextState_t(xdrs, &rsp->contextState) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_SetDefaultPDPContext_Req_t(void* xdrs, CAPI2_PdpApi_SetDefaultPDPContext_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_SetDefaultPDPContext_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->pDefaultContext,sizeof( PDPDefaultContext_t ), xdr_PDPDefaultContext_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PchExApi_GetDecodedProtConfig_Req_t(void* xdrs, CAPI2_PchExApi_GetDecodedProtConfig_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PchExApi_GetDecodedProtConfig_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PchExApi_GetDecodedProtConfig_Rsp_t(void* xdrs, CAPI2_PchExApi_GetDecodedProtConfig_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PchExApi_GetDecodedProtConfig_Rsp_t")

	if(
		xdr_PCHDecodedProtConfig_t(xdrs, &rsp->outParam) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PchExApi_BuildIpConfigOptions_Req_t(void* xdrs, CAPI2_PchExApi_BuildIpConfigOptions_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PchExApi_BuildIpConfigOptions_Req_t")

	if(
		xdr_char_ptr_t(xdrs, &rsp->username) &&
		xdr_char_ptr_t(xdrs, &rsp->password) &&
		xdr_IPConfigAuthType_t(xdrs, &rsp->authType) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PchExApi_BuildIpConfigOptions_Rsp_t(void* xdrs, CAPI2_PchExApi_BuildIpConfigOptions_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PchExApi_BuildIpConfigOptions_Rsp_t")

	if(
		xdr_PCHProtConfig_t(xdrs, &rsp->cie) &&
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PchExApi_BuildIpConfigOptions2_Req_t(void* xdrs, CAPI2_PchExApi_BuildIpConfigOptions2_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PchExApi_BuildIpConfigOptions2_Req_t")

	if(
		xdr_IPConfigAuthType_t(xdrs, &rsp->authType) &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->cc,sizeof( CHAP_ChallengeOptions_t ), xdr_CHAP_ChallengeOptions_t) &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->cr,sizeof( CHAP_ResponseOptions_t ), xdr_CHAP_ResponseOptions_t) &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->po,sizeof( PAP_CnfgOptions_t ), xdr_PAP_CnfgOptions_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PchExApi_BuildIpConfigOptions2_Rsp_t(void* xdrs, CAPI2_PchExApi_BuildIpConfigOptions2_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PchExApi_BuildIpConfigOptions2_Rsp_t")

	if(
		xdr_PCHProtConfig_t(xdrs, &rsp->ip_cnfg) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PchExApi_BuildIpConfigOptionsWithChapAuthType_Req_t(void* xdrs, CAPI2_PchExApi_BuildIpConfigOptionsWithChapAuthType_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PchExApi_BuildIpConfigOptionsWithChapAuthType_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->params,sizeof( PCHEx_ChapAuthType_t ), xdr_PCHEx_ChapAuthType_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PchExApi_BuildIpConfigOptionsWithChapAuthType_Rsp_t(void* xdrs, CAPI2_PchExApi_BuildIpConfigOptionsWithChapAuthType_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PchExApi_BuildIpConfigOptionsWithChapAuthType_Rsp_t")

	if(
		xdr_PCHProtConfig_t(xdrs, &rsp->ip_cnfg) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetDefaultQos_Rsp_t(void* xdrs, CAPI2_PdpApi_GetDefaultQos_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetDefaultQos_Rsp_t")

	if(
		xdr_PCHQosProfile_t(xdrs, &rsp->pQos) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_IsPDPContextActive_Req_t(void* xdrs, CAPI2_PdpApi_IsPDPContextActive_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_IsPDPContextActive_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_IsPDPContextActive_Rsp_t(void* xdrs, CAPI2_PdpApi_IsPDPContextActive_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_IsPDPContextActive_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_ActivateSNDCPConnection_Req_t(void* xdrs, CAPI2_PdpApi_ActivateSNDCPConnection_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_ActivateSNDCPConnection_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
		xdr_PCHL2P_t(xdrs, &rsp->l2p) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_ActivateSNDCPConnection_Rsp_t(void* xdrs, CAPI2_PdpApi_ActivateSNDCPConnection_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_ActivateSNDCPConnection_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( PDP_DataState_t ), xdr_PDP_DataState_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetPDPContext_Req_t(void* xdrs, CAPI2_PdpApi_GetPDPContext_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetPDPContext_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetPDPContext_Rsp_t(void* xdrs, CAPI2_PdpApi_GetPDPContext_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetPDPContext_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( PDPDefaultContext_t ), xdr_PDPDefaultContext_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetDefinedPDPContextCidList_Rsp_t(void* xdrs, CAPI2_PdpApi_GetDefinedPDPContextCidList_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetDefinedPDPContextCidList_Rsp_t")

	if(
		xdr_opaque(xdrs, (caddr_t)(void*)rsp->numCid,MAX_CID ) &&
		xdr_opaque(xdrs, (caddr_t)(void*)rsp->cidList,MAX_CID ) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYS_GetBootLoaderVersion_Req_t(void* xdrs, CAPI2_SYS_GetBootLoaderVersion_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYS_GetBootLoaderVersion_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->BootLoaderVersion_sz,"BootLoaderVersion_sz") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYS_GetBootLoaderVersion_Rsp_t(void* xdrs, CAPI2_SYS_GetBootLoaderVersion_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYS_GetBootLoaderVersion_Rsp_t")

	if(
		xdr_opaque(xdrs, (caddr_t)(void*)rsp->BootLoaderVersion,80 ) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYS_GetDSFVersion_Req_t(void* xdrs, CAPI2_SYS_GetDSFVersion_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYS_GetDSFVersion_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->DSFVersion_sz,"DSFVersion_sz") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYS_GetDSFVersion_Rsp_t(void* xdrs, CAPI2_SYS_GetDSFVersion_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYS_GetDSFVersion_Rsp_t")

	if(
		xdr_opaque(xdrs, (caddr_t)(void*)rsp->DSFVersion,80 ) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_USimApi_GetUstData_Rsp_t(void* xdrs, CAPI2_USimApi_GetUstData_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USimApi_GetUstData_Rsp_t")

	if(
		xdr_UST_DATA_t(xdrs, &rsp->outUstPtr) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PATCH_GetRevision_Rsp_t(void* xdrs, CAPI2_PATCH_GetRevision_Rsp_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_PATCH_GetRevision_Rsp_t")

	if(
		xdr_copy_len(&len, 16 ) && xdr_bytes(xdrs, (char **)(void*)&rsp->val, &len , ~0) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SS_SendCallForwardReq_Req_t(void* xdrs, CAPI2_SS_SendCallForwardReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_SendCallForwardReq_Req_t")

	if(
		xdr_SS_Mode_t(xdrs, &rsp->mode) &&
		xdr_SS_CallFwdReason_t(xdrs, &rsp->reason) &&
		xdr_SS_SvcCls_t(xdrs, &rsp->svcCls) &&
		_xdr_UInt8(xdrs, &rsp->waitToFwdSec,"waitToFwdSec") &&
		xdr_uchar_ptr_t(xdrs, &rsp->number) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SS_SendCallForwardReq_Rsp_t(void* xdrs, CAPI2_SS_SendCallForwardReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_SendCallForwardReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( CallForwardStatus_t ), xdr_CallForwardStatus_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SS_QueryCallForwardStatus_Req_t(void* xdrs, CAPI2_SS_QueryCallForwardStatus_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_QueryCallForwardStatus_Req_t")

	if(
		xdr_SS_CallFwdReason_t(xdrs, &rsp->reason) &&
		xdr_SS_SvcCls_t(xdrs, &rsp->svcCls) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SS_SendCallBarringReq_Req_t(void* xdrs, CAPI2_SS_SendCallBarringReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_SendCallBarringReq_Req_t")

	if(
		xdr_SS_Mode_t(xdrs, &rsp->mode) &&
		xdr_SS_CallBarType_t(xdrs, &rsp->callBarType) &&
		xdr_SS_SvcCls_t(xdrs, &rsp->svcCls) &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->password,sizeof( UInt8 ), xdr_UInt8) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SS_SendCallBarringReq_Rsp_t(void* xdrs, CAPI2_SS_SendCallBarringReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_SendCallBarringReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( CallBarringStatus_t ), xdr_CallBarringStatus_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SS_QueryCallBarringStatus_Req_t(void* xdrs, CAPI2_SS_QueryCallBarringStatus_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_QueryCallBarringStatus_Req_t")

	if(
		xdr_SS_CallBarType_t(xdrs, &rsp->callBarType) &&
		xdr_SS_SvcCls_t(xdrs, &rsp->svcCls) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SS_SendCallBarringPWDChangeReq_Req_t(void* xdrs, CAPI2_SS_SendCallBarringPWDChangeReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_SendCallBarringPWDChangeReq_Req_t")

	if(
		xdr_SS_CallBarType_t(xdrs, &rsp->callBarType) &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->oldPwd,sizeof( UInt8 ), xdr_UInt8) &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->newPwd,sizeof( UInt8 ), xdr_UInt8) &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->reNewPwd,sizeof( UInt8 ), xdr_UInt8) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SS_SendCallBarringPWDChangeReq_Rsp_t(void* xdrs, CAPI2_SS_SendCallBarringPWDChangeReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_SendCallBarringPWDChangeReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( NetworkCause_t ), xdr_NetworkCause_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SS_SendCallWaitingReq_Req_t(void* xdrs, CAPI2_SS_SendCallWaitingReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_SendCallWaitingReq_Req_t")

	if(
		xdr_SS_Mode_t(xdrs, &rsp->mode) &&
		xdr_SS_SvcCls_t(xdrs, &rsp->svcCls) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SS_SendCallWaitingReq_Rsp_t(void* xdrs, CAPI2_SS_SendCallWaitingReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_SendCallWaitingReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SS_ActivationStatus_t ), xdr_SS_ActivationStatus_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SS_QueryCallWaitingStatus_Req_t(void* xdrs, CAPI2_SS_QueryCallWaitingStatus_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_QueryCallWaitingStatus_Req_t")

	if(
		xdr_SS_SvcCls_t(xdrs, &rsp->svcCls) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SS_QueryCallingLineIDStatus_Rsp_t(void* xdrs, CAPI2_SS_QueryCallingLineIDStatus_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_QueryCallingLineIDStatus_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SS_ProvisionStatus_t ), xdr_SS_ProvisionStatus_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SS_QueryConnectedLineIDStatus_Rsp_t(void* xdrs, CAPI2_SS_QueryConnectedLineIDStatus_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_QueryConnectedLineIDStatus_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SS_ProvisionStatus_t ), xdr_SS_ProvisionStatus_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SS_QueryCallingLineRestrictionStatus_Rsp_t(void* xdrs, CAPI2_SS_QueryCallingLineRestrictionStatus_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_QueryCallingLineRestrictionStatus_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SS_ProvisionStatus_t ), xdr_SS_ProvisionStatus_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SS_QueryConnectedLineRestrictionStatus_Rsp_t(void* xdrs, CAPI2_SS_QueryConnectedLineRestrictionStatus_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_QueryConnectedLineRestrictionStatus_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SS_ProvisionStatus_t ), xdr_SS_ProvisionStatus_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SS_QueryCallingNAmePresentStatus_Rsp_t(void* xdrs, CAPI2_SS_QueryCallingNAmePresentStatus_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_QueryCallingNAmePresentStatus_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SS_ProvisionStatus_t ), xdr_SS_ProvisionStatus_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SS_SetCallingLineIDStatus_Req_t(void* xdrs, CAPI2_SS_SetCallingLineIDStatus_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_SetCallingLineIDStatus_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->enable,"enable") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SS_SetCallingLineIDStatus_Rsp_t(void* xdrs, CAPI2_SS_SetCallingLineIDStatus_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_SetCallingLineIDStatus_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( MS_LocalElemNotifyInd_t ), xdr_MS_LocalElemNotifyInd_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SS_SetCallingLineRestrictionStatus_Req_t(void* xdrs, CAPI2_SS_SetCallingLineRestrictionStatus_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_SetCallingLineRestrictionStatus_Req_t")

	if(
		xdr_CLIRMode_t(xdrs, &rsp->clir_mode) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SS_SetConnectedLineIDStatus_Req_t(void* xdrs, CAPI2_SS_SetConnectedLineIDStatus_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_SetConnectedLineIDStatus_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->enable,"enable") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SS_SetConnectedLineRestrictionStatus_Req_t(void* xdrs, CAPI2_SS_SetConnectedLineRestrictionStatus_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_SetConnectedLineRestrictionStatus_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->enable,"enable") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SS_SendUSSDConnectReq_Req_t(void* xdrs, CAPI2_SS_SendUSSDConnectReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_SendUSSDConnectReq_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->ussd,sizeof( USSDString_t ), xdr_USSDString_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SS_SendUSSDConnectReq_Rsp_t(void* xdrs, CAPI2_SS_SendUSSDConnectReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_SendUSSDConnectReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( USSDataInfo_t ), xdr_USSDataInfo_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SS_SendUSSDData_Req_t(void* xdrs, CAPI2_SS_SendUSSDData_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_SendUSSDData_Req_t")

	if(
		xdr_CallIndex_t(xdrs, &rsp->ussd_id) &&
		_xdr_UInt8(xdrs, &rsp->dcs,"dcs") &&
		_xdr_UInt8(xdrs, &rsp->dlen,"dlen") &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->ussdString,sizeof( UInt8 ), xdr_UInt8) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SsApi_DialStrSrvReq_Req_t(void* xdrs, CAPI2_SsApi_DialStrSrvReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SsApi_DialStrSrvReq_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->inDialStrSrvReqPtr,sizeof( SsApi_DialStrSrvReq_t ), xdr_SsApi_DialStrSrvReq_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SS_EndUSSDConnectReq_Req_t(void* xdrs, CAPI2_SS_EndUSSDConnectReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_EndUSSDConnectReq_Req_t")

	if(
		xdr_CallIndex_t(xdrs, &rsp->ussd_id) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SsApi_SsSrvReq_Req_t(void* xdrs, CAPI2_SsApi_SsSrvReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SsApi_SsSrvReq_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->inApiSrvReqPtr,sizeof( SsApi_SrvReq_t ), xdr_SsApi_SrvReq_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SsApi_UssdSrvReq_Req_t(void* xdrs, CAPI2_SsApi_UssdSrvReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SsApi_UssdSrvReq_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->inUssdSrvReqPtr,sizeof( SsApi_UssdSrvReq_t ), xdr_SsApi_UssdSrvReq_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SsApi_UssdDataReq_Req_t(void* xdrs, CAPI2_SsApi_UssdDataReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SsApi_UssdDataReq_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->inUssdDataReqPtr,sizeof( SsApi_UssdDataReq_t ), xdr_SsApi_UssdDataReq_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SsApi_SsReleaseReq_Req_t(void* xdrs, CAPI2_SsApi_SsReleaseReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SsApi_SsReleaseReq_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->inApiSrvReqPtr,sizeof( SsApi_SrvReq_t ), xdr_SsApi_SrvReq_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SsApi_DataReq_Req_t(void* xdrs, CAPI2_SsApi_DataReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SsApi_DataReq_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->inDataReqPtr,sizeof( SsApi_DataReq_t ), xdr_SsApi_DataReq_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SS_SsApiReqDispatcher_Req_t(void* xdrs, CAPI2_SS_SsApiReqDispatcher_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_SsApiReqDispatcher_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->inSsApiReqPtr,sizeof( SS_SsApiReq_t ), xdr_SS_SsApiReq_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SS_GetStr_Req_t(void* xdrs, CAPI2_SS_GetStr_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_GetStr_Req_t")

	if(
		xdr_SS_ConstString_t(xdrs, &rsp->strName) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SS_GetStr_Rsp_t(void* xdrs, CAPI2_SS_GetStr_Rsp_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SS_GetStr_Rsp_t")

	if(
		xdr_copy_len(&len, 60 ) && xdr_bytes(xdrs, (char **)(void*)&rsp->val, &len , ~0) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SS_GetClientID_Rsp_t(void* xdrs, CAPI2_SS_GetClientID_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_GetClientID_Rsp_t")

	if(
		 _xdr_UInt8(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SatkApi_GetCachedRootMenuPtr_Rsp_t(void* xdrs, CAPI2_SatkApi_GetCachedRootMenuPtr_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SatkApi_GetCachedRootMenuPtr_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SetupMenu_t ), xdr_SetupMenu_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SatkApi_SendLangSelectEvent_Req_t(void* xdrs, CAPI2_SatkApi_SendLangSelectEvent_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SatkApi_SendLangSelectEvent_Req_t")

	if(
		_xdr_UInt16(xdrs, &rsp->language,"language") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SatkApi_SendBrowserTermEvent_Req_t(void* xdrs, CAPI2_SatkApi_SendBrowserTermEvent_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SatkApi_SendBrowserTermEvent_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->user_term,"user_term") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SatkApi_CmdResp_Req_t(void* xdrs, CAPI2_SatkApi_CmdResp_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SatkApi_CmdResp_Req_t")

	if(
		xdr_SATK_EVENTS_t(xdrs, &rsp->toEvent) &&
		_xdr_UInt8(xdrs, &rsp->result1,"result1") &&
		_xdr_UInt8(xdrs, &rsp->result2,"result2") &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->inText,sizeof( SATKString_t ), xdr_SATKString_t) &&
		_xdr_UInt8(xdrs, &rsp->menuID,"menuID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SatkApi_CmdResp_Rsp_t(void* xdrs, CAPI2_SatkApi_CmdResp_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SatkApi_CmdResp_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SatkApi_DataServCmdResp_Req_t(void* xdrs, CAPI2_SatkApi_DataServCmdResp_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SatkApi_DataServCmdResp_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->response,sizeof( StkCmdRespond_t ), xdr_StkCmdRespond_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SatkApi_DataServCmdResp_Rsp_t(void* xdrs, CAPI2_SatkApi_DataServCmdResp_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SatkApi_DataServCmdResp_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SatkApi_SendDataServReq_Req_t(void* xdrs, CAPI2_SatkApi_SendDataServReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SatkApi_SendDataServReq_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->dsReq,sizeof( StkDataServReq_t ), xdr_StkDataServReq_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SatkApi_SendDataServReq_Rsp_t(void* xdrs, CAPI2_SatkApi_SendDataServReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SatkApi_SendDataServReq_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SatkApi_SendTerminalRsp_Req_t(void* xdrs, CAPI2_SatkApi_SendTerminalRsp_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SatkApi_SendTerminalRsp_Req_t")

	if(
		xdr_SATK_EVENTS_t(xdrs, &rsp->toEvent) &&
		_xdr_UInt8(xdrs, &rsp->result1,"result1") &&
		_xdr_UInt16(xdrs, &rsp->result2,"result2") &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->inText,sizeof( SATKString_t ), xdr_SATKString_t) &&
		_xdr_UInt8(xdrs, &rsp->menuID,"menuID") &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->ssFacIePtr,sizeof( UInt8 ), xdr_UInt8) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SatkApi_SendTerminalRsp_Rsp_t(void* xdrs, CAPI2_SatkApi_SendTerminalRsp_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SatkApi_SendTerminalRsp_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SatkApi_SetTermProfile_Req_t(void* xdrs, CAPI2_SatkApi_SetTermProfile_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SatkApi_SetTermProfile_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->data,sizeof( UInt8 ), xdr_UInt8) &&
		_xdr_UInt8(xdrs, &rsp->length,"length") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SatkApi_SendEnvelopeCmdReq_Req_t(void* xdrs, CAPI2_SatkApi_SendEnvelopeCmdReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SatkApi_SendEnvelopeCmdReq_Req_t")

	if(
		xdr_UInt8(xdrs, &rsp->data_len) &&
		xdr_copy_len(&len, (u_int)(rsp->data_len)) && xdr_bytes(xdrs, (char **)(void*)&rsp->data, &len, 0xFFFF) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SatkApi_SendEnvelopeCmdReq_Rsp_t(void* xdrs, CAPI2_SatkApi_SendEnvelopeCmdReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SatkApi_SendEnvelopeCmdReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( EnvelopeRspData_t ), xdr_EnvelopeRspData_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SatkApi_SendTerminalRspReq_Req_t(void* xdrs, CAPI2_SatkApi_SendTerminalRspReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SatkApi_SendTerminalRspReq_Req_t")

	if(
		xdr_UInt8(xdrs, &rsp->data_len) &&
		xdr_copy_len(&len, (u_int)(rsp->data_len)) && xdr_bytes(xdrs, (char **)(void*)&rsp->data, &len, 0xFFFF) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SatkApi_SendTerminalRspReq_Rsp_t(void* xdrs, CAPI2_SatkApi_SendTerminalRspReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SatkApi_SendTerminalRspReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( TerminalResponseRspData_t ), xdr_TerminalResponseRspData_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_StkApi_SendBrowsingStatusEvent_Req_t(void* xdrs, CAPI2_StkApi_SendBrowsingStatusEvent_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_StkApi_SendBrowsingStatusEvent_Req_t")

	if(
		xdr_UInt8(xdrs, &rsp->data_len) &&
		xdr_copy_len(&len, (u_int)(rsp->data_len)) && xdr_bytes(xdrs, (char **)(void*)&rsp->status, &len, 0xFFFF) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SatkApi_SendCcSetupReq_Req_t(void* xdrs, CAPI2_SatkApi_SendCcSetupReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SatkApi_SendCcSetupReq_Req_t")

	if(
		xdr_TypeOfNumber_t(xdrs, &rsp->ton) &&
		xdr_NumberPlanId_t(xdrs, &rsp->npi) &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->number,sizeof( char ), xdr_char) &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->bc1,sizeof( BearerCapability_t ), xdr_BearerCapability_t) &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->subaddr_data,sizeof( Subaddress_t ), xdr_Subaddress_t) &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->bc2,sizeof( BearerCapability_t ), xdr_BearerCapability_t) &&
		_xdr_UInt8(xdrs, &rsp->bc_repeat_ind,"bc_repeat_ind") &&
		_xdr_Boolean(xdrs, &rsp->simtk_orig,"simtk_orig") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SatkApi_Control_Setup_Rsp_Rsp_t(void* xdrs, CAPI2_SatkApi_Control_Setup_Rsp_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SatkApi_Control_Setup_Rsp_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( StkCallControlSetupRsp_t ), xdr_StkCallControlSetupRsp_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SatkApi_Control_SS_Rsp_Rsp_t(void* xdrs, CAPI2_SatkApi_Control_SS_Rsp_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SatkApi_Control_SS_Rsp_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( StkCallControlSsRsp_t ), xdr_StkCallControlSsRsp_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SatkApi_Control_USSD_Rsp_Rsp_t(void* xdrs, CAPI2_SatkApi_Control_USSD_Rsp_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SatkApi_Control_USSD_Rsp_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( StkCallControlUssdRsp_t ), xdr_StkCallControlUssdRsp_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SatkApi_SendCcSsReq_Req_t(void* xdrs, CAPI2_SatkApi_SendCcSsReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SatkApi_SendCcSsReq_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->ton_npi,"ton_npi") &&
		_xdr_UInt8(xdrs, &rsp->ss_len,"ss_len") &&
		xdr_copy_len(&len, (u_int)(rsp->ss_len)) && xdr_bytes(xdrs, (char **)(void*)&rsp->ss_data, &len, 0xFFFF) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SatkApi_SendCcUssdReq_Req_t(void* xdrs, CAPI2_SatkApi_SendCcUssdReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SatkApi_SendCcUssdReq_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->ussd_dcs,"ussd_dcs") &&
		_xdr_UInt8(xdrs, &rsp->ussd_len,"ussd_len") &&
		xdr_copy_len(&len, (u_int)(rsp->ussd_len)) && xdr_bytes(xdrs, (char **)(void*)&rsp->ussd_data, &len, 0xFFFF) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SatkApi_SendCcSmsReq_Req_t(void* xdrs, CAPI2_SatkApi_SendCcSmsReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SatkApi_SendCcSmsReq_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->sca_toa,"sca_toa") &&
		_xdr_UInt8(xdrs, &rsp->sca_number_len,"sca_number_len") &&
		xdr_copy_len(&len, (u_int)(rsp->sca_number_len)) && xdr_bytes(xdrs, (char **)(void*)&rsp->sca_number, &len, 0xFFFF) &&
		_xdr_UInt8(xdrs, &rsp->dest_toa,"dest_toa") &&
		_xdr_UInt8(xdrs, &rsp->dest_number_len,"dest_number_len") &&
		xdr_copy_len(&len, (u_int)(rsp->dest_number_len)) && xdr_bytes(xdrs, (char **)(void*)&rsp->dest_number, &len, 0xFFFF) &&
		_xdr_Boolean(xdrs, &rsp->simtk_orig,"simtk_orig") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SatkApi_Control_SMS_Rsp_Rsp_t(void* xdrs, CAPI2_SatkApi_Control_SMS_Rsp_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SatkApi_Control_SMS_Rsp_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( StkCallControlSmsRsp_t ), xdr_StkCallControlSmsRsp_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LCS_CpMoLrReq_Req_t(void* xdrs, CAPI2_LCS_CpMoLrReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_CpMoLrReq_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->inCpMoLrReq,sizeof( LcsCpMoLrReq_t ), xdr_LcsCpMoLrReq_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LCS_CpMoLrReq_Rsp_t(void* xdrs, CAPI2_LCS_CpMoLrReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_CpMoLrReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( LCS_SrvRsp_t ), xdr_LCS_SrvRsp_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LCS_CpMtLrVerificationRsp_Req_t(void* xdrs, CAPI2_LCS_CpMtLrVerificationRsp_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_CpMtLrVerificationRsp_Req_t")

	if(
		xdr_LCS_VerifRsp_t(xdrs, &rsp->inVerificationRsp) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LCS_CpMtLrRsp_Req_t(void* xdrs, CAPI2_LCS_CpMtLrRsp_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_CpMtLrRsp_Req_t")

	if(
		xdr_SS_Operation_t(xdrs, &rsp->inOperation) &&
		_xdr_Boolean(xdrs, &rsp->inIsAccepted,"inIsAccepted") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LCS_CpLocUpdateRsp_Req_t(void* xdrs, CAPI2_LCS_CpLocUpdateRsp_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_CpLocUpdateRsp_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->inTerminationCause,sizeof( LCS_TermCause_t ), xdr_LCS_TermCause_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LCS_DecodePosEstimate_Req_t(void* xdrs, CAPI2_LCS_DecodePosEstimate_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_DecodePosEstimate_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->inLocEstData,sizeof( LCS_LocEstimate_t ), xdr_LCS_LocEstimate_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LCS_DecodePosEstimate_Rsp_t(void* xdrs, CAPI2_LCS_DecodePosEstimate_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_DecodePosEstimate_Rsp_t")

	if(
		xdr_LcsPosEstimateInfo_t(xdrs, &rsp->outPosEst) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LCS_EncodeAssistanceReq_Req_t(void* xdrs, CAPI2_LCS_EncodeAssistanceReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_EncodeAssistanceReq_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->inAssistReq,sizeof( LcsAssistanceReq_t ), xdr_LcsAssistanceReq_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LCS_EncodeAssistanceReq_Rsp_t(void* xdrs, CAPI2_LCS_EncodeAssistanceReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_EncodeAssistanceReq_Rsp_t")

	if(
		xdr_LCS_GanssAssistData_t(xdrs, &rsp->outAssistData) &&
		 xdr_int(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_MakeVoiceCall_Req_t(void* xdrs, CAPI2_CcApi_MakeVoiceCall_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_MakeVoiceCall_Req_t")

	if(
		xdr_uchar_ptr_t(xdrs, &rsp->callNum) &&
		xdr_VoiceCallParam_t(xdrs, &rsp->voiceCallParam) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_MakeDataCall_Req_t(void* xdrs, CAPI2_CcApi_MakeDataCall_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_MakeDataCall_Req_t")

	if(
		xdr_uchar_ptr_t(xdrs, &rsp->callNum) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_MakeFaxCall_Req_t(void* xdrs, CAPI2_CcApi_MakeFaxCall_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_MakeFaxCall_Req_t")

	if(
		xdr_uchar_ptr_t(xdrs, &rsp->callNum) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_MakeVideoCall_Req_t(void* xdrs, CAPI2_CcApi_MakeVideoCall_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_MakeVideoCall_Req_t")

	if(
		xdr_uchar_ptr_t(xdrs, &rsp->callNum) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_EndCall_Req_t(void* xdrs, CAPI2_CcApi_EndCall_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_EndCall_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->callIdx,"callIdx") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_EndCallImmediate_Req_t(void* xdrs, CAPI2_CcApi_EndCallImmediate_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_EndCallImmediate_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->callIdx,"callIdx") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_AcceptVoiceCall_Req_t(void* xdrs, CAPI2_CcApi_AcceptVoiceCall_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_AcceptVoiceCall_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->callIndex,"callIndex") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_AcceptDataCall_Req_t(void* xdrs, CAPI2_CcApi_AcceptDataCall_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_AcceptDataCall_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->callIndex,"callIndex") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_AcceptVideoCall_Req_t(void* xdrs, CAPI2_CcApi_AcceptVideoCall_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_AcceptVideoCall_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->callIndex,"callIndex") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_HoldCall_Req_t(void* xdrs, CAPI2_CcApi_HoldCall_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_HoldCall_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->callIndex,"callIndex") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_RetrieveCall_Req_t(void* xdrs, CAPI2_CcApi_RetrieveCall_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_RetrieveCall_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->callIndex,"callIndex") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_SwapCall_Req_t(void* xdrs, CAPI2_CcApi_SwapCall_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_SwapCall_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->callIndex,"callIndex") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_SplitCall_Req_t(void* xdrs, CAPI2_CcApi_SplitCall_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_SplitCall_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->callIndex,"callIndex") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_JoinCall_Req_t(void* xdrs, CAPI2_CcApi_JoinCall_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_JoinCall_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->callIndex,"callIndex") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_TransferCall_Req_t(void* xdrs, CAPI2_CcApi_TransferCall_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_TransferCall_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->callIndex,"callIndex") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_smsModuleReady_Rsp_t(void* xdrs, CAPI2_smsModuleReady_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_smsModuleReady_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( smsModuleReady_t ), xdr_smsModuleReady_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsSimMsgDel_Rsp_t(void* xdrs, CAPI2_SmsSimMsgDel_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsSimMsgDel_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SmsSimMsg_t ), xdr_SmsSimMsg_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsSimMsgOta_Rsp_t(void* xdrs, CAPI2_SmsSimMsgOta_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsSimMsgOta_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SmsSimMsg_t ), xdr_SmsSimMsg_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsSimMsgPush_Rsp_t(void* xdrs, CAPI2_SmsSimMsgPush_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsSimMsgPush_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SmsSimMsg_t ), xdr_SmsSimMsg_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsStoredSmsStatus_Rsp_t(void* xdrs, CAPI2_SmsStoredSmsStatus_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsStoredSmsStatus_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SmsIncMsgStoredResult_t ), xdr_SmsIncMsgStoredResult_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsStoredSmsCb_Rsp_t(void* xdrs, CAPI2_SmsStoredSmsCb_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsStoredSmsCb_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SmsIncMsgStoredResult_t ), xdr_SmsIncMsgStoredResult_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsStoredSms_Rsp_t(void* xdrs, CAPI2_SmsStoredSms_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsStoredSms_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SmsIncMsgStoredResult_t ), xdr_SmsIncMsgStoredResult_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsCbData_Rsp_t(void* xdrs, CAPI2_SmsCbData_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsCbData_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SmsStoredSmsCb_t ), xdr_SmsStoredSmsCb_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_HomezoneIndData_Rsp_t(void* xdrs, CAPI2_HomezoneIndData_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_HomezoneIndData_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( HomezoneIndData_t ), xdr_HomezoneIndData_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SATK_EventDataDisp_Rsp_t(void* xdrs, CAPI2_SATK_EventDataDisp_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SATK_EventDataDisp_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SATK_EventData_t ), xdr_SATK_EventData_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SATK_EventDataGetInKey_Rsp_t(void* xdrs, CAPI2_SATK_EventDataGetInKey_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SATK_EventDataGetInKey_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SATK_EventData_t ), xdr_SATK_EventData_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SATK_EventDataGetInput_Rsp_t(void* xdrs, CAPI2_SATK_EventDataGetInput_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SATK_EventDataGetInput_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SATK_EventData_t ), xdr_SATK_EventData_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SATK_EventDataPlayTone_Rsp_t(void* xdrs, CAPI2_SATK_EventDataPlayTone_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SATK_EventDataPlayTone_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SATK_EventData_t ), xdr_SATK_EventData_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SATK_EventDataActivate_Rsp_t(void* xdrs, CAPI2_SATK_EventDataActivate_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SATK_EventDataActivate_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SATK_EventData_t ), xdr_SATK_EventData_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SATK_EventDataSelItem_Rsp_t(void* xdrs, CAPI2_SATK_EventDataSelItem_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SATK_EventDataSelItem_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SATK_EventData_t ), xdr_SATK_EventData_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SATK_EventDataSendSS_Rsp_t(void* xdrs, CAPI2_SATK_EventDataSendSS_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SATK_EventDataSendSS_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SATK_EventData_t ), xdr_SATK_EventData_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SATK_EventDataSendUSSD_Rsp_t(void* xdrs, CAPI2_SATK_EventDataSendUSSD_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SATK_EventDataSendUSSD_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SATK_EventData_t ), xdr_SATK_EventData_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SATK_EventDataSetupCall_Rsp_t(void* xdrs, CAPI2_SATK_EventDataSetupCall_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SATK_EventDataSetupCall_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SATK_EventData_t ), xdr_SATK_EventData_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SATK_EventDataSetupMenu_Rsp_t(void* xdrs, CAPI2_SATK_EventDataSetupMenu_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SATK_EventDataSetupMenu_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SATK_EventData_t ), xdr_SATK_EventData_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SATK_EventDataRefresh_Rsp_t(void* xdrs, CAPI2_SATK_EventDataRefresh_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SATK_EventDataRefresh_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SATK_EventData_t ), xdr_SATK_EventData_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIM_FatalInd_Rsp_t(void* xdrs, CAPI2_SIM_FatalInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_FatalInd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_FATAL_ERROR_t ), xdr_SIM_FATAL_ERROR_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SATK_CallSetupFail_Rsp_t(void* xdrs, CAPI2_SATK_CallSetupFail_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SATK_CallSetupFail_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( StkCallSetupFail_t ), xdr_StkCallSetupFail_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SATK_CallControlDisplay_Rsp_t(void* xdrs, CAPI2_SATK_CallControlDisplay_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SATK_CallControlDisplay_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( StkCallControlDisplay_t ), xdr_StkCallControlDisplay_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SATK_EventDataShortMsg_Rsp_t(void* xdrs, CAPI2_SATK_EventDataShortMsg_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SATK_EventDataShortMsg_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SATK_EventData_t ), xdr_SATK_EventData_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SATK_EventDataSendDtmf_Rsp_t(void* xdrs, CAPI2_SATK_EventDataSendDtmf_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SATK_EventDataSendDtmf_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SATK_EventData_t ), xdr_SATK_EventData_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SATK_EventData_Rsp_t(void* xdrs, CAPI2_SATK_EventData_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SATK_EventData_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SATK_EventData_t ), xdr_SATK_EventData_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SATK_EventDataLaunchBrowser_Rsp_t(void* xdrs, CAPI2_SATK_EventDataLaunchBrowser_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SATK_EventDataLaunchBrowser_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SATK_EventData_t ), xdr_SATK_EventData_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SATK_EventDataServiceReq_Rsp_t(void* xdrs, CAPI2_SATK_EventDataServiceReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SATK_EventDataServiceReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SATK_EventData_t ), xdr_SATK_EventData_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_StkLangNotificationLangNotify_Rsp_t(void* xdrs, CAPI2_StkLangNotificationLangNotify_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_StkLangNotificationLangNotify_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( StkLangNotification_t ), xdr_StkLangNotification_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimMmiSetupInd_Rsp_t(void* xdrs, CAPI2_SimMmiSetupInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimMmiSetupInd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( UInt16 ), xdr_UInt16) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SATK_EventDataRunATInd_Rsp_t(void* xdrs, CAPI2_SATK_EventDataRunATInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SATK_EventDataRunATInd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SATK_EventData_t ), xdr_SATK_EventData_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_StkRunAtReq_Rsp_t(void* xdrs, CAPI2_StkRunAtReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_StkRunAtReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( RunAT_Request ), xdr_RunAT_Request) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_StkRunIpAtReq_Rsp_t(void* xdrs, CAPI2_StkRunIpAtReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_StkRunIpAtReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( RunAT_Request ), xdr_RunAT_Request) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_StkMenuSelectionRes_Rsp_t(void* xdrs, CAPI2_StkMenuSelectionRes_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_StkMenuSelectionRes_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( STKMenuSelectionResCode_t ), xdr_STKMenuSelectionResCode_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_VOICECALL_CONNECTED_IND_Rsp_t(void* xdrs, CAPI2_VOICECALL_CONNECTED_IND_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_VOICECALL_CONNECTED_IND_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( VoiceCallConnectMsg_t ), xdr_VoiceCallConnectMsg_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_VOICECALL_PRECONNECTED_IND_Rsp_t(void* xdrs, CAPI2_VOICECALL_PRECONNECTED_IND_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_VOICECALL_PRECONNECTED_IND_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( VoiceCallPreConnectMsg_t ), xdr_VoiceCallPreConnectMsg_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SS_CALL_NOTIFICATION_Rsp_t(void* xdrs, CAPI2_SS_CALL_NOTIFICATION_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_CALL_NOTIFICATION_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SS_CallNotification_t ), xdr_SS_CallNotification_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MNCC_CLIENT_NOTIFY_SS_IND_Rsp_t(void* xdrs, CAPI2_MNCC_CLIENT_NOTIFY_SS_IND_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MNCC_CLIENT_NOTIFY_SS_IND_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( CC_NotifySsInd_t ), xdr_CC_NotifySsInd_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SS_NOTIFY_CLOSED_USER_GROUP_Rsp_t(void* xdrs, CAPI2_SS_NOTIFY_CLOSED_USER_GROUP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_NOTIFY_CLOSED_USER_GROUP_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SS_CallNotification_t ), xdr_SS_CallNotification_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SS_NOTIFY_EXTENDED_CALL_TRANSFER_Rsp_t(void* xdrs, CAPI2_SS_NOTIFY_EXTENDED_CALL_TRANSFER_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_NOTIFY_EXTENDED_CALL_TRANSFER_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SS_CallNotification_t ), xdr_SS_CallNotification_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SS_NOTIFY_CALLING_NAME_PRESENT_Rsp_t(void* xdrs, CAPI2_SS_NOTIFY_CALLING_NAME_PRESENT_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_NOTIFY_CALLING_NAME_PRESENT_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SS_CallNotification_t ), xdr_SS_CallNotification_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CALL_STATUS_IND_Rsp_t(void* xdrs, CAPI2_CALL_STATUS_IND_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CALL_STATUS_IND_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( CallStatusMsg_t ), xdr_CallStatusMsg_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_VOICECALL_ACTION_RSP_Rsp_t(void* xdrs, CAPI2_VOICECALL_ACTION_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_VOICECALL_ACTION_RSP_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( VoiceCallActionMsg_t ), xdr_VoiceCallActionMsg_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_VOICECALL_RELEASE_IND_Rsp_t(void* xdrs, CAPI2_VOICECALL_RELEASE_IND_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_VOICECALL_RELEASE_IND_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( VoiceCallReleaseMsg_t ), xdr_VoiceCallReleaseMsg_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_VOICECALL_RELEASE_CNF_Rsp_t(void* xdrs, CAPI2_VOICECALL_RELEASE_CNF_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_VOICECALL_RELEASE_CNF_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( VoiceCallReleaseMsg_t ), xdr_VoiceCallReleaseMsg_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_INCOMING_CALL_IND_Rsp_t(void* xdrs, CAPI2_INCOMING_CALL_IND_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_INCOMING_CALL_IND_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( CallReceiveMsg_t ), xdr_CallReceiveMsg_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_VOICECALL_WAITING_IND_Rsp_t(void* xdrs, CAPI2_VOICECALL_WAITING_IND_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_VOICECALL_WAITING_IND_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( VoiceCallWaitingMsg_t ), xdr_VoiceCallWaitingMsg_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CALL_AOCSTATUS_IND_Rsp_t(void* xdrs, CAPI2_CALL_AOCSTATUS_IND_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CALL_AOCSTATUS_IND_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( CallAOCStatusMsg_t ), xdr_CallAOCStatusMsg_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CALL_CCM_IND_Rsp_t(void* xdrs, CAPI2_CALL_CCM_IND_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CALL_CCM_IND_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( CallCCMMsg_t ), xdr_CallCCMMsg_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CALL_CONNECTEDLINEID_IND_Rsp_t(void* xdrs, CAPI2_CALL_CONNECTEDLINEID_IND_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CALL_CONNECTEDLINEID_IND_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( UInt8 ), xdr_UInt8) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATACALL_STATUS_IND_Rsp_t(void* xdrs, CAPI2_DATACALL_STATUS_IND_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATACALL_STATUS_IND_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( DataCallStatusMsg_t ), xdr_DataCallStatusMsg_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATACALL_RELEASE_IND_Rsp_t(void* xdrs, CAPI2_DATACALL_RELEASE_IND_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATACALL_RELEASE_IND_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( DataCallReleaseMsg_t ), xdr_DataCallReleaseMsg_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATACALL_ECDC_IND_Rsp_t(void* xdrs, CAPI2_DATACALL_ECDC_IND_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATACALL_ECDC_IND_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( DataECDCLinkMsg_t ), xdr_DataECDCLinkMsg_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATACALL_CONNECTED_IND_Rsp_t(void* xdrs, CAPI2_DATACALL_CONNECTED_IND_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATACALL_CONNECTED_IND_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( DataCallConnectMsg_t ), xdr_DataCallConnectMsg_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_API_CLIENT_CMD_IND_Rsp_t(void* xdrs, CAPI2_API_CLIENT_CMD_IND_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_API_CLIENT_CMD_IND_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( ApiClientCmdInd_t ), xdr_ApiClientCmdInd_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DTMF_STATUS_IND_Rsp_t(void* xdrs, CAPI2_DTMF_STATUS_IND_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DTMF_STATUS_IND_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( ApiDtmfStatus_t ), xdr_ApiDtmfStatus_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_USER_INFORMATION_Rsp_t(void* xdrs, CAPI2_USER_INFORMATION_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USER_INFORMATION_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SS_UserInfo_t ), xdr_SS_UserInfo_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcCipherInd_Rsp_t(void* xdrs, CAPI2_CcCipherInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcCipherInd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( CcCipherInd_t ), xdr_CcCipherInd_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_GPRS_ACTIVATE_IND_Rsp_t(void* xdrs, CAPI2_GPRS_ACTIVATE_IND_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_GPRS_ACTIVATE_IND_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( GPRSActInd_t ), xdr_GPRSActInd_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_GPRS_DEACTIVATE_IND_Rsp_t(void* xdrs, CAPI2_GPRS_DEACTIVATE_IND_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_GPRS_DEACTIVATE_IND_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( GPRSDeactInd_t ), xdr_GPRSDeactInd_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PDP_DEACTIVATION_IND_Rsp_t(void* xdrs, CAPI2_PDP_DEACTIVATION_IND_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PDP_DEACTIVATION_IND_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( GPRSDeactInd_t ), xdr_GPRSDeactInd_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_GPRS_MODIFY_IND_Rsp_t(void* xdrs, CAPI2_GPRS_MODIFY_IND_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_GPRS_MODIFY_IND_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( Inter_ModifyContextInd_t ), xdr_Inter_ModifyContextInd_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_GPRS_REACT_IND_Rsp_t(void* xdrs, CAPI2_GPRS_REACT_IND_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_GPRS_REACT_IND_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( GPRSReActInd_t ), xdr_GPRSReActInd_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PDP_ACTIVATION_NWI_IND_Rsp_t(void* xdrs, CAPI2_PDP_ACTIVATION_NWI_IND_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PDP_ACTIVATION_NWI_IND_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( PDP_ActivateNWI_Ind_t ), xdr_PDP_ActivateNWI_Ind_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_DATA_SUSPEND_IND_Rsp_t(void* xdrs, CAPI2_DATA_SUSPEND_IND_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_DATA_SUSPEND_IND_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( GPRSSuspendInd_t ), xdr_GPRSSuspendInd_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MNSS_CLIENT_LCS_SRV_IND_Rsp_t(void* xdrs, CAPI2_MNSS_CLIENT_LCS_SRV_IND_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MNSS_CLIENT_LCS_SRV_IND_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( LCS_SrvInd_t ), xdr_LCS_SrvInd_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MNSS_CLIENT_LCS_SRV_REL_Rsp_t(void* xdrs, CAPI2_MNSS_CLIENT_LCS_SRV_REL_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MNSS_CLIENT_LCS_SRV_REL_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SS_SrvRel_t ), xdr_SS_SrvRel_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MNSS_CLIENT_SS_SRV_RSP_Rsp_t(void* xdrs, CAPI2_MNSS_CLIENT_SS_SRV_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MNSS_CLIENT_SS_SRV_RSP_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SS_SrvRsp_t ), xdr_SS_SrvRsp_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MNSS_CLIENT_SS_SRV_REL_Rsp_t(void* xdrs, CAPI2_MNSS_CLIENT_SS_SRV_REL_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MNSS_CLIENT_SS_SRV_REL_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SS_SrvRel_t ), xdr_SS_SrvRel_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MNSS_CLIENT_SS_SRV_IND_Rsp_t(void* xdrs, CAPI2_MNSS_CLIENT_SS_SRV_IND_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MNSS_CLIENT_SS_SRV_IND_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SS_SrvInd_t ), xdr_SS_SrvInd_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MNSS_SATK_SS_SRV_RSP_Rsp_t(void* xdrs, CAPI2_MNSS_SATK_SS_SRV_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MNSS_SATK_SS_SRV_RSP_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( STK_SsSrvRel_t ), xdr_STK_SsSrvRel_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsVoicemailInd_Rsp_t(void* xdrs, CAPI2_SmsVoicemailInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsVoicemailInd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SmsVoicemailInd_t ), xdr_SmsVoicemailInd_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIM_SMS_DATA_RSP_Rsp_t(void* xdrs, CAPI2_SIM_SMS_DATA_RSP_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SMS_DATA_RSP_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SmsSimMsg_t ), xdr_SmsSimMsg_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetCNAPName_Req_t(void* xdrs, CAPI2_CcApi_GetCNAPName_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetCNAPName_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->callIndex,"callIndex") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetCNAPName_Rsp_t(void* xdrs, CAPI2_CcApi_GetCNAPName_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_GetCNAPName_Rsp_t")

	if(
		 xdr_CNAP_NAME_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYSPARM_GetHSUPASupported_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetHSUPASupported_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_GetHSUPASupported_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYSPARM_GetHSDPASupported_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetHSDPASupported_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_GetHSDPASupported_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_MSRadioActivityInd_Rsp_t(void* xdrs, CAPI2_MSRadioActivityInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_MSRadioActivityInd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( MSRadioActivityInd_t ), xdr_MSRadioActivityInd_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_IsCurrentStateMpty_Req_t(void* xdrs, CAPI2_CcApi_IsCurrentStateMpty_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_IsCurrentStateMpty_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->callIndex,"callIndex") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_IsCurrentStateMpty_Rsp_t(void* xdrs, CAPI2_CcApi_IsCurrentStateMpty_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_IsCurrentStateMpty_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetPCHContextState_Req_t(void* xdrs, CAPI2_PdpApi_GetPCHContextState_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetPCHContextState_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetPCHContextState_Rsp_t(void* xdrs, CAPI2_PdpApi_GetPCHContextState_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetPCHContextState_Rsp_t")

	if(
		 xdr_PCHContextState_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetPDPContextEx_Req_t(void* xdrs, CAPI2_PdpApi_GetPDPContextEx_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetPDPContextEx_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetPDPContextEx_Rsp_t(void* xdrs, CAPI2_PdpApi_GetPDPContextEx_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetPDPContextEx_Rsp_t")

	if(
		xdr_PDPDefaultContext_t(xdrs, &rsp->pDefaultContext) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_GetCurrLockedSimlockType_Rsp_t(void* xdrs, CAPI2_SimApi_GetCurrLockedSimlockType_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_GetCurrLockedSimlockType_Rsp_t")

	if(
		 xdr_SIM_PIN_Status_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PLMN_SELECT_CNF_Rsp_t(void* xdrs, CAPI2_PLMN_SELECT_CNF_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PLMN_SELECT_CNF_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( UInt16 ), xdr_UInt16) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SubmitSelectFileSendApduReq_Req_t(void* xdrs, CAPI2_SimApi_SubmitSelectFileSendApduReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SimApi_SubmitSelectFileSendApduReq_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->socketId,"socketId") &&
		xdr_APDUFileID_t(xdrs, &rsp->dfileId) &&
		xdr_APDUFileID_t(xdrs, &rsp->efileId) &&
		_xdr_UInt8(xdrs, &rsp->pathLen,"pathLen") &&
		xdr_copy_len(&len, (u_int)(rsp->pathLen)) && xdr_array(xdrs, (char **)(void*)&rsp->pPath, &len, ~0, sizeof( UInt16 ),(xdrproc_t) xdr_UInt16) &&
		_xdr_UInt16(xdrs, &rsp->apduLen,"apduLen") &&
		xdr_copy_len(&len, (u_int)(rsp->apduLen)) && xdr_bytes(xdrs, (char **)(void*)&rsp->pApdu, &len, 0xFFFF) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SubmitSelectFileSendApduReq_Rsp_t(void* xdrs, CAPI2_SimApi_SubmitSelectFileSendApduReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SubmitSelectFileSendApduReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SimApduRsp_t ), xdr_SimApduRsp_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SatkApi_SendTermProfileInd_Rsp_t(void* xdrs, CAPI2_SatkApi_SendTermProfileInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SatkApi_SendTermProfileInd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( StkTermProfileInd_t ), xdr_StkTermProfileInd_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SubmitMulRecordEFileReq_Req_t(void* xdrs, CAPI2_SimApi_SubmitMulRecordEFileReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SimApi_SubmitMulRecordEFileReq_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->socketId,"socketId") &&
		xdr_APDUFileID_t(xdrs, &rsp->efileId) &&
		xdr_APDUFileID_t(xdrs, &rsp->dfileId) &&
		_xdr_UInt8(xdrs, &rsp->firstRecNum,"firstRecNum") &&
		_xdr_UInt8(xdrs, &rsp->numOfRec,"numOfRec") &&
		_xdr_UInt8(xdrs, &rsp->recLen,"recLen") &&
		_xdr_UInt8(xdrs, &rsp->pathLen,"pathLen") &&
		xdr_copy_len(&len, (u_int)(rsp->pathLen)) && xdr_array(xdrs, (char **)(void*)&rsp->pSelectPath, &len, ~0, sizeof( UInt16 ),(xdrproc_t) xdr_UInt16) &&
		_xdr_Boolean(xdrs, &rsp->optimizationFlag1,"optimizationFlag1") &&
		_xdr_Boolean(xdrs, &rsp->optimizationFlag2,"optimizationFlag2") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SubmitMulRecordEFileReq_Rsp_t(void* xdrs, CAPI2_SimApi_SubmitMulRecordEFileReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SubmitMulRecordEFileReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_MUL_REC_DATA_t ), xdr_SIM_MUL_REC_DATA_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SatkApi_SendUiccCatInd_Rsp_t(void* xdrs, CAPI2_SatkApi_SendUiccCatInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SatkApi_SendUiccCatInd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( StkUiccCatInd_t ), xdr_StkUiccCatInd_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendSelectApplicationReq_Req_t(void* xdrs, CAPI2_SimApi_SendSelectApplicationReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SimApi_SendSelectApplicationReq_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->socketId,"socketId") &&
		xdr_UInt8(xdrs, &rsp->aid_len) &&
		xdr_copy_len(&len, (u_int)(rsp->aid_len)) && xdr_bytes(xdrs, (char **)(void*)&rsp->aid_data, &len, 0xFFFF) &&
		xdr_SIM_APP_OCCURRENCE_t(xdrs, &rsp->app_occur) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_SendSelectApplicationReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendSelectApplicationReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_SendSelectApplicationReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SIM_SELECT_APPLICATION_RES_t ), xdr_SIM_SELECT_APPLICATION_RES_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_PerformSteeringOfRoaming_Req_t(void* xdrs, CAPI2_SimApi_PerformSteeringOfRoaming_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_PerformSteeringOfRoaming_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->inPlmnListPtr,sizeof( SIM_MUL_PLMN_ENTRY_t ), xdr_SIM_MUL_PLMN_ENTRY_t) &&
		_xdr_UInt8(xdrs, &rsp->inListLen,"inListLen") &&
		_xdr_UInt8(xdrs, &rsp->inRefreshType,"inRefreshType") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SatkApi_SendProactiveCmdInd_Rsp_t(void* xdrs, CAPI2_SatkApi_SendProactiveCmdInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SatkApi_SendProactiveCmdInd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( ProactiveCmdData_t ), xdr_ProactiveCmdData_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SatkApi_SendProactiveCmdFetchingOnOffReq_Req_t(void* xdrs, CAPI2_SatkApi_SendProactiveCmdFetchingOnOffReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SatkApi_SendProactiveCmdFetchingOnOffReq_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->enableCmdFetching,"enableCmdFetching") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SatkApi_SendProactiveCmdFetchingOnOffReq_Rsp_t(void* xdrs, CAPI2_SatkApi_SendProactiveCmdFetchingOnOffReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SatkApi_SendProactiveCmdFetchingOnOffReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( StkProactiveCmdFetchingOnOffRsp_t ), xdr_StkProactiveCmdFetchingOnOffRsp_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SatkApi_SendExtProactiveCmdReq_Req_t(void* xdrs, CAPI2_SatkApi_SendExtProactiveCmdReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SatkApi_SendExtProactiveCmdReq_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->dataLen,"dataLen") &&
		xdr_copy_len(&len, (u_int)(rsp->dataLen)) && xdr_bytes(xdrs, (char **)(void*)&rsp->ptrData, &len, 0xFFFF) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SatkApi_SendTerminalProfileReq_Req_t(void* xdrs, CAPI2_SatkApi_SendTerminalProfileReq_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_SatkApi_SendTerminalProfileReq_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->dataLen,"dataLen") &&
		xdr_copy_len(&len, (u_int)(rsp->dataLen)) && xdr_bytes(xdrs, (char **)(void*)&rsp->ptrData, &len, 0xFFFF) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SatkApi_SendPollingIntervalReq_Req_t(void* xdrs, CAPI2_SatkApi_SendPollingIntervalReq_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SatkApi_SendPollingIntervalReq_Req_t")

	if(
		_xdr_UInt16(xdrs, &rsp->pollingInterval,"pollingInterval") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SatkApi_SendPollingIntervalReq_Rsp_t(void* xdrs, CAPI2_SatkApi_SendPollingIntervalReq_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SatkApi_SendPollingIntervalReq_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( StkPollingIntervalRsp_t ), xdr_StkPollingIntervalRsp_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SatkApi_SendExtProactiveCmdInd_Rsp_t(void* xdrs, CAPI2_SatkApi_SendExtProactiveCmdInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SatkApi_SendExtProactiveCmdInd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( StkExtProactiveCmdInd_t ), xdr_StkExtProactiveCmdInd_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_SetPDPActivationCallControlFlag_Req_t(void* xdrs, CAPI2_PdpApi_SetPDPActivationCallControlFlag_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_SetPDPActivationCallControlFlag_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->flag,"flag") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetPDPActivationCallControlFlag_Rsp_t(void* xdrs, CAPI2_PdpApi_GetPDPActivationCallControlFlag_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetPDPActivationCallControlFlag_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_SendPDPActivateReq_PDU_Req_t(void* xdrs, CAPI2_PdpApi_SendPDPActivateReq_PDU_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_SendPDPActivateReq_PDU_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
		xdr_PCHPDPActivateCallControlResult_t(xdrs, &rsp->callControlResult) &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->pdu,sizeof( PCHPDPActivatePDU_t ), xdr_PCHPDPActivatePDU_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_SendPDPActivateReq_PDU_Rsp_t(void* xdrs, CAPI2_PdpApi_SendPDPActivateReq_PDU_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_SendPDPActivateReq_PDU_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( PDP_SendPDPActivatePDUReq_Rsp_t ), xdr_PDP_SendPDPActivatePDUReq_Rsp_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_RejectNWIPDPActivation_Req_t(void* xdrs, CAPI2_PdpApi_RejectNWIPDPActivation_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_RejectNWIPDPActivation_Req_t")

	if(
		xdr_PCHPDPAddress_t(xdrs, &rsp->inPdpAddress) &&
		xdr_PCHRejectCause_t(xdrs, &rsp->inCause) &&
		xdr_PCHAPN_t(xdrs, &rsp->inApn) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_SetPDPBearerCtrlMode_Req_t(void* xdrs, CAPI2_PdpApi_SetPDPBearerCtrlMode_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_SetPDPBearerCtrlMode_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->inMode,"inMode") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetPDPBearerCtrlMode_Rsp_t(void* xdrs, CAPI2_PdpApi_GetPDPBearerCtrlMode_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetPDPBearerCtrlMode_Rsp_t")

	if(
		 _xdr_UInt8(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_RejectSecNWIPDPActivation_Req_t(void* xdrs, CAPI2_PdpApi_RejectSecNWIPDPActivation_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_RejectSecNWIPDPActivation_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->pActivateSecInd,sizeof( PDP_ActivateSecNWI_Ind_t ), xdr_PDP_ActivateSecNWI_Ind_t) &&
		xdr_PCHRejectCause_t(xdrs, &rsp->inCause) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_SetPDPNWIControlFlag_Req_t(void* xdrs, CAPI2_PdpApi_SetPDPNWIControlFlag_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_SetPDPNWIControlFlag_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->inFlag,"inFlag") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetPDPNWIControlFlag_Rsp_t(void* xdrs, CAPI2_PdpApi_GetPDPNWIControlFlag_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetPDPNWIControlFlag_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PDP_ACTIVATION_SEC_NWI_IND_Rsp_t(void* xdrs, CAPI2_PDP_ACTIVATION_SEC_NWI_IND_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PDP_ACTIVATION_SEC_NWI_IND_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( PDP_ActivateSecNWI_Ind_t ), xdr_PDP_ActivateSecNWI_Ind_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_CheckUMTSTft_Req_t(void* xdrs, CAPI2_PdpApi_CheckUMTSTft_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_CheckUMTSTft_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->inTftPtr,sizeof( PCHTrafficFlowTemplate_t ), xdr_PCHTrafficFlowTemplate_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_IsAnyPDPContextActive_Rsp_t(void* xdrs, CAPI2_PdpApi_IsAnyPDPContextActive_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_IsAnyPDPContextActive_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_IsAnyPDPContextActivePending_Rsp_t(void* xdrs, CAPI2_PdpApi_IsAnyPDPContextActivePending_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_IsAnyPDPContextActivePending_Rsp_t")

	if(
		 _xdr_Boolean(xdrs, &rsp->val,"val") &&
	1)
		return TRUE;
	else
		return FALSE;
}

static const struct xdr_discrim CAPI2_CcApi_ElementPtr_union_dscrm[] = {
	{ (int)CC_ELEM_AUTO_REJECT_SWITCH,_T("CC_ELEM_AUTO_REJECT_SWITCH"),		(xdrproc_t)xdr_Boolean,sizeof( Boolean ), NULL_capi2_proc_t, (xdrproc_t)xdr_Boolean,0 },
	{ (int)CC_ELEM_TEXT_TELEPHONY_SWITCH,_T("CC_ELEM_TEXT_TELEPHONY_SWITCH"),		(xdrproc_t)xdr_Boolean,sizeof( Boolean ), NULL_capi2_proc_t, (xdrproc_t)xdr_Boolean,0 },
	{ (int)CC_ELEM_MT_CALL_HANDLING,_T("CC_ELEM_MT_CALL_HANDLING"),		(xdrproc_t)xdr_UInt8,sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_UInt8,0 },
	{ (int)CC_ELEM_END_CALL_CAUSE,_T("CC_ELEM_END_CALL_CAUSE"),		(xdrproc_t)xdr_Cause_t,sizeof( Cause_t ), NULL_capi2_proc_t, (xdrproc_t)xdr_Cause_t,0 },
	{ (int)CC_ELEM_SPEECH_CODEC,_T("CC_ELEM_SPEECH_CODEC"),		(xdrproc_t)xdr_UInt16,sizeof( UInt16 ), NULL_capi2_proc_t, (xdrproc_t)xdr_UInt16,0 },
	{ (int)CC_ELEM_CHANNEL_MODE,_T("CC_ELEM_CHANNEL_MODE"),		(xdrproc_t)xdr_UInt16,sizeof( UInt16 ), NULL_capi2_proc_t, (xdrproc_t)xdr_UInt16,0 },
	{ (int)CC_ELEM_UMTS_WB_AMR,_T("CC_ELEM_UMTS_WB_AMR"),		(xdrproc_t)xdr_Boolean,sizeof( Boolean ), NULL_capi2_proc_t, (xdrproc_t)xdr_Boolean,0 },
	{ __dontcare__, _T(""), NULL_xdrproc_t ,0,0,0,0} 
};


static const struct xdr_discrim CAPI2_CcApi_CompareObjPtr_union_dscrm[] = {
	{ (int)CC_ELEM_AUTO_REJECT_SWITCH,_T("CC_ELEM_AUTO_REJECT_SWITCH"),		(xdrproc_t)xdr_Boolean,sizeof( Boolean ), NULL_capi2_proc_t, (xdrproc_t)xdr_Boolean,0 },
	{ (int)CC_ELEM_END_CALL_CAUSE,_T("CC_ELEM_END_CALL_CAUSE"),		(xdrproc_t)xdr_UInt8,sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_UInt8,0 },
	{ __dontcare__, _T(""), NULL_xdrproc_t ,0,0,0,0} 
};


UInt32 CAPI2_CcApi_SetElement_inCompareObjPtr_union_length(int inElement)
{
	const struct xdr_discrim *choices = CAPI2_CcApi_SetElement_inCompareObjPtr_union_dscrm;
	for (; choices->proc != NULL_xdrproc_t; choices++)
	{
		if (choices->value == inElement) return choices->unsize;
	}
	return 0;
}


UInt32 CAPI2_CcApi_SetElement_outElementPtr_union_length(int inElement)
{
	const struct xdr_discrim *choices = CAPI2_CcApi_SetElement_outElementPtr_union_dscrm;
	for (; choices->proc != NULL_xdrproc_t; choices++)
	{
		if (choices->value == inElement) return choices->unsize;
	}
	return 0;
}


bool_t xdr_CAPI2_CcApi_SetElement_Req_t(void* xdrs, CAPI2_CcApi_SetElement_Req_t *rsp)
{
	struct xdr_discrim *entry = NULL;

	XDR_LOG(xdrs,"CAPI2_CcApi_SetElement_Req_t")

	if(
		xdr_CcApi_Element_t(xdrs, &rsp->inElement) &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->inCompareObjPtr,CAPI2_CcApi_SetElement_inCompareObjPtr_union_length(rsp->inElement), xdr_void) &&
		(rsp->inCompareObjPtr== NULL ||  xdr_union(xdrs, (enum_t *)&(rsp->inElement),(caddr_t)(void *)(rsp->inCompareObjPtr),CAPI2_CcApi_SetElement_inCompareObjPtr_union_dscrm,NULL_xdrproc_t, &entry, NULL) ) &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->outElementPtr,CAPI2_CcApi_SetElement_outElementPtr_union_length(rsp->inElement), xdr_void) &&
		(rsp->outElementPtr== NULL ||  xdr_union(xdrs, (enum_t *)&(rsp->inElement),(caddr_t)(void *)(rsp->outElementPtr),CAPI2_CcApi_SetElement_outElementPtr_union_dscrm,NULL_xdrproc_t, &entry, NULL) ) &&
	1)
		return TRUE;
	else
		return FALSE;
}

UInt32 CAPI2_CcApi_GetElement_inCompareObjPtr_union_length(int inElement)
{
	const struct xdr_discrim *choices = CAPI2_CcApi_GetElement_inCompareObjPtr_union_dscrm;
	for (; choices->proc != NULL_xdrproc_t; choices++)
	{
		if (choices->value == inElement) return choices->unsize;
	}
	return 0;
}


UInt32 CAPI2_CcApi_GetElement_outElementPtr_union_length(int inElement)
{
	const struct xdr_discrim *choices = CAPI2_CcApi_GetElement_outElementPtr_union_dscrm;
	for (; choices->proc != NULL_xdrproc_t; choices++)
	{
		if (choices->value == inElement) return choices->unsize;
	}
	return 0;
}


bool_t xdr_CAPI2_CcApi_GetElement_Req_t(void* xdrs, CAPI2_CcApi_GetElement_Req_t *rsp)
{
	struct xdr_discrim *entry = NULL;

	XDR_LOG(xdrs,"CAPI2_CcApi_GetElement_Req_t")

	if(
		xdr_CcApi_Element_t(xdrs, &rsp->inElement) &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->inCompareObjPtr,CAPI2_CcApi_GetElement_inCompareObjPtr_union_length(rsp->inElement), xdr_void) &&
		(rsp->inCompareObjPtr== NULL ||  xdr_union(xdrs, (enum_t *)&(rsp->inElement),(caddr_t)(void *)(rsp->inCompareObjPtr),CAPI2_CcApi_GetElement_inCompareObjPtr_union_dscrm,NULL_xdrproc_t, &entry, NULL) ) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_GetElement_Rsp_t(void* xdrs, CAPI2_CcApi_GetElement_Rsp_t *rsp)
{
	struct xdr_discrim *entry = NULL;

	XDR_LOG(xdrs,"CAPI2_CcApi_GetElement_Rsp_t")

	if(
		xdr_CcApi_Element_t(xdrs, &rsp->inElement) &&
		xdr_union(xdrs, (enum_t *)&(rsp->inElement),(caddr_t)(void *)(rsp->outElementPtr),CAPI2_CcApi_GetElement_outElementPtr_union_dscrm,NULL_xdrproc_t, &entry, NULL) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_WL_SocketFilterList_t(void* xdrs, WL_SocketFilterList_t *rsp)
{
	XDR_LOG(xdrs,"WL_SocketFilterList_t")

	if(
		_xdr_UInt16(xdrs, &rsp->filterType,"filterType") &&
		_xdr_Boolean(xdrs, &rsp->bAutomaticRrcRelease,"bAutomaticRrcRelease") &&
		xdr_vector(xdrs, (caddr_t)(void*)rsp->udpList,SOCKET_PACKETFILTER_UDP_MAX, sizeof(UInt16 ), xdr_UInt16 ) &&
		xdr_vector(xdrs, (caddr_t)(void*)rsp->tcpList,SOCKET_PACKETFILTER_TCP_MAX, sizeof(UInt16 ), xdr_UInt16 ) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_WL_PsSetFilterList_Req_t(void* xdrs, CAPI2_WL_PsSetFilterList_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_WL_PsSetFilterList_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->cid,"cid") &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->inDataPtr,sizeof( WL_SocketFilterList_t ), xdr_WL_SocketFilterList_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetProtConfigOptions_Req_t(void* xdrs, CAPI2_PdpApi_GetProtConfigOptions_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetProtConfigOptions_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->inCid,"inCid") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_GetProtConfigOptions_Rsp_t(void* xdrs, CAPI2_PdpApi_GetProtConfigOptions_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_GetProtConfigOptions_Rsp_t")

	if(
		xdr_PCHProtConfig_t(xdrs, &rsp->outProtConfigPtr) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PdpApi_SetProtConfigOptions_Req_t(void* xdrs, CAPI2_PdpApi_SetProtConfigOptions_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PdpApi_SetProtConfigOptions_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->inCid,"inCid") &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->inProtConfigPtr,sizeof( PCHProtConfig_t ), xdr_PCHProtConfig_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LcsApi_RrlpSendDataToNetwork_Req_t(void* xdrs, CAPI2_LcsApi_RrlpSendDataToNetwork_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_LcsApi_RrlpSendDataToNetwork_Req_t")

	if(
		xdr_UInt32(xdrs, &rsp->inDataLen) &&
		xdr_copy_len(&len, (u_int)(rsp->inDataLen)) && xdr_bytes(xdrs, (char **)(void*)&rsp->inData, &len, 0xFFFF) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LcsApi_RrcSendUlDcch_Req_t(void* xdrs, CAPI2_LcsApi_RrcSendUlDcch_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"CAPI2_LcsApi_RrcSendUlDcch_Req_t")

	if(
		xdr_UInt32(xdrs, &rsp->inDataLen) &&
		xdr_copy_len(&len, (u_int)(rsp->inDataLen)) && xdr_bytes(xdrs, (char **)(void*)&rsp->inData, &len, 0xFFFF) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LcsApi_RrcMeasCtrlFailure_Req_t(void* xdrs, CAPI2_LcsApi_RrcMeasCtrlFailure_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LcsApi_RrcMeasCtrlFailure_Req_t")

	if(
		_xdr_UInt16(xdrs, &rsp->inTransactionId,"inTransactionId") &&
		xdr_LcsRrcMcFailure_t(xdrs, &rsp->inMcFailure) &&
		xdr_UInt32(xdrs, &rsp->inErrorCode) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LcsApi_RrcStatus_Req_t(void* xdrs, CAPI2_LcsApi_RrcStatus_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LcsApi_RrcStatus_Req_t")

	if(
		xdr_LcsRrcMcStatus_t(xdrs, &rsp->inStatus) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_PowerOnOffSim_Req_t(void* xdrs, CAPI2_SimApi_PowerOnOffSim_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_PowerOnOffSim_Req_t")

	if(
		_xdr_Boolean(xdrs, &rsp->power_on,"power_on") &&
		xdr_SIM_POWER_ON_MODE_t(xdrs, &rsp->mode) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SimApi_PowerOnOffSim_Rsp_t(void* xdrs, CAPI2_SimApi_PowerOnOffSim_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SimApi_PowerOnOffSim_Rsp_t")

	if(
		 xdr_SIMAccess_t(xdrs, &rsp->val) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PhoneCtrlApi_SetPagingStatus_Req_t(void* xdrs, CAPI2_PhoneCtrlApi_SetPagingStatus_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PhoneCtrlApi_SetPagingStatus_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->status,"status") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LcsApi_GetGpsCapabilities_Rsp_t(void* xdrs, CAPI2_LcsApi_GetGpsCapabilities_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LcsApi_GetGpsCapabilities_Rsp_t")

	if(
		_xdr_UInt16(xdrs, &rsp->outCapMaskPtr,"outCapMaskPtr") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LcsApi_SetGpsCapabilities_Req_t(void* xdrs, CAPI2_LcsApi_SetGpsCapabilities_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LcsApi_SetGpsCapabilities_Req_t")

	if(
		_xdr_UInt16(xdrs, &rsp->inCapMask,"inCapMask") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CcApi_AbortDtmfTone_Req_t(void* xdrs, CAPI2_CcApi_AbortDtmfTone_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CcApi_AbortDtmfTone_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->inDtmfObjPtr,sizeof( ApiDtmf_t ), xdr_ApiDtmf_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_NetRegApi_SetSupportedRATandBandEx_Req_t(void* xdrs, CAPI2_NetRegApi_SetSupportedRATandBandEx_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_NetRegApi_SetSupportedRATandBandEx_Req_t")

	if(
		xdr_RATSelect_t(xdrs, &rsp->RAT_cap) &&
		xdr_BandSelect_t(xdrs, &rsp->band_cap) &&
		xdr_RATSelect_t(xdrs, &rsp->RAT_cap2) &&
		xdr_BandSelect_t(xdrs, &rsp->band_cap2) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SmsReportInd_Rsp_t(void* xdrs, CAPI2_SmsReportInd_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsReportInd_Rsp_t")

	if(
		 xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( SmsSimMsg_t ), xdr_SmsSimMsg_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}
