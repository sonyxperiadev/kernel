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
*   @file   capi2_sms_ds.h
*
*   @brief  This file defines the interface for CAPI2 SMS API.
*
****************************************************************************/
/**

*   @ingroup    CAPI2_SMSGroup
*
*   @brief      This group defines the interfaces to the SMS system and provides
*				API documentation needed to create short message service applications.  
*				This group also deals with Cell Broadcast and VoiceMail indication services.
*				The APIs provided enables the user to write applications to create, store,
*				save, send and display SMS and Cell Broadcst messages.
****************************************************************************/
#ifndef _CAPI2_SMS_DS_H_
#define _CAPI2_SMS_DS_H_

#include "capi2_types.h"
#include "capi2_mstypes.h"
#include "capi2_msnu.h"
#include "capi2_sim_api.h"
//-------------------------------------------------
// Constant Definitions
//-------------------------------------------------

// GSM 3.40 <-> TE decoding/encoding utilities
//
// MTI supported values
//
// 3.40 values
#define DELIVER_340        0
#define SUBMIT_REPORT_340  1
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


#define SMS_UNKNOWN_TOA					0

#define SMS_STORE_NEXT_FREE_REC			0xFFFF

#define SMS_MAX_DATA_LENGTH				140		///< See GSM 03.38
#define SMS_MAX_PDU_STRING_LENGTH		SMSMESG_DATA_SZ
#define SMS_MAX_ADDRESS_SIZE_IN_OCTET	12
#define SMS_DATA_LENGTH_8BIT_APHABET	140		///< this should be the same as defined in mns_op5.c
#define SMS_DATA_LENGTH_7BIT_APHABET	160		///< this should be the same as defined in mns_op5.c

#define SMS_TYPE_0						64		///< 01000000

#define	CB_HDR_PER_PAGE_SZ	7			///< CB Header only
#define	CB_DATA_PER_PAGE_SZ	82			///< CB Data only
#define	CB_PAGE_SZ			(CB_HDR_PER_PAGE_SZ+CB_DATA_PER_PAGE_SZ)

#define MAX_NUMBER_OF_VMSC NUM_OF_MWI_TYPE

//---------------------------------------------------------------
// enum
//---------------------------------------------------------------

/**
	SMS Cause Values
**/
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

/**
	SMS coding group
**/
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
	WAP_CODING_GRP
} smsCodingGrp_t ;

/**
	SMS Waiting Indication
**/
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

/**
	SMS Languages
**/
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

/**
	Status Report type
**/
typedef enum
{
	NON_SR_TYPE,		///< message is not a status report type
	SR_TYPE,			///< message is a status report type
	SR_EITHER_TYPE		///< message could be either types
} SrType_t;

/**
 
 */
/** 
	SMS Message Class
	Must equal msg classes as specified in 3.38 for coding groups 00xxB and 1111B
**/
typedef enum
{   
	SMS_MSG_CLASS0,			///< Message Class 0	
	SMS_MSG_CLASS1,			///< Message Class 1, ME specific
	SMS_MSG_CLASS2,			///< Message Class 2, SIM specific
	SMS_MSG_CLASS3,			///< Message Class 3, TE specific
	SMS_MSG_NO_CLASS = 0xFF	///< Undetermined
} SmsMsgClass_t; 

/** 
	SMS Alphabet Type
	- enum must follow 3.38 values
**/

typedef enum 
{   
	SMS_ALPHABET_DEFAULT,   ///< Default SMS alphabet per AT+CSCS. See GSM 3.38
	SMS_ALPHABET_8BIT,		///< 8 bit character set
	SMS_ALPHABET_UCS2,		///< 16 bit universal character code
	SMS_ALPHABET_RESERVED	///< Reserved for future use
} SmsAlphabet_t; ///< SMS Alphabet
/** 
	SMS storage API.
**/

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
} SmsStorage_t; ///< SMS storage type


/** 
	SMS Message Status (3GPP 51.011 sec 10.5.3).
**/

typedef enum
{
	SMS_STATUS_FREE		= 0x00,		///< Status Free
	SMS_STATUS_READ 	= 0x01,		///< Message Read
	SMS_STATUS_UNREAD 	= 0x03,		///< Message Unread
	SMS_STATUS_SENT	 	= 0x05,		///< Message Sent from MS
	SMS_STATUS_UNSENT 	= 0x07,		///< Message not sent
	SMS_STATUS_INVALID	= 0xFF
} SmsMesgStatus_t;	///< Message Status

/**
	Incoming message storage wait states
**/
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
/** 
	SMS message types (3GPP 03.40)
**/

typedef enum{	
	SMS_DELIVER,		///< SMS Delivered
	SMS_SUBMIT_REPORT,	///< Submit report
	SMS_STATUS_REPORT,	///< Status report
	SMS_DELIVER_REPORT,	///< Delivery report
	SMS_COMMAND,		///< SMS Command
	SMS_SUBMIT			///< SMS Submit
} SmsMti_t;

/**
  VPF supported values
**/
typedef enum {
	SMS_VPF_NO_VP    ,	///< No validity period
	SMS_VPF_ENHANCED ,	///< enhanced validity period
	SMS_VPF_RELATIVE ,	///< relative validity period
	SMS_VPF_ABSOLUTE	///< absolute validity period
} SmsVpf_t ;

/** 
	SMS Transactions
**/

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
		SMS_TYPE_UPDATE_VMSC			///< Update Voice Mail Service Center number (VMSC)
} SmsTransactionType_t;

/** 
	SMS Network Acknowledgement
**/

typedef enum
{
	SMS_ACK_SUCCESS,			///< SMS_DELIVER received successful
	SMS_ACK_ERROR,				///< SMS_DELIVER received Error
	SMS_ACK_MEM_EXCEEDED,		///< Memory capacity exceeded
	SMS_ACK_PROTOCOL_ERROR,		///< Protocol Error
	SMS_ACK_TEMPORARY_FAILURE	///< Temporary Failure

} SmsAckNetworkType_t;

/**
	SMS_SUBMIT response type
**/
typedef enum
{
	SMS_SUBMIT_RSP_TYPE_INTERNAL,		///< SMS_SUBMIT response internal type 
	SMS_SUBMIT_RSP_TYPE_SUBMIT,			///< SMS_SUBMIT response type: SUBMIT
	SMS_SUBMIT_RSP_TYPE_PARAMETER_CHECK	///< SMS_SUBMIT response type: Parameter Check
} SmsSubmitRspType_t;



/** 
	SMS Bearer Prefernce
**/

typedef enum 
{
	SMS_OVER_GPRS_ONLY	= 0,	///< GPRS Only
	SMS_OVER_CS_ONLY	= 1,	///< Circuit Switched Only
	SMS_OVER_GPRS_PREF	= 2,	///< GPRS Preferred, fall back to CS
	SMS_OVER_CS_PREF	= 3,	///< Circuit preferred, fall back to GPRS

	SMS_INVALID_BEARER_PREF		///< Invalid Preference

} SMS_BEARER_PREFERENCE_t;	///< Bearer Preference Type

/** 
	SMS Push Type
**/
typedef enum 
{
	SMS_REGULAR_TYPE,		///< Regular SMS
	SMS_OTA_TYPE,			///< Over the Air SMS (usually done for maintainence/new features by operator)
	SMS_REGULAR_PUSH_TYPE,	///< Regular Push
	SMS_SECURE_PUSH_TYPE	///< Secure push
}SmsPushType_t;



//====================
// struct definition
//====================

/// SMS Module Ready Status
typedef struct {
	Result_t simSmsStatus;	///< SIM status for SMS
	Result_t meSmsStatus;	///< ME status for SMS
} smsModuleReady_t;


/// SMS Address Type (decoded)
typedef struct{
	UInt8		TypeOfAddress;				///< Type of Address
	UInt8		Number[SMS_MAX_DIGITS+1];	///< Number in string format
} SmsAddress_t;								///< SMS Address Type

/// SMS 411 Address Type (in accordance with GSM 4.11)
typedef struct {
	UInt8		Len;						///< Length
	UInt8		Toa;						///< Type of Address
	UInt8		Val[SMS_MAX_DIGITS / 2];	///< Address in BCD
} Sms_411Addr_t ;

/// SMS 340 Address Type (in accordance with GSM 3.40)
typedef struct {
	UInt8		NbDigit;					///< Number of Digits
	UInt8		Toa;						///< Type of Address
	UInt8		Val[SMS_MAX_DIGITS / 2];	///< Address in BCD
} Sms_340Addr_t ;




/// SMS Coding Type
typedef struct
{   
	SmsAlphabet_t	alphabet;   ///< SMS Alphabet
	SmsMsgClass_t	msgClass;	///< Message Class
	UInt8			codingGroup;///< Coding Group
} SmsCodingType_t;



/// SMS data coding scheme
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



/// SMS Absolute Time
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

/// SMS Relative Time
typedef struct
{
	UInt8		time;	        ///< relative time
} SmsRelative_t;

/// SMS Time
typedef struct{
   Boolean			isRelativeTime;
   SmsRelative_t	relTime;
   SmsAbsolute_t	absTime;
} SmsTime_t;


/// Structure used to decode received TP-DUs
typedef struct{
	UInt8			Fo;			  ///< SMS message first byte
	SmsMti_t		Mt;           ///< sms internal coding of MTi
	UInt8			TpMr;		  ///< TP MR (reference number)
	SmsAddress_t	DaOa;         ///< DA(Submit) or OA(Deliver)
	SmsAddress_t	Ra;           ///< RA(Status Report)
	UInt8			TpPid;		  ///< SMS protocol ID
	SmsDcs_t		Dcs;		  ///< SMS coding type 
	SmsTime_t		VpScts;       ///< VP(Submit) or SCTS(Deliver, Status Report)
//	UInt8			VpRelTime;	  ///< VP(Submit) in relative time format.
	SmsTime_t		Dt;           ///< DT(Status Report)
	UInt8			St;           ///< Status(Status Report)
	UInt8			Pi;           ///< Parameter Idicator (Status Report)
                                  ///< If Fo(status report and Pi == 0,
                                  ///< Pid, Dcs, Udl and all related fields are
                                  ///< non significant)
	UInt8			HeaderLen;    ///< TP header length (from fo to udl included)
	UInt8			UdTotalLen;   ///< TP-UD total lengh (including UDL Byte)
	UInt8			Udl;          ///< user data length 
	UInt8			Udhl;         ///< user data header Length in bytes
	UInt8			*Udh;         ///< Points to UDH in AI format, NULL if Udhl=0
	UInt8			DataLen;      ///< Ud length in bytes (UDL byte not included)
	UInt8			UdNbChar;     ///< Ud number of char
	UInt8			*Ud;          ///< points to SMS User Data in 3.40 format
	UInt8			udhSeptets;	  ///< number of septets for udh (include udhl and fill bits)
} SmsTpdu_t;


/// SMS Incoming Message Stored Result
typedef struct {
	SIMAccess_t result;					///< SIM access result (in the case of ME, borrow the result enum)
	UInt16		rec_no;					///< Record number found, range 0 to n-1
	SmsStorage_t storage;				///< storage type
	SmsStorageWaitState_t waitState;
} SmsIncMsgStoredResult_t;



/// SMS Delete Result
typedef struct {
	SIMAccess_t  result;				///< SIM/ME access result (in the case of ME, borrow the result enum)
	UInt16		 rec_no;				///< Record number found, range 0 to n-1
	SmsStorage_t storage;				///< storage type
} SmsMsgDeleteResult_t;					///< SMS Message delete Result



/// SMS status report data format
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



/// SMS Submit message format
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


/// SMS Receive Parameters
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

typedef enum 
{
	SMS_NLS_TYPE_NONE,			///< National Language Shift not specified
	SMS_NLS_TYPE_SINGLE_SHIFT,	///< National Language Shift: Single Shift
	SMS_NLS_TYPE_LOCKING_SHIFT	///< National Language Shift: Locking Shift
} SmsNLSTypes_t;

typedef enum 
{
	SMS_NLS_LANG_NONE,			///< National Language Shift Language not specified
	SMS_NLS_LANG_TURISH,		///< National Language Shift Language Turkish
	SMS_NLS_LANG_SPANISH,		///< National Language Shift Language Spanish
	SMS_NLS_LANG_PORTUGUESE,	///< National Language Shift Language Portuguese
	SMS_NLS_LANG_RESERVED		///< National Language Shift Language reserved
} SmsNLSLanguage_t;

typedef struct {
	SmsNLSTypes_t		nls_type;	///< National Language Shift types: 0 - none; 1 - single shift; 2 - locking shift
	SmsNLSLanguage_t	nls_lang;	///< National Language Shift: language.
} SmsNLS_t;

/// SMS SIM Message Parameters: Refer ::MSG_SMSPP_DELIVER_IND, ::MSG_SMSSR_REPORT_IND
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



/// SMS receiving setting
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



/// Voice Mail Indication
typedef struct
{	
	Boolean staL1;			///< voicemail indication for Line 1
	Boolean staL2;			///< voicemail indication for Line 2
	Boolean staFax;			///< voicemail indication for Fax
	Boolean staData;		///< voicemail indication for Data
	UInt8	msgCount;		///< num of voicemail msg waiting
	SmsWaitInd_t msgType;	///< type of voicemail msg waiting
} SmsVoicemailInd_t;


/// SMS Submit Response. Refer ::MSG_SMS_SUBMIT_RSP, ::MSG_SMS_COMMAND_RSP, ::MSG_SMS_MEM_AVAIL_RSP.
typedef struct
{
	SmsSubmitRspType_t submitRspType;///< submit response type
	T_SMS_PP_CAUSE NetworkErrCause;	///< causes defined in msnu.h
	Result_t InternalErrCause;		///< causes defined in at_rspcode.h
	UInt8				tpMr;		///< TP-MR value used if message is sent to network; Applicable 
									///< for submitRspType == SMS_SUBMIT_RSP_TYPE_SUBMIT case (message is sent to network) only
} SmsSubmitRspMsg_t;


/// SMS Stored Cb. Refer ::MSG_SMSCB_DATA_IND.
typedef struct
{
 UInt16 Serial;						///< CB msg serial number
 UInt16 MsgId;						///< CB msg ID
 UInt8  Dcs;						///< CB msg coding scheme
 UInt8  NbPages;					///< CB msg number of pages
 UInt8  NoPage;						///< CB msg page number
 UInt8  NoOctets;					///< CB msg page length
 UInt8  Msg [CB_DATA_PER_PAGE_SZ];	///< CB msg data
} SmsStoredSmsCb_t;

//------ CB types -------------
typedef enum
{
	HOMEZONE_CITYZONE_IND_OFF,		///< HomeZone/CityZone indication should be cleared
	HOMEZONE_IND_ON,				///< HomeZone indication should be displayed to user
	CITYZONE_IND_ON					///< CityZone indication should be displayed to user
} HomezoneCityzoneStatus_t;

/// HomeZone indication Refer ::MSG_HOMEZONE_STATUS_IND.
typedef struct
{
	HomezoneCityzoneStatus_t status;		///< HomeZone/CityZone indication status
	UInt8	tag_len;						///< Number of bytes in "tag_data"		
	UInt8	tag_data[SIM_HZ_TAGS_LEN];		///< HomeZone/CityZone tags in GSM Default Alphabet where each byte's most significant bit is 0
} HomezoneIndData_t;



//------end CB types ----------

/**
 * 
 */

//-------------------------------------------------
// Constant Definitions
//-------------------------------------------------

#define MASK_SIM_ACCESS   0x01F		///< SIM Access Mask
#define MASK_STORAGE_TYPE 0x07		///< Storage Type Mask
#define ATC_SR_STORAGE_SIZE   29	///< ATC Storage Size
#define USE_DEFAULT_SCA_NUMBER 0xFF	///< Default Service Center Address

// number of profiles can be in the range of 0~255, specified by the manufacturer.
// To save memory, only allow 2 profiles at this time.
#define NUM_SMS_PROFS 2	///< Number of Profiles. Can be in the range 0-255 but only allow 2 to save memory.

/*
  For SMS Data Coding Scheme byte (See Section 4 of GSM 03.38) 
*/
#define	SMSCODING_ALPHABET_BIT_MASK		0x0C	///<			
#define	SMSCODING_ALPHABET_DEFAULT_VAL	0x00	///<		
#define	SMSCODING_ALPHABET_8BIT_VAL		0x04	///<		
#define	SMSCODING_ALPHABET_UCS2_VAL		0x08	///<

/**
  For Cell Broadcast Data Coding Scheme byte (See Section 5 of GSM 03.38) 
**/
#define CBCODING_ALPHABET_DEFAULT_VAL	0x0F	///< Default Alphabet With Language Unspecified


//---------------------------------------------------------------
// enum
//---------------------------------------------------------------
/** 
	SMS Preferred Storage locations 
**/

typedef enum
{  
	SMS_STO_TYPE_SM,     ///< store in SIM 
	SMS_STO_TYPE_ME,     ///< store in ME  
	SMS_STO_TYPE_ME_SM,  ///< store in ME then SIM (if ME is full)  
	SMS_STO_TYPE_SM_ME   ///< store in SIM then ME (if SIM is full)
} SmsPrefStorageType_t;

/** 
	SMS Preferred New Message Display Mode
**/

typedef enum
{
	SMS_MODE,		///< SMS Mode
	SMS_MT,			///< Any of the Storages associated with ME
	SMS_BM,			///< Broadcast Message Storage
	SMS_DS,			///< DS
	SMS_BFR			///< Buffer for Unsolicited Responses
} NewMsgDisplayPref_t;

/** 
	SMS ME Access Mode
**/

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

//-------------------------------------------------
// Data Structure
//-------------------------------------------------

/// SMS Text Mode Parameters
typedef struct
{
	UInt8			fo;			///< Msg type and settings
	UInt8			vp;			///< Validity period (relative only)
	UInt8			pid;		///< Protocol identifier
	UInt8			dcs;		///< Data coding scheme (raw)	
} SmsTxTextModeParms_t;

/// SMS Transaction Type
typedef struct
{
	UInt8				 clientID;		///< Client ID The ID to identify the client which calls this API
	SmsTransactionType_t smsTrans;		///< SMS Transaction
	SmsStorage_t		 storageType;	///< Storage type
	SmsMesgStatus_t		 category;		///< used to specify msgBox for SMS list
} SmsTransaction_t;

/// SMS Cell Broadcast data type
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

/// SMS Message Wait Indication 
typedef struct
{	
	SIMAccess_t	result;		///< SIM access result	
	UInt8		data_len;	///< Length of data returned.	
	UInt8		*data;		///< Pointer to data buffer
} SmsParmMsgWtInd_t;

/// SMS Voice Mail
typedef struct
{	
	SIMAccess_t result;		///< SIM access result	
	UInt8		data_len;	///< Length of data returned.	
	UInt8		*data;		///< Pointer to data buffer
} SmsParmVmsc_t;

/// SMS Profile 
typedef struct
{
	Boolean			isProfUsed;	   ///< Is Profile Used Flag
	SmsTxParam_t	smsTxSettings; ///< Tx settings
	Sms_411Addr_t	sca;		   ///< Current service center address
} SmsProfile_t;

/// SMS Pre Profile 
typedef struct
{	
	UInt8			currPrf;			///< Current profile no
	SmsProfile_t	prf[NUM_SMS_PROFS]; ///< Msg type and settings
} SmsPrefProfile_t;

/** 
	Coding Scheme For SMS Data (See Section 4 of GSM 03.38)
**/
typedef enum
{
	SMS_CODING_DEFAULT_ALPHABET,	///< 7-bit default alphabet data
	SMS_CODING_8BIT,				///< 8-bit data
	SMS_CODING_UCS2					///< UCS2-80 data
} SmsData_Dcs_t;

/** 
	Coding Scheme For Cell Broadcast Data (See Section 5 of GSM 03.38)
**/
typedef enum
{
	CB_CODING_DEFAULT_ALPHABET,	///< 7-bit default alphabet data
	CB_CODING_8BIT,				///< 8-bit data
	CB_CODING_UCS2				///< UCS2-80 data
} CbData_Dcs_t;


/// SMS Application specific message data
typedef struct
{
	SmsSimMsg_t fSmsMsgData;	///< The SMS message content.
	SmsPushType_t fSmsType;		///< The SMS type.
} SmsAppSpecificData_t;

/** 
	Cell Broadcast Action
**/

typedef enum
{
	SMS_CB_START_CNF,	///< Start Confirm
	SMS_CB_START_REJ,	///< Start Reject
	SMS_CB_STOP_CNF,	///< Stop Confirm
	SMS_CB_STOP_REJ		///< Stop Reject
} SmsCBActionType_t;

/** 
	Cell Broadcast Stop Response
**/

typedef enum
{
	SMS_CB_STOP_TYPE_PARTIAL,	///< Partial
	SMS_CB_STOP_TYPE_ALL,		///< Stop All Cell Broadcast
	SMS_CB_STOP_TYPE_NONE		///< None
} SmsCBStopType_t;

//******************************************************************************
//              Typedefs
//******************************************************************************

/// SMS Report Indication event. Refer ::MSG_SMS_REPORT_IND.
typedef struct
{
	T_MN_MESSAGE_TYPE_INDICATOR type;     
	T_MN_RP_CAUSE				cause;     
} SmsReportInd_t;

/// Cell Broadcast Message Response. Refer ::MSG_SMS_CB_START_STOP_RSP
typedef struct
{
	SmsCBActionType_t			actType;     
	SmsCBStopType_t				stopType;     
} SmsCBMsgRspType_t;			

/// CAPI2_SMS_GetSMSStorageStatus message response. Refer ::MSG_SMS_GETSMSSTORAGESTATUS_RSP			
typedef struct
{
	SmsStorage_t storageType;
	UInt16 NbFree;	///< NbFree - Number of Free slots in a given storageType
	UInt16 NbUsed;	///< NbUsed - Number of Used slots in a given storageType
} CAPI2_SMS_GetSMSStorageStatus_t;

/// CAPI2_SMS_GetTransactionFromClientID message response. Refer::MSG_SMS_GETTRANSACTIONFROMCLIENTID_RSP
typedef struct
{
	SmsTransactionType_t trType;
	SmsMesgStatus_t category;
	SmsStorage_t storageType;
} CAPI2_SMS_GetTransactionFromClientID_t;

/// Enhanced Voice Mail Information IEI (3GPP 23.040 9.2.3.24.13)
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

/// SMS TE Address Type
typedef struct
{
	SmsAddress_t dst;		///<  SMS TE address response
	UInt8	len;			///< Len of address
} CAPI2_SMS_340AddrToTe_Rsp_t;


/**
	Max number of CB message ID ranges 
 **/
#define MAX_MSG_ID_RANGE_LIST_SIZE	(MN_MAX_CB_MSG_ID_LIST_LENGTH - 1)

/// CB message ID range arrays
typedef struct
{
    T_MN_CB_MSG_ID_RANGE A[MAX_MSG_ID_RANGE_LIST_SIZE];
}
SMS_CB_MSG_ID_RANGE_LIST_t;

/// CB message ID range structures 
typedef struct 
{
    UInt8 nbr_of_msg_id_ranges;
    SMS_CB_MSG_ID_RANGE_LIST_t msg_id_range_list;
}
SMS_CB_MSG_IDS_t;


//***********************************************************************************
//			Notifications
//***********************************************************************************
/**
	Message 
**/

#endif

