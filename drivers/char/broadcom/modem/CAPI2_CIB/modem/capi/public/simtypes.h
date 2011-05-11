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
*   @file   simtypes.h
*
*   @brief  This file contains the SIM message types.
*
****************************************************************************/


#ifndef _SIMTYPES_H_
#define _SIMTYPES_H_



//******************************************************************************
// Constant Definitions
//******************************************************************************
#define SI_SMS_LGTH          0xb0	// SI_SMS_LGTH is defined in sim_op1.h.
									// The first byte is status, the rest are datas.

//******************************************************************************
// Enumeration Definitions
//******************************************************************************

typedef enum
{
	SIMMSG_SIMI_INIT_IND,
	SIMMSG_SIMI_INFO_IND,
	SIMMSG_SUPPORT_APPLI_IND,
	SIMMSG_APPLI_SOCKET_ID,
	SIMMSG_SELECT_APPLI_IND,
	SIMMSG_DEACTIVATE_APPLI_ID,
	SIMMSG_CLOSE_SOCKET_IND,
	SIMMSG_SIMI_RESET_IND,
	SIMMSG_SIMI_ERR_IND,

	SIMMSG_SIMI_CHV_VERIFY_CNF,
	SIMMSG_SIMI_CHV_VERIFY_REJ,
	SIMMSG_SIMI_CHV_CHANGE_CNF,
	SIMMSG_SIMI_CHV_CHANGE_REJ,
	SIMMSG_SIMI_CHV_UNBLOCK_CNF,
	SIMMSG_SIMI_CHV_UNBLOCK_REJ,
	SIMMSG_SIMI_CHV1_DISABLE_CNF,
	SIMMSG_SIMI_CHV1_DISABLE_REJ,
	SIMMSG_SIMI_CHV1_ENABLE_CNF,
	SIMMSG_SIMI_CHV1_ENABLE_REJ,

	SIMMSG_SIMI_SST_CNF,
	SIMMSG_SIMI_SST_REJ,
	SIMMSG_SIMI_IMSI_CNF,
	SIMMSG_SIMI_IMSI_REJ,

	SIMMSG_SIMI_PBK_READ_CNF,
	SIMMSG_SIMI_PBK_READ_REJ,

	SIMMSG_SIMI_PBK_UPDATE_CNF,
	SIMMSG_SIMI_PBK_UPDATE_REJ,

	SIMMSG_SIMI_ENABLE_FDN_CNF,
	SIMMSG_SIMI_ENABLE_FDN_REJ,    
	SIMMSG_SIMI_DISABLE_FDN_CNF,
	SIMMSG_SIMI_DISABLE_FDN_REJ,
    SIMMSG_SIMI_ENABLE_BDN_CNF,
	SIMMSG_SIMI_ENABLE_BDN_REJ,
    SIMMSG_SIMI_DISABLE_BDN_CNF,
	SIMMSG_SIMI_DISABLE_BDN_REJ,

	SIMMSG_SIMI_DIAL_NO_INFO_CNF,

	SIMMSG_SIMI_ACM_READ_CNF,
	SIMMSG_SIMI_ACM_READ_REJ,
	SIMMSG_SIMI_ACM_UPDATE_CNF,
	SIMMSG_SIMI_ACM_UPDATE_REJ,
	SIMMSG_SIMI_ACM_INCREASE_CNF,
	SIMMSG_SIMI_ACM_INCREASE_REJ,

	SIMMSG_SIMI_ACMMAX_READ_CNF,
	SIMMSG_SIMI_ACMMAX_READ_REJ,
	SIMMSG_SIMI_ACMMAX_UPDATE_CNF,
	SIMMSG_SIMI_ACMMAX_UPDATE_REJ,

	SIMMSG_SIMI_PUCT_READ_CNF,
	SIMMSG_SIMI_PUCT_READ_REJ,
	SIMMSG_SIMI_PUCT_UPDATE_CNF,
	SIMMSG_SIMI_PUCT_UPDATE_REJ,

	SIMMSG_SIMI_SM_READ_CNF,
	SIMMSG_SIMI_SM_READ_REJ,
	SIMMSG_SIMI_SM_UPDATE_CNF,
	SIMMSG_SIMI_SM_UPDATE_REJ,
	SIMMSG_SIMI_SM_UPDATE_STATUS_CNF,
	SIMMSG_SIMI_SM_UPDATE_STATUS_REJ,

	SIMMSG_SI_SM_REF_NUMBER_IND, 

	SIMMSG_SIMI_SM_PARAM_READ_CNF,
	SIMMSG_SIMI_SM_PARAM_READ_REJ,
	SIMMSG_SIMI_SM_PARAM_UPD_CNF,
	SIMMSG_SIMI_SM_PARAM_UPD_REJ,

    SIMMSG_SIMI_SM_RAW_ATR_IND,
    SIMMSG_SIMI_SM_APDU_IND,

	SIMMSG_SIMI_SMS_CAPACITY_AVAIL_IND,
	SIMMSG_SIMI_SMS_CAPACITY_EXCEEDED_IND,

	SIMMSG_SI_GENERIC_CMD_IND,	// Restricted SIM access (e.g. AT+CRSM command)
	
	SIMMSG_SI_CHV_ATTEMPT_CNF,

} SIMMsgType_t;


//******************************************************************************
// Type Definitions
//******************************************************************************



//******************************************************************************
// Structure Typedefs
//******************************************************************************
typedef UInt8			L3SIM_DFileParm_t[sizeof(T_SI_DFP)];
typedef UInt8			L3SIM_EFileParm_t[sizeof(T_SI_EFP)];

typedef struct
{
	SIMAccess_t	status;
} SIMParmAccessStatus_t;

typedef struct
{
	UInt8 record_no;
	SIMAccess_t	status;
} SIMParmAccessIndexStatus_t;

/// CHV Status
typedef enum
{
	CHVSTATUS_CHV_NOT_AVAILABLE = SI_CHV_NOT_AVAILABLE,		///< CHV is not available
	CHVSTATUS_CHV_NEEDED 		= SI_CHV_NEEDED,			///< CHV available and must be entered
	CHVSTATUS_CHV_NOT_NEEDED 	= SI_CHV_NOT_NEEDED,		///< CHV available, but no need
	CHVSTATUS_CHV_BLOCKED 		= SI_CHV_BLOCKED,			///< blocked, but unblocking is possible
	CHVSTATUS_CHV_UNBL_BLOCKED 	= SI_CHV_UNBL_BLOCKED,		///< unblocking is not possible
	CHVSTATUS_CHV_VERIFIED 		= SI_CHV_VERIFIED,			///< CHV has been verified
	CHVSTATUS_CHV_REJECTED 		= SI_CHV_REJECTED,			///< CHV vlaue just presented is incorrect
	CHVSTATUS_CHV_CONTRADICTION = SI_CHV_IN_CONTRADICTION_WITH_CHV_STATUS, ///<STK++ SI_CHV_CONTRADICTION is changed to SI_CHV_IN_CONTRADICTION_WITH_CHV_STATUS STK
	CHVSTATUS_CHV1_DISABLE_NOT_ALLOWED = SI_CHV1_DISABLE_NOT_ALLOWED, ///< Disabling CHV1 is not allowed in EF-SST of 2G SIM 
	CHVSTATUS_CHV_MEMORY_PROBLEM = SI_CHV_MEMORY_PROBLEM	///< Memory problem status word returned by SIM 
} CHVStatus_t;

typedef struct
{
	CHV_t		chv_no;
	CHVStatus_t	chv_status;				// contains what SIM AP returns
	SIMAccess_t	chv_access_status;		// contains what MTI code expects
	PinStatusWord_t sw;
} SIMParmCHVStatus_t;

typedef struct
{
	SST_t				sst;
} SIMParmSSTCnf_t;

typedef struct
{
	IMSI_t 				imsi;
} SIMParmIMSICnf_t;

typedef struct
{
	UInt8 rec_no;
	SIMPBK_ID_t	  pbk_id;
	SIMPBK_Entry_t number;
} SIMParmPbkReadCnf_t;

typedef struct
{
	SIMAccess_t	status;
	UInt8 rec_no;
	SIMPBK_ID_t	  pbk_id;
	SIMPBK_Entry_t number;
} SIMParmPbkReadRej_t;

typedef struct
{
	SIMAccess_t	status;
} SIMParmPbkAccessRej_t;

typedef struct
{
	UInt32				max_adn_rec_no;
	UInt32				adn_alpha_len;
	UInt32				max_msisdn_rec_no;
	UInt32				msisdn_alpha_len;
	UInt32				max_fdn_rec_no;
	UInt32				fdn_alpha_len;
	UInt32				max_lnd_rec_no;
	UInt32				lnd_alpha_len;
    UInt32				max_bdn_rec_no;
    UInt32				bdn_alpha_len;
} SIMParmDialNoInfoCnf_t;

typedef struct
{
	UInt8	record_no;
	CallMeterUnit_t	acm_data;
	UInt32			total_num_record;	// total number of records
} SIMParmACMReadCnf_t;

typedef struct
{
	CallMeterUnit_t			acm_data;
	CallMeterUnit_t			acmmax_data;
	CurrencyName_t		currency;
	EPPU_t				eppu;
} SIMParmACMInfoCnf_t;

typedef struct
{
	CallMeterUnit_t			acmmax;
} SIMParmACMMaxReadCnf_t;

typedef struct
{
	CurrencyName_t		currency;
	EPPU_t				eppu;
} SIMParmPUCTReadCnf_t;


typedef struct
{
	UInt8	record_no;
} SIMParmGenRecordNoCnf_t;

typedef struct
{
	UInt8				record_no;
	Boolean				is_extension;
	UInt8				part_no;			// 1 is first part
	UInt8				data_sz;			// size of data_buf
	UInt8				data_buf[ SI_SMS_LGTH ];
} SIMParmSMReadCnf_t;

/// SIM SMS Status
typedef struct
{	// See GSM 11.11, Section 10.3.7
	UInt8				last_used_tp_mr;		///< last used TP-Message Reference number
	Boolean				is_mem_available;		///< TRUE if memory is available to store SMS
} SIMSMSStatus_t;

typedef struct
{
	SIMSMSStatus_t	sms_status;
} SIMParmSMStatusCnf_t;


typedef struct
{
	UInt8	tp_mr;
}SIMParmRefNumInd_t;

typedef struct
{
	Boolean			mem_avail;
}SIMParmMemAvailInd_t;


typedef struct
{
	UInt8				record_no;
	SIMPBK_Name_t 		name;				// null terminated
	Boolean 			is_first_part;
	UInt8				alpha_id_sz;		// only valid with is_first_part == TRUE
	Boolean				is_extension;
	UInt8				data_sz;			// size of data_buf
	UInt8				data_buf[32];		// sizeof(T_SI_DATA)
} SIMParmSMParamReadCnf_t;

typedef struct
{
	SIMAccess_t	status;
	SIMOperState_t		oper_state;
    SIMBdnOperState_t   bdn_oper_state;
} SIMParmInitInd_t;

typedef struct
{
	SIMPresent_t		sim_present;
	ICCID_BCD_t			iccid;
	SIMType_t			sim_type;
	UInt8				sim_phase;
	CHVStatus_t			chv1_status;
	CHVStatus_t			chv2_status;
	SIM_APPL_TYPE_t		card_type;	/* indicates whether 2G SIM or 3G USIM is inserted */
	UInt8				imsi_mnc_len; /* 0 if MNC digit length not known, e.g. for 2G SIM where EF-AD does not define it */
	T_SI_SIM_LANGUAGE_LIST	language_list1;	/* Data of EF-LI for 3G USIM or EF-ELP for 2G SIM */
	T_SI_SIM_LANGUAGE_LIST	language_list2;	/* Data of EF-PL for 3G USIM or EF-LP for 2G SIM */
} SIMParmInfoInd_t;

typedef struct
{
	T_SIM_ERROR_CODE	error_code;
} SIMParmErrInd_t;

typedef struct
{
	APDUFileID_t		dfile_id;
	UInt8				dfile_parm_length;
	L3SIM_DFileParm_t	dfile_parm_data;
	APDUFileID_t		efile_id;
	UInt8				efile_parm_length;
	L3SIM_EFileParm_t	efile_parm_data;
	APDU_t				*p_apdu;
} SIMParmGenericCmdInd_t;

/// Data for reading multiple records in a SIM file 
typedef struct
{
	UInt8 num_of_rec;	///< Number of records returned 
	UInt8 rec_length;	///< Length of each record 
	UInt8 *rec_data;	///< Dynamically allocated pointer pointing to the record data 
} SIMParmGenRecInd_t;

/// SIM toolkit data structure definition 
typedef struct
{
	UInt8				display_type;
	Boolean				sustained_text;		/**< TRUE for Sustained Display Text for which Terminal Response must be sent 
											 * without delay: see Section 6.4 of ETSI TS 102 223.
 											 */
	T_SI_TEXT			text;
	T_SI_ICON_ID		icon;	//optional icon id
    T_DURATION          duration;
} SIMParmDisplayTextReq_t;

/// SIM setup Idle mode text request
typedef struct
{
	UInt8				display_type;
	T_SI_TEXT			text;
	T_SI_ICON_ID		icon;	//optional icon id
} SIMParmSetupIdlemodeTextReq_t;

/// Get input Request
typedef struct
{
	UInt8				input_type;
	T_SI_TEXT			text;
	UInt8				min_rsp_length;
	UInt8				max_rsp_length;
	T_SI_TEXT_PTR		default_rsp_text_ptr;
	T_SI_ICON_ID		icon;
} SIMParmGetInputReq_t;

/// Get input Key request
typedef struct
{
	UInt8				inkey_type;
	T_SI_TEXT			text;
	T_SI_ICON_ID		icon;	//optional icon id
    T_DURATION          duration;
} SIMParmGetInkeyReq_t;

/// Play Tone Request
typedef struct
{
	UInt8				tonetype;
	T_DURATION			duration;
	T_ALPHA_IDENTIFIER	text;	//optional icon id
	T_SI_ICON_ID		icon;				//optional icon id
} SIMParmPlayToneReq_t;

/// Select Item request
typedef struct
{
	UInt8				help_info_available;	///< Bit 8: 0 or 1 ==> no help or help available
	UInt8				default_item_identifier; ///< which menu item is selected by default
	T_MI_DATA 			mi_data;				///< includes length and data pointer (JDN doc inconsistent)
} SIMParmSelectItemReq_t;

/// Setup Menu Request
typedef struct
{
	UInt8				help_info_available;	///< Bit 8: 0 or 1 ==> no help or help available
	T_MI_DATA 			mi_data;				///< includes length and data pointer (JDN doc inconsistent)
} SIMParmSetupMenuReq_t;

/// SIM toolkit refresh request
typedef struct
{
	STKRefreshType_t	refresh_type;

	/// File change list for Refresh of type File Change Notification, SIM Init With File Change or Full File Change.  
	REFRESH_FILE_LIST_t	file_list;	
	
	ProactiveCmdData_t  refresh_cmd; ///< Refresh command raw data for Generic STK interface: no data is available for Refresh of Reset type

	USIM_APPLICATION_TYPE appli_type;	///< Application type for the refresh. This parameter is set to "USIM_APPLICATION" for 2G SIM.

} SIMParmSimtoolkitRefreshReq_t;

/// SIAT SIM toolkit refresh request
typedef struct
{
	Boolean				sim_reset;			///< indicates SIM has been reset
	Boolean				full_file_change;	///< indicates that all Element Files have changed
} SIMParmSiatSimtoolkitRefreshReq_t;

/// Send SMS PDU request
typedef struct
{
	T_PDU					t_pdu;		///< the SMS daa
	T_ALPHA_IDENTIFIER 	text;			///< a string shich MI displays. Passed to MN and MN infomrs MI
	T_SI_ICON_ID		icon;			///< optional icon id
} SIMParmSmsSendPduReq_t;				// not used

/// Send Supplementary service request
typedef struct
{
	T_SIM_SS_DATA			ss_string;	///< the SMS daa
	T_ALPHA_IDENTIFIER 	text;			///< a string shich MI displays. Passed to MN and MN infomrs MI
	T_SI_ICON_ID		icon;			///< optional icon id
} SIMParmSendSsReq_t;

/// MO SMS alpha indication
typedef struct
{
	T_ALPHA_IDENTIFIER 	text;			///< a string shich MI displays. Passed to MN and MN infomrs MI
	T_SI_ICON_ID		icon;			///< optional icon id
	STK_SMS_DATA_t		sms_data;		///< SMS data including Service Center Address and TPDU
} SIMParmMoSMSAlphaInd_t;

/// Call setup request
typedef struct {
    T_EST_CAUSE  establish_cause;
    byte  cmd_qualifier;
    T_ADDRESS  phone_number;
    T_MN_BC  bearer_capability;
    T_CALLED_SUBADDR  subaddress;
    T_DURATION  duration;
    T_ALPHA_IDENTIFIER  text;
	T_SI_ICON_ID		icon;
    T_ALPHA_IDENTIFIER  text2;
	T_SI_ICON_ID		icon2;
}SIMParmSetupCallReq_t;

/// Send DTMF tone request
typedef struct {
    T_SIM_SS_DATA		dtmf;
    T_ALPHA_IDENTIFIER  text;
	T_SI_ICON_ID		icon;
}SIMParmSendDtmfReq_t;

typedef struct {
	T_SI_CHV_ATTEMPTS attempt;	/* Remaining attempts for PIN1, PIN2, PUK1, PUK2 */
} SIMChvAttempt_t;

/// Open Channel data structure to be passed to STK task
typedef struct
{
	Boolean					immediate_establish;
	Boolean					auto_reconnect;
	T_ALPHA_IDENTIFIER		alpha_id;
	T_SI_ICON_ID			icon_id;
	T_SI_BEARER_DESC		bearer_desc;
	UInt16					buffer_size;
	T_SI_IP_ADDRESS			local_ip_addr;
	T_SI_TEXT				user_login;
	T_SI_TEXT				login_pwd;
	T_SI_TRANSPORT_LAYER	transport_layer;
	UInt16					port_num;
	T_SI_IP_ADDRESS			dest_ip_addr;
	T_ADDRESS				csd_dial_num;
	T_DURATION				reconnect_duration;
	T_DURATION				link_timeout_duration;
	T_SI_NETWORK_ACC_NAME	apn_name;
} SIMParmOpenChannelReq_t;

/// Send Data data structure to be passed to STK task
typedef struct
{
	UInt8					channel_id;
	Boolean					send_data_immediately;
	T_ALPHA_IDENTIFIER		alpha_id;
	T_SI_ICON_ID			icon_id;
	T_SI_CHANNEL_DATA		channel_data;
} SIMParmSendDataReq_t;

/// Receive Channel data structure to be passed to STK task
typedef struct
{
	UInt8					channel_id;
	UInt8					data_length;
	T_ALPHA_IDENTIFIER		alpha_id;
	T_SI_ICON_ID			icon_id;
} SIMParmReceiveDataReq_t;

/// Close Channel data structure to be passed to STK task
typedef struct
{
	UInt8					channel_id;
	T_ALPHA_IDENTIFIER		alpha_id;
	T_SI_ICON_ID			icon_id;
	UInt8                   cmd_qual;  /* for UICC server mode */
} SIMParmCloseChannelReq_t;

/// Raw ATR response indication
typedef struct
{
    Boolean atr_valid;
    UInt8 len;
    T_SI_RAW_ATR_DATA data;
} SIMParmRawAtrInd_t;

typedef struct
{
    T_USIMAP_DATA_BUF data;
    UInt16 len;
}SIMParmRcvApduInd_t;

/// SIM message parameter
typedef union
{
	SIMParmInitInd_t		init_ind;
	SIMParmInfoInd_t		info_ind;
	SIMParmErrInd_t			err_ind;

	SIMParmCHVStatus_t		chv_generic_status;
	SIMParmCHVStatus_t		chv_verify_cnf;
	SIMParmCHVStatus_t		chv_verify_rej;
	SIMParmCHVStatus_t		chv_change_cnf;
	SIMParmCHVStatus_t		chv_change_rej;
	SIMParmCHVStatus_t		chv_unblock_cnf;
	SIMParmCHVStatus_t		chv_unblock_rej;
	SIMParmCHVStatus_t		chv1_disable_cnf;
	SIMParmCHVStatus_t		chv1_disable_rej;
	SIMParmCHVStatus_t		chv1_enable_cnf;
	SIMParmCHVStatus_t		chv1_enable_rej;

	SIMParmSSTCnf_t			sst_cnf;
	SIMParmAccessStatus_t	sst_rej;
	SIMParmIMSICnf_t		imsi_cnf;
	SIMParmAccessStatus_t	imsi_rej;

	SIMParmPbkReadCnf_t		pbk_read_cnf;
	SIMParmPbkReadRej_t		pbk_read_rej;

	SIMParmPbkAccessRej_t	pbk_update_rej;

	SIMParmAccessStatus_t	enable_fdn_cnf;
	SIMParmAccessStatus_t	enable_fdn_rej;
	SIMParmAccessStatus_t	disable_fdn_cnf;
	SIMParmAccessStatus_t	disable_fdn_rej;
    SIMParmAccessStatus_t	enable_bdn_cnf;
    SIMParmAccessStatus_t	enable_bdn_rej;
    SIMParmAccessStatus_t	disable_bdn_cnf;
    SIMParmAccessStatus_t	disable_bdn_rej;

	SIMParmDialNoInfoCnf_t	dial_no_info_cnf;

	SIMParmACMReadCnf_t		acm_read_cnf;
	SIMParmAccessStatus_t	acm_read_rej;
	SIMParmAccessStatus_t	acm_update_cnf;
	SIMParmAccessStatus_t	acm_update_rej;
	SIMParmACMInfoCnf_t		acm_increase_cnf;
	SIMParmAccessStatus_t	acm_increase_rej;

	SIMParmACMMaxReadCnf_t	acmmax_read_cnf;
	SIMParmAccessStatus_t	acmmax_read_rej;
	SIMParmAccessStatus_t	acmmax_update_cnf;
	SIMParmAccessStatus_t	acmmax_update_rej;

	SIMParmPUCTReadCnf_t	puct_read_cnf;
	SIMParmAccessStatus_t	puct_read_rej;
	SIMParmAccessStatus_t	puct_update_cnf;
	SIMParmAccessStatus_t	puct_update_rej;

	SIMParmSMReadCnf_t		sm_read_cnf;
	SIMParmAccessIndexStatus_t	sm_read_rej;
	SIMParmGenRecordNoCnf_t	sm_update_cnf;
	SIMParmAccessIndexStatus_t	sm_update_rej;
	SIMParmGenRecordNoCnf_t	sm_update_status_cnf;
	SIMParmAccessIndexStatus_t	sm_update_status_rej;

	SIMParmSMParamReadCnf_t	sm_param_read_cnf;
	SIMParmAccessIndexStatus_t sm_param_read_rej;
	SIMParmGenRecordNoCnf_t	sm_param_upd_cnf;
	SIMParmAccessIndexStatus_t sm_param_upd_rej;

	SIMParmGenericCmdInd_t	generic_cmd_ind;
	SIMParmGenRecInd_t		gen_rec_ind;

	SIMParmRefNumInd_t		sm_ref_num;
	SIMParmMemAvailInd_t	sm_capacity_exceeded_ind;
	
	SIMChvAttempt_t	chv_attempt_cnf;

	T_USIMAP_APP_LIST		appli_list;

	T_SI_GEN_EXT_RES		select_appli_result;
	
	T_SI_GEN_EXT_RES		deactivate_appli_result; 

	T_SI_GEN_EXT_RES		close_socket_result;    

	UInt8					socket_id;

    SIMParmRawAtrInd_t      sm_raw_atr_ind;
    SIMParmRcvApduInd_t     sm_rcv_apdu_ind;

} SIMMsgParm_t;

/// SIM message tupe
typedef struct
{
	SIMMsgType_t type;					///< SIM Message Type
	SIMMsgParm_t parm;					///< SIM Message Parameter
} SIMMsg_t;								///< SIM Message

#endif

