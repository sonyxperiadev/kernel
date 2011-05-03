//***************************************************************************
//
//	Copyright © 2005-2008 Broadcom Corporation
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
*   @file   phonebk_3g.h
*
*   @brief  This file contains prototypes for 3G phonebook functionality. 
*
****************************************************************************/
#ifndef _PHONEBK_3G_H_
#define _PHONEBK_3G_H_


//******************************************************************************
//							Include block
//******************************************************************************



//******************************************************************************
//							Definition block
//******************************************************************************

#define USIM_HIDDEN_KEY_FILE_LEN 4	///< EF-Hidden_Key length in USIM

/* Path information for the parent of DF-Phonebook */
typedef struct
{
	UInt8 path_len;		///< Number of "File ID" in "path" 
	const UInt16 *path;	///< File ID array for the parent of DF-Phonebook including MF 
} USIM_PBK_PATH_INFO_t;


/* Selected 3G Phonebook type */
typedef enum
{
	SEL_LOCAL_USIM_PBK,		///< Select local phonebook for phonebook operation, e.g. get info, read and write 		
	SEL_GLOBAL_USIM_PBK		///< Select global phonebook for phonebook operation, e.g. get info, read and write 	
} USIM_PBK_SELECT_t;


//******************************************************************************
//							Function Declarations
//******************************************************************************

//**************************************************************************************
/**
    This function initializes the 3G extended phonebook. It shall be called only if 
	a 3G USIM is inserted. 

    @return	
	@note
**/
void PBK_ReadUsimPbkFile(void);


//**************************************************************************************
/**
    This function returns the 3G extended phonebook configuration information. 

    @param pbk_type (in) Global or Local phonebook
	@param pbk_info (out) Buffer to store the returned phonebook configuration information
	
	@return	
	@note
**/
void PBK_GetUsimAdnInfo(USIM_PBK_SELECT_t pbk_type, USIM_PBK_INFO_t *pbk_info);


//**************************************************************************************
/**
    This function returns the 3G extended phonebook configuration information. 

    @param pbk_type (in) Global or Local phonebook
	@param rec_index (in) record index
	
	@return	
	@note
**/
void PBK_UpdateUsimRecNofication(USIM_PBK_SELECT_t pbk_type, UInt16 rec_index);


//**************************************************************************************
/**
    This function returns the 3G extended phonebook configuration information. 

    @param pbk_type (in) Global or Local phonebook
	
	@return	none
	@note
**/
void PBK_DeleteUsimAllNofication(USIM_PBK_SELECT_t pbk_type);


//**************************************************************************************
/**
    This function returns the 3G extended phonebook configuration information. 

    @param pbk_type (in) Global or Local phonebook
	
	@return	UInt16
	@note
**/
UInt16 PBK_GetUsimExt1FileId(USIM_PBK_SELECT_t pbk_type);


//**************************************************************************************
/**
    This function returns the 3G extended phonebook configuration information. 

    @param pbk_type (in) Global or Local phonebook
	
	@return	USIM_PBK_PATH_INFO_t
	@note
**/
USIM_PBK_PATH_INFO_t PBK_GetUsimPathInfo(USIM_PBK_SELECT_t pbk_type);


//**************************************************************************************
/**
    This function returns the 3G extended phonebook configuration information. 

    @param pbk_type (in) Global or Local phonebook
	
	@return	UInt16
	@note
**/
UInt16 PBK_GetUsimAdnFileId(USIM_PBK_SELECT_t pbk_type);


//**************************************************************************************
/**
    This function returns the 3G extended phonebook configuration information. 

    @param pbk_type (in) Global or Local phonebook
	@param adn_set (out) Buffer to store the ADN phonebook list
	
	@return	UInt16
	@note
**/
UInt16 PBK_GetUsimAdnRecNum(USIM_PBK_SELECT_t pbk_type, USIM_PBK_ADN_SET_t *adn_set);


//**************************************************************************************
/**
    This function returns the number of filled records in EF-ADN in 3G USIM

    @param pbk_type (in) Global or Local phonebook
	
	@return	Number of filled records in EF-ADN
	@note
**/
UInt16 PBK_GetUsimAdnNonFreeCount(USIM_PBK_SELECT_t pbk_type);


//**************************************************************************************
/**
    This function returns the length of alpha data in EF-ADN in 3G USIM. 

    @param pbk_type (in) Global or Local phonebook
	
	@return	Length of alpha data in EF-ADN
	@note
**/
UInt8 PBK_GetUsimAdnAlphaLen(USIM_PBK_SELECT_t pbk_type);


//**************************************************************************************
/**
    This function returns the preferred USIM phonebook type, i.e. Global phonebook or 
	Local phonebook. 

    @return	Global or Local phonebook
	@note
**/
USIM_PBK_PREFERRED_t PBK_GetUsimPrefPbk(void);


//**************************************************************************************
/**
    This function returns the contents of the EF-ADN plus extension digits in EF-EXT1 of
	a phonebook entry in 3G USIM.

    @param pbk_type (in) Global or Local phonebook
	@param adn_data (out) Buffer to store the returned phonebook contents
	@param index (in) 0-based phonebook index
	
	@return	none
	@note
**/
void PBK_GetUsimAdnData(USIM_PBK_SELECT_t pbk_type, PBK_ADN_DATA_t *adn_data, UInt16 index);


//**************************************************************************************
/**
    This function returns the contents of an EF-EXT1 record 

    @param pbk_type (in) Global or Local phonebook
	@param ext_data (out) Buffer to store the EF-EXT1 record contents
	@param index (in) 0-based EF-EXT1 record index
	
	@return	none
	@note
**/
void PBK_GetUsimAdnExtData(USIM_PBK_SELECT_t pbk_type, USIM_PBK_EXT_DATA_t *ext_data, UInt16 index);


//**************************************************************************************
/**
    This function updates the EF-ADN and EF-EXT1 data in 3G USIM.

    @param pbk_type (in) Global or Local phonebook
	@param adn_data (in) EF-ADN phonebook contents
	@param ext_data (in) EF-EXT1 record contents
	@param index (in) 0-based EF-ADN index
	
	@return	none
	@note
**/
PBK_WRITE_RESULT_t PBK_WriteUsimAdnData(USIM_PBK_SELECT_t pbk_type, const PBK_ADN_DATA_t *adn_data, const USIM_PBK_EXT_DATA_t *ext_data, UInt16 index);


//**************************************************************************************
/**
    This function returns the configuration information for the alpha data in 
	EF-AAS or EF-GAS in 3G USIM 

    @param pbk_type (in) Global or Local phonebook
	@param alpha_type (in) Specify EF-AAS or EF-GAS
	@param alpha_info (out) Buffer to store the configuration
	
	@return	none
	@note
**/
void PBK_GetUsimAlphaInfo(USIM_PBK_SELECT_t pbk_type, USIM_PBK_ALPHA_TYPE_t alpha_type, USIM_PBK_ALPHA_INFO_t *alpha_info);


//**************************************************************************************
/**
    This function returns the alpha data of a record in EF-AAS or EF-GAS in 3G USIM  

    @param pbk_type (in) Global or Local phonebook
	@param alpha_type (in) Specify EF-AAS or EF-GAS
	@param pbk_alpha (out) Buffer to store the returned alpha data
	@param index (in) 0-based index
	
	@return	TRUE if alpha data is successfully returned; FALSE otherwise
	@note
**/
Boolean PBK_GetUsimAlphaData(USIM_PBK_SELECT_t pbk_type, USIM_PBK_ALPHA_TYPE_t alpha_type, PBK_Name_t *pbk_alpha, UInt16 index);


//**************************************************************************************
/**
    This function updates the alpha data of a record in EF-AAS or EF-GAS in 3G USIM  

    @param pbk_type (in) Global or Local phonebook
	@param alpha_type (in) Specify EF-AAS or EF-GAS
	@param pbk_alpha (in) Buffer to store the alpha data
	@param index (in) 0-based index
	
	@return	TRUE if alpha data is successfully updated; FALSE otherwise
	@note
**/
Boolean PBK_WriteUsimAlphaData(USIM_PBK_SELECT_t pbk_type, USIM_PBK_ALPHA_TYPE_t alpha_type, const PBK_Name_t *pbk_alpha, UInt16 index);


//**************************************************************************************
/**
    This function returns the phonebook hiddeny key in 3G USIM

    @param hidden_key (in) Buffer to store the returned hidden key
	
	@return	TRUE if hidden key is successfully returned; FALSE otherwise.
	@note
**/
Boolean PBK_ReadUsimHdkData(HDKString_t *hidden_key);


//**************************************************************************************
/**
    This function updates the phonebook hiddeny key in 3G USIM. 

    @param hidden_key (out) Buffer to store the hidden key to be updated
	
	@return	TRUE if hidden key is successfully updated; FALSE otherwise
	@note
**/
Boolean PBK_WriteUsimHdkData(HDKString_t *hidden_key);


//**************************************************************************************
/**
    This function appends the extension digits to the passed phonebook digit data. 

    @param pbk_type (in) Global or Local phonebook
	@param pbk_ext1 (out) Pointer to the end of the phonebook digit string where extension
						  digits are added as ASCII digits. 
	@param index (in) 0-based index of EF-EXT1 record
	
	@return	none
	@note
**/
void PBK_GetUsimExt1Data(USIM_PBK_SELECT_t pbk_type,  UInt8 *pbk_ext1, UInt16 index);


//**************************************************************************************
/**
    This function checks whether the EF-EXT1 contains empty records. 

    @param pbk_id (in) Specify Global or Local phonebook
	
	@return	TRUE if empty record exists; FALSE otherwise.
	@note
**/
Boolean IsUsimExt1EmptyRecAvail(PBK_Id_t pbk_id);

#endif
