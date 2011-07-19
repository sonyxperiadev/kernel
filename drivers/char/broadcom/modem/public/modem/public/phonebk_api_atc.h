//***************************************************************************
//
//	Copyright © 2004-2010 Broadcom Corporation
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
*   @file   phonebk_api_atc.h
*
*   @brief  This file defines prototypes for the Phonebook API functions. (used by ATC only)
*
****************************************************************************/
/**

*   @brief      This group defines the types and prototypes for the Phonebook API functions.
*	
****************************************************************************/

#ifndef _PHONEBK_API_ATC_H_
#define _PHONEBK_API_ATC_H_



//**************************************************************************************
/**	This function sends the request to update the USIM PBK Hidden key information.
	
	The hidden key shall be 4-8 digits according to Section 4.2.42 of 4GPP 31.102. 
	It is applicable for USIM only and it needs to be verified for an ADN entry
	whose status is marked as "hidden" before a user can access the entry. 
	See "is_hidden" element in "USIM_PBK_EXT_DATA_t" structure returned for an ADN entry in 3G USIM. 

	The client shall verify that the hidden key is provisioned in USIM before calling
	this function. The hidden key provision status is provided in "hidden_key_exist" 
	element in USIM_PBK_INFO_t structure in the response for PbkApi_SendInfoReq() function. 

	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		*hidden_key Hidden key information.
	@param		*pbk_access_cb (in) Phonebook access callback
	@return		RESULT_OK 
	@note		A MSG_WRITE_USIM_PBK_HDK_ENTRY_RSP message will be posted back
**/
Result_t PbkApi_SendUsimHdkUpdateReq( ClientInfo_t* inClientInfoPtr, const HDKString_t *hidden_key,
								    CallbackFunc_t* pbk_access_cb );


//**************************************************************************************
/**
	This function enables/disables the FDN check in phonebook. 

    By default FDN check in phonebook is on; It can be disabled by client for valid reasons such as:
	
	1. Client handles the STK Setup Call; STK Send SS/USSD; STK Send SMS and thus we must
	   disable FDN check when a call is set up or when SMS is sent because the request originates
	   from SIM and according to GSM 11.14 those requests must not be checked against FDN phonebook. 

    2. MMI Client already performs FDN check, there is no need to do it again. 
	
	When FDN check in phonebook is disabled, PbkApi_SendIsNumDiallableReq, PbkApi_IsNumDiallable() & 
	PbkApi_IsUssdDiallable() always return TRUE to indicate FDN check is passed. 

	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		fdn_chk_on (in) TRUE to enable FDN check in phonebook; FALSE to disable it. 

	@return		None
	@note
**/
void PbkApi_SetFdnCheck(ClientInfo_t* inClientInfoPtr, Boolean fdn_chk_on);

/**
	This function returns the FDN check enabled/disabled status in phonebook. 

    By default FDN check in phonebook is on; It can be disabled by client for valid reasons such as:
	
	1. Client handles the STK Setup Call; STK Send SS/USSD; STK Send SMS and thus we must
	   disable FDN check when a call is set up or when SMS is sent because the request originates
	   from SIM and according to GSM 11.14 those requests must not be checked against FDN phonebook. 

    2. MMI Client already performs FDN check, there is no need to do it again. 
	
	When FDN check in phonebook is disabled, PbkApi_SendIsNumDiallableReq, PbkApi_IsNumDiallable() & 
	PbkApi_IsUssdDiallable() always return TRUE to indicate FDN check is passed. 

	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@return		TRUE if FDN check is enabled; FALSE otherwise. 
	@note
**/
Boolean PbkApi_GetFdnCheck(ClientInfo_t* inClientInfoPtr);

//**************************************************************************************
/**
	This function requests to check whether the passed non-USSD number can be dialed according to the
	FDN setting and the FDN phonebook contents.	For USSD number, use PbkApi_IsUssdDiallable() API instead. 

	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		*number (in) Pointer to NULL terminated number to be checked. 
	@param		*pbk_access_cb (in) Phonebook access callback

	@return		Result_t 
	@note
	The phonebook task returns the result in the MSG_CHK_PBK_DIALLED_NUM_RSP message
	by calling the passed callback function.
**/
Result_t PbkApi_SendIsNumDiallableReq(ClientInfo_t* inClientInfoPtr, char *number, CallbackFunc_t* pbk_access_cb);


//**************************************************************************************
/**
	This function checks whether the passed non-USSD number can be dialed according to the
	FDN setting and the FDN phonebook contents.	For USSD number checking, use 
	PbkApi_IsUssdDiallable() API instead. 

	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		number (in) Buffer that stores the NULL terminated number
	@param		is_voice_call (in) TRUE if the number is for voice call

	@return		TRUE if number is diallable; FALSE otherwise. 
	@note
	The result is returned synchronously. 
**/
Boolean PbkApi_IsNumDiallable(ClientInfo_t* inClientInfoPtr, const char *number, Boolean is_voice_call);


//**************************************************************************************
/**
	This function checks whether the passed USSD number can be dialed according to the
	FDN setting and the FDN phonebook contents.	
	
	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		data (in) Buffer that stores the USSD number
	@param		dcs (in) USSD DCS byte in CB DCS format
	@param		len (in) Number of bytes passed in "data". 

	@return		TRUE if USSD is diallable; FALSE otherwise. 
	@note
**/
Boolean PbkApi_IsUssdDiallable(ClientInfo_t* inClientInfoPtr, const UInt8 *data, UInt8 dcs, UInt8 len);

//**************************************************************************************
/**
	This function checks whether the passed non-USSD number is barred according to the
	BDN setting and the BDN phonebook contents.

	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		number (in) Buffer that stores the NULL terminated number
	@param		is_voice_call (in) TRUE if the number is for voice call

	@return		TRUE if number is barred; FALSE otherwise. 
	@note
	The result is returned synchronously. 
**/
Boolean PbkApi_IsNumBarred(ClientInfo_t* inClientInfoPtr, const char *number, Boolean is_voice_call);

#endif
