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
#include "xdr.h"
#include	"capi2_reqrep.h"
#include	"capi2_lcs_msg.h"
#define _xdr_char(a,b,c) xdr_char(a,b)
//#define _xdr_u_char(a,b,c) xdr_u_char(a,b)
//#define _xdr_u_long(a,b,c) _xdr_u_int32_t(a,b,c)
#define _xdr_u_int32_t(a,b,c) xdr_u_long(a,b)
#define _xdr_short(a,b,c) xdr_short(a,b)
#define _xdr_u_short(a,b,c) xdr_u_short(a,b)
#define _xdr_double(a,b,c) xdr_double(a,b)
#define _xdr_float(a,b,c) xdr_float(a,b)
#define _xdr_long(a,b,c)  xdr_long(a,b)

XDR_ENUM_DECLARE(LcsSuplConnectPriority_t)
XDR_ENUM_DECLARE(LcsSuplSlpAddrType_t)
XDR_ENUM_DECLARE(LcsSuplFormat_t)

XDR_ENUM_FUNC(LcsResult_t)
XDR_ENUM_FUNC(LcsPosSource_t)
//XDR_ENUM_FUNC(LcsServiceType_t)
XDR_ENUM_FUNC(LcsReqState_t)

XDR_ENUM_FUNC(LcsPowerState_t)

XDR_ENUM_FUNC(LcsSuplNotificationType_t)
XDR_ENUM_FUNC(LcsSuplEncodingType_t)
XDR_ENUM_FUNC(LcsSuplConnectPriority_t)
XDR_ENUM_FUNC(LcsSuplSlpAddrType_t)
XDR_ENUM_FUNC(LcsSuplFormat_t)

XDR_ENUM_FUNC(LCS_TermCause_t)
XDR_ENUM_FUNC(LCS_VerifRsp_t)
XDR_ENUM_FUNC(LCS_MoLr_t)
XDR_ENUM_FUNC(LCS_LocMethod_t)
XDR_ENUM_FUNC(LCS_RspTimeCat_t)
XDR_ENUM_FUNC(LCS_LocEstimateType_t)
XDR_ENUM_FUNC(LCS_RanTech_t)
XDR_ENUM_FUNC(LCS_FormatIndicator_t)
XDR_ENUM_FUNC(LCS_Area_t)
XDR_ENUM_FUNC(LCS_OccInfo_t)
XDR_ENUM_FUNC(LCS_Notification_t)



bool_t xdr_LcsCoordinate_t(XDR* xdrs, LcsCoordinate_t* data);
bool_t xdr_LcsUtcTime_t(XDR* xdrs, LcsUtcTime_t* data);
bool_t xdr_LcsSvInfo_t(XDR* xdrs, LcsSvInfo_t* data);
bool_t xdr_LcsSuplNotificationItem_t(XDR* xdrs, LcsSuplNotificationItem_t* data);

bool_t
xdr_LcsHandle_t(XDR* xdrs, LcsHandle_t* data)
{
	XDR_LOG(xdrs,"xdr_LcsHandle_t")

	if( xdr_u_long(xdrs, (u_long *)data)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_LcsServiceType_t(XDR* xdrs, LcsServiceType_t* data)
{
	XDR_LOG(xdrs,"xdr_LcsServiceType_t")

	if( xdr_u_char(xdrs, (u_char *)data)
		)
		return(TRUE);
	else
		return(FALSE);
}


bool_t
xdr_LcsCoordinate_t(XDR* xdrs, LcsCoordinate_t* data)
{
	XDR_LOG(xdrs,"xdr_LcsCoordinate_t")

	if( _xdr_char(xdrs, &data->bPosValid, "bPosValid")	&&
		xdr_LcsPosSource_t(xdrs, &data->aPosSource) &&
		xdr_u_long(xdrs, &data->ulInternalStatus) &&
		xdr_double(xdrs, &data->dLat) &&
		xdr_double(xdrs, &data->dLon) &&
		xdr_double(xdrs, &data->dAlt) &&
		xdr_double(xdrs, &data->dHDOP) &&
		xdr_double(xdrs, &data->dEstErr) 
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_LcsUtcTime_t(XDR* xdrs, LcsUtcTime_t* data)
{
	XDR_LOG(xdrs,"xdr_LcsUtcTime_t")

	if( _xdr_u_int16_t(xdrs, &data->usYear, "usYear") &&
		_xdr_u_int16_t(xdrs, &data->usMonth, "usMonth") &&
		_xdr_u_int16_t(xdrs, &data->usDay, "usDay") &&
		_xdr_u_int16_t(xdrs, &data->usHour, "usHour") &&
		_xdr_u_int16_t(xdrs, &data->usMin, "usMin") &&
		_xdr_u_int16_t(xdrs, &data->usSec, "usSec") &&
		_xdr_u_int16_t(xdrs, (u_short *) &data->usMiliSec, "usMiliSec") &&
		_xdr_u_int16_t(xdrs, &data->usMicroSec, "usMicroSec") &&
		_xdr_u_int32_t(xdrs, &data->ulPrecUsec, "ulPrecUsec")
	)
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_LcsSvInfo_t(XDR* xdrs, LcsSvInfo_t* data)
{
	XDR_LOG(xdrs,"xdr_LcsSvInfo_t")

	if( _xdr_char(xdrs, &data->bDetected, "bDetected")	&&
		_xdr_short(xdrs, &data->sPrn, "sPrn") &&
		_xdr_short(xdrs, &data->sElev, "sElev") &&
		_xdr_short(xdrs, &data->sAz, "sAz") &&
		_xdr_short(xdrs, &data->sCNo, "sCNo") &&
		_xdr_short(xdrs, &data->sSigStrength, "sSigStrength") &&
		_xdr_short(xdrs, &data->sCNoFT, "sCNoFT") &&
		_xdr_short(xdrs, &data->sSigStrengthFT, "sSigStrengthFT")
	)
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_LcsPosData_t(XDR* xdrs, LcsPosData_t* data)
{
	XDR_LOG(xdrs,"LcsPosData_t")

	if( _xdr_u_long(xdrs, &data->ulElapsedTimeLms, "ulElapsedTimeLms")	&&
		xdr_LcsUtcTime_t(xdrs, &data->utcTime) &&
		_xdr_short(xdrs, &data->sSvCount, "sSvCount") &&
		/* xdr_LcsSvInfo_t (aSvInfo) array, see code below in main scope */
		_xdr_char(xdrs, &data->bPosValid, "bPosValid") &&
		xdr_LcsPosSource_t(xdrs, &data->aPosSource) &&
		_xdr_u_long(xdrs, &data->ulInternalStatus, "ulInternalStatus") &&

		_xdr_double(xdrs, &data->dLat, "dLat") &&
		_xdr_double(xdrs, &data->dLon, "dLon") &&
		_xdr_double(xdrs, &data->dAlt, "dAlt") &&
		_xdr_double(xdrs, &data->dHDOP, "dHDOP") &&
		_xdr_double(xdrs, &data->dEstErr, "dEstErr") &&
		_xdr_double(xdrs, &data->dEstErrHigh, "dEstErrHigh") &&
		_xdr_double(xdrs, &data->dEstErrAlt, "dEstErrAlt") &&

		_xdr_long(xdrs, &data->lTimeTagDelta, "lTimeTagDelta") &&
		_xdr_u_long(xdrs, &data->ulTimeTagDeltaUncUs, "ulTimeTagDeltaUncUs") &&
		_xdr_double(xdrs, &data->dCbSec, "dCbSec") &&
		_xdr_char(xdrs, &data->cSpeedValid, "cSpeedValid") &&
		_xdr_double(xdrs, &data->dSpeedInKnots, "dSpeedInKnots") &&

		_xdr_char(xdrs, &data->cTrackAngleValid, "cTrackAngleValid") &&
		_xdr_double(xdrs, &data->dTrackAngle, "dTrackAngle") &&
		_xdr_long(xdrs, &data->slFreqOffst, "slFreqOffst") &&
		_xdr_u_short(xdrs, &data->usFreqOffsAcc, "usFreqOffsAcc") &&

		_xdr_short(xdrs, &data->sUsedSvCount, "sUsedSvCount") &&
		_xdr_u_long(xdrs, &data->ulUsedSvsMask, "ulUsedSvsMask") &&
		_xdr_u_short(xdrs, &data->usTimeTagDeltaUs, "usTimeTagDeltaUs") &&
		_xdr_u_long(xdrs, &data->ulSyncElapsedTimeUs, "ulSyncElapsedTimeUs") &&
		_xdr_float(xdrs, &data->fWer, "fWer") &&

		_xdr_u_long(xdrs, &data->ulNbWerWrd, "ulNbWerWrd") &&
		_xdr_double(xdrs, &data->dTtf, "dTtf") &&
		1)
	{
		int result=TRUE;
		int i;
		for (i=0; i<LCS_MAX_SV_COUNT; ++i)
		{
			result &= xdr_LcsSvInfo_t(xdrs, &data->aSvInfo[i]);
		}
		return(result);
	}
	else
		return(FALSE);

}

bool_t
xdr_LcsPosInfo_t(XDR* xdrs, LcsPosInfo_t* data)
{
	XDR_LOG(xdrs,"LcsPosInfo_t")

	if( xdr_u_long(xdrs, (u_long *) &data->mLcsHandle)	&&
	   _xdr_UInt8(xdrs, (u_char *)&data->mAppClientID, "mAppClientID")	&&
		xdr_LcsCoordinate_t(xdrs, &data->mFixCoordinate)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_LcsPosDetail_t(XDR* xdrs, LcsPosDetail_t* data)
{
	XDR_LOG(xdrs,"LcsPosDetail_t")

	if( xdr_u_long(xdrs, (u_long *) &data->mLcsHandle)	&&
	   _xdr_UInt8(xdrs, (u_char *)&data->mAppClientID, "mAppClientID")	&&
		xdr_LcsPosData_t(xdrs, &data->mPosData)
		)
		return(TRUE);
	else
		return(FALSE);
}


bool_t
xdr_LcsNmeaData_t(XDR* xdrs, LcsNmeaData_t *data)
{
	XDR_LOG(xdrs,"xdr_LcsNmeaData_t")

	if( xdr_u_long(xdrs, (u_long *) &data->mLcsHandle)	&&
	   _xdr_UInt8(xdrs, (u_char *)&data->mAppClientID, "mAppClientID")	&&
		_xdr_u_long(xdrs, &data->mNmeaLen, "mNmeaLen")	&&
		xdr_bytes(xdrs, (char **)(void*)&data->mNmeaData, (u_int*)&data->mNmeaLen, ~0)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_LcsPosReqResult_t(XDR* xdrs, LcsPosReqResult_t* data)
{
	XDR_LOG(xdrs,"LcsPosReqResult_t")

	if( _xdr_u_long(xdrs, (u_long *)&data->mLcsHandle, "mLcsHandle")	&&
		_xdr_UInt8(xdrs, (u_char *)&data->mAppClientID, "mAPClientID")	&&
		_xdr_u_long(xdrs, (u_long *)(void*)&data->eLcsReqState, "eLcsReqState") &&
	   1)
		return(TRUE);
	else
		return(FALSE);

}

bool_t
xdr_CAPI2_LcsCmdData_t(XDR* xdrs, CAPI2_LcsCmdData_t* data)
{
	if( _xdr_long(xdrs, &data->cmd, "cmd") &&
		_xdr_long(xdrs, &data->userTag, "tag") &&
		_xdr_long(xdrs, &data->parm1, "parm1") &&
		_xdr_long(xdrs, &data->parm2, "parm2") &&
		_xdr_u_long(xdrs, &data->dataSize, "dataSize")	&&
		xdr_bytes(xdrs, &data->dataPtr, (u_int *)&data->dataSize, ~0)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_LcsSuplData_t(XDR* xdrs, LcsSuplData_t* data)
{
	if( _xdr_u_long(xdrs, (u_long *) &data->suplSessionHdl, "suplSessionHdl") &&
		_xdr_u_long(xdrs, &data->suplDataLen, "suplDataLen")	&&
		xdr_bytes(xdrs, (char **)(void*)&data->suplData, (u_int *) &data->suplDataLen, ~0)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_LcsSuplConnection_t(XDR* xdrs, LcsSuplConnection_t* data)
{
	XDR_LOG(xdrs,"xdr_LcsSuplConnection_t")

	if( xdr_LcsSuplSessionHdl_t(xdrs, &data->suplSessionHdl) && 
		XDR_ENUM(xdrs, &data->eType, LcsSuplSlpAddrType_t) &&
		xdr_LcsSuplConnectPriority_t(xdrs, &data->ePriority) &&
		1)
	{
		switch (data->eType)
		{
		case LCS_SUPL_SLP_ADDR_DEFAULT:
			XDR_LOG(xdrs,"LCS_SUPL_SLP_ADDR_DEFAULT")
			return TRUE;

		case LCS_SUPL_SLP_ADDR_URL:
			XDR_LOG(xdrs,"LCS_SUPL_SLP_ADDR_URL")
			return 		xdr_uchar_ptr_t(xdrs, (unsigned char **) &data->u.url);

		case LCS_SUPL_SLP_ADDR_IPv4:
			XDR_LOG(xdrs,"LCS_SUPL_SLP_ADDR_IPv4")
			return xdr_opaque(xdrs, (caddr_t)data->u.ipv4, sizeof(data->u.ipv4) );

		case LCS_SUPL_SLP_ADDR_IPv6:
			XDR_LOG(xdrs,"LCS_SUPL_SLP_ADDR_IPv6")
			return xdr_opaque(xdrs, (caddr_t)data->u.ipv6, sizeof(data->u.ipv6) );

		default:
			XDR_LOG(xdrs,"Unknown data->eType")
			return TRUE; //*FIXE*, currently, just display a warning, and let it go  */
		}

	}
	else
		return(FALSE);
}

bool_t
xdr_LcsSuplNotificationItem_t(XDR* xdrs, LcsSuplNotificationItem_t* data)
{
	XDR_LOG(xdrs,"xdr_LcsSuplNotificationItem_t")
	
	if( _xdr_u_long(xdrs, &data->dataLen, "dataLen")	&&
		xdr_opaque(xdrs, (caddr_t) data->data, LCS_SUPL_NOTIFICATION_ITEM_LEN) &&
		1)
		return(TRUE);
	else
		return(FALSE);


}


bool_t
xdr_LcsSuplNotificationData_t(XDR* xdrs, LcsSuplNotificationData_t* data)
{
	XDR_LOG(xdrs,"xdr_LcsSuplNotificationData_t")

	if( _xdr_u_char(xdrs, &data->fieldPresentFlags, "fieldPresentFlags") &&
		xdr_LcsSuplSessionHdl_t(xdrs, &data->suplSessionHdl) &&
		XDR_ENUM(xdrs, &data->eNotificationType, LcsSuplNotificationType_t) &&
		XDR_ENUM(xdrs, &data->eEncodingType, LcsSuplEncodingType_t) &&
		xdr_LcsSuplNotificationItem_t(xdrs, &data->requestorId) &&
		XDR_ENUM(xdrs, &data->requestorIdType, LcsSuplFormat_t) &&
		xdr_LcsSuplNotificationItem_t(xdrs, &data->clientName) &&
		XDR_ENUM(xdrs, &data->clientNameType, LcsSuplFormat_t) &&
		1)
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_CAPI2_LCS_SuplInitHmacRsp_t(XDR* xdrs, CAPI2_LCS_SuplInitHmacRsp_t* data)
{
	XDR_LOG(xdrs,"xdr_CAPI2_LCS_SuplInitHmacRsp_t")

	if( xdr_LcsSuplSessionHdl_t(xdrs, &data->sessionHdl)	&&
		_xdr_u_long(xdrs,   &data->suplInitDataLen, "suplInitDataLen")	&&
		xdr_bytes(xdrs, (char **)(void*) &data->suplInitData, (u_int *) &data->suplInitDataLen, ~0) &&
		xdr_opaque(xdrs, (caddr_t) data->suplHmac, sizeof(SuplHmac_t)) &&
		1)
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_LcsSuplCommData_t(XDR* xdrs, LcsSuplCommData_t *data)
{
	XDR_LOG(xdrs,"LcsSuplCommData_t")

	if( _xdr_u_long(xdrs, (u_long*)(void*)&data->mSessionHdl, "mSessionHdl") &&
		_xdr_u_long(xdrs, (u_long*)(void*)&data->mConnectHdl, "mConnectHdl") &&
		_xdr_u_long(xdrs, (u_long*)(void*)&data->mDataLen, "mDataLen") &&
		xdr_bytes(xdrs, (char **)(void*) &data->mData, (u_int *) &data->mDataLen, ~0) &&
		1)
		return(TRUE);
	else
		return(FALSE);

}

bool_t
xdr_LcsSuplSessionInfo_t(XDR* xdrs, LcsSuplSessionInfo_t *data)
{
	XDR_LOG(xdrs,"LcsSuplSessionInfo_t")

	if( _xdr_u_long(xdrs, (u_long*)(void*)&data->suplSessionHdl, "suplSessionHdl") &&
		_xdr_u_long(xdrs, (u_long*)(void*)&data->suplConnectHdl, "suplConnectHdl") &&
		1)
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_LcsSuplConnectHdl_t(XDR* xdrs, LcsSuplConnectHdl_t *data)
{
	XDR_LOG(xdrs,"LcsSuplConnectHdl_t")

	if( xdr_u_long(xdrs, (u_long*)(void*)data) &&
		1)
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_LcsSuplSessionHdl_t(XDR* xdrs, LcsSuplSessionHdl_t *data)
{
	XDR_LOG(xdrs,"LcsSuplSessionHdl_t")

	if( xdr_u_long(xdrs, (u_long*)(void*)data) &&
		1)
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_CAPI2_LCS_RegisterSuplMsgHandler_Req_t(void* xdrs, CAPI2_LCS_RegisterSuplMsgHandler_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_RegisterSuplMsgHandler_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->inClientID,"inClientID") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_LCS_StartPosReqPeriodic_Req_t(void* xdrs, CAPI2_LCS_StartPosReqPeriodic_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_StartPosReqPeriodic_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->inClientID,"inClientID") &&
		xdr_UInt32(xdrs, &rsp->inPosOutputMask) &&
		xdr_UInt32(xdrs, &rsp->inPosFixPeriod) &&
	1)
		return TRUE;
	else
		return FALSE;
}



bool_t xdr_CAPI2_LCS_StartPosReqSingle_Req_t(void* xdrs, CAPI2_LCS_StartPosReqSingle_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_LCS_StartPosReqSingle_Req_t")

	if(
		_xdr_UInt8(xdrs, &rsp->inClientID,"inClientID") &&
		xdr_UInt32(xdrs, &rsp->inPosOutputMask) &&
		xdr_UInt32(xdrs, &rsp->inAccuracyInMeters) &&
		xdr_UInt32(xdrs, &rsp->inPosFixTimeOut) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_T_LBS_GSM_Cell(XDR* xdrs, T_LBS_GSM_Cell* data)
{
	XDR_LOG(xdrs,"T_LBS_GSM_Params")

	if( _xdr_u_int16_t(xdrs, &data->cell_id, "cell_id") &&
		_xdr_u_int16_t(xdrs, &data->arfcn, "arfcn") &&
		_xdr_UInt8(xdrs, &data->rxlev, "rxlev") && 
		_xdr_UInt8(xdrs, &data->bsic, "bsic")
	)
		return(TRUE);
	else
		return(FALSE);
}

#ifdef _BCM2153_
bool_t xdr_T_LBS_UMTS_MeasCell(XDR* xdrs, T_LBS_UMTS_MeasCell* data)
{
	XDR_LOG(xdrs,"T_LBS_UMTS_MeasCell")

	if( xdr_UInt32(xdrs, &data->cell_identity) &&
//		_xdr_u_int16_t(xdrs, &data->uarfcn_DL, "uarfcn_DL") &&
//		_xdr_u_int16_t(xdrs, &data->uarfcn_UL, "uarfcn_UL") &&
		_xdr_u_int16_t(xdrs, &data->psc, "psc") &&
		_xdr_Int8(xdrs, &data->rscp, "rscp") && 
		_xdr_u_int16_t(xdrs, &data->pathloss, "pathloss") &&
		_xdr_Int8(xdrs, &data->ecno, "ecno")
	)
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_T_LBS_UMTS_MeasFreq(XDR* xdrs, T_LBS_UMTS_MeasFreq* data)
{
	XDR_LOG(xdrs,"T_LBS_UMTS_MeasFreq")

	if( _xdr_u_int16_t(xdrs, &data->uarfcn_DL, "uarfcn_DL") &&
		_xdr_u_int16_t(xdrs, &data->uarfcn_UL, "uarfcn_UL") &&
		_xdr_Int8(xdrs, &data->rssi, "rssi") && 
		_xdr_UInt8(xdrs, &data->num_cell, "num_cell") && 
		xdr_vector(xdrs,  (char *)(void *)data->cells, LBS_MAX_NUM_CELL_PER_FREQ, sizeof(T_LBS_UMTS_MeasCell), (xdrproc_t)xdr_T_LBS_UMTS_MeasCell) 
	)
		return(TRUE);
	else
		return(FALSE);
}


bool_t xdr_T_LBS_UMTS_Params(XDR* xdrs, T_LBS_UMTS_Params* data)
{
	XDR_LOG(xdrs,"T_LBS_UMTS_Params")

	if( xdr_T_LBS_UMTS_MeasFreq(xdrs, &data->active) && 
		_xdr_UInt8(xdrs, &data->num_umts_freq, "num_umts_freq") && 
		xdr_vector(xdrs,  (char *)(void *)data->umts_freqs, LBS_MAX_NUM_UMTS_FREQ, sizeof(T_LBS_UMTS_MeasFreq), (xdrproc_t)xdr_T_LBS_UMTS_MeasFreq) && 
		_xdr_UInt8(xdrs, &data->num_gsm_ncell, "num_gsm_ncell") && 
		xdr_vector(xdrs,  (char *)(void *)data->gsm_ncells, LBS_MAX_NUM_GSM_NCELL, sizeof(T_LBS_GSM_Cell), (xdrproc_t)xdr_T_LBS_GSM_Cell) 
	)
		return(TRUE);
	else
		return(FALSE);
}
#endif

bool_t xdr_T_LBS_GSM_Params(XDR* xdrs, T_LBS_GSM_Params* data)
{
	XDR_LOG(xdrs,"T_LBS_GSM_Params")

	if( xdr_T_LBS_GSM_Cell(xdrs, &data->scell) && 
#ifdef _BCM2153_
		_xdr_UInt8(xdrs, &data->num_umts_freq, "num_umts_freq") && 
		xdr_vector(xdrs,  (char *)(void *)data->umts_freqs, LBS_MAX_NUM_UMTS_FREQ, sizeof(T_LBS_UMTS_MeasFreq), (xdrproc_t)xdr_T_LBS_UMTS_MeasFreq) && 
#endif
		_xdr_UInt8(xdrs, &data->num_gsm_ncells, "num_gsm_ncells") && 
		xdr_vector(xdrs,  (char *)(void *)data->gsm_ncells, LBS_MAX_NUM_GSM_NCELL, sizeof(T_LBS_GSM_Cell), (xdrproc_t)xdr_T_LBS_GSM_Cell) 
	)
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_T_LBS_MM_Params(XDR* xdrs, T_LBS_MM_Params* data)
{
	XDR_LOG(xdrs,"T_LBS_MM_Params")

	if( _xdr_u_int16_t(xdrs, &data->mcc, "mcc") &&
		_xdr_u_int16_t(xdrs, &data->mnc, "mnc") &&
		_xdr_u_int16_t(xdrs, &data->lac, "lac") 
	)
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_T_UE_LBS_PARAMS(XDR* xdrs, T_UE_LBS_PARAMS* data)
{
	XDR_LOG(xdrs,"T_UE_LBS_PARAMS")

	if( xdr_T_LBS_MM_Params(xdrs, (T_LBS_MM_Params*)&data->lbs_mm_params)	&&
		xdr_T_LBS_GSM_Params(xdrs, (T_LBS_GSM_Params*)&data->lbs_gsm_params)	&&
#ifdef _BCM2153_
		xdr_T_LBS_UMTS_Params(xdrs, (T_LBS_UMTS_Params*)&data->lbs_umts_params) && 
#endif
		1)
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_ServingCellInfo_t(XDR* xdrs, ServingCellInfo_t* data)
{
	XDR_LOG(xdrs,"ServingCellInfo_t")

	if( _xdr_UInt8(xdrs, &data->mRAT, "mRAT") && 
		xdr_T_UE_LBS_PARAMS(xdrs, (T_UE_LBS_PARAMS*) &data->mLbsParams)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_LCS_DeciphKey_t(XDR *xdrs, LCS_DeciphKey_t *args)
{


	XDR_LOG(xdrs,"LCS_DeciphKey_t ")
	if( 
		_xdr_u_char(xdrs, &args->ciphKeyFlag,"ciphKeyFlag") && 
		xdr_opaque(xdrs, (caddr_t) args->currentKey, LCS_DECIPHERING_KEY_SIZE) && 
		xdr_opaque(xdrs, (caddr_t) args->nextKey, LCS_DECIPHERING_KEY_SIZE)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_LCS_AddLocEstimate_t(XDR *xdrs, LCS_AddLocEstimate_t *args)
{


	XDR_LOG(xdrs,"LCS_AddLocEstimate_t ")
	if( 
		_xdr_u_char(xdrs, &args->len,"len") && 
		xdr_opaque(xdrs, (caddr_t) args->data, LCS_ADD_LOCATION_ESTIMATE_SIZE)
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_LCS_ReportPlmn_t(XDR *xdrs, LCS_ReportPlmn_t *args)
{


	XDR_LOG(xdrs,"LCS_ReportPlmn_t ")
	if( 
		_xdr_u_char(xdrs, &args->include,"include") && 
		xdr_opaque(xdrs, (caddr_t) args->plmnId, LCS_PLMN_ID_SIZE) && 
		XDR_ENUM(xdrs, &(args->ranTech), LCS_RanTech_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_LCS_RepPlmnList_t(XDR *xdrs, LCS_RepPlmnList_t *args)
{


	XDR_LOG(xdrs,"LCS_RepPlmnList_t ")
	if( 
		_xdr_u_char(xdrs, &args->include,"include") && 
		_xdr_u_char(xdrs, &args->rptPlmnListLen,"rptPlmnListLen") && 
		xdr_vector(xdrs, (char *) args->plmnList, LCS_PLMN_LIST_SIZE, sizeof(LCS_ReportPlmn_t), xdr_LCS_ReportPlmn_t) && 
		xdr_opaque(xdrs, (caddr_t) args->plmnId, LCS_PLMN_ID_SIZE) && 
		XDR_ENUM(xdrs, &(args->ranTech), LCS_RanTech_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_LcsAssistanceReq_t(XDR *xdrs, LcsAssistanceReq_t *args)
{


	XDR_LOG(xdrs,"LcsAssistanceReq_t ")
	if( 
		_xdr_u_long(xdrs, &args->ulAidMask,"ulAidMask") && 
		_xdr_u_char(xdrs, &args->ucNSAT,"ucNSAT") && 
		_xdr_u_int16_t(xdrs, &args->usGpsWeek,"usGpsWeek") && 
		_xdr_u_char(xdrs, &args->ucToe,"ucToe") && 
		_xdr_u_char(xdrs, &args->ucT_ToeLimit,"ucT_ToeLimit") && 
		xdr_opaque(xdrs, (caddr_t) args->ucSatID, 31) && 
		xdr_opaque(xdrs, (caddr_t) args->ucIODE, 31)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_LcsPosEstimateUncertainty_t(XDR *xdrs, LcsPosEstimateUncertainty_t *args)
{


	XDR_LOG(xdrs,"LcsPosEstimateUncertainty_t ")
	if( 
		_xdr_u_char(xdrs, &args->uncertaintySemiMajor,"uncertaintySemiMajor") && 
		_xdr_u_char(xdrs, &args->uncertaintySemiMinor,"uncertaintySemiMinor") && 
		_xdr_u_char(xdrs, &args->orientationMajorAxis,"orientationMajorAxis")
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_LcsAltitudeInfo_t(XDR *xdrs, LcsAltitudeInfo_t *args)
{


	XDR_LOG(xdrs,"LcsAltitudeInfo_t ")
	if( 
		XDR_ENUM(xdrs, &(args->isAltUncertaintyPresent), Boolean) && 
		_xdr_short(xdrs, &args->altitudeMeters,"altitudeMeters") && 
		_xdr_u_char(xdrs, &args->altUncertainty,"altUncertainty")
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_LcsPosEstimateInfo_t(XDR *xdrs, LcsPosEstimateInfo_t *args)
{


	XDR_LOG(xdrs,"LcsPosEstimateInfo_t ")
	if( 
		_xdr_u_char(xdrs, &args->fieldPresentFlags,"fieldPresentFlags") && 
		_xdr_u_char(xdrs, &args->etTypeOfShape,"etTypeOfShape") && 
		_xdr_double(xdrs, &args->latitudeDbl,"latitudeDbl") && 
		_xdr_double(xdrs, &args->longitudeDbl,"longitudeDbl") && 
		xdr_LcsPosEstimateUncertainty_t(xdrs, &args->uncertainty) && 
		_xdr_u_char(xdrs, &args->confidence,"confidence") && 
		xdr_LcsAltitudeInfo_t(xdrs, &args->altitudeInfo)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_LCS_HGmlcAdd_t(XDR *xdrs, LCS_HGmlcAdd_t *args)
{


	XDR_LOG(xdrs,"LCS_HGmlcAdd_t ")
	if( 
		_xdr_u_char(xdrs, &args->length,"length") && 
		xdr_opaque(xdrs, (caddr_t) args->data, LCS_H_GMLC_ADDRESS_SIZE)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_LCS_RGmlcAdd_t(XDR *xdrs, LCS_RGmlcAdd_t *args)
{


	XDR_LOG(xdrs,"LCS_RGmlcAdd_t ")
	if( 
		_xdr_u_char(xdrs, &args->length,"length") && 
		xdr_opaque(xdrs, (caddr_t) args->data, LCS_R_GMLC_ADDRESS_SIZE)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_LCS_AreaEventReport_t(XDR *xdrs, LCS_AreaEventReport_t *args)
{


	XDR_LOG(xdrs,"LCS_AreaEventReport_t ")
	if( 
		_xdr_u_char(xdrs, &args->include,"include") && 
		_xdr_u_char(xdrs, &args->refNumber,"refNumber") && 
		xdr_LCS_HGmlcAdd_t(xdrs, &args->hGmlcAdd) && 
		xdr_LCS_RGmlcAdd_t(xdrs, &args->rGmlcAdd)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_LCS_PrivateExt_t(XDR *xdrs, LCS_PrivateExt_t *args)
{


	XDR_LOG(xdrs,"LCS_PrivateExt_t ")
	if( 
		_xdr_u_char(xdrs, &args->extId,"extId") && 
		_xdr_u_char(xdrs, &args->extType,"extType")
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_LCS_ExtContainer_t(XDR *xdrs, LCS_ExtContainer_t *args)
{


	XDR_LOG(xdrs,"LCS_ExtContainer_t ")
	if( 
		_xdr_u_char(xdrs, &args->include,"include") && 
		_xdr_u_char(xdrs, &args->privateExtlistSize,"privateExtlistSize") && 
		xdr_vector(xdrs, (char *) args->privateExtList, LCS_PRIVATE_EXTENSION_LIST_SIZE, sizeof(LCS_PrivateExt_t), xdr_LCS_PrivateExt_t) && 
		_xdr_u_char(xdrs, &args->pcsExt,"pcsExt")
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_LCS_Qos_t(XDR *xdrs, LCS_Qos_t *args)
{


	XDR_LOG(xdrs,"LCS_Qos_t ")
	if( 
		_xdr_u_char(xdrs, &args->include,"include") && 
		_xdr_u_char(xdrs, &args->horizonAcc,"horizonAcc") && 
		_xdr_u_char(xdrs, &args->vertAccuracy,"vertAccuracy") && 
		XDR_ENUM(xdrs, &(args->rspTimeCat), LCS_RspTimeCat_t) && 
		xdr_LCS_ExtContainer_t(xdrs, &args->extContainer)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_LCS_ExtAddress_t(XDR *xdrs, LCS_ExtAddress_t *args)
{


	XDR_LOG(xdrs,"LCS_ExtAddress_t ")
	if( 
		_xdr_u_char(xdrs, &args->length,"length") && 
		xdr_opaque(xdrs, (caddr_t) args->data, LCS_EXTERNAL_ADDRESS_SIZE)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_LCS_ClientExtId_t(XDR *xdrs, LCS_ClientExtId_t *args)
{


	XDR_LOG(xdrs,"LCS_ClientExtId_t ")
	if( 
		_xdr_u_char(xdrs, &args->include,"include") && 
		xdr_LCS_ExtAddress_t(xdrs, &args->extAdd) && 
		xdr_LCS_ExtContainer_t(xdrs, &args->extContainer)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_LCS_MlcNumber_t(XDR *xdrs, LCS_MlcNumber_t *args)
{


	XDR_LOG(xdrs,"LCS_MlcNumber_t ")
	if( 
		_xdr_u_char(xdrs, &args->length,"length") && 
		xdr_opaque(xdrs, (caddr_t) args->number, LCS_MLC_NUMBER_LENGTH)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_LCS_GpsAssistData_t(XDR *xdrs, LCS_GpsAssistData_t *args)
{


	XDR_LOG(xdrs,"LCS_GpsAssistData_t ")
	if( 
		_xdr_u_char(xdrs, &args->length,"length") && 
		xdr_opaque(xdrs, (caddr_t) args->data, LCS_GPS_ASSISTED_DATA_LENGTH)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_LCS_Location_t(XDR *xdrs, LCS_Location_t *args)
{


	XDR_LOG(xdrs,"LCS_Location_t ")
	if( 
		XDR_ENUM(xdrs, &(args->locEstType), LCS_LocEstimateType_t) && 
		_xdr_u_char(xdrs, &args->include,"include") && 
		_xdr_u_int16_t(xdrs, &args->defLocEvent,"defLocEvent")
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_LCS_LocEstimate_t(XDR *xdrs, LCS_LocEstimate_t *args)
{


	XDR_LOG(xdrs,"LCS_LocEstimate_t ")
	if( 
		_xdr_u_char(xdrs, &args->length,"length") && 
		xdr_opaque(xdrs, (caddr_t) args->data, LCS_LOCATION_ESTIMATE_SIZE)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_LCS_VelocityEstimate_t(XDR *xdrs, LCS_VelocityEstimate_t *args)
{


	XDR_LOG(xdrs,"LCS_VelocityEstimate_t ")
	if( 
		_xdr_u_char(xdrs, &args->length,"length") && 
		xdr_opaque(xdrs, (caddr_t) args->data, LCS_ADD_VELOCITY_ESTIMATE_SIZE)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_LCS_PerLdrInfo_t(XDR *xdrs, LCS_PerLdrInfo_t *args)
{


	XDR_LOG(xdrs,"LCS_PerLdrInfo_t ")
	if( 
		xdr_int(xdrs, &args->reportAmount) && 
		xdr_int(xdrs, &args->reportInterval)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_LCS_GanssAssistData_t(XDR *xdrs, LCS_GanssAssistData_t *args)
{


	XDR_LOG(xdrs,"LCS_GanssAssistData_t ")
	if( 
		_xdr_u_char(xdrs, &args->length,"length") && 
		xdr_opaque(xdrs, (caddr_t) args->data, LCS_GANSS_ASSISTED_DATA_LENGTH)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_LCS_MoLrReq_t(XDR *xdrs, LCS_MoLrReq_t *args)
{


	XDR_LOG(xdrs,"LCS_MoLrReq_t ")
	if( 
		XDR_ENUM(xdrs, &(args->type), LCS_MoLr_t) && 
		_xdr_u_long(xdrs, &args->include,"include") && 
		XDR_ENUM(xdrs, &(args->locMethod), LCS_LocMethod_t) && 
		xdr_LCS_Qos_t(xdrs, &args->qos) && 
		xdr_LCS_ClientExtId_t(xdrs, &args->clientExtId) && 
		xdr_LCS_MlcNumber_t(xdrs, &args->mlcNumber) && 
		xdr_LCS_GpsAssistData_t(xdrs, &args->gpsAssData) && 
		_xdr_u_char(xdrs, &args->suppGadShapes,"suppGadShapes") && 
		_xdr_u_char(xdrs, &args->srvTypeId,"srvTypeId") && 
		_xdr_u_int16_t(xdrs, &args->ageLocInfo,"ageLocInfo") && 
		xdr_LCS_Location_t(xdrs, &args->locType) && 
		xdr_LCS_HGmlcAdd_t(xdrs, &args->hGmlcAdd) && 
		xdr_LCS_LocEstimate_t(xdrs, &args->locEst) && 
		xdr_LCS_VelocityEstimate_t(xdrs, &args->velocityEst) && 
		_xdr_u_char(xdrs, &args->refNumber,"refNumber") && 
		xdr_LCS_PerLdrInfo_t(xdrs, &args->perLdrInfo) && 
		_xdr_u_char(xdrs, &args->seqNumber,"seqNumber") && 
		XDR_ENUM(xdrs, &(args->termCause), LCS_TermCause_t) && 
		xdr_LCS_GanssAssistData_t(xdrs, &args->ganssAssData)
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_LCS_MoLrRsp_t(XDR *xdrs, LCS_MoLrRsp_t *args)
{


	XDR_LOG(xdrs,"LCS_MoLrRsp_t ")
	if( 
		_xdr_u_int16_t(xdrs, &args->include,"include") && 
		xdr_LCS_LocEstimate_t(xdrs, &args->locEst) && 
		xdr_LCS_DeciphKey_t(xdrs, &args->deciphKey) && 
		xdr_LCS_AddLocEstimate_t(xdrs, &args->addLocEst) && 
		xdr_LCS_VelocityEstimate_t(xdrs, &args->velocityEst) && 
		_xdr_u_char(xdrs, &args->refNumber,"refNumber") && 
		xdr_LCS_HGmlcAdd_t(xdrs, &args->hGmlcAdd) && 
		xdr_LCS_RepPlmnList_t(xdrs, &args->rptPlmnList)
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_LCS_IdString_t(XDR *xdrs, LCS_IdString_t *args)
{


	XDR_LOG(xdrs,"LCS_IdString_t ")
	if( 
		_xdr_u_char(xdrs, &args->dcs,"dcs") && 
		_xdr_u_char(xdrs, &args->strLen,"strLen") && 
		xdr_opaque(xdrs, (caddr_t) args->str, LCS_STRING_LENGTH)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_LCS_ClientName_t(XDR *xdrs, LCS_ClientName_t *args)
{


	XDR_LOG(xdrs,"LCS_ClientName_t ")
	if( 
		xdr_LCS_IdString_t(xdrs, &args->id) && 
		_xdr_u_char(xdrs, &args->include,"include") && 
		XDR_ENUM(xdrs, &(args->formatInd), LCS_FormatIndicator_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_LCS_ReqId_t(XDR *xdrs, LCS_ReqId_t *args)
{


	XDR_LOG(xdrs,"LCS_ReqId_t ")
	if( 
		xdr_LCS_IdString_t(xdrs, &args->id) && 
		_xdr_u_char(xdrs, &args->include,"include") && 
		XDR_ENUM(xdrs, &(args->formatInd), LCS_FormatIndicator_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_LCS_LocNotifInfo_t(XDR *xdrs, LCS_LocNotifInfo_t *args)
{


	XDR_LOG(xdrs,"LCS_LocNotifInfo_t ")
	if( 
		XDR_ENUM(xdrs, &(args->notifType), LCS_Notification_t) && 
		xdr_LCS_Location_t(xdrs, &args->locType) && 
		_xdr_u_char(xdrs, &args->include,"include") && 
		xdr_LCS_ClientExtId_t(xdrs, &args->clientExtId) && 
		xdr_LCS_ClientName_t(xdrs, &args->clientName) && 
		xdr_LCS_ReqId_t(xdrs, &args->reqId) && 
		xdr_LCS_IdString_t(xdrs, &args->codeWord) && 
		_xdr_u_char(xdrs, &args->srvIdType,"srvIdType")
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_LCS_AreaDef_t(XDR *xdrs, LCS_AreaDef_t *args)
{


	XDR_LOG(xdrs,"LCS_AreaDef_t ")
	if( 
		XDR_ENUM(xdrs, &(args->areaType), LCS_Area_t) && 
		_xdr_u_char(xdrs, &args->areaIdLen,"areaIdLen") && 
		xdr_opaque(xdrs, (caddr_t) args->areaId, LCS_AREA_IDENTIFICATION_SIZE)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_LCS_AreaEventInfo_t(XDR *xdrs, LCS_AreaEventInfo_t *args)
{


	XDR_LOG(xdrs,"LCS_AreaEventInfo_t ")
	if( 
		_xdr_u_char(xdrs, &args->include,"include") && 
		_xdr_u_char(xdrs, &args->areaListSize,"areaListSize") && 
		xdr_vector(xdrs, (char *) args->areaList, LCS_AREA_LIST_SIZE, sizeof(LCS_AreaDef_t), xdr_LCS_AreaDef_t) && 
		XDR_ENUM(xdrs, &(args->occInfo), LCS_OccInfo_t) && 
		xdr_int(xdrs, &args->intTime)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_LCS_AreaEventReq_t(XDR *xdrs, LCS_AreaEventReq_t *args)
{


	XDR_LOG(xdrs,"LCS_AreaEventReq_t ")
	if( 
		_xdr_u_char(xdrs, &args->include,"include") && 
		_xdr_u_char(xdrs, &args->refNumber,"refNumber") && 
		xdr_LCS_HGmlcAdd_t(xdrs, &args->hGmlcAdd) && 
		_xdr_u_int16_t(xdrs, &args->defLocEvent,"defLocEvent") && 
		xdr_LCS_AreaEventInfo_t(xdrs, &args->areaEventInfo)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_LCS_PeriodicLocReq_t(XDR *xdrs, LCS_PeriodicLocReq_t *args)
{


	XDR_LOG(xdrs,"LCS_PeriodicLocReq_t ")
	if( 
		_xdr_u_char(xdrs, &args->include,"include") && 
		_xdr_u_char(xdrs, &args->refNumber,"refNumber") && 
		xdr_LCS_PerLdrInfo_t(xdrs, &args->perLdrInfo) && 
		xdr_LCS_ClientExtId_t(xdrs, &args->clientExtId) && 
		xdr_LCS_Qos_t(xdrs, &args->qos) && 
		xdr_LCS_HGmlcAdd_t(xdrs, &args->hGmlcAdd) && 
		xdr_LCS_RepPlmnList_t(xdrs, &args->rptPlmnList)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_LCS_XxCancel_t(XDR *xdrs, LCS_XxCancel_t *args)
{


	XDR_LOG(xdrs,"LCS_XxCancel_t ")
	if( 
		_xdr_u_char(xdrs, &args->include,"include") && 
		_xdr_u_char(xdrs, &args->refNumber,"refNumber") && 
		xdr_LCS_HGmlcAdd_t(xdrs, &args->hGmlcAdd)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_LCS_LocUpdate_t(XDR *xdrs, LCS_LocUpdate_t *args)
{


	XDR_LOG(xdrs,"LCS_LocUpdate_t ")
	if( 
		_xdr_u_char(xdrs, &args->include,"include") && 
		_xdr_u_char(xdrs, &args->refNumber,"refNumber") && 
		xdr_LCS_AddLocEstimate_t(xdrs, &args->addLocEst) && 
		xdr_LCS_VelocityEstimate_t(xdrs, &args->velocityEst) && 
		xdr_int(xdrs, &args->seqNumber)
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_LcsCpMoLrReq_t(XDR *xdrs, LcsCpMoLrReq_t *args)
{

	XDR_LOG(xdrs,"LcsCpMoLrReq_t ")
	if( 
		xdr_LCS_MoLrReq_t(xdrs, &args->moLrReq) && 
		xdr_u_long(xdrs, &args->expTime)
	  )
		return(TRUE);
	else
		return(FALSE);

}

bool_t
xdr_CAPI2_LCS_CpMoLrReq_t(XDR *xdrs, CAPI2_LCS_CpMoLrReq_t *args)
{


	XDR_LOG(xdrs,"CAPI2_LCS_CpMoLrReq_t ")
	if( 
		xdr_pointer(xdrs, (char **) &args->inClientInfoPtr, sizeof(ClientInfo_t), (xdrproc_t) xdr_ClientInfo_t) && 
		xdr_pointer(xdrs, (char **) &args->inCpMoLrReq, sizeof( LcsCpMoLrReq_t), (xdrproc_t) xdr_LcsCpMoLrReq_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t 
xdr_CAPI2_LCS_CpMoLrAbort_t(XDR *xdrs, CAPI2_LCS_CpMoLrAbort_t *args)
{


	XDR_LOG(xdrs,"CAPI2_LCS_CpMoLrAbort_t ")
	return (xdr_pointer(xdrs, (char **) &args->inClientInfoPtr, sizeof(ClientInfo_t), (xdrproc_t) xdr_ClientInfo_t) );
}
bool_t
xdr_CAPI2_LCS_CpMtLrVerificationRsp_t(XDR *xdrs, CAPI2_LCS_CpMtLrVerificationRsp_t *args)
{


	XDR_LOG(xdrs,"CAPI2_LCS_CpMtLrVerificationRsp_t ")
	if( 
		xdr_pointer(xdrs, (char **) &args->inClientInfoPtr, sizeof(ClientInfo_t), (xdrproc_t) xdr_ClientInfo_t) && 
		XDR_ENUM(xdrs, &(args->inVerificationRsp), LCS_VerifRsp_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_CAPI2_LCS_CpMtLrRsp_t(XDR *xdrs, CAPI2_LCS_CpMtLrRsp_t *args)
{


	XDR_LOG(xdrs,"CAPI2_LCS_CpMtLrRsp_t ")
	if( 
		xdr_pointer(xdrs, (char **) &args->inClientInfoPtr, sizeof(ClientInfo_t), (xdrproc_t) xdr_ClientInfo_t) && 
		XDR_ENUM(xdrs, &(args->inOperation), SS_Operation_t) && 
		_xdr_u_char(xdrs, &args->inIsAccepted,"inIsAccepted")
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_CAPI2_LCS_CpLocUpdateRsp_t(XDR *xdrs, CAPI2_LCS_CpLocUpdateRsp_t *args)
{


	XDR_LOG(xdrs,"CAPI2_LCS_CpLocUpdateRsp_t ")
	if( 
		xdr_pointer(xdrs, (char **) &args->inClientInfoPtr, sizeof(ClientInfo_t), (xdrproc_t) xdr_ClientInfo_t) && 
		xdr_pointer(xdrs, (char **) &args->inTerminationCause, sizeof( LCS_TermCause_t), (xdrproc_t) XDR_ENUM_DEF(LCS_TermCause_t))
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t 
xdr_CAPI2_LCS_DecodePosEstimate_t(XDR *xdrs, CAPI2_LCS_DecodePosEstimate_t *args)
{


	XDR_LOG(xdrs,"CAPI2_LCS_DecodePosEstimate_t ")
	return ( xdr_pointer(xdrs, (char **) &args->inLocEstData, sizeof( LCS_LocEstimate_t), (xdrproc_t) xdr_LCS_LocEstimate_t) );
}
bool_t 
xdr_CAPI2_LCS_EncodeAssistanceReq_t(XDR *xdrs, CAPI2_LCS_EncodeAssistanceReq_t *args)
{


	XDR_LOG(xdrs,"CAPI2_LCS_EncodeAssistanceReq_t ")
	return ( xdr_pointer(xdrs, (char **) &args->inAssistReq, sizeof( LcsAssistanceReq_t), (xdrproc_t) xdr_LcsAssistanceReq_t) );
}
bool_t
xdr_CAPI2_MNSS_CLIENT_LCS_SRV_RSP_t(XDR *xdrs, CAPI2_MNSS_CLIENT_LCS_SRV_RSP_t *rsp)
{

	XDR_LOG(xdrs,"CAPI2_MNSS_CLIENT_LCS_SRV_RSP_t ")
	return(xdr_pointer(xdrs, (char **) &rsp->MNSS_CLIENT_LCS_SRV_RSP_Rsp, sizeof(LCS_SrvRsp_t), (xdrproc_t) xdr_LCS_SrvRsp_t) );

}
bool_t
xdr_CAPI2_MNSS_CLIENT_LCS_SRV_IND_t(XDR *xdrs, CAPI2_MNSS_CLIENT_LCS_SRV_IND_t *rsp)
{
	return(xdr_pointer(xdrs, (char **) &rsp->MNSS_CLIENT_LCS_SRV_IND_Rsp, sizeof(LCS_SrvInd_t), (xdrproc_t) xdr_LCS_SrvInd_t) );

}
bool_t
xdr_CAPI2_MNSS_CLIENT_LCS_SRV_REL_t(XDR *xdrs, CAPI2_MNSS_CLIENT_LCS_SRV_REL_t *rsp)
{
	return(xdr_pointer(xdrs, (char **) &rsp->MNSS_CLIENT_LCS_SRV_REL_Rsp, sizeof(SS_SrvRel_t), (xdrproc_t) xdr_SS_SrvRel_t) );

}
bool_t
xdr_CAPI2_MNSS_CLIENT_SS_SRV_RSP_t(XDR *xdrs, CAPI2_MNSS_CLIENT_SS_SRV_RSP_t *rsp)
{
	return(xdr_pointer(xdrs, (char **) &rsp->MNSS_CLIENT_SS_SRV_RSP_Rsp, sizeof(SS_SrvRsp_t), (xdrproc_t) xdr_SS_SrvRsp_t) );

}
bool_t
xdr_CAPI2_MNSS_CLIENT_SS_SRV_REL_t(XDR *xdrs, CAPI2_MNSS_CLIENT_SS_SRV_REL_t *rsp)
{
	return(xdr_pointer(xdrs, (char **) &rsp->MNSS_CLIENT_SS_SRV_REL_Rsp, sizeof(SS_SrvRel_t), (xdrproc_t) xdr_SS_SrvRel_t) );

}
bool_t
xdr_CAPI2_MNSS_CLIENT_SS_SRV_IND_t(XDR *xdrs, CAPI2_MNSS_CLIENT_SS_SRV_IND_t *rsp)
{
	return(xdr_pointer(xdrs, (char **) &rsp->MNSS_CLIENT_SS_SRV_IND_Rsp, sizeof(SS_SrvInd_t), (xdrproc_t) xdr_SS_SrvInd_t) );

}
bool_t
xdr_CAPI2_MNSS_SATK_SS_SRV_RSP_t(XDR *xdrs, CAPI2_MNSS_SATK_SS_SRV_RSP_t *rsp)
{
	return(xdr_pointer(xdrs, (char **) &rsp->MNSS_SATK_SS_SRV_RSP_Rsp, sizeof(STK_SsSrvRel_t), (xdrproc_t) xdr_STK_SsSrvRel_t) );

}
