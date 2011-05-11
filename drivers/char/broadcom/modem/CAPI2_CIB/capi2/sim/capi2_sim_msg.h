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
*   @file   capi2_sim_msg.h
*
*   @brief  This file defines the capi2 message structure and forward declares
*	the serialization/deserialization functions.
*
****************************************************************************/


#ifndef CAPI2_SIM_MSG_H
#define CAPI2_SIM_MSG_H

typedef struct
{
	UInt8 action;
	Boolean ph_sim_full_lock_on;
	SIMLockType_t lockType;
	UInt8* key;
	UInt8* imsi; 
	UInt8* gid1; 
	UInt8* gid2;
}CAPI2_SIMLockSetLock_t;



__BEGIN_DECLS

XDR_ENUM_DECLARE(SIMAccess_t)
XDR_ENUM_DECLARE(SIMSMSMesgStatus_t)
XDR_ENUM_DECLARE(SIM_MWI_TYPE_t)
XDR_ENUM_DECLARE(CHV_t)

XDR_ENUM_DECLARE(SIMLockType_t)

XDR_STRUCT_DECLARE(SIM_PBK_INFO_t)
XDR_STRUCT_DECLARE(SIM_MAX_ACM_t)
XDR_STRUCT_DECLARE(SIM_ACM_VALUE_t)
XDR_STRUCT_DECLARE(SIM_SVC_PROV_NAME_t)
XDR_STRUCT_DECLARE(SIM_PUCT_DATA_t)
XDR_STRUCT_DECLARE(CAPI2_SIMLockSetLock_t)

XDR_STRUCT_DECLARE(SIM_OPEN_SOCKET_RES_t)
XDR_STRUCT_DECLARE(SIM_SELECT_APPLI_RES_t)
XDR_STRUCT_DECLARE(SIM_SELECT_APPLICATION_RES_t)	
XDR_STRUCT_DECLARE(SIM_DEACTIVATE_APPLI_RES_t)
XDR_STRUCT_DECLARE(SIM_CLOSE_SOCKET_RES_t)
XDR_STRUCT_DECLARE(SIM_DFILE_INFO_t)
XDR_STRUCT_DECLARE(SIM_EFILE_INFO_t)
XDR_STRUCT_DECLARE(SIM_EFILE_DATA_t)
XDR_STRUCT_DECLARE(SIM_EFILE_UPDATE_RESULT_t)
XDR_STRUCT_DECLARE(SIM_SEEK_RECORD_DATA_t)
XDR_STRUCT_DECLARE(PIN_ATTEMPT_RESULT_t)
XDR_STRUCT_DECLARE(SIM_SERVICE_FLAG_STATUS_t)
XDR_STRUCT_DECLARE(SIM_ICCID_STATUS_t)
XDR_STRUCT_DECLARE(APDU_t)
XDR_STRUCT_DECLARE(CHVString_t)
XDR_STRUCT_DECLARE(PUKString_t)
XDR_STRUCT_DECLARE(SIM_CALL_FORWARD_UNCONDITIONAL_FLAG_t)

XDR_STRUCT_DECLARE(SIM_RESTRICTED_ACCESS_DATA_t)

XDR_STRUCT_DECLARE(SIM_PLMN_ENTRY_DATA_t)
XDR_STRUCT_DECLARE(SIM_MUL_PLMN_ENTRY_UPDATE_t)
XDR_STRUCT_DECLARE(SIM_PLMN_NUM_OF_ENTRY_t)
XDR_STRUCT_DECLARE(APN_NAME_t)
XDR_STRUCT_DECLARE(SIM_GENERIC_APDU_EVENT_t)
XDR_STRUCT_DECLARE(SIM_GENERIC_APDU_RESULT_t)
XDR_STRUCT_DECLARE(SIM_GENERIC_APDU_RES_INFO_t)
XDR_STRUCT_DECLARE(SIM_GENERIC_APDU_ATR_INFO_t)
XDR_STRUCT_DECLARE(SIM_GENERIC_APDU_XFER_RSP_t)
XDR_STRUCT_DECLARE(SIM_GENERIC_APDU_CMD_BUF_t)

XDR_ENUM_DECLARE(SIMOperState_t)
XDR_ENUM_DECLARE(SIMPBK_ID_t)
XDR_ENUM_DECLARE(SIMService_t)
XDR_ENUM_DECLARE(APDUFileID_t)
XDR_ENUM_DECLARE(SERVICE_CODE_STATUS_CPHS_t)
XDR_ENUM_DECLARE(CPHS_SST_ENTRY_t)
XDR_ENUM_DECLARE(SIM_PLMN_FILE_t)
XDR_ENUM_DECLARE(USIM_RAT_MODE_t)
XDR_ENUM_DECLARE(USIM_APPLICATION_TYPE)
XDR_ENUM_DECLARE(SIMBdnOperState_t)
XDR_ENUM_DECLARE(SIM_POWER_ON_MODE_t)
XDR_ENUM_DECLARE(SIM_GENERIC_APDU_EVENT_t)
XDR_ENUM_DECLARE(SIM_GENERIC_APDU_RESULT_t)
XDR_ENUM_DECLARE(SIM_FATAL_ERROR_t)
XDR_ENUM_DECLARE(SIM_SIM_INTERFACE_t)

XDR_ENUM_DECLARE(SIMPhase_t)
XDR_ENUM_DECLARE(SIMType_t)
XDR_ENUM_DECLARE(SIMPresent_t)
XDR_ENUM_DECLARE(SIM_PIN_Status_t)
XDR_ENUM_DECLARE(SERVICE_FLAG_STATUS_t)
XDR_STRUCT_DECLARE(SIM_APPL_TYPE_t)
XDR_STRUCT_DECLARE(CurrencyName_t)
XDR_STRUCT_DECLARE(EPPU_t)

XDR_STRUCT_DECLARE(IMSI_t)
XDR_STRUCT_DECLARE(GID_DIGIT_t)
XDR_STRUCT_DECLARE(SIM_SCA_DATA_t)
XDR_STRUCT_DECLARE(ICCID_ASCII_t)
XDR_STRUCT_DECLARE(APDUCmd_t)
XDR_ENUM_DECLARE(SIM_APP_OCCURRENCE_t)

bool_t xdr_CallMeterUnit_t(XDR*, CallMeterUnit_t*);


bool_t xdr_SIM_SMS_UPDATE_RESULT_t(XDR*, SIM_SMS_UPDATE_RESULT_t*);
bool_t xdr_SIM_SMS_DATA_t( XDR*, SIM_SMS_DATA_t* );
bool_t xdr_SIMSMSMesg_t( XDR*, SIMSMSMesg_t* );
bool_t xdr_xdr_string_t(XDR *xdrs, xdr_string_t* str);

bool_t xdr_USIM_FILE_UPDATE_RSP_t(XDR*, USIM_FILE_UPDATE_RSP_t*);
bool_t xdr_SIM_GENERIC_APDU_RES_INFO_t(XDR* xdrs, SIM_GENERIC_APDU_RES_INFO_t* data);
bool_t xdr_SIM_GENERIC_APDU_ATR_INFO_t(XDR* xdrs, SIM_GENERIC_APDU_ATR_INFO_t* info);
bool_t xdr_SIM_GENERIC_APDU_XFER_RSP_t(XDR* xdrs, SIM_GENERIC_APDU_XFER_RSP_t* info);
bool_t xdr_SIMLOCK_SIM_DATA_t(XDR* xdrs, SIMLOCK_SIM_DATA_t *sim_data);
bool_t xdr_SIMLOCK_STATE_t(XDR* xdrs, SIMLOCK_STATE_t *simlock_state);
bool_t xdr_SIM_DETECTION_t(XDR* xdrs, SIM_DETECTION_t *sim_detection);
bool_t xdr_SIM_2G_LANGUAGE_t(XDR* xdrs, SIM_2G_LANGUAGE_t *sim_2g_language);
bool_t xdr_SIM_3G_LANGUAGE_t(XDR* xdrs, SIM_3G_LANGUAGE_t *sim_3g_language);

bool_t xdr_PLMNId_t( XDR* xdrs, PLMNId_t* data);

bool_t xdr_SimApduRsp_t( XDR* xdrs, SimApduRsp_t* data);

bool_t xdr_SIM_MUL_PLMN_ENTRY_t( XDR* xdrs, SIM_MUL_PLMN_ENTRY_t* data);

bool_t xdr_SIM_MUL_REC_DATA_t( XDR* xdrs, SIM_MUL_REC_DATA_t* data);

bool_t xdr_USIM_AID_DATA_t( XDR* xdrs, USIM_AID_DATA_t* data);

bool_t xdr_UST_DATA_t( XDR* xdrs, UST_DATA_t* data);

bool_t xdr_PinStatusWord_t( XDR* xdrs, PinStatusWord_t* data);

bool_t xdr_SimPinRsp_t( XDR* xdrs, SimPinRsp_t* data);


__END_DECLS


#endif

