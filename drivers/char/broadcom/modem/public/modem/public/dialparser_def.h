//***************************************************************************
//
//	Copyright © 2002-2011 Broadcom Corporation
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
*   @file   dialparser_def.h
*
*   @brief  This file contains definitions for the type for dial parser API.
*
****************************************************************************/

#ifndef _DIALPARSER_DEF_
#define _DIALPARSER_DEF_

// ---- Include Files -------------------------------------------------------
//The following header files should be included before include dialedservice_def.h
// "mobcom_types.h"
// "resultcode.h"
// "common_defs.h"
// "ostypes.h"
// "taskmsgs.h"
// "ss_def.h"
// "rtc.h"
// "netreg_def.h" //PLMN
// "ss_api_old.h"


/**	GPRS call parameters 
 **/
typedef struct {
	UInt8	context_id;									///< context ID
}	GPRS_SrvReq_t ;	

/**	Supplementary service call parameters.  \note this is an interim implementation.
 *	\note the 'number' element will be delete once integration of the dialstring-to
 *	SS API is complete.
 */
typedef struct {
	UInt8				number[MAX_DIGITS+2];	///< (Note: 1 byte for null termination and the other for int code '+')
	SS_SsApiReq_t		ssApiReq ;				/**< parsed SS parameters */
}	CallParams_SS_t ;					

/**	Voice/CSD call parameters  \note this is an interim implementation.
 */
typedef struct {
	UInt8				number[MAX_DIGITS+2];///< (Note: 1 byte for null termination and the other for int code '+')
	VoiceCallParam_t	voiceCallParam;
	TelephoneNumber_t	phoneNumber;
}	CallParams_VCSD_t ;					

/**	USSD call parameters  
  */
typedef struct {
	UInt8				number[PHASE2_MAX_USSD_STRING_SIZE+1];		/**< the number */
}	CallParams_USSD_t ;

/**	GPRS call parameters 
 */
typedef struct {
	UInt8				context_id ;			/**< context ID */
}	CallParams_GPRS_t ;	

/** Call parameters parsed from dial string
 */

typedef struct
{
	CallType_t				callType ;			/**< the call type */

	union {
		CallParams_SS_t		ss ;				/**< supplementary service params */
		CallParams_VCSD_t	cc ;				/**< call control params */
		CallParams_USSD_t	ussd ;				/**< USSD params */
		CallParams_GPRS_t	gprs;				/**< GPRS params */
	}	params ;

	SS_SrvReq_t				ssSrvReq;			///< Supplementary Service parameters
	
} CallParams_t;

/**	Voice/CSD call parameters
 **/
typedef struct {
	CLIRMode_t		clir;								///< Calling Line ID Restriction
	UInt8			dtmfLength;							///< Dual-tone multi-frequency length
	DTMFTone_t		dtmfTones[DTMF_TONE_ARRAY_SIZE];	///< Dual-tone multi-frequency tone
	PartyAddress_t	partyAdd;							///< Party Address
}	CC_SrvReq_t ;	
				

#define	MAX_DIAL_STRING	128


/**
 Call parameters parsed from dial string 
**/

typedef struct
{
	CallType_t			callType;						///< the call type

	union {
		CC_SrvReq_t		ccSrvReq;						///< call control params
		SS_SrvReq_t		ssSrvReq;						///< supplementary service params
		GPRS_SrvReq_t	gprsSrvReq;						///< GPRS params
	} params ;

} DailedService_t;


/**
 Diastring parameters for parsing 
**/

typedef struct
{
	PLMN_t	plmn;										///< PLMN, mcc, mnc
	UInt8	dialStrLength;								///< dialed string length
	UInt8	dialStr[MAX_DIAL_STRING];					///< dialed string
} DailStrParam_t;


/**
 PLMN dynamic dialing related data structures start
**/


#ifndef	_WIN32

#define MAX_PLMN_DYNAMIC_DIALSTR_PREFIX_LEN			16
#define MAX_PLMN_DYNAMIC_DIALSTR_DN_LEN				64
#define MAX_PLMN_DYNAMIC_DIALSTR_SUFFIX_LEN			16
#define MAX_PLMN_DYNAMIC_DIALSTR_TABLE_ENTRIES		256

#else

#define MAX_PLMN_DYNAMIC_DIALSTR_PREFIX_LEN			8
#define MAX_PLMN_DYNAMIC_DIALSTR_DN_LEN				20
#define MAX_PLMN_DYNAMIC_DIALSTR_SUFFIX_LEN			8
#define MAX_PLMN_DYNAMIC_DIALSTR_TABLE_ENTRIES		8

#endif	//_WIN32

/**
 PLMN dynamic dialing entry.

 To make/add a valid entry, all following conditions must be satified
 - At least one valid field of prefix/Dialed-Number/suffix exist.
 - Both prefix and suffix are fixed length if exists.
 - If Dail-Number has variable length, at least one of prefix or suffix MUST exist
 - Call type: only CALLTYPE_SPEECH & CALLTYPE_MOUSSDSUPPSVC allowed

 Detail Entry fileds formated rules are commented below.
**/

typedef struct
{
	Boolean		bValid;											///< a valid entry
	PLMN_t		plmn;											///< PLMN, mcc, mnc

	UInt8		prefixLen;										///< When == 0, no prefix
//	Boolean		bPrefixAppend;									///< When TRUE, append with DN/suffix for API output
	UInt8		prefixStr[MAX_PLMN_DYNAMIC_DIALSTR_PREFIX_LEN];		///< any combination of */#/digits

	Int8		DNLen;											///< when == 0: no DN exist,
																///< when  < 0: veriable lenth DN
																///< when  > 0: fix length DN
	UInt8		DNStr[MAX_PLMN_DYNAMIC_DIALSTR_DN_LEN];				///< Dialed Number, \+?[0-9]{0,MAX_PLMN_DIAL_STRING_DN_LEN}

	UInt8		suffixLen;										///< when == 0: no suffix
//	Boolean		bSuffixAppend;									///< When TRUE, append with prefix/DN for API output
	UInt8		suffixStr[MAX_PLMN_DYNAMIC_DIALSTR_SUFFIX_LEN];		///< any combination of */#/digits

	CallType_t	callType;										///< the call type

} PLMN_Dynamic_DialStr_Entry_t;


/**
 Accessing type for PLMN dynamic dialing table
**/

typedef enum 
{
	PLMN_DYNAMIC_DIALSTR_ACCESS_INIT = 0,	///< Init PLMN dynamis dialing table
	PLMN_DYNAMIC_DIALSTR_ACCESS_ADD,		///< Add one entry to PLMN dynamis dialing table
	PLMN_DYNAMIC_DIALSTR_ACCESS_DELETE,		///< Delete one entry to PLMN dynamis dialing table
	PLMN_DYNAMIC_DIALSTR_ACCESS_CLEAN,		///< Clean all entries in PLMN dynamis dialing table
	PLMN_DYNAMIC_DIALSTR_ACCESS_STORE		///< Store table to database
} PLMN_Dynamic_DialStr_Access_t;;


#endif //_DIALPARSER_DEF_
