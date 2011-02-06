/****************************************************************************
*
*     Copyright (c) 2007-2008 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license 
*   agreement governing use of this software, this software is licensed to you 
*   under the terms of the GNU General Public License version 2, available 
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL"). 
*
*   Notwithstanding the above, under no circumstances may you combine this 
*   software in any way with any other Broadcom software provided under a license 
*   other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/
/**
*
*   @file   capi2_mstruct.h
*
*   @brief  This file defines the mstruct data structures
*
****************************************************************************/


#ifndef _CAPI2_MSTRUCT_H_
#define _CAPI2_MSTRUCT_H_

#include "capi2_types.h"
#include "capi2_mstypes.h"
#include "capi2_resultcode.h"
#include "capi2_msnu.h"


#define MAX_NUMBER_OF_UMTS_NCELLS 24
#define MAX_NUMBER_OF_GSM_NCELLS 6
#define MAX_PARAM_PER_MEAS 16

#define STACK_wedge 1


/** 
Per customer requirement, add TMSI, PTMSI, ACK_NACK_NO, RLC state variables, 
uplink and downlink TBF access technology. Tony, 11/05/08
**/

typedef struct
{
	Int8		A[4];
	
}	TMSI_Octets_t;

typedef struct 
{
	UInt16	ack_blocks;
	UInt16	nack_blocks;
	
}	ACK_NACK_CNT_t;


typedef struct 
{
	ACK_NACK_CNT_t dl_blocks;
	ACK_NACK_CNT_t ul_blocks;
	
}	ACK_NACK_Testparam_t;


typedef struct 
{
	UInt16		rlc_v_s;
	UInt16		rlc_v_a;
	UInt16		rlc_v_q;
	UInt16		rlc_v_r;
	
}	RLC_State_Variable_List_t;

typedef struct
{
	ACK_NACK_Testparam_t 		A[4];
	
}	RLC_ACK_NACK_List_t;

typedef struct 
{
	Int8		uplink_acc_tech;
	Int8		downlink_acc_tech;
	
}	Access_Tech_List_t;


typedef struct 
{
	RLC_ACK_NACK_List_t 			ack_nack_param_list;
	RLC_State_Variable_List_t		state_variable_param_list;
	Access_Tech_List_t 					acc_tech_param_list;
	
}	RLC_Testparam_t;

/**
PCH Attach State
**/
typedef enum 
{
	DETACHED,				///<	detached
	ATTACHED,				///<	attached
	ATTACH_IN_PROGRESS,		///<	attach pending
	DETACH_IN_PROGRESS,		///<	detach pending
	DETACH_IN_PROGRESS_ATTACH_PENDING	///<	internal use
}AttachState_t;

/// Call CCM message 
typedef struct
{
	UInt8		callIndex;
	Boolean		callRelease;
	UInt32		callDuration;
	UInt32		callCCM;

} CallCCMMsg_t;

/// Call AOC status message 
typedef struct
{	
	UInt8		callIndex;		///< Call Index
	AoCStatus_t	callAOCStatus;	///< Advice of charge status
 
} CallAOCStatusMsg_t;

/// Call Type
typedef enum{

	MOVOICE_CALL,	///< Mobile Originated Voice
	MTVOICE_CALL,	///< Mobile Terminated Voice
	MODATA_CALL,	///< Mobile Originated Data
	MTDATA_CALL,	///< Mobile Terminated Data
	MOFAX_CALL,		///< Mobile Originated Fax
	MTFAX_CALL,		///< Mobile Terminated Fax
	MOVIDEO_CALL,	///< Mobile Originated Video
	MTVIDEO_CALL,	///< Mobile Terminated Video

	UNKNOWN_TY		///< Unknown Type

} CCallType_t;

/// This structure is all the parameters related to CC API Client command indentifier.
typedef struct
{
	UInt8				callIndex;	///< Index associated with the call
	CCallType_t			callType;	///< The call type
	Result_t			result;		///< Result of the requested action
} ApiClientCmd_CcParam_t;

/// Call State
typedef enum{

	CC_CALL_IDLE		= 0,	///< Idle
	CC_CALL_BEGINNING	= 0,	///< Call Begin
	CC_CALL_CALLING		= 1,	///< Calling state
	CC_CALL_CONNECTED	= 2,	///< Connected state
	CC_CALL_ACTIVE		= 3,	///< Call Active
	CC_CALL_HOLD		= 4,	///< Call on hold
	CC_CALL_WAITING		= 5,	///< Call Waiting
	CC_CALL_ALERTING	= 6,	///< Alerting state
	CC_CALL_BUSY		= 7,	///< Call Busy state
	CC_CALL_DISCONNECT	= 8,	///< Call Disconnect
	UNKNOWN_ST			= 16	///< Unknown

} CCallState_t;


/// Data Call Release Message
typedef struct
{
	UInt8		callIndex;		///< Index associated with the call
	UInt32		callCCMUnit;	///< Call Meter Unit
	UInt32		callDuration;	///< Call Duration
	UInt32		callTxBytes;	///< Total transmitted bytes
	UInt32		callRxBytes;	///< Total bytes received
	Cause_t		exitCause;
} DataCallReleaseMsg_t;


//******************************************************************************
//
// MNATDS Message Types
//
//******************************************************************************

typedef enum
{
								// System messages
 	MNATDSMSG_NULL,
								// MN messages
	MNATDSMSG_MN_SETUP_REQ,
	MNATDSMSG_MN_RELEASE_REQ,
	MNATDSMSG_MN_SETUP_RESP,
								// ATDS messages
	MNATDSMSG_ATDS_SETUP_IND,
	MNATDSMSG_ATDS_CONNECT_IND,
	MNATDSMSG_ATDS_SERVICE_IND,
	MNATDSMSG_ATDS_SETUP_CNF,
	MNATDSMSG_ATDS_SETUP_REJ,
	MNATDSMSG_ATDS_RELEASE_IND,
	MNATDSMSG_ATDS_RELEASE_CNF,
	MNATDSMSG_ATDS_STATUS_IND,
	// fax
	MNATDSMSG_ATDS_FET_IND, 
	MNATDSMSG_ATDS_FIS_IND, 
	MNATDSMSG_ATDS_FCO_IND, 
	MNATDSMSG_ATDS_FCI_IND, 
	MNATDSMSG_ATDS_CONNECT,
	MNATDSMSG_ATDS_OK, 
	MNATDSMSG_ATDS_FCFR_IND, 
	MNATDSMSG_ATDS_FCS_IND, 
	MNATDSMSG_ATDS_FHS_IND, 
	MNATDSMSG_ATDS_FPS_IND, 
	MNATDSMSG_ATDS_FTI_IND, 
	MNATDSMSG_ATDS_FVO_IND, 
								// ATDTN messages
	MNATDSMSG_ATDTN_CONN_READY

} MNATDSMsgType_t;
//******************************************************************************
//
// Message Parameters
//
//******************************************************************************

typedef struct
{
	T_SYSTEM_MODE		system_mode;
	T_DATA_RATE			data_rate;
	T_SYNCHRON_TYPE		synchron_type;
	T_CONN_ELEMENT		conn_element;
	T_SERVICE_MODE		service_mode;
    // 8/18/2004 From Comneon code (atc_setup.c), Instead of a list of possible RLP, it is now simplified:
    // if GEM, then version 2, if compression then version 1, otherwise, version 0.
	T_RLP_PAR			rlp_par;

	T_EST_CAUSE			est_cause;
	T_CALLING_SUBADDR	calling_subaddr;
	T_TEL_NUMBER		tel_number;
	T_CALLED_SUBADDR	called_subaddr;
	T_MN_CLIR			mn_clir;
	T_MN_CUG			mn_cug;
	SDL_Boolean			autocall;

	T_DC_PAR			ds_datacomp;
	T_DC_PAR			ds_hw_datacomp;
	T_DC_NEGOTIATION	ds_datacomp_neg;
    // 8/18/2004 yuanliu - this is for AT+ETBM. If not supported, then all values in the struct should be set to 0.
	T_ETBM				etbm;

} MNATDSParmSetupReq_t;

typedef struct
{
	T_AT_CAUSE			at_cause;
	T_TI_PD				ti_pd;
} MNATDSParmReleaseReq_t;


typedef struct
{

	T_SYSTEM_MODE		system_mode;
	T_RLP_PAR			rlp_par;

      T_TI_PD		  	   	ti_pd;
      T_CONN_SUBADDR  	conn_subaddr;

	T_DC_PAR			ds_datacomp;
	T_DC_PAR			ds_hw_datacomp;
	T_DC_NEGOTIATION	ds_datacomp_neg;

	T_ETBM				etbm;

} MNATDSParmSetupResp_t;

typedef struct
{
	UInt8	p1;		//TBD, global measurement parameters
	UInt8	p2;		//TBD, global measurement parameters
} MS_GenMeasParam_t;

typedef struct
{
	UInt32	meas_bler;		//FFFFFFFF if not present
	Int32	target_sir;		//0xFFFF if not present
	Int32	meas_sir;		//0xFFFF if not present
} MS_UmtsDchReport_t;

typedef struct
{
	UInt8	meas_id;	// meas ID (0x00 - 0x0F)
	UInt8	event_id;	// for all meas types 1A-1F, 2A...2F,3A...3D, 4A, 4B, 5 Q, 6A...6G
						// first decimal digit for meas-type : 1:Intra, 2: Inter, 3: RAT, 4:Tvm, 5: Quality, 6: UE internal
						// second decimal digit for meas-event: 0:A, 1:B, 2:C, 3:D, 4:E, 5:F, 6:G
						//                                      9 means it's periodic instead of event triggered

	// generic data elements. Data is treated differently for each measurement type / mode
	// The description of the different layouts can be found in fspec_dual_mode_engineering_mode.doc
	UInt8	data_elements[6];
} MS_UmtsMeasIdParam_t;

typedef struct {
    Boolean valid; // validity of all the params in this struct. <False>. 
    UInt8 cmip; // Codec Mode Indication Phase. <0xFF>.
    UInt8 cmi;	// Codec Mode Indication in UL. <0xFF>.
    UInt8 cmr;	// Codec Mode Request in DL. <0xFF>.
    int dl_ci;	// DL Carrier-to-Interferer Ratio. <0xFF>.
    UInt8 acs;	// Active Codec Set. <0xFF>.
    UInt8 threshold[3]; // Threshold. <0xFF>.
    UInt8 hysteresis[3]; // Hysteresis. <0xFF>.
    UInt8 speech_rate;   // Full rate or half rate. <0xFF>.
    UInt8 subchan;  // Sub channel 0 or 1. <0xFF>.
} MS_AMRParam_t;


typedef struct
  {
    UInt8 A[8];			// USF value array (0~7) <0xFF>
  }
T_USF_ARRAY;

typedef struct {
    Boolean valid; // validity of all the params in this struct. <False>. 

	/** Indicates the timing advance value used by MS in packet
		transfer mode. Valid value for non-GSM400 is 0 to 63; For GSM400 is 0-219;
		Invalid/default value: 0xFF. See 3GPP TS 04.60, sub-clause 7.1.2.5, 12.12; TS
		05.10, sub-clause 5.5. */
	UInt8 timing_advance; 

	/** Bitmap of downlink timeslots assigned. Bit 0 indicates
		the status of timeslot 7, bit 1 timeslot 6, and so on. Bit value 0 = not assigned, 1
		= assigned. Invalid/default value: 0x00. See 3GPP TS 04.60, sub-clause 12.18. */
    UInt8 dl_timeslot_assigned;

	/** Bitmap of uplink timeslots assigned. Refer to dl_timeslot_assigned above. */
    UInt8 ul_timeslot_assigned;

	/** Array of downlink coding scheme per timeslot. Array
		element 0 indicates the coding scheme of timeslot 0, array element 1 timeslot 1,
		and so on. Valid values of each array element: see below; Invalid/default value:
		0xFF. See 3GPP TS 04.60, sub-clause 12.10b and 3GPP TS 05.03. Refer to the
		following for the integer values that represent each coding scheme:
		CS_1 = 0; CS_2 = 1; CS_3 = 2; CS_4 = 3;
		MCS_1 = 4; MCS_2 = 5; MCS_3 = 6; MCS_4 = 7; MCS_5 = 8; MCS_6 = 9;
		MCS_7 = 10; MCS_8 = 11; MCS_9 = 12. */
    UInt8 dl_cs_mode_per_ts[8];	

	/** Array of uplink coding scheme per timeslot. Refer to dl_cs_mode_per_ts above. */
    UInt8 ul_cs_mode_per_ts[8];

	/** Channel quality parameter C_VALUE. Valid values: 0 to 0xFF;
		Invalid/default value: 0xFF. Note: Integer value, could be negative. See 3GPP
		TS 05.08, sub-clause 10.2.3.1. */
    UInt8 c_value;

    UInt8 alpha; // Power control parameter ALPHA. <0xFF>.
    UInt8 gamma[8]; // Power control parameter GAMMA. gamma[0] is for timeslot 0, and so on. <0xFF>.
    UInt8 timing_adv_idx; // Timing Advance Index. <0xFF>
	UInt8 ra_reselect_hyst;	// RA_RESELECT_HYSTERSIS
	Boolean c31_hyst;	// C31_HYST
	Boolean c32_qual;	// C32_QUAL
	Boolean rand_acc_retry;	// RANDOM_ACCESS_RETRY
	Boolean epcr_sup;	// EGPRS_PACKET_CHANNEL_REQUEST.
    T_USF_ARRAY usf;	// FTD_SB_PH_USF, USF values.
    RLC_Testparam_t		rlc_param;
} MS_GPRSPacketParam_t;

// Invalid/default values are indicated in brackets
typedef struct {
    Boolean valid;  // validity of all the params in this struct. <False>.
    MS_GPRSPacketParam_t gprs_packet_param; // GPRS parameters.
    UInt8 mean_bep_gmsk; // Mean BEP value of GMSK. <0xFF>.
    UInt8 mean_bep_8psk; // Mean BEP value of 8PSK. <0xFF>.
    UInt8 cv_bep_gmsk; // CV BEP value of GMSK. <0xFF>.
    UInt8 cv_bep_8psk; // CV BEP value of 8PSK. <0xFF>.
    UInt8 bep_per;	// BEP_PERIOD or BEP_PERIOD2. <0xFF>.

} MS_EDGEPacketParam_t;

typedef struct {
    Boolean valid; // validity of all the params in this struct. <False>.
	Boolean edge_present; // EDGE present. <False>.
    // BRCM/Davis Zhu 5/27/2004: add EDGEPacketParam_t (Elbert TR#1029)
    MS_EDGEPacketParam_t edge_packet_param;
} MS_EDGEParam_t;

typedef struct
{
	MS_GenMeasParam_t		gen_param;
	MS_UmtsMeasIdParam_t	param_per_meas[MAX_PARAM_PER_MEAS];
} MS_UmtsMeasReport_t;

typedef struct {
	UInt16	arfcn;	// ARFCN <0xFFFF>
	UInt8	rxlev;	// RXLEV <0xFF>
	UInt16	mcc;	// MCC <0xFFFF>
	UInt8	mnc;	// MNC <0xFFFF>
	UInt16	lac;	// LAC <0xFFFF>
	UInt16	ci;		// Cell ID <0x0000>
	UInt8	bsic;	// BSIC <0xFF>
	int     c1;		// C1 <0x0>
	int     c2;		// C2 <0x8000>
	int     c31;	// C31 <0xFFFF>
	int     c32;	// C32 <0x8000>
	
	UInt8	rank_pos;	// Cell reselection ranking of the cell (0 for the best cell).
						// This value is used to order UMTS and GSM cells
						// for the presentation - only in UMTS mode.
	UInt16	ranking_value;		// The calculated ranking value %d - only in UMTS mode */
	UInt8	ranking_status;		//Reason why the cell was not ranked - only in UMTS only
								//0: CELL_SUITABLE
								//1: NOT_MEASURED
								//2: CELL_BARRED
								//3: WRONG_PLMN
								//4: HCS_CRITERIA_PRIO
								//5: HCS_H_VALUE
								//6: S_VALUE
} MS_UmtsGsmNcell_t;


typedef struct {
	MS_UmtsGsmNcell_t	A[MAX_NUMBER_OF_GSM_NCELLS];
} MS_UmtsGsmNcellList_t;

typedef struct
{
	UInt8		cell_type;			//Kind of cell
									//0: ACTIVE_SET
									//1: VIRTUAL_ACTIVE_SET
									//2: MONITORED
									//3: DETECTED
									//4: UMTS_CELL
									//5: UMTS_RANKED
									//6: UMTS_NOT_RANKED
									//7: INVALID_CELL_TYPE
	UInt16		dl_uarfcn;			//DL UARFCN (0xFFFF)      %d
	UInt16		cpich_sc;			//Primary scrambling code %d
	UInt16		cpich_rscp;			//RSCP (dBm - positive value presented
									//positive ) of serving cell (0xFF) %d
	UInt16		cpich_ecn0;			//EC2N0 (dB - positive value presented
									//positive) of serviceng cell (0xFF) %d
	UInt8		pathloss;   //Pathloss0 (dB - Always positive value
									//of serviceng cell (Invalid = 0xFF) %d
	UInt8		rank_pos;			//Cell reselection ranking of the cell (0
									//for the best cell). This value is used to
									//order UMTS and GSM cells for the
									//presentation.
									//- only if cell "UMTS_RANKED"
	UInt16		ranking_value;		//The calculated ranking value        %d
									//- only if cell "UMTS_RANKED"
	UInt8		ranking_status;		//Reason why the cell was not ranked  %d
									//- only if cell "UMTS_NOT_RANKED"
									//0: CELL_SUITABLE
									//1: NOT_MEASURED
									//2: CELL_BARRED
									//3: WRONG_PLMN
									//4: HCS_CRITERIA_PRIO
									//5: HCS_H_VALUE
									//6: S_VALUE
} MS_UmtsNcell_t;


typedef struct {
	MS_UmtsNcell_t	A[MAX_NUMBER_OF_UMTS_NCELLS];
} MS_UmtsNcellList_t;

// BRCM/Davis Zhu 4/27/2004: re-arrange and add c1, c2, c31, c32 (Elbert TR#1029)
// Note: The above added elements are only supported in EDGE protocol stack	at this time
// Invalid/default values are indicated in brackets

typedef struct {
	UInt16	arfcn;	// ARFCN <0xFFFF>
	UInt8	rxlev;	// RXLEV <0xFF>
	UInt16	mcc;	// MCC <0xFFFF>
	UInt8	mnc;	// MNC <0xFFFF>
	UInt16	lac;	// LAC <0xFFFF>
	UInt16	ci;		// Cell ID <0x0000>
	UInt8	bsic;	// BSIC <0xFF>
	int     c1;		// C1 <0x0>
	int     c2;		// C2 <0x8000>
	int     c31;	// C31 <0xFFFF>
	int     c32;	// C32 <0x8000>
	Boolean	barred;  // Barred <FALSE>
	UInt8	cell_priority;  // CELL_PRIORITY <0>
	UInt8	priority_class; // PRIORITY_CLASS <0>
	Boolean same_ra_as_serving_cell;	// SAME_RA_AS_SERVING_CELL <False>
	Boolean	cell_bar_acc_2;	// CELL_BAR_ACC_2 <False>
	Boolean exc_acc;	//EXC_ACC <False>
	

} MS_NcellMeas_t;

typedef struct {
	MS_NcellMeas_t	A[6];
} MS_NcellList_t;

typedef struct {
    UInt16 rf_chan_cnt; // Number of channels in Mobile Allocation. <0xFFFF>.  
	UInt16 rf_chan_array[64]; // Channel numbers in Mobile Allocation. <0xFFFF>.
} MS_MA_t;


typedef struct {
#ifdef  STACK_wedge
	Boolean	valid;	///< TRUE if the values in this structure are valid; FALSE otherwise

//	internal protocol stack states
	UInt8			grr_state;				///< internal state in GRR task
#endif  //STACK_wedge

// GSM info of serving cell (idle mode)
	/** Indicates frequency band. Valid values: 0 = GSM; 1 = DCS; 2 = PCS; 3 =
		850 MHZ; Invalid/default value: 0xFF. See 3GPP TS 05.05, sub-clause 2. */
	UInt16			band;

	/** Absolute radio frequency channel number in idle mode. Valid values: 0 to
		1023; Invalid/default value: 0xFFFF. See 3GPP TS 05.05, sub-clause 2. 
		@note the most significant bit is changed from 1 to 0 for PCS band. So it contains the
		actual arfcn number in any band. */
	UInt16			arfcn;			

	/** contains the value of the RXLEV (receive power level) parameter for the
		serving cell calculated by the mobile station. Valid values: 0 to 63; Invalid/default
		value: 0xFF. See 3GPP TS 05.08, sub-clause 8.1.3, 8.1.4. */
	UInt8           rxlev;				

	/** The base station identity code that allows a MS to distinguish between
		different neighbouring base stations. Valid values: 0 to 63; Invalid/default value:
		0xFF. See 3GPP TS 05.08, sub-clause 9. */
	UInt8			bsic;			

	/** Mobile country code that identifies uniquely the country of domicile of the
		mobile subscriber. Valid values: 0x0000 to 0xFFFF; Invalid/default value:
		0xFFFF. See 3GPP TS 03.03, sub-clause 4.1. Also see 3GPP TS 24.008, subclause
		10.5.1.3 for MCC contained in LAI (Location Area Identification). */
	UInt16			mcc; 			

	/** Mobile network code that identifies the home GSM PLMN of the mobile
		subscriber. Valid values: 0x0000 to 0xFFFF; Invalid/default value: 0xFFFF. See
		3GPP TS 03.03, sub-clause 4.1. Also see 3GPP TS 24.008, sub-clause 10.5.1.3
		for mnc contained in LAI (Location Area Identification). */
	UInt16			mnc; 		

	/** Location area code that identifies a location area within a GSM PLMN. Valid
		values: 0x0000 to 0xFFFF; Invalid/default value: 0xFFFF. Value of 0xFFFE
		represents deleted LAI. See 3GPP TS 03.03, sub-clause 4.1. Also see 3GPP TS
		24.008, sub-clause 10.5.1.3 for LAC contained in LAI (Location Area
		Identification). */
	UInt16			lac; 

	/** Cell identification that identifies the cell within a location area. Valid value:
		0x0000 to 0xFFFF; Invalid/default: 0x0000. See 3GPP TS 03.03, sub-clause
		4.3.1. */
	UInt16			ci;	 	

	/** Indicates the number of 51-multiframes between transmission of
		paging messages to mobiles of the same paging group. Valid values: 2 to 9;
		Invalid/default value: 2. See 3GPP TS 05.02, sub-clause 3.3.2.3. */
	UInt8			bs_pa_mfrms; 

	/** Indicates the number of blocks on each common control
		channel reserved for access grant messages. Valid values: 0 to 7; Invalid/default
		value: 0. See 3GPP TS 05.02, sub-clause 3.3.2.3. */
    UInt8           bs_ag_blks_res; 

	/** Indicates combined CCCH+SDCCH. It is a Boolean type.
		True means combined and False means not combined. Invalid/default value:
		False. */
    Boolean         bcch_combined;	

	/** Maximum TX power level an MS may use when accessing the
		system until otherwise commanded. Valid values: 0 to 31; Invalid/default value:
		0. See 3GPP TS 05.08, sub-clause 6.4. */
	UInt16			ms_txpwr; 		

	/** Minimum received signal level at the MS required for access to
		the system. Valid values: 0 to 63; Invalid/default value: 0xFF. See 3GPP TS
		05.08, sub-clause 6.4. */
	UInt16			rx_acc_min;

	/** Used by the network to control mobile station cell selection
		and reselection. Valid values: 0 to 1; Invalid/default value: 0xFF. See 3GPP TS
		05.08, sub-clause 9. */
	UInt8			cbq; 			

	/** Used by the network to control mobile station cell selection
		and reselection. Valid values: 0 to 1; Invalid/default value: 0xFF. See 3GPP TS
		05.08, sub-clause 9. */
	UInt8			cba; 

	/** Indicates whether c2 parameter is present. It is a Boolean type. True
		means present and False means not present. Invalid/default value: False. */
	UInt8			c2_valid; 		

	/** The radio sub-system link control parameter
		CELL_RESELECT_OFFSET that applies an offset to the C2
		reselection criterion. Valid values: 0 to 63; Invalid/default value: 0xFF. See 3GPP
		TS 05.08, sub-clause 9. */
	UInt8			cr_offset; 

	/** The radio sub-system link control parameter
		TEMPORARY_OFFSET that applies a negative offset to C2 for the duration of
		PENALTY_TIME is applied. Valid values: 0 to 7; Invalid/default value: 0. See
		3GPP TS 05.08, sub-clause 9. */
	UInt16			tmp_offset; 	

	/** The radio sub-system link control parameter PENALTY_TIME that
		gives the duration for which the temporary offset is applied. Valid values: 0 to 31;
		Invalid/default value: 0. See 3GPP TS 05.08, sub-clause 9. */
	UInt16			penalty_t; 

	/** The "path loss criterion" parameter used for cell selection and reselection.
		The path loss criterion (3GPP TS 03.22) is satisfied if c1 > 0. Valid values:
		0x0000 to 0x003F; Invalid/default value: 0x0000. Note: integer value, could be
		negative. See 3GPP TS 05.08, sub-clause 6.4 and 03.22, sub-clause 3.6. */
	int 			c1; 			

	/** The cell reselection parameter used for cell reselection only. Valid values:
		0x0000 to 0xFFFF; Invalid/default value: 0x8000. 
		@note integer value, could be negative. See 3GPP TS 05.08, sub-clasuse 6.4
		and 03.22, sub-clause 3.6. */
	int  			c2; 			

	/** Indicates the timeout value for periodic updating in deci-hours. Value
		0x0000 means no periodic update needed. Valid values: 0x0000 to 0xFFFF;
		Invalid/default value: 0x0000. See 3GPP TS 04.18. sub-clause 10.5.2.11. */
	UInt16			t3212; 		

	/** Bitmap of Access Control Class. It controls the MS access to network.
		Access Control Class N (bit 1-16). For a mobile station with Access Control
		Class =N access is not barred if the Access Control Class N bit is coded with a
		'0'; N = 0, 1,....9,11,...,15. Bit 11= the EC bit is the Emergency Call Allowed
		coded as specified in 3GPP TS 04.08.\n
		@verbatim
		Bits:    16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1
		Class N: 15 14 13 12 11 EC  9 8 7 6 5 4 3 2 1 0
		@endverbatim
		Valid values: 0 to 0xFFFF; Invalid/default value: 0x0000. See 3GPP TS 04.60,
		sub-clause 12.14. */
	UInt16			acc; 	

	/// \<NORMALPAGING\>
	UInt8			cs_page_mode;			

	/// \<False\>
	Boolean			pbcch_hopping_status;	

	/// \<False\>
	Boolean			pccch_hopping_status;	



// GPRS/EGPRS info of serving cell (idle mode)

	/** GPRS supported. It is a Boolean type. True means supported
		and False means not supported. Invalid/default value: False. */
	Boolean         gprs_present;

	/** The Routing Area Code. Valid values: 0 to 255; Invalid/default value: 0xFF.
		See 3GPP TS 04.60, sub-clause 11.2.25 and 03.03, 4.2. */
    UInt8           rac; 		

	/** Indicates Network Mode of Operation. 0 = No GPRS Cell; 1 = Network
		Mode of Operation I; 2 = Network Mode of Operation II; 3 = Network Mode of
		Operation III. Invalid/default value: 0. See 3GPP TS 03.60, sub-clause 6.3.3.1
		and 04.60, 12.24. */
	UInt16          nom; 		

	/** Indicates whether split page cycle is supported by network. Boolean
		type. True means supported and False means not supported. Invalid/default
		value: False. See 3GPP TS 04.60, sub-clause 5.5.1.5; 05.02, 6.5.6; 24.008,
		10.5.5.6. */
	Boolean         sp_pg_cy;	

	/** Indicates the timeout value of timer T3192. Upon expiration of the timer,
		MS shall release the resources, stop monitoring the PDCHs, and begin to
		monitor the paging channel. Valid values: 0 to 7; Invalid/default value: 0xFFFF.
		See 3GPP TS 04.60, sub-clause 12.24, 13.1. */
	UInt16          t3192; 				

	/** Indicates if the 8 or 11 bit format shall be used in the
		PACKET CHANNEL REQUEST message, the PTCCH uplink block and in the
		PACKET CONTROL ACKNOWLEDGMENT message when the format is four
		access bursts. 0 = 8-bit; 1 = 11-bit format. Invalid/default value: 0. See 3GPP TS
		04.60, sub-clause 12.24. */
	UInt32          access_burst_type; 	

	/** Indicates value of the parameter DRX_TIMER_MAX in GPRS cell
		options. Valid values: 0 to 7; Invalid/default value: 0xFF. See 3GPP TS 04.60,
		sub-clause 12.24. */
	UInt8           drx_max;

	/** The NETWORK_CONTROL_ORDER parameter that controls cell reselection
		and measurement reporting. Valid values: 0 to 3; Invalid/default value:
		0. See TS 04.60, sub-clause 11.2.25 and 05.08, 10.1.4, 10.4. */
	UInt32          nco;				

	/** The EXT_MEASUREMENT_ORDER parameter that controls a mobile
		station to send extended measurement reports. Valid values: 0 to 3;
		Invalid/default value: 0. See 3GPP TS 04.60, sub-clause 11.2.9b and 05.08,
		10.1.5, 10.4. */
	UInt32          emo; 			

	/** PBCCH present (indicated in the PSI13 message on the
		PACCH or on the PCCCH). It is a Boolean type. True means present and False
		means not present. Invalid/default value: False. See 3GPP TS 04.60, sub-clause
		11.2.25. */
	Boolean         pbcch_present; 		

	/** The occurrence of the PSI1 message (indicated in the PSI1 message
		on PBCCH). Valid values: 1 to 16; Invalid/default value: 0xFF. See
		3GPP TS 05.02, sub-clause 6.3.2.4 and 04.60, 11.2.18. */
	UInt8           psi1_rep; 	

	/** The number of PSI message instances sent with low repetition
		rate (indicated in the PSI1 message on PBCCH). Valid values: 0 to 16;
		Invalid/default value: 0xFF. See 3GPP TS 05.02, sub-clause 6.3.2.4 and 04.60,
		11.2.18. */
	UInt8           psi_count_lr; 

	/** The number of PSI message instances sent with high repetition
		rate (indicated in the PSI1 message on PBCCH). Valid values: 0 to 63;
		Invalid/default value: 0xFF. See 3GPP TS 05.02, sub-clause 6.3.2.4 and 04.60,
		11.2.18. */
	UInt8           psi_count_hr;

	/** The signal level threshold criterion parameter used to determine whether
		prioritized hierarchical GPRS and LSA cell re-selection shall apply. Valid values:
		0x0000 to 0xFFFF; Invalid/default value: 0xFFFF. Note: integer value, could be
		negative. See 3GPP TS 05.08, sub-clause 10.1.2. */
	int             c31; 			

	/** The cell ranking criterion parameter used to select cells among those with
		the same priority. Valid values: 0x0000 to 0xFFFF; Invalid/default value: 0x8000.
		@note integer value, could be negative. See 3GPP TS 05.08, sub-clause 10.1.2. */
	int             c32; 				

	/** The packet access parameter indicating whether packet
		access is allowed in the cell for the radio priority level associated with the packet
		transfer. Valid values: 0 to 7; Invalid/default value: 0xFF. See 3GPP TS 04.18,
		sub-clause 3.5.2, 10.5.2.37b and TS 04.60, 11.2.25. */
	UInt8           priority_access_thr; 

// Hezhi 01/16/08
	UInt8			t_avg_w;				///< t_avg_w <0>
	UInt8			t_avg_t;				///< t_avg_t <0>
	UInt8			n_avg_i;				///< n_avg_i <0>
	UInt8			pb;						///< pb <0>
	UInt8			ps_page_mode;			///< Hezhi
	Boolean			si13_pres;				///< \<FALSE\>
	UInt8			pc_meas_chan;			///< <0>
	UInt8			t_resel;				///< T_RESEL <5> 

// Neighbor cell info
	UInt16			no_ncells;				///< Number of neighbor cells <0>
	MS_NcellList_t	ncell_param;			///< Neighbor cell parameters


// 3G Neighbor cell inof
	UInt16			no_3g_ncells;			/// < Number of 3G neighbor cells <0>
	MS_UmtsNcellList_t	umts_ncell_param;			/// < 3G Neighbor cell parameters

// Dedicated mode info

	/** Indicates the channel type. Valid values: see below; Invalid/default
		value: 0. See 3GPP TS 04.18, sub-clause 10.5.2.5.
		0 = INVALID_CHN_TYPE; 1 = TCH_F; 2 = TCH_H; 3 = SDCCH_4;
		4 = SDCCH_8; 5 = TCH_H_H; 6 = TCH_F_M. */
	UInt8			chan_type;		

	/** Gives information of the mode on coding/decoding and
		transcoding. Valid values: see below; Invalid/default value: 0xFF. See 3GPP TS
		04.18, sub-clause 10.5.2.6.\n
		0 = MODE_SIG_ONLY;\n
		1 = MODE_SPEECH_F;\n
		2 = MODE_SPEECH_H;\n
		3 = MODE_DATA_96_F;\n
		4 = MODE_DATA_48_F;\n
		5 = MODE_DATA_48_H;\n
		6 = MODE_DATA_24_F;\n
		7 = MODE_DATA_24_H;\n
		8 = MODE_SPEECH_F_V2;\n
		9 = MODE_SPEECH_F_V3;\n
		10 = MODE_SPEECH_H_V2;\n
		11 = MODE_SPEECH_H_V3;\n
		12 = MODE_DATA_144_F. */
	UInt8			chan_mode;	

	/** Absolute radio frequency channel number of a
		non-hopping channel in dedicated mode. Its value is valid only if a non-hopping
		channel is assigned. Valid values: 0 to 1023; Invalid/default value: 0xFFFF. See
		3GPP TS 05.05, sub-clause 2. 
		@note the most significant bit is changed from 1 to
		0 for PCS band. So it contains the actual arfcn number in any band. */
	UInt16			arfcn_ded;		


	Boolean			hopping_status;			///< hopping_status \<FALSE\>

	/** Mobile allocation index offset. Its value is valid only if a hopping channel is
		assigned. Valid values: 0 to 63; Invalid/default value: 0xFF. See 3GPP TS 05.02,
		sub-clause 6.2.2. */
	UInt8           maio;			

	/** The hopping sequence number. Its value is valid only if a hopping channel
		is assigned. Valid values: 0 to 63; Invalid/default value: 0xFF. See 3GPP TS
		05.02, sub-clause 6.2.2. */
	UInt8           hsn;		

	/** Mobile allocation of radio frequency channels. It defines the set of radio
		frequency channels to be used in the mobiles hopping sequence. It has the
		following elements:
		- rf_chan_cnt: number of channels. Valid values: 0 to 64, 0 indicates that ma
		is not present. Invalid/default value: 0.
		- rf_chan_no: array of channel numbers in the MA. These are the arfcn
		numbers representing the radio channels. Refer to arfcn in previous section.
		See 3GPP TS 05.02, sub-clause 6.2.2. */
	MS_MA_t     	ma;		

	/** Bitmap of the timeslots assigned. Bit 7 indicates the status
		of timeslot 0, bit 6 indicates the status of timeslot 1, etc.
		- 0: Timeslot is not assigned
		- 1: Timeslot is assigned
		- Invalid/default value: 0x00.
		See 3GPP TS 04.60, sub-clause 12.18. */
	UInt8           timeslot_assigned;

	/** TX power level of the MS in dedicated mode. Valid values: 0 to 31;
		Invalid/default value: 15. See 3GPP TS 05.08, sub-clause 6.4 and 9. */
	UInt8			txpwr;	

	/** indicates whether or not the mobile station used DTX during the
		previous measurement period.
		- 0 DTX was not used
		- 1 DTX was used.
		- Invalid/default value: 0xFF.
		See 3GPP 04.18, sub-clause 10.5.2.20. */
	UInt8			dtx_used;	


	UInt8			dtx_from_bs;			///< FTD_SB_DTX_FROM_BS <0X0>

	UInt32			l1_status;			//L1 status

	/** The handover and power control parameter
		RXLEV_FULL_SERVING_CELL that indicates the RXLEV in the current serving
		cell accessed over all TDMA frames. Valid values: 0 to 63; Invalid/default value:
		0xFF. See 3GPP TS 05.08, sub-clause 9. */
	UInt8			rxlevfull;	

	/** The handover and power control parameter
		RXLEV_SUB_SERVING_CELL that indicates RXLEV in the current serving cell
		accessed over a subset of TDMA frames. Valid values: 0 to 63; Invalid/default
		value: 0xFF. See 3GPP TS 05.08, sub-clause 9. */
	UInt8			rxlevsub;

	/** The handover and power control parameter
		RXQUAL_FULL_SERVING_CELL that indicates the RXQUAL accessed over all
		TDMA frames. . Valid values: 0 to 7; Invalid/default value: 0xFF. See 3GPP TS
		05.08, sub-clause 9. */
	UInt8			rxqualfull;	

	/** The handover and power control parameter
		RXQUAL_SUB_SERVING_CELL that indicates the RXQUAL accessed over a
		subset of TDMA frames. Valid values: 0 to 7; Invalid/default value: 0xFF. See
		3GPP TS 05.08, sub-clause 9. */
	UInt8			rxqualsub;

	/** training sequence code. Valid values: 0 to 255; Invalid/default value: 0xFF.
		See 3GPP TS 04.60, sub-clause 11.2.19. */
	UInt8           tsc;

	/** Indicates the timing advance value used by MS in dedicated
		mode. Valid value for non-GSM400 is 0 to 63; For GSM400 is 0-219;
		Invalid/default value: 0xFF. See 3GPP TS 04.60, sub-clause 7.1.2.5, 12.12; TS
		05.10, sub-clause 5.5. */
	UInt8           timing_advance;	

	/** The radio sub-system link control parameter
		RADIO_LINK_TIMEOUT that contains the maximum value of the radio link
		counter 4-64 SACCH blocks, 15 steps of 4 SACCH blocks. Valid values: 0 to 15;
		Invalid/default value: 0xFF. See 3GPP TS 05.08, sub-clause 9. */
	UInt8           radio_link_timeout;	

	/** Indicates ciphering is on or off. It is a Boolean type. True means on
		and False means off. Invalid/default value: False. */
	Boolean         cipher_on;	
	UInt8			cipher_algorithm;		///< cipher_algorithm: <0>
#ifdef  STACK_wedge
	UInt8			chn_rel_cause;			///< Channel release cause (T_MMRR_CAUSE)
#endif  //STACK_wedge

	UInt8			rr_cause;				///< RR_CAUSE Hezhi
	
	// Enhanced  Enginerring mode, /BRCM /Wei Chen 01/22/08
	UInt8			max_retrans_cs;		///< Maximum number of Random Access retransmission <0x00>
	Boolean			att;				///< Attach-detach allowed \<False\>
	Boolean			neci;				///< NECI: half rate support \<False\>
	Boolean			acs;				///< Additional reselection parameter indication \<False\>
	Boolean			cbch_sup;			///< cell broadcast channel support \<False\>
	Boolean			reest_allowed;		///< Call Reestablishment Support \<False\>
	Boolean			ecsc;				///< Early Classmark Sending Control \<False\>
	Boolean			si2ter_sup;			///< Are 2-Ter messages supported? \<False\>
	UInt8			multib_rep;		///< Multiband reporting value <0x00>
	Boolean			cell_bar_acc_2;		///< CELL_BAR_ACCESS_2 \<False\>
	Boolean			exc_acc;			///< Exclusive access right for serving cell \<False\>
	UInt8			ra_resel_hyst;		///< RA_RESELECT_HYSTERSIS <0x00>
	Boolean			pfc_feat_sup;		///< PFC_FEATURE_MODE \<False\>
	Boolean			ext_upl_tbf_sup;	///< NW_EXT_UTBF \<False\>
	Boolean			pkt_si_status;		///< Packet SI_STATUS  \<False\>
	Boolean			ccn_active;			///< Network support Cell Change Notification procedure \<False\>
	

// BRCM/Davis Zhu 5/27/2004: add amr_params, gprs_packet_params, edge_params (Elbert TR#1029)
// AMR, GPRS/EDGE packet mode info
	MS_AMRParam_t		amr_param;				///< AMR test parameters
	MS_GPRSPacketParam_t 	gprs_packet_param;	///< GPRS packet mode test parameters
	MS_EDGEParam_t 		edge_param;				///< EDGE test parameters
#ifdef  STACK_wedge

} MS_GSMParam_t;


/// MM/NAS parameters
typedef struct {
	Boolean	valid;				///< TRUE if the values in this structure are valid; FALSE otherwise

	// internal protocol stack states
	UInt8	mmc_state;			///< internal state in MMC task
	UInt8	mme_state;			///< internal state in MME task
	UInt8	gmm_state;			///< internal state in GMM task

	// common info
	UInt8	rat;				///< radio access technology of serving cell (0: GSM, 1: UMTS)
	UInt8	nom;				///< network operation mode of serving cell
	UInt8	search_mode;		///< 0: automatic, 1: manual

	// CS related info
	UInt8	mm_update_status;	///< CS update status

	/** Bitmap of CS timer status
		bit 0 (LSB): T3210, bit 1: T3211, bit 2: T3212, bit 3: T3213
		bit 4: T3220, bit 5: T3230, bit 6: T3240 */
	UInt32	mm_timer_status;	

	UInt8	mm_reject_cause;	///< reject cause received in LU Reject
	UInt8	mm_lu_rej_cause;	///< LU Reject in normal LU 
	UInt8   mm_lu_imsi_rej_cause; 
	UInt8	mm_retrans_cnt;		///< Number of LU request retransmissions

	UInt8   mm_t3212_cnt;		///< Current value of LU update timer 
	PLMNId_t		last_reg;	///< Last registered PLMN	
	

	// PS related info
	UInt8	gmm_update_status;	///< PS update status

	/** Bitmap of PS timer status
		bit 0 (LSB): T3302, bit 1: T3310, bit 2: T3311, bit 3: T3312
		bit 4: T3314, bit 5: T3321, bit 6: T3330 */
	UInt32	gmm_timer_status;	

	UInt8	gmm_reject_cause;	///< GMM cause received in Attach Reject, RAU Reject, or Detach Request from network
	UInt8	gmm_retrans_cnt;	///< Number of attach request, RAU request, or detach request retransmissions
	UInt8	gmm_retry_cnt;		///< Number of attach or RAU procedure retries (due to T3302)

	/** type of the last Attach Request
		255 means no type, other values follow 3GPP 24.008 sec 10.5.5.2 */
	UInt8	gmm_attach_type;	

	/** type of the last RAU Request
		255 means no type, other values follow 3GPP 24.008 sec 10.5.5.18 */
	UInt8	gmm_rau_type;		

	/** type of the last Detach Request
		255 means no type, other values follow 3GPP 24.008 sec 10.5.5.5 */
	UInt8	gmm_detach_type;	

	/** direction of the last Detach Request
		255 means no direction, 0 for MS initiated, 1 for network initiated */
	UInt8	gmm_detach_dir;		
     /** MS Class parameter */
	UInt8  mm_class;                ///< 0 if GPRS_CLASS_A, 1 if GPRS_CLASS_B, 2 if GPRS_CLASS_C, 3 if GPRS_CLASS_CC,
	                                ///< 4 if GPRS_CLASS_CG 4, 5 if GPRS_NO_CLASS 5

	/** GPRS attach mode */
	AttachState_t	gprs_attach_mode;  
    /** GSM service state */
	Service_State_t   cs_state;
	/** GPRS service state */
	Service_State_t   ps_state;
    /** TMSI */
	TMSI_Octets_t	tmsi;
	/**PTMSI */
	TMSI_Octets_t	ptmsi;
    /** HPLMN timer */
	UInt32  hplmn_timer; ///< HPLMN timer
} MS_MMParam_t;

typedef struct
{
	Boolean					valid;	///< TRUE if the values in this structure
									///< are valid; FALSE otherwise

	// internal protocol stack states
	UInt8					rrc_state;		///< RRC state
	UInt16					rrcdc_state;	///< State and events in URRCDC
											///< this is	T_RRC_PROCESS_STATE.event << 8 |
											///<			T_RRC_PROCESS_STATE.process_state
	UInt16					rrcbp_state;	///< State and events in URRCBP
	UInt16					rrcm_state;		///< State and events in URRCM
 
	// serving cell info
	UInt16					p_sc;			///< primary scrambling code
	UInt16					lac;			///< location area code
	UInt8					rac;			///< routing area code
	UInt32					cell_id;		///< cell identity
	UInt16					ul_uarfcn;		///< uplink UARFCN of serving cell
	UInt16					dl_uarfcn;		///< downlink UARFCN of serving cell
	UInt16					cpich_rscp;		///< CPICH RSCP (dBm) of serving cell
	UInt8					cpich_ecn0;		///< CPICH Ec/No (dB) of serving cell
	UInt8					   pathloss;		// Pathloss (=PCPICH-Tx-Pwr - RSCP) of serving cell
	Boolean					cipher_on;		///< ciphering indicator
	Boolean					hcs_used;		///< HCS indicator
	Boolean					high_mobility;	///< high mobility indicator
	UInt16					ranking_value;	///< calculated ranking value

	UInt8					chn_rel_cause; 	///< cause of last channel release
	Int8					tx_pwr;			///< transmit output power

	Int8					rssi;			///< received signal strength
 
	// UMTS neighbor cell info
	UInt8					no_active_set_cells;			///< Number of ACTIVE SET cells (CELL_DCH)
	UInt8					no_virtual_active_set_cells;	///< Number of VIRTUAL ACTIVE SET cells (CELL_DCH)
	UInt8					no_umts_ncells;					///< Number of DETECTED UMTS cells (CELL_DCH) or
															///< Number of UMTS neighbour cells (all states but CELL_DCH)
	MS_UmtsNcellList_t		umts_ncell;						///< list of all UMTS cells
 
	// GSM neighbor cell info
	UInt8					no_gsm_ncells;					///< Number of GSM neighbor cells
	MS_UmtsGsmNcellList_t	gsm_ncell;						///< list of GSM neighbor cells

	// UMTS measurement report
	MS_UmtsMeasReport_t		meas_report; ///<mirrors T_UMTS_MEAS_EVAL_PARAM in stack
 
	// DCH quality report
	MS_UmtsDchReport_t		dch_report; ///<mirrors T_EM_DCH_REPORT struct in stack
 
    Boolean                 hsdch_reception_on;    ///< HS_DSCH reception ongoing indicator

    Boolean                 is_hsdpa_cell_capable;  ///<TRUE means cell is HSDPA capable
} MS_UMTSParam_t;    ///< this structure corresponds to T_RRC_TESTPARAM struct in stack
typedef struct
{
	UInt32		sms_mo_cs_att_cnt;
	UInt32		sms_mo_gprs_att_cnt;
	UInt32		sms_mt_cs_att_cnt;
	UInt32		sms_mt_gprs_att_cnt;

	UInt32		sms_mo_cs_ok_cnt;
	UInt32		sms_mo_gprs_ok_cnt;
	UInt32		sms_mt_cs_ok_cnt;
	UInt32		sms_mt_gprs_ok_cnt;

	UInt32		sms_TC1M_cnt;
	UInt32		sms_cb_sch_msg_cnt;
	UInt32		sms_TR1M_cnt;
	UInt32		sms_TR2M_cnt;
	UInt32		sms_TRAM_cnt;

	UInt8		sms_priority;

} MS_Ext_SMSParam_t;

typedef struct
{
//Location Update Parameters
	UInt32		mm_lu_ok_cnt;
	UInt32		mm_lu_imsi_ok_cnt;
	UInt32		mm_lu_att_cnt;
	UInt32		mm_lu_imsi_att_cnt;
	
//GPRS attach, detach parameters	
	UInt8		attach_fail_cause;
	UInt32		attach_attempt_cnt;
	UInt32		attach_ok_cnt;
	UInt8		ntw_init_det_cause;
	UInt32		ntw_init_detach_cnt;
	
//Routing Area Update parameters	
	UInt8		rau_fail_cause;	// Cause of last routing area update failure
	UInt32		rau_attempt_cnt;  // Counter for the RAU attempts 
	UInt32		rau_ok_cnt;  // Counter for the succeeded RAUs 	

//Periodic Routing Area Update parameters
	UInt8		prau_fail_cause;
	UInt32		prau_attempt_cnt;
	UInt32		prau_ok_cnt;

// gmm timer
	UInt32		t3312_current_value;
	UInt32		t3312_to_value;
	UInt32		t3314_to_value;

// gmm ready state: READY, FORCE, STAND, IDLE
	UInt8		ready_state;
	Boolean		forced_ready_state;

// gmm ciphering algorithm
	UInt8		gea_supported;  // PS ciphering algorithm

// gmm drx parameters
	UInt8		split_pg_cycle_code;
	UInt8		non_drx_tmr_val;

// equivelant PLMN
	EQUIV_PLMN_LIST_t	eqv_plmn_list;

} MS_Ext_MMParam_t;


typedef struct
{
	UInt32		gsm_gsm_resel_cnt;	///< Counter for GSM900->GSM900 cell reselection(max 99999) <0>
	UInt32		dcs_gsm_resel_cnt;	///< Counter for GSM1800->GSM900 cell reselection <0>
	UInt32		gsm_dcs_resel_cnt;	///< Counter for GSM900->GSM1800 cell reselection <0>
	UInt32		dcs_dcs_resel_cnt;	///< Counter for GSM1800->GSM1800 cell reselection <0>
	
	// grr engineering mode information
	UInt8		si13_location;
	UInt8		psi5_present;
	UInt8		psi_status_ind;

	UInt8		bs_cv_max;
	UInt8		pan_dec;
	UInt8		pan_inc;
	UInt8		pan_max;

	// HO (handover + channel assign) counters

	/// attemp counters
	UInt32		gsm_ho_att_cnt[GSM_HO_MAX_TYPES][GSM_HO_MAX_BAND_SWITCH];
	
	/// failed counters
	UInt32		gsm_ho_fail_cnt[GSM_HO_MAX_TYPES][GSM_HO_MAX_BAND_SWITCH];

	/// successful counters
	UInt32		gsm_ho_ok_cnt[GSM_HO_MAX_TYPES][GSM_HO_MAX_BAND_SWITCH];

	/// successfully back to previous counters
	UInt32		gsm_ho_back_cnt[GSM_HO_MAX_TYPES][GSM_HO_MAX_BAND_SWITCH];

	
} MS_Ext_RRParam_t;



typedef struct
{
	UInt8		tfi;				// FTD_SB_MAC_DL_TFI <default:0xff>
	UInt8		mac_mode;			// FTD_SB_MAC_ALLOC_MODE <default: MAC_MODE_UNDEFINED(6)> 
	UInt8		rlc_mode;
	UInt8		tbf_type;
	UInt32		n3102;


} MS_Ext_RLCParam_t;


typedef struct
{
	UInt32		ul_tbf_idle_attempt_cnt;
	UInt32		ul_tbf_idle_ok_cnt;
	UInt32		ul_tbf_trans_attempt_cnt;
	UInt32		ul_tbf_trans_ok_cnt;
	UInt32		dl_tbf_est_cnt;
	UInt32		ul_tbf_abnormal_rel_cnt;
	UInt8		lqmm;				// mac_tm_get_egprs_params

} MS_Ext_MACParam_t;

typedef struct
{
	UInt32		mo_pdp_attempt_cnt;	///< counter for the MS initiated PDP context avtivation attempt

	UInt8		pdp_priority;		///< radio priority of PDP context
	UInt8		sec_pdp_priority;	///< radio priority of second PDP context

} MS_Ext_SMParam_t;


typedef struct
{
	UInt8		frame_state;
	
} MS_Ext_L1Param_t;


/// This extended engineering mode parameters are set by direct function call
typedef struct
{
	MS_Ext_SMSParam_t	sms_ext_param;    ///< sms engineering mode parameters
	MS_Ext_MMParam_t	mm_ext_param;	  ///< mm engineering  mode parameters
	MS_Ext_RRParam_t	rr_ext_param;	  ///< rr engineering mode parameters
	MS_Ext_SMParam_t	sm_ext_param;	  ///< sm context parameters
	MS_Ext_RLCParam_t	rlc_ext_param;	  ///< rlc engineering mode parameters
	MS_Ext_MACParam_t	mac_ext_param; 	  ///< mac engineering mode parameters
	MS_Ext_L1Param_t	l1_ext_param;	  ///< L1 engineering mode parameters
} MS_ExtParam_t;



/// This is the structure sent in response to the DIAG_ApiMeasurmentReportReq() command
typedef struct {
	MS_MMParam_t	mm_param;		
	MS_GSMParam_t	gsm_param;		
	MS_UMTSParam_t	umts_param;		
	MS_ExtParam_t   ext_param;		
#endif  //STACK_wedge
} MS_RxTestParam_t;


//////////////////

typedef struct
{
	T_DATA_RATE			data_rate;
	T_SERVICE_MODE		service_mode;
	T_CONN_ELEMENT		conn_element;
	T_TI_PD				ti_pd;
	T_TEL_NUMBER		tel_number;
	T_CALLING_SUBADDR	calling_subaddr;
	T_CALLED_SUBADDR	called_subaddr;
	Boolean				ds_datacomp_ind;
} MNATDSParmSetupInd_t;


typedef struct
{
	T_TI_PD				ti_pd;
	T_DC_PAR			ds_neg_datacomp;
} MNATDSParmConnectInd_t;


typedef struct
{
	T_TI_PD				ti_pd;
	T_SERVICE_MODE		service_mode;
} MNATDSParmServiceInd_t;


typedef struct
{ 
	T_TI_PD				ti_pd;
	T_TEL_NUMBER		tel_number;
	T_CONN_SUBADDR		conn_subaddr;
	T_DC_PAR			ds_neg_datacomp;
} MNATDSParmSetupCnf_t;

typedef struct
{
    T_MN_CAUSE  		mn_cause;
} MNATDSParmSetupRej_t;


typedef struct
{
    T_TI_PD  			ti_pd;
    T_MN_CAUSE  		mn_cause;
} MNATDSParmReleaseInd_t;


typedef struct
{
	T_TI_PD				ti_pd;
} MNATDSParmReleaseCnf_t;

typedef struct
{
	T_LINESTATE			linestate1;
	T_LINESTATE			linestate2;
	T_LINESTATE			linestate3;
} MNATDSParmStatusInd_t;

typedef struct 
{
	UInt8	ppm; 
} MNATDSParmFET_t; 

typedef union
{
	MNATDSParmSetupReq_t		setup_req;
	MNATDSParmReleaseReq_t		release_req;
	MNATDSParmSetupResp_t		setup_resp;

	MNATDSParmSetupInd_t		setup_ind;
	MNATDSParmConnectInd_t		connect_ind;
	MNATDSParmServiceInd_t		service_ind;
	MNATDSParmSetupCnf_t		setup_cnf;
	MNATDSParmSetupRej_t		setup_rej;
	MNATDSParmReleaseInd_t		release_ind;
	MNATDSParmReleaseCnf_t		release_cnf;

	MNATDSParmStatusInd_t		status_ind;
	MNATDSParmFET_t				fet_ind;

} MNATDSMsgParm_t;						// MNATDS message parameters


//***********************
// MNATDS Message 
//***********************
typedef struct
{
	MNATDSMsgType_t 	type;			// MNATDS Message Type
	MNATDSMsgParm_t     parm;			// MNATDS Message Parameters
} MNATDSMsg_t;							// MNATDS Message

/// Data Call Status Message
typedef struct
{
	UInt8		  callIndex;
	MNATDSMsg_t   mnatds_msg;
	//...

} DataCallStatusMsg_t;


/// Error Correction Data Compression Link Message
typedef struct
{
	UInt8		  callIndex;	///< Index associated with the call
	UInt8		  ecdcResult;	///< Result of make link request
	UInt8		  ecMode;		///< Error Correction Mode
	UInt8		  dcMode;		///< Data Compression Mode

} DataECDCLinkMsg_t;


/**
Progress indicator type
**/
typedef enum {
	MNCC_PROG_IND_IE_NOT_PRESENT= 0x00,		///< Progress indicator not present
	MNCC_NO_END_TO_END_PLMN		= 0x01,		///< No end to end PLMN
	MNCC_DEST_NON_PLMN			= 0x02,		///< Destination not PLMN
	MNCC_ORGIN_NON_PLMN			= 0x03,		///< Origination not PLMN
	MNCC_RETURN_TO_PLMN			= 0x04,		///< Return to PLMN
	MNCC_INBAND_AVAIL			= 0x08,		///< Inband signal available
	MNCC_END_TO_END_PLMN		= 0x20,		///< End to end PLMN
	MNCC_QUEUING				= 0x40		///< Queueing
} ProgressInd_t;


/// Call status message structure
typedef struct
{
	UInt8		 callIndex;			///< Call Index
	CCallType_t	 callType;			///< Call type(speech/data/fax...)
	CCallState_t callstatus;		///< Call status
	ProgressInd_t	progress_desc;	///< Progress Indicator	
	UInt8			codecId;			///< provides a codecId based on 3GPP26.103 Sec 6.3.as of now the only valid
										///< values are 0x0a and 0x06.  
    UInt8           causeNoCli;     ///< Cause for no CLI
} CallStatusMsg_t;


/// From msc/inc/Mncctypes.h

typedef enum {
	CC_PRESENTATION_ALLOWED		= 0,
	CC_PRESENTATION_RESTRICTED	= 1,
	CC_NUMBER_NA_INTERWORKING	= 2,
	CC_NO_CONNECTED_NUMBER      = 3 // The numbere IEI doesn't exist
} PresentationInd_t;

typedef enum {
	CC_NOT_SCREENED				= 0,
	CC_VERIFIED_AND_PASSED		= 1,
	CC_VERIFIED_AND_FAILED		= 2,
	CC_NETWORK_PROVIDED			= 3
} ScreenInd_t;

typedef struct
{
	UInt8				ton;					///< Type of number
	UInt8				npi;					///< Numbering plan
	PresentationInd_t	present;
	ScreenInd_t			screen;
	UInt8				c_num;					///< Number of BCD digits in num
	UInt8				num[ MAX_DIGITS + 1];	///< BCD digits ( 1 digit / byte )
} CallingParty_t;

///

/// Voice Call Release Message
typedef struct
{
	UInt8		callIndex;		///< Index associated with the call
	Cause_t     exitCause;		///< Call release cause
	UInt32		callCCMUnit;	///< Call Meter Unit
	UInt32		callDuration;	///< Call Duration
	UInt8		codecId;			///< provides a codecId based on 3GPP26.103 Sec 6.3.as of now the only valid
										///< values are 0x0a and 0x06. 

} VoiceCallReleaseMsg_t;

/**
This structure is used for the indication sent from the API level
to the client as an indication to inform all of the clients about
which responded to the incomming call indication and whether the
response was successful or failed.
**/
typedef struct
{
	ClientCmd_t		clientCmdType;	///< The client ID
	ApiClientCmd_CcParam_t*			paramPtr;		///< The pointer to the parameters
} ApiClientCmdInd_t;


typedef enum {
	MNCC_SIG_DIAL_TONE_ON		=  0x00,
	MNCC_SIG_RINGBACK_TONE_ON	=  0x01,
	MNCC_SIG_INTERCEPT_TONE_ON	=  0x02,
	MNCC_SIG_CONGESTION_TONE_ON	=  0x03,
	MNCC_SIG_BUSY_TONE_ON		=  0x04,
	MNCC_SIG_CONFIRM_TONE_ON	=  0x05,
	MNCC_SIG_ANSWER_TONE_ON		=  0x06,
	MNCC_SIG_CALL_WAIT_TONE_ON	=  0x07,
	MNCC_SIG_OFF_HOOK_TONE_ON	=  0x08,
	MNCC_SIG_TONES_OFF			=  0x3F,
	MNCC_SIG_ALERTING_OFF		=  0x4F,
	MNCC_SIG_IE_NOT_PRESENT		=  0xFF
} Signal_t;

/// Call Receive Message
typedef struct
{
	UInt8			callIndex;			///< Index associated with the call
	CUGIndex_t		cug_index;			///< Closed User Group Index
	CallingParty_t	callingInfo;		///< Calling Information
	Boolean			auxiliarySpeech;	///< Aux Speech Line info
	Signal_t		setup_ind_signal;	///< setup ind signal type (tone type)
	UInt8			codecId;			///< provides a codecId based on 3GPP26.103 Sec 6.3.as of now the only valid
										///< values are 0x0a and 0x06.  


} CallReceiveMsg_t;


/// Voice Call Waiting Message
typedef struct
{
	UInt8			callIndex;			///< Index associated with the call
	CUGIndex_t		cug_index;			///< Closed User Group Index
	CallingParty_t	callingInfo;		///< Calling Information
	Boolean			auxiliarySpeech;	///< Aux Speech Line info
	Signal_t		setup_ind_signal;	///< setup ind signal type (tone type)

} VoiceCallWaitingMsg_t;

/// Voice Call Connect Message
typedef struct
{
	UInt8		callIndex;			///< Index associated with the call
	ProgressInd_t progress_desc;	///< Progress Indicator
	UInt8			codecId;			///< provides a codecId based on 3GPP26.103 Sec 6.3.as of now the only valid
										///< values are 0x0a and 0x06.  
} VoiceCallConnectMsg_t;

/// Voice Call Pre-Connect Message
typedef struct
{
	UInt8		callIndex;			///< Index associated with the call
	ProgressInd_t progress_desc;    ///< Progress Indicator
	UInt8			codecId;			///< provides a codecId based on 3GPP26.103 Sec 6.3.as of now the only valid
										///< values are 0x0a and 0x06.  
	//...

} VoiceCallPreConnectMsg_t;



#define   CALLING_NAME_PRESENT      0x01	
#define   CALLING_NAME_AVAILABLE    0x02
#define   CALLING_NUMBER_PRESENT    0x10
#define   CALLING_NUMBER_AVAILABLE  0x20

/**
	Calling Name Presentation Masks
**/




	
/// Voice Call Action Message
typedef struct
{
	UInt8			callIndex;		///< Call Index
	Result_t		callResult;		///< Result 
	//The ss error cause for the Multi-Party call actions
	NetworkCause_t	errorCause;		///< Multi part error cause

} VoiceCallActionMsg_t;

/// Bearer Serive Parameters for data call
typedef struct{

	UInt8 	speed;	///< Connection speed (9600 bps, 14400, 57.6K, 64K etc), 
					///< value range = (0,7,16,71,82,83,84,115,116,120,121,130,131,133,134). See section 6.7 of 27.007.
					///< value range = (39,14,43,75,16,48,80) HSCSD Data rates are NOT supported.
	UInt8	name;	///< synchronous or async mode, value range = (0, 1). 0: circuit asynchronous (UDI or 3.1 kHz modem),
					///< 1: circuit synchronous (UDI or 3.1 kHz modem)
	UInt8	ce;		///< connection element, value range = (0, 1) where 0 is for transparent mode and 1 for non-transparent
} CBST_t;


/// Data compression parameters
typedef struct {

    UInt16	direction; ///< Desired compression direction from TE point of view (none=0, tx-only=1, rx-only=2, tx-rx=3)
    UInt16	max_dict;  ///< Desired dictionary size 512-2048
    UInt16	max_string;///< Desired string length 6-32 default 32

} DCParam_t;

/// Radio Link Protocol Parameters
typedef struct
{  
	UInt8  ver;		///< Version (0,1,2)
	UInt16 iws;		///< IWF to MS window size, value range = (0-496) and default value is 61.
	UInt16 mws;		///< MS to IWF window size, value range = (0-496) and default value is 61.
	UInt8  t1;		///< Acknowledgement timer, value range = (38-255) and default value is 48.
	UInt8  n2;		///< Retransmission attempts, value range = (0-8) and default value is 6. 
	UInt8  t2;		///< Internal T2 timer, cannot be set with CRLP command 
	UInt8  t4;		///< Resequencing period, value range = (0-20) and default value is 0.
}RlpParam_t; 

/**
	Error correction parameter requested

	EC: either V42 or MNP protocol
	AUTO: try EC and if EC fails fallback to Normal or Direct
	RELIABLE: try EC and if EC fails Disconnect
	DIRECT: No EC, non-buffered (not supported by our MS)
	NORMAL: No EC, buffered
	V42: only V42 protocol is used for Error Correction
	MNP: only one of the MNP protocols is used for Error Correction
**/
typedef enum
{
	ECORIGREQ_DIRECT = 0,			///<  No Error Correction, DTE-DCE match line rate
	ECORIGREQ_NORMAL = 1,			///< No Error Correction, buffered
	ECORIGREQ_V42_NO_DETECTION = 2,	///< V42 Error Correction, no detection phase
	ECORIGREQ_V42 = 3,				///< V42 Error Correction
	ECORIGREQ_ALTERNATIVE = 4		///< Alternative Error Correction, i.e. MNP
} ECOrigReq_t;						///< Originator Request, see AT+ES

/**
	Error correction parameter fall back value if negotiation is not possible

	EC: either V42 or MNP protocol
	AUTO: try EC and if EC fails fallback to Normal or Direct
	RELIABLE: try EC and if EC fails Disconnect
	DIRECT: No EC, non-buffered (not supported by our MS)
	NORMAL: No EC, buffered
	V42: only V42 protocol is used for Error Correction
	MNP: only one of the MNP protocols is used for Error Correction
**/
typedef enum
{
	ECORIGFB_AUTO_NORMAL = 0,		///< MO, if EC fails, Normal mode
	ECORIGFB_AUTO_DIRECT = 1,		///< MO, if EC fails, Direct mode
	ECORIGFB_RELIABLE = 2,			///< MO, if EC fails, Disconnect
	ECORIGFB_RELIABLE_V42 = 3,		///< MO, if V42 fails, Disconnect
	ECORIGFB_RELIABLE_MNP = 4		///< MO, if MNP fails, Disconnect
} ECOrigFB_t;						///< Originator Fallback, see AT+ES

/**
	Error correction parameter answer fall back value

	EC: either V42 or MNP protocol
	AUTO: try EC and if EC fails fallback to Normal or Direct
	RELIABLE: try EC and if EC fails Disconnect
	DIRECT: No EC, non-buffered (not supported by our MS)
	NORMAL: No EC, buffered
	V42: only V42 protocol is used for Error Correction
	MNP: only one of the MNP protocols is used for Error Correction
**/
typedef enum
{
	ECANSFB_DIRECT = 0,				///< MT, Direct
	ECANSFB_NORMAL = 1,				///< MT, Normal
	ECANSFB_AUTO_NORMAL = 2,		///< MT, EC mode, if EC fails, then Normal
	ECANSFB_AUTO_DIRECT = 3,		///< MT, EC mode, if EC fails, then Direct
	ECANSFB_RELIABLE = 4,			///< MT, V42/MNP mode, if EC fails, Disconnect
	ECANSFB_RELIABLE_V42 = 5,		///< MT, V42 mode, if V42 fails, Disconnect
	ECANSFB_RELIABLE_MNP = 6		///< MT, MNP mode, if MNP fails, Disconnect
} ECAnsFB_t;						///< Answer Fallback, see AT+ES

typedef enum
{
	ECMODE_UNKNOWN = 0,  			///< Unknown,
	ECMODE_DIRECT = 1,  			///< Direct, No Error Correction, DTE-DCE match line rate
	ECMODE_NORMAL = 2,  			///< Normal, No Error Correction, buffered
	ECMODE_MNP2 = 3,	   			///< MNP2
	ECMODE_MNP3 = 4,				///< MNP3
	ECMODE_V42 = 5		   			///< V42, also called LAPM
} ECMode_t;

//******************************************************************************
//	Data Compression Stuff
//******************************************************************************

typedef enum
{
	DCMODE_NONE = 0,				///< No Data Compression
	DCMODE_COMPRESSION_ONLY = 1,	///< Data Compression Only
	DCMODE_DECOMPRESSION_ONLY = 2,	///< Data Decompression Only
	DCMODE_BOTH = 3					///< Compression/Decompression
} DCMode_t;							///< Data Compression Mode (P0), see AT+DS

/// Error correction Parameters
typedef struct
{
	  ECOrigReq_t  ES0;		///< Error correction parameter requested
	  ECOrigFB_t   ES1;		///< Error correction parameter fall back value if negotiation is not possible
	  ECAnsFB_t	   ES2;		///< Error correction parameter answer fall back value
}ECInfo_t;


/// Call Data Compression Parameters

typedef struct
{
	DCParam_t				ds_req_datacomp;	///< Data compression parameters negotiated between peers
	Boolean					ds_req_success_neg;	///< Boolean value to indicate if DC negotiation is necessary for call completion
}DCInfo_t;

/// Data Call Parameters
typedef struct
{
  	UInt8	 curr_ce;		///< Current call connection element(Transparent vs. non-transparent)
  	ECMode_t		EC;		///< Negotiated Error correction parameters
  	DCMode_t		DC;	    ///< Negotiated Data compression parameters
}DataCallInfo_t;

/// Speaker Parameters
typedef struct
{
  UInt8		L;				///< for speaker loudness
  UInt8		M;				///< for speaker monitor
}SpeakerInfo_t;


/** 
	Call Configuration Type
**/
typedef enum
{
	CALL_CFG_DATA_COMP_PARAMS,			///< DCInfo_t
	CALL_CFG_ERROR_CORRECTION_PARAMS,	///< ECInfo_t
	CALL_CFG_CBST_PARAMS,				///< CBST_t
	CALL_CFG_RLP_PARAMS,				///< RlpParam_t
	CALL_CFG_DATA_CALL_PARAMS,			///< DataCallInfo_t
	CALL_CFG_SPEAKER_PARAMS,			///< SpeakerInfo_t
	CALL_CFG_CCM_PARAMS					///< UInt32
}CallConfigType_t;

/// RLP Configuration Parameters 
typedef RlpParam_t	RlpParamList_t[2];

/// Call Configuration Parameters
typedef struct
{
	CallConfigType_t	configType;	///< Type

	union
	{
		DCInfo_t			datacomp_params;	///< union Data compression parameters requested by host
		ECInfo_t			error_correction_params;	///< union Error correction Parameters
		CBST_t				cbst_params;		///< union Bearer Service parameters
		RlpParamList_t		rlp_params;			///< union Radio Link Protocol Parameters
		DataCallInfo_t		datacall_params;	///< datacall related Parameters
		SpeakerInfo_t		speaker_params;		///< speaker setting Parameters
 		UInt32				CCM_params;			///< Current Call Meter setting
	}param_u;	
	
}CAPI2_CallConfig_t;

/// Data Call Connect Message
typedef struct
{
	UInt8		  callIndex;	///< Index associated with the call 
	CBST_t		  cbst;			///< Data call parameters: speed, name and transparent/non-transparent mode.
} DataCallConnectMsg_t;

#endif  // _CAPI2_MSTRUCT_H_

