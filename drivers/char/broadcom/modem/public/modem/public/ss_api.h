//*********************************************************************
//
//	Copyright © 2008-2009 Broadcom Corporation
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
*   @file   ss_api.h
*
*   @brief  This file defines the interface for Supplementary Services API.
*
****************************************************************************/ 
/**
*   @defgroup   SSAPIGroup   Supplementary Services
*   @ingroup    TelephonyGroup
*
*   @brief      This group defines the interfaces to the supplementary
*				services group.
*
*				This group defines the interface for supplementary services API.
*				It provides the function prototypes necessary to
*				activate, register, interrogate, enable, and disable supplementary
*				services such as call forwarding, call barring, calling line
*				presentation for speech, data, fax and other call types. 
*****************************************************************************/
#ifndef _SS_API_H_
#define _SS_API_H_




//**************************************************************************************
//*****************************************NEW SS***************************************
//**************************************************************************************
//**************************************************************************************

/**
 * @addtogroup SSAPIGroup
 * @{
 */

/**
    Function to initiate a SS or USSD session with the network. The response
    primitive which caries the network respone back to the client is as follows:
    For SS/USSD related cases:
        MSG_MNSS_CLIENT_SS_SRV_RSP (for Facility msg from the NW) or
        MSG_MNSS_CLIENT_SS_SRV_REL (for Release Complete msg from the network)
 
    For MO/MT-LR related cases:
        MSG_MNSS_CLIENT_LCS_SRV_RSP (for Facility msg from the NW) or
        MSG_MNSS_CLIENT_LCS_SRV_REL (for Release Complete msg from the network)

	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inApiSrvReqPtr (in) Pointer to the SS API service request object.
	@return		Result_t
**/
Result_t SsApi_SsSrvReq(ClientInfo_t*	inClientInfoPtr,
						SsApi_SrvReq_t* inApiSrvReqPtr);


/**
	Function to continues the current SS session with the network. The response
	primitive which caries the network respone back to the client;
	For SS/USSD related cases:
		MSG_MNSS_CLIENT_SS_SRV_RSP (for Facility msg from the NW) or
		MSG_MNSS_CLIENT_SS_SRV_REL (for Release Complete msg from the network)

	For MO/MT-LR related cases:
		MSG_MNSS_CLIENT_LCS_SRV_RSP (for Facility msg from the NW) or
		MSG_MNSS_CLIENT_LCS_SRV_REL (for Release Complete msg from the network)
	These messages are broadcast to all the registered clients.

	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inDataReqPtr (in) Pointer to the SS API data request object.
	@return		Result_t
**/
Result_t SsApi_DataReq(	ClientInfo_t*		inClientInfoPtr,
						SsApi_DataReq_t*	inDataReqPtr);


//**************************************************************************************
/**
	Function to initiate a new USSD request. which causes the MNSS state machine to
	establish a USSD session.
    The response primitive which carries the network respone back to the client:

		MSG_MNSS_CLIENT_SS_SRV_RSP (for Facility msg from the network) or
		MSG_MNSS_CLIENT_SS_SRV_REL (for Release Complete msg from the network).
	These messages are broadcast to all the registered clients.


	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inApiSrvReqPtr (in) Pointer to the USSD service request object.
	@return		Result_t

	@sa 		::SsApi_UssdDataReq()
**/
Result_t SsApi_UssdSrvReq(	ClientInfo_t*		inClientInfoPtr,
							SsApi_UssdSrvReq_t* inApiSrvReqPtr);


//**************************************************************************************
/**
	Function to continues the current USSD session with the network
	The response primitive which carries the network respone back to the client;
		MSG_MNSS_CLIENT_SS_SRV_RSP (for Facility msg from the network) or
		MSG_MNSS_CLIENT_SS_SRV_REL (for Release Complete msg from the network).
		
	These messages are broadcast to all the registered clients.

	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inApiSrvDataReqPtr (in) Pointer to the USSD data request object.
	@return		Result_t

	@sa			::SsApi_UssdSrvReq()
**/
Result_t SsApi_UssdDataReq(	ClientInfo_t*			inClientInfoPtr,
							SsApi_UssdDataReq_t*	inApiSrvDataReqPtr);


//**************************************************************************************
/**
	Function to initiate a release request, which causes the MNSS state machine to
	send a release complete message to the network. There is no response for this request.

	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inApiRelReqPtr (in) Pointer to the SS API service request object.
	@return		Result_t
**/
Result_t SsApi_SsReleaseReq(ClientInfo_t*	inClientInfoPtr,
							SsApi_SrvReq_t* inApiRelReqPtr);


//**************************************************************************************
/**
	Function pars the SS String, then it initiats a new SS request, which causes the
	MNSS state machine to establish a SS session.

	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inDialStrSrvReqPtr (in) Dial Str Service Request Pointer.
	@return		Result_t
**/
Result_t SsApi_DialStrSrvReq(	ClientInfo_t*			inClientInfoPtr,
								SsApi_DialStrSrvReq_t*	inDialStrSrvReqPtr);

/** @} */



#endif // _SS_API_H_

