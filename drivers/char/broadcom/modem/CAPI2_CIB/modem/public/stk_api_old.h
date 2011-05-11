//***************************************************************************
//
//	Copyright © 2004-2008 Broadcom Corporation
//	
//	This program is the proprietary software of Broadcom Corporation 
//	and/or its licensors, and may only be used, duplicated, modified 
//	or distributed pursuant to the terms and conditions of a separate, 
//	written license agreement executed between you and Broadcom (an 
//	"Authorized License").  Except as set forth in an Authorized 
//	License, Broadcom grants no license (express or implied), right 
//	to use, or waiver of any kind with respect to the Software, and 
//	Broadcom expressly reserves all rights in and to the Software and 
//	all intellectual property rights therein.  IF YOU HAVE NO 
//	AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE 
//	IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE 
//	ALL USE OF THE SOFTWARE.  
//	
//	Except as expressly set forth in the Authorized License,
//	
//	1.	This program, including its structure, sequence and 
//		organization, constitutes the valuable trade secrets 
//		of Broadcom, and you shall use all reasonable efforts 
//		to protect the confidentiality thereof, and to use 
//		this information only in connection with your use 
//		of Broadcom integrated circuit products.
//	
//	2.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE 
//		IS PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM 
//		MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, 
//		EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, 
//		WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY 
//		DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, 
//		MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A 
//		PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR 
//		COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR 
//		CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE 
//		RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.  
//
//	3.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT 
//		SHALL BROADCOM OR ITS LICENSORS BE LIABLE FOR 
//		(i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
//		EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY 
//		WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE 
//		SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE 
//		POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN 
//		EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE 
//		ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
//		LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE 
//		OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//***************************************************************************
/**
*
*   @file   stk_api_old.h
*
*   @brief  This file contains SIM Application ToolKit APIs.
*
*	Note:	The API functions in this file are obsoleted and will be removed in later release
*	
****************************************************************************/
#ifndef _STK_API_OLD_H_
#define _STK_API_OLD_H_

//**************************************************************************************
/**
	API function to get cached root menu list.

	@return Pointer to setup menu

**/

SetupMenu_t* SATK_GetCachedRootMenuPtr(void);


//**************************************************************************************
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
void SATK_SendCcSetupReq(	UInt8				clientID,
							TypeOfNumber_t		ton,
							NumberPlanId_t		npi,
							char*				number,
							BearerCapability_t*	bc1,
							Subaddress_t*		subaddr_data,
							BearerCapability_t*	bc2,
							UInt8				bc_repeat_ind,
							Boolean				simtk_orig);
							

//**************************************************************************************
/**
	API function to Send SS Call Control request to SIMMAP.
	@param		inClientInfoPtr (in) Information ID of the STK module
	@param		ton_npi (in) TON/NPI byte: one of UNKNOWN_TON_UNKNOWN_NPI (128), UNKNOWN_TON_ISDN_NPI (129), INTERNA_TON_ISDN_NPI (145) & SIM_RAW_EMPTY_VALUE(0xFF)
	@param		ss_len (in) Number of bytes in the passed "ss_data". 
	@param		ss_data (in) BCD encoded SS data 
	@note
	One of MSG_STK_CALL_CONTROL_SETUP_RSP, MSG_STK_CALL_CONTROL_SS_RSP, & MSG_STK_CALL_CONTROL_USSD_RSP will be 
	passed to the callback function to send back the response. 
**/	
void SATK_SendCcSsReq(	ClientInfo_t*	inClientInfoPtr,
						UInt8			ton_npi,
						UInt8			ss_len,
						const UInt8*	ss_data);
						

//**************************************************************************************
/**
	API function to Send USSD Call Control request to SIMMAP.
	@param		inClientInfoPtr (in) Information ID of the STK module
	@param		ussd_dcs (in) USSD Data Coding Scheme
	@param		ussd_len (in) Number of bytes of USSD data 
	@param		ussd_data (in) USSD data 
	@note
	One of MSG_STK_CALL_CONTROL_SETUP_RSP, MSG_STK_CALL_CONTROL_SS_RSP, & MSG_STK_CALL_CONTROL_USSD_RSP will be 
	passed to the callback function to send back the response. 
**/	
void SATK_SendCcUssdReq(ClientInfo_t*	inClientInfoPtr,
						UInt8			ussd_dcs,
						UInt8			ussd_len,
						const UInt8*	ussd_data);
						

//**************************************************************************************
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
void SATK_SendCcSmsReq( UInt8	clientID,
						UInt8	sca_toa,
						UInt8	sca_number_len,
						UInt8*	sca_number,
					    UInt8	dest_toa,
						UInt8	dest_number_len,
						UInt8*	dest_number,
					    Boolean	simtk_orig );
					    

//**************************************************************************************
/**
	Send an Envelope command to the SIM. This will result in a MSG_STK_ENVELOPE_RSP message to 
	be passed to the callback function to send back the response. 
	@param		client_id (in) ID of the STK module (currently not used, set to 0)
	@param		data (in) pointer to data encoded in GSM 11.14 format, excluding the command header (CLASS, INS, P1, P2 & P3 in section 9.2 of GSM 11.11).
	@param		data_len (in) number of bytes passed in "data".
	@param		sim_access_cb (in)	event callback function pointer
**/
Result_t SATK_SendEnvelopeCmdReq( UInt8 client_id, const UInt8 *data, UInt8 data_len, 
								  CallbackFunc_t* sim_access_cb );
								  

//**************************************************************************************
/**
	Send a Terminal Response command to the SIM. This will result in a MSG_STK_TERMINAL_RESPONSE_RSP message to 
	be passed to the callback function to send back the response. 
	@param		client_id (in) ID of the STK module (currently not used, set to 0)
	@param		data (in) pointer to data encoded in GSM 11.14 format, excluding the command header (CLASS, INS, P1, P2 & P3 in section 9.2 of GSM 11.11).
	@param		data_len (in) number of bytes passed in "data".
	@param		sim_access_cb (in)	event callback function pointer
**/
Result_t SATK_SendTerminalRspReq( UInt8 client_id, const UInt8 *data, UInt8 data_len, 
								  CallbackFunc_t* sim_access_cb );
								  

//**************************************************************************************
/**
	Send Terminal Response as a response to the STK Setup Call request.
	@param		result1 (in) general result code
    @param      result2 (in) additional result code
**/
void SATK_SendSetupCallRes(SATK_ResultCode_t result1, SATK_ResultCode2_t result2);


//**************************************************************************************
/**
	This function sends User Activity Event (one of the features in STK Event Download) to SIMMAP (SIMMAP 
	will decide whether to send the event to SIM based upon the Event List stored in SIMMAP).
**/	
void SATK_SendUserActivityEvent(void);


//**************************************************************************************
/**
	This function sends Idle Screen Available Event (one of the features in STK Event Download) to SIMMAP (SIMMAP 
	will decide whether to send the event to SIM based upon the Event List stored in SIMMAP). 
**/	
void SATK_SendIdleScreenAvaiEvent(void);


//**************************************************************************************
/**
	This function sends Language Selection Event (one of the features in STK Event Download) to SIMMAP (SIMMAP 
	will decide whether to send the event to SIM based upon the Event List stored in SIMMAP). 
	@param		language (in) language code of a pair of alpha-numeric characters, as defined in ISO 639. For the 
				example of English ("en"), pass language = 0x656E.
**/	
void SATK_SendLangSelectEvent(UInt16 language);


//**************************************************************************************
/**
	This function sends sends Browser Termination Event (one of the features in STK Event Download) to SIMMAP (SIMMAP 
	will decide whether to send the event to SIM based upon the Event List stored in SIMMAP). 
	@param		user_term (in) TRUE if browser is terminated by user. FALSE if browser is terminated because of 
				error. See Section 11.9.2 and  12.51 in ETSI TS 101 267 (GSM 11.14).
**/	
void SATK_SendBrowserTermEvent(Boolean user_term);


//***************************************************************************************
/**
	API fucntion to send user response back to SIM.
    This is a new function similar to SATKCmdResp but passes a 16 bit result2 and raw SS facility IE.	
	@param		clientID (in) ID of the STK module (currently not used, set to 0)
	@param		toEvent (in) event type of the STK event
	@param		result1 (in) result code for the event received
	@param		result2 (in) secondary result code for the event received 
	@param		*inText (in) string, any text string to be send back to SIMAP
	@param		menuID (in) menu ID to be selected from the menu list
    @param		ssFacIePtr (in) SS/USSD raw facility IE.

	@note
    For Provide Local Info: Date & Time Terminal Response:
         event = SATK_EVENT_PROV_LOCAL_DATE
         inText.string = Date & time info in the format of [YY MM DD HR MN SC TZ]
         TZ is measured against GMT. It has a postive or negative value in increments of 15 minutes
         0xFF = unknown timezone
         For example May 07, 2012, 14:08, CET = [0B 05 07 0D 08 00 04] 

	If any of the argument is not applicable to the particular case, simply fill it with 0 or NULL.
**/	
Boolean SATKSendTerminalRsp(UInt16 clientID, SATK_EVENTS_t toEvent, UInt8 result1, 
				UInt16 result2, SATKString_t* inText, UInt8 menuID, UInt8 *ssFacIePtr);
				

//***************************************************************************************
/**
	API function to send back the response for STK event. It could be used as simple a
	response or to select a menu item, etc.	
	@param		clientID (in) ID of the STK module (currently not used, set to 0)
	@param		toEvent (in) event type of the STK event
	@param		result1 (in) result code for the event received
	@param		result2 (in) secondary result code for the event received 
	@param		*inText (in) string, any text string to be send back to SIMAP
	@param		menuID (in) menu ID to be selected from the menu list
	@note
	If any of the argument is not applicable to the particular case, simply fill it with 0 or NULL.
**/	
Boolean SATKCmdResp(UInt16 clientID, SATK_EVENTS_t toEvent, UInt8 result1, 
				UInt8 result2, SATKString_t* inText, UInt8 menuID);
				

/**
	This is the API function to send user response back to SIM for "Open Channel" user confirmation. 
	@param		response (in) terminal response data
	@return
		- TRUE if user confirmation is accepted;
		- FALSE if error encountered.
    @note:
	@code
    For Open Channel Terminal Response:
        event = SATK_EVENT_OPEN_CHAN
        channelID = Channel ID;
        result1 = General Result
        result2 = Additional Result
        dataLen = Buffer Size               
        dataBuf = pointer to Bearer Data buffer
        bearerDataLen = Bearer Data Len
        bearerType = Bearer Type [GPRS = 2]
        chanStatus = Channel Status for UICC Server Mode

    For Send Data Terminal Response
        event = SATK_EVENT_SEND_DATA
        channelID = Channel ID
        result1 = General Result
        result2 = Additional Result
        dataLen = number of empty bytes

    For Receive Data Terminal Response
        event = SATK_EVENT_RECEIVE_DATA
        channelID = Channel ID
        result1 = General Result
        result2 = Additional Result
        numbytesAvail = number of bytes available        
        dataLen = received data len (should not exceed 237 bytes, otherwise it will assert)
        dataBuf = pointer to received data buffer

    For Close Channel Terminal Response
        event = SATK_EVENT_CLOSE_CHAN
        channelID = Channel ID
        result1 = General Result
        result2 = Additional Result

	@endcode
**/
Boolean SATKDataServCmdResp(const StkCmdRespond_t* response);

//******************************************************************************
/**
     This is the API fucntion to send DATA AVAILABLE, CHANNEL STATUS or 
     RECEIVE DATA commands to SIM 
    @param      clientID (in) Client ID
	@param		req (in) request data
	@return
		- TRUE if user confirmation is accepted;
		- FALSE if error encountered.
    @note:
	@code
    For Channel Status event download:
        event = SATK_EVENT_CHAN_STATUS_UPDATE
        linkEstablished = TRUE/FALSE
        chanState = SATK_CHAN_STATE_NO_INFO/SATK_CHAN_STATE_CHANGED_TO_DROPPED

    For Data Available event download:
        event = SATK_EVENT_DATA_AVAILABLE
        numBytesAvail = number of available bytes
	@endcode
**/
Boolean SATKSendDataServReq(UInt8 clientID, const StkDataServReq_t* req);


//**************************************************************************************
/**
	This is the API function to set the SIM Application Toolkit Terminal Profile data. 
	The data passed in this function will take precedence over the Terminal Profile
	data saved in Sys Parm. 

	@param		data (in) Terminal Profile data
	@param		length (in) length of the terminal profile data.

	@note
	The value of length must not be larger than MAX_TERMINAL_PROFILE_ARRAY_SIZE. 

	This function needs to be called before the system is powered up, e.g. before 
	SYS_ProcessPowerUpReq() or SYS_ProcessNoRfReq() is called. 
**/
void SATK_SetTermProfile(const UInt8 *data, UInt8 length);


//**************************************************************************************
/**
	This is the API function to return the SIM Application Toolkit Terminal Profile data. 
	
	If the data has been set by SATK_SetTermProfile() before, it returns that data. Otherwise
	it returns the Terminal Profile data in Sys Parm. 

	@param		length (out) Length of Terminal Profile data returned. 

	@return		Pointer to the Terminal Profile data. 
**/
const UInt8*  SATK_GetTermProfile(UInt8 *length);


//**************************************************************************************
/**
	This is the API function to return the Terminal Response for STK Play Tone. 
	
	If STK Play Tone is internally processed in Broadcom STK platform, the client 
	shall call SATKCmdResp() to return the Terminal Response. If STK Play Tone is 
	handled by client, this function shall be called to return the Terminal Response. 

	@param		result (in) STK Play Tone result 

	@return		None
**/
void SATK_SendPlayToneRes(SATK_ResultCode_t result);

#endif  // _STKAPI_OLD_H_

