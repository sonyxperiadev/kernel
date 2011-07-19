/*****************************************************************************
*
*    (c) 2007-2008 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or
* its licensors, and may only be used, duplicated, modified or distributed
* pursuant to the terms and conditions of a separate, written license
* agreement executed between you and Broadcom (an "Authorized License").
* Except as set forth in an Authorized License, Broadcom grants no license
* (express or implied), right to use, or waiver of any kind with respect to
* the Software, and Broadcom expressly reserves all rights in and to the
* Software and all intellectual property rights therein.
* IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS
* SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
* ALL USE OF THE SOFTWARE.  
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*    IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
*    FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
*    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU
*    ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
*****************************************************************************/
//
// Description:  This include file contains the function prototypes for ...
//
// Reference:  N/A
//
// $RCSfile: timer.h $
// $Revision: 1.2 $
// $Date: 1999/12/22 11:48:25 $
// $Author: awong $
//
//******************************** History *************************************
//
// $Log: timer.h $
// Revision 1.2  1999/12/22 11:48:25  awong
// Revision 1.1  1999/12/21 10:57:45  awong
// Initial revision
//
//******************************************************************************

//!
//! \file   timer.h
//! \brief  Timer driver header file.
//!

/* Requires the following header files before its inclusion in a c file
#include "mobcom_types.h"
#include "consts.h"
#include "ostypes.h"
#ifndef WIN32
#include "memmap.h"
#endif
*/
#ifndef _INC_DEV_TIMER_H_
#define _INC_DEV_TIMER_H_

#if defined(NUCLEUS_USES_GPTIMER)
extern UInt32 g_system_time_ext;

#else
#if !defined WIN32 && !defined UHT_HOST_BUILD
#define SMT_CLOCK_REG			(UInt32 volatile *)(SMT_TIMER_BASE + 16)
#define SMT_DOWN_COUNTER_REG	(UInt16 volatile *)(SMT_TIMER_BASE + 4 + WORD_1_OFFSET)
#define SMT_COMPARE_REG			(UInt32 volatile *)(SMT_TIMER_BASE + 12)
#endif

/* Setting this to "1" will enable code for switching from 13M (at boot) to 32kHz. This is to be used for old chips only.
   Setting this to "0" is used for the chips for which 32kHz is available right from the start */
#define USE_SMT_13MHz 0

#if USE_SMT_13MHz
#define SYSTEM_TICKS_IF13M	(794)		// based on 812.5K clock
#endif /* USE_SMT_13MHz */
#define SYSTEM_TICKS_IF32K	(32)		// based on 32768Hz clock
#endif /* NUCLEUS_USES_GPTIMER */

#if USE_SMT_13MHz
typedef enum
{
	CLOCKSTATE_WAIT,					// Wait for clock change delay
	CLOCKSTATE_13MHZ,					// Clock left on 13MHz clock
	CLOCKSTATE_32KHZ					// Clock switched to 32kHz clock
} ClockState_t;
#endif /* USE_SMT_13MHz */

extern UInt32 ARM_DisableIRQFIQ(void);
extern void ARM_RecoverIRQFIQ(UInt32);

//******************************************************************************
// Global Macros
//******************************************************************************

//******************************************************************************
// Global Typedefs
//******************************************************************************

//******************************************************************************
// Global Function Prototypes
//******************************************************************************
void TIMER_Init( void );				// This function initializes the timer device driver

void TIMER_Enable( void );				// This function enables the timer

void TIMER_Disable( void );				// This function disables the timer

UInt32	TIMER_GetMilliSeconds(void);	// Return the value of system timer in milliseconds.

#if !defined WIN32 && !defined UHT_HOST_BUILD
__inline Boolean TIMER_Is32kHzReady( void )		// TRUE, if 32kHz clock is available and ready
{
#if USE_SMT_13MHz
	extern ClockState_t clock_state;

	return ( clock_state == CLOCKSTATE_32KHZ );
#else
	return TRUE;
#endif /* USE_SMT_13MHz */
}
#else
// definition still needed here for Windows implementation
Boolean TIMER_Is32kHzReady( void );	
#endif

Ticks_t TIMER_GetValue( void );

Ticks_t TIMER_GetAccuValue( void );

#if defined(NUCLEUS_USES_GPTIMER)
#else
Ticks_t TIMER_CalcAccuValue(UInt32 ticks, UInt16 downcnt);

void TIMER_GetHistValues(UInt32 *msv, UInt16 *lsv);

void TIMER_CalcHistValues(UInt32 *msv, UInt16 *lsv);
#endif /* NUCLEUS_USES_GPTIMER */

UInt32 TIMER_GetDownCountValue( void );

void TIMER_Shutdown( void );			// This function shutsdown the OS timer.

Ticks_t TIMER_GetTicksToNextTimeout( void );	// This function returns the number of ticks

#endif
