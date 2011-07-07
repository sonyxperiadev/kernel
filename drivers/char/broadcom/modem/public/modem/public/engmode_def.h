//*********************************************************************
//
//	Copyright © 2009 Broadcom Corporation
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
*   @file   engmode_def.h
*
*   @brief  This file defines Engineering Mode structures
*
****************************************************************************/
#ifndef _TSTPARAM_DEF_H_
#define _TSTPARAM_DEF_H_

#ifdef __cplusplus
extern "C" {
#endif



#define EQUIV_PLMN_LIST_SIZE 16		///< Equivalment PLMN List
	
/// Equivalent PLMN List Structure
typedef struct
{
	UInt8	length;		///< Number of PLMN in "equiv_plmn" 
	PLMN_t	equiv_plmn[EQUIV_PLMN_LIST_SIZE];	///< Equivalent PLMN list
	
} EQUIV_PLMN_LIST_t;
	

#define MAX_NUMBER_OF_UMTS_NCELLS 24
#define MAX_NUMBER_OF_GSM_NCELLS 6
#define MAX_PARAM_PER_MEAS 16


typedef struct
{
	UInt8	p1;		//global measurement parameters
	UInt8	p2;		//global measurement parameters
} MS_GenMeasParam_t;


typedef struct
{
	UInt32	meas_bler;		             //FFFFFFFF if not present
	Int32	target_sir;		             //0xFFFF if not present
	Int32	meas_sir;		             //0xFFFF if not present
// Begin under_dev
    UInt32  dl_tpc_up_count;             //Number of DL TPC "UP" commands
    UInt32  dl_tpc_down_count;           //Number of DL TPC "DOWN" commands
    UInt32  ul_tpc_up_count;             //Number of UL TPC "UP" commands
    UInt32  ul_tpc_down_count;           //Number of UL TPC "DOWN" commands
// End under_dev
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
	MS_GenMeasParam_t		gen_param;
	MS_UmtsMeasIdParam_t	param_per_meas[MAX_PARAM_PER_MEAS];
} MS_UmtsMeasReport_t;


typedef struct 
{
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


typedef struct 
{
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
	UInt16		pathloss;           //Pathloss0 (dB - Always positive value
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


typedef struct 
{
	MS_UmtsNcell_t	A[MAX_NUMBER_OF_UMTS_NCELLS];
} MS_UmtsNcellList_t;


typedef struct 
{
    UInt16 rf_chan_cnt; // Number of channels in Mobile Allocation. <0xFFFF>.  
	UInt16 rf_chan_array[64]; // Channel numbers in Mobile Allocation. <0xFFFF>.
} MS_MA_t;


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
	Access_Tech_List_t 				acc_tech_param_list;
	
}	RLC_Testparam_t;



// BRCM/Davis Zhu 4/27/2004: re-arrange and add c1, c2, c31, c32 (Elbert TR#1029)
// Note: The above added elements are only supported in EDGE protocol stack	at this time
// Invalid/default values are indicated in brackets
typedef struct 
{
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

typedef struct 
{
	MS_NcellMeas_t	A[6];
} MS_NcellList_t;


typedef struct
{
    UInt8 A[8];			// USF value array (0~7) <0xFF>
}
T_USF_ARRAY;

typedef struct 
{
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



// BRCM/Davis Zhu 5/27/2004: add MS_EDGEPacketParam_t (Elbert TR#1029)
// Invalid/default values are indicated in brackets
typedef struct 
{
    Boolean valid;  // validity of all the params in this struct. <False>.
    MS_GPRSPacketParam_t gprs_packet_param; // GPRS parameters.
    UInt8 mean_bep_gmsk; // Mean BEP value of GMSK. <0xFF>.
    UInt8 mean_bep_8psk; // Mean BEP value of 8PSK. <0xFF>.
    UInt8 cv_bep_gmsk; // CV BEP value of GMSK. <0xFF>.
    UInt8 cv_bep_8psk; // CV BEP value of 8PSK. <0xFF>.
    UInt8 bep_per;	// BEP_PERIOD or BEP_PERIOD2. <0xFF>.

} MS_EDGEPacketParam_t;

typedef struct 
{
    Boolean valid; // validity of all the params in this struct. <False>.
	Boolean edge_present; // EDGE present. <False>.
    // BRCM/Davis Zhu 5/27/2004: add EDGEPacketParam_t (Elbert TR#1029)
    MS_EDGEPacketParam_t edge_packet_param;
} MS_EDGEParam_t;


/// MM/NAS parameters
typedef struct 
{
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
	UInt8   mm_t3212_cnt;		///< Current value of LU update timer 
	PLMNId_t		last_reg;	///< Last registered PLMN	

	UInt8	mm_retrans_cnt;		///< Number of LU request retransmissions

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
	UInt8  mm_class;                ///<  0 if GPRS_CLASS_A, 1 if GPRS_CLASS_B, 2 if GPRS_CLASS_C, 3 if GPRS_CLASS_CC,
	                                ///<  4 if GPRS_CLASS_CG 4, 5 if GPRS_NO_CLASS 5 
    /** GPRS attach mode */
	AttachState_t	gprs_attach_mode;
    /** GSM service state */
	Service_State_t   cs_state;
	/** GPRS service state */
	Service_State_t   ps_state;
	/* Per customer requirement, add tmsi and ptmsi. Tony, 11/05/08 */
	TMSI_Octets_t	tmsi;
	TMSI_Octets_t	ptmsi;
    /** HPLMN timer */
	UInt32  hplmn_timer; ///< HPLMN timer
	
} MS_MMParam_t;		

/// GSM/GPRS/EDGE specific parameters
typedef struct 
{
	Boolean	valid;	///< TRUE if the values in this structure are valid; FALSE otherwise

	//	internal protocol stack states
	UInt8			grr_state;				///< internal state in GRR task

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
	UInt8			chn_rel_cause;			///< Channel release cause (T_MMRR_CAUSE)

	// BRCM/Davis Zhu 5/27/2004: add amr_params, gprs_packet_params, edge_params (Elbert TR#1029)
	// AMR, GPRS/EDGE packet mode info
	MS_AMRParam_t		amr_param;				///< AMR test parameters
	MS_GPRSPacketParam_t 	gprs_packet_param;	///< GPRS packet mode test parameters
	MS_EDGEParam_t 		edge_param;				///< EDGE test parameters
	
	// Enhanced  Enginerring mode, BRCM: Wei/Hezhi
	
	UInt8			cs_page_mode;			/// \<NORMALPAGING\>	
	Boolean			pbcch_hopping_status;	/// \<False\>
	Boolean			pccch_hopping_status;	/// \<False\>

	UInt8			t_avg_w;				///< t_avg_w <0>
	UInt8			t_avg_t;				///< t_avg_t <0>
	UInt8			n_avg_i;				///< n_avg_i <0>
	UInt8			pb;						///< pb <0>
	UInt8			ps_page_mode;			///
	Boolean			si13_pres;				///< \<FALSE\>
	UInt8			pc_meas_chan;			///< <0>
	UInt8			t_resel;				///< T_RESEL <5> 
	
	Boolean			hopping_status;			///< hopping_status \<FALSE\>

	UInt8			dtx_from_bs;			///< FTD_SB_DTX_FROM_BS <0X0>

	UInt8			cipher_algorithm;	///< cipher_algorithm: <0>
	UInt8			rr_cause;			///< RR_CAUSE >	
	
	UInt8			max_retrans_cs;		///< Maximum number of Random Access retransmission <0x00>
	Boolean			att;				///< Attach-detach allowed \<False\>
	Boolean			neci;				///< NECI: half rate support \<False\>
	Boolean			acs;				///< Additional reselection parameter indication \<False\>
	Boolean			cbch_sup;			///< cell broadcast channel support \<False\>
	Boolean			reest_allowed;		///< Call Reestablishment Support \<False\>
	Boolean			ecsc;				///< Early Classmark Sending Control \<False\>
	Boolean			si2ter_sup;			///< Are 2-Ter messages supported? \<False\>
	UInt8			 multib_rep;		///< Multiband reporting value <0x00>
	Boolean			cell_bar_acc_2;		///< CELL_BAR_ACCESS_2 \<False\>
	Boolean			exc_acc;			///< Exclusive access right for serving cell \<False\>
	UInt8			ra_resel_hyst;		///< RA_RESELECT_HYSTERSIS <0x00>
	Boolean			pfc_feat_sup;		///< PFC_FEATURE_MODE \<False\>
	Boolean			ext_upl_tbf_sup;	///< NW_EXT_UTBF \<False\>
	Boolean			pkt_si_status;		///< Packet SI_STATUS  \<False\>
	Boolean			ccn_active;			///< Network support Cell Change Notification procedure \<False\>

} MS_GSMParam_t;			


/// UMTS specific parameters


//  MS_UmtsRrcMsg = T_EM_UMTS_RRC_MSG  
typedef enum {
 EM_RRC_MSG_NONE = 0,
 EM_RRC_MSG_ASU,
 EM_RRC_MSG_ASU_COMP,
 EM_RRC_MSG_ASU_FAIL,
 EM_RRC_MSG_ADD,
 EM_RRC_MSG_CCOFU,
 EM_RRC_MSG_CCOFU_FAIL,
 EM_RRC_MSG_CU,
 EM_RRC_MSG_CU_CNF,
 EM_RRC_MSG_CC,
 EM_RRC_MSG_CC_RESP,
 EM_RRC_MSG_HOFU,
 EM_RRC_MSG_HOFU_FAIL,
 EM_RRC_MSG_HOTU,
 EM_RRC_MSG_HOFU_COMP,
 EM_RRC_MSG_IRHI,
 EM_RRC_MSG_MEAS_CTRL,
 EM_RRC_MSG_MEAS_CTRL_FAIL,
 EM_RRC_MSG_PAGE1,
 EM_RRC_MSG_PAGE2,
 EM_RRC_MSG_PC_RECON,
 EM_RRC_MSG_PC_RECON_COMP,
 EM_RRC_MSG_PC_RECON_FAIL,
 EM_RRC_MSG_RB_RECON,
 EM_RRC_MSG_RB_RECON_COMP,
 EM_RRC_MSG_RB_RECON_FAIL,
 EM_RRC_MSG_RB_REL,
 EM_RRC_MSG_RB_REL_COMP,
 EM_RRC_MSG_RB_REL_FAIL,
 EM_RRC_MSG_RB_SETUP,
 EM_RRC_MSG_RB_SETUP_COMP,
 EM_RRC_MSG_RB_SETUP_FAIL,
 EM_RRC_MSG_CONN_REJ,
 EM_RRC_MSG_CONN_REL,
 EM_RRC_MSG_CONN_REL_COMP,
 EM_RRC_MSG_CONN_REQ,
 EM_RRC_MSG_CONN_SETUP,
 EM_RRC_MSG_CONN_SETUP_COMP,
 EM_RRC_MSG_STATUS,
 EM_RRC_MSG_SMC,
 EM_RRC_MSG_SMC_COMP,
 EM_RRC_MSG_SMC_FAIL,
 EM_RRC_MSG_SCR,
 EM_RRC_MSG_SCRI,
 EM_RRC_MSG_SICI,
 EM_RRC_MSG_TC_RECON,
 EM_RRC_MSG_TC_RECON_COMP,
 EM_RRC_MSG_TC_RECON_FAIL,
 EM_RRC_MSG_TFCC,
 EM_RRC_MSG_TFCC_FAIL,
 EM_RRC_MSG_UECE,
 EM_RRC_MSG_UECI,
 EM_RRC_MSG_URAU,
 EM_RRC_MSG_URAU_CONF,
 EM_RRC_MSG_UMI,
 EM_RRC_MSG_UMI_CONF
} MS_UmtsRrcMsg;


typedef struct {
    // Radio Bearer Information
    UInt8   rb_id;                 // RB Id
    Boolean status;                // RB status - TRUE == started, FALSE == stopped
} MS_UmtsPerRbInfo;

typedef struct {
    // Radio Bearer Information	
    UInt8 rab_id;
    UInt8 domain;                    // CN domain for each established RAB 0 == CS, 1 == PS
    UInt8 reestablishment_timer;     // Reestablishment timer 0 == T314, 1 == T315
    UInt8 no_rbs;                    // Number of RBs in RAB
    MS_UmtsPerRbInfo per_rb_info[8]; // Per RB info for this RAB
} MS_UmtsPerRabInfo;


typedef struct {
    // Radio Bearer Information	 
    UInt8 no_rabs;                      // Number of established RABS
    MS_UmtsPerRabInfo per_rab_info[6];  // RAB Id for each established RAB
    UInt8 no_srbs;                      // Number of established SRBs
    UInt8 srb_id[4];                    // SRB Id for each established SRB
} MS_UmtsRabRbInfo;


typedef struct {
    // L1 State
    Boolean    pccpch;               // PCCPCH configured
    Boolean    sccpch;               // SCCPCH configured
    Boolean    rach;                 // RACH configured
    Boolean    dpch;                 // DPCH configured
    
    // WCDMA decoder status
    UInt16     dl_spreading_factor;
    UInt8      fdpch_slot_format;    // 0..9

    // Dedicated uplink channel power control status
    UInt8      power_control_algorithm;     //1 == alorithm 1, 2 == algorithm 2
    UInt8      power_control_step_size;     //Power Control step size
} MS_UmtsL1Info;


typedef struct {
    UInt16      p_sc;                           // Primary scrambling code
    UInt8       status;                         // 0 - Serving RLS, 1 Non-Serving RLS
} MS_UmtsHsupaPerRlInfo;

typedef struct {
    // FDD HSUPA L2 
    UInt16 tx_rate;                    // Scheduled transmission rate in Kbps
    UInt8  re_tx_ratio;                // Retransmission ratio
    UInt8  happy_bit_ratio;            // Average happy bit value * 10
    // FDD HSUPA
    UInt16 serving_grant;              // Serving Grant * 10
    UInt8  uplink_power_headroom;      // Uplink power headroom
} MS_UmtsHsupaL1L2Info;

typedef struct {
    // FDD HSDPA
    UInt16 data_rate;                  // Kbits/s
    UInt8  cqi;
    UInt8  hsdsch_reception_ratio;
    UInt8  bler;    
} MS_UmtsHsdpaL1L2Info;

typedef struct {
    // Voice RLC-U Information
    UInt8    frame_rate_whole;         // Whole part of the AMR codec frame rate
    UInt8    frame_rate_frac;          // Fractional part of the AMR codec frame rate  
} MS_UmtsAmrInfo;

#define MAX_HSUPA_PERL_RL_INFO 4

typedef struct {
    // FDD HSDPA
    UInt16 hrnti;                               // 16 bit H-RNTI
    // FDD HSDPA
    UInt8 no_hsscch_codes;                      // Number of HSSCCH codes (max 4)
    UInt8 hsscch_codes[4];                      // Range 0..127
    // FDD HSDPA
    UInt8 meas_feedback_cycle;                  // Measurement Feedback Cycle in ms
    UInt8 cqi_repetition_factor;                // CQI Repetition Factor
    UInt8 delta_cqi;                            // Delta CQI
    // FDD HSDPA
    UInt8 harq_mem_partition_type;              // HARQ memory partition type, 0 - Implicit, 1 - Explicit
    // FDD HSDPA
    UInt8 no_harq_processes;                    // Number of HARQ processes
    UInt32 harq_mem_size[8];                    // HARQ process memory size (max 8 processes)
    // FDD HSDPA
	UInt8 delta_ack;                            // Delta ACK (0..8)
	UInt8 delta_nack;                           // Delta NACK (0..8)
	UInt8 ack_nack_repetition_factor;           // ACK NACK repetition factor (1..4)
    // FDD HSUPA L3
    UInt8 edch_tti;                             // EDCH TTI 0 == 2ms, 1 == 10ms
    // FDD HSUPA L3
    UInt16 primary_ernti;                       // 16 bit Primary ERNTI
    UInt16 secondary_ernti;                     // 16 bit Secondary ERNTI
    // FDD HSUPA L3
    UInt8 no_edch_rl;                           // Number of radio links in the EDCH active set 
    MS_UmtsHsupaPerRlInfo hsupa_per_rl_info[MAX_HSUPA_PERL_RL_INFO]; // Per RL info for EDCH radio links

} MS_UmtsHspaConfig;



typedef struct {
    // RRC Global State Change Counters
    UInt32 cell_fach_to_cell_dch;
    UInt32 cell_dch_to_cell_fach;
    UInt32 cell_fach_to_cell_pch;
    UInt32 cell_fach_to_ura_pch;
    UInt32 cell_dch_to_cell_pch;
    UInt32 cell_dch_to_ura_pch;
    // Counters for Timeouts - Screen 1
    UInt32 t300_expiry;
    UInt32 t301_expiry;
    UInt32 t302_expiry;
    UInt32 t304_expiry;
    UInt32 t305_expiry;
    UInt32 t307_expiry;
    UInt32 t308_expiry;
    UInt32 t309_expiry;
    // Counters for Timeouts - Screen 2
    UInt32 t310_expiry;
    UInt32 t311_expiry;
    UInt32 t312_expiry;
    UInt32 t313_expiry;
    UInt32 t314_expiry;
    UInt32 t315_expiry;
    UInt32 t316_expiry;
    UInt32 t317_expiry;
    // Peer Message Count- Screen1
    UInt32 mib;
    UInt32 sib1;
    UInt32 sib2;
    UInt32 sib3;
	UInt32 sib4;
    UInt32 sib5;
    UInt32 sib6;
    UInt32 sib7;
    UInt32 sib8;
    // Peer Message Count- Screen2
    UInt32 sib9;
    UInt32 sib10;
    UInt32 sib11;
    UInt32 sib12;
    UInt32 sib13;
    UInt32 sib14;
    UInt32 sib15;
    UInt32 sib16;
    UInt32 sib17;
    UInt32 sib18;
    // Peer Message Count-Screen3
    UInt32 rrc_connection_request;
    UInt32 rrc_connection_reject;
    UInt32 rrc_connection_setup;
    UInt32 rrc_connection_setup_complete;
    UInt32 rrc_connection_release;
    UInt32 rrc_connection_release_complete;
    UInt32 paging_type_1;
    UInt32 paging_type_2;
    // Peer Message Count-Screen4
    UInt32 radio_bearer_setup;
    UInt32 radio_bearer_setup_complete;
    UInt32 radio_bearer_setup_failure;
    UInt32 radio_bearer_reconfig;
    UInt32 radio_bearer_reconfig_complete;
    UInt32 radio_bearer_reconfig_failure;
    UInt32 radio_bearer_release;
    UInt32 radio_bearer_release_complete;
    UInt32 radio_bearer_release_failure;
    UInt32 transport_channel_reconfig;
    UInt32 transport_channel_reconfig_complete;
    UInt32 transport_channel_reconfig_failure;
    // Peer Message Count-Screen5
    UInt32 physical_channel_reconfig;
    UInt32 physical_channel_reconfig_complete;
    UInt32 physical_channel_reconfig_failure;
    UInt32 active_set_update;
    UInt32 active_set_update_complete;
    UInt32 active_set_update_failure;
    UInt32 handover_to_utran;
    UInt32 handover_to_utran_complete;
    UInt32 handover_from_utran;
    UInt32 handover_from_utran_failure;
    // Peer Message Count-Screen6
    UInt32 cell_update;
    UInt32 cell_update_confirm;
    UInt32 counter_check;
    UInt32 initial_direct_transfer;
    UInt32 downlink_direct_transfer;
    UInt32 uplink_direct_transfer;
    UInt32 measurement_control;
    UInt32 measurement_control_failure;
    UInt32 measurement_report;
    UInt32 security_mode_command;
    UInt32 security_mode_complete;
    UInt32 security_mode_failure;
    // Peer Message Count-Screen7
    UInt32 signalling_connection_release;
    UInt32 signalling_connection_release_ind;
    UInt32 ue_capability_enquiry;
    UInt32 ura_update;
    UInt32 rrc_status;
    UInt32 ura_update_confirm;
    UInt32 utran_mobility_info;
    UInt32 utran_mobility_info_confirm;
    UInt32 utran_mobility_info_failure;
    UInt32 cell_change_order_from_utran;
    UInt32 cell_change_order_from_utan_failure;
    // Call Failure Reasons
    UInt32 radio_link_failure;
    UInt32 umts_gsm_handover;
    UInt32 rlc_unrecoverable_error;
    UInt32 nas_triggered_release;
    UInt32 normal_release;
    UInt32 configuration_failure;
    UInt32 n300_failure;
    UInt32 t314_t315_failure;
    UInt32 n302_failure;
	UInt32 t316_t317_t307_failure;
    UInt32 other_failure;
    // Cell Reselection
    UInt32 idle_reselect_success;           // Number of successful Idle Mode reselections
    UInt32 idle_reselect_failure;           // Number of unsuccessful Idle Mode reselections
    UInt32 connected_reselect_success;      // Number of successful Connected Mode reselections
    UInt32 connected_reselect_failure;      // Number of unsuccessful Connected Mode reselections
    UInt32 umts_gsm_reselect_success;       // Number of successful 3G to 2G reselections
    UInt32 umts_gsm_reselect_failure;       // Number of unsuccessful 3G to 2G reselections
    UInt32 gsm_umts_reselect_success;       // Number of successful 2G to 3G reselections
    UInt32 gsm_umts_reselect_failure;       // Number of unsuccessful 2G to 3G reselections
    // DPCH Change Count
    UInt32 hho_success;                     // Number of successful Hard Handovers
    UInt32 dpch_reconfig_success;           // Number of successful DPCH reconfigurations
    // WCDMA decoder status
    UInt32 out_of_sync;                     // NUmber of times the UE has gone out of sync    

} MS_UmtsRrcCounters;

typedef struct {
    // RACH MSG TX profile
    Int8   initial_tx_pwr;
    Int8   po;
    Int8   pp_m;
    UInt16 start_timing;
    UInt8  first_access_slot;
    UInt16 subchannel_mask;
    UInt8  msg_len;                        // 1 - 10ms, 2 - 20ms
    UInt16 sig_sel_mask;
    UInt16 sig_sel_rand_seed;              // Signature selection random seed
    UInt8  ctrl_channel_gain;
    UInt8  data_channel_gain;
    UInt16 preamble_transmission_count;
    UInt8  msg_transmission_result;        // 0 - not transmitted, 1 - transmitted, 2 - denied
    UInt16 data_channel_spreading_factor;  // 256, 128,64 or 32
    Int8   msg_power;
} MS_UmtsRachInfo;



// End under_dev

#define MAX_RRC_MSG_SEQUENCE	8

typedef struct
{
	/** TRUE if the values in this structure are valid; FALSE otherwise */
	Boolean					valid;

	// internal protocol stack states
	UInt8					rrc_state;		///< RRC state

	/** State and events in URRCDC
		this is	T_RRC_PROCESS_STATE.event << 8 | T_RRC_PROCESS_STATE.process_state */
	UInt16					rrcdc_state;	

	UInt16					rrcbp_state;	///< State and events in URRCBP
	UInt16					rrcm_state;		///< State and events in URRCM

    // General RRC state info
    Boolean cs_active;                      ///< CS signalling connection status
    Boolean ps_active;                      ///< PS signalling connection status
    Boolean cs_ciphering;                   ///< CS signalling connection status
    Boolean ps_ciphering;                   ///< PS signalling connection status   

    // Identities
    UInt8  srnc_indentity;                  ///< SRNC Indentity
    UInt32 srnti;                           ///< SRNTI
    UInt16 crnti;                           ///< CRNTI

    Boolean sync_status;                    ///< Sync status TRUE == IN SYNC, FALSE == OUT OF SYNC

    UInt8 search_type;
    UInt8 search_trigger;

	// serving cell info
	UInt16					p_sc;			///< primary scrambling code
	UInt16					lac;			///< location area code
	UInt8					rac;			///< routing area code
	UInt32					cell_id;		///< cell identity
	UInt16					ul_uarfcn;		///< uplink UARFCN of serving cell
	UInt16					dl_uarfcn;		///< downlink UARFCN of serving cell
	UInt16					cpich_rscp;		///< CPICH RSCP (dBm) of serving cell
	UInt8					cpich_ecn0;		///< CPICH Ec/No (dB) of serving cell
	UInt8				    pathloss;		// Pathloss (=PCPICH-Tx-Pwr - RSCP) of serving cell
	Boolean					cipher_on;		///< ciphering indicator
	Boolean					hcs_used;		///< HCS indicator
	Boolean					high_mobility;	///< high mobility indicator
	UInt16					ranking_value;	///< calculated ranking value

	UInt8					chn_rel_cause; 	///< cause of last channel release
	Int8					tx_pwr;			///< transmit output power
	Boolean					scell_hsdpa_ind;	///< serving cell HSDPA indicator
	Boolean					scell_hsupa_ind;	///< serving cell HSUPA indicator

	Int8					rssi;			///< received signal strength
 
	// UMTS neighbor cell info
	UInt8					no_active_set_cells;			///< Number of ACTIVE SET cells (CELL_DCH)
	UInt8					no_virtual_active_set_cells;	///< Number of VIRTUAL ACTIVE SET cells (CELL_DCH)
	UInt8					no_umts_ncells;					///< Number of DETECTED UMTS cells (CELL_DCH) or

	/** Number of UMTS neighbour cells (all states but CELL_DCH) */
	MS_UmtsNcellList_t		umts_ncell;						
 
	// GSM neighbor cell info
	UInt8					no_gsm_ncells;					///< Number of GSM neighbor cells
	MS_UmtsGsmNcellList_t	gsm_ncell;						///< list of GSM neighbor cells

	// UMTS measurement report
	MS_UmtsMeasReport_t		meas_report; ///< mirrors T_UMTS_MEAS_EVAL_PARAM in stack
 
	// DCH quality report
	MS_UmtsDchReport_t		dch_report; ///< mirrors T_EM_DCH_REPORT struct in stack
    Boolean                 hsdch_reception_on;      ///< HS_DSCH reception ongoing indicator
    Boolean                 is_hsdpa_cell_capable;   ///<TRUE means cell is HSDPA capable

    Boolean                 edch_transmission_on;    ///< EDCH transmission ongoing indicator
    Boolean                 is_hsupa_cell_capable;   ///< TRUE means cell is HSUPA capable    

	MS_UmtsRrcMsg           rrc_msg_sequence[MAX_RRC_MSG_SEQUENCE]; ///< RRC message sequence - circular buffer - last entry is always EM_RRC_MSG_NONE
    MS_UmtsRrcCounters      rrc_counters;        ///< RRC protocol counters
    MS_UmtsHspaConfig       hspa_config;         ///< HSPA configuration information
    MS_UmtsHsupaL1L2Info    hsupa_l1_l2_info;    ///< L1 L2 HSUPA information
    MS_UmtsHsdpaL1L2Info    hsdpa_l1_l2_info;    ///< L1 L2 HSDPA information
    MS_UmtsL1Info           l1_info;             ///< L1 Info
    MS_UmtsRabRbInfo        rab_rb_info;         ///< RAB and RB Info
    MS_UmtsAmrInfo          amr_info;            ///< AMR codec info
    MS_UmtsRachInfo         rach_info;           ///< RACH info


} MS_UMTSParam_t;    // this structure corresponds to T_RRC_TESTPARAM struct in stack					

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

typedef enum
{

	EM_READY_STATE,
	EM_AA_READY_STATE,
	EM_STANDBY_STATE,
	EM_IDLE_STATE

} MS_Ext_ReadyState_t;

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

	// force to standy flag
	Boolean		force_to_standby;
	Boolean		ps_suspended;

} MS_Ext_MMParam_t;


typedef struct
{
	UInt32		gsm_gsm_resel_cnt;	// Counter for GSM900->GSM900 cell reselection(max 99999) <0>
	UInt32		dcs_gsm_resel_cnt;	// Counter for GSM1800->GSM900 cell reselection <0>
	UInt32		gsm_dcs_resel_cnt;	// Counter for GSM900->GSM1800 cell reselection <0>
	UInt32		dcs_dcs_resel_cnt;	// Counter for GSM1800->GSM1800 cell reselection <0>
	
	// grr engineering mode information
	UInt8		si13_location;
	UInt8		psi5_present;
	UInt8		psi_status_ind;

	UInt8		bs_cv_max;
	UInt8		pan_dec;
	UInt8		pan_inc;
	UInt8		pan_max;

	// HO (handover + channel assign) counters

	// attemp counters
	UInt32		gsm_ho_att_cnt[GSM_HO_MAX_TYPES][GSM_HO_MAX_BAND_SWITCH];
	
	// failed counters
	UInt32		gsm_ho_fail_cnt[GSM_HO_MAX_TYPES][GSM_HO_MAX_BAND_SWITCH];

	// successful counters
	UInt32		gsm_ho_ok_cnt[GSM_HO_MAX_TYPES][GSM_HO_MAX_BAND_SWITCH];

	// successfully back to previous counters
	UInt32		gsm_ho_back_cnt[GSM_HO_MAX_TYPES][GSM_HO_MAX_BAND_SWITCH];

	
} MS_Ext_RRParam_t;

typedef enum
{
	EM_MACMODE_MM_SINGLE_BLOCK,
	EM_MACMODE_SINGLE_BLOCK_WITHOUT_TBF,
	EM_MACMODE_DYNAMIC_ALLOCATION,
	EM_MACMODE_EXT_DYNAMIC_ALLOCATION,
	EM_MACMODE_FIXED_HALF_DUPLEX,
	EM_MACMODE_FIXED_FULL_DUPLEX,
	EM_MACMODE_MAC_MODE_UNDEFINED,
	EM_MACMODE_MM_EXT_DYNAMIC_ALLOCATION,
	EM_MACMODE_MM_DYNAMIC_ALLOCATION,
	EM_MACMODE_MM_FIXED_ALLOCATION

} MS_Ext_MACMode_t;

typedef enum
{
	EM_RLC_ACK,
	EM_RLC_UNACK,
	EM_RLC_MODE_UNDEFINED 
	
} MS_Ext_RLCMode_t;

typedef enum
{

	EM_TBF_TYPE_OPEN_ENDED,
	EM_TBF_TYPE_CLOSE_ENDED,
	EM_TBF_TYPE_UNDEFINED 

} MS_Ext_TbfType_t;

typedef struct
{
	UInt8		tfi;				// FTD_SB_MAC_DL_TFI <default:0xff>
	UInt8		mac_mode;			// FTD_SB_MAC_ALLOC_MODE <default: MAC_MODE_UNDEFINED(6)> 
	UInt8		rlc_mode;
	UInt8		tbf_type;
	UInt32		n3102;
	Boolean		usf_granularity; 

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
	UInt32		mo_pdp_attempt_cnt;	//counter for the MS initiated PDP context avtivation attempt

	UInt8		pdp_priority;		// radio priority of PDP context
	UInt8		sec_pdp_priority;	// radio priority of second PDP context

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
typedef struct 
{
	MS_MMParam_t	mm_param;		
	MS_GSMParam_t	gsm_param;		
	MS_UMTSParam_t	umts_param;		
	MS_ExtParam_t   ext_param;	
} MS_RxTestParam_t;


#define RTEM_NUM_REPORT_FIELDS 4

typedef struct 
{
    E_RTEM_EVENT_ID event;
    Int32	report_fields[RTEM_NUM_REPORT_FIELDS];	// assume these integers are enough to carry event details.
} MS_RtemReport_t;


extern MS_RxTestParam_t RxTestParam;  // Hezhi: 2/26/08: change to global so the new eng mode info direct written



#ifdef __cplusplus
}
#endif


#endif  //_TSTPARAM_DEF_H_




