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
*   @file   common_sim.h
*
*   @brief  This file defines the data types used in both SIM API functions and 
*			other modules in platform.
*
****************************************************************************/


#ifndef _COMMON_SIM_H_
#define _COMMON_SIM_H_
   

#define MAX_SEEK_RECORD_RSP_LEN	255	///< Maximum number of byte returned in Seek Record Response

#define NUM_OF_MWI_TYPE 4	///< Number of MWI (Message Waiting Type) defined in SIM_MWI_TYPE_t enum

#define	SIM_SMS_REC_LEN	176		///< Record size of EF-SMS
#define	SMSMESG_DATA_SZ	(SIM_SMS_REC_LEN - 1)	///< Data size of an EF-SMS record excluding the first SMS status byte


typedef enum
{
	SIMACCESS_SUCCESS,					///< Access/Verify was successful
	SIMACCESS_INCORRECT_CHV,			///< Verify failed, at least one attempt left
	SIMACCESS_BLOCKED_CHV,				///< Verify failed, CHV is blocked
	SIMACCESS_NEED_CHV1,				///< No access, need CHV1
	SIMACCESS_NEED_CHV2,				///< No access, need CHV2
	SIMACCESS_NOT_NEED_CHV1,			///< CHV1 available and must not be entered
	SIMACCESS_NOT_NEED_CHV2,			///< CHV2 available and must not be entered
	SIMACCESS_NO_ACCESS,				///< No access, not allowed (NVR or ADM)
	SIMACCESS_INVALID,					///< No access, unusable when invalidated file
	SIMACCESS_MEMORY_ERR,				///< Access failed, memory problem
	SIMACCESS_OUT_OF_RANGE,				///< Access failed, invalid address
	SIMACCESS_NOT_FOUND,				///< Access failed, no such file / pattern not found
	SIMACCESS_MAX_VALUE,				///< Increase failed, already at max val
	SIMACCESS_INCORRECT_PUK,			///< Unblock failed, incorrect PUK
	SIMACCESS_BLOCKED_PUK,				///< Unblock failed, PUK is blocked (dead card)
	SIMACCESS_NO_SIM,					///< No SIM
	SIMACCESS_MEMORY_EXCEEDED,			///< SIM Memmory exceeded
	SIMACCESS_CANNOT_REPLACE_SMS,		///< SIM can not replace sms
	SIMACCESS_BLOCKED_CHV2,             ///< CHV2 is blocked
	SIMACCESS_ALREADY_POWERED_ON,       ///< SIM already powered on when a power on request is received
	SIMACCESS_ALREADY_POWERED_OFF,		///< SIM already powered off when a power off request is received
	SIMACCESS_WARNING_CHV,              ///< Security status not satisfied or no info given, i.e. SW1/SW2=0x69/0x82 (3G USIM), SW1/SW2=0x98/0x04 (2G SIM) or SW1/SW2=0x62/0x00
	SIMACCESS_CHV1_DISABLE_NOT_ALLOWED,  ///< Disabling CHV1 is not allowed in EF-SST of 2G SIM 
	SIMACCESS_INVALID_PARAM,			///< Client passed invalid parameter
	SIMACCESS_CHV_CONTRADICTION_ERROR   ///< CHV security status contradiction, e.g. try to change PIN1 while PIN1 is disabled
} SIMAccess_t;							


/*************************Needed by SMS*********************************/
/// SIM Message Waiting Indicator: do not change the following enum values.
/// It is defined to adopt the record index defined in 2G CPHS spec. 
typedef enum
{
	MWI_L1 = 1,		///< MWI flag for Voice L1
	MWI_L2 = 2,		///< MWI flag for Voice L2
	MWI_FAX = 3,	///< MWI flag for Fax
	MWI_DATA = 4	///< MWI flag for Data
} SIM_MWI_TYPE_t;


/// SIM Message Status 
typedef enum
{
	SIMSMSMESGSTATUS_FREE	= 0x00,		///< SMS status is Free
	SIMSMSMESGSTATUS_READ 	= 0x01,		///< SMS status is Read
	SIMSMSMESGSTATUS_UNREAD = 0x03,		///< SMS status is Unread
	SIMSMSMESGSTATUS_SENT	 = 0x05,	///< SMS status is Sent
	SIMSMSMESGSTATUS_UNSENT = 0x07		///< SMS status is Unsent
} SIMSMSMesgStatus_t;					///< SIM SMS Message Status


/* Non-alpha data length in a SIM phonebook EF, e.g. EF-ADN, EF-LND, EF-SDN */
#define SIM_PBK_NON_ALPHA_SZ	14		///< Data size in a phonebook record excluding alpha data
#define SIM_PBK_DIGIT_SZ		10		///< Number of bytes for digits in phonebook record
#define SIM_PBK_ASCII_DIGIT_SZ	(SIM_PBK_DIGIT_SZ * 2)	///< Number of digits allocated in phonebook record


#define ECC_DIGITS						6	///< Maximum number of digits in emergency call code
#define ECC_ENTRY_SIZE	 (ECC_DIGITS / 2)	///< Number of bytes of each emergency call code


#define SIM_EXT_DIGIT_NUM_OF_BYTES	13 ///< Number of bytes in a record of EF-EXTx, e.g. EF-EXT1 & EF-EXT2


#define SIM_PBK_EXT_ASCII_DIGIT_SIZE ((SIM_EXT_DIGIT_NUM_OF_BYTES - 3) * 2) /// Max number of extension digits in EF-EXTx record


/// SIM SMS record data 
typedef struct
{
	SIMSMSMesgStatus_t 	status;			///< SMS Message Status, first byte of SMS record
	UInt8 mesg_data[SMSMESG_DATA_SZ];	///< SMS Message data excluding status byte
} SIMSMSMesg_t;


/// SIM SMS Update Result
typedef struct
{
	SIMAccess_t result;			///< SIM access result
	UInt16		rec_no;			///< 0-based record index
} SIM_SMS_UPDATE_RESULT_t;


/// SIM SMS Data
typedef struct
{
	SIMAccess_t result;			///< SIM access result
	UInt16		rec_no;			///< 0-based record index
	SIMSMSMesg_t sms_mesg;		///< SMS record data
} SIM_SMS_DATA_t;

/// Type of SIM
typedef enum
{
	SIMTYPE_NORMAL_SIM,				///< Normal SIM type
	SIMTYPE_NORMAL_SPECIAL_SIM,		///< Normal Special SIM type
	SIMTYPE_APPROVAL_SIM,			///< Approval SIM (test SIM) type
	SIMTYPE_APPROVAL_SPECIAL_SIM,	///< Approval Special SIM type
	SIMTYPE_SERVICE_CARD_SIM,		///< Service Card SIM type
	SIMTYPE_CELL_TEST_SIM,			///< Cell Test SIM type
	SIMTYPE_NO_CARD					///< No SIM is inserted
} SIMType_t;						///< SIM card type indicated in EF-ADN

#endif  // _COMMON_SIM_H_
