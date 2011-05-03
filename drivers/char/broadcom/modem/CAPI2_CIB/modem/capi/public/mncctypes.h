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
*   @file   mncctypes.h
*
*   @brief  This file contains definitions for MNCC message types.
*
****************************************************************************/

#ifndef _MSC_MNCCTYPES_H_
#define _MSC_MNCCTYPES_H_

//******************************************************************************
// Enumerations
//******************************************************************************
typedef enum
{
										// System messages:
										// ================
	CCMSG_NULL,							// 00, Null message
	CCMSG_SYS_SEND,						// 01, Initiate call
	CCMSG_SYS_SEND_DATA,				// 02, Initiate Data Call
	CCMSG_SYS_ACCEPT,					// 03, Accept incoming call
	CCMSG_SYS_HANGUP,					// 04, Hangup, all calls
	CCMSG_SYS_HOLD,						// 05, Place call on hold (swap calls)
	CCMSG_SYS_RETRIEVE,					// 06, Retrieve held call
	CCMSG_SYS_JOIN,						// 07, Merge all calls
	CCMSG_SYS_SPLIT,					// 08, Split selected call from MPTY
	CCMSG_SYS_ALLOC_TI_CNF,				// 09, Layer 3 Allocation of TI Confirm
	CCMSG_SYS_ALLOC_TI_REJ,				// 0A, Layer 3 Allocation of TI Reject 
	CCMSG_SYS_SETUP_ACC,				// 0B, MN accept the SETUP_IND
	CCMSG_SYS_SETUP_REJ,				// 0C, MN rejects the SETUP_IND
	CCMSG_SYS_CHARGE_PERMISSION,		// 0D, Call Charge Permission
	CCMSG_SYS_ECT,						// 0E, Explicit Call Transfer


										// Internal messages:
										// ==================
	CCMSG_SYS_BUILD_MPTY_TIMEOUT = 0x10,// 10, Build MPTY timeout 
	CCMSG_SYS_HOLD_MPTY_TIMEOUT,		// 11, Hold MPTY timeout
	CCMSG_SYS_RETRIEVE_MPTY_TIMEOUT,	// 12, Retrieve MPTY timeout
	CCMSG_SYS_SPLIT_MPTY_TIMEOUT,		// 13, Split MPTY timeout

										// MNCC messages:
										// =============
	CCMSG_MNCC_ALERT_IND,				// 14, Alerting called party
	CCMSG_MNCC_CALL_PROC_IND,			// 15,  
	CCMSG_MNCC_DISC_IND,				// 16, party has hung up
	CCMSG_MNCC_HOLD_CNF,				// 17, hold request confirmed
	CCMSG_MNCC_HOLD_REJ,				// 18, hold request rejected
	CCMSG_MNCC_MODIFY_CNF,				// 19,  
	CCMSG_MNCC_MODIFY_IND,				// 1A,  
	CCMSG_MNCC_NOTIFY_IND,				// 1B,  
	CCMSG_MNCC_PROGRESS_IND,			// 1C,  
	CCMSG_MNCC_REJECT_IND,				// 1D,  
	CCMSG_MNCC_RELEASE_CNF,				// 1E, call has terminated
	CCMSG_MNCC_RELEASE_IND,				// 1F, call has terminated
	CCMSG_MNCC_RETRIEVE_CNF,			// 20, retrieve request confirmed
	CCMSG_MNCC_RETRIEVE_REJ,			// 21, retrieve request rejected
	CCMSG_MNCC_SETUP_CNF,				// 22, caller has accepted call
	CCMSG_MNCC_SETUP_COMP_IND,			// 23, we have accepted call
	CCMSG_MNCC_SETUP_IND,				// 24, we are being alerted
	CCMSG_MNCC_SYNC_IND,				// 25, 
	CCMSG_MNCC_STATUS_IND,				// 26, 
	CCMSG_MNCC_USER_IND,				// 27, 
	CCMSG_MNCC_FACILITY_IND,			// 28, 

										// Internal messages: (more)
										// ==================
	CCMSG_SYS_MNCC_TIMER_TICK,			// 29, MNCC Timer Tick

										// message from RR to MN
	CCMSG_MNRR_CIPHERING_IND,			// 2A
	CCMSG_SYS_MODIFY_REQ,				// 2B HSCSD call modify Request

 	CCMSG_SYS_ECT_TIMEOUT,				// 2C, ECT timeout

	CCMSG_MNCC_SEND_DTMF_REQ,			// 2D Start DTMF Request
	CCMSG_MNCC_STOP_DTMF_REQ,			// 2E Stop DTMF Request
	CCMSG_MNCC_START_DTMF_CNF,			// 2F Start DTMF Confirmation
	CCMSG_MNCC_START_DTMF_REJ,			// 30 Start DTMF Reject
	CCMSG_MNCC_ABORT_DTMF_REQ,			// 31 Abort DTMF Request
	CCMSG_MNCC_STOP_DTMF_CNF,			// 32 Stop DTMF Confirmation
	CCMSG_MNCC_TIMER_EXP				// 33 Timer Expiration

} CCMsgType_t;


typedef enum {
	MNCC_USER_SUSPENDED		= 0,
	MNCC_USER_RESUMED		= 1,
	MNCC_BEARER_CHANGE		= 2
} NotificationInd_t;

typedef enum {
	MNCC_SUBADDRESS_NSAP			= 0,
	MNCC_SUBADDRESS_USER_SPECIFIED	= 1
} SubAddressType_t;



//******************************************************************************
// Structure Typedefs
//******************************************************************************

//*******************
// System parameters 
//*******************

typedef struct
{
	T_EST_CAUSE			est_cause;
	BearerCapability_t 	bc;
	LLCompatibility_t 	llc;
	HLCompatibility_t 	hlc;
	T_SUBADDR			calling_subaddr;
	TelephoneNumber_t 	phone_number;
	T_SUBADDR			called_subaddr;
	CLIRMode_t 			clir_mode;
	T_MN_CUG			cug;
	Boolean				auto_call_ind;
} CCParmSendData_t;

typedef struct
{
	CallIndex_t index;				// Call identification
	Cause_t	cause;
	Boolean acknowledge;			// TRUE, then acknowledge the Hangup
} CCParmHangup_t;

typedef struct
{
	CallIndex_t index;				// Call identification
} CCParmIndex_t;

//****************
// MNCC parameters 
//****************

typedef struct
{
	UInt8 ton;					// Type of number
	UInt8 npi;					// Numbering plan
	UInt8 c_num;				// Number of BCD digits in num
	UInt8 num[ MAX_DIGITS + 1];	// ASCII string, NULL terminated
} CalledParty_t;

typedef struct
{
	CallIndex_t			call_idx;
	UInt8				ti;
	PId_t				pid;
} CCParmAllocTICnf_t;

typedef struct
{
	CallIndex_t			call_idx;
} CCParmAllocTIRej_t;

typedef struct
{
	UInt8				ti;
	Cause_t				cause;
} CCParmRetrieveRej_t;

typedef struct
{
	ApiDtmf_t			dtmf;
} CCParmDtmfReq_t;

typedef struct
{
	UInt8				ti;
	UInt8				keypadFacility;
} CCParmStartDtmfCnf_t;

typedef struct
{
	UInt8				ti;
	Cause_t				cause;
} CCParmStartDtmfRej_t;

typedef struct
{
	UInt8			callIndex;
	SS_UserInfo_t	uusInfo;
} CCParmUusInfo_t;

typedef enum
{
	MNCC_START_DTMF_TIMER_TYPE,
	MNCC_STOP_DTMF_TIMER_TYPE,
	MNCC_DTMF_TONE_DURATION_TIMER_TYPE,
	MNCC_DTMF_WAIT_DURATION_TIMER_TYPE,
	MNCC_DTMF_PERIODIC_PAUSE_TIMER_TYPE
} MNCC_TimerType_t;

typedef struct
{
	UInt8 				callIndex;
	MNCC_TimerType_t	timerType;
} MNCC_Timer_t;

typedef struct
{
	UInt8				ti;
	ProgressInd_t		progress_desc;
} CCParmAlertInd_t;

typedef struct
{
	UInt8				ti;
	RepeatInd_t			ri;
	BearerCapability_t	bc1;
	BearerCapability_t	bc2;
	ProgressInd_t		progress_desc;
} CCParmCallProcInd_t;

typedef struct
{
	UInt8 ti;
	Cause_t				cause;
	ProgressInd_t		progress_desc;
    UInt8				raw_cause_ie[MAX_CAUSE_IE_LENGTH];
} CCParmDiscInd_t;

typedef struct
{
	UInt8				ti;
} CCParmGenericResult_t;

typedef struct
{
	UInt8				ti;
	Cause_t				cause;
	UInt8				rawCauseIe[MAX_CAUSE_IE_LENGTH];
} CCParmHoldRej_t;

typedef struct
{
	UInt8				ti;
	BearerCapability_t	bc;
	LLCompatibility_t	llc;
	HLCompatibility_t	hlc;
	Boolean				reverse;
} CCParmModifyCnf_t;

typedef struct
{
	UInt8				ti;
	BearerCapability_t	bc;
	LLCompatibility_t	llc;
	HLCompatibility_t	hlc;
	Boolean				reverse;
} CCParmModifyInd_t;

typedef struct 
{
	BearerCapability_t	bc;
	LLCompatibility_t	llc;
	HLCompatibility_t	hlc;

} CCParmModifyHSCSDCall_t;

typedef struct
{
	UInt8				ti;
	NotificationInd_t	nd;
} CCParmNotifyInd_t;

typedef struct
{
	UInt8				ti;
	ProgressInd_t		progress_desc;
} CCParmProgressInd_t;

typedef struct
{
	UInt8				ti;
	Cause_t				rej;		// mapped to Cause_t
} CCParmRejectInd_t;

typedef struct
{
	UInt8				ti;
	Cause_t				cause;
    UInt8				raw_cause_ie[MAX_CAUSE_IE_LENGTH];
} CCParmReleaseCnf_t;

typedef struct
{
	UInt8				ti;
	Cause_t				cause1;
	Cause_t				cause2;
    UInt8				raw_cause_ie[MAX_CAUSE_IE_LENGTH];
	Boolean				isInternalCause;	//this is used to indicate that MNCC needs to fake the rawcauseie sent in CALL_STATUS.
} CCParmReleaseInd_t;

typedef struct
{
	CallIndex_t index;				// Call identification
	BearerCapability_t	bc;
} CCParmSetupAcc_t;

typedef struct
{
	UInt8				ti;
	ProgressInd_t		progress_desc;
	CallPartyAdd_t		called;
	Subaddress_t		subaddress;
} CCParmSetupCnf_t;

typedef struct
{
	UInt8				ie;
	UInt8				ti;
	RepeatInd_t			bcri;
	BearerCapability_t	bc1;
	BearerCapability_t	bc2;
	ProgressInd_t		progress_desc;
	Signal_t			signal;
	CallingParty_t		calling;
	Subaddress_t		calling_sub;
	PartyAddress_t		calledPartyAdd;
	Subaddress_t		called_sub;
	RepeatInd_t			llc_ri;
	LLCompatibility_t	llc1;
	LLCompatibility_t	llc2;
	RepeatInd_t			hlc_ri;
	HLCompatibility_t	hlc1;
	HLCompatibility_t	hlc2;
	PId_t				pid;
	UInt8               causeNoCli;
} CCParmSetupInd_t;

typedef struct
{
	UInt8				ti;
	UInt8				sync_id;
	T_RAT				rat;
	Boolean				rab_assigned;
	UInt8				mode;
	UInt8				ts_uplink;
	UInt8				ts_downlink;
	UInt8				codecId;
} CCParmSyncInd_t;

typedef struct
{
	UInt8				ti;
	Cause_t				cause; 
} CCParmStatusInd_t;

typedef struct
{
	UInt8				ti;
	UInt8				congest_lev;
	UInt8				more_data;
} CCParmUserInd_t;

typedef struct
{
	Boolean is_call_permitted;
} CCParmChargePermission_t;

typedef struct
{
	UInt8				ti;
} CCParmFacilityInd_t;

typedef struct
{
	Boolean				full_changed;
	Boolean				acm_changed;
} CCParmSTKRefreshReq_t;

typedef struct
{
	Boolean				status;
} CCParmCipheringInd_t;

//*********************
// Combined parameters 
//*********************
typedef union
{
										// System parameters
										// =================
	CCParmSend_t			send;
	CCParmSendData_t		send_data;
	CCParmIndex_t			accept;
	CCParmHangup_t			hangup;
	CCParmIndex_t			hold;
	CCParmIndex_t			retrieve;
	CCParmIndex_t			join;
	CCParmIndex_t			split;
	CCParmIndex_t			ect;
	CCParmSetupAcc_t		setup_acc;
	CCParmHangup_t			setup_rej;
	CCParmChargePermission_t charge_permission;
	CCParmDtmfReq_t			api_dtmf_req;
	CCParmUusInfo_t			ccParamUusInfo;

										// MNCC parameters
										// ==============
	CCParmAllocTICnf_t		alloc_ti_cnf;
	CCParmAllocTIRej_t		alloc_ti_rej;
	CCParmStartDtmfCnf_t	start_dtmf_cnf;
	CCParmStartDtmfRej_t	start_dtmf_rej;
	CCParmGenericResult_t	stop_dtmf_cnf;
	MNCC_Timer_t			timer_exp;
	CCParmAlertInd_t		alert_ind;
	CCParmCallProcInd_t		call_proc_ind;
	CCParmDiscInd_t			disc_ind;
	CCParmGenericResult_t	hold_cnf;
	CCParmHoldRej_t			hold_rej;
	CCParmModifyCnf_t		modify_cnf;
	CCParmModifyInd_t		modify_ind;
	CCParmNotifyInd_t		notify_ind;
	CCParmProgressInd_t		progress_ind;
	CCParmRejectInd_t		reject_ind;
	CCParmReleaseCnf_t		release_cnf;
	CCParmReleaseInd_t		release_ind;
	CCParmGenericResult_t	retrieve_cnf;
	CCParmRetrieveRej_t		retrieve_rej;
	CCParmSetupCnf_t		setup_cnf;
	CCParmGenericResult_t	setup_comp_ind;
	CCParmSetupInd_t		setup_ind;
	CCParmSyncInd_t			sync_ind;
	CCParmStatusInd_t		status_ind;
	CCParmUserInd_t			user_ind;
	CCParmFacilityInd_t		facility_ind;
	CCParmSTKRefreshReq_t	stk_refresh_req;

	CCParmCipheringInd_t	ciphering_ind;	// Ciphering Indication
	CCParmModifyHSCSDCall_t modify_hscsd_call;

} CCMsgParm_t;							// MNCC message parameters

#define FACILITY_LEN				251
#define USER_LEN					131
#define SSVER_LEN					20	


typedef struct
{
	UInt8		val[FACILITY_LEN];
} Facility_t;

typedef Facility_t *FacilityPtr_t;

typedef struct
{
	UInt8		val[SSVER_LEN];
} SSVersionInd_t;

typedef struct
{
	UInt8		val[USER_LEN];
} UserToUser_t;

typedef UserToUser_t *UserToUserPtr_t;

//**************
// MNCC Message 
//**************
typedef struct
{
	CCMsgType_t		type;				///< MNCC Message Type
	ClientInfo_t	clientInfo;			///< API Client Identifier (ATC, MMI, and etc.)
	CCMsgParm_t		parm;				///< MNCC Message Parameters
	FacilityPtr_t	fac_inf;			///< NULL, if not used
	SS_UserToUser_t* user;		    	///< NULL, if not used
	UInt8			ti;					///< Transaction Identifier
	UInt8			ci;					///< Call Index
} MNCCMsg_t;							///< MNCC Message


#endif

