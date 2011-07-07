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
*   @file  hal_phonectrl_ds.h
*
*   @brief  This file defines the common data structures used by CAPI & CAPI2 in HAL.
*
****************************************************************************/
#ifndef _HAL_PHONECTRL_DS_H_
#define _HAL_PHONECTRL_DS_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
Real Time Clock Time structure
**/
typedef struct{
	UInt8  Sec;		///< 0-59 seconds
	UInt8  Min;		///< 0-59 minutes
	UInt8  Hour;	///< 0-23 hours
	UInt8  Week;	///< 0-6==sun-sat week
	UInt8  Day;		///< 1-31 day
	UInt8  Month;	///< 1-12 Month
	UInt16 Year;	///< (RTC_YEARBASE) - (RTC_YEARBASE + 99)
} RTCTime_t;

/**
*	HAL EM BATTMGR function call result
**/
typedef enum
{
	EM_BATTMGR_SUCCESS = 0,						///< Successful
	EM_BATTMGR_ERROR_ACTION_NOT_SUPPORTED,		///< Not supported by platform HW
	EM_BATTMGR_ERROR_INTERNAL_ERROR,			///< Internal error: i2c, comm failure, etc.
	EM_BATTMGR_ERROR_EVENT_HAS_A_CLIENT,		///< Error if trying to register more than
												///< 1 client to an event with one client only requirement
	EM_BATTMGR_ERROR_OTHERS						///< Undefined error
} HAL_EM_BATTMGR_Result_en_t;

/**
*	BATTMGR EVENT types
*	Upper layer can register a function to get notification on event below
*	Historically, only one client registered for callback (MMMI), therefore
*	this event management is designed to take one client only.
**/
typedef enum
{
	EM_BATTMGR_CHARGER_PLUG_IN_EVENT,			///< Charger plug in event for both USB and Wall (basic notification of
												///< charging current existence to APP and
												///< show start charging dialog and animate batt icon).
												///< To be more speficic, ie. if it's USB or WAC, use PMU HAL.
	EM_BATTMGR_CHARGER_PLUG_OUT_EVENT,			///< Charger plug out event
	EM_BATTMGR_ENDOFCHARGE_EVENT,		   		///< End of Charge event. Battery is full - charging is done.
												///< APP uses this to show BATT FULL dialog box.
	EM_BATTMGR_BATT_EXTREME_TEMP_EVENT,			///< BATT temp is outside window (safety) or extreme temperature
	EM_BATTMGR_LOW_BATT_EVENT,					///< BATT low is detected
	EM_BATTMGR_EMPTY_BATT_EVENT,				///< BATT empty is detected
	EM_BATTMGR_BATTLEVEL_CHANGE_EVENT			///< BATT level change is detected
} HAL_EM_BATTMGR_Event_en_t;

/**
*	HAL EM BATTMGR Level notification structure
**/
typedef struct
{
	HAL_EM_BATTMGR_Event_en_t	eventType; ///< The event type
	UInt8 inLevel;			///< The battery level, 0~N, depend the sysparm
	UInt16 inAdc_avg;		///< Adc value in mV. Ex, 4000 is 4.0V, 3800 is 3.8V
	UInt8 inTotal_levels;	///< total levels
}HAL_EM_BatteryLevel_t;

//! HAL EM BATTMGR charger type
typedef enum
{
	EM_BATTMGR_WALL_CHARGER = 0,
	EM_BATTMGR_USB_CHARGER
} HAL_EM_BATTMGR_Charger_t;

//! HAL EM BATTMGR charger in/out
typedef enum
{
	EM_BATTMGR_CHARGER_UNPLUGGED = 0,
	EM_BATTMGR_CHARGER_PLUGGED
} HAL_EM_BATTMGR_Charger_InOut_t;

#ifdef __cplusplus
}
#endif

#endif //_HAL_PHONECTRL_DS_H_


