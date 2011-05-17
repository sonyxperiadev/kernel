//*********************************************************************
//
//	Copyright © 2010 Broadcom Corporation
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
*   @file   mncc_def.h
*
*   @brief	This file contains MNCC internal related defenitions.
*
****************************************************************************/
#ifndef _MNCC_DEF_H_
#define _MNCC_DEF_H_

typedef Int16 MNCCTimerTick_t;			///< check above for tick value

typedef enum
{
	CC_CTX_PARAM_NONE,					///< CC Contex parameter; None
	CC_CTX_PARAM_CALL_INDEX,			///< CC Contex parameter; Call Index
	CC_CTX_PARAM_CLIENT_INFO,			///< CC Contex parameter; Client Information
	CC_CTX_PARAM_TRANS_ID,				///< CC Contex parameter; Transaction Identifier
	CC_CTX_PARAM_CALL_STATE,			///< CC Contex parameter; Call State
	CC_CTX_PARAM_FOR_LATE_RETRIEVE,		///< CC Contex parameter; 
	CC_CTX_PARAM_MT_FOR_LATE_ACCEPT,	///< CC Contex parameter; 
	CC_CTX_PARAM_MO_FOR_LATE_SEND,		///< CC Contex parameter; 
	CC_CTX_ESTABLISHMENT_PHASE,			///< CC Contex establishment phase
	CC_CTX_ACTIVE_PHASE,				///< CC Contex active phase
	CC_CTX_RELEASE_PHASE,				///< CC Contex release phase
	CC_CTX_IS_ACTIVE_CALL,				///< CC Contex; is there any active call
	CC_CTX_IS_TCH_ASSIGNED,				///< CC Contex; is there any traffic channel assigned 
	CC_CTX_IS_HELD_CALL,				///< CC Contex; is there any help call
	CC_CTX_IS_DATA_CALL,				///< CC Contex; is there andy data call
	CC_CTX_PARAM_DATA_CALL_TYPE,		///< CC Contex parameter; data call type
	CC_CTX_PARAM_EMERGENCY_TYPE			///< CC Contex parameter; emergencey call type

}CC_CtxParam_t;							///< CC Context Parameter Type



typedef enum
{
	CCSTATE_IDLE,										///< 00= No call
	CCSTATE_MOC_CHECKING_CHARGE_PERMISSION,				///< 01= Checking if MOC has proper AOC permits
	CCSTATE_MOC_CALLING,								///< 02= Initiate a call
	CCSTATE_MOC_ALERTING,								///< 03= Call is ringing network
	CCSTATE_MTC_WAITING_FOR_EARLY_ASSIGN_SYNC_SPEECH,	///< 04= Waiting for SYNC indication from the Stack, early assignment, speech
	CCSTATE_MTC_WAITING_FOR_EARLY_ASSIGN_SYNC_DATA,		///< 05= Waiting for SYNC indication from the Stack, early assignment, data
	CCSTATE_MTC_WAITING_FOR_LATE_ASSIGN_SYNC,			///< 06= Waiting for SYNC indication from the Stack, late assignment
	CCSTATE_MTC_WAITING_FOR_EARLY_ASSIGN_USER,			///< 07= Waiting for user to answer call, early assignment
	CCSTATE_MTC_WAITING_FOR_LATE_ASSIGN_USER,			///< 08= Waiting for user to answer call, late assignment
	CCSTATE_MTC_ACCEPTING,								///< 09= Waiting for network to accept our answer
	CCSTATE_ACTIVE,										///< 0A= Call is active (check w/active_state)
	CCSTATE_DISCONNECT,									///< 0B= Disconnecting call
	CCSTATE_WAITING_FOR_RELEASE,						///< 0C= Waiting for Release
	CCSTATE_MTC_WAITING_FOR_SETUP_ACC,					///< 0D= Waiting for Data Call Services Accept
	CCSTATE_DISCONNECT_INDICATION,						///< 0E= Disconnecting call indication
	CCSTATE_MOC_WAITING_FOR_TI							///< 0F= Waiting for TI
} CCState_t;											///< Call-Control States

typedef enum
{
	ACTIVESTATE_NOT_ACTIVE,				///< 00= Call is not active yet
	ACTIVESTATE_ACTIVE,					///< 01= Call is active
	ACTIVESTATE_HOLDING,				///< 02= Trying to put call on hold
	ACTIVESTATE_HELD,					///< 03= Call is on hold
	ACTIVESTATE_RETRIEVE,				///< 04= Trying to retrieve call
	ACTIVESTATE_BUILD,					///< 05= Trying to conference call
	ACTIVESTATE_JOIN,					///< 06= Trying to add to conference call
	ACTIVESTATE_MPTY,					///< 07= Call is a conference call
	ACTIVESTATE_MPTY_HOLDING,			///< 08= Trying to hold a conference call
	ACTIVESTATE_MPTY_HELD,				///< 09= Conference call is on hold
	ACTIVESTATE_MPTY_RETRIEVE,			///< 0A= Trying to retrieve a conference call
	ACTIVESTATE_SPLIT,					///< 0B= Trying to have a private call (in conference)
	ACTIVESTATE_ECT						///< 0C= Trying to do ECT
} ActiveState_t;						///< Active Call-Control Sub-states

typedef enum
{
	TIMEREVENT_NOT_USED,				///< TimerEvent not used
	TIMEREVENT_BUILD,					///< TimerEvent for Build MPTY process
	TIMEREVENT_HOLD,					///< TimerEvent for Hold MPTY process
	TIMEREVENT_RETRIEVE,				///< TimerEvent for Retrieve MPTY process
	TIMEREVENT_SPLIT,					///< TimerEvent for SPLIT MPTY process
	TIMEREVENT_ECT,						///< TimerEvent for ECT process
	TIMEREVENT_CONNECT,					///< TimerEvent for Connect timeout
} TimerEvent_t;


typedef struct
{
	ClientInfo_t		clientInfo;			///< Client information
	UInt32				callStateStamp;		///< Call state stamp
	UInt32				callActiveStamp;	///< Call active stamp
	SS_NameInd_t*		nameIndicatorPtr;	///< Calling name indicator for CNAP
	CallPartyAdd_t		called;				///< Called Party Address

	CCState_t cc;						///< Call-Control state
	ActiveState_t active;				///< Active Call-Control Sub-state
	ActiveState_t prev_active;			///< Remember the old Call-Control Sub-state
	CallIndex_t ci;						///< Call Index
	UInt8 ti;							///< Transaction ID
	PId_t pid;							///< Process ID for CC entity
	BearerCapability_t serv1;			///< Service
	BearerCapability_t serv2;			///< Alternate(?) service
	RepeatInd_t	bcRepeat;
	LLCompatibility_t llc;				///< Low Layer Compatibility
	HLCompatibility_t hlc;				///< High Layer Compatibility
	//CalledParty_t called;				///< ID of network side party
	Subaddress_t called_sub;			 
	CallingParty_t calling;
	Subaddress_t   calling_sub;


	BitField isDataCall: 1;				///< TRUE if it is an Data call
	BitField isEmergencyCall: 1;		///< TRUE if it is an Emergency call
	BitField is_llc_used: 1;			///< TRUE if LLC is used on MO Call
	BitField is_hlc_used: 1;			///< TRUE if HLC is used on MO Call
	BitField is_bc2_used: 1;			///< TRUE if BC2 is used on MO Call
	BitField clir_sup: 1;				 
	BitField clir_invoke: 1;			 
	BitField is_moc: 1;					///< TRUE if Mobile Originated Call
	BitField error_tone_on: 1;			///< TRUE, then Error Tone generation is On
	BitField acknowledge_hangup: 1;		///< TRUE, if hangup request requires acknowledgement
	BitField is_mo_hangup: 1;			///< TRUE, MO hangup
	BitField is_endmpty_hangup:1;		///< the call is hangup by using CALLACTION_ENDMPTY
	BitField endmpty_hangup_done:1;		///< the hangup request by using CALLACTION_ENDMPTY is sent
	BitField is_end_hangup:1;			///< the call is hangup by using CALLACTION_END
	BitField timer_active: 1;			///< TRUE, timer is counting
	
	BitField for_late_retrieve:1;       ///< flag to set late call retrieve when an another call in release
	BitField mt_for_late_accept:1;      ///< flag to set late MO call when an another call in release
	BitField mo_for_late_send:1;        ///< flag to set late MT call when an another call in release

	BitField callPreConnected: 1;

	CallAction_t outstanding_call_action_req;	///< Contains the current outstanding
												///<  call action request
	ProgressInd_t prog_ind;				///< progress indicator
	Signal_t setup_ind_signal;			///< saved setup_ind.signal, used for speech and DS
	CallType_t call_type;				///< Identifies the type of call
	MNCCMsg_t *saved_msg;				///< if not NULL, then contains MOC message
										///< used when verifying call permission
	CUGInfo_t cug_info;					///< CUG Info
	Cause_t error_tone_cause;			///< Remembers error tone cause

	TimerEvent_t timer_event;			///< Timer Event to use when timer expires
	MNCCTimerTick_t timer_val;

	BitField reportDisEvt: 1;			///< TRUE if report the call disconnect event 

	DtmfState_t			dtmfState;		///< The DTMF state
	UInt8				dtmfIndex;		///< The DTMF Index
	SingleLinkList_t	dtmfList;		///< The DTMF Link List

	Timer_t		startDtmfTimerPtr;			///< This pointer is used for both Start DTMF timer
	Timer_t		stopDtmfTimerPtr;			///< This pointer is used for both Stop DTMF timer
	Timer_t		dtmfWaitDurationTimerPtr;	///< This pointer is used for the wait timer
	Timer_t		dtmfToneDurationTimerPtr;	///< This pointer is used for the DTMF duration timer
	Timer_t		dtmfPeriodicPauseTimerPtr;	///< This pointer is used for the DTMF periodic pause timer
	ClientInfo_t dtmfClientInfo;            ///< Client info for DTMF request

	UInt8		apiClientId;			///< Api client identifier for this session.
	Boolean			dtmfStopReqReceived;///< used to keep track of whether user has requested a STOPbefore we
										///<  receive the START_CNF back from the network.
	UInt8*		rawCauseIePtr;			///< used to store the pointer to the heap memory that stores the rawcauseIe sent by network in the Disconnect , Release message.


	UInt8			codecId;			///< indicates the codec type for the call as sent by the network, only valid for UMTS
										///< if 0x0A then UMTS AMR WB else default to Narrow Band
	
	UInt8*		rawHoldCauseIePtr;		///< used to store the pointer to the heap memory that stores the rawcauseIe sent by network in the Hold Rej message.
	SS_UserInfo_t	uusInfo;			///< User To User Information
	Cause_t         relCause;           ///< Call disonnect cause set by client
	UInt8           emergencySvcCat;    ///< Emergency Service Category Value
    UInt8           causeNoCli;         ///< Cause for no CLI
} CallDesc_t;							///< Call Description (in CALL state)


/// Structure : MNCC Module Type
typedef struct
{
    struct
	{
		BitField			autoReject: 1;		///< Auto Rejection
		BitField			textTelephony: 1;	///< Text Telephony

		MtCallHandling_t	mtCallHandling;///< Reject MT Call Handling

	} clientConfig;

	BitField			dtmfToneOn: 1;

	Ticks_t				dtmfSendTimeStamp;
	Ticks_t				dtmfToneDuration;
	Ticks_t				dtmfWaitDuration;
	Ticks_t				dtmfPeriodicPause;
	DtmfMethod_t		dtmfMethod;
	UInt8				ConnectedCallCount;

	Boolean				speech_chn_assigned;///< Speech Channel Assigned
	CallIndex_t			cw_call_index;		///< Call Waiting Call Index
	Timer_t 			mncc_timer;			///< MNCC timer
    UInt8               mpty_rel_count;     ///< Number of remaining MPTY calls at time of release

	SingleLinkList_t	ctxList;			///< MNCC Ctx List

	T_RAT				Rat;	///< RAT

	MSChannelMode_t		channelMode;					///< channel mode for speech and data retrieved from ssyparm or user configured
}MNCC_Module_t;								///< MNCC Module Type
                           
#endif //_MNCC_DEF_H_


