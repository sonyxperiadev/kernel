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
*   @file   capi2_stk_api_old.h
*
*   @brief  This file contains SATK API related defines.
*
****************************************************************************/
/**

*   @defgroup   CAPI2_STKAPIOLDGroup   SIM Toolkit
*   @ingroup    CAPI2_STKOLDGroup
*
*   @brief      This group defines the types and prototypes for the SIM toolkit API functions.
*	
****************************************************************************/

#ifndef _CAPI2_STK_API_OLD_H_
#define _CAPI2_STK_API_OLD_H_


/**
 * @addtogroup CAPI2_STKAPIOLDGroup
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
void CAPI2_SATK_GetCachedRootMenuPtr(UInt32 tid, UInt8 clientID);

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
void CAPI2_SATK_SendUserActivityEvent(UInt32 tid, UInt8 clientID);

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
void CAPI2_SATK_SendIdleScreenAvaiEvent(UInt32 tid, UInt8 clientID);

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
void CAPI2_SATK_SendLangSelectEvent(UInt32 tid, UInt8 clientID, UInt16 language);

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
void CAPI2_SATK_SendBrowserTermEvent(UInt32 tid, UInt8 clientID, Boolean user_term);


//***************************************************************************************
/**
	API function to send back the response for STK event. It could be used as simple a
	response or to select a menu item, etc.	
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) ID of the STK module (currently not used, set to 0)
	@param		clientId (in)
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
void CAPI2_SATKCmdResp(UInt32 tid, UInt8 clientID, UInt16 clientId, SATK_EVENTS_t toEvent, UInt8 result1, UInt8 result2, SATKString_t *inText, UInt8 menuID);


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
void CAPI2_SATKDataServCmdResp(UInt32 tid, UInt8 clientID, const StkCmdRespond_t* response);

//***************************************************************************************
/**
	API fucntion to send user response back to SIM.
    This is a new function similar to SATKCmdResp but passes a 16 bit result2 and raw SS facility IE.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) ID of the STK module (currently not used, set to 0)
	@param		clientId (in)
	@param		toEvent (in) event type of the STK event
	@param		result1 (in) result code for the event received
	@param		result2 (in) secondary result code for the event received 
	@param		*inText (in) string, any text string to be send back to SIMAP
	@param		menuID (in) menu ID to be selected from the menu list
    @param		ssFacIePtr (in) SS/USSD raw facility IE.
	@note
	If any of the argument is not applicable to the particular case, simply fill it with 0 or NULL.
	@n@b Responses 
	@n@b MsgType_t : ::MSG_SATK_SENDTERMINALRSP_RSP
	@n@b Result_t : 
	@n@b ResultData : Boolean
**/	
void CAPI2_SATKSendTerminalRsp(UInt32 tid, UInt8 clientID, UInt16 clientId, SATK_EVENTS_t toEvent, UInt8 result1, UInt16 result2, SATKString_t *inText, UInt8 menuID, UInt8 *ssFacIePtr);

/**
	This is the API function to set the SIM Application Toolkit Terminal Profile data. 
	The data passed in this function will take precedence over the Terminal Profile
	data saved in Sys Parm. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
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
void CAPI2_SATK_SetTermProfile(UInt32 tid, UInt8 clientID, const UInt8 *data, UInt8 length);

/**
	This is the API function to return the Terminal Response for STK Play Tone. 
	
	If STK Play Tone is internally processed in Broadcom STK platform, the client 
	shall call SATKCmdResp() to return the Terminal Response. If STK Play Tone is 
	handled by client, this function shall be called to return the Terminal Response. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		result (in) STK Play Tone result 

	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_STK_SEND_PLAYTONE_RES_RSP
	@n@b Result_t : 
	@n@b ResultData : None
**/
void CAPI2_SATK_SendPlayToneRes(UInt32 tid, UInt8 clientID, SATK_ResultCode_t result);

/**
	Send Terminal Response as a response to the STK Setup Call request.
	
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		result1 (in) result code
	@param		result2 (in) result code

	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_STK_SETUP_CALL_RES_RSP
	@n@b Result_t : 
	@n@b ResultData : None

**/
void CAPI2_SATK_SendSetupCallRes(UInt32 tid, UInt8 clientID, SATK_ResultCode_t result1, SATK_ResultCode2_t result2);

/**
	Send an Envelope command to the SIM.
	
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		data (in) pointer to raw envelope command data buffer
    @param      data_len (in) length of data buffer

	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_STK_SETUP_CALL_RES_RSP
	@n@b Result_t : 
	@n@b ResultData : None

**/
void CAPI2_SATK_SendEnvelopeCmdReq(UInt32 tid, UInt8 clientID, const UInt8 *data, UInt8 data_len);

/**
	Send an Terminal Response to the SIM.
	
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		data (in) pointer to raw terminal response data buffer
    @param      data_len (in) length of data buffer

	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_STK_TERMINAL_RESPONSE_RSP
	@n@b Result_t : 
	@n@b ResultData : None

**/
void CAPI2_SATK_SendTerminalRspReq(UInt32 tid, UInt8 clientID, const UInt8 *data, UInt8 data_len);

/**
	API function to Send Call Control (voice/fax/data/GPRS) request to SIMMAP.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
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
							   Boolean				simtk_orig);

/**
	API function to Send SS Call Control request to SIMMAP.
    @param		client_info (in) 
	@param		ton_npi (in) TON/NPI byte: one of UNKNOWN_TON_UNKNOWN_NPI (128), UNKNOWN_TON_ISDN_NPI (129), INTERNA_TON_ISDN_NPI (145) & SIM_RAW_EMPTY_VALUE(0xFF)
	@param		ss_len (in) Number of bytes in the passed "ss_data". 
	@param		ss_data (in) BCD encoded SS data 
	@note
	One of MSG_STK_CALL_CONTROL_SETUP_RSP, MSG_STK_CALL_CONTROL_SS_RSP, & MSG_STK_CALL_CONTROL_USSD_RSP will be 
	passed to the callback function to send back the response. 
**/	
void CAPI2_SATK_SendCcSsReq(ClientInfo_t* client_info, UInt8 ton_npi, UInt8 ss_len, const UInt8 *ss_data);

/**
	API function to Send USSD Call Control request to SIMMAP.
    @param		client_info (in)
	@param		ussd_dcs (in) USSD Data Coding Scheme
	@param		ussd_len (in) Number of bytes of USSD data 
	@param		ussd_data (in) USSD data 
	@note
	One of MSG_STK_CALL_CONTROL_SETUP_RSP, MSG_STK_CALL_CONTROL_SS_RSP, & MSG_STK_CALL_CONTROL_USSD_RSP will be 
	passed to the callback function to send back the response. 
**/	
void CAPI2_SATK_SendCcUssdReq(ClientInfo_t* client_info, UInt8 ussd_dcs, UInt8 ussd_len, const UInt8 *ussd_data);

/**
	API function to Send SMS Control request to SIMMAP.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
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
					    Boolean	simtk_orig );

void CAPI2_SatkApi_CmdResp(ClientInfo_t* client_info, SATK_EVENTS_t toEvent, UInt8 result1, UInt8 result2, SATKString_t *inText, UInt8 menuID);
void CAPI2_SatkApi_SendTerminalRsp(ClientInfo_t* client_info, SATK_EVENTS_t toEvent, UInt8 result1, UInt16 result2, SATKString_t *inText, UInt8 menuID, UInt8 *ssFacIePtr);
void CAPI2_SatkApi_SendCcSsReq(ClientInfo_t* client_info, UInt8 ton_npi, UInt8 ss_len, const UInt8 *ss_data);
void CAPI2_SatkApi_SendCcUssdReq(ClientInfo_t* client_info, UInt8 ussd_dcs, UInt8 ussd_len, const UInt8 *ussd_data);

/** @} */
#endif  // _STKAPI_H_

