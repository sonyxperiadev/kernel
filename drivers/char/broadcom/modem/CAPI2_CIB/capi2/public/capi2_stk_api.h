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
*   @file   capi2_stk_api.h
*
*   @brief  This file contains SATK API related defines.
*
****************************************************************************/
/**

*   @defgroup   CAPI2_STKAPIGroup   SIM Toolkit
*   @ingroup    CAPI2_STKGroup
*
*   @brief      This group defines the types and prototypes for the SIM toolkit API functions.
*	
****************************************************************************/

#ifndef _CAPI2_STK_API_H_
#define _CAPI2_STK_API_H_


/**
 * @addtogroup CAPI2_STKAPIGroup
 * @{
 */


//***************************************************************************************
/**
	API function to get cached root menu list.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SATK_GETCACHEDROOTMENUPTR_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SetupMenu_t
**/
void CAPI2_SatkApi_GetCachedRootMenuPtr(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function sends User Activity Event one of the features in STK Event Download to SIMMAP SIMMAP <br>will decide whether to send the event to SIM based upon the Event List stored in SIMMAP.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SATK_SENDUSERACTIVITYEVENT_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SatkApi_SendUserActivityEvent(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function sends Idle Screen Available Event one of the features in STK Event Download to SIMMAP SIMMAP <br>will decide whether to send the event to SIM based upon the Event List stored in SIMMAP.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SATK_SENDIDLESCREENAVAIEVENT_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SatkApi_SendIdleScreenAvaiEvent(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function sends Language Selection Event one of the features in STK Event Download to SIMMAP SIMMAP <br>will decide whether to send the event to SIM based upon the Event List stored in SIMMAP.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		language (in) Param is language
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SATK_SENDLANGSELECTEVENT_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SatkApi_SendLangSelectEvent(ClientInfo_t* inClientInfoPtr, UInt16 language);

//***************************************************************************************
/**
	This function sends sends Browser Termination Event one of the features in STK Event Download to SIMMAP SIMMAP <br>will decide whether to send the event to SIM based upon the Event List stored in SIMMAP.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		user_term (in)  TRUE if browser is terminated by user.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SATK_SENDBROWSERTERMEVENT_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SatkApi_SendBrowserTermEvent(ClientInfo_t* inClientInfoPtr, Boolean user_term);

//***************************************************************************************
/**
	API function to send back the response for STK event. It could be used as simple a <br>response or to select a menu item etc.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		toEvent (in)  event type of the STK event
	@param		result1 (in)  result code for the event received
	@param		result2 (in)  secondary result code for the event received
	@param		inText (in)  string
	@param		menuID (in)  menu ID to be selected from the menu list
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SATK_CMDRESP_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_SatkApi_CmdResp(ClientInfo_t* inClientInfoPtr, SATK_EVENTS_t toEvent, UInt8 result1, UInt8 result2, SATKString_t *inText, UInt8 menuID);

//***************************************************************************************
/**
	This is the API function to send user response back to SIM for "Open Channel" user confirmation.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		response (in)  terminal response data
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SATK_DATASERVCMDRESP_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_SatkApi_DataServCmdResp(ClientInfo_t* inClientInfoPtr, const StkCmdRespond_t *response);

//***************************************************************************************
/**
	This is the API fucntion to send DATA AVAILABLE CHANNEL STATUS or <br>RECEIVE DATA commands to SIM
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		dsReq (in)  request data
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SATK_SENDDATASERVREQ_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_SatkApi_SendDataServReq(ClientInfo_t* inClientInfoPtr, const StkDataServReq_t *dsReq);

//***************************************************************************************
/**
	API fucntion to send user response back to SIM. <br>This is a new function similar to SatkApi_CmdResp but passes a 16 bit result2 and raw SS facility IE.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		toEvent (in)  event type of the STK event
	@param		result1 (in)  result code for the event received
	@param		result2 (in)  secondary result code for the event received 
	@param		inText (in)  string
	@param		menuID (in)  menu ID to be selected from the menu list
	@param		ssFacIePtr (in)  SS/USSD raw facility IE.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SATK_SENDTERMINALRSP_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_SatkApi_SendTerminalRsp(ClientInfo_t* inClientInfoPtr, SATK_EVENTS_t toEvent, UInt8 result1, UInt16 result2, SATKString_t *inText, UInt8 menuID, UInt8 *ssFacIePtr);

//***************************************************************************************
/**
	This is the API function to set the SIM Application Toolkit Terminal Profile data. <br>The data passed in this function will take precedence over the Terminal Profile <br>data saved in Sys Parm.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		data (in)  Terminal Profile data
	@param		length (in)  length of the terminal profile data.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SATK_SETTERMPROFILE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SatkApi_SetTermProfile(ClientInfo_t* inClientInfoPtr, const UInt8 *data, UInt8 length);

//***************************************************************************************
/**
	Send an Envelope command to the SIM. This will result in a MSG_STK_ENVELOPE_RSP message to <br>be passed to the callback function to send back the response.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		data (in)  pointer to data encoded in GSM 11.14 format
	@param		data_len (in)  number of bytes passed in "data".
	
	 Responses 
	 @n@b MsgType_t: ::MSG_STK_ENVELOPE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::EnvelopeRspData_t
**/
void CAPI2_SatkApi_SendEnvelopeCmdReq(ClientInfo_t* inClientInfoPtr, const UInt8 *data, UInt8 data_len);

//***************************************************************************************
/**
	Send a Terminal Response command to the SIM. This will result in a MSG_STK_TERMINAL_RESPONSE_RSP message to <br>be passed to the callback function to send back the response.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		data (in)  pointer to data encoded in GSM 11.14 format
	@param		data_len (in)  number of bytes passed in "data".
	
	 Responses 
	 @n@b MsgType_t: ::MSG_STK_TERMINAL_RESPONSE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::TerminalResponseRspData_t
**/
void CAPI2_SatkApi_SendTerminalRspReq(ClientInfo_t* inClientInfoPtr, const UInt8 *data, UInt8 data_len);

//***************************************************************************************
/**
	This function sends Browsing Status Event to <br>USIMMAP USIMMAP will decide whether to send the event to <br>SIM based upon the Event List stored in USIMMAP.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		status (in)  Browsing status bytes received from NW
	@param		data_len (in)  length of browsing status bytes
	
	 Responses 
	 @n@b MsgType_t: ::MSG_STK_SEND_BROWSING_STATUS_EVT_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_StkApi_SendBrowsingStatusEvent(ClientInfo_t* inClientInfoPtr, UInt8 *status, UInt8 data_len);

//***************************************************************************************
/**
	API function to Send Call Control voice/fax/data/GPRS request to SIMMAP.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		ton (in)  TON of the number
	@param		npi (in)  NPI of the number
	@param		number (in)  ASCII encoded called party number 
	@param		bc1 (in)  bearer capability 1
	@param		subaddr_data (in)  sub address data
	@param		bc2 (in)  bearer capability 2
	@param		bc_repeat_ind (in)  bc repeat ind
	@param		simtk_orig (in)  is STK originated
	
	 Responses 
	 @n@b MsgType_t: ::MSG_STK_CALL_CONTROL_SETUP_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SatkApi_SendCcSetupReq(ClientInfo_t* inClientInfoPtr, TypeOfNumber_t ton, NumberPlanId_t npi, char *number, BearerCapability_t *bc1, Subaddress_t *subaddr_data, BearerCapability_t *bc2, UInt8 bc_repeat_ind, Boolean simtk_orig);

//***************************************************************************************
/**
	PI function to Send SS Call Control request to SIMMAP.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		ton_npi (in)  TON/NPI byte: one of UNKNOWN_TON_UNKNOWN_NPI 128
	@param		ss_len (in)  Number of bytes in the passed "ss_data".
	@param		ss_data (in)  BCD encoded SS data 
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SATK_SEND_CC_SS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SatkApi_SendCcSsReq(ClientInfo_t* inClientInfoPtr, UInt8 ton_npi, UInt8 ss_len, const UInt8 *ss_data);

//***************************************************************************************
/**
	API function to Send USSD Call Control request to SIMMAP.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		ussd_dcs (in)  USSD Data Coding Scheme
	@param		ussd_len (in)  Number of bytes of USSD data
	@param		ussd_data (in)  USSD data 
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SATK_SEND_CC_USSD_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SatkApi_SendCcUssdReq(ClientInfo_t* inClientInfoPtr, UInt8 ussd_dcs, UInt8 ussd_len, const UInt8 *ussd_data);

//***************************************************************************************
/**
	API function to Send SMS Control request to SIMMAP.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		sca_toa (in)  SMS service center type of address
	@param		sca_number_len (in)  SMS service center address length
	@param		sca_number (in)  SMS service center number BCD
	@param		dest_toa (in)  SMS destination type of address
	@param		dest_number_len (in)  SMS destination address length
	@param		dest_number (in)  SMS destination number BCD
	@param		simtk_orig (in)  is STK originated
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SATK_SEND_CC_SMS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SatkApi_SendCcSmsReq(ClientInfo_t* inClientInfoPtr, UInt8 sca_toa, UInt8 sca_number_len, UInt8 *sca_number, UInt8 dest_toa, UInt8 dest_number_len, UInt8 *dest_number, Boolean simtk_orig);

//***************************************************************************************
/**
	Function to enable/disable proactive command fetching. This will result in a <br>MSG_STK_PROACTIVE_CMD_FETCHING_ON_OFF_RSP message to be passed to the callback function <br>to send back the response.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		enableCmdFetching (in)  TRUE to enable/FALSE to disable
	
	 Responses 
	 @n@b MsgType_t: ::MSG_STK_PROACTIVE_CMD_FETCHING_ON_OFF_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::StkProactiveCmdFetchingOnOffRsp_t
**/
void CAPI2_SatkApi_SendProactiveCmdFetchingOnOffReq(ClientInfo_t* inClientInfoPtr, Boolean enableCmdFetching);

//***************************************************************************************
/**
	This function sends external proactive command to STK. STK will forward it to USIMAP.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		dataLen (in)  Number of bytes in the passed data.
	@param		ptrData (in)  external proactive command.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SATK_SEND_EXT_PROACTIVE_CMD_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SatkApi_SendExtProactiveCmdReq(ClientInfo_t* inClientInfoPtr, UInt8 dataLen, const UInt8 *ptrData);

//***************************************************************************************
/**
	This function sends terminal profile to USIMAP. The MSG_STK_TERMINAL_PROFILE_IND has the status from USIMAP.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		dataLen (in)  Number of bytes in the passed data.
	@param		ptrData (in)  terminal profile data.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SATK_SEND_TERMINAL_PROFILE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SatkApi_SendTerminalProfileReq(ClientInfo_t* inClientInfoPtr, UInt8 dataLen, const UInt8 *ptrData);

//***************************************************************************************
/**
	Function to set the proactive command polling interval. This will result in a <br>MSG_STK_POLLING_INTERVAL_RSP message to be passed to the callback function to send back the response.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		pollingInterval (in)  polling interval in seconds 0 means polling off
	
	 Responses 
	 @n@b MsgType_t: ::MSG_STK_POLLING_INTERVAL_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::StkPollingIntervalRsp_t
**/
void CAPI2_SatkApi_SendPollingIntervalReq(ClientInfo_t* inClientInfoPtr, UInt16 pollingInterval);

//***************************************************************************************
/**
	Function to SendPlayToneRes
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		resultCode (in) Param is resultCode
	
	 Responses 
	 @n@b MsgType_t: ::MSG_STK_SEND_PLAYTONE_RES_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SatkApi_SendPlayToneRes(ClientInfo_t* inClientInfoPtr, SATK_ResultCode_t resultCode);





/** @} */
#endif  // _STKAPI_H_

