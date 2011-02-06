//***************************************************************************
//
//	Copyright ?2002-2008 Broadcom Corporation
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
*   @file   pch_api.h
*
*   @brief  This file contains definitions for PCH (GPRS PDP Context Handler) API.
*
*	This file provides the function prototypes necessary to handle data services.
*
****************************************************************************/
/**
*	@defgroup	PDPContextHandlerGroup	PDP Context Handler Group
*   @ingroup    DATAServiceGroup
*   @brief      This group provides functions to support handling Packet Data Protocol (PDP) data services.
*	
	\n Use the link below to navigate back to the Data Services Overview page. \n
    \li \ref DATAServiceOverview
****************************************************************************/

#ifndef _PCH_API_
#define _PCH_API_

#include "taskmsgs.h"
#include "pch_api_old.h"

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Beginning of New PCH API with ClientInfo_t
//  
//  PdpApi_XXXXX(ClientInfo_t inClientInfoPtr, ...)
//

/**
 * @addtogroup PDPContextHandlerGroup
 * @{
 */

//***************************************************************************************
/**
	Function to set PDP gprs QOS params for the given cid
	@param      inClientInfoPtr (in) Client information
	@param		cid (in) Context ID
	@param		NumPara (in) number of parameters
	@param		prec (in) precedence
	@param		delay (in) delay
	@param		rel (out) reliability
	@param		peak (in) peak
	@param		mean (in) mean
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PdpApi_SetGPRSQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid,UInt8 NumPara,UInt8 prec,UInt8 delay,UInt8 rel,UInt8 peak,UInt8 mean);

//***************************************************************************************
/**
	Function to get PDP gprs QOS params for the given cid
	@param      inClientInfoPtr (in) Client information
	@param		cid (in) Context ID
	@param		prec (out) precedence
	@param		delay (out) delay
	@param		rel (out) reliability
	@param		peak (out) peak
	@param		mean (out) mean
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PdpApi_GetGPRSQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid,UInt8 *prec,UInt8 *delay,UInt8 *rel,UInt8 *peak,UInt8 *mean);

//***************************************************************************************
/**
	Function to set Minimum Acceptable PDP gprs QOS params for the given cid
	@param      inClientInfoPtr (in) Client information
	@param		cid (in) Context ID
	@param		NumPara (in) number of parameters
	@param		prec (in) precedence
	@param		delay (in) delay
	@param		rel (out) reliability
	@param		peak (in) peak
	@param		mean (in) mean
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PdpApi_SetGPRSMinQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid,UInt8 NumPara,UInt8 prec,UInt8 delay,UInt8 rel,UInt8 peak,UInt8 mean);

//***************************************************************************************
/**
	Function to get Minimum Acceptable PDP gprs QOS params for the given cid
	@param      inClientInfoPtr (in) Client information
	@param		cid (in) Context ID
	@param		prec (out) precedence
	@param		delay (out) delay
	@param		rel (out) reliability
	@param		peak (out) peak
	@param		mean (out) mean
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PdpApi_GetGPRSMinQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid,UInt8 *prec,UInt8 *delay,UInt8 *rel,UInt8 *peak,UInt8 *mean);

//***************************************************************************************
/**
	Function to set PDP context params for the given cid
	@param      inClientInfoPtr (in) Client information
	@param		cid (in) Context ID
	@param		numParms (in) number of parameters
	@param		pdpType (in) PDP type
	@param		apn (in) Access Point Name
	@param		pdpAddr (in) PDP address
	@param		dComp (in) data compression
	@param		hComp (in) header compression
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PdpApi_SetPDPContext(ClientInfo_t* inClientInfoPtr, UInt8 cid,UInt8 numParms,UInt8 *pdpType,UInt8 *apn,UInt8 *pdpAddr,UInt8 dComp,UInt8 hComp);

 
//***************************************************************************************
/**
	Function to get PDP context params for the given cid
	@param      inClientInfoPtr (in) Client information
	@param		cid (in) Context ID
	@return		PDPDefaultContext_t (out) Default PDP context
	@note
**/
const PDPDefaultContext_t*	PdpApi_GetPDPContext(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to get PDP context params for the given cid
	@param		inClientInfoPtr (in) Client ID
	@param		cid (in) Context ID
	@param		pDefaultContext (out) Default PDP context
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PdpApi_GetPDPContextEx(ClientInfo_t* inClientInfoPtr, UInt8 cid, PDPDefaultContext_t* pDefaultContext);

//***************************************************************************************
/**
	Function to set secondary PDP context params for the given cid
	@param      inClientInfoPtr (in) Client information
	@param		cid (in) Context ID
	@param		numParms (in) number of parameters
	@param		priCid (in) primary context id
	@param		dComp (in) data compression
	@param		hComp (in) header compression
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PdpApi_SetSecPDPContext(ClientInfo_t* inClientInfoPtr, UInt8 cid,UInt8 numParms, UInt8 priCid,UInt8 dComp,UInt8 hComp);

//***************************************************************************************
/**
	Function to get the PDP address corresponding to given cid
	@param      inClientInfoPtr (in) Client information
	@param		cid (in) Context ID
	@param		pdpAddr (out) PDP address
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PdpApi_GetPDPAddress(ClientInfo_t* inClientInfoPtr, UInt8 cid, UInt8 *pdpAddr);

//***************************************************************************************
/**
	Function to get the list of defined/activated PDP context.	
	@param      inClientInfoPtr (in) Client information
	@param		numCid (out) number of cid
	@param		cidList (out) cid list
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PdpApi_GetDefinedPDPContextCidList(ClientInfo_t* inClientInfoPtr, UInt8 *numCid, UInt8 *cidList);

//***************************************************************************************
/**
	Function to get gets default Qos value.	
	@param      inClientInfoPtr (in) Client information
	@param		pQos (out) pointer to default Qos
	@return		void
	@note
**/
void			PdpApi_GetDefaultQos(ClientInfo_t* inClientInfoPtr, PCHQosProfile_t *pQos);

//***************************************************************************************
/**
	Function to query if the context is active.	
	@param      inClientInfoPtr (in) Client information
	@param		cid (in) context id
	@return		Boolean :		TRUE if context is active, FALSE else
	@note
**/
Boolean			PdpApi_IsPDPContextActive(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to check if any context has been activated.	
	@param      inClientInfoPtr (in) Client information
	@return		Boolean :		TRUE or FALSE
	@note
**/
Boolean			PdpApi_IsAnyPDPContextActive(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to check if any context activation is in progress.
	@param      inClientInfoPtr (in) Client information
	@return		Boolean :		TRUE or FALSE
	@note
**/
Boolean PdpApi_IsAnyPDPContextActivePending(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to query if the context is defined as "Secondary PDP context."	
	@param      inClientInfoPtr (in) Client information
	@param		cid (in) context id
	@return		Boolean :		TRUE if context is active, FALSE else
	@note
**/
Boolean			PdpApi_IsSecondaryPdpDefined(ClientInfo_t* inClientInfoPtr, UInt8 cid);


//***************************************************************************************
/**
	Function to get WAP context id
	@param      inClientInfoPtr (in) Client information
	@return		PCHCid_t : context id
	@note
**/
PCHCid_t		PdpApi_GetWAPCid(ClientInfo_t* inClientInfoPtr); 

//***************************************************************************************
/**
	Function to MMS context id
	@param      inClientInfoPtr (in) Client information
	@return		PCHCid_t : context id
	@note
**/
PCHCid_t		PdpApi_GetMMSCid(ClientInfo_t* inClientInfoPtr); 

//***************************************************************************************
/**
	Function to get PCH context state for the given cid
	@param      inClientInfoPtr (in) Client information
	@param		cid (in)  context id
	@return		PCHContextState_t  PCH context state
	@note
**/

PCHContextState_t PdpApi_GetPCHContextState(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to set PCH context state for the given cid
	@param      inClientInfoPtr (in) Client information
	@param		cid (in) : context id
	@param		contextState (in): PCH context state
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t PdpApi_SetPCHContextState(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHContextState_t contextState);

//***************************************************************************************
/**
	Function to set default PDP context for the given cid
	@param      inClientInfoPtr (in) Client information
	@param		cid (in) context id
	@param		pDefaultContext (in) PDP default context
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t PdpApi_SetDefaultPDPContext(ClientInfo_t* inClientInfoPtr, UInt8 cid, PDPDefaultContext_t *pDefaultContext);

//R99 UMTS
//***************************************************************************************
/**
	Function to set R99 UMTS Quality of Service for the given cid
	@param      inClientInfoPtr (in) Client information
	@param		cid (in) Context ID
	@param		pR99Qos (in) R99 QoS
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PdpApi_SetR99UMTSQoS(ClientInfo_t* inClientInfoPtr, UInt8	cid, PCHR99QosProfile_t	*pR99Qos);

//***************************************************************************************
/**
	Function to get R99 UMTS Quality of Service for the given cid
	@param      inClientInfoPtr (in) Client information
	@param		cid (in) Context ID
	@param		pR99Qos (out) R99 QoS
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PdpApi_GetR99UMTSQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHR99QosProfile_t	*pR99Qos);

//***************************************************************************************
/**
	Function to set minimum R99 UMTS Quality of Service for the given cid
	@param      inClientInfoPtr (in) Client information
	@param		cid (in) Context ID
	@param		pR99Qos (in) R99 QoS
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PdpApi_SetR99UMTSMinQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHR99QosProfile_t	*pR99Qos);

//***************************************************************************************
/**
	Function to get minimum R99 UMTS Quality of Service for the given cid
	@param      inClientInfoPtr (in) Client information
	@param		cid (in) Context ID
	@param		pR99Qos (out) R99 QoS
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PdpApi_GetR99UMTSMinQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHR99QosProfile_t	*pR99Qos);

//***************************************************************************************
/**
	Function to set UMTS Quality of Service for the given cid
	@param		inClientInfoPtr (in) Client ID
	@param		cid (in) Context ID
	@param		pSrcQos (in) UMTS QoS
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PdpApi_SetUMTSQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHUMTSQosProfile_t	*pSrcQos);

//***************************************************************************************
/**
	Function to get UMTS Quality of Service for the given cid
	@param		inClientInfoPtr (in) Client ID
	@param		cid (in) Context ID
	@param		pDstQos (out) UMTS QoS
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PdpApi_GetUMTSQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHUMTSQosProfile_t	*pDstQos);

//***************************************************************************************
/**
	Function to set minimum UMTS Quality of Service for the given cid
	@param		inClientInfoPtr (in) Client ID
	@param		cid (in) Context ID
	@param		pSrcQos (in) UMTS QoS
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PdpApi_SetUMTSMinQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHUMTSQosProfile_t	*pSrcQos);

//***************************************************************************************
/**
	Function to get minimum UMTS Quality of Service for the given cid
	@param		inClientInfoPtr (in) Client ID
	@param		cid (in) Context ID
	@param		pDstQos (out) UMTS QoS
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PdpApi_GetUMTSMinQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHUMTSQosProfile_t	*pDstQos);

//***************************************************************************************
/**
	Function to add a packet filter to Traffic Flow Template for the given cid
	@param      inClientInfoPtr (in) Client information
	@param		cid (in) Context ID
	@param		pPktFilter (in) packet filter
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t 		PdpApi_TftAddFilter(ClientInfo_t* inClientInfoPtr, UInt8	cid, PCHPacketFilter_T 	*pPktFilter);

//***************************************************************************************
/**
	Function to set UMTS Traffic Flow Template for the given cid
	@param      inClientInfoPtr (in) Client information
	@param		cid (in) Context ID
	@param		pUMTSPktFilter (in) packet filter
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PdpApi_SetUMTSTft(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHTrafficFlowTemplate_t* pUMTSPktFilter);

//***************************************************************************************
/**
	Function to get UMTS Traffic Flow Template for the given cid
	@param      inClientInfoPtr (in) Client information
	@param		cid (in) Context ID
	@param		pUMTSTft (out) packet filter
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t 		PdpApi_GetUMTSTft(ClientInfo_t* inClientInfoPtr, UInt8 cid,PCHTrafficFlowTemplate_t	*pUMTSTft );

//***************************************************************************************
/**
	Function to delete UMTS Traffic Flow Template for the given cid
	@param      inClientInfoPtr (in) Client information
	@param		cid (in) Context ID
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t        PdpApi_DeleteUMTSTft(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to get negotiated qos params for the given cid
	@param      inClientInfoPtr (in) Client information
	@param		cid (in) Context ID
	@param		pR99Qos (in) Negotiated QoS
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PdpApi_GetNegQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid,PCHR99QosProfile_t	*pR99Qos );

//***************************************************************************************
/**
	Function to get negotiated PCH parameters for the given cid
	@param      inClientInfoPtr (in) Client information
	@param		cid (in) Context ID
	@param		pParms (in) negotiated parameters (qos, xid, sapi)
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t 		PdpApi_GetNegotiatedParms(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHNegotiatedParms_t *pParms);

//***************************************************************************************
/**
	Function to set PDP activation call control flag 
	@param      inClientInfoPtr (in) Client information
	@param		flag (in) PDP activation call control flag, default is FALSE
	@return		void
	@note: should be used only once during system start up
**/
void 		PdpApi_SetPDPActivationCallControlFlag(ClientInfo_t* inClientInfoPtr, Boolean flag);

//***************************************************************************************
/**
	Function to get PDP activation call control flag
	@param      inClientInfoPtr (in) Client information
	@return		flag (Boolean): PDP activation call control flag
	@note
**/
Boolean		PdpApi_GetPDPActivationCallControlFlag(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to set NWI (Network Initiated) PDP activation control flag, TRUE if NWI PDP is controlled by 
	application.
	@param      inClientInfoPtr (in) Client information
	@param		flag (in) NWI PDP activation control flag, default is FALSE
	@return		void
	@note: should be used only once during system start up
**/
void 		PdpApi_SetPDPNWIControlFlag(ClientInfo_t* inClientInfoPtr, Boolean flag);

//***************************************************************************************
/**
	Function get NWI (Network Initiated) PDP activation control flag , TRUE if NWI PDP is controlled by 
	application.
	@param      inClientInfoPtr (in) Client information
	@return		flag (Boolean): NWI PDP call control flag
	@note
**/
Boolean		PdpApi_GetPDPNWIControlFlag(ClientInfo_t* inClientInfoPtr);


//***************************************************************************************
/**
	Function to reject NWI (Network Initiated) PDP activation 
	application.
	@param      inClientInfoPtr (in) Client information
	@param		pdpAddress (in) NWI PDP address
	@param		cause (in) reject cause
	@param		apn (in) access point name
	@return		void
	@note
**/
void 		PdpApi_RejectNWIPDPActivation(
					ClientInfo_t* inClientInfoPtr,
					PCHPDPAddress_t		pdpAddress,
					PCHRejectCause_t		cause,
					PCHAPN_t			apn);
	
//***************************************************************************************
/**
	Function to send pdp activation request to Network
	@param      inClientInfoPtr (in) Client information
	@param		cid (in) Context ID
	@param		activatecb (in) Activate callback function
	@param		reason (in) Reason for activation
	@param		protConfig (in) Protocol config options ( As per GSM L3 spec, see notes below )
	@return		None
	@note
		The purpose of the protocol configuration options information element is to 
		transfer external network protocol options associated with a PDP context activation. 
		( ex LCP, CHAP, PAP, and IPCP )
		The username and password chap/pap MD5 digest needs to be set in the options.
		The DNS primary address request etc.
		Refer to Protocol configuration options GSM 4.08 version 6.7.0, release 1997, 
		- section 10.5.6.3 ( Mobile radio interface layer 3 specification )
**/
Result_t		PdpApi_SendPDPActivateReq(
					ClientInfo_t* inClientInfoPtr,
					UInt8						cid,
					PCHGPRS_ActivateCb_t		activatecb,
					PCHActivateReason_t			reason,
					PCHProtConfig_t				*protConfig );

//***************************************************************************************
/**
	Function to send pdp activation request (with PDU) to network
	@param      inClientInfoPtr (in) Client information
	@param		cid (in) Context ID
	@param		callControlResult (in) call control result
	@param		pdu (in) PDP activation PDU as defined in 9.5.1 of 24.008
	@return		Result_t
	
**/
Result_t 		PdpApi_SendPDPActivateReq_PDU(
					ClientInfo_t* inClientInfoPtr,
					UInt8							cid,
					PCHPDPActivateCallControlResult_t	callControlResult,
					PCHPDPActivatePDU_t				pdu);

//***************************************************************************************
/**
	Function to send gprs pdp de-activation request to network
	@param      inClientInfoPtr (in) Client information
	@param		cid (in) Context ID
	@param		deactivatecb (in) Deactivate callback function
	@return		Result_t: RESULT_OK, RESULT_ERROR
	@note
**/
Result_t		PdpApi_SendPDPDeactivateReq(
					ClientInfo_t* inClientInfoPtr,
					UInt8						cid,
					PCHGPRS_DeactivateCb_t		deactivatecb );


//***************************************************************************************
/**
	Function to send gprs secondary pdp activation request to Network
	@param      inClientInfoPtr (in) Client information
	@param		cid (in) Context ID
	@param		activateSecCb (in) Activate Sec Call Back
	@return		Result_t: RESULT_OK, RESULT_ERROR
	@note
**/
Result_t		PdpApi_SendPDPActivateSecReq(
					ClientInfo_t* inClientInfoPtr,
					UInt8						cid,
					PCHGPRS_ActivateSecCb_t		activateSecCb );

//***************************************************************************************
/**
	Function to send pdp modification request to Network
	@param      inClientInfoPtr (in) Client information
	@param		cid (in) Context ID
	@param		modifyCb (in) modify call back
	@return		Result_t: RESULT_OK, RESULT_ERROR
	@note
		Refer to Protocol configuration options GSM 4.08 version 6.7.0, release 1997, 
		- section 10.5.6.3 ( Mobile radio interface layer 3 specification )
		QoS to be modified should be set using the QoS set API
**/
Result_t		PdpApi_SendPDPModifyReq(
					ClientInfo_t* inClientInfoPtr,
					UInt8						cid,
					PCHGPRS_ModifyCb_t			modifyCb );

//***************************************************************************************
/**
	Function to get active contexts
	@param      inClientInfoPtr (in) Client information
	@param		numActiveCid (out) number of active contexts
	@param		outCidActivate (out) array of CID and Activate State 
	@return		Result_t: RESULT_OK, RESULT_ERROR
	@note
**/
Result_t		PdpApi_GetGPRSActivateStatus(ClientInfo_t* inClientInfoPtr, UInt8 *numActiveCid, GPRSActivate_t *outCidActivate);

//***************************************************************************************
/**
	Function does the processing associated with an Enter Data State Request (AT_DATA_STATE). 

	@param      inClientInfoPtr (in) Client information
	@param		cid (in) Context ID
	@param		l2p (in) layer 2 protocol ("PPP")
	@param		cb (in) data state callback function
	@return		Result_t: RESULT_OK, RESULT_ERROR
	@note
**/
Result_t		PdpApi_ActivateSNDCPConnection(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHL2P_t l2p, PCHGPRS_DataStateCb_t cb);

//***************************************************************************************
/**
	Function does the processing associated with an Exit Data State Request. 

	@param      inClientInfoPtr (in) Client information
	@param		cid (in) Context ID
	@return		Result_t: RESULT_OK, RESULT_ERROR
	@note
**/
Result_t		PdpApi_DeactivateSNDCPConnection(ClientInfo_t* inClientInfoPtr, UInt8 cid);


//***************************************************************************************
/**
	Function to  create uplink memory pool in SNDCP for PPP-MODEM/MMI-IP-RELAY applications

	@param      inClientInfoPtr (in) Client information
	@param		cid (in)  context id
	@return		Result_t: RESULT_OK, RESULT_ERROR
	@note
**/  

Result_t		PdpApi_CreateULSNPool(ClientInfo_t* inClientInfoPtr, UInt8 cid);


//***************************************************************************************
/**
	Function to  delete uplink memory pool in SNDCP for PPP-MODEM/MMI-IP-RELAY applications

	@param      inClientInfoPtr (in) Client information
	@param		cid (in)  context id
	@return		Result_t: RESULT_OK, RESULT_ERROR
	@note
**/  

Result_t		PdpApi_DeleteULSNPool(ClientInfo_t* inClientInfoPtr, UInt8 cid);


//***************************************************************************************
/**
	This function is used to send numberBytes bytes of data to SNDCP.

	@param      inClientInfoPtr (in) Client information
	@param		cid (in) Context ID
	@param		numberBytes (in) no of bytes to send
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PdpApi_SendTBFData(ClientInfo_t* inClientInfoPtr, UInt8 cid, UInt32 numberBytes);


//***************************************************************************************
/**
	This function is used to read valid PPP Modem Cid, when PDP has been activated.

	@param      inClientInfoPtr (in) Client information
	@return		PCHCid_t : context id, NOT_USED_FF if no PDP context has been activated.
	@note
**/
PCHCid_t		PdpApi_GetPPPModemCid(ClientInfo_t* inClientInfoPtr); 



/** @} */



#endif //#define _PCH_API_


