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
*   @file   capi2_pch_api.h
*
*   @brief  This file defines the capi2 api's related to PCH
*
*	This file defines the interface for PCH API.This file provides
*	the function prototypes necessary to handle data services.
****************************************************************************/

#ifndef CAPI2_PCH_API_H
#define CAPI2_PCH_API_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup   CAPI2_PDP_APIGroup   PDP Context Handler
 * @addtogroup CAPI2_PDP_APIGroup
 * @{
 */

//***************************************************************************************
/**
	Function to set PDP context params for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		numParms (in)  number of parameters
	@param		pdpType (in)  PDP type
	@param		apn (in)  Access Point Name
	@param		pdpAddr (in)  PDP address
	@param		dComp (in)  data compression
	@param		hComp (in)  header compression
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_SETPDPCONTEXT_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_PdpApi_SetPDPContext(ClientInfo_t* inClientInfoPtr, UInt8 cid, UInt8 numParms, UInt8* pdpType, UInt8* apn, UInt8* pdpAddr, UInt8 dComp, UInt8 hComp);

//***************************************************************************************
/**
	Function to set secondary PDP context params for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		numParms (in)  number of parameters
	@param		priCid (in)  primary context id
	@param		dComp (in)  data compression
	@param		hComp (in)  header compression
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_SETSECPDPCONTEXT_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_PdpApi_SetSecPDPContext(ClientInfo_t* inClientInfoPtr, UInt8 cid, UInt8 numParms, UInt8 priCid, UInt8 dComp, UInt8 hComp);

//***************************************************************************************
/**
	Function to get PDP gprs QOS params for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_GETGPRSQOS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::Result_t
**/
void CAPI2_PdpApi_GetGPRSQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to SetGPRSQoS
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		NumPara (in) Param is NumPara
	@param		prec (in)  precedence
	@param		delay (in)  delay
	@param		rel (in)  reliability
	@param		peak (in)  peak
	@param		mean (in)  mean
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_SETGPRSQOS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_PdpApi_SetGPRSQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid, UInt8 NumPara, UInt8 prec, UInt8 delay, UInt8 rel, UInt8 peak, UInt8 mean);

//***************************************************************************************
/**
	Function to GetGPRSMinQoS
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_GETGPRSMINQOS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::Result_t
**/
void CAPI2_PdpApi_GetGPRSMinQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to SetGPRSMinQoS
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		NumPara (in) Param is NumPara
	@param		prec (in)  precedence
	@param		delay (in)  delay
	@param		rel (in)  reliability
	@param		peak (in)  peak
	@param		mean (in)  mean
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_SETGPRSMINQOS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_PdpApi_SetGPRSMinQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid, UInt8 NumPara, UInt8 prec, UInt8 delay, UInt8 rel, UInt8 peak, UInt8 mean);

//***************************************************************************************
/**
	Function to get UMTS Traffic Flow Template for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_GETUMTSTFT_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::Result_t
**/
void CAPI2_PdpApi_GetUMTSTft(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to set UMTS Traffic Flow Template for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		pTft_in (in)  packet filter
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_SETUMTSTFT_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_PdpApi_SetUMTSTft(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHTrafficFlowTemplate_t *pTft_in);

//***************************************************************************************
/**
	Function to delete UMTS Traffic Flow Template for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_DELETEUMTSTFT_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_PdpApi_DeleteUMTSTft(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function does the processing associated with an Exit Data State Request.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_DEACTIVATESNDCPCONNECTION_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_PdpApi_DeactivateSNDCPConnection(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to get minimum R99 UMTS Quality of Service for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_GETR99UMTSMINQOS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::Result_t
**/
void CAPI2_PdpApi_GetR99UMTSMinQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to get R99 UMTS Quality of Service for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_GETR99UMTSQOS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::Result_t
**/
void CAPI2_PdpApi_GetR99UMTSQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to get minimum UMTS Quality of Service for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_GETUMTSMINQOS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::Result_t
**/
void CAPI2_PdpApi_GetUMTSMinQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to get UMTS Quality of Service for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_GETUMTSQOS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::Result_t
**/
void CAPI2_PdpApi_GetUMTSQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to get negotiated qos params for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_GETNEGQOS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::Result_t
**/
void CAPI2_PdpApi_GetNegQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to set minimum R99 UMTS Quality of Service for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		pR99MinQos (in)  R99 QoS
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_SETR99UMTSMINQOS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_PdpApi_SetR99UMTSMinQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHR99QosProfile_t *pR99MinQos);

//***************************************************************************************
/**
	Function to set R99 UMTS Quality of Service for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		pR99Qos (in)  R99 QoS
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_SETR99UMTSQOS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_PdpApi_SetR99UMTSQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHR99QosProfile_t *pR99Qos);

//***************************************************************************************
/**
	Function to set minimum UMTS Quality of Service for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		pMinQos (in)  UMTS QoS
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_SETUMTSMINQOS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_PdpApi_SetUMTSMinQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHUMTSQosProfile_t *pMinQos);

//***************************************************************************************
/**
	Function to set UMTS Quality of Service for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		pQos (in)  UMTS QoS
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_SETUMTSQOS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_PdpApi_SetUMTSQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHUMTSQosProfile_t *pQos);

//***************************************************************************************
/**
	Function to get negotiated PCH parameters for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_GETNEGOTIATEDPARMS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::Result_t
**/
void CAPI2_PdpApi_GetNegotiatedParms(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to get the PDP address corresponding to given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_GETPDPADDRESS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::Result_t
**/
void CAPI2_PdpApi_GetPDPAddress(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	This function is used to send numberBytes bytes of data to SNDCP.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		numberBytes (in)  no of bytes to send
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_SENDTBFDATA_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_PdpApi_SendTBFData(ClientInfo_t* inClientInfoPtr, UInt8 cid, UInt32 numberBytes);

//***************************************************************************************
/**
	Function to add a packet filter to Traffic Flow Template for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		pPktFilter (in)  packet filter
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_TFTADDFILTER_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_PdpApi_TftAddFilter(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHPacketFilter_T *pPktFilter);

//***************************************************************************************
/**
	Function to set PCH context state for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		contextState (in)  PCH context state
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_SETPCHCONTEXTSTATE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_PdpApi_SetPCHContextState(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHContextState_t contextState);

//***************************************************************************************
/**
	Function to set default PDP context for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		pDefaultContext (in)  PDP default context
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_SETDEFAULTPDPCONTEXT_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_PdpApi_SetDefaultPDPContext(ClientInfo_t* inClientInfoPtr, UInt8 cid, PDPDefaultContext_t *pDefaultContext);

//***************************************************************************************
/**
	Function to get gets default Qos value.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_GET_DEFAULT_QOS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : void
**/
void CAPI2_PdpApi_GetDefaultQos(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to query if the context is active.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_ISCONTEXT_ACTIVE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : Boolean
**/
void CAPI2_PdpApi_IsPDPContextActive(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function does the processing associated with an Enter Data State Request AT_DATA_STATE.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		l2p (in)  layer 2 protocol "PPP"
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_ACTIVATE_SNDCP_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::PDP_DataState_t
**/
void CAPI2_PdpApi_ActivateSNDCPConnection(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHL2P_t l2p);

//***************************************************************************************
/**
	Function to get PDP context params for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_GETPDPCONTEXT_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::PDPDefaultContext_t
**/
void CAPI2_PdpApi_GetPDPContext(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to get the list of defined/activated PDP context.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_GETPDPCONTEXT_CID_LIST_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::Result_t
**/
void CAPI2_PdpApi_GetDefinedPDPContextCidList(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get PCH context state for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_GETPCHCONTEXTSTATE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::PCHContextState_t
**/
void CAPI2_PdpApi_GetPCHContextState(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to get PDP context params for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_GETPCHCONTEXT_EX_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::Result_t
**/
void CAPI2_PdpApi_GetPDPContextEx(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to set PDP activation call control flag
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		flag (in)  PDP activation call control flag
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_SETPDPACTIVATIONCCFLAG_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_PdpApi_SetPDPActivationCallControlFlag(ClientInfo_t* inClientInfoPtr, Boolean flag);

//***************************************************************************************
/**
	Function to get PDP activation call control flag
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_GETPDPACTIVATIONCCFLAG_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : Boolean
**/
void CAPI2_PdpApi_GetPDPActivationCallControlFlag(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to send pdp activation request with PDU to network
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		callControlResult (in)  call control result
	@param		pdu (in)  PDP activation PDU as defined in 9.5.1 of 24.008
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_ACTIVATION_PDU_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::PDP_SendPDPActivatePDUReq_Rsp_t
**/
void CAPI2_PdpApi_SendPDPActivateReq_PDU(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHPDPActivateCallControlResult_t callControlResult, PCHPDPActivatePDU_t *pdu);

//***************************************************************************************
/**
	Function to reject NWI Network Initiated PDP activation application.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inPdpAddress (in)  NWI PDP address
	@param		inCause (in)  reject cause
	@param		inApn (in)  access point name
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_REJECTNWIACTIVATION_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_PdpApi_RejectNWIPDPActivation(ClientInfo_t* inClientInfoPtr, PCHPDPAddress_t inPdpAddress, PCHRejectCause_t inCause, PCHAPN_t inApn);

//***************************************************************************************
/**
	This function is used to set MS Bearer Control Mode.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inMode (in) Param is inMode
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_SETBEARERCTRLMODE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_PdpApi_SetPDPBearerCtrlMode(ClientInfo_t* inClientInfoPtr, UInt8 inMode);

//***************************************************************************************
/**
	This function is used to get MS Bearer Control Mode.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_GETBEARERCTRLMODE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : UInt8
**/
void CAPI2_PdpApi_GetPDPBearerCtrlMode(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to reject the network initiated NWI activation <br>identified by secTiPd for the associated secondary PDP.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		pActivateSecInd (in)  information sent from HandleActivateSecInd.
	@param		inCause (in)  reject cause
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_REJECTSECNWIACTIVATION_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_PdpApi_RejectSecNWIPDPActivation(ClientInfo_t* inClientInfoPtr, PDP_ActivateSecNWI_Ind_t *pActivateSecInd, PCHRejectCause_t inCause);

//***************************************************************************************
/**
	Function to set NWI Network Initiated PDP activation control flag TRUE if NWI PDP is controlled by application.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inFlag (in)  NWI PDP activation control flag
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_SETPDPNWICONTROLFLAG_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_PdpApi_SetPDPNWIControlFlag(ClientInfo_t* inClientInfoPtr, Boolean inFlag);

//***************************************************************************************
/**
	Function get NWI Network Initiated PDP activation control flag  TRUE if NWI PDP is controlled by application.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_GETPDPNWICONTROLFLAG_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : Boolean
**/
void CAPI2_PdpApi_GetPDPNWIControlFlag(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to check UMTS Traffic Flow Template.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inTftPtr (in)  packet filter
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_CHECKUMTSTFT_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_PdpApi_CheckUMTSTft(ClientInfo_t* inClientInfoPtr, PCHTrafficFlowTemplate_t *inTftPtr);

//***************************************************************************************
/**
	Function to check if any context activation is in progress.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_ISANYPDPCONTEXTACTIVE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : Boolean
**/
void CAPI2_PdpApi_IsAnyPDPContextActive(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to check if any context activation is in progress.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_ISANYPDPCONTEXTPENDING_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : Boolean
**/
void CAPI2_PdpApi_IsAnyPDPContextActivePending(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to retrieve the previously stored Protocol Config Options for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inCid (in)  context id
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_GETPROTCONFIGOPTIONS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::Result_t
**/
void CAPI2_PdpApi_GetProtConfigOptions(ClientInfo_t* inClientInfoPtr, UInt8 inCid);

//***************************************************************************************
/**
	Function to store the Protocol Config Options for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inCid (in)  context id
	@param		inProtConfigPtr (in)  The Protocol Config Options to be stored
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_SETPROTCONFIGOPTIONS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_PdpApi_SetProtConfigOptions(ClientInfo_t* inClientInfoPtr, UInt8 inCid, PCHProtConfig_t *inProtConfigPtr);

//***************************************************************************************
/**
	Function to query if the context is defined as "Secondary PDP context."
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_ISSECONDARYPDPDEFINED_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::Boolean
**/
void CAPI2_PdpApi_IsSecondaryPdpDefined(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to get active contexts
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_GETGPRSACTIVATESTATUS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::Result_t
**/
void CAPI2_PdpApi_GetGPRSActivateStatus(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function is used to read valid PPP Modem Cid when PDP has been activated.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_GETPPPMODEMCID_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::PCHCid_t
**/
void CAPI2_PdpApi_GetPPPModemCid(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to send pdp activation request to Network
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		reason (in)  Reason for activation
	@param		protConfig (in)  Protocol config options
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_ACTIVATION_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::PDP_SendPDPActivateReq_Rsp_t
**/
void CAPI2_PchExApi_SendPDPActivateReq(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHActivateReason_t reason, PCHProtConfig_t *protConfig);

//***************************************************************************************
/**
	Function to send pdp modification request to Network
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_MODIFICATION_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::PDP_SendPDPModifyReq_Rsp_t
**/
void CAPI2_PchExApi_SendPDPModifyReq(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to send gprs pdp de-activation request to network
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_DEACTIVATION_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::PDP_SendPDPDeactivateReq_Rsp_t
**/
void CAPI2_PchExApi_SendPDPDeactivateReq(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to send gprs secondary pdp activation request to Network
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PDP_SEC_ACTIVATION_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::PDP_SendPDPActivateSecReq_Rsp_t
**/
void CAPI2_PchExApi_SendPDPActivateSecReq(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	To get the decoded protocol config options for the cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PCHEX_READDECODEDPROTCONFIG_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::Result_t
**/
void CAPI2_PchExApi_GetDecodedProtConfig(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	The purpose of the protocol configuration options information element is to <br>transfer external network protocol options associated with a PDP context activation. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		username (in)  null terminated username string
	@param		password (in)  null terminated password sring
	@param		authType (in)  authentication type
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PCHEX_BUILDPROTCONFIGOPTIONS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : Boolean
**/
void CAPI2_PchExApi_BuildIpConfigOptions(ClientInfo_t* inClientInfoPtr, char* username, char* password, IPConfigAuthType_t authType);

//***************************************************************************************
/**
	The purpose of the protocol configuration options information element is to <br>transfer external network protocol options associated with a PDP context activation.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		authType (in)  authorization type
	@param		cc (in)  challenge options
	@param		cr (in)  challenge response
	@param		po (in)  pap configure option
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PCHEX_BUILDPROTCONFIGOPTIONS2_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : void
**/
void CAPI2_PchExApi_BuildIpConfigOptions2(ClientInfo_t* inClientInfoPtr, IPConfigAuthType_t authType, CHAP_ChallengeOptions_t *cc, CHAP_ResponseOptions_t *cr, PAP_CnfgOptions_t *po);

//***************************************************************************************
/**
	The purpose of the protocol configuration options information element is to <br>transfer external network protocol options associated with a PDP context activation. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		params (in)  chap authentication type parameters
	
	 @n@b Responses 
	 @n@b MsgType_t: ::MSG_PCHEX_BUILDPROTCONFIGOPTION_CHAP_TYPE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : void
**/
void CAPI2_PchExApi_BuildIpConfigOptionsWithChapAuthType(ClientInfo_t* inClientInfoPtr, PCHEx_ChapAuthType_t *params);


/** @} */

#ifdef __cplusplus
}
#endif



#endif

