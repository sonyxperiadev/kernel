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
*   @file   stk.h
*
*   @brief  This file contains STK related definitions
*
****************************************************************************/

#ifndef _STK_STK_H_ 
#define _STK_STK_H_


/** @{ */

#define MAX_AT_REQUEST		512
#define MAX_AT_RESPONSE		256

#define STK_LOCAL_DATE_TIME_LEN 7
//****************************
// Define exportable typedefs 
//****************************
// for call routing
typedef enum
{
	O_MMI = 0,		///< from or to MMI
	O_ATC = 1,		///< from or to ATC
	O_SIM = 2,		///< from or to SIMAP
	O_UNKNOWN = 9
} CallOrigin_t;

typedef enum
{
	STK_CALLTYPE_SS = 0,	///< Call type supplementary service
	STK_CALLTYPE_SPEECH = 1, ///< Speech Call
	STK_CALLTYPE_UNKNOWN = 9 ///< Unknown call type
} StkCallType_t;

/// USSD result event type
typedef struct {
	UInt8	result;	///<	SUCCESS	= 1; FAIL = 0	///< opposite to output from simap
} SsUssdResult_t;

/// STK Call control event type
typedef struct {
	Boolean 				isAlphaIdProvided;	///< is AlphaID provided
	SATKCallControlType_t type;					///< call control type
	SATKString_t 			alphaText;			///< AlphaID string
} SATKCallControlEvent_t;

/// STK call connected event type
typedef struct
{
	Boolean is_mo_call;		///< is MO call
	T_TI_PD ti_pd;			///< ti_pd
} StkCallConnectedEvt_t;


/// STK data call setup event type
typedef struct {
	ProCallState_t	proCallState;			///< Proactive call setup state
	Boolean			isEmerCall;				///< is emergency call
	SATKCallType_t	callType;				///< call type
	SATKNum_t		num;					///< destination number
	UInt32			duration;				///< retry duation in milisec
	Timer_t			stopRetryTimer;			///< Timer handle
	UInt8			retryCount;				///< retry count
	Boolean			isIconNotDisplayed;		///< is icon not displayed
	Boolean			isDiscOrHoldForStkCall;	///< is disconnectable or holdable for STK call setup
    Subaddress_t    subAddr;                ///< Called Party Sub-Address: First byte contains Type of Sub-Address
    CC_BearerCap_t  bc;                     ///< Capability Configuration
} SetupCallData_t;

/// STK Send CC Setup Request info type
typedef struct
{    
    TypeOfNumber_t		ton;
	NumberPlanId_t		npi;
	char*				number;
	BearerCapability_t*	bc1;
	Subaddress_t*		subaddr_data;
	BearerCapability_t*	bc2;
	UInt8				bc_repeat_ind;
	Boolean				simtk_orig;    
} SATK_SendCcSetupReq_t;



/// STK Send CC SS Request info type
typedef struct
{    
    UInt8			ton_npi;
    UInt8			ss_len;
	const UInt8*	ss_data;    
} SATK_SendCcSsReq_t;


/// STK Send CC USSD Request info type
typedef struct
{    
    UInt8		    ussd_dcs;
    UInt8			ussd_len;
	const UInt8*	ussd_data;  
} SATK_SendCcUssdReq_t;


/// STK Send CC SMS Request info type
typedef struct
{    
    UInt8       sca_toa;
    UInt8       sca_number_len;
    UInt8*      sca_number;
    UInt8       dest_toa;
    UInt8       dest_number_len;
    UInt8*      dest_number;
    Boolean     simtk_orig;  
} SATK_SendCcSmsReq_t;


/// STK Send Envelope Command Request info type
typedef struct
{    
    const UInt8*    data;
    UInt8           data_len;
    CallbackFunc_t* sim_access_cb;
} SATK_SendEnvelopCmdReq_t;


/// STK Send Terminal Response Request info type
typedef struct
{    
    const UInt8*    data;
    UInt8           data_len;
    CallbackFunc_t* sim_access_cb;
} SATK_SendTerminalRspReq_t;



/// STK Browsing Status event info type
typedef struct
{
    UInt8 status[MAX_STK_BROWSING_STATUS_LEN];  ///< Browsing status bytes
    UInt8 len;                                  ///< Number of browsing status bytes
} StkBrowsingStatus_t;



//******************************************************************************
// Define STK messages types
//******************************************************************************

/**
STK send emergency call to network
**/
typedef struct {						///< Callback to request emergency connect.
	CallOrigin_t call_source;			///<  source of call initiated
	UInt8 *number;						///< Phone number (ascii null-term. string)
	}StkSendEmergencyCallReq_t;

/**
STK report PLMN list
**/
typedef struct {						///< Notify MMI of new/first PLMN list
	PLMNId_t *plmn_list; 				///< PLMN list ,jdn
	UInt8 size;							///< List size
	}StkReportPLMNList_t;

/**
STK report registration status
**/
typedef struct {						///< Notify MMI of registration status
	RegisterStatus_t status;			///< Registration status
	PLMNId_t	plmn_id;				///< valid when status != NO_SOURCE
	}StkReportRegisterStatus_t;

/**
STK Call Received data
**/
typedef struct {						///< Notification of call received
	CallIndex_t index;					///< Call identification
	UInt8 *number;						///< Phone number (ascii null-term. string), jdn
	CUGIndex_t cug_index;
	}StkReportCallReceived_t;

/**
STK call connected 
**/
typedef struct {						 ///< Notification of connected line id
	CallIndex_t index;					 ///< Call Index, identifying call
	COLPPresentStatus_t present_status;	 ///< connected line id presented status
	TelephoneNumber_t *number;			 ///< telephone number
	}StkReportCallConnectedID_t;

/**
STK Call action result
**/
typedef struct {						///< Notification of call action status
	CallIndex_t index;					///< Call identification 
	CallAction_t action;				///< Call action
	CallActionStatus_t result;			///< Call action result
	}StkReportCallActionResult_t;

/**
STK Call release type
**/
typedef struct {						///< Notification of call release
	CallIndex_t index;					///< Call Index, identifying call
	Cause_t cause;						///< Cause for the call release
	Duration_t duration;				///< Duration of the call
	CallMeterUnit_t CCM_units;			///< Current Call Meter Units	
	}StkReportCallRelease_t;

/**
STK Advice of Charge Status
**/
typedef struct {						///< Notify of new AoC status 
	CallIndex_t index;		 			///< Call Index
	AoCStatus_t AoCStatus;				// AoC status 
	}StkReportCallAoCStatus_t;

/**
STK report call CCM 
**/
typedef struct {						///< Notify MMI of Current Call Meter values
	CallIndex_t		index;		 		///< CallIndex
	Boolean			is_call_release;	///< TRUE, if report is due to a call release 
	Duration_t		duration;			///< Durartion of the call
	CallMeterUnit_t	CCM_units;			///< new Current Call Meter in units 
	}StkReportCallCCM_t;

/** STK Call control setup response.
** Data payload for normal Call Control (non-SS and non_USSD) response message
**/
typedef struct
{
	StkCallControlResult_t cc_result;	///< Call Control return result from SIM 
	StkCallControl_t old_type;	///< Call type when we sent the Call Control request 
	StkCallControl_t new_type;	///< Call type specified by SIM Call Control 
	
	Boolean addr_changed;		///< TRUE if the called party address is new or changed 
	Boolean bc1_changed;		///< TRUE if the BC1 parameters are new or changed 
	Boolean subaddr_changed;	///< TRUE if the called party subaddress is new or changed 
	Boolean alpha_id_changed;	///< TRUE if alpha identifier is new or changed 
	Boolean bc_repeat_changed;	///< TRUE if BC repeat indicator is new or changed 
	Boolean bc2_changed;		///< TRUE if the BC2 parameters are new or changed 
	
	/* Called party address parameters */
	gsm_TON_t ton;	///< Called party type of number
	gsm_NPI_t npi;	///< Called party nuber plan identifier

	char number[MAX_DIGITS + 1];	///< ASCII encoded dialling number 

	/* BC1 data */
	BearerCapability_t bc1; ///< Bearer capability 1 data

	/* Called Party Subaddress data */
	Subaddress_t subaddr_data; ///< Called part sub address

	/* Alpha Identifier data */
	Boolean alpha_id_valid;		///< TRUE if alpha identifier data is valid */
	UInt8 alpha_id_len;			///< Alpha ID length
	UInt8 alpha_id[ALPHA_ID_LEN];///< Alpha ID data

	BearerCapability_t bc2; ///< Bearer capability 2 data
	UInt8 bc_repeat;	///< BC Repeat Indicator

} StkCallControlSetupRsp_t;

/**
Data payload for SS Call Control response message 
**/
typedef struct
{
	StkCallControlResult_t cc_result;	///< Call Control return result from SIM 
	StkCallControl_t old_type;	///< Call type when we sent the Call Control request 
	StkCallControl_t new_type;	///< Call type specified by SIM Call Control 

	Boolean ss_str_changed;		///< TRUE if the SS dialing string is new or changed 

	UInt8	ton_npi;	///< TON/NPI byte: one of UNKNOWN_TON_UNKNOWN_NPI (128), UNKNOWN_TON_ISDN_NPI (129) & INTERNA_TON_ISDN_NPI (145)
	UInt8	ss_len;				///< Length of SS dialing string 
	UInt8	ss_data[MAX_DIGITS / 2]; ///< SS dialing string 

	/* Alpha Identifier data */
	Boolean alpha_id_valid;		///< TRUE if alpha identifier data is valid 
	UInt8 alpha_id_len;		///< Alpha ID length
	UInt8 alpha_id[ALPHA_ID_LEN];///< Alpha ID data

} StkCallControlSsRsp_t;

/**
Data payload for USSD Call Control response message 
**/
typedef struct
{
	StkCallControlResult_t cc_result;	///< Call Control return result from SIM 
	StkCallControl_t old_type;	///< Call type when we sent the Call Control request 
	StkCallControl_t new_type;	///< Call type specified by SIM Call Control 

	Boolean ussd_str_changed;		///< TRUE if the USSD dialing string is new or changed 

	UInt8	ussd_len;				///< Length of USSD dialing string 
	UInt8	ussd_data[PHASE2_MAX_USSD_STRING_SIZE+1]; ///< USSD dialing string 

	/* Alpha Identifier data */
	Boolean alpha_id_valid;		///< TRUE if alpha identifier data is valid 
	UInt8 alpha_id_len;			///< Alpha ID length
	UInt8 alpha_id[ALPHA_ID_LEN];///< Alpha ID data

	UInt8 ussd_data_dcs;			/// dcs for USSD string
} StkCallControlUssdRsp_t;

/**
Data payload for MO SMS Call Control response message 
**/
typedef struct
{
	StkCallControlResult_t cc_result;	///< Call Control return result from SIM */

	Boolean sca_changed;		///< TRUE if the service centre number is changed */
	Boolean dest_changed;		///< TRUE if destination number is changed */

	/* Service Center Number data */
	UInt8 sca_toa;				///< Service Center Address type of address
	UInt8 sca_number_len;		///< SCA number length
	UInt8 sca_number[SMS_MAX_DIGITS / 2]; ///<SCA number

	/* Destination number data */
	UInt8 dest_toa;		///< Destination type of address
	UInt8 dest_number_len;	///< Destination number length
	UInt8 dest_number[SMS_MAX_DIGITS / 2];///< Destination number

	/* Alpha Identifier data */
	Boolean alpha_id_valid;		///< TRUE if alpha identifier data is valid 
	UInt8 alpha_id_len;			///< Alpha ID length
	UInt8 alpha_id[ALPHA_ID_LEN];///< Alpha ID data

} StkCallControlSmsRsp_t;

/**
Data payload for Run AT-Command response message 
**/
typedef struct
{

	UInt16	length;                      ///< Length of AT cmd response
	UInt8	response[MAX_AT_RESPONSE+1]; ///< AT cmd response buffer 

} RunAT_Response;

/**
STK API Structure
**/
typedef struct
{

    Boolean GI_Packed_flag;

    SATK_EventData_t satkMsgData;

    //cached root menu
    SetupMenu_t *rootMenu;
    Boolean stkMenuEventPendingForward;

    // Pending msg for refresh indication
    InterTaskMsg_t *refreshMsg;

    Boolean isRefreshReset;

    SetupCallData_t stkSetupCallData;

    // Terminal Profile data set by SATK_SetTermProfile() 
    UInt8 Stk_Terminal_Profile_Data[MAX_TERMINAL_PROFILE_ARRAY_SIZE];
    UInt8 Stk_Terminal_Profile_Len;

} STK_API_STRUCT_t;

/**
payload for MSG_STK_PROACTIVE_CMD_FETCHING_ON_OFF_REQ
**/
typedef struct
{
	CallbackFunc_t* simAccessCb;
	Boolean         enableCmdFetching;
}StkProactiveCmdFetchingOnOffReq_t;

/**
payload for MSG_STK_SEND_POLLING_INTERVAL_REQ
**/
typedef struct
{
	CallbackFunc_t* simAccessCb;
    UInt16          pollingInterval;
}StkPollingIntervalReq_t;

//******************************************************************************
// Function Prototype
//******************************************************************************

/**
    Initialize all the resources used by STK
**/	
void STK_Init( void );

/**
    Create and start STK task
**/
void STK_Run( void  );
// shazheng 12/06/2010 Temporary change. Will be removed 
// after STK_Run is removed from fuse_wrapper.c file.
void STK_Run_tmp( void  ); 

/**
    Shutdown all resource used by STK
**/
void STK_Shutdown( void );

/**
    Post a message to STK task
    @param		msg (in) Intertask message to post
**/
void STK_PostMsg( InterTaskMsg_t* msg );

/**
    Set flag for Proactive SIM Setup Call command
    @param      isSimOrig (in) TRUE if SIM originated
**/
Result_t STK_SetSimOriginateCall(Boolean isSimOrig);

/**
    Set flag for STK modified call
    @param      isStkModified (in) TRUE if modified by SIM
**/
Result_t STK_SetStkModifiedCall(Boolean isStkModified);

/**
    Clean up the state for a call by deleting the call id from the call-context
**/
void STK_CleanCurrentCallSetup(void);

/**
    Processes Alpha ID data for Call Control By SIM response and notifies the
    client of response information using MSG_STK_CC_DISPLAY_IND event
    @param      clientID (in) ID of the STK module (currently not used, set to 0)
    @param      res (in) CC result code
    @param      oldTy (in) Old CC type
    @param      newTy (in) New CC type
    @param      validId (in) TRUE if Alpha ID valid
    @param      len (in) Length of Alpha ID
    @param      alphaId (in) Alpha ID data
**/
void STK_HandleAlphaID(UInt8 clientID, StkCallControlResult_t res,
						StkCallControl_t oldTy,StkCallControl_t newTy,
						Boolean validId,UInt8 len,UInt8* alphaId);

/**
    This function converts the passed Supplementary Service data in BCD format 
    to SS dialling string in ASCII format. 
    @param      ssStr (in) the buffer allocated by client to store ASCII SS string
    @param      ssStrLen (in) Size of ssStr buffer
    @param      ton_npi (in)  TON/NPI byte
    @param      ssLen (in) Number of SS data bytes in the passed "ssData"
    @param      ssData (in) SS data in BCD format to be converted

    @note: If a phone number is included in the SS dialling string and 
		   the passed TON/NPI indicates International Dialling Number, 
		   the phone number is prefixed with '+'.

**/
void STK_ConvertSsFromBcdToAscii(char *ssStr, UInt8 ssStrLen, UInt8 ton_npi, UInt8 ssLen, const UInt8 *ssData);

/**
    Process SS/USSD response from network and send Terminal Response to SIM accordingly
    @param    ssSrvRelPtr Pointer to STK SS Service Response  
**/
void STK_ProcessSsSrvRel(STK_SsSrvRel_t *ssSrvRelPtr);

/**
    Send SS/USSD Terminal response with no data from network
    @param    result1 (in) STK event
**/
void STK_ProcessSsSrvRelNoFacInfo(SATK_ResultCode_t result1);

/**
    This function forwards cached root menu event in the case it can not be forwarded earlier because SIM module was not ready
**/	
void SATK_ForwardStkMenuEvent(void);

/**
    This function forwards STK event to MMI/ATC
    @param      pMsg (in) STK message to forward
**/	
void SATK_Event_Api(STKMsg_t *pMsg);

/**
    This function relays the pending msg to client after the SIM API update is completed
**/	
void SATK_HandleRefreshCachedDataCompleteInd(void);


/*  This function sends STK Refresh Complete IND message */
void SATK_SendRefreshCompleteInd(void);

/**
	This function registers a callback function in order to bypass Broadcom's platform STK module and handle raw STK command 
	data in GSM 11.14 format.
	@param		stk_event_cb (in)	event callback function pointer
**/
Result_t SATK_RegisterSTKRawEvent(CallbackFunc_t* stk_event_cb);

/**
	This function deregisters a previously registered callback function so that Broadcom's platform STK module will 
	be re-invoked to handle STK functionality
**/
Result_t SATK_DeregisterSTKRawEvent(void);

/**
	This function returns TRUE if Broadcom's platform STK module shall be bypassed for STK proactive event.
**/
Boolean SATK_IsSTKRawEventEnabled(void);

/**
	This function sends Terminal Response as a response to the STK send DTMF request.
	@param		result (in) result code 1
	@param		result2 (in) result code 2
**/
Result_t SATK_SendSendDtmfRes(SATK_ResultCode_t result, SATK_ResultCode2_t result2);

/**
	This function sends MT Call Event (one of the features in STK Event Download) to SIMMAP (SIMMAP will decide whether
	to send the event to SIM based upon the Event List stored in SIMMAP). 
	@param		ton (in) TON of the number
	@param		npi (in) NPI of the number
	@param		number (in) ASCII encoded called party number 
	@param		subaddr_data (in) Called subadress data 
	@param		ti_value (in) TI value 
**/	
void SATK_SendMtCallEvent( gsm_TON_t ton, gsm_NPI_t npi, char *number,
						   Subaddress_t *subaddr_data, UInt8 ti_value );

/**
	This function sends Call Connected Event (one of the features in STK Event Download) to SIMMAP (SIMMAP will 
	decide whether to send the event to SIM based upon the Event List stored in SIMMAP).
	@param		mo_call (in) TRUE for mobile originated call
	@param		ti_value (in) TI value 
**/	
void SATK_SendCallConnectedEvent(Boolean mo_call, UInt8 ti_value);

/**
	This function sends Call Disconnected Event (one of the features in STK Event Download) to SIMMAP 
	(SIMMAP will decide whether to send the event to SIM based upon the Event List stored in SIMMAP). 
	@param		mo_call (in) TRUE for mobile originated call
	@param		net_disconn (in) TRUE if network initiates disconnect; FALSE if ME initiates
				disconnect or radio link fails. 
	@param		ti_value (in) TI value 
	@param		mn_cause (in) MN layer cause 
**/	
void SATK_SendCallDisconnectedEvent(Boolean mo_call, Boolean net_disconn, UInt8 ti_value, Cause_t mn_cause);

/**
	This function converts "RegisterStatus_t" registration status to "T_REGISTRATION_STATE" state 
	@param		reg_status (in) registration status
**/	
T_REGISTRATION_STATE SATK_convertRegStatus(RegisterStatus_t reg_status);

/**
	This function sends Location Status Event (one of the features in STK Event Download) to SIMMAP (SIMMAP 
	will decide whether to send the event to SIM based upon the Event List stored in SIMMAP). 
	@param		status (in) registration status received from network
	@param		mcc (in) MCC value which may contain the third digit of MNC. For example of
				Cingular Wireless, the passed MCC shall be 0x13F0.
	@param		mnc (in) MNC value. For Cingular Wireless in Sunnyvale, CA, the passed MNC shall be 0x71.
	@param		lac (in) Location area code
	@param		cell_id (in) Cell ID
	@param		rnc_id (in) RNC ID (Radio Network Controller for 3G, ignored for 2G)
	@param		access_technology (in) Radio Access Technology (UMTS, GSM, etc.)
	@note		"mcc", "mnc", "lac" and "cell_id" will be ignored if the passed "status" is not REGISTERSTATUS_NORMAL.
**/	
void SATK_SendLocationStatusEvent(RegisterStatus_t	status,
								  UInt16			mcc,
								  UInt8				mnc,
								  UInt16			lac,
								  UInt16			cell_id,
								  UInt16			rnc_id,
								  T_RAT				access_technology);

/**
	This function converts the passed "alpha_id" to the "SATKString_t" structure 
	@param		alpha_id (in) AlphaID string 
	@param		satk_str (out) SATKString_t string 
**/	
void SATK_ConvertAlphaId(const STKAlphaIdentifier_t *alpha_id, SATKString_t *satk_str);

/**
	This function converts the passed "icon_id" to the "SATKIcon_t" structure 
	@param		icon_id (in) icon 
	@param		satk_icon (out) SATKIcon_t icon 
**/	
void SATK_ConvertIconId(const STKIconId_t *icon_id, SATKIcon_t *satk_icon);

/**
    This function determines whether the IMSI detach-then-reattach procedure needs to be performed.
    @param      refresh_req (in) Refresh information
    @param      loci_refreshed (in) TRUE if EF-LOCI is refreshed
    @return     TRUE if the IMSI detach-then-reattach procedure needs to be performed

    @note       For STK Refresh with type "Init With Full File Change", 
                "Init With File Change" including EF-IMSI and 
				"File Change Notification" including EF-MISI, we need to 
				perform a detach-then-reattach procedure since the stack does 
				not perform the IMSI detach-then-reattach procedure for the new IMSI
**/
Boolean SATK_IsRefreshReattachNeeded(const Refresh_t *refresh_req, Boolean *loci_refreshed);

/**
    This function determines if the refresh file change list contains EF-RAT (AT&T RAT Mode USIM file)
    @param      refresh_req (in) Refresh information
    @return     TRUE if the refresh file change list contains EF-RAT
**/
Boolean SATK_IsRatModeChanged(const Refresh_t *refresh_req);

/**
	This function gets Refresh Reset flag 
**/	
Boolean SATK_IsRefreshReset(void);

/**
	This function resets Refresh Reset flag 
**/	
void SATK_ResetRefreshResetFlag(void);

/**
	This function posts the Setup Event List information to ATC/host. This should be called when a Setup Event List proactive 
	command is received from the SIM/USIM. 

	ATC/MMI needs to use the bit masks (e.g. BIT_MASK_USER_ACTIVITY_EVENT) defined in stk_api.h to check the enable/disable 
	status of each individual events in "setup_event_list". 
	@note
		- The Setup Event List information allows the host or MMI to filter out the relevant events if they are not enabled by 
		the SIM. Only the User Activity; Idle Screen Available; Language Selection; Browser Termination are MMI related and 
		the host/MMI needs to send relevant AT commands or call relevant API functions to send Event Download commands to SIM. 
		All other event downloads are currently performed by the BRCM platform. 
		- According to Section 11.5.1 and Section 11.6.1 in GSM 11.14, the User Activity and Idle Screen Available events must 
		be removed from the current event list after the event is sent. This needs to be properly maintained in the host and MMI. 
**/
void SATKPostSetupEventListInfo(UInt32 setup_event_list);

/**
	This function handles the Terminal Response for SS or USSD if they are converted from STK Call Setup due to STK Call Control 
	@param		result (in) result1 code
**/	
void SATK_TermRespForSsUssdFromSTKCall(Boolean result);

/**
	This function is a SATK proxy for the internal clients to build and
	send MSG_CLIENT_STK_SS_SRV_REQ to SATK module.. 

	@param		inClientInfoPtr (in) Client Information
	@param		inSsStringPtr (in) SS service request

	@return		None
**/
void SATK_SendSsSrvReq(	ClientInfo_t*	inClientInfoPtr,
						SS_String_t*	inSsStringPtr);


/**
	This function is a SATK proxy for the internal clients to build and
	send MSG_CLIENT_STK_SS_SRV_REQ to SATK module.. 

	@param		inClientInfoPtr (in) Client Information Pointer
	@param		inUssdSrvReqPtr (in) USSD service request

	@return		result (out) whether the operation was successful or not
**/
Result_t SATK_PostUssdSrvReq(ClientInfo_t*			inClientInfoPtr,
							 SsApi_UssdSrvReq_t*	inUssdSrvReqPtr);

/**
     This function sends IMEISV info to USIMMAP.
     (USIMMAP will send this information as part of the 
      Provide Local Info Terminal Response)
**/
void SATK_SendImeisvRes(void);

/**

     This function sends Network Search Mode info to USIMMAP 
     (USIMMAP will send this information as part of the 
     Provide Local Info Terminal Response)
**/
void SATK_SendNwSearchModeRes(void);

/** This function posts the MSG_STK_LOCAL_BATTERY_STATE_REQ
    signal to MSC for broadcast. This will generate a
    *MTQBATST event which expects an Terminal Response.
**/
void SATK_ProcessLocalBatteryStateReq(void);

/**
    This function sends Battery State info to USIMMAP 
    (USIMMAP will send this information as part of the 
    Provide Local Info Terminal Response)

    @param      batteryState (in)   Battery State (0-5, 0xFE, 0xFF)
**/
void SATK_SendBatteryStateRes(UInt8 batteryState);


/**
    This function sets proactive setup call state
    @param  state (in) proactive setup call state
**/
void SATK_SetProCallState(ProCallState_t state);

/**
    This function gets proactive setup call state
    @return   proactive setup call state
**/
ProCallState_t SATK_GetProCallState(void);

/**
    This function handles the case where STK Call setup is a Holdable, and the call status 
    has turned to hold
**/
void SATK_HandleStkCallHoldable(void);

/**
    This function handles the case where STK Call setup is a disconnectable, and a call has 
    been released
	@param		inClientInfoPtr (in) Client Information
    @param      exit_cause (in) Disconnect cause
**/
void SATK_HandleStkCallDisconnectable(ClientInfo_t* inClientInfoPtr, Cause_t exit_cause);

/**
`   This function handles the case where STK Call setup is a holdable, and the call hold has failed because of network
	@param		inClientInfoPtr (in) Client Information
	@param      index (in) Call Index
**/
void SATK_HandleStkCallHoldFailed(ClientInfo_t* inClientInfoPtr, UInt8 index);

/**
    This function initializes event message structure
**/
void SATK_InitEventMsgPtr(void);

/**
    This function stops Proactive Setup Call retry timer and sends Terminal Response to SIM
    @param      resultCode (in) Proactive Setup Call result code
**/
void SATK_HandleStopCallSetupRetryTimerReq(SATK_ResultCode_t resultCode);

/**
    This function gets Setup Call Data structure pointer
    @return     Setup Call Data structure pointer
**/
SetupCallData_t* SATK_GetSetupCallDataPtr(void);

/**
    This function determines if the passed proactive command shall be totally skipped by BRCM platform. 
    If it should be skipped, the proactive command raw data will be sent to the client 
    without any decoding.
    @param      proact_cmd (in) Proactive command data starting with 0xD0 byte
    @param      proact_len (in) Number of bytes in "proact_cmd"
    @return     TRUE if proactive command shall be skipped
**/
Boolean SATK_IsGenericSTKSupported(const UInt8 *proact_cmd, UInt16 proact_len);

/**
    This function posts the STK Run AT Command event to ATC/MMI
    @param      alpha_id (in) Alpha ID data
    @param      icon_id (in) Icon ID data
**/
void SATK_ProcessRunAtCmdInfoInd(const STKAlphaIdentifier_t *alpha_id, const STKIconId_t *icon_id);

/**
    This function processes the expiry of the STK Setup Call timer
**/
void SATK_SetupCallRetryTimeoutReq(void);

/**
    Gets STK API Structure pointer
**/
STK_API_STRUCT_t* getStkApiStructPtr(void);

/**
    Free up memory allocated for SATK event in stkapi.c
    @param    event (in) STK event
**/
void SATK_FreeSatkMem(SATK_EVENTS_t event);

/**
      sets the client info SIM ID in based on VM ID
	  @param		inClientInfoPtr (in) Client Information
**/
void STK_SetSimId(ClientInfo_t* inClientInfoPtr);

/**
  	Set the external proactive command status.
**/
void SATK_SetExtProactiveCmdStatus(Boolean extProactiveCmdStatus);

/**
  	Get the external proactive command status.
**/
Boolean SATK_GetExtProactiveCmdStatus(void);

/**
  	Gets STK client ID for proactive commands
**/
UInt8 STK_GetProactClientId(void);

#ifdef STK_CLIENT_ID
#undef STK_CLIENT_ID
#define STK_CLIENT_ID STK_GetProactClientId()
#endif

/** @} */ 
#endif

