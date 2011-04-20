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
* @file   chal_audiovipath.h
* @brief  Athena Audio cHAL interface to voice input path
*
******************************************************************************/
#ifndef _CHAL_AUDIOVIPATH_H_
#define _CHAL_AUDIOVIPATH_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup cHAL_Interface 
 * @{
 */

#define NUM_OF_ADC_VOICE_COEFF		35
#define COEF_PER_GROUP			5	///< VOICE_DAC/VOICE_ADC

/**
*  @brief  This function is the Init entry point for CHAL voice input path 
*			which should be the	first function to call.
*
*  @param  baseAddr		(in)  the base address of this voice input.
*
*  @return CHAL_HANDLE	(out) CHAL handle for this voice input path instance
*
******************************************************************************/
CHAL_HANDLE chal_audiovipath_Init( cUInt32 baseAddr );


/**
*  @brief  Enable or Disable Voice Input Path
*
*  @param  handle (in) this Voice Input Path instance
*  @param  enable (in) 0(FALSE): to disable; 1(TRUE): to enable
*
*  @return none
*
******************************************************************************/
cVoid chal_audiovipath_Enable(
		CHAL_HANDLE handle,
		Boolean		enable
		);

/**
*  @brief  Enable or Disable Voice Input Path 2
*
*  @param  handle (in) this Voice Input Path 2 instance
*  @param  enable (in) 0(FALSE): to disable; 1(TRUE): to enable
*
*  @return none
*
******************************************************************************/
cVoid chal_audiovipath2_Enable(
		CHAL_HANDLE handle,
		Boolean		enable
		);

/**
*  @brief  Select input source for voice input path
*
*  @param  handle    (in) this Voice Input Path instance
*  @param  input_src (in) input source selection
*
*  @return none
*
******************************************************************************/
cVoid chal_audiovipath_SelectInput(
		CHAL_HANDLE				handle,
		CHAL_AUDIO_MIC_INPUT_en input_source
		);

/**
*  @brief  Select input source for voice input path2
*
*  @param  handle    (in) this Voice Input Path instance
*  @param  input_src (in) input source selection
*
*  @return none
*
******************************************************************************/
cVoid chal_audiovipath2_SelectInput(
		CHAL_HANDLE				handle,
		CHAL_AUDIO_MIC_INPUT_en input_source
		);

/**
*  @brief  Enable HW loopback
*
*  @param  handle   (in) this Voice Input Path instance
*  @param  enable   (in) TRUE: enable loopback; FALSE: disable loopback
*
*  @return none
*
******************************************************************************/
cVoid chal_audiovipath_EnableLoopback (
		CHAL_HANDLE		handle,
		Boolean			enable,
		CHAL_AUDIO_MIC_INPUT_en  mic_in
		);

/**
*  @brief  Set voice input sample rate
*
*  @param  handle (in) this mic instance
*  @param  rate   (in) sample rate
*
*  @return none
*  @note    only 8 KHz and 16 KHz are supported.
*
******************************************************************************/
cVoid chal_audiovipath_SetSampleRate(
		CHAL_HANDLE				handle,
		AUDIO_SAMPLING_RATE_t	samp_rate
		);

/**
*  @brief  Clear FIFO in Audio Input Path
*
*  @param  handle (in) this Audio Input Path instance
*
*  @return none
*
******************************************************************************/
cVoid chal_audiovipath_ClrFifo(CHAL_HANDLE handle);

/**
*  @brief  Set FIFO threshold in Audio Input Path
*
*  @param  handle    (in) this Audio Input Path instance
*  @param  in_thres  (in) input  fifo threshold
*  @param  out_thres (in) output fifo threshold
*
*  @return none
*
******************************************************************************/
cVoid chal_audiovipath_SetFifoThres(
		CHAL_HANDLE handle,
		cUInt16		in_thres,
		cUInt16		out_thres
		);

/**
*  @brief  Read FIFO status from Audio Input Path
*
*  @param  handle   (in) this Audio Input Path instance
*
*  @return cUInt16  (out) The value of fifo status register
*
******************************************************************************/
cUInt16 chal_audiovipath_ReadFifoStatus(CHAL_HANDLE handle);

/**
*  @brief  Read data from voice input FIFO
*
*  @note   This is for ARM read from FIFO.
*
*  @param  handle (in) this Audio Input Path instance
*  @param  src    (in) address of buffer
*  @param  length (in) number of words to put in FIFO
*
*  @return none
*
******************************************************************************/
cVoid chal_audiovipath_ReadFifo(
		CHAL_HANDLE handle,
		cUInt16		*src,
		cUInt16		length
		);

/**
*  @brief  configure IIR filter coefficients in Voice Output Path
*
*  @param  handle  (in) this Voice Output Path instance
*  @param  coeff   (in) pointer to IIR filter coefficients
*
*  @return none
*
******************************************************************************/
cVoid chal_audiovipath_CfgIIRCoeff(
		CHAL_HANDLE		handle,
		const UInt16	*coeff
		);

/** @} */

#ifdef __cplusplus
}
#endif

#endif //_CHAL_AUDIOVIPATH_H_
