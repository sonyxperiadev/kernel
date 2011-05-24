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
*   @file   capi2_cc_api_old.h
*
*   @brief  This file defines the types and prototypes for the CAPI2 Call Control API functions.
*
********************************************************************************************/
/**

*   @defgroup   CAPI2_CCAPIOLDGroup   Call Control
*   @ingroup    CAPI2_CallControlOldGroup
*
*   @brief      This group defines the interfaces to the call control group.
*
*				This file defines the interface for call control API.This file provides
*				the function prototypes necessary to initiate and receive Voice,Data,Video calls.
*				Additionally, it provides functions for call session management such as
*				advice of charge, call meter etc. 
********************************************************************************************/

#ifndef _CAPI2_CALLCTRL_OLD_H
#define _CAPI2_CALLCTRL_OLD_H


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup CAPI2_CCAPIOLDGroup
 * @{
 */

//---------------------------------------------------------------
// enum
//---------------------------------------------------------------

//-------------------------------------------------
// Function Prototype
//-------------------------------------------------

//**************************************************************************************
/**
	Function to originate a Voice Call

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		callNum	(in) Calling Number
	@param		voiceCallParam (in) Voice Call Parameters
	@note
	The client ID is to identify the client which calls this API.
	"callNum" should already be parsed to exclude SS service string.
	See VoiceCallParam_t for voiceCallParam Parameter for voice. 

    The response includes multiple ::MSG_CALL_STATUS_IND (unsolicited) messages followed by either
	::MSG_VOICECALL_RELEASE_IND (if call can not be set up) or ::MSG_VOICECALL_CONNECTED_IND 
	(if call is set up successfully)

	@n@b Responses 
	@n@b Result_t: Possible values are ::CC_DISALLOW_AUXILIARY_CALL, ::RESULT_DIALSTR_INVALID,
	::CC_FAIL_CALL_SESSION, ::CC_FAIL_MAKE_CALL, ::CC_WRONG_CALL_TYPE.
	@n@b MsgType_t: ::MSG_CALL_STATUS_IND 
	@n@b ResultData: ::CallStatusMsg_t
	@n@b MsgType_t: ::MSG_VOICECALL_RELEASE_IND 
	@n@b ResultData: ::VoiceCallReleaseMsg_t
	@n@b MsgType_t: ::MSG_VOICECALL_CONNECTED_IND 
	@n@b ResultData: ::VoiceCallConnectMsg_t

**/

void CAPI2_CC_MakeVoiceCall(UInt32 tid, UInt8 clientID, UInt8* callNum, VoiceCallParam_t voiceCallParam);
void CAPI2_CcApi_MakeVoiceCall(ClientInfo_t* client_info, UInt8* callNum, VoiceCallParam_t voiceCallParam);


//**************************************************************************************
/**
	Function to originate a Data Call

   	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		callNum	(in) Calling Number
	@note
	The client ID is to identify the client which calls this API.
	"callNum" should already be parsed to exclude SS service string.

	The response includes multiple ::MSG_DATACALL_STATUS_IND (unsolicited) messages followed by either
	::MSG_DATACALL_RELEASE_IND (if call can not be set up) or ::MSG_DATACALL_ECDC_IND (unsolicited) and 
	::MSG_DATACALL_CONNECTED_IND (if call is set up successfully)

	@n@b Responses 
	@n@b Result_t: Possible values are ::CC_FAIL_CALL_SESSION, ::CC_FAIL_MAKE_CALL,
	::CC_WRONG_CALL_TYPE, ::RESULT_DIALSTR_INVALID.
	@n@b MsgType_t: ::MSG_DATACALL_STATUS_IND 
	@n@b ResultData: ::DataCallStatusMsg_t
	@n@b MsgType_t: ::MSG_DATACALL_RELEASE_IND 
	@n@b ResultData: ::DataCallReleaseMsg_t
	@n@b MsgType_t: ::MSG_DATACALL_ECDC_IND 
	@n@b ResultData: ::DataECDCLinkMsg_t
	@n@b MsgType_t: ::MSG_DATACALL_CONNECTED_IND 
	@n@b ResultData: ::DataCallConnectMsg_t
**/

void CAPI2_CC_MakeDataCall(UInt32 tid, UInt8 clientID, UInt8* callNum);
void CAPI2_CcApi_MakeDataCall(ClientInfo_t* client_info, UInt8 *callNum);

//**************************************************************************************
/**
	Function to originate a Video Call

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		callNum	(in) Calling Number
	@note
	The client ID is to identify the client which calls this API.
	"callNum" should already be parsed to exclude SS service string.

	The response includes multiple ::MSG_DATACALL_STATUS_IND (unsolicited) messages followed by either
	::MSG_DATACALL_RELEASE_IND (if call can not be set up) or ::MSG_DATACALL_ECDC_IND (unsolicited) and 
	::MSG_DATACALL_CONNECTED_IND (if call is set up successfully)

	@n@b Responses 
	@n@b Result_t: Possible values are ::CC_FAIL_CALL_SESSION, ::CC_FAIL_MAKE_CALL,
	::CC_WRONG_CALL_TYPE, ::RESULT_DIALSTR_INVALID.
	@n@b MsgType_t: ::MSG_DATACALL_STATUS_IND 
	@n@b ResultData: ::DataCallStatusMsg_t
	@n@b MsgType_t: ::MSG_DATACALL_RELEASE_IND 
	@n@b ResultData: ::DataCallReleaseMsg_t
	@n@b MsgType_t: ::MSG_DATACALL_ECDC_IND 
	@n@b ResultData: ::DataECDCLinkMsg_t
	@n@b MsgType_t: ::MSG_DATACALL_CONNECTED_IND 
	@n@b ResultData: ::DataCallConnectMsg_t
**/

void CAPI2_CC_MakeVideoCall(UInt32 tid, UInt8 clientID, UInt8* callNum);
void CAPI2_CcApi_MakeVideoCall(ClientInfo_t* client_info, UInt8 *callNum);

//Call Termination

//**************************************************************************************
/**
	Function to Terminate Call

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		callIdx (in) Call Index of the established Call
	
	@note
	This function is called to terminate the on going call with a specific call index. 
	The response is:
	for voice call: ::MSG_CALL_STATUS_IND (unsolicited) followed by ::MSG_VOICECALL_RELEASE_CNF. 
	for data call: ::MSG_DATACALL_STATUS_IND (unsolicited) followed by ::MSG_DATACALL_RELEASE_IND. 
	
	@n@b Responses 
	@n@b Result_t: Possible values are ::CC_END_CALL_FAIL.
	@n@b MsgType_t: ::MSG_CALL_STATUS_IND 
	@n@b ResultData: ::CallStatusMsg_t
	@n@b MsgType_t: ::MSG_VOICECALL_RELEASE_CNF 
	@n@b ResultData: ::VoiceCallReleaseMsg_t
	@n@b MsgType_t: ::MSG_DATACALL_STATUS_IND 
	@n@b ResultData: ::DataCallStatusMsg_t
	@n@b MsgType_t: ::MSG_DATACALL_RELEASE_IND 
	@n@b ResultData: ::DataCallReleaseMsg_t

**/

void	CAPI2_CC_EndCall(UInt32 tid, UInt8 clientID, UInt8 callIdx);
void CAPI2_CcApi_EndCall(ClientInfo_t* client_info, UInt8 callIdx);

//**************************************************************************************
/**
	Function to Terminate all Calls
	
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@note
	This function is called to terminate all the on-going call. One or multiple of the following
	response sequences are returned:
	for voice call: ::MSG_CALL_STATUS_IND (unsolicited) followed by ::MSG_VOICECALL_RELEASE_CNF. 
	for data call: ::MSG_DATACALL_STATUS_IND followed by ::MSG_DATACALL_RELEASE_IND. 

	@n@b Responses 
	@n@b Result_t: Possible values are ::CC_END_CALL_FAIL.
	@n@b MsgType_t: ::MSG_CALL_STATUS_IND 
	@n@b ResultData: ::CallStatusMsg_t
	@n@b MsgType_t: ::MSG_VOICECALL_RELEASE_CNF 
	@n@b ResultData: ::VoiceCallReleaseMsg_t
	@n@b MsgType_t: ::MSG_DATACALL_STATUS_IND 
	@n@b ResultData: ::DataCallStatusMsg_t
	@n@b MsgType_t: ::MSG_DATACALL_RELEASE_IND 
	@n@b ResultData: ::DataCallReleaseMsg_t
**/

void	CAPI2_CC_EndAllCalls(UInt32 tid, UInt8 clientID);
void CAPI2_CcApi_EndAllCalls(ClientInfo_t* client_info);

//**************************************************************************************
/**
	Function to Terminate Call immediately

	@param		client_info (in)
	@param		callIdx (in) Call Index of the established Call
	
	@note
	This function is called to terminate the on going call with a specific call index. 
	The response is:
	for voice call: ::MSG_CALL_STATUS_IND (unsolicited) followed by ::MSG_VOICECALL_RELEASE_CNF. 
	for data call: ::MSG_DATACALL_STATUS_IND (unsolicited) followed by ::MSG_DATACALL_RELEASE_IND. 
	
	@n@b Responses 
	@n@b Result_t: Possible values are ::CC_END_CALL_FAIL.
	@n@b MsgType_t: ::MSG_CALL_STATUS_IND 
	@n@b ResultData: ::CallStatusMsg_t
	@n@b MsgType_t: ::MSG_VOICECALL_RELEASE_CNF 
	@n@b ResultData: ::VoiceCallReleaseMsg_t
	@n@b MsgType_t: ::MSG_DATACALL_STATUS_IND 
	@n@b ResultData: ::DataCallStatusMsg_t
	@n@b MsgType_t: ::MSG_DATACALL_RELEASE_IND 
	@n@b ResultData: ::DataCallReleaseMsg_t

**/

void CAPI2_CcApi_EndCallImmediate(ClientInfo_t* client_info, UInt8 callIdx);

//**************************************************************************************
/**
	Function to Terminate all Calls immediately
	
	@param		client_info (in)
	@note
	This function is called to terminate all the on-going call. One or multiple of the following
	response sequences are returned:
	for voice call: ::MSG_CALL_STATUS_IND (unsolicited) followed by ::MSG_VOICECALL_RELEASE_CNF. 
	for data call: ::MSG_DATACALL_STATUS_IND followed by ::MSG_DATACALL_RELEASE_IND. 

	@n@b Responses 
	@n@b Result_t: Possible values are ::CC_END_CALL_FAIL.
	@n@b MsgType_t: ::MSG_CALL_STATUS_IND 
	@n@b ResultData: ::CallStatusMsg_t
	@n@b MsgType_t: ::MSG_VOICECALL_RELEASE_CNF 
	@n@b ResultData: ::VoiceCallReleaseMsg_t
	@n@b MsgType_t: ::MSG_DATACALL_STATUS_IND 
	@n@b ResultData: ::DataCallStatusMsg_t
	@n@b MsgType_t: ::MSG_DATACALL_RELEASE_IND 
	@n@b ResultData: ::DataCallReleaseMsg_t
**/

void CAPI2_CcApi_EndAllCallsImmediate( ClientInfo_t* client_info);

//**************************************************************************************
/**
	Function to Terminate All MultiParty Calls

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@note
	Terminate all active multiParty voice Calls. Multiple of the following response
	sequences are returned:
	::MSG_CALL_STATUS_IND (unsolicited) followed by ::MSG_VOICECALL_RELEASE_CNF. 

	@n@b Responses 
	@n@b Result_t: Possible values are ::CC_END_CALL_FAIL.
	@n@b MsgType_t: ::MSG_CALL_STATUS_IND 
	@n@b ResultData: ::CallStatusMsg_t
	@n@b MsgType_t: ::MSG_VOICECALL_RELEASE_CNF 
	@n@b ResultData: ::VoiceCallReleaseMsg_t
**/

void	CAPI2_CC_EndMPTYCalls(UInt32 tid, UInt8 clientID); 
void CAPI2_CcApi_EndMPTYCalls(ClientInfo_t* client_info);


//**************************************************************************************
/**
	Function to Terminate All Held Calls

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@note
	Terminates all held voice alls. One or multiple of the following response
	sequences are returned:
	::MSG_CALL_STATUS_IND (unsolicited) followed by ::MSG_VOICECALL_RELEASE_CNF.

	@n@b Responses 
	@n@b Result_t: Possible values are ::CC_END_CALL_FAIL.
	@n@b MsgType_t: ::MSG_CALL_STATUS_IND 
	@n@b ResultData: ::CallStatusMsg_t
	@n@b MsgType_t: ::MSG_VOICECALL_RELEASE_CNF 
	@n@b ResultData: ::VoiceCallReleaseMsg_t
**/

void	CAPI2_CC_EndHeldCall(UInt32 tid, UInt8 clientID);
void CAPI2_CcApi_EndHeldCall(ClientInfo_t* client_info);

//Call Acceptance
//**************************************************************************************
/**
	Function to Accept a Voice Call

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		callIndex (in) Call Index of the voice call to accept

	@note		Accept a voice call with a given index. The response is one ::MSG_CALL_STATUS_IND (unsolicited) 
	messages followed by either ::MSG_VOICECALL_RELEASE_IND (if call can not be accepted) or 
	::MSG_VOICECALL_CONNECTED_IND (if call is accepted successfully)

	@n@b Responses 
	@n@b Result_t: Possible values are ::CC_ACCEPT_CALL_FAIL.
	@n@b MsgType_t: ::MSG_CALL_STATUS_IND 
	@n@b ResultData: ::CallStatusMsg_t
	@n@b MsgType_t: ::MSG_VOICECALL_RELEASE_IND 
	@n@b ResultData: ::VoiceCallReleaseMsg_t
	@n@b MsgType_t: ::MSG_VOICECALL_CONNECTED_IND 
	@n@b ResultData: ::VoiceCallConnectMsg_t
	
**/

void	CAPI2_CC_AcceptVoiceCall(UInt32 tid, UInt8 clientID, UInt8 callIndex);
void CAPI2_CcApi_AcceptVoiceCall(ClientInfo_t* client_info, UInt8 callIndex);

//**************************************************************************************
/**
	Function to Accept a Data Call

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		callIndex (in) Call Index of the data call to accept

	@note		Accept a data call with a given index. The response is one ::MSG_DATACALL_STATUS_IND (unsolicited) 
	messages followed by either ::MSG_DATACALL_RELEASE_IND (if call can not be accepted) or 
	::MSG_DATACALL_CONNECTED_IND (if call is accepted successfully)

	@n@b Responses 
	@n@b Result_t: Possible values are ::CC_ACCEPT_CALL_FAIL.
	@n@b MsgType_t: ::MSG_DATACALL_STATUS_IND 
	@n@b ResultData: ::DataCallStatusMsg_t
	@n@b MsgType_t: ::MSG_DATACALL_RELEASE_IND 
	@n@b ResultData: ::DataCallReleaseMsg_t
	@n@b MsgType_t: ::MSG_DATACALL_CONNECTED_IND 
	@n@b ResultData: ::DataCallConnectMsg_t

**/

void	CAPI2_CC_AcceptDataCall(UInt32 tid, UInt8 clientID, UInt8 callIndex);
void CAPI2_CcApi_AcceptDataCall(ClientInfo_t* client_info, UInt8 callIndex);

//**************************************************************************************
/**
	Function to Accept the Waiting MT Call
	
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@note		Accept a waiting voice call. The response is one ::MSG_CALL_STATUS_IND (unsolicited) 
	messages followed by either ::MSG_VOICECALL_RELEASE_IND (if call can not be accepted) or 
	::MSG_VOICECALL_CONNECTED_IND (if call is accepted successfully)

	@n@b Responses 
	@n@b Result_t: Possible values are ::CC_ACCEPT_CALL_FAIL.
	@n@b MsgType_t: ::MSG_CALL_STATUS_IND 
	@n@b ResultData: ::CallStatusMsg_t
	@n@b MsgType_t: ::MSG_VOICECALL_RELEASE_IND 
	@n@b ResultData: ::VoiceCallReleaseMsg_t
	@n@b MsgType_t: ::MSG_VOICECALL_CONNECTED_IND 
	@n@b ResultData: ::VoiceCallConnectMsg_t
**/

void	CAPI2_CC_AcceptWaitingCall(UInt32 tid, UInt8 clientID);
void CAPI2_CcApi_AcceptWaitingCall(ClientInfo_t* client_info);

//**************************************************************************************
/**
	Function to Accept a Video Call

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		callIndex (in) Call Index of the video call to accept

	@note		Accept a video call with a given index. The response is one ::MSG_DATACALL_STATUS_IND (unsolicited) 
	messages followed by either ::MSG_DATACALL_RELEASE_IND (if call can not be accepted) or 
	::MSG_DATACALL_CONNECTED_IND (if call is accepted successfully)

	@n@b Responses 
	@n@b Result_t: Possible values are ::CC_ACCEPT_CALL_FAIL.
	@n@b MsgType_t: ::MSG_DATACALL_STATUS_IND 
	@n@b ResultData: ::DataCallStatusMsg_t
	@n@b MsgType_t: ::MSG_DATACALL_RELEASE_IND 
	@n@b ResultData: ::DataCallReleaseMsg_t
	@n@b MsgType_t: ::MSG_DATACALL_CONNECTED_IND 
	@n@b ResultData: ::DataCallConnectMsg_t
**/

void	CAPI2_CC_AcceptVideoCall(UInt32 tid, UInt8 clientID, UInt8 callIndex);
void CAPI2_CcApi_AcceptVideoCall(ClientInfo_t* client_info, UInt8 callIndex);

//Call Progress

//**************************************************************************************
/**
	Function to hold the current voice call

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@note		This function puts the current active call on hold and the call can be
	retrieved to become active call later. The response is a ::MSG_CALL_STATUS_IND (unsolicited) message 
	followed by a ::MSG_VOICECALL_ACTION_RSP message. 

	@n@b Responses 
	@n@b Result_t: Possible values are ::CC_HOLD_CALL_FAIL.
	@n@b MsgType_t: ::MSG_CALL_STATUS_IND 
	@n@b ResultData: ::CallStatusMsg_t
	@n@b MsgType_t: ::MSG_VOICECALL_ACTION_RSP 
	@n@b ResultData: ::VoiceCallActionMsg_t

**/

void	CAPI2_CC_HoldCurrentCall(UInt32 tid, UInt8 clientID);
void CAPI2_CcApi_HoldCurrentCall(ClientInfo_t* client_info);

//**************************************************************************************
/**
	Function to hold a voice call with call index

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		callIndex (in) Call Index of the voice call to hold
	@note		This function puts the active call on hold and the call can be retrieved to 
	become active call later. The response is a ::MSG_CALL_STATUS_IND (unsolicited) message 
	followed by a ::MSG_VOICECALL_ACTION_RSP message. 

	@n@b Responses 
	@n@b Result_t: Possible values are ::CC_HOLD_CALL_FAIL.
	@n@b MsgType_t: ::MSG_CALL_STATUS_IND 
	@n@b ResultData: ::CallStatusMsg_t
	@n@b MsgType_t: ::MSG_VOICECALL_ACTION_RSP 
	@n@b ResultData: ::VoiceCallActionMsg_t

**/

void	CAPI2_CC_HoldCall(UInt32 tid, UInt8 clientID, UInt8 callIndex);
void CAPI2_CcApi_HoldCall(ClientInfo_t* client_info, UInt8 callIndex);

//**************************************************************************************
/**
	Function to retrieve a voice call put on hold last.
	
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	@note		This function retrieves the voice call put on hold last. The response is a 
	::MSG_CALL_STATUS_IND (unsolicited) message followed by a ::MSG_VOICECALL_ACTION_RSP message. 

	@n@b Responses 
	@n@b Result_t: Possible values are ::CC_RESUME_CALL_FAIL.
	@n@b MsgType_t: ::MSG_CALL_STATUS_IND 
	@n@b ResultData: ::CallStatusMsg_t
	@n@b MsgType_t: ::MSG_VOICECALL_ACTION_RSP 
	@n@b ResultData: ::VoiceCallActionMsg_t
**/

void	CAPI2_CC_RetrieveNextHeldCall(UInt32 tid, UInt8 clientID);
void CAPI2_CcApi_RetrieveNextHeldCall(ClientInfo_t* client_info);

//**************************************************************************************
/**
	Function to retrieve a voice call with call index.
	
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		callIndex (in) Call Index of the voice call to retrieve
	
	@note		This function retrieves the voice call with a call index that is put on hold. 
	The response is a ::MSG_CALL_STATUS_IND (unsolicited) message followed by a ::MSG_VOICECALL_ACTION_RSP message. 

	@n@b Responses 
	@n@b Result_t: Possible values are  ::CC_RESUME_CALL_FAIL.
	@n@b MsgType_t: ::MSG_CALL_STATUS_IND 
	@n@b ResultData: ::CallStatusMsg_t
	@n@b MsgType_t: ::MSG_VOICECALL_ACTION_RSP 
	@n@b ResultData: ::VoiceCallActionMsg_t

**/

void	CAPI2_CC_RetrieveCall(UInt32 tid, UInt8 clientID, UInt8 callIndex);
void CAPI2_CcApi_RetrieveCall(ClientInfo_t* client_info, UInt8 callIndex);

//**************************************************************************************
/**
	Function to swap the call, i.e. put the current call on hold and retrieve the held call. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		callIndex (in) Call Index of the held call to retrieve
	@note		This function swaps the call. The response is a ::MSG_CALL_STATUS_IND (unsolicited) message 
	followed by a ::MSG_VOICECALL_ACTION_RSP message. 

	@n@b Responses 
	@n@b Result_t: Possible values are ::CC_SWAP_CALL_FAIL.
	@n@b MsgType_t: ::MSG_CALL_STATUS_IND 
	@n@b ResultData: ::CallStatusMsg_t
	@n@b MsgType_t: ::MSG_VOICECALL_ACTION_RSP 
	@n@b ResultData: ::VoiceCallActionMsg_t

**/

void	CAPI2_CC_SwapCall(UInt32 tid, UInt8 clientID, UInt8 callIndex);
void CAPI2_CcApi_SwapCall(ClientInfo_t* client_info, UInt8 callIndex);

//**************************************************************************************
/**
	Function to split an active call with the call index from the multi-party calls, 
	all other calls will be put on hold. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		callIndex (in) Call Index of call to remain active after split.

	@note		This function splits the call. The response is a ::MSG_CALL_STATUS_IND (unsolicited) message 
	followed by a ::MSG_VOICECALL_ACTION_RSP message.  

	@n@b Responses 
	@n@b Result_t: Possible values are ::CC_SPLIT_CALL_FAIL.
	@n@b MsgType_t: ::MSG_CALL_STATUS_IND 
	@n@b ResultData: ::CallStatusMsg_t
	@n@b MsgType_t: ::MSG_VOICECALL_ACTION_RSP 
	@n@b ResultData: ::VoiceCallActionMsg_t
**/

void	CAPI2_CC_SplitCall(UInt32 tid, UInt8 clientID, UInt8 callIndex);
void CAPI2_CcApi_SplitCall(ClientInfo_t* client_info, UInt8 callIndex);

//**************************************************************************************
/**
	Function to join an active call with the call index to the multi-party calls

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		callIndex (in) Call Index of call to join the multi-party call

	@note		This function joins the call. The response is a ::MSG_CALL_STATUS_IND (unsolicited) message 
	followed by a ::MSG_VOICECALL_ACTION_RSP message.  

	@n@b Responses 
	@n@b Result_t: Possible values are ::CC_JOIN_CALL_FAIL.
	@n@b MsgType_t: ::MSG_CALL_STATUS_IND 
	@n@b ResultData: ::CallStatusMsg_t
	@n@b MsgType_t: ::MSG_VOICECALL_ACTION_RSP 
	@n@b ResultData: ::VoiceCallActionMsg_t

**/

void	CAPI2_CC_JoinCall(UInt32 tid, UInt8 clientID, UInt8 callIndex);
void CAPI2_CcApi_JoinCall(ClientInfo_t* client_info, UInt8 callIndex);

//**************************************************************************************
/**
	Function to Request Explicit Call Transfer to connect an active call with a held call or a waiting call.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		callIndex (in) Call Index of the held or waiting call. 

	@note		This function transfers the call. The response is a ::MSG_CALL_STATUS_IND (unsolicited) message 
	followed by a ::MSG_VOICECALL_ACTION_RSP message.  

	@n@b Responses 
	@n@b Result_t: Possible values are ::CC_TRANS_CALL_FAIL.
	@n@b MsgType_t: ::MSG_CALL_STATUS_IND 
	@n@b ResultData: ::CallStatusMsg_t
	@n@b MsgType_t: ::MSG_VOICECALL_ACTION_RSP 
	@n@b ResultData: ::VoiceCallActionMsg_t

**/

void	CAPI2_CC_TransferCall(UInt32 tid, UInt8 clientID, UInt8 callIndex);
void CAPI2_CcApi_TransferCall(ClientInfo_t* client_info, UInt8 callIndex);

//Call Query

//**************************************************************************************
/**
	Function to get the call index for the current active call.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK
	@n@b MsgType_t: ::MSG_CC_GETCURRENTCALLINDEX_RSP 
	@n@b ResultData: ::UInt8
	
**/

void	CAPI2_CC_GetCurrentCallIndex(UInt32 tid, UInt8 clientID);

//**************************************************************************************
/**
	Function to get the last call index with the active status

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK
	@n@b MsgType_t: ::MSG_CC_GETNEXTACTIVECALLINDEX_RSP 
	@n@b ResultData: ::UInt8
**/

void	CAPI2_CC_GetNextActiveCallIndex(UInt32 tid, UInt8 clientID);

//**************************************************************************************
/**
	Function to get the next call index with the hold status.
	
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK
	@n@b MsgType_t: ::MSG_CC_GETNEXTHELDCALLINDEX_RSP 
	@n@b ResultData: ::UInt8
**/

void	CAPI2_CC_GetNextHeldCallIndex(UInt32 tid, UInt8 clientID);

//**************************************************************************************
/**
	Function to get the next call index with the wait status.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK
	@n@b MsgType_t: ::MSG_CC_GETNEXTWAITCALLINDEX_RSP 
	@n@b ResultData: ::UInt8
**/

void	CAPI2_CC_GetNextWaitCallIndex(UInt32 tid, UInt8 clientID);


//**************************************************************************************
/**
	Function to get the lowest call index of the multi-party calls.
		
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK
	@n@b MsgType_t: ::MSG_CC_GETMPTYCALLINDEX_RSP 
	@n@b ResultData: ::UInt8
**/

void	CAPI2_CC_GetMPTYCallIndex(UInt32 tid, UInt8 clientID); 

//**************************************************************************************
/**
	Function to get the call state of the call with the specified call index.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		callIndex (in) Index of call

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK
	@n@b MsgType_t: ::MSG_CC_GETCALLSTATE_RSP 
	@n@b ResultData: ::CCallState_t
**/

void	CAPI2_CC_GetCallState(UInt32 tid, UInt8 clientID, UInt8 callIndex);

//**************************************************************************************
/**
	Function to get the call type of the call with the specified call index.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		callIndex (in) Index of call

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK
	@n@b MsgType_t: ::MSG_CC_GETCALLTYPE_RSP 
	@n@b ResultData: ::CCallType_t

**/

void	CAPI2_CC_GetCallType(UInt32 tid, UInt8 clientID, UInt8 callIndex);

//**************************************************************************************
/**
	Function to get the call exit cause with the last ended call.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK
	@n@b MsgType_t: ::MSG_CC_GETLASTCALLEXITCAUSE_RSP 
	@n@b ResultData: ::Cause_t
		
**/

void	CAPI2_CC_GetLastCallExitCause(UInt32 tid, UInt8 clientID);

//**************************************************************************************
/**
	Function to get the name of calling party with the call index.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		callIndex (in) Index of call
	
	@n@b Responses 
	@n@b Result_t:	::RESULT_OK
	@n@b MsgType_t: ::MSG_CC_GETCNAPNAME_RSP 
	@n@b ResultData: ::CNAP_NAME_t
**/

void	CAPI2_CC_GetCNAPName(UInt32 tid, UInt8 clientID, UInt8 callIndex); 
void CAPI2_CcApi_GetCNAPName(ClientInfo_t* client_info, UInt8 callIndex);

//**************************************************************************************
/**
	Function to get the phone number of a MO voice or CSD call with the call index.
	For MT voice or CSD call, NULL string for phone number is returned.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		callIndex (in) Index of call

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK or ::RESULT_ERROR
	@n@b MsgType_t: ::MSG_CC_GETCALLNUMBER_RSP 
	@n@b ResultData: ::PHONE_NUMBER_STR_t
	
**/

void	CAPI2_CC_GetCallNumber(UInt32 tid, UInt8 clientID, UInt8 callIndex);

//**************************************************************************************
/**
	Function to get information about the call associated with a call index

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		callIndex (in) Index of call

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK or ::RESULT_ERROR
	@n@b MsgType_t: ::MSG_CC_GETCALLINGINFO_RSP 
	@n@b ResultData: ::CallingInfo_t

**/

void	CAPI2_CC_GetCallingInfo(UInt32 tid, UInt8 clientID, UInt8 callIndex);

//**************************************************************************************
/**
	Function to get the call states of all the calls.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK or ::RESULT_ERROR
	@n@b MsgType_t: ::MSG_CC_GETALLCALLSTATES_RSP 
	@n@b ResultData: ::ALL_CALL_STATE_t
**/

void	CAPI2_CC_GetAllCallStates(UInt32 tid, UInt8 clientID);

//**************************************************************************************
/**
	Function to get indices of all calls.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK or ::RESULT_ERROR
	@n@b MsgType_t: ::MSG_CC_GETALLCALLINDEX_RSP 
	@n@b ResultData: ::ALL_CALL_INDEX_t

**/

void	CAPI2_CC_GetAllCallIndex(UInt32 tid, UInt8 clientID);

//**************************************************************************************
/**
	Function to get indices of all held calls.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK or ::RESULT_ERROR
	@n@b MsgType_t: ::MSG_CC_GETALLHELDCALLINDEX_RSP 
	@n@b ResultData: ::ALL_CALL_INDEX_t
**/

void	CAPI2_CC_GetAllHeldCallIndex(UInt32 tid, UInt8 clientID);

//**************************************************************************************
/**
	Function to get indices of all active calls.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK or ::RESULT_ERROR
	@n@b MsgType_t: ::MSG_CC_GETALLACTIVECALLINDEX_RSP 
	@n@b ResultData: ::ALL_CALL_INDEX_t
**/

void	CAPI2_CC_GetAllActiveCallIndex(UInt32 tid, UInt8 clientID);

//**************************************************************************************
/**
	Function to get all calls indices participating in the multi-party call

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK or ::RESULT_ERROR
	@n@b MsgType_t: ::MSG_CC_GETALLMPTYCALLINDEX_RSP 
	@n@b ResultData: ::ALL_CALL_INDEX_t
**/

void	CAPI2_CC_GetAllMPTYCallIndex(UInt32 tid, UInt8 clientID);


//**************************************************************************************
/**
	Function to get the number of the calls participating in multi-party call.
	
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK 
	@n@b MsgType_t: ::MSG_CC_GETNUMOFMPTYCALLS_RSP 
	@n@b ResultData: ::UInt8
**/

void	CAPI2_CC_GetNumOfMPTYCalls(UInt32 tid, UInt8 clientID); 

//**************************************************************************************
/**
	Function to get the number of active calls.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK 
	@n@b MsgType_t: ::MSG_CC_GETNUMOFACTIVECALLS_RSP 
	@n@b ResultData: ::UInt8
**/

void	CAPI2_CC_GetNumofActiveCalls(UInt32 tid, UInt8 clientID);

//**************************************************************************************
/**
	Function to get the number of held calls.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK 
	@n@b MsgType_t: ::MSG_CC_GETNUMOFHELDCALLS_RSP 
	@n@b ResultData: ::UInt8
**/

void	CAPI2_CC_GetNumofHeldCalls(UInt32 tid, UInt8 clientID);

//**************************************************************************************
/**
	Function to determine if there is a waiting call.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK 
	@n@b MsgType_t: ::MSG_CC_ISTHEREWAITINGCALL_RSP 
	@n@b ResultData: ::Boolean
**/

void	CAPI2_CC_IsThereWaitingCall(UInt32 tid, UInt8 clientID);

//**************************************************************************************
/**
	Function to determine if there is an alerting call.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK 
	@n@b MsgType_t: ::MSG_CC_ISTHEREALERTINGCALL_RSP 
	@n@b ResultData: ::Boolean
**/

void	CAPI2_CC_IsThereAlertingCall(UInt32 tid, UInt8 clientID);


//**************************************************************************************
/**
	Function to get connected line ID with the call index.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		callIndex (in) Index of call

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK or ::RESULT_ERROR
	@n@b MsgType_t: ::MSG_CC_GETCONNECTEDLINEID_RSP 
	@n@b ResultData: ::PHONE_NUMBER_STR_t

**/

void	CAPI2_CC_GetConnectedLineID(UInt32 tid, UInt8 clientID, UInt8 callIndex);
	

//Helper functions
//**************************************************************************************
/**
	Function to determine if a call with the call index is part of multi-party calls.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		callIndex (in) Index of call

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK 
	@n@b MsgType_t: ::MSG_CC_ISMULTIPARTYCALL_RSP 
	@n@b ResultData: ::Boolean

**/

void	CAPI2_CC_IsMultiPartyCall(UInt32 tid, UInt8 clientID, UInt8 callIndex);

//**************************************************************************************
/**
	Function to determine if there is a voice call.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK 
	@n@b MsgType_t: ::MSG_CC_ISTHEREVOICECALL_RSP
	@n@b ResultData: ::Boolean

**/

void	CAPI2_CC_IsThereVoiceCall(UInt32 tid, UInt8 clientID);

//**************************************************************************************
/**
	Function to determine if the connected line ID is allowed to be presented with the call index.
	
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		callIndex (in) the call index to be checked
	
	@n@b Responses 
	@n@b Result_t:	::RESULT_OK 
	@n@b MsgType_t: ::MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_RSP
	@n@b ResultData: ::Boolean

**/

void	CAPI2_CC_IsConnectedLineIDPresentAllowed(UInt32 tid, UInt8 clientID, UInt8 callIndex);


//**************************************************************************************
/**
	Function to get the current on-going call duration in milli-seconds.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		callIndex (in) the call index to be checked
	
	@n@b Responses 
	@n@b Result_t:	::RESULT_OK 
	@n@b MsgType_t: ::MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_RSP
	@n@b ResultData: ::UInt32

**/

void	CAPI2_CC_GetCurrentCallDurationInMilliSeconds(UInt32 tid, UInt8 clientID, UInt8 callIndex);

//**************************************************************************************
/**
	Function to get Current Call Meter with the last ended call which allows the subscriber
	to get information about the cost of calls

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK 
	@n@b MsgType_t: ::MSG_CC_GETLASTCALLCCM_RSP
	@n@b ResultData: ::UInt32

**/

void	CAPI2_CC_GetLastCallCCM(UInt32 tid, UInt8 clientID);

//**************************************************************************************
/**
	Function to get the call duration with the last ended call in ticks. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK 
	@n@b MsgType_t: ::MSG_CC_GETLASTCALLDURATION_RSP
	@n@b ResultData: ::UInt32

**/

void	CAPI2_CC_GetLastCallDuration(UInt32 tid, UInt8 clientID);

//**************************************************************************************
/**
	Function to get the total received bytes for the last ended data call.
	
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	@n@b Responses 
	@n@b Result_t:	::RESULT_OK 
	@n@b MsgType_t: ::MSG_CC_GETLASTDATACALLRXBYTES_RSP
	@n@b ResultData: ::UInt32

**/

void	CAPI2_CC_GetLastDataCallRxBytes(UInt32 tid, UInt8 clientID);

//**************************************************************************************
/**
	Function to get the transmitted bytes for the last ended call.
		
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	@n@b Responses 
	@n@b Result_t:	::RESULT_OK 
	@n@b MsgType_t: ::MSG_CC_GETLASTDATACALLTXBYTES_RSP
	@n@b ResultData: ::UInt32

**/

void	CAPI2_CC_GetLastDataCallTxBytes(UInt32 tid, UInt8 clientID);

//**************************************************************************************
/**
	Function to get the call index of the data call.
	
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	@n@b Responses 
	@n@b Result_t:	::RESULT_OK 
	@n@b MsgType_t: ::MSG_CC_GETDATACALLINDEX_RSP
	@n@b ResultData: ::UInt8

**/

void	CAPI2_CC_GetDataCallIndex(UInt32 tid, UInt8 clientID);

//**************************************************************************************
/**
	Function to get client ID associated with a specific Call index for MO or MT calls.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		callIndex (in) the call index to be checked
	
	@n@b Responses 
	@n@b Result_t:	::RESULT_OK 
	@n@b MsgType_t: ::MSG_CC_GETCALLCLIENTID_RSP
	@n@b ResultData: ::UInt8

**/

void	CAPI2_CC_GetCallClientID(UInt32 tid, UInt8 clientID, UInt8 callIndex);

//**************************************************************************************
/**
	Function to get Type of Address of a phone number for the passed call index. 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		callIndex (in) the call index to be checked

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK 
	@n@b MsgType_t: ::MSG_CC_GETTYPEADD_RSP
	@n@b ResultData: ::UInt8 address type which is specified in GSM 04.08 subclause 10.5.4.7.

**/

void	CAPI2_CC_GetTypeAdd(UInt32 tid, UInt8 clientID, UInt8 callIndex);


//Enable/Disable the auto rejection to the MT voice call 

//**************************************************************************************
/**
	Function to enable or disable the auto reject of User Determined User Busy (UDUB) 
	for the MT voice call.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		enableAutoRej (in) Enable / Disable Auto rejection

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK 
	@n@b MsgType_t: ::MSG_CC_SETVOICECALLAUTOREJECT_RSP
	@n@b ResultData: None

**/

void	CAPI2_CC_SetVoiceCallAutoReject(UInt32 tid, UInt8 clientID, Boolean enableAutoRej);

//**************************************************************************************
/**
	Function to get status of the auto reject of User Determined User Busy (UDUB) to the MT voice call.
	
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK 
	@n@b MsgType_t: ::MSG_CC_ISVOICECALLAUTOREJECT_RSP
	@n@b ResultData: ::Boolean

**/

void	CAPI2_CC_IsVoiceCallAutoReject(UInt32 tid, UInt8 clientID);

//**************************************************************************************
/**
	Function to enable or disable the TTY call. It should be set to TRUE before a TTY 
	MO call will be made or TTY MT call will be received

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		enable (in) TRUE to enable TTY call
	
	@n@b Responses 
	@n@b Result_t:	::RESULT_OK 
	@n@b MsgType_t: ::MSG_CC_SETTTYCALL_RSP
	@n@b ResultData: None

**/
void	CAPI2_CC_SetTTYCall(UInt32 tid, UInt8 clientID, Boolean enable);

//**************************************************************************************
/**
	Function to query if the TTY call is enabled or disabled. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK 
	@n@b MsgType_t: ::MSG_CC_ISTTYENABLE_RSP
	@n@b ResultData: ::Boolean

**/
void	CAPI2_CC_IsTTYEnable(UInt32 tid, UInt8 clientID);

//**************************************************************************************
/**
	Function to query if the call is originated from SIM. 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		callIndex (in) the call index to be checked

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK 
	@n@b MsgType_t: ::MSG_CC_ISSIMORIGINEDCALL_RSP
	@n@b ResultData: ::Boolean 

**/

void	CAPI2_CC_IsSimOriginedCall(UInt32 tid, UInt8 clientID, UInt8 callIndex);

//**************************************************************************************
/**
	Function to set parameters for a video call.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		param (in) Videocall parameters

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK 
	@n@b MsgType_t: ::MSG_CC_SETVIDEOCALLPARAM_RSP
	@n@b ResultData: NONE 

**/
void CAPI2_CC_SetVideoCallParam(UInt32 tid, UInt8 clientID, VideoCallParam_t param);

//**************************************************************************************
/**
	Function to get parameters for a video call.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK 
	@n@b MsgType_t: ::MSG_CC_GETVIDEOCALLPARAM_RSP
	@n@b ResultData: ::VideoCallParam_t 

**/
void CAPI2_CC_GetVideoCallParam(UInt32 tid, UInt8 clientID);


// [oy - function commented out]
/*
	This API function gets the call config params

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cfg (in) Call Cfg param structure

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK 
	@n@b MsgType_t: ::MSG_CC_SETCALLCFG_RSP
	@n@b ResultData: None
**/
//void	CAPI2_CC_SetCallCfg(UInt32 tid, UInt8 clientID, CAPI2_CallConfig_t* cfg);

//**************************************************************************************
/**
	This API function gets the Current Call Meter value of a call. It returns Call Meter Units 
	calculated based on Facility message/IE received from the network if AOC (Advice of Charge) 
	is supported for this call. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		callIndex (in) the call index 

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK, ::RESULT_ERROR
	@n@b MsgType_t: ::MSG_CC_GETCCM_RSP
	@n@b ResultData: ::CallMeterUnit_t
**/
void CAPI2_CC_GetCCM(UInt32 tid, UInt8 clientID, UInt8 callIndex);

//**************************************************************************************
/**
	This API function enables the client to send a DTMF tone. The API would send the
	send DTMF request to the MNCC module. Based on the state of the MNCC state machine
	this request gets handled.

	The response includes multiple MSG_DTMF_STATUS_IND (unsolicited) messages.
	
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		inDtmfObjPtr (in) A pointer to the DTMF object

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK, ::RESULT_WRONG_INDEX, ::CC_WRONG_CALL_TYPE, ::RESULT_BUSY_TRY_LATER, 
	::CC_UNKNOWN_DTMF_TONE, ::RESULT_WRONG_STATE.
	@n@b MsgType_t: ::MSG_CCAPI_SENDDTMF_RSP
	@n@b ResultData: None
	@n@b MsgType_t: ::MSG_DTMF_STATUS_IND
	@n@b ResultData: ::ApiDtmfStatus_t
**/
void CAPI2_CcApi_SendDtmf(UInt32 tid, UInt8 clientID,  ApiDtmf_t* inDtmfObjPtr);


//**************************************************************************************
/**
	This API function enables the client to stop a DTMF tone. The API would send the
	stop DTMF request to the MNCC module. Based on the state of the MNCC state machine
	this request gets handled.

	The response includes multiple MSG_DTMF_STATUS_IND (unsolicited) messages.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		inDtmfObjPtr (in) A pointer to the DTMF object

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK, ::RESULT_WRONG_INDEX, ::RESULT_WRONG_STATE.
	@n@b MsgType_t: ::MSG_CCAPI_STOPDTMF_RSP
	@n@b ResultData: None
	@n@b MsgType_t: ::MSG_DTMF_STATUS_IND
	@n@b ResultData: ::ApiDtmfStatus_t
**/
void CAPI2_CcApi_StopDtmf(UInt32 tid, UInt8 clientID,  ApiDtmf_t* inDtmfObjPtr);


//**************************************************************************************
/**
	This API function lets the client stop a DTMF tone sequence

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		inDtmfObjPtr (in) A pointer to the DTMF object (here callIndex is just relevant)

	The response includes multiple MSG_DTMF_STATUS_IND (unsolicited) messages.

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK 
	@n@b MsgType_t: ::MSG_CCAPI_ABORTDTMF_RSP
	@n@b ResultData: None
	@n@b MsgType_t: ::MSG_DTMF_STATUS_IND
	@n@b ResultData: ::ApiDtmfStatus_t
**/
void CAPI2_CcApi_AbortDtmf(UInt32 tid, UInt8 clientID,  ApiDtmf_t* inDtmfObjPtr);


//**************************************************************************************
/**
	This API function sets the Dtmf timer period.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		inCallIndex (in) The call index. This argument isn't applicable yet.
	@param		inDtmfTimerType (in) The DTMF timer type.
	@param		inDtmfTimeInterval (in) The DTMF timer period.

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK 
	@n@b MsgType_t: ::MSG_CCAPI_SETDTMFTIMER_RSP
	@n@b ResultData: None
**/
void CAPI2_CcApi_SetDtmfTimer(	UInt32		tid, 
								UInt8		clientID, 
								UInt8		inCallIndex,
								DtmfTimer_t	inDtmfTimerType,
								Ticks_t		inDtmfTimeInterval);

//**************************************************************************************
/**
	This API function resets the Dtmf timer period..
		
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		inCallIndex (in) The call index. This argument isn't applicable yet.
	@param		inDtmfTimerType (in) The DTMF timer type.

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK 
	@n@b MsgType_t: ::MSG_CCAPI_RESETDTMFTIMER_RSP
	@n@b ResultData: None
**/
void CAPI2_CcApi_ResetDtmfTimer(	UInt32		tid, 
									UInt8		clientID, 
									UInt8		inCallIndex,
									DtmfTimer_t	inDtmfTimerType);


//**************************************************************************************
/**
	This API function gets the Dtmf duration timer period.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		inCallIndex (in) The call index. This argument isn't applicable yet.
	@param		inDtmfTimerType (in) The DTMF timer type.

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK 
	@n@b MsgType_t: ::MSG_CCAPI_GETDTMFTIMER_RSP
	@n@b ResultData: ::Ticks_t
**/
void CAPI2_CcApi_GetDtmfTimer(UInt32 tid, UInt8 clientID, UInt8 inCallIndex, DtmfTimer_t inDtmfTimerType);

//**************************************************************************************
/**
	This API function gets the TI for a particular callindex

	@param		client_info (in)
	@param		inCallIndex (in) The call index

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK 
	@n@b MsgType_t: ::
	@n@b ResultData: ::Ticks_t
**/
void CAPI2_CcApi_GetTiFromCallIndex( ClientInfo_t* client_info, CallIndex_t inCallIndex);

//**************************************************************************************
/**
	This API function determines if a bearer capability is supported

	@param		client_info (in)
	@param		inBcPtr (in) Bearer capability

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK 
	@n@b MsgType_t: ::
	@n@b ResultData: ::Ticks_t
**/
void CAPI2_CcApi_IsSupportedBC( ClientInfo_t* client_info, BearerCapability_t* inBcPtr);

//**************************************************************************************
/**
	This API function returns the value of Bearer Capability based on the 
	call index of the call

	@param		client_info (in)
	@param		inCallIndex (in) The call index

	@n@b Responses 
	@n@b Result_t:	::RESULT_OK 
	@n@b MsgType_t: ::
	@n@b ResultData: ::Ticks_t
**/
void CAPI2_CcApi_GetBearerCapability( ClientInfo_t* client_info, CallIndex_t inCallIndex);


void CAPI2_CC_IsThereEmergencyCall(UInt32 tid, UInt8 clientID);

void CAPI2_CC_GetCallPresent(UInt32 tid, UInt8 clientID, UInt8 inCallIndex);

void CAPI2_CcApi_GetCallClientInfo(UInt32 tid, UInt8 clientID, UInt8 callIndex);

void CAPI2_CcApi_GetCallClientID(ClientInfo_t* client_info, UInt8 callIndex);

/** @} */

#ifdef __cplusplus
}
#endif

#endif

