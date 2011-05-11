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
XDR_ENUM_FUNC(PCHPDPActivateCallControlResult_t)

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
		_xdr_u_char(xdrs, &args->traffic_direction,"traffic_direction") && 
		_xdr_u_char(xdrs, &args->packet_filter_id,"packet_filter_id") && 
		_xdr_u_char(xdrs, &args->evaluation_precedence_idx,"evaluation_precedence_idx") && 
		_xdr_u_char(xdrs, &args->protocol_number,"protocol_number") && 
		_xdr_u_char(xdrs, &args->present_SrcAddrMask,"present_SrcAddrMask") && 
		_xdr_u_char(xdrs, &args->present_IPv6SrcAddrMask,"present_IPv6SrcAddrMask") && 
		_xdr_u_char(xdrs, &args->present_prot_num,"present_prot_num") && 
		_xdr_u_char(xdrs, &args->present_dst_port_range,"present_dst_port_range") && 
		_xdr_u_char(xdrs, &args->present_src_port_range,"present_src_port_range") && 
		_xdr_u_char(xdrs, &args->present_ipsec_spi,"present_ipsec_spi") && 
		_xdr_u_char(xdrs, &args->present_tos,"present_tos") && 
		_xdr_u_char(xdrs, &args->present_flow_label,"present_flow_label") && 
		_xdr_u_int16_t(xdrs, &args->destination_port_low,"destination_port_low") && 
		_xdr_u_int16_t(xdrs, &args->destination_port_high,"destination_port_high") && 
		_xdr_u_int16_t(xdrs, &args->source_port_low,"source_port_low") && 
		_xdr_u_int16_t(xdrs, &args->source_port_high,"source_port_high") && 
		_xdr_u_long(xdrs, &args->ipsecSpi,"ipsecSpi") && 
		_xdr_u_char(xdrs, &args->tos_addr,"tos_addr") && 
		_xdr_u_char(xdrs, &args->tos_mask,"tos_mask") && 
		_xdr_u_long(xdrs, &args->flowLabel,"flowLabel") && 
		xdr_opaque(xdrs, (caddr_t) args->source_addr_subnet_mask, LEN_IPV6_SOURCE_ADDR_SUBNET_MASK)
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_PCHTFTParamList_t(XDR *xdrs, PCHTFTParamList_t *args)
{
	XDR_LOG(xdrs,"PCHTFTParamList_t ")
	if( 
		_xdr_u_char(xdrs, &args->tftParamId,"tftParamId") && 
		_xdr_u_long(xdrs, &args->authAPSessId,"authAPSessId") && 
		_xdr_u_char(xdrs, &args->authEntIdSubType,"authEntIdSubType") && 
		xdr_opaque(xdrs, (caddr_t) args->authEntId, LEN_AUTH_ENT_ID) &&
		_xdr_u_int16_t(xdrs, &args->mediaComponentNum,"mediaCompNum") && 
		_xdr_u_int16_t(xdrs, &args->ipFlowNum,"ipFlowNum") && 
		_xdr_u_char(xdrs, &args->numPktFilterId,"numPktFilterId") && 
		xdr_opaque(xdrs, (caddr_t) args->pktFilterId, MAX_NUM_FILTERS_PER_TFT)
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
		_xdr_u_char(xdrs, &args->tft_opcode,"tft_opcode") && 
		_xdr_u_char(xdrs, &args->tft_ebit,"tft_ebit") && 
		_xdr_u_char(xdrs, &args->num_filters,"num_filters") && 
		xdr_vector(xdrs, (char *) args->pkt_filters, MAX_NUM_FILTERS_PER_TFT, sizeof(PCHPacketFilter_T), (xdrproc_t)xdr_PCHPacketFilter_T) &&
		_xdr_u_char(xdrs, &args->numParamList,"numParamList") && 
		xdr_vector(xdrs, (char *) args->tftParamList, MAX_NUM_PARAM_LIST_PER_TFT, sizeof(PCHTFTParamList_t), (xdrproc_t)xdr_PCHTFTParamList_t)
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
xdr_PCHPDPAddressIE_t(XDR *xdrs, PCHPDPAddressIE_t *args)
{
	XDR_LOG(xdrs,"PCHPDPAddressIE_t ")

	return xdr_opaque(xdrs, (caddr_t)args, PCH_PDP_ADDR_IE_LEN-1);

}


bool_t
xdr_PCHPDPActivatedContext_t(XDR *xdrs, PCHPDPActivatedContext_t *args)
{
	XDR_LOG(xdrs,"PCHPDPActivatedContext_t ")

	if( 
		_xdr_u_char(xdrs, &args->cid,"cid") && 
		_xdr_u_char(xdrs, &args->nsapi,"nsapi") && 
		xdr_PCHPDPAddress_t(xdrs, &args->pdpAddress) && 
		xdr_PCHQosProfile_t(xdrs, &args->qos) && 
		xdr_PCHProtConfig_t(xdrs, &args->protConfig) &&
		xdr_PCHPDPAddressIE_t(xdrs, &args->pdpAddressIE ) &&
		XDR_ENUM(xdrs, &args->cause, PCHRejectCause_t) 
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
		xdr_opaque(xdrs, (caddr_t) args->pdpType, PCH_PDP_TYPE_LEN) && 
		xdr_PCHAPN_t(xdrs, &args->apn) && 
		xdr_PCHPDPAddress_t(xdrs, &args->reqPdpAddress) && 
		xdr_PCHQosProfile_t(xdrs, &args->qos) && 
		xdr_PCHQosProfile_t(xdrs, &args->qosMin) && 
		xdr_PCHXid_t(xdrs, &args->pchXid) && 
		XDR_ENUM(xdrs, &(args->ipAddrType), IP_AddrType_t) && 
		_xdr_u_char(xdrs, &args->gprsOnly,"gprsOnly") && 
		xdr_PCHTrafficFlowTemplate_t(xdrs, &args->tft) &&
	    _xdr_u_char(xdrs, &args->bMTPdp,"bMTPdp") && 		
		xdr_long(xdrs, &args->secTiPd) &&
		_xdr_u_char(xdrs, &args->isContextActivateAfterCallControl,"isContextActivateAfterCallControl") 
		
	  )
		return(TRUE);
	else
		return(FALSE);
}


bool_t
xdr_GPRSActInd_t(XDR *xdrs, GPRSActInd_t *args)
{
	XDR_LOG(xdrs,"GPRSActInd_t ")

	if(
		_xdr_u_char(xdrs, &args->cid,"cid") && 
		xdr_opaque(xdrs, (caddr_t) args->pdpType, PCH_PDP_TYPE_LEN) && 
		xdr_PCHPDPAddress_t(xdrs, &args->pdpAddress) 
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
		xdr_opaque(xdrs, (caddr_t) args->pdpType, PCH_PDP_TYPE_LEN) && 
		xdr_PCHPDPAddress_t(xdrs, &args->pdpAddress) &&
		XDR_ENUM(xdrs, &(args->cause), Result_t) &&
		XDR_ENUM(xdrs, &(args->reason), PCHActivateReason_t )
	  )
		return(TRUE);
	else
		return(FALSE);

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
		_xdr_u_char(xdrs, &args->ext_guaranteed_bit_rate_down,"ext_guaranteed_bit_rate_down") &&
		_xdr_u_char(xdrs, &args->ext_max_bit_rate_down,"ext_max_bit_rate_up") &&
		_xdr_u_char(xdrs, &args->ext_guaranteed_bit_rate_down,"ext_guaranteed_bit_rate_up") 
	  )
		return(TRUE);
	else
		return(FALSE);
}


bool_t
xdr_PCHPDPAddress_t(XDR *xdrs, PCHPDPAddress_t *args)
{
	XDR_LOG(xdrs,"PCHPDPAddress_t ")

	return xdr_opaque(xdrs, (caddr_t)args, PCH_PDP_ADDR_LEN);

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
		xdr_PCHDNSAddr_t(xdrs, &args->dnsSec1) &&
		_xdr_UInt8(xdrs, &args->msSupportNetBCMInd, "msSupportInd") &&
		_xdr_UInt8(xdrs, &args->netPolicyCtrlRejCode, "netRejCode") &&
		_xdr_UInt8(xdrs, &args->netSelectedBearerCtrlMode, "netSelBCM") &&
	    _xdr_UInt8(xdrs, &args->bNet2MS, "bNet2MS") 
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
xdr_PCHNegotiatedParms_t(XDR *xdrs, PCHNegotiatedParms_t *args)
{


	XDR_LOG(xdrs,"PCHNegotiatedParms_t ")
	if( 
		xdr_PCHQosProfile_t(xdrs, &args->qos) && 
		xdr_PCHXid_t(xdrs, &args->xid) && 
		_xdr_u_char(xdrs, &args->sapi,"sapi") &&
		_xdr_u_char(xdrs, &args->radioPriority, "radioPriority")
	  )
		return(TRUE);
	else
		return(FALSE);
}

static bool_t xdr_copy_len(u_int* destLen, u_int srcLen)
{
	*destLen = srcLen;
	return TRUE;
}


bool_t xdr_PCHEx_ChapAuthType_t(XDR* xdrs, PCHEx_ChapAuthType_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"PCHEx_ChapAuthType_t")

	if(
		xdr_UInt8(xdrs, &rsp->challengeLen) &&
		xdr_copy_len(&len, (u_int)(rsp->challengeLen)) && xdr_bytes(xdrs, (char **)(void*)&rsp->challengeData, &len, 0xFFFF) &&
		_xdr_UInt8(xdrs, &rsp->challengeId,"challengeId") &&
		xdr_UInt8(xdrs, &rsp->rspLen) &&
		xdr_copy_len(&len, (u_int)(rsp->rspLen)) && xdr_bytes(xdrs, (char **)(void*)&rsp->rspData, &len, 0xFFFF) &&
		_xdr_UInt8(xdrs, &rsp->rspId,"rspId") &&
		xdr_UInt8(xdrs, &rsp->usrNameLen) &&
		xdr_copy_len(&len, (u_int)(rsp->usrNameLen)) && xdr_bytes(xdrs, (char **)(void*)&rsp->usrNameData, &len, 0xFFFF) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t
xdr_PDP_DataState_t(XDR *xdrs, PDP_DataState_t *args)
{
	XDR_LOG(xdrs,"PDP_DataState_t ")

	if( 
		XDR_ENUM(xdrs, &(args->response), PCHResponseType_t) && 
		_xdr_u_char(xdrs, &args->nsapi,"nsapi") && 
		xdr_opaque(xdrs, (caddr_t) args->pdpType, PCH_PDP_TYPE_LEN) && 
		xdr_PCHPDPAddress_t(xdrs, &args->pdpAddress) && 
		XDR_ENUM(xdrs, &(args->cause), Result_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_PCHL2P_t(XDR *xdrs, PCHL2P_t *args)
{
	XDR_LOG(xdrs,"PCHL2P_t ")

	if( xdr_opaque(xdrs, (caddr_t)args, sizeof(PCHL2P_t)))
		return(TRUE);
	else
		return(FALSE);

}

bool_t
xdr_PCHPDPActivatePDU_t(XDR *xdrs, PCHPDPActivatePDU_t *args)
{
	XDR_LOG(xdrs,"xdr_PCHPDPActivatePDU_t ")

	if( 	xdr_UInt16(xdrs, &args->length) &&
		xdr_opaque(xdrs, (caddr_t)args->data, PCH_PDP_ACTIVATE_PDU_LEN))
		return(TRUE);
	else
		return(FALSE);

}

bool_t
xdr_PDP_SendPDPActivatePDUReq_Rsp_t(XDR *xdrs, PDP_SendPDPActivatePDUReq_Rsp_t *args)
{
	XDR_LOG(xdrs,"PDP_SendPDPActivatePDUReq_Rsp_t ")

	if( 	xdr_PCHCid_t(xdrs, &args->cid) &&
		XDR_ENUM(xdrs, &(args->cause), Result_t) && 
		XDR_ENUM(xdrs, &(args->response), PCHResponseType_t) && 
		xdr_PCHPDPActivatePDU_t(xdrs, &args->pdu))
		return(TRUE);
	else
		return(FALSE);

}

bool_t
xdr_PCHAPN_t(XDR *xdrs, PCHAPN_t *args)
{
	XDR_LOG(xdrs,"PCHAPN_t ")

	return xdr_opaque(xdrs, (caddr_t)args, PCH_APN_LEN);

}

bool_t
xdr_PDP_ActivateNWI_Ind_t(XDR *xdrs, PDP_ActivateNWI_Ind_t *args)
{
	XDR_LOG(xdrs,"PDP_ActivateNWI_Ind_t ")

	if( 	
		xdr_opaque(xdrs, (caddr_t) args->pdpType, PCH_PDP_TYPE_LEN) &&
		xdr_PCHPDPAddress_t(xdrs, &args->pdpAddress) &&
		xdr_PCHAPN_t(xdrs, &args->apn) &&
		xdr_PCHProtConfig_t(xdrs, &args->protConfig) 
	)
		return(TRUE);
	else
		return(FALSE);

}

bool_t
xdr_PDP_ActivateSecNWI_Ind_t(XDR *xdrs, PDP_ActivateSecNWI_Ind_t *args)
{

	XDR_LOG(xdrs,"PDP_ActivateSecNWI_Ind_t ")
	if(
		xdr_u_char(xdrs, &args->priNsapi) &&
		xdr_PCHQosProfile_t(xdrs, &args->qos) && 
		xdr_PCHTrafficFlowTemplate_t(xdrs, &args->tft) && 
		xdr_PCHProtConfig_t(xdrs, &args->protConfig) && 
		xdr_long(xdrs, &args->secTiPd) &&
	1)
		return TRUE;
	else
		return FALSE;

}

