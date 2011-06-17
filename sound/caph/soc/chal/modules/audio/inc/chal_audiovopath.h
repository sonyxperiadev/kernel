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
* @file   chal_audiovopath.h
* @brief  Athena Audio cHAL interface to voice output path
*
*****************************************************************************/
#ifndef _CHAL_AUDIOVOPATH_H_
#define _CHAL_AUDIOVOPATH_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup cHAL_Interface 
 * @{
 */

#define	NUM_OF_DAC_VOICE_COEFF			25
#define COEF_PER_GROUP			5	///< VOICE_DAC/VOICE_ADC

/**
*  @brief  This function is the Init entry point for CHAL voice output path 
*			which should be the	first function to call.
*
*  @param  baseAddr	   (in)  the base address of this voice output path.
*
*  @return CHAL_HANDLE (out) CHAL handle for this voice output path instance
*
****************************************************************************/
CHAL_HANDLE chal_audiovopath_Init( cUInt32 baseAddr );


/**
*  @brief  Set sample rate in Voice Output Path
*
*  @param  handle (in) this Voice Output Path instance
*  @param  samp_rate (in) 
*
*  @return none
*
****************************************************************************/
cVoid chal_audiovopath_SetSampleRate(
		CHAL_HANDLE					handle,
		AUDIO_SAMPLING_RATE_t		samp_rate
		);

/**
*  @brief  Enable or Disable AUDVOC Voice Output Path
*
*  @param  handle (in) this Voice Output Path instance
*  @param  enable (in) 0(FALSE): to disable; 1(TRUE): to enable
*
*  @return none
*
****************************************************************************/
cVoid chal_audiovopath_Enable(
		CHAL_HANDLE handle,
		Boolean		enable
		);

/**
*  @brief  Mute or UnMute AUDVOC Voice Output Path
*
*  @param  handle (in) this Voice Output Path instance
*  @param  mute   (in) 0(FALSE): to un-mute; 1(TRUE): to mute
*
*  @return none
*
****************************************************************************/
cVoid chal_audiovopath_Mute(
		CHAL_HANDLE handle,
		Boolean		mute
		);

/**
*  @brief  Clear FIFO in Voice Output Path
*
*  @param  handle (in) this Voice Output Path instance
*
*  @return none
*
****************************************************************************/
cVoid chal_audiovopath_ClrFifo(CHAL_HANDLE handle);

/**
*  @brief  Set FIFO threshold in Voice Output Path
*
*  @param  handle  (in) this Voice Output Path instance
*  @param  thres   (in) fifo threshold
*
*  @return none
*
****************************************************************************/
cVoid chal_audiovopath_SetFifoThres(CHAL_HANDLE handle, cUInt16 thres);

/**
*  @brief  Read FIFO status from Voice Output Path
*
*  @param  handle   (in) this Voice Output Path instance
*
*  @return cUInt16  (out) The value of fifo status register
*
****************************************************************************/
cUInt16 chal_audiovopath_ReadFifoStatus(CHAL_HANDLE handle);

/**
*  @brief  Fills FIFO with data
*
*  @note   This is to let ARM writes to FIFO.
*
*  @param  handle (in) this Voice Output Path instance
*  @param  src (in) 
*  @param  length (in) number of words to put in FIFO
*
*  @return none
*
****************************************************************************/
cVoid chal_audiovopath_WriteFifo(
		CHAL_HANDLE handle,
		cUInt16		*src,
		cUInt16		length
		);


/**
*  @brief  Set slope gain in Voice Output Path
*
*  @param  handle (in) this Voice Output Path instance
*  @param  gain  (in) slope gain. mB unit.
*
*  @return none
*
****************************************************************************/
cVoid chal_audiovopath_SetSlopeGain(
		CHAL_HANDLE handle,
		cUInt16		gain
		);

/**
*  @brief  Set slope gain as hex value in Voice Output Path
*
*  @param  handle (in) this Voice Output Path instance
*  @param  gain  (in) slope gain. 0.25 dB unit. hex value, 
*                     This is value directly copy to register.
*
*  @return none
*
****************************************************************************/
cVoid chal_audiovopath_SetSlopeGainHex(
		CHAL_HANDLE handle,
		cUInt16		gain_hex
		);


/**
*  @brief  Configure Vocie filter coefficients in Voice Output Path
*
*  @param  handle  (in) this Voice Output Path instance
*  @param  coeff   (in) pointer to IIR coefficients.
*
*  @return none
*
****************************************************************************/
cVoid chal_audiovopath_CfgIIRCoeff(
		CHAL_HANDLE handle,
		const UInt16 *coeff
		);

/**
*  @brief  Set VCFGR gain in Voice Output Path
*
*  @param  handle (in) this Voice Output Path instance
*  @param  gain   (in) slope gain.  mB unit.
*
*  @return none
*
****************************************************************************/
cVoid chal_audiovopath_SetVCFGR (
		CHAL_HANDLE handle,
		cUInt16		gain
		);

/**
*  @brief  Set VCFGR gain as Hex value in Voice Output Path
*
*  @param  handle (in) this Voice Output Path instance
*  @param  gain   (in) slope gain.  0.25 dB unit. Hex value, This is value directly copy to register.
*
*  @return none
*
****************************************************************************/
cVoid chal_audiovopath_SetVCFGRhex (
		CHAL_HANDLE handle,
		cUInt16		gain_hex
		);

//============================================================================
//
// Function Name: chal_audiovopath_DisableDitheringShuffling
//
// Description:   Disable Shuffling, Disable Dithering
//
//============================================================================
cVoid chal_audiovopath_DisableDitheringShuffling (
		CHAL_HANDLE handle,
		Boolean		enable
		);


/** @} */

#ifdef __cplusplus
}
#endif

#endif //_CHAL_AUDIOVOPATH_H_
