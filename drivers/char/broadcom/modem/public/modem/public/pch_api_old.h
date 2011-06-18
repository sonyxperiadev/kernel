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
*   @file   pch_api_old.h
*
*   @brief  This file contains definitions for OLD PCH (GPRS PDP Context Handler) API.
*
*	This file provides the function prototypes necessary to handle data services.
*
****************************************************************************/


/**
*	@defgroup	OldPDPContextHandlerGroup	Legacy PDP Context Handler Group
*	@ingroup	PDPContextHandlerGroup
*   @brief      This group provides legacy functions to support handling Packet Data
				Protocol (PDP) data services.  This group is no longer supported,
				and is being supported for backwards compatibility.
*	
	\n Use the link below to navigate back to the Data Services Overview page. \n
    \li \ref DATAServiceOverview
****************************************************************************/


#ifndef _PCH_API_OLD_
#define _PCH_API_OLD_



/**
 * @addtogroup OldPDPContextHandlerGroup
 * @{
 */
 
//***************************************************************************************
/**
	Function to set PDP gprs QOS params for the given cid
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
Result_t		PDP_SetGPRSQoS(UInt8 cid,UInt8 NumPara,UInt8 prec,UInt8 delay,UInt8 rel,UInt8 peak,UInt8 mean);

//***************************************************************************************
/**
	Function to get PDP gprs QOS params for the given cid
	@param		cid (in) Context ID
	@param		prec (out) precedence
	@param		delay (out) delay
	@param		rel (out) reliability
	@param		peak (out) peak
	@param		mean (out) mean
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PDP_GetGPRSQoS(UInt8 cid,UInt8 *prec,UInt8 *delay,UInt8 *rel,UInt8 *peak,UInt8 *mean);

//***************************************************************************************
/**
	Function to set Minimum Acceptable PDP gprs QOS params for the given cid
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
Result_t		PDP_SetGPRSMinQoS(UInt8 cid,UInt8 NumPara,UInt8 prec,UInt8 delay,UInt8 rel,UInt8 peak,UInt8 mean);

//***************************************************************************************
/**
	Function to get Minimum Acceptable PDP gprs QOS params for the given cid
	@param		cid (in) Context ID
	@param		prec (out) precedence
	@param		delay (out) delay
	@param		rel (out) reliability
	@param		peak (out) peak
	@param		mean (out) mean
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PDP_GetGPRSMinQoS(UInt8 cid,UInt8 *prec,UInt8 *delay,UInt8 *rel,UInt8 *peak,UInt8 *mean);

//***************************************************************************************
/**
	Function to set PDP context params for the given cid
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
Result_t		PDP_SetPDPContext(UInt8 cid,UInt8 numParms,UInt8 *pdpType,UInt8 *apn,UInt8 *pdpAddr,UInt8 dComp,UInt8 hComp);

 
//***************************************************************************************
/**
	Function to get PDP context params for the given cid
	@param		cid (in) Context ID
	@return		PDPDefaultContext_t (out) Default PDP context
	@note
**/
const PDPDefaultContext_t*	PDP_GetPDPContext(UInt8 cid);

//***************************************************************************************
/**
	Function to set secondary PDP context params for the given cid
	@param		cid (in) Context ID
	@param		numParms (in) number of parameters
	@param		priCid (in) primary context id
	@param		dComp (in) data compression
	@param		hComp (in) header compression
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PDP_SetSecPDPContext(UInt8 cid,UInt8 numParms, UInt8 priCid,UInt8 dComp,UInt8 hComp);

//***************************************************************************************
/**
	Function to get the PDP address corresponding to given cid
	@param		cid (in) Context ID
	@param		pdpAddr (out) PDP address
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PDP_GetPDPAddress(UInt8 cid, UInt8 *pdpAddr);

//***************************************************************************************
/**
	Function to get the list of defined/activated PDP context.	
	@param		numCid (out) number of cid
	@param		cidList (out) cid list
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PDP_GetDefinedPDPContextCidList(UInt8 *numCid, UInt8 *cidList);

//***************************************************************************************
/**
	Function to get gets default Qos value.	
	@param		pQos (out) pointer to default Qos
	@return		void
	@note
**/
void			PDP_GetDefaultQos(PCHQosProfile_t *pQos);

//***************************************************************************************
/**
	Function to query if the context is active.	
	@param		cid (in) context id
	@return		Boolean :		TRUE if context is active, FALSE else
	@note
**/
Boolean			PDP_IsPDPContextActive(UInt8 cid);

//***************************************************************************************
/**
	Function to check if any context has been activated..	
	@return		Boolean :		TRUE or FALSE
	@note
**/
Boolean			PDP_IsAnyPDPContextActive(void);

//***************************************************************************************
/**
	Function to check if any context activation is in progress..	
	@return		Boolean :		TRUE or FALSE
	@note
**/
Boolean PDP_IsAnyPDPContextActivePending(void);
//***************************************************************************************
/**
	Function to query if the context is defined as "Secondary PDP context."	
	@param		cid (in) context id
	@return		Boolean :		TRUE if context is active, FALSE else
	@note
**/
Boolean			PDP_IsSecondaryPdpDefined(UInt8 cid);


//***************************************************************************************
/**
	Function to get WAP context id
	@return		PCHCid_t : context id
	@note
**/
PCHCid_t		PDP_GetWAPCid(void); 

//***************************************************************************************
/**
	Function to MMS context id
	@return		PCHCid_t : context id
	@note
**/
PCHCid_t		PDP_GetMMSCid(void); 



//***************************************************************************************
/**
	Function to set PCH context state for the given cid
	@param		cid (in) : context id
	@param		contextState (in): PCH context state
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t PDP_SetPCHContextState(UInt8 cid, PCHContextState_t contextState);

//***************************************************************************************
/**
	Function to set default PDP context for the given cid
	@param		cid (in) context id
	@param		pDefaultContext (in) PDP default context
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t PDP_SetDefaultPDPContext(UInt8 cid, PDPDefaultContext_t *pDefaultContext);

//R99 UMTS
//***************************************************************************************
/**
	Function to set R99 UMTS Quality of Service for the given cid
	@param		cid (in) Context ID
	@param		pR99Qos (in) R99 QoS
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PDP_SetR99UMTSQoS(UInt8	cid, PCHR99QosProfile_t	*pR99Qos);

//***************************************************************************************
/**
	Function to get R99 UMTS Quality of Service for the given cid
	@param		cid (in) Context ID
	@param		pR99Qos (out) R99 QoS
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PDP_GetR99UMTSQoS(UInt8 cid, PCHR99QosProfile_t	*pR99Qos);

//***************************************************************************************
/**
	Function to set minimum R99 UMTS Quality of Service for the given cid
	@param		cid (in) Context ID
	@param		pR99Qos (in) R99 QoS
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PDP_SetR99UMTSMinQoS(UInt8 cid, PCHR99QosProfile_t	*pR99Qos);

//***************************************************************************************
/**
	Function to get minimum R99 UMTS Quality of Service for the given cid
	@param		cid (in) Context ID
	@param		pR99Qos (out) R99 QoS
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PDP_GetR99UMTSMinQoS(UInt8 cid, PCHR99QosProfile_t	*pR99Qos);

//***************************************************************************************
/**
	Function to set UMTS Quality of Service for the given cid
	@param		cid (in) Context ID
	@param		pSrcQos (in) UMTS QoS
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PDP_SetUMTSQoS(UInt8	cid, PCHUMTSQosProfile_t	*pSrcQos);

//***************************************************************************************
/**
	Function to get UMTS Quality of Service for the given cid
	@param		cid (in) Context ID
	@param		pDstQos (out) UMTS QoS
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PDP_GetUMTSQoS(UInt8 cid, PCHUMTSQosProfile_t	*pDstQos);

//***************************************************************************************
/**
	Function to set UMTS minimum Quality of Service for the given cid
	@param		cid (in) Context ID
	@param		pSrcQos (in) UMTS QoS
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PDP_SetUMTSMinQoS(UInt8	cid, PCHUMTSQosProfile_t	*pSrcQos);

//***************************************************************************************
/**
	Function to get UMTS minimum Quality of Service for the given cid
	@param		cid (in) Context ID
	@param		pDstQos (out) UMTS QoS
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PDP_GetUMTSMinQoS(UInt8 cid, PCHUMTSQosProfile_t	*pDstQos);

//***************************************************************************************
/**
	Function to add a packet filter to Traffic Flow Template for the given cid
	@param		cid (in) Context ID
	@param		pPktFilter (in) packet filter
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t 		PDP_TftAddFilter(UInt8	cid, PCHPacketFilter_T 	*pPktFilter);

//***************************************************************************************
/**
	Function to set UMTS Traffic Flow Template for the given cid
	@param		cid (in) Context ID
	@param		pUMTSPktFilter (in) packet filter
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PDP_SetUMTSTft(UInt8 cid, PCHTrafficFlowTemplate_t* pUMTSPktFilter);

//***************************************************************************************
/**
	Function to get UMTS Traffic Flow Template for the given cid
	@param		cid (in) Context ID
	@param		pUMTSTft (out) packet filter
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t 		PDP_GetUMTSTft(UInt8 cid,PCHTrafficFlowTemplate_t	*pUMTSTft );

//***************************************************************************************
/**
	Function to delete UMTS Traffic Flow Template for the given cid
	@param		cid (in) Context ID
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t        PDP_DeleteUMTSTft(UInt8 cid);

//***************************************************************************************
/**
	Function to check UMTS Traffic Flow Template.
	@param		pTft (in) packet filter
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PDP_CheckUMTSTft(PCHTrafficFlowTemplate_t* pTft);

//***************************************************************************************
/**
	Function to get negotiated qos params for the given cid
	@param		cid (in) Context ID
	@param		pR99Qos (in) Negotiated QoS
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PDP_GetNegQoS(UInt8 cid,PCHR99QosProfile_t	*pR99Qos );

//***************************************************************************************
/**
	Function to get negotiated PCH parameters for the given cid
	@param		cid (in) Context ID
	@param		pParms (in) negotiated parameters (qos, xid, sapi)
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t 		PDP_GetNegotiatedParms(UInt8 cid, PCHNegotiatedParms_t *pParms);

//******************************************************************************
//
// Function Name:	PdpApi_SetPDPBearerCtrlMode
//
// Description: This function is used to set network initiated PDP activation control flag.
// Notes:
//        The default for the bearer control mode is BEARER_CTRL_MODE_FOR_MS_ONLY.  
//        This function can be called to specify the bearer control mode as needed.  However, 
//        the mode will only be built into the protocol config table after calling  
//        PCHEx_BuildIpConfigOptions().
//
//******************************************************************************
/***
valid mode values:
BEARER_CTRL_MODE_FOR_MS_ONLY       0x01   ///< support MS Only
BEARER_CTRL_MODE_FOR_MS_AND_NW  0x02   ///< support MS and NW
***/
void PDP_SetPDPBearerCtrlMode(UInt8 inMode);


//******************************************************************************
//
// Function Name:	PdpApi_GetPDPBearerCtrlMode
//
// Description: This function is used to get network initiated PDP activation control flag
// Notes:
//
//******************************************************************************

UInt8 PDP_GetPDPBearerCtrlMode(void);


//***************************************************************************************
/**
	Function to reject the network initiated (NWI) activation 
	identified by secTiPd for the associated secondary PDP.
	@param	    inActivateSecIndPtr (in) information sent from HandleActivateSecInd.
	@param	    inCause (in) reject cause
	@return	    void
	@note
		Upon receipt of MSG_PDP_ACTIVATION_SEC_NWI_IND, the application can call 
		PdpApi_SendPDPActivateSecReq() to send the MT secondary pdp activation request or 
		call PDP_RejectSecNWIPDPActivation() to rejection the case with specific cause.
	
**/
void PDP_RejectSecNWIPDPActivation(
		PDP_ActivateSecNWI_Ind_t* inActivateSecIndPtr,
		PCHRejectCause_t		  inCause);


//***************************************************************************************
/**
	Function to set PDP activation call control flag 
	@param		inFlag (in) PDP activation call control flag, default is FALSE
	@return		void
	@note: should be used only once during system start up
**/
void 		PDP_SetPDPActivationCallControlFlag(Boolean inFlag);

//***************************************************************************************
/**
	Function to get PDP activation call control flag
	@return		flag (Boolean): PDP activation call control flag
	@note
**/
Boolean		PDP_GetPDPActivationCallControlFlag(void);

//***************************************************************************************
/**
	Function to set NWI (Network Initiated) PDP activation control flag, TRUE if NWI PDP is controlled by 
	application.
	@param		inFlag (in) NWI PDP activation control flag, default is FALSE
	@return		void
	@note: should be used only once during system start up
**/
void 		PDP_SetPDPNWIControlFlag(Boolean inFlag);

//***************************************************************************************
/**
	Function get NWI (Network Initiated) PDP activation control flag , TRUE if NWI PDP is controlled by 
	application.
	@return		flag (Boolean): NWI PDP call control flag
	@note
**/
Boolean		PDP_GetPDPNWIControlFlag(void);


//***************************************************************************************
/**
	Function to reject NWI (Network Initiated) PDP activation 
	application.
	@param		pdpAddress (in) NWI PDP address
	@param		cause (in) reject cause
	@param		apn (in) access point name
	@return		void
	@note
**/
void 		PDP_RejectNWIPDPActivation(
					PCHPDPAddress_t		pdpAddress,
					PCHRejectCause_t		cause,
					PCHAPN_t			apn);
	
//***************************************************************************************
/**
	Function to send pdp activation request to Network
	@param		clientID (in) Client ID
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
Result_t		PDP_SendPDPActivateReq(
					UInt8						clientID,
					UInt8						cid,
					PCHGPRS_ActivateCb_t		activatecb,
					PCHActivateReason_t			reason,
					PCHProtConfig_t				*protConfig );

//***************************************************************************************
/**
	Function to send pdp activation request (with PDU) to network
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@param		callControlResult (in) call control result 
	@param		pdu (in) PDP activation PDU as defined in 9.5.1 of 24.008
	@return		Result_t
	
**/
Result_t 		PDP_SendPDPActivateReq_PDU(
					UInt8							clientID,
					UInt8							cid,
					PCHPDPActivateCallControlResult_t	callControlResult,
					PCHPDPActivatePDU_t				*pdu);


//***************************************************************************************
/**
	Function to send gprs pdp de-activation request to network
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@param		deactivatecb (in) Deactivate callback function
	@return		Result_t: RESULT_OK, RESULT_ERROR
	@note
**/
Result_t		PDP_SendPDPDeactivateReq(
					UInt8						clientID,
					UInt8						cid,
					PCHGPRS_DeactivateCb_t		deactivatecb );


//***************************************************************************************
/**
	Function to send gprs secondary pdp activation request to Network
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@param		activateSecCb (in) Activate Sec Call Back
	@return		Result_t: RESULT_OK, RESULT_ERROR
	@note
**/
Result_t		PDP_SendPDPActivateSecReq(
					UInt8						clientID,
					UInt8						cid,
					PCHGPRS_ActivateSecCb_t		activateSecCb );

//***************************************************************************************
/**
	Function to send pdp modification request to Network
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@param		modifyCb (in) modify call back
	@return		Result_t: RESULT_OK, RESULT_ERROR
	@note
		Refer to Protocol configuration options GSM 4.08 version 6.7.0, release 1997, 
		- section 10.5.6.3 ( Mobile radio interface layer 3 specification )
		QoS to be modified should be set using the QoS set API
**/
Result_t		PDP_SendPDPModifyReq(
					UInt8						clientID,
					UInt8						cid,
					PCHGPRS_ModifyCb_t			modifyCb );

//***************************************************************************************
/**
	Function to get active contexts
	@param		numActiveCid (out) number of active contexts
	@param		outCidActivate (out) array of CID and Activate State 
	@return		Result_t: RESULT_OK, RESULT_ERROR
	@note
**/
Result_t		PDP_GetGPRSActivateStatus(UInt8 *numActiveCid, GPRSActivate_t *outCidActivate);

//***************************************************************************************
/**
	Function does the processing associated with an Enter Data State Request (AT_DATA_STATE). 

	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@param		l2p (in) layer 2 protocol ("PPP")
	@param		cb (in) data state callback function
	@return		Result_t: RESULT_OK, RESULT_ERROR
	@note
**/
Result_t		PDP_ActivateSNDCPConnection(UInt8 clientID, UInt8 cid, PCHL2P_t l2p, PCHGPRS_DataStateCb_t cb);

//***************************************************************************************
/**
	Function does the processing associated with an Exit Data State Request. 

	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@return		Result_t: RESULT_OK, RESULT_ERROR
	@note
**/
Result_t		PDP_DeactivateSNDCPConnection(UInt8 clientID, UInt8 cid);


//***************************************************************************************
/**
	Function to  create uplink memory pool in SNDCP for PPP-MODEM/MMI-IP-RELAY applications

	@return		Result_t: RESULT_OK, RESULT_ERROR
	@note
**/  

Result_t		PDP_CreateULSNPool(UInt8 cid);


//***************************************************************************************
/**
	Function to  delete uplink memory pool in SNDCP for PPP-MODEM/MMI-IP-RELAY applications

	@return		Result_t: RESULT_OK, RESULT_ERROR
	@note
**/  

Result_t		PDP_DeleteULSNPool(UInt8 cid);


//***************************************************************************************
/**
	This function is used to send numberBytes bytes of data to SNDCP.

	@param		cid (in) Context ID
	@param		numberBytes (in) no of bytes to send
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PDP_SendTBFData(UInt8 cid, UInt32 numberBytes);


//***************************************************************************************
/**
	This function is used to read valid PPP Modem Cid, when PDP has been activated.

	@return		PCHCid_t : context id, NOT_USED_FF if no PDP context has been activated.
	@note
**/
PCHCid_t		PDP_GetPPPModemCid(void); 

//***************************************************************************************
/**
	Function to get PCH context state for the given cid
	@param		cid (in)  context id
	@return		PCHContextState_t  PCH context state
	@note
**/
PCHContextState_t PDP_GetContextState(UInt8 cid);

Result_t PDP_GetPDPContextEx(UInt8 cid, PDPDefaultContext_t* pDefaultContext);


Result_t PDP_SetProtConfigOptions(
			UInt8 inCid, 
			PCHProtConfig_t* inProtConfigPtr);


Result_t PDP_GetProtConfigOptions(
			UInt8 inCid, 
			PCHProtConfig_t* outProtConfigPtr);


/** @} */

Result_t		SendSNPDUReq(UInt8 cid, UInt8 *pdu, UInt16 length);


#endif //#define _PCH_API_OLD_


