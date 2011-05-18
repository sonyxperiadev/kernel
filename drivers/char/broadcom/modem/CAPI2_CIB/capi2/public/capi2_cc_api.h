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
*   @file   capi2_cc_api.h
*
*   @brief  This file defines the types and prototypes for the CAPI2 Call Control API functions.
*
********************************************************************************************/
/**

*   @defgroup   CAPI2_CCAPIGroup   Call Control
*   @ingroup    CAPI2_CallControlGroup
*
*   @brief      This group defines the interfaces to the call control group.
*
*				This file defines the interface for call control API.This file provides
*				the function prototypes necessary to initiate and receive Voice,Data,Video calls.
*				Additionally, it provides functions for call session management such as
*				advice of charge, call meter etc. 
********************************************************************************************/

#ifndef _CAPI2_CALLCTRL_H
#define _CAPI2_CALLCTRL_H


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup CAPI2_CCAPIGroup
 * @{
 */

//***************************************************************************************
/**
	Function to get the call index for the current active call.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_GETCURRENTCALLINDEX_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: UInt8
**/
void CAPI2_CcApi_GetCurrentCallIndex(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get the last call index with the active status
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_GETNEXTACTIVECALLINDEX_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: UInt8
**/
void CAPI2_CcApi_GetNextActiveCallIndex(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get the last call index with the hold status.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_GETNEXTHELDCALLINDEX_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: UInt8
**/
void CAPI2_CcApi_GetNextHeldCallIndex(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get the last call index with the hold status.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_GETNEXTWAITCALLINDEX_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: UInt8
**/
void CAPI2_CcApi_GetNextWaitCallIndex(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get the lowest call index of the multi-party calls.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_GETMPTYCALLINDEX_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: UInt8
**/
void CAPI2_CcApi_GetMPTYCallIndex(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get the Call State of the call with the specified call index.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_GETCALLSTATE_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: ::CCallState_t
**/
void CAPI2_CcApi_GetCallState(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to get the Call Type of the call with the specified call index.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_GETCALLTYPE_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: ::CCallType_t
**/
void CAPI2_CcApi_GetCallType(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to get the call exit cause with the last ended call.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_GETLASTCALLEXITCAUSE_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: ::Cause_t
**/
void CAPI2_CcApi_GetLastCallExitCause(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get the phone number of a MO voice or CSD call with the call index. <br>For MT voice or CSD call this function returns NULL string for phone number.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_GETCALLNUMBER_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_CcApi_GetCallNumber(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to get information about the call associated with a call index
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_GETCALLINGINFO_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_CcApi_GetCallingInfo(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to get the Call States of all the calls.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_GETALLCALLSTATES_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: ::Result_t
**/
void CAPI2_CcApi_GetAllCallStates(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get Indexes of all calls.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_GETALLCALLINDEX_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: ::Result_t
**/
void CAPI2_CcApi_GetAllCallIndex(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get Indexes of all Held calls.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_GETALLHELDCALLINDEX_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: ::Result_t
**/
void CAPI2_CcApi_GetAllHeldCallIndex(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get Indexes of all Active calls.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_GETALLACTIVECALLINDEX_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: ::Result_t
**/
void CAPI2_CcApi_GetAllActiveCallIndex(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get all calls indexes participating in the multi-party call
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_GETALLMPTYCALLINDEX_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: ::Result_t
**/
void CAPI2_CcApi_GetAllMPTYCallIndex(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get the number of the calls participating in multi-party call.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_GETNUMOFMPTYCALLS_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: UInt8
**/
void CAPI2_CcApi_GetNumOfMPTYCalls(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get the number of active calls.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_GETNUMOFACTIVECALLS_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: UInt8
**/
void CAPI2_CcApi_GetNumofActiveCalls(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get the number of Held calls.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_GETNUMOFHELDCALLS_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: UInt8
**/
void CAPI2_CcApi_GetNumofHeldCalls(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Determine if there is a waiting call.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_ISTHEREWAITINGCALL_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_CcApi_IsThereWaitingCall(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Determine if there is an alerting call.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_ISTHEREALERTINGCALL_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_CcApi_IsThereAlertingCall(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get connected line ID whith the call index.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in)  Index of call
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_GETCONNECTEDLINEID_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: UInt8
**/
void CAPI2_CcApi_GetConnectedLineID(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to get Calling Party presentation status
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in)  Index of call
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_GET_CALL_PRESENT_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_CcApi_GetCallPresent(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to get the call index for a particular CC call state.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inCcCallState (in) Param is inCcCallState
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_GET_INDEX_STATE_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_CcApi_GetCallIndexInThisState(ClientInfo_t* inClientInfoPtr, CCallState_t inCcCallState);

//***************************************************************************************
/**
	Function to Determine if a call with the call index is part of multi-party calls.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in)  Index of call
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_ISMULTIPARTYCALL_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_CcApi_IsMultiPartyCall(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to Determine if there is the voice call.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_ISTHEREVOICECALL_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_CcApi_IsThereVoiceCall(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Determine if the connected line ID is allowed to be presented with the call index.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_CcApi_IsConnectedLineIDPresentAllowed(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to Get the current on-going call duration in milli-seconds.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: UInt32
**/
void CAPI2_CcApi_GetCurrentCallDurationInMilliSeconds(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to Get CCM with the last ended call.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_GETLASTCALLCCM_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: UInt32
**/
void CAPI2_CcApi_GetLastCallCCM(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Get the call duration with the last ended call.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_GETLASTCALLDURATION_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: UInt32
**/
void CAPI2_CcApi_GetLastCallDuration(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Get the Received bytes for the last ended call.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_GETLASTDATACALLRXBYTES_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: UInt32
**/
void CAPI2_CcApi_GetLastDataCallRxBytes(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Get the Transmitted bytes for the last ended call.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_GETLASTDATACALLTXBYTES_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: UInt32
**/
void CAPI2_CcApi_GetLastDataCallTxBytes(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Get the Call Index of the Data Call.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_GETDATACALLINDEX_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: UInt8
**/
void CAPI2_CcApi_GetDataCallIndex(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function obtains the client info with the call index.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		callIndex (in) Index of call
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_GETCALLCLIENT_INFO_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: void
**/
void CAPI2_CcApi_GetCallClientInfo(UInt32 tid, UInt8 clientID, UInt8 callIndex);

//***************************************************************************************
/**
	Function to Get Client ID associated with a specific Call index for MO or MT calls.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_GETCALLCLIENTID_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: UInt8
**/
void CAPI2_CcApi_GetCallClientID(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to Get Type of Address.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_GETTYPEADD_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: UInt8
**/
void CAPI2_CcApi_GetTypeAdd(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to Enable or disable the auto reject of  User Determined User Busy UDUB to the MT voice call.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		enableAutoRej (in)  Enable / Disable Auto rejection
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_SETVOICECALLAUTOREJECT_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_CcApi_SetVoiceCallAutoReject(ClientInfo_t* inClientInfoPtr, Boolean enableAutoRej);

//***************************************************************************************
/**
	Function to Get Status of the auto reject of User Determined User Busy UDUB to the MT voice call.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_ISVOICECALLAUTOREJECT_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_CcApi_IsVoiceCallAutoReject(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Enable or disable the TTY call. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		enable (in)  Enable / Disable TTY Call.TRUE: Enable TTY call.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_SETTTYCALL_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_CcApi_SetTTYCall(ClientInfo_t* inClientInfoPtr, Boolean enable);

//***************************************************************************************
/**
	Function to Query if the TTY call is enable or disable. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_ISTTYENABLE_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_CcApi_IsTTYEnable(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to check if the call is SIM originated call.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_ISSIMORIGINEDCALL_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_CcApi_IsSimOriginedCall(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Set Video Call parameters
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		param (in)  Video call parameter
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_SETVIDEOCALLPARAM_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_CcApi_SetVideoCallParam(ClientInfo_t* inClientInfoPtr, VideoCallParam_t param);

//***************************************************************************************
/**
	Function to return video call parameters
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_GETVIDEOCALLPARAM_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: ::VideoCallParam_t
**/
void CAPI2_CcApi_GetVideoCallParam(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get the Current Call Meter value of a call. It returns Call Meter Units <br>calculated based on Facility message/IE received from the network if AOC Advice of Charge <br>is supported for this call.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_GETCCM_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: ::Result_t
**/
void CAPI2_CcApi_GetCCM(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	This API function anables the client to send a DTMF tone. The API would send the <br>send DTMF request to the MNCC module. Based on the state of the MNCC state machine <br>this request gone get handled.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inDtmfObjPtr (in)  A pointer to the DTMF object
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CCAPI_SENDDTMF_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_CcApi_SendDtmfTone(ClientInfo_t* inClientInfoPtr, ApiDtmf_t *inDtmfObjPtr);

//***************************************************************************************
/**
	This API function anables the client to stop a DTMF tone. The API would send the <br>stop DTMF request to the MNCC module. Based on the state of the MNCC state machine <br>this request gone get handled.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inDtmfObjPtr (in) A pointer to the DTMF object.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CCAPI_STOPDTMF_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_CcApi_StopDtmfTone(ClientInfo_t* inClientInfoPtr, ApiDtmf_t *inDtmfObjPtr);

//***************************************************************************************
/**
	This API function anables the client to stop a DTMF tone sequence for now.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		inDtmfObjPtr (in) A pointer to the DTMF object.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CCAPI_ABORTDTMF_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_CcApi_AbortDtmf(UInt32 tid, UInt8 clientID, ApiDtmf_t *inDtmfObjPtr);

//***************************************************************************************
/**
	This API function sets the Dtmf timer period.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inCallIndex (in) The call index
	@param		inDtmfTimerType (in)  The DTMF timer type.
	@param		inDtmfTimeInterval (in)  The DTMF timer period.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CCAPI_SETDTMFTIMER_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_CcApi_SetDtmfToneTimer(ClientInfo_t* inClientInfoPtr, UInt8 inCallIndex, DtmfTimer_t inDtmfTimerType, Ticks_t inDtmfTimeInterval);

//***************************************************************************************
/**
	This API function resets the DTMF timer period based on the type of timer.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inCallIndex (in) The call index
	@param		inDtmfTimerType (in)  DTMF timer type
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CCAPI_RESETDTMFTIMER_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_CcApi_ResetDtmfToneTimer(ClientInfo_t* inClientInfoPtr, UInt8 inCallIndex, DtmfTimer_t inDtmfTimerType);

//***************************************************************************************
/**
	This API function gets the Dtmf duration timer period.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inCallIndex (in) The call index
	@param		inDtmfTimerType (in)  The DTMF timer type.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CCAPI_GETDTMFTIMER_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: ::Ticks_t
**/
void CAPI2_CcApi_GetDtmfToneTimer(ClientInfo_t* inClientInfoPtr, UInt8 inCallIndex, DtmfTimer_t inDtmfTimerType);

//***************************************************************************************
/**
	Function to get the TI of a particular call index
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inCallIndex (in) The call index
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CCAPI_GETTIFROMCALLINDEX_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: ::Result_t
**/
void CAPI2_CcApi_GetTiFromCallIndex(ClientInfo_t* inClientInfoPtr, CallIndex_t inCallIndex);

//***************************************************************************************
/**
	Function to determine if a bearer capability is supported
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inBcPtr (in)  Bearer Capability
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CCAPI_IS_SUPPORTEDBC_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_CcApi_IsSupportedBC(ClientInfo_t* inClientInfoPtr, BearerCapability_t *inBcPtr);

//***************************************************************************************
/**
	This API function gets the bearer capability per callindex
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inCallIndex (in)  used to access the bc per call
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CCAPI_IS_BEARER_CAPABILITY_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: ::Result_t
**/
void CAPI2_CcApi_GetBearerCapability(ClientInfo_t* inClientInfoPtr, CallIndex_t inCallIndex);

//***************************************************************************************
/**
	Function to originate a Voice Call
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callNum (in)  Calling Number
	@param		voiceCallParam (in)  Voice Call Parameters.
	
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
void CAPI2_CcApi_MakeVoiceCall(ClientInfo_t* inClientInfoPtr, UInt8* callNum, VoiceCallParam_t voiceCallParam);

//***************************************************************************************
/**
	Function to MakeDataCall
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callNum (in) Param is callNum
	
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
void CAPI2_CcApi_MakeDataCall(ClientInfo_t* inClientInfoPtr, UInt8* callNum);

//***************************************************************************************
/**
	Function to originate a Data Call
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callNum (in)  Calling Number
	
	 @n@b Responses 
	 @n@b MsgType_t: 
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_CcApi_MakeFaxCall(ClientInfo_t* inClientInfoPtr, UInt8* callNum);

//***************************************************************************************
/**
	Function to originate a Video Call
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callNum (in) Calling Number
	
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
void CAPI2_CcApi_MakeVideoCall(ClientInfo_t* inClientInfoPtr, UInt8* callNum);

//***************************************************************************************
/**
	Function to Terminate Call
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIdx (in)  Call Index of the established Call
	
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
void CAPI2_CcApi_EndCall(ClientInfo_t* inClientInfoPtr, UInt8 callIdx);

//***************************************************************************************
/**
	Function to Terminate All Calls
	@param		inClientInfoPtr (in) Client Information Pointer.
	
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
void CAPI2_CcApi_EndAllCalls(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Terminate Call immediately
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIdx (in)  Call Index of the established Call
	
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
void CAPI2_CcApi_EndCallImmediate(ClientInfo_t* inClientInfoPtr, UInt8 callIdx);

//***************************************************************************************
/**
	Function to Terminate All Calls immediately
	@param		inClientInfoPtr (in) Client Information Pointer.
	
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
void CAPI2_CcApi_EndAllCallsImmediate(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Terminate All MultiParty Calls
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	@n@b Responses 
	@n@b Result_t: Possible values are ::CC_END_CALL_FAIL.
	@n@b MsgType_t: ::MSG_CALL_STATUS_IND 
	@n@b ResultData: ::CallStatusMsg_t
	@n@b MsgType_t: ::MSG_VOICECALL_RELEASE_CNF 
	@n@b ResultData: ::VoiceCallReleaseMsg_t
**/
void CAPI2_CcApi_EndMPTYCalls(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Terminate All Held Calls
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	@n@b Responses 
	@n@b Result_t: Possible values are ::CC_END_CALL_FAIL.
	@n@b MsgType_t: ::MSG_CALL_STATUS_IND 
	@n@b ResultData: ::CallStatusMsg_t
	@n@b MsgType_t: ::MSG_VOICECALL_RELEASE_CNF 
	@n@b ResultData: ::VoiceCallReleaseMsg_t
**/
void CAPI2_CcApi_EndHeldCall(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Accept a Voice Call
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	@n@b Responses 
	@n@b Result_t: Possible values are ::CC_ACCEPT_CALL_FAIL.
	@n@b MsgType_t: ::MSG_CALL_STATUS_IND 
	@n@b ResultData: ::CallStatusMsg_t
	@n@b MsgType_t: ::MSG_VOICECALL_RELEASE_IND 
	@n@b ResultData: ::VoiceCallReleaseMsg_t
	@n@b MsgType_t: ::MSG_VOICECALL_CONNECTED_IND 
	@n@b ResultData: ::VoiceCallConnectMsg_t
**/
void CAPI2_CcApi_AcceptVoiceCall(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to Accept a Data Call
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	@n@b Responses 
	@n@b Result_t: Possible values are ::CC_ACCEPT_CALL_FAIL.
	@n@b MsgType_t: ::MSG_DATACALL_STATUS_IND 
	@n@b ResultData: ::DataCallStatusMsg_t
	@n@b MsgType_t: ::MSG_DATACALL_RELEASE_IND 
	@n@b ResultData: ::DataCallReleaseMsg_t
	@n@b MsgType_t: ::MSG_DATACALL_CONNECTED_IND 
	@n@b ResultData: ::DataCallConnectMsg_t
	
**/
void CAPI2_CcApi_AcceptDataCall(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to Accept the Waiting MT Call
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	@n@b Responses 
	@n@b Result_t: Possible values are ::CC_ACCEPT_CALL_FAIL.
	@n@b MsgType_t: ::MSG_CALL_STATUS_IND 
	@n@b ResultData: ::CallStatusMsg_t
	@n@b MsgType_t: ::MSG_VOICECALL_RELEASE_IND 
	@n@b ResultData: ::VoiceCallReleaseMsg_t
	@n@b MsgType_t: ::MSG_VOICECALL_CONNECTED_IND 
	@n@b ResultData: ::VoiceCallConnectMsg_t
**/
void CAPI2_CcApi_AcceptWaitingCall(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Accept a Video Call
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	@n@b Responses 
	@n@b Result_t: Possible values are ::CC_ACCEPT_CALL_FAIL.
	@n@b MsgType_t: ::MSG_DATACALL_STATUS_IND 
	@n@b ResultData: ::DataCallStatusMsg_t
	@n@b MsgType_t: ::MSG_DATACALL_RELEASE_IND 
	@n@b ResultData: ::DataCallReleaseMsg_t
	@n@b MsgType_t: ::MSG_DATACALL_CONNECTED_IND 
	@n@b ResultData: ::DataCallConnectMsg_t
**/
void CAPI2_CcApi_AcceptVideoCall(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to Hold Current Active Call
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_VOICECALL_ACTION_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_CcApi_HoldCurrentCall(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Hold Call with Call Index
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_VOICECALL_ACTION_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_CcApi_HoldCall(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to Retrieve Last Call with Held Status
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_VOICECALL_ACTION_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_CcApi_RetrieveNextHeldCall(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Retrieve Call with Call Index
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_VOICECALL_ACTION_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_CcApi_RetrieveCall(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to Swap the call with the held call index to the active status
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_VOICECALL_ACTION_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_CcApi_SwapCall(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to Split an active call with the call index from the multi-party calls
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_VOICECALL_ACTION_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_CcApi_SplitCall(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to Join an active call with the call index to the multi-party calls
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_VOICECALL_ACTION_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_CcApi_JoinCall(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to Request Explicit Call Transfer to connect an active call with a held <br>call or a waiting call.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_VOICECALL_ACTION_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_CcApi_TransferCall(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to get the name of calling party with the call index.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_GETCNAPNAME_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: ::CNAP_NAME_t
**/
void CAPI2_CcApi_GetCNAPName(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to IsCurrentStateMpty
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_ISCURRENTSTATEMPTY_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_CcApi_IsCurrentStateMpty(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Set call control configurable element
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inElement (in)  Element type
	@param		inCompareObjPtr (in)  pointer to the compare object
	@param		outElementPtr (in)  pointer to the element content
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CCAPI_SET_ELEMENT_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_CcApi_SetElement(ClientInfo_t* inClientInfoPtr, CcApi_Element_t inElement, void *inCompareObjPtr, void *outElementPtr);


//***************************************************************************************
/**
	Gets call control configurable element
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inElement (in)  Element type
	@param		inCompareObjPtr (in)  pointer to the compare object
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CCAPI_GET_ELEMENT_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: ::Result_t
**/
void CAPI2_CcApi_GetElement(ClientInfo_t* inClientInfoPtr, CcApi_Element_t inElement, void *inCompareObjPtr);

//***************************************************************************************
/**
	This API function anables the client to stop a DTMF tone sequence for now.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inDtmfObjPtr (in) A pointer to the DTMF object.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CCAPI_ABORTDTMF_TONE_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_CcApi_AbortDtmfTone(ClientInfo_t* inClientInfoPtr, ApiDtmf_t *inDtmfObjPtr);

//***************************************************************************************
/**
	Function to check whether there is ongoing emergency call.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_CC_IS_THERE_EMERGENCY_CALL_RSP
	 @n@b Result_t:		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_CcApi_IsThereEmergencyCall(ClientInfo_t* inClientInfoPtr);



/** @} */

#ifdef __cplusplus
}
#endif

#endif

