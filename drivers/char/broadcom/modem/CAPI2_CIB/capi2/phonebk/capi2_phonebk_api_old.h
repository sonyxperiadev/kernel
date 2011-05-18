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
*   @file   capi2_phonebk_api_old.h
*
*   @brief  This file defines the types and prototypes for the CAPI2 Phonebook API functions.
*
****************************************************************************/
/**

*   @defgroup   CAPI2_PBKAPIOLDGroup   PhoneBook
*   @ingroup    CAPI2_PBKOLDGroup
*
*   @brief      This group defines the types and prototypes for the Phonebook API functions.
*	
****************************************************************************/

#ifndef _CAPI2_PHONEBK_API_OLD_H_
#define _CAPI2_PHONEBK_API_OLD_H_


//******************************************************************************
//	 			include block
//******************************************************************************
#ifdef __cplusplus
extern "C" {
#endif
/**
 * @addtogroup CAPI2_PBKAPIOLDGroup
 * @{
 */


//**************************************************************************************
/**
	Request to find a alpha name within a phonebook.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		number (in) pointer to phone number 
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_PBK_GETALPHA_RSP
	@n@b Result_t : 
	@n@b ResultData : PBK_API_Name_t
**/
void CAPI2_PBK_GetAlpha(UInt32 tid, UInt8 clientID, char* number);

//**************************************************************************************
/**
	This function returns TRUE if the passed ASCII-coded phone number is one of the 
	emergency numbers which consist of 

	1. When SIM/USIM is present: 
	   The emergency numbers read from EF-ECC in SIM/USIM plus "112" and "911". 
 
	2. When SIM/USIM is not present:
 	   Number "000", "08", "112", "110", "118", "119", "911" and "999". 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		*phone_number (in) pointer to phone number to be checked
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_PBK_ISEMERGENCYCALLNUMBER_RSP
	@n@b Result_t : 
	@n@b ResultData : Boolean.  TRUE if the call is an emergency call	
**/

void CAPI2_PBK_IsEmergencyCallNumber(UInt32 tid, UInt8 clientID, char *phone_number);

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
   
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		*phone_number (in) pointer to phone number to be checked
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_RSP
	@n@b Result_t : 
	@n@b ResultData : Boolean.  TRUE if the number is a partial match			
**/

void CAPI2_PBK_IsPartialEmergencyCallNumber(UInt32 tid, UInt8 clientID, char *phone_number);

//**************************************************************************************
/**
	This function requests the information of a phonebook type, including alpha size, digit size,
	number of total entries, number of free entries, the index of the first free entry and the index
	of the first used entry.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	to identify response messages
	@param		pbk_id (in) Phone book ID
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_GET_PBK_INFO_RSP
	@n@b Result_t : 
	@n@b ResultData : ::PBK_INFO_RSP_t

**/

void CAPI2_PBK_SendInfoReq( UInt32 tid, UInt8 clientID, PBK_Id_t pbk_id );


//**************************************************************************************
/**
	This functions requests all the phonebook entries that match the passed alpha pattern
	(matching is case-insensitive for GSM Default Alphabet encoding).

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	to identify response messages
	@param		pbk_id (in) Phone book ID
	@param		alpha_coding (in) Type of coding employed(GSM/UCS..)
	@param		alpha_size (in) number of bytes passed in "alpha".
	@param		*alpha (in) pointer to actual phonebook alpha data
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_PBK_ENTRY_DATA_RSP
	@n@b Result_t : 
	@n@b ResultData : ::PBK_ENTRY_DATA_RSP_t
**/

void CAPI2_PBK_SendFindAlphaMatchMultipleReq( UInt32 tid, UInt8 clientID, PBK_Id_t pbk_id, 
	ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha );


//**************************************************************************************
/**
	This function requests the first phonebook entry that matches the passed alpha pattern
	(matching is case-insensitive for GSM default alphabet encoding). The matching is performed
	in the order of the passed phonebook types "pbkId".

  	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	to identify response messages
	@param		numOfPbk (in) number of phonebook types pointed by pbkId.
	@param		*pbkId (in) Phone book ID
	@param		alpha_coding (in) Type of coding employed(GSM/UCS..)
	@param		alpha_size (in) number of bytes passed in "alpha".
	@param		*alpha (in) pointer to actual phonebook alpha data
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_PBK_ENTRY_DATA_RSP
	@n@b Result_t : 
	@n@b ResultData : ::PBK_ENTRY_DATA_RSP_t
**/

void CAPI2_PBK_SendFindAlphaMatchOneReq( UInt32 tid, UInt8 clientID, UInt8 numOfPbk, PBK_Id_t *pbkId, 
	ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha );

//**************************************************************************************
/**
	Function to check if the phone book is ready for operation
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	to identify response messages
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_PBK_ISREADY_RSP
	@n@b Result_t : 
	@n@b ResultData : Boolean.  TRUE if ready	 
**/
void CAPI2_PBK_IsReady(UInt32 tid, UInt8 clientID);


//**************************************************************************************
/**
	This function requests the phonebook entries whose indices are in the range specified by the
	"startIndex" and "endIndex" arguments.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	to identify response messages
	@param		pbk_id (in) Phone book ID
	@param		start_index (in) Index to start reading from
	@param		end_index (in) Last index to be read from
	@note
	The phonebook task returns the phonebook entries in the ::MSG_PBK_ENTRY_DATA_RSP
	messages by calling the passed callback function.
	@n@b Responses 
	@n@b MsgType_t : ::MSG_PBK_ENTRY_DATA_RSP
	@n@b Result_t : 
	@n@b ResultData : ::PBK_ENTRY_DATA_RSP_t
**/


void CAPI2_PBK_SendReadEntryReq( UInt32 tid, UInt8 clientID, PBK_Id_t pbk_id, 
					UInt16 start_index, UInt16 end_index );


//**************************************************************************************
/**
	This function requests to update a phonebook entry.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	to identify response messages
	@param		pbk_id (in) Phone book ID
	@param		special_fax_num (in) set to TRUE for writing the Fax number in MSISDN phonebook.
	@param		index (in) Index to be written to
	
	@param		type_of_number (in) type of number: TON/NPI combined: see Section 10.5.4.7 of 3GPP 24.008. Typically 
				one of UNKNOWN_TON_UNKNOWN_NPI (128), UNKNOWN_TON_ISDN_NPI (129) & INTERNA_TON_ISDN_NPI (145), but it 
				can be another valid value defined in spec.

	@param		*number (in) Pointer to number to be written
	@param		alpha_coding (in) Type of coding employed(GSM/UCS..)
	@param		alpha_size (in) number of bytes passed in "alpha".
	@param		*alpha (in) pointer to actual phonebook alpha data

	@note
	If FIRST_PHONEBK_FREE_INDEX is passed in "index", the entry is written into the first free
	entry found (this is applicable only for PB_ME, PB_ADN and PB_FDN phonebooks).
	If NULL is passed in "number", the phonebook entry is deleted.
	@n@b Responses 
	@n@b MsgType_t : ::MSG_WRT_PBK_ENTRY_RSP
	@n@b Result_t : 
	@n@b ResultData : ::PBK_WRITE_ENTRY_RSP_t
**/

void CAPI2_PBK_SendWriteEntryReq( UInt32 tid, UInt8 clientID, PBK_Id_t pbk_id, Boolean special_fax_num, UInt16 index, UInt8 type_of_number, 
				char *number, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha );

//**************************************************************************************
/**
	This function requests to update a phonebook entry. It is different from CAPI2_PBK_SendWriteEntryReq()
	in that this function supports the extended 3G USIM ADN phonebook data (e.g. additional number and 
	email address). 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	to identify response messages
	@param		pbk_id (in) Phone book ID
	@param		special_fax_num (in) set to TRUE for writing the Fax number in MSISDN phonebook.
	@param		index (in) 0-based index to be written to
		
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
	
	@return		Result_t 
	@note
	The phonebook task returns the update result in the ::MSG_WRT_PBK_ENTRY_RSP message
	by calling the passed callback function.
	If FIRST_PHONEBK_FREE_INDEX is passed in "index", the entry is written into the first free
	entry found (this is applicable only for PB_ME, PB_ADN and PB_FDN phonebooks).
	If NULL is passed in "number", the phonebook entry is deleted.
**/
void CAPI2_PBK_SendUpdateEntryReq( UInt32 tid, UInt8 clientID, PBK_Id_t pbk_id, Boolean special_fax_num, UInt16 index, UInt8 type_of_number, 
	char *number, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha, 
	const USIM_PBK_EXT_DATA_t *usim_adn_ext_data);

//**************************************************************************************
/**
	This function requests to check whether the passed non-USSD number can be dialed according to the
	FDN setting and the FDN phonebook contents.	For USSD number, use PBK_IsUssdDiallable() API instead. 

  	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	to identify response messages
	@param		*number (in) Pointer to NULL terminated number to be checked. 

	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_CHK_PBK_DIALLED_NUM_RSP
	@n@b Result_t : 
	@n@b ResultData : ::PBK_CHK_NUM_DIALLABLE_RSP_t
**/
void CAPI2_PBK_SendIsNumDiallableReq(UInt32 tid, UInt8 clientID, char *number);

//**************************************************************************************
/**
	This function checks whether the passed non-USSD number is barred according to the
	BDN setting and the BDN phonebook contents.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	to identify response messages
	@param		number (in) Buffer that stores the NULL terminated number
	@param		is_voice_call (in) TRUE if the number is for voice call
	 
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_PBK_ISNUMBARRED_RSP
	@n@b Result_t : 
	@n@b ResultData : Boolean. TRUE if number is barred; FALSE otherwise. 
**/
void CAPI2_PBK_IsNumBarred(UInt32 tid, UInt8 clientID, const char *number, Boolean is_voice_call);

//**************************************************************************************
/**
	This function checks whether the passed USSD number can be dialed according to the
	FDN setting and the FDN phonebook contents.	
	
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	to identify response messages
	@param		data (in) Buffer that stores the USSD number
	@param		dcs (in) USSD DCS byte in CB DCS format
	@param		len (in) Number of bytes passed in "data". 
	
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_PBK_ISUSSDDIALLABLE_RSP
	@n@b Result_t : 
	@n@b ResultData : Boolean.  TRUE if USSD is diallable; FALSE otherwise.
**/
void CAPI2_PBK_IsUssdDiallable(UInt32 tid, UInt8 clientID, const UInt8 *data, UInt8 dcs, UInt8 len);

//**************************************************************************************
/**
	This function enables/disables the FDN check in phonebook. 

    By default FDN check in phonebook is on; It can be disabled by client for valid reasons such as:
	
	1. Client handles the STK Setup Call; STK Send SS/USSD; STK Send SMS and thus we must
	   disable FDN check when a call is set up or when SMS is sent because the request originates
	   from SIM and according to GSM 11.14 those requests must not be checked against FDN phonebook. 

    2. MMI Client already performs FDN check, there is no need to do it again. 
	
	When FDN check in phonebook is disabled, PBK_SendIsNumDiallableReq, PBK_IsNumDiallable() & 
	PBK_IsUssdDiallable() always return TRUE to indicate FDN check is passed. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	to identify response messages
	@param		fdn_chk_on (in) TRUE to enable FDN check in phonebook; FALSE to disable it. 

	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_PBK_SET_FDN_CHECK_RSP
	@n@b Result_t : 
	@n@b ResultData : None
**/
void CAPI2_PBK_SetFdnCheck(UInt32 tid, UInt8 clientID, Boolean fdn_chk_on);


/**
	This function returns the FDN check enabled/disabled status in phonebook. 

    By default FDN check in phonebook is on; It can be disabled by client for valid reasons such as:
	
	1. Client handles the STK Setup Call; STK Send SS/USSD; STK Send SMS and thus we must
	   disable FDN check when a call is set up or when SMS is sent because the request originates
	   from SIM and according to GSM 11.14 those requests must not be checked against FDN phonebook. 

    2. MMI Client already performs FDN check, there is no need to do it again. 
	
	When FDN check in phonebook is disabled, PBK_SendIsNumDiallableReq, PBK_IsNumDiallable() & 
	PBK_IsUssdDiallable() always return TRUE to indicate FDN check is passed. 
	
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	to identify response messages

	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_PBK_GET_FDN_CHECK_RSP
	@n@b Result_t : 
	@n@b ResultData : Boolean TRUE if FDN check is enabled; FALSE otherwise. 
	@note
**/
void CAPI2_PBK_GetFdnCheck(UInt32 tid, UInt8 clientID);

//**************************************************************************************
/**
	This function sends the request to read the USIM PBK Hidden key information.

	The hidden key shall be 4-8 digits according to Section 4.2.42 of 4GPP 31.102. 
	It is applicable for USIM only and it needs to be verified for an ADN entry
	whose status is marked as "hidden" before a user can access the entry. See "is_hidden" 
	element in "USIM_PBK_EXT_DATA_t" structure returned for an ADN entry in 3G USIM. 

	The client shall verify that the hidden key is provisioned in USIM before calling
	this function. The hidden key provision status is provided in "hidden_key_exist" 
	element in USIM_PBK_INFO_t structure in the response for PBK_SendInfoReq() function. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
  	@param		clientID (in) Client ID	to identify response messages
	@n@b Responses 
	@n@b MsgType_t : ::MSG_READ_USIM_PBK_HDK_ENTRY_RSP
	@n@b Result_t : 
	@n@b ResultData : ::USIM_PBK_READ_HDK_ENTRY_RSP_t
	
**/
void CAPI2_PBK_SendUsimHdkReadReq( UInt32 tid, UInt8 clientID);

//**************************************************************************************
/**	This function sends the request to update the USIM PBK Hidden key information.
	
	The hidden key shall be 4-8 digits according to Section 4.2.42 of 4GPP 31.102. 
	It is applicable for USIM only and it needs to be verified for an ADN entry
	whose status is marked as "hidden" before a user can access the entry. 
	See "is_hidden" element in "USIM_PBK_EXT_DATA_t" structure returned for an ADN entry in 3G USIM. 

	The client shall verify that the hidden key is provisioned in USIM before calling
	this function. The hidden key provision status is provided in "hidden_key_exist" 
	element in USIM_PBK_INFO_t structure in the response for CAPI2_PBK_SendInfoReq() function. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
  	@param		clientID (in) Client ID	to identify response messages
    @param      hidden_key Hidden key information.
    @return		::RESULT_OK 
	@note		A ::MSG_WRITE_USIM_PBK_HDK_ENTRY_RSP message will be posted back
**/
void CAPI2_PBK_SendUsimHdkUpdateReq( UInt32 tid, UInt8 clientID, const HDKString_t *hidden_key);

//**************************************************************************************
/**
	This function sends the request to update the USIM PBK additional number alpha string 
	in EF-AAS. 

    This function is applicable for 3G USIM only and shall be called only if EF-AAS
	is provisioned in USIM. 

    The EF-AAS provision information can be obtained in the "aas_data" element in the 
	USIM_PBK_INFO_t structure in the response for CAPI2_PBK_SendInfoReq() function or 
	USIM_PBK_ALPHA_INFO_RSP_t structure in the response for PBK_SendUsimAasInfoReq()

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
  	@param		clientID (in) Client ID	to identify response messages
	@param		index (in) 0-based index
	@param		pbk_id (in) Phone book ID. "pbk_id" shall be one of "PB_3G_LOCAL" and "PB_3G_GLOBAL".
	@param		alpha_coding (in) Type of coding employed(GSM/UCS..)
	@param		alpha_size (in) number of bytes passed in "alpha"
	@param		*alpha (in) pointer to actual phonebook alpha data
	@return		::RESULT_OK 
	@note		A ::MSG_WRITE_USIM_PBK_ALPHA_ENTRY_RSP message will be posted back
**/
void CAPI2_PBK_SendUsimAasUpdateReq( UInt32 tid, UInt8 clientID, UInt16 index, PBK_Id_t pbk_id, ALPHA_CODING_t alpha_coding, 
								   UInt8 alpha_size, const UInt8 *alpha );

//**************************************************************************************
/**
	This function sends the request to update the USIM PBK grouping Information alpha string 
	in EF-GAS. It is applicable for 3G USIM only and shall be called only if EF-GAS
	is provisioned in USIM. 

    The EF-GAS provision information can be obtained in the "gas_data" element in the 
	USIM_PBK_INFO_t structure in the response for CAPI2_PBK_SendInfoReq() function or 
	USIM_PBK_ALPHA_INFO_RSP_t structure in the response for PBK_SendUsimGasInfoReq()

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
  	@param		clientID (in) Client ID	to identify response messages
	@param		index (in) 0-based index
	@param		pbk_id (in) Phone book ID. "pbk_id" shall be one of "PB_3G_LOCAL" and "PB_3G_GLOBAL".
	@param		alpha_coding (in) Type of coding employed(GSM/UCS..)
	@param		alpha_size (in) number of bytes passed in "alpha"
	@param		*alpha (in) pointer to actual phonebook alpha data
	@return		::RESULT_OK 
	@note		A ::MSG_WRITE_USIM_PBK_ALPHA_ENTRY_RSP message will be posted back
**/
void CAPI2_PBK_SendUsimGasUpdateReq( UInt32 tid, UInt8 clientID, UInt16 index, PBK_Id_t pbk_id, ALPHA_CODING_t alpha_coding, 
								   UInt8 alpha_size, const UInt8 *alpha );

//**************************************************************************************
/**
	This function sends the request to read the USIM PBK additional number alpha string 
	in EF-AAS. For example, the AT&T 3G USIM has the following 5 entries in EF-AAS:
	"Work"; "Home"; "Mobile"; "Fax"; "Other". These entries are shared among all 
	ADN entries in AT&T USIM. 

    This function is applicable for 3G USIM only and shall be called only if EF-AAS
	is provisioned in USIM. 

    The EF-AAS provision information can be obtained in the "aas_data" element in the 
	USIM_PBK_INFO_t structure in the response for CAPI2_PBK_SendInfoReq() function or 
	USIM_PBK_ALPHA_INFO_RSP_t structure in the response for CAPI2_PBK_SendUsimAasInfoReq()

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
  	@param		clientID (in) Client ID	to identify response messages
	@param		index (in) 0-based index
	@param		pbk_id (in) Phone book ID. "pbk_id" shall be one of "PB_3G_LOCAL" and "PB_3G_GLOBAL".
	@return		::RESULT_OK 
	@note		A ::MSG_READ_USIM_PBK_ALPHA_ENTRY_RSP message will be posted back
**/
void CAPI2_PBK_SendUsimAasReadReq( UInt32 tid, UInt8 clientID, UInt16 index, PBK_Id_t pbk_id);

//**************************************************************************************
/**
	This function sends the request to get the USIM PBK additional number alpha string 
	EF-AAS information, e.g. alpha string length and number of records

    The "aas_data" element in the USIM_PBK_INFO_t structure in the response for 
	CAPI2_PBK_SendInfoReq() function also provides the configuration information in addition to 
	the actual EF-GAS record contents. The client can choose to use CAPI2_PBK_SendInfoReq() function
	instead. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	to identify response messages
	@param		pbk_id (in) Phone book ID. "pbk_id" shall be one of "PB_3G_LOCAL" and "PB_3G_GLOBAL".
	@return		::RESULT_OK 
	@note		A ::MSG_GET_USIM_PBK_ALPHA_INFO_RSP message will be posted back
**/
void CAPI2_PBK_SendUsimAasInfoReq( UInt32 tid, UInt8 clientID, PBK_Id_t pbk_id);

//**************************************************************************************
/**
	This function sends the request to get the USIM PBK grouping alpha string 
	EF-GAS information, e.g. alpha string length and number of records

    This function is applicable for 3G USIM only. The "gas_data" element in the 
	USIM_PBK_INFO_t structure in the response for CAPI2_PBK_SendInfoReq() function
	also provides the configuration information in addition to the actual
	EF-GAS record contents. The client can choose to use CAPI2_PBK_SendInfoReq() function
	instead. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	to identify response messages
	@param		pbk_id (in) Phone book ID. "pbk_id" shall be one of "PB_3G_LOCAL" and "PB_3G_GLOBAL".
	@return		::RESULT_OK 
	@note		A ::MSG_GET_USIM_PBK_ALPHA_INFO_RSP message will be posted back
**/
void CAPI2_PBK_SendUsimGasInfoReq( UInt32 tid, UInt8 clientID, PBK_Id_t pbk_id);

//**************************************************************************************
/**
	This function sends the request to read the USIM PBK grouping alpha string in 
	EF-GAS. For example, the AT&T 3G USIM has the following 5 entries in EF-GAS:
	"Business"; "Family"; "Friends"; "Services"; "Other". These entries are shared 
	among all ADN entries in AT&T USIM. 

    This function is applicable for 3G USIM only. 

    The EF-GAS provision and/or data information can be obtained in the "gas_data" element in the 
	USIM_PBK_INFO_t structure in the response for CAPI2_PBK_SendInfoReq() function or 
	USIM_PBK_ALPHA_INFO_RSP_t structure in the response for PBK_SendUsimGasInfoReq(). 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	to identify response messages
	@param		index (in) 0-based index
	@param		pbk_id (in) Phone book ID. "pbk_id" shall be one of "PB_3G_LOCAL" and "PB_3G_GLOBAL".
	@return		::RESULT_OK 
	@note		A ::MSG_READ_USIM_PBK_ALPHA_ENTRY_RSP message will be posted back
**/
void CAPI2_PBK_SendUsimGasReadReq( UInt32 tid, UInt8 clientID, UInt16 index, PBK_Id_t pbk_id);


/** @} */

#ifdef __cplusplus
}
#endif

#endif

