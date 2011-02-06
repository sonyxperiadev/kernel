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
*   @file   capi2_stk_msg.h
*
*   @brief  This file defines the capi2 message structure and forward declares
*	the serialization/deserialization functions.
*
****************************************************************************/


#ifndef CAPI2_STK_MSG_H
#define CAPI2_STK_MSG_H

#include "xdr.h"
#include "capi2_stk_ds.h"

typedef struct
{
	SetupMenu_t* rsp;
} CAPI2_SetupMenu_t;

typedef struct
{
	SATK_EVENTS_t toEvent;
	UInt8 result1; 
	UInt8 result2;
	SATKString_t* inText;
	UInt8 menuID;
} CAPI2_SATKCmdResp_t;

typedef struct
{
	StkCmdRespond_t* req;
} CAPI2_StkCmdRespond_t;

typedef struct
{
	SATK_EventData_t* rsp;
} CAPI2_SATK_EventData_t;

typedef struct
{
    StkCallSetupFail_t* rsp;
}CAPI2_SATK_CallSetupFail_t;

typedef struct
{
    StkCallControlDisplay_t* rsp;
}CAPI2_SATK_CallControlDisplay_t;

typedef struct
{
	StkLangNotification_t* rsp;
} CAPI2_StkLangNotification_t;

typedef struct
{
    TypeOfNumber_t		ton;
	NumberPlanId_t		npi;
	char*				number;
	BearerCapability_t*	bc1;
	Subaddress_t*		subaddr_data;
	BearerCapability_t*	bc2;
	UInt8				bc_repeat_ind;
	Boolean				simtk_orig;
}CAPI2_SATK_SendCcSetupReq_t;

typedef struct
{
    StkCallControlSetupRsp_t *rsp;
}CAPI2_StkCallControlSetupRsp_t;

typedef struct
{
    UInt8 ton_npi; 
    UInt8 ss_len;
    UInt8 ss_data[STK_SS_DATA_LEN];
    Boolean simtk_orig;
}CAPI2_SATK_SendCcSsReq_t;

typedef struct
{
    StkCallControlSsRsp_t *rsp;
}CAPI2_StkCallControlSsRsp_t;

typedef struct
{
    UInt8 ussd_dcs;
    UInt8 ussd_len;
    UInt8 ussd_data[STK_SS_DATA_LEN];
    Boolean simtk_orig;
}CAPI2_SATK_SendCcUssdReq_t;

typedef struct
{
    StkCallControlUssdRsp_t *rsp;
}CAPI2_StkCallControlUssdRsp_t;

typedef struct
{
    UInt8	sca_toa;
	UInt8	sca_number_len;
	UInt8	sca_number[STK_SMS_ADDRESS_LEN];
	UInt8	dest_toa;
	UInt8	dest_number_len;
	UInt8	dest_number[STK_SMS_ADDRESS_LEN];
	Boolean	simtk_orig;
}CAPI2_SATK_SendCcSmsReq_t;

typedef struct
{
    StkCallControlSmsRsp_t *rsp;
}CAPI2_StkCallControlSmsRsp_t;

typedef struct
{
    RunIpAtReq_t *rsp;
}
CAPI2_StkRunIpAtReq_t;

/// Request to run AT command
typedef struct {
	UInt16	length;                     ///< Length of request
	UInt8	request[MAX_AT_REQUEST+2];  ///< Content of request
}RunAT_Request;

typedef struct
{
    RunAT_Request* rsp;
} CAPI2_StkRunAtReq_t;

typedef struct
{
    STKMenuSelectionResCode_t resCode;
} StkMenuSelectionRes_t;

typedef struct
{
    StkMenuSelectionRes_t *rsp;
}CAPI2_StkMenuSelectionRes_t;

__BEGIN_DECLS
XDR_ENUM_DECLARE(Unicode_t)
XDR_ENUM_DECLARE(SATK_EVENTS_t)
XDR_ENUM_DECLARE(SATK_ResultCode_t)
XDR_ENUM_DECLARE(SATKInKeyType_t)
XDR_ENUM_DECLARE(SATKToneType_t)
XDR_ENUM_DECLARE(SATKSSType_t)
XDR_ENUM_DECLARE(SATKCallType_t)
XDR_ENUM_DECLARE(STKRefreshType_t)
XDR_ENUM_DECLARE(SIM_LAUNCH_BROWSER_ACTION_t)
XDR_ENUM_DECLARE(SATKDataServiceType_t)
XDR_ENUM_DECLARE(StkCallSetupFailResult_t)
XDR_ENUM_DECLARE(StkCallControl_t)
XDR_ENUM_DECLARE(StkCallControlResult_t)
XDR_ENUM_DECLARE(TypeOfNumber_t)
XDR_ENUM_DECLARE(NumberPlanId_t)
XDR_ENUM_DECLARE(STKMenuSelectionResCode_t)

bool_t xdr_STKIconListId_t(XDR*, STKIconListId_t*);
bool_t xdr_SATKIcon_t(XDR*, SATKIcon_t*);
bool_t xdr_SATKString_t(XDR*, SATKString_t*);
bool_t xdr_SetupMenu_t(XDR*, SetupMenu_t*);
bool_t xdr_CAPI2_SetupMenu_t(XDR*, CAPI2_SetupMenu_t*);
bool_t xdr_CAPI2_SATKCmdResp_t(XDR*, CAPI2_SATKCmdResp_t*);
bool_t xdr_StkCmdRespond_t(XDR*, StkCmdRespond_t*);
bool_t xdr_CAPI2_StkCmdRespond_t(XDR*, CAPI2_StkCmdRespond_t*);
bool_t xdr_SATKNum_t(XDR*, SATKNum_t*);
bool_t xdr_REFRESH_FILE_t(XDR*, REFRESH_FILE_t*);
bool_t xdr_ProvisionFile_t(XDR*, ProvisionFile_t*);
bool_t xdr_STK_SMS_DATA_t(XDR*, STK_SMS_DATA_t*);
bool_t xdr_DisplayText_t(XDR*, DisplayText_t*);
bool_t xdr_GetInkey_t(XDR*, GetInkey_t*);
bool_t xdr_GetInput_t(XDR*, GetInput_t*);
bool_t xdr_PlayTone_t(XDR*, PlayTone_t*);
bool_t xdr_SelectItem_t(XDR*, SelectItem_t*);
bool_t xdr_SendSs_t(XDR*, SendSs_t*);
bool_t xdr_SendUssd_t(XDR*, SendUssd_t*);
bool_t xdr_SetupCall_t(XDR*, SetupCall_t*);
bool_t xdr_Refresh_t(XDR*, Refresh_t*);
bool_t xdr_SendMOSMS_t(XDR*, SendMOSMS_t*);
bool_t xdr_SATK_EventData_t(XDR*, SATK_EventData_t*);
bool_t xdr_SendMOSMS_t(XDR*, SendMOSMS_t*);
bool_t xdr_SendStkDtmf_t(XDR*, SendStkDtmf_t*);
bool_t xdr_LaunchBrowserReq_t(XDR*, LaunchBrowserReq_t*);
bool_t xdr_IdleModeText_t(XDR*, IdleModeText_t*);
bool_t xdr_StkDataService_t(XDR*, StkDataService_t*);
bool_t xdr_CAPI2_SATK_EventData_t(XDR*, CAPI2_SATK_EventData_t*);
bool_t xdr_SATK_CallSetupFail_t(XDR*, StkCallSetupFail_t*);
bool_t xdr_CAPI2_SATK_CallSetupFail_t(XDR*, CAPI2_SATK_CallSetupFail_t*);
bool_t xdr_SATK_CallControlDisplay_t(XDR*, StkCallControlDisplay_t*);
bool_t xdr_CAPI2_SATK_CallControlDisplay_t(XDR*, CAPI2_SATK_CallControlDisplay_t*);
bool_t xdr_CAPI2_TermProfile_t(XDR*, CAPI2_TermProfile_t*);
bool_t xdr_StkLangNotification_t(XDR* , StkLangNotification_t*);
bool_t xdr_CAPI2_StkLangNotification_t(XDR*, CAPI2_StkLangNotification_t*);
bool_t xdr_StkRunAtCmd_t(XDR*, StkRunAtCmd_t*);
bool_t xdr_CAPI2_SATK_SendCcSetupReq_t(XDR* xdrs, CAPI2_SATK_SendCcSetupReq_t* req);
bool_t xdr_CAPI2_SATK_SendCcSsReq_t(XDR* xdrs, CAPI2_SATK_SendCcSsReq_t* req);
bool_t xdr_CAPI2_SATK_SendCcUssdReq_t(XDR* xdrs, CAPI2_SATK_SendCcUssdReq_t* req);
bool_t xdr_CAPI2_SATK_SendCcSmsReq_t(XDR* xdrs, CAPI2_SATK_SendCcSmsReq_t* req);
bool_t xdr_CAPI2_StkCallControlSetupRsp_t(XDR* xdrs, CAPI2_StkCallControlSetupRsp_t* rsp);
bool_t xdr_CAPI2_StkCallControlSsRsp_t(XDR* xdrs, CAPI2_StkCallControlSsRsp_t* rsp);
bool_t xdr_CAPI2_StkCallControlUssdRsp_t(XDR* xdrs, CAPI2_StkCallControlUssdRsp_t* rsp);
bool_t xdr_CAPI2_StkCallControlSmsRsp_t(XDR* xdrs, CAPI2_StkCallControlSmsRsp_t* rsp);
bool_t xdr_CAPI2_StkRunIpAtReq_t(XDR* xdrs, CAPI2_StkRunIpAtReq_t* rsp);
bool_t xdr_Subaddress_t(XDR* xdrs, Subaddress_t *subAddr);
bool_t xdr_CAPI2_StkRunAtReq_t(XDR*, CAPI2_StkRunAtReq_t*);
bool_t xdr_CAPI2_StkMenuSelectionRes_t(XDR* xdrs, CAPI2_StkMenuSelectionRes_t* res);
__END_DECLS

#endif

