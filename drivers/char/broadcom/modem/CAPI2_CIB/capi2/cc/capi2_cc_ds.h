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
*   @file   capi2_cc_ds.h
*
*   @brief  This file defines the capi2 Call Control related data types
*
****************************************************************************/
#ifndef _CAPI2_CC_DS_H_
#define _CAPI2_CC_DS_H_


/**
 * @addtogroup CAPI2_CCAPIGroup
 * @{
 */


// Data Definitions used by CAPI2 only


#define PHONE_NUMBER_LEN	40


/** 
	Get CNAP Name msg format
**/

typedef struct
{
	ALPHA_CODING_t nameCoding;	///< Alpha coding
	UInt8	cnapName[80];		///< Name Data
	UInt8	nameLength;			///< Number of bytes in "cnapName", 0 if CNAP does not exist
} CNAP_NAME_t;


/** 
	Phone number dial string
**/
typedef struct
{
	char phone_number[PHONE_NUMBER_LEN];	///< NULL terminated dial string
} PHONE_NUMBER_STR_t;

/** 
	States for all non-idle calls
**/
typedef struct
{
	CCallStateList_t stateList;	///< Call state array
	UInt8 listSz;								///< Number of call states
} ALL_CALL_STATE_t;

/** 
	Indices for all non-idle calls
**/
typedef struct
{
	CCallIndexList_t indexList;	///< Call index array
	UInt8 listSz;								///< Number of call indices
} ALL_CALL_INDEX_t;

/** @} */
#endif
