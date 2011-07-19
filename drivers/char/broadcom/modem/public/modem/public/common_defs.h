//*********************************************************************
//
//	Copyright 2008-2010 Broadcom Corporation
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
*   @file   common_defs.h
*
*   @brief	This file contains common definitions shared by CAPI components
*
*
****************************************************************************/

#ifndef _COMMON_DEFS_H_
#define _COMMON_DEFS_H_

//PLEASE DON"T ADD ANY HEADER FILE(S).

#ifdef __cplusplus
extern "C" {
#endif

//Temp flag added for backwards MMI compatibility with CIB and CDB modem
#define tempCAPI_HDR_FILE_DEPENDENCY_CLEANUP

/**
	GSM Type of Number
**/
typedef enum {								
	UnknownTON			= 0,	///< Unknown			
	InternationalTON	= 1,	///< International
	NationalTON			= 2,	///< National
	NetworkSpecificTON	= 3,	///< Network specific type
	DedicatedAccessTON	= 4,	///< Dedicated access
	DefaultAlphabetTON  = 5		///< Default Apha
} gsm_TON_t;///< Type of Number

/**
	GSM Number Plan Identifier
**/

typedef enum {								
	UnknownNP			= 0,		///< Unknown
	ISDNNP				= 1,		///< ISDN Number Plan
	DataNP				= 3,		///< Data Number
	TelexNP				= 4,		///< Telex type Number plan
	NationalNP			= 8,		///< National Number Plan
	PrivateNP			= 9			///< Private type
} gsm_NPI_t;///< Number Plan Identifier

//---------------------------------------------------------
//	RAT Defines
//---------------------------------------------------------
#define RAT_NOT_AVAILABLE		0
#define RAT_GSM					1
#define RAT_UMTS				2


// Definitions for fields not used
#define	NOT_USED_0			0			// for numeric fields where 0 is a value out of range: 
										// e.g. CID, Nsapi, Sapi								
#define NOT_USED_FF			0xFF		// for numeric fields where 0 is a valid value, but 
										// 0xFF is out of range: e.g. Compression, fields
										// of QosProfile structure
#define NOT_USED_STRING		""			// for string fields: e.g. PDPType, APN, PDPAddress 

//******************************************************************************
//
// Global Types 
//
//******************************************************************************
#define GPRS_SVC_CODE			99		//	GPRS service code
#define GPRS_CTXTID_MIN			1
#define GPRS_CTXTID_MAX			10

//******************************************************************************
//
// Alphabet Character Set
//
//******************************************************************************

typedef UInt8		CodePageMTI_t;			///< Code Page MTI Character Set

//******************************************************************************
//
// GSM Types
//
//******************************************************************************
///	Coding scheme for alpha data in SIM, as defined in Annex B. of GSM 11.11 
typedef enum
{
	/* Note: do not change the following enum values!!!!! */
	ALPHA_CODING_GSM_ALPHABET = 0x00,	///< Default 
	ALPHA_CODING_UCS2_80 = 0x80,
	ALPHA_CODING_UCS2_81 = 0x81,
	ALPHA_CODING_UCS2_82 = 0x82
} ALPHA_CODING_t;

/* Represent empty bytes in the SIM */
#define	SIM_RAW_EMPTY_VALUE				0xFF		///< Empty bytes in the SIM
#define	SIM_RAW_EMPTY_VALUE_TWO_BYTES	0xFFFF		///< Empty two bytes in the SIM
#define	SIM_RAW_EMPTY_VALUE_FOUR_BYTES	0xFFFFFFFF	///< Empty four bytes in the SIM


#define	INTERNATIONAL_CODE	'+'				///< '+' at the beginning
											///< of a number represents an international
											///< phone number

#define VOICE_CALL_CODE	';'			///< ';' at the end of a dialing string indicates it is voice call

/* Combined value to identify two important parameters of a phone number:
 * 1. Type of Number
 * 2. Numbering Plan
 * See section 10.5.4.7 of GSM 04.08. 
 */
#define UNKNOWN_TON_UNKNOWN_NPI ((UInt8) (0x80 | (UnknownTON << 4) | UnknownNP))	///< Unknown Type of Number and unknown Numbering Plan 
#define UNKNOWN_TON_ISDN_NPI	((UInt8) (0x80 | (UnknownTON << 4) | ISDNNP))		///< Unknown Type of Number and ISDN Numbering Plan 
#define INTERNA_TON_ISDN_NPI	((UInt8) (0x80 | (InternationalTON << 4) | ISDNNP))	///< International number and ISDN Numbering Plan 

///	Maximum Facility IE length definition
#define MAX_FACILITY_IE_LENGTH		251

#define MAX_CAUSE_IE_LENGTH 32 //refer 3GPP 24.008 10.5.4.11
  
//******************************************************************************
//
// Shared between MMI, MSC and MNCC 
//
//******************************************************************************
#define	SMS_MAX_DIGITS					40		///< Since this symbol MAX_DIGITS is used in SMS as well
												///< I created another symbol SMS_MAX_DIGITS with the same
												///< length in order not to break anything ...
												///< JAY MIGHT BE ABLE TO CONFIRM WHETHER IT IS SAFE TO
												///< USE THE SAME LENGTH OR NOT

#define	MAX_DIGITS						80		///< 04.08 Called Party BCD Number
												///< Max. number BCD Digits in a phone number
												///< 40 bytes in Packed BCD or 80 in unpacked BCD
												///< This value is limited by Network, so it
												///< should not be used for Phonebook telephone unless
												///< the Phonebook is also limited to this size.

#define MAX_PLMN_SEARCH					10		///< Maximum PLMN Search value
#define MAX_SIGNAL_STRENGTH_LEVEL		63		///< Max. Signal strength level 0x3f
#define	MAXNUM_CALLS					16 		///< Max. number of connected call allowed
#define PROTOCOLDISC_CC					0x03	///< CC protocol discriminator
#define PROTOCOLDISC_SS					0x0B	///< SS protocol discriminator
#define PHASE1_MAX_USSD_STRING_SIZE		200		///< Max. USSD string size for Phase1 (3GPP 24.080)
#define PHASE2_MAX_USSD_STRING_SIZE		160		///< Max. USSD string size for Phase2 (3GPP 24.080)
#define MAX_NAME_STRING_SIZE			160		///< Max. Calling Name string size


// String coding types according to 3GPP 3.38
#define STR_CODING_DEF_ALPHABET			0		// Default Alphabet (7bit)
#define STR_CODING_DEF_8bit				1		// 8bit Alphabet
#define STR_CODING_DEF_UCS2				2		// UCS2

//Match T_TEL_NUMBER in msnu.h
///	Phone number array
typedef struct{
	
    UInt8 Number[MAX_DIGITS/2+1];

} PhoneNumber_t;


// CallIndex_t is used to identify a call in the MS.  A call is defined
// as an Active Speech/Data or Supplementary Service

///	Call Index Values
typedef enum
{
	CALLINDEX_00 = 0,							///< 1st Call
	CALLINDEX_01 = 1,							///< 2nd Call 
	CALLINDEX_02 = 2,							///< 3rd Call
	CALLINDEX_03 = 3,							///< 4th Call
	CALLINDEX_04 = 4,							///< 5th Call
	CALLINDEX_05 = 5,							///< 6th Call
	CALLINDEX_06 = 6,							///< 7th Call
	CALLINDEX_07 = 7,							///< 8th Call
	CALLINDEX_08 = 8,							///< 9th Call
	CALLINDEX_09 = 9,							///< 10th Call
	CALLINDEX_10 = 10,							///< 11th Call
	CALLINDEX_11 = 11,							///< 12th Call
	CALLINDEX_12 = 12,							///< 13th Call
	CALLINDEX_13 = 13,							///< 14th Call
	CALLINDEX_14 = 14,							///< 15th Call
	CALLINDEX_15 = 15,							///< 16th Call
	MAX_CALL_INDEX = 16,						///< Max. Call Index
	CALLINDEX_NO_CALL_TONE = 17,				///< Special Call Index, when no
												///< call exist, but a special Tone
												///< Generation is required.
	CALLINDEX_INVALID = 0xFF					///< Invalid Call Index
} CallIndex_t;				///< MMI<-->MSC<-->MNCC Call Identifier

///	Binary Coded Digit Number type
typedef enum
{
	// BCDNUM_ZERO = 0					// Normal numbers are themselves
	//		thru						//
	// BCDNUM_NINE = 9					//
	BCDNUM_STAR = 10,					///< "*"
	BCDNUM_HASH = 11,					///< "#"
	BCDNUM_PAUSE = 12,					///< "p" 
	BCDNUM_A = 13,						///< "?"
	BCDNUM_B = 14,						///< "b"
	BCDNUM_END = 15						///< end of phone number;
} BCDNumber_t;

/* See Section 10.5.1 of GSM 11.11 */
#define DTMF_SEPARATOR_CHAR			'p'		/**< Correspond to BCDNUM_PAUSE */
#define DTMF_SEPARATOR_UPPER_CHAR	'P'		/**< Correspond to BCDNUM_PAUSE */
#define WILD_CARDING_CHAR			'?'		/**< Correspond to BCDNUM_A */
#define EXPANSION_CHAR				'b'		/**< Correspond to BCDNUM_B */



#define   CALLING_NAME_PRESENT      0x01	
#define   CALLING_NAME_AVAILABLE    0x02
#define   CALLING_NUMBER_PRESENT    0x10
#define   CALLING_NUMBER_AVAILABLE  0x20

///	Calling Name Presentation Masks
typedef enum
{
  CALLING_NAME_NUMBER_RESTRICTED        = 0x00,
  CALLING_NAME_PRESENT_AND_AVAILABLE    = 0x03,
  CALLING_NUMBER_PRESENT_AND_AVAILABLE  = 0x30,
  CALLING_NAME_NUMBER_PRESENT_AVAILABLE = 0x33
} PresentStatus_t;

/// Telephone Number Type
typedef struct
{
	gsm_TON_t	ton;					///< Type Of Number
	gsm_NPI_t	npi;					///< Numbering Plan Identification
	UInt8		number[MAX_DIGITS+1];	///< ASCII representation of
										///< telephone number (non-BCD format) size
										///< of telephone number is determine by
										///< the size of the string
} TelephoneNumber_t;


#define MAX_BCD_NUMBER_SIZE				20						///< Max BCD Number Size
#define MAX_PHONE_NUMBER_SIZE			MAX_BCD_NUMBER_SIZE * 2	///< Max Phone Number Size
#define MAX_PHONE_SUBADDRESS_SIZE		21						///< Max Phone Subaddress Size

/// Enum : Configuration Mode Type
typedef enum
{
	CONFIG_MODE_DEFAULT					= 0x00,	///< Default
	CONFIG_MODE_INVOKE					= 0x01,	///< Invoke
	CONFIG_MODE_SUPPRESS				= 0x02	///< Supress
} ConfigMode_t;									///< Configuration Mode Type


/// Enum : Type of Number
typedef enum
{
	TON_UNKNOWN							= 0x00,	///< Unknown
	TON_INTERNATIONAL_NUMBER			= 0x01,	///< International Number
	TON_NATIONAL_NUMBER					= 0x02,	///< Netional Number
	TON_NETWORK_SPECIFIC_NUMBER			= 0x03,	///< Network Specific Number
	TON_DEDICATED_ACCESS_SHORT_CODE		= 0x04,	///< Dedicated Access Short Code 
	TON_RESERVED						= 0x05	///< Reserved
} TypeOfNumber_t;								///< Type of Number

/// Enum : Number Plan ID
typedef enum
{
	NPI_UNKNOWN							= 0x00,	///< Unknown
	NPI_ISDN_TELEPHONY_NUMBERING_PLAN	= 0x01,	///< ISDN/Telephony numbering plan
	NPI_DATA_NUMBERING_PLAN				= 0x03,	///< Data numbering plan
	NPI_TELEX_NUMBERING_PLAN			= 0x04,	///< Telex numbering plan
	NPI_NATIONAL_NUMBERING_PLAN			= 0x08,	///< National numbering plan
	NPI_PRIVATE_NUMBERING_PLAN			= 0x09	///< Private numbering plan
} NumberPlanId_t;								///< Numbering Plan Identification

/// Enum : Type of Subaddress
typedef enum
{
	TOS_NSAP							= 0x00,	///< NSAP (X.213/ISO 8348 AD2
	TOS_USER_SPECIFIED					= 0x01	///< User Specified
} TypeOfSubAdd_t;								///< Type Of Subaddress

/// Enum : Odd Even Type
typedef enum
{
	ODD_NUMBER_OF_DIGITS				= 0x00,	///< Odd Number Of Digits
	EVEN_NUMBER_OF_DIGITS				= 0x01	///< Even Number Of Digits
} OddEven_t;									///< Odd Even Type

/// Enum : Presentation Indicator
typedef enum
{
	CC_PRESENTATION_ALLOWED		= 0,					///< Presentation Allowed
	CC_PRESENTATION_RESTRICTED	= 1,					///< Presentation Restricted
	CC_NUMBER_NA_INTERWORKING	= 2,					///< Number Not Available Due To Interworking
	CC_NO_CONNECTED_NUMBER      = 3						///< The numbere IEI doesn't exist
} PresentationInd_t;

/// Enum : Screening Indicator
typedef enum
{
	CC_NOT_SCREENED				= 0,					///< User Provided, Not Screened
	CC_VERIFIED_AND_PASSED		= 1,					///< User Provided, Verified And Passed
	CC_VERIFIED_AND_FAILED		= 2,					///< User Provided, Verified And Failed
	CC_NETWORK_PROVIDED			= 3						///< Network Provided
} ScreenInd_t;

/// Structure : Party Number
typedef struct
{
	TypeOfNumber_t	ton;								///< Type Of Number Type
	NumberPlanId_t	npi;								///< Number Plan Identification Type
	UInt8			length;								///< Length
	UInt8			number[MAX_PHONE_NUMBER_SIZE];		///< Phone Number
} PartyAddress_t;										///< Party Address

/// Structure : Party Sub Add
typedef struct
{
	TypeOfSubAdd_t	tos;								///< Type Of Subaddress
	OddEven_t		oddEven;							///< Odd Even Type
	UInt8			length;								///< Length
	UInt8			number[MAX_PHONE_SUBADDRESS_SIZE];	///< Phone Subaddress Number
} PartySubAdd_t;										///< Party Sub Address

/// Structure : Called/ing Party Number
typedef struct
{
	PresentationInd_t	presentInd;						///< Presentation Indicator
	ScreenInd_t			screenInd;						///< Screen Indicator
	PartyAddress_t		partyAdd;						///< Party Address
} CallPartyAdd_t;										///< Called/ing Party Address

/// Calling Name
typedef struct
{
	UInt8 data_cod_scheme;		///< Data Coding Schemes
	UInt8 length_in_char;		///< length
	UInt8 name_size;			///< Size of the name
	UInt8 name[MAX_NAME_STRING_SIZE+1];///< Array of the name
} CallingName_t;

/// Calling Information
typedef struct
{
   PresentStatus_t   present;		///< Call Line ID presentation
   CallingName_t	 name_info;		///< Calling name
   //TelephoneNumber_t number_info;
} CallingInfo_t;


#define SUB_LENGTH		 20		///< Length of sub address

/// Sub Address
typedef struct
{
	UInt8 		tos;
	UInt8 		odd_even;
	UInt8 		c_subaddr;
	UInt8 		subaddr[ SUB_LENGTH ];
} Subaddress_t;

/// USSD String Structure
typedef struct
{
	UInt8			used_size;
	UInt8			string[PHASE2_MAX_USSD_STRING_SIZE+1];
	UInt8			dcs;  // (dcs&0x0F)>>2: 0-Default Alphabet; 1-8bit; 2-UCS2
} USSDString_t;

#define	DTMF_TONE_ARRAY_SIZE 15	
#define	MNCC_DTMF_LIST_SIZE	300												///< The DTMF list size

//Note: MNCC_MAX_DTMF_TONE_DURATION shall not
//be bigger than MNCC_START_DTMF_TIMER_T336
#define MNCC_DTMF_TONE_TICK			   10							///<  10 milli seconds
#define MNCC_MIN_DTMF_TONE_DURATION	(  10 * MNCC_DTMF_TONE_TICK)	///< 100 milli seconds
#define MNCC_MAX_DTMF_TONE_DURATION	( 900 * MNCC_DTMF_TONE_TICK)	///< 9 Seconds
#define	MNCC_INVALID_DTMF_TONE_DURATION   0						///< used for DTMF_START_STOP method only 
#define	MNCC_START_DTMF_TIMER_T336	(1000 * MNCC_DTMF_TONE_TICK)	///< T336 = 10 Seconds
#define	MNCC_STOP_DTMF_TIMER_T337	(1000 * MNCC_DTMF_TONE_TICK)	///< T337 = 10 Seconds
#define MNCC_DTMF_CALLSETUP_PAUSE_DURATION (300 * MNCC_DTMF_TONE_TICK) ///< 3 seconds as per 3GPP 22.101 rel 6
typedef UInt8 DTMFTone_t;				///< 0 - 9,
										///< A - D, 'p' for Pause
										///< STAR(*) = 15,
										///< HASH(#) = 16


/// Enum :	DTMF Methode type
typedef enum
{
	DTMF_METHOD_NONE,					///< No particular DTMF methode.
	DTMF_METHOD_START_ONLY,				///< If at the API level the client just calls CcApi_StartDtmf
	DTMF_METHOD_START_STOP				///< If at the API level the client insequence calls CcApi_StartDtmf/CcApiStopDtmf
} DtmfMethod_t;							///< DTMF methode type


/// Enum :	DTMF Timer type
typedef enum
{
	DTMF_TONE_DURATION_TYPE,			///< This timer is used for the duration of the DTMF tone.
	DTMF_WAIT_DURATION_TYPE,			///< This timer is used for the pause or wait duration (P/W)
	DTMF_PERIODIC_PAUSE_TYPE			///< This timer is used for the periodic pause between each tone.
} DtmfTimer_t;							///< DTMF timer type


/// Enum :	DTMF Status type
typedef enum
{
	DTMF_STATE_IDLE,					///< In this stat there is no DTMF activities
	DTMF_STATE_SEND_PEND,				///< In this stat the start DTMF is sent to the nw and waiting for NW ack
	DTMF_STATE_SEND,					///< In this state the ack is receive but we wait for tone duration timer to expire
	DTMF_STATE_STOP_PEND,				///< In this stat the stop DTMF is sent to the nw and waiting for NW ack
	DTMF_STATE_STOP						///< In this state we wait for periodically pause timer to expire.
} DtmfState_t;							///< DTMF State type


/// Structure :	API DTMF type
typedef struct
{
	UInt8		callIndex;				///< The call index of the call session
	UInt16		dtmfIndex;				///< The DTMF tone index
	DTMFTone_t	dtmfTone;				///< DTMF Tone
	UInt8		toneValume;				///< DTMF tone volume
	Boolean		isSilent;				///< Silent Flag
	UInt32		duration;				///< Duration of the DTMF tone [MiliSecond]
} ApiDtmf_t;							///< API DTMF type


/**
Enum :	Coding group 0: "Data Codin Scheme GSM 3.38, section 5"
		Coding group 0 : CODING_GROUP_0_LANGUAGE_GSM_7_BIT_ALPHABET
**/
typedef enum
{
	LANGUAGE_GERMAN,							///<  0. German
	LANGUAGE_ENGLISH,							///<  1. English
	LANGUAGE_ITALINA,							///<  2. Italian
	LANGUAGE_FRENCH,							///<  3. French
	LANGUAGE_SPANISH,							///<  4. Spanish
	LANGUAGE_DUTCH,								///<  5. Dutch
	LANGUAGE_SWEDISH,							///<  6. Swedish
	LANGUAGE_DANISH,							///<  7. Danish
	LANGUAGE_PORTUGUESE,						///<  8. Portuguese
	LANGUAGE_FINNISH,							///<  9. Finnish
	LANGUAGE_NORWEGIAN,							///< 10. Norwegian
	LANGUAGE_GREEK,								///< 11. Greek
	LANGUAGE_TURKISH,							///< 12. Turkish
	LANGUAGE_HUNGARIAN,							///< 13. Hungarian
	LANGUAGE_POLISH								///< 14. Polish
} CodingGroup0_t;								///< Coding Group 0 Type


/**
Enum :	Coding Croup 1: "Data Codin Scheme GSM 3.38, section 5"
		Coding group 1 : CODING_GROUP_1_MESSAGE_PROTECTED_BY_LANGUAGE_IND

		0000	Default alphabet; message preceded by language indication. 
				The first 3 characters of the message are a two-character
				representation of the language encoded according to ISO 639 [12],
				followed by a CR character. The CR character is then followed by
				90 characters of text. A Pre-Phase 2+ MS will overwrite the start
				of the message up to the CR and present only the text.

		0001	UCS2; message preceded by language indication
				The message starts with a two 7-bit default alphabet character
				representation of the language encoded according to ISO 639 [12].
				This is padded to the octet boundary with two bits set to 0 and
				then followed by 40 characters of UCS2-encoded message.

				An MS not supporting UCS2 coding will present the two character
				language identifier followed by improperly interpreted user data.

		0010..1111	Reserved for European languages
**/
typedef enum
{
	GSM_7_BIT_DEFAULT_ALPHABET,					///< GSM 7 bit default alphabet
	UNIVERSAL_CHARACTER_SET_2					///< UCS2
} CodingGroup1_t;								///< Coding Group 0 Type


/**
Enum :	Coding Croup 2: "Data Codin Scheme GSM 3.38, section 5"
		Coding group 2 : CODING_GROUP_2_LANGUAGE_GSM_7_BIT_ALPHABET

		0000	Czech
		0001	Hebrew
		0010	Arabic
		0011	Russian
		0100	Icelandic

		0001 .. 1111 Reserved for European Languages using the default alphabet,
				with unspecified handling at the MS
**/
typedef enum
{
	LANGUAGE_CZECH,								///< Czech
	LANGUAGE_HEBREW,							///< Hebrew
	LANGUAGE_ARABIC,							///< Arabic
	LANGUAGE_RUSSIAN,							///< Russian
	LANGUAGE_ICELANDIC							///< Icelandic
} CodingGroup2_t;								///< Coding Group 2 Type


/**
Enum :	Message Class: "Data Codin Scheme GSM 3.38, section 5"
		Coding group 4 : CODING_GROUP_4_GENERAL_TEXT_UNCOMPRESSED
		Coding group 5 : CODING_GROUP_5_GENERAL_TEXT_UNCOMPRESSED
		Coding group 6 : CODING_GROUP_6_GENERAL_TEXT_COMPRESSED
		Coding group 7 : CODING_GROUP_7_GENERAL_TEXT_COMPRESSED
		Coding group 9 : CODING_GROUP_9_MSG_WITH_USER_DATA_HEADER
		Coding group F : CODING_GROUP_F_DATA_CODING_MSG_HANDLING
**/
typedef enum
{
	MESSAGE_CLASS_0,							///< Class 0
	MESSAGE_CLASS_1,							///< Class 1 ME Specific/User Defined
	MESSAGE_CLASS_2,							///< Class 2 (U)SIM Specific/User Defined
	MESSAGE_CLASS_3								///< Class 3 TE-Specific(3GPP TS 27.005)
} MessageClass_t;								///< Message Class



/**
Enum :	Message Coding: "Data Codin Scheme GSM 3.38, section 5"
		Coding group F : CODING_GROUP_F_DATA_CODING_MSG_HANDLING
**/
typedef enum
{
	MESSAGE_CODING_GSM_7_BIT_DEFAULT_ALPHABET,	///< GSM 7 bit default alphabet
	MESSAGE_CODING_GSM_8_BIT_DATA				///< GSM 8 bit data
} MsgCoding_t;									///< Message Coding Type



typedef struct
{
	ApiDtmf_t	dtmfObj;				///< DTMF object
	DtmfState_t	dtmfState;				///< DTMF state
	Result_t	cause;					///< Cause of the event
} ApiDtmfStatus_t;						///< API DTMF status type

typedef UInt32 Duration_t;				///< duration of call in Seconds
typedef UInt32 CallMeterUnit_t;			///< General Call Meter Unit
typedef CallMeterUnit_t ACM_t;			///< Accumulated Call Meter


/// Registration status of GSM and GPRS services 
typedef enum
{
	REGISTERSTATUS_NORMAL,			///< (GSM) Normal service
									///< (GPRS) GPRS service available and attached
	REGISTERSTATUS_LIMITED,			///< (GSM) Emergency calls only
									///< (GPRS) GPRS service available but detached
	REGISTERSTATUS_NO_ACCESS,		///< (GSM) No access (non-fatal reject from network or lower layer failure)
									///< (GPRS) N/A
	REGISTERSTATUS_NO_CELL,			///< (GSM/GPRS) No cell (temporary loss of network)
	REGISTERSTATUS_NO_SERVICE,		///< (GSM) N/A
									///< (GPRS) GPRS service not available
	REGISTERSTATUS_NO_NETWORK,		///< (GSM/GPRS) No Network available
	REGISTERSTATUS_INVALID_SIM,		///< (GSM/GPRS) Invalid SIM
	REGISTERSTATUS_INVALID_ME,		///< (GSM/GPRS) Illegal ME or MS
	REGISTERSTATUS_SERVICE_DISABLED, ///< (GSM/GPRS) Service disabled by higher layer (SIM detach, power down, detach req)
									 ///<            or by network (network initiated detach)
    REGISTERSTATUS_LU_TEMP_BARRED,     ///< (GSM/GPRS) Registeration temporary barred

	REGISTERSTATUS_NO_STATUS,		///< (GSM/GPRS) No status info; it can be ignored
	REGISTERSTATUS_NO_ACCESS_MAX_ATTEMPT,	///< (GSM/GPRS) Registration failure due to attempt counter of LU max (4)
	REGISTERSTATUS_REJ_BY_NW_MAX_ATTEMPT,	///< (GPRS) max attempt counter to 5 after GPRS reject by network (for RAU or GPRS Attach)
	REGISTERSTATUS_INTERNAL_FAILURE_MAX_ATTEMPT, ///< (GPRS) max attempt counter (RAU or GPRS Attach) to 5 after internal registration failure
	REGISTERSTATUS_NOT_REG_SEARCHING = 0xFF
} RegisterStatus_t;					///< MSC-->MMI : Registration status



/// PLMN structure
typedef struct
{
	UInt16 mcc;			///< Mobile Country Code (3-digits) and may include the 3rd digit of MNC (Example: 0x13F0 or 0x1300 for Cingular)
	UInt8  mnc;			///< Mobile Network Code (2-digits) (Example: 0x71 for Cingular)
	UInt8  order;		///< the order number of operator in SIM preferred operator list
} PLMN_ID_t;
// PLMN Identification

/// Call Type
typedef enum
{
	CALLTYPE_SPEECH,					///< Call is a Speech Call
	CALLTYPE_SUPPSVC,					///< Call is a Call-Independent Supplementary Service
	CALLTYPE_DATA,						///< Call is a Data Call, Data
	CALLTYPE_FAX,						///< Call is a Data Call, Fax
	CALLTYPE_MOUSSDSUPPSVC,				///< Call is a MO USSD Service
	CALLTYPE_MTUSSDSUPPSVC,				///< Call is a MT USSD Service
	CALLTYPE_GPRS,						///< Call is a GPRS call
	CALLTYPE_PPP_LOOPBACK,				///< Call is a PPP loopback test
	CALLTYPE_VIDEO,						///< Call type Video - used by Wedge
	CALLTYPE_UNKNOWN					///< Call type is unknown
} CallType_t;

/// Call status
typedef enum
{
	CALLSTATUS_CALLING,					///< 00, MO Call is Calling
	CALLSTATUS_ACTIVE,					///< 01, a Call is Active
	CALLSTATUS_HELD,					///< 02, a Call is Held
	CALLSTATUS_HANGUP,					///< 03, a Call is Hangup
	CALLSTATUS_MO_CI_ALLOC,				///< 04, Allocated a MO CallIndex
	CALLSTATUS_MT_CI_ALLOC,				///< 05, Allocated a MT CallIndex

	CALLSTATUS_ERR_CALL_BARRED,			///< 06, Error, Call is Barred (not used)
	CALLSTATUS_ERR_NETWORK_BUSY,		///< 07, Error, Network is Busy (not used)
	CALLSTATUS_ERR_NO_SERVICE,			///< 08, Error, No Service is Available (not used)
	CALLSTATUS_ERR_NO_NETWORK,			///< 09, Error, No Network

	CALLSTATUS_IDLE,					///< 0A,
	CALLSTATUS_ALERTING,				///< 0B, MO Call, Network is Alerting destination
	CALLSTATUS_MPTY_ACTIVE,				///< 0C, MPTY Call is Active
	CALLSTATUS_MPTY_HELD,				///< 0D, MPTY Call is Held 
	CALLSTATUS_CONNECTED,				///< 10, MO Call Connected, but not Active
	CALLSTATUS_VERIFIED,				///< 11, MO Call, UserId and Password is verified
	CALLSTATUS_USER_BUSY,				///< 12, MO Call, Called party busy
    CALLSTATUS_NETDISCONNECT,			///< 13, MO Call, netowrk reject with DISC_IND
    CALLSTATUS_PROCEEDING, 				///< 14, MO Call, network sends Call Proceeding
	CALLSTATUS_SYNC_IND                 ///< 15, MO/MT Call, networks sends Sync Ind
} CallStatus_t;							///< MSC-->MMI : call status

/// Call Action
typedef enum
{
	CALLACTION_ACCEPT,					///<  00, Accept the a Call
	CALLACTION_HANGUP,					///<  01, Hangup a Call
	CALLACTION_HOLD,					///<  02, Hold Active Call
	CALLACTION_RETRIEVE,				///<  03, Retrieve a Held Call
	CALLACTION_JOIN,					///<  04, Join two Calls
	CALLACTION_SPLIT,					///<  05, Split a Joined Call

	CALLACTION_SWAP,					///<  06, Swap an Active Call with a Held Call
	CALLACTION_END,						///<  07, End All Calls
	CALLACTION_ENDMPTY,					///<  08, End MPTY call
	CALLACTION_ECT,						///<  09, Explicit Call Transfer
	CALLACTION_START_DTMF_0,			///<  0A, Start generating DTMF Tone for 0
	CALLACTION_START_DTMF_1,			///<  0B, Start generating DTMF Tone for 1
	CALLACTION_START_DTMF_2,			///<  0C, Start generating DTMF Tone for 2
	CALLACTION_START_DTMF_3,			///<  0D, Start generating DTMF Tone for 3
	CALLACTION_START_DTMF_4,			///<  0E, Start generating DTMF Tone for 4
	CALLACTION_START_DTMF_5,			///<  0F, Start generating DTMF Tone for 5
	CALLACTION_START_DTMF_6,			///<  10, Start generating DTMF Tone for 6
	CALLACTION_START_DTMF_7,			///<  11, Start generating DTMF Tone for 7
	CALLACTION_START_DTMF_8,			///<  12, Start generating DTMF Tone for 8
	CALLACTION_START_DTMF_9,			///<  13, Start generating DTMF Tone for 9
	CALLACTION_START_DTMF_POUND,		///<  14, Start generating DTMF Tone for #
	CALLACTION_START_DTMF_STAR,			///<  15, Start generating DTMF Tone for *
	CALLACTION_STOP_DTMF,				///<  16, Stop generating DTMF Tone
	CALLACTION_START_DIAL_TONE,			///<  17, Start generating Dial Tone
	CALLACTION_STOP_DIAL_TONE,			///<  18, Stop generating Dial Tone
	CALLACTION_START_DTMF_0_SILENT,		///<  19, Start generating DTMF Tone for 0 but silent on speaker
	CALLACTION_START_DTMF_1_SILENT,		///<  1A, Start generating DTMF Tone for 1 but silent on speaker
	CALLACTION_START_DTMF_2_SILENT,		///<  1B, Start generating DTMF Tone for 2 but silent on speaker
	CALLACTION_START_DTMF_3_SILENT,		///<  1C, Start generating DTMF Tone for 3 but silent on speaker
	CALLACTION_START_DTMF_4_SILENT,		///<  1D, Start generating DTMF Tone for 4 but silent on speaker
	CALLACTION_START_DTMF_5_SILENT,		///<  1E, Start generating DTMF Tone for 5 but silent on speaker
	CALLACTION_START_DTMF_6_SILENT,		///<  1F, Start generating DTMF Tone for 6 but silent on speaker
	CALLACTION_START_DTMF_7_SILENT,		///<  2A, Start generating DTMF Tone for 7 but silent on speaker
	CALLACTION_START_DTMF_8_SILENT,		///<  2B, Start generating DTMF Tone for 8 but silent on speaker
	CALLACTION_START_DTMF_9_SILENT,		///<  2C, Start generating DTMF Tone for 9 but silent on speaker
	CALLACTION_START_DTMF_POUND_SILENT,	///<  2D, Start generating DTMF Tone for # but silent on speaker
	CALLACTION_START_DTMF_STAR_SILENT, 	///<  2E, Start generating DTMF Tone for * but silent on speaker
	CALLACTION_START_RING_TONE,			///<  2F, start ringing tone
	CALLACTION_STOP_RING_TONE,			///<  30, stop ringing tone
    CALLACTION_HANGUP_IMMEDIATE,        ///<  31, Hang up a call immediately
	CALLACTION_NONE						///<  32, None
} CallAction_t;							///<  MMI-->MSC : call action

/// Data Call Mode
typedef enum
{
	DATACALLMODE_DISALLOW,				///<  Data Calls are Disallowed
	DATACALLMODE_ALLOW					///<  Data Calls are Allowed
} DataCallMode_t;


#define	CUGINDEX_NONE		0xFFFF		///<  Defines no CUG index
#define	MAX_CUGINDEX		0x7FFF		///<  Defines Maximum value for CUG index
#define	MIN_CUGINDEX		0x0000		///<  Defines Minimum value for CUG index
typedef UInt16 CUGIndex_t;				///<  CUG Index value

/// Closed user Group Suppress Preference
typedef enum
{
	CUGSUPPRESSPREFCUG_DISABLE,			///<  Suppress Preferential CUG Indicator not sent
	CUGSUPPRESSPREFCUG_ENABLE			///<  Suppress Preferential CUG Indicator sent
} CUGSuppressPrefCUG_t;

/// CUG Suppress
typedef enum
{
	CUGSUPPRESSOA_DISABLE,				///<  Suppress OA Indicator not sent
	CUGSUPPRESSOA_ENABLE				///<  Suppress OA Indicator sent
} CUGSuppressOA_t;

/// Closed User Group Information
typedef struct
{
	CUGIndex_t				cug_index;			///<  CUG Index value
	CUGSuppressPrefCUG_t	suppress_pref_cug;	///<  Suppress Preferential CUG Indicator
	CUGSuppressOA_t			suppress_oa;		///<  Suppress Outside Access Indicator
} CUGInfo_t;									///<  Closed User Group (CUG)

/// Caller Line ID Restriction
typedef enum
{
	CLIRMODE_DEFAULT,					///<  Default Caller Line Indentification
	CLIRMODE_INVOKED,					///<  CLIR Invoked, Called Party doesn't see Calling Party's CLI
	CLIRMODE_SUPPRESSED					///<  CLIR Suppressed, Called Party see Calling Party's CLI
} CLIRMode_t;

#define MAX_BEARER_CAP_LEN  16			///< Maximum Bearer Capability Length
#define MAX_LLC_LEN			18			///< Maximum Lower Layer Capability Length
#define MAX_HLC_LEN			5			///< Maximum Higher Layer Capability Length
#define MAX_SUPPORTED_CODEC_LIST_LEN 10	///< currently only two technologies are supported, we will have 4 octets for each technology , one for IEI , one for length of the IE


/// Bearer Capability of a data call
typedef struct
{
	UInt8			val[MAX_BEARER_CAP_LEN];  ///< Release 99 BC could be upto 16 bytes.
} BearerCapability_t;

/**
Enum:	This enum is used for repeat indicator description. 
		refer 3gpp 24.008 section 10.5.4.22
*/
typedef enum 
{
	RI_NOT_PRESENT				= 0,	///< Repeat indicator not presented
	RI_CIRCULAR					= 1,	///< Repeat indicator Circular
	RI_SUP_FALLBACK_MODE1_PREF	= 2,	///< Repeat indicator Sup Fallback mode 1 prefered
	RI_SEQUENTIAL				= 3,	///< Repeat indictor Sequvential
	RI_SRV_CHANGE_FALLBACK_ALT	= 4		///< Repeat indictor Service change fallback alternated
} RepeatInd_t;

/**
Structure:	Bearer capability structure used to notify the bearer capability 
			used for the call
*/
typedef struct
{
	BearerCapability_t	Bc1;			///< bearercapability1  
	BearerCapability_t	Bc2;			///< bearercapability2
	RepeatInd_t 		RepeatInd;		///< repeat indicator
}CC_BearerCap_t;						///< Structure containing bearer capability related parameters.

/// Supplementary Service Type
typedef enum
{
	SUPPSVCTYPE_NONE,					///< 00, no SS
	SUPPSVCTYPE_CLIP,					///< 01, Calling Line ID Presentation
	SUPPSVCTYPE_CLIR,					///< 02, Calling Line ID Restriction
	SUPPSVCTYPE_COLP,					///< 03, Connected Line ID Presentation
	SUPPSVCTYPE_COLR,					///< 04, Connected Line ID Restriction
	SUPPSVCTYPE_CFU,					///< 05, Call Forwarding Unconditional
	SUPPSVCTYPE_CFB,					///< 06, Call Forwarding when Busy
	SUPPSVCTYPE_CFNRY,					///< 07, Call Forwarding on No Reply
	SUPPSVCTYPE_CFNRC,					///< 08, Call Forwarding on Not Reachable
	SUPPSVCTYPE_CF_ALL,					///< 09, Call Forwarding All
	SUPPSVCTYPE_CF_ALL_COND,			///< 0A, Call Forwarding All on Conditional
	SUPPSVCTYPE_CW,						///< 0B, Call Waiting
	SUPPSVCTYPE_BAR_PASSWORD,			///< 0C, Password Change requested
	SUPPSVCTYPE_BAOC,					///< 0D, Barring of All Outgoing Calls
	SUPPSVCTYPE_BOIC,					///< 0E, Barring Outgoing International Calls
	SUPPSVCTYPE_BOIC_EX_HC,				///< 0F, Barring Outgoing International Calls Excluding Home PLMN Country
	SUPPSVCTYPE_BAIC,					///< 10, Barring All Incoming Calls
	SUPPSVCTYPE_BAIC_ROAM,				///< 11, Barring All Incoming Calls when Roaming outside Home PLMN Country
	SUPPSVCTYPE_BAC,					///< 12, Barring of All Calls
	SUPPSVCTYPE_BOC,					///< 13, Barring of Outgoing Calls
	SUPPSVCTYPE_BIC,					///< 14, Barring of Incoming Calls
	SUPPSVCTYPE_ECT,					///< 15, Explicit Call Transfer
	SUPPSVCTYPE_USSD,					///< 16, Unstructure Supplementary Service Data
	SUPPSVCTYPE_NHOLD,					///< 17, Network Status of a Call on Hold
	SUPPSVCTYPE_NRETRIEVE,				///< 18, Network Status of a Retrieve Call
	SUPPSVCTYPE_CNAP,					///< 19, Calling NAme Presentation
	SUPPSVCTYPE_KT_CLIR 				///< 20, CLIR for KT and SKT only and used internal to CAPI.


} SuppSvcType_t;						///< MSC-->MMI : call-indep. service type

/// Supplementary Service Status
typedef struct
{
	SuppSvcType_t	ssSvcType;
	UInt8			cfgParamValue;
} SS_IntParSetInd_t;

typedef enum
{
	SUPPSVCSTATUS_DEACTIVATED,				///< Service deactivation succeeded
	SUPPSVCSTATUS_ACTIVATED,				///< Service activation succeeded
	SUPPSVCSTATUS_UNKNOWN_SUBSCRIBER,		///< Unknown subscriber, action did not succeed.
	SUPPSVCSTATUS_ILLEGAL_SUBSCRIBER,		///< Illegal subscriber - service not available
	SUPPSVCSTATUS_BS_NOTPROVISIONED,		///< Bearer Service requested is not proviosned, request failed.
	SUPPSVCSTATUS_TS_NOTPROVISIONED,		///< Tele Service requested is not proviosned, request failed.
	SUPPSVCSTATUS_ILLEGAL_EQUIPMENT,		///< Illegal ME / SIM.
	SUPPSVCSTATUS_CALL_BARRED,				///< This type of call has been barred
	SUPPSVCSTATUS_ILLEGAL_SSOPERATION,		///< Service failed - illegal SS operation.
	SUPPSVCSTATUS_SS_ERRORSTATUS,			///< Returned error status - request failed.
	SUPPSVCSTATUS_NOT_AVAILABLE,		    ///< Service isn't available
	SUPPSVCSTATUS_SS_SUBSCRIPTION_VIOLATION, ///< This service has not been subscribed.
	SUPPSVCSTATUS_SS_INCOMPATIBILITY,		///< Service incompatible, request failed.
	SUPPSVCSTATUS_FACILITY_NOT_SUPPORTED,	///< Requested facility is not supported
	SUPPSVCSTATUS_ABSENT_SUBSCRIBER,		///< Subscriber is absent
	SUPPSVCSTATUS_SHORTTERM_DENIAL,			///< Short term denial of service, need to try again later.
	SUPPSVCSTATUS_LONGTERM_DENIAL,			///< Long term denial of service
	SUPPSVCSTATUS_SYSTEM_FAILURE,			///< System failure
	SUPPSVCSTATUS_DATA_MISSING,				///< Data is incomplete in the request
	SUPPSVCSTATUS_UNEXPECTED_DATA_VALUE,	///< Unexpected data in the request. Check request and try again.
	SUPPSVCSTATUS_PW_REGISTRATION_FAILURE,	///< Password registration failure
	SUPPSVCSTATUS_NEGATIVEPW_CHECK,			///< Password check negative
	SUPPSVCSTATUS_NUMBEROFPW_ATTEMPT_VIOLATION,///< Exceeded allowed number of password check attempts.
	SUPPSVCSTATUS_UNKNOWN_ALPHABET,			///< Unknown alphabet in request
	SUPPSVCSTATUS_USSD_BUSY,				///< Unstructured SS Data busy.
	SUPPSVCSTATUS_REJECTED_BY_USER,			///< Request/result rejected by user.
	SUPPSVCSTATUS_REJECTED_BY_NETWORK,		///< Request rejected by network
	SUPPSVCSTATUS_DEFLECTION_TO_SERVED_SUBSCRIBER,///< Service deflection to served subscriber.
	SUPPSVCSTATUS_SPECIAL_SERVICE_CODE,		///< Special Service Code
	SUPPSVCSTATUS_INVALID_DEFLECTED_TO_NUMBER,///< Call deflected to an invalid number
	SUPPSVCSTATUS_MAX_NUMBER_OF_MPTY_EXCEEDED,///< Exceeded the number of multi party calls
	SUPPSVCSTATUS_RESOURCE_NOT_AVAILABLE,	///< Resources are unavailable
	SUPPSVCSTATUS_FAILED,					///< Service request failed
	SUPPSVCSTATUS_USSD_RESEND,				///< Resend USSD request.
	SUPPSVCSTATUS_ABORT,					///< Abort on going request
 	SUPPSVCSTATUS_POSITION_METHODE_FAILURE,	///< Position method failure
 	SUPPSVCSTATUS_NBR_SB_EXCEEDED,			///< Number of subscribers exceeded
	SUPPSVCSTATUS_NULL						///< NULL Supplementary service
} SuppSvcStatus_t;							///< MSC-->MMI : call-indep. service result

/// Supplementary Service Codes
typedef enum
{
	SUPPSVCBSCODE_ALL_BEARER_TELESERVICE 		= 0,///< not specified in GSM 2.30 Spec.
													// but 'C' needs a value
	SUPPSVCBSCODE_NOT_DECODED					= 1,///< Basic Service was not decoded
													// use 'bs_raw_list' value todetermine the Basic Service
	SUPPSVCBSCODE_ALL_TELESERVICE 				= 10,///< All Teleservices(speech,fax, SMS,data) 
	SUPPSVCBSCODE_ALL_SPEECH 					= 11,///< All services specific to speech calls
	SUPPSVCBSCODE_ALL_DATA_TELESERVICE 			= 12,///< All Services specific to Data calls
	SUPPSVCBSCODE_ALL_FAX_SERVICE 				= 13,///< All services specific to Fax calls
	SUPPSVCBSCODE_ALL_SHORT_MESSAGE_SERVICE 	= 16,///< All services specific to SMS
	SUPPSVCBSCODE_ALL_TELESERVICE_EXCEPT_SMS 	= 19,///< All teleservices except SMS(speech,data,fax)
	SUPPSVCBSCODE_ALL_BEARER_SERVICE 			= 20,///< All Bearer services( Data and Packet services)
	SUPPSVCBSCODE_ALL_ASYNC_SERVICE 			= 21,///< All asynchronous services
	SUPPSVCBSCODE_ALL_SYNC_SERVICE 				= 22,///< All synchronous services
	SUPPSVCBSCODE_ALL_DATA_CIRCUIT_SYNC 		= 24,///< Data Services synchronous(General and Data circuit duplex)
	SUPPSVCBSCODE_ALL_DATA_CIRCUIT_ASYNC 		= 25,///< Data Services asynchronous(General and Data circuit duplex)
	SUPPSVCBSCODE_ALL_DEDICATED_PACKET_ACCESS 	= 26,///< Dedicated Packet services(Data Packet duplex and General Packet Service) 
	SUPPSVCBSCODE_ALL_DEDICATED_PAD_ACCESS 		= 27,///< Genral PAD access and all PAD data services
	SUPPSVCBSCODE_ALL_DATA_CDA_SERVICES 		= 28,///< All Data CDA Services
	SUPPSVCBSCODE_ALL_DATA_CDS_SERVICES 		= 29,///< All Data CDS Services
	SUPPSVCBSCODE_ALL_ALT_SPEECH_DATA_CDA_SRVS 	= 30,///< All Alternate Speech Data CDA Services
	SUPPSVCBSCODE_ALL_ALT_SPEECH_DATA_CDS_SRVS	= 31,///< All Alternate Speech Data CDS Services
	SUPPSVCBSCODE_ALL_SPEECH_FOLLOW_BY_DATA_CDA = 32,///< All Speech Followed By Data CDA Services
	SUPPSVCBSCODE_ALL_SPEECH_FOLLOW_BY_DATA_CDS = 33,///< All Speech Followed By Data CDS Services

	// See GSM 2.30 for all PLMN Spec. values
	SUPPSVCTSCODE_ALL_PLMN_SPECIFIC_TELESERVICE = 50,	///< PLMN specific Tele Services. See GSM 2.30
	SUPPSVCBSCODE_ALL_PLMN_SPECIFIC_BEARER_SERVICE = 70,///< PLMN specific Bearer Services. See GSM 2.30

	/* Teleservice code for ALS (Alternative Line Service) added to GSM 02.30, see Appendix 1 of CPHS spec */
	SUPPSVCBSCODE_ALL_AUXILIARY_TELEPHONY = 89///< Auxillary telephony
} SuppSvcBSCode_t;							

/// SS Basic Service Codes
typedef struct
{
	Boolean is_used;			///< TRUE if the user specifies a Supplementary Service class
	SuppSvcBSCode_t bs_code;	///< Basic code corresponding to the Supplementary Service class 
								///< selected, relevant only if "is_used" is TRUE. 
} SuppSvcUsedBSCode_t;

/// SS Basic Service type
typedef enum
{
	SUPPSVCBSTYPE_ALL_SERVICE = 0,		///< All Services
	SUPPSVCBSTYPE_BEARER_SERVICE = 2,	///< Bearer Services(Data and Packet services) 
	SUPPSVCBSTYPE_TELESERVICE = 3		///< Tele Services(Speech,data,fax,SMS)
} SuppSvcBSType_t;

typedef UInt8 RawSSStatus_t;				///< Raw SS Status Type

/// SS Basic Services Raw Codes
typedef struct
{
	SuppSvcBSType_t bs_type;		///< Supplementary Services Bearer Services Type
	UInt8 bs_value;					///< Bearer Services Value
} SuppSvcBSRawCode_t;

/// Basic Services Group 
typedef struct
{
	UInt8 list_sz;							///< specifies the number of valid entries in list
	SuppSvcBSCode_t bs_decoded_list[13];	///< Decoded list of entries
	SuppSvcBSRawCode_t bs_raw_list[13];		///< Raw list as returned by the network
} BasicServiceGroupList_t;

/// Call forward feature
typedef struct
{
	SuppSvcStatus_t		ss_status;			///< Supplementary Service Status		
	RawSSStatus_t 		raw_ss_status;		///< Supplementary Service Raw Status
	SuppSvcBSCode_t 	bs_decoded;			///< Decoded Bearer Service Values
	SuppSvcBSRawCode_t 	bs_raw;				///< Raw bearer values
	TelephoneNumber_t 	forwarded_to_number;///< Number to which the call has been forwarded
	UInt8				noReplyTime;		///< 5 <= No Reply time <= 30
} FFeature_t;

/// Call forward feature list
typedef struct
{
	UInt8 list_sz;			///< specifies the number of valid entries in list
	Boolean bs_code_used;   ///< True if the call forward request specifies a specific Service Class
	FFeature_t ffeature_list[13];///< Call fwd feature
} FFeatureList_t;

/// Caller ID Restriction 
typedef enum
{
	CLIRESTRICTIONOPTION_PERMANENT = 0,		///< Calling Line ID Restriction Option Permamnent
	CLIRESTRICTIONOPTION_TEMPORARY_DEFAULT_RESTRICTED = 1,///< CLIR temporary mode presentation restricted
	CLIRESTRICTIONOPTION_TEMPORARY_DEFAULT_ALLOWED = 2	  ///< CLIR temporary mode presentation allowed
} CLIRestrictionOption_t;

/// Calling Line ID Restriction Information Structure
typedef struct
{
	Boolean					isCliRestOpt;		///< CLIR mode provisioned - TRUE / FALSE
	CLIRestrictionOption_t 	cli_restriction_opt;
	RawSSStatus_t 			raw_ss_status;		///< Raw SS status
} CLIRestrictionInfo_t;

/// USSD Service
typedef enum
{
	USSD_REQUEST,							///< USSD Request
	USSD_NOTIFICATION,						///< USSD Notification
	USSD_FACILITY_RETURN_RESULT,			///< USSD Requested facility returned a result
	USSD_FACILITY_RETURN_ERROR,				///< USSD Request returns an Error
	USSD_FACILITY_REJECT,					///< USSD facility rejected. Check request
	USSD_RELEASE_COMPLETE_RETURN_RESULT,	///< Request processed with a result, release is complete.
	USSD_RELEASE_COMPLETE_RETURN_ERROR,		///< Release complete, returning an error
	USSD_RELEASE_COMPLETE_REJECT,			///< Facility rejected, release complete
	USSD_RESEND,							///< Request to re-send USSD request
	USSD_REGISTRATION						///< Register USSD request
}USSDService_t;

/// USSD Data
typedef struct
{
	USSDService_t	service_type;	///< USSD service type

	UInt8			oldindex;		///<Old USSD index, used for RESEND
	UInt8			newindex;		///< New USSD index

	UInt8			prob_tag;		///<for REJ
	UInt8			prob_code;		///< 

	SuppSvcStatus_t	err_code;		///<Error status

	Unicode_t		code_type;		///<UNICODE_GSM & UNICODE_UCS1: nonunicode, UNICODE_80: unicode
	UInt8			dcs;			///<Data Coding Scheme
	UInt8			used_size;		///<Length of the data payload
	UInt8			string[PHASE1_MAX_USSD_STRING_SIZE+1];  ///< USSD data buffer
}USSDData_t;

/// Supplementary Service Information
typedef enum
{
	SSINFOUSE_DO_NOT_USE,					///< don't use the 'ss_info' union, data is invalid
	SSINFOUSE_RAW_SS_STATUS,				///< use 'raw_ss_status' in 'ss_info' union
	SSINFOUSE_FORWARDED_TO_NUMBER,			///< use 'forward_to_number' in 'ss_info' union
	SSINFOUSE_BS_GROUP_LIST,				///< use 'bs_group_list' in 'ss_info' union
	SSINFOUSE_FFEATURE_LIST,				///< use 'ffeature_list' in 'ss_info' union
	SSINFOUSE_CLI_RESTRICTION_INFO,			///< use 'cli_restriction_info' in 'ss_info' union
	SSINFOUSE_USSD_DATA,					///< use 'ussd_data' in 'ss_info' union
	SSINFOUSE_CALL_BARRING					///< Call Barring
} SSInfoUse_t;

/// Call Barring Information
typedef struct
{
	SuppSvcBSCode_t	basicSvcCode;			///< Basic Service Code
	UInt8			ssStatus;				///< Supplementary Service Status

} SS_CallBarringInfo_t;

/// Call Barring List
typedef struct
{	
	UInt8					size;			///< Call Barring list size
	SS_CallBarringInfo_t	info[13];		///< Call Barring Information

} SS_CallBarringList_t;

/// Supplementary Service Parameters
typedef struct
{
	// Following 'union' structure is only valid if 'ss_info_use' is not set to 'SSINFO_DO_NOT_USE'.
	// If 'ss_info_use' is set to 'SSINFO_DO_NO_USE', 'ss_info' contains invalid settings.
	SSInfoUse_t		ss_info_use;
	union 
	{
		FFeatureList_t 			ffeature_list;	///< Call forward feature list
		SS_CallBarringList_t	callBarList;	///< Call Barring information list
		RawSSStatus_t			raw_ss_status;	///< Raw(un decoded) SS information
		TelephoneNumber_t 		forwarded_to_number;///< Forwarded to telephone number
		BasicServiceGroupList_t bs_group_list;	///< Basic Services Group list
		CLIRestrictionInfo_t 	cli_restriction_info;  ///< Caller Line ID Restriction information
		USSDData_t				ussd_data_info;	///< USSD Data
	} ss_info;
} SuppSvcParam_t;			///< MSC-->MMI : call-indep. service result

/// USSD Data Information
typedef struct
{
	CallIndex_t call_index;		///< Call index associated with the USSD session
	USSDData_t ussd_data;		///< USSD data
} USSDataInfo_t;

/// Connected Line Presentation
typedef enum
{
	COLP_PRESENT_ALLOWED,				///< connected line id present
	COLP_PRESENT_RESTRICTED				///< connected line id not present
}COLPPresentStatus_t;

/// Call Notify Supplementary Services
typedef enum
{
    CALLNOTIFYSS_UNKNOWN,                   ///< UNKNOWN
	CALLNOTIFYSS_CF_ACTIVE,					///< all forwarding SS is active
	CALLNOTIFYSS_CFU_ACTIVE,				///< unconditional call forwarding is active
	CALLNOTIFYSS_CFC_ACTIVE,				///< some of the conditional call forwardings are active
	CALLNOTIFYSS_CFB_ACTIVE,				///< call forwarding on mobile subscriber busy is active
	CALLNOTIFYSS_CFNRY_ACTIVE,				///< call forwarding on no reply is active
	CALLNOTIFYSS_CFNRC_ACTIVE,				///< call forwarding on mobile subscriber not reachable is active
	CALLNOTIFYSS_OUTCALL_FORWARDED,			///< outgoing call forwarded 
	CALLNOTIFYSS_INCALL_FORWARDED,			///< incoming call forwarded 
	CALLNOTIFYSS_FORWARDED_CALL,			///< incoming call is a forwarded call 
	CALLNOTIFYSS_CALL_WAITING,				///< call waiting is active 
	CALLNOTIFYSS_CALLRETRIEVED,				///< retrieve a call 
	CALLNOTIFYSS_CALLONHOLD,				///< call on hold 
	CALLNOTIFYSS_MPTYIND,					///< multiparty call 
	CALLNOTIFYSS_CUGINDEX,					///< closed user group 
	CALLNOTIFYSS_CLIRSUPREJ,				///< connected line identification rejection 
	CALLNOTIFYSS_BAC,						///< Barring of all calls
	CALLNOTIFYSS_OUTCALL_BARRED,			///< Barring of outgoing Call
	CALLNOTIFYSS_BAOC,						///< Barring of all outgoing calls
	CALLNOTIFYSS_BOIC,						///< Barring of outgoing international calls
	CALLNOTIFYSS_BOIC_EX_HC,				///< Barring of outgoing international calls expect those directed to the home PLMN
	CALLNOTIFYSS_INCALL_BARRED,				///< Barring of incoming call
	CALLNOTIFYSS_INCOMING_BARRED,			///< Barrin of all incoming calls
	CALLNOTIFYSS_BAIC_ROAM,					///< Barring of incomming calls when roaming outside home PLMN country
	CALLNOTIFYSS_ECT_INDICATION,			///< ECT call indication
	CALLNOTIFYSS_NAME_INDICATOR,			///< Name Indicator, which is related to CNAP
	CALLNOTIFYSS_CALL_ON_HOLD_RELEASED,		///< Call on hold released
	CALLNOTIFYSS_FORWARD_CHECK_SS_MSG,		///< Forward check SS message
	CALLNOTIFYSS_CALL_DEFLECTED,			///< Call has been deflected (Outgoing)
	CALLNOTIFYSS_DEFLECTED_CALL,			///< This is a deflected call (Incomming)
	CALLNOTIFYSS_ALERTING_PATTERN			///< Alerting Pattern
	
} CallNotifySS_t;							///< Call Notiry SS Type


typedef enum 
{
	ECTSTATE_ALERTING,
	ECTSTATE_ACTIVE
} ECTCallState_t;


typedef struct
{
	ECTCallState_t		call_state;
	Boolean				present_allowed_add;
	Boolean				present_restricted;
 	Boolean				number_not_available;
	Boolean				present_restricted_add;
 	TelephoneNumber_t	phone_number;
} EctRdnInfo_t;

/// Structure : SS Norify Parameter Type
typedef union
{
   UInt16		cug_index;									///< CUG Index
   EctRdnInfo_t	ect_rdn_info;								///< ECT RDN Information
   UInt8		callingName[PHASE1_MAX_USSD_STRING_SIZE+1];	///< Calling Name (CNAP)
   UInt8		alertPatt;									///< Alerting Pattern

} SsNotifyParam_t;///< SS Norify Parameter Type

// Notify upper layer of SS call operation
/// SS - higher layer notification of call operation
typedef struct 
{										 
   CallIndex_t			index;          				///< Call identification
   CallNotifySS_t		NotifySS_Oper;  				///< SS operation
   SsNotifyParam_t		notify_param;					///< Notify Parameters
   UInt8				fac_ie[MAX_FACILITY_IE_LENGTH];	///< Raw facilityIE received from the network
} SS_CallNotification_t; 

/// Call Action Status
typedef enum
{
	CALLACTIONSTATUS_SUCCEEDED,			///< Call Action succeeded
	CALLACTIONSTATUS_FAILED				///< Call Action failed
} CallActionStatus_t;

/// Supplementary Service Action
typedef enum
{
	SSACTION_NONE,						///< No SS Action to do
	SSACTION_ACTIVATION,				///< SS Activation
	SSACTION_DEACTIVATION,				///< SS Deactivation
	SSACTION_INTERROGATION,				///< SS Interrogation
	SSACTION_REGISTRATION,				///< SS Registration
	SSACTION_ERASURE					///< SS Erasure
} SSAction_t;							///< SS Action to do


/// AT Command causes for Call release
typedef enum							///< Call Release Causes (see stack's msnu.h) 
{
	ATCAUSE_USER_REL 	= 0,
	ATCAUSE_V24_FAILURE = 1,
	ATCAUSE_TIMER_OUT   = 2	
} CallRelCause_t;

/// Line States Values
typedef enum							// Line States (see stack's msnu.h)
{
	LINESTATE_ON	= 0,		///< This represents the status of MS' readiness
	LINESTATE_OFF	= 1			///< for data transfer in a data call. The lines
								///< are DTR, RTS.
} ATDSLineState_t;

/// Advice of Charge Status
typedef enum 
{
	AOCSTATUS_DEACTIVE,					///< AOC services are deactive
	AOCSTATUS_ACTIVE,					///< AOC services are active
	AOCSTATUS_SUSPENDED,				///< AOC services are suspended
	AOCSTATUS_ACI_UPDATE				///< AOC ACI has been updated
} AoCStatus_t;  


//
//	NOTE: do not use the values from the Stack
/// Cause Values
typedef enum {

	MNCAUSE_UNASSIGNED_NUMBER					= 0x01,	///< 1.Number not assigned
	MNCAUSE_NO_ROUTE							= 0x03,	///< 3.No route available 
	MNCAUSE_CHANNEL_UNACCEPTABLE				= 0x06,	///< 6.Unacceptable Channel
	MNCAUSE_OPERATOR_BARRING					= 0x08,	///< 8.Request barred by operator
	MNCAUSE_NORMAL_CALL_CLEARING				= 0x10,	///< 16.Normal call clearing
	MNCAUSE_USER_BUSY							= 0x11,	///< 17.Called party is busy
	MNCAUSE_NO_USER_RESPONDING					= 0x12,	///< 18.User is not responding
	MNCAUSE_USER_ALERTING_NO_ANSWR				= 0x13,	///< 19.No answer to user alerting
	MNCAUSE_MN_CALL_REJECTED					= 0x15,	///< 21.Call rejected
	MNCAUSE_NUMBER_CHANGED						= 0x16,	///< 22.Number has been changed
	MNCAUSE_PRE_EMPTION							= 0x19,	///< 25.Pre-emption
	MNCAUSE_NON_SELECT_USER_CLR					= 0x1A,	///< 26.User Caller Line restriction
	MNCAUSE_DEST_OUT_OF_ORDER					= 0x1B,	///< 27.Destination number/equipment is out of order
	MNCAUSE_INVALID_NUMBER_FORMAT				= 0x1C,	///< 28.Invalid number format
	MNCAUSE_FACILITY_REJECTED					= 0x1D,	///< 29.Facility rejected
	MNCAUSE_RESPONSE_TO_STATUS_ENQ				= 0x1E,	///< 30.Reponse to status enquiry
	MNCAUSE_NORMAL_UNSPECIFIED					= 0x1F,	///< 31.Normal unspecified call release
	MNCAUSE_NO_CIRCUIT_AVAILABLE				= 0x22,	///< 34.No call cicuit available
	MNCAUSE_NETWORK_OUT_OF_ORDER				= 0x26,	///< 38.Network out of order
	MNCAUSE_TEMPORARY_FAILURE					= 0x29,	///< 41.Temporary failure.Try later.
	MNCAUSE_SWITCH_CONGESTION					= 0x2A,	///< 42.Network switch congestion
	MNCAUSE_ACCESS_INFO_DISCARDED				= 0x2B,	///< 43.Access information discarded
	MNCAUSE_REQUESTED_CIRCUIT_NOT_AVAILABLE		= 0x2C,	///< 44.Requested Circuit is not available.
	MNCAUSE_RESOURCES_UNAVAILABLE				= 0x2F,	///< 47.Resources not available
	MNCAUSE_QUALITY_UNAVAILABLE					= 0x31,	///< 49.Requested Quality of service is not available
	MNCAUSE_FACILITY_NOT_SUBSCRIBED				= 0x32,	///< 50.Requested facility is not subscribed
	MNCAUSE_INCOMING_CALLS_BARRED_IN_CUG		= 0x37,	///< 55.Incoming calls are barred in Closed User Group
	MNCAUSE_BEARER_CAPABILITY_NOT_ALLOWED		= 0x39,	///< 57.Data call bearer capability not allowed
	MNCAUSE_BEARER_CAPABILITY_NOT_AVAILABLE		= 0x3A,	///< 58.Requested bearer capability not available.
	MNCAUSE_SERVICE_NOT_AVAILABLE				= 0x3F,	///< 63.Service is not available
	MNCAUSE_BEARER_SERVICE_NOT_IMPLEMENTED		= 0x41,	///< 65.Bearer service is not implemented
	MNCAUSE_ACM_GREATER_OR_EQUAL_TO_ACMMAX		= 0x44,	///< 68.Call Meter value is equal or greater than max value.
	MNCAUSE_FACILITY_NOT_IMPLEMENTED			= 0x45,	///< 69.Facility not implemented.
	MNCAUSE_ONLY_RESTRICTED_DIGITAL				= 0x46,	///< 70.Restricted Digital
	MNCAUSE_SERVICE_NOT_IMPLEMENTED				= 0x4F,	///< 79.Service not implemented.
	MNCAUSE_INVALID_TI							= 0x51,	///< 81.Invalid Transaction Identifier.
	MNCAUSE_USER_NOT_IN_CUG						= 0x57,	///< 87.User is not in Closed User Group.
	MNCAUSE_INCOMPATIBLE_DESTINATION			= 0x58,	///< 88.Destination is incompatible.
	MNCAUSE_INVALID_TRANSIT_NETWORK				= 0x5B,	///< 91.Transit network is invalid
	MNCAUSE_SEMATICS_INCORRECT					= 0x5F,	///< 95.Semantics incorrect.
	MNCAUSE_INVALID_MANATORY_INFORMATION		= 0x60,	///< 96.Invalid mandatory information
	MNCAUSE_MESG_TYPE_NON_EXISTENT				= 0x61,	///< 97.Message type non existent
	MNCAUSE_MESG_TYPE_NOT_COMPATIBLE_WITH_STATE	= 0x62,	///< 98.Received message type non compatible with MS' state
	MNCAUSE_IE_NON_EXISTENT						= 0x63,	///< 99.Information Element does not exist
	MNCAUSE_CONDITIONAL_IE_ERROR				= 0x64,	///< 100.Conditional Info. Element error
	MNCAUSE_MESG_NOT_COMPATIBLE_WITH_STATE		= 0x65,	///< 101.Received message non compatible with MS' state
	MNCAUSE_RECOVERY_ON_TIMER_EXPIRY			= 0x66,	///< 102.Timer expiry recovery
	MNCAUSE_PROTOCOL_ERROR						= 0x6F,	///< 111.Protocol error
	MNCAUSE_INTERWORKING						= 0x7F,	///< 127.Interworking error.
	MNCAUSE_MAXSPECVALUE						= MNCAUSE_INTERWORKING, 
														///< As per 3GPP 24.008 Sec 10.5.4.11
														///< we should not receive any values greater than this from n/w
														///< any values greater are non spec values.

	MNCAUSE_VOID_CAUSE							= 0x100, ///< Should be same as MN_VOID_CAUSE value in stack
	MNCAUSE_OUT_OF_MEMORY						= 0x101,  ///< Should be same as MN_OUT_OF_MEMORY value in stack
    MNCAUSE_IMMEDIATE_REL						= 0x102,  ///< A special cause for Immediate Release feature (for CAPI only)
															
//	the following two causes are genereated by Stack, not the Network,
//	so it mapped into a cause for MN for the Stack.

	MNCAUSE_RADIO_LINK_FAILURE_APPEARED,		///< Sent when lower layer failure happen before RR is established. Use this cause for call retry 		
	MNCAUSE_REESTABLISHMENT_SUCCESSFUL,
	MNCAUSE_RADIO_LINK_FAILURE_AFTER_RRC		///< Sent when lower layer failure happen after RR is established.

	
} Cause_t;		

typedef enum {

	GSMCAUSE_SUCCESS,								///< Request Success
	GSMCAUSE_REG_INVALID_SIM,						///< SIM card is not valid
	GSMCAUSE_REG_INVALID_ME,						///< Invlaid Mobile Equipment
	GSMCAUSE_REG_NO_NETWORK,						///< couldn't find any network
	GSMCAUSE_REG_NO_GPRS,							///< GSM-only cell, no GPRS network
	GSMCAUSE_REG_NO_ACCESS,							///< No access to register
	GSMCAUSE_REG_SERVICE_DISABLED,					///< Registration service is disabled
	GSMCAUSE_REG_NO_CAUSE,							///< No Cause value reported
	GSMCAUSE_ERROR_TIMEOUT,							///< Timed out waiting for the request
	GSMCAUSE_ERROR_UNKNOWN_SUBSCRIBER,				///< Subscriber of this request is unknown
	GSMCAUSE_ERROR_ILLEGAL_SUBSCRIBER,				///< Subscriber illegal
	GSMCAUSE_ERROR_BEARER_SVC_NOT_PROVISIONED,		///< Bearer Service not provisioned
	GSMCAUSE_ERROR_TELE_SVC_NOT_PROVISIONED,		///< Tele Service not provisioned 
	GSMCAUSE_ERROR_ILLEGAL_EQUIPMENT,				///< Equipment is not legal
	GSMCAUSE_ERROR_CALL_BARRED,						///< Call is barred
	GSMCAUSE_ERROR_ILLEGAL_SS_OPERATION,			///< Illegal Supplementary Service operation
	GSMCAUSE_ERROR_SS_ERROR_STATUS,					///< Supplementary Service Error
	GSMCAUSE_ERROR_SS_NOT_AVAILABLE,				///< This SS service is unavailable
	GSMCAUSE_ERROR_SS_SUBSCRIPTION_VIOLATION,		///< The request is in violation of the services subscribed
	GSMCAUSE_ERROR_SS_INCOMPATIBILITY,				///< Service incompatible, request failed.
	GSMCAUSE_ERROR_FACILITY_NOT_SUPPORT,			///< Requested facility is not supported
	GSMCAUSE_ERROR_ABSENT_SUBSCRIBER,				///< Subscriber is absent
	GSMCAUSE_ERROR_SYSTEM_FAILURE,					///< System failure
	GSMCAUSE_ERROR_DATA_MISSING,					///< Data is incomplete in the request
	GSMCAUSE_ERROR_UNEXPECT_DATA_VALUE,				///< Unexpected data in the request. Check request and try again.
	GSMCAUSE_ERROR_PASSWD_REG_FAILURE,				///< This SS service is unavailable
	GSMCAUSE_ERROR_NEGATIVE_PASSWD_CHECK,			///< Password check negative
	GSMCAUSE_ERROR_MAX_PASSWD_ATTEMPTS_VOILATION,	///< Exceeded allowed number of password check attempts.
	GSMCAUSE_ERROR_UNKNOWN_ALPHABET,				///< Unknown alphabet in request
	GSMCAUSE_ERROR_USSD_BUSY,						///< Unstructured SS Data busy.
	GSMCAUSE_ERROR_MAX_MPTY_EXCEEDED,				///< Maximum number of multiparty calls exceeded
	GSMCAUSE_ERROR_RESOURCES_NOT_AVAIL,				///< Necessary resources unavailable by the network
	GSMCAUSE_ERROR_REJECTED,						///< Request rejected
	GSMCAUSE_REG_LU_TEMP_BARRED,					///< if accessclass is barred in the camped cell 
	GSMCAUSE_REG_NO_ACCESS_MAX_ATTEMPT, 			///< no access due to max LU attempt during location updating.
	GSMCAUSE_REG_REJ_BY_NW_MAX_ATTEMPT, 			///< max GPRS attempts of 5 have been tried after rejection from n/w, only applies to PS
	GSMCAUSE_REG_INTERNAL_FAILURE_MAX_ATTEMPT		///< max GPRS attempts of 5 due to some internal failure and network reject

} NetworkCause_t;

/// Enum : Error Code
typedef enum
{
	SS_ERROR_CODE_NONE								= 0,	///< 0x00 None
	SS_ERROR_CODE_UNKNOWN_SUBSCRIBER				= 1,	///< 0x01 Unknown Subscriber
	SS_ERROR_CODE_ILLEGAL_SUBSCRIBER				= 9,	///< 0x09 Illegal Subscriber
	SS_ERROR_CODE_BEARER_SVC_NOT_PROVISIONED		= 10,	///< 0x0A Bearer Service Not Provisioned
	SS_ERROR_CODE_TELE_SVC_NOT_PROVISIONED			= 11,	///< 0x0B Tele Service Not Provisioned
	SS_ERROR_CODE_ILLEGAL_EQUIPMENT					= 12,	///< 0x0C Illegal Equipment
	SS_ERROR_CODE_CALL_BARRED						= 13,	///< 0x0D Call Barred
	SS_ERROR_CODE_ILLEGAL_SS_OPERATION				= 16,	///< 0x10 Illegal SS Operation
	SS_ERROR_CODE_SS_ERROR_STATUS					= 17,	///< 0x11 SS Error Status
	SS_ERROR_CODE_SS_NOT_AVAILABLE					= 18,	///< 0x12 SS Not Available
	SS_ERROR_CODE_SS_SUBSCRIPTION_VIOLATION			= 19,	///< 0x13 SS Subscription Violation
	SS_ERROR_CODE_SS_INCOMPATIBILITY				= 20,	///< 0x14 SS Incompatibility
	SS_ERROR_CODE_FACILITY_NOT_SUPPORT				= 21,	///< 0x15 Facility Not Supported
	SS_ERROR_CODE_ABSENT_SUBSCRIBER					= 27,	///< 0x1B Absent Subscriber
	SS_ERROR_CODE_SHORT_TERM_DENIAL					= 29,	///< 0x1D Short Term Denial
	SS_ERROR_CODE_LONG_TERM_DENIAL					= 30, 	///< 0x1E Long Term Denial
	SS_ERROR_CODE_SYSTEM_FAILURE					= 34,	///< 0x22 System Failure
	SS_ERROR_CODE_DATA_MISSING						= 35,	///< 0x23 Data Missing
	SS_ERROR_CODE_UNEXPECT_DATA_VALUE				= 36,	///< 0x24 Unexpected Data Value
	SS_ERROR_CODE_PASSWORD_REGISTER_FAILURE			= 37,	///< 0x25 Password Register Failure
	SS_ERROR_CODE_NEGATIVE_PASSWORD_CHECK			= 38,	///< 0x26 Negative Password Check
	SS_ERROR_CODE_MAX_PASSWORD_ATTEMPTS_VOILATION	= 43,	///< 0x2B Max PWD Attempt Violation
	SS_ERROR_CODE_POSITION_METHODE_FAILURE			= 54,	///< 0x36 Position Methode Failure
	SS_ERROR_CODE_UNKNOWN_ALPHABET					= 71,	///< 0x47 Unknown Alphabet
	SS_ERROR_CODE_USSD_BUSY							= 72,	///< 0x48 USSD Busy
	SS_ERROR_CODE_NBR_SB_EXCEEDED					= 120,	///< 0x78 NBR SB Exceeded
	SS_ERROR_CODE_REJECTED_BY_USER					= 121,	///< 0x79 Rejected By User
	SS_ERROR_CODE_REJECTED_BY_NETWORK				= 122,	///< 0x7A Rejected By Network
	SS_ERROR_CODE_DEFLECTION_TO_SERVED_SUBSCRIBER	= 123,	///< 0x7B Deflection To Served Subscriber
	SS_ERROR_CODE_SPECIAL_SERVICE_CODE				= 124,	///< 0x7C Special Service Code
	SS_ERROR_CODE_INVALID_DEFLECTED_TO_NUMBER		= 125,	///< 0x7D Invalid Deflected To Number
	SS_ERROR_CODE_MAX_NUMBER_OF_MPTY_EXCEEDED		= 126,	///< 0x7E Max Number OF MPTY Exceeded
	SS_ERROR_CODE_RESOURCES_NOT_AVAILABLE_EXCEEDED	= 127	///< 0x7F Resources Not Available Exceeded
} SS_ErrorCode_t;

/// Call AOC status message 
typedef struct
{	
	UInt8		callIndex;		///< Call Index
	AoCStatus_t	callAOCStatus;	///< Advice of charge status
 
} CallAOCStatusMsg_t;

/// Lower layer compabilitibilty - part of BC of a data call
typedef struct
{
	UInt8			val[18];  ///< Release 99 LLC could be upto 18 bytes.
} LLCompatibility_t;

/// Higher layer compabilitibilty - part of BC of a data call
typedef struct
{
	UInt8			val[5];
} HLCompatibility_t;

/// Call Service Type
typedef enum {
	CALLSERVICETYPE_UNKNOWN			 = 0, ///< GSM stack const MNDS_SERVICE_TYPE_UNKNOWN,		
										  ///< for most MT Data Calls
	CALLSERVICETYPE_SPEECH_RECORDING = 1, ///< GSM stack const MNDS_SERVICE_TYPE_SPEECH,		
										  ///< for Stack Speech Recording. Currently not supported
	CALLSERVICETYPE_ALTER2NORMAL	 = 2, ///< GSM stack const MNDS_SERVICE_TYPE_ALTER2NORMAL,	
										  ///< for Speech/Fax, not supported by MTI
	CALLSERVICETYPE_SPEECH			 = (CALLSERVICETYPE_ALTER2NORMAL+1)
										  ///< GSM stack const MNDS_SERVICE_TYPE_ALTER2NORMAL+1	
										  ///< used by MTI only
} CallServiceType_t;


// Location Area and Cell Site Information
typedef UInt16 LACode_t;		///< Location Area Code

/// PLMN ID
typedef struct
{
	UInt16 mcc;					///< unconverted Mobile Country Code
	UInt8 mnc;					///< unconverted Mobile Network Code
	Boolean is_forbidden;		///< TRUE, if forbidden

	UInt8 mnc_len;				///< This parameter is currently applicable for SimApi_GetHomePlmn() function only to specify whether the HPLMN in SIM
								///< specifies number of digits for HPLMN MNC (HPLMN is obtained from either Acting-HPLMN or IMSI). Possible value:
								///< 2, 3 or 0. If 0 is specified, it means the number of digits is not specified in EF-AD in SIM.
} PLMNId_t;

/// Enum:	The type of plmn.  Should be in sync with T_NETWORK_TYPE provided by the stack
typedef enum {
	
	MSNWTYPE_HOME_PLMN,				///< The PLMN is a Home PLMN of the MS
	MSNWTYPE_PREFERRED_PLMN,		///< The PLMNID is present in the Preferred PLMN list in SIM
	MSNWTYPE_FORBIDDEN_PLMN,		///< The PLMNID is in the forbidden PLMN list stored in the MS
	MSNWTYPE_OTHER_PLMN,			///< Other PLMN
	MSNWTYPE_INVALID	= 255		///< Used when the stack doesn't provide the network type
} MSNwType_t;						///< MS Network Type

/**
	This enum is used to indicate whether the PLMN is stored in the EONS, NITZ
	or LOOKUPTABLE. This information is filled in by CAPI and used by upper layer
**/
typedef enum
{
	PLMN_NAME_TYPE_INVALID,
	PLMN_NAME_TYPE_EONS,		///< retreived from SIM EONS
	PLMN_NAME_TYPE_CPHS,		///< retrieved from SIM CPHS
	PLMN_NAME_TYPE_NITZ,		///< retrieved from NITZ
	PLMN_NAME_TYPE_LKUP_TABLE	///< retrieved from LookupTable
} PLMNNameType_t;

/// PLMN Name Structure
typedef struct
{
	ALPHA_CODING_t 	coding;			///< GSM Alphabet, UCS2_80, UCS2_81 or UCS2_82 
	UInt8			name_size;		///< Number of bytes used in "name" 
	UInt8			name[60];		///< For ASCII encoding, the data is not guaranteed to be NULL terminated. 
									///< For UCS2 encoding, the data begins with the byte after the UCS2 encoding byte, 
									///< i.e. After 0x80, 0x81 or 0x82.
	PLMNNameType_t	nameType;		///< Indicates whether the PLMN is retrieved from NITZ, EONS, lookuptable.

	Boolean			is_pnn_1st_rec; ///< TRUE if name is obtained from first record in EF-PNN in SIM. This element is applicable only if "nameType == PLMN_NAME_TYPE_EONS".
									///< Section 4.2.58 of 3GPP 31.102 says if a RPLMN's name is obtained from the first record in EF-PNN it is considered HPLMN.
} PLMN_NAME_t;

/// MS PLMN Name
typedef struct {
	Boolean			matchResult;	///< Match result
	PLMN_NAME_t		longName;		///< Long plmn name
	PLMN_NAME_t		shortName;		///< Short plmn name
}MsPlmnName_t;

/// Structure: PLMN returned in a PLMN search 
typedef struct
{
	UInt16				mcc;			///< unconverted Mobile Country Code - 3 digits
	UInt16				lac;			///< unconverted, store the LAC of the highest power
	UInt8				mnc;			///< unconverted Mobile Network Code 2-3 digits
	Boolean				is_forbidden;	///< TRUE, if forbidden
	UInt8				rat;			///< RAT_NOT_AVAILABLE(0),RAT_GSM(1),RAT_UMTS(2)
	MSNwType_t			network_type;	///< Type of PLMN , HPLMN, FORBIDDEN,PREFERRED as defined in MSNwType_t
	MsPlmnName_t		ucs2Name;		///< Name of the PLMN ,in ucs2 character format. 
	MsPlmnName_t		nonUcs2Name;	///< Name of the PLMN in non-UCS2 character format
} SEARCHED_PLMNId_t;				///< PLMN List sent in the PLMN_LIST_CNF message to the upper layer.


/// Cause Values returned in PLMN Search
typedef enum {
	PLMNCAUSE_SUCCESS,				///< Success, found networks
	PLMNCAUSE_NETWORK_NOT_FOUND,	///< Networks not found
	PLMNCAUSE_OPERATION_NOT_ALLOWED,///< Operation not allowed
	PLMNCAUSE_USER_ABORT,			///< Operation aborted by user
	PLMNCAUSE_ACTIVE_CALL,			///< when any call is active plmn search is rejected
} PlmnCause_t;

/// List returned after PLMN search
typedef struct
{
	PlmnCause_t			plmn_cause;
	UInt8				num_of_plmn;	///< Number of PLMN in "searched_plmn" list 
	SEARCHED_PLMNId_t 	searched_plmn[MAX_PLMN_SEARCH];
} SEARCHED_PLMN_LIST_t;

/// Location Area Information
typedef struct
{
	PLMNId_t plmn;				///< PLMN value, MCC and MNC
	LACode_t lac;				///< Location Area Code, unconverted
} LAInfo_t;

typedef UInt16 CellInfo_t;		///< Cell Site Information, unconverted

/// Enum : PlmnDisplayFormat
typedef enum
{
	LONG_FORMAT,
	SHORT_FORMAT,
	NUMERIC_FORMAT
} PLMNDispFormat_t;

//Need to move the following definition to cc_def.h after IRB process 
/**
@code
*         8         7         6          5        4         3       2       1     Bit
*       Rsvd       UMTS       UMTS      HR        FR        GSM     GSM     GSM
* 				   AMR2		  AMR		AMR      AMR       	EFR     HR      FR
*       0           0          0         x        x          x       x      x      Support
*
*         8         7         6          5        4         3       2       1     Bit	
*		rsvd 	  Rsvd 		rsvd 		rsvd 	rsvd      UMTS	    FR    rsvd
*														 AMRWB   AMRWB
*		0 			0 		  0 		  0      0         x        x       0

*		Where support bit states:
*		0 = codec not supported
*		1 = codec supported
@endcode
**/
typedef UInt16 MS_SpeechCodec_t;
#define CODEC_GSMFR	0x0001
#define CODEC_GSMHR	0x0002
#define CODEC_EFR	0x0004
#define CODEC_AMRFR	0x0008
#define	CODEC_AMRHR	0x0010
#define CODEC_UMTSAMR 0x0020
#define CODEC_UMTSAMR2 0x0040
#define CODEC_FRAMRWB 0x0200
#define CODEC_UMTSAMRWB 0x0400

// valid combinations of the codecs 
// As per 3GPP 24.008 10.5.4.5 we need to 
#define MSCHANNELMODE_GSMHR_GSMFR					(MSCHANNELMODE_SPEECH_FULL | MSCHANNELMODE_SPEECH_HALF)
#define MSCHANNELMODE_GSMFR_EFR					(MSCHANNELMODE_SPEECH_FULL | MSCHANNELMODE_SPEECH_FULL_V2)
#define MSCHANNELMODE_GSMFR_AMRFR					(MSCHANNELMODE_SPEECH_FULL | MSCHANNELMODE_SPEECH_FULL_V3)
#define MSCHANNELMODE_AMRHR_GSMFR					(MSCHANNELMODE_SPEECH_HALF_V3 | MSCHANNELMODE_SPEECH_FULL)
#define MSCHANNELMODE_GSMHR_GSMFR_EFR				(MSCHANNELMODE_SPEECH_FULL | MSCHANNELMODE_SPEECH_HALF | MSCHANNELMODE_SPEECH_FULL_V2)
#define MSCHANNELMODE_GSMHR_GSMFR_AMRFR 			(MSCHANNELMODE_SPEECH_HALF | MSCHANNELMODE_SPEECH_FULL | MSCHANNELMODE_SPEECH_FULL_V3)
#define MSCHANNELMODE_GSMHR_AMRHR_GSMFR 			(MSCHANNELMODE_SPEECH_HALF | MSCHANNELMODE_SPEECH_HALF_V3 | MSCHANNELMODE_SPEECH_FULL)
#define MSCHANNELMODE_GSMFR_EFR_AMRFR				(MSCHANNELMODE_SPEECH_FULL | MSCHANNELMODE_SPEECH_FULL_V2 | MSCHANNELMODE_SPEECH_FULL_V3)
#define MSCHANNELMODE_AMRHR_GSMFR_EFR				(MSCHANNELMODE_SPEECH_FULL | MSCHANNELMODE_SPEECH_HALF_V3 | MSCHANNELMODE_SPEECH_FULL_V2)
#define MSCHANNELMODE_AMRHR_GSMFR_AMRFR			(MSCHANNELMODE_SPEECH_HALF_V3 | MSCHANNELMODE_SPEECH_FULL | MSCHANNELMODE_SPEECH_FULL_V3)
#define MSCHANNELMODE_GSMHR_GSMFR_EFR_AMRFR 		(MSCHANNELMODE_SPEECH_HALF | MSCHANNELMODE_SPEECH_FULL | MSCHANNELMODE_SPEECH_FULL_V2 | MSCHANNELMODE_SPEECH_FULL_V3)
#define MSCHANNELMODE_AMRHR_GSMFR_EFR_AMRFR		(MSCHANNELMODE_SPEECH_HALF_V3 | MSCHANNELMODE_SPEECH_FULL | MSCHANNELMODE_SPEECH_FULL_V2 | MSCHANNELMODE_SPEECH_FULL_V3)
#define MSCHANNELMODE_GSMHR_AMRHR_GSMFR_EFR		(MSCHANNELMODE_SPEECH_HALF | MSCHANNELMODE_SPEECH_HALF_V3 | MSCHANNELMODE_SPEECH_FULL | MSCHANNELMODE_SPEECH_FULL_V2)
#define MSCHANNELMODE_GSMHR_AMRHR_GSMFR_AMRFR 		(MSCHANNELMODE_SPEECH_HALF_V3 | MSCHANNELMODE_SPEECH_HALF |MSCHANNELMODE_SPEECH_FULL_V3 | MSCHANNELMODE_SPEECH_FULL )
#define MSCHANNELMODE_GSMHR_AMRHR_GSMFR_EFR_AMRFR	(MSCHANNELMODE_SPEECH_FULL_V3 | MSCHANNELMODE_SPEECH_FULL_V2 | MSCHANNELMODE_SPEECH_FULL | MSCHANNELMODE_SPEECH_HALF_V3 | MSCHANNELMODE_SPEECH_HALF)



// represents MS Channel Mode, values assigned by sysparm
typedef UInt16	MSChannelMode_t;
#define MSCHANNELMODE_SIGNALLING_ONLY	SIGNALLING_ONLY		///< Signalling only Channel
#define MSCHANNELMODE_SPEECH_FULL		SPEECH_FULL			///< Full rate Speech Cahnnel
#define MSCHANNELMODE_SPEECH_HALF		SPEECH_HALF			///< Half rate Speech Cahnnel
#define MSCHANNELMODE_DATA_144			DATA_144			///< 14400bps Data Channel		
#define MSCHANNELMODE_DATA_96			DATA_96				///< 9600bps Data Channel		
#define MSCHANNELMODE_DATA_48_FULL		DATA_48_FULL		///< 4800bps Full rate Data Channel		
#define MSCHANNELMODE_DATA_48_HALF		DATA_48_HALF		///< 4800bps Half rate Data Channel		
#define MSCHANNELMODE_DATA_24_FULL		DATA_24_FULL		///< 2400bps Full rate Data Channel		
#define MSCHANNELMODE_DATA_24_HALF		DATA_24_HALF		///< 2400bps Half rate Data Channel		
#define MSCHANNELMODE_SPEECH_FULL_V2	SPEECH_FULL_V2		///< Full rate Version 2 Speech Cahnnel
#define MSCHANNELMODE_SPEECH_FULL_V3	SPEECH_FULL_V3		///< Full rate Version 3 Speech Cahnnel
#define MSCHANNELMODE_SPEECH_HALF_V2	SPEECH_HALF_V2		///< Half rate Version 2 Speech Cahnnel
#define MSCHANNELMODE_SPEECH_HALF_V3	SPEECH_HALF_V3		///< Half rate Version 3 Speech Cahnnel

//new definitions for channel mode due to AMR - WB
#define MSCHANNELMODE_SPEECH_FULL_V4	SPEECH_FULL_V4		///<Full rate version 4 OFR-AMR WB
#define MSCHANNELMODE_SPEECH_FULL_V5	SPEECH_FULL_V5		///<Full rate version 5 FB-AMR WB 
#define MSCHANNELMODE_SPEECH_HALF_V4	SPEECH_HALF_V4		///<Half rate version 4 OHR-AMR WB 
#define MSCHANNELMODE_SPEECH_HALF_V6	SPEECH_HALF_V6		///<Half rate version 6 OHR-AMR
#define MSCHANNELMODE_SPEECH_FULL_V6	SPEECH_FULL_V6		///<Full rate Version 6 UMTS-AMR WB  --Need to confirm this in the spec.

#define	IS_MSCHANNELMODE_SIGNALLING_ONLY( v )	\
	( v & MSCHANNELMODE_SIGNALLING_ONLY )

#define	IS_MSCHANNELMODE_SPEECH( v )	\
	( v & (MSCHANNELMODE_SPEECH_FULL | MSCHANNELMODE_SPEECH_HALF | \
	MSCHANNELMODE_SPEECH_FULL_V2 | MSCHANNELMODE_SPEECH_FULL_V3 | \
	MSCHANNELMODE_SPEECH_HALF_V2 | MSCHANNELMODE_SPEECH_HALF_V3 | \
	MSCHANNELMODE_SPEECH_FULL_V4| MSCHANNELMODE_SPEECH_FULL_V5 | \
	MSCHANNELMODE_SPEECH_HALF_V4 | MSCHANNELMODE_SPEECH_HALF_V6 | MSCHANNELMODE_SPEECH_FULL_V6 ) )	

	
#define	IS_MSCHANNELMODE_DATA( v )		\
	( v & (MSCHANNELMODE_DATA_96 | 		\
	MSCHANNELMODE_DATA_48_FULL | MSCHANNELMODE_DATA_48_HALF | \
	MSCHANNELMODE_DATA_24_FULL | MSCHANNELMODE_DATA_24_HALF | \
	MSCHANNELMODE_DATA_144 ))


#define BEARER_CAPABILITY_IEI	0x04		///< Bearer Capability Information Element Identifier

// Subaddresses Information Element Identifiers
#define CALLING_PARTY_SUBADD_IEI	0x5D	///< Calling Party Subaddress Information Element Identifier
#define CALLED_PARTY_SUBADD_IEI		0x6D	///< Called Party Subaddress Information Element Identifier
#define CONNECTED_SUBADDR_IEI		0x4D	///< Connected Party Subaddress Information Element Identifier

typedef UInt8	*NetworkName_t;					// See GSM 04.08 for Structure

// See GSM 09.02 sections 17.7.9 and 17.7.10

#define allBearerServices			0x00
#define allDataCDA					0x10
#define allDataCDS					0x18
#define allPADAccessCA				0x20
#define allPADAccessPDS				0x28
#define allDataPDS					0x28
#define allCircuitAsync				0x50
#define allCircuitSync				0x58
#define allAsync					0x60
#define allSync						0x68
#define allAltSpeechCDA				0x30
#define allAltSpeechCDS				0x38
#define allSpeechFollowedByCDA		0x40
#define allSpeechFollowedByCDS		0x48

#define allVoiceGroupCall			0x91
#define	allVoiceBroadcastCall		0x92

#define allTeleServices				0x00
#define allSpeech					0x10
#define allTelephony				0x11
#define allEmergencyCalls			0x12
#define allSMS						0x20
#define allFAX						0x60
#define allDataTeleservices			0x70
#define	allTeleServicesNoSMS		0x80

/* Teleservice code for ALS (Alternative Line Service) added to Section 17.7.9 of GSM 09.02, see Appendix 1 of CPHS spec */ 
#define allAuxiliarySpeech			0xD0  
#define allAuxiliaryTelephony		0xD1  



/// This enum is used to identify API Client command indentifier.
typedef enum
{
	CC_CALL_ACCEPT,	///< Used to identify the call to CC_Voice/Video/DataCallAccept() 
	CC_CALL_REJECT	///< Used to identify the call to CC_EndCall()
} ClientCmd_t;

/// This enum is used to indicate GPRS suspend cause
typedef enum
{
	SUSPEND_NO_CAUSE = 0,
	SUSPEND_LOCATION_UPDATING = 1,
	SUSPEND_MOBILE_ORIG_CALL_EST = 2,
	SUSPEND_MOBILE_TERM_CALL_EST = 3,
	SUSPEND_ROUTING_AREA_UPDATING = 4,
	SUSPEND_NO_COVERAGE = 5,
	SUSPEND_OTHER_CAUSE = 6
} SuspendCause_t;

/// This enum is used to indicate radio activity --- direction
typedef enum
{
	RADIO_UPLINK = 0,
	RADIO_DOWNLINK = 1,
	RADIO_BIDIRECTION = 2
} RadioDirection_t;

/// This enum is used to indicate radio activity --- status (on/off)
typedef enum
{
	RADIO_ON = 0,
	RADIO_OFF = 1
} RadioStatus_t;

/// Structure : Radio status indicating whether we are in TBF state
typedef struct
{
	RadioDirection_t 	radio_direction;
	RadioStatus_t 		radio_status;
	UInt32				max_dl_data_rate;		
	
} MS_Radio_Status_t;

/// Structure : HSDPA and DCH cell state
typedef struct
{
	Boolean in_cell_dch_state;  ///< TRUE if UE is in DCH cell state
	Boolean hsdpa_ch_allocated; ///< TRUE if HSDPA Channel allocated 
	Boolean hsupa_ch_allocated; ///< TRUE if HSUPA Channel allocated 
}MS_Umts_Dch_State_t;

/**
	This enum is used to indicate ms 2G radio establish state
**/
typedef enum
{
	MS_IDLE_STATE = 0,
	MS_SIGNAL_STATE = 1,
	MS_CALL_STATE = 2,	///< RR Established
} MSStatus_t;


/// Defines DTX_FROM_BS values
typedef enum {
	DTX_MAY = 0,
	DTX_USE,
	DTX_NOT
} MS_DTXFromBs_t;	

/// Defines the Cell Barr Change Status
typedef enum {
	CELL_BARR_ACCEPTED = 0, 	/* default */
	CELL_BARR_REVERSED,
	CELL_BARR_DISCARD
} CellBarrChangeStatus_t;

/// Defines the cell priority
typedef enum {
	CELL_PRIORITY_NORM = 0, 	/* default */
	CELL_PRIORITY_BARRED,
	CELL_PRIORITY_LOW
} CellPriorityStatus_t;

/// GPRS Ciphering Algorithm 
typedef enum {
	GPRS_CIPH_NO_USED	= 0,		/* default */
	GPRS_CIPH_GEA1		= 1,
	GPRS_CIPH_GEA2		= 2,
	GPRS_CIPH_GEA3		= 3,
	GPRS_CIPH_GEA4		= 4,
	GPRS_CIPH_GEA5		= 5,
	GPRS_CIPH_GEA6		= 6,
	GPRS_CIPH_GEA7		= 7
} GPRS_CiphAlg_t;


/// Defines 2G HO engineering mode
typedef enum {
	GSM_HO_INTER_CELL	= 0,			
	GSM_HO_INTRA_CELL,	
	GSM_HO_MAX_TYPES,		
	GSM_HO_INVALID					/* default */
} GSMHOFlag_t;


typedef enum {
	GSM_HO_GSM_TO_GSM	= 0,
	GSM_HO_GSM_TO_DCS,	
	GSM_HO_DCS_TO_GSM,	
	GSM_HO_DCS_TO_DCS,
	GSM_HO_MAX_BAND_SWITCH,
	GSM_HO_INVALID_BAND_SW 
} GSMHOBandSwitch_t;


typedef enum {
	CHANNEL_TYPE_THR0		= 0,
	CHANNEL_TYPE_THR1,
	CHANNEL_TYPE_AHS0,
	CHANNEL_TYPE_THS1,
	CHANNEL_TYPE_TFR,
	CHANNEL_TYPE_AFS,
	CHANNEL_TYPE_TEFR,
	CHANNEL_TYPE_F144,
	CHANNEL_TYPE_F96,
	CHANNEL_TYPE_F72,
	CHANNEL_TYPE_F48,
	CHANNEL_TYPE_F24,
	CHANNEL_TYPE_H480,
	CHANNEL_TYPE_H481,
	CHANNEL_TYPE_H240,
	CHANNEL_TYPE_H241,
	CHANNEL_TYPE_FA,
	CHANNEL_TYPE_FAH0,
	CHANNEL_TYPE_FAH1,
	CHANNEL_TYPE_PCCC,
	CHANNEL_TYPE_PBCC,
	CHANNEL_TYPE_PAGC,
	CHANNEL_TYPE_PNDR,
	CHANNEL_TYPE_PDTC,
	CHANNEL_TYPE_NDRX,
	CHANNEL_TYPE_SDCC,
	CHANNEL_TYPE_AGCH,
	CHANNEL_TYPE_CCCH,
	CHANNEL_TYPE_CBCH,
	CHANNEL_TYPE_BCCH,
	CHANNEL_TYPE_SEAR,
	CHANNEL_TYPE_NSPS

} GSMChannelType_t;


/** 
	This enum is used for MS STATUS Indicator for engineering mode
	This enum should be in sync with the stacks definition of T_MS_STATUS_IND_ENUM as defined in
	/stack/hedge/sdtc/msnu.h
**/
typedef enum {
	MN_MS_STATUS_IND_IDLE		= 0,
	MN_MS_STATUS_IND_SIGINAL,
	MN_MS_STATUS_IND_CALL,
	MN_MS_STATUS_IND_CALL_CS,
    MN_MS_STATUS_IND_CALL_PS,
    MN_MS_STATUS_IND_CALL_CS_PS
} MSStatusInd_t;

/// Service state
typedef enum
{
    UNKNOWNSTATE,
	NO_NETWORK_AVAILABLE,          ///<No Network avialable
    SEARCH_FOR_NETWORK,            ///<Mobile is searching for network to camp.Services are not yet available.
    EMERGENCY_CALLS_ONLY,          ///<Only Emergency calls are allowed. GPRS is available but not attached.
    LIMITEDSERVICE,               ///<Mobile is in limited service - only Emergency calls are allowed. GPRS is available but not attached.
    FULL_SERVICE,                  ///<Mobile is in normal service
    PLMN_LIST_AVAILABLE,           ///<PLMN List 
    DISABLEDSTATE,                      ///<Disabled
    DETACHEDSTATE,                        ///<Detached
    NOGPRSCELL,                  ///<MS not being camped on a cell or registered
    SUSPENDEDSTATE,                     ///<Suspended
} Service_State_t;

/**
	This structure is used for all the GAS related nvram classmark parameters
	that needs to be set in the stack at poweron.
	All the elements should be in sync with T_GAS_CONFIG_PARAMS defnd in msnu.h
**/
typedef struct 
{

    Boolean	auto_pwr_updated;
    Boolean auto_pwr_red;
    Boolean egprs_updated;
    Boolean egprs_support;
    Boolean egprs_msc_updated;
    UInt8 	egprs_msc;
    Boolean gprs_msc_updated;
    UInt8 	gprs_msc;
    Boolean rep_acch_updated;
    UInt8 	rep_acch;
    Boolean power_class_8psk_updated;
    UInt8 	power_class_8psk;
    Boolean power_cap_8psk_updated;
    UInt8 	power_cap_8psk;
    Boolean geran_fp1_updated;
    Boolean geran_fp1;
    Boolean u3g_ccn_updated;
    Boolean u3g_ccn_support;
    Boolean nondrx_timer_updated;
    UInt8 	nondrx_timer;
    Boolean split_pg_cycle_updated;
    UInt8 	split_pg_cycle;
    Boolean eda_support_updated;
    Boolean eda_support;
    Boolean darp_support_updated;
    Boolean darp_support;
    Boolean early_tbf_updated;
    Boolean early_tbf_req;
	Boolean gmsk_multislot_pwr_profile_updated;
	UInt8	gmsk_multislot_pwr_profile;
	Boolean random_l2_fill_bits_updated;
	Boolean random_l2_fill_bits;
    Boolean psk8_multislot_pwr_profile_updated;
    UInt8  	psk8_multislot_pwr_profile;
}
GASConfigParam_t;

/**
	This structure is used for all the UAS related nvram classmark parameters
	that needs to be set in the stack at poweron.
	All the elements should be in sync with T_UAS_CONFIG_PARAMS defnd in msnu.h
**/
typedef struct
{
    Boolean integrity_updated;
    Boolean integrity_support;
    Boolean ciphering_updated;
    Boolean ciphering_support;
    Boolean urrc_version_updated;
    UInt8 	urrc_version;
    Boolean fake_security_updated;
    Boolean fake_security;
	Boolean interrat_nacc_updated;
    Boolean interrat_nacc_support;
	Boolean primary_power_on_wcdma_band_updated;
	UInt8	primary_power_on_wcdma_band;
}
UASConfigParam_t;

/**
	This structure is used for all the nas related nvram classmark parameters
	that needs to be set in the stack at poweron.
	All the elements should be in sync with T_NAS_CONFIG_PARAMS defnd in msnu.h
**/
typedef struct
{
    Boolean a5_alg_updated;
    UInt8	a5_alg_supp;
    Boolean gea_alg_updated;
    UInt8 	gea_alg_supp;
    Boolean qos_ext_updated;
    UInt8 qos_ext;
    Boolean acq_order_updated;
    UInt8 	rat_acq_order;
/* MobC00061383/CSP204798 */
	Boolean non_3gpp_hplmn_timer_updated ;    
    Boolean non_3gpp_hplmn_timer_supported ; // Indicates if UE supports the non-3gpp HPLMN search timer
/* MobC00061383/CSP204798 */
	Boolean  r7_qos_ext_updated;
    UInt8  r7_qos_ext;
    Boolean watermark_updated;        
    UInt8   high_watermark;
    UInt8   low_watermark;
	Boolean three_g_bmc_support_updated;
	Boolean three_g_bmc_support;
	Boolean hplmn_over_lreg_in_manual_start_updated;
	Boolean hplmn_over_lreg_in_manual_start;
	Boolean modem_ens_updated;
	Boolean modem_ens;
	Boolean pref_hplmn_rat_updated;
	Boolean pref_hplmn_rat;
	Boolean pref_startup_rat_updated;
	Boolean pref_startup_rat;
	Boolean wcdma_hs_channel_support_updated;
	UInt8	wcdma_hs_channel_support;
	Boolean  gps_ms_a_updated;
    Boolean  gps_ms_a_supported;
    Boolean  gps_ms_b_updated;
    Boolean  gps_ms_b_supported;
    Boolean  gps_ms_conv_updated;
    Boolean  gps_ms_conv_supported;
    Boolean  gps_ms_lcsva_updated;
    Boolean  gps_ms_lcsva_supported;
    Boolean  gps_timing_of_cell_meas_updated;
    Boolean  gps_timing_of_cell_meas_supported;
	Boolean  gps_rrc_ue_a_updated;
    Boolean  gps_rrc_ue_a_supported;
    Boolean  gps_rrc_ue_b_updated;
    Boolean  gps_rrc_ue_b_supported;
    Boolean  gps_rrc_standalone_updated;
    Boolean  gps_rrc_standalone_supported;
    Boolean  h3g_cs_call_int_ps_stream_updated;					
    Boolean  h3g_cs_call_int_ps_stream;					
    Boolean  ps_act_ind_for_svc_rqst_updated;					
    Boolean  ps_act_ind_for_svc_rqst;					
    Boolean  hplmn_over_lreg_in_auto_mode_updated;					
    Boolean  hplmn_over_lreg_in_auto_mode;					

}
NASConfigParam_t;

/**
	This structure is used for all the nvram classmark parameters
	that needs to be set in the stack at poweron.
	All the elements should be in sync with T_NVRAM_CLASSMARK defnd in msnu.h
**/

typedef struct 
{
	GASConfigParam_t gasConfigParams;
    UASConfigParam_t uasConfigParams;
    NASConfigParam_t nasConfigParams;
	
}NVRAMClassmark_t;


#define MIN_QOS_PARM_VALUE		0
#define	MAX_QOS_PRECEDENCE		3
#define	MAX_QOS_DELAY			4
#define	MAX_QOS_RELIABILITY		5
#define	MAX_QOS_PEAK			9
#define	MAX_QOS_MEAN0			18	///< range should be 1-18 and 31 per GSM3.60
#define	BEST_QOS_MEAN			31	///< "best effort" per GSM3.60
#define	BEST_QOS_DELAY			4	///< "best effort" per GSM3.60

/// PCH Attach State
typedef enum 
{
	DETACHED,							///<	detached
	ATTACHED,							///<	attached
	ATTACH_IN_PROGRESS,					///<	attach pending
	DETACH_IN_PROGRESS,					///<	detach pending
	DETACH_IN_PROGRESS_ATTACH_PENDING,	///<	internal use
	ATTACH_IN_PROGRESS_DETACH_PENDING	///<	internal use
} AttachState_t;


/**
	This structure is used to relay the status indication sent by stack to the MMI
	The message sent to MMI is MSG_MS_STATUS_IND
**/
typedef struct 
{

	MSStatusInd_t status;
	
}MS_StatusIndication_t;

///User-To-User data structure
#define MAX_USER_TO_USER_SIZE	128		///< Max User To User Size

// Enum : User To User
typedef struct
{
	UInt8		uusProtDisc;					///< User To User Protocol Discreminator
	UInt8		length;							///< Length of the UUS Information
	UInt8		uusInfo[MAX_USER_TO_USER_SIZE];	///< User To User Information
} SS_UserToUser_t;

// Enum : User User Service
typedef enum
{
	SS_USER_USER_SERVICE_1,					///< User User Service 1
	SS_USER_USER_SERVICE_2,					///< User User Service 2
	SS_USER_USER_SERVICE_3					///< User User Service 3
} SS_UusSrvType_t;

// Enum : User User Service
typedef struct
{
	SS_UusSrvType_t	type;					///< Type
	Boolean			required;				///< Required
} SS_UserUserSrv_t;

/**
@code
Structure : User Information
			include       8   7   6   5   4   3   2   1
						|   |   |   |   |   |   |   |   |
						  ^   ^   ^   ^   ^   ^   ^   ^			   
			uus										  X		///< User To User
			moredata							  X 		///< More Data
			errorCode						  X				///< Error Code
			cause						  X					///< Cause
@endcode
**/
// Enum : User Information
typedef struct
{
	UInt8				include;				///< Included information
	SS_UserToUser_t		uus;					///< User To User
	Boolean				moreDate;				///< More Data
	UInt8				uuSrvCount;				///< User User Service Counter
	SS_UserUserSrv_t	uuSrv[3];				///< User User Service
	SS_ErrorCode_t		errorCode;				///< Error Codde
	Cause_t				cause;					///< Cause
	UInt8				returnResult;			///< ReturnResult (Bit:0 UUS1, Bit:1 UUS2, Bit:2 UUS3)
	UInt8				callIndex;				///< Call Index
} SS_UserInfo_t;

/**
	Voice Call Parameters
	NOTE: Any client that is using this structure in an API call should initialize
	the structure to 0 before assigning any values. 
	Any unused parameter should be defaulted to 0.
*/	
typedef struct
{
	Boolean			isFdnChkSkipped;					///< TRUE if FDN check by platform in CC_MakeVoiceCall() is to be skipped (e.g. for STK Setup Call)
	CLIRMode_t		clir;								///< Caller Line ID Restriction
	CUGInfo_t		cug_info;							///< Closed User Group Info.
	Boolean			auxiliarySpeech;					///< Auxillary Speech line
	Boolean			isEmergency;						///< Is this an emergency call
	Subaddress_t	subAddr;							///< Sub Address Type
	CC_BearerCap_t	bearerCap;							///< Bearer Capability to be used for the call
	UInt8			dtmfLength;							///< DTMF length
	DTMFTone_t		dtmfTones[DTMF_TONE_ARRAY_SIZE];	///< DTMF Tone
	SS_UserInfo_t	uusInfo;							///< User To User Information;
	UInt8           emergencySvcCat;                    ///< Emergency Service Category Value
} VoiceCallParam_t;

/**
	This structure is used to pass the call setup parameters internally in CAPI. Callcontrol uses this structure
	to send voice call request to SIM that in turn queries the SIM if call control by SIM is enabled and then passes the 
	same structure to MNCC to process the request.
**/
typedef struct
{
	PartyAddress_t			partyAdd;                           ///< Party Address
	Boolean					isEmergency;                        ///< TRUE if Emergency Call
	Boolean					auxiliarySpeech;                    ///< TRUE if auxiliary Speech
	Subaddress_t			subAddr;                            ///< Subaddress info
	CLIRMode_t				clir_mode;                          ///< CLIR Mode
	CUGInfo_t				cug_info;                           ///< CUG Info
	CC_BearerCap_t		 	bearerCap;                          ///< Bearer Capability info
	UInt8					dtmfLength;							///< DTMF length
	DTMFTone_t				dtmfTones[DTMF_TONE_ARRAY_SIZE];	///< DTMF Tone
	SS_UserInfo_t			uusInfo;							///< User To User Information;
	UInt8                   emergencySvcCat;                    ///< Emergency Service Category
} CCParmSend_t;

typedef struct
{
	UInt32                  mask;                               ///< mask
	Boolean					in_cell_dch_state;                  ///< True if UE in DCH cell state
	Boolean					hsdpa_ch_allocated;                 ///< True if HSDPA Channel allocated
	Boolean					hsupa_ch_allocated;                 ///< True if HSUPA Channel allocated
	Boolean					ue_out_of_service;					///< True if UE is out of service
}Uas_Conn_Info;

typedef struct
{
	Uas_Conn_Info in_uas_conn_info;		
} MSUe3gStatusInd_t;


/// Rx Signal Level
typedef struct
{
	UInt8 rssi;			///< In GSM mode, this is RXLEV as defined in Section 8.1.4 of 3GPP 05.08
						///< In UMTS mode, this is RSCP as defined in Section 9.1.1.3 of 3GPP 25.133

	UInt8 qual;			///< In GSM mode, this is RXQUAL as defined in Section 8.2.4 of 3GPP 05.08
						///< In UMTS mode, this is Ec/Io as defined in Section 9.1.2.3 if 3GPP 25.133

	UInt8 rat;			///< Current Radio Access Technology: RAT_NOT_AVAILABLE(0),RAT_GSM(1),RAT_UMTS(2)
} RxSignalInfo_t;

/// Rx Signal Level
typedef struct
{
	Boolean signal_lev_changed;
	Boolean signal_qual_changed;
} RX_SIGNAL_INFO_CHG_t;


////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
//
// Common Section:
//
// The following section of definition is commonly used in various components
//
//

#define DIAL_STRING_INTERNATIONAL_CODE		'+'			///< Dial String International Code	 

//
//
// End of Section: (Common)
//  
////////////////////////////////////////////////////////////////////////////////////////////


   

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
//
// CC and SS Section:
//
// The following section of definition is used by CC and SS components
//
//
#define MAX_NAME_SIZE						160		///< Max Name Size


/// enum : SS Service Response Type
typedef enum
{
	SS_SRV_TYPE_NONE,						///< NONE
	SS_SRV_TYPE_FORWARDING_INFORMATION,		///< Forwarding Information
	SS_SRV_TYPE_CALL_BARRING_INFORMATION,	///< Call Barring Information
	SS_SRV_TYPE_SS_DATA_INFORMATION,		///< Call Barring Information
	SS_SRV_TYPE_SS_STATUS,					///< SS Status
	SS_SRV_TYPE_FORWARDED_TO_NUMBER,		///< Forwarded To Number
	SS_SRV_TYPE_BASIC_SRV_INFORMATION,		///< Basic Service Group Information
	SS_SRV_TYPE_GENERIC_SRV_INFORMATION,	///< Generic Service Information
	SS_SRV_TYPE_NEW_PASSWORD,				///< New Passsord
	SS_SRV_TYPE_PH1_USSD_INFORMATION,		///< Phase 1 USSD Information
	SS_SRV_TYPE_PH2_USSD_INFORMATION,		///< Phase 2 USSD Information
	SS_SRV_TYPE_USSD_SS_NOTIFY,				///< USSD SS Notify
	SS_SRV_TYPE_RETURN_ERROR,				///< Return Error
	SS_SRV_TYPE_REJECT,						///< Reject
	SS_SRV_TYPE_LOCAL_ERROR,				///< Local Error (Internal Problem)
	SS_SRV_TYPE_LOCAL_RESULT,				///< Local Result (Internal Response)
	SS_SRV_TYPE_CAUSE_IE					///< Network Cause IE
} SS_SrvType_t;								///< Service Response Type


typedef enum
{
	SS_CODE_ALL_SS						= 0x00, ///<  0 : All SS
	SS_CODE_ALL_LINE_IDENTIFICATION		= 0x10, ///< 16 : All Line Identification SS
	SS_CODE_CLIP						= 0x11, ///< 17 : Calling Line Identification Presentation
	SS_CODE_CLIR						= 0x12, ///< 18 : Calling Line Identification Restriction
	SS_CODE_COLP						= 0x13, ///< 19 : Connected Line Identification Presetation
	SS_CODE_COLR						= 0x14, ///< 20 : Connected Line Identification Restriction
	SS_CODE_MCI							= 0x15, ///< 21 : Malicious Call Identification
	SS_CODE_ALL_NAME_IDENTIFICATION		= 0x18, ///< 24 : All Name Identification SS
	SS_CODE_CNAP						= 0x19, ///< 25 : Calling Name Presentation

	SS_CODE_ALL_FORWARDING				= 0x20, ///< 32 : All Forwarding
	SS_CODE_CFU							= 0x21, ///< 33 : Call Forwarding Unconditional
	SS_CODE_CALL_DEFLECTION				= 0x24, ///< 36 : Call Deflection
	SS_CODE_ACF							= 0x28, ///< 40 : All Conditional Forwarding
	SS_CODE_CFB							= 0x29, ///< 41 : Call Forwarding On Mobile Subscriber Busy
	SS_CODE_CFNRY						= 0x2A, ///< 42 : Call Forwarding On No Reply
	SS_CODE_CFNRC						= 0x2B, ///< 43 : Call Forwarding On Mobile Subscriber Not Reachable

	SS_CODE_ALL_CALL_OFFERING			= 0x30, ///< 48 : All Call Offering SS Includes All Forwarding SS
	SS_CODE_ECT							= 0x31, ///< 49 : Explicit Call Transfer
	SS_CODE_MAH							= 0x32, ///< 50 : Mobile Access Hunting

	SS_CODE_ALL_CALL_COMPLETION			= 0x40, ///< 64 : All Call Completion SS
	SS_CODE_CW							= 0x41, ///< 65 : Call Waiting
	SS_CODE_HOLD						= 0x42, ///< 66 : CAll Hold
	SS_CODE_CCBS_A						= 0x43, ///< 67 : Complition Of Call To Busy Subscriber (Originating Side)
	SS_CODE_CCBS_B						= 0x44, ///< 68 : Complition Of Call To Busy Subscriber (Terminating Side)
	SS_CODE_MC							= 0x45, ///< 69 : Multi Call

	SS_CODE_ALL_MPTY					= 0x50, ///< 80 : All Multiparty SS
	SS_CODE_MPTY						= 0x51, ///< 81 : Multiparty

	SS_CODE_ALL_COMMUNITY_OF_INTEREST	= 0x60, ///< 96 : All Community Of Interest SS
	SS_CODE_CUG							= 0x61, ///< 97 : Closed User Group

	SS_CODE_ALL_CHARGING_SS				= 0x70, ///<112 : All Charging SS
	SS_CODE_AOCI						= 0x71, ///<113 : Advice Of Charge Information
	SS_CODE_AOCC						= 0x72, ///<114 : Advice Of Charge Charging

	SS_CODE_ALL_ADD_INFO_TRANSFER		= 0x80, ///<128 : All Additional Information Transfer
	SS_CODE_UUS_1						= 0x81, ///<129 : User-To-User Signalling 1
	SS_CODE_UUS_2						= 0x82, ///<130 : User-To-User Signalling 2
	SS_CODE_UUS_3						= 0x83, ///<131 : User-To-User Signalling 3

	SS_CODE_ALL_CALL_RESTRICTION		= 0x90, ///<144 : All Call Restriction SS
	SS_CODE_BOC							= 0x91, ///<145 : Barring Of Outgoing Calls
	SS_CODE_BAOC						= 0x92, ///<146 : Barring Of All Outgoing Calls
	SS_CODE_BOIC						= 0x93, ///<147 : Barring Of Outgoing International Calls
	SS_CODE_BOIC_EXC_PLMN				= 0x94, ///<148 : Barring Of Outgoing International Calls Except Those Directed To The Home PLMN
	SS_CODE_BIC							= 0x99, ///<153 : Barring Of Incomming Calls
	SS_CODE_BAIC						= 0x9A, ///<154 : Barring Of All Incoming Calls
	SS_CODE_BAIC_ROAM					= 0x9B, ///<155 : Barring Of All Incoming Calls When Roaming Outside Home PLMN_Country

	SS_CODE_ALL_CALL_PRIORITY			= 0xA0, ///<160 : All Call Priority SS
	SS_CODE_EMLPP						= 0xA1, ///<161 : Enhanced Multilevel Precedence Pre-emption (EMLPP) Service

	SS_CODE_ALL_LCS_PRIVACY_EXCEPTION	= 0xB0, ///<176 : All LCS Privacy Exception - All LCS Privacy Exception Classes
	SS_CODE_UNIVERSAL_LCS				= 0xB1, ///<177 : Universal - Allow Location By Any LCS Client
	SS_CODE_CALL_RELATED_LCS			= 0xB2, ///<178 : Call Related - Allow Location By Any Value Added LCS Client To Which A Call Is Established From The Target MS
	SS_CODE_CALL_UNRELATED_LCS			= 0xB3, ///<179 : Call Unrelated - Allow Location By Designated External value Added LCS Client
	SS_CODE_PLMN_OPERATOR_LCS			= 0xB4, ///<180 : PLMN Operator - Allow Location By designated By PLMN Operator LCS Clients

	SS_CODE_ALL_MORLR					= 0xC0, ///<192 : All MORL SS - All Mobile Originating Location Request Classes
	SS_CODE_BASIC_SELF_LOCATION			= 0xC1, ///<193 : Basic Self Location- Allow An MS To Request Its Own Location
	SS_CODE_AUTONOMOUS_SELF_LOCATION	= 0xC2, ///<194 : Auto Nomous Self Location - Allow An MS To Perform Self Location Without Interaction With The PLMN For A Predetermined Period Of Time
	SS_CODE_TRANSFER_TO_THIRD_PARTY		= 0xC3, ///<195 : Transfer To Third Party - Allow An MS To Request Transfer Of Its Location To Another LCS Client

	SS_CODE_ALL_PLMN_SPECIFIC_SS		= 0xF0, ///<208 : All PLMN Specific SS
	SS_CODE_PLMN_SPECIFIC_SS_1			= 0xF1, ///<209 : PLMN Specific SS 1
	SS_CODE_PLMN_SPECIFIC_SS_2			= 0xF2, ///<210 : PLMN Specific SS 2
	SS_CODE_PLMN_SPECIFIC_SS_3			= 0xF3, ///<211 : PLMN Specific SS 3
	SS_CODE_PLMN_SPECIFIC_SS_4			= 0xF4, ///<212 : PLMN Specific SS 4
	SS_CODE_PLMN_SPECIFIC_SS_5			= 0xF5, ///<213 : PLMN Specific SS 5
	SS_CODE_PLMN_SPECIFIC_SS_6			= 0xF6, ///<214 : PLMN Specific SS 6
	SS_CODE_PLMN_SPECIFIC_SS_7			= 0xF7, ///<215 : PLMN Specific SS 7
	SS_CODE_PLMN_SPECIFIC_SS_8			= 0xF8, ///<216 : PLMN Specific SS 8
	SS_CODE_PLMN_SPECIFIC_SS_9			= 0xF9, ///<217 : PLMN Specific SS 9
	SS_CODE_PLMN_SPECIFIC_SS_A			= 0xFA, ///<218 : PLMN Specific SS A
	SS_CODE_PLMN_SPECIFIC_SS_B			= 0xFB, ///<219 : PLMN Specific SS B
	SS_CODE_PLMN_SPECIFIC_SS_C			= 0xFC, ///<220 : PLMN Specific SS C
	SS_CODE_PLMN_SPECIFIC_SS_D			= 0xFD, ///<221 : PLMN Specific SS D
	SS_CODE_PLMN_SPECIFIC_SS_E			= 0xFE, ///<222 : PLMN Specific SS E
	SS_CODE_PLMN_SPECIFIC_SS_F			= 0xFF  ///<223 : PLMN Specific SS F
} SS_Code_t;										///<SS Code type


/// Enum : Call On Hold
typedef enum
{
	SS_CALL_HOLD_IND_CALL_RETRIEVE,				///< Call Retrieve
	SS_CALL_HOLD_IND_CALL_ON_HOLD				///< Call On Hold
} SS_CallHold_t;


/// Enum : ECT Call State
typedef enum 
{
	SS_ECT_CALL_STATE_ALERTING,					///< Alerting
	SS_ECT_CALL_STATE_ACTIVE					///< Active
} SS_EctCallState_t;							///< ECT Call State Type


/// Enum : Presentation
typedef enum 
{
	SS_PRESENTATION_NONE,						///< None
	SS_PRESENTATION_ALLOWED_ADDRESS,			///< Presentation Allowed Address
	SS_PRESENTATION_RESTRICTED,					///< Presentation Allowed Address
	SS_PRESENTATION_UNAVAIABLE,					///< Number/Name Not Available
	SS_PRESENTATION_RESTRICTED_ADDRESS			///< Presentation Restricted Address
} SS_Presentation_t;							///< Presentation Type


/// Structure : ECT Indicator Information
typedef struct
{
	SS_EctCallState_t	ectCallState;			///< ECT Call State
	SS_Presentation_t	type;					///< Presentation Type
	PartyAddress_t		partyAdd;				///< Party Address
	PartySubAdd_t		partySubAdd;			///< Party Subaddress
} SS_EctIndInfo_t;								///< ECT Indicator Information



/// Structure : Name Indicator (CNAP feature)
typedef struct
{
	SS_Presentation_t	type;					///< Presentation Type
	UInt8				dcs;					///< Data Coding Scheme
	UInt8				lengthInChar;			///< Length In Character
	UInt8				dataLenght;				///< Data Length
	char				data[MAX_NAME_SIZE];	///< Name string
} SS_NameInd_t;

/**
Union:	Alerting Alert Pattern
		For the content type refere to SS_AlertCategory2_t, SS_AlertLevel_t and SS_AlertCategory2_t
		-- This type is used to represent Alerting Pattern
		--	bits 8765 : 0000 (unused)

		--	bits 43 : type of Pattern
		--		00 level
		--		01 category
		--		10 category
		--		all other values are reserved.

		--	bits 21 : type of alerting

		alertingLevel-0   AlertingPattern ::= '00000000'B
		alertingLevel-1   AlertingPattern ::= '00000001'B
		alertingLevel-2   AlertingPattern ::= '00000010'B

		-- all other values of Alerting level are reserved
		-- Alerting Levels are defined in GSM 02.07
	
		alertingCategory-1   AlertingPattern ::= '00000100'B
		alertingCategory-2   AlertingPattern ::= '00000101'B
		alertingCategory-3   AlertingPattern ::= '00000110'B
		alertingCategory-4   AlertingPattern ::= '00000111'B
		alertingCategory-5   AlertingPattern ::= '00001000'B

		-- all other values of Alerting Category are reserved
		-- Alerting categories are defined in GSM 02.07
**/
typedef UInt8 SS_AlertPattern_t;				///< Alerting Pattern Type



/// Enumeration of Basic Service Type
typedef enum
{
	BASIC_SERVICE_TYPE_UNSPECIFIED		= 0x00,	///<  0 : Unspecified service type
	BASIC_SERVICE_TYPE_UNKNOWN			= 0x01,	///<  1 : Unknown service type
	BASIC_SERVICE_TYPE_BEARER_SERVICES	= 0x82, ///<130 : Bearer Services		
	BASIC_SERVICE_TYPE_TELE_SERVICES	= 0x83	///<131 : Tele Services
} BasicSrvType_t;


/// Structure : Basic Service Group
typedef struct
{
	BasicSrvType_t	type;
	UInt8			content;					///< Content is of type BearerSrv_t/TeleSrv_t
}BasicSrvGroup_t;								///< Basic Service Group Type


/**
@code
Structure : CCBS Feature
			include			  8   7   6   5   4   3   2   1
							|   |   |   |   |   |   |   |   |
							  ^   ^   ^   ^   ^   ^   ^   ^					   
			partyAdd									  X		CCBS Index
			partyAdd								  X			Party Address
			partySubAdd							  X 			Party Subaddress
			basicSrv						  X					Basic Services
@endcode
**/
typedef struct
{
	UInt8				include;				///< Included Optional Elements
	UInt16				ccbsIndex;				///< CCBS Index
	PartyAddress_t		partyAdd;				///< Party Address
	PartySubAdd_t		partySubAdd;			///< Party Subaddress
	BasicSrvGroup_t 	basicSrv;				///< Basic Service Group
} SS_CcbsFeature_t;								///< CCBS Feature Type



/**
@code
Structure : Notify SS
			include       8   7   6   5   4   3   2   1
						|   |   |   |   |   |   |   |   |
						  ^   ^   ^   ^   ^   ^   ^   ^	
			ssCode									  X		SS Code
			ssStatus							  X 		SS Status
			ssNotific						  X				SS Notification
			callWait					  X					Call Waiting Indicator
			callHold				  X 					Call On Hold Indicator
			mptyInd				  X							MPTY Indicator
			cugInd			  X								CUG Index
			clirSupRej	  X 								CLIR Suppression Rejected

						  16 15  14  13  12  11  10   9
						|   |   |   |   |   |   |   |   |
						  ^   ^   ^   ^   ^   ^   ^   ^	
			ectInd									  X		ECT Indicator
			nameInd								  X			Name Indicator
			ccbsFeature						  X 			CCBS Feature
			alertPatt					  X					Alerting Pattern
@endcode
**/
typedef struct
{
	UInt16				include;				///< Included Optional Elements
	SS_Code_t			ssCode;					///< SS Code
	UInt8				ssStatus;				///< SS Status
	UInt8				ssNotific;				///< SS Notification See SS_Notification_t
	SS_CallHold_t		callHold;				///< Call On Hold Indicator
	UInt16				cugIndex;				///< CUG Index
	SS_EctIndInfo_t		ectInd;					///< ECT Indicator
	SS_NameInd_t		nameInd;				///< Name Indicator
	SS_CcbsFeature_t	ccbsFeature;			///< CCBS Feature
	SS_AlertPattern_t	alertPatt;				///< Alerting Pattern
}SS_NotifySs_t;									///< Notify SS Type



/// Enum : Return Error Component Type
typedef struct
{
	SS_ErrorCode_t				errorCode;			///< Error Code
#if 0
	union
	{
		SS_CallBarredError_t	callBarredError;	///< Call Barred Error
		UInt8					ssStatus;			///< SS Status
		SS_SubsOption_t			subsOption;			///< Subscription Option
		SS_SsIncompatibility_t	ssIncompatib;		///< SS Incompatibility
		SS_MwdError_t			mwdSubsAbsent;		///< MWD Subscriber Absent Code
		SS_SystemFailure_t		systemFailure;		///< System Failure
		SS_PwdRegFailure_t		pwdRegFailure;		///< PWD Register Failure
	} paramError;
#endif
} SS_ReturnError_t;	



/// Enum : Problem Code Type
typedef enum
{
	PROBLEM_CODE_TYPE_NONE,						///< Problem Code Type None
	PROBLEM_CODE_TYPE_GENERAL		= 0x80,		///< Problem Code Type General
	PROBLEM_CODE_TYPE_INVOKE		= 0x81,		///< Problem Code Type Invoke
	PROBLEM_CODE_TYPE_RETURN_RESULT	= 0x82,		///< Problem Code Type Return Result
	PROBLEM_CODE_TYPE_RETURN_ERROR	= 0x83		///< Problem Code Type Return Error
} SS_ProblemCode_t;								///< SS Problem Code


/// Structure : Reject Component Type
typedef struct
{
	SS_ProblemCode_t	problemType;
	UInt8				content;
} SS_Reject_t;	///< SS Reject Component Type

//
//
// End of Section: (CC and SS)
//  
////////////////////////////////////////////////////////////////////////////////////////////


/// PLMN List returned
typedef struct
{
   UInt8 size; 
   SEARCHED_PLMNId_t *list;
} PLMNList_t;


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



/// Call establish message structure
typedef struct
{
	UInt8					callNum[MAX_DIGITS+2];  ///< Calling number (Note: 1 byte for null
													///< termination and the other for int code '+')
	CLIRMode_t				clir;					///< Caller line ID restriction
	CUGInfo_t				cug_info;				///< Closed User Group info
	Boolean					auxiliarySpeech;		///< Aux. speech line
	Boolean					isEmergency;			///< Is this an emergency call
	Subaddress_t			subAddr;				///< Sub address
	CC_BearerCap_t 			bearerCap;				///< Required bearer capability for the call	
	
} CallEstablishMsg_t;


typedef struct
{
	UInt8				ton;					// Type of number
	UInt8				npi;					// Numbering plan
	PresentationInd_t	present;
	ScreenInd_t			screen;
	UInt8				c_num;					// Number of BCD digits in num
	UInt8				num[ MAX_DIGITS + 1];	// BCD digits ( 1 digit / byte )
} CallingParty_t;


/// Call Receive Message
typedef struct
{
	UInt8			callIndex;			///< Index associated with the call
	CUGIndex_t		cug_index;			///< Closed User Group Index
	CallingParty_t	callingInfo;		///< Calling Information
	Boolean			auxiliarySpeech;	///< Aux Speech Line info
	Signal_t		setup_ind_signal;	///< setup ind signal type (tone type)
	UInt8			codecId;			///< provides a codecId based on 3GPP26.103 Sec 6.3.as of now the only valid
										///< values are 0x0a and 0x06. 0xFF is invalid.
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

#ifdef __cplusplus
}
#endif

#endif // _COMMON_DEFS_H_
