/****************************************************************************
 * ©1999-2008 Broadcom Corporation
* This program is the proprietary software of Broadcom Corporation and/or its licensors, and may only be used, duplicated, 
* modified or distributed pursuant to the terms and conditions of a separate, written license agreement executed between 
* you and Broadcom (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants no license 
* (express or implied), right to use, or waiver of any kind with respect to the Software, and Broadcom expressly reserves all 
* rights in and to the Software and all intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU 
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE 
* ALL USE OF THE SOFTWARE.	
* 
* Except as expressly set forth in the Authorized License,
* 
* 1.	 This program, including its structure, sequence and organization, constitutes the valuable trade secrets of Broadcom,
*		  and you shall use all reasonable efforts to protect the confidentiality thereof, and to use this information only in connection with your
*		  use of Broadcom integrated circuit products.
* 
* 2.	 TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO 
*		  PROMISES, REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO THE SOFTWARE. 
*		  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A 
*		  PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE 
*		  TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3.	 TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, 
*		  INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR 
*		  INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN 
*		  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY 
*		  NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
****************************************************************************/



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



