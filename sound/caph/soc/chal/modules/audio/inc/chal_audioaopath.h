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
* @file   chal_audioaopath.h
* @brief  Athena Audio cHAL interface to stereo audio path.
*
*****************************************************************************/
#ifndef _CHAL_AUDIOAOPATH_H_
#define _CHAL_AUDIOAOPATH_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup cHAL_Interface 
 * @{
 */


#define CHAL_AOFIFO_SIZE			128 
#define	NUM_AUDIO_EQ_COEFF			25
#define	NUM_AUDIO_IIR_COEFF			25
#define	NUM_AUDIO_FIR_COEFF			65


/**
*  @brief  This function is the Init entry point for CHAL audio output path 
*			which should be the	first function to call.
*
*  @param  baseAddr		(in)  the base address of this hardware block.
*
*  @return CHAL_HANDLE	(out) CHAL handle for this audio output path instance
*
****************************************************************************/
CHAL_HANDLE chal_audioaopath_Init(cUInt32 baseAddr);


/**
*  @brief  Enable DMA in Audio Output Path
*
*  @param  handle (in) this Audio Output Path instance
*  @param  enable (in) TURE - enable DMA.  FALSE: disable.
*
*  @return none
*
****************************************************************************/
cVoid chal_audioaopath_EnableDMA(
		CHAL_HANDLE handle, 
		Boolean		enable 
		);

/**
*  @brief  Set sample rate on Audio Output Path
*
*  @param  handle    (in) this Audio Output Path instance
*  @param  samp_rate (in) the sampling rate in unit Hertz.
*
*  @return 
*  @note: We only support a list of 8 sampling rates,  
*		0000:  8000
*		0001:  12000
*		0010:  16000
*		0011:  24000
*		0100:  32000
*		0101:  48000
*		0110:  11030
*		0111:  22060
*		1000:  44100
*
****************************************************************************/
cVoid chal_audioaopath_SetSampleRate(
		CHAL_HANDLE					handle,
		AUDIO_SAMPLING_RATE_t		samp_rate
		);

/**
*  @brief  Disable audio interrupt in Audio Output Path
*
*  @param  handle (in) this Audio Output Path instance
*  @param  enable (in) TRUE: disable.
*
*  @return none
*
****************************************************************************/
cVoid chal_audioaopath_DisableAudInt(
		CHAL_HANDLE handle,
		Boolean		disable
		);

/**
*  @brief  Select Audio Output Path Filter
*
*  @param  handle        (in) this Audio Output Path instance
*  @param  filter_select (in) the filter type defined in CHAL_AUDIO_FilterType_en.
*
*  @return 
*  @note	Only support FIR in Audio Output Path. Not selective in Athena.
*
****************************************************************************/
cVoid chal_audioaopath_SelectFilter(
		CHAL_HANDLE					handle,
		CHAL_AUDIO_FILTER_TYPE_en	filter_select
		);


/**
*  @brief Enable or Disable AUDVOC Audio Output Path
*
*  @param  handle (in) this Audio Output Path instance
*  @param  enable (in) 0(FALSE): to disable; 1(TRUE): to enable
*
*  @return none
*
****************************************************************************/
cVoid chal_audioaopath_Enable(
        CHAL_HANDLE handle,
        Boolean     enable
		);

/**
*  @brief Mute or UnMute Audio Output Path
*
*  @param  handle (in) this Audio Output Path instance
*  @param  mute_left   (in) 0(FALSE): to un-mute; 1(TRUE): to mute
*  @param  mute_right  (in) 0(FALSE): to un-mute; 1(TRUE): to mute
*
*  @return none
*
****************************************************************************/
cVoid chal_audioaopath_Mute(
		CHAL_HANDLE handle, 
		Boolean		mute_left, 
		Boolean		mute_right
		);

/**
*  @brief Enable or Disable I2S_Mode in Audio Output Path
*
*  @param  handle (in) this Audio Output Path instance
*  @param  enable (in) 0(FALSE): to disable; 
*                      1(TRUE): to enable. audio path is driven by I2S input.
*  @return none
*
****************************************************************************/
cVoid chal_audioaopath_SetI2SMode(
		CHAL_HANDLE handle,
		Boolean		enable
		);

/**
*  @brief  Clear FIFO in Audio Output Path
*
*  @param  handle (in) this Audio Output Path instance
*
*  @return none
*
****************************************************************************/
cVoid chal_audioaopath_ClrFifo(CHAL_HANDLE handle);

/**
*  @brief  Set FIFO threshold in Audio Output Path
*
*  @param  handle    (in) this Audio Output Path instance
*  @param  in_thres  (in) input  fifo threshold
*  @param  out_thres (in) output fifo threshold
*
*  @return none
*  @note: threshold: 2 samples > 
*
****************************************************************************/
cVoid chal_audioaopath_SetFifoThres(
		CHAL_HANDLE handle,
		cUInt16		out_thres,
		cUInt16		in_thres
		);


/**
*  @brief  Read FIFO statusin Audio Output Path
*
*  @param  handle    (in) this Audio Output Path instance
*
*  @return cUInt16   (out) The value of fifo status register.
*  @note:   SW will likely not use this function.
*
****************************************************************************/
cUInt16 CHAL_AUDIOAOPATH_ReadFifoStatus(CHAL_HANDLE handle);

/**
*  @brief  Fills FIFO with data
*
*  @note   This is for ARM writes to FIFO. Usually SW shall enable DMA to fill FIFO.
*          Audio Driver (CSL) can set up DMA to fill FIFO.
*
*  @param  handle (in) this Audio Output Path instance
*  @param  src    (in) [31:16]: right channel sample, [15:00]: left channel sample.  
*  @param  length (in) number of 32-bits words to put in FIFO
*
*  @return none
*
****************************************************************************/
cVoid chal_audioaopath_WriteFifo(
		CHAL_HANDLE handle,
		cUInt32		*src,
		cUInt16		length
		);


/**
*  @brief  Set Left/Right slope gain in Audio Output Path
*
*  @param  handle     (in) this Audio Output Path instance
*  @param  left_gain  (in) slope gain in Left channel output. milliB unit.
*  @param  right_gain (in) slope gain in Right channel output. milliB unit.
*
*  @return none
*
****************************************************************************/
cVoid chal_audioaopath_SetSlopeGain(
		CHAL_HANDLE handle,
		cInt32		left_gain,
		cInt32		right_gain
		);

/**
*  @brief  Set Left slope gain in hex value in Audio Output Path
*
*  @param  handle    (in) this Audio Output Path instance
*  @param  gain_hex  (in) slope gain in Left channel output. milliB unit.
*
*  @return none
*
****************************************************************************/
cVoid chal_audioaopath_SetSlopeGainLeftHex(
		CHAL_HANDLE handle,
		cInt32		gain_hex
		);

/**
*  @brief  Set Right slope gain in hex value in Audio Output Path
*
*  @param  handle   (in) this Audio Output Path instance
*  @param  gain_hex (in) slope gain in Right channel. milliB unit.
*
*  @return none
*
****************************************************************************/
cVoid chal_audioaopath_SetSlopeGainRightHex(
		CHAL_HANDLE handle,
		cInt32		gain_hex
		);

/**
*  @brief  Configure 5-band EQ band gains in Audio Output Path
*
*  @param  handle  (in) this Audio Output Path instance
*  @param  gains   (in) pointer to 5 band gains.  register values.
*
*  @return none
*  @note: format of gains:
*	UInt16			band1_gain;
*	UInt16			band2_gain;
*	UInt16			band3_gain;
*	UInt16			band4_gain;
*	UInt16			band5_gain;
*
****************************************************************************/
void chal_audioaopath_SetAEQGain(
		CHAL_HANDLE		handle,
		const UInt16	*gains
		);

/**
*  @brief  Configure 5-band EQ filter coefficients in Audio Output Path
*
*  @param  handle  (in) this Audio Output Path instance
*  @param  coeff   (in) pointer to 5-band IIR coefficients.
*
*  @return none
*
****************************************************************************/
void chal_audioaopath_CfgAEQCoeff(
		CHAL_HANDLE		handle,
		const UInt16	*coeff
		);

/**
*  @brief  configure FIR filter coefficients in Audio Output Path
*
*  @param  handle  (in) this Audio Output Path instance
*  @param  coeff   (in) pointer to FIR coefficients
*
*  @return none
*  @note:  Don't have IIR in Audio Output Path
*
****************************************************************************/
void chal_audioaopath_CfgAFIRCoeff(
		CHAL_HANDLE		handle,
		const UInt16	*coeff
		);

/**
*  @brief  get AUDIO FIFO address
*
*  @param  handle  (in) this Audio Output Path instance
*
*  @return UInt32
*
****************************************************************************/
cUInt32 chal_audioaopath_getAFIFO_addr( CHAL_HANDLE	handle );


/** @} */

#ifdef __cplusplus
}
#endif

#endif //_CHAL_AUDIOAOPATH_H_
