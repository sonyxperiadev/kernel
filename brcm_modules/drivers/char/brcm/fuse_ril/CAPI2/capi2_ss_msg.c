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
*   @file   capi2_ss_msg.c
*
*   @brief  This file implements the serialize/deserialize the request/respose
*	for Supplementary Services.
*
****************************************************************************/
#include	"capi2_reqrep.h"
//#include "Capi2_phonectrl_api.h"


XDR_ENUM_FUNC(SS_Mode_t)
XDR_ENUM_FUNC(SS_CallFwdReason_t)
XDR_ENUM_FUNC(SS_SvcCls_t)
XDR_ENUM_FUNC(SS_CallBarType_t)
XDR_ENUM_FUNC(CallIndex_t)
XDR_ENUM_FUNC(NetworkCause_t)
XDR_ENUM_FUNC(ServiceStatus_t)
XDR_ENUM_FUNC(USSDService_t)
XDR_ENUM_FUNC(SuppSvcStatus_t)
XDR_ENUM_FUNC(CallType_t)
XDR_ENUM_FUNC(CallStatus_t)
XDR_ENUM_FUNC(SuppSvcType_t)
XDR_ENUM_FUNC(ConfigMode_t)
XDR_ENUM_FUNC(SS_Operation_t)
XDR_ENUM_FUNC(BasicSrvType_t)
XDR_ENUM_FUNC(SS_Component_t)
XDR_ENUM_FUNC(SS_Code_t)
XDR_ENUM_FUNC(SS_SrvType_t)
//XDR_ENUM_FUNC(TypeOfNumber_t)
//XDR_ENUM_FUNC(NumberPlanId_t)
XDR_ENUM_FUNC(TypeOfSubAdd_t)
XDR_ENUM_FUNC(OddEven_t)
XDR_ENUM_FUNC(SS_PartyNotif_t)
XDR_ENUM_FUNC(SS_FwdReason_t)
XDR_ENUM_FUNC(SS_SubsOptionType_t)
XDR_ENUM_FUNC(SS_ClirOption_t)
//XDR_ENUM_FUNC(SS_ErrorCode_t)
XDR_ENUM_FUNC(SS_ProblemCode_t)


bool_t xdr_TelephoneNumber_t(XDR *xdrs, TelephoneNumber_t *args);
bool_t xdr_CallForwardClassInfo_t(XDR *xdrs, CallForwardClassInfo_t *args);
bool_t xdr_SS_ActivationClassInfo_t(XDR *xdrs, SS_ActivationClassInfo_t *args);
bool_t xdr_USSDData_t(XDR *xdrs, USSDData_t *args);
bool_t xdr_StkReportCallStatus_t(XDR *xdrs, StkReportCallStatus_t *args);
bool_t xdr_SsCallReqFail_t(XDR *xdrs, SsCallReqFail_t *args);


bool_t
xdr_CAPI2_SS_SendCallForwardReq_t(XDR *xdrs, CAPI2_SS_SendCallForwardReq_t *args)
{
	XDR_LOG(xdrs,"CAPI2_SS_SendCallForwardReq_t ")

	if( 
		XDR_ENUM(xdrs,  &(args->mode), SS_Mode_t) && 
		XDR_ENUM(xdrs,  &(args->reason), SS_CallFwdReason_t) && 
		XDR_ENUM(xdrs,  &(args->svcCls), SS_SvcCls_t) && 
		_xdr_u_char(xdrs, &args->waitToFwdSec,"waitToFwdSec") && 
		xdr_xdr_string_t(xdrs, &args->number )
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_CAPI2_SS_QueryCallForwardStatus_t(XDR *xdrs, CAPI2_SS_QueryCallForwardStatus_t *args)
{
	XDR_LOG(xdrs,"CAPI2_SS_QueryCallForwardStatus_t ")

	if( 
		XDR_ENUM(xdrs,  &(args->reason), SS_CallFwdReason_t) && 
		XDR_ENUM(xdrs,  &(args->svcCls), SS_SvcCls_t)  
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_CAPI2_SS_SendCallBarringReq_t(XDR *xdrs, CAPI2_SS_SendCallBarringReq_t *args)
{
	XDR_LOG(xdrs,"CAPI2_SS_SendCallBarringReq_t ")

	if( 
		XDR_ENUM(xdrs,  &(args->mode), SS_Mode_t) && 
		XDR_ENUM(xdrs,  &(args->callBarType), SS_CallBarType_t) && 
		XDR_ENUM(xdrs,  &(args->svcCls), SS_SvcCls_t) && 
		xdr_xdr_string_t(xdrs, &args->password )
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_CAPI2_SS_QueryCallBarringStatus_t(XDR *xdrs, CAPI2_SS_QueryCallBarringStatus_t *args)
{
	XDR_LOG(xdrs,"CAPI2_SS_QueryCallBarringStatus_t ")

	if( 
		XDR_ENUM(xdrs,  &(args->callBarType), SS_CallBarType_t) && 
		XDR_ENUM(xdrs,  &(args->svcCls), SS_SvcCls_t)  
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_CAPI2_SS_SendCallBarringPWDChangeReq_t(XDR *xdrs, CAPI2_SS_SendCallBarringPWDChangeReq_t *args)
{
	XDR_LOG(xdrs,"CAPI2_SS_SendCallBarringPWDChangeReq_t ")

	if( 
		XDR_ENUM(xdrs,  &(args->callBarType), SS_CallBarType_t) && 
		xdr_xdr_string_t(xdrs, &args->oldPwd ) && 
		xdr_xdr_string_t(xdrs, &args->newPwd ) && 
		xdr_xdr_string_t(xdrs, &args->reNewPwd )
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_CAPI2_SS_SendCallWaitingReq_t(XDR *xdrs, CAPI2_SS_SendCallWaitingReq_t *args)
{
	XDR_LOG(xdrs,"CAPI2_SS_SendCallWaitingReq_t ")

	if( 
		XDR_ENUM(xdrs,  &(args->mode), SS_Mode_t) && 
		XDR_ENUM(xdrs,  &(args->svcCls), SS_SvcCls_t)  
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t 
xdr_CAPI2_SS_QueryCallWaitingStatus_t(XDR *xdrs, CAPI2_SS_QueryCallWaitingStatus_t *args)
{
	XDR_LOG(xdrs,"CAPI2_SS_QueryCallWaitingStatus_t ")

	return ( XDR_ENUM(xdrs,  &(args->svcCls), SS_SvcCls_t) );
}

bool_t 
xdr_CAPI2_SS_SetCallingLineIDStatus_t(XDR *xdrs, CAPI2_SS_SetCallingLineIDStatus_t *args)
{
	XDR_LOG(xdrs,"CAPI2_SS_SetCallingLineIDStatus_t ")

	return ( _xdr_u_char(xdrs, &args->enable,"enable") );
}
bool_t 
xdr_CAPI2_SS_SetCallingLineRestrictionStatus_t(XDR *xdrs, CAPI2_SS_SetCallingLineRestrictionStatus_t *args)
{
	XDR_LOG(xdrs,"CAPI2_SS_SetCallingLineRestrictionStatus_t ")

	return ( XDR_ENUM(xdrs, &(args->clir_mode), CLIRMode_t) );
}
bool_t 
xdr_CAPI2_SS_SetConnectedLineIDStatus_t(XDR *xdrs, CAPI2_SS_SetConnectedLineIDStatus_t *args)
{
	XDR_LOG(xdrs,"CAPI2_SS_SetConnectedLineIDStatus_t ")

	return ( _xdr_u_char(xdrs, &args->enable,"enable") );
}

bool_t 
xdr_CAPI2_SS_SetConnectedLineRestrictionStatus_t(XDR *xdrs, CAPI2_SS_SetConnectedLineRestrictionStatus_t *args)
{
	XDR_LOG(xdrs,"CAPI2_SS_SetConnectedLineRestrictionStatus_t ")
		
	return ( _xdr_u_char(xdrs, &args->enable, "enable") );
}

bool_t
xdr_SS_IntParSetInd_t(XDR *xdrs, SS_IntParSetInd_t *args)
{
	XDR_LOG(xdrs,"SS_IntParSetInd_t ")

	if( 
		XDR_ENUM(xdrs, &(args->ssSvcType), SuppSvcType_t) && 
		_xdr_u_char(xdrs, &args->cfgParamValue,"cfgParamValue")
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_USSDString_t(XDR *xdrs, USSDString_t *args)
{
	XDR_LOG(xdrs,"USSDString_t ")


	if( 
		_xdr_u_char(xdrs, &args->used_size,"used_size") &&
 		xdr_opaque(xdrs, (caddr_t) args->string , PHASE2_MAX_USSD_STRING_SIZE+1 )  &&
		_xdr_u_char(xdrs, &args->dcs,"dcs")
        )
		return(TRUE);
	else
		return(FALSE);

}
bool_t 
xdr_CAPI2_SS_SendUSSDConnectReq_t(XDR *xdrs, CAPI2_SS_SendUSSDConnectReq_t *args)
{
	XDR_LOG(xdrs,"CAPI2_SS_SendUSSDConnectReq_t ")

	return ( xdr_pointer(xdrs, (char **)(void*) &args->ussd, sizeof(USSDString_t), (xdrproc_t) xdr_USSDString_t) );
}
bool_t 
xdr_CAPI2_SS_EndUSSDConnectReq_t(XDR *xdrs, CAPI2_SS_EndUSSDConnectReq_t *args)
{
	XDR_LOG(xdrs,"CAPI2_SS_EndUSSDConnectReq_t ")

	return ( XDR_ENUM(xdrs,  &(args->ussd_id), CallIndex_t)  );
}
bool_t
xdr_CAPI2_SS_SendUSSDData_t(XDR *xdrs, CAPI2_SS_SendUSSDData_t *args)
{
	XDR_LOG(xdrs,"CAPI2_SS_SendUSSDData_t ")

	if( 
		XDR_ENUM(xdrs,  &(args->ussd_id), CallIndex_t) && 
		_xdr_u_char(xdrs, &args->dcs,"dcs") && 
		_xdr_u_char(xdrs, &args->len,"len") 
	  )
	{
		u_int len = (u_int)args->len;
		return xdr_bytes(xdrs, (char**)(void*) &args->ussdString, &len, 1024);
	}
	else
		return(FALSE);
}

bool_t
xdr_CallForwardClassInfo_t(XDR *xdrs, CallForwardClassInfo_t *args)
{
	XDR_LOG(xdrs,"CallForwardClassInfo_t ")

	if(
		_xdr_u_char(xdrs, &args->activated,"activated") && 
		XDR_ENUM(xdrs,  &(args->ss_class), SS_SvcCls_t) && 
		xdr_TelephoneNumber_t(xdrs,  &args->forwarded_to_number) && 
		_xdr_u_char(xdrs, &args->noReplyTime,"noReplyTime")
	  )
		return(TRUE);
	else
		return(FALSE);

}
bool_t
xdr_CallForwardStatus_t(XDR *xdrs, CallForwardStatus_t *args)
{
	XDR_LOG(xdrs,"CallForwardStatus_t ")

	if(
		XDR_ENUM(xdrs,  &(args->netCause), NetworkCause_t) && 
		XDR_ENUM(xdrs,  &(args->reason), SS_CallFwdReason_t) && 
		_xdr_u_char(xdrs, &args->class_size,"class_size") &&
		xdr_vector(xdrs, (char *) args->call_forward_class_info_list,MAX_SS_CLASS_SIZE, sizeof(CallForwardClassInfo_t), (xdrproc_t)xdr_CallForwardClassInfo_t)
	  )
	  	return(TRUE);
	else
		return(FALSE);

}
bool_t
xdr_SS_ActivationClassInfo_t(XDR *xdrs, SS_ActivationClassInfo_t *args)
{
	XDR_LOG(xdrs,"SS_ActivationClassInfo_t ")

	if(
		_xdr_u_char(xdrs, &args->activated,"activated") && 
		XDR_ENUM(xdrs,  &(args->ss_class), SS_SvcCls_t) 
	  )
		return(TRUE);
	else
		return(FALSE);

}
bool_t
xdr_CallBarringStatus_t(XDR *xdrs, CallBarringStatus_t *args)
{
	XDR_LOG(xdrs,"CallBarringStatus_t ")

	if(
		XDR_ENUM(xdrs,  &(args->netCause), NetworkCause_t) && 
		XDR_ENUM(xdrs,  &(args->call_barring_type), SS_CallBarType_t) && 
		_xdr_u_char(xdrs, &args->class_size,"class_size") &&
		xdr_vector(xdrs, (char *) args->ss_activation_class_info,MAX_SS_CLASS_SIZE, sizeof(SS_ActivationClassInfo_t), (xdrproc_t)xdr_SS_ActivationClassInfo_t)
	  )
	  	return(TRUE);
	else
		return(FALSE);

}
bool_t
xdr_SS_ProvisionStatus_t(XDR *xdrs, SS_ProvisionStatus_t *args)
{
	XDR_LOG(xdrs,"SS_ProvisionStatus_t ")

	if(
		XDR_ENUM(xdrs,  &(args->netCause), NetworkCause_t) && 
		_xdr_u_char(xdrs, &args->provision_status,"provision_status") && 
		XDR_ENUM(xdrs,  &(args->serviceStatus), ServiceStatus_t) 
	  )
		return(TRUE);
	else
		return(FALSE);

}
bool_t
xdr_SS_ActivationStatus_t(XDR *xdrs, SS_ActivationStatus_t *args)
{
	XDR_LOG(xdrs,"SS_ActivationStatus_t ")

	if(
		XDR_ENUM(xdrs,  &(args->netCause), NetworkCause_t) && 
		_xdr_u_char(xdrs, &args->class_size,"class_size")  &&
		xdr_vector(xdrs, (char *) args->ss_activation_class_info,MAX_SS_CLASS_SIZE, sizeof(SS_ActivationClassInfo_t), (xdrproc_t)xdr_SS_ActivationClassInfo_t)
	  )
	  	return(TRUE);
	else
		return(FALSE);

}
bool_t
xdr_USSDData_t(XDR *xdrs, USSDData_t *args)
{
	XDR_LOG(xdrs,"USSDData_t ")

	if(
		XDR_ENUM(xdrs,  &(args->service_type), USSDService_t) && 
		_xdr_u_char(xdrs, &args->oldindex,"oldindex") && 
		_xdr_u_char(xdrs, &args->newindex,"newindex") && 
		_xdr_u_char(xdrs, &args->prob_tag,"prob_tag") && 
		_xdr_u_char(xdrs, &args->prob_code,"prob_code") && 
		XDR_ENUM(xdrs,  &(args->err_code), SuppSvcStatus_t) && 
		XDR_ENUM(xdrs,  &(args->code_type), Unicode_t) && 
		_xdr_u_char(xdrs, &args->used_size,"used_size") && 
		xdr_opaque(xdrs, (caddr_t) args->string , PHASE1_MAX_USSD_STRING_SIZE+1 )  
	  )
		return(TRUE);
	else
		return(FALSE);

}
bool_t
xdr_USSDataInfo_t(XDR *xdrs, USSDataInfo_t *args)
{
	XDR_LOG(xdrs,"USSDataInfo_t ")

	if(
		XDR_ENUM(xdrs,  &(args->call_index), CallIndex_t) && 
		xdr_USSDData_t(xdrs,  &args->ussd_data)
	  )
		return(TRUE);
	else
		return(FALSE);

}
bool_t
xdr_StkReportCallStatus_t(XDR *xdrs, StkReportCallStatus_t *args)
{
	XDR_LOG(xdrs,"StkReportCallStatus_t ")

	if(
		XDR_ENUM(xdrs,  &(args->index), CallIndex_t) && 
		XDR_ENUM(xdrs,  &(args->call_type), CallType_t) && 
		XDR_ENUM(xdrs,  &(args->status), CallStatus_t)
	  )
		return(TRUE);
	else
		return(FALSE);

}

bool_t
xdr_SsCallReqFail_t(XDR *xdrs, SsCallReqFail_t *args)
{
	XDR_LOG(xdrs,"SsCallReqFail_t ")

	return	XDR_ENUM(xdrs,  &(args->result), Result_t);
}

bool_t 
xdr_MS_LocalElemNotifyInd_t(XDR *xdrs, MS_LocalElemNotifyInd_t *args)
{


	XDR_LOG(xdrs,"MS_LocalElemNotifyInd_t ")
	return ( XDR_ENUM(xdrs, &(args->elementType), MS_Element_t) );
}

bool_t
xdr_SsApi_DialStrSrvReq_t(XDR *xdrs, SsApi_DialStrSrvReq_t *args)
{

	u_int tempSize =  args->strLen?args->strLen:0;
	
	XDR_LOG(xdrs,"SsApi_DialStrSrvReq_t ")
	if( 
		XDR_ENUM(xdrs, &(args->fdnCheck), ConfigMode_t) && 
		XDR_ENUM(xdrs, &(args->stkCheck), ConfigMode_t) && 
		XDR_ENUM(xdrs, &(args->callType), CallType_t) && 
		_xdr_u_int16_t(xdrs, &args->strLen,"strLen") && 
		 xdr_bytes(xdrs, (char **)(void*) &args->strPtr, &tempSize, 1024)
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_ClientInfo_t(XDR *xdrs, ClientInfo_t *args)
{


	XDR_LOG(xdrs,"ClientInfo_t ")
	if( 
		_xdr_u_char(xdrs, &args->clientId,"clientId") && 
		_xdr_u_long(xdrs, &args->clientRef,"refId") &&
		_xdr_u_long(xdrs, &args->dialogId,"dialogId") 
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_BasicSrvGroup_t(XDR *xdrs, BasicSrvGroup_t *args)
{


	XDR_LOG(xdrs,"BasicSrvGroup_t ")
	if( 
		XDR_ENUM(xdrs, &(args->type), BasicSrvType_t) && 
		_xdr_u_char(xdrs, &args->content,"content")
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_PartyAddress_t(XDR *xdrs, PartyAddress_t *args)
{


	XDR_LOG(xdrs,"PartyAddress_t ")
	if( 
		XDR_ENUM(xdrs, &(args->ton), TypeOfNumber_t) && 
		XDR_ENUM(xdrs, &(args->npi), NumberPlanId_t) && 
		_xdr_u_char(xdrs, &args->length,"length") && 
		xdr_opaque(xdrs, (caddr_t) args->number, MAX_PHONE_NUMBER_SIZE)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_PartySubAdd_t(XDR *xdrs, PartySubAdd_t *args)
{


	XDR_LOG(xdrs,"PartySubAdd_t ")
	if( 
		XDR_ENUM(xdrs, &(args->tos), TypeOfSubAdd_t) && 
		XDR_ENUM(xdrs, &(args->oddEven), OddEven_t) && 
		_xdr_u_char(xdrs, &args->length,"length") && 
		xdr_opaque(xdrs, (caddr_t) args->number, MAX_PHONE_SUBADDRESS_SIZE)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_SS_FwdOption_t(XDR *xdrs, SS_FwdOption_t *args)
{


	XDR_LOG(xdrs,"SS_FwdOption_t ")
	if( 
		XDR_ENUM(xdrs, &(args->notifFwd), SS_PartyNotif_t) && 
		XDR_ENUM(xdrs, &(args->notifCalling), SS_PartyNotif_t) && 
		XDR_ENUM(xdrs, &(args->fwdReason), SS_FwdReason_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_SS_CallBarFeature_t(XDR *xdrs, SS_CallBarFeature_t *args)
{


	XDR_LOG(xdrs,"SS_CallBarFeature_t ")
	if( 
		_xdr_u_char(xdrs, &args->include,"include") && 
		xdr_BasicSrvGroup_t(xdrs, &args->basicSrv) && 
		_xdr_u_char(xdrs, &args->ssStatus,"ssStatus")
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_SS_CallBarInfo_t(XDR *xdrs, SS_CallBarInfo_t *args)
{


	XDR_LOG(xdrs,"SS_CallBarInfo_t ")
	if( 
		_xdr_u_char(xdrs, &args->listSize,"listSize") && 
		xdr_vector(xdrs, (char *) args->callBarFeatureList, MAX_FEATURE_LIST_SIZE, sizeof(SS_CallBarFeature_t), xdr_SS_CallBarFeature_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_SS_SubsOption_t(XDR *xdrs, SS_SubsOption_t *args)
{


	XDR_LOG(xdrs,"SS_SubsOption_t ")
	if( 
		XDR_ENUM(xdrs, &(args->type), SS_SubsOptionType_t) && 
		_xdr_u_char(xdrs, &args->content,"content")
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_SS_SsDataInfo_t(XDR *xdrs, SS_SsDataInfo_t *args)
{


	XDR_LOG(xdrs,"SS_SsDataInfo_t ")
	if( 
		_xdr_u_char(xdrs, &args->include,"include") && 
		_xdr_u_char(xdrs, &args->ssStatus,"ssStatus") && 
		xdr_SS_SubsOption_t(xdrs, &args->subsOpt) && 
		_xdr_u_char(xdrs, &args->listSize,"listSize") && 
		xdr_vector(xdrs, (char *) args->basicSrvGroupList, MAX_FEATURE_LIST_SIZE, sizeof(BasicSrvGroup_t), xdr_BasicSrvGroup_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_SS_BasicSrvInfo_t(XDR *xdrs, SS_BasicSrvInfo_t *args)
{


	XDR_LOG(xdrs,"SS_BasicSrvInfo_t ")
	if( 
		_xdr_u_char(xdrs, &args->listSize,"listSize") && 
		xdr_vector(xdrs, (char *) args->basicSrvGroupList, MAX_FEATURE_LIST_SIZE, sizeof(BasicSrvGroup_t), xdr_BasicSrvGroup_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_SS_CcbsFeature_t(XDR *xdrs, SS_CcbsFeature_t *args)
{


	XDR_LOG(xdrs,"SS_CcbsFeature_t ")
	if( 
		_xdr_u_char(xdrs, &args->include,"include") && 
		_xdr_u_int16_t(xdrs, &args->ccbsIndex,"ccbsIndex") && 
		xdr_PartyAddress_t(xdrs, &args->partyAdd) && 
		xdr_PartySubAdd_t(xdrs, &args->partySubAdd) && 
		xdr_BasicSrvGroup_t(xdrs, &args->basicSrv)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_SS_CcbsFeatureInfo_t(XDR *xdrs, SS_CcbsFeatureInfo_t *args)
{


	XDR_LOG(xdrs,"SS_CcbsFeatureInfo_t ")
	if( 
		_xdr_u_char(xdrs, &args->listSize,"listSize") && 
		xdr_vector(xdrs, (char *) args->ccbsList, MAX_CCBS_LIST_SIZE, sizeof(SS_CcbsFeature_t), xdr_SS_CcbsFeature_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_SS_GenSrvInfo_t(XDR *xdrs, SS_GenSrvInfo_t *args)
{


	XDR_LOG(xdrs,"SS_GenSrvInfo_t ")
	if( 
		_xdr_u_char(xdrs, &args->ssStatus,"ssStatus") && 
		_xdr_u_char(xdrs, &args->include,"include") && 
		XDR_ENUM(xdrs, &(args->clir), SS_ClirOption_t) && 
		_xdr_u_int16_t(xdrs, &args->maxEntPrio,"maxEntPrio") && 
		_xdr_u_int16_t(xdrs, &args->defaultPrio,"defaultPrio") && 
		xdr_SS_CcbsFeatureInfo_t(xdrs, &args->ccbsFeatureInfo)
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SS_ReturnError_t(XDR *xdrs, SS_ReturnError_t *args)
{


	XDR_LOG(xdrs,"SS_ReturnError_t ")
	return ( XDR_ENUM(xdrs, &(args->errorCode), SS_ErrorCode_t) );
}
bool_t
xdr_SS_Reject_t(XDR *xdrs, SS_Reject_t *args)
{


	XDR_LOG(xdrs,"SS_Reject_t ")
	if( 
		XDR_ENUM(xdrs, &(args->problemType), SS_ProblemCode_t) && 
		_xdr_u_char(xdrs, &args->content,"content")
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_CauseIe_t(XDR *xdrs, CauseIe_t *args)
{


	XDR_LOG(xdrs,"CauseIe_t ")
	if( 
		_xdr_u_char(xdrs, &args->codStandard,"codStandard") && 
		_xdr_u_char(xdrs, &args->location,"location") && 
		_xdr_u_char(xdrs, &args->recommendation,"recommendation") && 
		XDR_ENUM(xdrs,	&(args->cause), Cause_t) &&
		_xdr_u_char(xdrs, &args->diagnostic,"diagnostic")
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_SS_FwdFeature_t(XDR *xdrs, SS_FwdFeature_t *args)
{


	XDR_LOG(xdrs,"SS_FwdFeature_t ")
	if( 
		_xdr_u_char(xdrs, &args->include,"include") && 
		xdr_BasicSrvGroup_t(xdrs, &args->basicSrv) && 
		_xdr_u_char(xdrs, &args->ssStatus,"ssStatus") && 
		xdr_PartyAddress_t(xdrs, &args->partyAdd) && 
		xdr_PartySubAdd_t(xdrs, &args->partySubAdd) && 
		xdr_SS_FwdOption_t(xdrs, &args->fwdOption) && 
		_xdr_u_char(xdrs, &args->noReplyTime,"noReplyTime")
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_SS_FwdFeatureInfo_t(XDR *xdrs, SS_FwdFeatureInfo_t *args)
{


	XDR_LOG(xdrs,"SS_FwdFeatureInfo_t ")
	if( 
		_xdr_u_char(xdrs, &args->listSize,"listSize") && 
		xdr_vector(xdrs, (char *) args->fwdFeatureList, MAX_FEATURE_LIST_SIZE, sizeof(SS_FwdFeature_t), xdr_SS_FwdFeature_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}


bool_t
xdr_newPwd(XDR *xdrs, UInt8 *args)
{


	XDR_LOG(xdrs,"xdr_newPwd ")
	return	xdr_opaque(xdrs, (caddr_t) args, SS_PASSWORD_LENGTH);
}

bool_t
xdr_SS_UssdInfo_t(XDR *xdrs, SS_UssdInfo_t *args)
{


	XDR_LOG(xdrs,"SS_UssdInfo_t ")
	if( 
		_xdr_u_char(xdrs, &args->dcs,"dcs") && 
		_xdr_u_char(xdrs, &args->length,"length") && 
		xdr_opaque(xdrs, (caddr_t) args->data, MAX_USSD_SIZE)
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_SS_Password_t(XDR *xdrs, SS_Password_t *args)
{


	XDR_LOG(xdrs,"SS_Password_t ")
	if( 
		xdr_opaque(xdrs, (caddr_t) args->currentPwd, SS_PASSWORD_LENGTH) && 
		xdr_opaque(xdrs, (caddr_t) args->newPwd, SS_PASSWORD_LENGTH) && 
		xdr_opaque(xdrs, (caddr_t) args->reNewPwd, SS_PASSWORD_LENGTH) && 
		_xdr_u_long(xdrs, (u_long *) &args->pwdPtr,"pwdPtr")
	  )
		return(TRUE);
	else
		return(FALSE);
}


#define _T(a) a

static const struct xdr_discrim LCS_SrvRsp_dscrm[2] = {
	{ (int)SS_OPERATION_CODE_LCS_MOLR, _T("SS_OPERATION_CODE_LCS_MOLR"), (xdrproc_t) xdr_LCS_MoLrRsp_t },
	{ __dontcare__, _T(""), NULL_xdrproc_t } };
	
static const struct xdr_discrim SS_SrvReq_dscrm[9] = {
	
	{ (int)SS_OPERATION_CODE_REGISTER_PASSWORD, _T("SS_OPERATION_CODE_REGISTER_PASSWORD"), (xdrproc_t) xdr_SS_Password_t },
	{ (int)SS_OPERATION_CODE_PROCESS_UNSTRUCTURED_SS_DATA, _T("SS_OPERATION_CODE_PROCESS_UNSTRUCTURED_SS_DATA"), (xdrproc_t) xdr_SS_UssdInfo_t },
	{ (int)SS_OPERATION_CODE_PROCESS_UNSTRUCTURED_SS_REQ, _T("SS_OPERATION_CODE_PROCESS_UNSTRUCTURED_SS_REQ"), (xdrproc_t) xdr_SS_UssdInfo_t },
	{ (int)SS_OPERATION_CODE_UNSTRUCTURED_SS_REQEST, _T("SS_OPERATION_CODE_UNSTRUCTURED_SS_REQEST"), (xdrproc_t) xdr_SS_UssdInfo_t },
	{ (int)SS_OPERATION_CODE_UNSTRUCTURED_SS_NOTIFY, _T("SS_OPERATION_CODE_UNSTRUCTURED_SS_NOTIFY"), (xdrproc_t) xdr_SS_UssdInfo_t },
	{ (int)SS_OPERATION_CODE_LCS_LOCATION_NOTIFICATION, _T("SS_OPERATION_CODE_LCS_LOCATION_NOTIFICATION"), (xdrproc_t)XDR_ENUM_DEF(LCS_VerifRsp_t) },
	{ (int)SS_OPERATION_CODE_LCS_AREA_EVENT_REPORT, _T("SS_OPERATION_CODE_LCS_AREA_EVENT_REPORT"), (xdrproc_t) xdr_LCS_AreaEventReport_t },
	{ (int)SS_OPERATION_CODE_LCS_MOLR, _T("SS_OPERATION_CODE_LCS_MOLR"), (xdrproc_t) xdr_LCS_MoLrReq_t },
	{ __dontcare__, _T(""), NULL_xdrproc_t } };
	
static const struct xdr_discrim SS_SrvRel_dscrm[15] = {
	{ (int)SS_SRV_TYPE_FORWARDING_INFORMATION, _T("SS_SRV_TYPE_FORWARDING_INFORMATION"), (xdrproc_t) xdr_SS_FwdFeatureInfo_t },
	{ (int)SS_SRV_TYPE_CALL_BARRING_INFORMATION, _T("SS_SRV_TYPE_CALL_BARRING_INFORMATION"), (xdrproc_t) xdr_SS_CallBarInfo_t },
	{ (int)SS_SRV_TYPE_SS_DATA_INFORMATION, _T("SS_SRV_TYPE_SS_DATA_INFORMATION"), (xdrproc_t) xdr_SS_SsDataInfo_t },
	{ (int)SS_SRV_TYPE_SS_STATUS, _T("SS_SRV_TYPE_SS_STATUS"), (xdrproc_t) xdr_u_char},
	{ (int)SS_SRV_TYPE_FORWARDED_TO_NUMBER, _T("SS_SRV_TYPE_FORWARDED_TO_NUMBER"), (xdrproc_t) xdr_PartyAddress_t },
	{ (int)SS_SRV_TYPE_BASIC_SRV_INFORMATION, _T("SS_SRV_TYPE_BASIC_SRV_INFORMATION"), (xdrproc_t) xdr_SS_BasicSrvInfo_t },
	{ (int)SS_SRV_TYPE_GENERIC_SRV_INFORMATION, _T("SS_SRV_TYPE_GENERIC_SRV_INFORMATION"), (xdrproc_t) xdr_SS_GenSrvInfo_t },
	{ (int)SS_SRV_TYPE_NEW_PASSWORD, _T("SS_SRV_TYPE_NEW_PASSWORD"), (xdrproc_t) xdr_newPwd },
	{ (int)SS_SRV_TYPE_PH1_USSD_INFORMATION, _T("SS_SRV_TYPE_PH1_USSD_INFORMATION"), (xdrproc_t) xdr_SS_UssdInfo_t },
	{ (int)SS_SRV_TYPE_PH2_USSD_INFORMATION, _T("SS_SRV_TYPE_PH2_USSD_INFORMATION"), (xdrproc_t) xdr_SS_UssdInfo_t },
	{ (int)SS_SRV_TYPE_USSD_SS_NOTIFY, _T("SS_SRV_TYPE_USSD_SS_NOTIFY"), (xdrproc_t) xdr_SS_UssdInfo_t },
	{ (int)SS_SRV_TYPE_RETURN_ERROR, _T("SS_SRV_TYPE_RETURN_ERROR"), (xdrproc_t) xdr_SS_ReturnError_t },
	{ (int)SS_SRV_TYPE_REJECT, _T("SS_SRV_TYPE_REJECT"), (xdrproc_t) xdr_SS_Reject_t },
	{ (int)SS_SRV_TYPE_LOCAL_ERROR, _T("SS_SRV_TYPE_LOCAL_ERROR"), (xdrproc_t)XDR_ENUM_DEF(Result_t) },
	{ __dontcare__, _T(""), NULL_xdrproc_t } };
	
static const struct xdr_discrim LCS_SrvInd_dscrm[7] = {
	{ (int)SS_OPERATION_CODE_LCS_LOCATION_NOTIFICATION, _T("SS_OPERATION_CODE_LCS_LOCATION_NOTIFICATION"), (xdrproc_t) xdr_LCS_LocNotifInfo_t },
	{ (int)SS_OPERATION_CODE_LCS_AREA_EVENT_REQUEST, _T("SS_OPERATION_CODE_LCS_AREA_EVENT_REQUEST"), (xdrproc_t) xdr_LCS_AreaEventReq_t },
	{ (int)SS_OPERATION_CODE_LCS_PERIODIC_LOCATION_REQ, _T("SS_OPERATION_CODE_LCS_PERIODIC_LOCATION_REQ"), (xdrproc_t) xdr_LCS_PeriodicLocReq_t },
	{ (int)SS_OPERATION_CODE_LCS_PERIODIC_LOCATION_CANCEL, _T("SS_OPERATION_CODE_LCS_PERIODIC_LOCATION_CANCEL"), (xdrproc_t) xdr_LCS_XxCancel_t },
	{ (int)SS_OPERATION_CODE_LCS_AREA_EVENT_CANCEL, _T("SS_OPERATION_CODE_LCS_AREA_EVENT_CANCEL"), (xdrproc_t) xdr_LCS_XxCancel_t },
	{ (int)SS_OPERATION_CODE_LCS_LOCATION_UPDATE, _T("SS_OPERATION_CODE_LCS_LOCATION_UPDATE"), (xdrproc_t) xdr_LCS_LocUpdate_t },
	{ __dontcare__, _T(""), NULL_xdrproc_t } };
	
static const struct xdr_discrim STK_SsSrvRel_dscrm[4] = {
	{ (int)SS_COMPONENT_TYPE_NONE, _T("SS_COMPONENT_TYPE_NONE"), (xdrproc_t)XDR_ENUM_DEF(SS_ErrorCode_t) },
	{ (int)SS_COMPONENT_TYPE_RETURN_ERROR, _T("SS_COMPONENT_TYPE_RETURN_ERROR"), (xdrproc_t)XDR_ENUM_DEF(SS_ErrorCode_t) },
	{ (int)SS_COMPONENT_TYPE_REJECT, _T("SS_COMPONENT_TYPE_REJECT"), (xdrproc_t) xdr_SS_Reject_t },
	{ __dontcare__, _T(""), NULL_xdrproc_t } };
	
static const struct xdr_discrim SS_SrvRsp_dscrm[4] = {
	{ (int)SS_SRV_TYPE_PH1_USSD_INFORMATION, _T("SS_SRV_TYPE_PH1_USSD_INFORMATION"), (xdrproc_t) xdr_SS_UssdInfo_t },
	{ (int)SS_SRV_TYPE_PH2_USSD_INFORMATION, _T("SS_SRV_TYPE_PH2_USSD_INFORMATION"), (xdrproc_t) xdr_SS_UssdInfo_t },
	{ (int)SS_SRV_TYPE_USSD_SS_NOTIFY, _T("SS_SRV_TYPE_USSD_SS_NOTIFY"), (xdrproc_t) xdr_SS_UssdInfo_t },
	{ __dontcare__, _T(""), NULL_xdrproc_t } };
	
static const struct xdr_discrim SS_SrvInd_dscrm[3] = {
	{ (int)SS_SRV_TYPE_PH2_USSD_INFORMATION, _T("SS_SRV_TYPE_PH2_USSD_INFORMATION"), (xdrproc_t) xdr_SS_UssdInfo_t },
	{ (int)SS_SRV_TYPE_USSD_SS_NOTIFY, _T("SS_SRV_TYPE_USSD_SS_NOTIFY"), (xdrproc_t) xdr_SS_UssdInfo_t },
	{ __dontcare__, _T(""), NULL_xdrproc_t } };

static const struct xdr_discrim SsApi_DataReq_dscrm[8] = {
	{ (int)SS_OPERATION_CODE_PROCESS_UNSTRUCTURED_SS_DATA, _T("SS_OPERATION_CODE_PROCESS_UNSTRUCTURED_SS_DATA"), (xdrproc_t) xdr_SS_UssdInfo_t },
	{ (int)SS_OPERATION_CODE_PROCESS_UNSTRUCTURED_SS_REQ, _T("SS_OPERATION_CODE_PROCESS_UNSTRUCTURED_SS_REQ"), (xdrproc_t) xdr_SS_UssdInfo_t },
	{ (int)SS_OPERATION_CODE_UNSTRUCTURED_SS_REQEST, _T("SS_OPERATION_CODE_UNSTRUCTURED_SS_REQEST"), (xdrproc_t) xdr_SS_UssdInfo_t },
	{ (int)SS_OPERATION_CODE_UNSTRUCTURED_SS_NOTIFY, _T("SS_OPERATION_CODE_UNSTRUCTURED_SS_NOTIFY"), (xdrproc_t) xdr_SS_UssdInfo_t },
	{ (int)SS_OPERATION_CODE_LCS_LOCATION_NOTIFICATION, _T("SS_OPERATION_CODE_LCS_LOCATION_NOTIFICATION"), (xdrproc_t)XDR_ENUM_DEF(LCS_VerifRsp_t) },
	{ (int)SS_OPERATION_CODE_LCS_AREA_EVENT_REPORT, _T("SS_OPERATION_CODE_LCS_AREA_EVENT_REPORT"), (xdrproc_t) xdr_LCS_AreaEventReport_t },
	{ (int)SS_OPERATION_CODE_LCS_MOLR, _T("SS_OPERATION_CODE_LCS_MOLR"), (xdrproc_t) xdr_LCS_MoLrReq_t },
	{ __dontcare__, _T(""), NULL_xdrproc_t } };


bool_t
xdr_SS_SrvReq_t(XDR *xdrs, SS_SrvReq_t *args)
{

	struct xdr_discrim *entry = NULL;
	enum_t dscm = args->operation;	


	XDR_LOG(xdrs,"SS_SrvReq_t ")
	if( 
		XDR_ENUM(xdrs, &(args->operation), SS_Operation_t) && 
		xdr_BasicSrvGroup_t(xdrs, &args->basicSrv) && 
		XDR_ENUM(xdrs, &(args->ssCode), SS_Code_t) && 
		xdr_union(xdrs, (enum_t *)&dscm, (caddr_t)(void *)&(args->param),SS_SrvReq_dscrm, NULL_xdrproc_t, &entry, NULL) &&
		xdr_u_long(xdrs, (UInt32 *) &args->expTime)

	  )
		return(TRUE);
	else
		return(FALSE);
}


bool_t
xdr_SS_SrvRel_t(XDR *xdrs, SS_SrvRel_t *args)
{

	struct xdr_discrim *entry = NULL;
	enum_t dscm = args->type;	


	XDR_LOG(xdrs,"SS_SrvRel_t ")
	if( 
		xdr_ClientInfo_t(xdrs, &args->clientInfo) && 
		_xdr_u_char(xdrs, &args->callIndex,"callIndex") && 
		XDR_ENUM(xdrs, &(args->component), SS_Component_t) && 
		XDR_ENUM(xdrs, &(args->operation), SS_Operation_t) && 
		XDR_ENUM(xdrs, &(args->ssCode), SS_Code_t) && 
		xdr_BasicSrvGroup_t(xdrs, &args->basicSrv) && 
		XDR_ENUM(xdrs, &(args->type), SS_SrvType_t) && 
		XDR_ENUM(xdrs, &(args->localResult), Result_t) && 
		xdr_CauseIe_t(xdrs, &args->causeIe) && 
		xdr_int(xdrs, &args->facIeLength ) && 
	   	xdr_opaque(xdrs, (caddr_t) args->facIeData , MAX_FACILITY_IE_LENGTH ) 

	  )
	{    		
		if((args->type == SS_SRV_TYPE_NONE)	|| (args->type == SS_SRV_TYPE_CAUSE_IE))
		{
			//No union data in SS_SrvRel_t
			return(TRUE);
		}
		else
		{
			return xdr_union(xdrs, (enum_t *)&dscm, (caddr_t)(void *)&(args->param),SS_SrvRel_dscrm, NULL_xdrproc_t, &entry, NULL);	
		}
	} 
	else
	{
		return(FALSE);
	}
}

bool_t
xdr_LCS_SrvInd_t(XDR *xdrs, LCS_SrvInd_t *args)
{

	struct xdr_discrim *entry = NULL;
	enum_t dscm =  args->operation;	


	XDR_LOG(xdrs,"LCS_SrvInd_t ")
	if( 
		xdr_ClientInfo_t(xdrs, &args->clientInfo) && 
		XDR_ENUM(xdrs, &(args->operation), SS_Operation_t) && 
		xdr_union(xdrs, (enum_t *)&dscm, (caddr_t)(void *)&(args->param),LCS_SrvInd_dscrm, NULL_xdrproc_t, &entry, NULL)
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_LCS_SrvRsp_t(XDR *xdrs, LCS_SrvRsp_t *args)
{

	struct xdr_discrim *entry = NULL;
	enum_t dscm = args->operation;	//Check this


	XDR_LOG(xdrs,"LCS_SrvRsp_t ")
	if( 
		xdr_ClientInfo_t(xdrs, &args->clientInfo) && 
		XDR_ENUM(xdrs, &(args->operation), SS_Operation_t) && 
		xdr_union(xdrs, (enum_t *)&dscm, (caddr_t)(void *)&(args->param),LCS_SrvRsp_dscrm, NULL_xdrproc_t, &entry, NULL)
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_SS_SrvRsp_t(XDR *xdrs, SS_SrvRsp_t *args)
{

	struct xdr_discrim *entry = NULL;
	enum_t dscm = args->type;	//Check this


	XDR_LOG(xdrs,"SS_SrvRsp_t ")
	if( 
		xdr_ClientInfo_t(xdrs, &args->clientInfo) && 
		_xdr_u_char(xdrs, &args->callIndex,"callIndex") && 
		XDR_ENUM(xdrs, &(args->component), SS_Component_t) && 
		XDR_ENUM(xdrs, &(args->operation), SS_Operation_t) && 
		XDR_ENUM(xdrs, &(args->ssCode), SS_Code_t) && 
		XDR_ENUM(xdrs, &(args->type), SS_SrvType_t) && 
		xdr_union(xdrs, (enum_t *)&dscm, (caddr_t)(void *)&(args->param),SS_SrvRsp_dscrm, NULL_xdrproc_t, &entry, NULL) && 
		xdr_int(xdrs, &args->facIeLength) && 
	    xdr_opaque(xdrs, (caddr_t) args->facIeData , MAX_FACILITY_IE_LENGTH ) 
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_SS_SrvInd_t(XDR *xdrs, SS_SrvInd_t *args)
{

	struct xdr_discrim *entry = NULL;
	enum_t dscm = args->type;


	XDR_LOG(xdrs,"SS_SrvInd_t ")
	if( 
		xdr_ClientInfo_t(xdrs, &args->clientInfo) && 
		_xdr_u_char(xdrs, &args->callIndex,"callIndex") && 
		XDR_ENUM(xdrs, &(args->operation), SS_Operation_t) && 
		XDR_ENUM(xdrs, &(args->type), SS_SrvType_t) && 
		xdr_union(xdrs, (enum_t *)&dscm, (caddr_t)(void *)&(args->param),SS_SrvInd_dscrm, NULL_xdrproc_t, &entry, NULL) && 
		xdr_int(xdrs, &args->facIeLength) && 
		xdr_opaque(xdrs, (caddr_t) args->facIeData , MAX_FACILITY_IE_LENGTH ) 
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_STK_SsSrvRel_t(XDR *xdrs, STK_SsSrvRel_t *args)
{

	struct xdr_discrim *entry = NULL;
	enum_t dscm = args->component;	


	XDR_LOG(xdrs,"STK_SsSrvRel_t ")
	if( 
		xdr_ClientInfo_t(xdrs, &args->clientInfo) && 
		_xdr_u_char(xdrs, &args->callIndex,"callIndex") && 
		xdr_CauseIe_t(xdrs, &args->causeIe) && 
		XDR_ENUM(xdrs, &(args->operation), SS_Operation_t) && 
		XDR_ENUM(xdrs, &(args->component), SS_Component_t) && 
		xdr_union(xdrs, (enum_t *)&dscm, (caddr_t)(void *)&(args->param),STK_SsSrvRel_dscrm, NULL_xdrproc_t, &entry, NULL) && 
		xdr_int(xdrs, &args->paramLen) && 
		xdr_bytes(xdrs, (char **) &args->paramPtr, (u_int *)&args->paramLen, ~0)
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_SsApi_DataReq_t(XDR *xdrs, SsApi_DataReq_t *args)
{

	struct xdr_discrim *entry = NULL;
	enum_t dscm = args->operation;	

	XDR_LOG(xdrs,"SsApi_DataReq_t ")
	if( 
		XDR_ENUM(xdrs, &(args->operation), SS_Operation_t) && 
		xdr_union(xdrs, (enum_t *)&dscm, (caddr_t)(void *)&(args->param),SsApi_DataReq_dscrm, NULL_xdrproc_t, &entry, NULL)
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_SsApi_SrvReq_t(XDR *xdrs, SsApi_SrvReq_t *args)
{


	XDR_LOG(xdrs,"SsApi_SrvReq_t ")
	if( 
		XDR_ENUM(xdrs, &(args->fdnCheck), ConfigMode_t) && 
		XDR_ENUM(xdrs, &(args->stkCheck), ConfigMode_t) && 
		xdr_SS_SrvReq_t(xdrs, &args->ssSrvReq)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_SsApi_UssdSrvReq_t(XDR *xdrs, SsApi_UssdSrvReq_t *args)
{


	XDR_LOG(xdrs,"SsApi_UssdSrvReq_t ")
	if( 
		XDR_ENUM(xdrs, &(args->fdnCheck), ConfigMode_t) && 
		XDR_ENUM(xdrs, &(args->stkCheck), ConfigMode_t) && 
		XDR_ENUM(xdrs, &(args->operation), SS_Operation_t) && 
		xdr_SS_UssdInfo_t(xdrs, &args->ussdInfo)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_SsApi_UssdDataReq_t(XDR *xdrs, SsApi_UssdDataReq_t *args)
{


	XDR_LOG(xdrs,"SsApi_UssdDataReq_t ")
	if( 
		XDR_ENUM(xdrs, &(args->operation), SS_Operation_t) && 
		xdr_SS_UssdInfo_t(xdrs, &args->ussdInfo)
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_CAPI2_SsApi_DialStrSrvReq_t(XDR *xdrs, CAPI2_SsApi_DialStrSrvReq_t *args)
{


	XDR_LOG(xdrs,"CAPI2_SsApi_DialStrSrvReq_t ")
	if( 
		xdr_pointer(xdrs, (char **) &args->inClientInfoPtr, sizeof(ClientInfo_t), (xdrproc_t) xdr_ClientInfo_t) && 
		xdr_pointer(xdrs, (char **)(void*) &args->inDialStrSrvReqPtr, sizeof(SsApi_DialStrSrvReq_t), (xdrproc_t) xdr_SsApi_DialStrSrvReq_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_CAPI2_SsApi_SsSrvReq_t(XDR *xdrs, CAPI2_SsApi_SsSrvReq_t *args)
{


	XDR_LOG(xdrs,"CAPI2_SsApi_SsSrvReq_t ")
	if( 
		xdr_pointer(xdrs, (char **) &args->inClientInfoPtr, sizeof(ClientInfo_t), (xdrproc_t) xdr_ClientInfo_t) && 
		xdr_pointer(xdrs, (char **) &args->inApiSrvReqPtr, sizeof(SsApi_SrvReq_t), (xdrproc_t) xdr_SsApi_SrvReq_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_CAPI2_SsApi_UssdSrvReq_t(XDR *xdrs, CAPI2_SsApi_UssdSrvReq_t *args)
{


	XDR_LOG(xdrs,"CAPI2_SsApi_UssdSrvReq_t ")
	if( 
		xdr_pointer(xdrs, (char **) &args->inClientInfoPtr, sizeof(ClientInfo_t), (xdrproc_t) xdr_ClientInfo_t) && 
		xdr_pointer(xdrs, (char **) &args->inUssdSrvReqPtr, sizeof(SsApi_UssdSrvReq_t), (xdrproc_t) xdr_SsApi_UssdSrvReq_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_CAPI2_SsApi_UssdDataReq_t(XDR *xdrs, CAPI2_SsApi_UssdDataReq_t *args)
{


	XDR_LOG(xdrs,"CAPI2_SsApi_UssdDataReq_t ")
	if( 
		xdr_pointer(xdrs, (char **) &args->inClientInfoPtr, sizeof(ClientInfo_t), (xdrproc_t) xdr_ClientInfo_t) && 
		xdr_pointer(xdrs, (char **) &args->inUssdDataReqPtr, sizeof(SsApi_UssdDataReq_t), (xdrproc_t) xdr_SsApi_UssdDataReq_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_CAPI2_SsApi_SsReleaseReq_t(XDR *xdrs, CAPI2_SsApi_SsReleaseReq_t *args)
{


	XDR_LOG(xdrs,"CAPI2_SsApi_SsReleaseReq_t ")
	if( 
		xdr_pointer(xdrs, (char **) &args->inClientInfoPtr, sizeof(ClientInfo_t), (xdrproc_t) xdr_ClientInfo_t) && 
		xdr_pointer(xdrs, (char **) &args->inApiSrvReqPtr, sizeof(SsApi_SrvReq_t), (xdrproc_t) xdr_SsApi_SrvReq_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_CAPI2_SsApi_DataReq_t(XDR *xdrs, CAPI2_SsApi_DataReq_t *args)
{


	XDR_LOG(xdrs,"CAPI2_SsApi_DataReq_t ")
	if( 
		xdr_pointer(xdrs, (char **) &args->inClientInfoPtr, sizeof(ClientInfo_t), (xdrproc_t) xdr_ClientInfo_t) && 
		xdr_pointer(xdrs, (char **) &args->inDataReqPtr, sizeof(SsApi_DataReq_t), (xdrproc_t) xdr_SsApi_DataReq_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_CAPI2_SS_CALL_FORWARD_RSP_t(XDR *xdrs, CAPI2_SS_CALL_FORWARD_RSP_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_CALL_FORWARD_RSP_t ")

	return(xdr_pointer(xdrs, (char **)(void*) &rsp->SS_CALL_FORWARD_RSP_Rsp, sizeof(NetworkCause_t), (xdrproc_t) XDR_ENUM_DEF(NetworkCause_t)) );

}

bool_t
xdr_CAPI2_SS_CALL_FORWARD_STATUS_RSP_t(XDR *xdrs, CAPI2_SS_CALL_FORWARD_STATUS_RSP_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_CALL_FORWARD_STATUS_RSP_t ")

	return(xdr_pointer(xdrs, (char **)(void*) &rsp->SS_CALL_FORWARD_STATUS_RSP_Rsp, sizeof(CallForwardStatus_t), (xdrproc_t) xdr_CallForwardStatus_t) );

}
bool_t
xdr_CAPI2_SS_CALL_BARRING_RSP_t(XDR *xdrs, CAPI2_SS_CALL_BARRING_RSP_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_CALL_BARRING_RSP_t ")

	return(xdr_pointer(xdrs, (char **)(void*) &rsp->SS_CALL_BARRING_RSP_Rsp, sizeof(NetworkCause_t), (xdrproc_t) XDR_ENUM_DEF(NetworkCause_t)) );

}
bool_t
xdr_CAPI2_SS_CALL_BARRING_STATUS_RSP_t(XDR *xdrs, CAPI2_SS_CALL_BARRING_STATUS_RSP_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_CALL_BARRING_STATUS_RSP_t ")

	return(xdr_pointer(xdrs, (char **)(void*) &rsp->SS_CALL_BARRING_STATUS_RSP_Rsp, sizeof(CallBarringStatus_t), (xdrproc_t) xdr_CallBarringStatus_t) );

}
bool_t
xdr_CAPI2_SS_CALL_BARRING_PWD_CHANGE_RSP_t(XDR *xdrs, CAPI2_SS_CALL_BARRING_PWD_CHANGE_RSP_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_CALL_BARRING_PWD_CHANGE_RSP_t ")

	return(xdr_pointer(xdrs, (char **)(void*) &rsp->SS_CALL_BARRING_PWD_CHANGE_RSP_Rsp, sizeof(NetworkCause_t), (xdrproc_t) XDR_ENUM_DEF(NetworkCause_t)) );

}
bool_t
xdr_CAPI2_SS_CALLING_LINE_ID_STATUS_RSP_t(XDR *xdrs, CAPI2_SS_CALLING_LINE_ID_STATUS_RSP_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_CALLING_LINE_ID_STATUS_RSP_t ")

	return(xdr_pointer(xdrs, (char **)(void*) &rsp->SS_CALLING_LINE_ID_STATUS_RSP_Rsp, sizeof(SS_ProvisionStatus_t), (xdrproc_t) xdr_SS_ProvisionStatus_t) );

}
bool_t
xdr_CAPI2_SS_CALL_WAITING_RSP_t(XDR *xdrs, CAPI2_SS_CALL_WAITING_RSP_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_CALL_WAITING_RSP_t ")

	return(xdr_pointer(xdrs, (char **)(void*) &rsp->SS_CALL_WAITING_RSP_Rsp, sizeof(NetworkCause_t), (xdrproc_t) XDR_ENUM_DEF(NetworkCause_t)) );

}
bool_t
xdr_CAPI2_SS_CALL_WAITING_STATUS_RSP_t(XDR *xdrs, CAPI2_SS_CALL_WAITING_STATUS_RSP_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_CALL_WAITING_STATUS_RSP_t ")

	return(xdr_pointer(xdrs, (char **)(void*) &rsp->SS_CALL_WAITING_STATUS_RSP_Rsp, sizeof(SS_ActivationStatus_t), (xdrproc_t) xdr_SS_ActivationStatus_t) );

}
bool_t
xdr_CAPI2_SS_CONNECTED_LINE_STATUS_RSP_t(XDR *xdrs, CAPI2_SS_CONNECTED_LINE_STATUS_RSP_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_CONNECTED_LINE_STATUS_RSP_t ")

	return(xdr_pointer(xdrs, (char **)(void*) &rsp->SS_CONNECTED_LINE_STATUS_RSP_Rsp, sizeof(SS_ProvisionStatus_t), (xdrproc_t) xdr_SS_ProvisionStatus_t) );

}
bool_t
xdr_CAPI2_SS_CALLING_LINE_RESTRICTION_STATUS_RSP_t(XDR *xdrs, CAPI2_SS_CALLING_LINE_RESTRICTION_STATUS_RSP_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_CALLING_LINE_RESTRICTION_STATUS_RSP_t ")

	return(xdr_pointer(xdrs, (char **)(void*) &rsp->SS_CALLING_LINE_RESTRICTION_STATUS_RSP_Rsp, sizeof(SS_ProvisionStatus_t), (xdrproc_t) xdr_SS_ProvisionStatus_t) );

}
bool_t
xdr_CAPI2_SS_CONNECTED_LINE_RESTRICTION_STATUS_RSP_t(XDR *xdrs, CAPI2_SS_CONNECTED_LINE_RESTRICTION_STATUS_RSP_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_CONNECTED_LINE_RESTRICTION_STATUS_RSP_t ")

	return(xdr_pointer(xdrs, (char **)(void*) &rsp->SS_CONNECTED_LINE_RESTRICTION_STATUS_RSP_Rsp, sizeof(SS_ProvisionStatus_t), (xdrproc_t) xdr_SS_ProvisionStatus_t) );

}
bool_t
xdr_CAPI2_SS_CALLING_NAME_PRESENT_STATUS_RSP_t(XDR *xdrs, CAPI2_SS_CALLING_NAME_PRESENT_STATUS_RSP_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_CALLING_NAME_PRESENT_STATUS_RSP_t ")

	return(xdr_pointer(xdrs, (char **)(void*) &rsp->SS_CALLING_NAME_PRESENT_STATUS_RSP_Rsp, sizeof(SS_ProvisionStatus_t), (xdrproc_t) xdr_SS_ProvisionStatus_t) );

}
bool_t
xdr_CAPI2_SS_CALL_REQ_FAIL_t(XDR *xdrs, CAPI2_SS_CALL_REQ_FAIL_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_CALL_REQ_FAIL_t ")

	return(xdr_pointer(xdrs, (char **)(void*) &rsp->SS_CALL_REQ_FAIL_Rsp, sizeof(SsCallReqFail_t), (xdrproc_t) xdr_SsCallReqFail_t) );

}
bool_t
xdr_CAPI2_USSD_DATA_RSP_t(XDR *xdrs, CAPI2_USSD_DATA_RSP_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USSD_DATA_RSP_t ")

	return(xdr_pointer(xdrs, (char **)(void*) &rsp->USSD_DATA_RSP_Rsp, sizeof(USSDataInfo_t), (xdrproc_t) xdr_USSDataInfo_t) );

}
bool_t
xdr_CAPI2_USSD_DATA_IND_t(XDR *xdrs, CAPI2_USSD_DATA_IND_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USSD_DATA_IND_t ")

	return(xdr_pointer(xdrs, (char **)(void*) &rsp->USSD_DATA_IND_Rsp, sizeof(USSDataInfo_t), (xdrproc_t) xdr_USSDataInfo_t) );

}
bool_t
xdr_CAPI2_USSD_SESSION_END_IND_t(XDR *xdrs, CAPI2_USSD_SESSION_END_IND_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USSD_SESSION_END_IND_t ")

	return(xdr_pointer(xdrs, (char **)(void*) &rsp->USSD_SESSION_END_IND_Rsp, sizeof(CallIndex_t), (xdrproc_t) xdr_CallIndex_t) );

}
bool_t
xdr_CAPI2_USSD_CALLINDEX_IND_t(XDR *xdrs, CAPI2_USSD_CALLINDEX_IND_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USSD_CALLINDEX_IND_t ")

	return(xdr_pointer(xdrs, (char **)(void*) &rsp->USSD_CALLINDEX_IND_Rsp, sizeof(StkReportCallStatus_t), (xdrproc_t) xdr_StkReportCallStatus_t) );

}

bool_t
xdr_CAPI2_SS_INTERNAL_PARAM_SET_IND_t(XDR *xdrs, CAPI2_SS_INTERNAL_PARAM_SET_IND_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SS_INTERNAL_PARAM_SET_IND_t ")

	return(xdr_pointer(xdrs, (char **)(void*) &rsp->SS_INTERNAL_PARAM_SET_IND_Rsp, sizeof(SS_IntParSetInd_t), (xdrproc_t) xdr_SS_IntParSetInd_t) );

}

bool_t
xdr_CAPI2_MS_LOCAL_ELEM_NOTIFY_IND_t(XDR *xdrs, CAPI2_MS_LOCAL_ELEM_NOTIFY_IND_t *rsp)
{

	XDR_LOG(xdrs,"CAPI2_MS_LOCAL_ELEM_NOTIFY_IND_t ")
	return(xdr_pointer(xdrs, (char **)(void*) &rsp->MS_LOCAL_ELEM_NOTIFY_IND_Rsp, sizeof(MS_LocalElemNotifyInd_t), (xdrproc_t) xdr_MS_LocalElemNotifyInd_t) );

}

