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
*   @file   capi2_phonectrl_msg.c
*
*   @brief  This file implements the serialize/deserialize the request/respose
*	for phonectrl.
*
****************************************************************************/
#include	"capi2_reqrep.h"
#include	"capi2_phonectrl_msg.h"
#include	"capi2_pch_msg.h"


XDR_ENUM_DECLARE(PlmnCause_t)
XDR_ENUM_DECLARE(MSNwOperationMode_t)
XDR_ENUM_DECLARE(MSNwType_t)
XDR_ENUM_DECLARE(Service_State_t)

XDR_ENUM_FUNC(PowerOnCause_t)
XDR_ENUM_FUNC(RATSelect_t)
XDR_ENUM_FUNC(BandSelect_t)

XDR_ENUM_FUNC(PlmnSelectFormat_t)
XDR_ENUM_FUNC(PlmnSelectMode_t)
XDR_ENUM_FUNC(SystemState_t)
XDR_ENUM_FUNC(RegisterStatus_t)
XDR_ENUM_FUNC(PCHRejectCause_t)
XDR_ENUM_FUNC(PlmnCause_t)
XDR_ENUM_FUNC(TimeZoneUpdateMode_t)
XDR_ENUM_FUNC(SysFilterEnable_t)

XDR_ENUM_FUNC(HAL_EM_BATTMGR_Result_en_t)
XDR_ENUM_FUNC(HAL_EM_BATTMGR_Event_en_t)
XDR_ENUM_FUNC(HAL_EM_BATTMGR_Charger_t)
XDR_ENUM_FUNC(HAL_EM_BATTMGR_Charger_InOut_t)
XDR_ENUM_FUNC(MS_Element_t)
XDR_ENUM_FUNC(MSNwOperationMode_t)
XDR_ENUM_FUNC(MSNwType_t)

XDR_ENUM_FUNC(SpeakerTone_t)
XDR_ENUM_FUNC(AudioMode_t)

XDR_ENUM_FUNC(PLMNNameType_t) 
XDR_ENUM_FUNC(PchRespType_t)
XDR_ENUM_FUNC(Service_State_t)

XDR_ENUM_FUNC(EM_BATTMGR_ChargingStatus_en_t)


extern bool_t xdr_PLMNId_t( XDR* xdrs, PLMNId_t* data);
bool_t xdr_PLMN_t( XDR* xdrs, PLMN_t* data);
bool_t xdr_SEARCHED_PLMNId_t( XDR* xdrs, SEARCHED_PLMNId_t* data);
bool_t xdr_MSSimIccId(XDR* xdrs, ICCID_ASCII_t *data);
bool_t xdr_MSSimRawIccId(XDR* xdrs, ICCID_BCD_t *data);
bool_t xdr_MSSimCphsOns(XDR* xdrs, CPHS_ONS_t *data);
bool_t xdr_MSSimCphsOnss(XDR* xdrs, CPHS_ONSS_t *data);
bool_t xdr_MSSimSst(XDR* xdrs, SST_t *data);
bool_t xdr_MSSimCphsInfo(XDR* xdrs, CPHS_INFO_t *data);
bool_t xdr_CAPI2_Ex_MS_Element_t(XDR* xdrs, void *rsp, MS_Element_t inElemType);
bool_t xdr_MSSimEccRecList(XDR* xdrs, ECC_REC_LIST_t *data);


bool_t xdr_MsPlmnInfo_t(XDR *xdrs, MsPlmnInfo_t* data)
{
	XDR_LOG(xdrs,"MsPlmnInfo_t")

	if( _xdr_u_char(xdrs,  &data->matchResult,"matchResult") &&
		_xdr_u_int16_t(xdrs, &data->mcc,"mcc") &&
		_xdr_u_int16_t(xdrs, &data->mnc,"mnc") &&
		xdr_PLMN_NAME_t(xdrs,  &data->longName) &&
		xdr_PLMN_NAME_t(xdrs,  &data->shortName) &&
		xdr_PLMN_NAME_t(xdrs,  &data->countryName) 
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_PLMN_NAME_t(XDR *xdrs, PLMN_NAME_t* ptr)
{
	XDR_LOG(xdrs,"PLMN_NAME_t")

	if( XDR_ENUM(xdrs,  &ptr->coding, ALPHA_CODING_t) &&
		_xdr_u_char(xdrs,  &ptr->name_size,"name_size") &&
		xdr_opaque(xdrs, (caddr_t)ptr->name, PLMN_NAME_SIZE) &&
		XDR_ENUM(xdrs, &ptr->nameType, PLMNNameType_t) &&
		_xdr_Boolean(xdrs, &ptr->is_pnn_1st_rec, "is_pnn_1st_rec")
		)
		return(TRUE);
	else
		return(FALSE);

}

bool_t xdr_MsPlmnName_t(XDR *xdrs, MsPlmnName_t* ptr)
{
	XDR_LOG(xdrs,"MsPlmnName_t")

	if( _xdr_u_char(xdrs,  &ptr->matchResult,"matchResult") &&
		xdr_PLMN_NAME_t(xdrs,  &ptr->longName) &&
		xdr_PLMN_NAME_t(xdrs,  &ptr->shortName) 
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_PLMN_NAME_PTR_t(XDR *xdrs, PLMN_NAME_PTR_t* ptr)
{
	XDR_LOG(xdrs,"PLMN_NAME_PTR_t")

	return xdr_pointer(xdrs, (char **)(void*)&ptr, sizeof(PLMN_NAME_t), xdr_PLMN_NAME_t);
}

bool_t xdr_CAPI2_MS_ConvertPLMNNameStr_t(XDR *xdrs, CAPI2_MS_ConvertPLMNNameStr_t* ptr)
{
	XDR_LOG(xdrs,"CAPI2_MS_ConvertPLMNNameStr_t")

	if( _xdr_u_char(xdrs,  &ptr->ucs2,"ucs2") &&
		xdr_PLMN_NAME_t(xdrs, &ptr->plmn_name)
		)
		return(TRUE);
	else
		return(FALSE);

}

bool_t 
xdr_MsRxLevelData_t( XDR* xdrs, MsRxLevelData_t* data)
{
	XDR_LOG(xdrs,"MsRxLevelData_t")

	if( _xdr_u_char(xdrs, &data->RxLev,"RxLev")	&&
		_xdr_u_char(xdrs, &data->RxQual,"RxQual")
		)
		return(TRUE);
	else
		return(FALSE);
}

XDR_ENUM_FUNC(MSRegState_t)
XDR_ENUM_FUNC(MSNetAccess_t)
	
bool_t 
xdr_MSNetworkInfo_t( XDR* xdrs, MSNetworkInfo_t* data)
{
	XDR_LOG(xdrs,"MSNetworkInfo_t")

	if( _xdr_u_char(xdrs, &data->rat,"rat")	&&
		_xdr_u_char(xdrs, &data->bandInfo,"bandInfo")	&&
		XDR_ENUM(xdrs,  &data->msc_r99, MSNetAccess_t) &&
		XDR_ENUM(xdrs,  &data->sgsn_r99, MSNetAccess_t) &&
		XDR_ENUM(xdrs,  &data->gprs_supported, MSNetAccess_t) &&
		XDR_ENUM(xdrs,  &data->egprs_supported, MSNetAccess_t)&&
        XDR_ENUM(xdrs,  &data->dtm_supported, MSNetAccess_t)&&
        XDR_ENUM(xdrs,  &data->hsdpa_supported, MSNetAccess_t)&&
        XDR_ENUM(xdrs,  &data->hsupa_supported, MSNetAccess_t)&&
        XDR_ENUM(xdrs,  &data->nom, MSNwOperationMode_t)&&
		XDR_ENUM(xdrs,  &data->network_type, MSNwType_t)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_MSRegInfo_t( XDR* xdrs, MSRegInfo_t* data)
{
	XDR_LOG(xdrs,"MSRegInfo_t")

	if( XDR_ENUM(xdrs,  &data->regState, MSRegState_t) &&
		XDR_ENUM(xdrs,  &data->cause, NetworkCause_t) &&
		_xdr_u_int16_t(xdrs, &data->lac,"lac") &&
		_xdr_u_int16_t(xdrs, &data->cell_id,"cell_id") &&
		_xdr_u_int16_t(xdrs, &data->mcc,"mcc") &&
		_xdr_u_char(xdrs, &data->mnc,"mnc")	&&
		_xdr_u_char(xdrs, &data->netCause,"netCause")	&&
		xdr_MSNetworkInfo_t(xdrs, &data->netInfo) &&
		XDR_ENUM(xdrs,  &data->pchType, PchRespType_t) &&
		_xdr_u_char(xdrs, &data->rac,"rac") &&
		_xdr_u_int16_t(xdrs, &data->rncId, "rncId")
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_nitzNetworkName_t( XDR* xdrs, nitzNetworkName_t* data)
{
	XDR_LOG(xdrs,"nitzNetworkName_t")

	if( xdr_opaque(xdrs, (caddr_t)&data->longName, 255) &&
		xdr_opaque(xdrs, (caddr_t)&data->shortName, 255)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_RxSignalInfo_t( XDR* xdrs, RxSignalInfo_t* data)
{
	XDR_LOG(xdrs,"RxSignalInfo_t")

	if( _xdr_u_char(xdrs, &data->rssi,"rssi")	&&
		_xdr_u_char(xdrs, &data->qual,"qual")
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_RX_SIGNAL_INFO_CHG_t( XDR* xdrs, RX_SIGNAL_INFO_CHG_t* data)
{
	XDR_LOG(xdrs,"RX_SIGNAL_INFO_CHG_t")

	if( _xdr_u_char(xdrs, &data->signal_lev_changed,"signal_lev_changed")	&&
		_xdr_u_char(xdrs, &data->signal_qual_changed,"signal_qual_changed")
		)
		return(TRUE);
	else
		return(FALSE);
}


bool_t 
xdr_RTCTime_t( XDR* xdrs, RTCTime_t* data)
{
	XDR_LOG(xdrs,"RTCTime_t")

	if( _xdr_u_char(xdrs, &data->Sec,"Sec")	&&
		_xdr_u_char(xdrs, &data->Min,"Min")	&&
		_xdr_u_char(xdrs, &data->Hour,"Hour")	&&
		_xdr_u_char(xdrs, &data->Week,"Week")	&&
		_xdr_u_char(xdrs, &data->Day,"Day")	&&
		_xdr_u_char(xdrs, &data->Month,"Month")	&&
		_xdr_u_int16_t(xdrs, &data->Year,"Year")
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_TimeZoneDate_t( XDR* xdrs, TimeZoneDate_t* data)
{
	XDR_LOG(xdrs,"TimeZoneDate_t")

	if( xdr_char(xdrs, (char*)&data->timeZone)	&&
		_xdr_u_char(xdrs, &data->dstAdjust,"dstAdjust") &&
		xdr_RTCTime_t(xdrs, &data->adjustedTime))
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SEARCHED_PLMNId_t( XDR* xdrs, SEARCHED_PLMNId_t* data)
{
	XDR_LOG(xdrs,"SEARCHED_PLMNId_t")

	if( _xdr_u_int16_t(xdrs, &data->mcc,"mcc")	&&
		_xdr_u_int16_t(xdrs, &data->lac,"lac") &&
		_xdr_u_char(xdrs, &data->mnc,"mnc") &&
		_xdr_u_char(xdrs, &data->is_forbidden,"is_forbidden") &&
		_xdr_u_char(xdrs, &data->rat,"rat")&&
		XDR_ENUM(xdrs, &data->network_type, MSNwType_t) &&
		xdr_MsPlmnName_t(xdrs, &data->ucs2Name) &&
		xdr_MsPlmnName_t(xdrs, &data->nonUcs2Name)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SEARCHED_PLMN_LIST_t( XDR* xdrs, SEARCHED_PLMN_LIST_t* data)
{
	XDR_LOG(xdrs,"SEARCHED_PLMN_LIST_t")

	if( xdr_vector(xdrs,  (char*)data->searched_plmn, MAX_PLMN_SEARCH, sizeof(SEARCHED_PLMNId_t), (xdrproc_t)xdr_SEARCHED_PLMNId_t)	&&
		_xdr_u_char(xdrs, &data->num_of_plmn,"num_of_plmn") &&
		XDR_ENUM(xdrs,  &data->plmn_cause, PlmnCause_t))
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_AtResponse_t( XDR* xdrs, AtResponse_t* data)
{
	XDR_LOG(xdrs,"AtResponse_t")

	if(xdr_int16_t(xdrs, &data->len))
	{
		u_int len = (u_int)data->len;
		return (xdr_bytes(xdrs, (char **)(void*)&data->buffer, &len, 0xFFFF) &&
				_xdr_u_char(xdrs, &data->IsUnsolicited,"IsUnsolicited") &&
				_xdr_u_char(xdrs, &data->chan,"chan") );

	}

	return(FALSE);
}

bool_t 
xdr_MSImeiStr_t( XDR* xdrs, MSImeiStr_t* data)
{
	XDR_LOG(xdrs,"MSImeiStr_t")

	return xdr_opaque(xdrs, (caddr_t)&data->imei_str, MAX_IMEI_STR );
}

bool_t 
xdr_MSRegStateInfo_t( XDR* xdrs, MSRegStateInfo_t* data)
{
	XDR_LOG(xdrs,"MSRegStateInfo_t")

	if( XDR_ENUM(xdrs,  &data->gsm_reg_state, MSRegState_t) &&
		XDR_ENUM(xdrs,  &data->gprs_reg_state, MSRegState_t) &&
		_xdr_u_int16_t(xdrs, &data->mcc,"mcc") &&
		_xdr_u_char(xdrs, &data->mnc,"mnc")	&&
		_xdr_u_int16_t(xdrs, &data->lac,"lac") &&
		_xdr_u_int16_t(xdrs, &data->cell_id,"cell_id") &&
		_xdr_u_char(xdrs, &data->rat,"rat")	&&
		_xdr_u_char(xdrs, &data->band,"band")
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_CAPI2_SYS_SetRegisteredEventMask_Req_t(void* xdrs, CAPI2_SYS_SetRegisteredEventMask_Req_t *data)
{
	bool_t result = _xdr_u_char(xdrs, &data->maskLen,"maskLen");
	u_int len = (u_int)data->maskLen;

	if(result)
		result = xdr_array(xdrs,(char **)(void*) &data->maskList,&len,~0,sizeof(UInt16),(xdrproc_t) xdr_u_int16_t);

	return result;
}


bool_t xdr_CAPI2_SYS_SetFilteredEventMask_Req_t(void* xdrs, CAPI2_SYS_SetFilteredEventMask_Req_t *data)
{
	bool_t result = _xdr_u_char(xdrs, &data->maskLen,"maskLen");
	u_int len = (u_int)data->maskLen;

	if(result)
		result = xdr_array(xdrs,(char **)(void*) &data->maskList,&len,~0,sizeof(UInt16),(xdrproc_t) xdr_u_int16_t);
	
	if(result)
		result = XDR_ENUM(xdrs,  &data->enableFlag, SysFilterEnable_t);

	return result;
}

bool_t 
xdr_CAPI2_Class_t( XDR* xdrs, CAPI2_Class_t* data)
{
	XDR_LOG(xdrs,"CAPI2_Class_t")

	if( _xdr_u_int16_t(xdrs, &data->pgsm_supported,"pgsm_supported")	&&
		_xdr_u_int16_t(xdrs, &data->egsm_supported,"egsm_supported") &&
		_xdr_u_int16_t(xdrs, &data->dcs_supported,"dcs_supported") &&
		_xdr_u_int16_t(xdrs, &data->pcs_supported,"pcs_supported") &&
		_xdr_u_int16_t(xdrs, &data->gsm850_supported,"gsm850_supported") &&
		_xdr_u_int16_t(xdrs, &data->pgsm_pwr_class,"pgsm_pwr_class") &&
		_xdr_u_int16_t(xdrs, &data->egsm_pwr_class,"egsm_pwr_class") &&
		_xdr_u_int16_t(xdrs, &data->dcs_pwr_class,"dcs_pwr_class") &&
		_xdr_u_int16_t(xdrs, &data->pcs_pwr_class,"pcs_pwr_class") &&
		_xdr_u_int16_t(xdrs, &data->ms_class_hscsd,"ms_class_hscsd") &&
		_xdr_u_int16_t(xdrs, &data->gsm850_pwr_class,"gsm850_pwr_class")
	)
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_CAPI2_SYSPARM_IMEI_PTR_t( XDR* xdrs, CAPI2_SYSPARM_IMEI_PTR_t* data )
{
	u_int len = SYS_IMEI_LEN;
	
	XDR_LOG(xdrs,"CAPI2_SYSPARM_IMEI_PTR_t")

	return ( xdr_bytes(xdrs, (char **)(void*) data, &len, 512) );
}

bool_t 
xdr_HAL_EM_BatteryLevel_t( XDR* xdrs, HAL_EM_BatteryLevel_t* data)
{
	XDR_LOG(xdrs,"HAL_EM_BatteryLevel_t")

	if(	XDR_ENUM(xdrs,  &data->eventType, HAL_EM_BATTMGR_Event_en_t) &&
		_xdr_u_char(xdrs, &data->inLevel,"inLevel")	&&
		_xdr_u_int16_t(xdrs, &data->inAdc_avg,"inAdc_avg") &&
		_xdr_u_char(xdrs, &data->inTotal_levels,"inTotal_levels")
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_MSSimIccId(XDR* xdrs, ICCID_ASCII_t *data)
{
  
	XDR_LOG(xdrs,"xdr_MSSimIccId")

    return xdr_opaque(xdrs, (caddr_t)data, sizeof(ICCID_ASCII_t));

}

bool_t xdr_MSSimRawIccId(XDR* xdrs, ICCID_BCD_t *data)
{
    
	XDR_LOG(xdrs,"xdr_MSSimRawIccId")

    return xdr_opaque(xdrs, (caddr_t)data, sizeof(ICCID_BCD_t));
}

bool_t xdr_MSSimCphsOns(XDR* xdrs, CPHS_ONS_t *data)
{

	XDR_LOG(xdrs,"xdr_MSSimCphsOns")

    return xdr_opaque(xdrs, (caddr_t)data, sizeof(CPHS_ONS_t));

}

bool_t xdr_MSSimCphsOnss(XDR* xdrs, CPHS_ONSS_t *data)
{

	XDR_LOG(xdrs,"xdr_MSSimCphsOnss")

    return xdr_opaque(xdrs, (caddr_t)data, sizeof(CPHS_ONSS_t));

}

bool_t xdr_MSSimSst(XDR* xdrs, SST_t *data)
{

	XDR_LOG(xdrs,"xdr_MSSimSst")

    return xdr_opaque(xdrs, (caddr_t)data, sizeof(SST_t));

}

bool_t xdr_MSSimCphsInfo(XDR* xdrs, CPHS_INFO_t *data)
{

	XDR_LOG(xdrs,"xdr_MSSimCphsInfo")

    return xdr_opaque(xdrs, (caddr_t)data, sizeof(CPHS_INFO_t));

}

bool_t xdr_MSSimEccRecList(XDR* xdrs, ECC_REC_LIST_t *data)
{

	XDR_LOG(xdrs,"xdr_MSSimEccRecList")

    return xdr_opaque(xdrs, (caddr_t)data, sizeof(ECC_REC_LIST_t));

}

bool_t
xdr_GASConfigParam_t(XDR *xdrs, GASConfigParam_t *args)
{


	XDR_LOG(xdrs,"GASConfigParam_t ")
	if( 
		_xdr_u_char(xdrs, &args->auto_pwr_updated,"auto_pwr_updated") && 
		_xdr_u_char(xdrs, &args->auto_pwr_red,"auto_pwr_red") && 
		_xdr_u_char(xdrs, &args->egprs_updated,"egprs_updated") && 
		_xdr_u_char(xdrs, &args->egprs_support,"egprs_support") && 
		_xdr_u_char(xdrs, &args->egprs_msc_updated,"egprs_msc_updated") && 
		_xdr_u_char(xdrs, &args->egprs_msc,"egprs_msc") && 
		_xdr_u_char(xdrs, &args->gprs_msc_updated,"gprs_msc_updated") && 
		_xdr_u_char(xdrs, &args->gprs_msc,"gprs_msc") && 
		_xdr_u_char(xdrs, &args->rep_acch_updated,"rep_acch_updated") && 
		_xdr_u_char(xdrs, &args->rep_acch,"rep_acch") && 
		_xdr_u_char(xdrs, &args->power_class_8psk_updated,"power_class_8psk_updated") && 
		_xdr_u_char(xdrs, &args->power_class_8psk,"power_class_8psk") && 
		_xdr_u_char(xdrs, &args->power_cap_8psk_updated,"power_cap_8psk_updated") && 
		_xdr_u_char(xdrs, &args->power_cap_8psk,"power_cap_8psk") && 
		_xdr_u_char(xdrs, &args->geran_fp1_updated,"geran_fp1_updated") && 
		_xdr_u_char(xdrs, &args->geran_fp1,"geran_fp1") && 
		_xdr_u_char(xdrs, &args->u3g_ccn_updated,"u3g_ccn_updated") && 
		_xdr_u_char(xdrs, &args->u3g_ccn_support,"u3g_ccn_support") && 
		_xdr_u_char(xdrs, &args->nondrx_timer_updated,"nondrx_timer_updated") && 
		_xdr_u_char(xdrs, &args->nondrx_timer,"nondrx_timer") && 
		_xdr_u_char(xdrs, &args->split_pg_cycle_updated,"split_pg_cycle_updated") && 
		_xdr_u_char(xdrs, &args->split_pg_cycle,"split_pg_cycle") && 
		_xdr_u_char(xdrs, &args->eda_support_updated,"eda_support_updated") && 
		_xdr_u_char(xdrs, &args->eda_support,"eda_support") && 
		_xdr_u_char(xdrs, &args->darp_support_updated,"darp_support_updated") && 
		_xdr_u_char(xdrs, &args->darp_support,"darp_support")
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_UASConfigParam_t(XDR *xdrs, UASConfigParam_t *args)
{


	XDR_LOG(xdrs,"UASConfigParam_t ")
	if( 
		_xdr_u_char(xdrs, &args->integrity_updated,"integrity_updated") && 
		_xdr_u_char(xdrs, &args->integrity_support,"integrity_support") && 
		_xdr_u_char(xdrs, &args->ciphering_updated,"ciphering_updated") && 
		_xdr_u_char(xdrs, &args->ciphering_support,"ciphering_support") && 
		_xdr_u_char(xdrs, &args->urrc_version_updated,"urrc_version_updated") && 
		_xdr_u_char(xdrs, &args->urrc_version,"urrc_version") && 
		_xdr_u_char(xdrs, &args->fake_security_updated,"fake_security_updated") && 
		_xdr_u_char(xdrs, &args->fake_security,"fake_security") && 
		_xdr_u_char(xdrs, &args->interrat_nacc_updated,"interrat_nacc_updated") && 
		_xdr_u_char(xdrs, &args->interrat_nacc_support,"interrat_nacc_support")
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_NASConfigParam_t(XDR *xdrs, NASConfigParam_t *args)
{


	XDR_LOG(xdrs,"NASConfigParam_t ")
	if( 
		_xdr_u_char(xdrs, &args->a5_alg_updated,"a5_alg_updated") && 
		_xdr_u_char(xdrs, &args->a5_alg_supp,"a5_alg_supp") && 
		_xdr_u_char(xdrs, &args->gea_alg_updated,"gea_alg_updated") && 
		_xdr_u_char(xdrs, &args->gea_alg_supp,"gea_alg_supp") && 
		_xdr_u_char(xdrs, &args->qos_ext_updated,"qos_ext_updated") && 
		_xdr_u_char(xdrs, &args->qos_ext,"qos_ext") && 
		_xdr_u_char(xdrs, &args->acq_order_updated,"acq_order_updated") && 
		_xdr_u_char(xdrs, &args->rat_acq_order,"rat_acq_order")
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_NVRAMClassmark_t(XDR *xdrs, NVRAMClassmark_t *args)
{


	XDR_LOG(xdrs,"NVRAMClassmark_t ")
	if( 
		xdr_GASConfigParam_t(xdrs, &args->gasConfigParams) && 
		xdr_UASConfigParam_t(xdrs, &args->uasConfigParams) && 
		xdr_NASConfigParam_t(xdrs, &args->nasConfigParams)
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_MS_LOCAL_PHCTRL_ELEM_CGEREP(XDR* xdrs, void *rsp)
{
	XDR_LOG(xdrs,"xdr_MS_LOCAL_PHCTRL_ELEM_CGEREP ")
	return xdr_vector(xdrs,  (char*)rsp, 2, sizeof(UInt8), (xdrproc_t)xdr_u_char);
}

bool_t xdr_MS_LOCAL_PHCTRL_ELEM_CPWC(XDR* xdrs, void *rsp)
{
	XDR_LOG(xdrs,"xdr_MS_LOCAL_PHCTRL_ELEM_CPWC ")
	return xdr_vector(xdrs,  (char*)rsp, 3, sizeof(UInt8), (xdrproc_t)xdr_u_char);
}

bool_t xdr_MS_LOCAL_PHCTRL_ELEM_CSCS(XDR* xdrs, void *rsp)
{
	XDR_LOG(xdrs,"xdr_MS_LOCAL_PHCTRL_ELEM_CSCS ")
	return xdr_vector(xdrs,  (char*)rsp, 10, sizeof(UInt8), (xdrproc_t)xdr_u_char);
}

bool_t xdr_MS_LOCAL_PHCTRL_ELEM_IMEI(XDR* xdrs, void *rsp)
{
	XDR_LOG(xdrs,"xdr_MS_LOCAL_PHCTRL_ELEM_IMEI ")
	return xdr_vector(xdrs,  (char*)rsp, 15, sizeof(UInt8), (xdrproc_t)xdr_u_char);
}

bool_t xdr_MS_LOCAL_PHCTRL_ELEM_SW_VERSION(XDR* xdrs, void *rsp)
{
	XDR_LOG(xdrs,"xdr_MS_LOCAL_PHCTRL_ELEM_SW_VERSION ")
	return xdr_vector(xdrs,  (char*)rsp, 2, sizeof(UInt8), (xdrproc_t)xdr_u_char);
}

#define _T(a) a


//Simple elements
static const struct xdr_discrim CAPI2_MSElement_t_dscrm[] = {
		{ (int)MS_LOCAL_SS_ELEM_CLIP,_T("MS_LOCAL_SS_ELEM_CLIP"),		(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0 },
		{ (int)MS_LOCAL_SS_ELEM_CLIR,_T("MS_LOCAL_SS_ELEM_CLIR"),		(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0  },
		{ (int)MS_LOCAL_SS_ELEM_COLP,_T("MS_LOCAL_SS_ELEM_COLP"),		(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0  },
		{ (int)MS_LOCAL_SS_ELEM_CCWA,_T("MS_LOCAL_SS_ELEM_CCWA"),		(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0  },
		{ (int)MS_LOCAL_SS_ELEM_INVOKE_ID,_T("MS_LOCAL_SS_ELEM_INVOKE_ID"),		(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0  },

		{ (int)MS_LOCAL_TEST_ELEM_PROD_MODE,_T("MS_LOCAL_TEST_ELEM_PROD_MODE"),(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0  },
		{ (int)MS_LOCAL_CC_ELEM_VIDEO_CALL,_T("MS_LOCAL_CC_ELEM_VIDEO_CALL"),	(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0  },
		
		{ (int)MS_LOCAL_TEST_ELEM_CHAN,_T("MS_LOCAL_TEST_ELEM_CHAN"),	(xdrproc_t)xdr_MS_TestChan_t, (sizeof( MS_TestChan_t ) + MAX_TEST_ARFCNS*2), NULL_capi2_proc_t, (xdrproc_t)xdr_MS_TestChan_t ,0 },

		{ (int)MS_LOCAL_CC_ELEM_AUDIO,_T("MS_LOCAL_CC_ELEM_AUDIO"),								(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char ,0 },
		{ (int)MS_LOCAL_CC_ELEM_CLIENT_GEN_TONE,_T("MS_LOCAL_CC_ELEM_CLIENT_GEN_TONE"),								(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char ,0 },

		{ (int)MS_LOCAL_MS_ELEM_DIALOG_ID,_T("MS_LOCAL_MS_ELEM_DIALOG_ID"),								(xdrproc_t)xdr_u_int32_t, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_int32_t ,0 },

		{ (int)MS_LOCAL_CC_ELEM_IS_HSCSD_CALL,_T("MS_LOCAL_CC_ELEM_IS_HSCSD_CALL"),								(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char ,0 },

		{ (int)MS_LOCAL_CC_ELEM_RX_SLOTS,_T("MS_LOCAL_CC_ELEM_RX_SLOTS"),								(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0 },
		{ (int)MS_LOCAL_CC_ELEM_TX_SLOTS,_T("MS_LOCAL_CC_ELEM_TX_SLOTS"),								(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0 },
		{ (int)MS_LOCAL_CC_ELEM_AIR_IF_USER_RATE,_T("MS_LOCAL_CC_ELEM_AIR_IF_USER_RATE"),								(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0 },
		{ (int)MS_LOCAL_CC_ELEM_CURRENT_CODING,_T("MS_LOCAL_CC_ELEM_CURRENT_CODING"),								(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0 },
		{ (int)MS_LOCAL_CC_ELEM_CURRENT_CONNECT_ELEMENT,_T("MS_LOCAL_CC_ELEM_CURRENT_CONNECT_ELEMENT"),								(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0 },
		{ (int)MS_LOCAL_CC_ELEM_DS_REQ_SUCCESS_NEG,_T("MS_LOCAL_CC_ELEM_DS_REQ_SUCCESS_NEG"),								(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0 },
		{ (int)MS_LOCAL_CC_ELEM_MCLASS,_T("MS_LOCAL_CC_ELEM_MCLASS"),								(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0 },
		{ (int)MS_LOCAL_CC_ELEM_MAX_RX_SLOTS,_T("MS_LOCAL_CC_ELEM_MAX_RX_SLOTS"),								(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0 },
		{ (int)MS_LOCAL_CC_ELEM_MAX_TX_SLOTS,_T("MS_LOCAL_CC_ELEM_MAX_TX_SLOTS"),								(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0 },
		{ (int)MS_LOCAL_CC_ELEM_SUM_SLOTS,_T("MS_LOCAL_CC_ELEM_SUM_SLOTS"),								(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0 },
		{ (int)MS_LOCAL_CC_ELEM_CODINGS,_T("MS_LOCAL_CC_ELEM_CODINGS"),								(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0 },
		{ (int)MS_LOCAL_CC_ELEM_WANTED_RX_SLOTS,_T("MS_LOCAL_CC_ELEM_WANTED_RX_SLOTS"),								(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0 },
		{ (int)MS_LOCAL_CC_ELEM_WANTED_AIR_USER_RATE,_T("MS_LOCAL_CC_ELEM_WANTED_AIR_USER_RATE"),								(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0 },
		{ (int)MS_LOCAL_CC_ELEM_MAX_TX_SLOTS_SUPPORTED,_T("MS_LOCAL_CC_ELEM_MAX_TX_SLOTS_SUPPORTED"),								(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0 },
//		{ (int)MS_LOCAL_CC_ELEM_CURR_TI_PD,_T("MS_LOCAL_CC_ELEM_CURR_TI_PD"),								(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0 },
//		{ (int)MS_LOCAL_CC_ELEM_CURR_SERVICE_MODE,_T("MS_LOCAL_CC_ELEM_CURR_SERVICE_MODE"),								(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0 },
//		{ (int)MS_LOCAL_CC_ELEM_DS_DATACOMP_IND,_T("MS_LOCAL_CC_ELEM_DS_DATACOMP_IND"),								(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0 },
//		{ (int)MS_LOCAL_CC_ELEM_CUGI,_T("MS_LOCAL_CC_ELEM_CUGI"),								(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0 },
		{ (int)MS_LOCAL_CC_ELEM_SPKR_LOUDNESS,_T("MS_LOCAL_CC_ELEM_SPKR_LOUDNESS"),								(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0 },
		{ (int)MS_LOCAL_CC_ELEM_SPKR_MONITOR,_T("MS_LOCAL_CC_ELEM_SPKR_MONITOR"),								(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0 },
//		{ (int)MS_LOCAL_CC_ELEM_ENABLE_AUTO_REJECT,_T("MS_LOCAL_CC_ELEM_ENABLE_AUTO_REJECT"),								(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0 },
		{ (int)MS_LOCAL_CC_ELEM_FAX_FCLASS,_T("MS_LOCAL_CC_ELEM_FAX_FCLASS"),								(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0 },
        { (int)MS_LOCAL_CC_ELEM_END_CALL_CAUSE,_T("MS_LOCAL_CC_ELEM_END_CALL_CAUSE"),						(xdrproc_t)xdr_CallReleaseInfo_t, sizeof( CallReleaseInfo_t ), NULL_capi2_proc_t, (xdrproc_t)xdr_CallReleaseInfo_t,0 },

		{ (int)MS_LOCAL_CC_ELEM_CRLP,_T("MS_LOCAL_CC_ELEM_CRLP"),								(xdrproc_t)xdr_RlpParam_t, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_RlpParam_t,0 },
		{ (int)MS_LOCAL_CC_ELEM_CBST,_T("MS_LOCAL_CC_ELEM_CBST"),								(xdrproc_t)xdr_CBST_t, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_CBST_t,0 },
//		{ (int)MS_LOCAL_CC_ELEM_DS_NEG_DATACOMP,_T("MS_LOCAL_CC_ELEM_DS_NEG_DATACOMP"),								(xdrproc_t)xdr_DCParam_t, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_DCParam_t,0 },
		{ (int)MS_LOCAL_CC_ELEM_DS_REQ_DATACOMP,_T("MS_LOCAL_CC_ELEM_DS_REQ_DATACOMP"),								(xdrproc_t)xdr_DCParam_t, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_DCParam_t,0 },
		{ (int)MS_LOCAL_CC_ELEM_EC_REQUESTED,_T("MS_LOCAL_CC_ELEM_EC_REQUESTED"),								(xdrproc_t)XDR_ENUM_DEF(ECOrigReq_t), sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)XDR_ENUM_DEF(ECOrigReq_t),0 },
		{ (int)MS_LOCAL_CC_ELEM_EC_FALLBACK_NOT_NEGOTIABLE,_T("MS_LOCAL_CC_ELEM_EC_FALLBACK_NOT_NEGOTIABLE"),								(xdrproc_t)XDR_ENUM_DEF(ECOrigFB_t), sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)XDR_ENUM_DEF(ECOrigFB_t),0 },
		{ (int)MS_LOCAL_CC_ELEM_EC_FALLBACK_NO_ANSWER,_T("MS_LOCAL_CC_ELEM_EC_FALLBACK_NO_ANSWER"),								(xdrproc_t)XDR_ENUM_DEF(ECAnsFB_t), sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)XDR_ENUM_DEF(ECAnsFB_t),0 },
		{ (int)MS_LOCAL_CC_ELEM_EC_PARAMS,_T("MS_LOCAL_CC_ELEM_EC_PARAMS"),								(xdrproc_t)XDR_ENUM_DEF(ECMode_t), sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)XDR_ENUM_DEF(ECMode_t),0 },
		{ (int)MS_LOCAL_CC_ELEM_DC_PARAMS,_T("MS_LOCAL_CC_ELEM_DC_PARAMS"),								(xdrproc_t)XDR_ENUM_DEF(DCMode_t), sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)XDR_ENUM_DEF(DCMode_t),0 },
		{ (int)MS_LOCAL_CC_ELEM_CALL_METER,_T("MS_LOCAL_CC_ELEM_CALL_METER"),								(xdrproc_t)xdr_u_int32_t, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_int32_t,0 },
//		{ (int)MS_LOCAL_CC_ELEM_ETBM,_T("MS_LOCAL_CC_ELEM_ETBM"),								(xdrproc_t)xdr_T_ETBM, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_T_ETBM,0 },
//		{ (int)MS_LOCAL_CC_ELEM_CLIR,_T("MS_LOCAL_CC_ELEM_CLIR"),								(xdrproc_t)XDR_ENUM_DEF(CLIRMode_t), sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)XDR_ENUM_DEF(CLIRMode_t),0 },

		{ (int)MS_LOCAL_SATK_ELEM_SETUP_CALL_CTR,_T("MS_LOCAL_SATK_ELEM_SETUP_CALL_CTR"),		(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char ,0 },
		{ (int)MS_LOCAL_SATK_ELEM_SEND_SS_CTR,_T("MS_LOCAL_SATK_ELEM_SEND_SS_CTR"),				(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char ,0 },
		{ (int)MS_LOCAL_SATK_ELEM_SEND_USSD_CTR,_T("MS_LOCAL_SATK_ELEM_SEND_USSD_CTR"),			(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char ,0 },
		{ (int)MS_LOCAL_SATK_ELEM_SEND_SMS_CTR,_T("MS_LOCAL_SATK_ELEM_SEND_SMS_CTR"),			(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char ,0 },
		{ (int)MS_LOCAL_SATK_ELEM_SEND_DTMF_CTR,_T("MS_LOCAL_SATK_ELEM_SEND_DTMF_CTR"),			(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char ,0 },
		{ (int)MS_LOCAL_SATK_ELEM_PLAY_TONE_CTR,_T("MS_LOCAL_SATK_ELEM_PLAY_TONE_CTR"),			(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char ,0 },
		{ (int)MS_LOCAL_SATK_ELEM_CALL_CONTROL_CTR,_T("MS_LOCAL_SATK_ELEM_CALL_CONTROL_CTR"),	(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char ,0 },
		{ (int)MS_LOCAL_SATK_ELEM_SS_CONTROL_CTR,_T("MS_LOCAL_SATK_ELEM_SS_CONTROL_CTR"),		(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char ,0 },
		{ (int)MS_LOCAL_SATK_ELEM_USSD_CONTROL_CTR,_T("MS_LOCAL_SATK_ELEM_USSD_CONTROL_CTR"),	(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char ,0 },
		{ (int)MS_LOCAL_SATK_ELEM_SMS_CONTROL_CTR,_T("MS_LOCAL_SATK_ELEM_SMS_CONTROL_CTR"),		(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char ,0 },

		{ (int)MS_LOCAL_PHCTRL_ELEM_ATTACH_MODE,_T("MS_LOCAL_PHCTRL_ELEM_ATTACH_MODE"),			(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char ,0 },
		{ (int)MS_LOCAL_PHCTRL_ELEM_IS_PWRDWN_SOFTRESET,_T("MS_LOCAL_PHCTRL_ELEM_IS_PWRDWN_SOFTRESET"),			(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char ,0 },
        { (int)MS_LOCAL_PHCTRL_ELEM_USE_PNN_NAME,_T("MS_LOCAL_PHCTRL_ELEM_USE_PNN_NAME"),		(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char ,0 },
		{ (int)MS_LOCAL_PHCTRL_ELEM_PCM_MODE,_T("MS_LOCAL_PHCTRL_ELEM_PCM_MODE"),				(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char ,0 },
		{ (int)MS_LOCAL_PHCTRL_ELEM_EBSE,_T("MS_LOCAL_PHCTRL_ELEM_EBSE"),						(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char ,0 },
		{ (int)MS_LOCAL_PHCTRL_ELEM_SELECTED_PLMN,_T("MS_LOCAL_PHCTRL_ELEM_SELECTED_PLMN"),		(xdrproc_t)xdr_PLMN_t, sizeof( PLMN_t ), NULL_capi2_proc_t, (xdrproc_t)xdr_PLMN_t ,0 },
		{ (int)MS_LOCAL_PHCTRL_ELEM_PLMN_MODE,_T("MS_LOCAL_PHCTRL_ELEM_PLMN_MODE"),				(xdrproc_t)xdr_PlmnSelectMode_t, sizeof( PlmnSelectMode_t ), NULL_capi2_proc_t, (xdrproc_t)xdr_PlmnSelectMode_t ,0 },
		{ (int)MS_LOCAL_PHCTRL_ELEM_PREV_PLMN_MODE,_T("MS_LOCAL_PHCTRL_ELEM_PREV_PLMN_MODE"),	(xdrproc_t)xdr_PlmnSelectMode_t, sizeof( PlmnSelectMode_t ), NULL_capi2_proc_t, (xdrproc_t)xdr_PlmnSelectMode_t ,0 },		
		{ (int)MS_LOCAL_PHCTRL_ELEM_PLMN_FORMAT,_T("MS_LOCAL_PHCTRL_ELEM_PLMN_FORMAT"),			(xdrproc_t)xdr_PlmnSelectFormat_t, sizeof( PlmnSelectFormat_t ), NULL_capi2_proc_t, (xdrproc_t)xdr_PlmnSelectFormat_t ,0 },
		{ (int)MS_LOCAL_PHCTRL_ELEM_FORCE_SIM_NOT_RDY, _T("MS_LOCAL_PHCTRL_ELEM_FORCE_SIM_NOT_RDY"),	(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char ,0 },
		{ (int)MS_LOCAL_PHCTRL_ELEM_CURR_ATTACH_MODE,_T("MS_LOCAL_PHCTRL_ELEM_CURR_ATTACH_MODE"),			(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char ,0 },
		
		{ (int)MS_NETWORK_ELEM_DTM,_T("MS_NETWORK_ELEM_DTM"),									(xdrproc_t)xdr_MSNetAccess_t, sizeof( MSNetAccess_t ), NULL_capi2_proc_t,(xdrproc_t)xdr_MSNetAccess_t,0 },
		{ (int)MS_NETWORK_ELEM_HSDPA_SUPPORT,_T("MS_NETWORK_ELEM_HSDPA_SUPPORT"),				(xdrproc_t)xdr_MSNetAccess_t, sizeof( MSNetAccess_t ), NULL_capi2_proc_t,(xdrproc_t)xdr_MSNetAccess_t,0 },
		{ (int)MS_NETWORK_ELEM_HSUPA_SUPPORT,_T("MS_NETWORK_ELEM_HSUPA_SUPPORT"),				(xdrproc_t)xdr_MSNetAccess_t, sizeof( MSNetAccess_t ), NULL_capi2_proc_t,(xdrproc_t)xdr_MSNetAccess_t,0 },
		{ (int)MS_NETWORK_ELEM_GPRS_SUPPORT,_T("MS_NETWORK_ELEM_GPRS_SUPPORT"),				(xdrproc_t)xdr_MSNetAccess_t, sizeof( MSNetAccess_t ), NULL_capi2_proc_t,(xdrproc_t)xdr_MSNetAccess_t,0 },
		{ (int)MS_NETWORK_ELEM_EGPRS_SUPPORT,_T("MS_NETWORK_ELEM_EGPRS_SUPPORT"),				(xdrproc_t)xdr_MSNetAccess_t, sizeof( MSNetAccess_t ), NULL_capi2_proc_t,(xdrproc_t)xdr_MSNetAccess_t,0 },
		{ (int)MS_NETWORK_ELEM_NOM,_T("MS_NETWORK_ELEM_NOM"),									(xdrproc_t)xdr_MSNwOperationMode_t, sizeof( MSNwOperationMode_t ), NULL_capi2_proc_t,(xdrproc_t)xdr_MSNwOperationMode_t,0 },
		{ (int)MS_NETWORK_ELEM_NW_TYPE,_T("MS_NETWORK_ELEM_NW_TYPE"),							(xdrproc_t)xdr_MSNwType_t, sizeof( MSNwType_t ),	NULL_capi2_proc_t,(xdrproc_t)xdr_MSNwType_t,0 },
		{ (int)MS_NETWORK_ELEM_RAC,_T("MS_NETWORK_ELEM_RAC"),									(xdrproc_t)xdr_u_char, sizeof( UInt8 ),		NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0 },
		{ (int)MS_NETWORK_ELEM_RNC,_T("MS_NETWORK_ELEM_RNC"),									(xdrproc_t)xdr_u_int16_t, sizeof( UInt16 ),		NULL_capi2_proc_t, (xdrproc_t)xdr_u_int16_t,0 },

		{ (int)MS_NETWORK_ELEM_ISGSM_REGISTERED,_T("MS_NETWORK_ELEM_ISGSM_REGISTERED"),			(xdrproc_t)xdr_u_char, sizeof( UInt8 ),		NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0 },
		{ (int)MS_NETWORK_ELEM_ISGPRS_REGISTERED,_T("MS_NETWORK_ELEM_ISGPRS_REGISTERED"),		(xdrproc_t)xdr_u_char, sizeof( UInt8 ),		NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0 },
		{ (int)MS_NETWORK_ELEM_CURRENT_CAMPED_PLMN,_T("MS_NETWORK_ELEM_CURRENT_CAMPED_PLMN"),	(xdrproc_t)xdr_PLMNId_t, sizeof(PLMNId_t), NULL_capi2_proc_t, (xdrproc_t)xdr_PLMNId_t,0},
		{ (int)MS_NETWORK_ELEM_GSMREG_STATE,_T("MS_NETWORK_ELEM_GSMREG_STATE"),					(xdrproc_t)xdr_MSRegState_t, sizeof( MSRegState_t ),		NULL_capi2_proc_t, (xdrproc_t)xdr_MSRegState_t,0 },
		{ (int)MS_NETWORK_ELEM_GPRSREG_STATE,_T("MS_NETWORK_ELEM_GPRSREG_STATE"),				(xdrproc_t)xdr_MSRegState_t, sizeof( MSRegState_t ),		NULL_capi2_proc_t, (xdrproc_t)xdr_MSRegState_t,0 },

		{ (int)MS_LOCAL_PHCTRL_ELEM_SMSME_STORAGE_ENABLE,_T("MS_LOCAL_PHCTRL_ELEM_SMSME_STORAGE_ENABLE"),		(xdrproc_t)xdr_u_char, sizeof( UInt8 ),		NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0 },
		{ (int)MS_LOCAL_PHCTRL_ELEM_SMSME_MAX_CAPACITY,_T("MS_LOCAL_PHCTRL_ELEM_SMSME_MAX_CAPACITY"),		(xdrproc_t)xdr_u_int16_t, sizeof( UInt16 ),		NULL_capi2_proc_t, (xdrproc_t)xdr_u_int16_t,0 },

		{ (int)MS_LOCAL_SMS_ELEM_DEF_REC_NUMBER,_T("MS_LOCAL_SMS_ELEM_DEF_REC_NUMBER"),			(xdrproc_t)xdr_u_char, sizeof( UInt8 ),		NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0 },
		{ (int)MS_LOCAL_SMS_ELEM_CLIENT_HANDLE_MT_SMS,_T("MS_LOCAL_SMS_ELEM_CLIENT_HANDLE_MT_SMS"),	(xdrproc_t)xdr_u_char, sizeof( UInt8 ),		NULL_capi2_proc_t, (xdrproc_t)xdr_u_char ,0},
		{ (int)MS_LOCAL_AT_ELEM_AUDIO_CTRL,_T("MS_LOCAL_AT_ELEM_AUDIO_CTRL"),					(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0 },
		{ (int)MS_STACK_ELEM_NVRAM_CLASSMARK,_T("MS_STACK_ELEM_NVRAM_CLASSMARK"),	(xdrproc_t)xdr_NVRAMClassmark_t, sizeof( NVRAMClassmark_t ),		NULL_capi2_proc_t, (xdrproc_t)xdr_NVRAMClassmark_t ,0},

		{ (int)MS_DRIVER_ELEM_SPINNER_SLEEP_MODE,_T("MS_DRIVER_ELEM_SPINNER_SLEEP_MODE"),		(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0 },
		{ (int)MS_LOCAL_PHCTRL_ELEM_WAIT_BEFORE_DETACH_TIME,_T("MS_LOCAL_PHCTRL_ELEM_WAIT_BEFORE_DETACH_TIME"),	(xdrproc_t)xdr_u_int16_t,sizeof(UInt16),NULL_capi2_proc_t, (xdrproc_t)xdr_u_int16_t,0},

        { (int)MS_SIM_ELEM_PIN1_STATUS, _T("MS_SIM_ELEM_PIN1_STATUS"),                          (xdrproc_t)xdr_u_char, sizeof( UInt8 ),		NULL_capi2_proc_t, (xdrproc_t)xdr_u_char ,0 },
        { (int)MS_SIM_ELEM_PIN2_STATUS, _T("MS_SIM_ELEM_PIN2_STATUS"),                          (xdrproc_t)xdr_u_char, sizeof( UInt8 ),		NULL_capi2_proc_t, (xdrproc_t)xdr_u_char ,0 },
        { (int)MS_SIM_ELEM_ICC_ID, _T("MS_SIM_ELEM_ICC_ID"),                                    (xdrproc_t)xdr_MSSimIccId, sizeof( ICCID_ASCII_t),NULL_capi2_proc_t, (xdrproc_t)xdr_MSSimIccId ,0 },
        { (int)MS_SIM_ELEM_RAW_ICC_ID, _T("MS_SIM_ELEM_RAW_ICC_ID"),                            (xdrproc_t)xdr_MSSimRawIccId, sizeof( ICCID_BCD_t ),NULL_capi2_proc_t, (xdrproc_t)xdr_MSSimRawIccId ,0 },
        { (int)MS_SIM_ELEM_CPHS_ONS, _T("MS_SIM_ELEM_CPHS_ONS"),                                (xdrproc_t)xdr_MSSimCphsOns, sizeof( CPHS_ONS_t ), NULL_capi2_proc_t, (xdrproc_t)xdr_MSSimCphsOns ,0 },
        { (int)MS_SIM_ELEM_CPHS_ONS_LEN, _T("MS_SIM_ELEM_CPHS_ONS_LEN"),                        (xdrproc_t)xdr_u_char, sizeof( UInt8 ),		NULL_capi2_proc_t, (xdrproc_t)xdr_u_char ,0 },
        { (int)MS_SIM_ELEM_CPHS_ONSS, _T("MS_SIM_ELEM_CPHS_ONSS"),                              (xdrproc_t)xdr_MSSimCphsOnss, sizeof( CPHS_ONSS_t ), NULL_capi2_proc_t, (xdrproc_t)xdr_MSSimCphsOnss ,0 },
        { (int)MS_SIM_ELEM_CPHS_ONSS_LEN, _T("MS_SIM_ELEM_CPHS_ONSS_LEN"),                      (xdrproc_t)xdr_u_char, sizeof( UInt8 ),		NULL_capi2_proc_t, (xdrproc_t)xdr_u_char ,0 },
        { (int)MS_SIM_ELEM_SST, _T("MS_SIM_ELEM_SST"),                                          (xdrproc_t)xdr_MSSimSst, sizeof( SST_t ), NULL_capi2_proc_t, (xdrproc_t)xdr_MSSimSst ,0 },
        { (int)MS_SIM_ELEM_CPHS_INFO, _T("MS_SIM_ELEM_CPHS_INFO"),                              (xdrproc_t)xdr_MSSimCphsInfo, sizeof( CPHS_INFO_t ),		NULL_capi2_proc_t, (xdrproc_t)xdr_MSSimCphsInfo ,0 },
        { (int)MS_SIM_ELEM_ECC_REC_LIST, _T("MS_SIM_ELEM_ECC_REC_LIST"),                        (xdrproc_t)xdr_MSSimEccRecList, sizeof(ECC_REC_LIST_t), NULL_capi2_proc_t, (xdrproc_t)xdr_MSSimEccRecList ,0 },

        { (int)MS_LOCAL_PHCTRL_ELEM_CGEREP, _T("MS_LOCAL_PHCTRL_ELEM_CGEREP"),                    (xdrproc_t)xdr_MS_LOCAL_PHCTRL_ELEM_CGEREP, sizeof(UInt8)*2, NULL_capi2_proc_t, (xdrproc_t)xdr_MS_LOCAL_PHCTRL_ELEM_CGEREP ,0 },
        { (int)MS_LOCAL_PHCTRL_ELEM_CPWC, _T("MS_LOCAL_PHCTRL_ELEM_CPWC"),                        (xdrproc_t)xdr_MS_LOCAL_PHCTRL_ELEM_CPWC, sizeof(UInt8)*3, NULL_capi2_proc_t, (xdrproc_t)xdr_MS_LOCAL_PHCTRL_ELEM_CPWC ,0 },
        { (int)MS_LOCAL_PHCTRL_ELEM_CSCS, _T("MS_LOCAL_PHCTRL_ELEM_CSCS"),                        (xdrproc_t)xdr_MS_LOCAL_PHCTRL_ELEM_CSCS, sizeof(UInt8)*10, NULL_capi2_proc_t, (xdrproc_t)xdr_MS_LOCAL_PHCTRL_ELEM_CSCS ,0 },
		{ (int)MS_LOCAL_PHCTRL_ELEM_IMEI, _T("MS_LOCAL_PHCTRL_ELEM_IMEI"),                        (xdrproc_t)xdr_MS_LOCAL_PHCTRL_ELEM_IMEI, sizeof(UInt8)*15, NULL_capi2_proc_t, (xdrproc_t)xdr_MS_LOCAL_PHCTRL_ELEM_IMEI ,0 },
		{ (int)MS_LOCAL_PHCTRL_ELEM_SW_VERSION, _T("MS_LOCAL_PHCTRL_ELEM_SW_VERSION"),             (xdrproc_t)xdr_MS_LOCAL_PHCTRL_ELEM_SW_VERSION, sizeof(UInt8)*2, NULL_capi2_proc_t, (xdrproc_t)xdr_MS_LOCAL_PHCTRL_ELEM_SW_VERSION ,0 },
		{ (int)MS_LOCAL_SMS_ELEM_MORE_MESSAGE_TO_SEND, _T("MS_LOCAL_SMS_ELEM_MORE_MESSAGE_TO_SEND"),  (xdrproc_t)xdr_u_char, sizeof(UInt8), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char ,0 },
		{ (int)MS_LOCAL_SS_ELEM_NOTIFICATION_SWITCH, _T("MS_LOCAL_SS_ELEM_NOTIFICATION_SWITCH"),  (xdrproc_t)xdr_u_char, sizeof(UInt8), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char ,0 },
		{ __dontcare__, _T(""), NULL_xdrproc_t ,0,0,0,0} 
};


bool_t xdr_MS_TestChan_t( XDR* xdrs, MS_TestChan_t* data)
{
	u_int len;
	bool_t result;
	
	XDR_LOG(xdrs,"xdr_MS_TestChan_t")

	result = XDR_ENUM(xdrs,  &data->band, BandSelect_t);

	if(result)
		result = _xdr_u_int16_t(xdrs, &data->numChan,"numChan");

	len = (u_int)data->numChan;

	if(result)
		result = xdr_array(xdrs,(char **)(void*) &data->chanListPtr,&len,~0,sizeof(UInt16),(xdrproc_t) xdr_u_int16_t);

	return result;
}


Int32 GetCapi2MSElementSize(MS_Element_t inElemType)
{
	const struct xdr_discrim *choices = CAPI2_MSElement_t_dscrm;
	for (; choices->proc != NULL_xdrproc_t; choices++)
	{
		if (choices->value == inElemType)
			return choices->unsize;
	}
	return -1;
}

bool_t xdr_CAPI2_MS_Element_t(XDR* xdrs, CAPI2_MS_Element_t *rsp)
{
	struct xdr_discrim *entry = NULL;
	enum_t dscm = rsp->inElemType;
//	const struct xdr_discrim *choices = CAPI2_MSElement_t_dscrm;
//	UInt32 max_size = 0;

	XDR_LOG(xdrs,"xdr_CAPI2_MS_Element_t ")
	
	
	if(!XDR_ENUM(xdrs,  &(rsp->inElemType), MS_Element_t) )
	{
		return FALSE;
	}


/*	for (; choices->proc != NULL_xdrproc_t; choices++)
	{
		CAPI2_TRACE("MSDb[%s] %d %d %s\n", (sizeof(CAPI2_MS_Element_t) >= choices->unsize)?"OK":"FAIL",
											choices->unsize,
											max_size,
											(choices->debugStr)?choices->debugStr:"<>");
	}
*/

	if( xdr_union(xdrs, (enum_t *)&dscm, (caddr_t)(void *)&(rsp->data_u), CAPI2_MSElement_t_dscrm, NULL_xdrproc_t, &entry, NULL) )
	{
		if(xdrs->x_op == XDR_ENCODE || xdrs->x_op == XDR_DECODE)
		{
			//If you see this assert then add the element in data_u of CAPI2_MS_Element_t.
			xassert( (sizeof(entry->unsize) <= sizeof(rsp->data_u) ), dscm);
			
			_DBG_(CAPI2_TRACE("[CAPI2] MS_Element_t[%d] %s : value={%x %x %x %x %x %x %x %x}\r\n",rsp->inElemType, 
																	(entry->debugStr)?entry->debugStr:"<>", 
																	rsp->data_u.u10Bytes[0],  
																	rsp->data_u.u10Bytes[1],  
																	rsp->data_u.u10Bytes[2],  
																	rsp->data_u.u10Bytes[3],  
																	rsp->data_u.u10Bytes[4],  
																	rsp->data_u.u10Bytes[5],  
																	rsp->data_u.u10Bytes[6],  
																	rsp->data_u.u10Bytes[7]));
		}
		return(TRUE);
	}
	else
	{
		XDR_LOG(xdrs,"xdr_CAPI2_MS_Element_t unable to serialize element")
	}
	return FALSE;
}

bool_t xdr_PLMN_t( XDR* xdrs, PLMN_t* data)
{
	XDR_LOG(xdrs,"PLMN_t")

	if( _xdr_u_char(xdrs, &data->mnc,"mnc")	&&
		_xdr_u_int16_t(xdrs, &data->mcc,"mcc")
		)
		return(TRUE);
	else
		return(FALSE);

}

bool_t xdr_MS_AMRParam_t(XDR* xdrs, MS_AMRParam_t *param)
{
	XDR_LOG(xdrs,"MS_AMRParam_t")
    if( _xdr_u_char(xdrs, &param->valid,"valid")  &&
		_xdr_u_char(xdrs, &param->cmip,"cmip")  &&
        _xdr_u_char(xdrs, &param->cmi,"cmi")  &&
		_xdr_u_char(xdrs, &param->cmr,"cmr")  &&
		xdr_int(xdrs, &param->dl_ci) &&
		_xdr_u_char(xdrs, &param->acs,"acs")  &&
		xdr_opaque(xdrs, (caddr_t)&param->threshold, 3) &&
        xdr_opaque(xdrs, (caddr_t)&param->hysteresis, 3) &&
	    _xdr_u_char(xdrs, &param->speech_rate,"speech_rate")  &&
        _xdr_u_char(xdrs, &param->subchan,"subchan")  
		)	
		return(TRUE);
	else
		return(FALSE);
}
 
bool_t xdr_T_USF_ARRAY(void* xdrs, T_USF_ARRAY *param)
{
	XDR_LOG(xdrs,"T_USF_ARRAY")
    
    if( xdr_opaque(xdrs, (caddr_t)&param->A, 8) 
		)	
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_ACK_NACK_CNT_t(void* xdrs, ACK_NACK_CNT_t *param)
{
	XDR_LOG(xdrs,"RLC_State_Variable_List_t")
    if( 
	   	_xdr_u_int16_t(xdrs, &param->ack_blocks,"ack_blocks")  &&
        _xdr_u_int16_t(xdrs, &param->nack_blocks,"nack_blocks")  
        )	
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_ACK_NACK_Testparam_t(void* xdrs, ACK_NACK_Testparam_t *param)
{
	XDR_LOG(xdrs,"ACK_NACK_Testparam_t")
    if( 
	   	xdr_ACK_NACK_CNT_t(xdrs, &param->dl_blocks) &&
        xdr_ACK_NACK_CNT_t(xdrs, &param->ul_blocks) 
        )	
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_RLC_State_Variable_List_t(void* xdrs, RLC_State_Variable_List_t *param)
{
	XDR_LOG(xdrs,"RLC_State_Variable_List_t")
    if( 
	   	_xdr_u_int16_t(xdrs, &param->rlc_v_s,"rlc_v_s")  &&
        _xdr_u_int16_t(xdrs, &param->rlc_v_a,"rlc_v_a")  &&
        _xdr_u_int16_t(xdrs, &param->rlc_v_q,"rlc_v_q")  &&
        _xdr_u_int16_t(xdrs, &param->rlc_v_r,"rlc_v_r")  
        )	
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_RLC_ACK_NACK_List_t(void* xdrs, RLC_ACK_NACK_List_t *list)
{
	XDR_LOG(xdrs,"RLC_ACK_NACK_List_t")
    return xdr_vector(xdrs,  (char *)(void *)list->A, 4, sizeof(ACK_NACK_Testparam_t), (xdrproc_t)xdr_ACK_NACK_Testparam_t); 
}

bool_t xdr_Access_Tech_List_t(void* xdrs, Access_Tech_List_t *param)
{
	XDR_LOG(xdrs,"Access_Tech_List_t")
    if( 
	   	xdr_Int8(xdrs, (char *)&param->uplink_acc_tech) &&
        xdr_Int8(xdrs, (char *)&param->downlink_acc_tech) 
        )	
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_RLC_Testparam_t(void* xdrs, RLC_Testparam_t *param)
{
	XDR_LOG(xdrs,"RLC_Testparam_t")
    if( 
	   	xdr_RLC_ACK_NACK_List_t(xdrs, &param->ack_nack_param_list) &&
        xdr_RLC_State_Variable_List_t(xdrs, &param->state_variable_param_list) &&
        xdr_Access_Tech_List_t(xdrs, &param->acc_tech_param_list) 
		)	
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_MS_GPRSPacketParam_t(void* xdrs, MS_GPRSPacketParam_t *param)
{
	XDR_LOG(xdrs,"MS_GPRSPacketParam_t")
    if( _xdr_u_char(xdrs, &param->valid,"valid")  &&
		_xdr_u_char(xdrs, &param->timing_advance,"timing_advance")  &&
        _xdr_u_char(xdrs, &param->dl_timeslot_assigned,"dl_timeslot_assigned")  &&
		_xdr_u_char(xdrs, &param->ul_timeslot_assigned,"ul_timeslot_assigned")  &&
		xdr_opaque(xdrs, (caddr_t)&param->dl_cs_mode_per_ts, 8) &&
        xdr_opaque(xdrs, (caddr_t)&param->ul_cs_mode_per_ts, 8) &&
		_xdr_u_char(xdrs, &param->c_value,"c_value")  &&
		_xdr_u_char(xdrs, &param->alpha,"alpha")  &&
		xdr_opaque(xdrs, (caddr_t)&param->gamma, 8) &&
	   	_xdr_u_char(xdrs, &param->timing_adv_idx,"timing_adv_idx")  &&
        _xdr_u_char(xdrs, &param->ra_reselect_hyst,"ra_reselect_hyst")  &&
        _xdr_u_char(xdrs, &param->c31_hyst,"c31_hyst")  &&
        _xdr_u_char(xdrs, &param->c32_qual,"c32_qual")  &&
        _xdr_u_char(xdrs, &param->rand_acc_retry,"rand_acc_retry")  &&
        _xdr_u_char(xdrs, &param->epcr_sup,"epcr_sup")  &&
        xdr_T_USF_ARRAY(xdrs, &param->usf) &&
	   	xdr_RLC_Testparam_t(xdrs, &param->rlc_param) 
		)	
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_MS_EDGEPacketParam_t(void* xdrs, MS_EDGEPacketParam_t *param)
{
	XDR_LOG(xdrs,"MS_EDGEPacketParam_t")
    if( _xdr_u_char(xdrs, &param->valid,"valid")  &&
		xdr_MS_GPRSPacketParam_t(xdrs, &param->gprs_packet_param) &&
		_xdr_u_char(xdrs, &param->mean_bep_gmsk,"mean_bep_gmsk")  &&
        _xdr_u_char(xdrs, &param->mean_bep_8psk,"mean_bep_8psk")  &&
		_xdr_u_char(xdrs, &param->cv_bep_gmsk,"cv_bep_gmsk")  &&
		_xdr_u_char(xdrs, &param->cv_bep_8psk,"cv_bep_8psk") &&
		_xdr_u_char(xdrs, &param->bep_per,"bep_per") 
	   	)	
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_MS_EDGEParam_t(void* xdrs, MS_EDGEParam_t *param)
{
	XDR_LOG(xdrs,"MS_EDGEParam_t")
    if( _xdr_u_char(xdrs, &param->valid,"valid")  &&
		_xdr_u_char(xdrs, &param->edge_present,"edge_present")  &&
		xdr_MS_EDGEPacketParam_t(xdrs, &param->edge_packet_param) 
		)	
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_MS_NcellMeas_t(void* xdrs, MS_NcellMeas_t *data)
{
    XDR_LOG(xdrs,"MS_NcellMeas_t")
	if( _xdr_u_int16_t(xdrs, &data->arfcn,"arfcn") &&
		_xdr_u_char(xdrs, &data->rxlev,"rxlev")  &&
		_xdr_u_int16_t(xdrs, &data->mcc,"mcc")  &&
        _xdr_u_char(xdrs, &data->mnc,"mnc")  &&
        _xdr_u_int16_t(xdrs, &data->lac,"lac")  &&
        _xdr_u_int16_t(xdrs, &data->ci,"ci")  &&
		_xdr_u_char(xdrs, &data->bsic,"bsic")  &&
		xdr_int(xdrs, &data->c1) &&
        xdr_int(xdrs, &data->c2) &&
		xdr_int(xdrs, &data->c31) &&
        xdr_int(xdrs, &data->c32) &&
        _xdr_u_char(xdrs, &data->barred,"barred")  &&
        _xdr_u_char(xdrs, &data->cell_priority,"cell_priority")  &&
		_xdr_u_char(xdrs, &data->priority_class,"priority_class")  &&
	    _xdr_u_char(xdrs, &data->same_ra_as_serving_cell,"same_ra_as_serving_cell")  &&
        _xdr_u_char(xdrs, &data->cell_bar_acc_2,"cell_bar_acc_2")  &&
	    _xdr_u_char(xdrs, &data->exc_acc,"exc_acc")
		)	
		return(TRUE);
	else
		return(FALSE);

}

bool_t xdr_MS_NcellList_t(void* xdrs, MS_NcellList_t *list)
{
	XDR_LOG(xdrs,"MS_NcellList_t")
    return xdr_vector(xdrs,  (char *)(void *)list->A, 6, sizeof(MS_NcellMeas_t), (xdrproc_t)xdr_MS_NcellMeas_t); 
}

bool_t xdr_MS_MA_t(void* xdrs, MS_MA_t *param)
{
	XDR_LOG(xdrs,"MS_MA_t ")

	if( _xdr_u_int16_t(xdrs, &param->rf_chan_cnt,"rf_chan_cnt") &&
		xdr_vector(xdrs,  (char *)(void *)param->rf_chan_array, 64, sizeof(UInt16), (xdrproc_t)xdr_u_int16_t)
		)	
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_MS_GSMParam_t(void* xdrs, MS_GSMParam_t *param)
{
	XDR_LOG(xdrs,"MS_GSMParam_t ")

	if(	_xdr_u_char(xdrs, &param->valid,"valid")  &&
		_xdr_u_char(xdrs, &param->grr_state,"grr_state")  &&
		_xdr_u_int16_t(xdrs, &param->band,"band") &&
		_xdr_u_int16_t(xdrs, &param->arfcn,"arfcn") &&
		_xdr_u_char(xdrs, &param->rxlev,"rxlev")  &&
		_xdr_u_char(xdrs, &param->bsic,"bsic")  &&
        _xdr_u_int16_t(xdrs, &param->mcc,"mcc")  &&
        _xdr_u_int16_t(xdrs, &param->mnc,"mnc")  &&
        _xdr_u_int16_t(xdrs, &param->lac,"lac")  &&
        _xdr_u_int16_t(xdrs, &param->ci,"ci")  &&
        _xdr_u_char(xdrs, &param->bs_pa_mfrms,"bs_pa_mfrms")  &&
        _xdr_u_char(xdrs, &param->bs_ag_blks_res,"bs_ag_blks_res")  &&
        _xdr_u_char(xdrs, &param->bcch_combined,"bcch_combined")  &&
        _xdr_u_int16_t(xdrs, &param->ms_txpwr,"ms_txpwr")  &&
        _xdr_u_int16_t(xdrs, &param->rx_acc_min,"rx_acc_min")  &&
        _xdr_u_char(xdrs, &param->cbq,"cbq")  &&
		_xdr_u_char(xdrs, &param->cba,"cba")  &&
		_xdr_u_char(xdrs, &param->c2_valid,"c2_valid")  &&
		_xdr_u_char(xdrs, &param->cr_offset,"cr_offset")  &&
        _xdr_u_int16_t(xdrs, &param->tmp_offset,"tmp_offset")  &&
        _xdr_u_int16_t (xdrs, &param->penalty_t,"penalty_t") &&
		xdr_int(xdrs, &param->c1) &&
        xdr_int(xdrs, &param->c2) &&
		_xdr_u_int16_t(xdrs, &param->t3212,"t3212")  &&
        _xdr_u_int16_t (xdrs, &param->acc,"acc") &&
	    _xdr_u_char(xdrs, &param->cs_page_mode,"cs_page_mode")  &&
        _xdr_u_char(xdrs, &param->pbcch_hopping_status,"pbcch_hopping_status")  &&
        _xdr_u_char(xdrs, &param->pccch_hopping_status,"pccch_hopping_status")  &&
        _xdr_u_char(xdrs, &param->gprs_present,"gprs_present")  &&
        _xdr_u_char(xdrs, &param-> rac," rac")  &&
		_xdr_u_int16_t (xdrs, &param->nom,"nom") &&
        _xdr_u_char(xdrs, &param->sp_pg_cy,"sp_pg_cy")  &&
        _xdr_u_int16_t(xdrs, &param->t3192,"t3192")  &&
        xdr_u_int32_t(xdrs,(u_int32_t *)(void*) &param->access_burst_type)  &&
        _xdr_u_char(xdrs, &param->drx_max,"drx_max")  &&
        xdr_u_int32_t(xdrs,(u_int32_t *)(void*) &param->nco)  &&
        xdr_u_int32_t(xdrs,(u_int32_t *)(void*) &param->emo)  &&
        _xdr_u_char(xdrs, &param->pbcch_present,"pbcch_present")  &&
		_xdr_u_char(xdrs, &param->psi1_rep,"psi1_rep")  &&
		_xdr_u_char(xdrs, &param->psi_count_lr,"psi_count_lr")  &&
		_xdr_u_char(xdrs, &param->psi_count_hr,"psi_count_hr")  &&
		xdr_int(xdrs, &param->c31) &&
        xdr_int(xdrs, &param->c32) &&
		_xdr_u_char(xdrs, &param->priority_access_thr,"priority_access_thr")  &&
        _xdr_u_char(xdrs, &param->t_avg_w,"t_avg_w")  &&
		_xdr_u_char(xdrs, &param->t_avg_t,"t_avg_t")  &&
	    _xdr_u_char(xdrs, &param->n_avg_i,"n_avg_i")  &&
        _xdr_u_char(xdrs, &param->pb,"pb")  &&
	    _xdr_u_char(xdrs, &param->ps_page_mode,"ps_page_mode")  &&
        _xdr_u_char(xdrs, &param->si13_pres,"si13_pres")  &&
        _xdr_u_char(xdrs, &param->pc_meas_chan,"pc_meas_chan")  &&
	    _xdr_u_char(xdrs, &param->t_resel,"t_resel")  &&		
        _xdr_u_int16_t(xdrs, &param->no_ncells,"no_ncells")  &&
		xdr_MS_NcellList_t(xdrs, &param->ncell_param) &&
	    _xdr_u_int16_t(xdrs, &param->no_3g_ncells,"no_3g_ncells")  &&
		xdr_MS_UmtsNcellList_t(xdrs, &param->umts_ncell_param) &&
	    _xdr_u_char(xdrs, &param->chan_type,"chan_type")  &&
	  	_xdr_u_char(xdrs, &param->chan_mode,"chan_mode")  &&
        _xdr_u_int16_t(xdrs, &param->arfcn_ded,"arfcn_ded")  &&
        _xdr_u_char(xdrs, &param->maio,"maio")  &&
		_xdr_u_char(xdrs, &param->hopping_status,"hopping_status")  &&
	    _xdr_u_char(xdrs, &param->hsn,"hsn")  &&
		xdr_MS_MA_t(xdrs, &param->ma) &&
	    _xdr_u_char(xdrs, &param->timeslot_assigned,"timeslot_assigned")  &&
		_xdr_u_char(xdrs, &param->txpwr,"txpwr")  &&
		_xdr_u_char(xdrs, &param->dtx_used,"dtx_used")  &&
  		_xdr_u_char(xdrs, &param->dtx_from_bs,"dtx_from_bs")  &&
        _xdr_u_char(xdrs, &param->rxlevfull,"rxlevfull")  &&
        _xdr_u_char(xdrs, &param->rxlevsub,"rxlevsub")  &&
        _xdr_u_char(xdrs, &param->rxqualfull,"rxqualfull")  &&
        _xdr_u_char(xdrs, &param->rxqualsub,"rxqualsub")  &&
		_xdr_u_char(xdrs, &param->tsc,"tsc")  &&
		_xdr_u_char(xdrs, &param->timing_advance,"timing_advance")  &&
		_xdr_u_char(xdrs, &param->radio_link_timeout,"radio_link_timeout") &&
		_xdr_u_char(xdrs, &param->cipher_on,"cipher_on") &&
		_xdr_u_char(xdrs, &param->cipher_algorithm,"cipher_algorithm") &&
	    _xdr_u_char(xdrs, &param->chn_rel_cause,"chn_rel_cause")  &&
 	   	_xdr_u_char(xdrs, &param->rr_cause,"rr_cause")  &&
		_xdr_u_char(xdrs, &param->max_retrans_cs,"max_retrans_cs")  &&
		_xdr_u_char(xdrs, &param->att,"att")  &&
		_xdr_u_char(xdrs, &param->neci,"neci")  &&
        _xdr_u_char(xdrs, &param->acs,"acs")  &&
        _xdr_u_char(xdrs, &param->cbch_sup,"cbch_sup")  &&
        _xdr_u_char(xdrs, &param->reest_allowed,"reest_allowed")  &&
        _xdr_u_char(xdrs, &param->ecsc,"ecsc")  &&
		_xdr_u_char(xdrs, &param->si2ter_sup,"si2ter_sup")  &&
		_xdr_u_char(xdrs, &param->multib_rep,"multib_rep")  &&
		_xdr_u_char(xdrs, &param->cell_bar_acc_2,"cell_bar_acc_2") &&
		_xdr_u_char(xdrs, &param->exc_acc,"exc_acc") &&
		_xdr_u_char(xdrs, &param->ra_resel_hyst,"ra_resel_hyst")  &&
		_xdr_u_char(xdrs, &param->pfc_feat_sup,"pfc_feat_sup")  &&
		_xdr_u_char(xdrs, &param->ext_upl_tbf_sup,"ext_upl_tbf_sup") &&
		_xdr_u_char(xdrs, &param->pkt_si_status,"pkt_si_status") &&
		_xdr_u_char(xdrs, &param->ccn_active,"ccn_active")  &&
     	xdr_MS_AMRParam_t(xdrs, &param->amr_param) &&
        xdr_MS_GPRSPacketParam_t(xdrs, &param->gprs_packet_param) &&
        xdr_MS_EDGEParam_t(xdrs, &param->edge_param) 
		)
	    return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_TMSI_Octets_t(void* xdrs, TMSI_Octets_t *param)
{
	XDR_LOG(xdrs,"TMSI_Octets_t")
    
    if( xdr_vector(xdrs, (char *)(void *)param->A, 4, sizeof(Int8), (xdrproc_t)xdr_Int8) 
		)	
		return(TRUE);
	else
		return(FALSE);
}
bool_t xdr_MS_MMParam_t(void* xdrs, MS_MMParam_t *param)
{
	XDR_LOG(xdrs,"MS_MMParam_t ")

	if(	_xdr_u_char(xdrs, &param->valid,"valid")  &&
		_xdr_u_char(xdrs, &param->mmc_state,"mmc_state")  &&
		_xdr_u_char(xdrs, &param->mme_state,"mme_state")  &&
		_xdr_u_char(xdrs, &param->gmm_state,"gmm_state")  &&
        _xdr_u_char(xdrs, &param->rat,"rat")  &&
        _xdr_u_char(xdrs, &param->nom,"nom")  &&
        _xdr_u_char(xdrs, &param->search_mode,"search_mode")  &&
        _xdr_u_char(xdrs, &param->mm_update_status,"mm_update_status")  &&
        xdr_u_int32_t(xdrs,(u_int32_t *) &param->mm_timer_status)  &&
        _xdr_u_char(xdrs, &param->mm_reject_cause,"mm_reject_cause")  &&
        _xdr_u_char(xdrs, &param->mm_lu_rej_cause,"mm_lu_rej_cause")  &&
        _xdr_u_char(xdrs, &param->mm_lu_imsi_rej_cause,"mm_lu_imsi_rej_cause")  &&
        _xdr_u_char(xdrs, &param->mm_retrans_cnt,"mm_retrans_cnt")  &&
        _xdr_u_char(xdrs, &param->mm_t3212_cnt,"mm_t3212_cnt") &&
        xdr_PLMNId_t(xdrs, &param->last_reg) &&
        _xdr_u_char(xdrs, &param->gmm_update_status,"gmm_update_status")  &&
        xdr_u_int32_t(xdrs,(u_int32_t *) &param->gmm_timer_status)  &&
        _xdr_u_char(xdrs, &param->gmm_reject_cause,"gmm_reject_cause")  &&
		_xdr_u_char(xdrs, &param->gmm_retrans_cnt,"gmm_retrans_cnt")  &&
		_xdr_u_char(xdrs, &param->gmm_retry_cnt,"gmm_retry_cnt")  &&
		_xdr_u_char(xdrs, &param->gmm_attach_type,"gmm_attach_type")  &&
        _xdr_u_char(xdrs, &param->gmm_rau_type,"gmm_rau_type")  &&
        _xdr_u_char(xdrs, &param->gmm_detach_type,"gmm_detach_type")  &&
        _xdr_u_char(xdrs, &param->gmm_detach_dir,"gmm_detach_dir")  &&
		_xdr_u_char(xdrs, &param->mm_class,"mm_class")  &&
		XDR_ENUM(xdrs,  &param->gprs_attach_mode, AttachState_t) &&
		XDR_ENUM(xdrs,  &param->cs_state, Service_State_t) &&
        XDR_ENUM(xdrs,  &param->ps_state, Service_State_t) &&
		xdr_TMSI_Octets_t(xdrs, &param->tmsi) &&
        xdr_TMSI_Octets_t(xdrs, &param->ptmsi) && 
		xdr_u_int32_t(xdrs,(u_int32_t *) &param->hplmn_timer)
		)
		return(TRUE);
	else
		return(FALSE);
}


bool_t xdr_MS_GenMeasParam_t(void* xdrs, MS_GenMeasParam_t *param)
{
	XDR_LOG(xdrs,"MS_GenMeasParam_t ")

	if(	_xdr_u_char(xdrs, &param->p1,"p1")  &&
		_xdr_u_char(xdrs, &param->p2,"p2")  
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_MS_UmtsDchReport_t(void* xdrs, MS_UmtsDchReport_t *param)
{
	XDR_LOG(xdrs,"MS_UmtsDchReport_t ")

	if(	xdr_u_int32_t(xdrs,(u_int32_t *)(void*)&param->meas_bler)  &&
		xdr_u_int32_t(xdrs,(u_int32_t *)(void*)&param->target_sir)  &&
        	xdr_u_int32_t(xdrs,(u_int32_t *)(void*) &param->meas_sir)  
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_MS_UmtsMeasIdParam_t(void* xdrs, MS_UmtsMeasIdParam_t *param)
{
	XDR_LOG(xdrs,"MS_UmtsMeasIdParam_t ")

	if(	_xdr_u_char(xdrs, &param->meas_id,"meas_id")  &&
		_xdr_u_char(xdrs, &param->event_id,"event_id") && 
		xdr_opaque(xdrs, (caddr_t)&param->data_elements, 6)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_MS_UmtsMeasReport_t(void* xdrs, MS_UmtsMeasReport_t *param)
{
	XDR_LOG(xdrs,"MS_UmtsMeasReport_t ")

	if(	xdr_MS_GenMeasParam_t(xdrs, &param->gen_param) &&
		xdr_vector(xdrs,  (char *)(void *)param->param_per_meas, MAX_PARAM_PER_MEAS, sizeof(MS_UmtsMeasIdParam_t), xdr_MS_UmtsMeasIdParam_t) 
		)
	    return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_MS_UmtsGsmNcell_t(void* xdrs, MS_UmtsGsmNcell_t *param)
{
	XDR_LOG(xdrs,"MS_UmtsGsmNcell_t ")

	if(	_xdr_u_int16_t(xdrs, &param->arfcn,"arfcn") &&
		_xdr_u_char(xdrs, &param->rxlev,"rxlev")  &&
		_xdr_u_int16_t(xdrs, &param->mcc,"mcc") &&
		_xdr_u_char(xdrs, &param->mnc,"mnc")  &&
		_xdr_u_int16_t(xdrs, &param->lac,"lac") &&
		_xdr_u_int16_t(xdrs, &param->ci,"ci") &&
		_xdr_u_char(xdrs, &param->bsic,"bsic")  &&
	    xdr_int(xdrs, &param->c1) &&
        xdr_int(xdrs, &param->c2) &&	
		xdr_int(xdrs, &param->c31) &&
        xdr_int(xdrs, &param->c32) &&
		_xdr_u_char(xdrs, &param->rank_pos,"rank_pos")  &&
		_xdr_u_int16_t(xdrs, &param->ranking_value,"ranking_value")  &&
        _xdr_u_char(xdrs, &param->ranking_status,"ranking_status") 
		)
	    return(TRUE);
	else
		return(FALSE);
}





bool_t xdr_MS_UmtsGsmNcellList_t(void* xdrs, MS_UmtsGsmNcellList_t *param)
{
	XDR_LOG(xdrs,"MS_UmtsGsmNcellList_t ")
    return xdr_vector(xdrs,  (char *)(void *)param->A, MAX_NUMBER_OF_GSM_NCELLS, sizeof(MS_UmtsGsmNcell_t), (xdrproc_t)xdr_MS_UmtsGsmNcell_t); 
}




bool_t xdr_MS_UmtsNcellList_t(void* xdrs, MS_UmtsNcellList_t *param)
{
	XDR_LOG(xdrs,"MS_UmtsNcellList_t ")
    return xdr_vector(xdrs,  (char *)(void *)param->A, MAX_NUMBER_OF_UMTS_NCELLS, sizeof(MS_UmtsNcell_t), (xdrproc_t)xdr_MS_UmtsNcell_t); 
}


bool_t xdr_MS_UmtsNcell_t(void* xdrs, MS_UmtsNcell_t *param)
{
	XDR_LOG(xdrs,"MS_UmtsNcell_t ")

	if(	_xdr_u_char(xdrs, &param->cell_type,"cell_type")  &&
		_xdr_u_int16_t(xdrs, &param->dl_uarfcn,"dl_uarfcn") &&
		_xdr_u_int16_t(xdrs, &param->cpich_sc,"cpich_sc") &&
		_xdr_u_int16_t(xdrs, &param->cpich_rscp,"cpich_rscp") &&
		_xdr_u_int16_t(xdrs, &param->cpich_ecn0,"cpich_ecn0") &&
		_xdr_u_char(xdrs, &param->pathloss,"pathloss") &&
		_xdr_u_char(xdrs, &param->rank_pos,"rank_pos")  &&
		_xdr_u_int16_t(xdrs, &param->ranking_value,"ranking_value")  &&
        _xdr_u_char(xdrs, &param->ranking_status,"ranking_status") 
		)
	    return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_MS_UMTSParam_t(void* xdrs, MS_UMTSParam_t *param)
{
	XDR_LOG(xdrs,"MS_UMTSParam_t ")

	if(	_xdr_u_char(xdrs, &param->valid,"valid")  &&
		_xdr_u_char(xdrs, &param->rrc_state,"rrc_state")  &&
		_xdr_u_int16_t(xdrs, &param->rrcdc_state,"rrcdc_state") &&
		_xdr_u_int16_t(xdrs, &param->rrcbp_state,"rrcbp_state") &&
		_xdr_u_int16_t(xdrs, &param->rrcm_state,"rrcm_state") &&
		_xdr_u_int16_t(xdrs, &param->p_sc,"p_sc") &&
		_xdr_u_int16_t(xdrs, &param->lac,"lac") &&
		_xdr_u_char(xdrs, &param->rac,"rac")  &&
		xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->cell_id)  &&
		_xdr_u_int16_t(xdrs, &param->ul_uarfcn,"ul_uarfcn")  &&
        _xdr_u_int16_t(xdrs, &param->dl_uarfcn,"dl_uarfcn")  &&
        _xdr_u_int16_t(xdrs, &param->cpich_rscp,"cpich_rscp")  &&
        _xdr_u_char(xdrs, &param->cpich_ecn0,"cpich_ecn0")  &&
        _xdr_u_char(xdrs, &param->pathloss,"pathloss")  &&
        _xdr_u_char(xdrs, &param->cipher_on,"cipher_on")  &&
        _xdr_u_char(xdrs, &param->hcs_used,"hcs_used")  &&
        _xdr_u_char(xdrs, &param->high_mobility,"high_mobility")  &&
        _xdr_u_int16_t(xdrs, &param->ranking_value,"ranking_value")  &&
        _xdr_u_char(xdrs, &param->chn_rel_cause,"chn_rel_cause")  &&
		xdr_Int8(xdrs, (char *)&param->tx_pwr) &&
        xdr_Int8(xdrs, (char *)&param->rssi) &&
		_xdr_u_char(xdrs, &param->no_active_set_cells,"no_active_set_cells")  &&
		_xdr_u_char(xdrs, &param->no_virtual_active_set_cells,"no_virtual_active_set_cells")  &&
        _xdr_u_char(xdrs, &param->no_umts_ncells,"no_umts_ncells")  &&
        xdr_MS_UmtsNcellList_t(xdrs, &param->umts_ncell) &&
	    _xdr_u_char(xdrs, &param->no_gsm_ncells,"no_gsm_ncells")  &&
	  	xdr_MS_UmtsGsmNcellList_t(xdrs, &param->gsm_ncell) &&
        xdr_MS_UmtsMeasReport_t(xdrs, &param->meas_report) &&
        xdr_MS_UmtsDchReport_t(xdrs, &param->dch_report) &&
        _xdr_u_char(xdrs, &param->hsdch_reception_on,"hsdch_reception_on") &&
		_xdr_u_char(xdrs, &param->is_hsdpa_cell_capable,"is_hsdpa_cell_capable")
		)
	    return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_MS_Ext_MACParam_t(void* xdrs, MS_Ext_MACParam_t *param)
{
	XDR_LOG(xdrs,"MS_Ext_MACParam_t ")

	if(	xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->ul_tbf_idle_attempt_cnt)  &&
	    xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->ul_tbf_idle_ok_cnt)  &&
        xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->ul_tbf_trans_attempt_cnt)  &&
        xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->ul_tbf_trans_ok_cnt)  &&
        xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->dl_tbf_est_cnt)  &&
        xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->ul_tbf_abnormal_rel_cnt)  &&
		_xdr_u_char(xdrs, &param->lqmm,"lqmm") 
		)
	    return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_MS_Ext_SMParam_t(void* xdrs, MS_Ext_SMParam_t *param)
{
	XDR_LOG(xdrs,"MS_Ext_SMParam_t ")

	if(	xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->mo_pdp_attempt_cnt)  &&
	    _xdr_u_char(xdrs, &param->pdp_priority,"pdp_priority") &&
        _xdr_u_char(xdrs, &param->sec_pdp_priority,"sec_pdp_priority")
		)
	    return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_MS_Ext_L1Param_t(void* xdrs, MS_Ext_L1Param_t *param)
{
	XDR_LOG(xdrs,"MS_Ext_L1Param_t ")

	if(	_xdr_u_char(xdrs, &param->frame_state,"frame_state")  
        )
	    return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_MS_Ext_RLCParam_t(void* xdrs, MS_Ext_RLCParam_t *param)
{
	XDR_LOG(xdrs,"MS_Ext_RLCParam_t ")

	if(	_xdr_u_char(xdrs, &param->tfi,"tfi") &&
		_xdr_u_char(xdrs, &param->mac_mode,"mac_mode")  &&
        _xdr_u_char(xdrs, &param->rlc_mode,"rlc_mode")  &&
        _xdr_u_char(xdrs, &param->tbf_type,"tbf_type")  &&
		xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->n3102)  
        )
	    return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_MS_Ext_RRParam_t(void* xdrs, MS_Ext_RRParam_t *param)
{
	int i;
	bool_t ret = FALSE;

	XDR_LOG(xdrs,"MS_Ext_RRParam_t ")

	if(	xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->gsm_gsm_resel_cnt)  &&
	    xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->dcs_gsm_resel_cnt)  &&
        xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->gsm_dcs_resel_cnt)  &&
        xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->dcs_dcs_resel_cnt)  &&
        _xdr_u_char(xdrs, &param->si13_location,"si13_location")  &&
        _xdr_u_char(xdrs, &param->psi5_present,"psi5_present")  &&
        _xdr_u_char(xdrs, &param->psi_status_ind,"psi_status_ind")  &&
        _xdr_u_char(xdrs, &param->bs_cv_max,"bs_cv_max")  &&
        _xdr_u_char(xdrs, &param->pan_dec,"pan_dec")  &&
        _xdr_u_char(xdrs, &param->pan_inc,"pan_inc")  &&
        _xdr_u_char(xdrs, &param->pan_max,"pan_max")  
	  	)
	{
		ret = TRUE;
	}
	if(ret)
	{
		for( i = 0; i < GSM_HO_MAX_TYPES; i++)
		{
			if( xdr_vector(xdrs, (char *)(void *)&(param->gsm_ho_att_cnt[i]), GSM_HO_MAX_BAND_SWITCH, sizeof(UInt32),(xdrproc_t) xdr_u_int32_t) &&
		        xdr_vector(xdrs, (char *)(void *)&(param->gsm_ho_fail_cnt[i]), GSM_HO_MAX_BAND_SWITCH, sizeof(UInt32),(xdrproc_t) xdr_u_int32_t) &&
		        xdr_vector(xdrs, (char *)(void *)&(param->gsm_ho_ok_cnt[i]), GSM_HO_MAX_BAND_SWITCH, sizeof(UInt32),(xdrproc_t) xdr_u_int32_t) &&
                xdr_vector(xdrs, (char *)(void *)&(param->gsm_ho_back_cnt[i]), GSM_HO_MAX_BAND_SWITCH, sizeof(UInt32),(xdrproc_t) xdr_u_int32_t) 
		        )
			{
		        ret = TRUE;
			}
		}
	}
	return ret;	
}

bool_t xdr_EQUIV_PLMN_LIST_t(void* xdrs, EQUIV_PLMN_LIST_t *param)
{
	XDR_LOG(xdrs,"EQUIV_PLMN_LIST_t ")

	if(	
        _xdr_u_char(xdrs, &param->length,"length") &&
        xdr_vector(xdrs,  (char *)(void *)param->equiv_plmn, EQUIV_PLMN_LIST_SIZE, sizeof(PLMN_t), (xdrproc_t)xdr_PLMN_t)
		)
	    return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_MS_Ext_MMParam_t(void* xdrs, MS_Ext_MMParam_t *param)
{
	XDR_LOG(xdrs,"MS_Ext_MMParam_t ")

	if(	xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->mm_lu_ok_cnt)  &&
	    xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->mm_lu_imsi_ok_cnt)  &&
        xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->mm_lu_att_cnt)  &&
        xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->mm_lu_imsi_att_cnt)  &&
        _xdr_u_char(xdrs, &param->attach_fail_cause,"attach_fail_cause") &&
		xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->attach_attempt_cnt)  &&
        xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->attach_ok_cnt)  &&
        _xdr_u_char(xdrs, &param->ntw_init_det_cause,"ntw_init_det_cause") &&
		xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->ntw_init_detach_cnt)  &&
        _xdr_u_char(xdrs, &param->rau_fail_cause,"rau_fail_cause") && 
		xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->rau_attempt_cnt)  &&
        xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->rau_ok_cnt)  &&
        _xdr_u_char(xdrs, &param->prau_fail_cause,"prau_fail_cause") && 
		xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->prau_attempt_cnt)  &&
        xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->prau_ok_cnt)  &&
		xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->t3312_current_value)  &&
        xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->t3312_to_value)  &&
		xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->t3314_to_value)  &&
		_xdr_u_char(xdrs, &param->ready_state,"ready_state")  &&
        _xdr_u_char(xdrs, &param->forced_ready_state,"forced_ready_state")  &&
        _xdr_u_char(xdrs, &param->gea_supported,"gea_supported")  &&
        _xdr_u_char(xdrs, &param->split_pg_cycle_code,"split_pg_cycle_code")  &&
        _xdr_u_char(xdrs, &param->non_drx_tmr_val,"non_drx_tmr_val")  &&
        xdr_EQUIV_PLMN_LIST_t(xdrs, &param->eqv_plmn_list)
	  	)
	    return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_MS_Ext_SMSParam_t(void* xdrs, MS_Ext_SMSParam_t *param)
{
	XDR_LOG(xdrs,"MS_Ext_SMSParam_t ")

	if(	xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->sms_mo_cs_att_cnt)  &&
	    xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->sms_mo_gprs_att_cnt)  &&
        xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->sms_mt_cs_att_cnt)  &&
        xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->sms_mt_gprs_att_cnt)  &&
        xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->sms_mo_cs_ok_cnt)  &&
        xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->sms_mo_gprs_ok_cnt)  &&
        xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->sms_mt_cs_ok_cnt)  &&
        xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->sms_mt_gprs_ok_cnt)  &&
        xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->sms_TC1M_cnt)  &&
        xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->sms_cb_sch_msg_cnt)  &&
        xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->sms_TR1M_cnt)  &&
        xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->sms_TR2M_cnt)  &&
        xdr_u_int32_t(xdrs, (u_int32_t *)(void*) &param->sms_TRAM_cnt)  &&
		_xdr_u_char(xdrs, &param->sms_priority,"sms_priority")  
	  	)
	    return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_MS_ExtParam_t(void* xdrs, MS_ExtParam_t *param)
{
	XDR_LOG(xdrs,"MS_ExtParam_t ")
	if(	xdr_MS_Ext_SMSParam_t(xdrs, &param->sms_ext_param) &&
		xdr_MS_Ext_MMParam_t(xdrs, &param->mm_ext_param) &&
		xdr_MS_Ext_RRParam_t(xdrs, &param->rr_ext_param) &&
        xdr_MS_Ext_SMParam_t(xdrs, &param->sm_ext_param) &&
        xdr_MS_Ext_RLCParam_t(xdrs, &param->rlc_ext_param) &&
		xdr_MS_Ext_MACParam_t(xdrs, &param->mac_ext_param) &&
        xdr_MS_Ext_L1Param_t(xdrs, &param->l1_ext_param)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_MS_RxTestParam_t(void* xdrs, MS_RxTestParam_t *param)
{
	XDR_LOG(xdrs,"MS_RxTestParam_t ")

	if(	xdr_MS_MMParam_t(xdrs, &param->mm_param) &&
		xdr_MS_GSMParam_t(xdrs, &param->gsm_param) &&
		xdr_MS_UMTSParam_t(xdrs, &param->umts_param) &&
        xdr_MS_ExtParam_t(xdrs, &param->ext_param) 
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_Uas_Conn_Info(void* xdrs, Uas_Conn_Info *param)
{
    XDR_LOG(xdrs,"Uas_Conn_Info ")
	if( 		xdr_u_int32_t(xdrs, &param->mask) &&
		_xdr_u_char(xdrs, &param->in_cell_dch_state,"in_cell_dch_state") &&
		_xdr_u_char(xdrs, &param->hsdpa_ch_allocated,"hsdpa_ch_allocated")  
        )
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_MSUe3gStatusInd_t(void* xdrs, MSUe3gStatusInd_t *param)
{
	XDR_LOG(xdrs,"MSUe3gStatusInd_t ")

	if(	xdr_Uas_Conn_Info(xdrs, &param->in_uas_conn_info)  
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_MSCellInfoInd_t(void* xdrs, MSCellInfoInd_t *param)
{
	XDR_LOG(xdrs,"MSCellInfoInd_t ")

	if(	xdr_PLMNId_t(xdrs, &param->plmnId) &&  
		_xdr_u_int16_t(xdrs, &param->cellId,"cellId") &&
		_xdr_u_int16_t(xdrs, &param->lac,"lac") &&
		_xdr_u_char(xdrs, &param->rac,"rac") &&
		_xdr_u_int16_t(xdrs, &param->rncId, "rncId")
		)
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_CAPI2_Patch_Revision_Ptr_t( XDR* xdrs, CAPI2_Patch_Revision_Ptr_t* data )
{
	u_int len = SYS_IMEI_LEN;
	
	XDR_LOG(xdrs,"CAPI2_Patch_Revision_Ptr_t")

	return ( xdr_bytes(xdrs, (char **)(void*) data, &len, 512) );
}

bool_t xdr_Capi2AudioParams_t(void* xdrs, Capi2AudioParams_t *param)
{
	XDR_LOG(xdrs,"Capi2AudioParams_t")

	if(	_xdr_u_int16_t(xdrs, &param->speaker_pga,"speaker_pga") &&
		_xdr_u_int16_t(xdrs, &param->mic_pga,"mic_pga") &&
		_xdr_u_int16_t(xdrs, &param->audvoc_vslopgain,"audvoc_vslopgain") &&
		_xdr_u_int16_t(xdrs, &param->audvoc_vcfgr,"audvoc_vcfgr") &&
		_xdr_u_int16_t(xdrs, &param->voice_volume_max,"voice_volume_max") &&
		_xdr_u_int16_t(xdrs, &param->voice_volume_init,"voice_volume_init") &&
		_xdr_u_int16_t(xdrs, &param->audvoc_pslopgain,"audvoc_pslopgain") &&
		_xdr_u_int16_t(xdrs, &param->audvoc_aslopgain,"audvoc_aslopgain") &&
		_xdr_u_int16_t(xdrs, &param->audvoc_mixergain,"audvoc_mixergain")
		)
		return(TRUE);
	else
		return(FALSE);

}


bool_t xdr_ADC_StartRsp_t(XDR* xdrs, ADC_StartRsp_t *param)
{
	XDR_LOG(xdrs,"ADC_StartRsp_t ")

	if(	_xdr_u_int16_t(xdrs, &param->badr_value,"badr_value") &&
		xdr_u_int32_t(xdrs, &param->Context)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_GPIO_Field_t(XDR* xdrs, GPIO_Field_t *rsp)
{
	XDR_LOG(xdrs,"GPIO_Field_t ")

	return xdr_vector(xdrs,  (char*)rsp, GPIO_INIT_FIELD_NUM, sizeof(UInt8), (xdrproc_t)xdr_u_char);
}


bool_t xdr_Capi2GpioValue_t(XDR* xdrs, Capi2GpioValue_t *rsp)
{
	XDR_LOG(xdrs,"Capi2GpioValue_t ")

	return xdr_vector(xdrs,  (char*)rsp, GPIO_INIT_REC_NUM, sizeof(GPIO_Field_t), (xdrproc_t)xdr_GPIO_Field_t);
}


static const struct xdr_discrim CAPI2_InterTaskMsg_t_dscrm[] = {
		{ (int)MS_LOCAL_SS_ELEM_CLIP,_T("MS_LOCAL_SS_ELEM_CLIP"),		(xdrproc_t)xdr_u_char, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0 },
		{ (int)MSG_SIM_SMS_DATA_RSP,_T("MSG_SIM_SMS_DATA_RSP"),			(xdrproc_t)xdr_CAPI2_SmsSimMsg_t, sizeof( UInt8 ), NULL_capi2_proc_t, (xdrproc_t)xdr_u_char,0  },
		{ __dontcare__, _T(""), NULL_xdrproc_t ,0,0,0,0} 
};


bool_t xdr_InterTaskMsg_t(XDR* xdrs, InterTaskMsg_t *rsp)
{
	const struct xdr_discrim *choices = CAPI2_InterTaskMsg_t_dscrm;
	
	XDR_LOG(xdrs,"xdr_InterTaskMsg_t ")
	
	if( _xdr_u_int16_t(xdrs,  &(rsp->msgType), "msgType") &&
		_xdr_u_int16_t(xdrs,  &(rsp->dataLength), "dataLength") &&
		_xdr_u_char(xdrs,  &(rsp->clientID), "clientID") &&
		_xdr_u_char(xdrs,  &(rsp->usageCount), "usageCount") &&
		xdr_opaque(xdrs, (caddr_t)&rsp->sender, sizeof(T_NU_NAME)) &&
		xdr_opaque(xdrs, (caddr_t)&rsp->receiver, sizeof(T_NU_NAME))
		)
	{
		for (; choices->proc != NULL_xdrproc_t; choices++)
		{
			if (choices->value == rsp->msgType)
			{
				if(xdrs->x_op == XDR_ENCODE || xdrs->x_op == XDR_DECODE)
					_DBG_(CAPI2_TRACE("[CAPI2] InterTaskMsg_t[%d] %s : \r\n",rsp->msgType, (choices->debugStr)?choices->debugStr:"<>"));
				
				return xdr_pointer(xdrs, (char**)&(rsp->dataBuf), choices->unsize, choices->proc);

			}
		}
		_DBG_(CAPI2_TRACE("[CAPI2] InterTaskMsg_t[%d] Missing XDR entry : \r\n",rsp->msgType));
	}

	return FALSE;
}

bool_t xdr_Batt_Level_Table_t(XDR* xdrs, Batt_Level_Table_t *rsp)
{
	
	XDR_LOG(xdrs,"xdr_Batt_Level_Table_t ")
	return xdr_vector(xdrs,(char *) rsp->batt_level_table, MAX_BATTMGR_LEVELS, sizeof(UInt16),(xdrproc_t) xdr_u_int16_t);


}

bool_t xdr_CAPI2_ADC_ChannelRsp_t(XDR* xdrs, CAPI2_ADC_ChannelRsp_t *val)
{
	
	XDR_LOG(xdrs,"xdr_CAPI2_ADC_ChannelRsp_t ")

	if(_xdr_u_char(xdrs, &val->arrayLen,"arrayLen") )
		return xdr_vector(xdrs,(char *) val->IoAdc, MAX_ADC_CHANNELS, sizeof(UInt16),(xdrproc_t) xdr_u_int16_t);

	return FALSE;
}

bool_t xdr_CAPI2_ADC_ChannelReq_t(XDR* xdrs, CAPI2_ADC_ChannelReq_t *val)
{
	
	XDR_LOG(xdrs,"CAPI2_ADC_ChannelReq_t ")

	if(_xdr_u_char(xdrs, &val->arrayLen,"arrayLen") )
		return xdr_vector(xdrs,(char *) val->adcChNum, MAX_ADC_CHANNELS, sizeof(UInt16),(xdrproc_t) xdr_u_int16_t);

	return FALSE;
}


