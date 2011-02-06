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
*   @file   capi2_stk_api.c
*
*   @brief  This file contains SATK API related implementation.
*
****************************************************************************/
/**

*   @defgroup   CAPI2_STKAPIGroup   SIM Toolkit
*   @ingroup    CAPI2_STKGroup
*
*   @brief      This group defines the types and prototypes for the SIM toolkit API functions.
*	
****************************************************************************/

#include "capi2_reqrep.h"
#include "capi2_stk_ds.h"
#include "capi2_stk_api.h"
	
/**
 * @addtogroup CAPI2_STKAPIGroup
 * @{
 */




//-------------------------------------------------
// Function Prototype
//-------------------------------------------------

/**
API function to get cached root menu list
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_SATK_GETCACHEDROOTMENUPTR_RSP
	@n@b Result_t : 
	@n@b ResultData : ::SetupMenu_t
**/	
void CAPI2_SATK_GetCachedRootMenuPtr(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SATK_GETCACHEDROOTMENUPTR_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

/**
	This function sends User Activity Event (one of the features in STK Event Download) to SIMMAP (SIMMAP 
	will decide whether to send the event to SIM based upon the Event List stored in SIMMAP).
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_SATK_SENDUSERACTIVITYEVENT_RSP
	@n@b Result_t : 
	@n@b ResultData : None
**/	
void CAPI2_SATK_SendUserActivityEvent(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SATK_SENDUSERACTIVITYEVENT_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

/**
	This function sends Idle Screen Available Event (one of the features in STK Event Download) to SIMMAP (SIMMAP 
	will decide whether to send the event to SIM based upon the Event List stored in SIMMAP). 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_SATK_SENDIDLESCREENAVAIEVENT_RSP
	@n@b Result_t : 
	@n@b ResultData : None
**/	
void CAPI2_SATK_SendIdleScreenAvaiEvent(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SATK_SENDIDLESCREENAVAIEVENT_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

/**
	This function sends Language Selection Event (one of the features in STK Event Download) to SIMMAP (SIMMAP 
	will decide whether to send the event to SIM based upon the Event List stored in SIMMAP). 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		language (in) language code of a pair of alpha-numeric characters, as defined in ISO 639. For the 
				example of English ("en"), pass language = 0x656E.
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_SATK_SENDLANGSELECTEVENT_RSP
	@n@b Result_t : 
	@n@b ResultData : None
**/	
void CAPI2_SATK_SendLangSelectEvent(UInt32 tid, UInt8 clientID, UInt16 language)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SATK_SendLangSelectEvent_Req = language;
	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SATK_SENDLANGSELECTEVENT_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

/**
	This function sends sends Browser Termination Event (one of the features in STK Event Download) to SIMMAP (SIMMAP 
	will decide whether to send the event to SIM based upon the Event List stored in SIMMAP). 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		user_term (in) TRUE if browser is terminated by user. FALSE if browser is terminated because of 
				error. See Section 11.9.2 and  12.51 in ETSI TS 101 267 (GSM 11.14).
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_SATK_SENDBROWSERTERMEVENT_RSP
	@n@b Result_t : 
	@n@b ResultData : None
**/	
void CAPI2_SATK_SendBrowserTermEvent(UInt32 tid, UInt8 clientID, Boolean user_term)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SATK_SendBrowserTermEvent_Req = user_term;
	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SATK_SENDBROWSERTERMEVENT_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}


//***************************************************************************************
/**
	API function to send back the response for STK event. It could be used as simple a
	response or to select a menu item, etc.	
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) ID of the STK module (currently not used, set to 0)
	@param		toEvent (in) event type of the STK event
	@param		result1 (in) result code for the event received
	@param		result2 (in) secondary result code for the event received 
	@param		*inText (in) string, any text string to be send back to SIMAP
	@param		menuID (in) menu ID to be selected from the menu list
	@note
	If any of the argument is not applicable to the particular case, simply fill it with 0 or NULL.
	@n@b Responses 
	@n@b MsgType_t : ::MSG_SATK_CMDRESP_RSP
	@n@b Result_t : 
	@n@b ResultData : Boolean
**/	
void CAPI2_SATKCmdResp(UInt32 tid, UInt8 clientID, SATK_EVENTS_t toEvent, UInt8 result1, 
				UInt8 result2, SATKString_t* inText, UInt8 menuID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SATKCmdResp_req.toEvent = toEvent;
	req.req_rep_u.CAPI2_SATKCmdResp_req.result1 = result1;
	req.req_rep_u.CAPI2_SATKCmdResp_req.result2 = result2;
	req.req_rep_u.CAPI2_SATKCmdResp_req.inText = inText;
	req.req_rep_u.CAPI2_SATKCmdResp_req.menuID = menuID;
	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SATK_CMDRESP_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}


/**
	This is the API function to send user response back to SIM for "Open Channel" user confirmation. 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) ID of the STK module (currently not used, set to 0)
	@param		response (in) terminal response data
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_SATK_DATASERVCMDRESP_RSP
	@n@b Result_t : 
	@n@b ResultData : Boolean
		- TRUE if user confirmation is accepted;
		- FALSE if error encountered.
**/
void CAPI2_SATKDataServCmdResp(UInt32 tid, UInt8 clientID, const StkCmdRespond_t* response)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_StkCmdRespond_Req.req = (StkCmdRespond_t*)response;
	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SATK_DATASERVCMDRESP_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

/**
	This is the API function to send SIM Data Services Envelope commands
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) ID of the STK module (currently not used, set to 0)
	@param		response (in) terminal response data
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_SATK_DATASERVREQ_RSP
	@n@b Result_t : 
	@n@b ResultData : Boolean
		- TRUE if user confirmation is accepted;
		- FALSE if error encountered.
**/
void CAPI2_SATKSendDataServReq(UInt32 tid, UInt8 clientID, const StkDataServReq_t* dsReq)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SATKSendDataServReq_Req.req = (StkDataServReq_t*)dsReq;
	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SATK_SENDDATASERVREQ_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

/**
	This is the API function to set the SIM Application Toolkit Terminal Profile data. 
	The data passed in this function will take precedence over the Terminal Profile
	data saved in Sys Parm. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) ID of the STK module (currently not used, set to 0)
	@param		data (in) Terminal Profile data
	@param		length (in) length of the terminal profile data.

	@note
	The value of length must not be larger than MAX_TERMINAL_PROFILE_ARRAY_SIZE. 

	This function needs to be called before the system is powered up, e.g. before 
	SYS_ProcessPowerUpReq() or SYS_ProcessNoRfReq() is called. 
	@n@b Responses 
	@n@b MsgType_t : ::MSG_SATK_SETTERMPROFILE_RSP
	@n@b Result_t : 
	@n@b ResultData : None.
**/
void CAPI2_SATK_SetTermProfile(UInt32 tid, UInt8 clientID, const UInt8 *data, UInt8 length)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_TermProfile_ReqRsp.length = length;
	req.req_rep_u.CAPI2_TermProfile_ReqRsp.data = (UInt8*)data;
	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SATK_SETTERMPROFILE_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}


/**
	This is the API function to return the SIM Application Toolkit Terminal Profile data. 
	
	If the data has been set by SATK_SetTermProfile() before, it returns that data. Otherwise
	it returns the Terminal Profile data in Sys Parm. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) ID of the STK module (currently not used, set to 0)

	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_SATK_GETTERMPROFILE_RSP
	@n@b Result_t : 
	@n@b ResultData : ::CAPI2_TermProfile_t
	
**/
void CAPI2_SATK_GetTermProfile(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	
	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SATK_GETTERMPROFILE_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

/**
	API function to Send Call Control (voice/fax/data/GPRS) request to SIMMAP.
	@param		clientID (in) ID of the STK module (currently not used, set to 0)
	@param		ton (in) TON of the number
	@param		npi (in) NPI of the number
	@param		number (in) ASCII encoded called party number 
	@param		*bc1 (in) bearer capability 1
	@param		subaddr_data (in) sub address data
	@param		*bc2 (in) bearer capability 2
	@param		bc_repeat_ind (in) bc repeat ind
	@param		simtk_orig (in) is STK originated
	@note
	One of MSG_STK_CALL_CONTROL_SETUP_RSP, MSG_STK_CALL_CONTROL_SS_RSP, & MSG_STK_CALL_CONTROL_USSD_RSP will be 
	passed to the callback function to send back the response. 
**/	
void CAPI2_SATK_SendCcSetupReq(UInt32 tid,
                               UInt8				clientID,
							   TypeOfNumber_t		ton,
							   NumberPlanId_t		npi,
							   char*				number,
							   BearerCapability_t*	bc1,
							   Subaddress_t*		subaddr_data,
							   BearerCapability_t*	bc2,
							   UInt8				bc_repeat_ind,
							   Boolean				simtk_orig)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

    /* message specific */
	req.req_rep_u.CAPI2_StkSendCcSetupReq_Req.ton = ton;
	req.req_rep_u.CAPI2_StkSendCcSetupReq_Req.npi = npi;
	req.req_rep_u.CAPI2_StkSendCcSetupReq_Req.number = number;
	req.req_rep_u.CAPI2_StkSendCcSetupReq_Req.bc1 = bc1;
	req.req_rep_u.CAPI2_StkSendCcSetupReq_Req.subaddr_data = subaddr_data;
    req.req_rep_u.CAPI2_StkSendCcSetupReq_Req.bc_repeat_ind = bc_repeat_ind;
    req.req_rep_u.CAPI2_StkSendCcSetupReq_Req.simtk_orig = simtk_orig;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SATK_SEND_CC_SETUP_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

/**
	API function to Send SS Call Control request to SIMMAP.
	@param		clientID (in) ID of the STK module (currently not used, set to 0)
	@param		ton_npi (in) TON/NPI byte: one of UNKNOWN_TON_UNKNOWN_NPI (128), UNKNOWN_TON_ISDN_NPI (129), INTERNA_TON_ISDN_NPI (145) & SIM_RAW_EMPTY_VALUE(0xFF)
	@param		ss_len (in) Number of bytes in the passed "ss_data". 
	@param		ss_data (in) BCD encoded SS data 
	@param		simtk_orig (in) is STK originated
	@note
	One of MSG_STK_CALL_CONTROL_SETUP_RSP, MSG_STK_CALL_CONTROL_SS_RSP, & MSG_STK_CALL_CONTROL_USSD_RSP will be 
	passed to the callback function to send back the response. 
**/	
void CAPI2_SATK_SendCcSsReq(UInt32 tid, UInt8 clientID, UInt8 ton_npi, UInt8 ss_len, const UInt8 *ss_data, Boolean simtk_orig )
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

    /* message specific */
	req.req_rep_u.CAPI2_StkSendCcSsReq_Req.ton_npi = ton_npi;
	req.req_rep_u.CAPI2_StkSendCcSsReq_Req.ss_len = ss_len;
	memcpy(req.req_rep_u.CAPI2_StkSendCcSsReq_Req.ss_data, ss_data, ss_len);
	req.req_rep_u.CAPI2_StkSendCcSsReq_Req.simtk_orig = simtk_orig;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SATK_SEND_CC_SS_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}
/**
	API function to Send USSD Call Control request to SIMMAP.
	@param		clientID (in) ID of the STK module (currently not used, set to 0)
	@param		ussd_dcs (in) USSD Data Coding Scheme
	@param		ussd_len (in) Number of bytes of USSD data 
	@param		ussd_data (in) USSD data 
	@param		simtk_orig (in) is STK originated
	@note
	One of MSG_STK_CALL_CONTROL_SETUP_RSP, MSG_STK_CALL_CONTROL_SS_RSP, & MSG_STK_CALL_CONTROL_USSD_RSP will be 
	passed to the callback function to send back the response. 
**/	
void CAPI2_SATK_SendCcUssdReq(UInt32 tid, UInt8 clientID, UInt8 ussd_dcs, UInt8 ussd_len, const UInt8 *ussd_data, Boolean simtk_orig )
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

    /* message specific */
	req.req_rep_u.CAPI2_StkSendCcUssdReq_Req.ussd_dcs = ussd_dcs;
	req.req_rep_u.CAPI2_StkSendCcUssdReq_Req.ussd_len = ussd_len;
	memcpy(req.req_rep_u.CAPI2_StkSendCcUssdReq_Req.ussd_data, ussd_data, ussd_len);
	req.req_rep_u.CAPI2_StkSendCcUssdReq_Req.simtk_orig = simtk_orig;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SATK_SEND_CC_USSD_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

/**
	API function to Send SMS Control request to SIMMAP.
	@param		clientID (in) ID of the STK module (currently not used, set to 0)
	@param		sca_toa (in) SMS service center type of address
	@param		sca_number_len (in) SMS service center address length
	@param		sca_number (in) SMS service center number (BCD)
	@param		dest_toa (in) SMS destination type of address
	@param		dest_number_len (in) SMS destination address length
	@param		dest_number (in) SMS destination number (BCD)
	@param		simtk_orig (in) is STK originated
	@note
	MSG_STK_CALL_CONTROL_SMS_RSP will be passed to the callback function to send back the response. 
**/	
void CAPI2_SATK_SendCcSmsReq(UInt32 tid,
                        UInt8	clientID,
						UInt8	sca_toa,
						UInt8	sca_number_len,
						UInt8*	sca_number,
					    UInt8	dest_toa,
						UInt8	dest_number_len,
						UInt8*	dest_number,
					    Boolean	simtk_orig )
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

    /* message specific */
	req.req_rep_u.CAPI2_StkSendCcSmsReq_Req.sca_toa = sca_toa;
	req.req_rep_u.CAPI2_StkSendCcSmsReq_Req.sca_number_len = sca_number_len;
	memcpy(req.req_rep_u.CAPI2_StkSendCcSmsReq_Req.sca_number, sca_number, sca_number_len);
	req.req_rep_u.CAPI2_StkSendCcSmsReq_Req.dest_toa = dest_toa;
	req.req_rep_u.CAPI2_StkSendCcSmsReq_Req.dest_number_len = dest_number_len;
    memcpy(req.req_rep_u.CAPI2_StkSendCcSmsReq_Req.dest_number, dest_number, dest_number_len);
    req.req_rep_u.CAPI2_StkSendCcSmsReq_Req.simtk_orig = simtk_orig;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SATK_SEND_CC_SMS_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}
/** @} */

