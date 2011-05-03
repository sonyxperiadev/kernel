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
*   @file   capi2_lcs_cplane_msg.h
*
*   @brief  This file defines the capi2 message structure and forward declares
*	the serialization/deserialization functions.
*
****************************************************************************/

#ifndef CAPI2_LCS_CPLANE_MSG_H
#define CAPI2_LCS_CPLANE_MSG_H



__BEGIN_DECLS

XDR_ENUM_DECLARE(LcsRrcMcFailure_t)
XDR_ENUM_DECLARE(LcsRrcMcStatus_t)
XDR_ENUM_DECLARE(LcsRrcState_t)
XDR_ENUM_DECLARE(LcsRrcAdditionalReportPeriod_t)
XDR_ENUM_DECLARE(LcsRrcBroadcastMsgType_t)
XDR_ENUM_DECLARE(LcsRrcNetworkType_t)


__END_DECLS

bool_t xdr_LcsMsgData_t(XDR* xdrs, LcsMsgData_t *data);
bool_t xdr_LcsRrcMeasurement_t(XDR* xdrs, LcsRrcMeasurement_t *data);
bool_t xdr_LcsRrcBroadcastSysInfo_t(XDR* xdrs, LcsRrcBroadcastSysInfo_t *data);
bool_t xdr_LcsRrcUeState_t(XDR* xdrs, LcsRrcUeState_t *data);

bool_t xdr_LcsFttParams_t(XDR* xdrs, LcsFttParams_t *data);
bool_t xdr_LcsFttResult_t(XDR* xdrs, LcsFttResult_t *data);

#endif //CAPI2_LCS_CPLANE_MSG_H



