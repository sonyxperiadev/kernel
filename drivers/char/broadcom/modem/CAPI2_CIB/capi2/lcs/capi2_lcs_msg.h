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




__BEGIN_DECLS

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
XDR_ENUM_DECLARE(LcsCpMoLrReq_t)

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



__END_DECLS

#endif



