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
*   @file   phonebk_api.h
*
*   @brief  This file defines prototypes for the Phonebook API functions.
*
****************************************************************************/
/**

*   @defgroup   PBKAPIGroup   PhoneBook
*   @brief      This group defines the types and prototypes for the Phonebook API functions.
*	
****************************************************************************/

#ifndef _PHONEBK_API_H_
#define _PHONEBK_API_H_

/**
 * @addtogroup PBKAPIGroup
 * @{
 */


//**************************************************************************************
/**
	Request to find a alpha name within a phonebook.

	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		number (in) pointer to phone number 
	@param		alpha (in) pointer to alpha name

**/
void PbkApi_GetAlpha(ClientInfo_t* inClientInfoPtr, char* number,	PBK_API_Name_t* alpha);


//**************************************************************************************
/**
	This function returns TRUE if the passed ASCII-coded phone number is one of the 
	emergency numbers which consist of 

	1. When SIM/USIM is present: 
	   The emergency numbers read from EF-ECC in SIM/USIM plus "112" and "911". 
 
	2. When SIM/USIM is not present:
 	   Number "000", "08", "112", "110", "118", "119", "911" and "999". 

	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		*phone_number (in) pointer to phone number to be checked
	@return		Boolean ///< TRUE if the call is an emergency call
**/

Boolean PbkApi_IsEmergencyCallNumber(ClientInfo_t* inClientInfoPtr, char *phone_number);


//**************************************************************************************
/**
	This function returns TRUE if the digits provided in a passed ASCII-coded phone
	number are a subset (starting at the first character) of the emergency call numbers. 
	For example, "9", "91" and "911" are considered to partially match "911" and this 
	function returns TRUE. 

    The emergency numbers consist of the following:
	
	1. When SIM/USIM is present: 
	   The emergency numbers read from EF-ECC in SIM/USIM plus "112" and "911". 
 
	2. When SIM/USIM is not present:
 	   Number "000", "08", "112", "110", "118", "119", "911" and "999". 
   
	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		*phone_number (in) pointer to phone number to be checked
	@return		Boolean ///< TRUE if the number is a partial match
**/

Boolean PbkApi_IsPartialEmergencyCallNumber(ClientInfo_t* inClientInfoPtr, char *phone_number);


//**************************************************************************************
/**
	This function requests the information of a phonebook type, including alpha size, digit size,
	number of total entries, number of free entries, the index of the first free entry and the index
	of the first used entry.

	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		pbk_id (in) Phone book ID
	@param		*pbk_access_cb (in) Phonebook access callback
	@return		Result_t 
	@note
	The phonebook task returns the phonebook information in the ::MSG_GET_PBK_INFO_RSP
	message by calling the passed callback function.

**/

Result_t PbkApi_SendInfoReq( ClientInfo_t* inClientInfoPtr, PBK_Id_t pbk_id, CallbackFunc_t* pbk_access_cb );


//**************************************************************************************
/**
	This functions requests all the phonebook entries that match the passed alpha pattern
	(matching is case-insensitive for GSM Default Alphabet encoding).

	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		pbk_id (in) Phone book ID
	@param		alpha_coding (in) Type of coding employed(GSM/UCS..)
	@param		alpha_size (in) number of bytes passed in "alpha".
	@param		*alpha (in) pointer to actual phonebook alpha data
	@param		*pbk_access_cb (in) Phonebook access callback
	@return		Result_t 
	@note
	The phonebook task returns the phonebook entries in the ::MSG_PBK_ENTRY_DATA_RSP
	messages by calling the passed callback function.
**/

Result_t PbkApi_SendFindAlphaMatchMultipleReq( ClientInfo_t* inClientInfoPtr, PBK_Id_t pbk_id, 
	ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha, CallbackFunc_t* pbk_access_cb );


//**************************************************************************************
/**
	This function requests the first phonebook entry that matches the passed alpha pattern
	(matching is case-insensitive for GSM default alphabet encoding). The matching is performed
	in the order of the passed phonebook types "pbkId".

	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		numOfPbk (in) number of phonebook types pointed by pbkId.
	@param		*pbkId (in) Phone book ID
	@param		alpha_coding (in) Type of coding employed(GSM/UCS..)
	@param		alpha_size (in) number of bytes passed in alpha.
	@param		*alpha (in) pointer to actual phonebook alpha data
	@param		*pbkAccessCb (in) Phonebook access callback
	@return		Result_t 
	@note
	The phonebook task returns the phonebook entry in the MSG_PBK_ENTRY_DATA_RSP
	message by calling the passed callback function.
**/

Result_t PbkApi_SendFindAlphaMatchOneReq( ClientInfo_t* inClientInfoPtr, UInt8 numOfPbk, PBK_Id_t *pbkId, 
	ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha, CallbackFunc_t* pbkAccessCb );


//**************************************************************************************
/**
	Function to check if the phone book is ready for operation
	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@return		Boolean ///< TRUE if ready 

**/
Boolean PbkApi_IsReady(ClientInfo_t* inClientInfoPtr);


//**************************************************************************************
/**
	This function requests the phonebook entries whose indices are in the range specified by the
	start_index and end_index arguments. 

	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		pbk_id (in) Phone book ID
	@param		start_index (in) 0-based Index to start reading from
	@param		end_index (in) 0-based Last index to be read from
	@param		*pbk_access_cb (in) Phonebook access callback
	@return		Result_t 
	
	@note
	The phonebook task returns the phonebook entries in the MSG_PBK_ENTRY_DATA_RSP
	messages by calling the passed callback function. The message contains a
	PBK_ENTRY_DATA_RSP_t structure. If and only if the passed "pbk_id" corresponds
	to 3G phonebook (global or local) and USIM is inserted, the "usim_adn_ext_exist" element in the 
	PBK_ENTRY_DATA_RSP_t structure is set TRUE suggesting that the client needs to parse the extended
	3G phonebook data (e.g. additional number and grouping information) in "usim_adn_ext" element (type USIM_PBK_EXT_DATA_t). 

    If the "usim_adn_ext_exist" is FALSE, the "usim_adn_ext" structure must be completely ignored (e.g. for 2G SIM). 

    Due to limited queue size, "(end_index - start_index) + 1" can not be larger than MAX_PBK_ENTRY_READ otherwise error is returned.
**/

Result_t PbkApi_SendReadEntryReq( ClientInfo_t* inClientInfoPtr, PBK_Id_t pbk_id, 
					UInt16 start_index, UInt16 end_index, CallbackFunc_t* pbk_access_cb );


//**************************************************************************************
/**
	This function requests to update a phonebook entry. It is different from PbkApi_SendUpdateEntryReq()
	in that this function does not support the extended 3G USIM ADN phonebook data 
	(e.g. additional number and email address). For USIM ADN phonebook, only EF-ADN and EF-EXT1 can be updated and
	other files (e.g. EF-ANR and EF-EMAIL) are not updated at all. 

	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		pbk_id (in) Phone book ID
	@param		special_fax_num (in) set to TRUE for writing the Fax number in MSISDN phonebook.
	@param		index (in) 0-based Index to be written to

    @param		type_of_number (in) type of number: TON/NPI combined: see Section 10.5.4.7 of 3GPP 24.008. Typically 
				one of UNKNOWN_TON_UNKNOWN_NPI (128), UNKNOWN_TON_ISDN_NPI (129) & INTERNA_TON_ISDN_NPI (145), but it 
				can be another valid value defined in spec.
	
	@param		*number (in) Pointer to number to be written
	@param		alpha_coding (in) Type of coding employed(GSM/UCS..)
	@param		alpha_size (in) number of bytes passed in "alpha".
	@param		*alpha (in) pointer to actual phonebook alpha data
	@param		*pbk_access_cb (in) Phonebook access callback

	@return		Result_t 
	@note
	The phonebook task returns the update result in the MSG_WRT_PBK_ENTRY_RSP message
	by calling the passed callback function.
	If FIRST_PHONEBK_FREE_INDEX is passed in "index", the entry is written into the first free
	entry found (this is applicable only for PB_ME, PB_ADN and PB_FDN phonebooks).
	If NULL is passed in "number", the phonebook entry is deleted.
**/

Result_t PbkApi_SendWriteEntryReq( ClientInfo_t* inClientInfoPtr, PBK_Id_t pbk_id, Boolean special_fax_num, UInt16 index, UInt8 type_of_number, 
				char *number, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha, CallbackFunc_t* pbk_access_cb );


//**************************************************************************************
/**
	This function requests to update a phonebook entry. It is different from PbkApi_SendWriteEntryReq()
	in that this function supports the extended 3G USIM ADN phonebook data (e.g. additional number and 
	email address). 

	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		pbk_id (in) Phone book ID
	@param		special_fax_num (in) set to TRUE for writing the Fax number in MSISDN phonebook.
	@param		index (in) 0-based Index to be written to
	
	@param		type_of_number (in) type of number: TON/NPI combined: see Section 10.5.4.7 of 3GPP 24.008. Typically 
				one of UNKNOWN_TON_UNKNOWN_NPI (128), UNKNOWN_TON_ISDN_NPI (129) & INTERNA_TON_ISDN_NPI (145), but it 
				can be another valid value defined in spec.

	@param		*number (in) Pointer to number to be written
	@param		alpha_coding (in) Type of coding employed(GSM/UCS..)
	@param		alpha_size (in) number of bytes passed in "alpha".
	@param		*alpha (in) pointer to actual phonebook alpha data
	@param		*usim_adn_ext_data (in) extended 3G phonebook data (pass NULL for 2G ADN). The client
				can also pass NULL so that the phonebook module skips updating the extended 3G phonebook files in USIM, i.e. 
				only EF-ADN and EF-EXT1 are updated and other files (e.g. EF-ANR and EF-EMAIL) are not updated at all.
	@param		*pbk_access_cb (in) Phonebook access callback

	@return		Result_t 
	@note
	The phonebook task returns the update result in the MSG_WRT_PBK_ENTRY_RSP message
	by calling the passed callback function.
	If FIRST_PHONEBK_FREE_INDEX is passed in "index", the entry is written into the first free
	entry found (this is applicable only for PB_ME, PB_ADN and PB_FDN phonebooks).
	If NULL is passed in "number", the phonebook entry is deleted.
**/
Result_t PbkApi_SendUpdateEntryReq( ClientInfo_t* inClientInfoPtr, PBK_Id_t pbk_id, Boolean special_fax_num, UInt16 index, UInt8 type_of_number, 
	char *number, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha, 
	const USIM_PBK_EXT_DATA_t *usim_adn_ext_data, CallbackFunc_t* pbk_access_cb );


//**************************************************************************************
/**
	This function sends the request to get the USIM PBK additional number alpha string 
	EF-AAS information, e.g. alpha string length and number of records

    The "aas_data" element in the USIM_PBK_INFO_t structure in the response for 
	PbkApi_SendInfoReq() function also provides the configuration information in addition to 
	the actual EF-GAS record contents. The client can choose to use PbkApi_SendInfoReq() function
	instead. 

	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		pbk_id (in) Phone book ID."pbk_id" shall be one of "PB_3G_LOCAL" and "PB_3G_GLOBAL".
	@param		*pbk_access_cb (in) Phonebook access callback
	@return		RESULT_OK 
	@note		A MSG_GET_USIM_PBK_ALPHA_INFO_RSP message will be posted back
**/
Result_t PbkApi_SendUsimAasInfoReq(ClientInfo_t* inClientInfoPtr, PBK_Id_t pbk_id, CallbackFunc_t* pbk_access_cb);


//**************************************************************************************
/**
	This function sends the request to read the USIM PBK additional number alpha string 
	in EF-AAS. For example, the AT&T 3G USIM has the following 5 entries in EF-AAS:
	"Work"; "Home"; "Mobile"; "Fax"; "Other". These entries are shared among all 
	ADN entries in AT&T USIM. 

    This function is applicable for 3G USIM only and shall be called only if EF-AAS
	is provisioned in USIM. 

    The EF-AAS provision information can be obtained in the "aas_data" element in the 
	USIM_PBK_INFO_t structure in the response for PbkApi_SendInfoReq() function or 
	USIM_PBK_ALPHA_INFO_RSP_t structure in the response for PbkApi_SendUsimAasInfoReq()

	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		index (in) 0-based index
	@param		pbk_id (in) Phone book ID. "pbk_id" shall be one of "PB_3G_LOCAL" and "PB_3G_GLOBAL".
	@param		*pbk_access_cb (in) Phonebook access callback
	@return		RESULT_OK 
	@note		A MSG_READ_USIM_PBK_ALPHA_ENTRY_RSP message will be posted back
**/
Result_t PbkApi_SendUsimAasReadReq(ClientInfo_t* inClientInfoPtr, UInt16 index, PBK_Id_t pbk_id, CallbackFunc_t* pbk_access_cb);


//**************************************************************************************
/**
	This function sends the request to update the USIM PBK additional number alpha string 
	in EF-AAS. 

    This function is applicable for 3G USIM only and shall be called only if EF-AAS
	is provisioned in USIM. 

    The EF-AAS provision information can be obtained in the "aas_data" element in the 
	USIM_PBK_INFO_t structure in the response for PbkApi_SendInfoReq() function or 
	USIM_PBK_ALPHA_INFO_RSP_t structure in the response for PbkApi_SendUsimAasInfoReq()

	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		index (in) 0-based index
	@param		pbk_id (in) Phone book ID."pbk_id" shall be one of "PB_3G_LOCAL" and "PB_3G_GLOBAL". 
	@param		alpha_coding (in) Type of coding employed(GSM/UCS..)
	@param		alpha_size (in) number of bytes passed in "alpha"
	@param		*alpha (in) pointer to actual phonebook alpha data
	@param		*pbk_access_cb (in) Phonebook access callback
	@return		RESULT_OK 
	@note		A MSG_WRITE_USIM_PBK_ALPHA_ENTRY_RSP message will be posted back
**/
Result_t PbkApi_SendUsimAasUpdateReq( ClientInfo_t* inClientInfoPtr, UInt16 index, PBK_Id_t pbk_id, ALPHA_CODING_t alpha_coding, 
								   UInt8 alpha_size, const UInt8 *alpha, CallbackFunc_t* pbk_access_cb );


//**************************************************************************************
/**
	This function sends the request to get the USIM PBK grouping alpha string 
	EF-GAS information, e.g. alpha string length and number of records

    This function is applicable for 3G USIM only. The "gas_data" element in the 
	USIM_PBK_INFO_t structure in the response for PbkApi_SendInfoReq() function
	also provides the configuration information in addition to the actual
	EF-GAS record contents. The client can choose to use PbkApi_SendInfoReq() function
	instead. 

	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		pbk_id (in) Phone book ID. "pbk_id" shall be one of "PB_3G_LOCAL" and "PB_3G_GLOBAL".
	@param		*pbk_access_cb (in) Phonebook access callback
	@return		RESULT_OK 
	@note		A MSG_GET_USIM_PBK_ALPHA_INFO_RSP message will be posted back
**/
Result_t PbkApi_SendUsimGasInfoReq(ClientInfo_t* inClientInfoPtr, PBK_Id_t pbk_id, CallbackFunc_t* pbk_access_cb);


//**************************************************************************************
/**
	This function sends the request to read the USIM PBK grouping alpha string in 
	EF-GAS. For example, the AT&T 3G USIM has the following 5 entries in EF-GAS:
	"Business"; "Family"; "Friends"; "Services"; "Other". These entries are shared 
	among all ADN entries in AT&T USIM. 

    This function is applicable for 3G USIM only. 

    The EF-GAS provision and/or data information can be obtained in the "gas_data" element in the 
	USIM_PBK_INFO_t structure in the response for PbkApi_SendInfoReq() function or 
	USIM_PBK_ALPHA_INFO_RSP_t structure in the response for PbkApi_SendUsimGasInfoReq(). 

	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		index (in) 0-based index
	@param		pbk_id (in) Phone book ID. "pbk_id" shall be one of "PB_3G_LOCAL" and "PB_3G_GLOBAL".
	@param		*pbk_access_cb (in) Phonebook access callback
	@return		RESULT_OK 
	@note		A MSG_READ_USIM_PBK_ALPHA_ENTRY_RSP message will be posted back
**/
Result_t PbkApi_SendUsimGasReadReq(ClientInfo_t* inClientInfoPtr, UInt16 index, PBK_Id_t pbk_id, CallbackFunc_t* pbk_access_cb);


//**************************************************************************************
/**
	This function sends the request to update the USIM PBK grouping Information alpha string 
	in EF-GAS. It is applicable for 3G USIM only and shall be called only if EF-GAS
	is provisioned in USIM. 

    The EF-GAS provision information can be obtained in the "gas_data" element in the 
	USIM_PBK_INFO_t structure in the response for PbkApi_SendInfoReq() function or 
	USIM_PBK_ALPHA_INFO_RSP_t structure in the response for PbkApi_SendUsimGasInfoReq()

	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		index (in) 0-based index
	@param		pbk_id (in) Phone book ID. "pbk_id" shall be one of "PB_3G_LOCAL" and "PB_3G_GLOBAL".
	@param		alpha_coding (in) Type of coding employed(GSM/UCS..)
	@param		alpha_size (in) number of bytes passed in "alpha"
	@param		*alpha (in) pointer to actual phonebook alpha data
	@param		*pbk_access_cb (in) Phonebook access callback
	@return		RESULT_OK 
	@note		A MSG_WRITE_USIM_PBK_ALPHA_ENTRY_RSP message will be posted back
**/
Result_t PbkApi_SendUsimGasUpdateReq( ClientInfo_t* inClientInfoPtr, UInt16 index, PBK_Id_t pbk_id, ALPHA_CODING_t alpha_coding, 
								   UInt8 alpha_size, const UInt8 *alpha, CallbackFunc_t* pbk_access_cb );


//**************************************************************************************
/**
	This function sends the request to read the USIM PBK Hidden key information.

	The hidden key shall be 4-8 digits according to Section 4.2.42 of 4GPP 31.102. 
	It is applicable for USIM only and it needs to be verified for an ADN entry
	whose status is marked as "hidden" before a user can access the entry. See "is_hidden" 
	element in "USIM_PBK_EXT_DATA_t" structure returned for an ADN entry in 3G USIM. 

	The client shall verify that the hidden key is provisioned in USIM before calling
	this function. The hidden key provision status is provided in "hidden_key_exist" 
	element in USIM_PBK_INFO_t structure in the response for PbkApi_SendInfoReq() function. 

	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		*pbk_access_cb (in) Phonebook access callback
	@return		RESULT_OK 
	@note		A MSG_READ_USIM_PBK_HDK_ENTRY_RSP message will be posted back
**/
Result_t PbkApi_SendUsimHdkReadReq(ClientInfo_t* inClientInfoPtr, CallbackFunc_t* pbk_access_cb);






/** @} */


#endif


