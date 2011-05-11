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
/**
*
*   @file   capi2_sim_msg.c
*
*   @brief  This file implements the serialize/deserialize the request/respose
*	for SIM.
*
****************************************************************************/
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


#define _T(a) a

XDR_ENUM_DECLARE(SIMServiceStatus_t)

XDR_ENUM_DECLARE(SIM_APPL_TYPE_t)
XDR_ENUM_DECLARE(EFILE_TYPE_t)
XDR_ENUM_DECLARE(EFILE_ACCESS_CONDITION_t)
XDR_ENUM_DECLARE(USIM_EFILE_ACCESS_CONDITION_t)
XDR_ENUM_DECLARE(USIM_LIFE_CYCLE_STATYS_t)
XDR_ENUM_DECLARE(SIMLockType_t)
XDR_ENUM_DECLARE(SIMLock_Status_t)
XDR_ENUM_DECLARE(SIM_PLMN_FILE_t)
XDR_ENUM_DECLARE(APDUCmd_t)
XDR_ENUM_DECLARE(SIM_SECURITY_STATE_t)

XDR_ENUM_FUNC(SIMAccess_t)
XDR_ENUM_FUNC(SIM_MWI_TYPE_t)
XDR_ENUM_FUNC(CHV_t)

XDR_ENUM_FUNC(SIMOperState_t)
XDR_ENUM_FUNC(SIMPBK_ID_t)
XDR_ENUM_FUNC(SIMService_t)
XDR_ENUM_FUNC(APDUFileID_t)
XDR_ENUM_FUNC(SERVICE_CODE_STATUS_CPHS_t)
XDR_ENUM_FUNC(CPHS_SST_ENTRY_t)

XDR_ENUM_FUNC(SIMPhase_t)
XDR_ENUM_FUNC(SIMType_t)
XDR_ENUM_FUNC(SIMPresent_t)
XDR_ENUM_FUNC(SIMServiceStatus_t)
XDR_ENUM_FUNC(SIM_PIN_Status_t)

XDR_ENUM_FUNC(SIM_APPL_TYPE_t)
XDR_ENUM_FUNC(EFILE_TYPE_t)
XDR_ENUM_FUNC(EFILE_ACCESS_CONDITION_t)
XDR_ENUM_FUNC(USIM_EFILE_ACCESS_CONDITION_t)
XDR_ENUM_FUNC(USIM_LIFE_CYCLE_STATYS_t)
XDR_ENUM_FUNC(SERVICE_FLAG_STATUS_t)
XDR_ENUM_FUNC(SIMLockType_t)
XDR_ENUM_FUNC(SIMLock_Status_t)
XDR_ENUM_FUNC(SIM_PLMN_FILE_t)
XDR_ENUM_FUNC(APDUCmd_t)

XDR_ENUM_FUNC(USIM_APPLICATION_TYPE)
XDR_ENUM_FUNC(USIM_EST_SERVICE_t)
XDR_ENUM_FUNC(USIM_RAT_MODE_t)
XDR_ENUM_FUNC(SIMBdnOperState_t)
XDR_ENUM_FUNC(SIM_POWER_ON_MODE_t)
XDR_ENUM_FUNC(SIM_GENERIC_APDU_EVENT_t)
XDR_ENUM_FUNC(SIM_GENERIC_APDU_RESULT_t)
XDR_ENUM_FUNC(SIM_FATAL_ERROR_t)
XDR_ENUM_FUNC(SIM_SIM_INTERFACE_t)
XDR_ENUM_FUNC(SIM_SECURITY_STATE_t)
XDR_ENUM_FUNC(SIMSMSMesgStatus_t)
XDR_ENUM_FUNC(SIM_APP_OCCURRENCE_t)


bool_t xdr_EPPU_t( XDR* xdrs, EPPU_t* data);
bool_t xdr_PLMNId_t( XDR* xdrs, PLMNId_t* data);
bool_t xdr_PLMN_ID_t( XDR* xdrs, PLMN_ID_t* data);
bool_t xdr_SIM_PLMN_ENTRY_t( XDR* xdrs, SIM_PLMN_ENTRY_t* data);
bool_t xdr_SIM_2G_LANGUAGE_t(XDR* xdrs, SIM_2G_LANGUAGE_t *sim_2g_language);
bool_t xdr_SIM_3G_LANGUAGE_t(XDR* xdrs, SIM_3G_LANGUAGE_t *sim_3g_language);


static const struct xdr_discrim SIM_DETECTION_t_dscrm[] = 
{
	{ (int) SIM_APPL_2G, _T("SIM_APPL_2G"), (xdrproc_t) xdr_SIM_2G_LANGUAGE_t,0,0,0,0 },
	{ (int) SIM_APPL_3G, _T("SIM_APPL_3G"), (xdrproc_t) xdr_SIM_3G_LANGUAGE_t,0,0,0,0 },
	{ (int) SIM_APPL_INVALID, _T("SIM_APPL_INVALID"), (xdrproc_t) xdr_default_proc,0,0,0,0},
	{ __dontcare__, _T(""), NULL_xdrproc_t } 
};


bool_t 
xdr_SIM_SMS_UPDATE_RESULT_t(XDR* xdrs, SIM_SMS_UPDATE_RESULT_t* simSmsUpdResult)
{
	XDR_LOG(xdrs,"SIM_SMS_UPDATE_RESULT_t")

	if( _xdr_u_int16_t(xdrs, &simSmsUpdResult->rec_no,"rec_no") &&
		XDR_ENUM(xdrs, &simSmsUpdResult->result, SIMAccess_t)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SIMSMSMesg_t( XDR* xdrs, SIMSMSMesg_t* simsmsmesg)
{
	XDR_LOG(xdrs,"SIMSMSMesg_t")

	if( xdr_opaque(xdrs, (caddr_t)&simsmsmesg->mesg_data, SMSMESG_DATA_SZ) &&
			XDR_ENUM(xdrs, &simsmsmesg->status, SIMSMSMesgStatus_t)
		)
		return(TRUE);
	else
		return(FALSE);
}


bool_t 
xdr_SIM_SMS_DATA_t( XDR* xdrs, SIM_SMS_DATA_t* simsmsdata)
{
	XDR_LOG(xdrs,"SIM_SMS_DATA_t")

	if( _xdr_u_int16_t(xdrs, &simsmsdata->rec_no,"rec_no") &&
		XDR_ENUM(xdrs, &simsmsdata->result, SIMAccess_t) &&
		xdr_SIMSMSMesg_t(xdrs, &simsmsdata->sms_mesg)
		)
		return(TRUE);
	else
		return(FALSE);
}



bool_t 
xdr_PUKString_t(XDR* xdrs, PUKString_t* puk)
{
	XDR_LOG(xdrs,"PUKString_t")

	return xdr_opaque(xdrs, (caddr_t)puk, sizeof(PUKString_t));
}

bool_t 
xdr_CHVString_t(XDR* xdrs, CHVString_t* chv)
{
	XDR_LOG(xdrs,"CHVString_t")

	return xdr_opaque(xdrs, (caddr_t)chv, sizeof(CHVString_t));
}







bool_t 
xdr_SIM_PBK_INFO_t( XDR* xdrs, SIM_PBK_INFO_t* data)
{
	XDR_LOG(xdrs,"SIM_PBK_INFO_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		XDR_ENUM(xdrs, &data->pbk, SIMPBK_ID_t) &&
		_xdr_u_int16_t(xdrs, &data->total,"total") &&
		_xdr_u_int16_t(xdrs, &data->alpha_sz,"alpha_sz")
		)
		return(TRUE);
	else
		return(FALSE);
}



bool_t 
xdr_SIM_SVC_PROV_NAME_t( XDR* xdrs, SIM_SVC_PROV_NAME_t* data)
{
	XDR_LOG(xdrs,"SIM_SVC_PROV_NAME_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		_xdr_u_char(xdrs, &data->display_flag1,"display_flag1") &&
		_xdr_u_char(xdrs, &data->display_flag2,"display_flag2") &&
		xdr_opaque(xdrs, (caddr_t)&data->name, sizeof(SvcProvName_t))
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_EPPU_t( XDR* xdrs, EPPU_t* data)
{
	XDR_LOG(xdrs,"EPPU_t")

	if( _xdr_u_int16_t(xdrs, &data->mant,"mant") &&
		xdr_int16_t(xdrs, &data->exp)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SIM_PUCT_DATA_t( XDR* xdrs, SIM_PUCT_DATA_t* data)
{
	XDR_LOG(xdrs,"SIM_PUCT_DATA_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		xdr_opaque(xdrs, (caddr_t)&data->currency, sizeof(CurrencyName_t)) &&
		xdr_EPPU_t(xdrs, &data->eppu)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_CallMeterUnit_t(XDR* xdrs, CallMeterUnit_t* data)
{
	XDR_LOG(xdrs,"CallMeterUnit_t")

	return xdr_u_long(xdrs, data);
}



bool_t 
xdr_SIM_MAX_ACM_t( XDR* xdrs, SIM_MAX_ACM_t* data)
{
	XDR_LOG(xdrs,"SIM_MAX_ACM_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		xdr_CallMeterUnit_t(xdrs, &data->max_acm)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SIM_ACM_VALUE_t( XDR* xdrs, SIM_ACM_VALUE_t* data)
{
	XDR_LOG(xdrs,"SIM_ACM_VALUE_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		xdr_CallMeterUnit_t(xdrs, &data->acm)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SIM_CALL_FORWARD_UNCONDITIONAL_FLAG_t( XDR* xdrs, SIM_CALL_FORWARD_UNCONDITIONAL_FLAG_t* data)
{
	XDR_LOG(xdrs,"SIM_CALL_FORWARD_UNCONDITIONAL_FLAG_t")

	if( _xdr_u_char(xdrs, &data->call_forward_l1,"call_forward_l1") &&
		_xdr_u_char(xdrs, &data->call_forward_l2,"call_forward_l2")	&&
		_xdr_u_char(xdrs, &data->call_forward_fax,"call_forward_fax")	&&
		_xdr_u_char(xdrs, &data->call_forward_data,"call_forward_data")
		)
		return(TRUE);
	else
		return(FALSE);
}


bool_t 
xdr_SIM_CALL_FORWARD_UNCONDITIONAL_STATUS_t( XDR* xdrs, SIM_CALL_FORWARD_UNCONDITIONAL_STATUS_t* data)
{
	XDR_LOG(xdrs,"SIM_CALL_FORWARD_UNCONDITIONAL_STATUS_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		_xdr_u_char(xdrs, &data->sim_exist,"sim_exist")	&&
		xdr_SIM_CALL_FORWARD_UNCONDITIONAL_FLAG_t(xdrs, &data->value)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SIM_ICCID_STATUS_t( XDR* xdrs, SIM_ICCID_STATUS_t* data)
{
	XDR_LOG(xdrs,"SIM_ICCID_STATUS_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		xdr_opaque(xdrs, (caddr_t)&data->iccid, sizeof(ICCID_ASCII_t) )
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_APDU_t(XDR* xdrs, APDU_t* data)
{
	XDR_LOG(xdrs, "xdr_APDU_t")

	if( xdr_u_long(xdrs, &data->length) &&
		xdr_opaque(xdrs, (caddr_t) &data->data, sizeof(data->data) )
		)
		return(TRUE);
	else
		return(FALSE);
}




bool_t 
xdr_PIN_ATTEMPT_RESULT_t( XDR* xdrs, PIN_ATTEMPT_RESULT_t* data)
{
	XDR_LOG(xdrs,"PIN_ATTEMPT_RESULT_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		_xdr_u_char(xdrs, &data->pin1_attempt_left,"pin1_attempt_left")	&&
		_xdr_u_char(xdrs, &data->pin2_attempt_left,"pin2_attempt_left")	&&
		_xdr_u_char(xdrs, &data->puk1_attempt_left,"puk1_attempt_left")	&&
		_xdr_u_char(xdrs, &data->puk2_attempt_left,"puk2_attempt_left")
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SIM_EFILE_UPDATE_RESULT_t( XDR* xdrs, SIM_EFILE_UPDATE_RESULT_t* data)
{
	XDR_LOG(xdrs,"SIM_EFILE_UPDATE_RESULT_t")

		return XDR_ENUM(xdrs, &data->result, SIMAccess_t);
}


bool_t 
xdr_SIM_SEEK_RECORD_DATA_t( XDR* xdrs, SIM_SEEK_RECORD_DATA_t* data)
{
	XDR_LOG(xdrs,"SIM_SEEK_RECORD_DATA_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		_xdr_u_char(xdrs, &data->data_len,"data_len")
		)
	{
		u_int len = (u_int)data->data_len;
		return xdr_bytes(xdrs, (char **)(void*)&data->ptr, &len, 1024);
	}
	else
		return(FALSE);
}


bool_t 
xdr_SIM_EFILE_DATA_t( XDR* xdrs, SIM_EFILE_DATA_t* data)
{
	XDR_LOG(xdrs,"SIM_EFILE_DATA_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		_xdr_u_int16_t(xdrs, &data->data_len,"data_len")
		)
	{
		u_int len = (u_int)data->data_len;
		return xdr_bytes(xdrs, (char **)(void*)&data->ptr, &len, MAX_BINARY_FILE_DATA_LEN);
	}
	else
		return(FALSE);
}

bool_t 
xdr_SIM_EFILE_INFO_t( XDR* xdrs, SIM_EFILE_INFO_t* data)
{
	XDR_LOG(xdrs,"SIM_EFILE_INFO_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		_xdr_u_int16_t(xdrs, &data->file_size,"file_size")	&&
		_xdr_u_char(xdrs, &data->record_length,"record_length")	&&
		XDR_ENUM(xdrs, &data->efile_type, EFILE_TYPE_t) &&
		XDR_ENUM(xdrs, &data->update_access_cond, EFILE_ACCESS_CONDITION_t) &&
		XDR_ENUM(xdrs, &data->read_seek_access_cond, EFILE_ACCESS_CONDITION_t) &&
		XDR_ENUM(xdrs, &data->increase_access_cond, EFILE_ACCESS_CONDITION_t) &&
		XDR_ENUM(xdrs, &data->invalidate_access_cond, EFILE_ACCESS_CONDITION_t) &&
		XDR_ENUM(xdrs, &data->rehabilitate_access_cond, EFILE_ACCESS_CONDITION_t) &&

		XDR_ENUM(xdrs, &data->usim_update_access_cond, USIM_EFILE_ACCESS_CONDITION_t) &&
		XDR_ENUM(xdrs, &data->usim_read_access_cond, USIM_EFILE_ACCESS_CONDITION_t) &&
		XDR_ENUM(xdrs, &data->usim_activate_access_cond, USIM_EFILE_ACCESS_CONDITION_t) &&
		XDR_ENUM(xdrs, &data->usim_deactivate_access_cond, USIM_EFILE_ACCESS_CONDITION_t) &&
		_xdr_u_char(xdrs, &data->validated,"validated")	&&
		_xdr_u_char(xdrs, &data->accessible_invalidated,"accessible_invalidated")	&&
		XDR_ENUM(xdrs, &data->life_cycle_satus, USIM_LIFE_CYCLE_STATYS_t)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_PLMNId_t( XDR* xdrs, PLMNId_t* data)
{
	XDR_LOG(xdrs,"PLMNId_t")

	if( _xdr_u_int16_t(xdrs, &data->mcc,"mcc") &&
		_xdr_u_char(xdrs, &data->mnc,"mnc")	&&
		_xdr_u_char(xdrs, &data->is_forbidden,"is_forbidden") &&
		_xdr_u_char(xdrs, &data->mnc_len,"mnc_len")
		)
		return(TRUE);
	else
		return(FALSE);
}



bool_t
xdr_SIM_OPEN_SOCKET_RES_t(XDR* xdrs, SIM_OPEN_SOCKET_RES_t* data)
{
	XDR_LOG(xdrs,"SIM_OPEN_SOCKET_RES_t")

	if( _xdr_u_char(xdrs, &data->socket_id,"socket id") )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_SIM_SELECT_APPLI_RES_t(XDR* xdrs, SIM_SELECT_APPLI_RES_t* data)
{
	XDR_LOG(xdrs,"SIM_SELECT_APPLI_RES_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_SIM_SELECT_APPLICATION_RES_t(XDR* xdrs, SIM_SELECT_APPLICATION_RES_t* data)
{
	XDR_LOG(xdrs,"SIM_SELECT_APPLICATION_RES_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_SIM_DEACTIVATE_APPLI_RES_t(XDR* xdrs, SIM_DEACTIVATE_APPLI_RES_t* data)
{
	XDR_LOG(xdrs,"SIM_DEACTIVATE_APPLI_RES_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_SIM_CLOSE_SOCKET_RES_t(XDR* xdrs, SIM_CLOSE_SOCKET_RES_t* data)
{
	XDR_LOG(xdrs,"SIM_CLOSE_SOCKET_RES_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) )
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SIM_DFILE_INFO_t( XDR* xdrs, SIM_DFILE_INFO_t* data)
{
	XDR_LOG(xdrs,"SIM_DFILE_INFO_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		_xdr_u_int16_t(xdrs, &data->mem_not_allocated,"mem_not_allocated")	&&
		_xdr_u_char(xdrs, &data->file_charact,"file_charact")	&&
		_xdr_u_char(xdrs, &data->num_df,"num_df")	&&
		_xdr_u_char(xdrs, &data->num_ef,"num_ef")	&&
		_xdr_u_char(xdrs, &data->num_chv,"num_chv")	&&
		_xdr_u_char(xdrs, &data->chv1_code_initialized,"chv1_code_initialized")	&&
		_xdr_u_char(xdrs, &data->chv1_unlock_attempt,"chv1_unlock_attempt")	&&
		_xdr_u_char(xdrs, &data->chv1_unblock_attempt,"chv1_unblock_attempt")	&&
		_xdr_u_char(xdrs, &data->chv2_code_initialized,"chv2_code_initialized")	&&
		_xdr_u_char(xdrs, &data->chv2_unlock_attempt,"chv2_unlock_attempt")	&&
		_xdr_u_char(xdrs, &data->chv2_unblock_attempt,"chv2_unblock_attempt")
		)
		return(TRUE);
	else
		return(FALSE);
}






bool_t 
xdr_PLMN_ID_t( XDR* xdrs, PLMN_ID_t* data)
{
	XDR_LOG(xdrs,"PLMN_ID_t")

	if( _xdr_u_int16_t(xdrs, &data->mcc,"mcc") &&
		_xdr_u_char(xdrs, &data->mnc,"mnc")	&&
		_xdr_u_char(xdrs, &data->order,"order")
		)
		return(TRUE);
	else
		return(FALSE);
}



bool_t 
xdr_CurrencyName_t( XDR* xdrs, CurrencyName_t* data)
{
	XDR_LOG(xdrs,"CurrencyName_t")

	if( xdr_opaque(xdrs, (caddr_t)data, sizeof(CurrencyName_t))
		)
		return(TRUE);
	else
		return(FALSE);
}


bool_t 
xdr_SIM_RESTRICTED_ACCESS_DATA_t( XDR* xdrs, SIM_RESTRICTED_ACCESS_DATA_t* data)
{
	XDR_LOG(xdrs,"SIM_RESTRICTED_ACCESS_DATA_t")

	if(XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		_xdr_u_char(xdrs, &data->sw1,"sw1")	&&
		_xdr_u_char(xdrs, &data->sw2,"sw2")	&&
		_xdr_u_int16_t(xdrs, &data->data_len,"data_len"))
	{
		u_int len = (u_int)data->data_len;
		return xdr_bytes(xdrs, (char **)(void*)&data->data, &len, 0xFFFF);
	}
	else
		return(FALSE);
}




bool_t xdr_SIM_MUL_PLMN_ENTRY_UPDATE_t( XDR* xdrs, SIM_MUL_PLMN_ENTRY_UPDATE_t* data)
{
	XDR_LOG(xdrs,"SIM_MUL_PLMN_ENTRY_UPDATE_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		XDR_ENUM(xdrs, &data->plmn_file, SIM_PLMN_FILE_t) &&
		_xdr_u_int16_t(xdrs, &data->start_index,"start_index") &&
		_xdr_u_int16_t(xdrs, &data->num_of_entry,"num_of_entry")
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_SIM_PLMN_ENTRY_t( XDR* xdrs, SIM_PLMN_ENTRY_t* data)
{
	XDR_LOG(xdrs,"SIM_PLMN_ENTRY_t")

	if( _xdr_u_int16_t(xdrs, &data->index,"index") &&
		_xdr_u_int16_t(xdrs, &data->mcc,"mcc") &&
		_xdr_u_char(xdrs, &data->mnc,"mnc") &&
		_xdr_u_char(xdrs, &data->gsm_act_selected,"gsm_act_selected") &&
		_xdr_u_char(xdrs, &data->gsm_compact_act_selected,"gsm_compact_act_selected") &&
		_xdr_u_char(xdrs, &data->utra_act_selected,"utra_act_selected") 
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_SIM_MUL_PLMN_ENTRY_t( XDR* xdrs, SIM_MUL_PLMN_ENTRY_t* data)
{
	XDR_LOG(xdrs,"SIM_MUL_PLMN_ENTRY_t")

	if( _xdr_u_int16_t(xdrs, &data->mcc,"mcc") &&
		_xdr_u_char(xdrs, &data->mnc,"mnc") &&
		_xdr_u_char(xdrs, &data->gsm_act_selected,"gsm_act_selected") &&
		_xdr_u_char(xdrs, &data->gsm_compact_act_selected,"gsm_compact_act_selected") &&
		_xdr_u_char(xdrs, &data->utra_act_selected,"utra_act_selected") 
		)
		return(TRUE);
	else
		return(FALSE);
}


bool_t xdr_SIM_MUL_REC_DATA_t( XDR* xdrs, SIM_MUL_REC_DATA_t* data)
{
	XDR_LOG(xdrs, "xdr_SIM_MUL_REC_DATA_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		_xdr_u_char(xdrs, &data->num_of_rec, "num_of_rec") &&
		_xdr_u_char(xdrs, &data->rec_len, "rec_len") )
	{
		u_int len = (u_int) data->rec_len * data->num_of_rec;
		return xdr_bytes(xdrs, (char**) &data->rec_data, &len, 0xFFFF); 
	}	
	else
	{
		return (FALSE);
	}
}


bool_t xdr_USIM_AID_DATA_t( XDR* xdrs, USIM_AID_DATA_t* data)
{
	XDR_LOG(xdrs, "xdr_USIM_AID_DATA_t")

	if ( _xdr_u_char(xdrs, &data->aidLen, "aidLen") )
	{
		return xdr_opaque(xdrs, (caddr_t) &data->aidData, data->aidLen);
	}
	else
	{
		return FALSE;
	}
}


bool_t xdr_UST_DATA_t( XDR* xdrs, UST_DATA_t* data)
{
	XDR_LOG(xdrs, "xdr_UST_DATA_t")

	if ( _xdr_u_char(xdrs, &data->ustLen, "ustLen") )
	{
		return xdr_opaque(xdrs, (caddr_t) &data->ustData, data->ustLen);
	}
	else
	{
		return FALSE;
	}
}



bool_t xdr_SIM_PLMN_ENTRY_DATA_t( XDR* xdrs, SIM_PLMN_ENTRY_DATA_t* data)
{
	XDR_LOG(xdrs,"SIM_PLMN_ENTRY_DATA_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		XDR_ENUM(xdrs, &data->plmn_file, SIM_PLMN_FILE_t) &&
		_xdr_u_char(xdrs, &data->num_of_entry,"num_of_entry") &&
		xdr_vector(xdrs,  (char*)data->plmn_entry, MAX_NUM_OF_PLMN, sizeof(SIM_PLMN_ENTRY_t), (xdrproc_t)xdr_SIM_PLMN_ENTRY_t) 
		)
		return(TRUE);
	else
		return(FALSE);
}


bool_t xdr_SIM_PLMN_NUM_OF_ENTRY_t( XDR* xdrs, SIM_PLMN_NUM_OF_ENTRY_t* data)
{
	XDR_LOG(xdrs,"SIM_PLMN_NUM_OF_ENTRY_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		XDR_ENUM(xdrs, &data->plmn_file, SIM_PLMN_FILE_t) &&
		_xdr_u_int16_t(xdrs, &data->num_of_total_entry,"num_of_total_entry") 
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_USIM_FILE_UPDATE_RSP_t(XDR* xdrs, USIM_FILE_UPDATE_RSP_t* rsp)
{
	XDR_LOG(xdrs,"USIM_FILE_UPDATE_RSP_t")

	return XDR_ENUM(xdrs, &rsp->result, SIMAccess_t);
}


bool_t 
xdr_APN_NAME_t( XDR* xdrs, APN_NAME_t* data)
{
	XDR_LOG(xdrs,"APN_NAME_t")

	return xdr_opaque(xdrs, (caddr_t)data, sizeof(APN_NAME_t) );
}

bool_t
xdr_SIM_GENERIC_APDU_RES_INFO_t(XDR* xdrs, SIM_GENERIC_APDU_RES_INFO_t* data)
{
	XDR_LOG(xdrs,"SIM_GENERIC_APDU_RES_INFO_t")

    if( XDR_ENUM(xdrs, &data->eventType, SIM_GENERIC_APDU_EVENT_t) &&
        XDR_ENUM(xdrs, &data->resultCode, SIM_GENERIC_APDU_RESULT_t)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_SIM_GENERIC_APDU_ATR_INFO_t(XDR* xdrs, SIM_GENERIC_APDU_ATR_INFO_t* info)
{
	XDR_LOG(xdrs,"SIM_GENERIC_APDU_ATR_INFO_t")

    if( XDR_ENUM(xdrs, &info->eventType, SIM_GENERIC_APDU_EVENT_t) &&
        XDR_ENUM(xdrs, &info->resultCode, SIM_GENERIC_APDU_RESULT_t) &&
        _xdr_u_char(xdrs, &info->len,"len") &&
        xdr_opaque(xdrs, (caddr_t)&info->data, SIM_GENERIC_APDU_MAX_ATR_LEN)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SIM_GENERIC_APDU_XFER_RSP_t(XDR* xdrs, SIM_GENERIC_APDU_XFER_RSP_t* info)
{
	XDR_LOG(xdrs,"SIM_GENERIC_APDU_XFER_RSP_t")

    if( XDR_ENUM(xdrs, &info->resultCode, SIM_GENERIC_APDU_RESULT_t) &&
        _xdr_u_int16_t(xdrs, &info->len,"len") &&
        xdr_opaque(xdrs, (caddr_t)&info->data, SIM_GENERIC_APDU_MAX_RSP_LEN)
		)
		return(TRUE);
	else
		return(FALSE);

}

bool_t
xdr_SIM_GENERIC_APDU_CMD_BUF_t( XDR* xdrs, SIM_GENERIC_APDU_CMD_BUF_t* data)
{
	XDR_LOG(xdrs,"SIM_GENERIC_APDU_CMD_BUF_t")

	return xdr_opaque(xdrs, (caddr_t)data, sizeof(SIM_GENERIC_APDU_CMD_BUF_t) );
}






bool_t
xdr_SIM_DETECTION_t(XDR* xdrs, SIM_DETECTION_t *sim_detection)
{
	struct xdr_discrim *entry = NULL;
	enum_t dscm = sim_detection->sim_appl_type;	

	XDR_LOG(xdrs,"xdr_SIM_DETECTION_t");

	if ( XDR_ENUM(xdrs, &sim_detection->sim_appl_type, SIM_APPL_TYPE_t) &&
		 xdr_union(xdrs, (enum_t *) &dscm, (caddr_t) (void *) &(sim_detection->lang_info), SIM_DETECTION_t_dscrm, NULL_xdrproc_t, &entry, NULL) &&
		 xdr_Boolean(xdrs, &sim_detection->ruim_supported) )
	{
		return(TRUE);
	}
	else
	{
		return(FALSE);
	}
}


bool_t 
xdr_SIM_2G_LANGUAGE_t(XDR* xdrs, SIM_2G_LANGUAGE_t *sim_2g_language)
{
	XDR_LOG(xdrs,"xdr_SIM_2G_LANGUAGE_t");

	if ( xdr_vector(xdrs, (char *) sim_2g_language->elp_language_list, MAX_NUM_OF_SIM_LANGUAGE, sizeof(UInt16), (xdrproc_t) xdr_u_int16_t) &&
		 xdr_vector(xdrs, (char *) sim_2g_language->lp_language_list, MAX_NUM_OF_SIM_LANGUAGE, sizeof(UInt8), (xdrproc_t) xdr_u_char) )
	{
		return(TRUE);
	}
	else
	{
		return(FALSE);
	}
}


bool_t 
xdr_SIM_3G_LANGUAGE_t(XDR* xdrs, SIM_3G_LANGUAGE_t *sim_3g_language)
{
	XDR_LOG(xdrs,"xdr_SIM_3G_LANGUAGE_t");

	if ( xdr_vector(xdrs, (char *) sim_3g_language->li_language_list, MAX_NUM_OF_SIM_LANGUAGE, sizeof(UInt16), (xdrproc_t) xdr_u_int16_t) &&
		 xdr_vector(xdrs, (char *) sim_3g_language->pl_language_list, MAX_NUM_OF_SIM_LANGUAGE, sizeof(UInt16), (xdrproc_t) xdr_u_int16_t) )
	{
		return(TRUE);
	}
	else
	{
		return(FALSE);
	}
}




bool_t
xdr_SIMLOCK_SIM_DATA_t(XDR* xdrs, SIMLOCK_SIM_DATA_t *sim_data)
{
	XDR_LOG(xdrs,"xdr_SIMLOCK_SIM_DATA_t");

	if ( xdr_opaque(xdrs, (caddr_t) &sim_data->imsi_string, sizeof(IMSI_t)) &&
		 xdr_opaque(xdrs, (caddr_t) &sim_data->gid1, sizeof(GID_DIGIT_t)) &&
		 xdr_u_char(xdrs, &sim_data->gid1_len) &&
		 xdr_opaque(xdrs, (caddr_t) &sim_data->gid2, sizeof(GID_DIGIT_t)) &&
		 xdr_u_char(xdrs, &sim_data->gid2_len) )
	{
		return(TRUE);
	}
	else
	{
		return(FALSE);
	}	 
}


bool_t 
xdr_SIMLOCK_STATE_t(XDR* xdrs, SIMLOCK_STATE_t *simlock_state)
{
	XDR_LOG(xdrs,"xdr_SIMLOCK_STATE_t");

	if ( XDR_ENUM(xdrs, &simlock_state->network_lock, SIM_SECURITY_STATE_t) &&
		 XDR_ENUM(xdrs, &simlock_state->network_subset_lock, SIM_SECURITY_STATE_t) &&
		 XDR_ENUM(xdrs, &simlock_state->service_provider_lock, SIM_SECURITY_STATE_t) &&
		 XDR_ENUM(xdrs, &simlock_state->corporate_lock, SIM_SECURITY_STATE_t) &&
		 XDR_ENUM(xdrs, &simlock_state->phone_lock, SIM_SECURITY_STATE_t) )
	{
		return(TRUE);
	}
	else
	{
		return(FALSE);
	}	
}


bool_t 
xdr_GID_DIGIT_t( XDR* xdrs, GID_DIGIT_t* data)
{
	XDR_LOG(xdrs,"GID_DIGIT_t")

	if( xdr_opaque(xdrs, (caddr_t)data, sizeof(GID_DIGIT_t))
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_ICCID_ASCII_t( XDR* xdrs, ICCID_ASCII_t* data)
{
	XDR_LOG(xdrs,"ICCID_ASCII_t")

	if( xdr_opaque(xdrs, (caddr_t)data, sizeof(ICCID_ASCII_t))
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_IMSI_t( XDR* xdrs, IMSI_t* data)
{
	XDR_LOG(xdrs,"IMSI_t")

	if( xdr_opaque(xdrs, (caddr_t)data, sizeof(IMSI_t))
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SIM_SCA_DATA_t( XDR* xdrs, SIM_SCA_DATA_t* data)
{
	XDR_LOG(xdrs,"SIM_SCA_DATA_t")

	if( xdr_opaque(xdrs, (caddr_t)data, sizeof(SIM_SCA_DATA_t))
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SimApduRsp_t( XDR* xdrs, SimApduRsp_t* data)
{
	XDR_LOG(xdrs,"SimApduRsp_t")

	if (XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
        xdr_APDU_t(xdrs, &data->rApdu))
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_PinStatusWord_t( XDR* xdrs, PinStatusWord_t* data)
{
	XDR_LOG(xdrs,"xdr_PinStatusWord_t")

	if (_xdr_Boolean(xdrs, &data->swValid, "swValid") &&
        _xdr_u_char(xdrs,  &data->sw1,"sw1") &&        
        _xdr_u_char(xdrs,  &data->sw2,"sw2")) 
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SimPinRsp_t( XDR* xdrs, SimPinRsp_t* data)
{
	XDR_LOG(xdrs,"xdr_SimPinRsp_t")

	if (XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
        xdr_PinStatusWord_t(xdrs, &data->sw))
		return(TRUE);
	else
		return(FALSE);
}

