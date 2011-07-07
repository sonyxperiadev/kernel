//*********************************************************************
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
*   @file   cc_api_old.h
*
*   @brief  This file defines the interface for the old call control API.
*
*****************************************************************************/
/**
*   @defgroup   CallControlOldAPIGroup   Legacy Call Control
*   @ingroup    CallControlAPIGroup
*
*   @brief      This group defines the legacy interfaces to the call control group.
*
*				This file defines the interface for call control API.This
*				file provides the function prototypes necessary to initiate
*				and receive Voice,Data,Video calls. Additionally, it provides
*				functions for call session management such as advice of charge,
*				call meter etc.
*
*	\n Use the link below to navigate back to the Call Control Overview page. \n
    \li \if CAPI2
	\ref CAPI2CallCtrlOverview
	\endif
	\if CAPI
	\ref CCOverview
	\endif
*****************************************************************************/
#ifndef _CC_API_OLD_H_
#define _CC_API_OLD_H_

/**
 * @addtogroup CallControlOldAPIGroup
 * @{
 */

//-------------------------------------------------
// Function Prototype
//-------------------------------------------------

//API prototypes
//Init functions
//**************************************************************************************
/**
	Initialize default parameters
	@return		Void
	@note
		This will initialize all call control parameters including EC,DC
	
		See CallConfig_t for parameters being initialized.
		
**/

void CC_InitCallControl(void);

//**************************************************************************************
/**
	Initialize default Call Config parameters
	@param		curCallCfg (in) Current Call Configuration
	@return		Void
	@note 
		Initialize default parameters for Call type, Call duration, Call meter etc.
		The difference between this and CC_InitCallControl is that this function
		does not initialize error correction and data compression module although
		those values belonging to call config are initalized.
		
		See CallConfig_t for parameters being initialized.
**/

void CC_InitCallCfg(CallConfig_t* curCallCfg);

//**************************************************************************************
/**
	Function to initialize Call Config parameters when using AT&F command
	@param		curCallCfg (in) Current Call Configuration
	@return		Void
	@note
		Using this initialization means that some parameters will not be initialized.
		This is done specifically because typical DUNs issue a AT&F command just before 
		dialing. This will mean that settings such as CBST, HSCSD will go back to default.
		Calling this function will avoid this unpleasent situation.
**/

void		CC_InitCallCfgAmpF(CallConfig_t* curCallCfg);
//**************************************************************************************
/**
	Initialize default Fax Call Config parameters
	@param		faxCfg (in) Current Fax Configuration
	@return		Void
**/

void		CC_InitFaxConfig( Fax_Param_t* faxCfg );

//**************************************************************************************
/**
	Initialize default Video Call Config parameters
	@param		curCallCfg (in) Current Call Configuration
	@return		Void
**/

void		CC_InitVideoCallCfg(CallConfig_t* curCallCfg);

//**************************************************************************************
/**
	Set Video Call parameters
	@param		param (in) Video call parameter
	@return		Result_t
**/

Result_t CC_SetVideoCallParam ( VideoCallParam_t param );

//**************************************************************************************
/**
	Function to return video call parameters
	@return		VideoCallParam_t
**/

VideoCallParam_t CC_GetVideoCallParam ( void );


//**************************************************************************************
/**
	Function to originate a Voice Call
	@param		clientID (in) Client ID.  The ID to identify the client which calls this API.
	@param		callNum	(in) Calling Number, which should already be parsed to exclude SS service string.
	@param		voiceCallParam (in) Voice Call Parameters.  See VoiceCallParam_t for
	voiceCallParam Parameter for voice.
	@return		Result_t
	@note

	Possible return values are ::CC_DISALLOW_AUXILIARY_CALL, ::RESULT_DIALSTR_INVALID,
	::CC_FAIL_CALL_SESSION, ::CC_FAIL_MAKE_CALL, ::CC_MAKE_CALL_SUCCESS, ::CC_WRONG_CALL_TYPE.

**/

Result_t CC_MakeVoiceCall(UInt8 clientID,UInt8* callNum,
							VoiceCallParam_t voiceCallParam);
//**************************************************************************************
/**
	Function to originate a Data Call
	@param		clientID (in) Client ID.  The ID to identify the client which calls this API.
	@param		callNum	(in) Calling Number, whioh should already be parsed to exclude SS service string.
	@return		Result_t
	@note

	Possible return values are ::CC_FAIL_CALL_SESSION, ::CC_FAIL_MAKE_CALL,
	::CC_MAKE_CALL_SUCCESS, ::CC_WRONG_CALL_TYPE, ::RESULT_DIALSTR_INVALID.
**/

Result_t CC_MakeDataCall(UInt8 clientID,UInt8* callNum);

//**************************************************************************************
/**
	Function to originate a Fax Call
	@param		clientID (in) Client ID.  The ID to identify the client which calls this API.
	@param		callNum	(in) Calling Number, which should already be parsed to exclude SS service string.
	@return		Result_t
	@note

	Possible return values are ::CC_FAIL_CALL_SESSION, ::CC_FAIL_MAKE_CALL,
	::CC_MAKE_CALL_SUCCESS, ::CC_WRONG_CALL_TYPE, ::RESULT_DIALSTR_INVALID.

**/

Result_t CC_MakeFaxCall(UInt8 clientID,UInt8* callNum);

//**************************************************************************************
/**
	Function to originate a Video Call
	@param		clientID (in) Client ID.  The ID to identify the client which calls this API.
	@param		callNum	(in) Calling Number, which should already be parsed to exclude SS service string.
	@return		Result_t
	@note

	Possible return values are ::CC_FAIL_CALL_SESSION, ::CC_FAIL_MAKE_CALL,
	::CC_MAKE_CALL_SUCCESS, ::CC_WRONG_CALL_TYPE, ::RESULT_DIALSTR_INVALID.

**/

Result_t CC_MakeVideoCall( UInt8 clientID, UInt8* callNum);

//Call Termination

//**************************************************************************************
/**
	Function to Terminate Call
	@param		callIdx (in) Call Index of the established Call
	@return		Result_t
	@note
	This function is called to terminate the on going call with the 
	specific call index. Termination fail / success is returned.
	Possible return values are ::CC_FAIL_CALL_SESSION, ::CC_FAIL_MAKE_CALL,
	::CC_MAKE_CALL_SUCCESS, ::CC_WRONG_CALL_TYPE, ::RESULT_DIALSTR_INVALID.


**/

Result_t	CC_EndCall(UInt8 callIdx);

//**************************************************************************************
/**
	Function to Terminate All Calls
	@return		Result_t
	@note
	Terminates all Active Calls.
	Possible return values are ::CC_END_CALL_FAIL, ::CC_END_CALL_SUCCESS.


**/

Result_t	CC_EndAllCalls(void);


//**************************************************************************************
/**
	Function to Terminate Call immediately
	@param		callIdx (in) Call Index of the established Call
	@return		Result_t
	@note
	This function is called to immediately terminate the on going call with the 
	specific call index. Termination fail / success is returned.
	Possible return values are ::CC_FAIL_CALL_SESSION, ::CC_FAIL_MAKE_CALL,
	::CC_MAKE_CALL_SUCCESS, ::CC_WRONG_CALL_TYPE, ::RESULT_DIALSTR_INVALID.

    MS >> REL COMPL >> NW


**/

Result_t	CC_EndCallImmediate(UInt8 callIdx);

//**************************************************************************************
/**
	Function to Terminate All Calls immediately
	@return		Result_t
	@note
	Immediately terminates all Active Calls.
	Possible return values are ::CC_END_CALL_FAIL, ::CC_END_CALL_SUCCESS.

    MS >> REL COMPL >> NW
**/

Result_t	CC_EndAllCallsImmediate(void);

//**************************************************************************************
/**
	Function to Terminate All MultiParty Calls
	@return		Result_t
	@note
	Terminates all Active MultiParty Calls.
	Possible return values are ::CC_WRONG_CALL_INDEX, ::CC_END_CALL_SUCCESS.

**/

Result_t	CC_EndMPTYCalls(void); 

//**************************************************************************************
/**
	Function to Terminate All Held Calls
	@return		Result_t
	@note
	Terminates all Calls that are currently on Hold.
	Call termination success / fail is returned
	Possible return values are ::CC_WRONG_CALL_INDEX, ::CC_END_CALL_SUCCESS.
**/

Result_t	CC_EndHeldCall(void);
 

//Call Acceptance
//**************************************************************************************
/**
	Function to Accept a Voice Call
	@param		clientID (in) Client ID.  The ID to identify the client which calls this API.
	@param		callIndex (in) Calling Number.  The call with the callIndex to be answered.
	@return		Result_t
	@note

	Return ::CC_ACCEPT_CALL_SUCCESS ?if the API call is successful; otherwise
	::CC_ACCEPT_CALL_FAIL
**/

Result_t	CC_AcceptVoiceCall(UInt8 clientID,UInt8 callIndex);

//**************************************************************************************
/**
	Function to Accept a Data Call
	@param		clientID (in) Client ID.  The ID to identify the client which calls this API.
	@param		callIndex (in) Calling Number.  The call with the callIndex to be answered
	@return		Result_t
	@note

	Return ::CC_ACCEPT_CALL_SUCCESS ?if the API call is successful; otherwise
	::CC_ACCEPT_CALL_FAIL
**/

Result_t	CC_AcceptDataCall(UInt8 clientID,UInt8 callIndex);

//**************************************************************************************
/**
	Function to Accept the Waiting MT Call
	@param		clientID (in) Client ID.  The ID to identify the client which calls this API.
	@return		Result_t
	@note

	Return ::CC_ACCEPT_CALL_SUCCESS if the API call is successful; otherwise
	::CC_ACCEPT_CALL_FAIL
**/

Result_t	CC_AcceptWaitingCall(UInt8 clientID);

//**************************************************************************************
/**
	Function to Accept a Video Call
	@param		clientID (in) Client ID.  The ID to identify the client which calls this API.
	@param		callIndex (in) Calling Number.  The call with the callIndex to be answered
	@return		Result_t
	@note

	Return ::CC_ACCEPT_CALL_SUCCESS if the API call is successful; otherwise
	::CC_ACCEPT_CALL_FAIL
**/

Result_t	CC_AcceptVideoCall(UInt8 clientID,UInt8 callIndex);

//Call Progress

//**************************************************************************************
/**
	Function to Hold Current Active Call
	@return		Result_t
	@note
	This function allows one to put the current active call on hold.
	The call be retrieved and made active again later.
	Possible return values are ::CC_HOLD_CALL_FAIL, ::CC_HOLD_CALL_SUCCESS

**/

Result_t	CC_HoldCurrentCall(void);

//**************************************************************************************
/**
	Function to Hold Call with Call Index
	@param		callIndex (in) Index of call to be held
	@return		Result_t
	@note
	This function is put a current call specified by call index on hold.The call on hold
	can be retrieved later.
	Possible return values are ::CC_HOLD_CALL_FAIL, ::CC_HOLD_CALL_SUCCESS

**/

Result_t	CC_HoldCall(UInt8 callIndex);

//**************************************************************************************
/**
	Function to Retrieve Last Call with Held Status
	@return		Result_t
	@note
	Possible return values are ::CC_RESUME_CALL_SUCCESS, ::CC_RESUME_CALL_FAIL
**/

Result_t	CC_RetrieveNextHeldCall(void);

//**************************************************************************************
/**
	Function to Retrieve Call with Call Index
	@param		callIndex (in) Index of call to be retrieved
	@return		Result_t
	@note
	Possible return values are ::CC_RESUME_CALL_SUCCESS, ::CC_RESUME_CALL_FAIL

**/

Result_t	CC_RetrieveCall(UInt8 callIndex);

//**************************************************************************************
/**
	Function to Swap the call with the held call index to the active status
	@param		callIndex (in) Index of call 
	@return		Result_t
	@note
	Possible return values are ::CC_SWAP_CALL_FAIL, ::CC_SWAP_CALL_SUCCESS

**/

Result_t	CC_SwapCall(UInt8 callIndex);

//**************************************************************************************
/**
	Function to Split an active call with the call index from the multi-party calls
	@param		callIndex (in) Index of call to be split
	@return		Result_t
	@note
	Possible return values are ::CC_SPLIT_CALL_SUCCESS, ::CC_SPLIT_CALL_FAIL.
**/

Result_t	CC_SplitCall(UInt8 callIndex);

//**************************************************************************************
/**
	Function to Join an active call with the call index to the multi-party calls
	@param		callIndex (in) Index of call to be joined
	@return		Result_t
	Possible return values are ::CC_JOIN_CALL_SUCCESS, ::CC_JOIN_CALL_FAIL.
**/


Result_t	CC_JoinCall(UInt8 callIndex);

//**************************************************************************************
/**
	Function to Request Explicit Call Transfer to connect an active call with a held call or a waiting call.
	@param		callIndex (in) Index of call 
	@return		Result_t
	Possible return values are ::CC_TRANS_CALL_SUCCESS, ::CC_TRANS_CALL_FAIL.
**/

Result_t	CC_TransferCall(UInt8 callIndex);

//Call Query

//**************************************************************************************
/**
	Function to Get the call index for a particular CC call state.
	@return		TRUE if the call index exist otherwise FALSE
**/

UInt8			CC_GetCallIndexInThisState(	UInt8*			inCallIndexPtr,
											CCallState_t	inCcCallState);

//**************************************************************************************
/**
	Function to Get the call index for the current active call.
	@return		Call Index in UInt8 form
**/

UInt8			CC_GetCurrentCallIndex(void);

//**************************************************************************************
/**
	Function to Get the last call index with the active status
	@return		Call Index in UInt form
**/

UInt8			CC_GetNextActiveCallIndex(void);

//**************************************************************************************
/**
	Function to Get the last call index with the hold status.
	@return		Call Index in UInt form
**/

UInt8			CC_GetNextHeldCallIndex(void);

//**************************************************************************************
/**
	Function to Get the last call index with the hold status.
	@return		Call Index in UInt form
**/

UInt8			CC_GetNextWaitCallIndex(void);


//**************************************************************************************
/**
	Function to Get the lowest call index of the multi-party calls.
	@return		Call Index in UInt form
**/


UInt8			CC_GetMPTYCallIndex(void); 

//**************************************************************************************
/**
	Function to Get the Call State of the call with the specified call index.
	@param		callIndex (in) Index of call
	@return		CCallState_t
**/

CCallState_t	CC_GetCallState(UInt8 callIndex);

//**************************************************************************************
/**
	Function to Get the Call Type of the call with the specified call index.
	@param		callIndex (in) Index of call
	@return		CCallType_t
**/

CCallType_t		CC_GetCallType(UInt8 callIndex);

//**************************************************************************************
/**
	Function to Get the call exit cause with the last ended call.
	@return		Cause_t
	@note
		This will return the cause values associated with 
**/

Cause_t			CC_GetLastCallExitCause(void);

//**************************************************************************************
/**
	Function to Get the name of calling party with the call index.
	@param		callIndex (in) Index of call
	@param		outCcCnapNamePtr (out) pointer to the CNAP structure (UCS2 capable)
	@return		None 
	@note 
		The return value is a pointer to the data structure holding the call name and
		encoding scheme.  If the name (through CNAP) is available and present from the
		network while CSSI and CSSU is set correctly, return a pointer to the structure
		hodling a copy of the name.  Otherwise, returns NULL.  It's the responsibilty
		of the caller of this function to free the returned block.
**/

void  CC_GetCNAPName(UInt8 callIndex, CcCnapName_t* outCcCnapNamePtr); 

//**************************************************************************************
/**
	Function to get the phone number of a MO voice or CSD call with the call index.
	For MT voice or CSD call, this function returns NULL string for phone number. 

	@param		callIndex (in) Index of call
	@param		phNum (in) the phone number with the call index
	@return		Boolean 
	@note		The phNum points to the buffer to hold the phone number and is managed
				by the client. The size of this buffer shall at least be (MAX_DIGITS + 2)
**/

Boolean	CC_GetCallNumber(UInt8 callIndex,UInt8* phNum);


//**************************************************************************************
/**
	Function to Get information about the call associated with a call index
	@param		callIndex (in) Index of call
	@param		callingInfoPtr (in) Pointer to Call Info.
	@return		Boolean 
	@note 
		See CallingInfo_t and CallingName_t for kind of information about call.

	@see
		PresentStatus_t

**/

Boolean			CC_GetCallingInfo(UInt8 callIndex,CallingInfo_t* callingInfoPtr);


//**************************************************************************************
/**
	Function to Get Calling Party presentation status
	@param		inCallIndex (in) Index of call
	@param		inPresentPtr (in) Pointer to presentation indicator
	@return		Boolean 
	@note 
		See CallingParty_t and PresentationInd_t for kind of information about call.

	@see
		PresentationInd_t

**/

Boolean			CC_GetCallPresent(UInt8 inCallIndex, PresentationInd_t* inPresentPtr);

//**************************************************************************************
/**
	Function to Get the Call States of all the calls.
	@param		stateList (in) the pointer to the list of call states
	@param		listSz (in) returned list size
	@return		Result_t 
	@note 
	The stateList is allocated by the API client. It returns ::CC_OPERATION_SUCCESS 
	on a successful operation.
**/

Result_t		CC_GetAllCallStates(CCallStateList_t* stateList,UInt8* listSz);

//**************************************************************************************
/**
	Function to Get Indexes of all calls.
	@param		indexList (in) the pointer to the list of call indexes
	@param		listSz (in) returned list size
	@return		Result_t 
	@note 
	The indexList is allocated by the API client.It returns ::CC_OPERATION_SUCCESS 
	on a successful operation.
**/


Result_t		CC_GetAllCallIndex(CCallIndexList_t* indexList,UInt8* listSz);

//**************************************************************************************
/**
	Function to Get Indexes of all Held calls.
	@param		indexList (in) the pointer to the list of call indexes
	@param		listSz (in) returned list size
	@return		Result_t 
	@note 
	The indexList is allocated by the API client.It returns ::CC_OPERATION_SUCCESS 
	on a successful operation.
**/

Result_t		CC_GetAllHeldCallIndex(CCallIndexList_t* indexList,UInt8* listSz);

//**************************************************************************************
/**
	Function to Get Indexes of all Active calls.
	@param		indexList (in) the pointer to the list of call indexes
	@param		listSz (in) returned list size
	@return		Result_t 
	@note 
	The indexList is allocated by the API client.It returns ::CC_OPERATION_SUCCESS 
	on a successful operation.
**/

Result_t		CC_GetAllActiveCallIndex(CCallIndexList_t* indexList,UInt8* listSz);

//**************************************************************************************
/**
	Function to Get all calls?indexes participating in the multi-party call
	@param		indexList (in) the pointer to the list of call indexes
	@param		listSz (in) returned list size
	@return		Result_t 
	@note 
	The indexList is allocated by the API client.It returns ::CC_OPERATION_SUCCESS 
	on a successful operation.
**/

Result_t		CC_GetAllMPTYCallIndex(CCallIndexList_t* indexList,UInt8* listSz);


//Result_t	GetAllAlertingCallIndex(CallIndexList* indexList,UInt8* listSz);

//**************************************************************************************
/**
	Function to Get the number of the calls participating in multi-party call.
	@return		UInt8 Number of calls that are multiparty
**/

UInt8			CC_GetNumOfMPTYCalls(void); 

//**************************************************************************************
/**
	Function to Get the number of active calls.
	@return		UInt8 Number of calls that are active
**/

UInt8			CC_GetNumofActiveCalls(void);

//**************************************************************************************
/**
	Function to Get the number of Held calls.
	@return		UInt8 Number of calls on hold 
**/

UInt8			CC_GetNumofHeldCalls(void);

//**************************************************************************************
/**
	Function to Determine if there is a waiting call.
	@return		Boolean 
	@note
		Returns TRUE if there is a call waiting.
**/

Boolean			CC_IsThereWaitingCall(void);

//**************************************************************************************
/**
	Function to Determine if there is an alerting call.
	@return		Boolean 
	@note
		Returns TRUE if there is an incoming call with state alerting.
**/

Boolean			CC_IsThereAlertingCall(void);

//**************************************************************************************
/**
	Function to Get connected line ID whith the call index.
	@param		callIndex (in) Index of call
	@return		UInt8*  pointer to the connected line ID associated with the index. 
	@note
		The returned buffer pointer is managed by the API itself

**/

UInt8*			CC_GetConnectedLineID(UInt8 callIndex);
	

//Helper functions
//**************************************************************************************
/**
	Function to Determine if a call with the call index is part of multi-party calls..
	@param		callIndex (in) Index of call
	@return		Boolean 
	@note
		TRUE if the call associated with that call index is a Multiparty call.

**/

Boolean		CC_IsMultiPartyCall(UInt8 callIndex);
//**************************************************************************************
/**
	Function to Determine if there is the voice call.
	@return		Boolean 
	@note
		TRUE is returned if there is an active voice call.

**/
Boolean		CC_IsAnyCallActive(void);
//**************************************************************************************
/**
	Function to Determine if there is the voice call.
	@return		Boolean 
	@note
		TRUE is returned if there is an active voice call.

**/

Boolean		CC_IsThereVoiceCall(void);

//**************************************************************************************
/**
	Function to Determine if there is the data call.
	@return		Boolean 
	@note
		TRUE is returned if there is an active data call.

**/

Boolean		CC_IsThereDataCall(void);

//**************************************************************************************
/**
	Function to Determine if the connected line ID is allowed to be presented with the call index.
	@param		callIndex (in) the call index to be checked
	@return		Boolean 
	@note
		Value TRUE is returned if Connected Line ID presentation is allowed.

**/

Boolean		CC_IsConnectedLineIDPresentAllowed(UInt8 callIndex);


//**************************************************************************************
/**
	Function to Get the current on-going call duration in milli-seconds.
	@param		callIndex (in) the call index to be checked
	@return		UInt32 Time in milliseconds.

**/

UInt32		CC_GetCurrentCallDurationInMilliSeconds(UInt8 callIndex);

//**************************************************************************************
/**
	Function to Get CCM with the last ended call.
	@return		UInt32 CCM.
	@note
		This function returns the Current Call Meter value in 
		usigned integer 32 format. This allows the subscriber
		to get information about the cost of calls.

**/

UInt32		CC_GetLastCallCCM(void);

//**************************************************************************************
/**
	Function to Get the call duration with the last ended call.
	@return		UInt32  Time.
	@note
		This function will return the duration of the last call.

**/

UInt32		CC_GetLastCallDuration(void);


//**************************************************************************************
/**
	Function to Get the Call Index of the Data Call.
	@return		UInt8  Call Index.
	@note
		This function will return the assigned call index of the data call.

**/

UInt8		CC_GetDataCallIndex(void);

//**************************************************************************************
/**
	Function to Get Client ID associated with a specific Call index for MO or MT calls.
	@param		callIndex( in) Call Index
	@return		UInt8  Client ID.

**/

UInt8		CC_GetCallClientID(UInt8 callIndex);

//**************************************************************************************
/**
	Function to Get Type of Address.
	@param		callIndex( in) Call Index
	@return		UInt8  Address type
	@note
		This function can be used to return type of address
		associated with a specific call. The call is specified using the
		call index. Address type is specified in GSM 04.08 subclause 10.5.4.7.

**/

UInt8		CC_GetTypeAdd(UInt8 callIndex);

//Enable/Disable the auto rejection to the MT voice call 

//**************************************************************************************
/**
	Function to Enable or disable the auto reject of  User Determined User Busy (UDUB) to the MT voice call.
	@param		enableAutoRej( in) Enable / Disable Auto rejection
	@return		Result_t 
	@note
	Function returns ::RESULT_OK on completion.

**/

Result_t	CC_SetVoiceCallAutoReject(Boolean enableAutoRej);

//**************************************************************************************
/**
	Function to Get Status of the auto reject of User Determined User Busy (UDUB) to the MT voice call.
	@return		Boolean 
	@note
	This function returns TRUE if auto UDUB is enabled. Function returns ::RESULT_OK on completion.

**/

Boolean		CC_IsVoiceCallAutoReject(void);

//**************************************************************************************
/**
	Function to Enable or disable the TTY call. 
	@param		enable( in) Enable / Disable TTY Call.TRUE: Enable TTY call.
	@return		Result_t 
	@note
	It should be set to TRUE before a TTY MO call will be made
	or TTY MT Call will be received. Function return ::RESULT_OK after completion.

**/
Result_t	CC_SetTTYCall(Boolean enable);

//**************************************************************************************
/**
	Function to Query if the TTY call is enable or disable. 
	@return		Boolean  
	@note
	The return value indicates if TTY call is enabled or not.
	TRUE = Enabled

**/
Boolean		CC_IsTTYEnable(void);


//**************************************************************************************
/**
	Function to get the Current Call Meter value of a call. It returns Call Meter Units 
	calculated based on Facility message/IE received from the network if AOC (Advice of Charge) 
	is supported for this call. 

	@param		callIndex (in) Call Index
	@param		ccm (out) pointer to the returned call meter value. 

	@return		RESULT_OK if call meter value is correctly returned; 
				RESULT_ERROR otherwise, e.g. call index is invalid or AOC is not supported. 
**/
Result_t CC_GetCCM(UInt8 callIndex, CallMeterUnit_t *ccm);


//**************************************************************************************
/**
	This API function anables the client to send a DTMF tone. The API would send the
	send DTMF request to the MNCC module. Based on the state of the MNCC state machine
	this request gone get handled.

	@param		inApiClientId (in) The API client ID.
	@param		inDtmfObjPtr (in) A pointer to the DTMF object

	@return		cause (out) The cause of the operation
**/
Result_t CcApi_SendDtmf(UInt8 inApiClientId, ApiDtmf_t* inDtmfObjPtr);


//**************************************************************************************
/**
	This API function anables the client to stop a DTMF tone. The API would send the
	stop DTMF request to the MNCC module. Based on the state of the MNCC state machine
	this request gone get handled.

	@param		inApiClientId (in) The API client ID.
	@param		inDtmfObjPtr (in) A pointer to the DTMF object.

	@return		cause (out) The cause of the operation
**/
Result_t CcApi_StopDtmf(UInt8 inApiClientId, ApiDtmf_t* inDtmfObjPtr);


//**************************************************************************************
/**
	This API function anables the client to stop a DTMF tone sequence (for now).

	@param		inApiClientId (in) The API client ID.
	@param		inDtmfObjPtr (in) A pointer to the DTMF object (here callIndex is just relevant)

	@return		void
**/
void CcApi_AbortDtmf(UInt8 inApiClientId, ApiDtmf_t* inDtmfObjPtr);



//**************************************************************************************
/**
	This API function sets the Dtmf timer period..

	@param		inCallIndex (in) The call index. This argument isn't applicable yet.
	@param		inDtmfTimerType (in) The DTMF timer type.
	@param		inDtmfTimeInterval (in) The DTMF timer period.

	@return		void
**/
void CcApi_SetDtmfTimer(UInt8		inCallIndex,
						DtmfTimer_t	inDtmfTimerType,
						Ticks_t		inDtmfTimeInterval);

//**************************************************************************************
/**
	This API function resets the DTMF timer period based on the type of timer.

	@param		inCallIndex (in) The call index. This argument isn't applicable for this
				version.
	@param		inDtmfTimerType (in) DTMF timer type, which identifies which type of
				timer shall be reset.

	@return		void
**/
void CcApi_ResetDtmfTimer(	UInt8		inCallIndex,
							DtmfTimer_t	inDtmfTimerType);


//**************************************************************************************
/**
	This API function gets the Dtmf duration timer period.

	@param		inCallIndex (in) The call index. This argument isn't applicable yet.
	@param		inDtmfTimerType (in) The DTMF timer type.

	@return		The local database DTMF tone playing period.
**/
Ticks_t CcApi_GetDtmfTimer(UInt8 inCallIndex, DtmfTimer_t inDtmfTimerType);



//**************************************************************************************
/**
	This API function gets the bearer capability per callindex

	@param		inClientId	This is not used currently.
	@param		inCallIndex used to access the bc per call.
	@param		outBcPtr	This contains all the parameters related to the bearercapability to be retreived
				the bearer capability and repeat indicator will be copied to the location pointed to in the
				structure inBcPtr.
	
	@return		RESULT_OK if we were able to successfully copy the BC to the given location
				RESULT_ERROR if were not able to copy to the location due to BC not being initialized or invalid pointer.
**/
Result_t CC_GetBearerCapability(	UInt8					inClientId,
									CallIndex_t				inCallIndex,
									CC_BearerCap_t*			outBcPtr);




//**************************************************************************************
/**
	Function to Get the Received bytes for the last ended call.
	@return		UInt32  Total Received Bytes.
	@note
		This function is used in association with data calls to check
		the total number of received bytes for the last call. The 
		return value is in unsigned integer 32 format.

**/

UInt32 CC_GetLastDataCallRxBytes(void);

//**************************************************************************************
/**
	Function to Get the Transmitted bytes for the last ended call.
	@return		UInt32  Total Transmitted Bytes.
	@note
		This function is used in association with data calls to check
		the total number of transmitted bytes for the last call. The 
		return value is in unsigned integer 32 format.

**/

UInt32 CC_GetLastDataCallTxBytes(void);

//**************************************************************************************
/**
	Function to check if the call is SIM originated call,. 

	@param		callIndex (in) Index of call
	@return		Boolean 
**/
Boolean CC_IsSimOriginedCall(UInt8 callIndex);


//**************************************************************************************
/**
	Function to check whether there is ongoing emergency call.

	@return		TRUE if there is ongoing emergency call.
				FALSE No emergency call. 
**/
Boolean CC_IsThereEmergencyCall(void);


//**************************************************************************************
/**
	Function to get the TI of a particular call index

    @param      ti (out) pointer to TI
	@param		inCallIndex (in) Index of call
	@return		Boolean 
**/
Result_t CC_GetTiFromCallIndex(UInt8* ti, CallIndex_t inCallIndex);

//**************************************************************************************
/**
	Function to determine if there is an active voice call

	@return		Boolean 
**/
Boolean CC_IsThereVoiceCall(void);

//**************************************************************************************
/**
	Function to get call type based on call index

    @param      callIndex (in) 
	@return		CCallType_t call type
**/
CCallType_t CC_GetCallType(UInt8 callIndex);

/** Parse a dial string.  The return value is the call type.  
 */
CallType_t DIALSTR_ParseGetCallType(const UInt8* dialStr); //Has to be cleaned up


/** @} */

#endif //_CC_API_OLD_H_

