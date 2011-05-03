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
*   @file   capi2_sms_msg.c
*
*   @brief  This file implements the serialize/deserialize the request/respose
*	for phonebook.
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


XDR_ENUM_FUNC(SmsStorage_t)
XDR_ENUM_FUNC(SmsAlphabet_t)
XDR_ENUM_FUNC(SmsMsgClass_t)
XDR_ENUM_FUNC(SmsSubmitRspType_t)
XDR_ENUM_FUNC(MS_T_SMS_PP_CAUSE_t)
XDR_ENUM_FUNC(SmsMti_t)
XDR_ENUM_FUNC(HomezoneCityzoneStatus_t)
XDR_ENUM_FUNC(smsCodingGrp_t)
XDR_ENUM_FUNC(SmsWaitInd_t)
XDR_ENUM_FUNC(NewMsgDisplayPref_t)
XDR_ENUM_FUNC(SmsCBActionType_t)
XDR_ENUM_FUNC(SmsCBStopType_t)
XDR_ENUM_FUNC(SMS_BEARER_PREFERENCE_t)
XDR_ENUM_FUNC(MEAccess_t)
XDR_ENUM_FUNC(SmsTransactionType_t)
XDR_ENUM_FUNC(SmsStorageWaitState_t)
XDR_ENUM_FUNC(SmsMesgStatus_t)
XDR_ENUM_FUNC(SmsAckNetworkType_t)
XDR_ENUM_FUNC(SmsNLSTypes_t)
XDR_ENUM_FUNC(SmsNLSLanguage_t)
XDR_ENUM_FUNC(SmsPushType_t)


static const struct xdr_discrim SmsSimMsg_t_dscrm[4] = {
		{ (int)SMS_DELIVER,"", (xdrproc_t)xdr_SmsRxParam_t ,0,0,0,0},
		{ (int)SMS_SUBMIT,"", (xdrproc_t)xdr_SmsTxParam_t,0,0,0,0 },
		{ (int)SMS_STATUS_REPORT,"", (xdrproc_t)xdr_SmsSrParam_t ,0,0,0,0},
					/* Add other modules message to serialize/deserialize routine map */
	{ __dontcare__, "", NULL_xdrproc_t ,0,0,0,0} };

bool_t 
xdr_SmsAddress_t(XDR* xdrs, SmsAddress_t* smsAddr)
{
	XDR_LOG(xdrs,"SmsAddress_t")

	if( _xdr_u_char(xdrs, &smsAddr->TypeOfAddress,"TypeOfAddress") &&
		xdr_opaque(xdrs, (caddr_t)&smsAddr->Number, (SMS_MAX_DIGITS + 1))
		)
		return(TRUE);
	else
		return(FALSE);
}




bool_t
xdr_Sms_411Addr_t(XDR* xdrs, Sms_411Addr_t* addr)
{
	XDR_LOG(xdrs,"Sms_411Addr_t")

		if( _xdr_u_char(xdrs, &addr->Len,"Len") &&
			_xdr_u_char(xdrs, &addr->Toa,"Toa") &&
			xdr_opaque(xdrs, (caddr_t)&addr->Val, (SMS_MAX_DIGITS / 2))
		)
		return(TRUE);
	else
		return(FALSE);
}



bool_t
xdr_SmsCodingType_t(XDR* xdrs, SmsCodingType_t* codingType)
{
	XDR_LOG(xdrs,"SmsCodingType_t")

	if( _xdr_u_char(xdrs, &codingType->codingGroup,"codingGroup") &&
		XDR_ENUM(xdrs, &codingType->alphabet, SmsAlphabet_t) &&
		XDR_ENUM(xdrs, &codingType->msgClass, SmsMsgClass_t)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_SmsRelative_t(XDR* xdrs, SmsRelative_t* smsRelative)
{
	XDR_LOG(xdrs,"SmsRelative_t")

	if( _xdr_u_char(xdrs, &smsRelative->time,"time")
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SmsAbsolute_t(XDR* xdrs, SmsAbsolute_t* smsAbsolute)
{
	XDR_LOG(xdrs,"SmsAbsolute_t")

	if( _xdr_u_char(xdrs, &smsAbsolute->years,"years") &&
		_xdr_u_char(xdrs, &smsAbsolute->months,"months") &&
		_xdr_u_char(xdrs, &smsAbsolute->days,"days") &&
		_xdr_u_char(xdrs, &smsAbsolute->hours,"hours") &&
		_xdr_u_char(xdrs, &smsAbsolute->minutes,"minutes") &&
		_xdr_u_char(xdrs, &smsAbsolute->seconds,"seconds") &&
		_xdr_u_char(xdrs, &smsAbsolute->time_zone,"time_zone")
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_SmsTime_t(XDR* xdrs, SmsTime_t* smsTime)
{
	XDR_LOG(xdrs,"SmsTime_t")

	if( _xdr_u_char(xdrs, &smsTime->isRelativeTime,"isRelativeTime") &&
		xdr_SmsRelative_t(xdrs, &smsTime->relTime) &&
		xdr_SmsAbsolute_t(xdrs, &smsTime->absTime)
		)
		return(TRUE);
	else
		return(FALSE);
}


bool_t
xdr_SmsTxParam_t(XDR* xdrs, SmsTxParam_t* txParam)
{
	XDR_LOG(xdrs,"SmsTxParam_t")

	if( xdr_SmsCodingType_t(xdrs, &txParam->codingType) &&
		xdr_SmsTime_t(xdrs, &txParam->validatePeriod) &&
		_xdr_u_char(xdrs, &txParam->procId,"procId") &&
		_xdr_u_char(xdrs, &txParam->msgRefNum,"msgRefNum") &&
		_xdr_u_char(xdrs, &txParam->rejDupl,"rejDupl") &&
		_xdr_u_char(xdrs, &txParam->statusRptRequest,"statusRptRequest") &&
		_xdr_u_char(xdrs, &txParam->replyPath,"replyPath") &&
		_xdr_u_char(xdrs, &txParam->isCompression,"isCompression") &&
		_xdr_u_char(xdrs, &txParam->userDataHeaderInd,"userDataHeaderInd")
		)
		return(TRUE);
	else
		return(FALSE);
}



bool_t 
xdr_SmsSubmitRspMsg_t(XDR* xdrs, SmsSubmitRspMsg_t* smsSubmitRsp)
{
	XDR_LOG(xdrs,"SmsSubmitRspMsg_t")

	if( XDR_ENUM(xdrs, &smsSubmitRsp->submitRspType, SmsSubmitRspType_t) &&
		XDR_ENUM(xdrs, &smsSubmitRsp->InternalErrCause, Result_t) &&
		XDR_ENUM(xdrs, &smsSubmitRsp->NetworkErrCause, MS_T_SMS_PP_CAUSE_t) &&
		_xdr_u_char(xdrs, &smsSubmitRsp->tpMr,"tpMr") &&
		XDR_ENUM(xdrs, &smsSubmitRsp->NetworkErrCauseRp, MS_T_SMS_PP_CAUSE_t) &&
		XDR_ENUM(xdrs, &smsSubmitRsp->NetworkErrCauseTp, MS_T_SMS_PP_CAUSE_t) &&
		_xdr_u_char(xdrs, &smsSubmitRsp->tpdu_len,"tpdu_len") &&
		xdr_vector(xdrs,  (char*)smsSubmitRsp->tpdu, smsSubmitRsp->tpdu_len, sizeof(unsigned char), (xdrproc_t)xdr_u_char)
		)
		return(TRUE);
	else
		return(FALSE);
}




bool_t 
xdr_SmsTxTextModeParms_t(XDR* xdrs, SmsTxTextModeParms_t* smsTxTxtParams)
{
	XDR_LOG(xdrs,"SmsTxTextModeParms_t")

	if( _xdr_u_char(xdrs, &smsTxTxtParams->fo,"fo") &&
		_xdr_u_char(xdrs, &smsTxTxtParams->vp,"vp") &&
		_xdr_u_char(xdrs, &smsTxTxtParams->pid,"pid") &&
		_xdr_u_char(xdrs, &smsTxTxtParams->dcs,"dcs")
		)
		return(TRUE);
	else
		return(FALSE);
}



bool_t 
xdr_SmsMsgDeleteResult_t( XDR* xdrs, SmsMsgDeleteResult_t* smsMsgDelResult )
{
	XDR_LOG(xdrs,"SmsMsgDeleteResult_t")

	if( XDR_ENUM(xdrs,  &smsMsgDelResult->result, SIMAccess_t) &&
		_xdr_u_int16_t(xdrs, &smsMsgDelResult->rec_no,"rec_no") &&
		XDR_ENUM(xdrs,  &smsMsgDelResult->storage, SmsStorage_t)
		)
		return(TRUE);
	else
		return(FALSE);
}



bool_t 
xdr_SmsDcs_t(XDR* xdrs, SmsDcs_t* smsdcs)
{
	XDR_LOG(xdrs,"SmsDcs_t")

	if( _xdr_u_char(xdrs, &smsdcs->DcsRaw,"DcsRaw") &&
		XDR_ENUM(xdrs,  &smsdcs->CodingGrp, smsCodingGrp_t) &&
		_xdr_u_char(xdrs, &smsdcs->Compression,"Compression") &&
		XDR_ENUM(xdrs, &smsdcs->MsgClass, SmsMsgClass_t) &&
		_xdr_u_char(xdrs, &smsdcs->IndActive,"IndActive") &&
		XDR_ENUM(xdrs, &smsdcs->IndType, SmsWaitInd_t) &&
		XDR_ENUM(xdrs, &smsdcs->MsgAlphabet, SmsAlphabet_t) &&
		_xdr_u_char(xdrs, &smsdcs->vmMsgCount,"vmMsgCount")
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SmsRxParam_t(XDR* xdrs, SmsRxParam_t* smsrxparam)
{
	XDR_LOG(xdrs,"SmsRxParam_t")

	if( 	xdr_SmsAddress_t(xdrs, &smsrxparam->ServCenterAddress) &&
		xdr_SmsDcs_t(xdrs, &smsrxparam->codingScheme) &&
		xdr_SmsTime_t(xdrs, &smsrxparam->srvCenterTime) &&
		_xdr_u_char(xdrs, &smsrxparam->moreMsgFlag,"moreMsgFlag") &&
		_xdr_u_char(xdrs, &smsrxparam->replyPath,"replyPath") &&
		_xdr_u_char(xdrs, &smsrxparam->usrDataHeaderInd,"usrDataHeaderInd") &&
		_xdr_u_char(xdrs, &smsrxparam->statusReportInd,"statusReportInd") &&
		_xdr_u_char(xdrs, &smsrxparam->procId,"procId") &&
		_xdr_u_char(xdrs, &smsrxparam->fo,"fo")
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SmsSrParam_t(XDR* xdrs, SmsSrParam_t* smssrparam)
{
	XDR_LOG(xdrs,"SmsSrParam_t")

	if( _xdr_u_char(xdrs, &smssrparam->msgRefNum,"msgRefNum") &&
		xdr_SmsTime_t(xdrs, &smssrparam->srvCenterTime) &&
		xdr_SmsTime_t(xdrs, &smssrparam->discardTime) &&
		_xdr_u_char(xdrs, &smssrparam->status,"status") &&
		_xdr_u_char(xdrs, &smssrparam->paramInd,"paramInd") &&
		_xdr_u_char(xdrs, &smssrparam->procId,"procId") &&
		_xdr_u_char(xdrs, &smssrparam->fo,"fo") &&
		xdr_SmsDcs_t(xdrs, &smssrparam->codingScheme) &&
		_xdr_u_char(xdrs, &smssrparam->isUserDataPresent,"isUserDataPresent")
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SmsNLS_t(XDR* xdrs, SmsNLS_t* val)
{
        XDR_LOG(xdrs,"SmsNLS_t")

        if( XDR_ENUM(xdrs, &val->nls_type, SmsNLSTypes_t)  &&
                XDR_ENUM(xdrs, &val->nls_lang, SmsNLSLanguage_t)
                )
                return(TRUE);
        else
                return(FALSE);

}

bool_t 
xdr_SmsSimMsg_t(XDR* xdrs, SmsSimMsg_t* smssimmsg)
{

	struct xdr_discrim *entry = NULL;
	enum_t dscm = smssimmsg->msgTypeInd;
	XDR_LOG(xdrs,"SmsSimMsg_t")
	if( XDR_ENUM(xdrs,  &smssimmsg->msgTypeInd, SmsMti_t) &&
		_xdr_u_int16_t(xdrs, &smssimmsg->rec_no,"rec_no") &&
		XDR_ENUM(xdrs,  &smssimmsg->result, SIMAccess_t) &&
		XDR_ENUM(xdrs, &smssimmsg->status, SIMSMSMesgStatus_t) &&
		xdr_SmsAddress_t(xdrs, &smssimmsg->daoaAddress) &&
		_xdr_u_int16_t(xdrs, &smssimmsg->udhLen,"udhLen") &&
		_xdr_u_int16_t(xdrs, &smssimmsg->textLen,"textLen") &&
		xdr_opaque(xdrs, (caddr_t)&smssimmsg->text_data, SMS_DATA_LENGTH_7BIT_APHABET) &&
		xdr_opaque(xdrs, (caddr_t)&smssimmsg->PDU, SMSMESG_DATA_SZ) &&
		_xdr_u_int16_t(xdrs, &smssimmsg->pduSize,"pduSize") &&
		xdr_SmsNLS_t(xdrs, &smssimmsg->nls) &&
		xdr_union(xdrs, (enum_t *)&dscm, (caddr_t)(void *)&(smssimmsg->msg), 
							SmsSimMsg_t_dscrm, NULL_xdrproc_t, &entry, NULL)
		)
	{
		bool_t ret = FALSE;
		u_int offset = smssimmsg->udhData - smssimmsg->PDU;
		
		//smssimmsg->udhData is direct ofsfet of smssimmsg->PDU and hence just ser-deser offset and adjust the pointer during deser.
		ret = xdr_long(xdrs,(void*)&offset);
		
		if(xdrs->x_op == XDR_DECODE)
		{
			smssimmsg->udhData = smssimmsg->PDU + offset;
		}

		return ret;
	}
	return(FALSE);
}



bool_t
xdr_SmsCBMsgRspType_t(XDR* xdrs, SmsCBMsgRspType_t* smscbmsgrsptype)
{
	XDR_LOG(xdrs,"SmsCBMsgRspType_t")

	if( XDR_ENUM(xdrs,  &smscbmsgrsptype->actType, SmsCBActionType_t) &&
		XDR_ENUM(xdrs,  &smscbmsgrsptype->stopType, SmsCBStopType_t)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_MS_T_MN_CB_MSG_ID_RANGE(XDR* xdrs, MS_T_MN_CB_MSG_ID_RANGE* cbmsgidrange)
{
	XDR_LOG(xdrs,"MS_T_MN_CB_MSG_ID_RANGE")

	if( _xdr_u_int16_t(xdrs,  &cbmsgidrange->start_pos,"start_pos") &&
		_xdr_u_int16_t(xdrs,  &cbmsgidrange->stop_pos,"stop_pos")
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SMS_CB_MSG_ID_RANGE_LIST_t(XDR* xdrs, SMS_CB_MSG_ID_RANGE_LIST_t* cbmsgidrangelist)
{
	XDR_LOG(xdrs,"SMS_CB_MSG_ID_RANGE_LIST_t")

	return( xdr_vector(xdrs,  (char*)cbmsgidrangelist->A, MAX_MSG_ID_RANGE_LIST_SIZE, sizeof(MS_T_MN_CB_MSG_ID_RANGE), (xdrproc_t)xdr_MS_T_MN_CB_MSG_ID_RANGE) );
}

bool_t 
xdr_SMS_CB_MSG_IDS_t(XDR* xdrs, SMS_CB_MSG_IDS_t* cbmsgids)
{
	XDR_LOG(xdrs,"SMS_CB_MSG_IDS_t")

	if( _xdr_u_char(xdrs,  &cbmsgids->nbr_of_msg_id_ranges, "num_of_msgid") &&

		xdr_SMS_CB_MSG_ID_RANGE_LIST_t(xdrs,  &cbmsgids->msg_id_range_list)
		)
		return(TRUE);
	else
		return(FALSE);
}



bool_t 
xdr_MS_T_MN_CB_LANGUAGE_LIST(XDR* xdrs, MS_T_MN_CB_LANGUAGE_LIST* cblanglist)
{
	XDR_LOG(xdrs,"MS_T_MN_CB_LANGUAGE_LIST")

	return( xdr_vector(xdrs,  (char*)cblanglist->A, 13, sizeof(unsigned char), (xdrproc_t)xdr_u_char) );
}

bool_t 
xdr_MS_T_MN_CB_LANGUAGES(XDR* xdrs, MS_T_MN_CB_LANGUAGES* cblangs)
{
	XDR_LOG(xdrs,"MS_T_MN_CB_LANGUAGES")

	if( 
#if !defined(X_LONG_INT)
		xdr_int(xdrs,  &cblangs->nbr_of_languages) &&
#else
		xdr_long(xdrs,  &cblangs->nbr_of_languages) &&
#endif
		xdr_MS_T_MN_CB_LANGUAGE_LIST(xdrs,  &cblangs->language_list)
		)
		return(TRUE);
	else
		return(FALSE);
}



bool_t 
xdr_SmsVoicemailInd_t(XDR* xdrs, SmsVoicemailInd_t* smsvmind)
{
	XDR_LOG(xdrs,"SmsVoicemailInd_t")

	if( _xdr_u_char(xdrs, &smsvmind->staL1,"staL1") &&
		_xdr_u_char(xdrs, &smsvmind->staL2,"staL2") &&
		_xdr_u_char(xdrs, &smsvmind->staFax,"staFax") &&
		_xdr_u_char(xdrs, &smsvmind->staData,"staData") &&
		_xdr_u_char(xdrs, &smsvmind->msgCount,"msgCount") &&
		XDR_ENUM(xdrs, &smsvmind->msgType, SmsWaitInd_t)
		)
		return(TRUE);
	else
		return(FALSE);
}




bool_t 
xdr_smsModuleReady_t(XDR* xdrs, smsModuleReady_t* rsp)
{
	XDR_LOG(xdrs,"smsModuleReady_t")

	if( XDR_ENUM(xdrs,  &rsp->simSmsStatus, Result_t) &&
		XDR_ENUM(xdrs, &rsp->meSmsStatus, Result_t)
		)
		return(TRUE);
	else
		return(FALSE);
}



bool_t 
xdr_SmsIncMsgStoredResult_t(XDR* xdrs, SmsIncMsgStoredResult_t* rsp)
{
	XDR_LOG(xdrs,"SmsIncMsgStoredResult_t")

	if( XDR_ENUM(xdrs,  &rsp->result, SIMAccess_t) &&
		XDR_ENUM(xdrs,  &rsp->storage, SmsStorage_t) &&
		_xdr_u_int16_t(xdrs, &rsp->rec_no,"rec_no") &&
		XDR_ENUM(xdrs, &rsp->waitState, SmsStorageWaitState_t)
		)
		return(TRUE);
	else
		return(FALSE);
}


bool_t 
xdr_SmsStoredSmsCb_t(XDR* xdrs, SmsStoredSmsCb_t* rsp)
{
	XDR_LOG(xdrs,"SmsStoredSmsCb_t")

	if( _xdr_u_int16_t(xdrs, &rsp->MsgId,"MsgId") &&
		_xdr_u_char(xdrs, &rsp->Dcs,"Dcs") &&
		_xdr_u_char(xdrs, &rsp->NbPages,"NbPages") &&
		_xdr_u_char(xdrs, &rsp->NoPage,"NoPage") &&
		_xdr_u_int16_t(xdrs, &rsp->Serial,"Serial") &&
		xdr_vector(xdrs,  (char*)rsp->Msg, CB_DATA_PER_PAGE_SZ, sizeof(unsigned char), (xdrproc_t)xdr_u_char)
		)
		return(TRUE);
	else
		return(FALSE); 
}

bool_t 
xdr_HomezoneIndData_t(XDR* xdrs, HomezoneIndData_t* rsp)
{
	XDR_LOG(xdrs,"HomezoneIndData_t") 

	if( XDR_ENUM(xdrs,  &rsp->status, HomezoneCityzoneStatus_t) &&
		_xdr_u_char(xdrs, &rsp->tag_len, "Len") &&
		xdr_vector(xdrs, (char*) rsp->tag_data, SIM_HZ_TAGS_LEN, sizeof(unsigned char), (xdrproc_t)xdr_u_char)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_Sms_340Addr_t(XDR* xdrs, Sms_340Addr_t* addr)
{
	XDR_LOG(xdrs,"Sms_340Addr_t")

	if( _xdr_u_char(xdrs, &addr->NbDigit,"NbDigit") &&
		_xdr_u_char(xdrs, &addr->Toa,"Toa") &&
		xdr_opaque(xdrs, (caddr_t)&addr->Val, (SMS_MAX_DIGITS / 2))
	)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SmsEnhancedVMInd_t(XDR* xdrs, SmsEnhancedVMInd_t* rsp)
{
	XDR_LOG(xdrs,"SmsEnhancedVMInd_t")

	if( _xdr_u_char(xdrs, &rsp->isDataValid,"isDataValid") &&
		_xdr_u_char(xdrs, &rsp->isNotifType,"isNotifType") &&
		_xdr_u_char(xdrs, &rsp->multiSubProf,"multiSubProf") &&
		_xdr_u_char(xdrs, &rsp->isToBeStored,"isToBeStored") &&
		_xdr_u_char(xdrs, &rsp->vmBoxAlmostFull,"vmBoxAlmostFull") &&
		_xdr_u_char(xdrs, &rsp->vmBoxFull,"vmBoxFull") &&
		_xdr_u_char(xdrs, &rsp->vmStaExtInd,"vmStaExtInd") &&
		xdr_Sms_340Addr_t(xdrs, &rsp->accessAddr) &&
		_xdr_u_char(xdrs, &rsp->msgCount,"msgCount") &&
		_xdr_u_char(xdrs, &rsp->notifCount,"notifCount") &&
		_xdr_u_char(xdrs, &rsp->notifCount,"notifCount") &&
		_xdr_u_char(xdrs, &rsp->notifCount,"notifCount") &&
		_xdr_u_int16_t(xdrs, &rsp->msgID,"msgID") &&
		_xdr_u_char(xdrs, &rsp->vmMsgLength,"vmMsgLength") &&
		_xdr_u_char(xdrs, &rsp->vmMsgRetentionDays,"vmMsgRetentionDays") &&
		_xdr_u_char(xdrs, &rsp->isUrgent,"isUrgent") &&
		_xdr_u_char(xdrs, &rsp->vmMsgExtInd,"vmMsgExtInd") &&
		xdr_Sms_340Addr_t(xdrs, &rsp->cliAddr)
		)
	{
		u_int len = (u_int)rsp->staExtLength;
		if(xdr_bytes(xdrs, (char**)(void*)&rsp->staExtData, &len, 255))
		{
			u_int len = (u_int)rsp->vmMsgExtLength;
			return xdr_bytes(xdrs, (char**)(void*)&rsp->vmMsgExtData, &len, 255);
		}
	}
	return(FALSE);
}




bool_t xdr_SmsCommand_t(XDR* xdrs, SmsCommand_t* data)
{
 XDR_LOG(xdrs,"xdr_SmsCommand_t")
 
 if( _xdr_u_char(xdrs, &data->fo,"fo") &&
  _xdr_u_char(xdrs, &data->ct,"ct") &&
  _xdr_u_char(xdrs, &data->pid,"pid") &&
  _xdr_u_char(xdrs, &data->mn,"mn") )
  return(TRUE);
 else
  return(FALSE);
}


bool_t
xdr_SmsAppSpecificData_t(XDR* xdrs, SmsAppSpecificData_t* req)
{

	XDR_LOG(xdrs,"SmsAppSpecificData_t")

	if( xdr_SmsSimMsg_t(xdrs,  &req->fSmsMsgData) &&
		XDR_ENUM(xdrs,  &req->fSmsType, SmsPushType_t)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_CAPI2_HomezoneIndData_t(XDR* xdrs, CAPI2_HomezoneIndData_t* rsp)
{
	XDR_LOG(xdrs,"CAPI2_HomezoneIndData_t")

	if( xdr_pointer(xdrs, (char **)(void*) &rsp->rsp, sizeof(HomezoneIndData_t), (xdrproc_t)xdr_HomezoneIndData_t)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_CAPI2_SmsSimMsg_t(XDR* xdrs, CAPI2_SmsSimMsg_t* rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsSimMsg_t")

	return( xdr_pointer(xdrs, (char **)(void*) &rsp->rsp, sizeof(SmsSimMsg_t), (xdrproc_t)xdr_SmsSimMsg_t) );
}

bool_t 
xdr_CAPI2_SmsStoredSmsCb_t(XDR* xdrs, CAPI2_SmsStoredSmsCb_t* rsp)
{
	XDR_LOG(xdrs,"CAPI2_SmsStoredSmsCb_t")

	if( xdr_pointer(xdrs, (char **)(void*) &rsp->rsp, sizeof(SmsStoredSmsCb_t), (xdrproc_t)xdr_SmsStoredSmsCb_t)
		)
		return(TRUE);
	else
		return(FALSE);
}


bool_t 
xdr_CAPI2_smsModuleReady_t(XDR* xdrs, CAPI2_smsModuleReady_t* rsp)
{
	XDR_LOG(xdrs,"CAPI2_smsModuleReady_t")

	if( xdr_pointer(xdrs, (char **)(void*) &rsp->rsp, sizeof(smsModuleReady_t), (xdrproc_t)xdr_smsModuleReady_t)
		)
		return(TRUE);
	else
		return(FALSE);
}

