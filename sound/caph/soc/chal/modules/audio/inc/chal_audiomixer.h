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
* @file   chal_audiomixer.h
* @brief  Athena Audio cHAL interface to mixer
*
******************************************************************************/
#ifndef _CHAL_AUDIOMIXER_H_
#define _CHAL_AUDIOMIXER_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup cHAL_Interface 
 * @{
 */

#define	NUM_MIXER_COEF			60

//! input branches into Mixer
typedef enum
{
	CHAL_AUDIO_MIXER_INPUT_NONE	= 0,	///< Do not select any input path
	CHAL_AUDIO_MIXER_INPUT_AL	= 0x01,	///< Audio Left channel
	CHAL_AUDIO_MIXER_INPUT_AR	= 0x02,	///< Audio Right channel
	CHAL_AUDIO_MIXER_INPUT_PL	= 0x04,	///< Polyringer Left channel
	CHAL_AUDIO_MIXER_INPUT_PR	= 0x08,	///< Polyringer Right channel
	CHAL_AUDIO_MIXER_INPUT_VO	= 0x10,	///< Voice channel
	CHAL_AUDIO_MIXER_INPUT_ALL	= 0x1F	///< select all input path
} CHAL_AUDIO_MIXER_INPUT_en;

typedef struct
{
	UInt16			coeff[NUM_MIXER_COEF];
} CHAL_MIXER_COEF_t;


/**
*  @brief  This function is the Init entry point for CHAL audio mixer 
*			which should be the	first function to call.
*
*  @param  baseAddr		(in)  the base address of this mixer hardware block.
*
*  @return CHAL_HANDLE	(out) CHAL handle for this audio mixer instance
*
******************************************************************************/
CHAL_HANDLE chal_audiomixer_Init(cUInt32 baseAddr);


/**
*  @brief  Add a input source to mixer
*
*  @param  handle  (in) this Mixer instance
*  @param  mixer   (in) the mixer
*  @param  inputs  (in) input source to mixer
*
*  @return none
*
******************************************************************************/
cVoid chal_audiomixer_AddInput(
		CHAL_HANDLE					handle,
		CHAL_AUDIO_MIXER_en			mixer,
		CHAL_AUDIO_MIXER_INPUT_en	input
		);

/**
*  @brief  Remove a input source to mixer
*
*  @param  handle  (in) this Mixer instance
*  @param  mixer   (in) the mixer
*  @param  inputs  (in) input source
*
*  @return none
*
******************************************************************************/
cVoid chal_audiomixer_RemoveInput(
		CHAL_HANDLE					handle,
		CHAL_AUDIO_MIXER_en			mixer,
		CHAL_AUDIO_MIXER_INPUT_en	input
		);

/**
*  @brief  Set mixer output gain
*
*  @param  handle  (in) this Mixer instance
*  @param  mixer   (in) the mixer
*  @param  gain    (in) gain in  milliB
*
*  @return none
******************************************************************************/
cVoid chal_audiomixer_SetGain(CHAL_HANDLE handle, CHAL_AUDIO_MIXER_en mixer, cInt32 gain );

/**
*  @brief  Set mixer IIR number of biquads
*
*  @param  handle   (in) this Mixer instance
*  @param  biquads  (in) number of biquads.
*
*  @return none
*
******************************************************************************/
cVoid chal_audiomixer_SetBiquads(
		CHAL_HANDLE handle,
		cUInt16		biquads
		);

/**
*  @brief  Configure filter coefficients in Mixer
*
*  @param  handle  (in) this Mixer instance
*  @param  mixer   (in) the mixer
*  @param  coeff   (in) pointer to IIR coefficients.
*
*  @return none
*
******************************************************************************/
cVoid chal_audiomixer_CfgIirCoeff(
		CHAL_HANDLE			handle,
		CHAL_AUDIO_MIXER_en mixer,
		const cUInt32		*coeff
		);

/**
*  @brief  Set MPM DGA (gain)
*
*  @param  handle  (in) this Mixer instance
*  @param  mixer   (in) the mixer
*  @param  gain    (in) gain in  milliB
*
*  @return none
*
******************************************************************************/
cVoid chal_audiomixer_SetMpmDga(
		CHAL_HANDLE			handle,
		CHAL_AUDIO_MIXER_en mixer,
		cUInt16				gain
		);

/**
*  @brief  Set MPM DGA (gain) in form of register value
*
*  @param  handle  (in) this Mixer instance
*  @param  mixer   (in) the mixer
*  @param  gain    (in) gain as in register value
*
*  @return none
*
******************************************************************************/
cVoid chal_audiomixer_SetMpmDgaHex(
		CHAL_HANDLE			handle,
		CHAL_AUDIO_MIXER_en mixer,
		cUInt16				gain_hex 
		);

/**
*  @brief  Enable/Disable speaker protection (ALDC)
*
*  @param  handle  (in) this Mixer instance
*  @param  mixer   (in) the mixer
*  @param  enable  (in) TRUE = enable, FALSE = disable
*
*  @return none
*
******************************************************************************/
cVoid chal_audiomixer_EnableAldc(
		CHAL_HANDLE			handle,
		CHAL_AUDIO_MIXER_en mixer,
		Boolean				enable
		);

/**
*  @brief  Set speaker protection (ALDC) IIR coefficient
*
*  @param  handle  (in) this Mixer instance
*  @param  mixer   (in) the mixer
*  @param  n       (in) coefficient
*
*  @return none
*
******************************************************************************/
cVoid chal_audiomixer_SetAldcIirN(
		CHAL_HANDLE			handle,
		CHAL_AUDIO_MIXER_en mixer,
		cUInt16				n
		);

/**
*  @brief  Set speaker protection (ALDC) gain attack and decay
*
*  @param  handle  (in) this Mixer instance
*  @param  mixer   (in) the mixer
*  @param  gain_attack_thres   (in) 
*  @param  gain_attack_step   (in) 
*  @param  gain_attack_slop   (in) 
*  @param  gain_decay_slop   (in) 
*
*  @return none
*
******************************************************************************/
cVoid chal_audiomixer_ConfigAldcAttack(
		CHAL_HANDLE			handle, 
		CHAL_AUDIO_MIXER_en mixer,
		cUInt16				gain_attack_thres,
		cUInt16				gain_attack_step,
		cUInt16				gain_attack_slop,
		cUInt16				gain_decay_slop
		);

/** @} */

#ifdef __cplusplus
}
#endif

#endif //_CHAL_AUDIOMIXER_H_
