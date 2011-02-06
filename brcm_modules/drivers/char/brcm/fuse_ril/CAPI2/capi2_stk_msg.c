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


#include	"capi2_reqrep.h"

// Fixme: Generic
XDR_ENUM_FUNC(Unicode_t)
XDR_ENUM_FUNC(SATK_EVENTS_t)
XDR_ENUM_FUNC(SATK_ResultCode_t)
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
XDR_ENUM_FUNC(TypeOfNumber_t)
XDR_ENUM_FUNC(NumberPlanId_t)
XDR_ENUM_FUNC(STKMenuSelectionResCode_t)

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
		//Fixme: The correct max length needs to be determined
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
xdr_CAPI2_SetupMenu_t(XDR* xdrs, CAPI2_SetupMenu_t* rsp)
{
	XDR_LOG(xdrs,"CAPI2_SetupMenu_t")

	return (xdr_pointer(xdrs, (char **)(void*) &rsp->rsp, sizeof(SetupMenu_t), (xdrproc_t)xdr_SetupMenu_t) );
}


bool_t 
xdr_CAPI2_SATKCmdResp_t(XDR* xdrs, CAPI2_SATKCmdResp_t* req)
{
	XDR_LOG(xdrs,"CAPI2_SATKCmdResp_t")

	if( XDR_ENUM(xdrs,  &req->toEvent, SATK_EVENTS_t) &&
		_xdr_u_char(xdrs,  &req->result1,"result1") &&
		_xdr_u_char(xdrs,  &req->result2,"result2") &&
		xdr_pointer(xdrs, (char **)(void*) &req->inText, sizeof(SATKString_t), (xdrproc_t)xdr_SATKString_t) &&
		_xdr_u_char(xdrs,  &req->menuID,"menuID")
		)
		return(TRUE);
	else
		return(FALSE);
}


bool_t 
xdr_StkCmdRespond_t(XDR* xdrs, StkCmdRespond_t* stkcmdrespond)
{
	XDR_LOG(xdrs,"StkCmdRespond_t")

	if( _xdr_u_int16_t(xdrs, &stkcmdrespond->clientID,"clientID") &&
		XDR_ENUM(xdrs,  &stkcmdrespond->event, SATK_EVENTS_t) &&
		XDR_ENUM(xdrs,  &stkcmdrespond->result1, SATK_ResultCode_t) &&
		_xdr_u_char(xdrs,  &stkcmdrespond->result2,"result2") &&
		xdr_vector(xdrs,  (char*)stkcmdrespond->textStr1, SATK_LOGIN_PASSWD_LEN+1, sizeof(unsigned char), (xdrproc_t)xdr_u_char) &&
		xdr_vector(xdrs,  (char*)stkcmdrespond->textStr2, SATK_LOGIN_PASSWD_LEN+1, sizeof(unsigned char), (xdrproc_t)xdr_u_char) &&
		xdr_vector(xdrs,  (char*)stkcmdrespond->textStr3, SATK_MAX_APN_LEN+1, sizeof(unsigned char), (xdrproc_t)xdr_u_char)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_CAPI2_StkCmdRespond_t(XDR* xdrs, CAPI2_StkCmdRespond_t* req)
{
	XDR_LOG(xdrs,"CAPI2_StkCmdRespond_t")

	return (xdr_pointer(xdrs, (char **)(void*) &req->req, sizeof(StkCmdRespond_t), (xdrproc_t)xdr_StkCmdRespond_t) );
}

bool_t 
xdr_DisplayText_t(XDR* xdrs, DisplayText_t* displaytxt)
{
	XDR_LOG(xdrs,"DisplayText_t")

	if( xdr_SATKString_t(xdrs,  &displaytxt->stkStr) &&
		_xdr_u_char(xdrs,  &displaytxt->isHighPrio,"isHighPrio") &&
		xdr_SATKIcon_t(xdrs,  &displaytxt->icon) &&
		_xdr_u_char(xdrs,  &displaytxt->isDelay,"isDelay") &&
		_xdr_u_char(xdrs,  &displaytxt->isSustained,"isSustained")
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
		_xdr_u_char(xdrs,  &getinkey->isHelpAvailable,"isHelpAvailable")
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
xdr_StkDataService_t(XDR* xdrs, StkDataService_t* stkds)
{
	XDR_LOG(xdrs,"StkDataService_t")

	if( _xdr_u_char(xdrs,  &stkds->chanID,"chanID") &&
		XDR_ENUM(xdrs,  &stkds->actionType, SATKDataServiceType_t) &&
		xdr_SATKString_t(xdrs,  &stkds->text) &&
		xdr_SATKIcon_t(xdrs,  &stkds->icon) &&
		_xdr_u_char(xdrs,  &stkds->isLoginNeeded,"isLoginNeeded") &&
		_xdr_u_char(xdrs,  &stkds->isPasswdNeeded,"isPasswdNeeded") &&
		_xdr_u_char(xdrs,  &stkds->isApnNeeded,"isApnNeeded")
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
			case SATK_EVENT_ACTIVATE:
				return ( xdr_pointer(xdrs, (char **)(void*) &satkeventdata->u.activate_req, sizeof(UInt8), (xdrproc_t)xdr_u_char) );
            case SATK_EVENT_RUN_AT_CMD:
				return (xdr_pointer(xdrs, (char **)(void*) &satkeventdata->u.runatcmd, sizeof(StkRunAtCmd_t), (xdrproc_t)xdr_StkRunAtCmd_t) );

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

bool_t xdr_SATK_CallSetupFail_t(XDR* xdrs, StkCallSetupFail_t* callsetupfail)
{
    XDR_LOG(xdrs,"StkCallSetupFail_t")

    if(XDR_ENUM(xdrs,  &callsetupfail->failRes, StkCallSetupFailResult_t))
	    return TRUE;
    else
        return FALSE;
}

bool_t
xdr_CAPI2_SATK_CallSetupFail_t(XDR* xdrs, CAPI2_SATK_CallSetupFail_t* req)
{
    XDR_LOG(xdrs,"CAPI2_SATK_CallSetupFail_t")

	return (xdr_pointer(xdrs, (char **)(void*) &req->rsp, sizeof(StkCallSetupFail_t), (xdrproc_t)xdr_SATK_CallSetupFail_t) );
}

bool_t
xdr_SATK_CallControlDisplay_t(XDR* xdrs, StkCallControlDisplay_t* callcontroldisp)
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
xdr_CAPI2_SATK_CallControlDisplay_t(XDR* xdrs, CAPI2_SATK_CallControlDisplay_t* req)
{
    XDR_LOG(xdrs,"CAPI2_SATK_CallControlDisplay_t")

	return (xdr_pointer(xdrs, (char **)(void*) &req->rsp, sizeof(StkCallControlDisplay_t), (xdrproc_t)xdr_SATK_CallControlDisplay_t) );
}

bool_t 
xdr_CAPI2_SATK_EventData_t(XDR* xdrs, CAPI2_SATK_EventData_t* req)
{
	XDR_LOG(xdrs,"CAPI2_SATK_EventData_t")

	return (xdr_pointer(xdrs, (char **)(void*) &req->rsp, sizeof(SATK_EventData_t), (xdrproc_t)xdr_SATK_EventData_t) );
}

bool_t
xdr_CAPI2_TermProfile_t(XDR* xdrs, CAPI2_TermProfile_t* reqrsp)
{
	XDR_LOG(xdrs,"CAPI2_TermProfile_t")

	if( _xdr_u_char(xdrs, &reqrsp->length,"length")
		)
	{
		u_int len = (u_int)reqrsp->length;
		return ( xdr_bytes(xdrs, (char**)(void*)&reqrsp->data, &len,255) );
	}
	return(FALSE);
}

bool_t 
xdr_StkLangNotification_t(XDR* xdrs, StkLangNotification_t* rsp)
{
	XDR_LOG(xdrs,"StkLangNotification_t")

	return(xdr_opaque(xdrs, (caddr_t)&rsp->language, 3));
}

bool_t 
xdr_CAPI2_StkLangNotification_t(XDR* xdrs, CAPI2_StkLangNotification_t* rsp)
{
	XDR_LOG(xdrs,"CAPI2_StkLangNotification_t")

	return (xdr_pointer(xdrs, (char **)(void*) &rsp->rsp, sizeof(StkLangNotification_t), (xdrproc_t)xdr_StkLangNotification_t) );
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

bool_t 
xdr_StkRunAtReq_t(XDR* xdrs, RunAT_Request* rsp)
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
xdr_CAPI2_StkRunAtReq_t(XDR* xdrs, CAPI2_StkRunAtReq_t* rsp)
{
	XDR_LOG(xdrs,"CAPI2_StkRunAtReq_t")

    return (xdr_pointer(xdrs, (char **)(void*) &rsp->rsp, sizeof(RunAT_Request), (xdrproc_t)xdr_StkRunAtReq_t) );
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
xdr_CAPI2_SATK_SendCcSetupReq_t(XDR* xdrs, CAPI2_SATK_SendCcSetupReq_t* req)
{
	XDR_LOG(xdrs,"CAPI2_SATK_SendCcSetupReq_t")

	if( XDR_ENUM(xdrs, &req->ton, TypeOfNumber_t) &&
		XDR_ENUM(xdrs, &req->npi, NumberPlanId_t) &&
        xdr_uchar_ptr_t(xdrs, (unsigned char **)&req->number)	&&
        xdr_pointer(xdrs, (char **)(void*) &req->bc1, sizeof(BearerCapability_t), (xdrproc_t)xdr_BearerCapability_t) &&
        xdr_pointer(xdrs, (char **)(void*) &req->subaddr_data, sizeof(Subaddress_t), (xdrproc_t)xdr_Subaddress_t) &&
        xdr_pointer(xdrs, (char **)(void*) &req->bc2, sizeof(BearerCapability_t), (xdrproc_t)xdr_BearerCapability_t) &&
        _xdr_u_char(xdrs, &req->bc_repeat_ind, "bc_repeat_ind") &&
        _xdr_u_char(xdrs, &req->simtk_orig, "simtk_orig")
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
xdr_CAPI2_StkCallControlSetupRsp_t(XDR* xdrs, CAPI2_StkCallControlSetupRsp_t* rsp)
{

	XDR_LOG(xdrs,"CAPI2_StkCallControlSetupRsp_t")

      return xdr_pointer(xdrs, (char **)(void*) &rsp->rsp, sizeof(StkCallControlSetupRsp_t), (xdrproc_t)xdr_StkCallControlSetupRsp_t);



}

bool_t 
xdr_CAPI2_SATK_SendCcSsReq_t(XDR* xdrs, CAPI2_SATK_SendCcSsReq_t* req)
{
	XDR_LOG(xdrs,"CAPI2_SATK_SendCcSsReq_t")

	if( _xdr_u_char(xdrs, &req->ton_npi, "ton_npi") &&
        _xdr_u_char(xdrs, &req->ss_len, "ss_len") &&
        xdr_opaque(xdrs, (caddr_t)&req->ss_data, req->ss_len) &&        
        _xdr_u_char(xdrs, &req->simtk_orig, "simtk_orig")
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
xdr_CAPI2_StkCallControlSsRsp_t(XDR* xdrs, CAPI2_StkCallControlSsRsp_t* rsp)
{
	XDR_LOG(xdrs,"CAPI2_StkCallControlSsRsp_t")

      return xdr_pointer(xdrs, (char **)(void*) &rsp->rsp, sizeof(StkCallControlSsRsp_t), (xdrproc_t)xdr_StkCallControlSsRsp_t);

}

bool_t 
xdr_CAPI2_SATK_SendCcUssdReq_t(XDR* xdrs, CAPI2_SATK_SendCcUssdReq_t* req)
{
	XDR_LOG(xdrs,"CAPI2_SATK_SendCcUssdReq_t")

	if( _xdr_u_char(xdrs, &req->ussd_dcs, "ussd_dcs") &&
        _xdr_u_char(xdrs, &req->ussd_len, "ussd_len") &&
        xdr_opaque(xdrs, (caddr_t)&req->ussd_data, req->ussd_len) &&
        _xdr_u_char(xdrs, &req->simtk_orig, "simtk_orig")
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
xdr_CAPI2_StkCallControlUssdRsp_t(XDR* xdrs, CAPI2_StkCallControlUssdRsp_t* rsp)
{
	XDR_LOG(xdrs,"CAPI2_StkCallControlUssdRsp_t")
      return xdr_pointer(xdrs, (char **)(void*) &rsp->rsp, sizeof(StkCallControlUssdRsp_t), (xdrproc_t)xdr_StkCallControlUssdRsp_t);

}

bool_t 
xdr_CAPI2_SATK_SendCcSmsReq_t(XDR* xdrs, CAPI2_SATK_SendCcSmsReq_t* req)
{
	XDR_LOG(xdrs,"CAPI2_SATK_SendCcSmsReq_t")

	if( _xdr_u_char(xdrs, &req->sca_toa, "sca_toa") &&
        _xdr_u_char(xdrs, &req->sca_number_len, "sca_number_len") &&
        xdr_opaque(xdrs, (caddr_t)&req->sca_number, req->sca_number_len) &&
        _xdr_u_char(xdrs, &req->dest_toa, "dest_toa") &&
        _xdr_u_char(xdrs, &req->dest_number_len, "dest_number_len") &&
        xdr_opaque(xdrs, (caddr_t)&req->dest_number, req->dest_number_len) &&
        _xdr_u_char(xdrs, &req->simtk_orig, "simtk_orig")
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

bool_t
xdr_CAPI2_StkCallControlSmsRsp_t(XDR* xdrs, CAPI2_StkCallControlSmsRsp_t* rsp)
{
	XDR_LOG(xdrs,"CAPI2_StkCallControlSmsRsp_t")
      return xdr_pointer(xdrs, (char **)(void*) &rsp->rsp, sizeof(StkCallControlSmsRsp_t), (xdrproc_t)xdr_StkCallControlSmsRsp_t);

}

bool_t
xdr_StkRunIpAtReq_t(XDR* xdrs, RunIpAtReq_t *rsp)
{
	XDR_LOG(xdrs,"StkRunIpAtReq_t")

    if( _xdr_u_int16_t(xdrs, &rsp->length,"length") &&
        xdr_opaque(xdrs, (caddr_t)&rsp->request, MAX_AT_REQUEST+2)        
      )
        return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_CAPI2_StkRunIpAtReq_t(XDR* xdrs, CAPI2_StkRunIpAtReq_t* rsp)
{
	XDR_LOG(xdrs,"CAPI2_StkRunIpAtReq_t")

    return (xdr_pointer(xdrs, (char **)(void*) &rsp->rsp, sizeof(RunIpAtReq_t), (xdrproc_t)xdr_StkRunIpAtReq_t) );
}

bool_t
xdr_StkMenuSelectionRes_t(XDR* xdrs, StkMenuSelectionRes_t* res)
{
    XDR_LOG(xdrs,"xdr_StkMenuSelectionRes_t")

    if(XDR_ENUM(xdrs, &res->resCode, STKMenuSelectionResCode_t))
    {
        return(TRUE);
    }
    else
        return(FALSE);
}

bool_t
xdr_CAPI2_StkMenuSelectionRes_t(XDR* xdrs, CAPI2_StkMenuSelectionRes_t* res)
{
	XDR_LOG(xdrs,"CAPI2_StkMenuSelectionRes_t")
      return xdr_pointer(xdrs, (char **)(void*) &res->rsp, sizeof(STKMenuSelectionResCode_t), (xdrproc_t)xdr_StkMenuSelectionRes_t);
}
