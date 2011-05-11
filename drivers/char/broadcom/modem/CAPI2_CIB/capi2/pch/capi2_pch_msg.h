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

/**
PDP Data State
**/
typedef struct
{
	PCHResponseType_t		response;  ///<  PCH Response Type
	PCHNsapi_t				nsapi;  ///<  values: NOT_USED_0, MIN_NSAPI through MAX_NSAPI
	PCHPDPType_t			pdpType;  ///<  PDP_TYPE_IP
	PCHPDPAddress_t			pdpAddress;  ///<  values: NOT_USED_STRING, "0.0.0.0" to "255.255.255.255"  
	Result_t					cause;  	///<   result cause
} PDP_DataState_t;

__BEGIN_DECLS
bool_t 	xdr_MSNetworkInfo_t(XDR *, MSNetworkInfo_t *);
bool_t 	xdr_MSRegInfo_t(XDR *, MSRegInfo_t *);
bool_t  xdr_MsState_t(XDR *, MsState_t *);
bool_t 	xdr_PCHQosProfile_t(XDR *, PCHQosProfile_t *);
bool_t 	xdr_PCHPacketFilter_T(XDR *, PCHPacketFilter_T *);
bool_t 	xdr_PCHTrafficFlowTemplate_t(XDR *, PCHTrafficFlowTemplate_t *);
bool_t 	xdr_PCHXid_t(XDR *, PCHXid_t *);
//bool_t 	xdr_PDPContext_t(XDR *, PDPContext_t *);
//bool_t 	xdr_PDP_GPRSMinQoS_t(XDR *, PDP_GPRSMinQoS_t *);
bool_t 	xdr_PCHProtConfig_t(XDR *, PCHProtConfig_t *);
bool_t 	xdr_PCHPDPActivatedContext_t(XDR *, PCHPDPActivatedContext_t *);
bool_t 	xdr_PCHPDPActivatedSecContext_t(XDR *, PCHPDPActivatedSecContext_t *);
bool_t 	xdr_PDP_SendPDPActivateReq_Rsp_t(XDR *, PDP_SendPDPActivateReq_Rsp_t *);
bool_t 	xdr_PDP_SendPDPDeactivateReq_Rsp_t(XDR *, PDP_SendPDPDeactivateReq_Rsp_t *);
bool_t 	xdr_PDP_SendPDPActivateSecReq_Rsp_t(XDR *, PDP_SendPDPActivateSecReq_Rsp_t *);
bool_t 	xdr_GPRSActivate_t(XDR *, GPRSActivate_t *);
//bool_t 	xdr_PDP_GetGPRSActivateStatus_Rsp_t(XDR *, PDP_GetGPRSActivateStatus_Rsp_t *);
//bool_t	xdr_PDP_DataState_t(XDR *, PDP_DataState_t *);
bool_t	xdr_PDPDefaultContext_t(XDR *, PDPDefaultContext_t *);
//bool_t	xdr_PDP_GetPCHContext_Rsp_t(XDR *, PDP_GetPCHContext_Rsp_t *);
bool_t	xdr_GPRSActInd_t(XDR *, GPRSActInd_t *);
bool_t	xdr_GPRSDeactInd_t(XDR *, GPRSDeactInd_t *);
bool_t	xdr_PDP_PDPDeactivate_Ind_t(XDR *, PDP_PDPDeactivate_Ind_t *);
bool_t  xdr_PCHPDPModifiedContext_t(XDR *xdrs, PCHPDPModifiedContext_t *args);

bool_t	xdr_PCHR99QosProfile_t(XDR *, PCHR99QosProfile_t *);
bool_t	xdr_PCHUMTSQosProfile_t(XDR *, PCHUMTSQosProfile_t *);
bool_t	xdr_PCHPDPAddress_t(XDR *, PCHPDPAddress_t *);

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
bool_t	xdr_PCHNegotiatedParms_t(XDR *, PCHNegotiatedParms_t *);
bool_t	xdr_PCHPDPActivatePDU_t(XDR *, PCHPDPActivatePDU_t *);
bool_t	xdr_PDP_SendPDPActivatePDUReq_Rsp_t(XDR *, PDP_SendPDPActivatePDUReq_Rsp_t *);
bool_t	xdr_PDP_ActivateNWI_Ind_t(XDR *, PDP_ActivateNWI_Ind_t *);
bool_t	xdr_PCHAPN_t(XDR *, PCHAPN_t *);
bool_t	xdr_PDP_ActivateSecNWI_Ind_t(XDR *, PDP_ActivateSecNWI_Ind_t *);

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
XDR_ENUM_DECLARE(PCHPDPActivateCallControlResult_t)

XDR_STRUCT_DECLARE(PCHProtConfig_t)

XDR_STRUCT_DECLARE(PDP_SendPDPModifyReq_Rsp_t)
XDR_STRUCT_DECLARE(PCHCid_t)
XDR_STRUCT_DECLARE(CHAP_ChallengeOptions_t)
XDR_STRUCT_DECLARE(CHAP_ResponseOptions_t)
XDR_STRUCT_DECLARE(PAP_CnfgOptions_t)
XDR_STRUCT_DECLARE(PCHEx_ChapAuthType_t)
XDR_STRUCT_DECLARE(PLMN_t)
XDR_STRUCT_DECLARE(PDP_DataState_t)
XDR_STRUCT_DECLARE(PCHL2P_t)

__END_DECLS

#define xdr_MSClass_t	xdr_UInt8
#define xdr_PCHCid_t	xdr_UInt8

#endif

