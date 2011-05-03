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



__BEGIN_DECLS

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
bool_t xdr_ApiDtmf_t(XDR *xdrs, ApiDtmf_t *args);
bool_t xdr_Ticks_t(XDR *xdrs, Ticks_t *args);



bool_t 	xdr_RlpParam_t(XDR *, RlpParam_t *);
bool_t	xdr_CBST_t(XDR *, CBST_t *);
bool_t	xdr_DCParam_t(XDR *, DCParam_t *);
bool_t  xdr_CcCipherInd_t(XDR* xdrs, CcCipherInd_t *param);
bool_t  xdr_SS_UserInfo_t(XDR *xdrs, SS_UserInfo_t *args);

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
XDR_ENUM_DECLARE(CCallStateList_t)
XDR_ENUM_DECLARE(CCallIndexList_t)
XDR_ENUM_DECLARE(CC_NotifySsInd_t)
XDR_ENUM_DECLARE(ApiClientCmdInd_t)
XDR_ENUM_DECLARE(CcApi_Element_t)









__END_DECLS

#endif
