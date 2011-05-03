   
//***************************************************************************
//
//	Copyright ?2005-2008 Broadcom Corporation
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
*   @file   sim_mi.h
*
*   @brief  This file contains the function prototypes for the SIM/USIM interface.
*
****************************************************************************/
#ifndef _SIM_MI_H_
#define _SIM_MI_H_
   

#ifdef UHT_HOST_BUILD
extern void uht_set_sim_semaphore(void);
extern void uht_release_sim_semaphore(void);
#endif //UHT_HOST_BUILD


//-------------------------------------------------
// Macro Definitions
//-------------------------------------------------
#define SEMAPHORE_CREATE()		sim_semaphore = OSSEMAPHORE_Create(1, OSSUSPEND_PRIORITY);

#ifndef UHT_HOST_BUILD
#define	SEMAPHORE_OBTAIN()		OSSEMAPHORE_Obtain(sim_semaphore, TICKS_FOREVER); \
								sim_db.sim_state = SIMSTATE_ACTIVE
									
#define	SEMAPHORE_RELEASE()		sim_db.sim_state = SIMSTATE_IDLE; \
								OSSEMAPHORE_Release(sim_semaphore)

#define SEMAPHORE_DELETE()		OSSEMAPHORE_Destroy(sim_semaphore)	
#else
#define	SEMAPHORE_OBTAIN()		OSSEMAPHORE_Obtain(sim_semaphore, TICKS_FOREVER); \
                           		uht_set_sim_semaphore(); \
								sim_db.sim_state = SIMSTATE_ACTIVE
									
#define	SEMAPHORE_RELEASE()		sim_db.sim_state = SIMSTATE_IDLE; \
                           		uht_release_sim_semaphore(); \
								OSSEMAPHORE_Release(sim_semaphore)	
#endif //UHT_HOST_BUILD

/* Return TRUE if there is a current SIM acesss active */
#define SIM_IS_ACCESS_ACTIVE()		(sim_db.sim_state != SIMSTATE_IDLE)
									

//-------------------------------------------------
// Data Structure
//-------------------------------------------------

/// APDU Class
typedef enum
{
	APDUCLASS_2G_A0	= 0xA0,
	APDUCLASS_3G_00	= 0x00,
	APDUCLASS_3G_80	= 0x80
} APDUClass_t;


typedef enum
{
    LOCK_STATUS_DISABLED,
    LOCK_STATUS_LOCKED,
    LOCK_STATUS_UNLOCKED,
    LOCK_STATUS_BLOCKED,
    LOCK_STATUS_PUK_BLOCKED,
    LOCK_STATUS_INVALID
} SIM_LOCK_STATUS_t;


/**
 SIM Lock type enumaration
**/
typedef enum
{
    LOCK_PIN1,
    LOCK_PIN2,
    LOCK_PHONE_DEV,
    LOCK_HIDDEN_KEY,
    LOCK_USIM_APP,
    LOCK_USIM_APP2,
    LOCK_UNIVERSAL_PIN
}SIM_LOCK_TYPE_t;


/**
 Operator Name String info structure.
**/
typedef struct
{
    UInt8 *ons_ptr;
    UInt8 ons_len;
    UInt8 *onss_ptr;
    UInt8 onss_len;
} SIM_CPHS_ONS_TYPE_t;

typedef void (*SIM_PROC_FUNC_t)(const InterTaskMsg_t *msg);

typedef struct
{
	MsgType_t msg_type;
	SIM_PROC_FUNC_t sim_proc_func;
} SIM_PROC_HANDLER_t;


#define	PBK_ENTRY_FIXED_SZ	14		// size of the fixed part of a phonebook entry

#define	SIM_NOT_PRESENT()	(sim_db.present != SIMPRESENT_INSERTED )


#define SIM_MWI_NUM_UNKNOWN	0xFF

typedef enum
{
	SIMSTATE_IDLE,						// normal running state
	SIMSTATE_ACTIVE,					// SIM active, doing an access	
	SIMSTATE_SETTING_OPER_STATE,		// setting SIM operation state
    SIMSTATE_SETTING_BDN_STATE,         // Enable/disable BDN
	SIMSTATE_VERIFYING_CHV,				// verifying CHV
	SIMSTATE_SETTING_CHV,				// setting (changing) CHV
	SIMSTATE_ENABLING_CHV,				// enabling CHV
	SIMSTATE_UNBLOCKING_CHV,			// unblocking CHV
	SIMSTATE_READING_SMS_MESG,			// reading SMSPP Message
	SIMSTATE_WRITING_SMS_MESG,			// writing SMSPP Message
	SIMSTATE_READING_SMS_PARAM,			// reading SMSPP Parameter
	SIMSTATE_WRITING_SMS_PARAM,			// writing SMSPP Parameter

	SIMSTATE_GETTING_TP_MR,             // getting new SMS reference number

	SIMSTATE_GETTING_PBK_STATUS,		// getting Phonebook status
	SIMSTATE_READING_PBK,				// reading Phonebook record
	SIMSTATE_WRITING_PBK,				// writing Phonebook record
	SIMSTATE_READING_ACM,				// reading ACM
	SIMSTATE_WRITING_ACM,				// writing ACM
	SIMSTATE_INCREASING_ACM,			// increaseing ACM
	SIMSTATE_READING_ACMmax,			// reading Max ACM
	SIMSTATE_WRITING_ACMmax,			// writing Max ACM
	SIMSTATE_READING_PUCT,				// reading PUCT
	SIMSTATE_WRITING_PUCT,				// writing PUCT
	SIMSTATE_WRITING_SMS_MESG_STATUS,	// writing SMSPP Mesg Status
	SIMSTATE_GETTING_GENERIC_DFILE_INFO,	// getting DFile/MFile Info through generic SIM Access
	SIMSTATE_GETTING_GENERIC_EFILE_INFO,	// getting EFile Info through generic SIM Access
	SIMSTATE_SENDING_GENERIC_CMD,		// Sending generic command to SIM/USIM
	SIMSTATE_GETTING_SST_STATUS,		// getting SST, request from other subsystem (environment)
	SIMSTATE_READING_ECC_FILE_SIZE,		// reading the Emergency Call Code (ECC) File Size
	SIMSTATE_READING_ECC_VALUE,			// reading the ECC values
	SIMSTATE_READING_GID_FILE_SIZE,		// reading the GID(Group Identifier) File Size
	SIMSTATE_READING_GID_VALUE,			// reading the GID values
	SIMSTATE_READING_SVC_PROV_NAME,		// reading the Service Provider Name
	SIMSTATE_RESTRICTED_STATUS_ACCESS,	// SIM restricted access for status
	SIMSTATE_RESTRICTED_DATA_ACCESS,	// SIM restricted access for data
	SIMSTATE_API_READING_EFILE_DATA,	// Reading EFile data through SIM API
	SIMSTATE_API_UPDATING_EFILE_DATA,	// Updating EFile data through SIM API
	SIMSTATE_API_SEEKING_RECORD,		// Searching record through SIM API
	SIMSTATE_API_GETTING_REMAINING_PIN_ATTEMPT,	// Getting the remaining PIN1/PIN2/PUK1/PUK2 attempts

	SIMSTATE_SETTING_EST,				// Activating/Deactivating service in EF-EST in USIM
	SIMSTATE_UPDATING_ONE_APN,			// Updating one APN name in EF-ACL in USIM
	SIMSTATE_DELETING_ALL_APN,			// Deleting all APN name's in EF-ACL in USIM

	SIMSTATE_GETTING_SOCKET_ID,			// Getting Socket ID for non-USIM application
	SIMSTATE_SELECTING_APPLI,			// Selecting a non-USIM application
	SIMSTATE_DEACTIVATING_APPLI,		// Deactivating a non-USIM application
	SIMSTATE_CLOSING_SOCKET,			// Close the channel associated with a socket ID
    SIMSTATE_SELECT_FILE_SEND_APDU,     // Select file and send APDU
    SIMSTATE_POWER_ON_OFF_CARD          // SIM Card is being powered On/off
} SIMState_t;							// SIM MI States


/* Number of bytes in Act-HPLMN EF */
#define SIM_ACT_HPLMN_DATA_LEN 3

typedef struct
{
	/* Elements of one-byte or one-byte array, including enum type */
	SIMState_t			sim_state;		// state of current SIM state machine
	SIMState_t			prev_sim_state;	// state of previous SIM state machine
	SIMPresent_t		present;		// is SIM present	
	SIMOperState_t		oper_state;		// No Oper. or Restricted or Unrestricted
    SIMBdnOperState_t   bdn_oper_state; // No Oper. or Restricted or Unrestricted for BDN
	SIMType_t			type;			// SIM type: normal SIM, type approval SIM, or other types defined in EF-AD
	SIM_APPL_TYPE_t		appl_type;		// Application type: is SIM runnning 2G or 3G application
	CHVStatus_t			chv1_status;	// Whether CHV1 protection is active
	CHVStatus_t			chv2_status;	// Whether CHV2 is supported	
	GID_t				curr_gid;
	SIMPBK_ID_t			pbk_id;			// used for callback, currently used
										// for getting pbk state/status
	SST_t				sst;			// SIM Service Table
	SIMService_t		check_service;		// current SIM Service to check
	SIMAccess_t 		prior_req_status;	// holds the prior request status, used
											// updating the SST from another request
	SIM_PIN_Status_t	curr_pin_status;	// Current SIM PIN status

	SIMSMSMesgStatus_t	sms_updated_status;	// SMS status to be written 

	SIM_LANGUAGE_INFO_t language_info;		// Preferred language in SIM (EF-ELP & EF-LP for 2G SIM; EF-LI & EF-PL for 3G USIM)

	ICCID_BCD_t			iccid;				// SIM Chip ID

	ClientInfo_t		clientInfo;			// Client Info passed to the SIM API functions

	UInt8				socket_id;			// Socket ID for SIM/USIM application
	UInt8				cphs_ons_len;		// Number of bytes in "cphs_ons"
	UInt8				cphs_ons[MAX_CPHS_ONS_LEN];			// CPHS Operator Name String, stored in raw SIM format
	UInt8				cphs_onss_len;						// Number of bytes in "cphs_onss"
	UInt8				cphs_onss[MAX_CPHS_ONSS_LEN];		// CPHS Operator Name String Shortform, stored in raw SIM format

	UInt8				act_hplmn[SIM_ACT_HPLMN_DATA_LEN];	// Act-HPLMN EF data 

	/* Elements of UInt8 type */
	UInt8				phase;					// SIM phase
	UInt8				mnc_digit_len;			// MNC digit length 
	UInt8				max_adn_rec_no;
	UInt8				max_fdn_rec_no;
	UInt8				max_lnd_rec_no;
	UInt8				max_msisdn_rec_no;
    UInt8				max_bdn_rec_no;
	UInt8				adn_alpha_sz;
	UInt8				fdn_alpha_sz;
	UInt8				lnd_alpha_sz;
	UInt8				sms_param_rec_no;	// Number of SMS Service Parameter records in SIM
	UInt8				msisdn_alpha_sz;
    UInt8				bdn_alpha_sz;
	UInt8				smsparam_alpha_id_sz;
	UInt8				sms_total_number;	// Total number of SMS's in SIM	
	UInt8				data_len;	
	UInt8				sms_work_idx;
	UInt8				clientID;			///< Client ID passed to the SIM API functions

	UInt8				pnn_rec_num;		// Number of records in EF-PNN.
	UInt8				pnn_rec_len;		// Record length of EF-PNN.

	UInt8				opl_rec_num;		// Number of records in EF-OPL.

	UInt8				file_path_len;		// Number of elements in "file_path" array

	UInt8				general_rec_num;	// Record Number for general use

	UInt8				general_data[4];	// 4-byte data buffer for general purpose

	UInt8				gid1_len;			// Number of GID1 data bytes
	UInt8				gid2_len;			// Number of GID2 data bytes

	/* Two-byte elements */
	UInt16				pbk_rec_no;			// storage the last attempt at reading or writing to a phonebook record
	UInt16				pbk_last_rec_no;	// Index of the last record to read

	UInt16				setup_event_list;	// STK event enable flags stored as bitmaps

	UInt16				file_path[MAX_SIM_FILE_PATH_LEN];	/* Complete path of the SIM file */

	/* Elements of Structure type */	
	IMSI_t				imsi;
	GID_DIGIT_t			gid1;
	GID_DIGIT_t			gid2;

	/* Elements of pointer type */
	APDU_t				*p_apdu;
	APDU_t				*p_apdu_rsp;
	SIMSMSMesg_t		*p_sms_mesg;
	CallbackFunc_t*		sim_access_cb_ptr;	/* Call back function for all SIM access */
	void				*additional_pbk_cb;	// Additional call back function for phonebook
	SIMPBK_Entry_t		*pbk_entry;			// Buffer to keep phonebook data in a phonebook write
	SIMSMSMesgStatus_t	*sms_status_list;	// List to store the status of each SMS in SIM
	SIMSMSParam_t		*sms_param_list;	// List to store SMS Service Parameter records
	SIMSMSParam_t		*sms_param_buffer;	// Buffer to keep SMS parameter in a SMS Service Parameter update.

	UInt8				*pnn_data;			// Dynamically allocated buffer to store EF-PNN data in raw format
	UInt8				*opl_data;			// Dynamically allocated buffer to store EF-OPL data in raw format

	UInt8				*general_data_buf;	// Dynamically allocated data buffer for general use

	SIMAccess_t			*sim_access_result_ptr;		// Pointer to SIM access result;
	SIMAccess_t			*sim_gen_apdu_result_ptr;	// Pointer to SIM Generic APDU access result;
	APDU_t				*p_gen_apdu_rsp;			// Pointer to the buffer to store SIM resposne for Generic APDU access
	UInt16				*file_size_ptr;				// Pointer to return file size
	UInt8				*rec_len_ptr;				// Pointer to return record length
	UInt8				*file_data_ptr;				// Pointer to return file data
        
    CallbackFunc_t*     genericApduAtrCback;		// Generic APDU event callback for ATR response
    CallbackFunc_t*     genericApduRspCback;		// Generic APDU Response callback
    UInt8               genericApduClientID;		// Generic APDU Transfer Client ID
    CHV1Info_t          chv1Info;					// Stored PIN1 information for automatic re-entry

    SIMLockCbackFunc_t*     simlockCback;           // SIM lock callback

	/* Elements of BitField type */
	BitField			is_dial_no_info_avail: 1;	// flag to indicate if dial number
													// is available or requires SIM access to get it	
	BitField			is_reading_sst: 1;
	BitField			is_sst_avail: 1;
	BitField			is_invalid_sim: 1;			// TRUE if inserted SIM/USIM is invalid for normal operation, e.g. IMSI does not exist. 
	BitField			is_chv1_required: 1;
	BitField			is_chv1_pin_blocked: 1;
	BitField			is_chv1_puk_blocked: 1;
	BitField			is_chv2_pin_blocked: 1;
	BitField			is_chv2_puk_blocked: 1;
	BitField			is_chv1_verified: 1;
	BitField			is_chv2_verified: 1;
	BitField			is_background_updating_sst: 1; 
	BitField			is_reading_simlock_data: 1; /* Reading simlock data: imsi; GID1; GID2 */
	BitField			is_cached_data_ready: 1;	/* Cached SIM data is ready to be read */
	BitField			is_eons_data_ready: 1;		/* TRUE if EF-PNN and EF-OPL has been read into SRAM */
	BitField			is_arr_data_ready: 1;		/* EF-ARR data is ready */
	BitField            is_fatal_error_sent: 1;     /* SIM fatal error has been sent */
	BitField			is_first_time_powerup: 1;
	BitField			is_refresh_active: 1;		/* TRUE if SIM Refresh operation is active */
	BitField			is_homezone_refreshed: 1;	/* TRUE if SIM Refresh operation includes HomeZone data */
	BitField			is_sms_cap_exceeded: 1;		/* TRUE if SMS Capacity Exceeded is set in EF-SMSS */

	BitField			is_homezone_data_cached: 1;	/* TRUE if HomeZone data has been cached in case where HomeZone data exists */

	/* Call Forward Unconditional flags read from Call Forward Flags 
 	 * EFile (6F13, defined in CPHS spec).
 	 */ 	
	BitField	is_call_forward_flags_efile_supported: 1;		// True if the efile exists in SIM */
	BitField	is_call_forward_flags_efile_one_byte_length: 1; // True if data/fax flags not supporoted: just L1/L2
	BitField	is_call_forward_unconditional_l1_on: 1;
	BitField	is_call_forward_unconditional_l2_on: 1;
	BitField	is_call_forward_unconditional_data_on: 1;
	BitField	is_call_forward_unconditional_fax_on: 1;

	BitField	is_generic_apdu_mode: 1;	// True if the SIM card is powered on for Generic APDU mode (e.g. BT-SAP) where SIM access on UE is disabled

} SIMDb_t;


/* Message waiting information in EF-MWI (2G SIM) or EF-MWIS (3G USIM) */
typedef struct
{
	Boolean line1_ind;
	Boolean line2_ind;
	Boolean fax_ind;
	Boolean data_ind;
	UInt8 line1_count;	/* NA for 2G SIM */
	UInt8 line2_count;	/* NA for 2G SIM */
	UInt8 fax_count;	/* NA for 2G SIM */
	UInt8 data_count;	/* NA for 2G SIM */
} SIM_MWI_t;


//---------------------------------------------------------------
// Data Declaration
//---------------------------------------------------------------
extern Semaphore_t	sim_semaphore;		// Semaphore to protect SIM access
extern SIMDb_t		sim_db;				// SIM variable database


//-------------------------------------------------
// Function Prototype
//-------------------------------------------------

//***************************************************************************************
/**
    This function returns whether SIM PIN needs to be entered to unlock the SIM.

	@return TRUE if SIM PIN needs to be entered; FALSE otherwise
**/	
Boolean SIM_IsPINRequired(void);

Boolean SimApi_IsPINRequired(ClientInfo_t* inClientInfoPtr); ///< TRUE if SIM PIN needs to be entered;

//***************************************************************************************
/**
    This function returns whether SIM PIN has been successfully verified.

	@return TRUE if SIM PIN has been successfully verified; FALSE otherwise
**/	
Boolean SIM_IsPINVerified(void);


//***************************************************************************************
/**
    This function returns whether SIM PIN2 has been successfully verified.

	@return TRUE if SIM PIN2 has been successfully verified; FALSE otherwise
**/	
Boolean SIM_IsPIN2Verified(void);


//***************************************************************************************
/**
    This function resets the variables in the SIM database structure. This function is called 
	whenever the SIM is inserted or removed after initial power-on state.
**/	
void resetToSIMKnown(void);


//***************************************************************************************
/**
    This function sends back the response message for a SIM update request based on the 
	current state. 
	
	@param access_status (in) SIM access result
**/	
void simmi_SendSimAccessRsp(SIMAccess_t access_status);


//***************************************************************************************
/**
    This function resets the call back function and the associated SEMAPHRE used for SIM 
	accesss. This function needs to be called after SIM accesss is finished.
**/	
void simmi_ResetSimAccessCb(void);


//***************************************************************************************
/**
    This function sends fatal error related to SIM to ATC/MMI
**/	
void simmi_SendFatalError(SIM_FATAL_ERROR_t fatal_error);

//***************************************************************************************
/**
    If this function returns TRUE, the platform will allow voice/CSD/GPRS calls to be made 
	without SIM's. USIMAP will fabricate the necessary SIM file data if this function 
	returns TRUE even though no SIM is inserted. 

	@return TRUE if we allow voice/CSD/GPRS calls without SIM; FALSE otherwise.
	@note This function should return TRUE only for making test image for FPGA verification 
		  which does not have SIM HW.
**/	
Boolean SIM_IsCallAllowedNoSIM(void);


//***************************************************************************************
/**
    This function posts a message to the SIM_MI task.

	@param sim_msg (in) Pointer to the dynamically allocated SIM message buffer
**/	
void SIM_PostSIMMsg(SIMMsg_t *sim_msg);


//***************************************************************************************
/**
    This function handles the Phone Book information returned from the SIM task. The phonebook
	information should have been stored in sim_db before this function is called.

	@param sim_access (in) SIM access result
**/	
void SIM_HandleDialNumInfo(SIMAccess_t sim_access);


//***************************************************************************************
/**
    This function obtains the SIMLOCK status from the client and updates the status
	in our internal "current SIM PIN status" variable. 
**/	
	void SIM_UpdateSimlockStatus(ClientInfo_t* inClientInfoPtr);


//***************************************************************************************
/**
    This function returns the language information in the SIM. It should be called only 
	if SIM is inserted and after SIM data cacheing is finished.
	
	@return SIM language information structure.
**/	
const SIM_LANGUAGE_INFO_t *SIM_GetLangInfo(void);


//***************************************************************************************
/**
    This function selects the mandatory RUIM UIMID EF to determine whether RUIM SIM card 
	is inserted and updates the internal flag in SIM module. 
**/
void SIM_InitRuimSuppFlag(void);


//***************************************************************************************
/**
    This function updates internal settings about the CHV access based on the CHV status 
	from USIMAP regarding PIN1 or PIN2. 

	@param chv_no (in) Select PIN1 or PIN2
	@param chv_status (in) CHV status from USIMAP
**/
void updateCHVStatus(CHV_t chv_no, CHVStatus_t chv_status);

//***************************************************************************************
/**
    Get the ATR data from SIMIO driver. This function is different from SIM_GetRawAtr() 
	in that it returns the ATR data synchronously.

	@param atr_data (out) Buffer to store the returned ATR data

	@return RESULT_OK if ATR data is returned successfully; SIMACCESS_NO_ACCESS or 
			SIMACCESS_NO_SIM otherwise
**/
SIMAccess_t SIM_GetAtrData(APDU_t *atr_data);

SIMAccess_t SimApi_GetAtrData(ClientInfo_t* inClientInfoPtr, APDU_t *atr_data);

//***************************************************************************************
/**
    This function returns the CPHS file information in EF-CPHS-INFO. 

	@param info_buf (out) Buffer to store the CPHS file info, at least three bytes. 

	@return RESULT_OK if CPHS file info is returned successfully; RESULT_ERROR otherwise
**/
Result_t SIM_GetCphsInfo(UInt8 *info_buf);


//***************************************************************************************
/**
    This function returns SIMLOCK status for a SIMLOCK type.

	@param lockType (in) SIMLOCK type. 

	@return SIMLOCK status
**/
SIM_LOCK_STATUS_t SIM_GetLockInfo(SIM_LOCK_TYPE_t lockType);


//***************************************************************************************
/**
    This function updates the data of a particular Mailbox Dialling Number record in 
	SIM/USIM. If the update is successful, the cached copy of the record is also updated. 
	For 2G SIM, the Mailbox Dialling Numbers are stored in CPHS EF-MBN. For 3G USIM, 
	they are stored in EF-MBDN. A MSG_SIM_EFILE_UPDATE_RSP message will be passed back
	to the passed callback function

	@param clientID (in) Client ID
	@param mwi_type (in) Mailbox Dialling Number type, i.e. voice L1/L2, Fax, Data. 
	@param mbx_data (in) Pointer to the Mailbox Dialling Number record data
	@param sim_access_cb (in) Callback function to receive the MSG_SIM_EFILE_UPDATE_RSP message

	@return TRUE if the update request is sent to SIM/USIM; FALSE if the request is rejected, e.g. the record
			to be updated does not exist in SIM/USIM.  
**/
Boolean SIM_UpdateMbxNum(UInt8 clientID, SIM_MWI_TYPE_t mwi_type, const UInt8 *mbx_data, CallbackFunc_t *sim_access_cb);


//***************************************************************************************
/**
    This function returns the number of Mailbox Dialling Number records in SIM. For 2G SIM, 
	the Mailbox Dialling Numbers are stored in CPHS EF-MBN. For 3G USIM, they are stored in 
	EF-MBDN. 

	@return The number of Mailbox Dialling Number records. 
**/
UInt8 SIM_GetNumOfMbxNum(void);


//***************************************************************************************
/**
    This function returns the length of Mailbox Dialling Number records in SIM. For 2G SIM, 
	the Mailbox Dialling Numbers are stored in CPHS EF-MBN. For 3G USIM, they are stored in 
	EF-MBDN. 

	@return The length of Mailbox Dialling Number records. 
**/
UInt8 SIM_GetMbxRecLen(void);



// SIM_MWI_TYPE_t mwi_type - Type of Mailbox Dialling Number. 
// UInt8 *data_len - Variable to store the record length returned. 
// UInt8 *mbxExt - extended mailbox number, if not exist, NULL is returned.
//
// Return: pointer to the cached Mailbox Dialling Number record data requested if
//		   the requested record exists; NULL otherwise. 

//***************************************************************************************
/**
    This function returns the data of a particular Mailbox Dialling Number record in SIM/USIM. 
	For 2G SIM, the Mailbox Dialling Numbers are stored in CPHS EF-MBN. For 3G USIM, they 
	are stored in EF-MBDN. 

	@param mwi_type (in) Mailbox Dialling Number type, i.e. voice L1/L2, Fax, Data. 
	@param data_len (out) Variable to store the record length returned
	@param mbxExt_ptr (out) Pointer to store the EF-EXT1 (2G SIM) or EF-EXT6 (3G USIM) data. Buffer must have at least 10 bytes. 

	@return Pointer to the Mailbox Dialling Number data.
**/
const UInt8* SIM_GetMbxNum(SIM_MWI_TYPE_t mwi_type, UInt8 *data_len, UInt8* mbxExt_ptr);


//***************************************************************************************
/**
    This function returns whether both phone (according to Terminal Profile in SysParm)
	and SIM support STK call control feature.
	
	@return TRUE if both phone (according to Terminal Profile in SysParm) and SIM 
			support STK call control feature; FALSE otherwise
**/
Boolean SIM_IsStkCallCtrEnabled(void);


//***************************************************************************************
/**
    This function returns whether both phone (according to Terminal Profile in SysParm)
	and SIM support STK MO SMS control feature.
	
	@return TRUE if both phone (according to Terminal Profile in SysParm) and SIM 
			support STK MO SMS control feature; FALSE otherwise
**/
Boolean SIM_IsMoSmsCallCtrEnabled(void);


//***************************************************************************************
/**
    This function updates the SIM files cached in SIM module that has been changed in SIM
	refresh.
	
	@param file_id_list (in) File ID array of the File Change List in the Refresh command
	@param file_change_noti (in) TRUE if the refresh request is "file change notification only"; FALSE otherwise
	@param appli_type (in) Application type for the refresh.
**/
void SIM_RefreshCachedData(const REFRESH_FILE_LIST_t *file_id_list, Boolean file_change_noti, USIM_APPLICATION_TYPE appli_type);


//***************************************************************************************
/**
    This function sets the Call Forward Unconditional flags stored in CPHS Call Forward 
	Flags EF (for 2G SIM) or EF-CFIS (for 3G USIM).

	@param flag (in) Call Forwarding status
**/
void SIM_SetCallForwardUnconditionalFlag(SIM_CALL_FORWARD_UNCONDITIONAL_FLAG_t flag);


//***************************************************************************************
/**
    This function gets the EONS (Enhanced Operator Name String) PLMN Name based upon 
	MCC-MNC-LAC tuple according to the EF-PNN & EF-OPL data. If no match is found and 
	the passed network is the home PLMN, the function tries to get the PLMN names from 
	CPHS EF-ONS and EF-ONSS.  

	@param mcc (in) MCC of reigstered PLMN, e.g. 0x13F0 for AT&T in Sunnyvale, CA
	@param mnc (in) MNC of registered PLMN, e.g. 0x71 for AT&T in Sunnvayle, CA
	@param lac (in) Location Area Code of registered PLMN

	@param long_name (out) Structure to store the returned long PLMN name
	@param short_name (out) Structure to store the returned short PLMN name
**/
void SIM_GetEonsPlmnName( UInt16 mcc, UInt8 mnc, UInt16 lac,
						  PLMN_NAME_t *long_name, PLMN_NAME_t *short_name );


//***************************************************************************************
/**
    This function returns the pointer to the EF-SST data in 2G SIM.

	@return Pointer to the EF-SST data in 2G SIM
**/
const UInt8* SIM_GetSst(void);


//***************************************************************************************
/**
    This function returns whether the passed file ID is for a DF or MF. 

	@param file_id (in) SIM file ID

	@return TRUE if the passed file ID is for a DF or MF; FALSE otherwise
**/
Boolean SIM_IsDfMfFileId(UInt16 file_id);


//***************************************************************************************
/**
    This function returns the Message Waiting Indication Status in SIM. For 2G SIM, it 
	is read from CPHS EF-MWI file. For 3G USIM, it is read from EF-MWIS. If the relevant 
	SIM/USIM file does not exist, then the indicator defaults to "OFF" unless we have 
	received an "ON" indication from the network. 

	@param sim_mwi (out) Strcture to store the returned Message Waiting Indication Status
**/
void SIM_GetMwiInd(SIM_MWI_t *sim_mwi);


//***************************************************************************************
/**
    This function updates the Message Waiting Indication Status in SIM. For 2G SIM, it 
	is updated in CPHS EF-MWI file. For 3G USIM, it is updated in EF-MWIS. If the relevant 
	SIM/USIM file does not exist, only the local cached copy of Message Waiting Indication 
	status is updated. 
	
	@param mwi_type (in) Mailbox Dialling Number type, i.e. voice L1/L2, Fax, Data
	@param ind_on (in) TRUE if message waiting is turned on
	@param mwi_count (in) How many messages are waiting
**/
void SIM_UpdateMwiInd(SIM_MWI_TYPE_t mwi_type, Boolean ind_on, UInt8 mwi_count);


//***************************************************************************************
/**
    This function reads cached SIM file data into RAM. It should be called during the phone 
	powerup procedures.
**/
void SIM_InitCachedSimData(void);


//***************************************************************************************
/**
    This function returns the SIM ICCID in raw format. 

	@return Pointer to the SIM ICCID data in raw format.
**/
UInt8* SIM_GetIccidRaw(void);


//***************************************************************************************
/**
	This function returns the number of SMS records in SIM.

	@param sms_number (in) Variable to store the returned SMS record number
	
	@return RESULT_OK if SMS record number is successfully returned; RESULT_ERROR otherwise
**/	
Result_t SIM_GetSmsTotalNumber(UInt8 *sms_number);


//***************************************************************************************
/**
	This function returns the status of a SMS message.

	@param		rec_no (in) 0-based record number
	@param		*sms_status (out) Variable to store the returned SMS status

	@return		RESULT_OK if SMS status is successfully returned; RESULT_ERROR otherwise
**/	
Result_t SIM_GetSmsStatus(UInt8 rec_no, SIMSMSMesgStatus_t *sms_status);


//***************************************************************************************
/**
	This function returns TRUE if both the SIM and our ME support EONS feature. 

	@return	TRUE if EONS is supported; FALSE otherwise
**/	
Boolean SIM_IsEonsEnabled(void);


//***************************************************************************************
/**
	This function sends the request to USIMAP to read one or multiple phonebook records. 
	one or more MSG_SIM_PBK_DATA_RSP messages will be passed back to the 
	callback function.

	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		id (in) Phonebook type
	@param		index (in) 0-based index of the first record to read
	@param		end_index (in) 0-based index of the last record to read
	@param		sim_access_cb (in) Callback function
	@param		additional_pbk_cb (in) Additional callback function to be returned in response. 

	@return		RESULT_OK
**/	
Result_t SIM_SendReadPbkReq(ClientInfo_t* inClientInfoPtr,	SIMPBK_ID_t id,	UInt16 index, UInt16 end_index, CallbackFunc_t* sim_access_cb, void *additional_pbk_cb);


//***************************************************************************************
/**
	This function sends the request to USIMAP to update one phonebook record. A 
	MSG_SIM_PBK_WRITE_RSP messages will be passed back to the callback function.

	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		id (in) Phonebook type
	@param		index (in) 0-based record index
	@param		buffer (in) Phonebook data
	@param		sim_access_cb (in) Callback function
	@param		additional_pbk_cb (in) Additional callback function to be returned in response. 		

	@return		RESULT_OK
**/	
Result_t SIM_SendWritePbkReq(ClientInfo_t* inClientInfoPtr, SIMPBK_ID_t id, UInt16 index, SIMPBK_Entry_t *buffer, CallbackFunc_t* sim_access_cb, void *additional_pbk_cb);

//***************************************************************************************
/**
	This function posts a SIMMSG_SIMI_RESET_IND message to SIM task to re-initialize SIM data
**/	
void SIM_PostResetReqMsg(void);


//***************************************************************************************
/**
	This function perform initialization of the SIM module
**/	
void SIM_Init( void );	


//***************************************************************************************
/**
	This function runs the SIM task
**/	
void SIM_Run( void );


//***************************************************************************************
/**
	This function returns whether a phonebook can be accessed, i.e. entries can be read
	and written.

	@param		id (in) Phonebook type

	@return		TRUE if phonebook can be accessed; FALSE otherwise.
**/	
Boolean SIM_IsPbkAccessAllowed(SIMPBK_ID_t id);

Boolean SimApi_IsPbkAccessAllowed(ClientInfo_t* inClientInfoPtr, SIMPBK_ID_t id);

//***************************************************************************************
/**
	This function sends request to USIMAP to enable the SIM presence test so that we poll
	the SIM by sending Status command. It shall be called once a call is established. 

	@return		RESULT_OK if success; RESULT_ERROR otherwise
**/	
Result_t SIM_EnableSIMPresenceTest(void);		


//***************************************************************************************
/**
	This function sends request to USIMAP to disable the SIM presence test.

	@return		RESULT_OK if success; RESULT_ERROR otherwise
**/	
Result_t SIM_DisableSIMPresenceTest(void);	


//***************************************************************************************
/**
	This function updates the sim_db.curr_pin_status variable according
	to the current SIM status. 

	@param		set_pin2 (in) TRUE if the setting PIN2 status; FALSE otherwise.
	@param		power_up (in) TRUE if the status is being updated during powerup; FALSE otherwise.
**/	
void SIM_UpdatePinStatus(Boolean set_pin2, Boolean power_up);


//***************************************************************************************
/**
	This function updates the sim_db.curr_pin_status variable with the passed status.

	@param		pin_status (in) SIM PIN status
**/	
void SIM_SetDedicatedPinStatus(SIM_PIN_Status_t pin_status);


//***************************************************************************************
/**
	This function returns class byte for the passed command byte based on whether a 
	2G SIM or 3G USIM is inserted.

	@param		cmd (in) SIM command byte
	
	@return		Class byte for the SIM command
**/	
UInt8 SIM_GetClassByte(APDUCmd_t cmd); 


Boolean SIM_IsPnnFirstRec(UInt16 mcc, UInt8 mnc, UInt16 lac);


//***************************************************************************************
/**
	This function returns whether the EONS data in SIM has been cached.
	
	@return		TRUE if EONS data has been cached; FALSE otherwise
**/	
Boolean SIM_IsEonsDataReady(void);


void SIM_HandleSMSWriteStatus(SIMAccess_t sim_access, UInt16 rec_no);


//***************************************************************************************
/**
	This function checks the validity of mandatory EF-IMSI, EF-EST (for 2G SIM) and EF-UST (for 3G SIM). 
	If any fatal error is found, the fatal error is posted to ATC/MMI.
**/	
void SIM_CheckImsiEstUst(void);

#endif  // _SIM_MI_H_

