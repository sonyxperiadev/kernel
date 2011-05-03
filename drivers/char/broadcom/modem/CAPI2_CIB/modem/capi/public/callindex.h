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
*   @file   callindex.h
*
*   @brief  This file contains definitions for Call Index Database.
*
****************************************************************************/

#ifndef _MSC_CALLINDEX_H_
#define _MSC_CALLINDEX_H_


//******************************************************************************
// Enumerations
//******************************************************************************

typedef enum
{
	CALLINDEXTYPE_FREE 		= 0x00,	// Call Index is not used
	CALLINDEXTYPE_MO_CC		= 0x01,	// Call Index is used for MO Call Control
	CALLINDEXTYPE_MO_CC_DS	= 0x02,	// Call Index is used for MO Call Control, Data Service
	CALLINDEXTYPE_MO_SS		= 0x03,	// Call Index is used for MO Supplementary Service
	CALLINDEXTYPE_MO_SMS	= 0x04,	// Call Index is used for MO Short Message Service
	CALLINDEXTYPE_MT_CC		= 0x05,	// Call Index is used for MT Call Control
	CALLINDEXTYPE_MT_CC_DS	= 0x06,	// Call Index is used for MT Call Control, Data Service
	CALLINDEXTYPE_MT_SS		= 0x07,	// Call Index is used for MT Supplementary Service
	CALLINDEXTYPE_MT_SMS	= 0x08,	// Call Index is used for MT Short Message Service
									// where MO = Mobile Originated
									//       MT = Mobile Terminated
	CALLINDEXTYPE_NO_TI		= 0x09	// TI hasn't been set
} CallIndexType_t;

//******************************************************************************
// Callbacks
//******************************************************************************


//******************************************************************************
// Function Prototypes
//******************************************************************************

void CALLINDEX_Init( void );			// Initialize the call-control manager



Result_t CALLINDEX_AllocateCallIndex(
	CallIndex_t *ci,				// newly allocated Call Index
	CallIndexType_t proposed_type	// proposed type of call
	);

Result_t CALLINDEX_UpdateCallIndex(
	CallIndex_t ci,					// newly allocated Call Index
	UInt8 ti,						// Transaction Id
	CallIndexType_t allocated_type	// allocated type of call
	);

void CALLINDEX_FreeCallIndex(
	CallIndex_t ci					// Call Index to be free
	);

Result_t CALLINDEX_GetCallIndexInfo(
	CallIndex_t ci,					// Call Index
	UInt8 *ti,						// Transaction Id
	CallIndexType_t *type			// type of call
	);



Boolean  CALLINDEX_IsCIDataCall(
	CallIndex_t ci
	);

CallIndex_t CALLINDEX_GetCI(
	UInt8 ti
	);

UInt8 CALLINDEX_GetTI(
	CallIndex_t ci
	);


#endif

