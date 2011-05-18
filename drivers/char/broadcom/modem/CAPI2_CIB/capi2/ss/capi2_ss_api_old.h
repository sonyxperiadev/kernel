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
*   @file   capi2_ss_api_old.h
*
*   @brief  This file defines the interface for Supplementary Services API for the CAPI2 interface
*
*			This file defines the interface for supplementary services API.  This file provides
*			the function prototypes necessary to activate, register, interrogate, enable & disable
*			supplementary services such as call forwarding, call barring, calling line presentation
*			for speech, data, fax and other call types. 
********************************************************************************************/
/**

*   @defgroup    CAPI2_SSAPIOLDGroup   Supplementary Services
*   @ingroup     CAPI2_SSOLDGroup
*
*   @brief      This group defines the interfaces to the supplementary services.
*
********************************************************************************************/

#ifndef _CAPI2_SS_API_OLD_H_
#define _CAPI2_SS_API_OLD_H_


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup  CAPI2_SSAPIOLDGroup
 * @{
 */


//-----------------------------------------------------------------------------
//	Call forwarding
//-----------------------------------------------------------------------------


//**************************************************************************************
/**
	Function to Send a call-forwarding configuration request to the network.  

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		mode (in) SS Mode 
	@param		reason (in) Reason for Call forward
	@param		svcCls (in) SS Service Class 
	@param		waitToFwdSec (in) Time to wait before forwarding, in seconds 
	@param		number (in) Call forward number for the mode of enable or registration. 

	@n@b Responses 
	@n@b MsgType_t: ::MSG_SS_CALL_FORWARD_RSP or MSG_SS_CALL_FORWARD_STATUS_RSP
	@n@b Result_t: Possible values are ::RESULT_OK, ::SS_INVALID_SS_REQUEST, ::STK_DATASVRC_BUSY, ::SS_OPERATION_IN_PROGRESS. 
	@n@b ResultData: either ::NetworkCause_t with ::MSG_SS_CALL_FORWARD_RSP or ::CallForwardStatus_t with MSG_SS_CALL_FORWARD_STATUS_RSP
**/

void CAPI2_SS_SendCallForwardReq( 
	UInt32					tid,
	UInt8					clientID, 
	SS_Mode_t				mode, 
	SS_CallFwdReason_t		reason, 
	SS_SvcCls_t				svcCls, 
	UInt8					waitToFwdSec, 
	UInt8*					number) ;

//**************************************************************************************
/**
	Function to Request call-forwarding status from the network.  

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		reason (in) Reason for Call forward
	@param		svcCls (in) SS Service Class 

	@n@b Responses 
	@n@b MsgType_t: ::MSG_SS_CALL_FORWARD_RSP or MSG_SS_CALL_FORWARD_STATUS_RSP
	@n@b Result_t: Possible values are ::RESULT_OK, ::SS_INVALID_SS_REQUEST, ::STK_DATASVRC_BUSY, ::SS_OPERATION_IN_PROGRESS. 
	@n@b ResultData: either ::NetworkCause_t with ::MSG_SS_CALL_FORWARD_RSP or ::CallForwardStatus_t with MSG_SS_CALL_FORWARD_STATUS_RSP
**/

void CAPI2_SS_QueryCallForwardStatus(
	UInt32					tid,
	UInt8					clientID, 
	SS_CallFwdReason_t		reason, 
	SS_SvcCls_t				svcCls );


//-----------------------------------------------------------------------------
//	call barring
//-----------------------------------------------------------------------------

//**************************************************************************************
/**
	Function to Send a call-barring configuration request to the network.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		mode (in) SS_MODE_ENABLE to enable call barring; SS_MODE_DISABLE to disable call barring
	@param		callBarType (in) an enumerator indicating the call barring type.
	@param		svcCls (in) SS Service Class 
	@param		password (in)call barring password. 

	@n@b Responses 
	@n@b MsgType_t: ::MSG_SS_CALL_BARRING_RSP or ::MSG_SS_CALL_BARRING_STATUS_RSP
	@n@b Result_t: Possible values are ::RESULT_OK, ::SS_INVALID_SS_REQUEST, ::STK_DATASVRC_BUSY, ::SS_OPERATION_IN_PROGRESS, ::SS_INVALID_PASSWORD_LENGTH. 
	@n@b ResultData: either ::NetworkCause_t with ::MSG_SS_CALL_BARRING_RSP or ::CallBarringStatus_t with ::MSG_SS_CALL_BARRING_STATUS_RSP
**/

void CAPI2_SS_SendCallBarringReq(
	UInt32					tid,
	UInt8					clientID, 
	SS_Mode_t				mode,
	SS_CallBarType_t		callBarType, 
	SS_SvcCls_t 			svcCls, 
	UInt8*					password) ;

//**************************************************************************************
/**
	Function to Request call-barring status from the network.  

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		callBarType (in) an enumerator indicating the call barring type.
	@param		svcCls (in) SS Service Class 

	@n@b Responses 
	@n@b MsgType_t: ::MSG_SS_CALL_BARRING_RSP or ::MSG_SS_CALL_BARRING_STATUS_RSP
	@n@b Result_t: Possible values are ::RESULT_OK, ::SS_INVALID_SS_REQUEST, ::STK_DATASVRC_BUSY, ::SS_OPERATION_IN_PROGRESS, ::SS_INVALID_PASSWORD_LENGTH. 
	@n@b ResultData: either ::NetworkCause_t with ::MSG_SS_CALL_BARRING_RSP or ::CallBarringStatus_t with ::MSG_SS_CALL_BARRING_STATUS_RSP
**/

void CAPI2_SS_QueryCallBarringStatus(
	UInt32					tid,
	UInt8					clientID,
	SS_CallBarType_t		callBarType, 
	SS_SvcCls_t				svcCls) ;

//**************************************************************************************
/**
	Function to Send a call barring change password request to the network.  

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		callBarType (in) an enumerator indicating the call barring type.
	@param		oldPwd (in) Old Password 
	@param		newPwd (in) New Password 
	@param		reNewPwd (in) Re-enter New Password

	@n@b Responses 
	@n@b MsgType_t: ::MSG_SS_CALL_BARRING_PWD_CHANGE_RSP
	@n@b Result_t: Possible values are ::RESULT_OK, ::SS_INVALID_SS_REQUEST, ::STK_DATASVRC_BUSY, ::SS_OPERATION_IN_PROGRESS, ::SS_INVALID_PASSWORD_LENGTH.
	@n@b ResultData: ::NetworkCause_t
**/

void CAPI2_SS_SendCallBarringPWDChangeReq(
	UInt32					tid,
	UInt8					clientID, 
	SS_CallBarType_t		callBarType, 
	UInt8*					oldPwd, 
	UInt8*					newPwd,
	UInt8*					reNewPwd) ;

//-----------------------------------------------------------------------------
//	call waiting
//-----------------------------------------------------------------------------

//**************************************************************************************
/**
	Function to Send a call-waiting configuration request to the network. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		mode (in) SS_MODE_ENABLE to enable call waiting setting in the network.
	@param		svcCls (in) an enumerator indicating the service class

	@n@b Responses 
	@n@b MsgType_t: ::MSG_SS_CALL_WAITING_RSP or ::MSG_SS_CALL_WAITING_STATUS_RSP 
	@n@b Result_t: Possible values are ::RESULT_OK, ::SS_INVALID_SS_REQUEST, ::STK_DATASVRC_BUSY, ::SS_OPERATION_IN_PROGRESS.
	@n@b ResultData: either ::NetworkCause_t with ::MSG_SS_CALL_WAITING_RSP or ::SS_ActivationStatus_t with ::MSG_SS_CALL_WAITING_STATUS_RSP  
**/

void CAPI2_SS_SendCallWaitingReq(
	UInt32					tid,
	UInt8					clientID, 
	SS_Mode_t				mode, 
	SS_SvcCls_t				svcCls);

//**************************************************************************************
/**
	Function to Request call-waiting status from the network.  

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		svcCls (in) an enumerator indicating the service class

	@n@b Responses 
	@n@b MsgType_t: ::MSG_SS_CALL_WAITING_RSP or ::MSG_SS_CALL_WAITING_STATUS_RSP 
	@n@b Result_t: Possible values are ::RESULT_OK, ::SS_INVALID_SS_REQUEST, ::STK_DATASVRC_BUSY, ::SS_OPERATION_IN_PROGRESS.
	@n@b ResultData: either ::NetworkCause_t with ::MSG_SS_CALL_WAITING_RSP or ::SS_ActivationStatus_t with ::MSG_SS_CALL_WAITING_STATUS_RSP  
**/

void CAPI2_SS_QueryCallWaitingStatus(
	UInt32					tid,
	UInt8					clientID, 
	SS_SvcCls_t				svcCls);

//-----------------------------------------------------------------------------
//	CLIP/COLP/CLIR/CNAP interrogation
//-----------------------------------------------------------------------------

//**************************************************************************************
/**
	Function to Send a request to the network to query the calling line ID provisioin status.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	@n@b Responses 
	@n@b MsgType_t: ::MSG_SS_CALLING_LINE_ID_STATUS_RSP
	@n@b Result_t: Possible values are ::RESULT_OK, ::SS_INVALID_SS_REQUEST, ::STK_DATASVRC_BUSY, ::SS_OPERATION_IN_PROGRESS.
	@n@b ResultData: ::SS_ProvisionStatus_t
**/
void CAPI2_SS_QueryCallingLineIDStatus(UInt32 tid, UInt8 clientID) ;

//**************************************************************************************
/**
	Function to Send a request to the network to query the connected line ID 
	provision status in the network. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@n@b Responses 
	@n@b MsgType_t: ::MSG_SS_CONNECTED_LINE_STATUS_RSP
	@n@b Result_t: Possible values are ::RESULT_OK, ::SS_INVALID_SS_REQUEST, ::STK_DATASVRC_BUSY, ::SS_OPERATION_IN_PROGRESS.
	@n@b ResultData: ::SS_ProvisionStatus_t
**/

void CAPI2_SS_QueryConnectedLineIDStatus(UInt32 tid, UInt8 clientID) ;

//**************************************************************************************
/**
	Function to Send a request to the network to query the calling line restriction  
	provision status in the network.  

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@n@b Responses 
	@n@b MsgType_t: ::MSG_SS_CALLING_LINE_RESTRICTION_STATUS_RSP
	@n@b Result_t: Possible values are ::RESULT_OK, ::SS_INVALID_SS_REQUEST, ::STK_DATASVRC_BUSY, ::SS_OPERATION_IN_PROGRESS.
	@n@b ResultData: ::SS_ProvisionStatus_t
**/

void CAPI2_SS_QueryCallingLineRestrictionStatus(UInt32 tid, UInt8 clientID) ;

//**************************************************************************************
/**
	Function to Send a request to the network to query the connected line restriction 
	provision status.  

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@n@b Responses 
	@n@b MsgType_t: ::MSG_SS_CONNECTED_LINE_RESTRICTION_STATUS_RSP
	@n@b Result_t: Possible values are ::RESULT_OK, ::SS_INVALID_SS_REQUEST, ::STK_DATASVRC_BUSY, ::SS_OPERATION_IN_PROGRESS.
	@n@b ResultData: ::SS_ProvisionStatus_t
**/

void CAPI2_SS_QueryConnectedLineRestrictionStatus(UInt32 tid, UInt8 clientID) ;

//**************************************************************************************
/**
	Function to Send a request to the network to query the calling name presentation 
	status.
	
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@n@b Responses 
	@n@b MsgType_t: ::MSG_SS_CALLING_NAME_PRESENT_STATUS_RSP
	@n@b Result_t: Possible values are ::RESULT_OK, ::SS_INVALID_SS_REQUEST, ::STK_DATASVRC_BUSY, ::SS_OPERATION_IN_PROGRESS.
	@n@b ResultData: ::SS_ProvisionStatus_t
**/

void CAPI2_SS_QueryCallingNAmePresentStatus(UInt32 tid, UInt8 clientID);

//**************************************************************************************
/**
	Function to Send a request to enable/disable Calling Line Presentation status. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		enable (in) TRUE to enable; FALSE to disable

	@n@b Result_t: Possible values are ::RESULT_OK, ::SS_INVALID_SS_REQUEST, ::STK_DATASVRC_BUSY, ::SS_OPERATION_IN_PROGRESS.
	@n@b The response comes back in one of the following two messages:
	@n@b 1. MSG_MS_LOCAL_ELEM_NOTIFY_IND if the status is successfully set. 
	@n@b ResultData: ::MS_LocalElemNotifyInd_t
	@n@b 2. MSG_SS_CALL_REQ_FAIL for failure (e.g. The SS dialling string is blocked by FDN). 
	@n@b ResultData: ::SsCallReqFail_t
**/
void CAPI2_SS_SetCallingLineIDStatus( UInt32 tid, UInt8 clientID,  Boolean enable ) ;

//**************************************************************************************
/**
	Function to Send a request to erase/activate/deactivate Calling Line Restriction status. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		clir_mode (in) mode defined in ::CLIRMode_t

	@n@b Result_t: Possible values are ::RESULT_OK, ::SS_INVALID_SS_REQUEST, ::STK_DATASVRC_BUSY, ::SS_OPERATION_IN_PROGRESS.
	@n@b The response comes back in one of the following two messages:
	@n@b 1. MSG_MS_LOCAL_ELEM_NOTIFY_IND if the status is successfully set. 
	@n@b ResultData: ::MS_LocalElemNotifyInd_t
	@n@b 2. MSG_SS_CALL_REQ_FAIL for failure (e.g. The SS dialling string is blocked by FDN). 
	@n@b ResultData: ::SsCallReqFail_t
**/
void CAPI2_SS_SetCallingLineRestrictionStatus( UInt32 tid, UInt8 clientID,  CLIRMode_t clir_mode ) ;

//**************************************************************************************
/**
	Function to Send a request to enable/disable Connected Line Presentation status. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		enable (in) TRUE to enable; FALSE to disable

	@n@b Result_t: Possible values are ::RESULT_OK, ::SS_INVALID_SS_REQUEST, ::STK_DATASVRC_BUSY, ::SS_OPERATION_IN_PROGRESS.
	@n@b The response comes back in one of the following two messages:
	@n@b 1. MSG_MS_LOCAL_ELEM_NOTIFY_IND if the status is successfully set. 
	@n@b ResultData: ::MS_LocalElemNotifyInd_t
	@n@b 2. MSG_SS_CALL_REQ_FAIL for failure (e.g. The SS dialling string is blocked by FDN). 
	@n@b ResultData: ::SsCallReqFail_t
**/
void CAPI2_SS_SetConnectedLineIDStatus( UInt32 tid, UInt8 clientID,  Boolean enable ) ;

//**************************************************************************************
/**
	Function to Send a request to activate/deactivate Connected Line Restriction status.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		enable (in) TRUE to enable; FALSE to disable

	@n@b Result_t: Possible values are ::RESULT_OK, ::SS_INVALID_SS_REQUEST, ::STK_DATASVRC_BUSY, ::SS_OPERATION_IN_PROGRESS.
	@n@b The response comes back in one of the following two messages:
	@n@b 1. MSG_MS_LOCAL_ELEM_NOTIFY_IND if the status is successfully set. 
	@n@b ResultData: ::MS_LocalElemNotifyInd_t
	@n@b 2. MSG_SS_CALL_REQ_FAIL for failure (e.g. The SS dialling string is blocked by FDN). 
	@n@b ResultData: ::SsCallReqFail_t
**/

void CAPI2_SS_SetConnectedLineRestrictionStatus( UInt32 tid, UInt8 clientID,  Boolean enable ) ;

//-----------------------------------------------------------------------------
//	USSD
//-----------------------------------------------------------------------------

//**************************************************************************************
/**
	Function to Send a USSD connection request to establish a USSD session.  The 
	response to this request is a MSG_USSD_CALLINDEX_IND (unsolicited) followed by a MSG_USSD_DATA_RSP message. 

    @param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		ussd (in) USSD string, 
				- the length is the byte of the ussd string (whether it is 7bit/8bit or ucs2)
				- the dcs is the value according to GSM3.38: (dcs&0x0F)>>2 = 0,1,2 => 7bit, 8bit, ucs2
				- the string is the ussd string.  Even if the dcs is 7bit, the string is to be passed as 8bit (before packing)
	
	@n@b Responses 
	@n@b MsgType_t: ::MSG_USSD_CALLINDEX_IND
	@n@b Result_t: Possible values are ::RESULT_OK, ::STK_DATASVRC_BUSY, ::SS_INVALID_USSD_DCS, ::SS_INVALID_USSD_STRING, ::RESULT_ERROR.
	@n@b ResultData: ::StkReportCallStatus_t

	@n@b MsgType_t: ::MSG_USSD_DATA_RSP
	@n@b Result_t: Possible values are ::RESULT_OK.
	@n@b ResultData: ::USSDataInfo_t
**/
void CAPI2_SS_SendUSSDConnectReq(UInt32 tid, UInt8 clientID, USSDString_t* ussd);


//**************************************************************************************
/**
 Function to End the current USSD session.  
 
 @param  tid (in) Unique exchange/transaction id which is passed back in the response
 @param  clientID (in) Client ID
 @param  ussd_id (in) USSD call ID
 
 @n@b Responses 
 @n@b MsgType_t: ::MSG_SS_ENDUSSDCONNECTREQ_RSP
 @n@b Result_t: Possible values are ::RESULT_OK, ::SS_INVALID_USSD_ID. 
 @n@b ResultData: None
**/

void CAPI2_SS_EndUSSDConnectReq(UInt32 tid, UInt8 clientID, CallIndex_t ussd_id);

//**************************************************************************************
/**
	Function to Send MO USSD data to network.  

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		ussd_id (in) USSD call ID
	@param		dcs (in) dcs supported
	@param		len (in) Length of the USSD string
	@param		ussdString (in) Pointer to USSD String

	@n@b Responses 
	@n@b MsgType_t: ::MSG_USSD_DATA_RSP
	@n@b Result_t: Possible values are ::RESULT_OK, ::SS_INVALID_USSD_STRING. 
	@n@b ResultData: ::USSDataInfo_t
**/

void CAPI2_SS_SendUSSDData(UInt32 tid, UInt8 clientID, CallIndex_t ussd_id, UInt8 dcs, UInt8 len, UInt8* ussdString) ;

/** @} */

//**************************************************************************************
/**
	Function to send a SS Dial Str Service request that pars the SS String, then it initiats a new SS request, which causes the
	MNSS state machine to establish a SS session. 
   
	@param		inClientInfoPtr (in) Client information
	@param		inDialStrSrvReqPtr (in) Pointer to SS API Dial String Service Request Type

	@n@b Responses 
	@n@b Result_t: Possible values are ::RESULT_OK, ::RESULT_ERROR. 
	@n@b MsgType_t: 
	@n@b ResultData: 
**/

void CAPI2_SsApi_DialStrSrvReq( ClientInfo_t *inClientInfoPtr, SsApi_DialStrSrvReq_t *inDialStrSrvReqPtr);

void CAPI2_SsApi_SsSrvReq( ClientInfo_t *inClientInfoPtr, SsApi_SrvReq_t *inApiSrvReqPtr);
void CAPI2_SsApi_UssdSrvReq( ClientInfo_t *inClientInfoPtr, SsApi_UssdSrvReq_t *inUssdSrvReqPtr);
void CAPI2_SsApi_UssdDataReq( ClientInfo_t *inClientInfoPtr, SsApi_UssdDataReq_t *inUssdDataReqPtr);
void CAPI2_SsApi_SsReleaseReq( ClientInfo_t *inClientInfoPtr, SsApi_SrvReq_t *inApiSrvReqPtr);
void CAPI2_SsApi_DataReq( ClientInfo_t *inClientInfoPtr, SsApi_DataReq_t *inDataReqPtr);

void CAPI2_SsApi_ResetSsAlsFlag(UInt32 tid, UInt8 clientID);
void CAPI2_SS_SsApiReqDispatcher(UInt32 tid, UInt8 clientID, SS_SsApiReq_t *inSsApiReqPtr);
void CAPI2_SS_GetStr(UInt32 tid, UInt8 clientID, SS_ConstString_t strName);

void CAPI2_SS_SetClientID(UInt32 tid, UInt8 clientID);
void CAPI2_SS_GetClientID(UInt32 tid, UInt8 clientID);
void CAPI2_SS_ResetClientID(UInt32 tid, UInt8 clientID);


#ifdef __cplusplus
}
#endif

#endif // _CAPI2_SS_API_H_

