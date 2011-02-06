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
*   @file   capi2_ss_api.c
*
*   @brief  This file implements the interface for Supplementary Services API for the CAPI2 interface
*
********************************************************************************************/
/**

*   @defgroup    CAPI2_SSAPIGroup   Supplementary Services
*   @ingroup     CAPI2_SSGroup
*
*   @brief      This group defines the interfaces to the supplementary services.
*
*				This file defines the interface for supplementary services API.  This file provides
*				the function prototypes necessary to activate, register, interrogate, enable & disable
*				supplementary services such as call forwarding, call barring, calling line presentation
*				for speech, data, fax and other call types. 
********************************************************************************************/



#include "capi2_msnu.h"
#include "capi2_mstruct.h"
#include "capi2_mstypes.h"
#include "capi2_resultcode.h"
#include "capi2_taskmsgs.h"
#include "capi2_types.h"
#include "capi2_ss_api.h"
#include	"capi2_reqrep.h"

/**
 * @addtogroup  CAPI2_SSAPIGroup
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
	UInt8*					number) 
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SS_SendCallForwardReq_Req.mode = mode;
	req.req_rep_u.CAPI2_SS_SendCallForwardReq_Req.reason = reason;
	req.req_rep_u.CAPI2_SS_SendCallForwardReq_Req.svcCls = svcCls;
	req.req_rep_u.CAPI2_SS_SendCallForwardReq_Req.waitToFwdSec = waitToFwdSec;
	req.req_rep_u.CAPI2_SS_SendCallForwardReq_Req.number.len = (number)?(strlen((const char*)number)+1):0;
	req.req_rep_u.CAPI2_SS_SendCallForwardReq_Req.number.str = (char*) number;
	req.respId = MSG_SS_CALL_FORWARD_STATUS_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_SENDCALLFORWARDREQ_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}
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
	SS_SvcCls_t				svcCls )
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SS_QueryCallForwardStatus_Req.reason = reason;
	req.req_rep_u.CAPI2_SS_QueryCallForwardStatus_Req.svcCls = svcCls;
	req.respId = MSG_SS_CALL_FORWARD_STATUS_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_QUERYCALLFORWARDSTATUS_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}
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
	UInt8*					password) 
{

	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SS_SendCallBarringReq_Req.mode = mode;
	req.req_rep_u.CAPI2_SS_SendCallBarringReq_Req.callBarType = callBarType;
	req.req_rep_u.CAPI2_SS_SendCallBarringReq_Req.svcCls = svcCls;
	req.req_rep_u.CAPI2_SS_SendCallBarringReq_Req.password.len = (password)?(strlen((const char*)password)+1):0;
	req.req_rep_u.CAPI2_SS_SendCallBarringReq_Req.password.str = (char*) password;
	req.respId = MSG_SS_CALL_BARRING_STATUS_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_SENDCALLBARRINGREQ_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//**************************************************************************************
/**
	Function to Request call-barring status from the network.  

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		callBarType (in) an enumerator indicating the call barring type.
	@param		svcCls (in) SS Service Class 

	@n@b Responses 
	@n@b MsgType_t: ::MSG_SS_CALL_BARRING_RSP or ::MSG_SS_CALL_BARRING_STATUS_RSP
	@n@b Result_t: Possible values are ::RESULT_OK, ::SS_INVALID_SS_REQUEST, ::STK_DATASVRC_BUSY, ::SS_OPERATION_IN_PROGRESS. 
	@n@b ResultData: either ::NetworkCause_t with ::MSG_SS_CALL_BARRING_RSP or ::CallBarringStatus_t with ::MSG_SS_CALL_BARRING_STATUS_RSP
**/

void CAPI2_SS_QueryCallBarringStatus(
	UInt32					tid,
	UInt8					clientID,
	SS_CallBarType_t		callBarType, 
	SS_SvcCls_t				svcCls) 
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SS_QueryCallBarringStatus_Req.callBarType = callBarType;
	req.req_rep_u.CAPI2_SS_QueryCallBarringStatus_Req.svcCls = svcCls;
	req.respId = MSG_SS_CALL_BARRING_STATUS_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_QUERYCALLBARRINGSTATUS_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

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
	UInt8*					reNewPwd)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SS_SendCallBarringPWDChangeReq_Req.callBarType = callBarType;
	req.req_rep_u.CAPI2_SS_SendCallBarringPWDChangeReq_Req.oldPwd.len = (oldPwd)?(strlen((const char*)oldPwd)+1):0;
	req.req_rep_u.CAPI2_SS_SendCallBarringPWDChangeReq_Req.oldPwd.str = (char*) oldPwd;
	req.req_rep_u.CAPI2_SS_SendCallBarringPWDChangeReq_Req.newPwd.len = (newPwd)?(strlen((const char*)newPwd)+1):0;
	req.req_rep_u.CAPI2_SS_SendCallBarringPWDChangeReq_Req.newPwd.str = (char*) newPwd;
	req.req_rep_u.CAPI2_SS_SendCallBarringPWDChangeReq_Req.reNewPwd.len = (reNewPwd)?(strlen((const char*)reNewPwd)+1):0;
	req.req_rep_u.CAPI2_SS_SendCallBarringPWDChangeReq_Req.reNewPwd.str = (char*) reNewPwd;
	req.respId = MSG_SS_CALL_BARRING_PWD_CHANGE_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_SENDCALLBARRINGPWDCHANGEREQ_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

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
	SS_SvcCls_t				svcCls)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SS_SendCallWaitingReq_Req.mode = mode;
	req.req_rep_u.CAPI2_SS_SendCallWaitingReq_Req.svcCls = svcCls;
	req.respId = MSG_SS_CALL_WAITING_STATUS_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_SENDCALLWAITINGREQ_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

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
	SS_SvcCls_t				svcCls)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SS_QueryCallWaitingStatus_Req.svcCls = svcCls;
	req.respId = MSG_SS_CALL_WAITING_STATUS_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_QUERYCALLWAITINGSTATUS_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

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
void CAPI2_SS_QueryCallingLineIDStatus(UInt32 tid, UInt8 clientID) 
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.respId = MSG_SS_CALLING_LINE_ID_STATUS_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_QUERYCALLINGLINEIDSTATUS_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}


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

void CAPI2_SS_QueryConnectedLineIDStatus(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.respId = MSG_SS_CONNECTED_LINE_STATUS_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_QUERYCONNECTEDLINEIDSTATUS_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

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

void CAPI2_SS_QueryCallingLineRestrictionStatus(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.respId = MSG_SS_CALLING_LINE_RESTRICTION_STATUS_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_QUERYCALLINGLINERESTRICTIONSTATUS_REQ, clientID, result, &stream, &len);
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

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

void CAPI2_SS_QueryConnectedLineRestrictionStatus(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.respId = MSG_SS_CONNECTED_LINE_RESTRICTION_STATUS_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_QUERYCONNECTEDLINERESTRICTIONSTATUS_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

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

void CAPI2_SS_QueryCallingNamePresentStatus(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.respId = MSG_SS_CALLING_NAME_PRESENT_STATUS_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_QUERYCALLINGNAMEPRESENTSTATUS_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}
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
void CAPI2_SS_SetCallingLineIDStatus(UInt32 tid, UInt8 clientID, Boolean enable)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SS_SetCallingLineIDStatus_Req.enable = enable;
	req.respId = MSG_MS_LOCAL_ELEM_NOTIFY_IND;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_SETCALLINGLINEIDSTATUS_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

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
void CAPI2_SS_SetCallingLineRestrictionStatus(UInt32 tid, UInt8 clientID, CLIRMode_t clir_mode)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SS_SetCallingLineRestrictionStatus_Req.clir_mode = clir_mode;
	req.respId = MSG_MS_LOCAL_ELEM_NOTIFY_IND;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_SETCALLINGLINERESTRICTIONSTATUS_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}


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
void CAPI2_SS_SetConnectedLineIDStatus(UInt32 tid, UInt8 clientID, Boolean enable)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SS_SetConnectedLineIDStatus_Req.enable = enable;
	req.respId = MSG_MS_LOCAL_ELEM_NOTIFY_IND;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_SETCONNECTEDLINEIDSTATUS_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

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

void CAPI2_SS_SetConnectedLineRestrictionStatus(UInt32 tid, UInt8 clientID, Boolean enable)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SS_SetConnectedLineRestrictionStatus_Req.enable = enable;
	req.respId = MSG_MS_LOCAL_ELEM_NOTIFY_IND;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_SETCONNECTEDLINERESTRICTIONSTATUS_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}
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
void CAPI2_SS_SendUSSDConnectReq(UInt32 tid, UInt8 clientID, USSDString_t* ussd)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SS_SendUSSDConnectReq_Req.ussd = ussd;
	req.respId = MSG_USSD_DATA_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_SENDUSSDCONNECTREQ_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

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

void CAPI2_SS_EndUSSDConnectReq(UInt32 tid, UInt8 clientID, CallIndex_t ussd_id)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SS_EndUSSDConnectReq_Req.ussd_id = ussd_id;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_ENDUSSDCONNECTREQ_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

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

void CAPI2_SS_SendUSSDData(UInt32 tid, UInt8 clientID, CallIndex_t ussd_id, UInt8 dcs, UInt8 dlen, UInt8* ussdString)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SS_SendUSSDData_Req.ussd_id = ussd_id;
	req.req_rep_u.CAPI2_SS_SendUSSDData_Req.dcs = dcs;
	req.req_rep_u.CAPI2_SS_SendUSSDData_Req.len = dlen;
	req.req_rep_u.CAPI2_SS_SendUSSDData_Req.ussdString = ussdString;
	req.respId = MSG_USSD_DATA_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_SENDUSSDDATA_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

/** @} */

void CAPI2_SS_ResetSsAlsFlag(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	/* NONE */

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SS_RESETSSALSFLAG_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

void CAPI2_SsApi_DialStrSrvReq(UInt32 tid, UInt8 clientID, ClientInfo_t *inClientInfoPtr, SsApi_DialStrSrvReq_t *inDialStrSrvReqPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SsApi_DialStrSrvReq_Req.inClientInfoPtr = inClientInfoPtr;
	req.req_rep_u.CAPI2_SsApi_DialStrSrvReq_Req.inDialStrSrvReqPtr = inDialStrSrvReqPtr;

	req.respId = MSG_SSAPI_DIALSTRSRVREQ_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SSAPI_DIALSTRSRVREQ_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

void CAPI2_SsApi_SsSrvReq(UInt32 tid, UInt8 clientID, ClientInfo_t *inClientInfoPtr, SsApi_SrvReq_t *inApiSrvReqPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SsApi_SsSrvReq_Req.inClientInfoPtr = inClientInfoPtr;
	req.req_rep_u.CAPI2_SsApi_SsSrvReq_Req.inApiSrvReqPtr = inApiSrvReqPtr;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SSAPI_SSSRVREQ_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

void CAPI2_SsApi_UssdSrvReq(UInt32 tid, UInt8 clientID, ClientInfo_t *inClientInfoPtr, SsApi_UssdSrvReq_t *inUssdSrvReqPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SsApi_UssdSrvReq_Req.inClientInfoPtr = inClientInfoPtr;
	req.req_rep_u.CAPI2_SsApi_UssdSrvReq_Req.inUssdSrvReqPtr = inUssdSrvReqPtr;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SSAPI_USSDSRVREQ_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

void CAPI2_SsApi_UssdDataReq(UInt32 tid, UInt8 clientID, ClientInfo_t *inClientInfoPtr, SsApi_UssdDataReq_t *inUssdDataReqPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SsApi_UssdDataReq_Req.inClientInfoPtr = inClientInfoPtr;
	req.req_rep_u.CAPI2_SsApi_UssdDataReq_Req.inUssdDataReqPtr = inUssdDataReqPtr;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SSAPI_USSDDATAREQ_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

void CAPI2_SsApi_SsReleaseReq(UInt32 tid, UInt8 clientID, ClientInfo_t *inClientInfoPtr, SsApi_SrvReq_t *inApiSrvReqPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SsApi_SsReleaseReq_Req.inClientInfoPtr = inClientInfoPtr;
	req.req_rep_u.CAPI2_SsApi_SsReleaseReq_Req.inApiSrvReqPtr = inApiSrvReqPtr;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SSAPI_SSRELEASEREQ_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

void CAPI2_SsApi_DataReq(UInt32 tid, UInt8 clientID, ClientInfo_t *inClientInfoPtr, SsApi_DataReq_t *inDataReqPtr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SsApi_DataReq_Req.inClientInfoPtr = inClientInfoPtr;
	req.req_rep_u.CAPI2_SsApi_DataReq_Req.inDataReqPtr = inDataReqPtr;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SSAPI_DATAREQ_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}
