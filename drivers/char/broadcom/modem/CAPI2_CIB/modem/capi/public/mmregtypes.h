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
*   @file   mmregtypes.h
*
*   @brief  This file contains the MMREG message types.
*
****************************************************************************/

#ifndef _MMREGTYPES_H_
#define _MMREGTYPES_H_


//******************************************************************************
// Enumerations
//******************************************************************************

typedef enum
{
	MMMSG_GMMREG_ATTACH_CNF,
	MMMSG_GMMREG_ATTACH_REJ,
	MMMSG_GMMREG_DETACH_CNF,
	MMMSG_GMMREG_DETACH_IND,
	MMMSG_GMMREG_SERVICE_IND,
	MMMSG_MNMM_PLMN_LIST_CNF,
	MMMSG_MNMM_PLMN_LIST_REJ,
	MMMSG_MNMM_PLMN_SEARCH_IND,

	MMMSG_NET_NAME_IND,					// Network Name Indication
	MMMSG_TZ_IND,						// Timezone Indication
	MMMSG_DATE_IND,						// Date Indication
	MMMSG_LSA_IND,						// Localized service area ID

	MMMSG_GMMREG_ATTACH_REQ, 
	MMMSG_GMMREG_DETACH_REQ,
	MMMSG_MNMM_ALLOC_REQ,
	MMMSG_MNMM_PLMN_LIST_REQ, 
	MMMSG_MNMM_PLMN_LIST_RETRY, 
	MMMSG_MNMM_PLMN_LIST_ABORT,
	MMMSG_MNMM_PREF_PLMN_CHANGE,
	MMMSG_MNMM_AUTO_MODE_REQ,
	MMMSG_MNMM_AUTO_SEARCH_REQ,
	MMMSG_MNGMM_DATA_SUSPEND_IND,
	MMMSG_MNGMM_DATA_RESUME_IND,
	MMMSG_OM_BAND_CNF,
	MMMSG_OM_BAND_REJ,
	MMMSG_MNMM_RADIO_ACTIVITY_IND,
	MMMSG_MNMM_UMTS_DCH_IND,
	MMMSG_GMMREG_FORCED_READY_STATE_REQ,
	MMMSG_MNMM_MS_STATUS_IND,
	MMMSG_MNMM_CELL_INFO_IND,
	MMMSG_MNGMM_PS_CONN_REL_REQ

} MMMsgType_t;							// MMREG Message Types


//******************************************************************************
// Structure Typedefs
//******************************************************************************
typedef SDL_PId				CMEntity_t;


typedef struct 
{
	Boolean					isNetAccessValid;
	UInt8					rat;			//RAT_NOT_AVAILABLE(0),RAT_GSM(1),RAT_UMTS(2)
	UInt8					bandInfo;
	T_NETWORK_ACCESS_INFO	networkAccess;
} MMNetworkInfo_t;							// Network information from stack

typedef struct 
{
	PLMNId_t				plmnId;
	UInt16					cellId;
	UInt16					rncId;
	UInt16					lac;
	UInt8					rac;
} MMPlmnInfo_t;								// PLMN information from stack

typedef struct 
{
	T_PLMN					plmn;
	T_REGISTRATION_STATE	cs_regstate;
	T_REGISTRATION_STATE	gprs_regstate;
	word					lac;
	byte					rac;
	word					cell_id;
	word					rnc_id;
	MMNetworkInfo_t			networkInfo;
} MMMsgParmAttachCnf_t;			// Attach Confirm parameters


typedef struct
{
	T_REGISTRATION_STATE	regstate;
	T_REG_REJECT_CAUSE		cause;
	Boolean					net_cause_avl_ind;	
	UInt8					net_cause;			
} ServiceInfo_t;


typedef struct
{
	ServiceInfo_t			cs_service;
	ServiceInfo_t			gprs_service;		
	T_PLMN					plmn;
	word					lac;
	byte					rac;
	word					cell_id;
	word					rnc_id;
	MMNetworkInfo_t			networkInfo;
} MMMsgParmAttachRej_t;			// Attach Reject parameters	


typedef struct
{
	T_USER_CAUSE			cause;
	T_REGISTRATION_STATE	cs_regstate;
	T_REGISTRATION_STATE	gprs_regstate;
	T_PLMN					plmn;
	word					lac;
	byte					rac;
	word					cell_id;
	word					rnc_id;
	MMNetworkInfo_t			networkInfo;
} MMMsgParmDetachCnf_t;			// Detach Confirm parameters	


typedef struct
{
	T_REGISTRATION_STATE	cs_regstate;
	T_REGISTRATION_STATE	gprs_regstate;
	Boolean					reattach_req_ind;		
	T_PLMN					plmn;
	word					lac;
	byte					rac;
	word					cell_id;
	word					rnc_id;
	MMNetworkInfo_t			networkInfo;
} MMMsgParmDetachInd_t;			// Detach Indication parameters


typedef MMMsgParmAttachRej_t	MMMsgParmServiceInd_t; // Service Indication parameters

typedef struct 
{
	T_PLMN					plmn;
 	UInt16					lac;	/* Location area code of the highest power */
	Boolean					forbidden_ind;
	UInt8					placement;
	T_RAT					rat_ind;
	T_NETWORK_TYPE			network_type;     ///< Indicates whether the PLMN is forbidden, preferred or HPLMN

} SelSinglePLMN_t;


typedef struct
{
	UInt8					tid;	// Transaction identifier
	SDL_PId					pid;	// Id of allocated process
} MMMsgParmAllocCnf_t;			// Connection Mgmt allocated by MM


typedef struct
{
	UInt8					nbr_plmns;
	SelSinglePLMN_t			list_entry[ MAX_SEL_PLMNS ];
	Boolean					more_entry_ind;  /* TRUE if more PLMN entries to follow, FALSE if list complete */
} MMMsgParmPLMNListCnf_t;		// PLMN List Confirm parameters


typedef struct
{
	T_USER_CAUSE			cause;
} MMMsgParmPLMNListRej_t;		// PLMN List Reject parameters


typedef struct
{
	Boolean					sel_started_ind;
} MMMsgParmPLMNSearchInd_t;		// PLMN Search Indication parameters


typedef struct 
{
	Boolean 				isGmmInfo;
	NetworkName_t			LongName;
	NetworkName_t			ShortName;
} MMMsgParmNetworkNameInd_t;


typedef struct 
{
	Boolean 				isGmmInfo;
	UInt8					tz;
	Boolean					dst_exist;		// TRUE if DST (Daylight Saving Time) info exists
	UInt8					dst_adjust;		// Applicable only if "dst_exist" is TRUE, possible value = 0, 1, 2 (see Section 
											// 10.5.3.12 of 3GPP 24.008)
} MMMsgParmTimeZoneInd_t;


typedef struct 
{
	Boolean 				isGmmInfo;
	UInt8					year;			// See GSM 3.40 for encoding rules
	UInt8					month;
	UInt8					day;
	UInt8					hour;
	UInt8					minute;
	UInt8					second;
	UInt8					time_zone;
	Boolean					dst_exist;		// TRUE if DST (Daylight Saving Time) info exists
	UInt8					dst_adjust;		// Applicable only if "dst_exist" is TRUE, possible value = 0, 1, 2 (see Section 
											// 10.5.3.12 of 3GPP 24.008)
} MMMsgParmDateInd_t;


// See Section 10.5.3.11 of 3GPP 24.008.
typedef struct
{
	Boolean isGmmInfo;			// TRUE if LSA is from GMM Info; FALSE if from MM Info
	UInt8	lsaLength;			// If LSA length is 0, it indicates that MS has moved to an area where no LSA is available
	UInt8	lsaArray[3];		// LSA ID, applicable only if lsaLength not equal to 0 (if not equal to 0, length must be 3)
	
} MMMsgParmLSAInd_t;


typedef struct
{
	T_SIM_INDICATOR			sim_ind;
	T_OPERATING_MODE		oper_mode;
	T_PLMN_INDICATOR		plmn_ind;	
	T_PLMN					plmn;
	T_SIM_TYPE				sim_type;
	T_USER_CAUSE			cause;
	Boolean					cs_service_ind;		// Circuit-switched service ind.
	Boolean					gprs_service_ind;	// GPRS service ind.
	T_GSM_SERVICE			pref_service;
	Boolean					ready_timer_ind;	// READY timer presence ind.
	UInt8					ready_timer_val;	// Preferred value of READY timer	
	Boolean					follow_on_req;		// indicate if the attach request will be followed by PDP activation request

	UInt8					manual_rat;		// Applicable only for manual selection: RAT_NOT_AVAILABLE (no RAT specified); RAT_GSM; RAT_UMTS
} MMMsgParmAttachReq_t;


typedef struct
{
	T_USER_CAUSE			cause;
	Boolean					cs_service_ind;
	Boolean					gprs_service_ind;
} MMMsgParmDetachReq_t;


typedef struct
{
	CallIndex_t				call_idx;
	UInt8					protocol_disc;
} MMMsgParmAllocReq_t;			// Allocate TI Request


typedef struct
{
	Boolean					auto_mode;
} MMMsgParmAutoModeReq_t;		// Auto Mode Request

typedef struct
{
	SuspendCause_t		suspend_cause;
} MMMsgParmDataSuspendInd_t;			// Data Suspend Indication

typedef struct
{
	T_STACK_RADIO_ACTION 	radio_direction;
	T_STACK_RADIO_STATUS 	radio_status;
	T_CID 					cid;		
	UInt32					max_dl_data_rate;	//should be the same as ulong in stack
	
} MMMsgParmRadioActivityInd_t;			// Radio Activity Indication parameters


typedef struct
{
	T_UAS_CONN_INFO in_uas_conn_info;   //UAS Channel Info		
} MMMsgParmUe3gStatusInd_t;

typedef struct
{
	Boolean 					forcedReadyState;		
} MMMsgParmForcedReadyStateReq_t;			// Forced ready req parameters

typedef struct
{
	MSStatusInd_t			msStatus;
}
MMMsgParmMsStatusInd_t;

typedef struct
{
	T_PLMN	plmn;
	word	lac;
	word	cellId;
}
MMMsgParmCellInfoInd_t;


typedef union
{
	MMMsgParmAttachCnf_t		attach_cnf;	
	MMMsgParmAttachRej_t		attach_rej;
	MMMsgParmDetachCnf_t		detach_cnf;
	MMMsgParmDetachInd_t		detach_ind;
	MMMsgParmServiceInd_t		service_ind;
	MMMsgParmAllocCnf_t			alloc_cnf;
	MMMsgParmPLMNListCnf_t		plmn_list_cnf;
	MMMsgParmPLMNListRej_t		plmn_list_rej;
	MMMsgParmPLMNSearchInd_t	plmn_search_ind;

	MMMsgParmNetworkNameInd_t 	network_name;	
	MMMsgParmTimeZoneInd_t		timezone;		
	MMMsgParmDateInd_t  		date_ind;
	MMMsgParmLSAInd_t			lsa_ind;

	MMMsgParmAttachReq_t		attach_req;
	MMMsgParmDetachReq_t		detach_req;
	MMMsgParmAllocReq_t			alloc_req;

	MMMsgParmAutoModeReq_t		auto_mode_req;

	MMMsgParmDataSuspendInd_t	data_suspend_ind;

	MMMsgParmRadioActivityInd_t	radio_activity_ind;
	MMMsgParmUe3gStatusInd_t		umts_dch_ind;	
	MMMsgParmForcedReadyStateReq_t	forced_ready_state_req;
	MMMsgParmMsStatusInd_t			ms_status_ind;
	MMMsgParmCellInfoInd_t			cell_info_ind;

} MMMsgParm_t;					// MMREG Msg parameters


typedef struct
{
	MMMsgType_t type; 
	MMMsgParm_t parm;
	ClientInfo_t clientInfo;
} MMREGMsg_t;					// MMREG message


void MN_PostMMREGMsg(					// Post message to MMREG
	MMREGMsg_t *mmreg_msg				// Message to send
	);

#endif

