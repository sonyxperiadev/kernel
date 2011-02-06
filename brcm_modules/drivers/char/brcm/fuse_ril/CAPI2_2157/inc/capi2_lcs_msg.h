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
*   @file   capi2_lcs_msg.h
*
*   @brief  This file defines the capi2 message structure and forward declares
*	the serialization/deserialization functions.
*
****************************************************************************/

#ifndef CAPI2_LCS_MSG_H
#define CAPI2_LCS_MSG_H

#include "xdr.h"
#include "capi2_lcs_api.h"

typedef struct
{

	ClientInfo_t *inClientInfoPtr;
	const LcsCpMoLrReq_t *inCpMoLrReq;

} CAPI2_LCS_CpMoLrReq_t;

typedef struct
{

	ClientInfo_t *inClientInfoPtr;

} CAPI2_LCS_CpMoLrAbort_t;

typedef struct
{

	ClientInfo_t *inClientInfoPtr;
	LCS_VerifRsp_t inVerificationRsp;

} CAPI2_LCS_CpMtLrVerificationRsp_t;

typedef struct
{

	ClientInfo_t *inClientInfoPtr;
	SS_Operation_t inOperation;
	Boolean inIsAccepted;

} CAPI2_LCS_CpMtLrRsp_t;

typedef struct
{

	ClientInfo_t *inClientInfoPtr;
	const LCS_TermCause_t *inTerminationCause;

} CAPI2_LCS_CpLocUpdateRsp_t;

typedef struct
{

	const LCS_LocEstimate_t *inLocEstData;

} CAPI2_LCS_DecodePosEstimate_t;

typedef struct
{

	const LcsAssistanceReq_t *inAssistReq;

} CAPI2_LCS_EncodeAssistanceReq_t;

typedef struct
{
	LCS_SrvRsp_t* MNSS_CLIENT_LCS_SRV_RSP_Rsp;
} CAPI2_MNSS_CLIENT_LCS_SRV_RSP_t;

typedef struct
{
	LCS_SrvInd_t* MNSS_CLIENT_LCS_SRV_IND_Rsp;
} CAPI2_MNSS_CLIENT_LCS_SRV_IND_t;

typedef struct
{
	SS_SrvRel_t* MNSS_CLIENT_LCS_SRV_REL_Rsp;
} CAPI2_MNSS_CLIENT_LCS_SRV_REL_t;

typedef struct
{
	SS_SrvRsp_t* MNSS_CLIENT_SS_SRV_RSP_Rsp;
} CAPI2_MNSS_CLIENT_SS_SRV_RSP_t;

typedef struct
{
	SS_SrvRel_t* MNSS_CLIENT_SS_SRV_REL_Rsp;
} CAPI2_MNSS_CLIENT_SS_SRV_REL_t;

typedef struct
{
	SS_SrvInd_t* MNSS_CLIENT_SS_SRV_IND_Rsp;
} CAPI2_MNSS_CLIENT_SS_SRV_IND_t;

typedef struct
{
	STK_SsSrvRel_t* MNSS_SATK_SS_SRV_RSP_Rsp;
} CAPI2_MNSS_SATK_SS_SRV_RSP_t;


__BEGIN_DECLS

XDR_ENUM_DECLARE(LcsResult_t)
XDR_ENUM_DECLARE(LcsPosSource_t)
//XDR_ENUM_DECLARE(LcsServiceType_t)
XDR_ENUM_DECLARE(LcsReqState_t)
XDR_ENUM_DECLARE(LcsSuplNotificationType_t)
XDR_ENUM_DECLARE(LcsSuplEncodingType_t)

XDR_ENUM_DECLARE(LcsPowerState_t)

XDR_STRUCT_DECLARE(LcsPosReqResult_t)
XDR_STRUCT_DECLARE(LcsPosInfo_t)
XDR_STRUCT_DECLARE(LcsPosData_t)

XDR_STRUCT_DECLARE(LcsSuplData_t)
XDR_STRUCT_DECLARE(LcsSuplConnection_t)
XDR_STRUCT_DECLARE(LcsSuplNotificationData_t)


XDR_STRUCT_DECLARE(LcsSuplSessionInfo_t)
XDR_STRUCT_DECLARE(LcsSuplCommData_t)

XDR_STRUCT_DECLARE(LcsNmeaData_t)
XDR_STRUCT_DECLARE(LcsPosDetail_t)

XDR_STRUCT_DECLARE(CAPI2_LcsCmdData_t)
XDR_STRUCT_DECLARE(CAPI2_LCS_SuplInitHmacRsp_t)
XDR_STRUCT_DECLARE(ServingCellInfo_t)

XDR_ENUM_DECLARE(LCS_TermCause_t)
XDR_ENUM_DECLARE(LCS_VerifRsp_t)
XDR_ENUM_DECLARE(LCS_MoLr_t)
XDR_ENUM_DECLARE(LCS_LocMethod_t)
XDR_ENUM_DECLARE(LCS_RspTimeCat_t)
XDR_ENUM_DECLARE(LCS_LocEstimateType_t)
XDR_ENUM_DECLARE(LCS_RanTech_t)
XDR_ENUM_DECLARE(LCS_FormatIndicator_t)
XDR_ENUM_DECLARE(LCS_Area_t)
XDR_ENUM_DECLARE(LCS_OccInfo_t)
XDR_ENUM_DECLARE(LCS_Notification_t)

bool_t	xdr_LcsAssistanceReq_t(XDR *, LcsAssistanceReq_t *);
bool_t	xdr_LcsPosEstimateUncertainty_t(XDR *, LcsPosEstimateUncertainty_t *);
bool_t	xdr_LcsAltitudeInfo_t(XDR *, LcsAltitudeInfo_t *);
bool_t	xdr_LcsPosEstimateInfo_t(XDR *, LcsPosEstimateInfo_t *);
bool_t	xdr_LCS_GanssAssistData_t(XDR *, LCS_GanssAssistData_t *);

bool_t	xdr_LCS_DeciphKey_t(XDR *, LCS_DeciphKey_t *);
bool_t	xdr_LCS_AddLocEstimate_t(XDR *, LCS_AddLocEstimate_t *);
bool_t	xdr_LCS_ReportPlmn_t(XDR *, LCS_ReportPlmn_t *);
bool_t	xdr_LCS_RepPlmnList_t(XDR *, LCS_RepPlmnList_t *);

bool_t	xdr_LCS_HGmlcAdd_t(XDR *, LCS_HGmlcAdd_t *);
bool_t	xdr_LCS_RGmlcAdd_t(XDR *, LCS_RGmlcAdd_t *);
bool_t	xdr_LCS_AreaEventReport_t(XDR *, LCS_AreaEventReport_t *);
bool_t	xdr_LCS_PrivateExt_t(XDR *, LCS_PrivateExt_t *);
bool_t	xdr_LCS_ExtContainer_t(XDR *, LCS_ExtContainer_t *);
bool_t	xdr_LCS_Qos_t(XDR *, LCS_Qos_t *);
bool_t	xdr_LCS_ExtAddress_t(XDR *, LCS_ExtAddress_t *);
bool_t	xdr_LCS_ClientExtId_t(XDR *, LCS_ClientExtId_t *);
bool_t	xdr_LCS_MlcNumber_t(XDR *, LCS_MlcNumber_t *);
bool_t	xdr_LCS_GpsAssistData_t(XDR *, LCS_GpsAssistData_t *);
bool_t	xdr_LCS_Location_t(XDR *, LCS_Location_t *);
bool_t	xdr_LCS_LocEstimate_t(XDR *, LCS_LocEstimate_t *);
bool_t	xdr_LCS_VelocityEstimate_t(XDR *, LCS_VelocityEstimate_t *);
bool_t	xdr_LCS_PerLdrInfo_t(XDR *, LCS_PerLdrInfo_t *);
bool_t	xdr_LCS_GanssAssistData_t(XDR *, LCS_GanssAssistData_t *);
bool_t	xdr_LCS_MoLrReq_t(XDR *, LCS_MoLrReq_t *);
bool_t	xdr_LCS_MoLrRsp_t(XDR *, LCS_MoLrRsp_t *);

bool_t	xdr_LCS_IdString_t(XDR *, LCS_IdString_t *);
bool_t	xdr_LCS_ClientName_t(XDR *, LCS_ClientName_t *);
bool_t	xdr_LCS_ReqId_t(XDR *, LCS_ReqId_t *);
bool_t	xdr_LCS_LocNotifInfo_t(XDR *, LCS_LocNotifInfo_t *);
bool_t	xdr_LCS_AreaDef_t(XDR *, LCS_AreaDef_t *);
bool_t	xdr_LCS_AreaEventInfo_t(XDR *, LCS_AreaEventInfo_t *);
bool_t	xdr_LCS_AreaEventReq_t(XDR *, LCS_AreaEventReq_t *);
bool_t	xdr_LCS_PeriodicLocReq_t(XDR *, LCS_PeriodicLocReq_t *);
bool_t	xdr_LCS_XxCancel_t(XDR *, LCS_XxCancel_t *);
bool_t	xdr_LCS_LocUpdate_t(XDR *, LCS_LocUpdate_t *);


bool_t	xdr_CAPI2_LCS_CpMoLrReq_t(XDR *, CAPI2_LCS_CpMoLrReq_t *);
bool_t	xdr_CAPI2_LCS_CpMoLrAbort_t(XDR *, CAPI2_LCS_CpMoLrAbort_t *);
bool_t	xdr_CAPI2_LCS_CpMtLrVerificationRsp_t(XDR *, CAPI2_LCS_CpMtLrVerificationRsp_t *);
bool_t	xdr_CAPI2_LCS_CpMtLrRsp_t(XDR *, CAPI2_LCS_CpMtLrRsp_t *);
bool_t	xdr_CAPI2_LCS_CpLocUpdateRsp_t(XDR *, CAPI2_LCS_CpLocUpdateRsp_t *);
bool_t	xdr_CAPI2_LCS_DecodePosEstimate_t(XDR *, CAPI2_LCS_DecodePosEstimate_t *);
bool_t	xdr_CAPI2_LCS_EncodeAssistanceReq_t(XDR *, CAPI2_LCS_EncodeAssistanceReq_t *);
bool_t	xdr_CAPI2_MNSS_CLIENT_LCS_SRV_RSP_t(XDR *, CAPI2_MNSS_CLIENT_LCS_SRV_RSP_t *);
bool_t	xdr_CAPI2_MNSS_CLIENT_LCS_SRV_IND_t(XDR *, CAPI2_MNSS_CLIENT_LCS_SRV_IND_t *);
bool_t	xdr_CAPI2_MNSS_CLIENT_LCS_SRV_REL_t(XDR *, CAPI2_MNSS_CLIENT_LCS_SRV_REL_t *);
bool_t	xdr_CAPI2_MNSS_CLIENT_SS_SRV_RSP_t(XDR *, CAPI2_MNSS_CLIENT_SS_SRV_RSP_t *);
bool_t	xdr_CAPI2_MNSS_CLIENT_SS_SRV_REL_t(XDR *, CAPI2_MNSS_CLIENT_SS_SRV_REL_t *);
bool_t	xdr_CAPI2_MNSS_CLIENT_SS_SRV_IND_t(XDR *, CAPI2_MNSS_CLIENT_SS_SRV_IND_t *);
bool_t	xdr_CAPI2_MNSS_SATK_SS_SRV_RSP_t(XDR *, CAPI2_MNSS_SATK_SS_SRV_RSP_t *);

bool_t xdr_LcsHandle_t(XDR *xdrs, LcsHandle_t *data);
bool_t xdr_LcsServiceType_t(XDR* xdrs, LcsServiceType_t* data);
bool_t xdr_LcsSuplConnectHdl_t(XDR* xdrs, LcsSuplConnectHdl_t *data);
bool_t xdr_LcsSuplSessionHdl_t(XDR* xdrs, LcsSuplSessionHdl_t *data);
bool_t xdr_LcsSuplSessionInfo_t(XDR* xdrs, LcsSuplSessionInfo_t *data);


bool_t xdr_CAPI2_LCS_RegisterSuplMsgHandler_Req_t(void* xdrs, CAPI2_LCS_RegisterSuplMsgHandler_Req_t *rsp);
bool_t xdr_CAPI2_LCS_StartPosReqPeriodic_Req_t(void* xdrs, CAPI2_LCS_StartPosReqPeriodic_Req_t *rsp);
bool_t xdr_CAPI2_LCS_StartPosReqSingle_Req_t(void* xdrs, CAPI2_LCS_StartPosReqSingle_Req_t *rsp);

__END_DECLS

#endif



