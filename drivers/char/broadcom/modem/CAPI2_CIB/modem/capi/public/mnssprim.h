//***************************************************************************
//
//	Copyright © 2007-2008 Broadcom Corporation
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
*   @file   mnssprim.h
*
*   @brief  This file defines the types and prototypes for the MNSS primitive functions.
*
****************************************************************************/

#ifndef _MSC_MNSSPRIM_H_
#define _MSC_MNSSPRIM_H_


//******************************************************************************
// Function Prototypes
//******************************************************************************

void MNSSPRIM_RxPrim(PrimPtr_t Primitive);


//**************************************************************************************
/**
	MnssPrim_RegisterMsg() function builds and send the Register messages.
	@param		inMnssCtxPtr (in) MNSS Context Pointer
	@return		(out) void
**/
void MnssPrim_RegisterMsg(MNSS_Context_t* inMnssCtxPtr);

//**************************************************************************************
/**
	MnssPrim_RegisterMsg() function builds and send the Facilityt messages.
	@param		inMnssCtxPtr (in) MNSS context pointer.
	@param		inCompType (in) component type.
	@return		(out) void
**/
void MnssPrim_FacilityMsg(MNSS_Context_t*	inMnssCtxPtr,
						  SS_Component_t	inCompType);

//**************************************************************************************
/**
	MnssPrim_ReleaseCompleteMsg() function builds and send the Release_Complete messages.
	@param		inTransId (in) transaction Id of the SS session.
	@param		inProcessId (in) process Id of the stack task.
	@param		inMnssCtxPtr (in) MNSS context pointer.
	@param		inCause (in) release cause to be sent to the network.
	@return		(out) void
**/
void MnssPrim_ReleaseCompleteMsg(UInt8				inTransId,
								 PId_t				inProcessId,
								 MNSS_Context_t*	inMnssCtxPtr,
								 Cause_t			inCause);

#endif //_MSC_MNSSPRIM_H_

