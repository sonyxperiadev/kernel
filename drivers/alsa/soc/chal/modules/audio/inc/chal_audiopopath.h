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
* @file   chal_audiopopath.h
* @brief  Athena Audio cHAL interface to poly ringer path
*
*****************************************************************************/
#ifndef _CHAL_AUDIOPOPATH_H_
#define _CHAL_AUDIOPOPATH_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup cHAL_Interface 
 * @{
 */

#define CHAL_POFIFO_SIZE			128 
#define	NUM_POLY_EQ_COEFF			25
#define	NUM_POLY_IIR_COEFF			25
#define	NUM_POLY_FIR_COEFF			65

/**
*  @brief  This function is the Init entry point for CHAL Polyringer path 
*			which should be the	first function to call.
*
*  @param  baseAddr		(in)  the base address of this Polyringer path.
*
*  @return CHAL_HANDLE	(out) CHAL handle for this Polyringer path instance
*
****************************************************************************/
CHAL_HANDLE chal_audiopopath_Init(cUInt32 baseAddr);


/**
*  @brief  Enable DMA in Polyringer Output Path
*
*  @param  handle (in) this Polyringer Output Path instance
*  @param  enable (in) TURE - enable DMA.  FALSE: disable.
*
*  @return none
*
****************************************************************************/
cVoid chal_audiopopath_EnableDMA(
		CHAL_HANDLE handle,
		Boolean		enable
		);

/**
*  @brief  Set sampling rate Polyringer Output Path
*
*  @param  handle (in) this Audio Output Path instance
*  @param  samp_rate (in) the sampling rate in unit HZ.
*
*  @return 
*  @note: We only support a list of 8 sampling rates,  
*		0000:  8000
*		0001:  12000
*		0010:  16000
*		0011:  24000
*		0100:  32000
*		0101: 48000
*		0110: 11030
*		0111: 22060
*		1000: 44100
****************************************************************************/
cVoid chal_audiopopath_SetSampleRate(
		CHAL_HANDLE				handle,
		AUDIO_SAMPLING_RATE_t	samp_rate
		);

/**
*  @brief  Select Polyringer Output Path Filter
*
*  @param  handle        (in) this Polyringer Output Path instance
*  @param  filter_select (in) the filter type defined in CHAL_AUDIO_FilterType_en.
*
*  @return 
*  @note	Only support IIR in Polyringer Output Path. Not selective in Athena
*
****************************************************************************/
cVoid chal_audiopopath_SelectFilter(
		CHAL_HANDLE					handle,
		CHAL_AUDIO_FILTER_TYPE_en	filter_select
		);


/**
*  @brief  Enable or Disable Polyringer Output Path
*
*  @param  handle (in) this Polyringer Output Path instance
*  @param  enable (in) 0(FALSE): to disable; 1(TRUE): to enable
*
*  @return none
*
****************************************************************************/
cVoid chal_audiopopath_Enable(
		CHAL_HANDLE handle,
		Boolean		enable
		);

/**
*  @brief  Mute or UnMute Polyringer Output Path
*
*  @param  handle (in) this Polyringer Output Path instance
*  @param  mute_left   (in) 0(FALSE): to un-mute; 1(TRUE): to mute
*  @param  mute_right  (in) 0(FALSE): to un-mute; 1(TRUE): to mute
*
*  @return none
*
****************************************************************************/
cVoid chal_audiopopath_Mute(
		CHAL_HANDLE handle,
		Boolean		mute_left,
		Boolean		mute_right
		);

/**
*  @brief  Clear FIFO in Polyringer Output Path
*
*  @param  handle (in) this Polyringer Output Path instance
*
*  @return none
*
****************************************************************************/
cVoid chal_audiopopath_ClrFifo(CHAL_HANDLE handle);

/**
*  @brief  Set FIFO threshold in Polyringer Output Path
*
*  @param  handle    (in) this Polyringer Output Path instance
*  @param  in_thres  (in) input  fifo threshold
*  @param  out_thres (in) output fifo threshold
*
*  @return none
*
****************************************************************************/
cVoid chal_audiopopath_SetFifoThres(
		CHAL_HANDLE handle,
		cUInt16		out_thres,
		cUInt16		in_thres
		);

/**
*  @brief  Read FIFO status in Polyringer Output Path
*
*  @param  handle    (in) this Polyringer Output Path instance
*
*  @return The value of fifo status register.
*  @note:   SW will likely not use this function.
*
****************************************************************************/
cUInt16 chal_audiopopath_ReadFifoStatus(CHAL_HANDLE handle);

/**
*  @brief  Fills FIFO with data
*
*  @note   This is for ARM writes to FIFO. Usually SW shall enable DMA to fill FIFO.
*          Audio Driver (CSL) can set up DMA to fill FIFO.
*
*  @param  handle (in) this Polyringer Output Path instance
*  @param  src    (in) [31:16]: right channel sample, [15:00]: left channel sample. 
*  @param  length (in) number of 32-bit words to put in FIFO
*
*  @return none
*
****************************************************************************/
cVoid chal_audiopopath_WriteFifo(
		CHAL_HANDLE handle,
		cUInt32		*src,
		cUInt16		length
		);


/**
*  @brief  Set Left/Right slope gain in Polyringer Output Path
*
*  @param  handle      (in) this Polyringer Output Path instance
*  @param  left_gain  (in) slope gain in Left channel output. milliB unit.
*  @param  right_gain (in) slope gain in Right channel output. milliB unit.
*
*  @return none
*
****************************************************************************/
cVoid chal_audiopopath_SetSlopeGain(
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
cVoid chal_audiopopath_SetSlopeGainLeftHex(
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
cVoid chal_audiopopath_SetSlopeGainRightHex(
		CHAL_HANDLE handle,
		cInt32		gain_hex
		);

/**
*  @brief  Configure 5-band EQ band gains in Polyringer Output Path
*
*  @param  handle  (in) this Polyringer Output Path instance
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
cVoid chal_audiopopath_SetAEQGain(
		CHAL_HANDLE		handle,
		const cUInt16	*gains
		);

/**
*  @brief  Configure 5-band EQ filter coefficients in Polyringer Output Path
*
*  @param  handle  (in) this Polyringer Output Path instance
*  @param  coeff   (in) pointer to 5-band IIR coefficients.
*
*  @return none
*
****************************************************************************/
cVoid chal_audiopopath_CfgAEQCoeff(
		CHAL_HANDLE		handle,
		const cUInt16	*coeff
		);

// Don't support FIR in Polyringer Output Path
//cVoid chal_audiopopath_cfg_afir_coeff(CHAL_HANDLE handle, const cUInt16 *coeff);

/**
*  @brief  configure IIR filter coefficients in Polyringer Output Path
*
*  @param  handle  (in) this Polyringer Output Path instance
*  @param  coeff   (in) pointer to IIR filter coefficients
*
*  @return none
****************************************************************************/
cVoid chal_audiopopath_CfgPIIRCoeff(
		CHAL_HANDLE		handle,
		const cUInt16	*coeff
		);

/**
*  @brief  get POLY FIFO address
*
*  @param  handle  (in) this Polyringer Output Path instance
*
*  @return UInt32
*
****************************************************************************/
cUInt32 chal_audiopopath_getPFIFO_addr( CHAL_HANDLE	handle );


/** @} */

#ifdef __cplusplus
}
#endif

#endif //_CHAL_AUDIOPOPATH_H_
