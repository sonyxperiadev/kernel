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
* @file   chal_audiomixertap.h
* @brief  Athena Audio cHAL interface to mixer tap
*
******************************************************************************/
#ifndef _CHAL_AUDIOMIXERTAP_H_
#define _CHAL_AUDIOMIXERTAP_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup cHAL_Interface 
 * @{
 */

#define CHAL_AUDIOMIXERTAP_FIFO_SIZE		128


/**
*  @brief  This function is the Init entry point for CHAL audio mixer tap
*			which should be the	first function to call.
*
*  @param  baseAddr		(in)  the base address of this mixer tap.
*
*  @return CHAL_HANDLE	(out) CHAL handle for this audio mixer tap instance
*  @note:  we only need one global Init( ) in audio cHAL.
*
******************************************************************************/
CHAL_HANDLE chal_audiomixertap_Init(cUInt32 baseAddr);


/**
*  @brief  Set sample rate in NB MIXER TAP
*
*  @param  handle    (in) this MIXER TAP instance
*  @param  samp_rate (in) 8KHz or 16KHz
*
*  @return none
*
******************************************************************************/
cVoid chal_audiomixertap_SetNbSampleRate(
		CHAL_HANDLE				handle,
		AUDIO_SAMPLING_RATE_t	samp_rate
		);

/**
*  @brief  Set sample rate in WB MIXER TAP
*
*  @param  handle    (in) this MIXER TAP instance
*  @param  samp_rate (in) 48KHz or 44.1KHz
*
*  @return none
*
******************************************************************************/
cVoid chal_audiomixertap_SetWbSampleRate(
		CHAL_HANDLE				handle,
		AUDIO_SAMPLING_RATE_t	samp_rate
		);

/**
*  @brief  Set gain in WB MIXER TAP
*
*  @param  handle          (in) this MIXER TAP instance
*  @param  left_gain_hex   (in) mixer tap left channel gain in hex value
*  @param  right_gain_hex  (in) mixer tap right channel gain in hex value
*
*  @return none
*
******************************************************************************/
cVoid chal_audiomixertap_SetWbGain(
		CHAL_HANDLE handle,
		cUInt16		left_gain_hex,
		cUInt16		right_gain_hex
		);

/**
*  @brief  Enable DMA in WB mixer Tap
*
*  @param  handle (in) this WB  mixer Tap instance
*  @param  enable (in) TURE - enable DMA.  FALSE: disable.
*
*  @return none
*
******************************************************************************/
cVoid chal_audiomixertap_EnableWbDma(
		CHAL_HANDLE handle,
		Boolean		enable
		);

/**
*  @brief  Disable audio interrupt in WB mixer Tap
*
*  @param  handle (in) this WB mixer Tap instance
*  @param  enable (in) TRUE: disable.
*
*  @return none
*
******************************************************************************/
cVoid chal_audiomixertap_DisableWbAudInt(
		CHAL_HANDLE handle,
		Boolean		disable
		);

/**
*  @brief  Read BT Taps wide band interrupt flag
*
*  @param  handle  (in) this WB mixer Tap instance
*
*  @return cUInt16 (out) interrupt flag
*
******************************************************************************/
cUInt16 chal_audiomixertap_ReadWbIntFlag( CHAL_HANDLE handle );

/**
*  @brief  Clear BT Taps wide band interrupt flag
*
*  @param  handle (in) this WB mixer Tap instance
*
*  @return none
*
******************************************************************************/
cVoid chal_audiomixertap_ClearWbIntFlag( CHAL_HANDLE handle );

/**
*  @brief  Read BT Taps narrow band interrupt flag
*
*  @param  handle  (in) this NB mixer Tap instance
*
*  @return cUInt16 (out) interrupt flag
*
******************************************************************************/
cUInt16 chal_audiomixertap_ReadNbIntFlag( CHAL_HANDLE handle );

/**
*  @brief  Clear BT Taps narrow band interrupt flag
*
*  @param  handle (in) this NB mixer Tap instance
*
*  @return none
*
******************************************************************************/
cVoid chal_audiomixertap_ClearNbIntFlag( CHAL_HANDLE handle );

/**
*  @brief  Swap left and right channel in WB mixer Tap
*
*  @param  handle (in) this WB mixer Tap instance
*  @param  enable (in) TRUE: swap.
*
*  @return none
*
******************************************************************************/
cVoid chal_audiomixertap_SwapWbLeftRight(
		CHAL_HANDLE handle,
		Boolean		swap
		);

/**
*  @brief  Enable or Disable NB MIXER TAP
*
*  @param  handle (in) this NB MIXER TAP instance
*  @param  enable (in) 0(FALSE): to disable; 1(TRUE): to enable
*
*  @return none
*
******************************************************************************/
cVoid chal_audiomixertap_EnableNb(
		CHAL_HANDLE handle,
		Boolean		enable
		);

/**
*  @brief  Enable or Disable WB MIXER TAP
*
*  @param  handle       (in) this WB MIXER TAP instance
*  @param  enable_wbtap (in) 0(FALSE): to disable; 1(TRUE): to enable WB MIXER TAP
*  @param  enable_dac   (in) 0(FALSE): to disable; 1(TRUE): to also enable HIFI DAC
*
*  @return none
*
******************************************************************************/
cVoid chal_audiomixertap_EnableWb(
		CHAL_HANDLE handle,
		Boolean		enable_wbtap,
		Boolean		enable_dac
		);

/**
*  @brief  Select input source for mixer tap input
*
*  @param  handle    (in) this MIXER TAP instance
*  @param  input_src (in) input source selection
*
*  @return none
*
******************************************************************************/
cVoid chal_audiomixertap_SelectInput(
		CHAL_HANDLE				handle,
		CHAL_AUDIO_MIXER_TAP_en tap,
		CHAL_AUDIO_MIXER_en		input_source
		);

/**
*  @brief  Clear FIFO in NB MIXER TAP
*
*  @param  handle (in) this NB MIXER TAP instance
*  @param  clrbit (in) =TRUE to clear fifo; need to toggle this bit
*
*  @return none
*
******************************************************************************/
cVoid chal_audiomixertap_ClrNbFifo(CHAL_HANDLE handle, Boolean clrbit);

/**
*  @brief  Clear FIFO in WB MIXER TAP ?
*
*  @param  handle (in) this WB MIXER TAP instance
*  @param  clrbit (in) =TRUE to clear fifo; need to toggle this bit
*
*  @return none
*
******************************************************************************/
cVoid chal_audiomixertap_ClrWbFifo(CHAL_HANDLE handle, Boolean clrbit);

/**
*  @brief  Clear BTMIXER_CFG reg in WB MIXER TAP
*
*  @param  handle (in) this WB MIXER TAP instance
*
*  @return none
*
******************************************************************************/
cVoid chal_audiomixertap_ResetWbTap(CHAL_HANDLE handle);

/**
*  @brief  Set FIFO threshold in NB MIXER TAP
*
*  @param  handle (in) this NB MIXER TAP instance
*  @param  thres  (in) fifo threshold.  not exist?
*
*  @return none
*
******************************************************************************/
cVoid chal_audiomixertap_SetNbFifoThres(
		CHAL_HANDLE handle,
		cUInt16		thres
		);

/**
*  @brief  Set FIFO threshold in WB MIXER TAP
*
*  @param  handle (in) this MIXER TAP instance
*  @param  thres  (in) fifo threshold
*
*  @return none
*
******************************************************************************/
cVoid chal_audiomixertap_SetWbFifoThres(
		CHAL_HANDLE handle,
		cUInt16		thres
		);

/**
*  @brief  Read FIFO status from narrow band MIXER TAP
*
*  @param  handle    (in) this MIXER TAP instance
*
*  @return The value of fifo status register
*
******************************************************************************/
cUInt16 chal_audiomixertap_ReadNbFifoStatus(CHAL_HANDLE handle);

/**
*  @brief  Read FIFO status from wideband MIXER TAP
*
*  @param  handle    (in) this MIXER TAP instance
*
*  @return The value of fifo status register
*
******************************************************************************/
cUInt16 chal_audiomixertap_ReadWbFifoStatus(CHAL_HANDLE handle);

/**
*  @brief  Read wide band MIXER TAP fifo entry count
*
*  @param  handle    (in) this MIXER TAP instance
*
*  @return cUInt16   (out) fifo entry count
*
******************************************************************************/
cUInt16 chal_audiomixertap_ReadWbFifoEntryCnt(CHAL_HANDLE handle);

/**
*  @brief  Read narrow band MIXER TAP fifo entry count
*
*  @param  handle    (in) this MIXER TAP instance
*
*  @return cUInt16   (out) fifo entry count
*
******************************************************************************/
cUInt16 chal_audiomixertap_ReadNbFifoEntryCnt(CHAL_HANDLE handle);

/**
*  @brief  Read from stereo FIFO in NB mixer tap
*
*  @note   This is for ARM read from FIFO.
*
*  @param  handle (in) this MIXER TAP instance
*  @param  src    (in) address of data buffer
*  @param  length (in) number of 32-bit words to put in FIFO
*
*  @return none
*
******************************************************************************/
cVoid chal_audiomixertap_ReadNbFifoStereo(
		CHAL_HANDLE handle,
		cUInt16		*src,
		cUInt16		length
		);

/**
*  @brief  Read data from left channel in NB mixer tap
*
*  @note   This is for ARM read from FIFO.
*
*  @param  handle (in) this MIXER TAP instance
*  @param  src    (in) address of data buffer
*  @param  length (in) number of 16-bit words to put in FIFO
*
*  @return none
******************************************************************************/
cVoid chal_audiomixertap_ReadNbFifoLeft(
		CHAL_HANDLE handle,
		cUInt16		*src,
		cUInt16		length
		);

/**
*  @brief  Read data from right channel in NB mixer tap
*
*  @note   This is for ARM read from FIFO.
*
*  @param  handle (in) this MIXER TAP instance
*  @param  src    (in) address of data buffer
*  @param  length (in) number of 16-bit words to put in FIFO
*
*  @return none
******************************************************************************/
cVoid chal_audiomixertap_ReadNbFifoRight(
		CHAL_HANDLE handle,
		cUInt16		*src,
		cUInt16		length
		);

/**
*  @brief  Read data from stereo channel in WB mixer tap
*
*  @note   This is for ARM read from FIFO.
*
*  @param  handle (in) this MIXER TAP instance
*  @param  src    (in) address of data buffer
*  @param  length (in) number of 32-bit words to put in FIFO
*
*  @return none
******************************************************************************/
cVoid chal_audiomixertap_ReadWbFifo(
		CHAL_HANDLE handle,
		cUInt32		*src,
		cUInt16		length
		);

/**
*  @brief  get Mixer Tap Wideband FIFO address
*
*  @param  handle  (in) this MIXER TAP instance
*
*  @return UInt32
*
****************************************************************************/
cUInt32 chal_audiomixertap_getAFIFO_addr( CHAL_HANDLE	handle );


/** @} */

#ifdef __cplusplus
}
#endif

#endif //_CHAL_AUDIOMIXERTAP_H_
