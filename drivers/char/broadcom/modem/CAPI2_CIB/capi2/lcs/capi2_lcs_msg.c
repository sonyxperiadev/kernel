//*********************************************************************
//
//	Copyright © 2004-2008 Broadcom Corporation
//	
//	This program is the proprietary software of Broadcom Corporation 
//	and/or its licensors, and may only be used, duplicated, modified 
//	or distributed pursuant to the terms and conditions of a separate, 
//	written license agreement executed between you and Broadcom (an 
//	"Authorized License").  Except as set forth in an Authorized 
//	License, Broadcom grants no license (express or implied), right 
//	to use, or waiver of any kind with respect to the Software, and 
//	Broadcom expressly reserves all rights in and to the Software and 
//	all intellectual property rights therein.  IF YOU HAVE NO 
//	AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE 
//	IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE 
//	ALL USE OF THE SOFTWARE.  
//	
//	Except as expressly set forth in the Authorized License,
//	
//	1.	This program, including its structure, sequence and 
//		organization, constitutes the valuable trade secrets 
//		of Broadcom, and you shall use all reasonable efforts 
//		to protect the confidentiality thereof, and to use 
//		this information only in connection with your use 
//		of Broadcom integrated circuit products.
//	
//	2.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE 
//		IS PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM 
//		MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, 
//		EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, 
//		WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY 
//		DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, 
//		MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A 
//		PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR 
//		COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR 
//		CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE 
//		RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.  
//
//	3.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT 
//		SHALL BROADCOM OR ITS LICENSORS BE LIABLE FOR 
//		(i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
//		EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY 
//		WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE 
//		SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE 
//		POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN 
//		EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE 
//		ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
//		LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE 
//		OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//***************************************************************************
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



#define _xdr_char(a,b,c) xdr_char(a,b)
//#define _xdr_u_char(a,b,c) xdr_u_char(a,b)
//#define _xdr_u_long(a,b,c) _xdr_u_int32_t(a,b,c)
#define _xdr_u_int32_t(a,b,c) xdr_u_long(a,b)
#define _xdr_short(a,b,c) xdr_short(a,b)
#define _xdr_u_short(a,b,c) xdr_u_short(a,b)
#define _xdr_double(a,b,c) xdr_double(a,b)
#define _xdr_float(a,b,c) xdr_float(a,b)
#define _xdr_long(a,b,c)  xdr_long(a,b)

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

#if defined(UMTS) || defined(CPLANE_RRC_API_INCLUDED)
bool_t xdr_T_LBS_UMTS_MeasCell(XDR* xdrs, T_LBS_UMTS_MeasCell* data)
{
	XDR_LOG(xdrs,"T_LBS_UMTS_MeasCell")

	if( xdr_UInt32(xdrs, &data->cell_identity) &&
//		_xdr_u_int16_t(xdrs, &data->uarfcn_DL, "uarfcn_DL") &&
//		_xdr_u_int16_t(xdrs, &data->uarfcn_UL, "uarfcn_UL") &&
		_xdr_u_int16_t(xdrs, &data->psc, "psc") &&
		_xdr_Int8(xdrs, (char *)&data->rscp, "rscp") && 
		_xdr_u_int16_t(xdrs, &data->pathloss, "pathloss") &&
		_xdr_Int8(xdrs, (char *)&data->ecno, "ecno")
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
		_xdr_Int8(xdrs, (char *)&data->rssi, "rssi") && 
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
#if defined(UMTS) || defined(CPLANE_RRC_API_INCLUDED)
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
#if defined(UMTS) || defined(CPLANE_RRC_API_INCLUDED)
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

