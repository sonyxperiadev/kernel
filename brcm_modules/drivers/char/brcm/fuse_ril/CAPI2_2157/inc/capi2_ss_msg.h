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
*   @file   capi2_ss_msg.h
*
*   @brief  This file defines the capi2 message structure and forward declares
*	the serialization/deserialization functions.
*
****************************************************************************/


#ifndef CAPI2_SS_MSG_H
#define CAPI2_SS_MSG_H

#include "xdr.h"
#include "capi2_reqrep.h"
#include "capi2_ss_ds.h"
#include "capi2_phonectrl_api.h"

typedef struct
{

	SS_Mode_t mode;
	SS_CallFwdReason_t reason;
	SS_SvcCls_t svcCls;
	UInt8 waitToFwdSec;
	xdr_string_t number;

} CAPI2_SS_SendCallForwardReq_t;

typedef struct
{

	SS_CallFwdReason_t reason;
	SS_SvcCls_t svcCls;

} CAPI2_SS_QueryCallForwardStatus_t;

typedef struct
{

	SS_Mode_t mode;
	SS_CallBarType_t callBarType;
	SS_SvcCls_t svcCls;
	xdr_string_t password;

} CAPI2_SS_SendCallBarringReq_t;

typedef struct
{

	SS_CallBarType_t callBarType;
	SS_SvcCls_t svcCls;

} CAPI2_SS_QueryCallBarringStatus_t;

typedef struct
{

	SS_CallBarType_t callBarType;
	xdr_string_t oldPwd;
	xdr_string_t newPwd;
	xdr_string_t reNewPwd;

} CAPI2_SS_SendCallBarringPWDChangeReq_t;

typedef struct
{

	SS_Mode_t mode;
	SS_SvcCls_t svcCls;

} CAPI2_SS_SendCallWaitingReq_t;

typedef struct
{

	SS_SvcCls_t svcCls;

} CAPI2_SS_QueryCallWaitingStatus_t;

typedef struct
{

	Boolean enable;

} CAPI2_SS_SetCallingLineIDStatus_t;

typedef struct
{

	CLIRMode_t clir_mode;

} CAPI2_SS_SetCallingLineRestrictionStatus_t;

typedef struct
{

	Boolean enable;

} CAPI2_SS_SetConnectedLineIDStatus_t;

typedef struct
{

	Boolean enable;

} CAPI2_SS_SetConnectedLineRestrictionStatus_t;

typedef struct
{

	USSDString_t* ussd;

} CAPI2_SS_SendUSSDConnectReq_t;

typedef struct
{

	CallIndex_t ussd_id;

} CAPI2_SS_EndUSSDConnectReq_t;

typedef struct
{

	CallIndex_t ussd_id;
	UInt8 dcs;
	UInt8 len;
	UInt8* ussdString;

} CAPI2_SS_SendUSSDData_t;

typedef struct
{

	ClientInfo_t *inClientInfoPtr;
	SsApi_DialStrSrvReq_t *inDialStrSrvReqPtr;

} CAPI2_SsApi_DialStrSrvReq_t;

typedef struct
{

	ClientInfo_t *inClientInfoPtr;
	SsApi_SrvReq_t *inApiSrvReqPtr;

} CAPI2_SsApi_SsSrvReq_t;

typedef struct
{

	ClientInfo_t *inClientInfoPtr;
	SsApi_UssdSrvReq_t *inUssdSrvReqPtr;

} CAPI2_SsApi_UssdSrvReq_t;

typedef struct
{

	ClientInfo_t *inClientInfoPtr;
	SsApi_UssdDataReq_t *inUssdDataReqPtr;

} CAPI2_SsApi_UssdDataReq_t;

typedef struct
{

	ClientInfo_t *inClientInfoPtr;
	SsApi_SrvReq_t *inApiSrvReqPtr;

} CAPI2_SsApi_SsReleaseReq_t;

typedef struct
{

	ClientInfo_t *inClientInfoPtr;
	SsApi_DataReq_t *inDataReqPtr;

} CAPI2_SsApi_DataReq_t;

typedef struct
{
	NetworkCause_t* SS_CALL_FORWARD_RSP_Rsp;
} CAPI2_SS_CALL_FORWARD_RSP_t;
typedef struct
{
	CallForwardStatus_t* SS_CALL_FORWARD_STATUS_RSP_Rsp;
} CAPI2_SS_CALL_FORWARD_STATUS_RSP_t;
typedef struct
{
	NetworkCause_t* SS_CALL_BARRING_RSP_Rsp;
} CAPI2_SS_CALL_BARRING_RSP_t;
typedef struct
{
	CallBarringStatus_t* SS_CALL_BARRING_STATUS_RSP_Rsp;
} CAPI2_SS_CALL_BARRING_STATUS_RSP_t;
typedef struct
{
	NetworkCause_t* SS_CALL_BARRING_PWD_CHANGE_RSP_Rsp;
} CAPI2_SS_CALL_BARRING_PWD_CHANGE_RSP_t;
typedef struct
{
	SS_ProvisionStatus_t* SS_CALLING_LINE_ID_STATUS_RSP_Rsp;
} CAPI2_SS_CALLING_LINE_ID_STATUS_RSP_t;
typedef struct
{
	NetworkCause_t* SS_CALL_WAITING_RSP_Rsp;
} CAPI2_SS_CALL_WAITING_RSP_t;
typedef struct
{
	SS_ActivationStatus_t* SS_CALL_WAITING_STATUS_RSP_Rsp;
} CAPI2_SS_CALL_WAITING_STATUS_RSP_t;
typedef struct
{
	SS_ProvisionStatus_t* SS_CONNECTED_LINE_STATUS_RSP_Rsp;
} CAPI2_SS_CONNECTED_LINE_STATUS_RSP_t;
typedef struct
{
	SS_ProvisionStatus_t* SS_CALLING_LINE_RESTRICTION_STATUS_RSP_Rsp;
} CAPI2_SS_CALLING_LINE_RESTRICTION_STATUS_RSP_t;
typedef struct
{
	SS_ProvisionStatus_t* SS_CONNECTED_LINE_RESTRICTION_STATUS_RSP_Rsp;
} CAPI2_SS_CONNECTED_LINE_RESTRICTION_STATUS_RSP_t;
typedef struct
{
	SS_ProvisionStatus_t* SS_CALLING_NAME_PRESENT_STATUS_RSP_Rsp;
} CAPI2_SS_CALLING_NAME_PRESENT_STATUS_RSP_t;
typedef struct
{
	SsCallReqFail_t* SS_CALL_REQ_FAIL_Rsp;
} CAPI2_SS_CALL_REQ_FAIL_t;
typedef struct
{
	USSDataInfo_t* USSD_DATA_RSP_Rsp;
} CAPI2_USSD_DATA_RSP_t;
typedef struct
{
	USSDataInfo_t* USSD_DATA_IND_Rsp;
} CAPI2_USSD_DATA_IND_t;
typedef struct
{
	CallIndex_t* USSD_SESSION_END_IND_Rsp;
} CAPI2_USSD_SESSION_END_IND_t;
typedef struct
{
	StkReportCallStatus_t* USSD_CALLINDEX_IND_Rsp;
} CAPI2_USSD_CALLINDEX_IND_t;
typedef struct
{
	SS_IntParSetInd_t* SS_INTERNAL_PARAM_SET_IND_Rsp;
} CAPI2_SS_INTERNAL_PARAM_SET_IND_t;

typedef struct
{
	MS_LocalElemNotifyInd_t* MS_LOCAL_ELEM_NOTIFY_IND_Rsp;
} CAPI2_MS_LOCAL_ELEM_NOTIFY_IND_t;

__BEGIN_DECLS
bool_t	xdr_MS_LocalElemNotifyInd_t(XDR *, MS_LocalElemNotifyInd_t *);
bool_t	xdr_CAPI2_MS_LOCAL_ELEM_NOTIFY_IND_t(XDR *, CAPI2_MS_LOCAL_ELEM_NOTIFY_IND_t *);
bool_t	xdr_CAPI2_SS_SendCallForwardReq_t(XDR *, CAPI2_SS_SendCallForwardReq_t *);
bool_t	xdr_CAPI2_SS_QueryCallForwardStatus_t(XDR *, CAPI2_SS_QueryCallForwardStatus_t *);
bool_t	xdr_CAPI2_SS_SendCallBarringReq_t(XDR *, CAPI2_SS_SendCallBarringReq_t *);
bool_t	xdr_CAPI2_SS_QueryCallBarringStatus_t(XDR *, CAPI2_SS_QueryCallBarringStatus_t *);
bool_t	xdr_CAPI2_SS_SendCallBarringPWDChangeReq_t(XDR *, CAPI2_SS_SendCallBarringPWDChangeReq_t *);
bool_t	xdr_CAPI2_SS_SendCallWaitingReq_t(XDR *, CAPI2_SS_SendCallWaitingReq_t *);
bool_t	xdr_CAPI2_SS_QueryCallWaitingStatus_t(XDR *, CAPI2_SS_QueryCallWaitingStatus_t *);
bool_t	xdr_CAPI2_SS_SetCallingLineIDStatus_t(XDR *, CAPI2_SS_SetCallingLineIDStatus_t *);
bool_t	xdr_CAPI2_SS_SetCallingLineRestrictionStatus_t(XDR *, CAPI2_SS_SetCallingLineRestrictionStatus_t *);
bool_t	xdr_CAPI2_SS_SetConnectedLineIDStatus_t(XDR *, CAPI2_SS_SetConnectedLineIDStatus_t *);
bool_t	xdr_CAPI2_SS_SetConnectedLineRestrictionStatus_t(XDR *, CAPI2_SS_SetConnectedLineRestrictionStatus_t *);
bool_t	xdr_CAPI2_SS_SendUSSDConnectReq_t(XDR *, CAPI2_SS_SendUSSDConnectReq_t *);
bool_t	xdr_CAPI2_SS_EndUSSDConnectReq_t(XDR *, CAPI2_SS_EndUSSDConnectReq_t *);
bool_t	xdr_CAPI2_SS_SendUSSDData_t(XDR *, CAPI2_SS_SendUSSDData_t *);
bool_t	xdr_USSDString_t(XDR *, USSDString_t *);

bool_t	xdr_SS_SrvReq_t(XDR *, SS_SrvReq_t *);
bool_t	xdr_SS_SrvRel_t(XDR *, SS_SrvRel_t *);
bool_t	xdr_LCS_SrvInd_t(XDR *, LCS_SrvInd_t *);
bool_t	xdr_SS_SrvRsp_t(XDR *, SS_SrvRsp_t *);
bool_t	xdr_SS_SrvInd_t(XDR *, SS_SrvInd_t *);
bool_t	xdr_STK_SsSrvRel_t(XDR *, STK_SsSrvRel_t *);
bool_t	xdr_SsApi_DataReq_t(XDR *, SsApi_DataReq_t *);

bool_t	xdr_SsApi_DialStrSrvReq_t(XDR *, SsApi_DialStrSrvReq_t *);
bool_t	xdr_SsApi_SrvReq_t(XDR *, SsApi_SrvReq_t *);
bool_t	xdr_SsApi_UssdSrvReq_t(XDR *, SsApi_UssdSrvReq_t *);
bool_t	xdr_SsApi_UssdDataReq_t(XDR *, SsApi_UssdDataReq_t *);

bool_t	xdr_CAPI2_SsApi_DialStrSrvReq_t(XDR *, CAPI2_SsApi_DialStrSrvReq_t *);
bool_t	xdr_CAPI2_SsApi_SsSrvReq_t(XDR *, CAPI2_SsApi_SsSrvReq_t *);
bool_t	xdr_CAPI2_SsApi_UssdSrvReq_t(XDR *, CAPI2_SsApi_UssdSrvReq_t *);
bool_t	xdr_CAPI2_SsApi_UssdDataReq_t(XDR *, CAPI2_SsApi_UssdDataReq_t *);
bool_t	xdr_CAPI2_SsApi_SsReleaseReq_t(XDR *, CAPI2_SsApi_SsReleaseReq_t *);
bool_t	xdr_CAPI2_SsApi_DataReq_t(XDR *, CAPI2_SsApi_DataReq_t *);

bool_t	xdr_CAPI2_SS_CALL_FORWARD_RSP_t(XDR *, CAPI2_SS_CALL_FORWARD_RSP_t *);
bool_t	xdr_CAPI2_SS_CALL_FORWARD_STATUS_RSP_t(XDR *, CAPI2_SS_CALL_FORWARD_STATUS_RSP_t *);
bool_t	xdr_CAPI2_SS_CALL_BARRING_RSP_t(XDR *, CAPI2_SS_CALL_BARRING_RSP_t *);
bool_t	xdr_CAPI2_SS_CALL_BARRING_STATUS_RSP_t(XDR *, CAPI2_SS_CALL_BARRING_STATUS_RSP_t *);
bool_t	xdr_CAPI2_SS_CALL_BARRING_PWD_CHANGE_RSP_t(XDR *, CAPI2_SS_CALL_BARRING_PWD_CHANGE_RSP_t *);
bool_t	xdr_CAPI2_SS_CALLING_LINE_ID_STATUS_RSP_t(XDR *, CAPI2_SS_CALLING_LINE_ID_STATUS_RSP_t *);
bool_t	xdr_CAPI2_SS_CALL_WAITING_RSP_t(XDR *, CAPI2_SS_CALL_WAITING_RSP_t *);
bool_t	xdr_CAPI2_SS_CALL_WAITING_STATUS_RSP_t(XDR *, CAPI2_SS_CALL_WAITING_STATUS_RSP_t *);
bool_t	xdr_CAPI2_SS_CONNECTED_LINE_STATUS_RSP_t(XDR *, CAPI2_SS_CONNECTED_LINE_STATUS_RSP_t *);
bool_t	xdr_CAPI2_SS_CALLING_LINE_RESTRICTION_STATUS_RSP_t(XDR *, CAPI2_SS_CALLING_LINE_RESTRICTION_STATUS_RSP_t *);
bool_t	xdr_CAPI2_SS_CONNECTED_LINE_RESTRICTION_STATUS_RSP_t(XDR *, CAPI2_SS_CONNECTED_LINE_RESTRICTION_STATUS_RSP_t *);
bool_t	xdr_CAPI2_SS_CALLING_NAME_PRESENT_STATUS_RSP_t(XDR *, CAPI2_SS_CALLING_NAME_PRESENT_STATUS_RSP_t *);
bool_t	xdr_CAPI2_SS_CALL_REQ_FAIL_t(XDR *, CAPI2_SS_CALL_REQ_FAIL_t *);
bool_t	xdr_CAPI2_USSD_DATA_RSP_t(XDR *, CAPI2_USSD_DATA_RSP_t *);
bool_t	xdr_CAPI2_USSD_DATA_IND_t(XDR *, CAPI2_USSD_DATA_IND_t *);
bool_t	xdr_CAPI2_USSD_SESSION_END_IND_t(XDR *, CAPI2_USSD_SESSION_END_IND_t *);
bool_t	xdr_CAPI2_USSD_CALLINDEX_IND_t(XDR *, CAPI2_USSD_CALLINDEX_IND_t *);
bool_t	xdr_CAPI2_SS_INTERNAL_PARAM_SET_IND_t(XDR *, CAPI2_SS_INTERNAL_PARAM_SET_IND_t *);
bool_t	xdr_CallForwardStatus_t(XDR *, CallForwardStatus_t *);
bool_t	xdr_CallBarringStatus_t(XDR *, CallBarringStatus_t *);
bool_t	xdr_SS_ProvisionStatus_t(XDR *, SS_ProvisionStatus_t *);
bool_t	xdr_SS_ActivationStatus_t(XDR *, SS_ActivationStatus_t *);
bool_t	xdr_USSDataInfo_t(XDR *, USSDataInfo_t *);
bool_t	xdr_USSDataInfo_t(XDR *, USSDataInfo_t *);
bool_t	xdr_CallIndex_t(XDR *, CallIndex_t *);
bool_t	xdr_SS_IntParSetInd_t(XDR *, SS_IntParSetInd_t *);
bool_t	xdr_ClientInfo_t(XDR *, ClientInfo_t *);
bool_t	xdr_LCS_SrvRsp_t(XDR *, LCS_SrvRsp_t *);


XDR_ENUM_DECLARE(SS_Mode_t)
XDR_ENUM_DECLARE(SS_CallFwdReason_t)
XDR_ENUM_DECLARE(SS_SvcCls_t)
XDR_ENUM_DECLARE(SS_CallBarType_t)
XDR_ENUM_DECLARE(CallIndex_t)
XDR_ENUM_DECLARE(NetworkCause_t)
XDR_ENUM_DECLARE(ServiceStatus_t)
XDR_ENUM_DECLARE(USSDService_t)
XDR_ENUM_DECLARE(SuppSvcStatus_t)
XDR_ENUM_DECLARE(Unicode_t)
XDR_ENUM_DECLARE(CallType_t)
XDR_ENUM_DECLARE(CallStatus_t)
XDR_ENUM_DECLARE(SuppSvcType_t)
XDR_ENUM_DECLARE(ConfigMode_t)
XDR_ENUM_DECLARE(SS_Operation_t)
XDR_ENUM_DECLARE(SS_ErrorCode_t)
__END_DECLS


#endif
