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
*   @file   capi2_phonebk_msg.c
*
*   @brief  This file implements the serialize/deserialize the request/respose
*	for Call Control.
*
****************************************************************************/
#include    "mobcom_types.h"
#include    "resultcode.h"
#include "taskmsgs.h"
#include "consts.h"

#include "xdr_porting_layer.h"
#include "xdr.h"

#include    "common_defs.h"
#include "uelbs_api.h"
#include    "ms_database_def.h"
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
#include "capi2_isim_msg.h"
#include "capi2_sim_msg.h"


#define _T(a) a

XDR_ENUM_FUNC(CUGSuppressPrefCUG_t)
XDR_ENUM_FUNC(CUGSuppressOA_t)
XDR_ENUM_FUNC(CLIRMode_t)
XDR_ENUM_FUNC(PresentStatus_t)
XDR_ENUM_FUNC(DtmfTimer_t)
XDR_ENUM_FUNC(ProgressInd_t)
XDR_ENUM_FUNC(ECTCallState_t)
XDR_ENUM_FUNC(CallNotifySS_t)
XDR_ENUM_FUNC(CCallType_t)
XDR_ENUM_FUNC(CCallState_t)
XDR_ENUM_FUNC(Cause_t)
XDR_ENUM_FUNC(PresentationInd_t)
XDR_ENUM_FUNC(ScreenInd_t)
XDR_ENUM_FUNC(AoCStatus_t)
XDR_ENUM_FUNC(MNATDSMsgType_t)
XDR_ENUM_FUNC(VideoCallSpeed_t)
XDR_ENUM_FUNC(ECOrigReq_t)
XDR_ENUM_FUNC(ECOrigFB_t)
XDR_ENUM_FUNC(ECAnsFB_t)
XDR_ENUM_FUNC(CallConfigType_t)
XDR_ENUM_FUNC(ClientCmd_t)
XDR_ENUM_FUNC(DtmfState_t)
XDR_ENUM_FUNC(ECMode_t)
XDR_ENUM_FUNC(DCMode_t)
XDR_ENUM_FUNC(RepeatInd_t)
XDR_ENUM_FUNC(Signal_t)
XDR_ENUM_FUNC(CcApi_Element_t)
XDR_ENUM_FUNC(SS_UusSrvType_t)

bool_t 
xdr_BearerCapability_t(XDR *xdrs, BearerCapability_t *args)
{

	XDR_LOG(xdrs,"BearerCapability_t ")
	return ( xdr_opaque(xdrs, (caddr_t) args->val, 16) );
}
bool_t
xdr_CC_BearerCap_t(XDR *xdrs, CC_BearerCap_t *args)
{

	XDR_LOG(xdrs,"CC_BearerCap_t ")
	if( 
		xdr_BearerCapability_t(xdrs, &args->Bc1) && 
		xdr_BearerCapability_t(xdrs, &args->Bc2) && 
		XDR_ENUM(xdrs, &(args->RepeatInd), RepeatInd_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}




bool_t
xdr_PHONE_NUMBER_STR_t(XDR *xdrs, PHONE_NUMBER_STR_t *pnum)
{
	XDR_LOG(xdrs,"PHONE_NUMBER_STR_t ")

	if( xdr_opaque(xdrs, (caddr_t)pnum->phone_number, 40))
		return(TRUE);
	else
		return(FALSE);
}

bool_t     xdr_CUGInfo_t(XDR *xdrs, CUGInfo_t *info)
{
	XDR_LOG(xdrs,"CUGInfo_t ")

	if( 
		_xdr_u_int16_t(xdrs, &info->cug_index,"cug_index") &&
		XDR_ENUM(xdrs,  &(info->suppress_pref_cug), CUGSuppressPrefCUG_t) && 
		XDR_ENUM(xdrs,  &(info->suppress_oa), CUGSuppressOA_t)  
        )
		return(TRUE);
	else
		return(FALSE);
}
bool_t     xdr_VoiceCallParam_t(XDR *xdrs, VoiceCallParam_t *voiceCallParam)
{
	XDR_LOG(xdrs,"VoiceCallParam_t ")

	if( 
		XDR_ENUM(xdrs,  &(voiceCallParam->clir), CLIRMode_t) && 
 		xdr_CUGInfo_t(xdrs, &voiceCallParam->cug_info) &&
		_xdr_u_char(xdrs, &voiceCallParam->auxiliarySpeech,"auxiliarySpeech") &&
		_xdr_u_char(xdrs, &voiceCallParam->isEmergency,"isEmergency") &&
		_xdr_u_char(xdrs, &voiceCallParam->isFdnChkSkipped,"isFdnChkSkipped") &&
		xdr_opaque(xdrs, (caddr_t) &voiceCallParam->subAddr, sizeof(Subaddress_t)) &&
		xdr_CC_BearerCap_t(xdrs, &voiceCallParam->bearerCap)&&
        xdr_SS_UserInfo_t(xdrs, &voiceCallParam->uusInfo) &&
		_xdr_u_char(xdrs, &voiceCallParam->emergencySvcCat,"emergencySvcCat")
        )
		return(TRUE);
	else
		return(FALSE);
}




bool_t     xdr_CNAP_NAME_t(XDR *xdrs, CNAP_NAME_t *args)
{
	XDR_LOG(xdrs,"CNAP_NAME_t ")


	if( 
		XDR_ENUM(xdrs,  &(args->nameCoding), ALPHA_CODING_t) && 
		xdr_opaque(xdrs, (caddr_t) args->cnapName, 80) &&                   /* Need to Check */
		_xdr_u_char(xdrs, &args->nameLength,"nameLength") 
        )
		return(TRUE);
	else
		return(FALSE);

}
bool_t xdr_CallingName_t(XDR *xdrs, CallingName_t *args)
{
	XDR_LOG(xdrs,"CallingName_t ")


	if( 
		_xdr_u_char(xdrs, &args->data_cod_scheme,"data_cod_scheme") &&
		_xdr_u_char(xdrs, &args->length_in_char,"length_in_char") &&
		_xdr_u_char(xdrs, &args->name_size,"name_size") &&
		xdr_opaque(xdrs, (caddr_t) args->name, MAX_NAME_STRING_SIZE+1)
        )
		return(TRUE);
	else
		return(FALSE);

}

bool_t xdr_CallingInfo_t(XDR *xdrs, CallingInfo_t *args)
{
	XDR_LOG(xdrs,"CallingInfo_t ")

	
	if( 
		XDR_ENUM(xdrs,  &(args->present), PresentStatus_t) && 
		xdr_CallingName_t(xdrs, &args->name_info) 
        )
		return(TRUE);
	else
		return(FALSE);

}

bool_t xdr_ALL_CALL_STATE_t(XDR *xdrs, ALL_CALL_STATE_t *args)
{
	XDR_LOG(xdrs,"ALL_CALL_STATE_t ")

	
	if(
		xdr_vector(xdrs, (char *) args->stateList, MAX_CALLS_NO, sizeof(CCallState_t), XDR_ENUM_DEF(CCallState_t)) &&
		_xdr_u_char(xdrs, &args->listSz,"listSz")
        )
		return(TRUE);
	else
		return(FALSE);

}

bool_t xdr_ALL_CALL_INDEX_t(XDR *xdrs, ALL_CALL_INDEX_t *args)
{
	XDR_LOG(xdrs,"ALL_CALL_INDEX_t ")

	
	if( 
		xdr_opaque(xdrs, (caddr_t) args->indexList , MAX_CALLS_NO )  &&
		_xdr_u_char(xdrs, &args->listSz,"listSz")
        )
		return(TRUE);
	else
		return(FALSE);

}

bool_t xdr_ApiDtmf_t(XDR *xdrs, ApiDtmf_t *args)
{
	XDR_LOG(xdrs,"ApiDtmf_t ")

	
	if( 
		_xdr_u_char(xdrs, &args->callIndex,"callIndex") &&
		_xdr_u_int16_t(xdrs, &args->dtmfIndex,"dtmfIndex") &&
		_xdr_u_char(xdrs, &args->dtmfTone,"dtmfTone") &&
		_xdr_u_char(xdrs, &args->toneValume,"toneValume") &&
		_xdr_u_char(xdrs, &args->isSilent,"isSilent") &&

		xdr_u_long(xdrs, (UInt32 *) &args->duration)
        )
		return(TRUE);
	else
		return(FALSE);

}



bool_t
xdr_VideoCallParam_t(XDR *xdrs, VideoCallParam_t *args)
{
	XDR_LOG(xdrs,"VideoCallParam_t ")

	if( 
		XDR_ENUM(xdrs,  &(args->speed), VideoCallSpeed_t) && 
		XDR_ENUM(xdrs,  &(args->clir), CLIRMode_t) 
	  )
		return(TRUE);
	else
		return(FALSE);
}


bool_t 
xdr_RlpParam_t(XDR *xdrs, RlpParam_t *args)
{
	XDR_LOG(xdrs,"RlpParam_t ")

	if( 
		_xdr_u_char(xdrs, &args->ver,"ver") && 
		_xdr_u_int16_t(xdrs, &args->iws,"iws") && 
		_xdr_u_int16_t(xdrs, &args->mws,"mws") && 
		_xdr_u_char(xdrs, &args->t1,"t1") && 
		_xdr_u_char(xdrs, &args->n2,"n2") && 
		_xdr_u_char(xdrs, &args->t2,"t2") && 
		_xdr_u_char(xdrs, &args->t4,"t4") 
	  )
		return(TRUE);
	else
		return(FALSE);

}

bool_t 
xdr_RlpParamList_t(XDR *xdrs, RlpParamList_t *args)
{
	XDR_LOG(xdrs,"RlpParamList_t ")


	return xdr_vector(xdrs, (char *) args, 2, sizeof(RlpParam_t), xdr_RlpParam_t);
}

bool_t
xdr_CBST_t(XDR *xdrs, CBST_t *args)
{
	XDR_LOG(xdrs,"CBST_t ")

	if( 
		_xdr_u_char(xdrs, &args->speed,"speed") && 
		_xdr_u_char(xdrs, &args->name,"name") && 
		_xdr_u_char(xdrs, &args->ce,"ce") 
	  )
		return(TRUE);
	else
		return(FALSE);

}


bool_t
xdr_ECInfo_t(XDR *xdrs, ECInfo_t *args)
{
	XDR_LOG(xdrs,"ECInfo_t ")

	if( 
		XDR_ENUM(xdrs,  &(args->ES0), ECOrigReq_t)  && 
		XDR_ENUM(xdrs,  &(args->ES1), ECOrigFB_t)  && 
		XDR_ENUM(xdrs,  &(args->ES2), ECAnsFB_t) 
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t 
xdr_DCParam_t(XDR *xdrs, DCParam_t *args)
{
	XDR_LOG(xdrs,"DCParam_t ")

	if( 
		_xdr_u_int16_t(xdrs, &args->direction,"direction") && 
		_xdr_u_int16_t(xdrs, &args->max_dict,"max_dict") && 
		_xdr_u_int16_t(xdrs, &args->max_string,"max_string") 
	  )
		return(TRUE);
	else
		return(FALSE);

}

bool_t 
xdr_DCInfo_t(XDR *xdrs, DCInfo_t *args)
{
	XDR_LOG(xdrs,"DCInfo_t ")

	if( 
		_xdr_u_char(xdrs, &args->ds_req_success_neg,"ds_req_success_neg") && 
		xdr_DCParam_t(xdrs, &args->ds_req_datacomp)
	  )
		return(TRUE);
	else
		return(FALSE);

}

bool_t 
xdr_DataCallInfo_t(XDR *xdrs, DataCallInfo_t *args)
{
	XDR_LOG(xdrs,"DataCallInfo_t ")

	if( 
		_xdr_u_char(xdrs, &args->curr_ce,"curr_ce") && 
		XDR_ENUM(xdrs,  &(args->EC), ECMode_t)  && 
		XDR_ENUM(xdrs,  &(args->DC), DCMode_t) 
	  )
		return(TRUE);
	else
		return(FALSE);

}

bool_t 
xdr_SpeakerInfo_t(XDR *xdrs, SpeakerInfo_t *args)
{
	XDR_LOG(xdrs,"SpeakerInfo_t ")

	if( 
		_xdr_u_char(xdrs, &args->L,"L") && 
		_xdr_u_char(xdrs, &args->M,"M")
	  )
		return(TRUE);
	else
		return(FALSE);

}






// XDR serializer/deserializer for response msg data type

bool_t
xdr_VoiceCallConnectMsg_t(XDR *xdrs, VoiceCallConnectMsg_t *args)
{
	XDR_LOG(xdrs,"VoiceCallConnectMsg_t ")

	if( 
		_xdr_u_char(xdrs, &args->callIndex,"callIndex") && 
		XDR_ENUM(xdrs,  &(args->progress_desc), ProgressInd_t) &&
		_xdr_u_char(xdrs, &args->codecId, "codecId")
	  )
		return(TRUE);
	else
		return(FALSE);

}
bool_t
xdr_VoiceCallPreConnectMsg_t(XDR *xdrs, VoiceCallPreConnectMsg_t *args)
{
	XDR_LOG(xdrs,"VoiceCallPreConnectMsg_t ")

	if( 
		_xdr_u_char(xdrs, &args->callIndex,"callIndex") && 
		XDR_ENUM(xdrs,  &(args->progress_desc), ProgressInd_t)  &&
		_xdr_u_char(xdrs, &args->codecId, "codecId")
	  )
		return(TRUE);
	else
		return(FALSE);

}


bool_t
xdr_TelephoneNumber_t(XDR *xdrs, TelephoneNumber_t *args)
{
	XDR_LOG(xdrs,"TelephoneNumber_t ")

	if(
		XDR_ENUM(xdrs,  &(args->ton), gsm_TON_t) &&
		XDR_ENUM(xdrs,  &(args->npi), gsm_NPI_t) &&
		xdr_opaque(xdrs, (caddr_t) args->number , MAX_DIGITS+1 ) 
	  )
		return(TRUE);
	else
		return(FALSE);

}

bool_t
xdr_EctRdnInfo_t(XDR *xdrs, EctRdnInfo_t *args)
{
	XDR_LOG(xdrs,"EctRdnInfo_t ")

	if(
		XDR_ENUM(xdrs,  &(args->call_state), ECTCallState_t) &&
		_xdr_u_char(xdrs, &args->present_allowed_add,"present_allowed_add") && 
		_xdr_u_char(xdrs, &args->present_restricted,"present_restricted") && 
		_xdr_u_char(xdrs, &args->number_not_available,"number_not_available") && 
		_xdr_u_char(xdrs, &args->present_restricted_add,"present_restricted_add") && 
		xdr_TelephoneNumber_t(xdrs,  &args->phone_number)
	  )
		return(TRUE);
	else
		return(FALSE);

}


bool_t
xdr_SsNotifyParam_t(XDR *xdrs, SsNotifyParam_t *args)
{
	XDR_LOG(xdrs,"SsNotifyParam_t ")

	if(
		_xdr_u_int16_t(xdrs, &args->cug_index,"cug_index") && 
		xdr_EctRdnInfo_t(xdrs,  &args->ect_rdn_info) &&
		xdr_opaque(xdrs, (caddr_t) args->callingName , PHASE1_MAX_USSD_STRING_SIZE+1 ) 
	  )
		return(TRUE);
	else
		return(FALSE);

}


bool_t
xdr_SS_CallNotification_t(XDR *xdrs, SS_CallNotification_t *args)
{
	XDR_LOG(xdrs,"SS_CallNotification_t ")

	if(
		XDR_ENUM(xdrs,  &(args->index), CallIndex_t) && 
		XDR_ENUM(xdrs,  &(args->NotifySS_Oper), CallNotifySS_t) && 
		xdr_SsNotifyParam_t(xdrs,  &args->notify_param) &&
		xdr_opaque(xdrs, (caddr_t) args->fac_ie , MAX_FACILITY_IE_LENGTH ) 
		
	  )
		return(TRUE);
	else
		return(FALSE);

}
bool_t
xdr_SS_NameInd_t(XDR *xdrs, SS_NameInd_t *args)
{

	XDR_LOG(xdrs,"SS_NameInd_t ")

	if	(
			XDR_ENUM(xdrs, &args->type, SS_Presentation_t) &&
			_xdr_u_char(xdrs, &args->dcs, "dcs") &&
			_xdr_u_char(xdrs, &args->lengthInChar, "lengthInChar") &&
			_xdr_u_char(xdrs, &args->dataLenght, "dataLenght") &&
			xdr_opaque(xdrs, (caddr_t) args->data, MAX_NAME_SIZE ) 
		)
		return(TRUE);
	else
		return(FALSE);

}
bool_t
xdr_SS_EctIndInfo_t(XDR *xdrs, SS_EctIndInfo_t *args)
{
	XDR_LOG(xdrs,"SS_EctIndInfo_t ")	

	if	(
			XDR_ENUM(xdrs, &args->ectCallState, SS_EctCallState_t) &&
			XDR_ENUM(xdrs, &args->type, SS_Presentation_t) &&
			xdr_PartyAddress_t(xdrs, &args->partyAdd) && 
			xdr_PartySubAdd_t(xdrs, &args->partySubAdd) 
		)
		return(TRUE);
	else
		return(FALSE);

}
bool_t
xdr_SS_NotifySs_t(XDR *xdrs, SS_NotifySs_t *args)
{
	XDR_LOG(xdrs,"SS_NotifySs_t ")

	if	(
			_xdr_u_int16_t(xdrs, &args->include,"include") &&
			XDR_ENUM(xdrs, &(args->ssCode), SS_Code_t) &&
			_xdr_u_char(xdrs, &args->ssStatus,"ssStatus") &&
			_xdr_u_char(xdrs, &args->ssNotific,"ssNotific") &&
			XDR_ENUM(xdrs, &(args->callHold), SS_CallHold_t) &&
			_xdr_u_int16_t(xdrs, &args->cugIndex,"cug_index") && 
			xdr_SS_EctIndInfo_t(xdrs, &args->ectInd) &&
			xdr_SS_NameInd_t(xdrs, &args->nameInd) &&
			xdr_SS_CcbsFeature_t(xdrs, &args->ccbsFeature) &&
			_xdr_u_char(xdrs, &args->alertPatt,"alertPatt")  	
		)
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_CC_NotifySsInd_t(XDR *xdrs, CC_NotifySsInd_t *args)
{
	XDR_LOG(xdrs,"CC_NotifySsInd_t ")

	if( 
		xdr_SS_NotifySs_t(xdrs,  &args->notifySs) &&
		xdr_opaque(xdrs, (caddr_t) args->facIeBuf, 255 ) 
		
	  )
		return(TRUE);
	else
		return(FALSE);

}	
bool_t
xdr_CallStatusMsg_t(XDR *xdrs, CallStatusMsg_t *args)
{
	XDR_LOG(xdrs,"CallStatusMsg_t ")

	if(
		_xdr_u_char(xdrs, &args->callIndex,"callIndex") && 
		XDR_ENUM(xdrs,  &(args->callType), CCallType_t) && 
		XDR_ENUM(xdrs,  &(args->callstatus), CCallState_t) && 
		XDR_ENUM(xdrs,  &(args->progress_desc), ProgressInd_t) &&
		XDR_ENUM(xdrs,	&(args->cause), Cause_t) &&
        xdr_opaque(xdrs, (caddr_t) args->raw_cause_ie , MAX_CAUSE_IE_LENGTH) &&
		_xdr_u_char(xdrs, &args->codecId, "codecId") &&
		_xdr_u_char(xdrs, &args->causeNoCli, "causeNoCli")
	  )
		return(TRUE);
	else
		return(FALSE);

}

bool_t
xdr_VoiceCallActionMsg_t(XDR *xdrs, VoiceCallActionMsg_t *args)
{
	XDR_LOG(xdrs,"VoiceCallActionMsg_t ")

	if(
		_xdr_u_char(xdrs, &args->callIndex,"callIndex") && 
		XDR_ENUM(xdrs,  &(args->callResult), Result_t) && 
		XDR_ENUM(xdrs,  &(args->errorCause), NetworkCause_t)
	  )
		return(TRUE);
	else
		return(FALSE);

}

bool_t
xdr_VoiceCallReleaseMsg_t(XDR *xdrs, VoiceCallReleaseMsg_t *args)
{
	XDR_LOG(xdrs,"VoiceCallReleaseMsg_t ")

	if(
		_xdr_u_char(xdrs, &args->callIndex,"callIndex") && 
		XDR_ENUM(xdrs,  &(args->exitCause), Cause_t) && 
		xdr_u_long(xdrs, &args->callCCMUnit) &&
		xdr_u_long(xdrs, &args->callDuration)&&
		_xdr_u_char(xdrs, &args->codecId, "codecId")
	  )
		return(TRUE);
	else
		return(FALSE);

}
bool_t
xdr_CallingParty_t(XDR *xdrs, CallingParty_t *args)
{
	XDR_LOG(xdrs,"CallingParty_t ")

	if(
		_xdr_u_char(xdrs, &args->ton,"ton") && 
		_xdr_u_char(xdrs, &args->npi,"npi") && 
		XDR_ENUM(xdrs,  &(args->present), PresentationInd_t) && 
		XDR_ENUM(xdrs,  &(args->screen), ScreenInd_t) && 
		_xdr_u_char(xdrs, &args->c_num,"c_num") && 
		xdr_opaque(xdrs, (caddr_t) args->num , MAX_DIGITS+1 ) 
	  )
		return(TRUE);
	else
		return(FALSE);

}

bool_t
xdr_CallReceiveMsg_t(XDR *xdrs, CallReceiveMsg_t *args)
{
	XDR_LOG(xdrs,"CallReceiveMsg_t ")

	if(
		_xdr_u_char(xdrs, &args->callIndex,"callIndex") && 
		_xdr_u_int16_t(xdrs, &args->cug_index,"cug_index") && 
		xdr_CallingParty_t(xdrs,  &args->callingInfo)&& 
		_xdr_u_char(xdrs, &args->auxiliarySpeech,"auxiliarySpeech") &&
		XDR_ENUM(xdrs,  &(args->setup_ind_signal), Signal_t)   &&
		_xdr_u_char(xdrs, &args->codecId, "codecId")
	  )
		return(TRUE);
	else
		return(FALSE);

}
bool_t
xdr_VoiceCallWaitingMsg_t(XDR *xdrs, VoiceCallWaitingMsg_t *args)
{
	XDR_LOG(xdrs,"VoiceCallWaitingMsg_t ")

	if(
		_xdr_u_char(xdrs, &args->callIndex,"callIndex") && 
		_xdr_u_int16_t(xdrs, &args->cug_index,"cug_index") && 
		xdr_CallingParty_t(xdrs,  &args->callingInfo)&& 
		_xdr_u_char(xdrs, &args->auxiliarySpeech,"auxiliarySpeech")  &&
		XDR_ENUM(xdrs,  &(args->setup_ind_signal), Signal_t)  

	  )
		return(TRUE);
	else
		return(FALSE);

}
bool_t
xdr_CallAOCStatusMsg_t(XDR *xdrs, CallAOCStatusMsg_t *args)
{
	XDR_LOG(xdrs,"CallAOCStatusMsg_t ")

	if(
		_xdr_u_char(xdrs, &args->callIndex,"callIndex") && 
		XDR_ENUM(xdrs,  &(args->callAOCStatus), AoCStatus_t)  
	  )
		return(TRUE);
	else
		return(FALSE);

}
bool_t
xdr_CallCCMMsg_t(XDR *xdrs, CallCCMMsg_t *args)
{
	XDR_LOG(xdrs,"CallCCMMsg_t ")

	if(
		_xdr_u_char(xdrs, &args->callIndex,"callIndex") && 
		_xdr_u_char(xdrs, &args->callRelease,"callRelease") && 
		xdr_u_long(xdrs, &args->callDuration) && 
		xdr_u_long(xdrs, &args->callCCM)
	  )
		return(TRUE);
	else
		return(FALSE);

}



bool_t
xdr_DataCallReleaseMsg_t(XDR *xdrs, DataCallReleaseMsg_t *args)
{
	XDR_LOG(xdrs,"DataCallReleaseMsg_t ")

	if(
		_xdr_u_char(xdrs, &args->callIndex,"callIndex") && 
		xdr_u_long(xdrs, &args->callCCMUnit) && 
		xdr_u_long(xdrs, &args->callDuration) && 
		xdr_u_long(xdrs, &args->callTxBytes) && 
		xdr_u_long(xdrs, &args->callRxBytes) && 
		XDR_ENUM(xdrs,  &(args->exitCause), Cause_t) 
	  )
		return(TRUE);
	else
		return(FALSE);

}

bool_t
xdr_DataECDCLinkMsg_t(XDR *xdrs, DataECDCLinkMsg_t *args)
{
	XDR_LOG(xdrs,"DataECDCLinkMsg_t ")

	if(
		_xdr_u_char(xdrs, &args->callIndex,"callIndex") && 
		_xdr_u_char(xdrs, &args->ecdcResult,"ecdcResult") && 
		_xdr_u_char(xdrs, &args->ecMode,"ecMode") && 
		_xdr_u_char(xdrs, &args->dcMode,"dcMode") 
	  )
		return(TRUE);
	else
		return(FALSE);

}
bool_t
xdr_DataCallConnectMsg_t(XDR *xdrs, DataCallConnectMsg_t *args)
{
	XDR_LOG(xdrs,"DataCallConnectMsg_t ")

	if(
		_xdr_u_char(xdrs, &args->callIndex,"callIndex") &&
		xdr_CBST_t(xdrs, &args->cbst)
	  )
		return(TRUE);
	else
		return(FALSE);

}

bool_t
xdr_T_RLP_PAR(XDR *xdrs, RLP_PAR_t *args)
{
	XDR_LOG(xdrs,"RLP_PAR ")

	if(
		xdr_Int32(xdrs, &args->rlp_vers)  && 
		xdr_Int32(xdrs, &args->rlp_kim)  && 
		xdr_Int32(xdrs, &args->rlp_kmi)  && 
		xdr_Int32(xdrs, &args->rlp_t1)  && 
		xdr_Int32(xdrs, &args->rlp_n2)  && 
		xdr_Int32(xdrs, &args->rlp_t2)  
	  )
		return(TRUE);
	else
		return(FALSE);


}

bool_t
xdr_T_MN_CUG(XDR *xdrs, MN_CUG_t *args)
{
	XDR_LOG(xdrs,"MN_CUG_t ")

	if(
		xdr_Int32(xdrs, &args->select_cug)  && 
		xdr_Int32(xdrs, &args->index)  && 
		_xdr_u_char(xdrs, &args->suppress_oa,"suppress_oa")  
	  )
		return(TRUE);
	else
		return(FALSE);


}

bool_t
xdr_T_ETBM(XDR *xdrs, ETBM_t *args)
{
	XDR_LOG(xdrs,"T_ETBM ")

	if(
		xdr_Int32(xdrs, &args->pending_td)  && 
		xdr_Int32(xdrs, &args->pending_rd)  && 
		xdr_Int32(xdrs, &args->max_time)  
	  )
		return(TRUE);
	else
		return(FALSE);


}

bool_t
xdr_T_DC_PAR(XDR *xdrs, DC_PAR_t *args)
{
	XDR_LOG(xdrs,"T_DC_PAR ")

	if(
		xdr_Int32(xdrs, &args->direction)  && 
		xdr_UInt32(xdrs, &args->max_dict)  && 
		xdr_Int32(xdrs, &args->max_string)  
	  )
		return(TRUE);
	else
		return(FALSE);


}
bool_t
xdr_MNATDSParmSetupReq_t(XDR *xdrs, DS_MNATDSParmSetupReq_t *args)
{
	XDR_LOG(xdrs,"MNATDSParmSetupReq_t ")


	if(
		xdr_Int32(xdrs, &args->system_mode) &&
		xdr_Int32(xdrs, &args->data_rate) &&
		xdr_Int32(xdrs, &args->synchron_type) &&
		xdr_Int32(xdrs, &args->conn_element) &&
		xdr_Int32(xdrs, &args->service_mode) &&
    // 8/18/2004 From Comneon code (atc_setup.c), Instead of a list of possible RLP, it is now simplified:
    // if GEM, then version 2, if compression then version 1, otherwise, version 0.
		xdr_T_RLP_PAR(xdrs, &args->rlp_par) &&
		xdr_Int32(xdrs, &args->est_cause) &&
		xdr_opaque(xdrs, (char *) &args->calling_subaddr, sizeof(SUBADDR_t)) &&
		xdr_opaque(xdrs, (char *) &args->tel_number, sizeof(TEL_NUMBER_t)) &&
		xdr_opaque(xdrs, (char *) &args->called_subaddr, sizeof(SUBADDR_t)) &&
		xdr_Int32(xdrs, &args->mn_clir) &&
		xdr_T_MN_CUG(xdrs, &args->mn_cug) &&
		_xdr_u_char(xdrs, &args->autocall,"autocall")  && 
    // 8/18/2004 yuanliu - this is for AT+ETBM. If not supported, then all values in the struct should be set to 0.
		xdr_T_ETBM(xdrs, &args->etbm) &&
		xdr_T_DC_PAR(xdrs, &args->ds_datacomp) &&
		xdr_T_DC_PAR(xdrs, &args->ds_hw_datacomp) &&
		xdr_Int32(xdrs, &args->ds_datacomp_neg)  

	)
		return(TRUE);
	else
		return(FALSE);


}

bool_t
xdr_MNATDSParmReleaseReq_t(XDR *xdrs, DS_MNATDSParmReleaseReq_t *args)
{
	XDR_LOG(xdrs,"DS_MNATDSParmReleaseReq_t ")


	if(
		xdr_Int32(xdrs, &args->at_cause) &&
		xdr_Int32(xdrs, &args->ti_pd)  
	  )
		return(TRUE);
	else
		return(FALSE);


}

bool_t
xdr_MNATDSParmSetupResp_t(XDR *xdrs, DS_MNATDSParmSetupResp_t *args)
{
	XDR_LOG(xdrs,"DS_MNATDSParmSetupResp_t ")


	if(
		xdr_Int32(xdrs, &args->system_mode) &&
		xdr_T_RLP_PAR(xdrs, &args->rlp_par) &&
  		xdr_Int32(xdrs, &args->ti_pd)  &&
 		xdr_opaque(xdrs, (char *) &args->conn_subaddr, sizeof(SUBADDR_t)) &&
		xdr_T_ETBM(xdrs, &args->etbm) &&
		xdr_T_DC_PAR(xdrs, &args->ds_datacomp) &&
		xdr_T_DC_PAR(xdrs, &args->ds_hw_datacomp) &&
		xdr_Int32(xdrs, &args->ds_datacomp_neg)  
	)
		return(TRUE);
	else
		return(FALSE);

}

bool_t
xdr_MNATDSParmSetupInd_t(XDR *xdrs, DS_MNATDSParmSetupInd_t *args)
{
	XDR_LOG(xdrs,"DS_MNATDSParmSetupInd_t ")


	if(
		xdr_Int32(xdrs, &args->data_rate) &&
		xdr_Int32(xdrs, &args->service_mode) &&
		xdr_Int32(xdrs, &args->conn_element) &&
		xdr_Int32(xdrs, &args->ti_pd)  &&
		xdr_opaque(xdrs, (char *) &args->tel_number, sizeof(TEL_NUMBER_t)) &&
		xdr_opaque(xdrs, (char *) &args->calling_subaddr, sizeof(SUBADDR_t)) &&
		xdr_opaque(xdrs, (char *) &args->called_subaddr, sizeof(SUBADDR_t)) &&
		_xdr_u_char(xdrs, &args->ds_datacomp_ind,"ds_datacomp_ind")  
	  )
		return(TRUE);
	else
		return(FALSE);


}

bool_t
xdr_MNATDSParmConnectInd_t(XDR *xdrs, DS_MNATDSParmConnectInd_t *args)
{
	XDR_LOG(xdrs,"DS_MNATDSParmConnectInd_t ")


	if(
		xdr_Int32(xdrs, &args->ti_pd)  &&
		xdr_T_DC_PAR(xdrs, &args->ds_neg_datacomp)
	  )
		return(TRUE);
	else
		return(FALSE);


}

bool_t
xdr_MNATDSParmServiceInd_t(XDR *xdrs, DS_MNATDSParmServiceInd_t *args)
{
	XDR_LOG(xdrs,"DS_MNATDSParmServiceInd_t ")


	if(
		xdr_Int32(xdrs, &args->ti_pd)  &&
		xdr_Int32(xdrs, &args->service_mode) 
	  )
		return(TRUE);
	else
		return(FALSE);


}

bool_t
xdr_MNATDSParmSetupCnf_t(XDR *xdrs, DS_MNATDSParmSetupCnf_t *args)
{
	XDR_LOG(xdrs,"DS_MNATDSParmSetupCnf_t ")


	if(
		xdr_Int32(xdrs, &args->ti_pd)  &&
		xdr_opaque(xdrs, (char *) &args->tel_number, sizeof(TEL_NUMBER_t)) &&
		xdr_opaque(xdrs, (char *) &args->conn_subaddr, sizeof(SUBADDR_t)) &&
		xdr_T_DC_PAR(xdrs, &args->ds_neg_datacomp)
	  )
		return(TRUE);
	else
		return(FALSE);


}

bool_t
xdr_MNATDSParmSetupRej_t(XDR *xdrs, DS_MNATDSParmSetupRej_t *args)
{
	XDR_LOG(xdrs,"DS_MNATDSParmSetupRej_t ")


	return	xdr_Int32(xdrs, &args->mn_cause);  
}


bool_t
xdr_MNATDSParmReleaseInd_t(XDR *xdrs, DS_MNATDSParmReleaseInd_t *args)
{
	XDR_LOG(xdrs,"DS_MNATDSParmReleaseInd_t ")


	if(
		xdr_Int32(xdrs, &args->ti_pd)  &&
		xdr_Int32(xdrs, &args->mn_cause) 
	  )
		return(TRUE);
	else
		return(FALSE);


}

bool_t
xdr_MNATDSParmReleaseCnf_t(XDR *xdrs, DS_MNATDSParmReleaseCnf_t *args)
{
	XDR_LOG(xdrs,"DS_MNATDSParmReleaseCnf_t ")


	return	xdr_Int32(xdrs, &args->ti_pd);  
}

bool_t
xdr_MNATDSParmStatusInd_t(XDR *xdrs, DS_MNATDSParmStatusInd_t *args)
{
	XDR_LOG(xdrs,"DS_MNATDSParmStatusInd_t ")


	if(
		xdr_Int32(xdrs, &args->linestate1)  &&
		xdr_Int32(xdrs, &args->linestate2)  &&
		xdr_Int32(xdrs, &args->linestate3) 
	  )
		return(TRUE);
	else
		return(FALSE);


}

bool_t
xdr_MNATDSParmFET_t(XDR *xdrs, DS_MNATDSParmFET_t *args)
{
	XDR_LOG(xdrs,"MNATDSParmFET_t ")


	return	_xdr_u_char(xdrs, &args->ppm,"ppm");  
}

static const struct xdr_discrim MNATDSMsg_t_dscrm[15] = {
	// MN messages

		{ (int)MNATDSMSG_MN_SETUP_REQ, _T("MNATDSMSG_MN_SETUP_REQ"), (xdrproc_t)xdr_MNATDSParmSetupReq_t ,0,0,0,0},
		{ (int)MNATDSMSG_MN_RELEASE_REQ, _T("MNATDSMSG_MN_RELEASE_REQ"), (xdrproc_t)xdr_MNATDSParmReleaseReq_t ,0,0,0,0},
		{ (int)MNATDSMSG_MN_SETUP_RESP, _T("MNATDSMSG_MN_SETUP_RESP"), (xdrproc_t)xdr_MNATDSParmSetupResp_t ,0,0,0,0},
	// ATDS messages
		
		{ (int)MNATDSMSG_ATDS_SETUP_IND, _T("MNATDSMSG_ATDS_SETUP_IND"), (xdrproc_t)xdr_MNATDSParmSetupInd_t,0,0,0,0 },
		{ (int)MNATDSMSG_ATDS_CONNECT_IND, _T("MNATDSMSG_ATDS_CONNECT_IND"), (xdrproc_t)xdr_MNATDSParmConnectInd_t ,0,0,0,0},
		{ (int)MNATDSMSG_ATDS_SERVICE_IND, _T("MNATDSMSG_ATDS_SERVICE_IND"), (xdrproc_t)xdr_MNATDSParmServiceInd_t ,0,0,0,0},
		{ (int)MNATDSMSG_ATDS_SETUP_CNF, _T("MNATDSMSG_ATDS_SETUP_CNF"), (xdrproc_t)xdr_MNATDSParmSetupCnf_t ,0,0,0,0},
		{ (int)MNATDSMSG_ATDS_SETUP_REJ, _T("MNATDSMSG_ATDS_SETUP_REJ"), (xdrproc_t)xdr_MNATDSParmSetupRej_t ,0,0,0,0},
		{ (int)MNATDSMSG_ATDS_RELEASE_IND, _T("MNATDSMSG_ATDS_RELEASE_IND"), (xdrproc_t)xdr_MNATDSParmReleaseInd_t ,0,0,0,0},
		{ (int)MNATDSMSG_ATDS_RELEASE_CNF, _T("MNATDSMSG_ATDS_RELEASE_CNF"), (xdrproc_t)xdr_MNATDSParmReleaseCnf_t ,0,0,0,0},
		{ (int)MNATDSMSG_ATDS_STATUS_IND, _T("MNATDSMSG_ATDS_STATUS_IND"), (xdrproc_t)xdr_MNATDSParmStatusInd_t ,0,0,0,0},
	// fax
		{ (int)MNATDSMSG_ATDS_FET_IND, _T("MNATDSMSG_ATDS_FET_IND"), (xdrproc_t)xdr_MNATDSParmFET_t ,0,0,0,0},
		
					/* Add other modules message to serialize/deserialize routine map */
	{ __dontcare__, _T(""), NULL_xdrproc_t ,0,0,0,0} };

		

bool_t
xdr_MNATDSMsg_t(XDR *xdrs, DS_MNATDSMsg_t *args)
{


	struct xdr_discrim *entry = NULL;
	enum_t dscm = args->type;
	XDR_LOG(xdrs,"DS_MNATDSMsg_t ")

	if(
		XDR_ENUM(xdrs,  &(args->type), MNATDSMsgType_t) && 
		xdr_union(xdrs, (enum_t *)&dscm, (caddr_t)(void *)&(args->parm), 
							MNATDSMsg_t_dscrm, NULL_xdrproc_t, &entry, NULL)
	  )
		return(TRUE);
	else
		return(FALSE);


}

bool_t
xdr_DataCallStatusMsg_t(XDR *xdrs, DataCallStatusMsg_t *args)
{
	XDR_LOG(xdrs,"DataCallStatusMsg_t ")


	if(
		_xdr_u_char(xdrs, &args->callIndex,"callIndex")  && 
		xdr_MNATDSMsg_t(xdrs, &args->mnatds_msg)  
	  )
		return(TRUE);
	else
		return(FALSE);


}

bool_t
xdr_ApiClientCmd_CcParam_t(XDR *xdrs, ApiClientCmd_CcParam_t *args)
{
	XDR_LOG(xdrs,"ApiClientCmd_CcParam_t ")

	if( 
		_xdr_u_char(xdrs, &args->callIndex,"callIndex") && 
		XDR_ENUM(xdrs, &(args->callType), CCallType_t) && 
		XDR_ENUM(xdrs, &(args->result), Result_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_ApiClientCmdInd_t(XDR *xdrs, ApiClientCmdInd_t *args)
{
	XDR_LOG(xdrs,"ApiClientCmdInd_t ")

	if( 
		XDR_ENUM(xdrs, &(args->clientCmdType), ClientCmd_t) && 
		xdr_pointer(xdrs, (char **)(void*) &args->paramPtr, sizeof(ApiClientCmd_CcParam_t), (xdrproc_t) xdr_ApiClientCmd_CcParam_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_ApiDtmfStatus_t(XDR *xdrs, ApiDtmfStatus_t *args)
{
	XDR_LOG(xdrs,"ApiDtmfStatus_t ")

	if( 
		xdr_ApiDtmf_t(xdrs, &args->dtmfObj) && 
		XDR_ENUM(xdrs, &(args->dtmfState), DtmfState_t) && 
		XDR_ENUM(xdrs, &(args->cause), Result_t)
	  )
		return(TRUE);
	else
		return(FALSE);
}



bool_t xdr_CcCipherInd_t(XDR* xdrs, CcCipherInd_t *rsp)
{
	XDR_LOG(xdrs,"CcCipherInd_t ")

	if(	_xdr_Boolean(xdrs, &rsp->status, "status") &&
		_xdr_u_char(xdrs, &rsp->rat, "rat")
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_CCallStateList_t(XDR *xdrs, CCallStateList_t *args)
{
	return xdr_vector(xdrs, (char *) args, MAX_CALLS_NO, sizeof(CCallState_t), XDR_ENUM_DEF(CCallState_t));
}

bool_t xdr_CCallIndexList_t(XDR *xdrs, CCallIndexList_t *args)
{
	return xdr_opaque(xdrs, (caddr_t) args , MAX_CALLS_NO );
}

bool_t
xdr_SS_UserToUser_t(XDR *xdrs, SS_UserToUser_t *args)
{
	XDR_LOG(xdrs,"xdr_SS_UserToUser_t ")

	if( 
		_xdr_u_char(xdrs, &args->uusProtDisc,"uusProtDisc") &&
		_xdr_u_char(xdrs, &args->length,"length") &&
        xdr_opaque(xdrs, (caddr_t) args->uusInfo, MAX_USER_TO_USER_SIZE)
        
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_SS_UserUserSrv_t(XDR *xdrs, SS_UserUserSrv_t *args)
{
	XDR_LOG(xdrs,"SS_UserUserSrv_t ")

	if( 
		XDR_ENUM(xdrs,  &(args->type), SS_UusSrvType_t) &&
		xdr_Boolean(xdrs, &args->required)
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_SS_UserInfo_t(XDR *xdrs, SS_UserInfo_t *args)
{
	XDR_LOG(xdrs,"SS_UserInfo_t ")

	if( 
		_xdr_u_char(xdrs, &args->include,"include") &&
        xdr_SS_UserToUser_t(xdrs, &args->uus) && 
        xdr_Boolean(xdrs, &args->moreDate) &&
        _xdr_u_char(xdrs, &args->uuSrvCount,"uuSrvCount") &&
        xdr_vector(xdrs, (char *) args, 3, sizeof(SS_UserUserSrv_t), xdr_SS_UserUserSrv_t) &&
        XDR_ENUM(xdrs,  &(args->errorCode), SS_ErrorCode_t) && 
		XDR_ENUM(xdrs,  &(args->cause), Cause_t) && 
        _xdr_u_char(xdrs, &args->callIndex,"callIndex")
	  )
		return(TRUE);
	else
		return(FALSE);
}
