//***************************************************************************
//
//	Copyright © 2009 Broadcom Corporation
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
*   @file   phonectrl_api_old.h
*
*   @brief  This file defines the interface for OLD phone ctrl related API functions.
*
*	Note:	The API functions in this file are obsoleted and will be removed in later release
*
****************************************************************************/

#ifndef _PHONECTRL_API_OLD_H_
#define _PHONECTRL_API_OLD_H_
Boolean IsPowerDownInProgress(void);
void 		SYS_ProcessReset(void);
/**
 * @addtogroup OldPhoneControlAPIGroup
 * @{
 */
/**
	This function de-registers the mobile from the network and
	powers down the system.
**/
void				SYS_ProcessPowerDownReq(void);


/**
	This function powers-up the platform with No RF activity.  In this state
	the system is functional but can not access the network.  It is typically
	used to allow access to run the mobile in a restricted environment such as
	an airplane.
**/
void				SYS_ProcessNoRfReq(void);


/**
	This function powers-up the platform.  The mobile will start to search for
	a network on which to camp and will broadcast events to registered clients.

**/
void				SYS_ProcessPowerUpReq(void);


/**
	Function to set the threshold parameters to control whether RSSI indication message MSG_RSSI_IND is 
	posted to clients. Once this function is called, the difference between the new and old RXLEV/RXQUAL 
	values (if current RAT is GSM) or RSCP/Ec/Io values (if current RAT is UMTS) must be larger or equal 
	to the threshold in order for the MSG_RSSI_IND message to be sent. 

    The caller can pass 0 to a threshold to disable the threshold checking. 

	@param gsm_rxlev_thresold (in) GSM RXLEV threshold. See section 8.1.4 of GSM 05.08 for RXLEV values. 
	@param gsm_rxqual_thresold (in) GSM RXQUAL threshold. See Section 8.2.4 of GSM 05.08 for RXQUAL values. 
	@param umts_rscp_thresold (in) UMTS RSCP threshold. See Section 9.1.1.3 of 3GPP 25.133 for RSCP values.
	@param umts_ecio_thresold (in) UMTS Ec/Io threshold. See Section 9.1.2.3 of 3GPP 25.133 for Ec/Io values.

	@return		 None

**/
void SYS_SetRssiThreshold(UInt8 gsm_rxlev_thresold, UInt8 gsm_rxqual_thresold, UInt8 umts_rscp_thresold, UInt8 umts_ecio_thresold);

/**
	This function is used to check the system state value.

	@return		SystemState_t

	Possible return values are ::SYSTEM_STATE_OFF, ::SYSTEM_STATE_ON, ::SYSTEM_STATE_ON_NO_RF, ::SYSTEM_STATE_OFF_IN_PROGRESS,
	::SYSTEM_STATE_OFF_CHARGING.
**/
SystemState_t SYS_GetSystemState(void);

/**
	This function is used to set the system state value.

	@param		state  Possible values are ::SYSTEM_STATE_OFF, ::SYSTEM_STATE_ON, ::SYSTEM_STATE_ON_NO_RF, ::SYSTEM_STATE_OFF_IN_PROGRESS,
	::SYSTEM_STATE_OFF_CHARGING.
**/
Result_t	SYS_SetSystemState(SystemState_t state);
/**
	This function returns the recieved signal level and signal quality 

	@param		RxLev  signal level passed by reference to be filled in by the called function
	@param		RxQual rxquality passed by reference to be filled in by the called function
	
	@return		void
**/
void	SYS_GetRxSignalInfo(UInt8 *RxLev, UInt8 *RxQual); 

/** @} */

#endif
