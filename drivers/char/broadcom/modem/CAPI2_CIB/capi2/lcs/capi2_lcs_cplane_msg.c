/****************************************************************************
*
*     Copyright (c) 2004 Broadcom Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           Broadcom Corporation
*           16215 Alton Parkway
*           P.O. Box 57013
*           Irvine, California 92619-7013
*
*     All information contained in this document is Broadcom Corporation
*     company private, proprietary, and trade secret.
*
****************************************************************************/
/**
*
*   @file   capi2_lcs_msg.c
*
*   @brief  This file implements the serialize/deserialize the request/respose
*	for LCS.
*
****************************************************************************/
#include "mobcom_types.h"
#include "resultcode.h"
#include "taskmsgs.h"
#include "consts.h"

#include "xdr_porting_layer.h"
#include "xdr.h"

#include "common_defs.h"
#include "uelbs_api.h"
#include "ms_database_def.h"
#include "common_sim.h"
#include "sim_def.h"
#ifndef UNDER_LINUX
#include <string.h>
#endif
#include "assert.h"
#include "sysparm.h"
#include "engmode_api.h"
#include "sysparm.h"
///
#include "i2c_drv.h"
#include "ecdc.h"
#include "uelbs_api.h"
#include "common_sim.h"
#include "sim_def.h"
#include "stk_def.h"
#include "mti_trace.h"
#include "logapi.h"
#include "log.h"
#include "tones_def.h"
#include "phonebk_def.h"
#include "phonectrl_def.h"
#include "phonectrl_api.h"
#include "rtc.h"
#include "netreg_def.h"
#include "ms_database_old.h"
#include "ms_database_api.h"
#include "netreg_util_old.h"
#include "netreg_util.h"
#include "netreg_api.h"
#include "common_sim.h"
#include "sim_def.h"
#include "stk_def.h"
#include "ss_def.h"
#include "sim_api.h"
#include "phonectrl_def.h"
#include "isim_def.h"
#include "ostypes.h"
#include "pch_def.h"
#include "pchex_def.h"
#include "hal_em_battmgr.h"
#include "cc_def.h"
#include "rtem_events.h"
#include "rtc.h"
#include "engmode_def.h"
#include "sms_def.h"
#include "simlock_def.h"

#include "capi2_global.h"
#include "capi2_mstruct.h"
#include "capi2_cc_ds.h"
#include "capi2_cc_msg.h"
#include "capi2_msnu.h"
#include "ss_api_old.h"
#include "ss_lcs_def.h"
#include "capi2_ss_msg.h"
#include "capi2_cp_socket.h"
#include "capi2_cp_msg.h"
#include "capi2_pch_msg.h"
#include "capi2_sms_msg.h"
#include "capi2_phonectrl_api.h"
#include "capi2_phonectrl_msg.h"
#include "capi2_sim_msg.h"
#include "capi2_ss_msg.h"
#include "capi2_stk_ds.h"
#include "capi2_stk_msg.h"
#include "lcs_cplane_api.h"
#include "capi2_lcs_msg.h"
#include "lcs_cplane_rrc_api.h"
#include "lcs_cplane_shared_def.h"
#include "lcs_ftt_api.h"
#include "capi2_common_xdr.h"

#define _xdr_char(a,b,c) xdr_char(a,b)
//#define _xdr_u_char(a,b,c) xdr_u_char(a,b)
//#define _xdr_u_long(a,b,c) _xdr_u_int32_t(a,b,c)
#define _xdr_u_int32_t(a,b,c) xdr_u_long(a,b)
#define _xdr_short(a,b,c) xdr_short(a,b)
#define _xdr_u_short(a,b,c) xdr_u_short(a,b)
#define _xdr_double(a,b,c) xdr_double(a,b)
#define _xdr_float(a,b,c) xdr_float(a,b)
#define _xdr_long(a,b,c)  xdr_long(a,b)


XDR_ENUM_FUNC(LcsRrcMcFailure_t)
XDR_ENUM_FUNC(LcsRrcMcStatus_t)
XDR_ENUM_FUNC(LcsRrcState_t)
XDR_ENUM_FUNC(LcsRrcAdditionalReportPeriod_t)
XDR_ENUM_FUNC(LcsRrcBroadcastMsgType_t)
XDR_ENUM_FUNC(LcsRrcNetworkType_t)




bool_t
xdr_LcsMsgData_t(XDR* xdrs, LcsMsgData_t *data)
{
	XDR_LOG(xdrs,"xdr_LcsMsgData_t")

	if( xdr_ClientInfo_t(xdrs, (ClientInfo_t *) &data->mClientInfo)	&&
		_xdr_u_long(xdrs, &data->mDataLen, "mDataLen")	&&
		xdr_bytes(xdrs, (char **)(void*)&data->mData, (u_int*)&data->mDataLen, ~0)
		)
		return(TRUE);
	else
		return(FALSE);
}





bool_t xdr_LcsRrcMeasurement_t(XDR* xdrs, LcsRrcMeasurement_t *data)
{
	XDR_LOG(xdrs,"xdr_LcsRrcMeasurement_t")

	if( xdr_ClientInfo_t(xdrs, (ClientInfo_t *) &data->mClientInfo)	&&
		_xdr_u_long(xdrs, &data->mDataLen, "mDataLen")	&&
		xdr_bytes(xdrs, (char **)(void*)&data->mData, (u_int*)&data->mDataLen, ~0) &&
		xdr_LcsRrcAdditionalReportPeriod_t(xdrs, &data->mReportPeriod) &&
		1)
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_LcsRrcBroadcastSysInfo_t(XDR* xdrs, LcsRrcBroadcastSysInfo_t *data)
{
	XDR_LOG(xdrs,"xdr_LcsRrcBroadcastSysInfo_t")

	if( xdr_ClientInfo_t(xdrs, (ClientInfo_t *) &data->mClientInfo)	&&
		_xdr_u_long(xdrs, &data->mDataLen, "mDataLen")	&&
		xdr_bytes(xdrs, (char **)(void*)&data->mData, (u_int*)&data->mDataLen, ~0) &&
		xdr_LcsRrcNetworkType_t(xdrs, &data->mNetType) &&
		xdr_LcsRrcBroadcastMsgType_t(xdrs, &data->mBroadcastMsgType) &&
		1)
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_LcsRrcUeState_t(XDR* xdrs, LcsRrcUeState_t *data)
{
	XDR_LOG(xdrs,"xdr_LcsRrcUeState_t")

	if( xdr_ClientInfo_t(xdrs, (ClientInfo_t *) &data->mClientInfo)	&&
		xdr_LcsRrcState_t(xdrs, &data->mState) &&
		1)
		return(TRUE);
	else
		return(FALSE);
}


bool_t xdr_LcsFttParams_t(XDR* xdrs, LcsFttParams_t *data)
{
	XDR_LOG(xdrs,"xdr_LcsFttParams_t")

	if( xdr_u_long(xdrs, (u_long *) &data->S_fn)	&&
		_xdr_u_int16_t(xdrs,   &data->U_arfcn, "U_arfcn")	&&
		_xdr_u_int16_t(xdrs,   &data->psc_bsic, "psc_bsic")	&&
		_xdr_UInt8(xdrs, (u_char *)&data->ta, "ta")	&&
		_xdr_UInt8(xdrs, (u_char *)&data->slot, "slot")	&&
		_xdr_u_int16_t(xdrs,   &data->bits, "bits")	&&
		1)
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_LcsFttResult_t(XDR* xdrs, LcsFttResult_t *data)
{
	XDR_LOG(xdrs,"xdr_LcsFttResult_t")

	if( xdr_ClientInfo_t(xdrs, (ClientInfo_t *) &data->mClientInfo)	&& 
		xdr_LcsFttParams_t(xdrs,   &data->mLcsFttParam)	&& 
		1)
		return(TRUE);
	else
		return(FALSE);
}
