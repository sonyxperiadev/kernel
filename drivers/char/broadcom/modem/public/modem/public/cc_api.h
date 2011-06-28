//*********************************************************************
//
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
*   @file   cc_api.h
*
*   @brief  This file defines the interface for call control API.
*
*****************************************************************************/
/**
*   @defgroup   CallControlAPIGroup   Call Control
*   @ingroup    SystemGroup
*
*   @brief      This group defines the interfaces to the call control group.
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
#ifndef _CC_API_H_
#define _CC_API_H_

/**
 * @addtogroup CallControlAPIGroup
 * @{
 */

//-------------------------------------------------
// Function Prototype
//-------------------------------------------------

//**************************************************************************************
/**
	Set call control configurable element
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inElement (in) Element type
	@param		inCompareObjPtr (in) pointer to the compare object
	@param		inElementPtr (in) pointer to the element content
	@return		retsult type
	@note 
		This will set update the content of a configurable call controll element
		
**/
Result_t CcApi_SetElement(	ClientInfo_t*	inClientInfoPtr,
							CcApi_Element_t	inElement,
							void*			inCompareObjPtr,
							void*			inElementPtr);



//**************************************************************************************
/**
	Gets call control configurable element
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inElement (in) Element type
	@param		inCompareObjPtr (in) pointer to the compare object
	@param		outElementPtr (in) pointer to the element which shall be updated
	@return		retsult type
	@note 
		This will gets the current setting of a configurable call controll element
		
**/
Result_t CcApi_GetElement(	ClientInfo_t*	inClientInfoPtr,
							CcApi_Element_t	inElement,
							void*			inCompareObjPtr,
							void*			outElementPtr);


//**************************************************************************************
/**
	Initialize default parameters
	@param		inClientInfoPtr (in) pointer to the client Info
	@return		Void
	@note 
		This will initialize all call control parameters including ECDC
	
		See CallConfig_t for parameters being initialized.
		
**/
void CcApi_InitCallControl(ClientInfo_t* inClientInfoPtr);

//**************************************************************************************
/**
	Initialize default Call Config parameters
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCurCallCfgPtr (in) Current Call Configuration
	@return		Void
	@note 
		Initialize default parameters for Call type, Call duration, Call meter etc.
		The difference between this and CC_InitCallControl is that this function
		does not initialize error correction and data compression module although
		those values belonging to call config are initalized.
		
		See CallConfig_t for parameters being initialized.
**/
void CcApi_InitCallCfg(ClientInfo_t* inClientInfoPtr,
					   CallConfig_t* inCurCallCfgPtr);

//**************************************************************************************
/**
	Function to initialize Call Config parameters when using AT&F command
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCurCallCfgPtr (in) Current Call Configuration
	@return		Void
	@note
		Using this initialization means that some parameters will not be initialized.
		This is done specifically because typical DUNs issue a AT&F command just before 
		dialing. This will mean that settings such as CBST, HSCSD will go back to default.
		Calling this function will avoid this unpleasent situation.
**/
void CcApi_InitCallCfgAmpF(	ClientInfo_t* inClientInfoPtr,
							CallConfig_t* inCurCallCfgPtr);

//**************************************************************************************
/**
	Initialize default Fax Call Config parameters
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inFaxCfgPtr (in) Current Fax Configuration
	@return		Void
**/
void CcApi_InitFaxConfig(	ClientInfo_t*	inClientInfoPtr,
							Fax_Param_t*	inFaxCfgPtr);

//**************************************************************************************
/**
	Initialize default Video Call Config parameters
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCurCallCfgPtr (in) Current Call Configuration
	@return		Void
**/
void CcApi_InitVideoCallCfg(ClientInfo_t*	inClientInfoPtr,
							CallConfig_t*	inCurCallCfgPtr);

//**************************************************************************************
/**
	Set Video Call parameters
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inVideoParam (in) Video call parameter
	@return		Result_t
**/
Result_t CcApi_SetVideoCallParam(	ClientInfo_t*		inClientInfoPtr,
									VideoCallParam_t	inVideoParam);

//**************************************************************************************
/**
	Function to return video call parameters
	@param		inClientInfoPtr (in) pointer to the client Info
	@return		VideoCallParam_t
**/
VideoCallParam_t CcApi_GetVideoCallParam(ClientInfo_t*	inClientInfoPtr);


//**************************************************************************************
/**
	Function to originate a Voice Call
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCallNumPtr	(in) Calling Number, which should already be parsed to
								exclude SS service string.
	@param		inVoiceCallParam (in) Voice Call Parameters.  See VoiceCallParam_t for
									voiceCallParam Parameter for voice.
	@return		Result_t
	@note

	Possible return values are ::CC_DISALLOW_AUXILIARY_CALL, ::RESULT_DIALSTR_INVALID,
	::CC_FAIL_CALL_SESSION, ::CC_FAIL_MAKE_CALL, ::CC_MAKE_CALL_SUCCESS, ::CC_WRONG_CALL_TYPE.

**/
Result_t CcApi_MakeVoiceCall(	ClientInfo_t*		inClientInfoPtr,
								UInt8*				inCallNumPtr,
								VoiceCallParam_t	inVoiceCallParam);

//**************************************************************************************
/**
	Function to originate a Data Call
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCallNumPtr	(in) Calling Number, whioh should already be parsed to
								exclude SS service string.
	@return		Result_t
	@note

	Possible return values are ::CC_FAIL_CALL_SESSION, ::CC_FAIL_MAKE_CALL,
	::CC_MAKE_CALL_SUCCESS, ::CC_WRONG_CALL_TYPE, ::RESULT_DIALSTR_INVALID.
**/
Result_t CcApi_MakeDataCall(ClientInfo_t*	inClientInfoPtr,
							UInt8*			inCallNumPtr);

//**************************************************************************************
/**
	Function to originate a Fax Call
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCallNumPtr	(in) Calling Number, which should already be parsed to
								exclude SS service string.
	@return		Result_t
	@note

	Possible return values are ::CC_FAIL_CALL_SESSION, ::CC_FAIL_MAKE_CALL,
	::CC_MAKE_CALL_SUCCESS, ::CC_WRONG_CALL_TYPE, ::RESULT_DIALSTR_INVALID.

**/
Result_t CcApi_MakeFaxCall(	ClientInfo_t*	inClientInfoPtr,
							UInt8*			inCallNumPtr);

//**************************************************************************************
/**
	Function to originate a Video Call
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCallNumPtr	(in) Calling Number, which should already be parsed to
								exclude SS service string.
	@return		Result_t
	@note

	Possible return values are ::CC_FAIL_CALL_SESSION, ::CC_FAIL_MAKE_CALL,
	::CC_MAKE_CALL_SUCCESS, ::CC_WRONG_CALL_TYPE, ::RESULT_DIALSTR_INVALID.

**/
Result_t CcApi_MakeVideoCall(	ClientInfo_t*	inClientInfoPtr,
								UInt8*			inCallNumPtr);


//**************************************************************************************
/**
	Function to Terminate Call
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCallIndex (in) Call Index of the established Call
	@return		Result_t
	@note
	This function is called to terminate the on going call with the 
	specific call index. Termination fail / success is returned.
	Possible return values are ::CC_FAIL_CALL_SESSION, ::CC_FAIL_MAKE_CALL,
	::CC_MAKE_CALL_SUCCESS, ::CC_WRONG_CALL_TYPE, ::RESULT_DIALSTR_INVALID.


**/
Result_t CcApi_EndCall(	ClientInfo_t*	inClientInfoPtr,
						UInt8			inCallIndex);


//**************************************************************************************
/**
	Function to Terminate Call immediately
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCallIndex (in) Call Index of the established Call
	@return		Result_t
	@note
	This function is called to immediately terminate the on going call with the 
	specific call index. Termination fail / success is returned.
	Possible return values are ::CC_FAIL_CALL_SESSION, ::CC_FAIL_MAKE_CALL,
	::CC_MAKE_CALL_SUCCESS, ::CC_WRONG_CALL_TYPE, ::RESULT_DIALSTR_INVALID.
    MS >> REL COMPL >> NW

**/
Result_t CcApi_EndCallImmediate(ClientInfo_t*	inClientInfoPtr,
								UInt8			inCallIndex);


//**************************************************************************************
/**
	Function to Terminate All Calls
	@param		inClientInfoPtr (in) pointer to the client Info
	@return		Result_t
	@note
	Terminates all Active Calls.
	Possible return values are ::CC_END_CALL_FAIL, ::CC_END_CALL_SUCCESS.


**/
Result_t CcApi_EndAllCalls(ClientInfo_t* inClientInfoPtr);


//**************************************************************************************
/**
	Function to Terminate All Calls immediately
	@param		inClientInfoPtr (in) pointer to the client Info
	@return		Result_t
	@note
	Immediately terminates all Active Calls.
	Possible return values are ::CC_END_CALL_FAIL, ::CC_END_CALL_SUCCESS.

    MS >> REL COMPL >> NW
**/
Result_t CcApi_EndAllCallsImmediate(ClientInfo_t* inClientInfoPtr);


//**************************************************************************************
/**
	Function to Terminate All MultiParty Calls
	@param		inClientInfoPtr (in) pointer to the client Info
	@return		Result_t
	@note
	Terminates all Active MultiParty Calls.
	Possible return values are ::CC_WRONG_CALL_INDEX, ::CC_END_CALL_SUCCESS.

**/
Result_t CcApi_EndMPTYCalls(ClientInfo_t* inClientInfoPtr); 

//**************************************************************************************
/**
	Function to Terminate All Held Calls
	@param		inClientInfoPtr (in) pointer to the client Info
	@return		Result_t
	@note
	Terminates all Calls that are currently on Hold.
	Call termination success / fail is returned
	Possible return values are ::CC_WRONG_CALL_INDEX, ::CC_END_CALL_SUCCESS.
**/
Result_t CcApi_EndHeldCall(ClientInfo_t* inClientInfoPtr);
 

//**************************************************************************************
/**
	Function to Accept a Voice Call
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCallIndex (in) Client ID.  The ID to identify the client which calls
								this API.
	@return		Result_t
	@note

	Return ::CC_ACCEPT_CALL_SUCCESS ?if the API call is successful; otherwise
	::CC_ACCEPT_CALL_FAIL
**/
Result_t CcApi_AcceptVoiceCall(	ClientInfo_t*	inClientInfoPtr,
								UInt8			inCallIndex);

//**************************************************************************************
/**
	Function to Accept a Data Call
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCallIndex (in) Calling Number.  The call with the callIndex to be answered
	@return		Result_t
	@note

	Return ::CC_ACCEPT_CALL_SUCCESS ?if the API call is successful; otherwise
	::CC_ACCEPT_CALL_FAIL
**/
Result_t CcApi_AcceptDataCall(	ClientInfo_t*	inClientInfoPtr,
								UInt8			inCallIndex);

//**************************************************************************************
/**
	Function to Accept the Waiting MT Call
	@param		inClientInfoPtr (in) pointer to the client Info
	@return		Result_t
	@note

	Return ::CC_ACCEPT_CALL_SUCCESS if the API call is successful; otherwise
	::CC_ACCEPT_CALL_FAIL
**/
Result_t CcApi_AcceptWaitingCall(ClientInfo_t* inClientInfoPtr);

//**************************************************************************************
/**
	Function to Accept a Video Call
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCallIndex (in) Calling Number.  The call with the callIndex to be answered
	@return		Result_t
	@note

	Return ::CC_ACCEPT_CALL_SUCCESS if the API call is successful; otherwise
	::CC_ACCEPT_CALL_FAIL
**/
Result_t CcApi_AcceptVideoCall(	ClientInfo_t*	inClientInfoPtr,
								UInt8			inCallIndex);

//Call Progress

//**************************************************************************************
/**
	Function to Hold Current Active Call
	@param		inClientInfoPtr (in) pointer to the client Info
	@return		Result_t
	@note
	This function allows one to put the current active call on hold.
	The call be retrieved and made active again later.
	Possible return values are ::CC_HOLD_CALL_FAIL, ::CC_HOLD_CALL_SUCCESS

**/
Result_t CcApi_HoldCurrentCall(ClientInfo_t* inClientInfoPtr);

//**************************************************************************************
/**
	Function to Hold Call with Call Index
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCallIndex (in) Index of call to be held
	@return		Result_t
	@note
	This function is put a current call specified by call index on hold.The call on hold
	can be retrieved later.
	Possible return values are ::CC_HOLD_CALL_FAIL, ::CC_HOLD_CALL_SUCCESS

**/
Result_t CcApi_HoldCall(ClientInfo_t*	inClientInfoPtr,
						UInt8			inCallIndex);

//**************************************************************************************
/**
	Function to Retrieve Last Call with Held Status
	@param		inClientInfoPtr (in) pointer to the client Info
	@return		Result_t
	@note
	Possible return values are ::CC_RESUME_CALL_SUCCESS, ::CC_RESUME_CALL_FAIL
**/
Result_t CcApi_RetrieveNextHeldCall(ClientInfo_t* inClientInfoPtr);

//**************************************************************************************
/**
	Function to Retrieve Call with Call Index
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCallIndex (in) Index of call to be retrieved
	@return		Result_t
	@note
	Possible return values are ::CC_RESUME_CALL_SUCCESS, ::CC_RESUME_CALL_FAIL

**/
Result_t CcApi_RetrieveCall(ClientInfo_t*	inClientInfoPtr,
							UInt8			inCallIndex);

//**************************************************************************************
/**
	Function to Swap the call with the held call index to the active status
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCallIndex (in) Index of call 
	@return		Result_t
	@note
	Possible return values are ::CC_SWAP_CALL_FAIL, ::CC_SWAP_CALL_SUCCESS

**/
Result_t CcApi_SwapCall(ClientInfo_t*	inClientInfoPtr,
						UInt8			inCallIndex);

//**************************************************************************************
/**
	Function to Split an active call with the call index from the multi-party calls
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCallIndex (in) Index of call to be split
	@return		Result_t
	@note
	Possible return values are ::CC_SPLIT_CALL_SUCCESS, ::CC_SPLIT_CALL_FAIL.
**/
Result_t CcApi_SplitCall(	ClientInfo_t*	inClientInfoPtr,
							UInt8			inCallIndex);

//**************************************************************************************
/**
	Function to Join an active call with the call index to the multi-party calls
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCallIndex (in) Index of call to be joined
	@return		Result_t
	Possible return values are ::CC_JOIN_CALL_SUCCESS, ::CC_JOIN_CALL_FAIL.
**/
Result_t CcApi_JoinCall(ClientInfo_t*	inClientInfoPtr,
						UInt8			inCallIndex);

//**************************************************************************************
/**
	Function to Request Explicit Call Transfer to connect an active call with a held
	call or a waiting call.
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCallIndex (in) Index of call 
	@return		Result_t
	Possible return values are ::CC_TRANS_CALL_SUCCESS, ::CC_TRANS_CALL_FAIL.
**/
Result_t CcApi_TransferCall(ClientInfo_t*	inClientInfoPtr,
							UInt8			inCallIndex);


//**************************************************************************************
/**
	Function to get the call index for a particular CC call state.
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		outCallIndexPtr (in) pointer to the call index list
	@param		inCcCallState (in) CC call state
	@return		TRUE if the call index exist otherwise FALSE
**/
Boolean CcApi_GetCallIndexInThisState(	ClientInfo_t*	inClientInfoPtr,
										UInt8*			outCallIndexPtr,
										CCallState_t	inCcCallState);


//**************************************************************************************
/**
	Function to get the call index for the current active call.
	@param		inClientInfoPtr (in) pointer to the client Info
	@return		Call Index in UInt8 form
**/
UInt8 CcApi_GetCurrentCallIndex(ClientInfo_t* inClientInfoPtr);


//**************************************************************************************
/**
	Function to get the last call index with the active status
	@param		inClientInfoPtr (in) pointer to the client Info
	@return		Call Index in UInt form
**/
UInt8 CcApi_GetNextActiveCallIndex(ClientInfo_t* inClientInfoPtr);


//**************************************************************************************
/**
	Function to get the last call index with the hold status.
	@param		inClientInfoPtr (in) pointer to the client Info
	@return		Call Index in UInt form
**/
UInt8 CcApi_GetNextHeldCallIndex(ClientInfo_t* inClientInfoPtr);


//**************************************************************************************
/**
	Function to get the last call index with the hold status.
	@param		inClientInfoPtr (in) pointer to the client Info
	@return		Call Index in UInt form
**/
UInt8 CcApi_GetNextWaitCallIndex(ClientInfo_t* inClientInfoPtr);


//**************************************************************************************
/**
	Function to get the lowest call index of the multi-party calls.
	@param		inClientInfoPtr (in) pointer to the client Info
	@return		Call Index in UInt form
**/
UInt8 CcApi_GetMPTYCallIndex(ClientInfo_t* inClientInfoPtr);


//**************************************************************************************
/**
	Function to get the Call State of the call with the specified call index.
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCallIndex (in) Index of call
	@return		CCallState_t
**/
CCallState_t CcApi_GetCallState(ClientInfo_t*	inClientInfoPtr,
								UInt8			inCallIndex);

//**************************************************************************************
/**
	Function to get the Call Type of the call with the specified call index.
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCallIndex (in) Index of call
	@return		CCallType_t
**/
CCallType_t CcApi_GetCallType(	ClientInfo_t*	inClientInfoPtr,
								UInt8			inCallIndex);

//**************************************************************************************
/**
	Function to get the call exit cause with the last ended call.
	@param		inClientInfoPtr (in) pointer to the client Info
	@return		Cause_t
	@note
		This will return the cause values associated with 
**/
Cause_t CcApi_GetLastCallExitCause(ClientInfo_t* inClientInfoPtr);

//**************************************************************************************
/**
	Function to get the name of calling party with the call index.
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCallIndex (in) Index of call
	@param		outCcCnapNamePtr (out) pointer to the CNAP structure (UCS2 capable).
	@return		None 
	@note 
		The return value is a pointer to the data structure holding the call name and
		encoding scheme.  If the name (through CNAP) is available and present from the
		network while CSSI and CSSU is set correctly, return a pointer to the structure
		hodling a copy of the name.  Otherwise, returns NULL.  It's the responsibilty
		of the caller of this function to free the returned block.
**/
void  CcApi_GetCNAPName(ClientInfo_t*	inClientInfoPtr,
						UInt8			inCallIndex,
						CcCnapName_t*   outCcCnapNamePtr);


//**************************************************************************************
/**
	Function to get the phone number of a MO voice or CSD call with the call index.
	For MT voice or CSD call, this function returns NULL string for phone number. 

	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCallIndex (in) Index of call
	@param		outCalledNumberPtr (out) the phone number with the call index
	@return		Boolean 
	@note		The phNum points to the buffer to hold the phone number and is managed
				by the client. The size of this buffer shall at least be (MAX_DIGITS + 2)
**/
Boolean	CcApi_GetCallNumber(ClientInfo_t*	inClientInfoPtr,
							UInt8			inCallIndex,
							UInt8*			outCalledNumberPtr);


//**************************************************************************************
/**
	Function to get information about the call associated with a call index
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCallIndex (in) Index of call
	@param		outCallingInfoPtr (out) Pointer to Call Info.
	@return		Boolean 
	@note 
		See CallingInfo_t and CallingName_t for kind of information about call.

	@see
		PresentStatus_t

**/
Boolean CcApi_GetCallingInfo(	ClientInfo_t*	inClientInfoPtr,
								UInt8			inCallIndex,
								CallingInfo_t*	outCallingInfoPtr);


//**************************************************************************************
/**
	Function to get Calling Party presentation status
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCallIndex (in) Index of call
	@param		outPresentPtr (out) Pointer to presentation indicator
	@return		Boolean 
	@note 
		See CallingParty_t and PresentationInd_t for kind of information about call.

	@see
		PresentationInd_t

**/
Boolean CcApi_GetCallPresent(	ClientInfo_t*		inClientInfoPtr,
								UInt8				inCallIndex,
								PresentationInd_t*	outPresentPtr);

//**************************************************************************************
/**
	Function to get the Call States of all the calls.
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		outStateListPtr (out) the pointer to the list of call states
	@param		outListSizePtr (out) returned list size
	@return		Result_t 
	@note 
	The stateList is allocated by the API client. It returns ::CC_OPERATION_SUCCESS 
	on a successful operation.
**/
Result_t CcApi_GetAllCallStates(ClientInfo_t*		inClientInfoPtr,
								CCallStateList_t*	outStateListPtr,
								UInt8*				outListSizePtr);


//**************************************************************************************
/**
	Function to get Indexes of all calls.
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		outCallIndexPtr (out) the pointer to the list of call indexes
	@param		outListSizePtr (out) returned list size
	@return		Result_t 
	@note 
	The indexList is allocated by the API client.It returns ::CC_OPERATION_SUCCESS 
	on a successful operation.
**/
Result_t CcApi_GetAllCallIndex(	ClientInfo_t*		inClientInfoPtr,
								CCallIndexList_t*	outCallIndexPtr,
								UInt8*				outListSizePtr);

//**************************************************************************************
/**
	Function to get Indexes of all Held calls.
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		outCallIndexPtr (out) the pointer to the list of call indexes
	@param		outListSizePtr (out) returned list size
	@return		Result_t 
	@note 
	The indexList is allocated by the API client.It returns ::CC_OPERATION_SUCCESS 
	on a successful operation.
**/
Result_t CcApi_GetAllHeldCallIndex(	ClientInfo_t*		inClientInfoPtr,
									CCallIndexList_t*	outCallIndexPtr,
									UInt8*				outListSizePtr);

//**************************************************************************************
/**
	Function to get Indexes of all Active calls.
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		outCallIndexPtr (out) the pointer to the list of call indexes
	@param		outListSizePtr (out) returned list size
	@return		Result_t 
	@note 
	The indexList is allocated by the API client.It returns ::CC_OPERATION_SUCCESS 
	on a successful operation.
**/
Result_t CcApi_GetAllActiveCallIndex(	ClientInfo_t*		inClientInfoPtr,
										CCallIndexList_t*	outCallIndexPtr,
										UInt8*				outListSizePtr);

//**************************************************************************************
/**
	Function to get all calls indexes participating in the multi-party call
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		outCallIndexPtr (out) the pointer to the list of call indexes
	@param		outListSizePtr (out) returned list size
	@return		Result_t 
	@note 
	The indexList is allocated by the API client.It returns ::CC_OPERATION_SUCCESS 
	on a successful operation.
**/
Result_t CcApi_GetAllMPTYCallIndex(	ClientInfo_t*		inClientInfoPtr,
									CCallIndexList_t*	outCallIndexPtr,
									UInt8*				outListSizePtr);


//**************************************************************************************
/**
	Function to get the number of the calls participating in multi-party call.
	@param		inClientInfoPtr (in) pointer to the client Info
	@return		UInt8 Number of calls that are multiparty
**/
UInt8 CcApi_GetNumOfMPTYCalls(ClientInfo_t* inClientInfoPtr); 


//**************************************************************************************
/**
	Function to get the number of active calls.
	@param		inClientInfoPtr (in) pointer to the client Info
	@return		UInt8 Number of calls that are active
**/
UInt8 CcApi_GetNumofActiveCalls(ClientInfo_t* inClientInfoPtr);


//**************************************************************************************
/**
	Function to get the number of Held calls.
	@param		inClientInfoPtr (in) pointer to the client Info
	@return		UInt8 Number of calls on hold 
**/
UInt8 CcApi_GetNumofHeldCalls(ClientInfo_t* inClientInfoPtr);


//**************************************************************************************
/**
	Function to Determine if there is a waiting call.
	@param		inClientInfoPtr (in) pointer to the client Info
	@return		Boolean 
	@note
		Returns TRUE if there is a call waiting.
**/
Boolean CcApi_IsThereWaitingCall(ClientInfo_t* inClientInfoPtr);


//**************************************************************************************
/**
	Function to Determine if there is an alerting call.
	@param		inClientInfoPtr (in) pointer to the client Info
	@return		Boolean 
	@note
		Returns TRUE if there is an incoming call with state alerting.
**/
Boolean	CcApi_IsThereAlertingCall(ClientInfo_t* inClientInfoPtr);


//**************************************************************************************
/**
	Function to get connected line ID whith the call index.
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCallIndex (in) Index of call
	@return		UInt8*  pointer to the connected line ID associated with the index. 
	@note
		The returned buffer pointer is managed by the API itself

**/
UInt8*	CcApi_GetConnectedLineID(	ClientInfo_t*	inClientInfoPtr,
									UInt8			inCallIndex);

//**************************************************************************************
/**
	Function to Determine if a call with the call index is part of multi-party calls.
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCallIndex (in) Index of call
	@return		Boolean 
	@note
		TRUE if the call associated with that call index is a Multiparty call.

**/
Boolean CcApi_IsMultiPartyCall(ClientInfo_t*	inClientInfoPtr,
							   UInt8			inCallIndex);


//**************************************************************************************
/**
	Function to Determine if there is the voice call.
	@param		inClientInfoPtr (in) pointer to the client Info
	@return		Boolean 
	@note
		TRUE is returned if there is an active voice call.

**/
Boolean CcApi_IsAnyCallActive(ClientInfo_t* inClientInfoPtr);


//**************************************************************************************
/**
	Function to Determine if there is the voice call.
	@param		inClientInfoPtr (in) pointer to the client Info
	@return		Boolean 
	@note
		TRUE is returned if there is an active voice call.

**/
Boolean CcApi_IsThereVoiceCall(ClientInfo_t* inClientInfoPtr);

//**************************************************************************************
/**
	Function to Determine if there is the data call.
	@param		inClientInfoPtr (in) pointer to the client Info
	@return		Boolean 
	@note
		TRUE is returned if there is an active data call.

**/
Boolean CcApi_IsThereDataCall(ClientInfo_t* inClientInfoPtr);

//**************************************************************************************
/**
	Function to Determine if the connected line ID is allowed to be presented with the call index.
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCallIndex (in) the call index to be checked
	@return		Boolean 
	@note
		Value TRUE is returned if Connected Line ID presentation is allowed.

**/
Boolean CcApi_IsConnectedLineIDPresentAllowed(	ClientInfo_t*	inClientInfoPtr,
												UInt8			inCallIndex);


//**************************************************************************************
/**
	Function to Get the current on-going call duration in milli-seconds.
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCallIndex (in) the call index to be checked
	@return		UInt32 Time in milliseconds.

**/
UInt32  CcApi_GetCurrentCallDurationInMilliSeconds(ClientInfo_t*	inClientInfoPtr,
												   UInt8			inCallIndex);


//**************************************************************************************
/**
	Function to Get CCM with the last ended call.
	@param		inClientInfoPtr (in) pointer to the client Info
	@return		UInt32 CCM.
	@note
		This function returns the Current Call Meter value in 
		usigned integer 32 format. This allows the subscriber
		to get information about the cost of calls.
**/
UInt32 CcApi_GetLastCallCCM(ClientInfo_t* inClientInfoPtr);


//**************************************************************************************
/**
	Function to Get the call duration with the last ended call.
	@param		inClientInfoPtr (in) pointer to the client Info
	@return		UInt32  Time.
	@note
		This function will return the duration of the last call.

**/
UInt32 CcApi_GetLastCallDuration(ClientInfo_t* inClientInfoPtr)
;

//**************************************************************************************
/**
	Function to Get the Received bytes for the last ended call.
	@param		inClientInfoPtr (in) pointer to the client Info
	@return		UInt32  Total Received Bytes.
	@note
		This function is used in association with data calls to check
		the total number of received bytes for the last call. The 
		return value is in unsigned integer 32 format.

**/
UInt32 CcApi_GetLastDataCallRxBytes(ClientInfo_t* inClientInfoPtr);


//**************************************************************************************
/**
	Function to Get the Transmitted bytes for the last ended call.
	@param		inClientInfoPtr (in) pointer to the client Info
	@return		UInt32  Total Transmitted Bytes.
	@note
		This function is used in association with data calls to check
		the total number of transmitted bytes for the last call. The 
		return value is in unsigned integer 32 format.

**/
UInt32 CcApi_GetLastDataCallTxBytes(ClientInfo_t* inClientInfoPtr);


//**************************************************************************************
/**
	Function to Get the Call Index of the Data Call.
	@param		inClientInfoPtr (in) pointer to the client Info
	@return		UInt8  Call Index.
	@note
		This function will return the assigned call index of the data call.

**/
UInt8 CcApi_GetDataCallIndex(ClientInfo_t* inClientInfoPtr);


//**************************************************************************************
/**
	Function to Get Client ID associated with a specific Call index for MO or MT calls.
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCallIndex( in) Call Index
	@return		UInt8  Client ID.

**/
UInt8 CcApi_GetCallClientID(ClientInfo_t*	inClientInfoPtr,
							UInt8			inCallIndex);

//**************************************************************************************
/**
	Function to Get Type of Address.
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCallIndex( in) Call Index
	@return		UInt8  Address type
	@note
		This function can be used to return type of address
		associated with a specific call. The call is specified using the
		call index. Address type is specified in GSM 04.08 subclause 10.5.4.7.

**/
UInt8 CcApi_GetTypeAdd(	ClientInfo_t*	inClientInfoPtr,
						UInt8			inCallIndex);



//**************************************************************************************
/**
	Function to Enable or disable the auto reject of  User Determined User Busy (UDUB) to the MT voice call.
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inEnableAutoRej( in) Enable / Disable Auto rejection
	@return		Result_t 
	@note
	Function returns ::RESULT_OK on completion.

**/
Result_t CcApi_SetVoiceCallAutoReject(ClientInfo_t* inClientInfoPtr,
									  Boolean		inEnableAutoRej);

//**************************************************************************************
/**
	Function to Get Status of the auto reject of User Determined User Busy (UDUB) to the MT voice call.
	@param		inClientInfoPtr (in) pointer to the client Info
	@return		Boolean 
	@note
	This function returns TRUE if auto UDUB is enabled. Function returns ::RESULT_OK on completion.

**/
Boolean	CcApi_IsVoiceCallAutoReject(ClientInfo_t* inClientInfoPtr);

//**************************************************************************************
/**
	Function to Enable or disable the TTY call. 
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inEnable( in) Enable / Disable TTY Call.TRUE: Enable TTY call.
	@return		Result_t 
	@note
	It should be set to TRUE before a TTY MO call will be made
	or TTY MT Call will be received. Function return ::RESULT_OK after completion.

**/
Result_t CcApi_SetTTYCall(	ClientInfo_t*	inClientInfoPtr,
							Boolean			inEnable);

//**************************************************************************************
/**
	Function to Query if the TTY call is enable or disable. 
	@param		inClientInfoPtr (in) pointer to the client Info
	@return		Boolean  
	@note
	The return value indicates if TTY call is enabled or not.
	TRUE = Enabled

**/
Boolean CcApi_IsTTYEnable(ClientInfo_t* inClientInfoPtr);


//**************************************************************************************
/**
	Function to get the Current Call Meter value of a call. It returns Call Meter Units 
	calculated based on Facility message/IE received from the network if AOC (Advice of Charge) 
	is supported for this call. 

	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCallIndex (in) Call Index
	@param		outCcmPtr (out) pointer to the returned call meter value. 

	@return		RESULT_OK if call meter value is correctly returned; 
				RESULT_ERROR otherwise, e.g. call index is invalid or AOC is not supported. 
**/
Result_t CcApi_GetCCM(ClientInfo_t*		inClientInfoPtr,
					  UInt8				inCallIndex,
					  CallMeterUnit_t*	outCcmPtr);

//**************************************************************************************
/**
	This API function anables the client to send a DTMF tone. The API would send the
	send DTMF request to the MNCC module. Based on the state of the MNCC state machine
	this request gone get handled.

	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inDtmfObjPtr (in) A pointer to the DTMF object

	@return		cause (out) The cause of the operation
**/
Result_t CcApi_SendDtmfTone(ClientInfo_t*	inClientInfoPtr,
							ApiDtmf_t*		inDtmfObjPtr);


//**************************************************************************************
/**
	This API function anables the client to stop a DTMF tone. The API would send the
	stop DTMF request to the MNCC module. Based on the state of the MNCC state machine
	this request gone get handled.

	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inDtmfObjPtr (in) A pointer to the DTMF object.

	@return		cause (out) The cause of the operation
**/
Result_t CcApi_StopDtmfTone(ClientInfo_t*	inClientInfoPtr,
							ApiDtmf_t*		inDtmfObjPtr);


//**************************************************************************************
/**
	This API function anables the client to stop a DTMF tone sequence (for now).

	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inDtmfObj (in) A pointer to the DTMF object (here callIndex is just relevant)

	@return		void
**/
void CcApi_AbortDtmfTone(ClientInfo_t*	inClientInfoPtr,
						 ApiDtmf_t*		inDtmfObj);



//**************************************************************************************
/**
	This API function sets the Dtmf timer period..

	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCallIndex (in) The call index. This argument isn't applicable yet.
	@param		inDtmfTimerType (in) The DTMF timer type.
	@param		inDtmfTimeInterval (in) The DTMF timer period.

	@return		void
**/
void CcApi_SetDtmfToneTimer(ClientInfo_t*	inClientInfoPtr,
							UInt8			inCallIndex,
							DtmfTimer_t		inDtmfTimerType,
							Ticks_t			inDtmfTimeInterval);

//**************************************************************************************
/**
	This API function resets the DTMF timer period based on the type of timer.

	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCallIndex (in) The call index. This argument isn't applicable for this
				version.
	@param		inDtmfTimerType (in) DTMF timer type, which identifies which type of
				timer shall be reset.

	@return		void
**/
void CcApi_ResetDtmfToneTimer(	ClientInfo_t*	inClientInfoPtr,
								UInt8			inCallIndex,
								DtmfTimer_t		inDtmfTimerType);


//**************************************************************************************
/**
	This API function gets the Dtmf duration timer period.

	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCallIndex (in) The call index. This argument isn't applicable yet.
	@param		inDtmfTimerType (in) The DTMF timer type.

	@return		The local database DTMF tone playing period.
**/
Ticks_t CcApi_GetDtmfToneTimer(	ClientInfo_t*	inClientInfoPtr,
								UInt8			inCallIndex,
								DtmfTimer_t		inDtmfTimerType);


//**************************************************************************************
/**
	This API function gets the bearer capability per callindex

	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCallIndex (in) used to access the bc per call.
	@param		outBcPtr (out) This contains all the parameters related to the
				bearercapability to be retreived the bearer capability and repeat
				indicator will be copied to the location pointed to in the structure
				inBcPtr.
	
	@return		RESULT_OK if we were able to successfully copy the BC to the given location
				RESULT_ERROR if were not able to copy to the location due to BC not
				being initialized or invalid pointer.
**/
Result_t CcApi_GetBearerCapability(	ClientInfo_t*	inClientInfoPtr,
									CallIndex_t		inCallIndex,
									CC_BearerCap_t*	outBcPtr);


//**************************************************************************************
/**
	Function to determine if a bearer capability is supported

    @param      inClientInfoPtr (in) pointer to client info structure
	@param		inBcPtr (in) Bearer Capability
	@return		Boolean 
**/
Boolean	CcApi_IsSupportedBC(ClientInfo_t*		inClientInfoPtr,
							BearerCapability_t*	inBcPtr);


//**************************************************************************************
/**
	Function to check if the call is SIM originated call. 

	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCallIndex (in) Index of call
	@return		Boolean 
**/
Boolean CcApi_IsSimOriginedCall(ClientInfo_t*	inClientInfoPtr,
								UInt8			inCallIndex);


//**************************************************************************************
/**
	Function to check whether there is ongoing emergency call.

	@param		inClientInfoPtr (in) pointer to the client Info
	@return		TRUE if there is ongoing emergency call.
				FALSE No emergency call. 
**/
Boolean CcApi_IsThereEmergencyCall(ClientInfo_t* inClientInfoPtr);


//**************************************************************************************
/**
	Function obtains the client info with the call index.

	@param		inClientInfoPtr (out) pointer to the client Info
	@param		inCallIndex	(in) call index
	@return		void. 
**/
void CcApi_GetCallClientInfo(	ClientInfo_t*	inClientInfoPtr,
								UInt8			inCallIndex);



//**************************************************************************************
/**
	Function to get the TI of a particular call index

    @param      inClientInfoPtr (in) pointer to client info structure
	@param		inCallIndex (in) Index of call
	@param      outTransId (out) 
	@return		Boolean 
**/
Result_t CcApi_GetTiFromCallIndex(	ClientInfo_t*	inClientInfoPtr,
									CallIndex_t		inCallIndex,
									UInt8*			outTransId);

Boolean CcApi_IsCurrentStateMpty(ClientInfo_t*	inClientInfoPtr, UInt8 inCallIndex);

/** @} */

#endif //_CC_API_H_

