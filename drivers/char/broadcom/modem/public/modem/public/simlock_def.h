//***************************************************************************
//
//	Copyright © 2001-2008 Broadcom Corporation
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
*   @file   simlock_def.h
*
*   @brief  This file defines common definitionss for simlock.
*
****************************************************************************/
/**
*   @defgroup   SIMLOCKGroup   SIM Security definitions
*   @brief      This group defines the interfaces to the SIM Lock module 
*   @ingroup    SIMSTK
*
*****************************************************************************/

#ifndef _SIMLOCK_DEF_H_
#define _SIMLOCK_DEF_H_


/**
 * @addtogroup SIMLOCKGroup
 * @{
 */

/**
 Simlock type, first SIMLOCK type must start with 0 and subsequent SIMLOCK 
 types must be consecutive. Code implementation assumes this.
**/
typedef enum {
	SIMLOCK_NETWORK_LOCK = 0, 
	SIMLOCK_NET_SUBSET_LOCK,
	SIMLOCK_PROVIDER_LOCK,
	SIMLOCK_CORP_LOCK,
	SIMLOCK_PHONE_LOCK,

	SIMLOCK_INVALID_LOCK ///< This must be put in the last

} SIMLockType_t;


/**
 Status of unlocking all type of SIM Locks (Network, Service Provider, etc).
**/
typedef enum
{
	SIMLOCK_SUCCESS,			///< Lock successfully unlocked
	SIMLOCK_FAILURE,			///< Lock not unlocked, but can try again
	SIMLOCK_PERMANENTLY_LOCKED,	///< Permanent locked	
	SIMLOCK_WRONG_KEY,          ///< Unlock password is wrong, can retry
	SIMLOCK_NOT_ALLOWED,        ///< Unlock not allowed
	SIMLOCK_DISABLED,           ///< Unlock disabled

	SIMLOCK_INVALID_STATUS      ///< Invalid status of SIMLOCK

} SIMLock_Status_t;



/** @} */

#endif  // _SIMLOCK_DEF_H_
