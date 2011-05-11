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




__BEGIN_DECLS
bool_t	xdr_MS_LocalElemNotifyInd_t(XDR *, MS_LocalElemNotifyInd_t *);
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


bool_t	xdr_CallForwardStatus_t(XDR *, CallForwardStatus_t *);
bool_t	xdr_CallBarringStatus_t(XDR *, CallBarringStatus_t *);
bool_t	xdr_SS_ProvisionStatus_t(XDR *, SS_ProvisionStatus_t *);
bool_t	xdr_SS_ActivationStatus_t(XDR *, SS_ActivationStatus_t *);
bool_t	xdr_USSDataInfo_t(XDR *, USSDataInfo_t *);
bool_t	xdr_USSDataInfo_t(XDR *, USSDataInfo_t *);
bool_t	xdr_CallIndex_t(XDR *, CallIndex_t *);
bool_t	xdr_SS_IntParSetInd_t(XDR *, SS_IntParSetInd_t *);
bool_t	xdr_LCS_SrvRsp_t(XDR *, LCS_SrvRsp_t *);
bool_t	xdr_PartyAddress_t(XDR *, PartyAddress_t *);
bool_t	xdr_PartySubAdd_t(XDR *, PartySubAdd_t *);
bool_t	xdr_SS_CcbsFeature_t(XDR *, SS_CcbsFeature_t *);

XDR_ENUM_DECLARE(SS_Mode_t)
XDR_ENUM_DECLARE(SS_CallFwdReason_t)
XDR_ENUM_DECLARE(SS_SvcCls_t)
XDR_ENUM_DECLARE(SS_CallBarType_t)
XDR_ENUM_DECLARE(CallIndex_t)
XDR_ENUM_DECLARE(NetworkCause_t)
XDR_ENUM_DECLARE(ServiceStatus_t)
XDR_ENUM_DECLARE(USSDService_t)
XDR_ENUM_DECLARE(SuppSvcStatus_t)
XDR_ENUM_DECLARE(CallType_t)
XDR_ENUM_DECLARE(CallStatus_t)
XDR_ENUM_DECLARE(SuppSvcType_t)
XDR_ENUM_DECLARE(ConfigMode_t)
XDR_ENUM_DECLARE(SS_Operation_t)

XDR_ENUM_DECLARE(BasicSrvType_t)
XDR_ENUM_DECLARE(SS_Component_t)
XDR_ENUM_DECLARE(SS_Code_t)
XDR_ENUM_DECLARE(SS_SrvType_t)
XDR_ENUM_DECLARE(TypeOfNumber_t)
XDR_ENUM_DECLARE(NumberPlanId_t)
XDR_ENUM_DECLARE(TypeOfSubAdd_t)
XDR_ENUM_DECLARE(OddEven_t)
XDR_ENUM_DECLARE(SS_PartyNotif_t)
XDR_ENUM_DECLARE(SS_FwdReason_t)
XDR_ENUM_DECLARE(SS_SubsOptionType_t)
XDR_ENUM_DECLARE(SS_ClirOption_t)
XDR_ENUM_DECLARE(SS_ErrorCode_t)
XDR_ENUM_DECLARE(SS_ProblemCode_t)
XDR_ENUM_DECLARE(SS_CallHold_t)
XDR_ENUM_DECLARE(SS_EctCallState_t)
XDR_ENUM_DECLARE(SS_Presentation_t)
XDR_ENUM_DECLARE(SS_SsApiReq_t)
XDR_ENUM_DECLARE(SS_ConstString_t)

//XDR_ENUM_DECLARE(Unicode_t)

__END_DECLS


#endif
