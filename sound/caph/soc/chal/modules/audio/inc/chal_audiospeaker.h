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
* @file   chal_audiospeaker.h
* @brief  Athena Audio cHAL interface to onchip speaker driver
*
*****************************************************************************/
#ifndef _CHAL_AUDIOSPEAKER_H_
#define _CHAL_AUDIOSPEAKER_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup cHAL_Interface 
 * @{
 */

/**
*  @brief  This function is the Init entry point for CHAL speaker driver 
*			which should be the	first function to call.
*
*  @param  syscfg_baseAddr	   (in) the base address of system config.
*  @param  dsp_audio_baseAddr  (in) the base address of audio block.
*
*  @return CHAL_HANDLE	(out) CHAL handle for this speaker driver instance
*
****************************************************************************/
CHAL_HANDLE chal_audiospeaker_Init( cUInt32 audio_baseAddr );

/**
*  @brief  Power Ramp UP or Down
*
*  @param  handle    (in) this speaker instance
*  @param  ramp_hex (in) 
*
*  @return none
*
****************************************************************************/
cVoid chal_audiospeaker_SetAnaPwrRamp(
		CHAL_HANDLE handle,
		UInt16		ramp_hex
		);

/** @} */

#ifdef __cplusplus
}
#endif

#endif //_CHAL_AUDIOSPEAKER_H_
