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
*   @file   capi2_stk_msg.c
*
*   @brief  This file implements the serialize/deserialize the request/respose
*	for sim tool kit.
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

XDR_ENUM_FUNC(Unicode_t)
XDR_ENUM_FUNC(SATK_EVENTS_t)
XDR_ENUM_FUNC(SATK_ResultCode_t)
XDR_ENUM_FUNC(SATK_ResultCode2_t)
XDR_ENUM_FUNC(SATKInKeyType_t)
XDR_ENUM_FUNC(SATKToneType_t)
XDR_ENUM_FUNC(SATKSSType_t)
XDR_ENUM_FUNC(SATKCallType_t)
XDR_ENUM_FUNC(STKRefreshType_t)
XDR_ENUM_FUNC(SIM_LAUNCH_BROWSER_ACTION_t)
XDR_ENUM_FUNC(SATKDataServiceType_t)
XDR_ENUM_FUNC(StkCallSetupFailResult_t)
XDR_ENUM_FUNC(StkCallControl_t)
XDR_ENUM_FUNC(StkCallControlResult_t)
XDR_ENUM_FUNC(STK_DATA_SERV_BEARER_TYPE_t)
XDR_ENUM_FUNC(SATK_CHAN_SOCKET_t)
XDR_ENUM_FUNC(SATK_CHAN_STATE_t)
XDR_ENUM_FUNC(STKMenuSelectionResCode_t)
XDR_ENUM_FUNC(StkCardType_t)
XDR_ENUM_FUNC(StkStatus_t)	
XDR_ENUM_FUNC(SATKDataCloseType_t)

bool_t xdr_REFRESH_FILE_LIST_t(XDR* xdrs, REFRESH_FILE_LIST_t* rfrshfilelist);

bool_t 
xdr_STKIconListId_t(XDR* xdrs, STKIconListId_t* stkiconlistid)
{
	XDR_LOG(xdrs,"STKIconListId_t")

	if( xdr_vector(xdrs,  (char*)stkiconlistid->Id, STK_MAX_LIST_SIZE, sizeof(unsigned char), (xdrproc_t)xdr_u_char) &&
		_xdr_u_char(xdrs,  &stkiconlistid->IsSelfExplanatory,"IsSelfExplanatory") &&
		_xdr_u_char(xdrs,  &stkiconlistid->IsExist,"IsExist")
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SATKIcon_t(XDR* xdrs, SATKIcon_t* satkicon)
{
	XDR_LOG(xdrs,"SATKIcon_t")

	if( _xdr_u_char(xdrs,  &satkicon->Id,"Id") &&
		_xdr_u_char(xdrs,  &satkicon->IsSelfExplanatory,"IsSelfExplanatory") &&
		_xdr_u_char(xdrs,  &satkicon->IsExist,"IsExist")
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_SATKString_t(XDR* xdrs, SATKString_t* satkstring)
{
	XDR_LOG(xdrs,"SATKString_t")


	if( XDR_ENUM(xdrs,  &satkstring->unicode_type, Unicode_t) &&
		_xdr_u_int16_t(xdrs, &satkstring->len,"len")
		)
	{
		u_int len = (u_int)satkstring->len;
		return xdr_bytes(xdrs, (char**)(void*)&satkstring->string, &len, 512); 
	}

	return(FALSE);
}

bool_t
xdr_SATKAlphaID_t(XDR* xdrs, SATKAlphaID_t* satkalphaid)
{
	XDR_LOG(xdrs,"SATKAlphaID_t")

	if( XDR_ENUM(xdrs,  &satkalphaid->unicode_type, Unicode_t) &&
		_xdr_u_int16_t(xdrs, &satkalphaid->len,"len") &&
        xdr_opaque(xdrs, (caddr_t)&satkalphaid->string, STK_MAX_ALPHA_TEXT_LEN+1)
		)
	{
		return(TRUE);
	}
	return(FALSE);
}

bool_t 
xdr_SetupMenu_t(XDR* xdrs, SetupMenu_t* setupmenu)
{
	XDR_LOG(xdrs,"SetupMenu_t")

	if( xdr_SATKString_t(xdrs,  &setupmenu->title) &&
		xdr_SATKIcon_t(xdrs, &setupmenu->titleIcon) &&
		xdr_opaque(xdrs, (caddr_t)&setupmenu->pItemIdList, MAX_ITEM) &&
		xdr_STKIconListId_t(xdrs, &setupmenu->pIconList) &&
		xdr_STKIconListId_t(xdrs, &setupmenu->pNextActIndList) &&
		_xdr_u_char(xdrs,  &setupmenu->listSize,"listSize") &&
		_xdr_u_char(xdrs,  &setupmenu->isHelpAvailable,"isHelpAvailable")
		
		)
	{
		u_int len = (u_int)setupmenu->listSize;

		return( xdr_array(xdrs,
		 (char **)(void*) &setupmenu->pItemList,
		 &len,
		 ~0,
		 sizeof(SATKString_t),
		 (xdrproc_t) xdr_SATKString_t) );
	}
	return(FALSE);
}






bool_t 
xdr_StkCmdRespond_t(XDR* xdrs, StkCmdRespond_t* stkcmdrespond)
{
    u_int len = 0;        

	XDR_LOG(xdrs,"StkCmdRespond_t")        

    if(stkcmdrespond->event == SATK_EVENT_OPEN_CHAN)
    {
        len = (u_int)stkcmdrespond->bearerDataLen;
    }
    else if((stkcmdrespond->event == SATK_EVENT_RECEIVE_DATA) || (stkcmdrespond->event == SATK_EVENT_GET_CHAN_STATUS))
    {
        len = (u_int)stkcmdrespond->dataLen;
    }    

	if( _xdr_u_int16_t(xdrs, &stkcmdrespond->clientID,"clientID") &&
		XDR_ENUM(xdrs,  &stkcmdrespond->event, SATK_EVENTS_t) &&
        _xdr_u_char(xdrs,  &stkcmdrespond->channelID,"channelID") &&
		XDR_ENUM(xdrs,  &stkcmdrespond->result1, SATK_ResultCode_t) &&
		_xdr_u_int16_t(xdrs,  &stkcmdrespond->result2,"result2") &&
        _xdr_u_int16_t(xdrs,  &stkcmdrespond->chanStatus,"chanStatus") &&
		xdr_vector(xdrs,  (char*)stkcmdrespond->textStr1, SATK_LOGIN_PASSWD_LEN+1, sizeof(unsigned char), (xdrproc_t)xdr_u_char) &&
		xdr_vector(xdrs,  (char*)stkcmdrespond->textStr2, SATK_LOGIN_PASSWD_LEN+1, sizeof(unsigned char), (xdrproc_t)xdr_u_char) &&
		xdr_vector(xdrs,  (char*)stkcmdrespond->textStr3, SATK_MAX_APN_LEN+1, sizeof(unsigned char), (xdrproc_t)xdr_u_char) &&
        _xdr_u_int16_t(xdrs, &stkcmdrespond->dataLen,"dataLen") &&
        xdr_bytes(xdrs, (char **)(void*)&stkcmdrespond->dataBuf, (u_int *)&len, 4096) &&
        _xdr_u_int16_t(xdrs,  &stkcmdrespond->bearerDataLen,"bearerDataLen") &&
        _xdr_u_char(xdrs,  &stkcmdrespond->bearerType,"bearerType") &&
        _xdr_u_int16_t(xdrs,  &stkcmdrespond->numBytesAvail,"numBytesAvail")
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_StkDataServReq_t(XDR* xdrs, StkDataServReq_t* stkDataServReq)
{
	XDR_LOG(xdrs,"StkDataServReq_t")        

	if( XDR_ENUM(xdrs,  &stkDataServReq->event, SATK_EVENTS_t) &&
        _xdr_u_char(xdrs,  &stkDataServReq->channelID,"channelID") &&
        XDR_ENUM(xdrs,  &stkDataServReq->chanSocket, SATK_CHAN_SOCKET_t) && 
		XDR_ENUM(xdrs,  &stkDataServReq->chanState, SATK_CHAN_STATE_t) &&
		_xdr_u_int16_t(xdrs,  &stkDataServReq->numBytesAvail,"numBytesAvail")
		)
		return(TRUE);
	else
		return(FALSE);
}


bool_t 
xdr_DisplayText_t(XDR* xdrs, DisplayText_t* displaytxt)
{
	XDR_LOG(xdrs,"DisplayText_t")

	if( xdr_SATKString_t(xdrs,  &displaytxt->stkStr) &&
		_xdr_u_char(xdrs,  &displaytxt->isHighPrio,"isHighPrio") &&
		xdr_SATKIcon_t(xdrs,  &displaytxt->icon) &&
		_xdr_u_char(xdrs,  &displaytxt->isDelay,"isDelay") &&
		_xdr_u_char(xdrs,  &displaytxt->isSustained,"isSustained") &&
        _xdr_u_long(xdrs, &displaytxt->duration, "duration")
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_GetInkey_t(XDR* xdrs, GetInkey_t* getinkey)
{
	XDR_LOG(xdrs,"GetInkey_t")

	if( xdr_SATKString_t(xdrs,  &getinkey->stkStr) &&
		xdr_SATKIcon_t(xdrs,  &getinkey->icon) &&
		XDR_ENUM(xdrs,  &getinkey->inKeyType, SATKInKeyType_t) &&
		_xdr_u_char(xdrs,  &getinkey->isHelpAvailable,"isHelpAvailable") &&
        _xdr_u_long(xdrs, &getinkey->duration, "duration")
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_GetInput_t(XDR* xdrs, GetInput_t* getinput)
{
	XDR_LOG(xdrs,"GetInput_t")

	if( _xdr_u_char(xdrs,  &getinput->minLen,"minLen") &&
		_xdr_u_char(xdrs,  &getinput->maxLen,"maxLen") &&
		xdr_SATKString_t(xdrs,  &getinput->stkStr) &&
		xdr_SATKString_t(xdrs,  &getinput->defaultSATKStr) &&
		xdr_SATKIcon_t(xdrs,  &getinput->icon) &&
		XDR_ENUM(xdrs,  &getinput->inPutType, SATKInKeyType_t) &&
		_xdr_u_char(xdrs,  &getinput->isHelpAvailable,"isHelpAvailable") &&
		_xdr_u_char(xdrs,  &getinput->isEcho,"isEcho") &&
		_xdr_u_char(xdrs,  &getinput->isPacked,"isPacked")
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_PlayTone_t(XDR* xdrs, PlayTone_t* playtone)
{
	XDR_LOG(xdrs,"PlayTone_t")

	if( xdr_SATKString_t(xdrs,  &playtone->stkStr) &&
        xdr_Boolean(xdrs, &playtone->defaultStr) &&
		XDR_ENUM(xdrs,  &playtone->toneType, SATKToneType_t) &&
		xdr_u_long(xdrs,  &playtone->duration) &&
		xdr_SATKIcon_t(xdrs,  &playtone->icon)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SelectItem_t(XDR* xdrs, SelectItem_t* selectitem)
{
	XDR_LOG(xdrs,"SelectItem_t")

	if( xdr_SATKString_t(xdrs,  &selectitem->title) &&
		xdr_SATKIcon_t(xdrs, &selectitem->titleIcon) &&
		_xdr_u_char(xdrs,  &selectitem->isAlphaIdProvided,"isAlphaIdProvided") &&
		xdr_opaque(xdrs, (caddr_t)&selectitem->pItemIdList, MAX_ITEM) &&
		xdr_STKIconListId_t(xdrs, &selectitem->pIconList) &&
		xdr_STKIconListId_t(xdrs, &selectitem->pNextActIndList) &&
		_xdr_u_char(xdrs,  &selectitem->listSize,"listSize") &&
		_xdr_u_char(xdrs,  &selectitem->isHelpAvailable,"isHelpAvailable") &&
		_xdr_u_char(xdrs,  &selectitem->defaultItem,"defaultItem")
		)
	{
		u_int len = (u_int)selectitem->listSize;

		return( xdr_array(xdrs,
		 (char **)(void*) &selectitem->pItemList,
		 &len,
		 ~0,
		 sizeof(SATKString_t),
		 (xdrproc_t) xdr_SATKString_t) );
	}
	return(FALSE);
}

bool_t 
xdr_SATKNum_t(XDR* xdrs, SATKNum_t* satknum)
{
	XDR_LOG(xdrs,"SATKNum_t")

	if( xdr_opaque(xdrs, (caddr_t)&satknum->Num, PHASE2_MAX_USSD_STRING_SIZE +1) &&
		XDR_ENUM(xdrs,  &satknum->Ton, gsm_TON_t) &&
		XDR_ENUM(xdrs,  &satknum->Npi, gsm_NPI_t) &&
		_xdr_u_char(xdrs,  &satknum->dcs,"dcs") &&
		_xdr_u_char(xdrs,  &satknum->len,"len")
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SendSs_t(XDR* xdrs, SendSs_t* sendss)
{
	XDR_LOG(xdrs,"SendSs_t")

	if( XDR_ENUM(xdrs,  &sendss->ssType, SATKSSType_t) &&
		xdr_SATKNum_t(xdrs,  &sendss->num) &&
		_xdr_u_char(xdrs,  &sendss->isAlphaIdProvided,"isAlphaIdProvided") &&
		xdr_SATKString_t(xdrs,  &sendss->text) &&
		xdr_SATKIcon_t(xdrs,  &sendss->icon)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SendUssd_t(XDR* xdrs, SendUssd_t* sendussd)
{
	XDR_LOG(xdrs,"SendUssd_t")

	if( XDR_ENUM(xdrs,  &sendussd->ssType, SATKSSType_t) &&
		xdr_SATKNum_t(xdrs,  &sendussd->num) &&
		_xdr_u_char(xdrs,  &sendussd->isAlphaIdProvided,"isAlphaIdProvided") &&
		xdr_SATKString_t(xdrs,  &sendussd->text) &&
		xdr_SATKIcon_t(xdrs,  &sendussd->icon)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SetupCall_t(XDR* xdrs, SetupCall_t* setupcall)
{
	XDR_LOG(xdrs,"SetupCall_t")

	if( _xdr_u_char(xdrs,  &setupcall->isEmerCall,"isEmerCall") &&
		XDR_ENUM(xdrs,  &setupcall->callType, SATKCallType_t) &&
		_xdr_u_char(xdrs,  &setupcall->IsSetupAlphaIdProvided,"IsSetupAlphaIdProvided") &&
		_xdr_u_char(xdrs,  &setupcall->IsConfirmAlphaIdProvided,"IsConfirmAlphaIdProvided") &&
		xdr_SATKNum_t(xdrs,  &setupcall->num) &&
		xdr_SATKString_t(xdrs,  &setupcall->confirmPhaseStr) &&
		xdr_SATKString_t(xdrs,  &setupcall->setupPhaseStr) &&
		xdr_SATKIcon_t(xdrs,  &setupcall->confirmPhaseIcon) &&
		xdr_SATKIcon_t(xdrs,  &setupcall->setupPhaseIcon) &&
		xdr_u_long(xdrs,  &setupcall->duration) &&
       _xdr_u_char(xdrs, &setupcall->subAddr.tos, "tos") &&
       _xdr_u_char(xdrs, &setupcall->subAddr.odd_even, "odd_even") &&
       _xdr_u_char(xdrs, &setupcall->subAddr.c_subaddr, "c_subaddr") &&
       xdr_opaque(xdrs, (caddr_t)&setupcall->subAddr.subaddr, SUB_LENGTH) &&
       xdr_CC_BearerCap_t(xdrs, &setupcall->bc)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_REFRESH_FILE_t(XDR* xdrs, REFRESH_FILE_t* rfrshfile)
{
	XDR_LOG(xdrs,"REFRESH_FILE_t")

	if( _xdr_u_char(xdrs,  &rfrshfile->path_len,"path_len") &&
		xdr_vector(xdrs,  (char *)(void *)rfrshfile->file_path, MAX_SIM_FILE_PATH_LEN, sizeof(UInt16), (xdrproc_t)xdr_u_int16_t)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_REFRESH_FILE_LIST_t(XDR* xdrs, REFRESH_FILE_LIST_t* rfrshfilelist)
{
	XDR_LOG(xdrs,"REFRESH_FILE_LIST_t")

	if( _xdr_u_char(xdrs,  &rfrshfilelist->number_of_file,"number_of_file") &&
		xdr_vector(xdrs,  (char *)(void *)rfrshfilelist->changed_file, MAX_NUM_OF_FILE_IN_CHANGE_LIST, sizeof(REFRESH_FILE_t), (xdrproc_t)xdr_REFRESH_FILE_t)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_Refresh_t(XDR* xdrs, Refresh_t* refresh)
{
	XDR_LOG(xdrs,"Refresh_t")

	if( XDR_ENUM(xdrs,  &refresh->refreshType, STKRefreshType_t) &&
		xdr_REFRESH_FILE_LIST_t(xdrs,  &refresh->FileIdList) &&
		XDR_ENUM(xdrs, &refresh->appliType, USIM_APPLICATION_TYPE) 
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_STK_SMS_DATA_t(XDR* xdrs, STK_SMS_DATA_t* stksmsdata)
{
	XDR_LOG(xdrs,"STK_SMS_DATA_t")

	if( _xdr_u_char(xdrs,  &stksmsdata->sca_ton_npi,"sca_ton_npi") &&
		_xdr_u_char(xdrs,  &stksmsdata->sca_len,"sca_len") &&
		xdr_vector(xdrs,  (char*)stksmsdata->sca_data, 20, sizeof(UInt8), (xdrproc_t)xdr_u_char) &&
		_xdr_u_char(xdrs,  &stksmsdata->pdu_len,"pdu_len") &&
		xdr_vector(xdrs,  (char*)stksmsdata->pdu_data, 176, sizeof(UInt8), (xdrproc_t)xdr_u_char)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SendMOSMS_t(XDR* xdrs, SendMOSMS_t* sendmosms)
{
	XDR_LOG(xdrs,"SendMOSMS_t")

	if( _xdr_u_char(xdrs,  &sendmosms->isAlphaIdProvided,"isAlphaIdProvided") &&
		xdr_SATKString_t(xdrs,  &sendmosms->text) &&
		xdr_SATKIcon_t(xdrs,  &sendmosms->icon) &&
		xdr_STK_SMS_DATA_t(xdrs,  &sendmosms->sms_data)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_SendStkDtmf_t(XDR* xdrs, SendStkDtmf_t* sendstkdtmf)
{
	XDR_LOG(xdrs,"SendStkDtmf_t")

	if( _xdr_u_char(xdrs,  &sendstkdtmf->isAlphaIdProvided,"isAlphaIdProvided") &&
		xdr_vector(xdrs,  (char*)sendstkdtmf->dtmf, STK_MAX_DTMF_LEN, sizeof(UInt8), (xdrproc_t)xdr_u_char) &&
		xdr_SATKString_t(xdrs,  &sendstkdtmf->alphaString) &&
		xdr_SATKIcon_t(xdrs,  &sendstkdtmf->dtmfIcon)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_ProvisionFile_t(XDR* xdrs, ProvisionFile_t* provsfile)
{
	XDR_LOG(xdrs,"ProvisionFile_t")

	if( _xdr_u_char(xdrs,  &provsfile->length,"length") &&
		xdr_vector(xdrs,  (char*)provsfile->prov_file_data, MAX_SIM_PROV_FILE_LENGTH, sizeof(unsigned char), (xdrproc_t)xdr_u_char)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_LaunchBrowserReq_t(XDR* xdrs, LaunchBrowserReq_t* launchbrowserreq)
{
	XDR_LOG(xdrs,"LaunchBrowserReq_t")

	if( XDR_ENUM(xdrs,  &launchbrowserreq->browser_action, SIM_LAUNCH_BROWSER_ACTION_t) &&
		_xdr_u_char(xdrs,  &launchbrowserreq->browser_id_exist,"browser_id_exist") &&
		_xdr_u_char(xdrs,  &launchbrowserreq->browser_id,"browser_id") &&
		xdr_vector(xdrs,  (char*)launchbrowserreq->url, MAX_SIM_URL_LENGTH, sizeof(char), (xdrproc_t)xdr_char) &&
		_xdr_u_char(xdrs,  &launchbrowserreq->bearer_length,"bearer_length") &&
		xdr_vector(xdrs,  (char*)launchbrowserreq->bearer, MAX_SIM_BEARER_LENGTH, sizeof(unsigned char), (xdrproc_t)xdr_u_char) &&
		_xdr_u_char(xdrs,  &launchbrowserreq->prov_length,"prov_length") &&
		xdr_vector(xdrs,  (char *)(void *)launchbrowserreq->prov_file, MAX_SIM_PROV_FILE_NUM, sizeof(ProvisionFile_t), (xdrproc_t)xdr_ProvisionFile_t) &&
		xdr_SATKString_t(xdrs,  &launchbrowserreq->text) &&
		xdr_SATKString_t(xdrs,  &launchbrowserreq->alpha_id) &&
		xdr_SATKIcon_t(xdrs,  &launchbrowserreq->icon_id) &&
        _xdr_u_char(xdrs,  &launchbrowserreq->default_alpha_id,"default_alpha_id")
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_IdleModeText_t(XDR* xdrs, IdleModeText_t* idlemodetext)
{
	XDR_LOG(xdrs,"IdleModeText_t")

	if( xdr_SATKString_t(xdrs,  &idlemodetext->stkStr) &&
		xdr_SATKIcon_t(xdrs,  &idlemodetext->icon)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_StkOpenChanParam_t(XDR* xdrs, SATK_OPEN_CHAN_PARAM_t* stkOpenChanParam)
{
	XDR_LOG(xdrs,"SATK_OPEN_CHAN_PARAM_t")

	if( xdr_Boolean(xdrs,  &stkOpenChanParam->immediate_establish) && 
        xdr_Boolean(xdrs,  &stkOpenChanParam->auto_reconnect) &&
        xdr_Boolean(xdrs,  &stkOpenChanParam->transparent_mode) &&
		XDR_ENUM(xdrs,  &stkOpenChanParam->bearer_type, STK_DATA_SERV_BEARER_TYPE_t) &&
        _xdr_u_char(xdrs,  &stkOpenChanParam->bearer_data_len,"bearer_data_len") &&
		xdr_opaque(xdrs, (caddr_t)&stkOpenChanParam->bearer_data, sizeof(stkOpenChanParam->bearer_data)) &&
		xdr_SATKString_t(xdrs,  &stkOpenChanParam->user_login) &&
		xdr_SATKString_t(xdrs,  &stkOpenChanParam->login_pwd) &&
        xdr_Boolean(xdrs,  &stkOpenChanParam->static_ip_addr_exist) && 
        xdr_opaque(xdrs, (caddr_t)&stkOpenChanParam->static_ip_addr, STK_IPV4_ADDRESS_LEN) &&
        xdr_Boolean(xdrs,  &stkOpenChanParam->dest_ip_addr_exist) && 
        xdr_opaque(xdrs, (caddr_t)&stkOpenChanParam->dest_ip_addr, STK_IPV4_ADDRESS_LEN) &&
       _xdr_u_int16_t(xdrs, &stkOpenChanParam->buffer_size,"buffer_size") &&
		_xdr_u_char(xdrs,  &stkOpenChanParam->transport_layer,"transport_layer") &&
        _xdr_u_int16_t(xdrs, &stkOpenChanParam->port_num,"port_num") &&
        xdr_opaque(xdrs, (caddr_t)&stkOpenChanParam->apn_name, STK_MAX_APN_LEN) &&
        xdr_opaque(xdrs, (caddr_t)&stkOpenChanParam->pdp_type, sizeof(stkOpenChanParam->pdp_type)) &&
        xdr_Boolean(xdrs,  &stkOpenChanParam->dataCompression) &&
        xdr_Boolean(xdrs,  &stkOpenChanParam->headerCompression) &&
		_xdr_u_char(xdrs,  &stkOpenChanParam->qos_precedence,"qos_precedence") &&
		_xdr_u_char(xdrs,  &stkOpenChanParam->qos_delay,"qos_delay") &&
        _xdr_u_char(xdrs,  &stkOpenChanParam->qos_reliability,"qos_reliability") &&
		_xdr_u_char(xdrs,  &stkOpenChanParam->qos_peak,"qos_peak") &&
        _xdr_u_char(xdrs,  &stkOpenChanParam->qos_mean,"qos_mean")
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_StkOpenChanSvrParam_t(XDR* xdrs, SATK_OPEN_CHAN_SVR_PARAM_t* stkOpenChanSvrParam)
{
	XDR_LOG(xdrs,"SATK_OPEN_CHAN_SVR_PARAM_t")

    if( _xdr_u_int16_t(xdrs, &stkOpenChanSvrParam->buffer_size,"buffer_size") &&
        _xdr_u_int16_t(xdrs, &stkOpenChanSvrParam->port_num,"port_num")        
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_StkSendDataParam_t(XDR* xdrs, SATK_SEND_DATA_PARAM_t* stkSendDataParam)
{
    u_int len = (u_int)stkSendDataParam->dataLen;

	XDR_LOG(xdrs,"SATK_SEND_DATA_PARAM_t")

	if( xdr_bytes(xdrs, (char **)(void*)&stkSendDataParam->dataBuf, &len, 4096) &&
        _xdr_u_int16_t(xdrs, &stkSendDataParam->dataLen,"dataLen") &&
        xdr_Boolean(xdrs,  &stkSendDataParam->send_data_immediately) 
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_StkDataService_t(XDR* xdrs, StkDataService_t* stkds)
{
	XDR_LOG(xdrs,"StkDataService_t")

	if( _xdr_u_char(xdrs,  &stkds->chanID,"chanID") &&
		XDR_ENUM(xdrs,  &stkds->actionType, SATKDataServiceType_t) &&
		xdr_SATKString_t(xdrs,  &stkds->text) &&
		xdr_SATKIcon_t(xdrs,  &stkds->icon) &&
		_xdr_u_char(xdrs,  &stkds->isLoginNeeded,"isLoginNeeded") &&
		_xdr_u_char(xdrs,  &stkds->isPasswdNeeded,"isPasswdNeeded") &&
		_xdr_u_char(xdrs,  &stkds->isApnNeeded,"isApnNeeded") &&
        _xdr_u_char(xdrs,  &stkds->dataLen,"dataLen") &&
		_xdr_Boolean(xdrs,  &stkds->is_UICC_server_mode,"is_UICC_server_mode") &&
        XDR_ENUM(xdrs,  &stkds->server_close_type, SATKDataCloseType_t) &&
        _xdr_Boolean(xdrs,  &stkds->dsReqParamValid,"dsReqParamValid")
      )
    {

		switch(stkds->actionType)
        {
            case SATK_DATA_SERV_OPEN:
                if(stkds->is_UICC_server_mode)
                {
                    if(xdr_StkOpenChanSvrParam_t(xdrs, &stkds->u.open_ch_svr))
                        return(TRUE);
                    else
                        return(FALSE);
                }
                else
                {
                if(xdr_StkOpenChanParam_t(xdrs, &stkds->u.open_ch))
                    return(TRUE);
                else
                    return(FALSE);
                }

            case SATK_DATA_SERV_SEND_DATA:
                    if(xdr_StkSendDataParam_t(xdrs, &stkds->u.send_data))
                        return(TRUE);
                    else
                        return(FALSE);

            case SATK_DATA_SERV_REC_DATA:
            case SATK_DATA_SERV_CLOSE:
            default:
		        return(TRUE);
        }
    }
	else
		return(FALSE);
}

bool_t 
xdr_StkRunAtCmd_t(XDR* xdrs, StkRunAtCmd_t* rsp)
{
	XDR_LOG(xdrs,"StkRunAtCmd_t")

	if( xdr_SATKAlphaID_t(xdrs,  &rsp->text) &&
		xdr_SATKIcon_t(xdrs,  &rsp->icon)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_SATK_EventData_t(XDR* xdrs, SATK_EventData_t* satkeventdata)
{
	XDR_LOG(xdrs,"SATK_EventData_t")

	if(XDR_ENUM(xdrs,  &satkeventdata->msgType, SATK_EVENTS_t))
	{
		switch(satkeventdata->msgType)
		{
			case SATK_EVENT_DISPLAY_TEXT:
				return (xdr_pointer(xdrs, (char **)(void*) &satkeventdata->u.display_text, sizeof(DisplayText_t), (xdrproc_t)xdr_DisplayText_t) );
			case SATK_EVENT_GET_INKEY:
				return (xdr_pointer(xdrs, (char **)(void*) &satkeventdata->u.get_inkey, sizeof(GetInkey_t), (xdrproc_t)xdr_GetInkey_t) );
			case SATK_EVENT_GET_INPUT:
				return (xdr_pointer(xdrs, (char **)(void*) &satkeventdata->u.get_input, sizeof(GetInput_t), (xdrproc_t)xdr_GetInput_t) );
			case SATK_EVENT_PLAY_TONE:
				return (xdr_pointer(xdrs, (char **)(void*) &satkeventdata->u.play_tone, sizeof(PlayTone_t), (xdrproc_t)xdr_PlayTone_t) );
			case SATK_EVENT_SELECT_ITEM:
				return (xdr_pointer(xdrs, (char **)(void*) &satkeventdata->u.select_item, sizeof(SelectItem_t), (xdrproc_t)xdr_SelectItem_t) );
			case SATK_EVENT_SEND_SS:
				return (xdr_pointer(xdrs, (char **)(void*) &satkeventdata->u.send_ss, sizeof(SendSs_t), (xdrproc_t)xdr_SendSs_t) );
			case SATK_EVENT_SEND_USSD:
				return (xdr_pointer(xdrs, (char **)(void*) &satkeventdata->u.send_ussd, sizeof(SendUssd_t), (xdrproc_t)xdr_SendUssd_t) );
			case SATK_EVENT_SETUP_CALL:
				return (xdr_pointer(xdrs, (char **)(void*) &satkeventdata->u.setup_call, sizeof(SetupCall_t), (xdrproc_t)xdr_SetupCall_t) );
			case SATK_EVENT_SETUP_MENU:
				return (xdr_pointer(xdrs, (char **)(void*) &satkeventdata->u.setup_menu, sizeof(SetupMenu_t), (xdrproc_t)xdr_SetupMenu_t) );
			case SATK_EVENT_REFRESH:
				return (xdr_pointer(xdrs, (char **)(void*) &satkeventdata->u.refresh, sizeof(Refresh_t), (xdrproc_t)xdr_Refresh_t) );
			case SATK_EVENT_SEND_SHORT_MSG:
				return (xdr_pointer(xdrs, (char **)(void*) &satkeventdata->u.send_short_msg, sizeof(SendMOSMS_t), (xdrproc_t)xdr_SendMOSMS_t) );
			case SATK_EVENT_SEND_DTMF:
				return (xdr_pointer(xdrs, (char **)(void*) &satkeventdata->u.send_dtmf, sizeof(SendStkDtmf_t), (xdrproc_t)xdr_SendStkDtmf_t) );
			case SATK_EVENT_LAUNCH_BROWSER:
				return (xdr_pointer(xdrs, (char **)(void*) &satkeventdata->u.launch_browser, sizeof(LaunchBrowserReq_t), (xdrproc_t)xdr_LaunchBrowserReq_t) );
			case SATK_EVENT_IDLEMODE_TEXT:
				return (xdr_pointer(xdrs, (char **)(void*) &satkeventdata->u.idlemode_text, sizeof(IdleModeText_t), (xdrproc_t)xdr_IdleModeText_t) );
			case SATK_EVENT_DATA_SERVICE_REQ:
				return (xdr_pointer(xdrs, (char **)(void*) &satkeventdata->u.dataservicereq, sizeof(StkDataService_t), (xdrproc_t)xdr_StkDataService_t) );
			case SATK_EVENT_RUN_AT_CMD:
				return (xdr_pointer(xdrs, (char **)(void*) &satkeventdata->u.runatcmd, sizeof(StkRunAtCmd_t), (xdrproc_t)xdr_StkRunAtCmd_t) );
			case SATK_EVENT_ACTIVATE:
				return ( xdr_pointer(xdrs, (char **)(void*) &satkeventdata->u.activate_req, sizeof(UInt8), (xdrproc_t)xdr_u_char) );

			case SATK_EVENT_PROV_LOCAL_LANG:
				// No implementation in atleast ATC and CAPI
			case SATK_EVENT_MENU_SELECTION:
				return(TRUE);
			default:
				break;
		}
	}

	return FALSE;
}

bool_t xdr_StkCallSetupFail_t(XDR* xdrs, StkCallSetupFail_t* callsetupfail)
{
    XDR_LOG(xdrs,"StkCallSetupFail_t")

    if(XDR_ENUM(xdrs,  &callsetupfail->failRes, StkCallSetupFailResult_t))
	    return TRUE;
    else
        return FALSE;
}



bool_t
xdr_StkCallControlDisplay_t(XDR* xdrs, StkCallControlDisplay_t* callcontroldisp)
{
    XDR_LOG(xdrs,"StkCallControlDisplay_t")

    if(_xdr_u_char(xdrs, &callcontroldisp->alphaIdValid, "alphaIdValid") &&
       _xdr_u_char(xdrs, &callcontroldisp->displayTextLen, "displayTextLen") &&
       xdr_opaque(xdrs, (caddr_t)&callcontroldisp->displayText, ALPHA_ID_LEN) &&
       XDR_ENUM(xdrs, &callcontroldisp->oldCCType, StkCallControl_t) &&
       XDR_ENUM(xdrs, &callcontroldisp->newCCType, StkCallControl_t) &&
       XDR_ENUM(xdrs, &callcontroldisp->ccResult, StkCallControlResult_t)
      )
	    return TRUE;
    else
        return FALSE;
}




bool_t 
xdr_StkLangNotification_t(XDR* xdrs, StkLangNotification_t* rsp)
{
	XDR_LOG(xdrs,"StkLangNotification_t")

	return(xdr_opaque(xdrs, (caddr_t)&rsp->language, 3));
}


bool_t 
xdr_RunAT_Request(XDR* xdrs, RunAT_Request* rsp)
{
	XDR_LOG(xdrs,"RunAT_Request")

	if( _xdr_u_int16_t(xdrs, &rsp->length,"length") &&
		xdr_opaque(xdrs, (caddr_t)&rsp->request, MAX_AT_REQUEST+2)
		)
		return(TRUE);
	else
		return(FALSE);
}


bool_t
xdr_EnvelopeRspData_t(XDR* xdrs, EnvelopeRspData_t *rsp)
{
    XDR_LOG(xdrs,"EnvelopeRspData_t")
    if( _xdr_u_char(xdrs, &rsp->sw1,"sw1") &&
        _xdr_u_char(xdrs, &rsp->sw2,"sw2") &&
        _xdr_u_int16_t(xdrs, &rsp->rsp_len, "rsp_len") &&
		xdr_opaque(xdrs, (caddr_t)&rsp->rsp_data, sizeof(rsp->rsp_data))
	  )
		return(TRUE);
	else
		return(FALSE);
}


bool_t
xdr_TerminalResponseRspData_t(XDR* xdrs, TerminalResponseRspData_t *rsp)
{
    XDR_LOG(xdrs,"TerminalResponseRspData_t")
    if( _xdr_u_char(xdrs, &rsp->sw1,"sw1") &&
        _xdr_u_char(xdrs, &rsp->sw2,"sw2")
	  )
		return(TRUE);
	else
		return(FALSE);
}



bool_t
xdr_Subaddress_t(XDR* xdrs, Subaddress_t *subAddr)
{
    XDR_LOG(xdrs,"Subaddress_t")

    if(_xdr_u_char(xdrs, &subAddr->tos, "tos") &&
       _xdr_u_char(xdrs, &subAddr->odd_even, "odd_even") &&
       _xdr_u_char(xdrs, &subAddr->c_subaddr, "c_subaddr") &&
       xdr_opaque(xdrs, (caddr_t)&subAddr->subaddr, SUB_LENGTH)
      )
        return(TRUE);
    else
        return(FALSE);

}



bool_t
xdr_StkCallControlSetupRsp_t(XDR* xdrs, StkCallControlSetupRsp_t* rsp)
{

	XDR_LOG(xdrs,"xdr_StkCallControlSetupRsp_t")

	if( XDR_ENUM(xdrs, &rsp->cc_result, StkCallControlResult_t) &&
		XDR_ENUM(xdrs, &rsp->old_type, StkCallControl_t) &&
		XDR_ENUM(xdrs, &rsp->new_type, StkCallControl_t) &&

		_xdr_u_char(xdrs, &rsp->addr_changed, "addr_changed") &&
		_xdr_u_char(xdrs, &rsp->bc1_changed, "bc1_changed") &&
		_xdr_u_char(xdrs, &rsp->subaddr_changed, "subaddr_changed") &&
		_xdr_u_char(xdrs, &rsp->alpha_id_changed, "alpha_id_changed") &&
		_xdr_u_char(xdrs, &rsp->bc_repeat_changed, "bc_repeat_changed") &&
		_xdr_u_char(xdrs, &rsp->bc2_changed, "bc2_changed") &&

		XDR_ENUM(xdrs, &rsp->ton, gsm_TON_t) &&
		XDR_ENUM(xdrs, &rsp->npi, gsm_NPI_t) &&
		xdr_opaque(xdrs, (caddr_t)&rsp->number, MAX_DIGITS + 1) &&

		xdr_BearerCapability_t(xdrs, &rsp->bc1) &&
		xdr_Subaddress_t(xdrs, &rsp->subaddr_data) &&

		_xdr_u_char(xdrs, &rsp->alpha_id_valid, "alpha_id_valid") &&
		_xdr_u_char(xdrs, &rsp->alpha_id_len, "alpha_id_len") &&
		xdr_opaque(xdrs, (caddr_t)&rsp->alpha_id, ALPHA_ID_LEN) &&
		xdr_BearerCapability_t(xdrs, &rsp->bc2) &&
		_xdr_u_char(xdrs, &rsp->bc_repeat, "bc_repeat")
		)
		return(TRUE);
	else
		return(FALSE);

}





bool_t 
xdr_StkCallControlSsRsp_t(XDR* xdrs, StkCallControlSsRsp_t* rsp)
{
	XDR_LOG(xdrs,"StkCallControlSsRsp_t")

	if( XDR_ENUM(xdrs, &rsp->cc_result, StkCallControlResult_t) &&
		XDR_ENUM(xdrs, &rsp->old_type, StkCallControl_t) &&
        XDR_ENUM(xdrs, &rsp->new_type, StkCallControl_t) &&
        _xdr_u_char(xdrs, &rsp->ss_str_changed, "ss_str_changed") &&
        _xdr_u_char(xdrs, &rsp->ton_npi, "ton_npi") &&
        _xdr_u_char(xdrs, &rsp->ss_len, "ss_len") &&
        xdr_opaque(xdrs, (caddr_t)&rsp->ss_data, MAX_DIGITS >> 1) &&
        _xdr_u_char(xdrs, &rsp->alpha_id_valid, "alpha_id_valid") &&
        _xdr_u_char(xdrs, &rsp->alpha_id_len, "alpha_id_len") &&
        xdr_opaque(xdrs, (caddr_t)&rsp->alpha_id, ALPHA_ID_LEN)
		)
		return(TRUE);
	else
		return(FALSE);
}



bool_t 
xdr_StkCallControlUssdRsp_t(XDR* xdrs, StkCallControlUssdRsp_t* rsp)
{
	XDR_LOG(xdrs,"StkCallControlUssdRsp_t")

	if( XDR_ENUM(xdrs, &rsp->cc_result, StkCallControlResult_t) &&
		XDR_ENUM(xdrs, &rsp->old_type, StkCallControl_t) &&
        XDR_ENUM(xdrs, &rsp->new_type, StkCallControl_t) &&
        _xdr_u_char(xdrs, &rsp->ussd_str_changed, "ussd_str_changed") &&
        _xdr_u_char(xdrs, &rsp->ussd_len, "ussd_len") &&
        xdr_opaque(xdrs, (caddr_t)&rsp->ussd_data, PHASE2_MAX_USSD_STRING_SIZE+1) &&
        _xdr_u_char(xdrs, &rsp->alpha_id_valid, "alpha_id_valid") &&
        _xdr_u_char(xdrs, &rsp->alpha_id_len, "alpha_id_len") &&
        xdr_opaque(xdrs, (caddr_t)&rsp->alpha_id, ALPHA_ID_LEN) &&
        _xdr_u_char(xdrs, &rsp->ussd_data_dcs, "ussd_data_dcs")
		)
		return(TRUE);
	else
		return(FALSE);
}





bool_t 
xdr_StkCallControlSmsRsp_t(XDR* xdrs, StkCallControlSmsRsp_t* rsp)
{
	XDR_LOG(xdrs,"StkCallControlSmsRsp_t")

	if( XDR_ENUM(xdrs, &rsp->cc_result, StkCallControlResult_t) &&
        _xdr_u_char(xdrs, &rsp->sca_changed, "sca_changed") &&
        _xdr_u_char(xdrs, &rsp->dest_changed, "dest_changed") &&        
        _xdr_u_char(xdrs, &rsp->sca_toa, "sca_toa") &&
        _xdr_u_char(xdrs, &rsp->sca_number_len, "sca_number_len") &&
        xdr_opaque(xdrs, (caddr_t)&rsp->sca_number, SMS_MAX_DIGITS >> 1) &&
        _xdr_u_char(xdrs, &rsp->dest_toa, "dest_toa") &&
        _xdr_u_char(xdrs, &rsp->dest_number_len, "dest_number_len") &&
        xdr_opaque(xdrs, (caddr_t)&rsp->dest_number, SMS_MAX_DIGITS >> 1) &&
        _xdr_u_char(xdrs, &rsp->alpha_id_valid, "alpha_id_valid") &&
        _xdr_u_char(xdrs, &rsp->alpha_id_len, "alpha_id_len") &&
        xdr_opaque(xdrs, (caddr_t)&rsp->alpha_id, ALPHA_ID_LEN) 
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_StkTermProfileInd_t(XDR* xdrs, StkTermProfileInd_t* res)
{
	XDR_LOG(xdrs,"xdr_STKTermProfileInd_t")

    if(XDR_ENUM(xdrs, &res->status, StkStatus_t))
    {
        return(TRUE);
    }
    else
	{
        return(FALSE);
	}
}

bool_t xdr_StkUiccCatInd_t(XDR* xdrs, StkUiccCatInd_t* res)
{
	XDR_LOG(xdrs,"xdr_STKUiccCatInd_t")

    if(XDR_ENUM(xdrs, &res->cardType, StkCardType_t) &&
       xdr_Boolean(xdrs, &res->isTpRequired))
    {
        return(TRUE);
    }
    else
	{
        return(FALSE);
	}
}

bool_t xdr_ProactiveCmdData_t(XDR* xdrs, ProactiveCmdData_t* res)
{
	XDR_LOG(xdrs,"xdr_ProactiveCmdData_t")

	if( _xdr_UInt16(xdrs, &res->data_len,"data_len") &&
		xdr_opaque(xdrs, (caddr_t)&res->data, 256) )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

bool_t xdr_StkExtProactiveCmdInd_t(XDR* xdrs, StkExtProactiveCmdInd_t* res)
{
	XDR_LOG(xdrs,"xdr_StkExtProactiveCmdInd_t")

    if( _xdr_UInt8(xdrs, &res->sw1, "sw1") &&
		_xdr_UInt8(xdrs, &res->dataLen, "dataLen") &&
		xdr_opaque(xdrs, (caddr_t)&res->data, 256) )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

bool_t xdr_StkProactiveCmdFetchingOnOffRsp_t(void* xdrs, StkProactiveCmdFetchingOnOffRsp_t *rsp)
{
	XDR_LOG(xdrs,"xdr_StkProactiveCmdFetchingOnOffRsp_t")

    if(XDR_ENUM(xdrs, &rsp->result, StkStatus_t))
    {
        return(TRUE);
    }
    else
	{
        return(FALSE);
	}
}

bool_t xdr_StkPollingIntervalRsp_t(void* xdrs, StkPollingIntervalRsp_t* rsp)
{
	XDR_LOG(xdrs,"xdr_StkPollingIntervalRsp_t")

    if(XDR_ENUM(xdrs, &rsp->result, StkStatus_t))
    {
        return(TRUE);
    }
    else
	{
        return(FALSE);
	}
}
