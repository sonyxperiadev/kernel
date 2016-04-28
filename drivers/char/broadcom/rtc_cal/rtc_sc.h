/*******************************************************************************
* Copyright 2012 Broadcom Corporation.  All rights reserved.
*
* @file rtc_sc.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#ifndef _RTC_SC_H_
#define _RTC_SC_H_

/* ---- Macro Declarations ----------------------------------------------- */
#define RTCSC_INVALID_RATIO		0
#define RTCSC_INVALID_TEMP			0

/* ---- Type Definitions -------------------------------------------------- */

/**  Slow Clock EVENT types
*
**/
enum RTC_SC_Event {
	RTC_SC_EVENT_CONFIG, /* to configure resolution/threshold, duration */
	RTC_SC_EVENT_RATIO, /* to transfer clock ratio */
	RTC_SC_EVENT_REGIST, /* to register callback */
	RTC_SC_EVENT_SIMUL1, /* to generate simulation CP message for RATIO */
	RTC_SC_EVENT_SIMUL2, /* to generate simulation CP message for REGIST */
	RTC_SC_EVENT_MAX
};

extern void TRACE_Printf_Sio(const char *, ...);

#ifdef CNEON_COMMON
/* It's to hook up prm sleep clock cal function to be handled in rtc API.
 * chipset_sw/broadcom_sw/glue_rhea/prm/
 */
extern void prm_sleep_clock_calibration_handler(unsigned int ratio);
#endif

/* ---- Function Declarations ----------------------------------------- */

typedef void (*RTCSC_Config) (UInt32 resolution_ppb, UInt32 duration_ms);

/**
	This function is used to send ratio event to RTC CAL task

	@param		ratio (in) UInt32 for ratio
	@param		temp (in) UInt16 for temp
	@note

**/
void RTC_SC_RatioEvent(UInt32 ratio, UInt16 temp);

/**
	This function is used to register config_func to RTC Slow Clock component

	@param		config_func (in) RTCSC_Config for config_func
	@note

**/
void RTC_SC_RegisterEvent(RTCSC_Config config_func);

/**
	This function is used to send config info to modem/CAL

	@param		threshold (in) UInt32 for threshold
	@param		duration (in) UInt32 for duration
	@note

**/
Boolean RTCSC_sendCalConfig(UInt32 threshold, UInt32 duration);

/**
	This function is used to send simulation msg to CP for generating ratio event

	@param		ratio (in) UInt32 for ratio
	@param		temp (in) UInt16 for temp
	@note

**/
void RTC_SC_GenSimulMsg1(UInt32 ratio, UInt16 temp);

/**
	This function is used to send simulation msg to CP for generating register event

	@note

**/
void RTC_SC_GenSimulMsg2(void);

/**
	This function is used to initialize RPC interface

	@note

**/
void RTCSC_InitRpc(void);

#endif
