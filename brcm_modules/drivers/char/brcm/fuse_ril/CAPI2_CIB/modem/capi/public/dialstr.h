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
*   @file   dialstr.h
*
*   @brief  This file contains definitions for Dial string parsing API
*
*	@note	This is an interim version that is to be used only for ATD parsing
*
****************************************************************************/

#ifndef _DIALSTR_H_
#define _DIALSTR_H_

#include "resultcode.h"
#include "mobcom_types.h"
#include "callparser.h"
#include "ss_def.h"
#include "ss_api.h"
#include "ss_api_old.h"
#include "dialstr_api.h"

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



/** Parse a dial string.  The return value is the call type.  
 *	The DialParms_t argument contains the parameters associated 
 *	with the call type.
 */
CallType_t DIALSTR_Parse( 
	const UInt8			*dialStr,				/**< the dial string */
	CallParams_t		*callParams				/**< the call parameters */
	) ;


/**	Convert call parameters to a dial string.  \note This function
 *	is required by STK and should not be published as an API function.
 */

Result_t DIALSTR_CvtCallParamsToDialStr( 
	CallParams_t		*callParams,			/**< the call parameters */
	UInt8				dialStr [MAX_DIGITS+2] 	///< (Note: 1 byte for null termination and the other for int code '+')
	) ;

Result_t DIALSTR_DialStrParse( 
	const UInt8*		dialStr,
	CallParams_t*		callParams, 
	Boolean*			voiceSuffix
	);

	
void SS_SsType2Reason(SS_CallFwdReason_t* inReasonPtr, SuppSvcType_t inSsType);
void SS_BarSsType2CallBarType(SS_CallBarType_t*	inCallBarPtr, SuppSvcType_t inBarSsType);
UInt8* SsApiCmd2String(SS_SsApiReq_t* inSsApiReqPtr, UInt8* inDialStrPtr);
void SS_BsCode2SvcCls(SS_SvcCls_t* inSvcClsPtr, OptBService_t* inBasicSvcPtr);
void SS_SsData2SsApiReq(SSData_t* inSsDataPtr, SS_SsApiReq_t* inSsApiReqPtr);



#endif // _DIALSTR_H_

