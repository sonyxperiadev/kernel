//***************************************************************************
//
//	Copyright © 2005-2008 Broadcom Corporation
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
*   @file   datacomm_api.h
*
*   @brief  This file defines the interface function for dlink(data connection) API 
			Services.
*
****************************************************************************/
/**

*   @defgroup   DATAConnectionGroup   Data Connection Management
*   @ingroup    DATAServiceGroup
*
*   @brief      This group defines the interface functions to intialize/setup/shutdown  
				data connections
*
*
****************************************************************************/

#ifndef _DATACOMM_API_H
#define _DATACOMM_API_H


/**
 * @addtogroup DATAConnectionGroup
 * @{
 */

/**
	This function handles data setup request. ATC/MMI must register their message posting callback function by using
	inClientId = SYS_RegisterForMSEvent(). ATC/MMI must store the ID for later purpose(like DeRegister).Data service 
	layer will notify application clients the data connection status with asynchronous events (refer to DC_ReportCallStatus_t)
	
	@param inClientID	(in)	Client id that registered to receive the DC connection status messages
	@param acctId		(in)	Data account id
	@param linkType		(in)	DC_MODEM_INITIATED=0, when use the phone as a modem (ppp server);								
								DC_MS_INITIATED=1, when use as GPRS phone application
	@return Result_t, Return PDP_CIRCUIT_CALL_ACTIVE if there is a voice or CSD call active and it is not UMTS network
					  Otherwise return generic RESULT_OK or RESULT_ERROR depending on DC setup conditions
**/
Result_t DC_SetupDataConnection(	UInt8							inClientID,
									UInt8							acctId, 
									DC_ConnectionType_t				linkType );

/**
	This function handles data setup requests for clients preferring to share a PDP context and APN. If no match is found, a new
    connection is established. Data service layer will notify application clients the data connection status with asynchronous 
    events (refer to DC_ReportCallStatus_t)
	
	@param inClientID	(in)	Client id that registered to receive the DC connection status messages
	@param acctId		(in)	Data account id
	@param linkType		(in)	DC_MODEM_INITIATED=0, when use the phone as a modem (ppp server);								
								DC_MS_INITIATED=1, when use as GPRS phone application
    @param apnCheck     (in)    APN associated with the connection client wants to share
    @param actDCAcctId  (out)   Account id of the active connection that is shared
	@return Result_t, Return PDP_CIRCUIT_CALL_ACTIVE if there is a voice or CSD call active and it is not UMTS network
					  Otherwise return generic RESULT_OK or RESULT_ERROR depending on DC setup conditions
**/
Result_t DC_SetupDataConnectionEx(	UInt8					inClientID,
									UInt8					acctId, 
									DC_ConnectionType_t		linkType,
                                    UInt8 *apnCheck, 
                                    UInt8 *actDCAcctId );

/**
	This function handles PDP context modification. User need to call DATA_SetGPRSQos() to set the new QoS parameters
	before calling this API to modify the QoS of the corresponding PDP context
	
	@param inClientID	(in)	Client id that registered to receive the DC connection status messages
	@param acctId		(in)	Data account id
	@param modifyCb 		(in) modify call back
	@return Result_t, Return RESULT_ERROR if the context can not be modified (ex. if the context is not active)
					  Otherwise return RESULT_OK 
**/
Result_t DC_ModifyDataConnection(	UInt8							inClientID,
									UInt8							acctId, 
									PCHGPRS_ModifyCb_t				modifyCb );


/**
	This function handles data shutdown request

	@param inClientID	(in)	Client id that registered to receive the DC connection status messages
	@param acctId		(in)	Data account id
	@return Result_t
**/
Result_t DC_ShutdownDataConnection(	UInt8							inClientID,								   
									UInt8							acctId );


/**
	This function retrieves the P-CSCF address received from the network upon successful PDP activation.

	@param acctId		(in)	Data account id
	@return const UInt8 *
**/
const UInt8 *DC_GetPCscfAddr(UInt8 acctId);

#ifdef DUAL_SIM_DATACOMM
Result_t DcApi_SetupDataConnection(	ClientInfo_t* inClientInfoPtr,
									UInt8					inAcctId, 
									DC_ConnectionType_t		linkType,		
									Result_t				rejectCode );
									
Result_t DcApi_ShutdownDataConnection(ClientInfo_t* inClientInfoPtr,	
								   UInt8	acctId );
#endif

/** @} */


#endif //_DATACOMM_API_H


