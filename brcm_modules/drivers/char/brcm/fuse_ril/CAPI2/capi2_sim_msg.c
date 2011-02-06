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
#include	"capi2_reqrep.h"
#include	"capi2_sim_msg.h"

#define _T(a) a

XDR_ENUM_DECLARE(SIMPhase_t)
XDR_ENUM_DECLARE(SIMType_t)
XDR_ENUM_DECLARE(SIMPresent_t)
XDR_ENUM_DECLARE(SIMServiceStatus_t)
XDR_ENUM_DECLARE(SIM_PIN_Status_t)

XDR_ENUM_DECLARE(SIM_APPL_TYPE_t)
XDR_ENUM_DECLARE(EFILE_TYPE_t)
XDR_ENUM_DECLARE(EFILE_ACCESS_CONDITION_t)
XDR_ENUM_DECLARE(USIM_EFILE_ACCESS_CONDITION_t)
XDR_ENUM_DECLARE(USIM_LIFE_CYCLE_STATYS_t)
XDR_ENUM_DECLARE(SERVICE_FLAG_STATUS_t)
XDR_ENUM_DECLARE(SIMPLMNAction_t)
XDR_ENUM_DECLARE(SIMLockType_t)
XDR_ENUM_DECLARE(SIMLock_Status_t)
XDR_ENUM_DECLARE(SIM_PLMN_FILE_t)
XDR_ENUM_DECLARE(APDUCmd_t)
XDR_ENUM_DECLARE(SIM_SECURITY_STATE_t)

XDR_ENUM_FUNC(SIMAccess_t)
XDR_ENUM_FUNC(SIMSMSMesgStatus_t)
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
XDR_ENUM_FUNC(SIMPLMNAction_t)
XDR_ENUM_FUNC(SIMLockType_t)
XDR_ENUM_FUNC(SIMLock_Status_t)
XDR_ENUM_FUNC(SIM_PLMN_FILE_t)
XDR_ENUM_FUNC(APDUCmd_t)

XDR_ENUM_FUNC(USIM_APPLICATION_TYPE)
XDR_ENUM_FUNC(USIM_EST_SERVICE_t)
XDR_ENUM_FUNC(USIM_RAT_MODE_t)
XDR_ENUM_FUNC(SIMBdnOperState_t)
XDR_ENUM_FUNC(SIM_SECURITY_STATE_t)

XDR_ENUM_FUNC(SIM_POWER_ON_MODE_t)
XDR_ENUM_FUNC(SIM_GENERIC_APDU_EVENT_t)
XDR_ENUM_FUNC(SIM_GENERIC_APDU_RESULT_t)
XDR_ENUM_FUNC(SIM_FATAL_ERROR_t)

bool_t xdr_EPPU_t( XDR* xdrs, EPPU_t* data);
bool_t xdr_PLMNId_t( XDR* xdrs, PLMNId_t* data);
bool_t xdr_PLMN_ID_t( XDR* xdrs, PLMN_ID_t* data);
bool_t xdr_SIM_PLMN_ENTRY_t( XDR* xdrs, SIM_PLMN_ENTRY_t* data);


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
xdr_CAPI2_SIM_PBK_INFO_t(XDR* xdrs, CAPI2_SIM_PBK_INFO_t* rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_PBK_INFO_t")

	if( xdr_pointer(xdrs, (char **)(void*) &rsp->rsp, sizeof(SIM_PBK_INFO_t), (xdrproc_t)xdr_SIM_PBK_INFO_t)
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
xdr_SIM_BOOLEAN_DATA_t( XDR* xdrs, SIM_BOOLEAN_DATA_t* data)
{
	XDR_LOG(xdrs,"SIM_BOOLEAN_DATA_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		_xdr_u_char(xdrs, &data->value,"value")
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SIM_INTEGER_DATA_t( XDR* xdrs, SIM_INTEGER_DATA_t* data)
{
	XDR_LOG(xdrs,"SIM_INTEGER_DATA_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		xdr_u_long(xdrs, &data->value)
		)
		return(TRUE);
	else
		return(FALSE);
}

//////////
bool_t 
xdr_SIM_PHASE_RESULT_t( XDR* xdrs, SIM_PHASE_RESULT_t* data)
{
	XDR_LOG(xdrs,"SIM_PHASE_RESULT_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		XDR_ENUM(xdrs, &data->phase, SIMPhase_t)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SIM_TYPE_RESULT_t( XDR* xdrs, SIM_TYPE_RESULT_t* data)
{
	XDR_LOG(xdrs,"SIM_TYPE_RESULT_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		XDR_ENUM(xdrs, &data->type, SIMType_t)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SIM_PRESENT_RESULT_t( XDR* xdrs, SIM_PRESENT_RESULT_t* data)
{
	XDR_LOG(xdrs,"SIM_PRESENT_RESULT_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		XDR_ENUM(xdrs, &data->status, SIMPresent_t)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SIM_SERVICE_STATUS_RESULT_t( XDR* xdrs, SIM_SERVICE_STATUS_RESULT_t* data)
{
	XDR_LOG(xdrs,"SIM_SERVICE_STATUS_RESULT_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		XDR_ENUM(xdrs, &data->status, SIMServiceStatus_t)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SIM_ACCESS_RESULT_t( XDR* xdrs, SIM_ACCESS_RESULT_t* data)
{
	XDR_LOG(xdrs,"SIM_ACCESS_RESULT_t")

	return XDR_ENUM(xdrs, &data->result, SIMAccess_t);
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

bool_t 
xdr_SIM_PIN_STATUS_RESULT_t( XDR* xdrs, SIM_PIN_STATUS_RESULT_t* data)
{
	XDR_LOG(xdrs,"SIM_PIN_STATUS_RESULT_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		XDR_ENUM(xdrs, &data->pin_status, SIM_PIN_Status_t)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SIM_IMSI_RESULT_t( XDR* xdrs, SIM_IMSI_RESULT_t* data)
{
	XDR_LOG(xdrs,"SIM_IMSI_RESULT_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		xdr_opaque(xdrs, (caddr_t)&data->imsi, sizeof(IMSI_t) )
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SIM_GID_RESULT_t( XDR* xdrs, SIM_GID_RESULT_t* data)
{
	XDR_LOG(xdrs,"SIM_GID_RESULT_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		xdr_opaque(xdrs, (caddr_t)&data->gid, sizeof(GID_DIGIT_t) )
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_USIM_UST_DATA_RSP_t( XDR* xdrs, USIM_UST_DATA_RSP_t* data)
{
	XDR_LOG(xdrs,"USIM_UST_DATA_RSP_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		xdr_opaque(xdrs, (caddr_t)&data->Ust_Data, MAX_UST_LEN )
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SIM_APPL_TYPE_RESULT_t( XDR* xdrs, SIM_APPL_TYPE_RESULT_t* data)
{
	XDR_LOG(xdrs,"SIM_APPL_TYPE_RESULT_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		XDR_ENUM(xdrs, &data->type, SIM_APPL_TYPE_t)
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
xdr_SIM_ATR_DATA_t(XDR* xdrs, SIM_ATR_DATA_t* data)
{
	XDR_LOG(xdrs, "SIM_ATR_DATA_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		xdr_APDU_t(xdrs, &data->atr_data) 
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SIM_SCA_DATA_RESULT_t( XDR* xdrs, SIM_SCA_DATA_RESULT_t* data)
{
	XDR_LOG(xdrs,"SIM_SCA_DATA_RESULT_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		xdr_opaque(xdrs, (caddr_t)&data->data, sizeof(SIM_SCA_DATA_t) )
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SIM_SERVICE_FLAG_STATUS_t( XDR* xdrs, SIM_SERVICE_FLAG_STATUS_t* data)
{
	XDR_LOG(xdrs,"SIM_SERVICE_FLAG_STATUS_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		XDR_ENUM(xdrs, &data->value, SERVICE_FLAG_STATUS_t)
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
xdr_SIM_GEN_REC_DATA_t( XDR* xdrs, SIM_GEN_REC_DATA_t* data)
{
	XDR_LOG(xdrs,"SIM_GEN_REC_DATA_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		_xdr_u_char(xdrs, &data->num_of_rec,"num_of_rec")	&&
		_xdr_u_char(xdrs, &data->rec_len,"rec_len")
		)
	{
		u_int len = (u_int)data->rec_len;
		return xdr_bytes(xdrs, (char **)(void*)&data->rec_data, &len, 1536);
	}
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
		_xdr_u_char(xdrs, &data->is_forbidden,"is_forbidden")
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SIM_HOME_PLMN_RESULT_t( XDR* xdrs, SIM_HOME_PLMN_RESULT_t* data)
{
	XDR_LOG(xdrs,"SIM_HOME_PLMN_RESULT_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		xdr_PLMNId_t(xdrs, &data->plmn)
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
xdr_SIM_APDU_FILEID_RESULT_t( XDR* xdrs, SIM_APDU_FILEID_RESULT_t* data)
{
	XDR_LOG(xdrs,"SIM_APDU_FILEID_RESULT_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		XDR_ENUM(xdrs, &data->simFileId, APDUFileID_t)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_CAPI2_SimFileInfo_t( XDR* xdrs, CAPI2_SimFileInfo_t* data)
{
	XDR_LOG(xdrs,"CAPI2_SimFileInfo_t")

	if( XDR_ENUM(xdrs, &data->efile_id, APDUFileID_t) &&
		XDR_ENUM(xdrs, &data->dfile_id, APDUFileID_t) &&
		_xdr_u_int16_t(xdrs, &data->ptr_len, "ptr_len") &&
		_xdr_u_int16_t(xdrs, &data->length, "length") &&
		_xdr_u_char(xdrs, &data->path_len, "path_len") &&
		_xdr_u_int16_t(xdrs, &data->offset, "offset") &&
		_xdr_u_char(xdrs, &data->rec_no, "rec_no") &&
		_xdr_u_char(xdrs, &data->socket_id, "socket_id")
		)
	{
		bool_t result;
		u_int len = (u_int)data->ptr_len;
		result = xdr_bytes(xdrs, (char **)(void*)&data->ptr, &len, MAX_BINARY_FILE_DATA_LEN);
		
		if(result)
		{
			len = (u_int)data->path_len;
			result = xdr_array(xdrs,(char **)(void*) &data->select_path,&len,~0,sizeof(UInt16),(xdrproc_t) xdr_u_int16_t);

		}
		return result;
	}
	else
		return(FALSE);
}

bool_t 
xdr_CAPI2_SimSelectAppli_t(XDR* xdrs, CAPI2_SimSelectAppli_t* data)
{
	XDR_LOG(xdrs,"CAPI2_SimSelectAppli_t")

	if( _xdr_u_char(xdrs, &data->socket_id, "socket_id") &&
		_xdr_u_char(xdrs, &data->aid_len, "aid_len") )
	{		
		u_int len = (u_int) data->aid_len;
		bool_t result = xdr_bytes(xdrs, (char **)(void*) &data->aid_data, &len, 0xFF);
		
		return result;
	}
	else
		return(FALSE);
}

bool_t 
xdr_CAPI2_SimDeactivateAppli_t(XDR* xdrs, CAPI2_SimDeactivateAppli_t* data)
{
	XDR_LOG(xdrs,"CAPI2_SimDeactivateAppli_t")

	if( _xdr_u_char(xdrs, &data->socket_id, "socket_id") )		
		return (TRUE);
	else
		return (FALSE);
}

bool_t 
xdr_CAPI2_SimCloseSocket_t(XDR* xdrs, CAPI2_SimCloseSocket_t* data)
{
	XDR_LOG(xdrs,"CAPI2_SimCloseSocket_t")

	if( _xdr_u_char(xdrs, &data->socket_id, "socket_id") )
	{		
		return (TRUE);
	}
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
xdr_CAPI2_SIM_SendUpdatePrefListReq_t( XDR* xdrs, CAPI2_SIM_SendUpdatePrefListReq_t* data)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendUpdatePrefListReq_t")

	if( xdr_pointer(xdrs, (char **)(void*)&data->plmn, sizeof(PLMN_ID_t), xdr_PLMN_ID_t) &&
		XDR_ENUM(xdrs, &data->action, SIMPLMNAction_t)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_CAPI2_SIM_SendWritePuctReq_t( XDR* xdrs, CAPI2_SIM_SendWritePuctReq_t* data)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendWritePuctReq_t")

	if( xdr_pointer(xdrs, (char **)(void*)&data->eppu, sizeof(EPPU_t), xdr_EPPU_t) &&
		xdr_opaque(xdrs, (caddr_t)&data->currency, sizeof(CurrencyName_t))
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_CAPI2_SIM_SendGenericAccessReq_t( XDR* xdrs, CAPI2_SIM_SendGenericAccessReq_t* data)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendGenericAccessReq_t")

	if(_xdr_u_int16_t(xdrs, &data->data_length,"data_length"))
	{
		u_int len = (u_int)data->data_length;
		return xdr_bytes(xdrs, (char **)(void*)&data->data, &len, 0xFFFF);
	}
	else
		return(FALSE);
}

bool_t 
xdr_CAPI2_SIM_SendRestrictedAccessReq_t( XDR* xdrs, CAPI2_SIM_SendRestrictedAccessReq_t* data)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendRestrictedAccessReq_t")

	if( XDR_ENUM(xdrs, &data->command, APDUCmd_t) &&
		XDR_ENUM(xdrs, &data->efile_id, APDUFileID_t) &&
		XDR_ENUM(xdrs, &data->dfile_id, APDUFileID_t) &&
		_xdr_u_char(xdrs, &data->p1,"p1")	&&
		_xdr_u_char(xdrs, &data->p2,"p2")	&&
		_xdr_u_char(xdrs, &data->p3,"p3")	&&
		_xdr_u_char(xdrs, &data->path_len,"path_len")
		)
	{
		bool_t result;
		u_int len = (u_int)data->p3;
		result = xdr_bytes(xdrs, (char **)(void*)&data->data, &len, 1024);
		
		if(result)
		{
			len = (u_int)data->path_len;
			result = xdr_array(xdrs,(char **)(void*) &data->select_path,&len,~0,sizeof(UInt16),(xdrproc_t) xdr_u_int16_t);
		}
		return result;
	}
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

bool_t 
xdr_SIM_GENERIC_ACCESS_DATA_t( XDR* xdrs, SIM_GENERIC_ACCESS_DATA_t* data)
{
	XDR_LOG(xdrs,"SIM_GENERIC_ACCESS_DATA_t")

	if(XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		_xdr_u_int16_t(xdrs, &data->data_len,"data_len"))
	{
		u_int len = (u_int)data->data_len;
		return xdr_bytes(xdrs, (char **)(void*)&data->data, &len, 0xFFFF);
	}
	else
		return(FALSE);
}


bool_t 
xdr_CAPI2_SIMLockSetLock_t( XDR* xdrs, CAPI2_SIMLockSetLock_t* data)
{
	XDR_LOG(xdrs,"CAPI2_SIMLockSetLock_t")

	if( _xdr_u_char(xdrs, &data->action,"action")	&&
		_xdr_u_char(xdrs, &data->ph_sim_full_lock_on,"ph_sim_full_lock_on")	&&
		XDR_ENUM(xdrs, &data->lockType, SIMLockType_t) &&
		xdr_uchar_ptr_t(xdrs, &data->key)	&&
		xdr_uchar_ptr_t(xdrs, &data->imsi)	&&
		xdr_uchar_ptr_t(xdrs, &data->gid1)	&&
		xdr_uchar_ptr_t(xdrs, &data->gid2)
		)
		return(TRUE);
	else
		return(FALSE);
}

///////////////////////////////////

bool_t 
xdr_SIM_LOCK_ON_RESULT_t( XDR* xdrs, SIM_LOCK_ON_RESULT_t* data)
{
	XDR_LOG(xdrs,"SIM_LOCK_ON_RESULT_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		_xdr_u_char(xdrs, &data->phsLock,"phsLock")	&&
		_xdr_u_char(xdrs, &data->simLock,"simLock")
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SIM_LOCK_RESULT_t( XDR* xdrs, SIM_LOCK_RESULT_t* data)
{
	XDR_LOG(xdrs,"SIM_LOCK_RESULT_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		XDR_ENUM(xdrs, &data->lockStatus, SIMLock_Status_t)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SIM_LOCK_TYPE_RESULT_t( XDR* xdrs, SIM_LOCK_TYPE_RESULT_t* data)
{
	XDR_LOG(xdrs,"SIM_LOCK_TYPE_RESULT_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		_xdr_u_char(xdrs, &data->lockBlocked,"lockBlocked")	&&
		XDR_ENUM(xdrs, &data->lockType, SIMLockType_t)
		)
		return(TRUE);
	else
		return(FALSE);
}


bool_t 
xdr_SIM_LOCK_SIG_DATA_t( XDR* xdrs, SIM_LOCK_SIG_DATA_t* data)
{
	XDR_LOG(xdrs,"SIM_LOCK_SIG_DATA_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		xdr_opaque(xdrs, (caddr_t)&data->data, sizeof(MAX_SIM_SIG_DATA) )
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SIM_LOCK_IMEI_DATA_t( XDR* xdrs, SIM_LOCK_IMEI_DATA_t* data)
{
	XDR_LOG(xdrs,"SIM_LOCK_IMEI_DATA_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		xdr_opaque(xdrs, (caddr_t)&data->data, sizeof(MAX_SIM_IMEI_DATA) )
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


bool_t xdr_SIM_PLMN_ENTRY_UPDATE_t( XDR* xdrs, SIM_PLMN_ENTRY_UPDATE_t* data)
{
	XDR_LOG(xdrs,"SIM_PLMN_ENTRY_UPDATE_t")

	if( XDR_ENUM(xdrs, &data->result, SIMAccess_t) &&
		XDR_ENUM(xdrs, &data->plmn_file, SIM_PLMN_FILE_t) &&
		_xdr_u_int16_t(xdrs, &data->index,"index") 
		)
		return(TRUE);
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

bool_t xdr_CAPI2_SendWritePLMNEntry_t( XDR* xdrs, CAPI2_SendWritePLMNEntry_t* data)
{
	XDR_LOG(xdrs,"CAPI2_SendWritePLMNEntry_t")

	if( XDR_ENUM(xdrs, &data->plmn_file, SIM_PLMN_FILE_t)&&
		_xdr_u_int16_t(xdrs, &data->index,"index") &&
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

bool_t xdr_CAPI2_SendWriteMulPLMNEntry_t( XDR* xdrs, CAPI2_SendWriteMulPLMNEntry_t* data)
{
	XDR_LOG(xdrs,"CAPI2_SendWriteMulPLMNEntry_t")

	if( XDR_ENUM(xdrs, &data->plmn_file, SIM_PLMN_FILE_t)&&
		_xdr_u_int16_t(xdrs, &data->first_idx,"first_idx") &&
		_xdr_u_int16_t(xdrs, &data->number_of_entry,"number_of_entry") &&
		xdr_vector(xdrs,  (char*)data->plmn_entry, MAX_NUM_OF_PLMN, sizeof(SIM_MUL_PLMN_ENTRY_t), (xdrproc_t)xdr_SIM_MUL_PLMN_ENTRY_t) 
		)
		return(TRUE);
	else
		return(FALSE);
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

bool_t xdr_CAPI2_SIM_SendGenericApduCmd_Req_t(XDR* xdrs, CAPI2_SIM_SendGenericApduCmd_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SIM_SendGenericApduCmd_Req_t")

	if(	_xdr_u_long(xdrs, &rsp->apdu_len,"apdu_len") &&
		xdr_bytes(xdrs, (char **)(void*) &rsp->apdu, (u_int *) &rsp->apdu_len, SIM_GENERIC_APDU_MAX_CMD_LEN) )
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SIM_FatalErrorInd_t(XDR* xdrs, CAPI2_SIM_FatalErrorInd_t *rsp)
{
	XDR_LOG(xdrs,"xdr_CAPI2_SIM_FatalErrorInd_t ")

	return(xdr_pointer(xdrs, (char **)(void*) &rsp->err, sizeof(SIM_FATAL_ERROR_t), (xdrproc_t) XDR_ENUM_DEF(SIM_FATAL_ERROR_t)) );

}


