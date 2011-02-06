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

#include "xdr.h"
#include "capi2_sim_api.h"

typedef struct
{
	SIM_PBK_INFO_t* rsp;
}CAPI2_SIM_PBK_INFO_t;

typedef struct
{
	UInt8 socket_id;
	UInt8 *aid_data;
	UInt8 aid_len;
} CAPI2_SimSelectAppli_t;

typedef struct
{
	UInt8 socket_id;
} CAPI2_SimDeactivateAppli_t;

typedef struct
{
	UInt8 socket_id;
} CAPI2_SimCloseSocket_t;

typedef struct
{
	APDUFileID_t efile_id;
	APDUFileID_t dfile_id;
	UInt8		*ptr;
	UInt16		ptr_len; 
	UInt16		length; 
	UInt8		path_len; 
	UInt16		offset;
	UInt8		rec_no;
	UInt16		*select_path;
	UInt8		socket_id;
}CAPI2_SimFileInfo_t;

typedef struct
{
	PLMN_ID_t *plmn;
	SIMPLMNAction_t action;
}CAPI2_SIM_SendUpdatePrefListReq_t;

typedef struct
{
	CurrencyName_t currency;
	EPPU_t *eppu;
}CAPI2_SIM_SendWritePuctReq_t;

typedef struct
{
	UInt8 *data;
	UInt16 data_length;
}CAPI2_SIM_SendGenericAccessReq_t;

typedef struct
{
	APDUCmd_t command;
	APDUFileID_t efile_id;				
	APDUFileID_t dfile_id;				
	UInt8 p1;				
	UInt8 p2;							
	UInt8 p3;							
	UInt8 path_len;
	UInt16 *select_path;
	UInt8 *data;
	UInt8  socket_id;
}CAPI2_SIM_SendRestrictedAccessReq_t;

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

typedef struct
{
	SIM_PLMN_FILE_t plmn_file;
	UInt16 index;
	UInt16 mcc;
	UInt8 mnc;
	Boolean gsm_act_selected;
	Boolean gsm_compact_act_selected;
	Boolean utra_act_selected;
}CAPI2_SendWritePLMNEntry_t;

typedef struct
{
	SIM_PLMN_FILE_t plmn_file;
	UInt16 first_idx;
	UInt16 number_of_entry;
	SIM_MUL_PLMN_ENTRY_t plmn_entry[MAX_NUM_OF_PLMN];	
}CAPI2_SendWriteMulPLMNEntry_t;

typedef struct
{
	UInt8 * apdu;
	UInt32  apdu_len;
}CAPI2_SIM_SendGenericApduCmd_Req_t;

typedef struct
{
	SIM_FATAL_ERROR_t *err;
	
}CAPI2_SIM_FatalErrorInd_t;

__BEGIN_DECLS

XDR_ENUM_DECLARE(SIMAccess_t)
XDR_ENUM_DECLARE(SIMSMSMesgStatus_t)
XDR_ENUM_DECLARE(SIM_MWI_TYPE_t)
XDR_ENUM_DECLARE(CHV_t)

XDR_ENUM_DECLARE(SIMLockType_t)
XDR_ENUM_DECLARE(SIM_FATAL_ERROR_t)

XDR_STRUCT_DECLARE(SIM_PHASE_RESULT_t)
XDR_STRUCT_DECLARE(SIM_TYPE_RESULT_t)
XDR_STRUCT_DECLARE(SIM_PRESENT_RESULT_t)
XDR_STRUCT_DECLARE(SIM_ACCESS_RESULT_t)
XDR_STRUCT_DECLARE(SIM_PBK_INFO_t)
XDR_STRUCT_DECLARE(SIM_MAX_ACM_t)
XDR_STRUCT_DECLARE(SIM_ACM_VALUE_t)
XDR_STRUCT_DECLARE(SIM_SVC_PROV_NAME_t)
XDR_STRUCT_DECLARE(SIM_PUCT_DATA_t)
XDR_STRUCT_DECLARE(SIM_SERVICE_STATUS_RESULT_t)
XDR_STRUCT_DECLARE(SIM_IMSI_RESULT_t)
XDR_STRUCT_DECLARE(SIM_GID_RESULT_t)
XDR_STRUCT_DECLARE(SIM_HOME_PLMN_RESULT_t)
XDR_STRUCT_DECLARE(SIM_PIN_STATUS_RESULT_t)
XDR_STRUCT_DECLARE(CAPI2_SIMLockSetLock_t)

XDR_STRUCT_DECLARE(SIM_OPEN_SOCKET_RES_t)
XDR_STRUCT_DECLARE(SIM_SELECT_APPLI_RES_t)
XDR_STRUCT_DECLARE(SIM_DEACTIVATE_APPLI_RES_t)
XDR_STRUCT_DECLARE(SIM_CLOSE_SOCKET_RES_t)
XDR_STRUCT_DECLARE(SIM_DFILE_INFO_t)
XDR_STRUCT_DECLARE(SIM_EFILE_INFO_t)
XDR_STRUCT_DECLARE(SIM_EFILE_DATA_t)
XDR_STRUCT_DECLARE(SIM_EFILE_UPDATE_RESULT_t)
XDR_STRUCT_DECLARE(SIM_SEEK_RECORD_DATA_t)
XDR_STRUCT_DECLARE(SIM_GEN_REC_DATA_t)
XDR_STRUCT_DECLARE(PIN_ATTEMPT_RESULT_t)
XDR_STRUCT_DECLARE(SIM_SERVICE_FLAG_STATUS_t)
XDR_STRUCT_DECLARE(SIM_INTEGER_DATA_t)
XDR_STRUCT_DECLARE(SIM_SCA_DATA_RESULT_t)
XDR_STRUCT_DECLARE(SIM_ICCID_STATUS_t)
XDR_STRUCT_DECLARE(APDU_t)
XDR_STRUCT_DECLARE(SIM_ATR_DATA_t)
XDR_STRUCT_DECLARE(SIM_CALL_FORWARD_UNCONDITIONAL_FLAG_t)
XDR_STRUCT_DECLARE(SIM_APPL_TYPE_RESULT_t)
XDR_STRUCT_DECLARE(USIM_UST_DATA_RSP_t)
XDR_STRUCT_DECLARE(CHVString_t)
XDR_STRUCT_DECLARE(PUKString_t)
XDR_STRUCT_DECLARE(SIM_BOOLEAN_DATA_t)
XDR_STRUCT_DECLARE(CAPI2_SIM_PBK_INFO_t)
XDR_STRUCT_DECLARE(SIM_CALL_FORWARD_UNCONDITIONAL_STATUS_t)
XDR_STRUCT_DECLARE(SIM_APDU_FILEID_RESULT_t)
XDR_STRUCT_DECLARE(CAPI2_SimFileInfo_t)
XDR_STRUCT_DECLARE(CAPI2_SimSelectAppli_t)
XDR_STRUCT_DECLARE(CAPI2_SimDeactivateAppli_t)
XDR_STRUCT_DECLARE(CAPI2_SimCloseSocket_t)

XDR_STRUCT_DECLARE(CAPI2_SIM_SendUpdatePrefListReq_t)
XDR_STRUCT_DECLARE(CAPI2_SIM_SendWritePuctReq_t)
XDR_STRUCT_DECLARE(CAPI2_SIM_SendGenericAccessReq_t)
XDR_STRUCT_DECLARE(CAPI2_SIM_SendRestrictedAccessReq_t)
XDR_STRUCT_DECLARE(SIM_RESTRICTED_ACCESS_DATA_t)
XDR_STRUCT_DECLARE(SIM_GENERIC_ACCESS_DATA_t)

XDR_STRUCT_DECLARE(SIM_LOCK_ON_RESULT_t)
XDR_STRUCT_DECLARE(SIM_LOCK_RESULT_t)
XDR_STRUCT_DECLARE(SIM_LOCK_TYPE_RESULT_t)
XDR_STRUCT_DECLARE(SIM_LOCK_SIG_DATA_t)
XDR_STRUCT_DECLARE(SIM_LOCK_IMEI_DATA_t)
XDR_STRUCT_DECLARE(CAPI2_SendWritePLMNEntry_t)
XDR_STRUCT_DECLARE(CAPI2_SendWriteMulPLMNEntry_t)
XDR_STRUCT_DECLARE(CAPI2_SIM_SendGenericApduCmd_Req_t)

XDR_STRUCT_DECLARE(SIM_PLMN_ENTRY_DATA_t)
XDR_STRUCT_DECLARE(SIM_PLMN_ENTRY_UPDATE_t)
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

bool_t xdr_CallMeterUnit_t(XDR*, CallMeterUnit_t*);


bool_t xdr_SIM_SMS_UPDATE_RESULT_t(XDR*, SIM_SMS_UPDATE_RESULT_t*);
bool_t xdr_SIM_SMS_DATA_t( XDR*, SIM_SMS_DATA_t* );
bool_t xdr_SIMSMSMesg_t( XDR*, SIMSMSMesg_t* );
bool_t xdr_xdr_string_t(XDR *xdrs, xdr_string_t* str);

bool_t xdr_USIM_FILE_UPDATE_RSP_t(XDR*, USIM_FILE_UPDATE_RSP_t*);
bool_t xdr_SIMLOCK_SIM_DATA_t(XDR* xdrs, SIMLOCK_SIM_DATA_t *sim_data);
bool_t xdr_SIMLOCK_STATE_t(XDR* xdrs, SIMLOCK_STATE_t *simlock_state);
bool_t xdr_SIM_DETECTION_t(XDR* xdrs, SIM_DETECTION_t *sim_detection);
bool_t xdr_SIM_2G_LANGUAGE_t(XDR* xdrs, SIM_2G_LANGUAGE_t *sim_2g_language);
bool_t xdr_SIM_3G_LANGUAGE_t(XDR* xdrs, SIM_3G_LANGUAGE_t *sim_3g_language);

bool_t xdr_SIM_GENERIC_APDU_RES_INFO_t(XDR* xdrs, SIM_GENERIC_APDU_RES_INFO_t* data);
bool_t xdr_SIM_GENERIC_APDU_ATR_INFO_t(XDR* xdrs, SIM_GENERIC_APDU_ATR_INFO_t* info);
bool_t xdr_SIM_GENERIC_APDU_XFER_RSP_t(XDR* xdrs, SIM_GENERIC_APDU_XFER_RSP_t* info);
bool_t xdr_CAPI2_SIM_FatalErrorInd_t(XDR* xdrs, CAPI2_SIM_FatalErrorInd_t *rsp);

__END_DECLS

#endif

