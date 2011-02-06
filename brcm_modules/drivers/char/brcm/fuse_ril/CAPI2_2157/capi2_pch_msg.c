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
*   @file   capi2_pch_msg.c
*
*   @brief  This file implements the serialize/deserialize the request/respose
*	for PCH.
*
****************************************************************************/
#include	"capi2_reqrep.h"
#include	"capi2_pch_msg.h"

//XDR_ENUM_FUNC(MSRegState_t)
//XDR_ENUM_FUNC(MSNetAccess_t)
//XDR_ENUM_FUNC(PCHRejectCause_t)
XDR_ENUM_FUNC(PCHResponseType_t)
XDR_ENUM_FUNC(ActivateState_t)
XDR_ENUM_FUNC(RegType_t)
XDR_ENUM_FUNC(PCHActivateReason_t)
XDR_ENUM_FUNC(PCHDeactivateReason_t)
XDR_ENUM_FUNC(AttachState_t)
XDR_ENUM_FUNC(DeRegCause_t)
XDR_ENUM_FUNC(PCHContextState_t)
XDR_ENUM_FUNC(PCHSapiEnum_t)
XDR_ENUM_FUNC(IP_AddrType_t)
XDR_ENUM_FUNC(SuspendCause_t)
XDR_ENUM_FUNC(IPConfigAuthType_t)

bool_t xdr_PDP_SendPDPModifyReq_Rsp_t(XDR *xdrs, PDP_SendPDPModifyReq_Rsp_t *args);

bool_t
xdr_PCHQosProfile_t(XDR *xdrs, PCHQosProfile_t *args)
{
	XDR_LOG(xdrs,"PCHQosProfile_t ")

	if( 
		_xdr_u_char(xdrs, &args->precedence,"precedence") && 
		_xdr_u_char(xdrs, &args->delay,"delay") && 
		_xdr_u_char(xdrs, &args->reliability,"reliability") && 
		_xdr_u_char(xdrs, &args->peak,"peak") && 
		_xdr_u_char(xdrs, &args->mean,"mean") && 
		_xdr_u_char(xdrs, &args->present_3g,"present_3g") && 
		_xdr_u_char(xdrs, &args->traffic_class,"traffic_class") && 
		_xdr_u_char(xdrs, &args->delivery_order,"delivery_order") && 
		_xdr_u_char(xdrs, &args->error_sdu_delivery,"error_sdu_delivery") && 
		_xdr_u_char(xdrs, &args->max_sdu_size,"max_sdu_size") && 
		_xdr_u_char(xdrs, &args->max_bit_rate_up,"max_bit_rate_up") && 
		_xdr_u_char(xdrs, &args->max_bit_rate_down,"max_bit_rate_down") && 
		_xdr_u_char(xdrs, &args->residual_ber,"residual_ber") && 
		_xdr_u_char(xdrs, &args->sdu_error_ratio,"sdu_error_ratio") && 
		_xdr_u_char(xdrs, &args->transfer_delay,"transfer_delay") && 
		_xdr_u_char(xdrs, &args->traffic_priority,"traffic_priority") && 
		_xdr_u_char(xdrs, &args->guaranteed_bit_rate_up,"guaranteed_bit_rate_up") && 
		_xdr_u_char(xdrs, &args->guaranteed_bit_rate_down,"guaranteed_bit_rate_down") &&
		_xdr_u_char(xdrs, &args->ssd,"ssd") &&
		_xdr_u_char(xdrs, &args->si,"si") &&
		_xdr_u_char(xdrs, &args->ext_max_bit_rate_down,"ext_max_bit_rate_down") &&
		_xdr_u_char(xdrs, &args->ext_guaranteed_bit_rate_down,"ext_guaranteed_bit_rate_down") 

	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_PCHPacketFilter_T(XDR *xdrs, PCHPacketFilter_T *args)
{
	XDR_LOG(xdrs,"PCHPacketFilter_T ")

	if( 
		_xdr_u_char(xdrs, &args->packet_filter_id,"packet_filter_id") && 
		_xdr_u_char(xdrs, &args->evaluation_precedence_idx,"evaluation_precedence_idx") && 
		_xdr_u_char(xdrs, &args->protocol_number,"protocol_number") && 
		_xdr_u_char(xdrs, &args->present_SrcAddrMask,"present_SrcAddrMask") && 
		_xdr_u_char(xdrs, &args->present_prot_num,"present_prot_num") && 
		_xdr_u_char(xdrs, &args->present_dst_port_range,"present_dst_port_range") && 
		_xdr_u_char(xdrs, &args->present_src_port_range,"present_src_port_range") && 
		_xdr_u_char(xdrs, &args->present_tos,"present_tos") && 
		_xdr_u_int16_t(xdrs, &args->destination_port_low,"destination_port_low") && 
		_xdr_u_int16_t(xdrs, &args->destination_port_high,"destination_port_high") && 
		_xdr_u_int16_t(xdrs, &args->source_port_low,"source_port_low") && 
		_xdr_u_int16_t(xdrs, &args->source_port_high,"source_port_high") && 
		_xdr_u_char(xdrs, &args->tos_addr,"tos_addr") && 
		_xdr_u_char(xdrs, &args->tos_mask,"tos_mask") && 
		xdr_opaque(xdrs, (caddr_t) args->source_addr_subnet_mask, LEN_SOURCE_ADDR_SUBNET_MASK)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_PCHTrafficFlowTemplate_t(XDR *xdrs, PCHTrafficFlowTemplate_t *args)
{
	XDR_LOG(xdrs,"PCHTrafficFlowTemplate_t ")

	if( 
		_xdr_u_char(xdrs, &args->num_filters,"num_filters") && 
		xdr_vector(xdrs, (char *) args->pkt_filters, MAX_NUM_FILTERS_PER_TFT, sizeof(PCHPacketFilter_T), (xdrproc_t)xdr_PCHPacketFilter_T)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_PCHXid_t(XDR *xdrs, PCHXid_t *args)
{
	XDR_LOG(xdrs,"PCHXid_t ")

	if( 
		_xdr_u_char(xdrs, &args->hdrComp,"hdrComp") && 
		_xdr_u_char(xdrs, &args->dataComp,"dataComp")
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_PDPContext_t(XDR *xdrs, PDPContext_t *args)
{
	XDR_LOG(xdrs,"PDPContext_t ")

	if( 
		_xdr_u_char(xdrs, &args->cid,"cid") && 
		xdr_opaque(xdrs, (caddr_t) args->pdpType, 20) && 
		xdr_opaque(xdrs, (caddr_t) args->apn, 101) && 
		xdr_opaque(xdrs, (caddr_t) args->reqPdpAddress, 20) && 
		xdr_PCHXid_t(xdrs, &args->pchXid)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_PDP_GPRSMinQoS_t(XDR *xdrs, PDP_GPRSMinQoS_t *args)
{
	XDR_LOG(xdrs,"PDP_GPRSMinQoS_t ")

	if( 
		_xdr_u_char(xdrs, &args->cid,"cid") && 
		_xdr_u_char(xdrs, &args->precedence,"precedence") && 
		_xdr_u_char(xdrs, &args->delay,"delay") && 
		_xdr_u_char(xdrs, &args->reliability,"reliability") && 
		_xdr_u_char(xdrs, &args->peak,"peak") && 
		_xdr_u_char(xdrs, &args->mean,"mean")
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_PCHProtConfig_t(XDR *xdrs, PCHProtConfig_t *args)
{
	XDR_LOG(xdrs,"PCHProtConfig_t ")

	if( 
		_xdr_u_char(xdrs, &args->length,"length") && 
		xdr_opaque(xdrs, (caddr_t) args->options, 251)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_PCHPDPActivatedContext_t(XDR *xdrs, PCHPDPActivatedContext_t *args)
{
	XDR_LOG(xdrs,"PCHPDPActivatedContext_t ")

	if( 
		_xdr_u_char(xdrs, &args->cid,"cid") && 
		_xdr_u_char(xdrs, &args->nsapi,"nsapi") && 
		xdr_opaque(xdrs, (caddr_t) args->pdpAddress, 20) && 
		xdr_PCHQosProfile_t(xdrs, &args->qos) && 
		xdr_PCHProtConfig_t(xdrs, &args->protConfig)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_PCHPDPActivatedSecContext_t(XDR *xdrs, PCHPDPActivatedSecContext_t *args)
{
	XDR_LOG(xdrs,"PCHPDPActivatedSecContext_t ")

	if( 
		_xdr_u_char(xdrs, &args->cid,"cid") && 
		_xdr_u_char(xdrs, &args->nsapi,"nsapi") && 
		xdr_PCHQosProfile_t(xdrs, &args->qos) && 
		_xdr_u_char(xdrs, &args->pfi_ind,"pfi_ind") && 
		_xdr_u_char(xdrs, &args->indicatedPFI,"indicatedPFI")
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_PDP_SendPDPActivateReq_Rsp_t(XDR *xdrs, PDP_SendPDPActivateReq_Rsp_t *args)
{
	XDR_LOG(xdrs,"PDP_SendPDPActivateReq_Rsp_t ")

	if( 
		XDR_ENUM(xdrs, &(args->cause), Result_t) && 
		XDR_ENUM(xdrs, &(args->response), PCHResponseType_t) && 
		xdr_PCHPDPActivatedContext_t(xdrs, &args->activatedContext)
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_PDP_SendPDPModifyReq_Rsp_t(XDR *xdrs, PDP_SendPDPModifyReq_Rsp_t *args)
{
	XDR_LOG(xdrs,"PDP_SendPDPModifyReq_Rsp_t ")

	if( 
		XDR_ENUM(xdrs, &(args->cause), Result_t) && 
		XDR_ENUM(xdrs, &(args->response), PCHResponseType_t) && 
		xdr_PCHPDPModifiedContext_t(xdrs, &args->modifiedContext)
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_PCHPDPModifiedContext_t(XDR *xdrs, PCHPDPModifiedContext_t *args)
{
	XDR_LOG(xdrs,"PCHPDPModifiedContext_t")

	if( 
		_xdr_u_char(xdrs, &args->cid,"cid") && 
		_xdr_u_char(xdrs, &args->nsapi,"nsapi") && 
		xdr_PCHQosProfile_t(xdrs, &args->qos) 
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_PDP_SendPDPDeactivateReq_Rsp_t(XDR *xdrs, PDP_SendPDPDeactivateReq_Rsp_t *args)
{
	XDR_LOG(xdrs,"PDP_SendPDPDeactivateReq_Rsp_t ")

	if( 
		_xdr_u_char(xdrs, &args->cid,"cid") && 
		XDR_ENUM(xdrs, &(args->response), PCHResponseType_t) && 
		XDR_ENUM(xdrs, &(args->cause), Result_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_PDP_SendPDPActivateSecReq_Rsp_t(XDR *xdrs, PDP_SendPDPActivateSecReq_Rsp_t *args)
{
	XDR_LOG(xdrs,"PDP_SendPDPActivateSecReq_Rsp_t ")

	if( 
		XDR_ENUM(xdrs, &(args->response), PCHResponseType_t) && 
		XDR_ENUM(xdrs, &(args->cause), Result_t) && 
//		xdr_pointer(xdrs, (char **)(void*) &args->actContext, sizeof(PCHPDPActivatedSecContext_t), (xdrproc_t) xdr_PCHPDPActivatedSecContext_t)
		xdr_PCHPDPActivatedSecContext_t(xdrs, &args->actContext )
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_GPRSActivate_t(XDR *xdrs, GPRSActivate_t *args)
{
	XDR_LOG(xdrs,"GPRSActivate_t ")

	if( 
		_xdr_u_char(xdrs, &args->cid,"cid") && 
		XDR_ENUM(xdrs, &(args->state), ActivateState_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_PDP_GetGPRSActivateStatus_Rsp_t(XDR *xdrs, PDP_GetGPRSActivateStatus_Rsp_t *args)
{
	XDR_LOG(xdrs,"PDP_GetGPRSActivateStatus_Rsp_t ")

	if( 
		_xdr_u_char(xdrs, &args->NumContexts,"NumContexts") && 
		xdr_vector(xdrs, (char *) args->Context, MAX_CID, sizeof(GPRSActivate_t), (xdrproc_t)xdr_GPRSActivate_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_PDP_DataState_t(XDR *xdrs, PDP_DataState_t *args)
{
	XDR_LOG(xdrs,"PDP_DataState_t ")

	if( 
		XDR_ENUM(xdrs, &(args->response), PCHResponseType_t) && 
		_xdr_u_char(xdrs, &args->nsapi,"nsapi") && 
		xdr_opaque(xdrs, (caddr_t) args->pdpType, 20) && 
		xdr_opaque(xdrs, (caddr_t) args->pdpAddress, 20) && 
		XDR_ENUM(xdrs, &(args->cause), Result_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_PDPDefaultContext_t(XDR *xdrs, PDPDefaultContext_t *args)
{
	XDR_LOG(xdrs,"PDPDefaultContext_t ")

	if( 
		_xdr_u_char(xdrs, &args->cid,"cid") && 
		_xdr_u_char(xdrs, &args->priCid,"priCid") && 
		_xdr_u_char(xdrs, &args->isSecondaryPdp,"isSecondaryPdp") && 
		_xdr_u_char(xdrs, &args->nsapi,"nsapi") && 
		_xdr_u_char(xdrs, &args->sapi,"sapi") && 
		XDR_ENUM(xdrs, &(args->contextState), PCHContextState_t) && 
		xdr_opaque(xdrs, (caddr_t) args->pdpType, 20) && 
		xdr_opaque(xdrs, (caddr_t) args->apn, 101) && 
		xdr_opaque(xdrs, (caddr_t) args->reqPdpAddress, 20) && 
		xdr_PCHQosProfile_t(xdrs, &args->qos) && 
		xdr_PCHQosProfile_t(xdrs, &args->qosMin) && 
		xdr_PCHXid_t(xdrs, &args->pchXid) && 
		XDR_ENUM(xdrs, &(args->ipAddrType), IP_AddrType_t) && 
		_xdr_u_char(xdrs, &args->gprsOnly,"gprsOnly") && 
		xdr_PCHTrafficFlowTemplate_t(xdrs, &args->tft)
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_PDP_GetPCHContext_Rsp_t(XDR *xdrs, PDP_GetPCHContext_Rsp_t *args)
{
	XDR_LOG(xdrs,"PDP_GetPCHContext_Rsp_t ")

	return ( xdr_vector(xdrs, (char *) args->contextState, MAX_PDP_CONTEXTS, sizeof(PCHContextState_t), XDR_ENUM_DEF(PCHContextState_t)) );
}

bool_t
xdr_GPRSActInd_t(XDR *xdrs, GPRSActInd_t *args)
{
	XDR_LOG(xdrs,"GPRSActInd_t ")

	if(
		_xdr_u_char(xdrs, &args->cid,"cid") && 
		xdr_opaque(xdrs, (caddr_t) args->pdpType, 20) && 
		xdr_opaque(xdrs, (caddr_t) args->pdpAddress, 20) 
	  )
		return(TRUE);
	else
		return(FALSE);

}
bool_t
xdr_GPRSDeactInd_t(XDR *xdrs, GPRSDeactInd_t *args)
{
	XDR_LOG(xdrs,"GPRSDeactInd_t ")

	if(
		_xdr_u_char(xdrs, &args->cid,"cid") && 
		_xdr_u_char(xdrs, &args->cause,"cause")
	  )
		return(TRUE);
	else
		return(FALSE);

}
bool_t
xdr_PDP_PDPDeactivate_Ind_t(XDR *xdrs, PDP_PDPDeactivate_Ind_t *args)
{
	XDR_LOG(xdrs,"PDP_PDPDeactivate_Ind_t ")

	if(
		_xdr_u_char(xdrs, &args->cid,"cid") && 
		xdr_opaque(xdrs, (caddr_t) args->pdpType, 20) && 
		xdr_opaque(xdrs, (caddr_t) args->pdpAddress, 20) &&
		XDR_ENUM(xdrs, &(args->cause), Result_t) &&
		XDR_ENUM(xdrs, &(args->reason), PCHActivateReason_t )
	  )
		return(TRUE);
	else
		return(FALSE);

}

bool_t 
xdr_CAPI2_PDP_ContextID_t(XDR *xdrs, CAPI2_PDP_ContextID_t *args)
{
	XDR_LOG(xdrs,"CAPI2_PDP_ContextID_t ")

	return ( _xdr_u_char(xdrs, &args->cid,"cid") );
}

bool_t
xdr_CAPI2_PDP_SetPDPContext_t(XDR *xdrs, CAPI2_PDP_SetPDPContext_t *args)
{
	XDR_LOG(xdrs,"CAPI2_PDP_SetPDPContext_t ")

	if( 
		_xdr_u_char(xdrs, &args->cid,"cid") && 
		xdr_pointer(xdrs, (char **)(void*) &args->pcontext, sizeof(PDPContext_t), (xdrproc_t) xdr_PDPContext_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_CAPI2_PDP_SetSecPDPContext_t(XDR *xdrs, CAPI2_PDP_SetSecPDPContext_t *args)
{
	XDR_LOG(xdrs,"CAPI2_PDP_SetSecPDPContext_t ")

	if( 
		_xdr_u_char(xdrs, &args->cid,"cid") && 
		_xdr_u_char(xdrs, &args->priCid,"priCid") && 
		_xdr_u_char(xdrs, &args->dComp,"dComp") && 
		_xdr_u_char(xdrs, &args->hComp,"hComp")
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_CAPI2_PDP_SetGPRSQoS_t(XDR *xdrs, CAPI2_PDP_SetGPRSQoS_t *args)
{
	XDR_LOG(xdrs,"CAPI2_PDP_SetGPRSQoS_t ")

	if( 
		_xdr_u_char(xdrs, &args->cid,"cid") && 
		xdr_pointer(xdrs, (char **)(void*) &args->params, sizeof(PCHQosProfile_t), (xdrproc_t) xdr_PCHQosProfile_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_CAPI2_PDP_SetGPRSMinQoS_t(XDR *xdrs, CAPI2_PDP_SetGPRSMinQoS_t *args)
{
	XDR_LOG(xdrs,"CAPI2_PDP_SetGPRSMinQoS_t ")

	if( 
		_xdr_u_char(xdrs, &args->cid,"cid") && 
		xdr_pointer(xdrs, (char **)(void*) &args->qos, sizeof(PDP_GPRSMinQoS_t), (xdrproc_t) xdr_PDP_GPRSMinQoS_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_CAPI2_PDP_SendPDPActivateReq_t(XDR *xdrs, CAPI2_PDP_SendPDPActivateReq_t *args)
{
	XDR_LOG(xdrs,"CAPI2_PDP_SendPDPActivateReq_t ")

	if( 
		_xdr_u_char(xdrs, &args->cid,"cid") && 
		XDR_ENUM(xdrs, &(args->reason), PCHActivateReason_t) && 
		xdr_pointer(xdrs, (char **)(void*) &args->protConfig, sizeof(PCHProtConfig_t), (xdrproc_t) xdr_PCHProtConfig_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_CAPI2_PDP_SendPDPModifyReq_t(XDR *xdrs, CAPI2_PDP_SendPDPModifyReq_t *args)
{
	XDR_LOG(xdrs,"CAPI2_PDP_SendPDPModifyReq_t ")

	if( 
		_xdr_u_char(xdrs, &args->cid,"cid") 
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_CAPI2_PDP_SetMSClass_t(XDR *xdrs, CAPI2_PDP_SetMSClass_t *args)
{
	XDR_LOG(xdrs,"CAPI2_PDP_SetMSClass_t ")

	return ( _xdr_u_char(xdrs, &args->msClass,"msClass") );
}
bool_t
xdr_CAPI2_MS_SendDetachReq_t(XDR *xdrs, CAPI2_MS_SendDetachReq_t *args)
{
	XDR_LOG(xdrs,"CAPI2_MS_SendDetachReq_t ")

	if( 
		XDR_ENUM(xdrs, &(args->cause), DeRegCause_t) && 
		XDR_ENUM(xdrs, &(args->regType), RegType_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_CAPI2_MS_SetAttachMode_t(XDR *xdrs, CAPI2_MS_SetAttachMode_t *args)
{
	XDR_LOG(xdrs,"CAPI2_MS_SetAttachMode_t ")

	return ( _xdr_u_char(xdrs, &args->mode,"mode") );
}


bool_t
xdr_CAPI2_PDP_SetUMTSTft_t(XDR *xdrs, CAPI2_PDP_SetUMTSTft_t *args)
{
	XDR_LOG(xdrs,"CAPI2_PDP_SetUMTSTft_t ")

	if( 
		_xdr_u_char(xdrs, &args->cid,"cid") && 
		xdr_pointer(xdrs, (char **)(void*) &args->pTft_in, sizeof(PCHTrafficFlowTemplate_t), (xdrproc_t) xdr_PCHTrafficFlowTemplate_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t 
xdr_CAPI2_MS_IsGprsCallActive_t(XDR *xdrs, CAPI2_MS_IsGprsCallActive_t *args)
{
	XDR_LOG(xdrs,"CAPI2_MS_IsGprsCallActive_t ")

	return ( _xdr_u_char(xdrs, &args->chan,"chan") );
}
bool_t
xdr_CAPI2_MS_SetChanGprsCallActive_t(XDR *xdrs, CAPI2_MS_SetChanGprsCallActive_t *args)
{
	XDR_LOG(xdrs,"CAPI2_MS_SetChanGprsCallActive_t ")

	if( 
		_xdr_u_char(xdrs, &args->chan,"chan") && 
		_xdr_u_char(xdrs, &args->active,"active")
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_CAPI2_MS_SetCidForGprsActiveChan_t(XDR *xdrs, CAPI2_MS_SetCidForGprsActiveChan_t *args)
{
	XDR_LOG(xdrs,"CAPI2_MS_SetCidForGprsActiveChan_t ")

	if( 
		_xdr_u_char(xdrs, &args->chan,"chan") && 
		_xdr_u_char(xdrs, &args->cid,"cid")
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_CAPI2_MS_GetGprsActiveChanFromCid_t(XDR *xdrs, CAPI2_MS_GetGprsActiveChanFromCid_t *args)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetGprsActiveChanFromCid_t ")

	return ( _xdr_u_char(xdrs, &args->cid,"cid") );
}
bool_t 
xdr_CAPI2_MS_GetCidFromGprsActiveChan_t(XDR *xdrs, CAPI2_MS_GetCidFromGprsActiveChan_t *args)
{
	XDR_LOG(xdrs,"CAPI2_MS_GetCidFromGprsActiveChan_t ")

	return ( _xdr_u_char(xdrs, &args->chan,"chan") );
}


bool_t
xdr_PCHR99QosProfile_t(XDR *xdrs, PCHR99QosProfile_t *args)
{
	XDR_LOG(xdrs,"PCHR99QosProfile_t ")

	if( 
		_xdr_u_char(xdrs, &args->present_3g,"present_3g") && 
		_xdr_u_char(xdrs, &args->traffic_class,"traffic_class") && 
		_xdr_u_char(xdrs, &args->max_bit_rate_up,"max_bit_rate_up") && 
		_xdr_u_char(xdrs, &args->max_bit_rate_down,"max_bit_rate_down") && 
		_xdr_u_char(xdrs, &args->guaranteed_bit_rate_up,"guaranteed_bit_rate_up") && 
		_xdr_u_char(xdrs, &args->guaranteed_bit_rate_down,"guaranteed_bit_rate_down") && 
		_xdr_u_char(xdrs, &args->delivery_order,"delivery_order") && 
		_xdr_u_char(xdrs, &args->max_sdu_size,"max_sdu_size") && 
		_xdr_u_char(xdrs, &args->sdu_error_ratio,"sdu_error_ratio") && 
		_xdr_u_char(xdrs, &args->residual_ber,"residual_ber") && 
		_xdr_u_char(xdrs, &args->error_sdu_delivery,"error_sdu_delivery") && 
		_xdr_u_char(xdrs, &args->transfer_delay,"transfer_delay") && 
		_xdr_u_char(xdrs, &args->traffic_priority,"traffic_priority")
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_PCHUMTSQosProfile_t(XDR *xdrs, PCHUMTSQosProfile_t *args)
{
	XDR_LOG(xdrs,"PCHUMTQosProfile_t ")

	if( 
		xdr_PCHR99QosProfile_t(xdrs, &args->r99Qos) && 
		_xdr_u_char(xdrs, &args->ssd,"ssd") &&
		_xdr_u_char(xdrs, &args->si,"si") &&
		_xdr_u_char(xdrs, &args->ext_max_bit_rate_down,"ext_max_bit_rate_down") &&
		_xdr_u_char(xdrs, &args->ext_guaranteed_bit_rate_down,"ext_guaranteed_bit_rate_down") 
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_CAPI2_PDP_SendTBFData_t(XDR *xdrs, CAPI2_PDP_SendTBFData_t *args)
{
	XDR_LOG(xdrs,"CAPI2_PDP_SendTBFData_t ")

	if( 
		_xdr_u_char(xdrs, &args->cid,"cid") && 
		xdr_u_long(xdrs, &args->numberBytes)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_CAPI2_PDP_SetR99UMTSMinQoS_t(XDR *xdrs, CAPI2_PDP_SetR99UMTSMinQoS_t *args)
{
	XDR_LOG(xdrs,"CAPI2_PDP_SetR99UMTSMinQoS_t ")

	if( 
		_xdr_u_char(xdrs, &args->cid,"cid") && 
		xdr_pointer(xdrs, (char **)(void*) &args->pR99Qos, sizeof(PCHR99QosProfile_t), (xdrproc_t) xdr_PCHR99QosProfile_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_CAPI2_PDP_SetR99UMTSQoS_t(XDR *xdrs, CAPI2_PDP_SetR99UMTSQoS_t *args)
{
	XDR_LOG(xdrs,"CAPI2_PDP_SetR99UMTSQoS_t ")

	if( 
		_xdr_u_char(xdrs, &args->cid,"cid") && 
		xdr_pointer(xdrs, (char **)(void*) &args->pR99Qos, sizeof(PCHR99QosProfile_t), (xdrproc_t) xdr_PCHR99QosProfile_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_CAPI2_PDP_SetUMTSQoS_t(XDR *xdrs, CAPI2_PDP_SetUMTSQoS_t *args)
{
	XDR_LOG(xdrs,"CAPI2_PDP_SetUMTSQoS_t ")

	if( 
		_xdr_u_char(xdrs, &args->cid,"cid") && 
		xdr_pointer(xdrs, (char **)(void*) &args->pQos, sizeof(PCHUMTSQosProfile_t), (xdrproc_t) xdr_PCHUMTSQosProfile_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_CAPI2_PDP_TftAddFilter_t(XDR *xdrs, CAPI2_PDP_TftAddFilter_t *args)
{
	XDR_LOG(xdrs,"CAPI2_PDP_TftAddFilter_t ")

	if( 
		_xdr_u_char(xdrs, &args->cid,"cid") && 
		xdr_pointer(xdrs, (char **)(void*) &args->pPktFilter, sizeof(PCHPacketFilter_T), (xdrproc_t) xdr_PCHPacketFilter_T)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_CAPI2_PDP_SetPCHContextState_t(XDR *xdrs, CAPI2_PDP_SetPCHContextState_t *args)
{
	XDR_LOG(xdrs,"CAPI2_PDP_SetPCHContextState_t ")

	if( 
		_xdr_u_char(xdrs, &args->cid,"cid") && 
		XDR_ENUM(xdrs, &(args->contextState), PCHContextState_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_CAPI2_PDP_SetDefaultPDPContext_t(XDR *xdrs, CAPI2_PDP_SetDefaultPDPContext_t *args)
{
	XDR_LOG(xdrs,"CAPI2_PDP_SetDefaultPDPContext_t ")

	if( 
		_xdr_u_char(xdrs, &args->cid,"cid") && 
		xdr_pointer(xdrs, (char **)(void*) &args->pDefaultContext, sizeof(PDPDefaultContext_t), (xdrproc_t) xdr_PDPDefaultContext_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_PCHPDPAddress_t(XDR *xdrs, PCHPDPAddress_t *args)
{
	XDR_LOG(xdrs,"PCHPDPAddress_t ")

	if( xdr_opaque(xdrs, (caddr_t)args, 20))
		return(TRUE);
	else
		return(FALSE);

}

bool_t
xdr_Inter_ModifyContextInd_t(XDR *xdrs, Inter_ModifyContextInd_t *args)
{
	XDR_LOG(xdrs,"Inter_ModifyContextInd_t ")

	if(	_xdr_u_char(xdrs, &args->nsapi,"nsapi") && 
		_xdr_u_char(xdrs, &args->sapi,"sapi") && 
		xdr_PCHQosProfile_t(xdrs, &args->qosProfile) 
	  )
		return(TRUE);
	else
		return(FALSE);

}
bool_t
xdr_GPRSReActInd_t(XDR *xdrs, GPRSReActInd_t *args)
{

	XDR_LOG(xdrs,"GPRSReActInd_t ")

	if(	_xdr_u_char(xdrs, &args->cid,"cid") && 
		_xdr_u_char(xdrs, &args->reActivateInd,"reActivateInd") 
	  )
		return(TRUE);
	else
		return(FALSE);

}
bool_t
xdr_GPRSSuspendInd_t(XDR *xdrs, GPRSSuspendInd_t *args)
{

	XDR_LOG(xdrs,"GPRSSuspendInd_t ")

	return XDR_ENUM(xdrs, &(args->suspend_cause), SuspendCause_t); 

}

bool_t
xdr_CAPI2_PDP_ACTIVATION_RSP_t(XDR *xdrs, CAPI2_PDP_ACTIVATION_RSP_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PDP_ACTIVATION_RSP_t ")

	return(xdr_pointer(xdrs, (char **)(void*) &rsp->PDP_ACTIVATION_RSP_Rsp, sizeof(PDP_SendPDPActivateReq_Rsp_t), (xdrproc_t) xdr_PDP_SendPDPActivateReq_Rsp_t) );

}
bool_t
xdr_CAPI2_PDP_DEACTIVATION_RSP_t(XDR *xdrs, CAPI2_PDP_DEACTIVATION_RSP_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PDP_DEACTIVATION_RSP_t ")

	return(xdr_pointer(xdrs, (char **)(void*) &rsp->PDP_DEACTIVATION_RSP_Rsp, sizeof(PDP_SendPDPDeactivateReq_Rsp_t), (xdrproc_t) xdr_PDP_SendPDPDeactivateReq_Rsp_t) );

}
bool_t
xdr_CAPI2_PDP_SEC_ACTIVATION_RSP_t(XDR *xdrs, CAPI2_PDP_SEC_ACTIVATION_RSP_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PDP_SEC_ACTIVATION_RSP_t ")

	return(xdr_pointer(xdrs, (char **)(void*) &rsp->PDP_SEC_ACTIVATION_RSP_Rsp, sizeof(PDP_SendPDPActivateSecReq_Rsp_t), (xdrproc_t) xdr_PDP_SendPDPActivateSecReq_Rsp_t) );

}
bool_t
xdr_CAPI2_PDP_ACTIVATE_SNDCP_RSP_t(XDR *xdrs, CAPI2_PDP_ACTIVATE_SNDCP_RSP_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PDP_ACTIVATE_SNDCP_RSP_t ")

	return(xdr_pointer(xdrs, (char **)(void*) &rsp->PDP_ACTIVATE_SNDCP_RSP_Rsp, sizeof(PDP_DataState_t), (xdrproc_t) xdr_PDP_DataState_t) );

}
bool_t
xdr_CAPI2_GPRS_ACTIVATE_IND_t(XDR *xdrs, CAPI2_GPRS_ACTIVATE_IND_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_GPRS_ACTIVATE_IND_t ")

	return(xdr_pointer(xdrs, (char **)(void*) &rsp->GPRS_ACTIVATE_IND_Rsp, sizeof(GPRSActInd_t), (xdrproc_t) xdr_GPRSActInd_t) );

}
bool_t
xdr_CAPI2_GPRS_DEACTIVATE_IND_t(XDR *xdrs, CAPI2_GPRS_DEACTIVATE_IND_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_GPRS_DEACTIVATE_IND_t ")

	return(xdr_pointer(xdrs, (char **)(void*) &rsp->GPRS_DEACTIVATE_IND_Rsp, sizeof(GPRSDeactInd_t), (xdrproc_t) xdr_GPRSDeactInd_t) );

}
bool_t
xdr_CAPI2_PDP_DEACTIVATION_IND_t(XDR *xdrs, CAPI2_PDP_DEACTIVATION_IND_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PDP_DEACTIVATION_IND_t ")

	return(xdr_pointer(xdrs, (char **)(void*) &rsp->PDP_DEACTIVATION_IND_Rsp, sizeof(PDP_PDPDeactivate_Ind_t), (xdrproc_t) xdr_PDP_PDPDeactivate_Ind_t) );

}

bool_t
xdr_CAPI2_PDP_MODIFICATION_RSP_t(XDR *xdrs, CAPI2_PDP_MODIFICATION_RSP_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PDP_MODIFICATION_RSP_t ")

	return(xdr_pointer(xdrs, (char **)(void*) &rsp->PDP_MODIFICATION_RSP_Rsp, sizeof(PDP_SendPDPModifyReq_Rsp_t), (xdrproc_t) xdr_PDP_SendPDPModifyReq_Rsp_t) );

}

bool_t
xdr_CAPI2_GPRS_MODIFY_IND_t(XDR *xdrs, CAPI2_GPRS_MODIFY_IND_t *rsp)
{
	return(xdr_pointer(xdrs, (char **)(void*) &rsp->GPRS_MODIFY_IND_Rsp, sizeof(Inter_ModifyContextInd_t), (xdrproc_t) xdr_Inter_ModifyContextInd_t) );

}
bool_t
xdr_CAPI2_GPRS_REACT_IND_t(XDR *xdrs, CAPI2_GPRS_REACT_IND_t *rsp)
{
	return(xdr_pointer(xdrs, (char **)(void*) &rsp->GPRS_REACT_IND_Rsp, sizeof(GPRSReActInd_t), (xdrproc_t) xdr_GPRSReActInd_t) );

}
bool_t
xdr_CAPI2_DATA_SUSPEND_IND_t(XDR *xdrs, CAPI2_DATA_SUSPEND_IND_t *rsp)
{
	return(xdr_pointer(xdrs, (char **)(void*) &rsp->DATA_SUSPEND_IND_Rsp, sizeof(GPRSSuspendInd_t), (xdrproc_t) xdr_GPRSSuspendInd_t) );

}


bool_t
xdr_PCHDNSAddr_t(XDR *xdrs, PCHDNSAddr_t *args)
{
	XDR_LOG(xdrs,"PCHDNSAddr_t ")

	if( xdr_opaque(xdrs, (caddr_t)args, sizeof(PCHDNSAddr_t)))
		return(TRUE);
	else
		return(FALSE);

}

bool_t
xdr_PCHPCsCfAddr_t(XDR *xdrs, PCHPCsCfAddr_t *args)
{
	XDR_LOG(xdrs,"PCHPCsCfAddr_t ")

	if( xdr_opaque(xdrs, (caddr_t)args, sizeof(PCHPCsCfAddr_t)))
		return(TRUE);
	else
		return(FALSE);

}

bool_t
xdr_PCHDecodedProtConfig_t(XDR *xdrs, PCHDecodedProtConfig_t *args)
{
	XDR_LOG(xdrs,"PCHDecodedProtConfig_t ")

	if(	xdr_PCHPCsCfAddr_t(xdrs, &args->pcscfAddr) && 
		xdr_PCHDNSAddr_t(xdrs, &args->dnsPri) &&
		xdr_PCHDNSAddr_t(xdrs, &args->dnsPri1) &&
		xdr_PCHDNSAddr_t(xdrs, &args->dnsSec) &&
		xdr_PCHDNSAddr_t(xdrs, &args->dnsSec1)
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_CAPI2_BuildProtocolConfigOption_t(XDR *xdrs, CAPI2_BuildProtocolConfigOption_t *args)
{
	XDR_LOG(xdrs,"CAPI2_BuildProtocolConfigOption_t ")

	if(	xdr_xdr_string_t(xdrs, &args->username) && 
		xdr_xdr_string_t(xdrs, &args->password) &&
		XDR_ENUM(xdrs, &(args->authType), IPConfigAuthType_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_CHAP_ChallengeOptions_t(XDR *xdrs, CHAP_ChallengeOptions_t *args)
{
	XDR_LOG(xdrs,"CHAP_ChallengeOptions_t ")

	if(   _xdr_u_char(xdrs, &args->flag, "flag") &&
		_xdr_u_char(xdrs, &args->len, "len") &&
		xdr_opaque(xdrs, (caddr_t) &args->content, MAX_CHALLENGE_LENGTH+MAXSECRETLEN+CHAP_HEADERLEN+1))
		return(TRUE);
	else
		return(FALSE);

}

bool_t
xdr_CHAP_ResponseOptions_t(XDR *xdrs, CHAP_ResponseOptions_t *args)
{
	XDR_LOG(xdrs,"CHAP_ResponseOptions_t ")

	if(   _xdr_u_char(xdrs, &args->flag, "flag") &&
		_xdr_u_char(xdrs, &args->len, "len") &&
		xdr_opaque(xdrs, (caddr_t) &args->content, MAX_CHALLENGE_LENGTH+MAXSECRETLEN+CHAP_HEADERLEN+1))
		return(TRUE);
	else
		return(FALSE);

}

bool_t
xdr_PAP_CnfgOptions_t(XDR *xdrs, PAP_CnfgOptions_t *args)
{
	XDR_LOG(xdrs,"PAP_CnfgOptions_t ")

	if(   _xdr_u_char(xdrs, &args->flag, "flag") &&
		_xdr_u_char(xdrs, &args->len, "len") &&
		xdr_opaque(xdrs, (caddr_t) &args->content, (2*MAX_USERLENGTH)+UPAP_HEADERLEN+2))
		return(TRUE);
	else
		return(FALSE);

}

bool_t
xdr_CAPI2_BuildProtocolConfigOption2_t(XDR *xdrs, CAPI2_BuildProtocolConfigOption2_t *args)
{
	XDR_LOG(xdrs,"CAPI2_BuildProtocolConfigOption2_t ")

	if(	XDR_ENUM(xdrs, &args->authType, IPConfigAuthType_t ) &&
		xdr_pointer(xdrs, (char **)(void*) &args->cc, sizeof(CHAP_ChallengeOptions_t), (xdrproc_t) xdr_CHAP_ChallengeOptions_t) &&
		xdr_pointer(xdrs, (char **)(void*) &args->cr, sizeof(CHAP_ResponseOptions_t), (xdrproc_t) xdr_CHAP_ResponseOptions_t) &&
		xdr_pointer(xdrs, (char **)(void*) &args->po, sizeof(PAP_CnfgOptions_t), (xdrproc_t) xdr_PAP_CnfgOptions_t) )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_PCHNegotiatedParms_t(XDR *xdrs, PCHNegotiatedParms_t *args)
{


	XDR_LOG(xdrs,"PCHNegotiatedParms_t ")
	if( 
		xdr_PCHQosProfile_t(xdrs, &args->qos) && 
		xdr_PCHXid_t(xdrs, &args->xid) && 
		_xdr_u_char(xdrs, &args->sapi,"sapi")
	  )
		return(TRUE);
	else
		return(FALSE);
}
