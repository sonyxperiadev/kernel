/****************************************************************************
*
*     Copyright (c) 2007-2008 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license 
*   agreement governing use of this software, this software is licensed to you 
*   under the terms of the GNU General Public License version 2, available 
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL"). 
*
*   Notwithstanding the above, under no circumstances may you combine this 
*   software in any way with any other Broadcom software provided under a license 
*   other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/
/**
*
*   @file   capi2_phonebk_api.h
*
*   @brief  This file defines the types and prototypes for the CAPI2 Phonebook API functions.
*
****************************************************************************/
/**

*   @defgroup   CAPI2_PBKAPIGroup   PhoneBook
*   @ingroup    CAPI2_PBKGroup
*
*   @brief      This group defines the types and prototypes for the Phonebook API functions.
*	
****************************************************************************/

#ifndef _CAPI2_PHONEBK_API_H_
#define _CAPI2_PHONEBK_API_H_


//******************************************************************************
//	 			include block
//******************************************************************************
#ifdef __cplusplus
extern "C" {
#endif
/**
 * @addtogroup CAPI2_PBKAPIGroup
 * @{
 */

//***************************************************************************************
/**
	This function sends the request to read the USIM PBK Hidden key information.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_READ_USIM_PBK_HDK_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::USIM_PBK_READ_HDK_ENTRY_RSP_t
**/
void CAPI2_PbkApi_SendUsimHdkReadReq(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function sends the request to update the USIM PBK Hidden key information.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		hidden_key (in)  Hidden key information.
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_WRITE_USIM_PBK_HDK_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::USIM_PBK_UPDATE_HDK_ENTRY_RSP_t
**/
void CAPI2_PbkApi_SendUsimHdkUpdateReq(ClientInfo_t* inClientInfoPtr, const HDKString_t *hidden_key);

//***************************************************************************************
/**
	This function sends the request to read the USIM PBK additional number alpha string <br>in EF-AAS. For example the AT&T 3G USIM has the following 5 entries in EF-AAS:<br>"Work"; "Home"; "Mobile"; "Fax"; "Other". These entries are shared among all <br>ADN entries in AT&T USIM.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		index (in)  0-based index
	@param		pbk_id (in)  Phone book ID. "pbk_id" shall be one of "PB_3G_LOCAL" and "PB_3G_GLOBAL".
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_READ_USIM_PBK_ALPHA_AAS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::USIM_PBK_READ_ALPHA_ENTRY_RSP_t
**/
void CAPI2_PbkApi_SendUsimAasReadReq(ClientInfo_t* inClientInfoPtr, UInt16 index, PBK_Id_t pbk_id);

//***************************************************************************************
/**
	This function sends the request to update the USIM PBK additional number alpha string <br>in EF-AAS. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		index (in)  0-based index
	@param		pbk_id (in)  Phone book ID."pbk_id" shall be one of "PB_3G_LOCAL" and "PB_3G_GLOBAL".
	@param		alpha_coding (in)  Type of coding employedGSM/UCS..
	@param		alpha_size (in)  number of bytes passed in "alpha"
	@param		alpha (in)  pointer to actual phonebook alpha data
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_WRITE_USIM_PBK_ALPHA_AAS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::USIM_PBK_UPDATE_ALPHA_ENTRY_RSP_t
**/
void CAPI2_PbkApi_SendUsimAasUpdateReq(ClientInfo_t* inClientInfoPtr, UInt16 index, PBK_Id_t pbk_id, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, const UInt8* alpha);

//***************************************************************************************
/**
	This function sends the request to read the USIM PBK grouping alpha string in <br>EF-GAS. For example the AT&T 3G USIM has the following 5 entries in EF-GAS:<br>"Business"; "Family"; "Friends"; "Services"; "Other". These entries are shared <br>among all ADN entries in AT&T USIM.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		index (in)  0-based index
	@param		pbk_id (in)  Phone book ID. "pbk_id" shall be one of "PB_3G_LOCAL" and "PB_3G_GLOBAL".
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_READ_USIM_PBK_ALPHA_GAS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::USIM_PBK_READ_ALPHA_ENTRY_RSP_t
**/
void CAPI2_PbkApi_SendUsimGasReadReq(ClientInfo_t* inClientInfoPtr, UInt16 index, PBK_Id_t pbk_id);

//***************************************************************************************
/**
	This function sends the request to update the USIM PBK grouping Information alpha string <br>in EF-GAS. It is applicable for 3G USIM only and shall be called only if EF-GAS <br>is provisioned in USIM.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		index (in)  0-based index
	@param		pbk_id (in)  Phone book ID. "pbk_id" shall be one of "PB_3G_LOCAL" and "PB_3G_GLOBAL".
	@param		alpha_coding (in)  Type of coding employedGSM/UCS..
	@param		alpha_size (in)  number of bytes passed in "alpha"
	@param		alpha (in)  pointer to actual phonebook alpha data
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_WRITE_USIM_PBK_ALPHA_GAS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::USIM_PBK_UPDATE_ALPHA_ENTRY_RSP_t
**/
void CAPI2_PbkApi_SendUsimGasUpdateReq(ClientInfo_t* inClientInfoPtr, UInt16 index, PBK_Id_t pbk_id, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, const UInt8* alpha);

//***************************************************************************************
/**
	This function sends the request to get the USIM PBK additional number alpha string <br>EF-AAS information e.g. alpha string length and number of records
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		pbk_id (in)  Phone book ID."pbk_id" shall be one of "PB_3G_LOCAL" and "PB_3G_GLOBAL".
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_GET_USIM_PBK_ALPHA_INFO_AAS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::USIM_PBK_ALPHA_INFO_RSP_t
**/
void CAPI2_PbkApi_SendUsimAasInfoReq(ClientInfo_t* inClientInfoPtr, PBK_Id_t pbk_id);

//***************************************************************************************
/**
	This function sends the request to get the USIM PBK grouping alpha string <br>EF-GAS information e.g. alpha string length and number of records
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		pbk_id (in)  Phone book ID. "pbk_id" shall be one of "PB_3G_LOCAL" and "PB_3G_GLOBAL".
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_GET_USIM_PBK_ALPHA_INFO_GAS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::USIM_PBK_ALPHA_INFO_RSP_t
**/
void CAPI2_PbkApi_SendUsimGasInfoReq(ClientInfo_t* inClientInfoPtr, PBK_Id_t pbk_id);

//***************************************************************************************
/**
	Request to find a alpha name within a phonebook.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		number (in)  pointer to phone number 
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_PBK_GETALPHA_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : void
**/
void CAPI2_PbkApi_GetAlpha(ClientInfo_t* inClientInfoPtr, char* number);

//***************************************************************************************
/**
	This function returns TRUE if the passed ASCII-coded phone number is one of the <br>emergency numbers which consist of <br>1. When SIM/USIM is present: <br>The emergency numbers read from EF-ECC in SIM/USIM plus "112" and "911". <br><br>2. When SIM/USIM is not present: <br>Number "000" "08" "112" "110" "118" "119" "911" and "999".
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		phone_number (in)  pointer to phone number to be checked
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_PBK_ISEMERGENCYCALLNUMBER_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : Boolean
**/
void CAPI2_PbkApi_IsEmergencyCallNumber(ClientInfo_t* inClientInfoPtr, char* phone_number);

//***************************************************************************************
/**
	This function returns TRUE if the digits provided in a passed ASCII-coded phone <br>number are a subset starting at the first character of the emergency call numbers. <br>For example "9" "91" and "911" are considered to partially match "911" and this <br>function returns TRUE.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		phone_number (in)  pointer to phone number to be checked
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : Boolean
**/
void CAPI2_PbkApi_IsPartialEmergencyCallNumber(ClientInfo_t* inClientInfoPtr, char* phone_number);

//***************************************************************************************
/**
	This function requests the information of a phonebook type including alpha size digit size <br>number of total entries number of free entries the index of the first free entry and the index <br>of the first used entry.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		pbk_id (in)  Phone book ID
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_GET_PBK_INFO_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::PBK_INFO_RSP_t
**/
void CAPI2_PbkApi_SendInfoReq(ClientInfo_t* inClientInfoPtr, PBK_Id_t pbk_id);

//***************************************************************************************
/**
	This functions requests all the phonebook entries that match the passed alpha pattern <br>matching is case-insensitive for GSM Default Alphabet encoding.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		pbk_id (in)  Phone book ID
	@param		alpha_coding (in)  Type of coding employedGSM/UCS..
	@param		alpha_size (in)  number of bytes passed in "alpha".
	@param		alpha (in)  pointer to actual phonebook alpha data
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_PBK_ENTRY_DATA_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::PBK_ENTRY_DATA_RSP_t
**/
void CAPI2_PbkApi_SendFindAlphaMatchMultipleReq(ClientInfo_t* inClientInfoPtr, PBK_Id_t pbk_id, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha);

//***************************************************************************************
/**
	This function requests the first phonebook entry that matches the passed alpha pattern <br>matching is case-insensitive for GSM default alphabet encoding. The matching is performed <br>in the order of the passed phonebook types "pbkId".
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		numOfPbk (in) Param is numOfPbk
	@param		pbk_id (in)  Phone book ID
	@param		alpha_coding (in)  Type of coding employedGSM/UCS..
	@param		alpha_size (in)  number of bytes passed in "alpha".
	@param		alpha (in)  pointer to actual phonebook alpha data
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_PBK_ENTRY_DATA_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::PBK_ENTRY_DATA_RSP_t
**/
void CAPI2_PbkApi_SendFindAlphaMatchOneReq(ClientInfo_t* inClientInfoPtr, UInt8 numOfPbk, PBK_Id_t *pbk_id, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha);

//***************************************************************************************
/**
	Function to check if the phone book is ready for operation
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_PBK_ISREADY_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : Boolean
**/
void CAPI2_PbkApi_IsReady(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function requests the phonebook entries whose indices are in the range specified by the <br>start_index and end_index arguments.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		pbk_id (in)  Phone book ID
	@param		start_index (in)  0-based Index to start reading from
	@param		end_index (in)  0-based Last index to be read from
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_PBK_ENTRY_DATA_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::PBK_ENTRY_DATA_RSP_t
**/
void CAPI2_PbkApi_SendReadEntryReq(ClientInfo_t* inClientInfoPtr, PBK_Id_t pbk_id, UInt16 start_index, UInt16 end_index);

//***************************************************************************************
/**
	This function requests to update a phonebook entry. It is different from PbkApi_SendUpdateEntryReq <br>in that this function does not support the extended 3G USIM ADN phonebook data <br>e.g. additional number and email address.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		pbk_id (in)  Phone book ID
	@param		special_fax_num (in)  set to TRUE for writing the Fax number in MSISDN phonebook.
	@param		index (in)  0-based Index to be written to
	@param		type_of_number (in)  type of number: TON/NPI combined
	@param		number (in)  Pointer to number to be written
	@param		alpha_coding (in)  Type of coding employedGSM/UCS..
	@param		alpha_size (in)  number of bytes passed in "alpha".
	@param		alpha (in)  pointer to actual phonebook alpha data
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_WRT_PBK_ENTRY_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::PBK_WRITE_ENTRY_RSP_t
**/
void CAPI2_PbkApi_SendWriteEntryReq(ClientInfo_t* inClientInfoPtr, PBK_Id_t pbk_id, Boolean special_fax_num, UInt16 index, UInt8 type_of_number, char* number, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha);

//***************************************************************************************
/**
	This function requests to update a phonebook entry. It is different from PbkApi_SendWriteEntryReq <br>in that this function supports the extended 3G USIM ADN phonebook data e.g. additional number and <br>email address.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		pbk_id (in)  Phone book ID
	@param		special_fax_num (in)  set to TRUE for writing the Fax number in MSISDN phonebook.
	@param		index (in)  0-based Index to be written to
	@param		type_of_number (in)  type of number: TON/NPI combined
	@param		number (in)  Pointer to number to be written
	@param		alpha_coding (in)  Type of coding employedGSM/UCS..
	@param		alpha_size (in)  number of bytes passed in "alpha".
	@param		alpha (in)  pointer to actual phonebook alpha data
	@param		usim_adn_ext_data (in) Param is usim_adn_ext_data
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_WRT_PBK_ENTRY_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::PBK_WRITE_ENTRY_RSP_t
**/
void CAPI2_PbkApi_SendUpdateEntryReq(ClientInfo_t* inClientInfoPtr, PBK_Id_t pbk_id, Boolean special_fax_num, UInt16 index, UInt8 type_of_number, char* number, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha, const USIM_PBK_EXT_DATA_t *usim_adn_ext_data);

//***************************************************************************************
/**
	This function requests to check whether the passed non-USSD number can be dialed according to the <br>FDN setting and the FDN phonebook contents.For USSD number use PbkApi_IsUssdDiallable API instead.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		number (in)  Pointer to NULL terminated number to be checked. 
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_CHK_PBK_DIALLED_NUM_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : ::PBK_CHK_NUM_DIALLABLE_RSP_t
**/
void CAPI2_PbkApi_SendIsNumDiallableReq(ClientInfo_t* inClientInfoPtr, char* number);

//***************************************************************************************
/**
	This function checks whether the passed non-USSD number can be dialed according to the <br>FDN setting and the FDN phonebook contents.For USSD number checking use <br>PbkApi_IsUssdDiallable API instead.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		number (in)  Buffer that stores the NULL terminated number
	@param		is_voice_call (in)  TRUE if the number is for voice call
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_PBK_ISNUMDIALLABLE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : Boolean
**/
void CAPI2_PbkApi_IsNumDiallable(ClientInfo_t* inClientInfoPtr, const char* number, Boolean is_voice_call);

//***************************************************************************************
/**
	This function checks whether the passed non-USSD number is barred according to the <br>BDN setting and the BDN phonebook contents.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		number (in)  Buffer that stores the NULL terminated number
	@param		is_voice_call (in)  TRUE if the number is for voice call
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_PBK_ISNUMBARRED_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : Boolean
**/
void CAPI2_PbkApi_IsNumBarred(ClientInfo_t* inClientInfoPtr, const char* number, Boolean is_voice_call);

//***************************************************************************************
/**
	This function checks whether the passed USSD number can be dialed according to the <br>FDN setting and the FDN phonebook contents.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		data (in)  Buffer that stores the USSD number
	@param		dcs (in)  USSD DCS byte in CB DCS format
	@param		dcsLen (in)  Number of bytes passed in "data".
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_PBK_ISUSSDDIALLABLE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : Boolean
**/
void CAPI2_PbkApi_IsUssdDiallable(ClientInfo_t* inClientInfoPtr, const UInt8 *data, UInt8 dcs, UInt8 dcsLen);

//***************************************************************************************
/**
	This function enables/disables the FDN check in phonebook.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		fdn_chk_on (in) Param is fdn_chk_on
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_PBK_SET_FDN_CHECK_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_PbkApi_SetFdnCheck(ClientInfo_t* inClientInfoPtr, Boolean fdn_chk_on);

//***************************************************************************************
/**
	This function returns the FDN check enabled/disabled status in phonebook.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_PBK_GET_FDN_CHECK_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData : Boolean
**/
void CAPI2_PbkApi_GetFdnCheck(ClientInfo_t* inClientInfoPtr);

/** @} */

#ifdef __cplusplus
}
#endif

#endif

