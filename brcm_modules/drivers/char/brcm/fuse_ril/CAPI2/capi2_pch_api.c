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
*   @file   capi2_pch_api.c
*
*   @brief  This file defines the capi2 api's related to PCH
*
****************************************************************************/
//#include	"capi2_pch_ds.h"
#include	"capi2_reqrep.h"


//***************************************************************************************
/**
	Function to get PDP context params for the given cid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_GETPDPCONTEXT_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	PDPContext_t
**/
void CAPI2_PDP_GetPDPContext(UInt32 tid, UInt8 clientID, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PDP_GetPDPContext_Req.cid = cid;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_GETPDPCONTEXT_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

//***************************************************************************************
/**
	Function to set PDP context params for the given cid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@param		pcontext (in) pointer to the PDP Context params
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_SETPDPCONTEXT_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_PDP_SetPDPContext(UInt32 tid, UInt8 clientID, UInt8 cid, PDPContext_t *pcontext)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PDP_SetPDPContext_Req.cid = cid;
	req.req_rep_u.CAPI2_PDP_SetPDPContext_Req.pcontext = pcontext;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_SETPDPCONTEXT_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}
//***************************************************************************************
/**
	Function to delete PDP context params for the given cid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_DELETEPDPCONTEXT_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_PDP_DeletePDPContext(UInt32 tid, UInt8 clientID, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PDP_DeletePDPContext_Req.cid = cid;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_DELETEPDPCONTEXT_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

//***************************************************************************************
/**
	Function to set the secondary PDP context params for the given cid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@param		priCid (in) the Primary PDP Context ID
	@param		dComp (in) data compression
	@param		hComp (in) header compression
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_SETSECPDPCONTEXT_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_PDP_SetSecPDPContext(UInt32 tid, UInt8 clientID, UInt8 cid, UInt8 priCid, UInt8 dComp, UInt8 hComp)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PDP_SetSecPDPContext_Req.cid = cid;
	req.req_rep_u.CAPI2_PDP_SetSecPDPContext_Req.priCid = priCid;
	req.req_rep_u.CAPI2_PDP_SetSecPDPContext_Req.dComp = dComp;
	req.req_rep_u.CAPI2_PDP_SetSecPDPContext_Req.hComp = hComp;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_SETSECPDPCONTEXT_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

//***************************************************************************************
/**
	Function to get PDP gprs QOS params for the given cid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_GETGPRSQOS_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	PCHQosProfile_t
**/
void CAPI2_PDP_GetGPRSQoS(UInt32 tid, UInt8 clientID, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PDP_GetGPRSQoS_Req.cid = cid;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_GETGPRSQOS_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

//***************************************************************************************
/**
	Function to set PDP gprs qos params for the given cid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@param		params (in) PDP gprs qos params
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_SETGPRSQOS_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_PDP_SetGPRSQoS(UInt32 tid, UInt8 clientID, UInt8 cid, PCHQosProfile_t* params)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PDP_SetGPRSQoS_Req.cid = cid;
	req.req_rep_u.CAPI2_PDP_SetGPRSQoS_Req.params = params;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_SETGPRSQOS_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

//***************************************************************************************
/**
	Function to delete PDP gprs qos params for the given cid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_DELETEGPRSQOS_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_PDP_DeleteGPRSQoS(UInt32 tid, UInt8 clientID, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PDP_DeleteGPRSQoS_Req.cid = cid;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_DELETEGPRSQOS_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

//***************************************************************************************
/**
	Function to get PDP gprs min qos params for the given cid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_GETGPRSMINQOS_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	PDP_GPRSMinQoS_t
**/
void CAPI2_PDP_GetGPRSMinQoS(UInt32 tid, UInt8 clientID, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PDP_GetGPRSMinQoS_Req.cid = cid;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_GETGPRSMINQOS_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

//***************************************************************************************
/**
	Function to set PDP min qos params for the given cid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@param		qos (in) Min QOS Params
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_SETGPRSMINQOS_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_PDP_SetGPRSMinQoS(UInt32 tid, UInt8 clientID, UInt8 cid, PDP_GPRSMinQoS_t* qos)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PDP_SetGPRSMinQoS_Req.cid = cid;
	req.req_rep_u.CAPI2_PDP_SetGPRSMinQoS_Req.qos = qos;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_SETGPRSMINQOS_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

//***************************************************************************************
/**
	Function to delete PDP min qos params for the given cid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_DELETEGPRSMINQOS_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_PDP_DeleteGPRSMinQoS(UInt32 tid, UInt8 clientID, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PDP_DeleteGPRSMinQoS_Req.cid = cid;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_DELETEGPRSMINQOS_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

//***************************************************************************************
/**
	Function to send request for gprs attach
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_MS_SENDCOMBINEDATTACHREQ_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_MS_SendCombinedAttachReq(UInt32 tid, UInt8 clientID)
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
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_SENDCOMBINEDATTACHREQ_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

//***************************************************************************************
/**
	Function to send request for gprs detach
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cause (in) cause of dereg
	@param		regType (in) option to select gsm,gprs or both
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_MS_SENDDETACHREQ_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_MS_SendDetachReq(UInt32 tid, UInt8 clientID, DeRegCause_t cause, RegType_t regType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_MS_SendDetachReq_Req.cause = cause;
	req.req_rep_u.CAPI2_MS_SendDetachReq_Req.regType = regType;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_SENDDETACHREQ_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

//***************************************************************************************
/**
	Function to get gprs attach state
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_MS_GETGPRSATTACHSTATUS_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	AttachState_t
**/
void CAPI2_MS_GetGPRSAttachStatus(UInt32 tid, UInt8 clientID)
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
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GETGPRSATTACHSTATUS_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

//***************************************************************************************
/**
	Function to set gprs attach mode
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		mode (in) mode ( ATTACH_MODE_GSM_GPRS=1,ATTACH_MODE_GSM_ONLY=2,ATTACH_MODE_GPRS_ONLY=3)
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_MS_SETATTACHMODE_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_MS_SetAttachMode(UInt32 tid, UInt8 clientID, UInt8 mode)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_MS_SetAttachMode_Req.mode = mode;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_SETATTACHMODE_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

//***************************************************************************************
/**
	Function to get gprs attach mode
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		mode (in) mode ( ATTACH_MODE_GSM_GPRS,ATTACH_MODE_GSM_ONLY,ATTACH_MODE_GPRS_ONLY)
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_MS_GETATTACHMODE_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData: UInt8 ( ATTACH_MODE_GSM_GPRS=1,ATTACH_MODE_GSM_ONLY=2,ATTACH_MODE_GPRS_ONLY=3)
**/
void CAPI2_MS_GetAttachMode(UInt32 tid, UInt8 clientID)
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
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GETATTACHMODE_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}
//***************************************************************************************
/**
	Function to check if the secondary pdp context is defined
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_ISSECONDARYPDPDEFINED_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData: Boolean (TRUE if Secondary PDP is defined)
**/
void CAPI2_PDP_IsSecondaryPdpDefined(UInt32 tid, UInt8 clientID, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PDP_IsSecondaryPdpDefined_Req.cid = cid;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_ISSECONDARYPDPDEFINED_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

//***************************************************************************************
/**
	Function to send pdp activation request to Network
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@param		reason (in) Reason for activation
	@param		protConfig (in) Protocol config options ( As per GSM L3 spec, see notes below )
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_ACTIVATION_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	PDP_SendPDPActivateReq_Rsp_t
	@note
		The purpose of the protocol configuration options information element is to 
		transfer external network protocol options associated with a PDP context activation. 
		( ex LCP, CHAP, PAP, and IPCP )
		The username and password chap/pap MD5 digest needs to be set in the options.
		The DNS primary address request etc.
		Refer to Protocol configuration options GSM 4.08 version 6.7.0, release 1997, 
		- section 10.5.6.3 ( Mobile radio interface layer 3 specification )
**/
void CAPI2_PDP_SendPDPActivateReq(UInt32 tid, UInt8 clientID, UInt8 cid, PCHActivateReason_t reason, PCHProtConfig_t* protConfig)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PDP_SendPDPActivateReq_Req.cid = cid;
	req.req_rep_u.CAPI2_PDP_SendPDPActivateReq_Req.reason = reason;
	req.req_rep_u.CAPI2_PDP_SendPDPActivateReq_Req.protConfig = protConfig;
	req.respId = MSG_PDP_ACTIVATION_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_SENDPDPACTIVATEREQ_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

//***************************************************************************************
/**
	Function to send pdp modification request to Network
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@param		reason (in) Reason for activation
	@param		protConfig (in) Protocol config options ( As per GSM L3 spec, see notes below )
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_MODIFY_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	PDP_SendPDPModifyReq_Rsp_t
	@note
**/
void CAPI2_PDP_SendPDPModifyReq(UInt32 tid, UInt8 clientID, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PDP_SendPDPModifyReq_Req.cid = cid;
	req.respId = MSG_PDP_MODIFICATION_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_SENDPDPMODIFYREQ_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

//***************************************************************************************
/**
	Function to send gprs pdp de-activation request to network
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_DEACTIVATION_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData: PDP_SendPDPDeactivateReq_Rsp_t
**/
void CAPI2_PDP_SendPDPDeactivateReq(UInt32 tid, UInt8 clientID, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PDP_SendPDPDeactivateReq_Req.cid = cid;
	req.respId = MSG_PDP_DEACTIVATION_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_SENDPDPDEACTIVATEREQ_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}
//***************************************************************************************
/**
	Function to send gprs secondary pdp activation request to Network
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_SEC_ACTIVATION_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData: PDP_SendPDPActivateSecReq_Rsp_t
**/
void CAPI2_PDP_SendPDPActivateSecReq(UInt32 tid, UInt8 clientID, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PDP_SendPDPActivateSecReq_Req.cid = cid;
	req.respId = MSG_PDP_SEC_ACTIVATION_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_SENDPDPACTIVATESECREQ_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}
//***************************************************************************************
/**
	Function to request activation status for all context ids
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_GETGPRSACTIVATESTATUS_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData: PDP_GetGPRSActivateStatus_Rsp_t
**/
void CAPI2_PDP_GetGPRSActivateStatus(UInt32 tid, UInt8 clientID)
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
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_GETGPRSACTIVATESTATUS_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}


//***************************************************************************************
/**
	Function to set MS class
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		msClass (in) MS Class type
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_SETMSCLASS_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData: None
**/
void CAPI2_PDP_SetMSClass(UInt32 tid, UInt8 clientID, MSClass_t msClass)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PDP_SetMSClass_Req.msClass = msClass;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_SETMSCLASS_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

//***************************************************************************************
/**
	Function to get MS class
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_GETMSCLASS_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData: MSClass_t
**/
void CAPI2_PDP_GetMSClass(UInt32 tid, UInt8 clientID)
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
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_GETMSCLASS_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

//***************************************************************************************
/**
	Function to get UMTS Traffic Flow Template for the given cid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_GETUMTSTFT_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	PCHTrafficFlowTemplate_t
**/
void CAPI2_PDP_GetUMTSTft(UInt32 tid, UInt8 clientID, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PDP_GetUMTSTft_Req.cid = cid;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_GETUMTSTFT_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}
//***************************************************************************************
/**
	Function to set UMTS Traffic Flow Template for the given cid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@param		pTft_in (in) pointer to the UMTS Traffic Flow Template params
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_SETUMTSTFT_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_PDP_SetUMTSTft(UInt32 tid, UInt8 clientID, UInt8 cid, PCHTrafficFlowTemplate_t *pTft_in)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PDP_SetUMTSTft_Req.cid = cid;
	req.req_rep_u.CAPI2_PDP_SetUMTSTft_Req.pTft_in = pTft_in;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_SETUMTSTFT_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}
//***************************************************************************************
/**
	Function to delete UMTS Traffic Flow Template for the given cid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_DELETEUMTSTFT_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_PDP_DeleteUMTSTft(UInt32 tid, UInt8 clientID, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PDP_DeleteUMTSTft_Req.cid = cid;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_DELETEUMTSTFT_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}
//***************************************************************************************
/**
	Function to send request to activate SNDCP connection
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_ACTIVATE_SNDCP_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData: PDP_DataState_t
**/
void CAPI2_PDP_ActivateSNDCPConnection(UInt32 tid, UInt8 clientID, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PDP_ActivateSNDCPConnection_Req.cid = cid;
	req.respId = MSG_PDP_ACTIVATE_SNDCP_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_ACTIVATESNDCPCONNECTION_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}
//***************************************************************************************
/**
	Function to send request to deactivate SNDCP connection
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_DEACTIVATE_SNDCP_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData: None
**/
void CAPI2_PDP_DeactivateSNDCPConnection(UInt32 tid, UInt8 clientID, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PDP_DeactivateSNDCPConnection_Req.cid = cid;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_DEACTIVATESNDCPCONNECTION_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

//***************************************************************************************
/**
	Function to get default context from CP for the given cid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_GETPDPDEFAULTCONTEXT_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData: PDPDefaultContext_t
**/
void CAPI2_PDP_GetPDPDefaultContext(UInt32 tid, UInt8 clientID, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PDP_GetPDPDefaultContext_Req.cid = cid;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_GETPDPDEFAULTCONTEXT_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

//***************************************************************************************
/**
	Function to get PCH context State from CP ( for all cid's )
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_GETPCHCONTEXT_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData: PDP_GetPCHContext_Rsp_t
**/
void CAPI2_PDP_GetPCHContext(UInt32 tid, UInt8 clientID)
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
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_GETPCHCONTEXT_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}
//***************************************************************************************
/**
	Function to get the PCH context state for the given cid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_GETPCHCONTEXTSTATE_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData: PCHContextState_t
**/
void CAPI2_PDP_GetPCHContextState(UInt32 tid, UInt8 clientID, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PDP_GetPCHContextState_Req.cid = cid;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_GETPCHCONTEXTSTATE_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

//***************************************************************************************
/**
	Function to get R99 UMTS min qos params for the given cid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_GETR99UMTSMINQOS_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	PCHR99QosProfile_t
**/

void CAPI2_PDP_GetR99UMTSMinQoS(UInt32 tid, UInt8 clientID, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PDP_GetR99UMTSMinQoS_Req.cid = cid;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_GETR99UMTSMINQOS_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

//***************************************************************************************
/**
	Function to get R99 UMTS qos params for the given cid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_GETR99UMTSQOS_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	PCHR99QosProfile_t
**/

void CAPI2_PDP_GetR99UMTSQoS(UInt32 tid, UInt8 clientID, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PDP_GetR99UMTSQoS_Req.cid = cid;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_GETR99UMTSQOS_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

//***************************************************************************************
/**
	Function to get Negotiated qos params for the given cid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_GETNEGQOS_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	PCHR99QosProfile_t
**/

void CAPI2_PDP_GetNegQoS(UInt32 tid, UInt8 clientID, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PDP_GetNegQoS_Req.cid = cid;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_GETNEGQOS_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}


//***************************************************************************************
/**
	Function to set R99 UMTS min qos params for the given cid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@param		params (in) R99 UMTS min qos params
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_SETR99UMTSMINQOS_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	None
**/

void CAPI2_PDP_SetR99UMTSMinQoS(UInt32 tid, UInt8 clientID, UInt8 cid, PCHR99QosProfile_t *pR99MinQos)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PDP_SetR99UMTSMinQoS_Req.cid = cid;
	req.req_rep_u.CAPI2_PDP_SetR99UMTSMinQoS_Req.pR99Qos = pR99MinQos;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_SETR99UMTSMINQOS_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

//***************************************************************************************
/**
	Function to set R99 UMTS qos params for the given cid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@param		params (in) R99 UMTS qos params
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_SETR99UMTSQOS_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	None
**/

void CAPI2_PDP_SetR99UMTSQoS(UInt32 tid, UInt8 clientID, UInt8 cid, PCHR99QosProfile_t *pR99Qos)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PDP_SetR99UMTSQoS_Req.cid = cid;
	req.req_rep_u.CAPI2_PDP_SetR99UMTSQoS_Req.pR99Qos = pR99Qos;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_SETR99UMTSQOS_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

void CAPI2_PDP_GetNegotiatedParms(UInt32 tid, UInt8 clientID, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PDP_GetNegotiatedParms_Req.cid = cid;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_GETNEGOTIATEDPARMS_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

void CAPI2_MS_IsGprsCallActive(UInt32 tid, UInt8 clientID, UInt8 chan)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_MS_IsGprsCallActive_Req.chan = chan;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_ISGPRSCALLACTIVE_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

void CAPI2_MS_SetChanGprsCallActive(UInt32 tid, UInt8 clientID, UInt8 chan, Boolean active)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_MS_SetChanGprsCallActive_Req.chan = chan;
	req.req_rep_u.CAPI2_MS_SetChanGprsCallActive_Req.active = active;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_SETCHANGPRSCALLACTIVE_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

void CAPI2_MS_SetCidForGprsActiveChan(UInt32 tid, UInt8 clientID, UInt8 chan, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_MS_SetCidForGprsActiveChan_Req.chan = chan;
	req.req_rep_u.CAPI2_MS_SetCidForGprsActiveChan_Req.cid = cid;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_SETCIDFORGPRSACTIVECHAN_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

void CAPI2_PDP_GetPPPModemCid(UInt32 tid, UInt8 clientID)
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
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_GETPPPMODEMCID_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

void CAPI2_MS_GetGprsActiveChanFromCid(UInt32 tid, UInt8 clientID, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_MS_GetGprsActiveChanFromCid_Req.cid = cid;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GETGPRSACTIVECHANFROMCID_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

void CAPI2_MS_GetCidFromGprsActiveChan(UInt32 tid, UInt8 clientID, UInt8 chan)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_MS_GetCidFromGprsActiveChan_Req.chan = chan;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_GETCIDFROMGPRSACTIVECHAN_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}


void CAPI2_PDP_GetPDPAddress(UInt32 tid, UInt8 clientID, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PDP_GetPDPAddress_Req.cid = cid;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_GETPDPADDRESS_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}


void CAPI2_PDP_SendTBFData(UInt32 tid, UInt8 clientID, UInt8 cid, UInt32 numberBytes)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PDP_SendTBFData_Req.cid = cid;
	req.req_rep_u.CAPI2_PDP_SendTBFData_Req.numberBytes = numberBytes;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_SENDTBFDATA_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

void CAPI2_PDP_TftAddFilter(UInt32 tid, UInt8 clientID, UInt8 cid, PCHPacketFilter_T *pPktFilter)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PDP_TftAddFilter_Req.cid = cid;
	req.req_rep_u.CAPI2_PDP_TftAddFilter_Req.pPktFilter = pPktFilter;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_TFTADDFILTER_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

void CAPI2_PDP_SetPCHContextState(UInt32 tid, UInt8 clientID, UInt8 cid, PCHContextState_t contextState)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PDP_SetPCHContextState_Req.cid = cid;
	req.req_rep_u.CAPI2_PDP_SetPCHContextState_Req.contextState = contextState;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_SETPCHCONTEXTSTATE_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}
void CAPI2_PDP_SetDefaultPDPContext(UInt32 tid, UInt8 clientID, UInt8 cid, PDPDefaultContext_t *pDefaultContext)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PDP_SetDefaultPDPContext_Req.cid = cid;
	req.req_rep_u.CAPI2_PDP_SetDefaultPDPContext_Req.pDefaultContext = pDefaultContext;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PDP_SETDEFAULTPDPCONTEXT_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}


void CAPI2_ReadDecodedProtConfig(UInt32 tid, UInt8 clientID, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_ReadDecodedProtConfig_Req = cid;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_READDECODEDPROTCONFIG_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

void CAPI2_BuildProtocolConfigOption(UInt32 tid, UInt8 clientID, char* username, char* password, IPConfigAuthType_t authType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_BuildProtocolConfigOption_Req.username.len= (username)?(strlen((char*)username)+1):0;
	req.req_rep_u.CAPI2_BuildProtocolConfigOption_Req.username.str = (char*)username;
	req.req_rep_u.CAPI2_BuildProtocolConfigOption_Req.password.len= (password)?(strlen((char*)password)+1):0;
	req.req_rep_u.CAPI2_BuildProtocolConfigOption_Req.password.str = (char*)password;
	req.req_rep_u.CAPI2_BuildProtocolConfigOption_Req.authType	= authType;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_BUILDPROTCONFIGOPTIONS_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

void CAPI2_BuildProtocolConfigOption2(UInt32 tid, UInt8 clientID, IPConfigAuthType_t authType, 
							CHAP_ChallengeOptions_t *cc,
							CHAP_ResponseOptions_t *cr,
							PAP_CnfgOptions_t *po)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_BuildProtocolConfigOption2_Req.authType	= authType;
	req.req_rep_u.CAPI2_BuildProtocolConfigOption2_Req.cc = cc;
	req.req_rep_u.CAPI2_BuildProtocolConfigOption2_Req.cr = cr;
	req.req_rep_u.CAPI2_BuildProtocolConfigOption2_Req.po = po;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_BUILDPROTCONFIGOPTIONS2_REQ, clientID, result, &stream, &len);

	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

