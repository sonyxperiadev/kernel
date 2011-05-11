//***************************************************************************
//
//	Copyright © 2003-2008 Broadcom Corporation
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
*   @file   phonebk_stub.h
*
*   @brief  This file contains definitions for phonebook stub functions.
*
****************************************************************************/

#ifndef _PHONEBK_STUB_H_
#define _PHONEBK_STUB_H_


//******************************************************************************
//	 			include block
//******************************************************************************


//******************************************************************************
//						Global Varilable Declaration
//******************************************************************************
#define MAX_NUMBER_OF_PBK_ENTRY 1000	///< Maximum number of phonebook entries

/* If USE_PBK_STUB_FUNC is defined, bypass the platform phonebook implementation 
 * and use the phonebook stub functions defined by customers in stubs/src/phonebk_stubs.c.
 */
//#define USE_PBK_STUB_FUNC


//******************************************************************************
//					           Type Definitions
//******************************************************************************

/* Phonebook information */
typedef struct
{
	UInt8 mx_alpha_size;	///< Max number of bytes for alpha data
	UInt8 mx_digit_size;	///< Max number of digits
	UInt16 total_entries;	///< Total number of phonebook records
	UInt16 free_entries;	///< Number of free entries in phonebook records
	UInt16 first_free_entry;	///< 0-based index of the first empty phonebook entry
	UInt16 first_used_entry;	///< 0-based index of the first filled phonebook entry
} PBK_STUB_INFO_t;

typedef struct
{
	UInt16 number_of_entry;					///< Number of entries in "index"
	UInt16 index[MAX_NUMBER_OF_PBK_ENTRY];	///< Array of phonebook list
} PBK_STUB_ENTRY_LIST_t;

typedef enum
{
	PBK_SUCCESS,			///< Success in accessing phonebook 
	PBK_NOT_READY,			///< PBK is still being initialized, not ready yet 
	PBK_NOT_SUPPORTED,		///< The requested phonebook type is not supported 
	PBK_NO_ENTRY_FOUND,		///< No appropriate phonebook entry is found 
	PBK_INVALID_INDEX		///< The requested index is invalid 
} PBK_STUB_Result_t;


//******************************************************************************
//						Macro Definition
//******************************************************************************


//******************************************************************************
//						Function Prototype
//******************************************************************************

//**************************************************************************************
/**
    This function returns the configuration information for a phonebook type. 

    @param pbk_id (in) Specify phonebook type
	@param pbk_info (out) Buffer to store the returned phonebook configuration information
	
	@return	Phonebook access result
	@note
**/
PBK_STUB_Result_t PBK_STUB_GetInfo(PBK_Id_t pbk_id, PBK_STUB_INFO_t *pbk_info);


//**************************************************************************************
/**
    This function returns the alpha data of a phonebook record that matches the passed
	phone number. 

    @param pbk_id (in) Specify phonebook type
	@param number (in) NULL terminated phone number in ASCII format
	@param alpha (out) Buffer to store the returned alpha data
	
	@return	Phonebook access result
	@note
**/
PBK_STUB_Result_t PBK_STUB_GetAlphaForMatchedNumber( PBK_Id_t pbk_id, char *number, 
													 PBK_API_Name_t *alpha );


//**************************************************************************************
/**
    This function returns the list of indices of entries that match the passed phonebook
	alpha data.

    @param pbk_id (in) Specify phonebook type
	@param alpha (in) Phonebook alpha data to be matched
	@param entry_list (out) Buffer to store the returned matching 0-based entry indices

	@return	Phonebook access result
	@note
**/
PBK_STUB_Result_t PBK_STUB_GetMatchedAlphaIndexList( PBK_Id_t pbk_id, 
					PBK_API_Name_t *alpha, PBK_STUB_ENTRY_LIST_t *entry_list );


//**************************************************************************************
/**
    This function returns the first index of the phonebook record that matches the passed
	alpha data. 

    @param pbk_id (in) Specify phonebook type
	@param alpha (in) Phonebook alpha data to be matched
	@param index (out) To return the 0-based phonebook index
	
	@return	Phonebook access result
	@note
**/
PBK_STUB_Result_t PBK_STUB_GetMatchedAlphaFirstIndex( PBK_Id_t pbk_id, 
										PBK_API_Name_t *alpha, UInt16 *index );


//**************************************************************************************
/**
    This function returns the contents of a phonebook record

    @param pbk_id (in) Specify phonebook type
	@param index (in) 0-based phonebook index
	@param pbk_rec (out) Buffer to store the returned phonebook contents
	
	@return	Phonebook access result
	@note
**/
PBK_STUB_Result_t PBK_STUB_GetEntryContent( PBK_Id_t pbk_id, UInt16 index, 
										    PBK_Record_t *pbk_rec );


//**************************************************************************************
/**
    This function sends the request to update a phonebook record. A MSG_WRT_PBK_ENTRY_RSP
	message needs to be posted back to the passed callback function.

	@param *inClientInfoPtr (in) ClientInfo_t pointer
	@param pbk_id (in) Specify phonebook type
	@param pbk_rec (in) Phonebook contents to be updated
	@param pbk_access_cb (in) Callback function to post the MSG_WRT_PBK_ENTRY_RSP message
	
	@return	Phonebook access result
	@note
**/
PBK_STUB_Result_t PBK_STUB_SendWriteEntryReq( ClientInfo_t* inClientInfoPtr, PBK_Id_t pbk_id, 
						PBK_Record_t *pbk_rec, CallbackFunc_t* pbk_access_cb );


//**************************************************************************************
/**
    This function whether the passed phone number is in the FDN phonebook.

    @param number (in) NULL terminated phone number in ASCII format to be checked
	@param is_voice_call (in) TRUE if the number is for voice call; FALSE otherwise
	
	@return	TRUE the passed number is in FDN list; FALSE otherwise.
	@note
**/
Boolean	PBK_STUB_IsFdnNumber(const char *number, Boolean is_voice_call);


//**************************************************************************************
/**
    This function checks whether the passed USSD number can be dialled according to FDN setting.

    @param data (in) USSD data to be checked
	@param dcs (in) Data Coding Scheme byte in CB DCS format
	@param len (in) Number of bytes in "data" buffer
	
	@return	TRUE USSD data is diallable; FALSE otherwise
	@note
**/
Boolean PBK_STUB_IsUssdDiallable(const UInt8 *data, UInt8 dcs, UInt8 len);


//**************************************************************************************
/**
    This function checks whether the passed phone number is Emergency Call number.

    @param number (in) NULL terminated phone number in ASCII format
	
	@return	TRUE if the passed phone number is Emergency Call number; FALSE otherwise
	@note
**/
Boolean PBK_STUB_IsEmergencyNumber(const char *number);


//**************************************************************************************
/**
    This function returns the additional Emergency Call numbers in addition to 112, 911 and
	those specified in SIM. 

    @param ecc_num (out) Buffer to store the additional Emergency Call number list
	@param sim_present (in) TRUE if SIM is inserted; FALSE otherwise
	
	@return	Number of additional Emeregency Call numbers returned
	@note
**/
UInt8 PBK_STUB_GetAdditionalEcc(const ECC_NUM_t **ecc_num, Boolean sim_present);

#endif

