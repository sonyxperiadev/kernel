//*********************************************************************
//
//	Copyright ?2008 Broadcom Corporation
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
*   @file   sms_def.h
*
*   @brief  This file defines structures for sms configuration used by both CAPI and CAPI2
*
****************************************************************************/
#ifndef _SMS_DEF_H_
#define _SMS_DEF_H_

//-------------------------------------------------
// Constant Definitions
//-------------------------------------------------

// GSM 3.40 <-> TE decoding/encoding utilities
//
// MTI supported values
//
// 3.40 values

#define STATUS_REPORT_340  2
#define DELIVER_REPORT_340 0
#define SUBMIT_340         1
#define COMMAND_340        2

//-------------------------------------------------
// Struct Type Definitions
//-------------------------------------------------

// Utilities
#define MASKL1  0x01      /*          1  */
#define MASKL2  0x03      /*         11  */
#define MASKL3  0x07      /*        111  */
#define MASKL4  0x0f      /*       1111  */
#define MASKL5  0x1f      /*      11111  */
#define MASKL6  0x3f      /*     111111  */
#define MASKL7  0x7f      /*    1111111  */
#define MASKL8  0xff      /*   11111111  */

#define MASK_SIM_ACCESS   0x01F	///< SIM Access Mask

#define SMS_GET8_BITS(src,pos,mask)   (((src)>>pos) & (mask))
#define SMS_SET8_BITS(src,pos,mask)   (((src)&mask) << pos)

#define SMS_GET_RDIGIT(byte)      (byte & 0x0F)
#define SMS_GET_LDIGIT(byte)      ((byte >> 4) & 0x0F)

#define SMS_SET_RDIGIT(byte)      (byte & 0x0F)
#define SMS_SET_LDIGIT(byte)      ((byte & 0x0F) << 4)

#define SMS_SWAP(byte)      ( SMS_SET_LDIGIT(byte) | SMS_SET_RDIGIT(byte>>4) )



// Access to AI bit fields

// Addresses
//----------
#define SMS_GET_TON(src)    SMS_GET8_BITS(src,4,MASKL3) ///< see REC. 4.08
#define SMS_SET_TON(src)    SMS_SET8_BITS(src,4,MASKL3) ///< see REC. 4.08
#define SMS_GET_NPI(src)    SMS_GET8_BITS(src,0,MASKL4) ///< see REC. 4.08
#define SMS_SET_NPI(src)    SMS_SET8_BITS(src,0,MASKL4) ///< see REC. 4.08

//-----
// <fo>
//-----
#define SMS_GET_MTI(fo)  SMS_GET8_BITS(fo,0,MASKL2) ///< Message type ind  BI-DIR
#define SMS_GET_RD(fo)   SMS_GET8_BITS(fo,2,MASKL1) ///< Reject duplicate  MO
#define SMS_GET_MMS(fo)  SMS_GET8_BITS(fo,2,MASKL1) ///< More message to send
#define SMS_GET_VPF(fo)  SMS_GET8_BITS(fo,3,MASKL2) ///< VP format         MO
#define SMS_GET_SRR(fo)  SMS_GET8_BITS(fo,5,MASKL1) ///< Status report req MO
#define SMS_GET_SRQ(fo)  SMS_GET8_BITS(fo,5,MASKL1) ///< SRQ
#define SMS_GET_SRI(fo)  SMS_GET8_BITS(fo,5,MASKL1) ///< Status Report Indicator
#define SMS_GET_UDHI(fo) SMS_GET8_BITS(fo,6,MASKL1) ///< User data hdr ind BI-DIR
#define SMS_GET_RP(fo)   SMS_GET8_BITS(fo,7,MASKL1) ///< Reply path        BI-DIR

#define SMS_SET_MTI(val)    SMS_SET8_BITS(val,0,MASKL2) ///< Message type ind
#define SMS_SET_RD(val)     SMS_SET8_BITS(val,2,MASKL1) ///< Reject duplicate
#define SMS_SET_MMS(val)    SMS_SET8_BITS(val,2,MASKL1) ///< More msg to send
#define SMS_SET_VPF(val)    SMS_SET8_BITS(val,3,MASKL2) ///< VP format
#define SMS_SET_SRR(val)    SMS_SET8_BITS(val,5,MASKL1) ///< SRR
#define SMS_SET_SRQ(val)    SMS_SET8_BITS(val,5,MASKL1) ///< SRQ
#define SMS_SET_SRI(val)    SMS_SET8_BITS(val,5,MASKL1) ///< SRI
#define SMS_SET_UDHI(val)   SMS_SET8_BITS(val,6,MASKL1) ///< User data hdr ind
#define SMS_SET_RP(val)     SMS_SET8_BITS(val,7,MASKL1) ///< Reply path

#define SMS_GET_CODING_GRP(dcs) SMS_GET8_BITS(dcs, 4, MASKL4) ///<  coding group
#define SMS_PI_PID   1
#define SMS_PI_DCS   2
#define SMS_PI_UDL   4

#define USE_DEFAULT_SCA_NUMBER 0xFF	///< Use Default Service Center Address

#define SMS_UNKNOWN_TOA					0

#define SMS_STORE_NEXT_FREE_REC			0xFFFF

#define SMS_MAX_DATA_LENGTH				140		///< See GSM 03.38
#define SMS_MAX_PDU_STRING_LENGTH		SMSMESG_DATA_SZ
#define SMS_MAX_ADDRESS_SIZE_IN_OCTET	12
#define SMS_DATA_LENGTH_8BIT_APHABET	140		///< this should be the same as defined in mns_op5.c


#define SMS_TYPE_0						64		///< 01000000

#define SMS_TYPE_REPLACE_1				65		///< 01000001
#define SMS_TYPE_REPLACE_2				66		///< 01000010
#define SMS_TYPE_REPLACE_3				67		///< 01000011
#define SMS_TYPE_REPLACE_4				68		///< 01000100
#define SMS_TYPE_REPLACE_5				69		///< 01000101
#define SMS_TYPE_REPLACE_6				70		///< 01000110
#define SMS_TYPE_REPLACE_7				71		///< 01000111

#define SMS_STORE_REPLACE_REC			0xFFFE

#define	CB_HDR_PER_PAGE_SZ	7					///< CB Header only

#define	CB_PAGE_SZ			(CB_HDR_PER_PAGE_SZ+CB_DATA_PER_PAGE_SZ)

#define MAX_NUMBER_OF_VMSC NUM_OF_MWI_TYPE

#define SMS_DATA_LENGTH_7BIT_APHABET	160		///< this should be the same as defined in mns_op5.c
#define SIM_SMS_REC_LEN		176					///< Record size of EF-SMS
#define	SMSMESG_DATA_SZ		(SIM_SMS_REC_LEN - 1)
#define	CB_DATA_PER_PAGE_SZ	82					///< CB Data only

#define DELIVER_340        0


#define	SMS_MAX_DIGITS					40		///< Since this symbol MAX_DIGITS is used in SMS as well
												///< I created another symbol SMS_MAX_DIGITS with the same
												///< length in order not to break anything ...
												///< JAY MIGHT BE ABLE TO CONFIRM WHETHER IT IS SAFE TO
												///< USE THE SAME LENGTH OR NOT
#define NUM_SMS_PROFS 2	///< Number of Profiles. Can be in the range 0-255 but only allow 2 to save memory.
#define SUBMIT_REPORT_340  1

#define SMSPARAM_SCA_DIGIT_SZ ((SMSPARAM_SCA_SZ - 2) << 1) /* Minus 2 to exclude length and type of number bytes */


#define MS_MN_MTI_DELIVER_IND 0
#define MS_MN_MTI_DELIVER_RESP 1
#define MS_MN_MTI_STATUS_REPORT_IND 2
#define MS_MN_MTI_STATUS_REPORT_RESP 3
#define MS_MN_MTI_COMMAND_REQ 4
#define MS_MN_MTI_COMMAND_RESP 5
#define MS_MN_MTI_SUBMIT_REQ 6
#define MS_MN_MTI_SUBMIT_RESP 7
#define MS_MN_MTI_MEMORY_AVAILABLE 8
#define MS_MN_SMS_SEND_PDU_REQ 9
#define MS_MN_MTI_PP_DOWNLOAD_IND 10
#define MS_MN_MTI_RESERVED 11


typedef enum
{
    MS_MN_SMS_UNASSIGNED_NUMBER = 1,
    MS_MN_SMS_OP_DETERMINED_BARRING = 8,
    MS_MN_SMS_CALL_BARRED = 10,
    MS_MN_SMS_CP_NETWORK_FAILURE = 17,
    MS_MN_SMS_TRANSFER_REJECTED = 21,
    MS_MN_SMS_MEMORY_CAPACITY_EXCEEDED = 22,
    MS_MN_SMS_DEST_OUT_OF_SERVICE = 27,
    MS_MN_SMS_UNIDENTIFIED_SUBSCRIBER = 28,
    MS_MN_SMS_FACILITY_REJECTED = 29,
    MS_MN_SMS_UNKNOWN_SUBSCRIBER = 30,
    MS_MN_SMS_NETWORK_OUT_OF_ORDER = 38,
    MS_MN_SMS_TEMPORARY_FAILURE = 41,
    MS_MN_SMS_CONGESTION = 42,
    MS_MN_SMS_RESOURCES_UNAVAILABLE = 47,
    MS_MN_SMS_FACILITY_NOT_SUBSCRIBED = 50,
    MS_MN_SMS_REQ_FACILTY_NON_IMPL = 69,
    MS_MN_SMS_INVALID_REFERENCE_VALUE = 81,
    MS_MN_SMS_SEMANT_INCORRECT_MSG = 95,
    MS_MN_SMS_INVALID_MANDATORY_INFO = 96,
    MS_MN_SMS_MSG_TYPE_NON_EXISTENT = 97,
    MS_MN_SMS_MSG_NOT_COMPATIBLE = 98,
    MS_MN_SMS_IE_NON_EXISTENT = 99,
    MS_MN_SMS_PROTOCOL_ERROR = 111,
    MS_MN_SMS_INTERWORKING = 127,

    MS_TP_FCS_NO_ERROR = 0x00,
    MS_TP_FCS_TELEMATIC_NOT_SUPPORTED = 0x80,
    MS_TP_FCS_SM_TYPE_0_NOT_SUPPORTED = 0x81,
    MS_TP_FCS_CANNOT_REPLACE_SM = 0x82,
    MS_TP_FCS_UNSPEC_TP_PID_ERROR = 0x8F,
    MS_TP_FCS_DCS_NOT_SUPPORTED = 0x90,
    MS_TP_FCS_MSG_CLASS_NOT_SUPPORTED = 0x91,
    MS_TP_FCS_UNSPEC_TP_DCS_ERROR = 0x9F,
    MS_TP_FCS_CMD_NOT_ACTIONED = 0xA0,
    MS_TP_FCS_CMD_NOT_SUPPORTED = 0xA1,
    MS_TP_FCS_UNSPEC_TP_CMD_ERROR = 0xAF,
    MS_TP_FCS_TPDU_NOT_SUPPORTED = 0xB0,
    MS_TP_FCS_SC_BUSY = 0xC0,
    MS_TP_FCS_NO_SC_SUBSCRIPTION = 0xC1,
    MS_TP_FCS_SC_SYSTEM_FAILURE = 0xC2,
    MS_TP_FCS_INVALID_SME_ADDRESS = 0xC3,
    MS_TP_FCS_DEST_SME_BARRED = 0xC4,
    MS_TP_FCS_SM_REJ_DUPL_SM = 0xC5,
    MS_TP_FCS_SIM_SMS_STORE_FULL = 0xD0,
    MS_TP_FCS_NO_SMS_ON_SIM = 0xD1,
    MS_TP_FCS_ERROR_IN_MS = 0xD2,
    MS_TP_FCS_MEM_CAP_EXCEEDED = 0xD3,
    MS_TP_FCS_TOOLKIT_BUSY = 0xD4,
    MS_TP_FCS_DATADOWNLOAD_ERROR = 0xD5,
    MS_TP_FCS_APPL_ERR_START = 0xE0,
    MS_TP_FCS_APPL_ERR_STOP = 0xFE,
    MS_TP_FCS_UNSPECIFIED = 0xFF,

    MS_MN_SMS_RP_ACK = 512,
    MS_MN_SMS_TIMER_EXPIRED,
    MS_MN_SMS_FORW_AVAIL_FAILED,
    MS_MN_SMS_FORW_AVAIL_ABORTED,

    MS_MN_TP_INVALID_MTI,
    MS_MN_TP_SRF_NOT_IN_PHASE1,
    MS_MN_TP_RDF_NOT_IN_PHASE1,
    MS_MN_TP_RPF_NOT_IN_PHASE1,
    MS_MN_TP_UDHF_NOT_IN_PHASE1,
    MS_MN_TP_MISSING_VALIDITY_PERIOD,
    MS_MN_TP_INVALID_TIME_STAMP,
    MS_MN_TP_MISSING_DEST_ADDRESS,
    MS_MN_TP_INVALID_DEST_ADDRESS,
    MS_MN_TP_MISSING_SC_ADDRESS,
    MS_MN_TP_INVALID_SC_ADDRESS,
    MS_MN_TP_INVALID_ALPHABET,
    MS_MN_TP_INVALID_USER_DATA_LENGTH,
    MS_MN_TP_MISSING_USER_DATA,
    MS_MN_TP_USER_DATA_TOO_LARGE,
    MS_MN_TP_CMD_REQ_NOT_IN_PHASE1,
    MS_MN_TP_INVALID_DEST_ADDR_SPEC_CMDS,
    MS_MN_TP_INVALID_CMD_DATA_LENGTH,
    MS_MN_TP_MISSING_CMD_DATA,
    MS_MN_TP_INVALID_CMD_DATA_TYPE,
    MS_MN_TP_CREATION_OF_MNR_FAILED,
    MS_MN_TP_CREATION_OF_CMM_FAILED,
    MS_MN_TP_MT_CONNECTION_LOST,
    MS_MN_TP_PENDING_MO_SMS,
    MS_MN_TP_CM_REJ_MSG_NOT_COMPAT,
    MS_MN_SMS_REJ_BY_SMS_CONTROL,


    MS_MN_SMS_NO_ERROR,
    MS_MN_SMS_NO_ERROR_NO_ICON_DISPLAY,

    MS_MN_SMS_FDN_FAILED,
    MS_MN_SMS_BDN_FAILED,

    MS_SMS_PP_UNSPECIFIED


} MS_T_SMS_PP_CAUSE_t;

typedef struct MS_T_MN_CB_MSG_ID_RANGE_s
  {
    unsigned short start_pos;
    unsigned short stop_pos;
  }
MS_T_MN_CB_MSG_ID_RANGE;

#define MS_MN_MAX_CB_MSG_ID_LIST_LENGTH 10



typedef int MS_T_MN_CB_LANGUAGE_LIST_LENGTH;

typedef struct
  {
    Int8 A[13];
  }
MS_T_MN_CB_LANGUAGE_LIST;

typedef struct MS_T_MN_CB_LANGUAGES_s
  {
    MS_T_MN_CB_LANGUAGE_LIST_LENGTH nbr_of_languages;
    MS_T_MN_CB_LANGUAGE_LIST language_list;
  }
MS_T_MN_CB_LANGUAGES;


/// CBMI message ID's configurable by user: we reserve one slot for German O2 HomeZone feature which uses dedicated Message ID of 221. 
#define MAX_MSG_ID_RANGE_LIST_SIZE	(MS_MN_MAX_CB_MSG_ID_LIST_LENGTH - 1)  ///< number of ranges for CB message ID's

#define MAX_LANG_RANGE_LIST_SIZE	MN_MAX_CB_LANGUAGE_LIST_LENGTH		///< number of ranges for languages
#define MAX_MSG_ID_RANGE_WIDTH		MN_SMS_CB_MAX_MSG_ID_RANGE_WIDTH						///< (stop_pos - start_pos)
#define MAX_EF_CBMI_BYTES			(MAX_MSG_ID_RANGE_LIST_SIZE*MAX_MSG_ID_RANGE_WIDTH*2)	///< 2 bytes per record


///	SMS Cause Values
typedef enum
{  
	SMS_NO_ERROR,						///< Successful - No error
	SMS_UNASSIGNED_NUMBER = 1,			///< Number is not assigned
	SMS_OP_DETERMINED_BARRING = 8,		///< Barred by operator
	SMS_CALL_BARRED = 10,				///< Call has been barred
	SMS_CP_NETWORK_FAILURE = 17,		///< Network Failure
	SMS_TRANSFER_REJECTED = 21,			///< Transfer of SMS rejected
	SMS_MEMORY_CAPACITY_EXCEEDED = 22,	///< Memory capacity exceeded,delete old message to make room for new
	SMS_DEST_OUT_OF_SERVICE = 27,		///< Destination MS out of service
	SMS_UNIDENTIFIED_SUBSCRIBER = 28,	///< Subscriber cannot be identified
	SMS_FACILITY_REJECTED = 29,			///< SMS facility rejected
	SMS_UNKNOWN_SUBSCRIBER = 30,		///< Subscriber unknown
	SMS_NETWORK_OUT_OF_ORDER = 38,		///< Network out of order, try later
	SMS_TEMPORARY_FAILURE = 41,			///< Temporary failure, try later
	SMS_CONGESTION = 42,				///< Network congestion, try later
	SMS_RESOURCES_UNAVAILABLE = 47,		///< Resources unavailable at the network.
	SMS_FACILITY_NOT_SUBSCRIBED = 50,	///< Facility has not been subscribed.
	SMS_REQ_FACILTY_NON_IMPL = 69,		///< Facility has not been implemented
	SMS_INVALID_REFERENCE_VALUE = 81,	///< Invalid Reference Value
	SMS_SEMANT_INCORRECT_MSG = 95,		///< Incorrect Message Semantics
	SMS_INVALID_MANDATORY_INFO = 96,	///< Invalid Mandatory Information
	SMS_MSG_TYPE_NON_EXISTENT = 97,		///< Message type non existant
	SMS_MSG_NOT_COMPATIBLE = 98,		///< Message is not compatible
	SMS_IE_NON_EXISTENT = 99,			///< Information Element non existent
	SMS_PROTOCOLL_ERROR = 111,			///< Protocol Error
	SMS_INTERWORKING = 127,				///< Interworking function error
	SMS_ERROR_UNKNOWN					///< Unknown Error
} SMSCause_t;

/// SMS coding group
typedef enum{
	GENERAL_DATA_CODING_GRP,	///< General Data Coding Group
	RESERVED_CODING_GRP,		///< Reserved Coding Group
	MSG_WAITING_DISCARD_GRP,	///< Message waiting discard coding group
	MSG_WAITING_STORE_DEFAULT_GRP,	///< Message waiting store default group
	MSG_WAITING_STORE_UCS2_GRP,	///< Message waiting store in UCS2 group
	MSG_DATA_CODING_MSGCLASS_GRP, ///< data coding message group
	DEFAULT_LANG0_CODING_GRP,    ///< CB DCS coding group 0000
	DEFAULT_LANG1_CODING_GRP,    ///< CB DCS coding group 0001
	DEFAULT_LANG2_CODING_GRP,    ///< CB DCS coding group 0010
	DEFAULT_LANG3_CODING_GRP,    ///< CB DCS coding group 0011
	WAP_CODING_GRP,
	MSG_AUTOMATIC_DELETION_GRP
} smsCodingGrp_t ;

/// SMS Waiting Indication 
typedef enum
{
	SMS_WAITINGIND_VOICE_MESG,	///< Voice Message is waiting
	SMS_WAITINGIND_FAX_MESG,	///< Fax message waiting	
	SMS_WAITINGIND_EMAIL_MESG,	///< E-Mail is waiting
	SMS_WAITINGIND_OTHER_MESG,	///< Other messages
	SMS_WAITINGIND_VIDEO_MESG,	///< Video messages  (R6 feature)
	SMS_WAITINGIND_EVM_MESG		///< Enhanced Voicemail Messages  (R6 feature)

} SmsWaitInd_t;  // Beware, enum must follow 3.38 values
// Beware, enum must follow 3.38 values
///	SMS Languages
typedef enum
{
	SMS_LANG_GERMAN = 0,		///< German
	SMS_LANG_ENGLISH,			///< English
	SMS_LANG_ITALIAN,			///< Italian
	SMS_LANG_FRENCH,			///< French
	SMS_LANG_SPANISH,			///< Spanish
	SMS_LANG_DUTCH,				///< Dutch
	SMS_LANG_SWEDISH,			///< Swedish
	SMS_LANG_DANISH,			///< Danish
	SMS_LANG_PORTUGUESE,		///< Portugese
	SMS_LANG_FINNISH,			///< Finnish
	SMS_LANG_NORWEGIAN,			///< Norwegian
	SMS_LANG_GREEK,				///< Greek
	SMS_LANG_TURKISH,			///< Turkish
	SMS_LANG_HUNGARIAN,			///< Hungarian
	SMS_LANG_POLISH,			///< Polish
	SMS_LANG_UNSPECIFIED,		///< Not Specified
	SMS_LANG_CZECH,   			///< Czech                        // Phase 2+
	SMS_LANG_EUROP,
	SMS_LANG_CODED
} SmsLanguage_t;


/// Status Report type
typedef enum
{
	NON_SR_TYPE,		///< message is not a status report type
	SR_TYPE,			///< message is a status report type
	SR_EITHER_TYPE		///< message could be either types
} SrType_t;


/// SMS Message Class.  Must equal msg classes as specified in 3.38 for coding groups 00xxB and 1111B
typedef enum
{   
	SMS_MSG_CLASS0,			///< Message Class 0	
	SMS_MSG_CLASS1,			///< Message Class 1, ME specific
	SMS_MSG_CLASS2,			///< Message Class 2, SIM specific
	SMS_MSG_CLASS3,			///< Message Class 3, TE specific
	SMS_MSG_NO_CLASS = 0xFF	///< Undetermined
} SmsMsgClass_t; 

/// SMS Alphabet Type - enum must follow 3.38 values
//<EXEMPT_INAPPROPRIATE_WORDS>
typedef enum 
{   
	SMS_ALPHABET_DEFAULT,   ///< Default SMS alphabet per AT+CSCS. See GSM 3.38
	SMS_ALPHABET_8BIT,		///< 8 bit character set
	SMS_ALPHABET_UCS2,		///< 16 bit universal character code
	SMS_ALPHABET_RESERVED,	///< Reserved for future use
	SMS_ALPHABET_KSC5601
} SmsAlphabet_t;
//<ENFORCE_INAPPROPRIATE_WORDS>

/// SMS storage API.
typedef enum
{
	SM_STORAGE, ///< SIM storage type
	BM_STORAGE, ///< Broadcast Message Storage type
	ME_STORAGE, ///< ME storage type
	MT_STORAGE, ///< Any of the storages associated with ME
	TA_STORAGE,	///< TA Storage
	SR_STORAGE,	///< Status Report Storage
	NB_MAX_STORAGE,
	NO_STORAGE = 0xFF
} SmsStorage_t;

/// SMS Message Status (3GPP 51.011 sec 10.5.3).
typedef enum
{
	SMS_STATUS_FREE		= 0x00,		///< Status Free
	SMS_STATUS_READ 	= 0x01,		///< Message Read
	SMS_STATUS_UNREAD 	= 0x03,		///< Message Unread
	SMS_STATUS_SENT	 	= 0x05,		///< Message Sent from MS
	SMS_STATUS_UNSENT 	= 0x07,		///< Message not sent
	SMS_STATUS_INVALID	= 0xFF
} SmsMesgStatus_t;	

/// Incoming message storage wait states
typedef enum
{
	SMS_STORAGE_WAIT_NONE,	///< Wait state none
	SMS_MT_STORAGE_WAIT,	///< Mobile Terminated storage wait state
	SMS_SR_STORAGE_WAIT,	///< Status Report wait state
	SMS_CB_STORAGE_WAIT,	///< Cell Broadcast wait state
	SMS_CMGW_STORAGE_WAIT,	///< Message writing(AT+CMGW command) wait state
	SMS_CMSS_STORAGE_WAIT	///< AT+CMSS command wait state
} SmsStorageWaitState_t;

// SMS internal values
/// SMS message types (3GPP 03.40)
typedef enum{	
	SMS_DELIVER,		///< SMS Delivered
	SMS_SUBMIT_REPORT,	///< Submit report
	SMS_STATUS_REPORT,	///< Status report
	SMS_DELIVER_REPORT,	///< Delivery report
	SMS_COMMAND,		///< SMS Command
	SMS_SUBMIT,			///< SMS Submit
    SMS_MTI_INVALID = 0xFF  ///< Invalid Value
} SmsMti_t;

/// VPF supported values
typedef enum {
	SMS_VPF_NO_VP    ,	///< No validity period
	SMS_VPF_ENHANCED ,	///< enhanced validity period
	SMS_VPF_RELATIVE ,	///< relative validity period
	SMS_VPF_ABSOLUTE	///< absolute validity period
} SmsVpf_t ;

/// SMS Transactions
typedef enum {
        SMS_TYPE_INVALID,
        SMS_TYPE_SUBMIT_MSG,			///< Submit SMS message (CMGS)
        SMS_TYPE_SUBMIT_MSG_FROM_SENT,	///< Submit SMS from sent box (CMSS)
        SMS_TYPE_WRITE_MSG,				///< Write SMS message for store (CMGW)
        SMS_TYPE_READ_MSG,				///< Read SMS message (CMGR)
        SMS_TYPE_LIST_MSGS,				///< List all SMS messages (CMGL)
        SMS_TYPE_DELETE_MSGS,			///< Delete SMS message (CMGD)
        SMS_TYPE_WRITE_SCA_NUMBER,		///< Write SMS service center number (CSCA)
        SMS_TYPE_ACCESS_VM,				///< Access Voice Mail indication (MVMIND)
        SMS_TYPE_READ_VMC_NUMBER,		///< Read Voice Mail Service Center number (MVMSC)
		SMS_TYPE_GET_MSG_BOX_STAT,		///< Get Message box status (MMGSR)
		SMS_TYPE_SET_CB_MSG_TYPE,		///< Set Cell Broadcast message type (CSCB)
		SMS_TYPE_UPDATE_VMSC,			///< Update Voice Mail Service Center number (VMSC)
		SMS_TYPE_SMS_COMMAND			///< SMS Command (CMSC)
} SmsTransactionType_t;


/// SMS Network Acknowledgement
typedef enum
{
	SMS_ACK_SUCCESS,			///< SMS_DELIVER received successful
	SMS_ACK_ERROR,				///< SMS_DELIVER received Error
	SMS_ACK_MEM_EXCEEDED,		///< Memory capacity exceeded
	SMS_ACK_PROTOCOL_ERROR,		///< Protocol Error
	SMS_ACK_TEMPORARY_FAILURE	///< Temporary Failure

} SmsAckNetworkType_t;

/// SMS_SUBMIT response type
typedef enum
{
	SMS_SUBMIT_RSP_TYPE_INTERNAL,		///< SMS_SUBMIT response internal type 
	SMS_SUBMIT_RSP_TYPE_SUBMIT,			///< SMS_SUBMIT response type: SUBMIT
	SMS_SUBMIT_RSP_TYPE_PARAMETER_CHECK, ///< SMS_SUBMIT response type: Parameter Check
	SMS_SUBMIT_RSP_TYPE_COMMAND,		///< SMS_SUBMIT response type: SMS COMMAND
	SMS_SUBMIT_RSP_TYPE_CMD_PARAM_CHECK ///< SMS_SUBMIT response type: Command Parameter Check
} SmsSubmitRspType_t;



///	SMS Incoming Message Stored Result

typedef struct {
	SIMAccess_t result;					///< SIM access result (in the case of ME, borrow the result enum)
	UInt16		rec_no;					///< Record number found, range 0 to n-1
	SmsStorage_t storage;				///< storage type
	SmsStorageWaitState_t waitState;
} SmsIncMsgStoredResult_t;

typedef struct
{
	SmsStorageWaitState_t smsStorageWaitState;
	SmsStorage_t	storageType;
	UInt8			msgData[SMSMESG_DATA_SZ];
	UInt16			dataLen;
	UInt16			recNum;
	UInt8			tp_pid;
	SIMSMSMesgStatus_t smsState;
} SmsMsgToBeStored_t;

//------ CB types -------------
typedef struct {
   UInt8              DcsRaw;         // raw value
   smsCodingGrp_t	  CodingGrp;
   Boolean            Compression;
   SmsLanguage_t      Language;
   SmsMsgClass_t      MsgClass;
   SmsAlphabet_t      MsgAlphabet;
} SmsCbDcs_t;

///	Structure for parameters in SMS Command

typedef struct{
	UInt8			fo;
	UInt8			ct;
	UInt8			pid;
	UInt8			mn;
}SmsCommand_t;

/// SMS Bearer Prefernce
typedef enum 
{
	SMS_OVER_GPRS_ONLY	= 0,	///< GPRS Only
	SMS_OVER_CS_ONLY	= 1,	///< Circuit Switched Only
	SMS_OVER_GPRS_PREF	= 2,	///< GPRS Preferred, fall back to CS
	SMS_OVER_CS_PREF	= 3,	///< Circuit preferred, fall back to GPRS

	SMS_INVALID_BEARER_PREF		///< Invalid Preference

} SMS_BEARER_PREFERENCE_t;	///< Bearer Preference Type


/// SMS Push Type
typedef enum 
{
	SMS_REGULAR_TYPE,		///< Regular SMS
	SMS_OTA_TYPE,			///< Over the Air SMS (usually done for maintainence/new features by operator)
	SMS_REGULAR_PUSH_TYPE,	///< Regular Push
	SMS_SECURE_PUSH_TYPE,	///< Secure push
	SMS_SUPL_INIT_TYPE,		///< SUPL INIT SMS
	SMS_OTA_SYNC_TYPE		///< Over the Air SyncML configuration settings
}SmsPushType_t;

/// SMS National Language Shift Type
typedef enum 
{
	SMS_NLS_TYPE_NONE,			///< National Language Shift not specified
	SMS_NLS_TYPE_SINGLE_SHIFT,	///< National Language Shift: Single Shift
	SMS_NLS_TYPE_LOCKING_SHIFT	///< National Language Shift: Locking Shift
} SmsNLSTypes_t;

/// SMS National Language Shift Language
typedef enum 
{
	SMS_NLS_LANG_NONE,			///< National Language Shift Language not specified
	SMS_NLS_LANG_TURISH,		///< National Language Shift Language Turkish
	SMS_NLS_LANG_SPANISH,		///< National Language Shift Language Spanish
	SMS_NLS_LANG_PORTUGUESE,	///< National Language Shift Language Portuguese
	SMS_NLS_LANG_RESERVED		///< National Language Shift Language reserved
} SmsNLSLanguage_t;


//====================
// struct definition
//====================

/// SMS Bearer DataBase
typedef struct {
	SMS_BEARER_PREFERENCE_t prefer;	///< Preferred Bearer for Sending SMS 
	SMS_BEARER_PREFERENCE_t current;///< Current Bearer for Sending SMS 
} smsBearerDb_t;

/// SMS Module Ready Status
typedef struct {
	Result_t simSmsStatus;	///< SIM status for SMS
	Result_t meSmsStatus;	///< ME status for SMS
} smsModuleReady_t;

/// SMS Address Type (decoded)
typedef struct{
	UInt8		TypeOfAddress;				///< Type of Address
	UInt8		Number[SMS_MAX_DIGITS+1];	///< Number in string format
} SmsAddress_t;		


///	SMS 411 Address Type (in accordance with GSM 4.11)
typedef struct {
	UInt8		Len;					 ///< Length
	UInt8		Toa;					 ///< Type of Address
	UInt8		Val[SMS_MAX_DIGITS / 2]; ///< Address in BCD
} Sms_411Addr_t ;

///	SMS 340 Address Type (in accordance with GSM 3.40)
typedef struct {
	UInt8		NbDigit;					///< Number of Digits
	UInt8		Toa;						///< Type of Address
	UInt8		Val[SMS_MAX_DIGITS / 2];	///< Address in BCD
} Sms_340Addr_t ;


///	SMS Coding Type
typedef struct
{   
	SmsAlphabet_t	alphabet;   ///< SMS Alphabet
	SmsMsgClass_t	msgClass;	///< Message Class
	UInt8			codingGroup;///< Coding Group
} SmsCodingType_t;


///	SMS data coding scheme
typedef struct{
	UInt8			DcsRaw;         ///< DCS raw data
	smsCodingGrp_t	CodingGrp;		///< SMS coding group
	Boolean			Compression;	///< SMS string compression
	SmsMsgClass_t	MsgClass;		///< SMS message class
	Boolean			IndActive;		///< voice mail waiting indication active
	SmsWaitInd_t	IndType;		///< voice mail waiting indication type
	SmsAlphabet_t	MsgAlphabet;	///< SMS message alphabet type
	UInt8			vmMsgCount;		///< voice mail message count
} SmsDcs_t;

///	SMS Absolute Time
typedef struct
{    
	UInt8		years;           ///< year field
	UInt8		months;          ///< month field    
	UInt8		days;            ///< day field 
	UInt8		hours;           ///< hour field  
	UInt8		minutes;         ///< min field    
	UInt8		seconds;         ///< sec field    
	UInt8		time_zone;       ///< time zone field
} SmsAbsolute_t;

///	SMS Relative Time
typedef struct
{
	UInt8		time;	        ///< relative time
} SmsRelative_t;

#define MS_MN_TP_VP_ENHANCED	7  // this parameter should be the same as the MN_TP_VP_ENHANCED defined in stack's msnu.h

///	SMS Time
typedef struct{
   Boolean			isRelativeTime;
   SmsRelative_t	relTime;
   SmsAbsolute_t	absTime;
   UInt8			enhTime[MS_MN_TP_VP_ENHANCED];			///< enhanced time (msnu.h: MN_TP_VP_ENHANCED(7) )
} SmsTime_t;

typedef union{
   UInt8            RelTime;
   SmsAbsolute_t	AbsTime;
   UInt8			EnhTime[MS_MN_TP_VP_ENHANCED];			///< enhanced time (msnu.h: MN_TP_VP_ENHANCED(7) )
} Sms_340Time_t;

///	Structure used to decode received TP-DUs
typedef struct{
	UInt8			Fo;			  ///< SMS message first byte
	SmsMti_t		Mt;           ///< sms internal coding of MTi
	UInt8			TpMr;		  ///< TP MR (reference number)
	SmsAddress_t	DaOa;         ///< DA(Submit) or OA(Deliver)
	SmsAddress_t	Ra;           ///< RA(Status Report)
	UInt8			TpPid;		  ///< SMS protocol ID
	SmsDcs_t		Dcs;		  ///< SMS coding type 
	SmsTime_t		VpScts;      ///< VP(Submit) or SCTS(Deliver, Status Report)
//	UInt8			VpRelTime;	 ///< VP(Submit) in relative time format.
	SmsTime_t		Dt;          ///< DT(Status Report)
	UInt8			St;          ///< Status(Status Report)
	UInt8			Pi;          ///< Parameter Idicator (Status Report)
                                   ///< If Fo(status report and Pi == 0,
                                   ///< Pid, Dcs, Udl and all related fields are
                                   ///< non significant)
	UInt8			HeaderLen;   ///< TP header length (from fo to udl included)
	UInt8			UdTotalLen;  ///< TP-UD total lengh (including UDL Byte)
	UInt8			Udl;         ///< user data length 
	UInt8			Udhl;        ///< user data header Length in bytes
	UInt8			*Udh;        ///< Points to UDH in AI format, NULL if Udhl=0
	UInt8			DataLen;     ///< Ud length in bytes (UDL byte not included)
	UInt8			UdNbChar;    ///< Ud number of char
	UInt8			*Ud;          ///< points to SMS User Data in 3.40 format
	UInt8			udhSeptets;	  ///< number of septets for udh (include udhl and fill bits)
	Sms_340Time_t	VpSctsPdu;    ///< VP(Submit) or SCTS(Deliver, Status Report) without decoding 
} SmsTpdu_t;

typedef struct {
	SmsNLSTypes_t		nls_type;	///< National Language Shift types: 0 - none; 1 - single shift; 2 - locking shift
	SmsNLSLanguage_t	nls_lang;	///< National Language Shift: language.
} SmsNLS_t;

///	SMS status report data format
typedef struct
{  
	UInt8			msgRefNum;			///< message reference number
	SmsTime_t		srvCenterTime; 		///< service center time
	SmsTime_t		discardTime;		///< discard time
	UInt8			status;				///< status
	UInt8			paramInd;			///< parameter indication
	UInt8			procId;  			///< protocol ID
	UInt8			fo;					///< first octet
	SmsDcs_t		codingScheme; 		///< coding scheme
	Boolean			isUserDataPresent;	///< is user data present
} SmsSrParam_t;

///	SMS Submit message format
typedef struct
{  
	SmsCodingType_t	codingType;			///< Coding type
	SmsTime_t		validatePeriod;		///< Validity Period
	UInt8			procId;				///< protocol ID
	UInt8			msgRefNum;			///< Message Reference Number
	Boolean			rejDupl;			///< Reject Duplicate Messages Flag
	Boolean			statusRptRequest;	///< Status Report Request Flag
	Boolean			replyPath;			///< Reply Path flag
	Boolean			isCompression;		///< Compression enabled flag
	Boolean			userDataHeaderInd;	///< User Data Header Indication Flag
} SmsTxParam_t;

typedef struct {
	UInt16		rec_no;					///< Record number found, range 0 to n-1
	SmsStorage_t storage;				///< SMS Storage
} SmsGetMsgFromBuffer_t;

 
///	SMS Receive Parameters


 
///	SMS Deliver message format
typedef struct
{    
	SmsAddress_t	ServCenterAddress;  ///< Service center address  
	SmsDcs_t		codingScheme;		///< Coding scheme
	SmsTime_t		srvCenterTime;		///< service center time
	Boolean			moreMsgFlag;		///< more message flag
	Boolean			replyPath;			///< Reply Path
	Boolean			usrDataHeaderInd;   ///< user data header indication
	Boolean			statusReportInd;    ///< status report indication
	UInt8			procId;				///< protocol ID
	UInt8			fo;					///< first octet
} SmsRxParam_t;

///	SMS receiving setting
typedef struct
{
	UInt8 bufferMode;
	UInt8 mtMode;
	UInt8 bmMode;
	UInt8 dsMode;
	UInt8 bfrMode;
	SmsStorage_t prefStorage;
	SmsStorage_t lastUsedIncMsgStorage;
	SmsStorage_t accessStorage;
} SmsRxSettings_t;


///	SMS SIM Message Parameters
typedef struct
{
	SmsMti_t	 msgTypeInd;		///< decoded from first octet		
	SIMAccess_t	 result;			///< access result (when read from SIM)
	UInt16		 rec_no;			///< record number (when read from SIM)
	SIMSMSMesgStatus_t 	status;		///< SMS message status
	SmsAddress_t daoaAddress;		///< destination address (Tx), or origination address (Rx), or destination address status report
	UInt16		 udhLen;			///< user data header length
	UInt8*		 udhData;			///< user data header data
	UInt16		 textLen;			///< SMS text length
	UInt8		 text_data[SMS_DATA_LENGTH_7BIT_APHABET]; ///< SMS text 
	union{
		SmsRxParam_t msgRxData;		///< union SMS_DELIVER data
		SmsTxParam_t msgTxData;		///< union SMS_SUBMIT data
		SmsSrParam_t msgSrData;		///< union SMS_STATUS_REPORT data
	} msg;
	UInt8 PDU[SMSMESG_DATA_SZ];		///< Raw PDU data
	UInt16 pduSize;					///< PDU data size
	SmsNLS_t	nls;				///< SMS National Language Shift parameters
} SmsSimMsg_t;

 
///	SMS Application specific message data
typedef struct
{
	SmsSimMsg_t fSmsMsgData;	///< The SMS message content.
	SmsPushType_t fSmsType;		///< The SMS type.
} SmsAppSpecificData_t;


 
///	Voice Mail Indication
typedef struct
{	
	Boolean staL1;			///< voicemail indication for Line 1
	Boolean staL2;			///< voicemail indication for Line 2
	Boolean staFax;			///< voicemail indication for Fax
	Boolean staData;		///< voicemail indication for Data
	UInt8	msgCount;		///< num of voicemail msg waiting
	SmsWaitInd_t msgType;	///< type of voicemail msg waiting
} SmsVoicemailInd_t;

 
///	SMS Delete Result


typedef struct {
	SIMAccess_t  result;				///< SIM/ME access result (in the case of ME, borrow the result enum)
	UInt16		 rec_no;				///< Record number found, range 0 to n-1
	SmsStorage_t storage;				///< storage type
} SmsMsgDeleteResult_t;					///< SMS Message delete Result

 
///	SMS Submit Response



typedef struct
{
	SmsSubmitRspType_t	submitRspType;		///< submit response type
	MS_T_SMS_PP_CAUSE_t		NetworkErrCause;	///< causes defined in msnu.h
	Result_t			InternalErrCause;	///< causes defined in at_rspcode.h
	UInt8				tpMr;				///< TP-MR value used if message is sent to network; Applicable 
											///< for submitRspType == SMS_SUBMIT_RSP_TYPE_SUBMIT case (message is sent to network) only
	MS_T_SMS_PP_CAUSE_t		NetworkErrCauseRp;	///< TP causes defined in msnu.h
	MS_T_SMS_PP_CAUSE_t		NetworkErrCauseTp;	///< RP causes defined in msnu.h
	UInt8				tpdu_len;			///< The length of the tpdu array.  If "0", there's no tpdu array.
	UInt8				tpdu[250];			///< The tpdu array from network, need to check the tpdu_len first before using it, and need to delete this buffer after use to avoid mem leak.
} SmsSubmitRspMsg_t;



///	SMS Stored Cb
typedef struct
{
 UInt16 Serial;						///< CB msg serial number
 UInt16 MsgId;						///< CB msg ID
 UInt8  Dcs;						///< CB msg coding scheme
 UInt8  NbPages;					///< CB msg number of pages
 UInt8  NoPage;						///< CB msg page number
 UInt8	NoOctets;					///< CB msg page length
 UInt8  Msg [CB_DATA_PER_PAGE_SZ];	///< CB msg data
 UInt8	stack_padded_len;			///< Stack padding length
} SmsStoredSmsCb_t;

///	SMS Preferred Storage locations 
typedef enum
{  
	SMS_STO_TYPE_SM,     ///< store in SIM 
	SMS_STO_TYPE_ME,     ///< store in ME  
	SMS_STO_TYPE_ME_SM,  ///< store in ME then SIM (if ME is full)  
	SMS_STO_TYPE_SM_ME   ///< store in SIM then ME (if SIM is full)
} SmsPrefStorageType_t;

///	SMS Preferred New Message Display Mode
typedef enum
{
	SMS_MODE,		///< SMS Mode
	SMS_MT,			///< Any of the Storages associated with ME
	SMS_BM,			///< Broadcast Message Storage
	SMS_DS,			///< DS
	SMS_BFR			///< Buffer for Unsolicited Responses
} NewMsgDisplayPref_t;

///	SMS ME Access Mode
typedef enum
{
	MEACCESS_SUCCESS,				///< Access was successful
	MEACCESS_NO_ACCESS,				///< No access
	MEACCESS_MEMORY_ERR,			///< Access failed, memory problem
	MEACCESS_OUT_OF_RANGE,			///< Access failed, invalid address
	MEACCESS_NOT_FOUND,				///< Access failed, no such file 
	MEACCESS_MEMORY_EXCEEDED,		///< ME Memmory exceeded
	MEACCESS_CANNOT_REPLACE_SMS		///< ME can not replace sms
} MEAccess_t;	

///	SMS Text Mode Parameters
typedef struct
{
	UInt8			fo;			///< Msg type and settings
	UInt8			vp;			///< Validity period (relative only)
	UInt8			pid;		///< Protocol identifier
	UInt8			dcs;		///< Data coding scheme (raw)	
} SmsTxTextModeParms_t;

///	SMS Transaction Type
typedef struct
{
	UInt8				 clientID;		///< Client ID The ID to identify the client which calls this API
	SmsTransactionType_t smsTrans;		///< SMS Transaction
	SmsStorage_t		 storageType;	///< Storage type
	SmsMesgStatus_t		 category;		///< used to specify msgBox for SMS list
	ClientInfo_t		 clientInfo;	///< clientInfo
} SmsTransaction_t;

///	SMS Cell Broadcast data type
typedef struct
{       
	UInt16			smsHandle;	
	UInt16			serialNo; 	
	UInt16			MsgId; 	
	SmsCodingType_t	codingScheme; 	
	UInt8			NbPages; 	
	UInt8			NoPage; 	
	UInt8			Msg [CB_DATA_PER_PAGE_SZ];
} SmsCBData_t;


///	SMS Profile 
typedef struct
{
	Boolean			isProfUsed;	   ///< Is Profile Used Flag
	SmsTxParam_t	smsTxSettings; ///< Tx settings
	Sms_411Addr_t	sca;		   ///< Current service center address
} SmsProfile_t;

///	SMS Pre Profile 
typedef struct
{	
	UInt8			currPrf;			///< Current profile no
	SmsProfile_t	prf[NUM_SMS_PROFS]; ///< Msg type and settings
} SmsPrefProfile_t;

/// SMS settings to be stored in FS
typedef struct
{
	SmsPrefProfile_t	prfProfile;		// sms preferred profiles
	SmsTxParam_t		smsDefaultTxParams;
	SmsRxSettings_t		smsDefRxSettings;
	smsBearerDb_t		smsBearerPref;
	MS_T_MN_CB_LANGUAGES	cbLanguages;
} SmsSettings_t;

///	Coding Scheme For SMS Data (See Section 4 of GSM 03.38)
typedef enum
{
	SMS_CODING_DEFAULT_ALPHABET,	///< 7-bit default alphabet data
	SMS_CODING_8BIT,				///< 8-bit data
	SMS_CODING_UCS2					///< UCS2-80 data
} SmsData_Dcs_t;

///	Coding Scheme For Cell Broadcast Data (See Section 5 of GSM 03.38)
typedef enum
{
	CB_CODING_DEFAULT_ALPHABET,	///< 7-bit default alphabet data
	CB_CODING_8BIT,				///< 8-bit data
	CB_CODING_UCS2				///< UCS2-80 data
} CbData_Dcs_t;

/// Cell Broadcast Action
typedef enum
{
	SMS_CB_START_CNF,	///< Start Confirm
	SMS_CB_START_REJ,	///< Start Reject
	SMS_CB_STOP_CNF,	///< Stop Confirm
	SMS_CB_STOP_REJ		///< Stop Reject
} SmsCBActionType_t;

///	Cell Broadcast Stop Response
typedef enum
{
	SMS_CB_STOP_TYPE_PARTIAL,	///< Partial
	SMS_CB_STOP_TYPE_ALL,		///< Stop All Cell Broadcast
	SMS_CB_STOP_TYPE_NONE		///< None
} SmsCBStopType_t;

/// Array of CB message ID ranges
typedef struct
{
    MS_T_MN_CB_MSG_ID_RANGE A[MAX_MSG_ID_RANGE_LIST_SIZE];	
} SMS_CB_MSG_ID_RANGE_LIST_t;

///	CB message ID ranges structures
typedef struct 
{
    UInt8 nbr_of_msg_id_ranges;						///< Number of CB message ID ranges in "msg_id_range_list". 
    SMS_CB_MSG_ID_RANGE_LIST_t msg_id_range_list;	///< Array of CB message ID ranges
} SMS_CB_MSG_IDS_t;

///	Cell Broadcast Message Response. Refer ::MSG_SMS_CB_START_STOP_RSP
typedef struct
{
	SmsCBActionType_t			actType;     
	SmsCBStopType_t				stopType;     
} SmsCBMsgRspType_t;

///	Enhanced Voice Mail Information IEI (3GPP 23.040 9.2.3.24.13)
typedef struct
{	
	Boolean isDataValid;	///< TRUE: the data struct has been filled with valid data; FALSE: never been used
	Boolean isNotifType;	///< isNotif:					1 = Notification; 0 = Delete Comfirmation
	Boolean multiSubProf;	///< bit2-3: 00=profID1; 10=profID2, 01=profID3, 11=profID4
	Boolean isToBeStored;	///< isToBeStored:				1 = SMS should be stored; 0 = discard
	Boolean vmBoxAlmostFull;///< vm box almost full:		1 = TRUE; 0 = FALSE
	Boolean vmBoxFull;		///< vm box is full:			1 = TRUE; 0 = FALSE
	Boolean vmStaExtInd;	///< vm status extension ind:	1 = TRUE; 0 = FALSE
	Sms_340Addr_t accessAddr; ///< Access address
	UInt8	msgCount;		///< num of unread voicemail msg (0-255)
	UInt8	notifCount;		///< num of notification msg to follow in this IE (0-15)
	UInt8	staExtLength;	///< vm box status extension length (1-255), conditional field based on vmExtInd.
	UInt8*	staExtData;		///< vm box status extension data, length can be up to extLength. conditional on vmExtInd.
	UInt16	msgID;			///< vm msg ID of this notification, 16 bytes.
	UInt8	vmMsgLength;	///< vm msg length in seconds. 0-254 means actual seconds, 255 means 255 or more seconds.
	UInt8	vmMsgRetentionDays;	///< vm msg retention days. 0-30 means actual days, 31 means 31 or more days.
	Boolean isUrgent;		///< vm msg priority ind:		1 = urgent; 0 = otherwise
	Boolean vmMsgExtInd;	///< vm msg extension ind:		1 = Extension; 0 = no extension
	Sms_340Addr_t cliAddr;	///< calling line identification address
	UInt8	vmMsgExtLength;	///< vm msg extension length (1-255), conditional field based on vmMsgExtInd.
	UInt8*	vmMsgExtData;	///< vm msg extension data, length can be up to vmMsgExtLength. conditional on vmMsgExtInd.
} SmsEnhancedVMInd_t;




#endif // _SMS_DEF_H_
