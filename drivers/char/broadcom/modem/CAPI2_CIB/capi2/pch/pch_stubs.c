//*********************************************************************
//
//	Copyright © 2004-2008 Broadcom Corporation
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
*   @file   pch_stubs.c
*
*   @brief  This file implements the functions needed by pch and snprim.
*			There are two main functionality. The implementation is from dlink.
*			1. creation/deletion of T_OS_PP_ID.
*			2. update tft into dc data structure.
*
****************************************************************************/
#define	ENABLE_LOGGING
#include	<stdio.h>
#ifndef UNDER_LINUX
#include <string.h>
#endif

#include	"mobcom_types.h"
#include    "ms.h" 
#include    "resultcode.h"
#include    "common_defs.h"
#include	"taskmsgs.h"
#include	"pch_def.h"
#include	"consts.h"
#include 	"ostypes.h"
#include	"mti_trace.h"
#include	"xassert.h"
#include	"log.h" //for PPP_LOGV
#include	"logapi.h" //for Log_DebugOutputString
#include	"pchex_def.h"
#include	"pchex_api.h"



Result_t PCHStubs_PDP_SendPDPActivateReq(UInt8	clientID,
	UInt8						cid,
	PCHActivateReason_t			reason,
	PCHProtConfig_t				*protConfig)
{
	return PCHEx_SendPDPActivateReq(clientID, cid, reason, protConfig);
}


Result_t PCHStubs_PDP_SendPDPActivateSecReq(
	UInt8						clientID,
	UInt8						cid
	)
{
	return PCHEx_SendPDPActivateSecReq(clientID, cid);
}



Result_t PCHStubs_PDP_SendPDPDeactivateReq(
	UInt8						clientID,
	UInt8						cid
	)
{
	return PCHEx_SendPDPDeactivateReq(clientID, cid);
}

Result_t PCHStubs_PDP_SendPDPModifyReq(UInt8	clientID,
	UInt8						cid)
{
	return PCHEx_SendPDPModifyReq(clientID, cid);
}



