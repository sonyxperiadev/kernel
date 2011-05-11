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
*   @file   capi2_pch_api_old.h
*
*   @brief  This file defines the capi2 api's related to PCH
*
*	This file defines the interface for PCH API.This file provides
*	the function prototypes necessary to handle data services.
****************************************************************************/

#ifndef CAPI2_PCH_API_OLD_H
#define CAPI2_PCH_API_OLD_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup   CAPI2_PDP_APIOLDGroup   PDP Context Handler
 * @addtogroup CAPI2_PDP_APIOLDGroup
 * @{
 */
 
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
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	PDPContext_t
**/
void CAPI2_PDP_GetPDPContext(UInt32 tid, UInt8 clientID, UInt8 cid);

//***************************************************************************************
/**
	Function to set PDP context params for the given cid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@param		numParms (in) 
	@param		pdpType
	@param		apn
	@param		pdpAddr
	@param		dComp (in) data compression
	@param		hComp (in) header compression
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_SETPDPCONTEXT_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_PDP_SetPDPContext(UInt32 tid, UInt8 clientID, UInt8 cid, UInt8 numParms, UInt8* pdpType, UInt8* apn, UInt8* pdpAddr, UInt8 dComp, UInt8 hComp);

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
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_PDP_DeletePDPContext(UInt32 tid, UInt8 clientID, UInt8 cid);

//***************************************************************************************
/**
	Function to set the secondary PDP context params for the given cid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@param		numParms (in) 
	@param		priCid (in) the Primary PDP Context ID
	@param		dComp (in) data compression
	@param		hComp (in) header compression
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_SETSECPDPCONTEXT_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_PDP_SetSecPDPContext(UInt32 tid, UInt8 clientID, UInt8 cid, UInt8 numParms, UInt8 priCid, UInt8 dComp, UInt8 hComp);


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
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	PCHQosProfile_t
**/
void CAPI2_PDP_GetGPRSQoS(UInt32 tid, UInt8 clientID, UInt8 cid);

//***************************************************************************************
/**
	Function to set PDP gprs qos params for the given cid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@param		NumPara (in) 
	@param		prec (in) 
	@param		delay (in) 
	@param		rel (in) 
	@param		peak (in) 
	@param		mean (in) 
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_SETGPRSQOS_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_PDP_SetGPRSQoS(UInt32 tid, UInt8 clientID, UInt8 cid, UInt8 NumPara, UInt8 prec, UInt8 delay, UInt8 rel, UInt8 peak, UInt8 mean);

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
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_PDP_DeleteGPRSQoS(UInt32 tid, UInt8 clientID, UInt8 cid);

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
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	PDP_GPRSMinQoS_t
**/
void CAPI2_PDP_GetGPRSMinQoS(UInt32 tid, UInt8 clientID, UInt8 cid);

//***************************************************************************************
/**
	Function to set PDP min qos params for the given cid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@param		NumPara (in) 
	@param		prec (in) 
	@param		delay (in) 
	@param		rel (in) 
	@param		peak (in) 
	@param		mean (in) 
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_SETGPRSMINQOS_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_PDP_SetGPRSMinQoS(UInt32 tid, UInt8 clientID, UInt8 cid, UInt8 NumPara, UInt8 prec, UInt8 delay, UInt8 rel, UInt8 peak, UInt8 mean);

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
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_PDP_DeleteGPRSMinQoS(UInt32 tid, UInt8 clientID, UInt8 cid);

//***************************************************************************************
/**
	Function to send request for gprs attach
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		SIMPresent (in)
	@param		SIMType (in)
	@param		regType (in)
	@param		plmn (in)
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_MS_SENDCOMBINEDATTACHREQ_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_MS_SendCombinedAttachReq(UInt32 tid, UInt8 clientID, Boolean SIMPresent, SIMType_t SIMType, RegType_t regType, PLMN_t plmn);

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
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_MS_SendDetachReq(UInt32 tid, UInt8 clientID, DeRegCause_t cause, RegType_t regType);

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
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	AttachState_t
**/
void CAPI2_MS_GetGPRSAttachStatus(UInt32 tid, UInt8 clientID);

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
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_MS_SetAttachMode(UInt32 tid, UInt8 clientID, MSAttachMode_t mode);

//***************************************************************************************
/**
	Function to get gprs attach mode
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_MS_GETATTACHMODE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData: MSAttachMode_t ( ATTACH_MODE_GSM_GPRS=1,ATTACH_MODE_GSM_ONLY=2,ATTACH_MODE_GPRS_ONLY=3)
**/
void CAPI2_MS_GetAttachMode(UInt32 tid, UInt8 clientID);

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
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData: Boolean (TRUE if Secondary PDP is defined)
**/
void CAPI2_PDP_IsSecondaryPdpDefined(UInt32 tid, UInt8 clientID, UInt8 cid);

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
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
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
void CAPI2_PDP_SendPDPActivateReq(
					UInt32						tid,
					UInt8						clientID,
					UInt8						cid,
					PCHActivateReason_t			reason,
					PCHProtConfig_t				*protConfig );

//***************************************************************************************
/**
	Function to send pdp modification request to Network
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_MODIFY_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	PDP_SendPDPModifyReq_Rsp_t
	@note
**/
void CAPI2_PDP_SendPDPModifyReq(UInt32 tid, UInt8 clientID, UInt8 cid);

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
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData: PDP_SendPDPDeactivateReq_Rsp_t
**/
void CAPI2_PDP_SendPDPDeactivateReq(UInt32 tid, UInt8 clientID, UInt8 cid);

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
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData: PDP_SendPDPActivateSecReq_Rsp_t
**/
void CAPI2_PDP_SendPDPActivateSecReq(UInt32 tid, UInt8 clientID, UInt8 cid);

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
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData: PDP_GetGPRSActivateStatus_Rsp_t
**/
void CAPI2_PDP_GetGPRSActivateStatus(UInt32 tid, UInt8 clientID);


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
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData: None
**/
void CAPI2_PDP_SetMSClass(UInt32 tid, UInt8 clientID, MSClass_t msClass);

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
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData: MSClass_t
**/
void CAPI2_PDP_GetMSClass(UInt32 tid, UInt8 clientID);

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
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	PCHTrafficFlowTemplate_t
**/
void CAPI2_PDP_GetUMTSTft(UInt32 tid, UInt8 clientID, UInt8 cid);

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
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_PDP_SetUMTSTft(UInt32 tid, UInt8 clientID, UInt8 cid, PCHTrafficFlowTemplate_t *pTft_in );

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
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_PDP_DeleteUMTSTft(UInt32 tid, UInt8 clientID, UInt8 cid);

//***************************************************************************************
/**
	Function to send request to activate SNDCP connection
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@param		l2p (in) layer 2 protocol ("PPP")
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_ACTIVATE_SNDCP_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData: PDP_DataState_t
**/
void CAPI2_PDP_ActivateSNDCPConnection(UInt32 tid, UInt8 clientID, UInt8 cid, PCHL2P_t l2p);

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
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData: None
**/
void CAPI2_PDP_DeactivateSNDCPConnection(UInt32 tid, UInt8 clientID, UInt8 cid);

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
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData: PDPDefaultContext_t
**/
void CAPI2_PDP_GetPDPDefaultContext(UInt32 tid, UInt8 clientID, UInt8 cid);

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
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData: PDP_GetPCHContext_Rsp_t
**/
void CAPI2_PDP_GetPCHContext(UInt32 tid, UInt8 clientID);

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
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData: PCHContextState_t
**/
void CAPI2_PDP_GetPCHContextState(UInt32 tid, UInt8 clientID, UInt8 cid);

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
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	PCHR99QosProfile_t
**/

void CAPI2_PDP_GetR99UMTSMinQoS(UInt32 tid, UInt8 clientID, UInt8 cid);

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
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	PCHR99QosProfile_t
**/

void CAPI2_PDP_GetR99UMTSQoS(UInt32 tid, UInt8 clientID, UInt8 cid);

//***************************************************************************************
/**
	Function to get UMTS min qos params for the given cid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_GETUMTSMINQOS_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	PCHUMTSQosProfile_t
**/

void CAPI2_PDP_GetUMTSMinQoS(UInt32 tid, UInt8 clientID, UInt8 cid);

//***************************************************************************************
/**
	Function to get UMTS qos params for the given cid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_GETUMTSQOS_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	PCHUMTSQosProfile_t
**/

void CAPI2_PDP_GetUMTSQoS(UInt32 tid, UInt8 clientID, UInt8 cid);

//***************************************************************************************
/**
	Function to get negotiated qos params for the given cid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_GETNEGQOS_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	PCHR99QosProfile_t
**/

void CAPI2_PDP_GetNegQoS(UInt32 tid, UInt8 clientID, UInt8 cid);

//***************************************************************************************
/**
	Function to set R99 UMTS min qos params for the given cid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@param		pR99MinQos (in) R99 UMTS min qos params
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_SETR99UMTSMINQOS_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/

void CAPI2_PDP_SetR99UMTSMinQoS(UInt32 tid, UInt8 clientID, UInt8 cid, PCHR99QosProfile_t *pR99MinQos);

//***************************************************************************************
/**
	Function to set R99 UMTS qos params for the given cid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@param		pR99Qos (in) R99 UMTS qos params
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_SETR99UMTSQOS_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/

void CAPI2_PDP_SetR99UMTSQoS(UInt32 tid, UInt8 clientID, UInt8 cid, PCHR99QosProfile_t *pR99Qos);

//***************************************************************************************
/**
	Function to set UMTS min qos params for the given cid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@param		pMinQos (in) UMTS min qos params
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_SETUMTSMINQOS_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/

void CAPI2_PDP_SetUMTSMinQoS(UInt32 tid, UInt8 clientID, UInt8 cid, PCHUMTSQosProfile_t *pMinQos);

//***************************************************************************************
/**
	Function to set UMTS qos params for the given cid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@param		pQos (in) UMTS qos params
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_SETUMTSQOS_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/

void CAPI2_PDP_SetUMTSQoS(UInt32 tid, UInt8 clientID, UInt8 cid, PCHUMTSQosProfile_t *pQos);

//***************************************************************************************
/**
	Function to get negotiated PCH parameters for the given cid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_GETNEGOTIATEDPARMS_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	PCHNegotiatedParms_t
**/

void CAPI2_PDP_GetNegotiatedParms(UInt32 tid, UInt8 clientID, UInt8 cid);


//***************************************************************************************
/**
	Utility function to build IP Config options for the CAPI2_PDP_SendPDPActivateReq function.
	@param		ipcnfg (in/out) The PCHProtConfig_t structure will be filled with config options
	@param		username (in) Username. Can be NULL or empty string
	@param		password (in) Username. Can be NULL or empty string
	@param		authType (in) REQUIRE_CHAP or REQUIRE_PAP

	@return		TRUE if OK or FALSE on error. 

	@note
	The purpose of the protocol configuration options information element is to 
	transfer external network protocol options associated with a PDP context activation. 
	( ex LCP, CHAP, PAP, and IPCP )
	The username and password chap/pap MD5 digest needs to be set in the options.
	The DNS primary address request etc.
	Refer to Protocol configuration options GSM 4.08 version 6.7.0, release 1997, 
	- section 10.5.6.3 ( Mobile radio interface layer 3 specification )
**/
Boolean Capi2BuildIpConfigOptions(PCHProtConfig_t *ipcnfg,
										char* username,
										char* password,
										IPConfigAuthType_t authType);

//***************************************************************************************
/**
	Utility function to parse IP Config options and retrieve DNS server address. 
	The config options is returned to user in the response to CAPI2_PDP_SendPDPActivateReq.
	@param		ipcnfg (in) The PCHProtConfig_t structure to parse.
	@param		primaryDns1 (in/out) First set primary DNS server address( Host byte order ).
	@param		secDns1 (in/out) First set secondary DNS server address( Host byte order ).
	@param		primaryDns2 (in/out) First set primary DNS server address( Host byte order ).
	@param		secDns2 (in/out) First set secondary DNS server address( Host byte order ).
	
	@return		TRUE if OK or FALSE on error. 

	@note
	If the ipcnfg is malformed OR does not contain the dns address then the input
	params are set to zero.
**/
Boolean Capi2ReadDnsSrv( PCHProtConfig_t *ipcnfg,
						   UInt32* primaryDns1,
						   UInt32* secDns1,
						   UInt32* primaryDns2,
						   UInt32* secDns2);

//***************************************************************************************
/**
	Function to get decoded protocol config options for the given cid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_READDECODEDPROTCONFIG_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	PCHDecodedProtConfig_t
**/
void CAPI2_ReadDecodedProtConfig( UInt32 tid, UInt8 clientID, UInt8 cid);

//***************************************************************************************
/**
	Function to build protocol config options
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		username (in) username
	@param		password (in) password
	@param		authType (in) authenticationType
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_BUILDPROTCONFIGOPTIONS_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	PCHProtConfig_t
**/
void CAPI2_BuildProtocolConfigOption(UInt32 tid, UInt8 clientID, char* username, char* password, IPConfigAuthType_t authType);

//***************************************************************************************
/**
	Function to build protocol config options with CHAP cc/cr or PAP po
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		authType (in) authenticationType
	@param		cc (in) CHAP challenge option
	@param		cr (in) CHAP response option
	@param		po (in) PAP configure option
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_BUILDPROTCONFIGOPTIONS2_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	PCHProtConfig_t
**/
void CAPI2_BuildProtocolConfigOption2(UInt32 tid, UInt8 clientID, IPConfigAuthType_t authType, CHAP_ChallengeOptions_t *cc, CHAP_ResponseOptions_t *cr, PAP_CnfgOptions_t *po);

/** @} */

void CAPI2_MS_IsGprsCallActive(UInt32 tid, UInt8 clientID, UInt8 chan);
void CAPI2_MS_SetChanGprsCallActive(UInt32 tid, UInt8 clientID, UInt8 chan, Boolean active);
void CAPI2_MS_SetCidForGprsActiveChan(UInt32 tid, UInt8 clientID, UInt8 chan, UInt8 cid);
void CAPI2_PDP_GetPPPModemCid(UInt32 tid, UInt8 clientID);
void CAPI2_MS_GetGprsActiveChanFromCid(UInt32 tid, UInt8 clientID, UInt8 cid);
void CAPI2_MS_GetCidFromGprsActiveChan(UInt32 tid, UInt8 clientID, UInt8 chan);

void CAPI2_PDP_GetPDPAddress(UInt32 tid, UInt8 clientID, UInt8 cid);
void CAPI2_PDP_SendTBFData(UInt32 tid, UInt8 clientID, UInt8 cid, UInt32 numberBytes);
void CAPI2_PDP_TftAddFilter(UInt32 tid, UInt8 clientID, UInt8 cid, PCHPacketFilter_T *pPktFilter);
void CAPI2_PDP_SetPCHContextState(UInt32 tid, UInt8 clientID, UInt8 cid, PCHContextState_t contextState);
void CAPI2_PDP_SetDefaultPDPContext(UInt32 tid, UInt8 clientID, UInt8 cid, PDPDefaultContext_t *pDefaultContext);


void CAPI2_PCHEx_SendPDPActivateReq(UInt32 tid, UInt8 clientID, UInt8 cid, PCHActivateReason_t reason, PCHProtConfig_t *protConfig);
void CAPI2_PCHEx_SendPDPModifyReq(UInt32 tid, UInt8 clientID, UInt8 cid);
void CAPI2_PCHEx_SendPDPDeactivateReq(UInt32 tid, UInt8 clientID, UInt8 cid);
void CAPI2_PCHEx_SendPDPActivateSecReq(UInt32 tid, UInt8 clientID, UInt8 cid);
void CAPI2_PDP_GetContextState(UInt32 tid, UInt8 clientID, UInt8 cid);
void CAPI2_PCHEx_GetDecodedProtConfig(UInt32 tid, UInt8 clientID, UInt8 cid);
void CAPI2_PCHEx_BuildIpConfigOptions(UInt32 tid, UInt8 clientID, char* username, char* password, IPConfigAuthType_t authType);
void CAPI2_PCHEx_BuildIpConfigOptions2(UInt32 tid, UInt8 clientID, IPConfigAuthType_t authType, CHAP_ChallengeOptions_t *cc, CHAP_ResponseOptions_t *cr, PAP_CnfgOptions_t *po);
void CAPI2_PCHEx_BuildIpConfigOptionsWithChapAuthType(UInt32 tid, UInt8 clientID, PCHEx_ChapAuthType_t *params);
void CAPI2_PDP_GetDefaultQos(UInt32 tid, UInt8 clientID);
void CAPI2_PDP_IsPDPContextActive(UInt32 tid, UInt8 clientID, UInt8 cid);
void CAPI2_PDP_GetDefinedPDPContextCidList(UInt32 tid, UInt8 clientID);
void CAPI2_PDP_SetPDPActivationCallControlFlag(UInt32 tid, UInt8 clientId, Boolean flag);
void CAPI2_PDP_GetPDPActivationCallControlFlag(UInt32 tid, UInt8 clientId);
void CAPI2_PDP_SendPDPActivateReq_PDU(UInt32 tid, UInt8 clientId, UInt8 cid, PCHPDPActivateCallControlResult_t callControlResult, PCHPDPActivatePDU_t *pdu);
void CAPI2_PDP_RejectNWIPDPActivation(UInt32 tid, UInt8 clientId, PCHPDPAddress_t pdpAddress, PCHRejectCause_t cause, PCHAPN_t apn);
void CAPI2_PDP_SetPDPBearerCtrlMode(UInt32 tid, UInt8 clientId, UInt8 mode);
void CAPI2_PDP_GetPDPBearerCtrlMode(UInt32 tid, UInt8 clientId);
void CAPI2_PDP_HandleActivateSecInd(UInt32 tid, UInt8 clientId, PDP_ActivateSecNWI_Ind_t *pActivateSecInd);
void CAPI2_PDP_SetPDPNWIControlFlag(UInt32 tid, UInt8 clientId, Boolean flag);
void CAPI2_PDP_GetPDPNWIControlFlag(UInt32 tid, UInt8 clientId);

#ifdef __cplusplus
}
#endif



#endif

