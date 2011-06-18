//***************************************************************************
//
//	Copyright © 2002-2008 Broadcom Corporation
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
*   @file   phonectrl_def.h
*
*   @brief  This file contains definitions for the type for PCH 
*			(GPRS PDP Context Handler) API.
*
****************************************************************************/

#ifndef _PHONECTRL_DEF_
#define _PHONECTRL_DEF_


#define RX_SIGNAL_INFO_UNKNOWN 0xFF
#define PHONECTRL_MAX_NMR_LEN 96
typedef enum
{
	POWER_DOWN_STATE_OFF,		///< System is running
	POWER_DOWN_STATE_DETACH,	///< System is in Power Down Detach process
	POWER_DOWN_STATE_RESET		///< System is waiting for reset

} PowerDownState_t;				///< System power down state

///	CAPI system states
typedef enum
{
 	SYSTEM_STATE_OFF,				///< System is off, waiting for power-on
 	SYSTEM_STATE_ON,				///< System is on
 	SYSTEM_STATE_ON_NO_RF,			///< System is On but no radio activity
 	SYSTEM_STATE_OFF_IN_PROGRESS,	///< System is powering down in progress
 	SYSTEM_STATE_OFF_CHARGING		///< Charger is plugged in while handset is off
} SystemState_t;					///< System state

typedef enum
{
    PHONECTRL_INVALID_DOMAIN,
    PHONECTRL_CS_DOMAIN,
    PHONECTRL_PS_DOMAIN,
} PhonectrlDomain_t;

typedef enum
{
    PHONECTRL_CONTEXT_UNKNOWN,
    PHONECTRL_CONTEXT_GSM,
    PHONECTRL_CONTEXT_UMTS,
    PHONECTRL_CONTEXT_EPS
} PhonectrlCipherContext_t;

typedef struct
{
    PhonectrlDomain_t domain;
    PhonectrlCipherContext_t context;
    GPRS_CiphAlg_t ps_alg;  
} PhonectrlCipherAlg_t;

typedef struct
{
  UInt8 len;                        ///< length of NMR info
  UInt8 nmr[PHONECTRL_MAX_NMR_LEN]; ///< NMR info
  UInt8 rat;                        ///< current RAT (RAT_GSM, RAT_UMTS)
  UInt8 regState;                   ///< registration state based on MSRegState_t enum
} Phonectrl_Nmr_t;

#endif
