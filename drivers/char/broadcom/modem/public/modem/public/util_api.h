//***************************************************************************
//
//	Copyright © 2006-2008 Broadcom Corporation
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
*   @file   util_api.h
*
*   @brief  This file defines the utility functions used by the platform.
*
********************************************************************************************/
#ifndef _UTIL_API_H_
#define _UTIL_API_H_

//***********************************************************************************
/**
	UtilApi_DialStr2PartyAdd() This function parses the dialed string, and coverts it to
	partry address structure, which  consists of Type of Number (TON), Numbering Plan
	Identification (NPI) and number of digits. See GSM 02.30, Section 2.3 for encoding
	rules.
	@param		outPartyAddPtr (out) Called/Calling party phone address pointer.
	@param		inDataPtr (in) Pointer to the dialed string.
	@return		void (out)
**/
void UtilApi_DialStr2PartyAdd(	PartyAddress_t*	outPartyAddPtr,
								const UInt8*	inDataPtr);

UInt8 UtilApi_GetNumOffsetInSsStr(const char *ssStr);

NetworkCause_t UtilApi_Cause2NetworkCause(Cause_t inCause);

NetworkCause_t UtilApi_ErrCodeToNetCause(SS_ErrorCode_t inErrCode);

const UInt8* UtilApi_GetTeToGsmConvTbl ( ClientInfo_t*   inClientInfoPtr );

UInt8* UtilApi_GetGsmToTeConvTbl ( ClientInfo_t*   inClientInfoPtr );

Boolean UtilApi_UCS2CheckAlphaValidity(ALPHA_CODING_t coding, const UInt8 *alpha, UInt16 alpha_size);

Boolean UtilApi_UCS2ConvertToUcs2_80(ALPHA_CODING_t coding, const UInt8 *alpha, UInt16 alpha_size, 
		UInt8 *alpha_out, UInt16 *alpha_size_out );

char* UtilApi_UCS2ConvertToAlphaStr(Boolean ucs2, ALPHA_CODING_t coding, const UInt8 *alpha, UInt16 alpha_size); 

Boolean UtilApi_UCS2ConvertToAlphaData( Boolean ucs2, char *alpha_str, 
		ALPHA_CODING_t *coding, UInt8 *alpha_out, UInt16 *alpha_len );

Int16 UtilApi_UCS2AlphaCompare(UInt8 *a, UInt8 *b, UInt16 size_a, UInt16 size_b, Boolean partial_match);

UInt16 UtilApi_UCS2AlphaDataCpy(ALPHA_CODING_t coding, UInt8 *alpha_dest, UInt8 *alpha_src, UInt16 dest_len, UInt16 src_len);


Boolean UtilApi_DecodeEF_EPSNSC ( UInt8* epsnscRaw, UInt8 len, /*Epsnsc_t*/ void* epsnsc );
Boolean UtilApi_DecodeEF_EPSLOCI ( UInt8* epslociRaw, UInt8 len, /* Epsloci_t*/ void* epsloci );

#endif //_UTIL_API_H_

