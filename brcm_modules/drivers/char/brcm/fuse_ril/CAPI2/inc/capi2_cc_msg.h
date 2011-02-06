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
*   @file   capi2_cc_msg.h
*
*   @brief  This file defines the capi2 message structure and forward declares
*	the serialization/deserialization functions.
*
****************************************************************************/
#ifndef CAPI2_CC_MSG_H
#define CAPI2_CC_MSG_H

#include "xdr.h"
#include "capi2_reqrep.h"
#include "capi2_cc_ds.h"
#include "capi2_mstruct.h"

typedef struct {
	xdr_string_t callNum;
	VoiceCallParam_t voiceCallParam;
} CAPI2_CC_MakeVoiceCall_t;

typedef struct {
	UInt8 callIndex;
} CAPI2_CC_CallIndex_t;

typedef struct {
	Boolean turn_off;
} CAPI2_CC_MuteCall_t;

typedef struct {
	Boolean enableAutoRej;
} CAPI2_CC_SetVoiceCallAutoReject_t;
typedef struct {
	Boolean enable;
} CAPI2_CC_SetTTYCall_t;

typedef struct
{
	VideoCallParam_t param;
} CAPI2_CC_SetVideoCallParam_t;

typedef struct
{
	CallConfigType_t type;
} CAPI2_CC_GetCallCfg_t;

typedef struct
{
	CAPI2_CallConfig_t* cfg;
} CAPI2_CC_SetCallCfg_t;
typedef struct
{

	CCallState_t inCcCallState;

} CAPI2_CC_GetCallIndexInThisState_t;

typedef struct
{

	ApiDtmf_t* inDtmfObjPtr;

} CAPI2_CcApi_SendDtmf_t;

typedef struct
{

	ApiDtmf_t* inDtmfObjPtr;

} CAPI2_CcApi_StopDtmf_t;

typedef struct
{

	ApiDtmf_t* inDtmfObjPtr;

} CAPI2_CcApi_AbortDtmf_t;

typedef struct
{

	UInt8 inCallIndex;
	DtmfTimer_t inDtmfTimerType;
	Ticks_t inDtmfTimeInterval;

} CAPI2_CcApi_SetDtmfTimer_t;

typedef struct
{

	UInt8 inCallIndex;
	DtmfTimer_t inDtmfTimerType;

} CAPI2_CcApi_ResetDtmfTimer_t;

typedef struct
{

	UInt8 inCallIndex;
	DtmfTimer_t inDtmfTimerType;

} CAPI2_CcApi_GetDtmfTimer_t;

typedef struct
{

	UInt8 inCallIndex;

} CAPI2_CC_GetCallPresent_t;


typedef struct
{
	VoiceCallConnectMsg_t* VOICECALL_CONNECTED_IND_Rsp;
} CAPI2_VOICECALL_CONNECTED_IND_t;
typedef struct
{
	VoiceCallPreConnectMsg_t* VOICECALL_PRECONNECT_IND_Rsp;
} CAPI2_VOICECALL_PRECONNECT_IND_t;
typedef struct
{
	SS_CallNotification_t* SS_CALL_NOTIFICATION_Rsp;
} CAPI2_SS_CALL_NOTIFICATION_t;
typedef struct
{
	CallStatusMsg_t* CALL_STATUS_IND_Rsp;
} CAPI2_CALL_STATUS_IND_t;
typedef struct
{
	VoiceCallActionMsg_t* VOICECALL_ACTION_RSP_Rsp;
} CAPI2_VOICECALL_ACTION_RSP_t;
typedef struct
{
	VoiceCallReleaseMsg_t* VOICECALL_RELEASE_IND_Rsp;
} CAPI2_VOICECALL_RELEASE_IND_t;
typedef struct
{
	CallReceiveMsg_t* INCOMING_CALL_IND_Rsp;
} CAPI2_INCOMING_CALL_IND_t;
typedef struct
{
	VoiceCallWaitingMsg_t* VOICECALL_WAITING_IND_Rsp;
} CAPI2_VOICECALL_WAITING_IND_t;
typedef struct
{
	CallAOCStatusMsg_t* CALL_AOCSTATUS_IND_Rsp;
} CAPI2_CALL_AOCSTATUS_IND_t;
typedef struct
{
	CallCCMMsg_t* CALL_CCM_IND_Rsp;
} CAPI2_CALL_CCM_IND_t;
typedef struct
{
	UInt8* CALL_CONNECTEDLINEID_IND_Rsp;
} CAPI2_CALL_CONNECTEDLINEID_IND_t;
typedef struct
{
	DataCallStatusMsg_t* DATACALL_STATUS_IND_Rsp;
} CAPI2_DATACALL_STATUS_IND_t;
typedef struct
{
	DataCallReleaseMsg_t* DATACALL_RELEASE_IND_Rsp;
} CAPI2_DATACALL_RELEASE_IND_t;
typedef struct
{
	DataECDCLinkMsg_t* DATACALL_ECDC_IND_Rsp;
} CAPI2_DATACALL_ECDC_IND_t;
typedef struct
{
	DataCallConnectMsg_t* DATACALL_CONNECTED_IND_Rsp;
} CAPI2_DATACALL_CONNECTED_IND_t;
typedef struct
{
	ApiClientCmdInd_t* API_CLIENT_CMD_IND_Rsp;
} CAPI2_API_CLIENT_CMD_IND_t;
typedef struct
{
	ApiDtmfStatus_t* DTMF_STATUS_IND_Rsp;
} CAPI2_DTMF_STATUS_IND_t;

typedef struct
{
	SS_UserInfo_t* USER_INFORMATION_Rsp;
} CAPI2_USER_INFORMATION_t;

__BEGIN_DECLS
bool_t	xdr_CAPI2_CC_MakeVoiceCall_t(XDR *, CAPI2_CC_MakeVoiceCall_t *);
bool_t	xdr_CAPI2_CC_CallIndex_t(XDR *, CAPI2_CC_CallIndex_t *);
bool_t	xdr_CAPI2_CC_MuteCall_t(XDR *, CAPI2_CC_MuteCall_t *);
bool_t	xdr_CAPI2_CC_SetVoiceCallAutoReject_t(XDR *, CAPI2_CC_SetVoiceCallAutoReject_t *);
bool_t	xdr_CAPI2_CC_SetTTYCall_t(XDR *, CAPI2_CC_SetTTYCall_t *);
bool_t	xdr_CAPI2_CC_SetVideoCallParam_t(XDR *, CAPI2_CC_SetVideoCallParam_t *);
bool_t	xdr_CAPI2_CcApi_SendDtmf_t(XDR *, CAPI2_CcApi_SendDtmf_t *);
bool_t	xdr_CAPI2_CcApi_StopDtmf_t(XDR *, CAPI2_CcApi_StopDtmf_t *);
bool_t	xdr_CAPI2_CcApi_AbortDtmf_t(XDR *, CAPI2_CcApi_AbortDtmf_t *);
bool_t	xdr_CAPI2_CcApi_SetDtmfTimer_t(XDR *, CAPI2_CcApi_SetDtmfTimer_t *);
bool_t	xdr_CAPI2_CcApi_ResetDtmfTimer_t(XDR *, CAPI2_CcApi_ResetDtmfTimer_t *);
bool_t	xdr_CAPI2_CcApi_GetDtmfTimer_t(XDR *, CAPI2_CcApi_GetDtmfTimer_t *);
bool_t	xdr_CAPI2_CC_GetCallPresent_t(XDR *, CAPI2_CC_GetCallPresent_t *);

bool_t	xdr_PHONE_NUMBER_STR_t(XDR *, PHONE_NUMBER_STR_t *);
bool_t     xdr_CUGInfo_t(XDR *, CUGInfo_t *);
bool_t     xdr_VoiceCallParam_t(XDR *, VoiceCallParam_t *);
bool_t     xdr_CNAP_NAME_t(XDR *, CNAP_NAME_t *);
bool_t     xdr_CallingName_t(XDR *, CallingName_t *);
bool_t     xdr_CallingInfo_t(XDR *, CallingInfo_t *);
bool_t	xdr_ALL_CALL_STATE_t(XDR *, ALL_CALL_STATE_t*); 
bool_t	xdr_ALL_CALL_INDEX_t(XDR *, ALL_CALL_INDEX_t*); 
bool_t  xdr_ApiDtmfStatus_t(XDR *xdrs, ApiDtmfStatus_t *args);
bool_t	xdr_ApiDtmfStatus_t(XDR *, ApiDtmfStatus_t *);
bool_t	xdr_BearerCapability_t(XDR *, BearerCapability_t *);
bool_t	xdr_CC_BearerCap_t(XDR *, CC_BearerCap_t *);

bool_t	xdr_CAPI2_VOICECALL_CONNECTED_IND_t(XDR *, CAPI2_VOICECALL_CONNECTED_IND_t *);
bool_t	xdr_CAPI2_VOICECALL_PRECONNECT_IND_t(XDR *, CAPI2_VOICECALL_PRECONNECT_IND_t *);
bool_t	xdr_CAPI2_SS_CALL_NOTIFICATION_t(XDR *, CAPI2_SS_CALL_NOTIFICATION_t *);
bool_t	xdr_CAPI2_CALL_STATUS_IND_t(XDR *, CAPI2_CALL_STATUS_IND_t *);
bool_t	xdr_CAPI2_VOICECALL_ACTION_RSP_t(XDR *, CAPI2_VOICECALL_ACTION_RSP_t *);
bool_t	xdr_CAPI2_VOICECALL_RELEASE_IND_t(XDR *, CAPI2_VOICECALL_RELEASE_IND_t *);
bool_t	xdr_CAPI2_INCOMING_CALL_IND_t(XDR *, CAPI2_INCOMING_CALL_IND_t *);
bool_t	xdr_CAPI2_VOICECALL_WAITING_IND_t(XDR *, CAPI2_VOICECALL_WAITING_IND_t *);
bool_t	xdr_CAPI2_CALL_AOCSTATUS_IND_t(XDR *, CAPI2_CALL_AOCSTATUS_IND_t *);
bool_t	xdr_CAPI2_CALL_CCM_IND_t(XDR *, CAPI2_CALL_CCM_IND_t *);
bool_t	xdr_CAPI2_CALL_CONNECTEDLINEID_IND_t(XDR *, CAPI2_CALL_CONNECTEDLINEID_IND_t *);
bool_t	xdr_CAPI2_DATACALL_STATUS_IND_t(XDR *, CAPI2_DATACALL_STATUS_IND_t *);
bool_t	xdr_CAPI2_DATACALL_RELEASE_IND_t(XDR *, CAPI2_DATACALL_RELEASE_IND_t *);
bool_t	xdr_CAPI2_DATACALL_ECDC_IND_t(XDR *, CAPI2_DATACALL_ECDC_IND_t *);
bool_t	xdr_CAPI2_DATACALL_CONNECTED_IND_t(XDR *, CAPI2_DATACALL_CONNECTED_IND_t *);
bool_t	xdr_CAPI2_API_CLIENT_CMD_IND_t(XDR *, CAPI2_API_CLIENT_CMD_IND_t *);
bool_t	xdr_CAPI2_DTMF_STATUS_IND_t(XDR *, CAPI2_DTMF_STATUS_IND_t *);
bool_t	xdr_VoiceCallConnectMsg_t(XDR *, VoiceCallConnectMsg_t *);
bool_t	xdr_VoiceCallPreConnectMsg_t(XDR *, VoiceCallPreConnectMsg_t *);
bool_t	xdr_SS_CallNotification_t(XDR *, SS_CallNotification_t *);
bool_t	xdr_CallStatusMsg_t(XDR *, CallStatusMsg_t *);
bool_t	xdr_VoiceCallActionMsg_t(XDR *, VoiceCallActionMsg_t *);
bool_t	xdr_VoiceCallReleaseMsg_t(XDR *, VoiceCallReleaseMsg_t *);
bool_t	xdr_CallReceiveMsg_t(XDR *, CallReceiveMsg_t *);
bool_t	xdr_VoiceCallWaitingMsg_t(XDR *, VoiceCallWaitingMsg_t *);
bool_t	xdr_CallAOCStatusMsg_t(XDR *, CallAOCStatusMsg_t *);
bool_t	xdr_CallCCMMsg_t(XDR *, CallCCMMsg_t *);
bool_t	xdr_UInt8(XDR *, UInt8 *);
bool_t	xdr_DataCallStatusMsg_t(XDR *, DataCallStatusMsg_t *);
bool_t	xdr_DataCallReleaseMsg_t(XDR *, DataCallReleaseMsg_t *);
bool_t	xdr_DataECDCLinkMsg_t(XDR *, DataECDCLinkMsg_t *);
bool_t	xdr_DataCallConnectMsg_t(XDR *, DataCallConnectMsg_t *);
bool_t	xdr_VideoCallParam_t(XDR *, VideoCallParam_t *);
bool_t	xdr_CAPI2_CC_SetCallCfg_t(XDR *, CAPI2_CC_SetCallCfg_t *);
bool_t	xdr_CAPI2_CallConfig_t(XDR *, CAPI2_CallConfig_t *);
bool_t	xdr_CAPI2_CC_GetCallCfg_t(XDR *, CAPI2_CC_GetCallCfg_t *);
bool_t	xdr_CAPI2_CC_GetCallIndexInThisState_t(XDR *, CAPI2_CC_GetCallIndexInThisState_t *);
bool_t  xdr_SS_UserInfo_t(XDR *xdrs, SS_UserInfo_t *args);
bool_t  xdr_CAPI2_USER_INFORMATION_t(XDR *xdrs, CAPI2_USER_INFORMATION_t *rsp);
bool_t  xdr_CallReleaseInfo_t(XDR *xdrs, CallReleaseInfo_t* args);

bool_t 	xdr_RlpParam_t(XDR *, RlpParam_t *);
bool_t	xdr_CBST_t(XDR *, CBST_t *);
bool_t	xdr_DCParam_t(XDR *, DCParam_t *);

XDR_ENUM_DECLARE(CUGSuppressPrefCUG_t)
XDR_ENUM_DECLARE(CUGSuppressOA_t)
XDR_ENUM_DECLARE(CLIRMode_t)
XDR_ENUM_DECLARE(ALPHA_CODING_t)
XDR_ENUM_DECLARE(PresentStatus_t)
XDR_ENUM_DECLARE(DtmfTimer_t)
XDR_ENUM_DECLARE(ProgressInd_t)
XDR_ENUM_DECLARE(gsm_TON_t)
XDR_ENUM_DECLARE(gsm_NPI_t)
XDR_ENUM_DECLARE(ECTCallState_t)
XDR_ENUM_DECLARE(CallNotifySS_t)
XDR_ENUM_DECLARE(CCallType_t)
XDR_ENUM_DECLARE(CCallState_t)
XDR_ENUM_DECLARE(Result_t)
XDR_ENUM_DECLARE(Cause_t)
XDR_ENUM_DECLARE(PresentationInd_t)
XDR_ENUM_DECLARE(ScreenInd_t)
XDR_ENUM_DECLARE(AoCStatus_t)
XDR_ENUM_DECLARE(MNATDSMsgType_t)
XDR_ENUM_DECLARE(VideoCallSpeed_t)
XDR_ENUM_DECLARE(ECOrigReq_t)
XDR_ENUM_DECLARE(ECOrigFB_t)
XDR_ENUM_DECLARE(ECAnsFB_t)
XDR_ENUM_DECLARE(CallConfigType_t)
XDR_ENUM_DECLARE(ClientCmd_t)
XDR_ENUM_DECLARE(DtmfState_t)
XDR_ENUM_DECLARE(ECMode_t)
XDR_ENUM_DECLARE(DCMode_t)
XDR_ENUM_DECLARE(RepeatInd_t)
XDR_ENUM_DECLARE(Signal_t)

__END_DECLS

#endif
