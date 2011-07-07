//***************************************************************************
//
//	Copyright ?2004-2008 Broadcom Corporation
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
*   @file   stk_api.h
*
*   @brief  This file contains SIM Application ToolKit APIs.
*
****************************************************************************/
/**

*   @defgroup   STKAPIGroup   SIM Toolkit
*   @ingroup    SIMSTK
*
*   @brief      This group defines prototypes for the SIM Application toolkit API functions.
*	
*	\n Use the link below to navigate back to the SIM Application Toolkit Overview page. \n
    \li \if CAPI2
	\ref CAPI2SATKOverview
	\endif
	\if CAPI
	\ref STKOverview
	\endif

****************************************************************************/
/**

*   @defgroup   STKOLDAPIGroup   Legacy SIM Toolkit
*   @ingroup    STKAPIGroup
*
*   @brief      This group defines legacy prototypes for the SIM Application toolkit API functions.
*	
*	\n Use the link below to navigate back to the SIM Application Toolkit Overview page. \n
    \li \if CAPI2
	\ref CAPI2SATKOverview
	\endif
	\if CAPI
	\ref STKOverview
	\endif
****************************************************************************/
#ifndef _STK_API_H_
#define _STK_API_H_


/**
 * @addtogroup STKOLDAPIGroup
 * @{
 */


//**************************************************************************************
/**
    This function processes saved emergency call after STK DS session
    is terminated

  	@param inClientInfoPtr (in) Information ID of the STK module
    @param isReady         (in) TRUE if ready to process emergency call

**/
void StkApi_EmergencyCallRsp(ClientInfo_t* inClientInfoPtr, Boolean isReady);

/** @} */


/////////////////////////////////////////////////////////////////////////////////////////
//
//  Below are new SMS API with ClientInfo_t, 
//
//  Note this is NOT added in the doxygen groups yet.
//

/**
 * @addtogroup STKAPIGroup
 * @{
 */

/**
	API function to get cached root menu list.

    @param		*inClientInfoPtr (in) ClientInfo_t pointer
	@return Pointer to setup menu

**/
SetupMenu_t* SatkApi_GetCachedRootMenuPtr(ClientInfo_t* inClientInfoPtr);

/**
	This function stops Proactive Setup Call retry timer and sends Terminal Response to SIM
    @param		inClientInfoPtr (in) Information ID of the STK module
	@param		resultCode (in) Proactive Setup Call result code
	@return		none
**/	
void SatkApi_HandleStopCallSetupRetryTimerReq(ClientInfo_t* inClientInfoPtr, SATK_ResultCode_t resultCode);

//***************************************************************************************
/**
	API fucntion to send user response back to SIM.
    This is a new function similar to SatkApi_CmdResp but passes a 16 bit result2 and raw SS facility IE.	
	@param		inClientInfoPtr (in) Information ID of the STK module
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
Boolean SatkApi_SendTerminalRsp(ClientInfo_t* inClientInfoPtr, SATK_EVENTS_t toEvent, UInt8 result1,
				UInt16 result2, SATKString_t* inText, UInt8 menuID, UInt8 *ssFacIePtr);

//***************************************************************************************
/**
	API function to send back the response for STK event. It could be used as simple a
	response or to select a menu item, etc.	
	@param		inClientInfoPtr (in) Information ID of the STK module
	@param		toEvent (in) event type of the STK event
	@param		result1 (in) result code for the event received
	@param		result2 (in) secondary result code for the event received 
	@param		*inText (in) string, any text string to be send back to SIMAP
	@param		menuID (in) menu ID to be selected from the menu list
	@return		Boolean
	@note
	If any of the argument is not applicable to the particular case, simply fill it with 0 or NULL.
**/	
Boolean SatkApi_CmdResp(ClientInfo_t* inClientInfoPtr, SATK_EVENTS_t toEvent, UInt8 result1, 
				UInt8 result2, SATKString_t* inText, UInt8 menuID);

/**
	Handles the Terminal Response for SS or USSD if they are converted from STK Call Setup due to STK Call Control
    @param		inClientInfoPtr (in) Information ID of the STK module
	@param		result (in) result1 code
**/	
void SatkApi_TermRespForSsUssdFromSTKCall(ClientInfo_t* inClientInfoPtr, Boolean result);

/**
	This is the API function to send user response back to SIM for "Open Channel" user confirmation.
    @param		inClientInfoPtr (in) Information ID of the STK module
	@param		response (in) terminal response data
	@return
		- TRUE if user confirmation is accepted;
		- FALSE if error encountered.
    @note:
	@code
    For Open Channel Terminal Response:
        event = SATK_EVENT_OPEN_CHAN
        channelID = Channel ID;
        dataLen = Buffer Size               
        result2 = Bearer Data Len
        textStr1[0] = Bearer Type [GPRS = 2]
        dataBuf = pointer to Bearer Data buffer

    For Send Data Terminal Response
        event = SATK_EVENT_SEND_DATA
        channelID = Channel ID
        dataLen = number of empty bytes

    For Receive Data Terminal Response
        event = SATK_EVENT_RECEIVE_DATA
        channelID = Channel ID
        result2 = number of bytes available
        dataLen = received data len
        dataBuf = pointer to received data buffer

    For Close Channel Terminal Response
        event = SATK_EVENT_CLOSE_CHAN
        channelID = Channel ID
	@endcode
**/
Boolean SatkApi_DataServCmdResp(ClientInfo_t* inClientInfoPtr, const StkCmdRespond_t* response);

//******************************************************************************
/**
     This is the API fucntion to send DATA AVAILABLE, CHANNEL STATUS or 
     RECEIVE DATA commands to SIM 
    @param		inClientInfoPtr (in) Information ID of the STK module
	@param		req (in) request data
	@return
		- TRUE if user confirmation is accepted;
		- FALSE if error encountered.
**/
Boolean SatkApi_SendDataServReq(ClientInfo_t* inClientInfoPtr, const StkDataServReq_t* req);

/**
	This is the API function to set the SIM Application Toolkit Terminal Profile data. 
	The data passed in this function will take precedence over the Terminal Profile
	data saved in Sys Parm. 

    @param		inClientInfoPtr (in) Information ID of the STK module
	@param		data (in) Terminal Profile data
	@param		length (in) length of the terminal profile data.
	@return		none	
	@note
	The value of length must not be larger than MAX_TERMINAL_PROFILE_ARRAY_SIZE. 

	This function needs to be called before the system is powered up, e.g. before 
	SYS_ProcessPowerUpReq() or SYS_ProcessNoRfReq() is called. 
**/
void SatkApi_SetTermProfile(ClientInfo_t* inClientInfoPtr, const UInt8 *data, UInt8 length);


/**
	This is the API function to return the SIM Application Toolkit Terminal Profile data. 
	
	If the data has been set by SATK_SetTermProfile() before, it returns that data. Otherwise
	it returns the Terminal Profile data in Sys Parm. 

    @param		inClientInfoPtr (in) Information ID of the STK module
	@param		length (out) Length of Terminal Profile data returned. 

	@return		Pointer to the Terminal Profile data. 
**/
const UInt8*  SatkApi_GetTermProfile(ClientInfo_t* inClientInfoPtr, UInt8 *length);

/**
	API function to Send Call Control (voice/fax/data/GPRS) request to SIMMAP.
	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		ton (in) TON of the number
	@param		npi (in) NPI of the number
	@param		number (in) ASCII encoded called party number 
	@param		*bc1 (in) bearer capability 1
	@param		subaddr_data (in) sub address data
	@param		*bc2 (in) bearer capability 2
	@param		bc_repeat_ind (in) bc repeat ind
	@param		simtk_orig (in) is STK originated
	@return		none
	@note
	One of MSG_STK_CALL_CONTROL_SETUP_RSP, MSG_STK_CALL_CONTROL_SS_RSP, & MSG_STK_CALL_CONTROL_USSD_RSP will be 
	passed to the callback function to send back the response. 
**/	
void SatkApi_SendCcSetupReq(ClientInfo_t* inClientInfoPtr,
							TypeOfNumber_t		ton,
							NumberPlanId_t		npi,
							char*				number,
							BearerCapability_t*	bc1,
							Subaddress_t*		subaddr_data,
							BearerCapability_t*	bc2,
							UInt8				bc_repeat_ind,
							Boolean				simtk_orig);

/**
	API function to Send SMS Control request to SIMMAP.
	@param		inClientInfoPtr (in) Information ID of the STK module
	@param		sca_toa (in) SMS service center type of address
	@param		sca_number_len (in) SMS service center address length
	@param		sca_number (in) SMS service center number (BCD)
	@param		dest_toa (in) SMS destination type of address
	@param		dest_number_len (in) SMS destination address length
	@param		dest_number (in) SMS destination number (BCD)
	@param		simtk_orig (in) is STK originated
	@return		none
	@note
	MSG_STK_CALL_CONTROL_SMS_RSP will be passed to the callback function to send back the response. 
**/	
void SatkApi_SendCcSmsReq(ClientInfo_t* inClientInfoPtr,
						UInt8	sca_toa,
						UInt8	sca_number_len,
						UInt8*	sca_number,
					    UInt8	dest_toa,
						UInt8	dest_number_len,
						UInt8*	dest_number,
					    Boolean	simtk_orig );

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
void SatkApi_SendCcSsReq(	ClientInfo_t*	inClientInfoPtr,
							UInt8			ton_npi,
							UInt8			ss_len,
							const UInt8*	ss_data);

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
void SatkApi_SendCcUssdReq(	ClientInfo_t*	inClientInfoPtr,
							UInt8			ussd_dcs,
							UInt8			ussd_len,
							const UInt8*	ussd_data);

/**
	Send an Envelope command to the SIM. This will result in a MSG_STK_ENVELOPE_RSP message to 
	be passed to the callback function to send back the response. 
	@param		inClientInfoPtr (in) Information ID of the STK module
	@param		data (in) pointer to data encoded in GSM 11.14 format, excluding the command header (CLASS, INS, P1, P2 & P3 in section 9.2 of GSM 11.11).
	@param		data_len (in) number of bytes passed in "data".
	@param		sim_access_cb (in)	event callback function pointer
	@return		result code
**/
Result_t SatkApi_SendEnvelopeCmdReq(ClientInfo_t* inClientInfoPtr, const UInt8 *data, UInt8 data_len, 
								  CallbackFunc_t* sim_access_cb );

/**
	Send a Terminal Response command to the SIM. This will result in a MSG_STK_TERMINAL_RESPONSE_RSP message to 
	be passed to the callback function to send back the response. 
	@param		inClientInfoPtr (in) Information ID of the STK module
	@param		data (in) pointer to data encoded in GSM 11.14 format, excluding the command header (CLASS, INS, P1, P2 & P3 in section 9.2 of GSM 11.11).
	@param		data_len (in) number of bytes passed in "data".
	@param		sim_access_cb (in)	event callback function pointer
	@return		result code
**/
Result_t SatkApi_SendTerminalRspReq(ClientInfo_t* inClientInfoPtr, const UInt8 *data, UInt8 data_len, 
								  CallbackFunc_t* sim_access_cb );

/**
	This function sends MT Call Event (one of the features in STK Event Download) to SIMMAP (SIMMAP will decide whether
	to send the event to SIM based upon the Event List stored in SIMMAP).
    @param		inClientInfoPtr (in) Information ID of the STK module
	@param		ton (in) TON of the number
	@param		npi (in) NPI of the number
	@param		number (in) ASCII encoded called party number 
	@param		subaddr_data (in) Called subadress data 
	@param		ti_value (in) TI value
	@return		none
**/	
void SatkApi_SendMtCallEvent(ClientInfo_t* inClientInfoPtr, gsm_TON_t ton, gsm_NPI_t npi, char *number,
						   Subaddress_t *subaddr_data, UInt8 ti_value );

/**
	This function sends Call Connected Event (one of the features in STK Event Download) to SIMMAP (SIMMAP will 
	decide whether to send the event to SIM based upon the Event List stored in SIMMAP).
    @param		inClientInfoPtr (in) Information ID of the STK module
	@param		mo_call (in) TRUE for mobile originated call
	@param		ti_value (in) TI value
	@return		none
**/	
void SatkApi_SendCallConnectedEvent(ClientInfo_t* inClientInfoPtr, Boolean mo_call, UInt8 ti_value);

/**
	This function sends Call Disconnected Event (one of the features in STK Event Download) to SIMMAP 
	(SIMMAP will decide whether to send the event to SIM based upon the Event List stored in SIMMAP).
    @param		inClientInfoPtr (in) Information ID of the STK module
	@param		mo_call (in) TRUE for mobile originated call
	@param		net_disconn (in) TRUE if network initiates disconnect; FALSE if ME initiates
				disconnect or radio link fails. 
	@param		ti_value (in) TI value 
	@param		mn_cause (in) MN layer cause
	@return		none
**/	
void SatkApi_SendCallDisconnectedEvent(ClientInfo_t* inClientInfoPtr, Boolean mo_call, Boolean net_disconn, 
                                       UInt8 ti_value, Cause_t mn_cause);

/**
	This function sends Location Status Event (one of the features in STK Event Download) to SIMMAP (SIMMAP 
	will decide whether to send the event to SIM based upon the Event List stored in SIMMAP).
    @param		inClientInfoPtr (in) Information ID of the STK module
	@param		status (in) registration status received from network
	@param		mcc (in) MCC value which may contain the third digit of MNC. For example of
				Cingular Wireless, the passed MCC shall be 0x13F0.
	@param		mnc (in) MNC value. For Cingular Wireless in Sunnyvale, CA, the passed MNC shall be 0x71.
	@param		lac (in) Location area code
	@param		cell_id (in) Cell ID
	@param		rnc_id (in) RNC ID (Radio Network Controller for 3G, ignored for 2G)
	@param		access_technology (in) Radio Access Technology (UMTS, GSM, etc.)
	@return		none
	@note		"mcc", "mnc", "lac" and "cell_id" will be ignored if the passed "status" is not REGISTERSTATUS_NORMAL.
**/	
void SatkApi_SendLocationStatusEvent(ClientInfo_t* inClientInfoPtr,
                                     RegisterStatus_t	status,
								     UInt16			mcc,
								     UInt8			mnc,
								     UInt16			lac,
								     UInt16			cell_id,
								     UInt16			rnc_id,
								     T_RAT			access_technology);

/**
	This function sends User Activity Event (one of the features in STK Event Download) to SIMMAP (SIMMAP 
	will decide whether to send the event to SIM based upon the Event List stored in SIMMAP).
    @param		inClientInfoPtr (in) Information ID of the STK module
**/	
void SatkApi_SendUserActivityEvent(ClientInfo_t* inClientInfoPtr);

/**
	This function sends Idle Screen Available Event (one of the features in STK Event Download) to SIMMAP (SIMMAP 
	will decide whether to send the event to SIM based upon the Event List stored in SIMMAP).
    @param		inClientInfoPtr (in) Information ID of the STK module
**/	
void SatkApi_SendIdleScreenAvaiEvent(ClientInfo_t* inClientInfoPtr);

/**
	This function sends Language Selection Event (one of the features in STK Event Download) to SIMMAP (SIMMAP 
	will decide whether to send the event to SIM based upon the Event List stored in SIMMAP). 
    @param		inClientInfoPtr (in) Information ID of the STK module
	@param		language (in) language code of a pair of alpha-numeric characters, as defined in ISO 639. For the 
				example of English ("en"), pass language = 0x656E.
	@return		none
**/	
void SatkApi_SendLangSelectEvent(ClientInfo_t* inClientInfoPtr, UInt16 language);

/**
	This function sends sends Browser Termination Event (one of the features in STK Event Download) to SIMMAP (SIMMAP 
	will decide whether to send the event to SIM based upon the Event List stored in SIMMAP). 
    @param		inClientInfoPtr (in) Information ID of the STK module
	@param		user_term (in) TRUE if browser is terminated by user. FALSE if browser is terminated because of 
				error. See Section 11.9.2 and  12.51 in ETSI TS 101 267 (GSM 11.14).
    @return		none
**/	
void SatkApi_SendBrowserTermEvent(ClientInfo_t* inClientInfoPtr, Boolean user_term);

/**
	This function determines whether the IMSI detach-then-reattach procedure needs to be performed. 
    @param		inClientInfoPtr (in) Information ID of the STK module
	@param		refresh_req - Refresh information
    @param      loci_refreshed - indicate whether EF-LOCI is refreshed
**/	
Boolean SatkApi_IsRefreshReattachNeeded(ClientInfo_t* inClientInfoPtr, const Refresh_t *refresh_req, Boolean *loci_refreshed);

void SatkApi_SendPlayToneRes(ClientInfo_t* inClientInfoPtr, SATK_ResultCode_t result);

/**
	This function sends terminal profile to USIMAP. The MSG_STK_TERMINAL_PROFILE_IND has the status from USIMAP.
	@param		inClientInfoPtr (in) Information ID of the STK module.
	@param		dataLen (in) Number of bytes in the passed data. 
	@param		ptrData (in) terminal profile data. 
	@return		result code
**/	
Result_t SatkApi_SendTerminalProfileReq(ClientInfo_t *inClientInfoPtr,
                                        UInt8 dataLen, const UInt8 *ptrData);

/**
	This function sends external proactive command to STK. STK will forward it to USIMAP.
	@param		inClientInfoPtr (in) Information ID of the SIM module.
	@param		dataLen (in) Number of bytes in the passed data. 
	@param		ptrData (in) external proactive command.
**/	
Result_t SatkApi_SendExtProactiveCmdReq(ClientInfo_t *inClientInfoPtr,
                                        UInt8 dataLen, const UInt8 *ptrData);

/**
	Function to enable/disable proactive command fetching. This will result in a 
    MSG_STK_PROACTIVE_CMD_FETCHING_ON_OFF_RSP message to be passed to the callback function
    to send back the response. 

	@param		inClientInfoPtr   (in) Information ID of the SIM module.
	@param		enableCmdFetching (in) TRUE to enable/FALSE to disable  
	@param		simAccessCb       (in) callback  
**/	

Result_t SatkApi_SendProactiveCmdFetchingOnOffReq(ClientInfo_t*   inClientInfoPtr,
                                                  Boolean         enableCmdFetching,
                                                  CallbackFunc_t* simAccessCb);

/**
	Function to set the proactive command polling interval. This will result in a
    MSG_STK_POLLING_INTERVAL_RSP message to be passed to the callback function to send back the response. 

	@param		inClientInfoPtr   (in) Information ID of the SIM module.
	@param		pollingInterval   (in) polling interval in seconds (0 means polling off) 
	@param		simAccessCb       (in) callback  
**/	
Result_t SatkApi_SendPollingIntervalReq(ClientInfo_t*   inClientInfoPtr,
                                        UInt16          pollingInterval,
                                        CallbackFunc_t* simAccessCb);

//**************************************************************************************
/**

    This function sends Browsing Status Event to 
    USIMMAP (USIMMAP will decide whether to send the event to 
    SIM based upon the Event List stored in USIMMAP).

      @param	inClientInfoPtr   (in) Information ID of the SIM module.
      @param    status (in) Browsing status bytes received from NW
      @param    len (in) number of browsing status bytes    
**/
void StkApi_SendBrowsingStatusEvent(ClientInfo_t* inClientInfoPtr, UInt8 *status, UInt8 len);


//**************************************************************************************
/**
    This function sends Network Search Mode Change Event to 
    USIMMAP (USIMMAP will decide whether to send the event to 
    SIM based upon the Event List stored in USIMMAP). 

    @param	   inClientInfoPtr   (in) Information ID of the SIM module.
    @param     plmnMode (in) new PLMN search Mode (Manual/Automatic)

**/
void StkApi_SendNwSearchModeChgEvent(ClientInfo_t* inClientInfoPtr, PlmnSelectMode_t plmnMode);
/** @} */

#endif  // _STKAPI_H_

