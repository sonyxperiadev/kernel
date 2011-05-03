//***************************************************************************
//
//	Copyright © 1997-2008 Broadcom Corporation
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
*   @file   callparser.h
*
*   @brief  This file contains definitions for Call Parser.
*
****************************************************************************/
#ifndef _CALLPARSER_H_
#define _CALLPARSER_H_


//******************************************************************************
// Enumerations
//******************************************************************************
#define	MAX_PARM_LIST			4
#define	USSD_LEN				40


//******************************************************************************
// Structure Typedefs
//******************************************************************************
typedef UInt8	PWord_t[SS_PASSWORD_LENGTH+1];				// Password type

typedef UInt16	NoReplyCondTime_t;

typedef enum
{
	BSTYPE_BEARER			= 2,	// Bearer Service
	BSTYPE_TELE				= 3,	// Tele Service
	BSTYPE_UNKNOWN			= 0xFF	// Unknown Type
} BSType_t;

typedef struct
{
	BSType_t		type;
	UInt8			service;
} BService_t;


typedef struct
{
	Boolean			is_used;
	BService_t		value;
} OptBService_t;


typedef struct
{
	Boolean				is_used;
	TelephoneNumber_t	value;
} OptRegCFNum_t;

typedef struct
{
	Boolean			is_used;
	NoReplyCondTime_t value;
} OptNoReplyCondTime_t;


typedef struct
{
	SuppSvcType_t	ss_type;			// SS Type 
	SSAction_t		ss_action;			// SS Action to perform
	UInt8			total_parm;			// running total number of parameters
	UInt8			ussd[PHASE2_MAX_USSD_STRING_SIZE+1];	// for USSD data
	OptBService_t	bs_code;
	OptRegCFNum_t	forwarded_num;
	OptNoReplyCondTime_t reply_timeout;
	PWord_t			old_password;		// used for BAR_PASSWORD and other Barring
	PWord_t			new_password1;		// used for BAR_PASSWORD
	PWord_t			new_password2;		// used for BAR_PASSWORD
	SuppSvcType_t	bar_ss_type;		// used for BAR_PASSWORD
	Boolean			pound_end_flag;		// TRUE if the string is ended with '#'
} SSData_t;

typedef struct
{
	TelephoneNumber_t phone_number;
	CLIRMode_t		clir_mode;
	DTMFTone_t		dtmfTones[DTMF_TONE_ARRAY_SIZE];
	UInt8			dtmfLen;	
} CallData_t;
	
typedef struct
{
	CallType_t		type;
	SS_SrvReq_t		ssSrvReq;
	SSData_t		ss_data;
	CallData_t		call_data;
} CallCommand_t;


//******************************************************************************
// Function Prototypes 
//******************************************************************************

void CALLPARSER_ParseCmdStr(
	CallCommand_t *p_ccmd,				// pointer to parsed command
	UInt8 *input_str					// pointer to input string
	);

void ConvertPhoneNumber(const UInt8* s, TelephoneNumber_t* d);

#endif // _CALLPARSER_H_

