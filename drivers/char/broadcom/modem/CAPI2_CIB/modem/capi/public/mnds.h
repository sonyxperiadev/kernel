//***************************************************************************
//
//	Copyright © 1997-2008 Broadcom Corporation
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
*   @file   mnds.h
*
*   @brief  This file contains definitions for MNDS.
*
****************************************************************************/
#ifndef _MSC_MNDS_H_
#define _MSC_MNDS_H_

//******************************************************************************
// Function Prototypes
//******************************************************************************
// AoC 
UInt32 MNDS_ObtainDataBytes (	// Obtain the number of data bytes received 
								// or transmitted 
	CallIndex_t		call_index	// CallIndex
	);
// end 

//******************************************************************************
// The following are the general services provided by all interfaces
//******************************************************************************
void MNDS_Init( void );		   	// Initialize the MNDS API
void MNDS_Run( void );
void MNDS_Shutdown( void );
void MNDS_ProcessMsg( MNDSMsg_t	*msg );


//******************************************************************************
// Direct Call Definitions
//******************************************************************************
void MNDS_ReportDataCallReceived(	// Receive Data Call
	ClientInfo_t*		inClientInfoPtr,
	CallIndex_t			ci,					// CallIndex
	UInt8				ti,					// Transaction Id
	BearerCapability_t 	*bc,				// Bearer Capability
	LLCompatibility_t 	*llc,				// Low Level Compatibility
	HLCompatibility_t 	*hlc,				// High Level Compatibility
	TelephoneNumber_t 	*phone_number,		// Phone number of calling party
	Subaddress_t		*calling_subaddr,	// Calling subaddress
	Subaddress_t		*called_subaddr		// Called subaddress
	);
	
void MNDS_ReportDataCallConnected(	// Data Call Connected
	ClientInfo_t*		inClientInfoPtr,
	CallIndex_t 		ci,
	UInt8				ti,
	TelephoneNumber_t 	*phone_number,
	Subaddress_t		*subaddr 
	);
	
void MNDS_ReportDataCallCalling(	// Data Call has started
	ClientInfo_t*		inClientInfoPtr,
	CallIndex_t ci,					// CallIndex
	UInt8		ti					// Transaction Id
	);

void MNDS_RequestDataCallHangUp(	// Data Call is requested to Hang Up
	ClientInfo_t*		inClientInfoPtr,
	CallIndex_t ci,					// CallIndex
	Cause_t cause					// cause of Hang Up request
	);

void MNDS_ReportBearerCapabilityReceived(// Receive Bearer Capability
	ClientInfo_t*		inClientInfoPtr,
	CallIndex_t 		ci,
	UInt8				ti,
	BearerCapability_t *bc	
	);

void MNDS_SetDataCallMode(			// Set Data Service Mode
	DataCallMode_t mode				// Either allow or disallow DS
	);

void MNDS_DataCallSyncInd(
	ClientInfo_t*		inClientInfoPtr,
	CallIndex_t 		ci,	
	UInt8				ti,
    T_RAT               rat,
    Boolean             rab_assigned,
	UInt8				channel_mode,
	UInt8				ts_uplink,
	UInt8				ts_downlink
	);


void MNDS_GetDataCallBytes(UInt32* rxBytes,
						   UInt32* txBytes);

void MNDS_ResetDataCallBytes(void);

/* callback functions for MNDS_Register() */
void MNDS_SendDataCallReq(ClientInfo_t*	inClientInfoPtr,			// Callback to request connection
	T_EST_CAUSE			est_cause,
	BearerCapability_t 	*bc,				// Bearer Capability
	LLCompatibility_t 	*llc,				// Low Level Compatibility
	HLCompatibility_t 	*hlc,				// High Level Compatibility
	T_SUBADDR			*calling_subaddr,			
	TelephoneNumber_t 	*phone_number, 		// Phone Number to call
	T_SUBADDR			*called_subaddr,
	CLIRMode_t 			clir_mode,
	T_MN_CUG			*cug,
	Boolean				auto_call_ind);

void MNDS_ReportDataCallServiceStatus(
	ClientInfo_t* inClientInfoPtr,
	CallIndex_t ci,						// CallIndex
	Boolean is_accepted,				// TRUE, if Data Call Service accepted
	Cause_t reject_cause,				// if Data Call Service is rejected, then this is used
	BearerCapability_t *bc				// if Data Call Service is accepted, then this is used
	);

void MNDS_AcceptDataCall(ClientInfo_t* inClientInfoPtr, CallIndex_t c_idx);

void MNDS_DisconnectDataCall(	ClientInfo_t*	inClientInfoPtr,
								CallIndex_t		c_idx,
								Cause_t			cause);

void MN_ModifyHSCSDCall(ClientInfo_t*		inClientInfoPtr,
						BearerCapability_t 	*bc,
						LLCompatibility_t 	*llc,
						HLCompatibility_t 	*hlc);

void MN_PostMNDSMsg(MNDSMsg_t *mnds_msg);					// Post message to MNDS

int IsMNDS_InIdle( void );

#endif

