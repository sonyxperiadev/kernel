//*********************************************************************
//
// (c)1999-2011 Broadcom Corporation
//
// Unless you and Broadcom execute a separate written software license agreement governing use of this software,
// this software is licensed to you under the terms of the GNU General Public License version 2,
// available at http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
//
//*********************************************************************


#ifndef _DSP_FEATURE_DEF_H_
#define _DSP_FEATURE_DEF_H_

//TURN ON NO_DSP_TEST ONLY WHEN DSP IS NOT AVAILABLE[GNATS 8874]
//#define NO_DSP_TEST


#include "chip_version.h"

#if CHIPVERSION < CHIP_VERSION(BCM2153,40)
#error	DO NOT SUPPORT 2153C0 and earlier in this file any more.
#endif
#define MULTIBAND_SEARCH



#define NEW_FRAME_ENTRY


#define RFIC_REGISTER_READ

#define DSP_MULTIPLE_TEMPLATE

#ifdef UMTS
#define DSP_UMTS_GPEN_TOGGLE_NEW
#endif //UMTS
#define DSP_USE_ENTRY_MARGIN_LESS
#define DSP_EARLY_WAKEUP_SUPPORT

#define DSP_SEQ_ID_SUPPORT

#if defined(DEVELOPMENT_ONLY)
#define DSP_TRACK_LOGGING
#define DSP_TESTPOINT_TRACE
#endif

#define DSP_CLASS12_SUPPORT

#define DSP_FAST_COMMAND

#define DSP_RX_GPIO_STATE_IN_PG5

#ifndef STACK_WIN32_BUILD

#define OCTABAND_RF

#endif //STACK_WIN32_BUILD

#ifdef OCTABAND_RF
#define DSP_MAX_BANDS	4
#else
#define DSP_MAX_BANDS	2
#endif

#define ARM_MP_ENDIAN_MATCH

#define DSP_EVENT_TIMER_STOP

// To enable ARM side TX power control calulations.
// swchen, 2007-7
#define ARM_PS_DYN_PWRCTRL


#ifdef R6_SUPPORT
#define PS_HO_SUPPORT
#define REPEAT_FACCH
#define REPEAT_SACCH
#define EPC
#endif



#define SOFT_SYMBOL_SCALING
#define SOFT_SYMBOL_SCALING_HEDGE


#define DSP_FEATURE_MULTISLOT_ACCESSBURST	// MobC00089575, swchen, 9/2009

#define	DSP_FEATURE_DUAL_SIM

#endif	//_DSP_FEATURE_DEF_H_



