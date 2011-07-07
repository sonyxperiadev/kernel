//*********************************************************************
//
//	Copyright © 2008 Broadcom Corporation
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
*   @file   ms_database_old.h
*
*   @brief  This file defines ms_database related data structure.
*   @note	This file contains definitions shared by CAPI components only.
*
****************************************************************************/
/**
*
*   @defgroup   OldMSDatabaseGroup	Legacy Mobile Station Database APIs
*   @ingroup    MSDatabaseGroup				
*
*   @brief      This group contains legacy APIs allowing getting and setting MS Database elements.
*
	\n Use the link below to navigate back to the main page. \n
    \li \ref index
****************************************************************************/

#ifndef __MS_DATABASE_OLD_H__
#define __MS_DATABASE_OLD_H__

/**
 * @addtogroup OldPhoneControlAPIGroup
 * @{
 */ 

//**************************************************************************************
/**
    This function determines if STK Control flags are enabled

     @param     ms_element (in) The database element type for STK control.

     @return	TRUE if control enabled. FALSE otherwise.
**/
Boolean MS_IsStkCtrlEnabled(MS_Element_t ms_element);

//**************************************************************************************
/**
	This function initializes all system data base elements related to the current 
	call control configuration.  

	@param		inClientId (in) Client ID, used to identify that the client is allowed to
				access the element reference.

	@return		Result_t
**/
Result_t MS_InitCallCfg( UInt8 inClientId );

//**************************************************************************************
/**
	This function initializes FAX-specific system data base elements related to the current 
	call control configuration.  

	@param		inClientId (in) Client ID, used to identify that the client is allowed to
				access the element reference.

	@return		Result_t
**/
Result_t MS_InitFaxConfig( UInt8 inClientId );

//**************************************************************************************
/**
	This function initializes Video-specific system data base elements related to the current 
	call control configuration.  

	@param		inClientId (in) Client ID, used to identify that the client is allowed to
				access the element reference.

	@return		Result_t
**/
Result_t MS_InitVideoCallCfg( UInt8 inClientId ) ;

//**************************************************************************************
/**
	This function is used to check if a gprs call is active in the specified channel

	@param		chan (in) 

	@return		Boolean
**/
Boolean	MS_IsGprsCallActive(UInt8 chan);

//**************************************************************************************
/**
	This function is used to set the GPRS channel active

	@param		chan (in) 
	@param		active (in) Boolean to either disable the channel or enable for GPRS

	@return		void
**/
void	MS_SetChanGprsCallActive(UInt8 chan, Boolean active);

//**************************************************************************************
/**
	This function is used to set the client id for the gprs chan

	@param		chan (in) 
	@param		cid (in)

	@return		void
**/
void	MS_SetCidForGprsActiveChan(UInt8 chan, UInt8 cid);

//**************************************************************************************
/**
	This function is used to get the GPRS channel active associated with the cid

	@param		cid (in) 

	@return		chan
**/
UInt8 MS_GetGprsActiveChanFromCid(UInt8 cid);

//**************************************************************************************
/**
	This function is used to get the cid associated with the GPRS channel active

	@param		chan (in) 

	@return		cid
**/
UInt8 MS_GetCidFromGprsActiveChan(UInt8 chan);


//**************************************************************************************
/**
	Function to initialize Call Config parameters when using AT&F command
	@param		inClientId (in) Client ID, used to identify that the client is allowed to
				access the element reference.
	@return		Result_t
**/
Result_t MS_InitCallCfgAmpF( UInt8 inClientId ) ;

/** @} */


void	MsDatabase_Init(void);

void	MsDb_Init_networkElem(void);

/**
 * @addtogroup OldMSDatabaseGroup
 * @{
 */ 

//=========================================================================
// MS Database API's for accessing of the elements 
//=========================================================================
//**************************************************************************************
/**
	This function is a generic interface that will be used by any clients (external/
	internal) to update the MS Database elements. This function will copy the element
	passed in the third argument in to the database.
	Note: After proper range checks in neccessary the value will be updated.

	@param		inClientId (in) Client ID, used to verify whether the client is allowed to modify
				the element. Currently this is not done. Could be implemented later.
	@param		inElemType (in) The database element type.
	@param		inElemPtr  (in) A pointer to the location of the new value of the element.

	@return		Result_t
**/
Result_t MS_SetElement(	UInt8			inClientId,
						MS_Element_t	inElemType,
						void*			inElemPtr);

//**************************************************************************************
/**
	This function is a generic interface that will be used by any clients (external/
	internal) to read any MS Database elements. This function will copy the contents of
	the database value to the memory location passed in by the last argument.  
	The calling entity should know what will be the typecast used to retreive the element.

	@param		inClientId (in) Client ID, used to identify that the client is allowd to
				access the element or not.
	@param		inElemType (in) The database element type.
	@param		inElemPtr  (in) The address where the element shall be copied to

	@return		Result_t
**/
Result_t MS_GetElement(	UInt8			inClientId,
						MS_Element_t	inElemType,
						void*			inElemPtr);

/** @} */

void	SYS_EnableCellInfoMsg(Boolean inEnableCellInfoMsg);


#endif	//	__MS_DATABASE_OLD_H__

