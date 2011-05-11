//***************************************************************************
//
//	Copyright © 2000-2008 Broadcom Corporation
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
*   @file   mndstypes.h
*
*   @brief  This file contains definitions for MNDS types.
*
****************************************************************************/

#ifndef _MSC_MNDSTYPES_H_
#define _MSC_MNDSTYPES_H_



//******************************************************************************
//
//	Typedefs for MNDS Messages
//
//******************************************************************************


//******************************************************************************
//
// Structure Typedefs
//
//******************************************************************************


//******************************************************************************
//
// MNDS Message Type Enumerations
//
//******************************************************************************

typedef enum
{
										// System messages:
										// ================
	MNDSMSG_NULL,						// Null message

										// DS messages:
										// =============
	MNDSMSG_DS_CALL_ACC,
	MNDSMSG_DS_DISC_REQ,
	MNDSMSG_DS_SERVICE_ACC,
	MNDSMSG_DS_SERVICE_REJ,
	MNDSMSG_DS_SETUP_REQ,
										// MN messages:
										// =============
	MNDSMSG_MN_BEARER_IND,
	MNDSMSG_MN_DISC_IND,
	MNDSMSG_MN_DISC_CNF,
	MNDSMSG_MN_SETUP_ACC,
	MNDSMSG_MN_SETUP_CNF,
	MNDSMSG_MN_SETUP_IND,
	MNDSMSG_MN_SETUP_REJ,
	MNDSMSG_MN_SYNC_IND,
									// SIMAP messages:
	MNDSMSG_SIMTOOLKIT_REFRESH_REQ,
	MNDSMSG_SIMTOOLKIT_RST_REQ,
	MNDSMSG_SIMTOOLKIT_RST_CONFIRM_REQ,
	MNDSMSG_DS_MODIFY_REQ


} MNDSMsgType_t;




//******************************************************************************
//
// System Messages, messages from MMI->MSC->MN->MNDS
//
//******************************************************************************

typedef struct
{
	UInt8				ti;
	T_SUBADDR			conn_subaddr;
} MNDSParmDSCallAcc_t;

typedef struct
{
	UInt8				ti;
	Cause_t 			cause;
} MNDSParmDSDiscReq_t;

typedef struct
{
	UInt8				ti;
	BearerCapability_t 	bc;
} MNDSParmDSServiceAcc_t;

typedef struct
{
	UInt8				ti;
	Cause_t 			cause;
} MNDSParmDSServiceRej_t;

typedef struct
{
	T_EST_CAUSE			est_cause;
	BearerCapability_t 	bc;
	LLCompatibility_t 	llc;
	HLCompatibility_t 	hlc;
	T_SUBADDR			calling_subaddr;
	TelephoneNumber_t 	phone_number;
	T_SUBADDR			called_subaddr;
	CLIRMode_t			clir_mode;
	T_MN_CUG			cug;
	Boolean				auto_call_ind;
} MNDSParmDSSetupReq_t;

typedef struct 
{
	BearerCapability_t 	bc;
	LLCompatibility_t 	llc;
	HLCompatibility_t 	hlc;
}MNDSParmMNModifyReq_t;

typedef struct
{
	CallIndex_t			ci;
	UInt8				ti;
	BearerCapability_t 	bc;
} MNDSParmMNBearerInd_t;

typedef struct
{
	CallIndex_t			ci;
	UInt8				ti;
	Cause_t 			cause;
} MNDSParmMNDiscInd_t;

typedef struct
{
	CallIndex_t			ci;
	UInt8				ti;
} MNDSParmMNDiscCnf_t;

typedef struct
{
	CallIndex_t			ci;
	UInt8				ti;
} MNDSParmMNSetupAcc_t;

typedef struct
{
	CallIndex_t			ci;
	UInt8				ti;
	TelephoneNumber_t 	phone_number;
	Subaddress_t		conn_subaddr;  
} MNDSParmMNSetupCnf_t;

typedef struct
{
	CallIndex_t			ci;
	UInt8				ti;
	BearerCapability_t 	bc;
	LLCompatibility_t 	llc;
	HLCompatibility_t 	hlc;
	TelephoneNumber_t 	phone_number;
	Subaddress_t		calling_subaddr;
	Subaddress_t		called_subaddr;
} MNDSParmMNSetupInd_t;

typedef struct
{
	CallIndex_t			ci;
	Cause_t 			cause;
} MNDSParmMNSetupRej_t;

typedef struct
{
	UInt8				ti;
    T_RAT               rat;
    Boolean             rab_assigned;   // For UMTS
	UInt8				chan_mode;
	UInt8				ts_uplink;		// Uplink Allocated Timeslots
	UInt8				ts_downlink;	// Downlink Allocated Timeslots
} MNDSParmMNSyncInd_t;

typedef struct
{
	Boolean			full_changed;
} MNDSParmSTKRefreshReq_t;

//******************************************************************************
//
// Combined MNDS messages
//
//******************************************************************************
typedef union
{
										// System parameters
										// =================
	
	
										// Internal parameters
										// ===================

										// DS parameters
										// ==============
	MNDSParmDSCallAcc_t		call_acc;
	MNDSParmDSDiscReq_t		disc_req;
	MNDSParmDSServiceAcc_t	service_acc;
	MNDSParmDSServiceRej_t	service_rej;
	MNDSParmDSSetupReq_t	setup_req;
										// MN parameters
										// ==============
	MNDSParmMNBearerInd_t	bearer_ind;
	MNDSParmMNDiscInd_t		disc_ind;
	MNDSParmMNDiscCnf_t		disc_cnf;
	MNDSParmMNSetupAcc_t	setup_acc;
	MNDSParmMNSetupCnf_t	setup_cnf;
	MNDSParmMNSetupInd_t	setup_ind;
	MNDSParmMNSetupRej_t	setup_rej;
	MNDSParmMNSyncInd_t		sync_ind;
	MNDSParmMNModifyReq_t	modify_req;

										 // SIMAP parameters
	MNDSParmSTKRefreshReq_t	stkRefreshReq;
} MNDSMsgParm_t;						// MNDS message parameters

//**************
// MNDS Message 
//**************
typedef struct
{
	MNDSMsgType_t	type;					// MNDS Message Type
	ClientInfo_t	clientInfo;				//< Client Information
	MNDSMsgParm_t	parm;					// MNDS Message Parameters
} MNDSMsg_t;							// MNDS Message

#define MNDS_MSG_SIZE					sizeof( MNDSMsg_t )

#endif

