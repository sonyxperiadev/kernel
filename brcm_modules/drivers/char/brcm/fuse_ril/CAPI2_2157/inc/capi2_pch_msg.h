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
*   @file   capi2_pch_msg.h
*
*   @brief  This file defines the capi2 message structure and forward declares
*	the serialization/deserialization functions.
*
****************************************************************************/
#ifndef CAPI2_PCH_MSG_H
#define CAPI2_PCH_MSG_H

#include "capi2_pch_api.h"

typedef struct
{

	UInt8 cid;

} CAPI2_PDP_ContextID_t;

typedef struct
{

	UInt8 cid;
	PDPContext_t *pcontext;

} CAPI2_PDP_SetPDPContext_t;


typedef struct
{

	UInt8 cid;
	UInt8 priCid;
	UInt8 dComp;
	UInt8 hComp;

} CAPI2_PDP_SetSecPDPContext_t;

typedef struct
{

	UInt8 cid;
	PCHQosProfile_t* params;

} CAPI2_PDP_SetGPRSQoS_t;
typedef struct
{

	UInt8 cid;
	PDP_GPRSMinQoS_t* qos;

} CAPI2_PDP_SetGPRSMinQoS_t;

typedef struct
{

	UInt8 cid;
	PCHActivateReason_t reason;
	PCHProtConfig_t* protConfig;

} CAPI2_PDP_SendPDPActivateReq_t;

typedef struct
{

	MSClass_t msClass;

} CAPI2_PDP_SetMSClass_t;

typedef struct
{

	DeRegCause_t cause;
	RegType_t regType;

} CAPI2_MS_SendDetachReq_t;

typedef struct
{

	UInt8 mode;

} CAPI2_MS_SetAttachMode_t;

typedef struct
{

	UInt8 cid;
	PCHTrafficFlowTemplate_t *pTft_in;

} CAPI2_PDP_SetUMTSTft_t;

typedef struct
{

	UInt8 chan;

} CAPI2_MS_IsGprsCallActive_t;

typedef struct
{

	UInt8 chan;
	Boolean active;

} CAPI2_MS_SetChanGprsCallActive_t;

typedef struct
{

	UInt8 chan;
	UInt8 cid;

} CAPI2_MS_SetCidForGprsActiveChan_t;

typedef struct
{

	UInt8 cid;

} CAPI2_MS_GetGprsActiveChanFromCid_t;

typedef struct
{

	UInt8 chan;

} CAPI2_MS_GetCidFromGprsActiveChan_t;

typedef struct
{

	UInt8 cid;
	UInt32 numberBytes;

} CAPI2_PDP_SendTBFData_t;

typedef struct
{

	UInt8 cid;
	PCHR99QosProfile_t *pR99Qos;

} CAPI2_PDP_SetR99UMTSMinQoS_t;

typedef struct
{

	UInt8 cid;
	PCHR99QosProfile_t *pR99Qos;

} CAPI2_PDP_SetR99UMTSQoS_t;

typedef struct
{

	UInt8 cid;
	PCHUMTSQosProfile_t *pQos;

} CAPI2_PDP_SetUMTSQoS_t;

typedef struct
{

	UInt8 cid;
	PCHPacketFilter_T *pPktFilter;

} CAPI2_PDP_TftAddFilter_t;


typedef struct
{

	UInt8 cid;
	PCHContextState_t contextState;

} CAPI2_PDP_SetPCHContextState_t;

typedef struct
{

	UInt8 cid;
	PDPDefaultContext_t *pDefaultContext;

} CAPI2_PDP_SetDefaultPDPContext_t;

typedef struct
{
	PDP_SendPDPActivateReq_Rsp_t* PDP_ACTIVATION_RSP_Rsp;
} CAPI2_PDP_ACTIVATION_RSP_t;

typedef struct
{
	UInt8 cid;
} CAPI2_PDP_SendPDPModifyReq_t;

typedef struct
{
	PDP_SendPDPModifyReq_Rsp_t* PDP_MODIFICATION_RSP_Rsp;
} CAPI2_PDP_MODIFICATION_RSP_t;

typedef struct
{
	PDP_SendPDPDeactivateReq_Rsp_t* PDP_DEACTIVATION_RSP_Rsp;
} CAPI2_PDP_DEACTIVATION_RSP_t;
typedef struct
{
	PDP_SendPDPActivateSecReq_Rsp_t* PDP_SEC_ACTIVATION_RSP_Rsp;
} CAPI2_PDP_SEC_ACTIVATION_RSP_t;

typedef struct
{
	PDP_DataState_t* PDP_ACTIVATE_SNDCP_RSP_Rsp;
} CAPI2_PDP_ACTIVATE_SNDCP_RSP_t;

typedef struct
{
	GPRSActInd_t* GPRS_ACTIVATE_IND_Rsp;
} CAPI2_GPRS_ACTIVATE_IND_t;
typedef struct
{
	GPRSDeactInd_t* GPRS_DEACTIVATE_IND_Rsp;
} CAPI2_GPRS_DEACTIVATE_IND_t;
typedef struct
{
	PDP_PDPDeactivate_Ind_t* PDP_DEACTIVATION_IND_Rsp;
} CAPI2_PDP_DEACTIVATION_IND_t;

typedef struct
{
	Inter_ModifyContextInd_t* GPRS_MODIFY_IND_Rsp;
} CAPI2_GPRS_MODIFY_IND_t;
typedef struct
{
	GPRSReActInd_t* GPRS_REACT_IND_Rsp;
} CAPI2_GPRS_REACT_IND_t;
typedef struct
{
	GPRSSuspendInd_t* DATA_SUSPEND_IND_Rsp;
} CAPI2_DATA_SUSPEND_IND_t;

typedef struct
{
	xdr_string_t username;
	xdr_string_t password;
	IPConfigAuthType_t authType;
} CAPI2_BuildProtocolConfigOption_t;

typedef struct
{
	IPConfigAuthType_t authType;
	CHAP_ChallengeOptions_t *cc;
	CHAP_ResponseOptions_t *cr;
	PAP_CnfgOptions_t *po;
} CAPI2_BuildProtocolConfigOption2_t;

__BEGIN_DECLS
bool_t 	xdr_MSNetworkInfo_t(XDR *, MSNetworkInfo_t *);
bool_t 	xdr_MSRegInfo_t(XDR *, MSRegInfo_t *);
bool_t 	xdr_PCHQosProfile_t(XDR *, PCHQosProfile_t *);
bool_t 	xdr_PCHPacketFilter_T(XDR *, PCHPacketFilter_T *);
bool_t 	xdr_PCHTrafficFlowTemplate_t(XDR *, PCHTrafficFlowTemplate_t *);
bool_t 	xdr_PCHXid_t(XDR *, PCHXid_t *);
bool_t 	xdr_PDPContext_t(XDR *, PDPContext_t *);
bool_t 	xdr_PDP_GPRSMinQoS_t(XDR *, PDP_GPRSMinQoS_t *);
bool_t 	xdr_PCHProtConfig_t(XDR *, PCHProtConfig_t *);
bool_t 	xdr_PCHPDPActivatedContext_t(XDR *, PCHPDPActivatedContext_t *);
bool_t 	xdr_PCHPDPActivatedSecContext_t(XDR *, PCHPDPActivatedSecContext_t *);
bool_t 	xdr_PDP_SendPDPActivateReq_Rsp_t(XDR *, PDP_SendPDPActivateReq_Rsp_t *);
bool_t 	xdr_PDP_SendPDPDeactivateReq_Rsp_t(XDR *, PDP_SendPDPDeactivateReq_Rsp_t *);
bool_t 	xdr_PDP_SendPDPActivateSecReq_Rsp_t(XDR *, PDP_SendPDPActivateSecReq_Rsp_t *);
bool_t 	xdr_GPRSActivate_t(XDR *, GPRSActivate_t *);
bool_t 	xdr_PDP_GetGPRSActivateStatus_Rsp_t(XDR *, PDP_GetGPRSActivateStatus_Rsp_t *);
bool_t	xdr_PDP_DataState_t(XDR *, PDP_DataState_t *);
bool_t	xdr_PDPDefaultContext_t(XDR *, PDPDefaultContext_t *);
bool_t	xdr_PDP_GetPCHContext_Rsp_t(XDR *, PDP_GetPCHContext_Rsp_t *);
bool_t	xdr_GPRSActInd_t(XDR *, GPRSActInd_t *);
bool_t	xdr_GPRSDeactInd_t(XDR *, GPRSDeactInd_t *);
bool_t	xdr_PDP_PDPDeactivate_Ind_t(XDR *, PDP_PDPDeactivate_Ind_t *);
bool_t  xdr_PCHPDPModifiedContext_t(XDR *xdrs, PCHPDPModifiedContext_t *args);
bool_t	xdr_CAPI2_PDP_ContextID_t(XDR *, CAPI2_PDP_ContextID_t *);

bool_t	xdr_CAPI2_PDP_SetPDPContext_t(XDR *, CAPI2_PDP_SetPDPContext_t *);
bool_t	xdr_CAPI2_PDP_SetGPRSQoS_t(XDR *, CAPI2_PDP_SetGPRSQoS_t *);
bool_t	xdr_CAPI2_PDP_SetSecPDPContext_t(XDR *, CAPI2_PDP_SetSecPDPContext_t *);
bool_t	xdr_CAPI2_PDP_SetGPRSMinQoS_t(XDR *, CAPI2_PDP_SetGPRSMinQoS_t *);
bool_t	xdr_CAPI2_PDP_SendPDPActivateReq_t(XDR *, CAPI2_PDP_SendPDPActivateReq_t *);
bool_t	xdr_CAPI2_PDP_SetMSClass_t(XDR *, CAPI2_PDP_SetMSClass_t *);
bool_t	xdr_CAPI2_MS_SendDetachReq_t(XDR *, CAPI2_MS_SendDetachReq_t *);
bool_t	xdr_CAPI2_MS_SetAttachMode_t(XDR *, CAPI2_MS_SetAttachMode_t *);
bool_t	xdr_CAPI2_PDP_SetUMTSTft_t(XDR *, CAPI2_PDP_SetUMTSTft_t *);
bool_t	xdr_CAPI2_MS_IsGprsCallActive_t(XDR *, CAPI2_MS_IsGprsCallActive_t *);
bool_t	xdr_CAPI2_MS_SetChanGprsCallActive_t(XDR *, CAPI2_MS_SetChanGprsCallActive_t *);
bool_t	xdr_CAPI2_MS_SetCidForGprsActiveChan_t(XDR *, CAPI2_MS_SetCidForGprsActiveChan_t *);
bool_t	xdr_CAPI2_MS_GetGprsActiveChanFromCid_t(XDR *, CAPI2_MS_GetGprsActiveChanFromCid_t *);
bool_t	xdr_CAPI2_MS_GetCidFromGprsActiveChan_t(XDR *, CAPI2_MS_GetCidFromGprsActiveChan_t *);
bool_t	xdr_PCHR99QosProfile_t(XDR *, PCHR99QosProfile_t *);
bool_t	xdr_PCHUMTSQosProfile_t(XDR *, PCHUMTSQosProfile_t *);
bool_t	xdr_PCHPDPAddress_t(XDR *, PCHPDPAddress_t *);

bool_t	xdr_CAPI2_PDP_SendTBFData_t(XDR *, CAPI2_PDP_SendTBFData_t *);
bool_t	xdr_CAPI2_PDP_SetR99UMTSMinQoS_t(XDR *, CAPI2_PDP_SetR99UMTSMinQoS_t *);
bool_t	xdr_CAPI2_PDP_SetR99UMTSQoS_t(XDR *, CAPI2_PDP_SetR99UMTSQoS_t *);
//bool_t	xdr_CAPI2_PDP_SetUMTSMinQoS_t(XDR *, CAPI2_PDP_SetUMTSQoS_t *);
bool_t	xdr_CAPI2_PDP_SetUMTSQoS_t(XDR *, CAPI2_PDP_SetUMTSQoS_t *);
bool_t	xdr_CAPI2_PDP_TftAddFilter_t(XDR *, CAPI2_PDP_TftAddFilter_t *);
bool_t	xdr_CAPI2_PDP_SetPCHContextState_t(XDR *, CAPI2_PDP_SetPCHContextState_t *);
bool_t	xdr_CAPI2_PDP_SetDefaultPDPContext_t(XDR *, CAPI2_PDP_SetDefaultPDPContext_t *);

bool_t	xdr_CAPI2_PDP_ACTIVATION_RSP_t(XDR *, CAPI2_PDP_ACTIVATION_RSP_t *);
bool_t	xdr_CAPI2_PDP_DEACTIVATION_RSP_t(XDR *, CAPI2_PDP_DEACTIVATION_RSP_t *);
bool_t	xdr_CAPI2_PDP_SEC_ACTIVATION_RSP_t(XDR *, CAPI2_PDP_SEC_ACTIVATION_RSP_t *);
bool_t	xdr_CAPI2_PDP_ACTIVATE_SNDCP_RSP_t(XDR *, CAPI2_PDP_ACTIVATE_SNDCP_RSP_t *);
bool_t	xdr_CAPI2_GPRS_ACTIVATE_IND_t(XDR *, CAPI2_GPRS_ACTIVATE_IND_t *);
bool_t	xdr_CAPI2_GPRS_DEACTIVATE_IND_t(XDR *, CAPI2_GPRS_DEACTIVATE_IND_t *);
bool_t	xdr_CAPI2_PDP_DEACTIVATION_IND_t(XDR *, CAPI2_PDP_DEACTIVATION_IND_t *);
bool_t	xdr_CAPI2_GPRS_MODIFY_IND_t(XDR *, CAPI2_GPRS_MODIFY_IND_t *);
bool_t	xdr_CAPI2_GPRS_REACT_IND_t(XDR *, CAPI2_GPRS_REACT_IND_t *);
bool_t	xdr_CAPI2_DATA_SUSPEND_IND_t(XDR *, CAPI2_DATA_SUSPEND_IND_t *);
bool_t	xdr_Inter_ModifyContextInd_t(XDR *, Inter_ModifyContextInd_t *);
bool_t	xdr_GPRSReActInd_t(XDR *, GPRSReActInd_t *);
bool_t	xdr_GPRSSuspendInd_t(XDR *, GPRSSuspendInd_t *);
bool_t	xdr_PCHDecodedProtConfig_t(XDR *, PCHDecodedProtConfig_t *);
bool_t	xdr_PCHPCsCfAddr_t(XDR *, PCHPCsCfAddr_t *);
bool_t	xdr_PCHDNSAddr_t(XDR *, PCHDNSAddr_t *);
bool_t	xdr_CAPI2_BuildProtocolConfigOption_t(XDR *, CAPI2_BuildProtocolConfigOption_t *);
bool_t	xdr_CAPI2_BuildProtocolConfigOption2_t(XDR *, CAPI2_BuildProtocolConfigOption2_t *);
bool_t  xdr_CAPI2_PDP_SendPDPModifyReq_t(XDR *xdrs, CAPI2_PDP_SendPDPModifyReq_t *args);
bool_t  xdr_CAPI2_PDP_MODIFICATION_RSP_t(XDR *xdrs, CAPI2_PDP_MODIFICATION_RSP_t *rsp);
bool_t	xdr_PCHNegotiatedParms_t(XDR *, PCHNegotiatedParms_t *);

XDR_ENUM_DECLARE(MSRegState_t)
XDR_ENUM_DECLARE(MSNetAccess_t)
XDR_ENUM_DECLARE(PCHRejectCause_t)
XDR_ENUM_DECLARE(PCHResponseType_t)
XDR_ENUM_DECLARE(ActivateState_t)
XDR_ENUM_DECLARE(RegType_t)
XDR_ENUM_DECLARE(PCHActivateReason_t)
XDR_ENUM_DECLARE(PCHDeactivateReason_t)
XDR_ENUM_DECLARE(AttachState_t)
XDR_ENUM_DECLARE(DeRegCause_t)
XDR_ENUM_DECLARE(PCHContextState_t)
XDR_ENUM_DECLARE(PCHSapiEnum_t)
XDR_ENUM_DECLARE(IP_AddrType_t)
XDR_ENUM_DECLARE(SuspendCause_t)
XDR_ENUM_DECLARE(IPConfigAuthType_t)

XDR_STRUCT_DECLARE(PCHProtConfig_t)
__END_DECLS
#endif

