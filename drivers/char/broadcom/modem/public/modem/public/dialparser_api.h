//*********************************************************************
//
//	Copyright © 2011 Broadcom Corporation
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
*   @file   dialparser_api.h
*
*   @brief  This file defines the dial parser API.
*
****************************************************************************/
/**
*
*   @defgroup   DialedServiceGroup	Mobile Station DialedService APIs
*   @ingroup    SystemGroup				
*
*   @brief      This group provides dialing related API functions.
*
****************************************************************************/

#ifndef __DIALEDSERVICE_API_H__
#define __DIALEDSERVICE_API_H__


/**
 * @addtogroup DialedServiceGroup
 * @{
 */ 

//**************************************************************************************
/**
	This function will parse the dialed string and return detail instrction and infomation of a call.

  	@param		inClientInfoPtr		(in) pointer to the client Info
	@param		inDailStrParamPtr	(in) dialed string related paramaters
	@param		outDailSrvPtr		(out) result of parsed dialed string detail infomation

**/

void UtilApi_DialParser(
			ClientInfo_t*		inClientInfoPtr,
			DailStrParam_t*		inDailStrParamPtr,
			DailedService_t*	outDailSrvPtr
			);


//**************************************************************************************
/**
	This function access the PLMN dynamic dialed string entry table

  	@param		inClientInfoPtr		(in) pointer to the client Info
	@param		inAccessType		(in) access type
	@param		inPlmnEntryPtr		(in) entry need to be added/deleted
	@param		inTableIndex		(in) index of the table entry, 0 base.

	@return		RESULT_OK or NOT

**/


Result_t UtilApi_DialParserDynamicAccess(
				ClientInfo_t*						inClientInfoPtr,
				PLMN_Dynamic_DialStr_Access_t		inAccessType,
				PLMN_Dynamic_DialStr_Entry_t*		inPlmnEntryPtr,
				UInt16								inTableIndex
				);


//**************************************************************************************
/**
	This function search the PLMN dynamic dialed string entry table see if an entry exist

  	@param		inClientInfoPtr		(in) pointer to the client Info
	@param		inDailStrParamPtr	(in) entry need to be lookup
	@param		outDailSrvPtr		(out) result of found and parsed dialed string infomation

	@return		RESULT_OK or NOT

**/


Result_t UtilApi_DialParserDynamicLookup(
				ClientInfo_t* inClientInfoPtr,
				DailStrParam_t* inDailStrParamPtr,
				DailedService_t* outDailSrvPtr
				);


//**************************************************************************************
/**
	This generic function will parse the dialed string and return detail infomation of a call.

	@param		inDialStr		(in) dialed string
	@param		inDialStrLen	(in) dialed string length
	@param		outDailSrvPtr	(out) result of parsed dialed string detail infomation

	@return		none

**/

void UtilApi_DialParserGen(
				UInt8*				inDialStr,
				UInt8				inDialStrLen,
				DailedService_t*	outDailSrvPtr
				);

/** @} */


#endif	//	__DIALEDSERVICE_API_H__



