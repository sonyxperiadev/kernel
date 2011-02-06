//***************************************************************************
//
//	Copyright © 2007-2009 Broadcom Corporation
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
*   @file   mnss_misc.h
*
*   @brief  This file defines the types and prototypes for the MNSS misc info.
*
****************************************************************************/

#ifndef _MNSS_MISC_H_
#define _MNSS_MISC_H_


/// Structure : MNSS Context
typedef struct
{
	ClientInfo_t		clientInfo;			///< Client Information
	CallIndex_t			callIndex;			///< Call Index
	PId_t				processId;			///< Process Identifier
	UInt8				transId;			///< Transaction Identifier
	Boolean				broadcastRsp;		///< Broadcast Response
	MNSS_State_t		state;				///< SS state
	Timer_t				timerPtr;			///< Termination Timer
	SS_Component_t		component;			///< Component Type
	UInt8				invokeId;			///< Invoke Id
	SS_Operation_t		operation;			///< SS Operation
	SS_SrvReq_t*		srvReqPtr;			///< SS Service Request Pointer	
	UInt8				resend;				///< Resend the request to the network

} MNSS_Context_t;							///< MNSS Context Type



/// Structure : MNSS Message Type
typedef struct
{
	ClientInfo_t	clientInfo;			///< Client Information
	UInt8			callIndex;			///< Call Index                     Shall be removed
	PId_t			processId;			///< Process Identifier
	UInt8			transId;			///< Transaction Identifier
	MNSS_Prim_t		primType;			///< MNSS Primitive Type
	void*			dataPtr;			///< Data Pointer
} MNSS_Msg_t;							///< MNSS Message Type


/// Structure : MNSS Service Request Context Type
typedef struct
{
	Timer_t				timerPtr;			///< Termination timer pointer
	MNSS_Msg_t*			mnssMsgPtr;			///< MNSS msg ptr "Service Request"

}MNSS_SrvReqCtx_t;							///< MNSS Service Request Context Type


#endif //_MNSS_MISC_H_

