//***************************************************************************
//
//	Copyright © 2004-2008 Broadcom Corporation
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
*   @file   pchex_api.h
*
*   @brief  This file defines the extended capi api's related to establish 
*			data connection and data path for PS data.
*
****************************************************************************/
/**
*	@defgroup	PCHApiGroup	PCH API Group
*   @ingroup    DATAServiceGroup
*   @brief      This group provides functions to support handling PCH data services.
*
	\n Use the link below to navigate back to the Data Services Overview page. \n
    \li \ref DATAServiceOverview
*	
*	@defgroup	LegacyPCHApiGroup	Legacy PCH API Group
*	@ingroup	PCHApiGroup
*   @brief      This group provides legacy functions to support handling PCH data services.
*
	\n Use the link below to navigate back to the Data Services Overview page. \n
    \li \ref DATAServiceOverview
*	
**/
/********************************************************************************************/
#ifndef _PCHEX_API_H_
#define _PCHEX_API_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
	@addtogroup LegacyPCHApiGroup
	@{
*/

//***************************************************************************************
/**
	Function to initialize the PS data module.
	@return		None
	@note
**/
void PCHEx_Init(void);

//***************************************************************************************
/**
	Function to send pdp activation request to Network
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
Result_t PCHEx_SendPDPActivateReq(
					UInt8						clientID,
					UInt8						cid,
					PCHActivateReason_t			reason,
					PCHProtConfig_t				*protConfig );


//***************************************************************************************
/**
	Function to send pdp modification request to Network
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@return		Result_t
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_MODIFICATION_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	PDP_SendPDPModificateReq_Rsp_t
	@note
		Refer to Protocol configuration options GSM 4.08 version 6.7.0, release 1997, 
		- section 10.5.6.3 ( Mobile radio interface layer 3 specification )
		QoS to be modified should be set using the QoS set API
**/
Result_t PCHEx_SendPDPModifyReq(
					UInt8						clientID,
					UInt8						cid );


//***************************************************************************************
/**
	Function to send gprs pdp de-activation request to network
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@return		Result_t
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_DEACTIVATION_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData: PDP_SendPDPDeactivateReq_Rsp_t
**/
Result_t PCHEx_SendPDPDeactivateReq(UInt8 clientID, UInt8 cid);

//***************************************************************************************
/**
	Function to send gprs secondary pdp activation request to Network
	@param		clientID (in) Client ID
	@param		cid (in) Context ID
	@return		Result_t
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_SEC_ACTIVATION_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData: PDP_SendPDPActivateSecReq_Rsp_t
**/
Result_t PCHEx_SendPDPActivateSecReq(UInt8 clientID, UInt8 cid);

//***************************************************************************************
/**
	This function registers the callback functions to send the downlink 
	SN_DATA_IND/SN_UNITDATA_IND to clients.
	@param		sendSNDataToModem (in) callback to notify data on modem
	@param		sendSNDataToIpRelay (in) callback to notify data on iprelay
	@param		sendSNDataToSTK (in) callback to notify data on STK
	@param		Cb (in) flow control callback
	@return		None
	@note
**/
void PCHEx_RegisterRouteSNDataInd(	PCHGPRS_RouteSNDataCb_t		sendSNDataToModem,
									PCHGPRS_RouteSNDataCb_t		sendSNDataToIpRelay,
									PCHGPRS_RouteSNDataCb_t		sendSNDataToSTK,
									PCHEx_FlowCntrl_t		Cb
									);


//***************************************************************************************
/**
	This function registers the auxiliary callback functions to send the downlink 
	SN_DATA_IND/SN_UNITDATA_IND to clients.
	@param		sendSNDataToModem (in) callback to notify data on modem
	@param		sendSNDataToIpRelay (in) callback to notify data on iprelay
	@param		sendSNDataToSTK (in) callback to notify data on STK
	@param		Cb (in) flow control callback
	@param		getPDPActivateReason (in) callback to get PDP activate reason
	@return		None
	@note
**/
void PCHEx_RegisterRouteSNDataInd2(	PCHGPRS_RouteSNDataCb_t		sendSNDataToModem,
									PCHGPRS_RouteSNDataCb_t		sendSNDataToIpRelay,
									PCHGPRS_RouteSNDataCb_t		sendSNDataToSTK,
									PCHEx_FlowCntrl_t		Cb,
									PCHEx_GetActivateReason_t		getPDPActivateReason
									);


//***************************************************************************************
/**
	This function registers the auxiliary callback functions to send the downlink 
	SN_DATA_IND/SN_UNITDATA_IND to DUN (Dial Up Network) clilent.
	@param		sendSNDataToModem (in) callback to notify data on modem
	@param		Cb (in) flow control callback
	@param		getPDPActivateReason (in) callback to get PDP activate reason
	@return		None
	@note
**/
void PCHEx_RegisterRouteSNDataInd_DUN(	PCHGPRS_RouteSNDataCb_t		sendSNDataToModem,
									PCHEx_FlowCntrl_t		Cb,
									PCHEx_GetActivateReason_t		getPDPActivateReason
									);

//***************************************************************************************
/**
	Function to send the uplink data. 
	@param		cid (in) Context ID
	@param		pdu (in) packet data
	@param		length (in) length of the data packet in octets
	@return		Result_t
	@note
	Returns RESULT_ERROR for invalid cid, PS data connection does'nt exist for cid,
**/
Result_t PCHEx_SendSNPDUReq(UInt8 cid, UInt8 *pdu, UInt16 length);

//***************************************************************************************
/**
	Function to send the uplink data. 
	@param		cid (in) Context ID
	@param		pdu (in) packet data
	@param		length (in) length of the data packet in octets
	@return		Result_t
	@note
	Returns RESULT_ERROR for invalid cid, PS data connection does'nt exist for cid,
			RESULT_LOW_MEMORY if not free uplink buffer available
**/
Result_t PCHEx_SendSNPDUReqNoWait(UInt8 cid, UInt8 *pdu, UInt16 length);

//***************************************************************************************
/**
	The purpose of the protocol configuration options information element is to 
	transfer external network protocol options associated with a PDP context activation. 
	( ex LCP, CHAP, PAP, and IPCP )
	The username and password chap/pap MD5 digest needs to be set in the options.
	The DNS primary address request etc.
	Refer to Protocol configuration options GSM 4.08 version 6.7.0, release 1997, 
	- section 10.5.6.3 ( Mobile radio interface layer 3 specification )
	@param		cie (out) PCH protocol config option
	@param		username (in) null terminated username string
	@param		password (in) null terminated password sring
	@param		authType (in) authentication type
	@return		Boolean
	@note
	Returns TRUE if success else FALSE. For no password and username must pass empty string.
**/
Boolean PCHEx_BuildIpConfigOptions( PCHProtConfig_t *cie,
									char* username,
									char* password,
									IPConfigAuthType_t authType);

//***************************************************************************************
/**
	The purpose of the protocol configuration options information element is to 
	transfer external network protocol options associated with a PDP context activation. 
	( ex LCP, CHAP, PAP, and IPCP ).
	This function is used to build PCO with user provided cc, cr or po.
	Refer to Protocol configuration options GSM 4.08 version 6.7.0, release 1997, 
	- section 10.5.6.3 ( Mobile radio interface layer 3 specification )
	@param		ip_cnfg (out) PCH protocol config option
	@param		authType (in) authorization type
	@param		cc (in) challenge options
	@param		cr (in) challenge response
	@param		po (in) pap configure option
	@return		void
	@note
**/
void PCHEx_BuildIpConfigOptions2(	PCHProtConfig_t *ip_cnfg,
									IPConfigAuthType_t authType,
									CHAP_ChallengeOptions_t *cc,
									CHAP_ResponseOptions_t *cr,
									PAP_CnfgOptions_t *po);

//***************************************************************************************
/**
	The purpose of the protocol configuration options information element is to 
	transfer external network protocol options associated with a PDP context activation. 
	( ex LCP, CHAP, PAP, and IPCP )
	The chap authentication type data structure are set by the caller.
	The DNS primary address request etc.
	Refer to Protocol configuration options GSM 4.08 version 6.7.0, release 1997, 
	- section 10.5.6.3 ( Mobile radio interface layer 3 specification )
	@param		ip_cnfg (out) PCH protocol config option
	@param		params (in) chap authentication type parameters
	@return		none
	@note
**/
void PCHEx_BuildIpConfigOptionsWithChapAuthType( PCHProtConfig_t *ip_cnfg, PCHEx_ChapAuthType_t* params );

//***************************************************************************************
/**
	Function to get the received/sent byte count.
	@param		cid (in) Context ID
	@param		count (out) send/receive octet count on the cid
	@return		Result_t
	@note
**/
Result_t PCHEx_GetByteCount( UInt8 cid, PSDataByteCount_t* count);

//***************************************************************************************
/**
	To get the decoded protocol config options for the cid
	@param		cid  (in) context ID
	@param		out (out) decoded protocol config options.
	@return		RESULT_ERROR if error otherwise RESULT_OK.
	@note
**/
Result_t PCHEx_GetDecodedProtConfig( UInt8 cid,
						   PCHDecodedProtConfig_t* out); 


//***************************************************************************************
/**
	The purpose of this function is to handle UL data.
	@param		cid  (in) context ID
	@param		dataLen  (in) length of the data
	@param		pData  (in) pointer to the data
	@param		bFree  (in) whether or not to free memory
	@return		False if failed to handle UL data
	@note
**/
Boolean PCHEx_SendSNPDUReqNb(UInt8 cid,  UInt16 dataLen, UInt8* pData,Boolean bFree);

//***************************************************************************************
/**
	Function to get the flow control status for a specific PDP.
	@param		cid (in) Context ID
	@param		status (out) flow control status of the PDP associated with the cid. 
	@return		Result_t OK if no error
	@note
**/
Result_t PCHEX_GetFlowControlStatus(UInt8 cid, Boolean *status);


//***************************************************************************************
/**
	This function is called to confirm that PPP can now close.
	@param		cid (in) Context ID
	@return		none
	@note
**/
void PCHEX_PPP_ReportPPPCloseInd(UInt8	cid);
				
//***************************************************************************************
/**
	This function is called to deactivate the PDP context and shut down PPP stack.
	@param		cid (in) Context ID
	@return		Result_t OK if no error
	@note
**/
Result_t PCHEX_SendPPPCloseReq(UInt8 cid);


//***************************************************************************************
/**
	This function handles network initiated pdp deactivation from PCH layer.
	@param		cid (in) Context ID
	@param		pdpType (in) PCH PDP type
	@param		pdpAddress (in) PDP address
	@param		cause (in) OK if no error
	@return		none
	@note
**/
void PCHEX_PPP_ReportDeactivateContextInd(
	PCHCid_t 				cid,
	PCHPDPType_t			pdpType,
	PCHPDPAddress_t			pdpAddress,
	Result_t				cause
	);

//***************************************************************************************
/**
	Function to shutdown Tx task.
	@return		None
	@note
**/
void PCHEx_TxTaskShutdown(void);

//***************************************************************************************
/**
	Function to run Tx task.
	@return		None
	@note
**/
void PCHEx_TXTaskRun(void);

/** @} */

Boolean PCHEx_FlowControlEvent(UInt8 cid, Boolean bStart);

/**
	@addtogroup PCHApiGroup
	@{
*/

//***************************************************************************************
/**
	Function to send pdp activation request to Network
	@param		inClientInfoPtr (in) inClientInfoPtr
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
Result_t PchExApi_SendPDPActivateReq(
					ClientInfo_t*				inClientInfoPtr,
					UInt8						cid,
					PCHActivateReason_t			reason,
					PCHProtConfig_t				*protConfig );


//***************************************************************************************
/**
	Function to send pdp modification request to Network
	@param		inClientInfoPtr (in) inClientInfoPtr
	@param		cid (in) Context ID
	@return		Result_t
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_MODIFICATION_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	PDP_SendPDPModificateReq_Rsp_t
	@note
		Refer to Protocol configuration options GSM 4.08 version 6.7.0, release 1997, 
		- section 10.5.6.3 ( Mobile radio interface layer 3 specification )
		QoS to be modified should be set using the QoS set API
**/
Result_t PchExApi_SendPDPModifyReq(
					ClientInfo_t*				inClientInfoPtr,
					UInt8						cid );


//***************************************************************************************
/**
	Function to send gprs pdp de-activation request to network
	@param		inClientInfoPtr (in) inClientInfoPtr
	@param		cid (in) Context ID
	@return		Result_t
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_DEACTIVATION_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData: PDP_SendPDPDeactivateReq_Rsp_t
**/
Result_t PchExApi_SendPDPDeactivateReq(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to send gprs secondary pdp activation request to Network
	@param		inClientInfoPtr (in) inClientInfoPtr
	@param		cid (in) Context ID
	@return		Result_t
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_PDP_SEC_ACTIVATION_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData: PDP_SendPDPActivateSecReq_Rsp_t
**/
Result_t PchExApi_SendPDPActivateSecReq(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	This function registers the callback functions to send the downlink 
	SN_DATA_IND/SN_UNITDATA_IND to clients.
	@param		inClientInfoPtr (in) inClientInfoPtr
	@param		sendSNDataToModem (in) callback to notify data on modem
	@param		sendSNDataToIpRelay (in) callback to notify data on iprelay
	@param		sendSNDataToSTK (in) callback to notify data on STK
	@param		Cb (in) flow control callback
	@return		None
	@note
**/
void PchExApi_RegisterRouteSNDataInd(
									ClientInfo_t* 				inClientInfoPtr, 
									PCHGPRS_RouteSNDataCb_t		sendSNDataToModem,
									PCHGPRS_RouteSNDataCb_t		sendSNDataToIpRelay,
									PCHGPRS_RouteSNDataCb_t		sendSNDataToSTK,
									PCHEx_FlowCntrl_t			Cb
									);

//***************************************************************************************
/**
	This function registers the callback functions(with ClientInfo/simId) to send the downlink 
	SN_DATA_IND/SN_UNITDATA_IND to clients.
	@param		ClientInfoPtr (in) inClientInfoPtr
	@param		sendSNDataToModem (in) callback to notify data on modem
	@param		sendSNDataToIpRelay (in) callback to notify data on iprelay
	@param		sendSNDataToSTK (in) callback to notify data on STK
	@param		FlowCntrlCb (in) flow control callback
	@return		None
	@note
**/
void PchExApi_RegisterRouteSNDataInd2(	ClientInfo_t* ClientInfoPtr,
									PCHExApi_RouteSNDataCb_t		sendSNDataToModem,
									PCHExApi_RouteSNDataCb_t		sendSNDataToIpRelay,
									PCHExApi_RouteSNDataCb_t		sendSNDataToSTK,
									PCHExApi_FlowCntrl_t		FlowCntrlCb
									);

//***************************************************************************************
/**
	Function to send the uplink data. 
	@param		inClientInfoPtr (in) inClientInfoPtr
	@param		cid (in) Context ID
	@param		pdu (in) packet data
	@param		length (in) length of the data packet in octets
	@return		Result_t
	@note
	Returns RESULT_ERROR for invalid cid, PS data connection does'nt exist for cid,
**/
Result_t PchExApi_SendSNPDUReq(ClientInfo_t* inClientInfoPtr, UInt8 cid, UInt8 *pdu, UInt16 length);


//***************************************************************************************
/**
	Function to send the uplink data. 
	@param		ClientInfoPtr (in) inClientInfoPtr
	@param		cid (in) Context ID
	@param		pdu (in) packet data
	@param		length (in) length of the data packet in octets
	@return		Result_t
	@note
	Returns RESULT_ERROR for invalid cid, PS data connection does'nt exist for cid,
			RESULT_LOW_MEMORY if not free uplink buffer available
**/
Result_t PchExApi_SendSNPDUReqNoWait(ClientInfo_t* ClientInfoPtr, UInt8 cid, UInt8 *pdu, UInt16 length);


//***************************************************************************************
/**
	The purpose of the protocol configuration options information element is to 
	transfer external network protocol options associated with a PDP context activation. 
	( ex LCP, CHAP, PAP, and IPCP )
	The username and password chap/pap MD5 digest needs to be set in the options.
	The DNS primary address request etc.
	Refer to Protocol configuration options GSM 4.08 version 6.7.0, release 1997, 
	- section 10.5.6.3 ( Mobile radio interface layer 3 specification )
	@param		inClientInfoPtr (in) inClientInfoPtr
	@param		cie (out) PCH protocol config option
	@param		username (in) null terminated username string
	@param		password (in) null terminated password sring
	@param		authType (in) authentication type
	@return		Boolean
	@note
	Returns TRUE if success else FALSE. For no password and username must pass empty string.
**/
Boolean PchExApi_BuildIpConfigOptions(
									ClientInfo_t* inClientInfoPtr, 
									PCHProtConfig_t *cie,
									char* username,
									char* password,
									IPConfigAuthType_t authType);

//***************************************************************************************
/**
	The purpose of the protocol configuration options information element is to 
	transfer external network protocol options associated with a PDP context activation. 
	( ex LCP, CHAP, PAP, and IPCP )
	The chap authentication type data structure are set by the caller.
	The DNS primary address request etc.
	Refer to Protocol configuration options GSM 4.08 version 6.7.0, release 1997, 
	- section 10.5.6.3 ( Mobile radio interface layer 3 specification )
	@param		inClientInfoPtr (in) inClientInfoPtr
	@param		ip_cnfg (out) PCH protocol config option
	@param		params (in) chap authentication type parameters
	@return		none
	@note
**/
void PchExApi_BuildIpConfigOptionsWithChapAuthType( ClientInfo_t* inClientInfoPtr, PCHProtConfig_t *ip_cnfg, PCHEx_ChapAuthType_t* params  );

//***************************************************************************************
/**
	Function to get the received/sent byte count.
	@param		inClientInfoPtr (in) inClientInfoPtr
	@param		cid (in) Context ID
	@param		count (out) send/receive octet count on the cid
	@return		Result_t
	@note
**/
Result_t PchExApi_GetByteCount( ClientInfo_t* inClientInfoPtr, UInt8 cid, PSDataByteCount_t* count);

//***************************************************************************************
/**
	To get the decoded protocol config options for the cid
	@param		inClientInfoPtr (in) inClientInfoPtr
	@param		cid  (in) context ID
	@param		out (out) decoded protocol config options.
	@return		RESULT_ERROR if error otherwise RESULT_OK.
	@note
**/
Result_t PchExApi_GetDecodedProtConfig( ClientInfo_t* inClientInfoPtr, UInt8 cid,
						   PCHDecodedProtConfig_t* out); 

//***************************************************************************************
/**
	The purpose of this function is to handle UL data.
	@param		inClientInfoPtr (in) inClientInfoPtr
	@param		cid  (in) context ID
	@param		dataLen  (in) length of the data
	@param		pData  (in) pointer to the data
	@param		bFree  (in) whether or not to free memory
	@return		False if failed to handle UL data
	@note
**/
Boolean PchExApi_SendSNPDUReqNb(ClientInfo_t* inClientInfoPtr, UInt8 cid,  UInt16 dataLen, UInt8* pData,Boolean bFree);

//***************************************************************************************
/**
	Function to get the flow control status for a specific PDP.
	@param		inClientInfoPtr (in) inClientInfoPtr
	@param		cid (in) Context ID
	@param		status (out) flow control status of the PDP associated with the cid. 
	@return		Result_t OK if no error
	@note
**/
Result_t PchExApi_GetFlowControlStatus(ClientInfo_t* inClientInfoPtr, UInt8 cid, Boolean *status);

//***************************************************************************************
/**
	The purpose of the protocol configuration options information element is to 
	transfer external network protocol options associated with a PDP context activation. 
	( ex LCP, CHAP, PAP, and IPCP ).
	This function is used to build PCO with user provided cc, cr or po.
	Refer to Protocol configuration options GSM 4.08 version 6.7.0, release 1997, 
	- section 10.5.6.3 ( Mobile radio interface layer 3 specification )
	@param		inClientInfoPtr (in) inClientInfoPtr
	@param		ip_cnfg (out) PCH protocol config option
	@param		authType (in) authorization type
	@param		cc (in) challenge options
	@param		cr (in) challenge response
	@param		po (in) pap configure option
	@return		void
	@note
**/
void PchExApi_BuildIpConfigOptions2(
						  ClientInfo_t* inClientInfoPtr,
						  PCHProtConfig_t *ip_cnfg,
						  IPConfigAuthType_t authType,
						  CHAP_ChallengeOptions_t *cc,
						  CHAP_ResponseOptions_t *cr,
						  PAP_CnfgOptions_t *po);

typedef Result_t (*PchExApi_SendDataToNetwork_t)(ClientInfo_t* inClienInfoPtr, UInt8 inCid, UInt8* inBufferPtr, int inLen); 
void PchExApi_RegisterSendDataToNetworkCb(PchExApi_SendDataToNetwork_t inCbPtr );


/** @} */

#ifdef __cplusplus
}
#endif

#endif
