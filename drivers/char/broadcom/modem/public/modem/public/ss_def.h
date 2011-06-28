//*********************************************************************
//
//	Copyright © 2008 Broadcom Corporation
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
*   @file   ss_def.h
*
*   @brief	This file contains Supplementary Services related definitions.
*
****************************************************************************/
#ifndef _SS_DEF_H_
#define _SS_DEF_H_


#define	IEI_CAUSE							0x08	///< Cause Information Element identifier
#define	IEI_FACILITY						0x1C	///< Facility Information Element identifier
#define	IEI_USER_TO_USER					0x7E	///< User To User Information Element identifier
#define	IEI_MORE_DATA						0xA0	///< More Data Information Element identifier
#define	IEI_SS_VERSION						0x7F	///< SS Version Information Element identifier

#define MAX_USSD_SIZE						200		///< Max USSD Size
#define	SS_PASSWORD_LENGTH					4		///< Password Length
#define MAX_FEATURE_LIST_SIZE				13		///< Max Feature List Size
#define MAX_CCBS_LIST_SIZE					5		///< Max CCBS List Size
#define ALL_CCBS_INDEX						0xFF	///< All CCBS Indexes
#define INDEFINITE_LENGTH_INDICATOR			0x80	///< Infinite Length Indicator
#define MAX_SS_REQ_RESEND					0x02	///< Resend limite in case SATK initiated svc req failure
#define	TERMITATE_TIMER_60_SEC		(Ticks_t)(60 * TICKS_ONE_SECOND )	///< Terminate Timer 60 Second
#define	RESEND_STK_SS_TIMER_5_SEC	(Ticks_t)(5 * TICKS_ONE_SECOND )	///< Resend Timer 30 Second

//masks for ssStatus as per 3GPP 29.002 17.7.4
#define SS_STATUS_MASK_ACTIVE				0x01	///< SS-Status Mas Active
#define SS_STATUS_MASK_REGISTER				0x02	///< SS-Status Mas Register
#define SS_STATUS_MASK_PROVISIONED			0x04	///< SS-Status Mas Provision
#define SS_STATUS_MASK_QUIESCENT			0x08	///< SS-Status Mas Quenscent

//Masks for SS-Notification content
#define SS_NOTIF_INCOMING_CALL_IS_FWD_CALL	0x01	///< SS-Notification Incoming call is a forwarded call
#define SS_NOTIF_INCOMING_CALL_IS_FWD_TO_C	0x02	///< SS-Notification Incoming call has been forwarded to C
#define SS_NOTIF_OUTGOING_CALL_IS_FWD_TO_C	0x04	///< SS-Notification Outgoing call is forwarded to C

#define	SS_INVOKE_ID_ELEMENT_TAG			0x02	///< Invoke Identifier Element Tag
#define SS_LINK_ID_ELEMENT_TAG				0x80	///< Link Identifier Element Tag
#define SS_OPERATION_CODE_ELEMENT_TAG		0x02	///< Operation Code Element Tag
#define	SS_SS_CODE_TAG						0x04	///< SS Code Tag
#define SS_ENUMERATED_TAG					0X0A	///< Enumarated Tag
#define SS_SS_STATUS_TAG					0x04	///< SS Status Tag
#define SS_PASSWORD_TAG						0x12	///< Password Tag
#define SS_USER_DATA_IA5_STRING_TAG			0x16	///< User Data IA5 String Tag
#define SS_DATA_CODE_SCHEME_TAG				0x04	///< Data Code Scheme Tag
#define SS_USSD_STRING_TAG					0x04	///< Error Code Tag
#define	SS_ERROR_CODE_TAG					0x02	///< Guidance Information Tag
#define SS_GUIDANCE_INFO_TAG				0X0A	///< Empty Tag
#define SS_EMPTY_TAG						0x05	///< Implicit Tag
#define	SS_SEQUENCE_TAG						0x30	///< Sequence Tag
#define SS_IMPLICIT_TAG						0x80	///< Implicite Tag
#define SS_IMPLICITE_SEQ_TAG				0xA0	///< Implicite Sequence Tag


#define SS_ALERTING_PATTERN_TAG				0x04	///< Alerting Pattern Tag 

//Macro
#define	SS_IMPLICIT(tag)		(SS_IMPLICIT_TAG | tag)
#define	SS_IMPLICIT_SEQ(tag)	(SS_IMPLICITE_SEQ_TAG | tag)

#define SS_OPERATION_CODE_REGISTER_STR	"**"	///< SS Operation code register code
#define SS_CODE_ALL_FORWARDING_STR		"002"	///< SS Code all forwarding code
#define SS_CODE_CFU_STR					"21"	///< SS Code CFU code
#define SS_CODE_ACF_STR					"004"	///< SS Code ACF code
#define SS_CODE_CFB_STR					"67"	///< SS Code CFB code
#define SS_CODE_CFNRY_STR				"61"	///< SS Code CFNRY code
#define SS_CODE_CFNRC_STR				"62"	///< SS Code CFNRC code


//temp CLIR modes as define in 07.07
#define TEMP_ALLOW_CLIR_CODE		'i'
#define TEMP_RESTRICT_CLIR_CODE		'I'
#define TEMP_ALLOW_CLIR_STR			"*31#"
#define TEMP_RESTRICT_CLIR_STR		"#31#"
#define TEMP_ALLOW_CLIR_KT_STR		"*23#"
#define TEMP_RESTRICT_CLIR_KT_STR	"#23#"


//******************************************************************************
//
//	Typedefs for SS Message
//
//******************************************************************************
#define	SS_RELCOMPLETE_MSG		0x2A			// same as Begin Ind Mesg
#define	SS_FACILITY_MSG			0x3A			// same as Facility Ind Mesg
#define	SS_REGISTER_MSG			0x3B			// same as End Ind Mesg
/// Structure : Cause Type, refer to 24.008, section 10.5.4.11
typedef struct
{
	UInt8	codStandard;					///< Coding Standard
	UInt8	location;						///< Location
	UInt8	recommendation;					///< Recommendation
	Cause_t	cause;							///< Cause
	UInt8	diagnostic;						///< Diagnostic
} CauseIe_t;								///< Cause Information Element Type

/// Enum : Type of Number and National Plan Idetifier
typedef struct
{
	BitField extended:1;
	BitField typeOfNumb:3;
	BitField numbPlanId:4;
}TonNpi_t;

/// Enum : BCD Number
typedef enum
{
	BCD_NUMBER_1,								///< BCD Number 1
	BCD_NUMBER_2,								///< BCD Number 2
	BCD_NUMBER_3,								///< BCD Number 3
	BCD_NUMBER_4,								///< BCD Number 4
	BCD_NUMBER_5,								///< BCD Number 5
	BCD_NUMBER_6,								///< BCD Number 6
	BCD_NUMBER_7,								///< BCD Number 7
	BCD_NUMBER_8,								///< BCD Number 8
	BCD_NUMBER_9,								///< BCD Number 9
	BCD_NUMBER_star,							///< BCD Number *
	BCD_NUMBER_pound,							///< BCD Number #
	BCD_NUMBER_P,								///< BCD Number a
	BCD_NUMBER_A,								///< BCD Number b
	BCD_NUMBER_B,								///< BCD Number c
	BCD_NUMBER_END								///< BCD Number End
} BcdNumber_t;									///< BCD Number Type



/// Enumeration of SS Operation
typedef enum
{
	SS_OPERATION_CODE_NONE							= 0x00,	///< No Operation code
															///< MAP-Mobile Service Operation
	SS_OPERATION_CODE_FORWARD_CHECK_SS_INDICATION	= 0x26,	///< 38 : Forward Check SS Indication

															///< MAP-SS Operation
	SS_OPERATION_CODE_REGISTER						= 0x0A,	///< 10 : Register
	SS_OPERATION_CODE_ERASE							= 0x0B,	///< 11 : Erase
	SS_OPERATION_CODE_ACTIVATE						= 0x0C,	///< 12 : Activate
	SS_OPERATION_CODE_DEACTIVATE					= 0x0D,	///< 13 : Deactivate
	SS_OPERATION_CODE_INTERROGATE					= 0x0E,	///< 14 : Interrogate
	SS_OPERATION_CODE_REGISTER_PASSWORD				= 0x11,	///< 17 : Register Password
	SS_OPERATION_CODE_GET_PASSWORD					= 0x12,	///< 18 : Get Password
	SS_OPERATION_CODE_PROCESS_UNSTRUCTURED_SS_DATA	= 0x13,	///< 19 : Process Unstructured SS Data
	SS_OPERATION_CODE_PROCESS_UNSTRUCTURED_SS_REQ	= 0x3B,	///< 59 : Process Unstructured SS Request
	SS_OPERATION_CODE_UNSTRUCTURED_SS_REQEST		= 0x3C,	///< 60 : Unstructured SS Request
	SS_OPERATION_CODE_UNSTRUCTURED_SS_NOTIFY		= 0x3D,	///< 61 : Unstructured SS Notify
	SS_OPERATION_CODE_ERASE_CC_ENTRY				= 0x4D,	///< 77 : Erase CC Entry
	SS_OPERATION_CODE_LCS_PERIODIC_LOCATION_CANCEL	= 0x67,	///<109 : LCS Periodic Location Cancellation
	SS_OPERATION_CODE_LCS_LOCATION_UPDATE			= 0x68,	///<110 : LCS Location Update
	SS_OPERATION_CODE_LCS_PERIODIC_LOCATION_REQ		= 0x69,	///<111 : LCS Periodic Location Request
	SS_OPERATION_CODE_LCS_AREA_EVENT_CANCEL			= 0x70,	///<112 : LCS Area Event Cancellation
	SS_OPERATION_CODE_LCS_AREA_EVENT_REPORT			= 0x71,	///<113 : LCS Area Event Report
	SS_OPERATION_CODE_LCS_AREA_EVENT_REQUEST		= 0x72,	///<114 : LCS Area Event Request
	SS_OPERATION_CODE_LCS_MOLR						= 0x73, ///<115 : LCS_MOLR
	SS_OPERATION_CODE_LCS_LOCATION_NOTIFICATION		= 0x74, ///<116 : LCS Location Notification

															///< SS Operation
	SS_OPERATION_CODE_NOTIFY_SS						= 0x10, ///< 16 : Notify SS
	SS_OPERATION_CODE_CALL_DEFLECTION				= 0x75, ///<117 : Call Deflection
	SS_OPERATION_CODE_USER_USER_SERVICE				= 0x76, ///<118 : User User Service
	SS_OPERATION_CODE_ACCESS_REGISTER_CC_ENTRY		= 0x77, ///<119 : Access Register CC Entry
	SS_OPERATION_CODE_FORWARD_CUG_INFO				= 0x78, ///<120 : Forward CUG-Info
	SS_OPERATION_CODE_SPLIT_MPTY					= 0x79, ///<121 : Split MPTY
	SS_OPERATION_CODE_RETIEVE_MPTY					= 0x7A, ///<122 : Retrieve MPTY
	SS_OPERATION_CODE_HOLD_MPTY						= 0x7B, ///<123 : Hold MPTY
	SS_OPERATION_CODE_BUILD_MPTY					= 0x7C, ///<124 : Build MPTY
	SS_OPERATION_CODE_FORWARD_CHARGE_ADVICE			= 0x7D, ///<125 : Forward Charge Advice
	SS_OPERATION_CODE_EXPLICIT_CT					= 0x7E  ///<126 : Explicit CT
} SS_Operation_t;


/// Enumeration of Basic Service String
typedef enum
{
	STR_BEARER_SRV_ALL_BEARER_SERVICES				= 20,
	STR_BEARER_SRV_ALL_ASYNCHRONOUS_SERVICES		= 21, 
	STR_BEARER_SRV_ALL_SYNCHRONOUS_SERVICES			= 22,
	STR_BEARER_SRV_ALL_DATA_CIRCUIT_SYNCH			= 24,
	STR_BEARER_SRV_ALL_DATA_CIRCUIT_ASYNCH			= 25,
	STR_BEARER_SRV_ALL_PLMN_SPECIFIC_BASIC_SRV		= 70,
	STR_BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_1		= 71,
	STR_BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_2		= 72,
	STR_BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_3		= 73,
	STR_BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_4		= 74,
	STR_BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_5		= 75,
	STR_BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_6		= 76,
	STR_BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_7		= 77,
	STR_BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_8		= 78,
	STR_BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_9		= 79,
	STR_BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_A		= 80,
	STR_BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_B		= 81,
	STR_BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_C		= 82,
	STR_BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_D		= 83,
	STR_BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_E		= 84,
	STR_BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_F		= 85,
	STR_BEARER_SRV_ALL_DATA_CDA_SERVICES			= 99,
	STR_BEARER_SRV_DATA_CDA_300_BPS					= 99,
	STR_BEARER_SRV_DATA_CDA_1200_BPS				= 99,
	STR_BEARER_SRV_DATA_CDA_1200_75_BPS				= 99,
	STR_BEARER_SRV_DATA_CDA_2400_BPS				= 99,
	STR_BEARER_SRV_DATA_CDA_4800_BPS				= 99,
	STR_BEARER_SRV_DATA_CDA_9600_BPS				= 99,
	STR_BEARER_SRV_ALL_DATA_CDS_SERVICES			= 99,
	STR_BEARER_SRV_DATA_CDS_1200_BPS				= 99,
	STR_BEARER_SRV_DATA_CDS_2400_BPS				= 99,
	STR_BEARER_SRV_DATA_CDS_4800_BPS				= 99,
	STR_BEARER_SRV_DATA_CDS_9600_BPS				= 99,
	STR_BEARER_SRV_ALL_PAD_ACCESS_CA_SERVICES		= 99,
	STR_BEARER_SRV_PAD_ACCESS_CA_300_BPS			= 99,
	STR_BEARER_SRV_PAD_ACCESS_CA_1200_BPS			= 99,
	STR_BEARER_SRV_PAD_ACCESS_CA_1200_75_BPS		= 99,
	STR_BEARER_SRV_PAD_ACCESS_CA_2400_BPS			= 99,
	STR_BEARER_SRV_PAD_ACCESS_CA_4800_BPS			= 99,
	STR_BEARER_SRV_PAD_ACCESS_CA_9600_BPS			= 99,
	STR_BEARER_SRV_ALL_DATA_PDS_SERVICES			= 99,
	STR_BEARER_SRV_DATA_PDS_2400_BPS				= 99,
	STR_BEARER_SRV_DATA_PDS_4800_BPS				= 99,
	STR_BEARER_SRV_DATA_PDS_9600_BPS				= 99,
	STR_BEARER_SRV_ALL_ALTER_SPEECH_DATA_CDA		= 99,
	STR_BEARER_SRV_ALL_ALTER_SPEECH_DATA_CDS		= 99,
	STR_BEARER_SRV_ALL_SPEECH_FOLLOWED_DATA_CDA		= 99,
	STR_BEARER_SRV_ALL_SPEECH_FOLLOWED_DATA_CDS		= 99,

	STR_TELE_SRV_ALL_TELE_SERVICES					= 10,
	STR_TELE_SRV_ALL_SPEECH_TRANSMISSION_SERVICES	= 11,
	STR_TELE_SRV_TELEPHONY							= 11,
	STR_TELE_SRV_EMERGENCY_CALLS					= 12,
	STR_TELE_SRV_ALL_DATA_TELE_SERVICES				= 12,
	STR_TELE_SRV_ALL_FACIMILE_TRANSMISSION_SERVICES	= 13,
	STR_TELE_SRV_FACIMILE_GROUP_3_ALTER_SPEECH		= 13,
	STR_TELE_SRV_AUTOMATIC_FACIMILE_GROUP_3			= 13,
	STR_TELE_SRV_FACIMILE_GROUP_4					= 13,
	STR_TELE_SRV_ALL_SHORT_MESSAGE_SERVICES			= 16,
	STR_TELE_SRV_SHORT_MESSAGE_MT_PP				= 16,
	STR_TELE_SRV_SHORT_MESSAGE_MO_PP				= 16,
	STR_TELE_SRV_VOICE_GROUP_CALL_SERVICE			= 17,
	STR_TELE_SRV_VOICE_BROADCAST_SERVICE			= 18,
	STR_TELE_SRV_ALL_TELE_SRV_EXCEPT_SMS			= 19,
	STR_TELE_SRV_TELEPHONY_AND_ALL_SYNC_SERVICES	= 26,
	STR_TELE_SRV_ALL_PLMN_SPECIFIC_SERVICES			= 50,
	STR_TELE_SRV_PLMN_SPECIFIC_1					= 51,
	STR_TELE_SRV_PLMN_SPECIFIC_2					= 52,
	STR_TELE_SRV_PLMN_SPECIFIC_3					= 53,
	STR_TELE_SRV_PLMN_SPECIFIC_4					= 54,
	STR_TELE_SRV_PLMN_SPECIFIC_5					= 55,
	STR_TELE_SRV_PLMN_SPECIFIC_6					= 56,
	STR_TELE_SRV_PLMN_SPECIFIC_7					= 57,
	STR_TELE_SRV_PLMN_SPECIFIC_8					= 58,
	STR_TELE_SRV_PLMN_SPECIFIC_9					= 59,
	STR_TELE_SRV_PLMN_SPECIFIC_A					= 60,
	STR_TELE_SRV_PLMN_SPECIFIC_B					= 61,
	STR_TELE_SRV_PLMN_SPECIFIC_C					= 62,
	STR_TELE_SRV_PLMN_SPECIFIC_D					= 63,
	STR_TELE_SRV_PLMN_SPECIFIC_E					= 64,
	STR_TELE_SRV_PLMN_SPECIFIC_F					= 65,
	STR_TELE_SRV_ALL_AUXILIARY_TELEPHONY			= 89
} BasicSrvSrvCode_t;


/// Enumeration of Bearer Service Group
typedef enum
{
	BEARER_SRV_GROUP_ALL_BEARER_SERVICES				= 0x00, ///< 00 : All Bearer Services
	BEARER_SRV_GROUP_ALL_DATA_CDA_SERVICES				= 0x01, ///< 01 : All Data CDA Services
	BEARER_SRV_GROUP_ALL_PAD_ACCESS_CA_SERVICES			= 0x02, ///< 02 : All PAD Access CA Services
	BEARER_SRV_GROUP_ALL_ALTERNATIVE_SPEECH_DATA_CDA	= 0x03, ///< 03 : All Alternative speech Data CDA
	BEARER_SRV_GROUP_ALL_SPEECH_FOLLOWED_BY_DATA_CDA	= 0x04, ///< 04 : All Speech Followed By Data CDA
	BEARER_SRV_GROUP_ALL_DATA_CIRCUIT_ASYNCHRONOUS		= 0x05, ///< 05 : All Data Circuit Asynchronous
	BEARER_SRV_GROUP_ALL_SYNC_ASYNCHRONOUS_SERVICES		= 0x06, ///< 06 : All Sync/Asynchronous Services
	BEARER_SRV_GROUP_ALL_PLMN_SPECIFIC_SERVICES			= 0x0D  ///< 14 : All PLMN Specific Services
} BearerSrvGroup_t;												///< Bearer Service Group Type

/// Enumeration of Bearer Services
typedef enum
{
	BEARER_SRV_ALL_BEARER_SERVICES			= 0x00, ///< 00 : All Bearer Services
	
	BEARER_SRV_ALL_DATA_CDA_SERVICES		= 0x10, ///< 16 : All Data CDA Services
	BEARER_SRV_DATA_CDA_300_BPS				= 0x11, ///< 17 : Data CDA 300 bps
	BEARER_SRV_DATA_CDA_1200_BPS			= 0x12, ///< 18 : Data CDA 1200 bps
	BEARER_SRV_DATA_CDA_1200_75_BPS			= 0x13, ///< 19 : Data CDA 1200-75 bps
	BEARER_SRV_DATA_CDA_2400_BPS			= 0x14, ///< 20 : Data CDA 2400 bps
	BEARER_SRV_DATA_CDA_4800_BPS			= 0x15, ///< 21 : Data CDA 4800 bps
	BEARER_SRV_DATA_CDA_9600_BPS			= 0x16, ///< 22 : Data CDA 9600 bps

	BEARER_SRV_ALL_DATA_CDS_SERVICES		= 0x18, ///< 24 : All Data CDS Services
	BEARER_SRV_DATA_CDS_1200_BPS			= 0x1A, ///< 26 : Data CDS 1200 bps
	BEARER_SRV_DATA_CDS_2400_BPS			= 0x1C, ///< 28 : Data CDS 2400 bps
	BEARER_SRV_DATA_CDS_4800_BPS			= 0x1D, ///< 29 : Data CDS 4800 bps
	BEARER_SRV_DATA_CDS_9600_BPS			= 0x1E, ///< 30 : Data CDS 9600 bps

	BEARER_SRV_ALL_PAD_ACCESS_CA_SERVICES	= 0x20, ///< 32 : All PAD Access CA Services
	BEARER_SRV_PAD_ACCESS_CA_300_BPS		= 0x21, ///< 33 : PAD ACCESS CA 300 bps
	BEARER_SRV_PAD_ACCESS_CA_1200_BPS		= 0x22, ///< 34 : PAD ACCESS CA 1200 bps
	BEARER_SRV_PAD_ACCESS_CA_1200_75_BPS	= 0x23, ///< 35 : PAD ACCESS CA 1200-75 bps
	BEARER_SRV_PAD_ACCESS_CA_2400_BPS		= 0x24, ///< 36 : PAD ACCESS CA 2400 bps
	BEARER_SRV_PAD_ACCESS_CA_4800_BPS		= 0x25, ///< 38 : PAD ACCESS CA 4800 bps
	BEARER_SRV_PAD_ACCESS_CA_9600_BPS		= 0x26, ///< 39 : PAD ACCESS CA 9600 bps

	BEARER_SRV_ALL_DATA_PDS_SERVICES		= 0x28, ///< 41 : All Data PDS Services
	BEARER_SRV_DATA_PDS_2400_BPS			= 0x2C, ///< 45 : Data PDS 2400 bps
	BEARER_SRV_DATA_PDS_4800_BPS			= 0x2D, ///< 46 : Data PDS 4800 bps
	BEARER_SRV_DATA_PDS_9600_BPS			= 0x2E, ///< 47 : Data PDS 9600 bps

	BEARER_SRV_ALL_ALTER_SPEECH_DATA_CDA	= 0x30, ///< 40 : All Alternate Speech Data CDA
	BEARER_SRV_ALL_ALTER_SPEECH_DATA_CDS	= 0x38, ///< 48 : All Alternate Speech Data CDS

	BEARER_SRV_ALL_SPEECH_FOLLOWED_DATA_CDA	= 0x40, ///< 64 : All Speech Followd Data CDA
	BEARER_SRV_ALL_SPEECH_FOLLOWED_DATA_CDS	= 0x48, ///< 72 : All Speech Followed Data CDS

	BEARER_SRV_ALL_DATA_CIRCUIT_ASYNCH		= 0x50, ///< 80 : All Data Circuit Asynchronous
	BEARER_SRV_ALL_DATA_CIRCUIT_SYNCH		= 0x58, ///< 88 : All Data Circuit Syncronous

	BEARER_SRV_ALL_ASYNCHRONOUS_SERVICES	= 0x60, ///< 96 : All Asynchronous Services
	BEARER_SRV_ALL_SYNCHRONOUS_SERVICES		= 0x68, ///<104 : All Synchronous Services

	BEARER_SRV_ALL_PLMN_SPECIFIC_BASIC_SRV	= 0xD0, ///<208 : All PLMN Specific Services
	BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_1	= 0xD1, ///<209 : PLMN Specific Basic Service 1
	BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_2	= 0xD2, ///<210 : PLMN Specific Basic Service 2
	BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_3	= 0xD3, ///<211 : PLMN Specific Basic Service 3
	BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_4	= 0xD4, ///<212 : PLMN Specific Basic Service 4
	BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_5	= 0xD5, ///<213 : PLMN Specific Basic Service 5
	BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_6	= 0xD6, ///<214 : PLMN Specific Basic Service 6
	BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_7	= 0xD7, ///<215 : PLMN Specific Basic Service 7
	BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_8	= 0xD8, ///<216 : PLMN Specific Basic Service 8
	BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_9	= 0xD9, ///<217 : PLMN Specific Basic Service 9
	BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_A	= 0xDA, ///<218 : PLMN Specific Basic Service A
	BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_B	= 0xDB, ///<219 : PLMN Specific Basic Service B
	BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_C	= 0xDC, ///<220 : PLMN Specific Basic Service C
	BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_D	= 0xDD, ///<221 : PLMN Specific Basic Service D
	BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_E	= 0xDE, ///<222 : PLMN Specific Basic Service E
	BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_F	= 0xDF  ///<223 : PLMN Specific Basic Service F
} BearerSrv_t;

/// Enumeration of Tele Service Group
typedef enum
{
	TELE_SRV_GROUP_ALL_TELE_SERVICES					= 0x00, ///< 00 : All Tele Services
	TELE_SRV_GROUP_ALL_SPEECH_TRANSMISSION_SERVICES		= 0x01, ///< 01 : All Speech Transmission Services
	TELE_SRV_GROUP_ALL_SHORT_MESSAGE_SERVICES			= 0x02, ///< 02 : All Short Message Services
	TELE_SRV_GROUP_ALL_FACIMILE_TRANSMISSION_SERVICES	= 0x06, ///< 06 : All Facimile Transmission Services
	TELE_SRV_GROUP_ALL_DATA_TELE_SERVICES				= 0x07, ///< 07 : All Data Tele Services
	TELE_SRV_GROUP_ALL_TELE_SERVICES_EXCEPT_SMS			= 0x08, ///< 08 : All Tele Services Except SMS
	TELE_SRV_GROUP_ALL_PLMN_SPECIFIC_SERVICES			= 0x0D  ///< 14 : All PLMN Specific Services
} TeleSrvGroup_t;

/// Enumeration of Tele Service
typedef enum
{
	TELE_SRV_ALL_TELE_SERVICES					= 0x00, ///< 00 : All Tele Services
	
	TELE_SRV_ALL_SPEECH_TRANSMISSION_SERVICES	= 0x10, ///< 16 : All Speech Transmission Services
	TELE_SRV_TELEPHONY							= 0x11, ///< 17 : Telephony
	TELE_SRV_EMERGENCY_CALLS					= 0x12, ///< 18 : Emergency Calls

	TELE_SRV_ALL_SHORT_MESSAGE_SERVICES			= 0x20, ///< 32 : All Short Message Services
	TELE_SRV_SHORT_MESSAGE_MT_PP				= 0x21, ///< 33 : Short Message MT PP
	TELE_SRV_SHORT_MESSAGE_MO_PP				= 0x22, ///< 34 : Short Message MP PP

	TELE_SRV_ALL_AUXILIARY_TELEPHONY			= 0x59, ///< 89 : ALS (Alternative Line Service) added to GSM 02.30, see Appendix 1 of CPHS spec

	TELE_SRV_ALL_FACIMILE_TRANSMISSION_SERVICES	= 0x60, ///< 96 : All Facimile Transmission Services
	TELE_SRV_FACIMILE_GROUP_3_ALTER_SPEECH		= 0x61, ///< 97 : Facimile Group 3 Alter Speech
	TELE_SRV_AUTOMATIC_FACIMILE_GROUP_3			= 0x62, ///< 98 : Automatic Facimile Group 3
	TELE_SRV_FACIMILE_GROUP_4					= 0x63, ///< 99 : Facimile Group 4

	TELE_SRV_ALL_DATA_TELE_SERVICES				= 0x70, ///<112 : All Data Tele Services

	TELE_SRV_ALL_TELE_SRV_EXCEPT_SMS			= 0x80, ///<128 : All Tele Service Except SMS

	TELE_SRV_TELEPHONY_AND_ALL_SYNC_SERVICES	= 0x90, ///<144 : Telephony And All Sync Services
	TELE_SRV_VOICE_GROUP_CALL_SERVICE			= 0x91, ///<145 : Voice Group Call Service
	TELE_SRV_VOICE_BROADCAST_SERVICE			= 0x92, ///<146 : Voice Broadcast Service

	TELE_SRV_ALL_PLMN_SPECIFIC_SERVICES			= 0xD0, ///<208 : All PLMN Specific Services
	TELE_SRV_PLMN_SPECIFIC_1					= 0xD1, ///<209 : PLMN Specific 1
	TELE_SRV_PLMN_SPECIFIC_2					= 0xD2, ///<210 : PLMN Specific 2
	TELE_SRV_PLMN_SPECIFIC_3					= 0xD3, ///<211 : PLMN Specific 3
	TELE_SRV_PLMN_SPECIFIC_4					= 0xD4, ///<212 : PLMN Specific 4
	TELE_SRV_PLMN_SPECIFIC_5					= 0xD5, ///<213 : PLMN Specific 5
	TELE_SRV_PLMN_SPECIFIC_6					= 0xD6, ///<214 : PLMN Specific 6
	TELE_SRV_PLMN_SPECIFIC_7					= 0xD7, ///<215 : PLMN Specific 7
	TELE_SRV_PLMN_SPECIFIC_8					= 0xD8, ///<216 : PLMN Specific 8
	TELE_SRV_PLMN_SPECIFIC_9					= 0xD9, ///<217 : PLMN Specific 9
	TELE_SRV_PLMN_SPECIFIC_A					= 0xDA, ///<218 : PLMN Specific A
	TELE_SRV_PLMN_SPECIFIC_B					= 0xDB, ///<219 : PLMN Specific B
	TELE_SRV_PLMN_SPECIFIC_C					= 0xDC, ///<220 : PLMN Specific C
	TELE_SRV_PLMN_SPECIFIC_D					= 0xDD, ///<221 : PLMN Specific D
	TELE_SRV_PLMN_SPECIFIC_E					= 0xDE, ///<222 : PLMN Specific E
	TELE_SRV_PLMN_SPECIFIC_F					= 0xDF  ///<223 : PLMN Specific F
} TeleSrv_t;

/**
	This type is used to represent the code identifying a single
	supplementary service, a group of supplementary services, or
	all supplementary services. The services and abbreviations
	used are defined in TS GSM 02.04. The internal structure is
	defined as follows:
	bits 87654321:
	group (bits 8765), and specific service (bits 4321)
**/
typedef enum
{
	//SRV_CODE_SS_CODE_ALL_SS					= 0x????, ///<    : All SS
	//SRV_CODE_SS_CODE_ALL_LINE_IDENTIFICATION	= 0x????, ///<    : All Line Identification SS
	SRV_CODE_SS_CODE_CLIP						= 0x001E, ///< 30 : Calling Line Identification Presentation
	SRV_CODE_SS_CODE_CLIR						= 0x001F, ///< 31 : Calling Line Identification Restriction
	SRV_CODE_SS_CODE_COLP						= 0x004C, ///< 76 : Connected Line Identification Presetation
	SRV_CODE_SS_CODE_COLR						= 0x004D, ///< 77 : Connected Line Identification Restriction
	//SRV_CODE_SS_CODE_MCI						= 0x????, ///<    : Malicious Call Identification
	//SRV_CODE_SS_CODE_ALL_NAME_IDENTIFICATION	= 0x????, ///<    : All Name Identification SS
	SRV_CODE_SS_CODE_CNAP						= 0x012C, ///<300 : Calling Name Presentation

	SRV_CODE_SS_CODE_ALL_FORWARDING				= 0x0002, ///<002 : All Forwarding
	SRV_CODE_SS_CODE_ACF						= 0x0004, ///<004 : All Conditional Forwarding
	SRV_CODE_SS_CODE_CFU						= 0x0015, ///< 21 : Call Forwarding Unconditional
	SRV_CODE_SS_CODE_CALL_DEFLECTION			= 0x0042, ///< 66 : Call Deflection
	SRV_CODE_SS_CODE_CFB						= 0x0043, ///< 67 : Call Forwarding On Mobile Subscriber Busy
	SRV_CODE_SS_CODE_CFNRY						= 0x003D, ///< 61 : Call Forwarding On No Reply
	SRV_CODE_SS_CODE_CFNRC						= 0x003E, ///< 62 : Call Forwarding On Mobile Subscriber Not Reachable

	//SRV_CODE_SS_CODE_ALL_CALL_OFFERING		= 0x????, ///<    : All Call Offering SS Includes All Forwarding SS
	SRV_CODE_SS_CODE_ECT						= 0x0060, ///< 96 : Explicit Call Transfer
	//SRV_CODE_SS_CODE_MAH						= 0x????, ///<    : Mobile Access Hunting

	//SRV_CODE_SS_CODE_ALL_CALL_COMPLETION		= 0x????, ///<    : All Call Completion SS
	SRV_CODE_SS_CODE_CW							= 0x002B, ///< 43 : Call Waiting
	//SRV_CODE_SS_CODE_HOLD						= 0x????, ///<    : CAll Hold
	SRV_CODE_SS_CODE_CCBS_A						= 0x0025, ///< 37 : Complition Of Call To Busy Subscriber (Originating Side)
	//SRV_CODE_SS_CODE_CCBS_B					= 0x????, ///<    : Complition Of Call To Busy Subscriber (Terminating Side)
	SRV_CODE_SS_CODE_MC							= 0x0058, ///< 88 : Multi Call

	//SRV_CODE_SS_CODE_ALL_MPTY					= 0x????, ///<    : All Multiparty SS
	//SRV_CODE_SS_CODE_MPTY						= 0x????, ///<    : Multiparty

	//SRV_CODE_SS_CODE_ALL_COMMUNITY_OF_INTEREST= 0x????, ///<    : All Community Of Interest SS
	//SRV_CODE_SS_CODE_CUG						= 0x????, ///<    : Closed User Group

	//SRV_CODE_SS_CODE_ALL_CHARGING_SS			= 0x????, ///<    : All Charging SS
	//SRV_CODE_SS_CODE_AOCI						= 0x????, ///<    : Advice Of Charge Information
	//SRV_CODE_SS_CODE_AOCC						= 0x????, ///<    : Advice Of Charge Charging

	SRV_CODE_SS_CODE_ALL_ADD_INFO_TRANSFER		= 0x0168, ///<360 : All Additional Information Transfer
	SRV_CODE_SS_CODE_UUS_1						= 0x0169, ///<361 : User-To-User Signalling 1
	SRV_CODE_SS_CODE_UUS_2						= 0x0170, ///<362 : User-To-User Signalling 2
	SRV_CODE_SS_CODE_UUS_3						= 0x0171, ///<363 : User-To-User Signalling 3

	SRV_CODE_SS_CODE_ALL_CALL_RESTRICTION		= 0x014A, ///<330 : All Call Restriction SS
	SRV_CODE_SS_CODE_BOC						= 0x014D, ///<333 : Barring Of Outgoing Calls
	SRV_CODE_SS_CODE_BAOC						= 0x0021, ///< 33 : Barring Of All Outgoing Calls
	SRV_CODE_SS_CODE_BOIC						= 0x014B, ///<331 : Barring Of Outgoing International Calls
	SRV_CODE_SS_CODE_BOIC_EXC_PLMN				= 0x014C, ///<332 : Barring Of Outgoing International Calls Except Those Directed To The Home PLMN
	SRV_CODE_SS_CODE_BIC						= 0x0161, ///<353 : Barring Of Incomming Calls
	SRV_CODE_SS_CODE_BAIC						= 0x0023, ///< 35 : Barring Of All Incoming Calls
	SRV_CODE_SS_CODE_BAIC_ROAM					= 0x015F, ///<351 : Barring Of All Incoming Calls When Roaming Outside Home PLMN_Country

	//SRV_CODE_SS_CODE_ALL_CALL_PRIORITY		= 0x????, ///<    : All Call Priority SS
	SRV_CODE_SS_CODE_EMLPP						= 0x004B, ///< 75 : Enhanced Multilevel Precedence Pre-emption (EMLPP) Service

	//SRV_CODE_SS_CODE_ALL_LCS_PRIVACY_EXCEPTION= 0x????, ///<    : All LCS Privacy Exception - All LCS Privacy Exception Classes
	//SRV_CODE_SS_CODE_UNIVERSAL_LCS			= 0x????, ///<    : Universal - Allow Location By Any LCS Client
	//SRV_CODE_SS_CODE_CALL_RELATED_LCS			= 0x????, ///<    : Call Related - Allow Location By Any Value Added LCS Client To Which A Call Is Established From The Target MS
	//SRV_CODE_SS_CODE_CALL_UNRELATED_LCS		= 0x????, ///<    : Call Unrelated - Allow Location By Designated External value Added LCS Client
	//SRV_CODE_SS_CODE_PLMN_OPERATOR_LCS		= 0x????, ///<    : PLMN Operator - Allow Location By designated By PLMN Operator LCS Clients

	//SRV_CODE_SS_CODE_ALL_MORLR				= 0x????, ///<    : All MORL SS - All Mobile Originating Location Request Classes
	//SRV_CODE_SS_CODE_BASIC_SELF_LOCATION		= 0x????, ///<    : Basic Self Location- Allow An MS To Request Its Own Location
	//SRV_CODE_SS_CODE_AUTONOMOUS_SELF_LOCATION	= 0x????, ///<    : Auto Nomous Self Location - Allow An MS To Perform Self Location Without Interaction With The PLMN For A Predetermined Period Of Time
	//SRV_CODE_SS_CODE_TRANSFER_TO_THIRD_PARTY	= 0x????, ///<    : Transfer To Third Party - Allow An MS To Request Transfer Of Its Location To Another LCS Client

	//SRV_CODE_SS_CODE_ALL_PLMN_SPECIFIC_SS		= 0x????, ///<    : All PLMN Specific SS
	//SRV_CODE_SS_CODE_PLMN_SPECIFIC_SS_1		= 0x????, ///<    : PLMN Specific SS 1
	//SRV_CODE_SS_CODE_PLMN_SPECIFIC_SS_2		= 0x????, ///<    : PLMN Specific SS 2
	//SRV_CODE_SS_CODE_PLMN_SPECIFIC_SS_3		= 0x????, ///<    : PLMN Specific SS 3
	//SRV_CODE_SS_CODE_PLMN_SPECIFIC_SS_4		= 0x????, ///<    : PLMN Specific SS 4
	//SRV_CODE_SS_CODE_PLMN_SPECIFIC_SS_5		= 0x????, ///<    : PLMN Specific SS 5
	//SRV_CODE_SS_CODE_PLMN_SPECIFIC_SS_6		= 0x????, ///<    : PLMN Specific SS 6
	//SRV_CODE_SS_CODE_PLMN_SPECIFIC_SS_7		= 0x????, ///<    : PLMN Specific SS 7
	//SRV_CODE_SS_CODE_PLMN_SPECIFIC_SS_8		= 0x????, ///<    : PLMN Specific SS 8
	//SRV_CODE_SS_CODE_PLMN_SPECIFIC_SS_9		= 0x????, ///<    : PLMN Specific SS 9
	//SRV_CODE_SS_CODE_PLMN_SPECIFIC_SS_A		= 0x????, ///<    : PLMN Specific SS A
	//SRV_CODE_SS_CODE_PLMN_SPECIFIC_SS_B		= 0x????, ///<    : PLMN Specific SS B
	//SRV_CODE_SS_CODE_PLMN_SPECIFIC_SS_C		= 0x????, ///<    : PLMN Specific SS C
	//SRV_CODE_SS_CODE_PLMN_SPECIFIC_SS_D		= 0x????, ///<    : PLMN Specific SS D
	//SRV_CODE_SS_CODE_PLMN_SPECIFIC_SS_E		= 0x????, ///<    : PLMN Specific SS E
	//SRV_CODE_SS_CODE_PLMN_SPECIFIC_SS_F		= 0x????  ///<    : PLMN Specific SS F
	SRV_CODE_USPECIFIED							= 0xFFFF  ///<	  : Unspecified
} MmiSrvCode_t;											  ///< MMI Service Code

/// enum : No Reply Condition Time
typedef enum
{
	SS_NO_REPLY_CONDITION_TIME_5_SEC	=  5,	///<  5 seconds no reply condition time
	SS_NO_REPLY_CONDITION_TIME_10_SEC	= 10,	///< 10 seconds no reply condition time
	SS_NO_REPLY_CONDITION_TIME_15_SEC	= 15,	///< 15 seconds no reply condition time
	SS_NO_REPLY_CONDITION_TIME_20_SEC	= 20,	///< 20 seconds no reply condition time
	SS_NO_REPLY_CONDITION_TIME_25_SEC	= 25,	///< 25 seconds no reply condition time
	SS_NO_REPLY_CONDITION_TIME_30_SEC	= 30	///< 30 seconds no reply condition time
} SS_NoReplyTime_t;								///< No Reply Condition Time

/// enum : Notification to Calling Party
typedef enum
{
	SS_PARTY_NO_NOTIFICATION,					///< No Notification
	SS_PARTY_NOTIFICATION						///< Notification
} SS_PartyNotif_t;								///< Notification To Calling/Forwarding Party

/// enum : Forwarding Reason
typedef enum
{
	SS_FWD_REASON_ME_NOT_REACHABLE,				///< MS Not Reachable
	SS_FWD_REASON_ME_BUSY,						///< MS Busy
	SS_FWD_REASON_NO_REPLY,						///< No Reply
	SS_FWD_REASON_UNCONDITIONAL					///< Unconditional 
} SS_FwdReason_t;								///< Forwarding Reason Type

/// structure : Forwarding Option
typedef struct
{
	SS_PartyNotif_t	notifFwd;				///< Notification to Forwarding Party
	SS_PartyNotif_t	notifCalling;			///< Notification to Calling Party
	SS_FwdReason_t	fwdReason;				///< Forwarding Reason
} SS_FwdOption_t;								///< Forwarding Option

/// Enum : Subscription Option Type
typedef enum
{
	SS_SUBSCRIPTION_OPTION_CLI_RESTRICTION		= 0x82,	///< CLI Restriction Option
	SS_SUBSCRIPTION_OPTION_OVERRIDE_CATEGORY	= 0x81	///< Override Category
} SS_SubsOptionType_t;									///< Subscription Option Type

/// Enum : CLI Restriction Option Type
typedef enum
{
	SS_CLI_RESTRICTION_PERMENENT			= 0x00,	///< Permenent
	SS_CLI_RESTRICTION_TEMPORARY_RESTRICTED	= 0x01,	///< Temporary (Default Restricted)
	SS_CLI_RESTRICTION_TEMPORARY_ALLOWED	= 0x02	///< Temporary (Default Allowed)  
} SS_ClirOption_t;									///< CLI Restriction Option

/// Enum : CLI Restriction Option Type
typedef enum
{
	SS_OVERRIDE_CATEGORY_OVERRIDE_ENABLE	= 0x00,	///< Override Enable
	SS_OVERRIDE_CATEGORY_OVERRIDE_DISABLE	= 0x01	///< Override Disable
} SS_OverrideOption_t;								///< Override Category Option

/// Structure : Subscription Option
typedef struct
{
	SS_SubsOptionType_t	type;					///< Subscription Option Type
	UInt8				content;				///< Content of type SS_ClirOption_t/SS_OverrideOption_t

}SS_SubsOption_t;								///< Subscription Option

/**
@code
Structure : Forwarding Feature
			include       8   7   6   5   4   3   2   1
						|   |   |   |   |   |   |   |   |
						  ^   ^   ^   ^   ^   ^   ^   ^			   
			basicSrv								  X		Basic Service Group
			ssStatus							  X 		SS Status
			partyAdd						  X				Party Address
			phoneSubAdd					  X					Phone Number Subaddress
			fwdOption				  X						Forwarding Option
			noReplyTime			  X			 				No Reply Time
			LongPartyAdd	  X								Long Forwarded To Number Supported
@endcode
**/
typedef struct
{
	UInt16				include;							///< Include Options
	BasicSrvGroup_t		basicSrv;							///< Basic Service Group
	UInt8				ssStatus;							///< SS Status
	PartyAddress_t		partyAdd;							///< Party Address
	PartySubAdd_t		partySubAdd;						///< Party Subaddress
	SS_FwdOption_t		fwdOption;							///< Forwarding Option
	UInt8				noReplyTime;						///< No Reply Time
	PartyAddress_t		LongPartyAdd;						///< Long Forwarded to Address

} SS_FwdFeature_t;											///< Forwarding Feature Type

/// Structure : Forwarding Feature List
typedef struct
{
	UInt8			listSize;								///< List Size
	SS_FwdFeature_t	fwdFeatureList[MAX_FEATURE_LIST_SIZE];	///< Forwarding Feature List
} SS_FwdFeatureInfo_t;										///< Forwarding Info

/**
@code
Structure : Call Barring Feature
			include       8   7   6   5   4   3   2   1
						|   |   |   |   |   |   |   |   |
						  ^   ^   ^   ^   ^   ^   ^   ^	   
			basicSrv								  X		Basic Service Group
			ssStatus							  X 		SS Status
@endcode
**/
typedef struct
{
	UInt8			include;								///< Include Options
	BasicSrvGroup_t basicSrv;								///< Basic Service Group
	UInt8			ssStatus;								///< SS Status
} SS_CallBarFeature_t;										///< Call Barring Feature Type

/// Structure : Call Barring Feature List
typedef struct
{
	UInt8				listSize;									///< List Size
	SS_CallBarFeature_t	callBarFeatureList[MAX_FEATURE_LIST_SIZE];	///< Call Barring Feature List
} SS_CallBarInfo_t;													///< Call Barring Information

/**
@code
Structure : SS Data Information
			include       8   7   6   5   4   3   2   1
						|   |   |   |   |   |   |   |   |
						  ^   ^   ^   ^   ^   ^   ^   ^					   
			ssStatus								  X		SS Status
			subsOpt								  X 		Subscription Option
@endcode
**/
typedef struct
{
	UInt8			include;								///< Include Options
	UInt8			ssStatus;								///< SS Status
	SS_SubsOption_t	subsOpt;								///< Subscription Option
	UInt8			listSize;								///< List Size
	BasicSrvGroup_t	basicSrvGroupList[MAX_FEATURE_LIST_SIZE];///< Basic Service Group List
} SS_SsDataInfo_t;											///< SS data Information


/// Structure : SS Basic Service Information
typedef struct
{
	UInt8				listSize;									///< List Size
	BasicSrvGroup_t		basicSrvGroupList[MAX_FEATURE_LIST_SIZE];	///< Basic Service Group List
} SS_BasicSrvInfo_t;												///< SS data Information


/// Structure : CCBS Feature List
typedef struct
{
	UInt8				listSize;						///< List Size
	SS_CcbsFeature_t 	ccbsList[MAX_CCBS_LIST_SIZE];	///< CCBS Feature List
} SS_CcbsFeatureInfo_t;									///< CCBS Feature Information Type


/**
@code
Structure : Generic Service Information
			include			  8   7   6   5   4   3   2   1
							|   |   |   |   |   |   |   |   |
							  ^   ^   ^   ^   ^   ^   ^   ^					   
			clir										  X		CLI Restriction Option
			maxEntPrio								  X			Max Entitle Priority Option
			defaultPrio							  X				Default Priority Option
			ccbsFeatureInfo					  X					CCBS Feature Information
@endcode
**/
typedef struct
{
	UInt8					ssStatus;			///< SS Status

	UInt8					include;			///< Include Options
	SS_ClirOption_t			clir;				///< CLI Restriction Option
	UInt16					maxEntPrio;			///< Max Entitle Priority Option
	UInt16					defaultPrio;		///< Default Priority
	SS_CcbsFeatureInfo_t	ccbsFeatureInfo;	///< CCBS Feature Information
} SS_GenSrvInfo_t;								///< Generic Service Information

/// enum : Call Barred Error
typedef enum
{
	SS_CALL_BARRED_ERROR_BARRING_SRV_ACTIVE,		///< Barring Service Active
	SS_CALL_BARRED_ERROR_OPERATOR_BARRING	 		///< Operator Barring
} SS_CallBarredError_t;								///< Call Barred Error Type

/// enum : MWD Error
typedef enum
{
	SS_MWD_SET_FALSE_ERROR,							///< MWD Set False
	SS_MWD_SET_TRUE_ERROR							///< MWD Set True
} SS_MwdError_t;									///< Mwd Error Type

/// enum : System Failure
typedef enum
{
	SS_SYSTEM_FAILURE_PLMN,							///< PLMN
	SS_SYSTEM_FAILURE_HOR,							///< HOR
	SS_SYSTEM_FAILURE_VLR,							///< VLR
	SS_SYSTEM_FAILURE_PVLR,							///< PVLR
	SS_SYSTEM_FAILURE_CONTROLLING_MSC,				///< Controlling MSC
	SS_SYSTEM_FAILURE_VMSC,							///< VMSC
	SS_SYSTEM_FAILURE_EIR,							///< EIR
	SS_SYSTEM_FAILURE_RSS							///< RSS
} SS_SystemFailure_t;								///< System Failure Type

/// Enum : Component Type
typedef enum
{
	SS_COMPONENT_TYPE_NONE,						///< Component Type None
	SS_COMPONENT_TYPE_INVOKE		= 0xA1,		///< Component Type Invoke
	SS_COMPONENT_TYPE_RETURN_RESULT	= 0xA2,		///< Component Type Return Result
	SS_COMPONENT_TYPE_RETURN_ERROR	= 0xA3,		///< Component Type Return Error
	SS_COMPONENT_TYPE_REJECT		= 0xA4		///< Component Type Reject
} SS_Component_t;								///< SS Component
/**
@code
Structure : SS Incompatibility
			include       8   7   6   5   4   3   2   1
						|   |   |   |   |   |   |   |   |
						  ^   ^   ^   ^   ^   ^   ^   ^					   
			ssCode									  X		SS Code
			basicSrv							  X 		Basic Service Group
			ssStatus						  X				SS Status
@endcode
**/
typedef struct
{
	UInt8			include;						///< Include Options
	SS_Code_t		ssCode;							///< SS Code
	BasicSrvGroup_t	basicSrv;						///< Basic Service Information
	UInt8			ssStatus;						///< SS Status
} SS_SsIncompatibility_t;							///< SS Incompatibility Type

/// enum : System Failure
typedef enum
{
	SS_PWD_REGISTER_FAILURE_UNDETERMINED,			///< Undeterminded
	SS_PWD_REGISTER_FAILURE_INVALID_FORMAT,			///< Invalid Format
	SS_PWD_REGISTER_FAILURE_NEW_PWD_MISMATCH		///< New PWD Mismatch
} SS_PwdRegFailure_t;								///< PWD Register Failure Type



/// Structure : SS Guidance Information
typedef enum
{
	SS_GUIDANCE_INFO_ENTER_PW,						///< Enter Password
	SS_GUIDANCE_INFO_ENTER_NEW_PW,					///< Enter New Password
	SS_GUIDANCE_INFO_ENTER_NEW_PW_AGAIN,			///< Enter New Password Again
	SS_GUIDANCE_INFO_BAD_PW_TRY_AGAIN,				///< Bad Password Try Again
	SS_GUIDANCE_INFO_BAD_PW_FORMAT_TRY_AGAIN		///< Bad Password Format Try Again

} SS_GuidanceInfo_t;								///< Guidance Information Type


/**
@code
Structure : SS Call Forwarding Information Type
			include       8   7   6   5   4   3   2   1
						|   |   |   |   |   |   |   |   |
						  ^   ^   ^   ^   ^   ^   ^   ^			   
			partyAdd								  X		Party Address
			phoneSubAdd							  X			Phone Number Subaddress
			noReplyTime						  X			 	No Reply Time
			defaultPrio					  X					Default Priority
			nbrUser					  X						Number of Users
			LongFtnSup			  X							Long Forwarded To Number Supported
@endcode
**/
typedef struct
{
	UInt16				include;							///< Included Optional Elements
	PartyAddress_t		partyAdd;							///< Party Address
	PartySubAdd_t		partySubAdd;						///< Party Subaddress
	UInt8				noReplyTime;						///< No Reply Time
	UInt8				defaultPrio;						///< Default Priority		
	UInt8				nbrUser;							///< Number of Users		

} SS_CallForwardInfo_t;										///< Call Forwarding Information Type


/// Structure : SS Password Type
typedef struct
{
	UInt8	currentPwd[SS_PASSWORD_LENGTH];		///< Current password
	UInt8	newPwd[SS_PASSWORD_LENGTH];			///< New password
	UInt8	reNewPwd[SS_PASSWORD_LENGTH];		///< Re-new password
	UInt8*	pwdPtr;								///< Pointer to the PWD to be used. NA to API level

} SS_Password_t;								///< SS Password Type


/// Enum : General Problem Type
typedef enum
{
	GENERAL_PROBLEM_UNRECOGNIZED_COMPONENT,		///< Unrecognized Component
	GENERAL_PROBLEM_MISTYPEDED_COMPONENT,		///< Mistyped Component
	GENERAL_PROBLEM_BADLY_STRUCTURED_COMPONENT	///< Badly Structured Component
} SS_GeneralProblem_t;							///< General Problem Type

/// Enum : Invoke Problem Type
typedef enum
{
	INVOKE_PROBLEM_DUPLICATE_INVOKE_ID,			///< Duplicated Invoke Id
	INVOKE_PROBLEM_UNRECOGNIZED_OPERATION,		///< Unrecognized Operation
	INVOKE_PROBLEM_MISTYPED_PARAMETER,			///< Mistyped Parameter
	INVOKE_PROBLEM_RESOURCE_LIMITATION,			///< Resource Limitaion
	INVOKE_PROBLEM_UNRECOGNIZED_LINE_ID,		///< Unrecognized Line Id
	INVOKE_PROBLEM_LINKED_RESPONSE_UNEXPECTED,	///< Linked Response Unexpected
	INVOKE_PROBLEM_UNEXPECTED_LINKED_OPERATION	///< UnexpectedLinked Operation
} SS_InvokeProblem_t;							///< Invoke Problem Type

/// Enum : Return Result Problem Type
typedef enum
{
	RETURN_RESULT_PROBLEM_UNRECOGNIZED_INVOKE_ID,	///< Unrecognized Invoke Id
	RETURN_RESULT_PROBLEM_UNEXPECTED,				///< Unexpected
	RETURN_RESULT_PROBLEM_MISTYPED_PARAMETER		///< Mistyped Parameter
} SS_ReturnResultProblem_t;							///< Return Result Problem Type

/// Enum : Return Error Problem Type
typedef enum
{
	RETURN_ERROR_PROBLEM_UNRECOGNIZED_INVOKE_ID,	///< Unrecognized Invoke Id
	RETURN_ERROR_PROBLEM_UNEXPECTED,				///< Unexpected
	RETURN_ERROR_PROBLEM_UNRECOGNIZED_ERROR,		///< Unrecognized Error
	RETURN_ERROR_PROBLEM_UNEXPECTED_ERROR,			///< Unexpected Error
	RETURN_ERROR_PROBLEM_MISTYPED_PARAMETER			///< Mistyped Parameter
} SS_ReturnErrorProblem_t;							///< Return Error Problem Type


/// Enum : SS Notification
typedef enum
{
	SS_NOTIFICATION_INCOMING_CALL_FWD	= 0x01,	///< Call is forwarded indication to C-subscriber (forwarded-to subscriber)
	SS_NOTIFICATION_CALL_FWD			= 0x02,	///< Call is forwarded indication to B-subscriber (forwarding subscriber)
	SS_NOTIFICATION_OUTGOING_CALL_FWD	= 0x04	///< Call is forwarded indication to A-subscriber (calling subscriber)
} SS_Notification_t;							///< SS Notification


/// Enum: Alerting Pattern Type
typedef enum
{
	SS_ALERTING_PATTERN_TYPE_LEVEL,						///< Level
	SS_ALERTING_PATTERN_TYPE_CATEGORY_1,				///< Category 1	
	SS_ALERTING_PATTERN_TYPE_CATEGORY_2					///< Category 2
} SS_AlertPatType_t;									///< Alerting Pattern Type

/// Enum: Alerting level
typedef enum
{
	SS_ALERTING_LEVEL_0,								///< Alerting  Level : 0
	SS_ALERTING_LEVEL_1,								///< Alerting  Level : 1
	SS_ALERTING_LEVEL_2									///< Alerting  Level : 2
} SS_AlertLevel_t;										///< Alerting level, refer to GSM 02.07

/// Enum: Alerting Category 1
typedef enum
{
	SS_ALERTING_CATEGORY_1_PATTERN_0,					///< Category 1 Patern : 0
	SS_ALERTING_CATEGORY_1_PATTERN_1,					///< Category 1 Patern : 1
	SS_ALERTING_CATEGORY_1_PATTERN_2					///< Category 1 Patern : 2
} SS_AlertCategory1_t;									///< Alerting Category 1

/// Enum: Alerting Category 2
typedef enum
{
	SS_ALERTING_CATEGORY_2_PATTERN_0					///< Category 2 Patern : 0
} SS_AlertCategory2_t;									///< Alerting Category 


#define MAX_MISDN_SIZE		20					///< MSISDN Size		

/// Structure : MS ISDN Type
typedef struct
{
	UInt8				length;					///< Length
	UInt8				data[MAX_MISDN_SIZE];	///< MSISDN payload
												///< this will also include the TON/NPI byte 
												///< Refer 3GPP 29.002 V5.4.0

}SS_MsIsdn_t;									///< MSISDN Type

/**
@code
Structure : SS USSD Information Type

			include       8   7   6   5   4   3   2   1
						|   |   |   |   |   |   |   |   |
						  ^   ^   ^   ^   ^   ^   ^   ^	
			alertPatt								  X		Alerting Pattern
			msIsdn								  X 		MSISDN
@endcode
**/
typedef struct
{
	UInt8				include;				///< Included Optional Elements
	UInt8				dcs;					///< Data Coding Scheme "GSM 3.38, section 5"
	UInt8				length;					///< Length
	UInt8				data[MAX_USSD_SIZE];	///< USSD string
	SS_AlertPattern_t	alertPatt;				///< Alerting Pattern
	SS_MsIsdn_t			msIsdn;					///< MSISDN

}SS_UssdInfo_t;									///< SS USSD Information Type


#define SS_API_STR_SIZE	(MAX_DIGITS + 50)


/// Structure : SS Facility Information Element
typedef struct
{
	SS_Component_t	component;					///< Component Type 
	UInt8			invokeId;					///< Invoke Identifier
	UInt8			linkedId;					///< Linked Identifier
	SS_Operation_t	operation;					///< Operation
	SS_Code_t		ssCode;						///< SS Code

	SS_ErrorCode_t	errorCode;					///< Error Code
	SS_Reject_t		reject;						///< Reject

	UInt8*			facIePtr;					///< Facility IE Pointer

	int				paramLen;					///< Parameter Length
	UInt8*			paramPtr;					///< Parameter Pointer
} SS_FacilityIe_t;								///< SS Facility IE

/// Structure : SS API Service Request
typedef struct
{
	SS_Operation_t	operation;					///< SS Operation
	BasicSrvGroup_t	basicSrv;					///< Basic Service
	SS_Code_t		ssCode;						///< SS Code 

	union
	{
		SS_CallForwardInfo_t	cfwInfo;		///< Call Forwarding Parameters
		SS_Password_t			ssPassword;		///< SS Password
		UInt8					ccbsIndex;		///< CCBS Index
		SS_UssdInfo_t			ussdInfo;		///< USSD Parameters
	} param;

	Ticks_t			expTime;					///< Expiration Time [s]		
} SS_SrvReq_t;

/// Structure : SS-API  Service Request.  Used for mobile originated 
typedef struct
{
	ConfigMode_t	fdnCheck;				///< FDN Check
	ConfigMode_t	stkCheck;				///< STK Check
	SS_SrvReq_t		ssSrvReq;				///< SS Service Request
} SsApi_SrvReq_t;							///< SS API Service request

/// Structure : SS-API USSD Service Request.  Used for mobile originated USSD
typedef struct
{
	ConfigMode_t	fdnCheck;				///< FDN Check
	ConfigMode_t	stkCheck;				///< STK Check
	SS_Operation_t	operation;				///< SS Operation
	SS_UssdInfo_t	ussdInfo;				///< USSD Service Request
} SsApi_UssdSrvReq_t;						///< SS-API USSD Service Request

/// Structure : SS-API USSD Service Data.  Used for continuation of USSD dialog with the network.
typedef struct
{
	SS_Operation_t	operation;				///< SS Operation
	SS_UssdInfo_t	ussdInfo;				///< USSD Service Request
} SsApi_UssdDataReq_t;						///< SS-API USSD Data Request


/// Structure : SS-API Data Request.  Used for continuation of SS dialog with the network.
typedef struct
{
	SS_Operation_t	operation;					///< SS Operation

	union
	{
		SS_UssdInfo_t			ussdInfo;		///< USSD Information
	} param;

} SsApi_DataReq_t;								///< SS-API USSD Data Request


/// Structure : API Dial String Service Request
typedef struct
{
	ConfigMode_t	fdnCheck;				///< FDN Check
	ConfigMode_t	stkCheck;				///< STK Check
	CallType_t		callType;				///< Call Type
	UInt16			strLen;					///< SS Operation
	UInt8*			strPtr;					///< USSD Service Request
} SsApi_DialStrSrvReq_t;					///< SS API Dial String Service Request Type

/// Structure : MNSS State
typedef enum
{
	MNSS_STATE_IDLE,						///< No activity
	MNSS_STATE_PEND,						///< Pending for TI
	MNSS_STATE_ACTIVE						///< Connected to the network
} MNSS_State_t;								///< MNSS State Type


#if 0
/// Structure : STK SS Service Response Type
typedef struct
{
	UInt8				callIndex;			///< Call Index
	CauseIe_t			causeIe;			///< Cause IE
	SS_Operation_t		operation;			///< Operation
	SS_Component_t		component;			///< Component


	int				paramLen;				///< Parameter Length
	UInt8*			paramPtr;				///< Parameter Pointer

} STK_SsSrvRsp_t;							///< STK SS Service Response Type

#endif


/// Structure : STK SS Service Response Type
typedef struct
{
#ifndef CAPI_SS_REMOVE_ss_msg_clientInfo
	ClientInfo_t	clientInfo;				///< Client Information       //To be removed
#endif	
	UInt8			callIndex;				///< Call Index            ///Just for old SS
	CauseIe_t		causeIe;				///< Cause IE
	SS_Operation_t	operation;				///< Operation
	SS_Component_t	component;				///< Component

	union
	{
		SS_ErrorCode_t		errorCode;		///< Error Code
		SS_Reject_t			reject;			///< Reject
	} param;								///< Parameters

	int				paramLen;				///< Parameter Length
	UInt8*			paramPtr;				///< Parameter Pointer

} STK_SsSrvRel_t;							///< STK SS Service Response Type


/// Structure : SS Service Indication
typedef struct
{
#ifndef CAPI_SS_REMOVE_ss_msg_clientInfo
	ClientInfo_t	clientInfo;				///< Client Information       //To be removed
#endif	
	UInt8				callIndex;				///< Call Index   //Used for the old SS
	SS_Operation_t		operation;				///< Operation
	SS_SrvType_t		type;					///< Service Response Type

	union
	{
		SS_UssdInfo_t	ussdInfo;				///< USSD Information
	} param;									///< Parameters

	int					facIeLength;			///< Facility IE Length
	UInt8				facIeData[MAX_FACILITY_IE_LENGTH]; ///< Facility IE data
#ifndef CAPI_SS_API_REMOVE_facIePtr
	UInt8*				facIePtr;				///< Facility IE Pointer
#endif
}SS_SrvInd_t;									///< SS Service Indication Type

/// Structure : SS Service Response
typedef struct
{
#ifndef CAPI_SS_REMOVE_ss_msg_clientInfo
	ClientInfo_t	clientInfo;				///< Client Information       //To be removed
#endif	
	UInt8					callIndex;			///< Call Index
	SS_Component_t			component;			///< Component
	SS_Operation_t			operation;			///< Operation
	SS_Code_t				ssCode;				///< SS Code
	SS_SrvType_t			type;				///< Service Response Type

	union
	{
		SS_UssdInfo_t		ussdInfo;			///< USSD Information
	} param;

	int							facIeLength;	///< Facility IE Length
	UInt8						facIeData[MAX_FACILITY_IE_LENGTH]; ///< Facility IE dta
#ifndef CAPI_SS_API_REMOVE_facIePtr
	UInt8*				facIePtr;				///< Facility IE Pointer
#endif
} SS_SrvRsp_t;									///< SS Service Response


/// Structure : SS Service Release
typedef struct
{
#ifndef CAPI_SS_REMOVE_ss_msg_clientInfo
	ClientInfo_t	clientInfo;				///< Client Information       //To be removed
#endif	
	UInt8						callIndex;			///< Call Index
	SS_Component_t				component;			///< Component Type
	SS_Operation_t				operation;			///< Operation
	SS_Code_t					ssCode;				///< SS Code 
	BasicSrvGroup_t				basicSrv;			///< Basic Service
	SS_SrvType_t				type;				///< Service Response Type

	union
	{
		SS_FwdFeatureInfo_t		forwardInfo;		///< Forwarding Information
		SS_CallBarInfo_t		barringInfo;		///< Call Barring Information
		SS_SsDataInfo_t			ssDataInfo;			///< SS Data Information
		UInt8					ssStatus;			///< SS Status
		PartyAddress_t			fwdToPartyAdd;		///< Forwarded To Party Address
		SS_BasicSrvInfo_t		basicSrvInfo;		///< Basic Service Information
		SS_GenSrvInfo_t			genSrvInfo;			///< Generic Service Information
		UInt8					newPwd[SS_PASSWORD_LENGTH];	///< New password
		SS_UssdInfo_t			ussdInfo;			///< USSD Information

		SS_ReturnError_t		returnError;		///< Return Error
		SS_Reject_t				reject;				///< Reject
		Result_t				localResult;		///< Local causes
	} param;

#ifndef CAPI_SS_API_REMOVE_duplicated_localResult
	Result_t					localResult;		// To be removed
#endif
	CauseIe_t					causeIe;			///< Network Cases

	int							facIeLength;		///< Facility IE Length
	UInt8						facIeData[MAX_FACILITY_IE_LENGTH]; ///< Facility IE data
#ifndef CAPI_SS_API_REMOVE_facIePtr
	UInt8*				facIePtr;				///< Facility IE Pointer
#endif
} SS_SrvRel_t;										///< SS Service Reject


/// Structure : SS Service Indication
typedef struct
{
	UInt8	length;						///< Length of the SS String
	char	data[MAX_USSD_SIZE];		///< SS string data
}SS_String_t;							///< SS Service Indication Type


/// Enum : MNSS Primitive Type
typedef enum
{
	CLIENT_MNSS_SS_SRV_REQ,				///< Client's msg to MNSS for Service request
	CLIENT_MNSS_SS_DATA_REQ,			///< Clinet's msg to MNSS for sending data
	CLIENT_MNSS_SS_REL_REQ,				///< Client's msg to MNSS to release the session

	MMREG_MNSS_TRANS_ID_RES,			///< MMREG message to MNSS for successful transaction ID allocation
	MMREG_MNSS_TRANS_ID_REJ,			///< MMREG message to MNSS for failed transaction ID allocation

	CMSS_MNSS_REGISTER_MSG,				///< CMSS msg to MNSS for Register msg
	CMSS_MNSS_FACILITY_MSG,				///< CMSS msg to MNSS for Facility msg
	CMSS_MNSS_RELEASE_COMPLETE_MSG,		///< CMSS msg to MNSS for Release Complete msg

	CMSS_MNSS_SERV_ACC_IND,				///< CMSS msg to MNSS indicating the service is accepted

	TIMER_MNSS_TIMER_EXPIRE				///< TIMER msg to MNSS for expiration of SS timer

} MNSS_Prim_t;							///< MNSS Primitive Type

typedef struct
{
	UInt8*	payloadPtr;
	int		length;
} SS_Test_t;

#endif //  _SS_DEF_H_

