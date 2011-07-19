//***************************************************************************
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
*   @file   system_api.h
*
*   @brief  This file contains definitions for Phone Control / System API
*
****************************************************************************/
/**
*
*   @defgroup   PhoneControlAPIGroup	Phone Control API
*   @ingroup    SystemGroup				System API
*
*   @brief      This group defines the interfaces for network registration
*               and platform power up/down.	

	\n Use the link below to navigate back to the Phone Control Overview page. \n
    \li \if CAPI2
	\ref CAPI2PhoneCtrlOverview
	\endif
	\if CAPI
	\ref PhoneCtrlOverview
	\endif
****************************************************************************/


#ifndef __SYSTEM_API_H__
#define __SYSTEM_API_H__



/**
 * @addtogroup OldPhoneControlAPIGroup
 * @{
 *
**/  

//******************************************************************************
/**
	Set Power Down Timer for data storage purpose. If the timer
	is already running as a result of calling SYS_ProcessPowerDownReq(),  
	the original timer will be destroyed and a new timer with the 
	new timeout value will be started.  

	@param powerDownTimer powerdown timer value
**/
Result_t	MS_SetPowerDownTimer(UInt8 powerDownTimer);  

//***************************************************************************************
/**
	This API allows the client to set RUA ready timer for negotiation.

	@param		inRuaReadyTimer (in) RUA ready timer in seconds. 

	@return		None
**/
void	MS_SetRuaReadyTimer(Ticks_t inRuaReadyTimer);  

/** @} */

void CAPI_InitRpc(void);

#endif	//	__SYSTEM_API_H__

