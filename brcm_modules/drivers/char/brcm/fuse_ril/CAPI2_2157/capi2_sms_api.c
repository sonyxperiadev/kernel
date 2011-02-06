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
*   @file   capi2_sms_api.c
*
*   @brief  This file implementa the interface for CAPI2 SMS API.
*
****************************************************************************/
/**

*   @defgroup   CAPI2_SMSAPIGroup   Short Message Services
*   @ingroup    CAPI2_SMSGroup
*
*   @brief      This group defines the interfaces to the SMS system and provides
*				API documentation needed to create short message service applications.  
*				This group also deals with Cell Broadcast and VoiceMail indication services.
*				The APIs provided enables the user to write applications to create, store,
*				save, send and display SMS and Cell Broadcst messages.
*	
****************************************************************************/

#include "capi2_sms_ds.h"
#include "capi2_reqrep.h"

#include "capi2_sms_api.h"

//#define BYPASS_IPC
//-------------------------------------------------
// Function Prototype
//-------------------------------------------------

/**
 * @addtogroup CAPI2_SMSAPIGroup
 * @{
 */

//**************************************************************************************
/**
	Set Service Number in SIM function
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		*sca (in) Service Center Address
	@param		rec_no (in) Record Number 
	@note
	Record Number - SCA index. Number of SCA records supported depends on specific SIM cards.
	If default SCA is to be used, use ::USE_DEFAULT_SCA_NUMBER as rec_no. If multiple SCA
	need to be used, call ::CAPI2_SIM_GetSmsParamRecNum(UInt8* numRec) to get the number of SCA
	first before specify a record number other than the default number.
	@n@b Responses 
	@n@b MsgType_t : ::MSG_SMS_SENDSMSSRVCENTERNUMBERUPDATEREQ_RSP
	@n@b Result_t : Possible values are ::SMS_SIM_BUSY, ::RESULT_OK, ::SMS_INVALID_SCA_ADDRESS,
	::SMS_SCA_INVALID_CHAR_INSTR, ::RESULT_PENDING
	@n@b ResultData : None
**/

void CAPI2_SMS_SendSMSSrvCenterNumberUpdateReq(UInt32 tid, UInt8 clientID, SmsAddress_t* sca, UInt8 rec_no)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_SendSMSSrvCenterNumberUpdateReq_Req.rec_no = rec_no;
	req.req_rep_u.CAPI2_SMS_SendSMSSrvCenterNumberUpdateReq_Req.sca = sca;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_SENDSMSSRVCENTERNUMBERUPDATE_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
	Get SMS service center number function
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		rec_no (in) Record Number
	@return		Result_t
	@note
	* Record Number - SCA index. Number of SCA records supported depends on specific SIM cards.
	* SCA is defined as follows:
	If default SCA is to be used, use ::USE_DEFAULT_SCA_NUMBER as rec_no.  If multiple SCA
	need to be used, call CAPI2_SIM_GetSmsParamRecNum(UInt8* numRec) to get the number of SCA
	first before specify a record number other than the default number.
	@n@b Responses 
	@n@b MsgType_t : ::MSG_SMS_GETSMSSRVCENTERNUMBER_RSP
	@n@b Result_t
	@n@b ResultData : Refer ::SmsAddress_t
**/

void CAPI2_SMS_GetSMSrvCenterNumber(UInt32 tid, UInt8 clientID, UInt8 rec_no)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_GetSMSrvCenterNumber_Req.rec_no = rec_no;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_GETSMSSRVCENTERNUMBER_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**	
	Get SMS Capacity Exceeded flag stored in EFsmss function
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		 Result_t
	@note 
	@n@b Responses 
	@n@b MsgType_t : ::MSG_SMS_GETSIMSMSCAPACITYEXCEEDEDFLAG_RSP
	@n@b Result_t : Possible return values are ::RESULT_ERROR, ::SMS_SIM_BUSY,
	::RESULT_OK.
	@n@b ResultData : ::Boolean /// the SMS capacity exceeded flag.
	
**/

void CAPI2_SMS_GetSIMSMSCapacityExceededFlag(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_GETSIMSMSCAPACITYEXCEEDEDFLAG_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**	
	Function to check if network SMS bearer is available
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@note
	This is a function that allows the user to check if a network
	SMS bearer is available based on the setting. eg.GPRS/GSM.
	@n@b Responses 
	@n@b MsgType_t : ::MSG_SMS_ISSMSSERVICEAVAIL_RSP
	@n@b Result_t : Possible return values are ::RESULT_ERROR, ::SMS_NETWORK_GPRS_ONLY,
	::SMS_NETWORK_CS_ONLY, ::RESULT_OK.
	@n@b ResultData : None.
**/

void CAPI2_SMS_IsSmsServiceAvail(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_ISSMSSERVICEAVAIL_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
	Get SMS stored status at a given index and storage 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		storageType (in) Storage Type
	@param		index (in) Index ///< integer type; value in the range of location numbers supported by the associated storage
	@note 
	@n@b Responses 
	@n@b MsgType_t : ::MSG_SMS_GETSMSSTOREDSTATE_RSP
	@n@b Result_t : Possible return values ;
	::SMS_SIM_BUSY, ::SMS_ME_SMS_NOT_SUPPORTED, ::SMS_INVALID_INDEX, ::SMS_SIM_BUSY, ::RESULT_ERROR,
	::RESULT_OK.
	@n@b ResultData : See ::SIMSMSMesgStatus_t for SIM SMS message status.
**/

void CAPI2_SMS_GetSmsStoredState (UInt32 tid, UInt8 clientID, SmsStorage_t storageType, UInt16 index)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_GetSmsStoredState_Req.index = index;
	req.req_rep_u.CAPI2_SMS_GetSmsStoredState_Req.storageType = storageType;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_GETSMSSTOREDSTATE_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}


//***************************************************************************************
/**
	Function to write SMS PDU to storage
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		length (in) Length of the PDU
	@param		*inSmsPdu (in) Pointer to the PDU string to be written
	@param		*sca (in) Service Center Address
	@param		storageType (in) Storage Type
	@note
	If NULL is passed to SCA, default SCA will be used
	confirmation will be returned in the intertask message ::MSG_SMS_WRITE_RSP_IND.
	Message data has the following type ::SIM_SMS_UPDATE_RESULT_t
	One has to wait for the ::MSG_SMS_READY_IND msg before using this API.
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_WRITE_RSP_IND
	@n@b Result_t :	
	@n@b ResultData : Refer ::SIM_SMS_UPDATE_RESULT_t
**/

void CAPI2_SMS_WriteSMSPduReq(UInt32 tid, UInt8 clientID, UInt8 length, UInt8* inSmsPdu, 
							Sms_411Addr_t* sca, SmsStorage_t storageType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_WriteSMSPduReq_Req.inSmsPdu = inSmsPdu;
	req.req_rep_u.CAPI2_SMS_WriteSMSPduReq_Req.storageType = storageType;
	req.req_rep_u.CAPI2_SMS_WriteSMSPduReq_Req.length = length;
	req.req_rep_u.CAPI2_SMS_WriteSMSPduReq_Req.sca = sca;
	req.respId = MSG_SMS_WRITE_RSP_IND;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_WRITESMSPDU_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
	Write text SMS to storage Function
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		*inNum (in) Destination Number, NULL terminated.
	@param		*inSMS (in) Pointer to the text to be written
	@param		*params (in) SMS transmit params.
	@param		*inSca (in) Service Center Address
	@param		storageType (in) Storage Type
	@note
	Default sca will be used, or caller can set SCA first.
	SMS Tx parameters can be passed in or pass NULL to use the default setting.
	confirmation will be returned in the intertask message ::MSG_SMS_WRITE_RSP_IND.
	Message data has the following type:
	::SIM_SMS_UPDATE_RESULT_t;
	One has to wait for the ::MSG_SMS_READY_IND msg before using this API.
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_WRITE_RSP_IND
	@n@b Result_t :	Possible return values are ::RESULT_OK, ::SMS_ME_SMS_NOT_SUPPORTED, ::SMS_SIM_BUSY,
	::SMS_OPERATION_NOT_ALLOWED.
	@n@b ResultData : Refer ::SIM_SMS_UPDATE_RESULT_t
**/

void CAPI2_SMS_WriteSMSReq(UInt32 tid, UInt8 clientID, UInt8* inNum, UInt8* inSMS, 
						 SmsTxParam_t* params, UInt8* inSca, SmsStorage_t storageType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_WriteSMSReq_Req.inNum.len= (inNum)?(strlen((char*)inNum)+1):0;
	req.req_rep_u.CAPI2_SMS_WriteSMSReq_Req.inNum.str = (char*)inNum;
	req.req_rep_u.CAPI2_SMS_WriteSMSReq_Req.inSca.len= (inSca)?(strlen((char*)inSca)+1):0;
	req.req_rep_u.CAPI2_SMS_WriteSMSReq_Req.inSca.str = (char*)inSca;
	req.req_rep_u.CAPI2_SMS_WriteSMSReq_Req.inSMS.len = (inSMS)?(strlen((char*)inSMS)+1):0;
	req.req_rep_u.CAPI2_SMS_WriteSMSReq_Req.inSMS.str = (char*)inSMS;
	req.req_rep_u.CAPI2_SMS_WriteSMSReq_Req.params = params;
	req.req_rep_u.CAPI2_SMS_WriteSMSReq_Req.storageType = storageType;
	req.respId = MSG_SMS_WRITE_RSP_IND;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_WRITESMSREQ_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}


//***************************************************************************************
/**
	Send text SMS to Network Function
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		*inNum (in) inNum - destination number, NULL terminated
	@param		*inSMS (in) inSMS - text contents of SMS,NULL terminated
	@param		*params (in) params - SMS transmit parameters(See GetSmsTxParams)
	@param		*inSca (in) Service Center Address
	@note
	When NULL SCA is passed in,default SCA will be used, or caller can set SCA first.
	SMS Tx parameters can be passed in or pass NULL to use the default setting.
	Network ack/nack will be returned in the intertask message MSG_SMS_SUBMIT_RSP.
	See ::SmsSubmitRspMsg_t; for submit response structure.
	One has to wait for the ::MSG_SMS_READY_IND msg before using this API.
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_SUBMIT_RSP
	@n@b Result_t :	Possible return values are ::RESULT_OK, ::SMS_NO_SERVICE, ::SMS_SIM_BUSY,
	::SMS_OPERATION_NOT_ALLOWED.
	@n@b ResultData: Refer ::SmsSubmitRspMsg_t.
**/

void CAPI2_SMS_SendSMSReq(UInt32 tid, UInt8 clientID, UInt8* inNum, UInt8* inSMS, SmsTxParam_t* params, UInt8* inSca)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_SendSMSReq_Req.inNum.len= (inNum)?(strlen((char*)inNum)+1):0;
	req.req_rep_u.CAPI2_SMS_SendSMSReq_Req.inNum.str = (char*)inNum;
	req.req_rep_u.CAPI2_SMS_SendSMSReq_Req.inSca.len= (inSca)?(strlen((char*)inSca)+1):0;
	req.req_rep_u.CAPI2_SMS_SendSMSReq_Req.inSca.str = (char*)inSca;
	req.req_rep_u.CAPI2_SMS_SendSMSReq_Req.inSMS.len = (inSMS)?(strlen((char*)inSMS)+1):0;
	req.req_rep_u.CAPI2_SMS_SendSMSReq_Req.inSMS.str = (char*)inSMS;
	req.req_rep_u.CAPI2_SMS_SendSMSReq_Req.params = params;
	req.respId = MSG_SMS_SUBMIT_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_SENDSMSREQ_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}


//***************************************************************************************
/**
	Send SMS PDU to Network Function
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		length (in) Length of the PDU Strig
	@param		*inSmsPdu (in) Pointer to the PDU string of the SMS
	@param		*sca (in) SMS Service Center Address
	@note
	If NULL is passed to SCA, default SCA will be used.
	Network ack/nack will be returned in the intertask message ::MSG_SMS_SUBMIT_RSP.
	See ::SmsSubmitRspMsg_t for response structure and details.
	One has to wait for the ::MSG_SMS_READY_IND msg before using this API.
	@n@b Responses
	@n@b MsgType_t : :MSG_SMS_SUBMIT_RSP
	@n@b Result_t :	Possible return values are ::SMS_NO_SERVICE, ::SMS_SIM_BUSY, ::SMS_OPERATION_NOT_ALLOWED,
	::RESULT_OK.
	@n@b ResultData: Refer ::SmsSubmitRspMsg_t.
**/

void CAPI2_SMS_SendSMSPduReq(UInt32 tid, UInt8 clientID, UInt8 length, UInt8* inSmsPdu, Sms_411Addr_t* sca)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_SendSMSPduReq_Req.inSmsPdu = inSmsPdu;
	req.req_rep_u.CAPI2_SMS_SendSMSPduReq_Req.length = length;
	req.req_rep_u.CAPI2_SMS_SendSMSPduReq_Req.sca = sca;
	req.respId = MSG_SMS_SUBMIT_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_SENDSMSPDUREQ_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
	Send SMS stored at a given index and storage
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		storageType (in) Storage Type
	@param		index (in) Index///< integer type; value in the range of location numbers supported by the associated storage
	@note
	Only SMS message of "STO UNSENT" or "STO SENT" are allowed to be sent with this
	function.
	Network ack/nack will be returned in the intertask message ::MSG_SMS_SUBMIT_RSP.
	One has to wait for the ::MSG_SMS_READY_IND msg before using this API.
	@n@b Responses
	@n@b MsgType_t : :MSG_SMS_SUBMIT_RSP
	@n@b Result_t :	Possible return values are ::RESULT_OK, ::SMS_ME_SMS_NOT_SUPPORTED, ::SMS_SIM_BUSY,
	::SMS_OPERATION_NOT_ALLOWED, ::SMS_NO_SERVICE.
	@n@b ResultData: Refer ::SmsSubmitRspMsg_t.
**/

void CAPI2_SMS_SendStoredSmsReq(UInt32 tid, UInt8 clientID, SmsStorage_t storageType, UInt16 index)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_SendStoredSmsReq_Req.index = index;
	req.req_rep_u.CAPI2_SMS_SendStoredSmsReq_Req.storageType = storageType;
	req.respId = MSG_SMS_SUBMIT_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_SENDSTOREDSMSREQ_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}


//***************************************************************************************
/**
	Function to move SMS from ME to SIM
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		length (in) Length of the PDU
	@param		*inSmsPdu (in) Pointer to the PDU to be written
	@param		smsState (in) Message Status
	@note
*	The stored ACK will be broadcast to all clients.
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_WRITE_RSP_IND 
	@n@b Result_t :	Possible return values are ::RESULT_ERROR, ::SMS_SIM_BUSY, ::SMS_OPERATION_NOT_ALLOWED,
	::RESULT_OK.
	@n@b ResultData: ::SIM_SMS_UPDATE_RESULT_t
**/

void CAPI2_SMS_WriteSMSPduToSIMReq(UInt32 tid, UInt8 clientID, UInt8 length, UInt8* inSmsPdu, SIMSMSMesgStatus_t smsState)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_WriteSMSPduToSIMReq_Req.inSmsPdu = inSmsPdu;
	req.req_rep_u.CAPI2_SMS_WriteSMSPduToSIMReq_Req.smsState = smsState;
	req.req_rep_u.CAPI2_SMS_WriteSMSPduToSIMReq_Req.length = length;
	req.respId = MSG_SMS_WRITE_RSP_IND;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_WRITESMSPDUTOSIM_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
	Function returns the Message Reference for the last submitted SMS
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID		
	@note
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_GETLASTTPMR_RSP 
	@n@b Result_t :
	@n@b ResultData: UInt8 ///< Message Reference
**/

void CAPI2_SMS_GetLastTpMr(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_GETLASTTPMR_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
	Function return the default Tx Parameters
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID		
	@param		*params (in) Transmit Params
	@note
	This function fills the default parameters needed to compose an SMS. A default set of 
	parameters is stored on the SMS module side.
	See SmsTxParam_t for parameters.
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_GETSMSTXPARAMS_RSP
	@n@b Result_t :
	@n@b ResultData: Refer ::SmsTxParam_t.
	@see	
	SmsAlphabet_t
	SmsTime_t;
	SmsMsgClass_t
	SmsAbsolute_t;
**/

void CAPI2_SMS_GetSmsTxParams(UInt32 tid, UInt8 clientID, SmsTxParam_t* params)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_GetSmsTxParams_Req.params = params;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_GETSMSTXPARAMS_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
	Function return the default Tx Parameters in text mode
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID		
	@param		*smsParms (in) Transmit Params
	@note
*   This is specifically for ATC client.
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_GETTXPARAMINTEXTMODE_RSP
	@n@b Result_t :
	@n@b ResultData: Refer ::SmsTxTextModeParms_t.
**/

void CAPI2_SMS_GetTxParamInTextMode( UInt32 tid, UInt8 clientID, SmsTxTextModeParms_t* smsParms )
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_GetTxParamInTextMode_Req.smsParms = smsParms;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_GETTXPARAMINTEXTMODE_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Set the protocol Id in the default SMS Tx param set 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	
	@param		pid (in) Process ID
	@note
*	Default is 0
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_SETSMSTXPARAMPROCID_RSP
	@n@b Result_t :
	@n@b ResultData: None.
**/	

void CAPI2_SMS_SetSmsTxParamProcId(UInt32 tid, UInt8 clientID, UInt8 pid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_SetSmsTxParamProcId_Req = pid;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_SETSMSTXPARAMPROCID_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Set the coding type in the default SMS Tx param set 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	
	@param		*codingType (in) Coding Type
	@note
*	Default is GSM default alphabet and no class.
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_SETSMSTXPARAMCODINGTYPE_RSP
	@n@b Result_t :
	@n@b ResultData: None.
**/	

void CAPI2_SMS_SetSmsTxParamCodingType(UInt32 tid, UInt8 clientID, SmsCodingType_t* codingType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_SetSmsTxParamCodingType_Req.codingType = codingType;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_SETSMSTXPARAMCODINGTYPE_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Set the Validity Period for SMS submit. 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	
	@param		validatePeriod (in) Validity Period
	@note
*	Default is 167 (1 day). Only relative validity period is supported.
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_SETSMSTXPARAMVALIDPERIOD_RSP
	@n@b Result_t : Possible return values are ::RESULT_ERROR, ::RESULT_OK.
	@n@b ResultData: None.
**/	

void CAPI2_SMS_SetSmsTxParamValidPeriod(UInt32 tid, UInt8 clientID, UInt8 validatePeriod)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_SetSmsTxParamValidPeriod_Req = validatePeriod;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_SETSMSTXPARAMVALIDPERIOD_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Set the Compression Flag for SMS submit. 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	
	@param		compression (in) Compression( TRUE or FALSE )
	@note
*	Default is Off.
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_SETSMSTXPARAMCOMPRESSION_RSP
	@n@b Result_t : Possible return values are ::RESULT_ERROR, ::RESULT_OK.
	@n@b ResultData: None.
**/	

void CAPI2_SMS_SetSmsTxParamCompression(UInt32 tid, UInt8 clientID, Boolean compression)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_SetSmsTxParamCompression_Req = compression;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_SETSMSTXPARAMCOMPRESSION_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Set the Reply Path for SMS submit. 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	
	@param		replyPath (in) Reply Path( TRUE or FALSE )
	@note
*	Default is FALSE.
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_SETSMSTXPARAMREPLYPATH_RSP
	@n@b Result_t : Possible return values are ::RESULT_ERROR, ::RESULT_OK.
	@n@b ResultData: None.
**/	

void CAPI2_SMS_SetSmsTxParamReplyPath(UInt32 tid, UInt8 clientID, Boolean replyPath)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_SetSmsTxParamReplyPath_Req = replyPath;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_SETSMSTXPARAMREPLYPATH_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Set the Data Header Indication for SMS submit. 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	
	@param		udhi (in) Data Header Indication( TRUE or FALSE )
	@note
*	Default is FALSE.
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_SETSMSTXPARAMUSERDATAHDRIND_RSP
	@n@b Result_t :
	@n@b ResultData: None.
**/	

void CAPI2_SMS_SetSmsTxParamUserDataHdrInd(UInt32 tid, UInt8 clientID, Boolean udhi)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_SetSmsTxParamUserDataHdrInd_Req = udhi;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_SETSMSTXPARAMUSERDATAHDRIND_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Set the Status Report Request for SMS submit. 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	
	@param		srr (in) Status Report Request( TRUE or FALSE )
*	Default is FALSE.
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_SETSMSTXPARAMSTATUSRPTREQFLAG_RSP
	@n@b Result_t :
	@n@b ResultData: None.
**/	

void CAPI2_SMS_SetSmsTxParamStatusRptReqFlag(UInt32 tid, UInt8 clientID, Boolean srr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_SetSmsTxParamStatusRptReqFlag_Req = srr;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_SETSMSTXPARAMSTATUSRPTREQFLAG_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Set the Reject Duplicate Flag for SMS submit. 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	
	@param		rejDupl (in) Reject Duplicate Flag( TRUE or FALSE )
	@note
*	Default is FALSE.
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_SETSMSTXPARAMREJDUPL_RSP
	@n@b Result_t :
	@n@b ResultData: None.
**/	

void CAPI2_SMS_SetSmsTxParamRejDupl(UInt32 tid, UInt8 clientID, Boolean rejDupl)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_SetSmsTxParamRejDupl_Req = rejDupl;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_SETSMSTXPARAMREJDUPL_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Delete SMS at a given slot and storage. 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		storeType (in) Storage Type
	@param		rec_no (in) Record Number
	@note
*	Confirmation returns in ::MSG_SIM_SMS_STATUS_UPD_RSP.
*	See SmsMsgDeleteResult_t for possible return results.
	@n@b Responses
	@n@b MsgType_t : ::MSG_SIM_SMS_STATUS_UPD_RSP
	@n@b Result_t : Possible return values are ::SMS_SIM_BUSY, ::SMS_ME_SMS_NOT_SUPPORTED,
	::SMS_INVALID_INDEX, ::SMS_OPERATION_NOT_ALLOWED, ::RESULT_OK.
	@n@b ResultData: SmsMsgDeleteResult_t.
**/	

void CAPI2_SMS_DeleteSmsMsgByIndexReq(UInt32 tid, UInt8 clientID, SmsStorage_t storeType, UInt16 rec_no)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_DeleteSmsMsgByIndexReq_Req.rec_no = rec_no;
	req.req_rep_u.CAPI2_SMS_DeleteSmsMsgByIndexReq_Req.storeType = storeType;
	req.respId = MSG_SIM_SMS_STATUS_UPD_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_DELETESMSMSGBYINDEX_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}


//***************************************************************************************
/**
    Function to Read SMS from a given slot and storage. 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		storeType (in) Storage Type
	@param		rec_no (in) Record Number
	@return		Result_t
	@note
*	The SMS being read is returned in the intertask message ::MSG_SIM_SMS_DATA_RSP.
*	The CB message will be returned in the intertask message ::MSG_SMSCB_READ_RSP.
*   Returned data are in ::SmsSimMsg_t type, except CB message which is in ::SIM_SMS_DATA_t format.
*	Where msgTypeInd is used to determine the type of core data in union.
	Example:msgTypeInd = SMS_DELIVER:
	Union msg = SmsRxParam_t msgRxData;
	Text data is given in ASCII type(converted to ASCII in the case the SMS is 7 bit packed).
*	Returned data provide both structure type and PDU type for easy processing.
*	Payload type for for ::MSG_SMSCB_READ_RSP:
	SIMSMSMesg_t;
*	Where the mesg_data can be cast to ::SmsStoredSmsCb_t for processing.
	@n@b Responses
	Recieve either ::MSG_SMSCB_READ_RSP or MSG_SIM_SMS_DATA_RSP.
	@n@b MsgType_t : ::MSG_SMSCB_READ_RSP.
	@n@b Result_t : 	Possible return values are ::SMS_SIM_BUSY, ::SMS_ME_SMS_NOT_SUPPORTED,
	::SMS_INVALID_INDEX, ::SMS_SIM_BUSY, ::SMS_OPERATION_NOT_ALLOWED, ::RESULT_OK.
	@n@b ResultData: ::SIM_SMS_DATA_t.
	@n@b MsgType_t :::MSG_SIM_SMS_DATA_RSP.
	@n@b Result_t : 	Possible return values are ::SMS_SIM_BUSY, ::SMS_ME_SMS_NOT_SUPPORTED,
	::SMS_INVALID_INDEX, ::SMS_SIM_BUSY, ::SMS_OPERATION_NOT_ALLOWED, ::RESULT_OK.
	@n@b ResultData: ::SmsSimMsg_t.
**/	

void CAPI2_SMS_ReadSmsMsgReq(UInt32 tid, UInt8 clientID, SmsStorage_t storeType, UInt16 rec_no)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_ReadSmsMsgReq_Req.rec_no = rec_no;
	req.req_rep_u.CAPI2_SMS_ReadSmsMsgReq_Req.storeType = storeType;
	req.respId = MSG_SIM_SMS_DATA_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_READSMSMSG_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to List SMS from a given storage type. 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		storeType (in) Storage Type
	@param		msgBox (in) Message Box
	@note
*	The returned data has the same format as the read function above.
*   Each SMS listed are returned in its own ::MSG_SIM_SMS_DATA_RSP message.
*   The last message with zero length indicates there's no more message to list.
*	@see SIMSMSMesgStatus_t;
*	Returned data provide both structure type and PDU type for easy processing.
	@n@b Responses
	@n@b MsgType_t : ::MSG_SIM_SMS_DATA_RSP.
	@n@b Result_t : Possible return values are ::SMS_SIM_BUSY, ::SMS_ME_SMS_NOT_SUPPORTED, ::SMS_OPERATION_NOT_ALLOWED,
	::SMS_NO_MSG_TO_LIST, ::RESULT_OK.
	@n@b ResultData: ::SmsSimMsg_t.
**/	

void CAPI2_SMS_ListSmsMsgReq(UInt32 tid, UInt8 clientID, SmsStorage_t storeType, SIMSMSMesgStatus_t msgBox)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_ListSmsMsgReq_Req.storeType = storeType;
	req.req_rep_u.CAPI2_SMS_ListSmsMsgReq_Req.msgBox = msgBox;
	req.respId = MSG_SIM_SMS_DATA_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_LISTSMSMSG_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Set the display preference for the received SMS. 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		type (in) Display Preference Type
	@param		mode (in) Mode
	@note
	mode - 0,1,2,3 (defined in GSM 07.05)
	Example:
	When no class information is given, a new incoming SMS message will be displayed 
	based on the setting of mode of SMS_MT:
	0 - message will be stored, but no indication will be shown on AT.
	1 - message will be stored and indication will be given together with the rec_no and storageType
	where the message is stored.
	2 - message will not be saved, but the whole message will be sent to the client.
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_SETNEWMSGDISPLAYPREF_RSP.
	@n@b Result_t : 
	@n@b ResultData: None.

	@see 
		NewMsgDisplayPref_t for type
**/	

void CAPI2_SMS_SetNewMsgDisplayPref(UInt32 tid, UInt8 clientID, NewMsgDisplayPref_t type, UInt8 mode)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_SetNewMsgDisplayPref_Req.type = type;
	req.req_rep_u.CAPI2_SMS_SetNewMsgDisplayPref_Req.mode = mode;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_SETNEWMSGDISPLAYPREF_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Get the display preference for the received SMS. 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		type (in) Display Preference Type
	@return		UInt8	
	@note
*	The mode for the given type is returned as UInt8 value (defined in GSM 07.05)
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_GETNEWMSGDISPLAYPREF_RSP.
	@n@b Result_t : 
	@n@b ResultData: UInt8.
**/	

void CAPI2_SMS_GetNewMsgDisplayPref(UInt32 tid, UInt8 clientID, NewMsgDisplayPref_t type)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_GetNewMsgDisplayPref_Req = type;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_GETNEWMSGDISPLAYPREF_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Set the preferred storage type for the incoming SMS. 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		storageType (in) Storage Type
	@return		Result_t
	@note
*	Set type to ::ME_STORAGE will result in all incoming SMS to be saved in ME
	storage first and then to SIM storage once ME storage is full.
*	Set type to ::SM_STORAGE will result in all incoming SMS to be saved in SIM 
	storage only.
*	The default is ::ME_STORAGE.
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_SETSMSPREFSTORAGE_RSP.
	@n@b Result_t : 
	@n@b ResultData: None.
*	@see SmsStorage_t 

**/	

void CAPI2_SMS_SetSMSPrefStorage(UInt32 tid, UInt8 clientID, SmsStorage_t storageType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_SetSMSPrefStorage_Req = storageType;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_SETSMSPREFSTORAGE_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Get the preferred storage type for the incoming SMS. 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	
	@note
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_GETSMSPREFSTORAGE_RSP.
	@n@b Result_t : 
	@n@b ResultData: ::SmsStorage_t ///< SMS storage type.
*	@see SmsStorage_t
**/	

void CAPI2_SMS_GetSMSPrefStorage(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_GETSMSPREFSTORAGE_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Get the usage of a given storage. 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	
	@param		storageType (in) Storage Type
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_GETSMSSTORAGESTATUS_RSP.
	@n@b Result_t : Possible return values are ::SMS_SIM_BUSY, ::SMS_ME_SMS_NOT_SUPPORTED, ::RESULT_OK.
	@n@b ResultData: ::CAPI2_SMS_GetSMSStorageStatus_t.
**/	

void CAPI2_SMS_GetSMSStorageStatus(UInt32 tid, UInt8 clientID, SmsStorage_t storageType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_SetSMSPrefStorage_Req = storageType;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_GETSMSSTORAGESTATUS_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Save the SMS profile. 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	
	@param		profileIndex (in) Profile Index
	@note
*	profileIndex: 0...255
*	Manufacturer specific profile number where settings are to be stored.
*	Should be called periodically or every update to save user settings
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_SAVESMSSERVICEPROFILE_RSP.
	@n@b Result_t : Possible return values are ::RESULT_ERROR, ::RESULT_OK.
	@n@b ResultData: None.
**/	

void CAPI2_SMS_SaveSmsServiceProfile(UInt32 tid, UInt8 clientID, UInt8 profileIndex)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_SaveSmsServiceProfile_Req = profileIndex;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_SAVESMSSERVICEPROFILE_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Restore the SMS profile.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	
	@param		profileIndex (in) Profile Index
	@note
*	profileIndex: 0...255
*	Manufacturer specific profile number where settings are to be stored.
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_RESTORESMSSERVICEPROFILE_RSP.
	@n@b Result_t : Possible return values are ::SMS_OPERATION_NOT_ALLOWED, ::RESULT_OK.
	@n@b ResultData: None.
**/	
void CAPI2_SMS_RestoreSmsServiceProfile(UInt32 tid, UInt8 clientID, UInt8 profileIndex)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_RestoreSmsServiceProfile_Req = profileIndex;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_RESTORESMSSERVICEPROFILE_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Start/Stop Cell Broadcast with types(Channels) 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		mode (in) Mode
	@param		*chnlIDs (in) Channel IDs
	@param		*codings (in) Codings
	@note
*	mode : 0 message types specified in chnlIDs and codings are accepted
		   1 message types specified in chnlIDs and codings are not accepted. 
		   Setting mode = 1,chnlIDs = NULL,codings = NULL stops all CB messages.

*	chnlIDs : String type; all different possible combinations of CBM message identifiers(channels).
			  An empty string means NOT to receive any Cb message. Range setting using "-" is
			  supported up to 10(i.e 15-25 is OK, 15-26 results in error)

*	Codings : String type; all different possible combinations of CBM data coding schemes. Default is 
			  an empty string.

*	CNF/REJ comes back in the intertask message ::MSG_SMS_CB_START_STOP_RSP. 

	See SmsCBMsgRspType_t for Cell Broadcast response structure. Also see ::SmsCBActionType_t for cell broadcast action
	and ::SmsCBStopType_t for CB stop type.
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_CB_START_STOP_RSP
	@n@b Result_t : Possible return values are ::RESULT_OK, ::SMS_NO_SERVICE, ::SMS_SIM_BUSY.
	@n@b ResultData: ::SmsCBMsgRspType_t
**/	

void CAPI2_SMS_SetCellBroadcastMsgTypeReq(UInt32 tid, UInt8 clientID, UInt8 mode, UInt8* chnlIDs, UInt8* codings)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_SetCellBroadcastMsgTypeReq_Req.chnlIDs.len= (chnlIDs)?(strlen((char*)chnlIDs)+1):0;
	req.req_rep_u.CAPI2_SMS_SetCellBroadcastMsgTypeReq_Req.chnlIDs.str = (char*)chnlIDs;
	req.req_rep_u.CAPI2_SMS_SetCellBroadcastMsgTypeReq_Req.codings.len= (codings)?(strlen((char*)codings)+1):0;
	req.req_rep_u.CAPI2_SMS_SetCellBroadcastMsgTypeReq_Req.codings.str = (char*)codings;
	req.req_rep_u.CAPI2_SMS_SetCellBroadcastMsgTypeReq_Req.mode = mode;
	req.respId = MSG_SMS_CB_START_STOP_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_SETCELLBROADCASTMSGTYPE_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to suspend Cell Broadcast Channels filtering and allow all CB channels
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param      suspend_filtering; 0 - filter CB based on cbmids, 1 - suspend filtering and allow CB of all channels. 
	@note
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_CBALLOWALLCHNLREQ_RSP
	@n@b Result_t : 
	@n@b ResultData: None.
**/	

void CAPI2_SMS_CBAllowAllChnlReq(UInt32 tid, UInt8 clientID, Boolean suspend_filtering)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_CBAllowAllChnlReq_Req = suspend_filtering;
	
	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_CBALLOWALLCHNLREQ_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}


//***************************************************************************************
/**
    Function to Add new Cell Broadcast Channels
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		*newCBMI (in) New Cell Broadcast Message IDs(Channels)
	@param		cbmiLen (in) Length of the Message IDs
	@note
*	NewCBMI is the ptr to the Mids data, where one or more Mids (each occupies 2 bytes) are contained
*	cbmiLen is the length of the data. For example, 2 Mids will have a length of 4 (bytes).
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_ADDCELLBROADCASTCHNLREQ_RSP
	@n@b Result_t : 
	@n@b ResultData: None.
**/	

void CAPI2_SMS_AddCellBroadcastChnlReq(UInt32 tid, UInt8 clientID, UInt8* newCBMI, UInt8 cbmiLen)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_AddCellBroadcastChnlReq_Req.len = cbmiLen;
	req.req_rep_u.CAPI2_SMS_AddCellBroadcastChnlReq_Req.str = (char*)newCBMI;
	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_ADDCELLBROADCASTCHNLREQ_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Remove a particular Cell Broadcast Message ID(Channels)
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		*newCBMI (in) Cell Broadcast Message ID(Channels)
	@note
*	Only one is allowed at a time
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_REMOVECELLBROADCASTCHNLREQ_RSP
	@n@b Result_t : Possible return values are ::SMS_SIM_BUSY, ::RESULT_OK.
	@n@b ResultData: None.
**/	

void CAPI2_SMS_RemoveCellBroadcastChnlReq(UInt32 tid, UInt8 clientID, UInt8* newCBMI)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_RemoveCellBroadcastChnlReq_Req.len = len= (newCBMI)?(strlen((char*)newCBMI)+1):0;;
	req.req_rep_u.CAPI2_SMS_RemoveCellBroadcastChnlReq_Req.str = (char*)newCBMI;
	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_REMOVECELLBROADCASTCHNLREQ_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}


//***************************************************************************************
/**
    Function to Remove all Cell Broadcast Message IDs from the search list and SIM file.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@note
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_REMOVEALLCBCHNLFROMSEARCHLIST_RSP
	@n@b Result_t : 
	@n@b ResultData: None.
	
**/	
void CAPI2_SMS_RemoveAllCBChnlFromSearchList(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_REMOVEALLCBCHNLFROMSEARCHLIST_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}


//***************************************************************************************
/**
    Function to Return the pointer to the local copy of EFcbmi data
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@note
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_GETCBMI_RSP
	@n@b Result_t : Possible return values are ::SMS_SIM_BUSY, ::RESULT_OK.
	@n@b ResultData: SMS_CB_MSG_IDS_t.
**/	

void CAPI2_SMS_GetCBMI(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_GETCBMI_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Return the pointer to the local copy of CB Language data
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@note
		Cell Broadcast Language - see GSM 3.38, section 5 for details.
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_GETCBLANGUAGE_RSP
	@n@b Result_t : 
	@n@b ResultData: T_MN_CB_LANGUAGES.
**/	

void CAPI2_SMS_GetCbLanguage(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_GETCBLANGUAGE_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Add a particular CB Language
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		newCBLang (in) Cell Broadcast Language
	@note
	Cell Broadcast Language - see GSM 3.38, section 5 for details.	
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_ADDCELLBROADCASTLANGREQ_RSP
	@n@b Result_t : 
	@n@b ResultData: None.
**/	

void CAPI2_SMS_AddCellBroadcastLangReq(UInt32 tid, UInt8 clientID, UInt8 newCBLang)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_AddCellBroadcastLangReq_Req = newCBLang;
	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_ADDCELLBROADCASTLANGREQ_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Remove a particular CB Language
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		*cbLang (in) Pointer to Cell Broadcast Language
	@note
	Cell Broadcast Language - see GSM 3.38, section 5 for details.
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_REMOVECELLBROADCASTLANGREQ_RSP
	@n@b Result_t : 
	@n@b ResultData: None.
**/	

void CAPI2_SMS_RemoveCellBroadcastLangReq(UInt32 tid, UInt8 clientID, UInt8* cbLang)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_RemoveCellBroadcastLangReq.len = len= (cbLang)?(strlen((char*)cbLang)+1):0;;
	req.req_rep_u.CAPI2_SMS_RemoveCellBroadcastLangReq.str = (char*)cbLang;
	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_REMOVECELLBROADCASTLANGREQ_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Start Receiving Cell broadcast Messages
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@note
	*	CNF/REJ comes back in the intertask message ::MSG_SMS_CB_START_STOP_RSP. 
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_CB_START_STOP_RSP
	@n@b Result_t : 
	@n@b ResultData: ::SmsCBMsgRspType_t.
**/	

void CAPI2_SMS_StartReceivingCellBroadcastReq(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.respId = MSG_SMS_CB_START_STOP_RSP;
	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_STARTRECEIVINGCELLBROADCAST_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Stop Receiving Cell broadcast Messages
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@note
	*	CNF/REJ comes back in the intertask message ::MSG_SMS_CB_START_STOP_RSP.
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_CB_START_STOP_RSP
	@n@b Result_t : Possible return values are ::SMS_NO_SERVICE, ::SMS_OPERATION_NOT_ALLOWED, ::RESULT_OK
	@n@b ResultData: ::SmsCBMsgRspType_t.
**/	

void CAPI2_SMS_StopReceivingCellBroadcastReq(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.respId = MSG_SMS_CB_START_STOP_RSP;
	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_STOPRECEIVINGCELLBROADCAST_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}


//***************************************************************************************
/**
    Function to set the CB ignore duplicate flag
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		ignoreDupl (in) CB ignore duplicate flag
	@note 
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_SETCBIGNOREDUPLFLAG_RSP
	@n@b Result_t : 
	@n@b ResultData: None.
**/	

void CAPI2_SMS_SetCBIgnoreDuplFlag(UInt32 tid, UInt8 clientID, Boolean ignoreDupl)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_SetCBIgnoreDuplFlag_Req = ignoreDupl;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_SETCBIGNOREDUPLFLAG_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Retrieve the CB ignore duplicate flag
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@note 
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_GETCBIGNOREDUPLFLAG_RSP
	@n@b Result_t : 
	@n@b ResultData: Boolean.
**/	

void CAPI2_SMS_GetCBIgnoreDuplFlag(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_GETCBIGNOREDUPLFLAG_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}


//***************************************************************************************
/**
    Function to Toggle Voice Mail Indication(Enable/Disable)
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		on_off (in) Boolean ( ON/OFF)
	@note
*	on_off: FALSE -	Off(no unsolicited event will be sent to the client)
			TRUE - On(Unsolicited event will be sent to the clent)
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_SETVMINDONOFF_RSP
	@n@b Result_t : 
	@n@b ResultData: None.	
**/	

void CAPI2_SMS_SetVMIndOnOff(UInt32 tid, UInt8 clientID, Boolean on_off)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_SetVMIndOnOff_Req = on_off;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_SETVMINDONOFF_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Check if Voice Mail Indication is Enabled
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_ISVMINDENABLED_RSP
	@n@b Result_t : 
	@n@b ResultData: Boolean - TRUE if enabled.		
**/	

void CAPI2_SMS_IsVMIndEnabled(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_ISVMINDENABLED_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Get Voice Mail Indication Status
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@note
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_GETVMWAITINGSTATUS_RSP
	@n@b Result_t : Possible return values are ::SMS_SIM_BUSY, ::VM_NOT_AVAILABLE, ::RESULT_OK.
	@n@b ResultData: SmsVoicemailInd_t.		
**/	


void CAPI2_SMS_GetVMWaitingStatus(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_GETVMWAITINGSTATUS_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Get number of voicemail service center numbers
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@note
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_GETNUMOFVMSCNUMBER_RSP
	@n@b Result_t : return ::SMS_INVALID_INDEX if the given line is out-of-range.
	@n@b ResultData: UInt8.		
		

* One has to wait for the ::MSG_SMS_READY_IND msg before using this API.
**/	

void CAPI2_SMS_GetNumOfVmscNumber(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_GETNUMOFVMSCNUMBER_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Get the Voice Mail Service Center Number
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		vmsc_type (in) Line
	@note
*	Valid values for vmsc_type is in the range of 1-4 (for l1,L2,fax,data).
*	Retrieved vmsc is defined as ::SmsAddress_t;
*   One has to wait for the ::MSG_SMS_READY_IND msg before using this API.
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_GETVMSCNUMBER_RSP
	@n@b Result_t : Possible return values are ::RESULT_OK, ::SMS_INVALID_INDEX, ::RESULT_ERROR.
	@n@b ResultData: SmsAddress_t.			
**/	

void CAPI2_SMS_GetVmscNumber(UInt32 tid, UInt8 clientID, SIM_MWI_TYPE_t vmsc_type)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_GetVmscNumber_Req = vmsc_type;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_GETVMSCNUMBER_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Update the Voice Mail Service Center Number
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		vmsc_type (in) Line
	@param		*vmscNum (in) Pointer to Voice Mail Service Center Address
	@param		numType (in) Number Type
	@param		*alpha (in) Pointer to Alpha
	@param		alphaCoding (in) Alpha Coding
	@param		alphaLen (in) Alpha Length
	@note
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_UPDATEVMSCNUMBERREQ_RSP
	@n@b Result_t : Possible return values are ::SMS_OPERATION_NOT_ALLOWED, ::SMS_ADDR_NUMBER_STR_TOO_LONG,
	::SMS_INVALID_INDEX, ::SMS_INVALID_DIALSTR_CHAR, ::SMS_OPERATION_NOT_ALLOWED,
	::SMS_OPERATION_NOT_ALLOWED.
	@n@b ResultData: None.	
**/	

void CAPI2_SMS_UpdateVmscNumberReq(UInt32 tid, UInt8 clientID, SIM_MWI_TYPE_t vmsc_type, UInt8* vmscNum, UInt8 numType, 
								 UInt8* alpha, UInt8 alphaCoding, UInt8 alphaLen)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_UpdateVmscNumberReq_Req.vmsc_type = vmsc_type;
	req.req_rep_u.CAPI2_SMS_UpdateVmscNumberReq_Req.alpha = alpha;
	req.req_rep_u.CAPI2_SMS_UpdateVmscNumberReq_Req.alphaCoding = alphaCoding;
	req.req_rep_u.CAPI2_SMS_UpdateVmscNumberReq_Req.numType = numType;
	req.req_rep_u.CAPI2_SMS_UpdateVmscNumberReq_Req.alphaLen = alphaLen;
	req.req_rep_u.CAPI2_SMS_UpdateVmscNumberReq_Req.vmscNum.len = (vmscNum) ? (strlen((char*)vmscNum) +1): 0;
	req.req_rep_u.CAPI2_SMS_UpdateVmscNumberReq_Req.vmscNum.str = (char*)vmscNum;
	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_UPDATEVMSCNUMBERREQ_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Get the SMS Bearer Preference ///< GMS/GPRS
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@note
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_GETSMSBEARERPREFERENCE_RSP
	@n@b Result_t : 
	@n@b ResultData: ::SMS_BEARER_PREFERENCE_t  ///< Preferred Bearer setting..	 
**/	

void	CAPI2_GetSMSBearerPreference(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_GETSMSBEARERPREFERENCE_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Set the SMS Bearer Preference 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		pref (in) Preferred bearer
	@note
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_SETSMSBEARERPREFERENCE_RSP
	@n@b Result_t : 
	@n@b ResultData: None.
	@see     SMS_BEARER_PREFERENCE_t 
**/	

void CAPI2_SetSMSBearerPreference(UInt32 tid, UInt8 clientID, SMS_BEARER_PREFERENCE_t pref)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SetSMSBearerPreference_Req = pref;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_SETSMSBEARERPREFERENCE_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Set the Status Change Mode
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		mode (in) Read status change mode 
	@note
*	Default setting is TRUE, i.e. after read the SMS will change its status from UNREAD to READ.
*	By setting the mode to FALSE, it allows MMI/ATC to list all the messages without changing
	the staus of the messages.
*	When the mode is  set to FALSE, one needs to call SMS_ChangeSmsStatusReq() to change the 
	status from	UNREAD to READ,because the status will not change automatically.
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_SETSMSREADSTATUSCHANGEMODE_RSP
	@n@b Result_t : 
	@n@b ResultData: None.
**/	

void CAPI2_SMS_SetSmsReadStatusChangeMode(UInt32 tid, UInt8 clientID, Boolean mode)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_SetSmsReadStatusChangeMode = mode;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_SETSMSREADSTATUSCHANGEMODE_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Get the Status Change Mode
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@note 
		This mode determines whether the READ/UNREAD status will change
		mode = TRUE: the status will change. mode = FALSE: not change
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_GETSMSREADSTATUSCHANGEMODE_RSP
	@n@b Result_t : 
	@n@b ResultData: Boolean.
**/	

void CAPI2_SMS_GetSmsReadStatusChangeMode(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_GETSMSREADSTATUSCHANGEMODE_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Change the status of SMS at a given slot of a given storage.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientId (in) Client ID
	@param		storageType (in) Storage Type
	@param		index (in) Index - integer type; value in the range of location numbers supported by the associated storage
	@note
*	Only allow to change SMS status from UNREAD to READ.
	index : integer type; value in the range of location numbers supported by the associated memory
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_CHANGESTATUSREQ_RSP
	@n@b Result_t : Possible return values are ::SMS_SIM_BUSY, ::SMS_ME_SMS_NOT_SUPPORTED, ::SMS_INVALID_INDEX,
	::SMS_SIM_BUSY, ::RESULT_OK.
	@n@b ResultData: None.
**/	

void CAPI2_SMS_ChangeSmsStatusReq(UInt32 tid, UInt8 clientId, SmsStorage_t storageType, UInt16 index)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_ChangeSmsStatusReq_Req.index = index;
	req.req_rep_u.CAPI2_SMS_ChangeSmsStatusReq_Req.storageType = storageType;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_CHANGESTATUSREQ_REQ, clientId, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function for the ME module to send an async stored status indication msg to Platform
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientId (in) Client ID
	@param		result (in) ME Access Result
	@param		slotNumber (in) Slot Number
	@return		Result_t
	@note 
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_SENDMESTOREDSTATUSIND_RSP
	@n@b Result_t : return ::RESULT_ERROR if parameters out of range.Return ::RESULT_OK if parameters are OK.
	@n@b ResultData: None.
**/	

void CAPI2_SMS_SendMEStoredStatusInd(UInt32 tid, UInt8 clientId, MEAccess_t result, UInt16 slotNumber)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t resultT = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_SendMEStoredStatusInd_Req.result = result;
	req.req_rep_u.CAPI2_SMS_SendMEStoredStatusInd_Req.slotNumber = slotNumber;
	req.respId = MSG_SMS_SENDMESTOREDSTATUSIND_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_SENDMESTOREDSTATUSIND_REQ, clientId, resultT, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}


//***************************************************************************************
/**
    Function for the ME module to send an async SMS read response msg to Platform 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientId (in) Client ID
	@param		result (in) ME Access Result
	@param		slotNumber (in) Slot Number
	@param		*inSms (in) Pointer to SMS Message
	@param		inLen (in) Length
	@param		status (in) Status
	@note 
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_SENDMERETRIEVESMSDATAIND_RSP
	@n@b Result_t : return ::RESULT_ERROR if parameters out of range.Return ::RESULT_OK if parameters are OK.
	@n@b ResultData: None.
**/	

void CAPI2_SMS_SendMERetrieveSmsDataInd(UInt32 tid, UInt8 clientId, MEAccess_t result, UInt16 slotNumber, UInt8* inSms, 
									  UInt16 inLen, SIMSMSMesgStatus_t status)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t resultT = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_SendMERetrieveSmsDataInd_Req.result = result;
	req.req_rep_u.CAPI2_SMS_SendMERetrieveSmsDataInd_Req.slotNumber = slotNumber;
	req.req_rep_u.CAPI2_SMS_SendMERetrieveSmsDataInd_Req.inSms = inSms;
	req.req_rep_u.CAPI2_SMS_SendMERetrieveSmsDataInd_Req.inLen = inLen;
	req.req_rep_u.CAPI2_SMS_SendMERetrieveSmsDataInd_Req.status = status;
	req.respId = MSG_SMS_SENDMERETRIEVESMSDATAIND_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_SENDMERETRIEVESMSDATAIND_REQ, clientId, resultT, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function for the ME module to send an async delete status indication msg to Platform
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientId (in) Client ID
	@param		result (in) ME Access Result
	@param		slotNumber (in) Slot Number
	@note 
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_SENDMEREMOVEDSTATUSIND_RSP
	@n@b Result_t : return ::RESULT_ERROR if parameters out of range.return ::RESULT_OK if parameters are OK.
	@n@b ResultData: None.
**/	


void CAPI2_SMS_SendMERemovedStatusInd(UInt32 tid, UInt8 clientId, MEAccess_t result, UInt16 slotNumber)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t resultT = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_SendMERemovedStatusInd_Req.result = result;
	req.req_rep_u.CAPI2_SMS_SendMERemovedStatusInd_Req.slotNumber = slotNumber;
	req.respId = MSG_SMS_SENDMEREMOVEDSTATUSIND_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_SENDMEREMOVEDSTATUSIND_REQ, clientId, resultT, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to set stored state at given index and storage.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientId (in) Client ID
	@param		storageType (in) Storage Type
	@param		index (in) Index - integer type; value in the range of location numbers supported by the associated storage
	@param		status (in) Status
	@note 
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_SETSMSSTOREDSTATE_RSP
	@n@b Result_t : return ::RESULT_ERROR if parameters out of range.return ::RESULT_OK if parameters are OK.
	@n@b ResultData: None.
**/	


void CAPI2_SMS_SetSmsStoredState(UInt32 tid, UInt8 clientId, SmsStorage_t storageType, UInt16 index, SIMSMSMesgStatus_t status)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t resultT = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_SetSmsStoredState_Req.index = index;
	req.req_rep_u.CAPI2_SMS_SetSmsStoredState_Req.storageType = storageType;
	req.req_rep_u.CAPI2_SMS_SetSmsStoredState_Req.status = status;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_SETSMSSTOREDSTATE_REQ, clientId, resultT, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to Get SMS transaction from clientID.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientId (in) Client ID
	@note 
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_GETTRANSACTIONFROMCLIENTID_RSP
	@n@b Result_t : return ::RESULT_ERROR if parameters out of range.return ::RESULT_OK if parameters are OK.
	@n@b ResultData: CAPI2_SMS_GetTransactionFromClientID_t.
**/	


void CAPI2_SMS_GetTransactionFromClientID(UInt32 tid, UInt8 clientId)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_GETTRANSACTIONFROMCLIENTID_REQ, clientId, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to check if the sms cached data is ready.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientId (in) Client ID
	@note 
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_ISCACHEDDATAREADY_RSP
	@n@b Result_t : return ::RESULT_ERROR if parameters out of range.return ::RESULT_OK if parameters are OK.
	@n@b ResultData: Boolean.
**/	


void CAPI2_SMS_IsCachedDataReady(UInt32 tid, UInt8 clientId)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_ISCACHEDDATAREADY_REQ, clientId, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Function to get the Enhanced Voice Mail Info IEI.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientId (in) Client ID
	@note 
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_GETENHANCEDVMINFOIEI_RSP
	@n@b Result_t : return ::RESULT_ERROR if parameters out of range.return ::RESULT_OK if parameters are OK.
	@n@b ResultData: SmsEnhancedVMInd_t.
**/


void CAPI2_SMS_GetEnhancedVMInfoIEI(UInt32 tid, UInt8 clientId)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_GETENHANCEDVMINFOIEI_REQ, clientId, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Expands a GSM 3.40 Address field (NbDigit, type of address, value) into an SmsAddress_t structure
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		Src (in)
	@note 
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_340ADDRTOTE_RSP
	@n@b Result_t : return ::RESULT_ERROR if parameters out of range.return ::RESULT_OK if parameters are OK.
	@n@b ResultData: UInt8
**/	
void CAPI2_SMS_340AddrToTe(UInt32 tid, UInt8 clientId, Sms_340Addr_t *Src)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_340AddrToTe_Req.Src = Src;
	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_340ADDRTOTE_REQ, clientId, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Set all the display preferences at once
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		mode (in)
	@param		mt (in)
	@param		bm (in)
	@param		ds (in)
	@param		bfr (in)
	@param		mode (in)
	@note 
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_SETALLNEWMSGDISPLAYPREF_RSP
	@n@b Result_t : return ::RESULT_ERROR if parameters out of range.return ::RESULT_OK if parameters are OK.
	@n@b ResultData: None.
**/	
void CAPI2_SMS_SetAllNewMsgDisplayPref(UInt32 tid, UInt8 clientId, UInt8* mode, UInt8* mt, UInt8* bm, UInt8* ds, UInt8* bfr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_SetAllNewMsgDisplayPref_Req.bfr = bfr;
	req.req_rep_u.CAPI2_SMS_SetAllNewMsgDisplayPref_Req.bm = bm;
	req.req_rep_u.CAPI2_SMS_SetAllNewMsgDisplayPref_Req.ds = ds;
	req.req_rep_u.CAPI2_SMS_SetAllNewMsgDisplayPref_Req.mode = mode;
	req.req_rep_u.CAPI2_SMS_SetAllNewMsgDisplayPref_Req.mt = mt;
	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_SETALLNEWMSGDISPLAYPREF_REQ, clientId, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    API function to send response to network (for SMS_DELIVER message)
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		mti (in) SMS message type
	@param		ackType (in) ACK type
	@note 
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_ACKTONETWORK_RSP
	@n@b Result_t : return ::RESULT_ERROR if parameters out of range.return ::RESULT_OK if parameters are OK.
	@n@b ResultData: None.
**/	
void CAPI2_SMS_SendAckToNetwork(UInt32 tid, UInt8 clientId, SmsMti_t mti, SmsAckNetworkType_t ackType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SMS_SendAckToNetwork_Req.mti = mti;
	req.req_rep_u.CAPI2_SMS_SendAckToNetwork_Req.ackType = ackType;
	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_ACKTONETWORK_REQ, clientId, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//***************************************************************************************
/**
    Request to send Response this routine is responsible for deletion of the memory pointed by 'status_rsp'
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		status_rsp (in)
	@note 
	@n@b Responses
	@n@b MsgType_t : ::MSG_SMS_SMSRESPONSE_RSP
	@n@b Result_t : return ::RESULT_ERROR if parameters out of range.return ::RESULT_OK if parameters are OK.
	@n@b ResultData: None.
**/	
void CAPI2_MNSMS_SMResponse(UInt32 tid, UInt8 clientId, T_MN_TP_STATUS_RSP * status_rsp )
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_MNSMS_SMResponse_Req.status_rsp = status_rsp;
	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_SMSRESPONSE_REQ, clientId, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

/** @} */

