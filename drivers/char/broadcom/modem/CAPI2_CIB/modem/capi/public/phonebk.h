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
*   @file   phonebk.h
*
*   @brief  This file contains definitions for 2G phone book subsystem prototypes.
*
****************************************************************************/

#ifndef _PHONEBK_H_
#define _PHONEBK_H_


//******************************************************************************
//							include block
//******************************************************************************



//-------------------------------------------------
// Constant Definitions
//-------------------------------------------------

/** Invalid phone book index */
#define INVALID_PBK_INDEX 0xFFFF

/** The index of a matching record in EF-EXT1, EF-EXT2 etc can not be found */
#define EXT_REC_INDEX_NOT_FOUND 0xFF

/// Maximum number of EF-PBR records in USIM we support 
#define MAX_NUM_OF_PBR_REC 	MAX_NUM_OF_ADN_SET

/** Indicate requested phone entry is empty. Note that the first bit must be '0' as 
 * the PBK_EMPTY constant is used extensively in unsigned comparison.
 */
#define	PBK_EMPTY ((PBK_RawIndex_t) 0x7FFF)


//-------------------------------------------------
// Data Structure
//-------------------------------------------------


typedef Int16	PBK_Index_t;		///< Index into sorted phone book list
typedef Int16	PBK_RawIndex_t;		///< Index into unsorted phone book list


/// Phonebook Information request
typedef struct
{
	PBK_Id_t pbk_id;				///< Phonebook ID
	CallbackFunc_t* pbk_access_cb;	///< Callback to return access info.
} PBK_INFO_REQ_t;


/// Request for multiple alpha search
typedef struct
{
	PBK_Id_t pbk_id;				///< Phonebook ID
	PBK_API_Name_t alpha_data;		///< Actual alpha data
	CallbackFunc_t* pbk_access_cb;	///< Callback to return result
} PBK_FIND_ALPHA_MUL_REQ_t;

/// Request for first match of alpha
typedef struct
{
	UInt8 num_of_pbk;					///< Number of phonebook
	PBK_Id_t pbk_id[NUM_OF_PBK_TYPE];	///< Phonebook ID
	PBK_API_Name_t alpha_data;			///< Alpha data
	CallbackFunc_t* pbk_access_cb;		///< Callback function
} PBK_FIND_ALPHA_ONE_REQ_t;


/// Phonebook write entry request
typedef struct
{
	PBK_Id_t pbk_id;                        ///< Phonebook ID
	Boolean	special_fax_num_handling;       
	PBK_Record_t pbk_rec;                   ///< Phonebook entry data
	Boolean usim_adn_ext_data_exist;        ///< TRUE if extended 3G ADN phonebook data exisits
	USIM_PBK_EXT_DATA_t adn_ext_data;       ///< Extended 3G ADN phonebook data
	CallbackFunc_t* pbk_access_cb;          ///< Callback to return result
} PBK_WRITE_ENTRY_REQ_t;


/// Data to check if the number is dialable
typedef struct
{
	PBK_Dialled_Num_t dialled_num;         ///< Dialled number
	CallbackFunc_t* pbk_access_cb;         ///< Callback to return result
} PBK_CHK_NUM_DIALLABLE_REQ_t;


/// Phonebook entry read request
typedef struct
{
	PBK_Id_t pbk_id;                     ///< Phonebook ID
	UInt16 start_index;                  ///< Start index to read
	UInt16 end_index;                    ///< End index to read
	CallbackFunc_t* pbk_access_cb;       ///< Callback to return result
} PBK_ENTRY_READ_REQ_t;


/// Phonebook Alpha info request 
typedef struct
{
	USIM_PBK_ALPHA_TYPE_t alpha_type;      ///< Alpha type AAS/GAS
	PBK_Id_t pbk_id;                       ///< Phonebook ID
	CallbackFunc_t* pbk_access_cb;         ///< Callback to return result
} USIM_PBK_ALPHA_INFO_REQ_t;


/// Read Alpha data for a Phonebook entry request 
typedef struct
{
	USIM_PBK_ALPHA_TYPE_t alpha_type;      ///< Alpha type AAS/GAS
	UInt16 index; /* 0-based */            ///< Phonebook entry index
	PBK_Id_t pbk_id;                       ///< Phonebook ID
	CallbackFunc_t* pbk_access_cb;         ///< Callback to return result
} USIM_PBK_READ_ALPHA_ENTRY_REQ_t;

/// Write Alpha data to a Phonebook entry request 
typedef struct
{
	USIM_PBK_ALPHA_TYPE_t	alpha_type;    ///< Alpha type AAS/GAS
	UInt16 index; /* 0-based */            ///< Phonebook entry index
	PBK_Id_t pbk_id;                       ///< Phonebook ID 
	PBK_API_Name_t alpha_data;             ///< Alpha data
	CallbackFunc_t* pbk_access_cb;         ///< Callback to return result
} USIM_PBK_UPDATE_ALPHA_ENTRY_REQ_t;

/// Write Hidden Key data request 
typedef struct
{
	HDKString_t hidden_key;                ///< Hidden key string
	CallbackFunc_t* pbk_access_cb;         ///< Callback to return result

} USIM_PBK_UPDATE_HDK_ENTRY_REQ_t;

/// Read Hidden Key data request 
typedef struct
{
    CallbackFunc_t* pbk_access_cb;         ///< Callback to return result
} USIM_PBK_READ_HDK_ENTRY_REQ_t;           ///< Phonebook Hidden Key result

 
/**
	struct for phone book BCD number
**/
typedef struct
{
	gsm_TON_t		ton;                        ///< GSM Type of Number
	gsm_NPI_t		npi;                        ///< GSM Number Plan Identifier
	UInt8			digits[PBK_DIGITS >> 1];    ///< BCD digit data
	UInt8			ext_rec_index;	            ///< 1-based extension record in EF-EXT1, EF-EXT2 etc
	UInt8           cmp;                        ///< Comparison Method Pointer for BDN
} PBK_BCD_Number_t;


/**
	enum for phone book ext file
**/
typedef enum
{
	PBK_EXT1,
	PBK_EXT2
} PBK_EXT_FILE_t;


/* Buffer to store a record in EF-EXT1, EF-EXT2 etc */
typedef UInt8 PBK_EXT_DIGIT_t[SIM_EXT_DIGIT_NUM_OF_BYTES];

/**
   Structure to hold extension digit data for a phonebook entry: 
   We support one level of extension, i.e. chaining inside the EF-EXT1 and EF-EXT2
   SIM file is not supported.
**/
typedef struct 
{
	Boolean ext_update_required;	///< TRUE if digit extension file needs to be updated
	PBK_EXT_FILE_t ext_file;		///< Digit extension file, e.g. EF-EXT1, EF-EXT2
	UInt8 ext_update_index;			///< 1-based digit extension record index of the EF-EXT to be updated
	UInt8 pbk_ext_index;			///< 1-based digit extension record index for this phonebook entry
	PBK_EXT_DIGIT_t ext_rec_data;	///< Digit Extension record data: coded as defined in Section 10.5.10 of GSM 11.11
} PBK_EXT_DATA_t;


//-------------------------------------------------
// Function Prototype
//-------------------------------------------------
void PostMsgToPbkTask(InterTaskMsg_t* outMsg);


//**************************************************************************************
/**
	Start the Phone book processing
**/

void PBK_Run( void );			

//**************************************************************************************
/**
	Initialize Phone book Processing
**/

void PBK_Init( void );			


//**************************************************************************************
/**
	Function to build all phonebooks
**/

void PBK_BuildPbkReq(void);


//**************************************************************************************
/**
	Function to rebuild ADN phonebook
**/

void PBK_RebuildADNPhonebk(void);


//***************************************************************************************
/**
    This function adds an ECC number to the ECC number table.

	@param		ecc_num (in) ECC number in BCD format	
	@return		
	@note	

**/

void PBK_AddEccNumber(const ECC_NUM_t *ecc_num);


//***************************************************************************************
/**
    This function adds ECC number & service category (3G) to ECC table.

	@param		rec_ptr (in) pointer to ECC record (number and service category)
	@param		rec_len (in) ECC record length
	@param		ecc_src (in) ECC source
	@return		
	@note
**/

void PBK_AddEccRecord(UInt8 *rec_ptr, UInt8 rec_len, ECC_SRC_t ecc_src);


//***************************************************************************************
/**
    This function deletes ECC records from ECC table based on source (MS, SIM or Network).

	@param		ecc_src (in) ECC source	
	@return		
	@note	

**/

void PBK_DelEccRecords(ECC_SRC_t ecc_src);


//***************************************************************************************
/**
    This function checks whether there exists a phonebook record that uses the passed 
    0-based extension record index.

	@param		pbk_id (in) phone book id
	@param		ext_rec_index (in) 0-based extension record index
	@return		Boolean
    @note
	The return value: TRUE - if exists, FALSE - otherwise
	
**/

Boolean IsPbkExtRecIndexUsed(PBK_Id_t pbk_id, UInt8 ext_rec_index);


//***************************************************************************************
/**
    This function checks whether the passed extension record index is
    chained by other extension records of the same extension file. 

	@param		ext_rec_index (in) 0-based extension record index
	@param		ext_List (in) pointer to the extension list
	@param		rec_num (in) record number of the extension list
	@return		Boolean
    @note
	The return value: TRUE - if chained, FALSE - otherwise
	
**/

Boolean IsExtRecChained(UInt8 ext_rec_index, UInt8 *ext_List, UInt8 rec_num);


//***************************************************************************************
/**
    This function checks whether digit extension is needed for the passed phone 
    number and whether the digit extension can be done. 

	@param		pbk_id (in) phonebook id
	@param		pbk_index (in) phonebook index in the number list, e.g. ADN, FDN
	@param		number (in) passed phone number
    @param		ext_data (in) pointer to the extension data structure
	@param		ext_List (in) pointer to the extension list
	@param		rec_num (in) record number of the extension list
	
	@return		Boolean
    @note
	The return value: TRUE - the passed number can be saved in phonebook
	                  FALSE - the passed number cannot be saved in phonebook because
	                          the number is too long
	If digit extension is needed, this function fills in the "ext_data" structure
    with the appropriate data. 
**/

Boolean PBK_CheckDigitExtRequired( PBK_Id_t pbk_id, UInt16 pbk_index, 
									UInt8 *number, PBK_EXT_DATA_t *ext_data, UInt8 *ext_List, UInt8 rec_num );


//***************************************************************************************
/**
    This function obtains the selection path information for EF-EXT1 or EF-EXT2.

	@param		ext_file (in) phone book extension file
	@param		path_info (in) selection path information for a SIM/USIM file
	@param		pbk_id (in) phonebook id
	@return		
    @note
		
**/
									
void PBK_GetExtFilePathInfo(PBK_EXT_FILE_t ext_file, SIM_FILE_PATH_INFO_t *path_info, PBK_Id_t pbk_id);


#ifdef UHT_HOST_BUILD
/* Init function for host */
void PBK_host_init(void);
/* PBK_Entry called with inMsg as param */
void PBK_Entry(InterTaskMsg_t		*inMsg);
#else
//***************************************************************************************
/**
    This function is the entry point to the phonebook task.	
	@return		
    @note
**/
void PBK_Entry( void );
#endif


//***************************************************************************************
/**
    This function posts the Phonebook Ready Ind to ATC/MMI.

	@return		
    @note
		
**/

void PBK_PostReadyInd(void);


//***************************************************************************************
/**
    This function Convert phonebook alpha from internal format to API format.

    @param		alpha_data (in) phonebook API name struct
	@param		name (in) alpha associated with name.  Use same type as SIM.
	@return		
    @note
			
**/

void PBK_ConvertAlpha(PBK_API_Name_t *alpha_data, PBK_Name_t *name);


//***************************************************************************************
/**
    This function gets the combined ECC Phonebook list when SIM is inserted.

    @return		ECC_REC_t *
    @note

    Do not use this API if SIM is not inserted.
			
**/

ECC_REC_t *PBK_GetCachedEccRecList(void);


//***************************************************************************************
/**
    This function gets the basic information of a requested phonebook.

    @param		pbk_id (in) phone book id
    @param		infoRsp (in) Phonebook Information response
	@return		
    @note
			
**/

void PBK_GetCachedPbkInfo(PBK_Id_t pbk_id, PBK_INFO_RSP_t *infoRsp);


//***************************************************************************************
/**
    This function returns the 0-based index of the first empty record in the corresponding 
    phonebook digit extension file (EF-EXT1 for ADN phonebook, EF-EXT2 for FDN phonebook) 
    of the passed phonebook type.

    @param		ext_list (in) extension list
    @param		rec_num (in) record number in the extension list
	@return		UInt8
    @note
			
**/

UInt8 GetEmptyExtRecIndex(UInt8 *ext_list, UInt8 rec_num );

//**************************************************************************************
/**
    This function returns the Emergency Service Category associated
    with the provided emergency number.

	1. When SIM/USIM is present: 
	   The emergency service category read from EF-ECC in SIM/USIM

	2. When SIM/USIM is not present: 0
	          
    @return  Emergency Service Category associated with number
    @note     		
**/
UInt8 PBK_GetEmergencyServiceCategory(char *phone_number);


//**************************************************************************************
/**
    This function gets the basic information of a requested phonebook
 
	@return		TRUE if data is valid; FALSE otherwise. 
	@note
**/
Result_t PBK_GetAllPbkInfoReq(PBK_ALL_INFO_RSP_t *infoRsp);


//**************************************************************************************
/**
    This function gets the Comparison Method identifier

    @param      cmp Method Pointer

	@return		Comparison Method identifier
	@note
**/
UInt8 PBK_GetCmi(UInt8 cmp);


//**************************************************************************************
/**
    This function returns the current state of the phonebook module.
 
	@return		Phonebook state.
	@note
**/
PBK_State_t PBK_GetPbkState(void);


#endif
