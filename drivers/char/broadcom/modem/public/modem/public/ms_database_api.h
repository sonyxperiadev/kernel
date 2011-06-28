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
*   @file   ms_database_api.h
*
*   @brief  This file defines the global information for MS.
*
****************************************************************************/
/**
*
*   @defgroup   MSDatabaseGroup	Mobile Station Database APIs
*   @ingroup    SystemGroup				
*
*   @brief      This group allows getting and setting the MS Database elements.
*
	\n Use the link below to navigate back to the main page. \n
    \li \ref index
****************************************************************************/

#ifndef __MS_DATABASE_API_H__
#define __MS_DATABASE_API_H__


//=========================================================================
// MS Database API's for accessing of the elements 
//=========================================================================

/**
 * @addtogroup MSDatabaseGroup
 * @{
 */ 

//**************************************************************************************
/**
	This function is a generic interface that will be used by any clients (external/
	internal) to update the MS Database elements. This function will copy the element
	passed in the third argument in to the database.
	Note: After proper range checks in neccessary the value will be updated.

	@param      clientInfoPtr	(in) Client information
	@param		inElemType		(in) The database element type.
	@param		inElemPtr		(in) A pointer to the location of the new value of the element.

	@return		Result_t
**/
Result_t MsDbApi_SetElement(	ClientInfo_t	*clientInfoPtr,
								MS_Element_t	inElemType,
								void*			inElemPtr);

//**************************************************************************************
/**
	This function is a generic interface that will be used by any clients (external/
	internal) to read any MS Database elements. This function will copy the contents of
	the database value to the memory location passed in by the last argument.  
	The calling entity should know what will be the typecast used to retreive the element.

	@param      clientInfoPtr	(in) Client information
	@param		inElemType		(in) The database element type.
	@param		inElemPtr		(in) The address where the element shall be copied to

	@return		Result_t
**/
Result_t MsDbApi_GetElement(	ClientInfo_t	*clientInfoPtr,
								MS_Element_t	inElemType,
								void*			inElemPtr);

Result_t MsDbApi_InitCallCfg(ClientInfo_t* inClientInfoPtr);

Boolean MsDbApi_IsCallActive(void);


void	MsDbApi_SYS_EnableCellInfoMsg(ClientInfo_t* inClientInfoPtr, Boolean inEnableCellInfoMsg);


/** @} */

#endif	//	__MS_DATABASE_API_H__

