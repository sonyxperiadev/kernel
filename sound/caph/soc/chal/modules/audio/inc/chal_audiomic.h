/*******************************************************************************************
Copyright 2009, 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement governing use 
of this software, this software is licensed to you under the terms of the GNU General Public 
License version 2, available at http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software in any way 
with any other Broadcom software provided under a license other than the GPL, without 
Broadcom's express prior written consent.
*******************************************************************************************/

/**
*
* @file   chal_audiomic.h
* @brief  Athena Audio cHAL interface for mic 
*
*****************************************************************************/
#ifndef _CHAL_AUDIOMIC_H_
#define _CHAL_AUDIOMIC_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup cHAL_Interface 
 * @{
 */


/**
*  @brief  This function is the Init entry point for CHAL microphone 
*			which should be the	first function to call.
*
*  @param  syscfg_baseAddr	(in)  the base address of system config.
*  @param  auxmic_baseAddr	(in)  the base address of aux mic.
*  @param  audio_baseAddr	(in)  the base address of digi mic.
*
*  @return CHAL_HANDLE	(out) CHAL handle for this audio input path instance
*
****************************************************************************/
CHAL_HANDLE chal_audiomic_Init( cUInt32 audio_baseAddr );

/**
*  @brief  Enable DMIC path (stereo)
*
*  @param  handle      (in) this mic instance
*  @param  on  (in) TRUE: on.  FALSE: off
*
*  @return none
*
****************************************************************************/
cVoid chal_audiomic_EnableDmic(
		CHAL_HANDLE	handle,
		Boolean		on
		);

/**
*  @brief  Set DMIC clock to 3.25MHz
*
*  @param  handle      (in) this mic instance
*  @param  use  (in) TRUE: 3.25MHz.  FALSE: 1.625MHz
*
*  @return none
*
****************************************************************************/
cVoid chal_audiomic_UseDmicClock3M(
		CHAL_HANDLE	handle,
		Boolean		use
		);

/**
*  @brief  DMIC phase select
*
*  @param  handle      (in) this mic instance
*  @param  pos  (in) TRUE: L+R-.  FALSE: L-R+
*
*  @return none
*
****************************************************************************/
cVoid chal_audiomic_SetDmicPhase(
		CHAL_HANDLE	handle,
		Boolean		pos
		);

/** @} */

#ifdef __cplusplus
}
#endif

#endif //_CHAL_AUDIOMIC_H_
