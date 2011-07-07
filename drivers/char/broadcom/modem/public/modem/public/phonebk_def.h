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
*   @file   phonebk_def.h
*
*   @brief  This file contains definitions for phonebook API data types.
*
****************************************************************************/

#ifndef _PHONEBK_DEF_H_
#define _PHONEBK_DEF_H_


#define MAX_PBK_ENTRY_READ 30 ///< Number of maximum phonebook entries allowed in the MSG_READ_PBK_ENTRY_REQ read request

#define PBK_NAME	40	///< Phonebook name length

#define MAX_NUM_OF_TYPE1_ANR_FILE 6	///< Maximum number of type1 EF-ANR files supported per EF-PBR record 

#define MAX_NUM_OF_GROUP 10	///< Maximum number of groups an ADN can belong to

#define MAX_NUM_OF_TYPE1_EMAIL_FILE 6 ///< Maximum number of type1 EF-EMAIL files supported per EF-PBR record 

///< Maximum number of type2 EF-ANR files supported per EF-PBR record 
#define MAX_NUM_OF_TYPE2_ANR_FILE 6

///< Maximum number of type2 EF-EMAIL files supported per EF-PBR record 
#define MAX_NUM_OF_TYPE2_EMAIL_FILE 6

/// Maximum number of second name (type1 and type2)
#define MAX_NUM_OF_SECOND_NAME 2

/// Maximum number of type2 files (email*6, sne*1, and anr*1)
#define MAX_NUM_OF_TYPE2_FILES 13


#define MAX_NUM_OF_ADN_SET 	6 ///< Maximum Number of ADN PBK set which is equal to number of non-empty EF-PBR records

#define MAX_NUM_OF_ALPHA_ENTRY	30	///< Maximum number of EF-AAS/EF-GAS records for each EF-PBR entry

#define 	HDK_MIN_LENGTH	4	///< Min. number of digits in hidden key
#define 	HDK_MAX_LENGTH	8	///< Max. number of digits in hidden key

#define LAST_PHONEBK_INDEX 0x7FFE ///< Get all phonebook data until last index

#define FIRST_PHONEBK_FREE_INDEX 0x7FFF ///< Write to the first empty phonebook slot

#define ECC_REC_LIST_SIZE  18 ///< Maximum size for ECC Phonebook list with SIM

typedef UInt8 ECC_NUM_t[ECC_ENTRY_SIZE]; ///< Emergency Call Code: BCD encoded

/// The source of ECC number 
typedef enum
{
    ECC_SRC_MS,             ///< ECC source is MS
    ECC_SRC_SIM,            ///< ECC source is SIM
    ECC_SRC_NW,             ///< ECC source is network
    ECC_SRC_UNKNOWN = 0xFF  ///< ECC source is unknown
}ECC_SRC_t;


/// ECC record struct
typedef struct
{
  ECC_NUM_t ecc_num;            ///< ECC number
  UInt8 emergency_svc_category; ///< Emergency serivce categry
  ECC_SRC_t ecc_src;            ///< ECC source
} ECC_REC_t;


typedef ECC_REC_t ECC_REC_LIST_t[ECC_REC_LIST_SIZE]; ///< ECC list

/// phonebook ID
typedef	enum {
	PB_ME,			///< ME Phonebook
	PB_FDN,			///< fixdialling-phonebook
	PB_ADN,			///< Abreviated dialing-phonebook for 2G.
	PB_EN,     		///< SIM Emergency Call Code phonebook
	PB_SDN,			///< service dialing number phone book,
	PB_LND,			///< Last Dialled Bumber
	PB_MSISDN,		///< Mobile Own Phone Number
   	PB_BDN,    		///< Barred Dialling Numbers
	PB_3G_GLOBAL,	///< 3G Global Phonebook
	PB_3G_LOCAL,	///< 3G Local Phonebook
	PB_INVALID_TYPE	///< Invalid phonebook type: equal to the number of phonebook type
} PBK_Id_t;


typedef enum
{
	USIM_PBK_AAS,	///< EF-AAS
	USIM_PBK_GAS	///< EF-GAS
} USIM_PBK_ALPHA_TYPE_t;


/**
Preferred 3G Phonebook selection
**/
typedef enum
{
	PREF_GLOBAL_USIM_PBK,	///< Support global phonebook under MF/DF-Telecom only if it exists, otherwise support local phonebook only 
	PREF_LOCAL_USIM_PBK,	///< Support local phonebook under MF/USIM-ADF only if it exists, otherwise support global phonebook only
	PREF_USIM_PBK_BOTH		///< Support both local and global phonebooks: default
} USIM_PBK_PREFERRED_t;


/// phonebook state
typedef enum
{
	PBKSTATE_INIT,					///< Initial state of Phonebook
	PBKSTATE_INITIALIZING_PBK,		///< Initializiing Phonebook database
	PBKSTATE_READY					///< Phonebook database is ready for use
} PBK_State_t;


/** Sequence 1.4 of TC 27.22.4.11 (STK Send SS) sends a SS string of 48 bytes:
 * "**21*+01234567890123456789012345678901234567*11#". Thus we need a length
 * longer than that. 
 */
typedef UInt8 PBK_Dialled_Num_t[70]; ///< Array of the dialed number


/// Phonebook Write Entry Inidication
typedef struct
{
	PBK_Id_t pbk_id;	///< Phonebook type
	UInt16 index;		/// 0-based index
} PBK_WRITE_ENTRY_IND_t;


/// Response to number dialable request. ::MSG_PBK_ISNUMDIALABLE_RSP.
typedef struct
{
	PBK_Dialled_Num_t dialled_num;	///< Number to be dialled
	Boolean diallable;				///< TRUE if number can be dialled
} PBK_CHK_NUM_DIALLABLE_RSP_t;


typedef UInt8	PBK_Name_t[PBK_NAME];	///< Alpha associated with name

typedef enum
{
	USIM_PBK_HDK_SUCCESS,	///< USIM PBK Hidden Key result in EF-AAS and EF-GAS is success
	USIM_PBK_HDK_FAIL		///< USIM PBK Alpha result in EF-AAS and EF-GAS is failure
} USIM_PBK_HDK_RESULT_t;

typedef enum
{
	USIM_PBK_ALPHA_SUCCESS,	///< USIM PBK Alpha result in EF-AAS and EF-GAS is success
	USIM_PBK_ALPHA_FAIL		///< USIM PBK Alpha result in EF-AAS and EF-GAS is failure
} USIM_PBK_ALPHA_RESULT_t;


/// Phonebook entry data result
typedef enum
{
	PBK_ENTRY_VALID_NOT_LAST,	///< Indicate the entry data is valid and more phonebook data to follow 
	PBK_ENTRY_VALID_IS_LAST,	///< Indicate the entry data is valid and the entry is last entry 
	PBK_ENTRY_INVALID_IS_LAST,	///< Indicate the entry data is not valid and the entry is last entry 
	PBK_SIM_BUSY,				///< Entry data is invalid as SIM is busy 
	PBK_ENTRY_INDEX_INVALID,	///< Requested index is invalid 
	PBK_ENTRY_NOT_ACCESSIBLE	///< PBK not accessible, e.g. SIM Serivce Table disables specific PBK 
} PBK_ENTRY_DATA_RESULT_t;


/// Phonebook API name struct
typedef struct
{
	ALPHA_CODING_t alpha_coding;	///< GSM Alphabet, UCS2_80, UCS2_81 or UCS2_82 
	UInt8 alpha_size;				///< Number of bytes used in "pbk_name" 
	UInt8 alpha[sizeof(PBK_Name_t) * 2]; ///< Phonebook name data 
} PBK_API_Name_t;


#define NUM_OF_PBK_TYPE PB_INVALID_TYPE


// This implementation of Phonebook is limited to 20 digits for the Phonebook entries.
#define 	PBK_DIGITS		(SIM_PBK_ASCII_DIGIT_SZ + 1)	///< Number length, limit this implementation
															///< of phonebook to 20 digit telephone numbers
															///< plus one for '+'

/* We support one level of EF-EXT1, EF-EXT2 extension, i.e. chaining inside 
 * EF-EXT1 and EF-EXT2 is not supported. 
 */
typedef UInt8 PBK_Digits_t[PBK_DIGITS + SIM_PBK_EXT_ASCII_DIGIT_SIZE + 1];	///< Null terminated phone number in ASCII format


typedef struct
{
	PBK_Digits_t	addi_num;	///< Additional numbers (ASCII Null Terminated) including EF-EXT1 extension
	UInt8			ton;		///< Type of Number + Number Plan (One of UNKNOWN_TON_UNKNOWN_NPI, UNKNOWN_TON_ISDN_NPI, INTERNA_TON_ISDN_NPI)
	PBK_API_Name_t	addi_alpha;	///< Alpha for the additional numbers
} PBK_EXT_Number_t;


/// Extended ADN phonebook data that exists only in 3G USIM
typedef struct
{
	UInt8					num_of_addi_num;							///< Number of entries in "addi_num" 
	PBK_EXT_Number_t		addi_num[MAX_NUM_OF_TYPE1_ANR_FILE + MAX_NUM_OF_TYPE2_ANR_FILE]; 
	                                                                    ///< Additional Number (EF-ANR) and Names (EF-AAS)
#undef PBKAPI_SUPPORT_TYPE2
#ifdef PBKAPI_SUPPORT_TYPE2
    UInt8                   num_of_sname;                               ///< Number of entries in "second name"
	PBK_API_Name_t			second_name[MAX_NUM_OF_SECOND_NAME];		///< Second name (EF-SNE)
#else
	PBK_API_Name_t			second_name;								///< Second name (EF-SNE)
#endif
		
	UInt8					num_of_group_name;							///< Number of group names in "group_name"
	PBK_API_Name_t			group_name[MAX_NUM_OF_GROUP];				///< Group name (EF-PBC & EF-GAS)
	
	UInt8					num_of_email;								///< Number of entries in "email"
	PBK_API_Name_t			email[MAX_NUM_OF_TYPE1_EMAIL_FILE + MAX_NUM_OF_TYPE2_EMAIL_FILE];				
																		///< Email alpha (EF-EMAIL)
	Boolean					skip_update_hidden;	///< TRUE to skip updating hidden status (e.g. "is_hidden" is ignored)
	Boolean					is_hidden;	///< TRUE if ADN entry is hidden (need to verify Hidden Key before entry can be accessed user)

} USIM_PBK_EXT_DATA_t;

/// Phonebook Record
typedef	struct{
	PBK_API_Name_t	alpha_data;		///< fFor saving name
	PBK_Digits_t	number;			///< or storing number (ASCII Null Terminated)
	gsm_TON_t		ton;			///< Type of Number
	gsm_NPI_t		npi;			///< Number Plan Identifier
	UInt16			location;		///< record number in phone book
} PBK_Record_t;


typedef UInt8 	HDKString_t[HDK_MAX_LENGTH + 1]; ///< Hidden Key string


typedef struct
{
    UInt8 num_of_adn_set;                           ///< Number of non-empty EF-PBR records
    UInt8 num_of_adn_entry[MAX_NUM_OF_ADN_SET];     ///< Number of ADN records in each non-empty EF-PBR records
} USIM_PBK_ADN_SET_t;


typedef struct
{
	UInt16 total_num_of_rec;					///< Total number of EF-AAS/EF-GAS records in USIM (total number of multiple phonebook sets if
												///< multiple non-empty EF-PBR records exist in USIM)

	UInt8 num_of_alpha_set;						///< Number of EF-AAS/EF-GAS sets (same value as number of non-empty EF-PBR records)
	UInt8 num_of_rec[MAX_NUM_OF_ADN_SET];		///< Number of records in each EF-AAS/EF-GAS set
	UInt8 rec_len[MAX_NUM_OF_ADN_SET];			///< Record length in each EF-AAS/EF-GAS set

	Boolean same_file_shared_amoung_set;		///< If TRUE, this flag indicates the same EF-AAS/EF-GAS file is shared among the 
												///< multiple EF-PBR records (i.e. "num_of_alpha_set" > 1). In this case the 
												///< client can just update the EF-AAS/EF-GAS records in the first set and it will be  
												///< applicable for all phonebook sets
} USIM_PBK_ALPHA_INFO_t;


/// EF-AAS (alpha for additional numbers) and EF-GAS (grouping information alpha) configuration information and record contents
typedef struct
{
	USIM_PBK_ALPHA_INFO_t alpha_info;	///< EF-AAS/EF-GAS configuration information
	PBK_API_Name_t alpha_data[MAX_NUM_OF_ADN_SET][MAX_NUM_OF_ALPHA_ENTRY];	///< EF-AAS/EF-GAS contents
} USIM_PBK_ALPHA_DATA_t;


/// Extended 3G phonebook configuration informatioin (applicable for 3G USIM only)
typedef struct
{	
	USIM_PBK_ADN_SET_t adn_set;		///< How many EF-ADN sets in USIM and how many EF-ADN records in each set
	
	UInt8 num_of_anr_entry[MAX_NUM_OF_ADN_SET];		///< Number of additional numbers in EF-ANR for each ADN entry (e.g. AT&T USIM has two)
	UInt8 num_of_anr_digit[MAX_NUM_OF_ADN_SET];		///< Number of digits in each additional number entry (40 digits if EF-EXT1 has empty entries, otherwise up to 20 digits)

	UInt8 num_of_sne_entry[MAX_NUM_OF_ADN_SET];	    ///< Number of second names in EF-SNE for each ADN entry (maximum two)
	UInt8 sne_len[MAX_NUM_OF_ADN_SET];	            ///< Number of bytes allowed in Second Name EF-SNE. 0 if Second Name not provisioned in USIM

	UInt8 num_of_email_entry[MAX_NUM_OF_ADN_SET]; ///< Number of EF-EMAIL's per ADN entry
	UInt8 email_len[MAX_NUM_OF_ADN_SET];	///< Number of bytes allowed in EF-EMAIL

	UInt8 num_of_group[MAX_NUM_OF_ADN_SET];	///< Maximum number of groups an ADN entry can belong to (e.g. a number can belong to both "Work" and "Fax" in AT&T USIM)

	USIM_PBK_ALPHA_DATA_t aas_data;	///< EF-AAS configuration information and EF-AAS contents. For example, At&T USIM has the following 
									///< 5 EF-AAS strings: "Work"; "Home"; "Mobile"; "Fax"; "Other".

	USIM_PBK_ALPHA_DATA_t gas_data;	///< EF-GAS configuration information and EF-AAS contents. For example, At&T USIM has the following 5 EF-GAS strings:
									///< "Business"; "Family"; "Friends"; "Services"; "Other".

	Boolean hidden_key_exist;			///< TRUE if hidden key provisioned in USIM
	HDKString_t hidden_key;				///< Hidden key (null terminated): applicable only if "hidden_key_exist" is TRUE

} USIM_PBK_INFO_t;


/// Phonebook Information response. Refer ::MSG_GET_PBK_INFO_RSP.
typedef struct
{	
	PBK_Id_t pbk_id;			///< Phonebook ID
	Boolean result;				///< Boolean result
	UInt8 mx_alpha_size;		///< Maximum size of alpha
	UInt8 mx_digit_size;		///< Maximum size of digit
	UInt16 total_entries;		///< Total entries in the phonebook
	UInt16 free_entries;		///< Total free entries
	UInt16 first_free_entry;	///< location of the first free entry
	UInt16 first_used_entry;	///< location of the first used entry
	Boolean usim_adn_info_exist;	///< TRUE for USIM ADN PBK; FALSE otherwise
	USIM_PBK_INFO_t usim_adn_info;	///< USIM ADN PBK information (not relevant for 2G SIM)
} PBK_INFO_RSP_t;


/// Phonebook Entry data response. Refer ::MSG_PBK_ENTRY_DATA_RSP
typedef struct
{
	PBK_Id_t pbk_id;						///< Phonebook type
	PBK_ENTRY_DATA_RESULT_t data_result;	///< phonebook result
	UInt16 total_entries;					///< Total number of entries 
	PBK_Record_t pbk_rec;					///< Phonebook record data

	Boolean usim_adn_ext_exist;			///< TRUE if extended 3G ADN phonebook data exisits
	USIM_PBK_EXT_DATA_t usim_adn_ext;	///< Extended 3G ADN phonebook data

} PBK_ENTRY_DATA_RSP_t;


/// Phonebook write result. ::MSG_WRT_PBK_ENTRY_RSP
typedef enum
{
	PBK_WRITE_SUCCESS,				///< Phonebook entry write succeeded 
	PBK_WRITE_FDN_PIN2_REQUIRED,	///< Error: PIN2 needs to be entered before writing Fixed Dialing phonebook 
	PBK_WRITE_REJECTED,				///< Error: SIM rejects phonebook update, pbk not updatable, SIM busy etc 
	PBK_WRITE_INDEX_INVALID,		///< Error: phonebook entry index is not valid 
	PBK_WRITE_DATA_INVALID,			///< Error: phonebook data to be written is invalid 
	PBK_WRITE_NUMBER_TOO_LONG,		///< Error: the number of digits exceeds maximum allowed 
	PBK_WRITE_FDN_PUK2_REQUIRED,	///< Error: PIN2 needs to be unblocked before writing Fixed Dialing phonebook 
    PBK_WRITE_NON_FREE_ENTRY,       ///< Error: phonebook is full

	/* 3G USIM extended ADN phonebook data write error */
	PBK_USIM_ANR_NOT_EXIST,			///< Error: additional number not provisioned in USIM
	PBK_USIM_ANR_EXCEEDED,			///< Error: number of additional number exceeds that provisioned in USIM
	PBK_USIM_ANR_TOO_LONG,			///< Error: additional number digits too long
	PBK_USIM_AAS_NOT_EXIST,			///< Error: additional name corresponding to additional number does not exist in USIM
	PBK_USIM_AAS_NOT_MATCH,			///< Error: additional name corresponding to additional number does not match an entry in EF-AAS
	PBK_USIM_SNE_NOT_EXIST,			///< Error: second name is not provisioned in USIM
	PBK_USIM_SNE_TOO_LONG,			///< Error: second name string too long
	PBK_USIM_SNE_EXCEEDED,			///< Error: number of second name exceeds that provisioned in USIM
	PBK_USIM_GAS_NOT_EXIST,			///< Error: group name does not exist in USIM
	PBK_USIM_GAS_EXCEEDED,			///< Error: number of group names exceeds that provisioned in USIM
	PBK_USIM_GAS_NOT_MATCH,			///< Error: group name does not match an entry in EF-GAS
	PBK_USIM_EMAIL_NOT_EXIST,		///< Error: email not provisioned in USIM
	PBK_USIM_EMAIL_EXCEEDED,		///< Error: number of emails exceeds that provisioned in USIM
	PBK_USIM_EMAIL_TOO_LONG,		///< Error: email string too long
	PBK_USIM_HIDDEN_ENTRY_NOT_SUPPORTED	///< Error: hidden ADN entry not supported in USIM
} PBK_WRITE_RESULT_t;


/// Phonebook write entry response
typedef struct
{
	PBK_Id_t pbk_id;					///< Phonebook ID
	PBK_WRITE_RESULT_t write_result;	///< Result of phonebook write
	UInt16 index;						///< Phonebook entry index
} PBK_WRITE_ENTRY_RSP_t;


/// Phonebook Info Array
typedef PBK_INFO_RSP_t PBK_ALL_INFO_RSP_t[NUM_OF_PBK_TYPE - 1]; ///< Configuration for all phonebook types


typedef struct
{
	USIM_PBK_HDK_RESULT_t result;	///< Result for reading 3G phonebook hidden key
	HDKString_t hidden_key;			///< 3G phonebook hidden key: applicable only if "result" indicate success
} USIM_PBK_READ_HDK_ENTRY_RSP_t;

typedef struct
{
	USIM_PBK_HDK_RESULT_t result;	///< Result for updating 3G phonebook hidden key
} USIM_PBK_UPDATE_HDK_ENTRY_RSP_t;	

typedef struct
{
	USIM_PBK_ALPHA_RESULT_t result;	///< Result for updating alpha in EF-AAS or EF-GAS in 3G phonebook
    PBK_Id_t pbk_id;				///< Phonebook ID, global or local phonebook
} USIM_PBK_UPDATE_ALPHA_ENTRY_RSP_t;


typedef struct
{
	USIM_PBK_ALPHA_RESULT_t result;	///< Result for reading alpha in EF-AAS or EF-GAS in 3G phonebook
	PBK_API_Name_t alpha_data;		///< EF-AAS or EF-GAS alpha data
    PBK_Id_t pbk_id;				///< Phonebook ID, global or local phonebook
} USIM_PBK_READ_ALPHA_ENTRY_RSP_t;	


typedef struct
{
	USIM_PBK_ALPHA_RESULT_t result;		///< Result for reading configuration of EF-AAS or EF-GAS in 3G phonebook
	USIM_PBK_ALPHA_INFO_t alpha_info;	///< Configuration info, applicable only if "result" indicates success
    PBK_Id_t pbk_id;					///< Phonebook ID, global or local phonebook
} USIM_PBK_ALPHA_INFO_RSP_t;


#endif
