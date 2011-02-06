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
#include "common_defs.h"
#include "xdr.h"
#include	"capi2_reqrep.h"
#include	"capi2_lcs_cplane_msg.h"

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
