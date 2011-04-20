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
* @file   chal_audioaipath.h
* @brief  Athena Audio cHAL interface for input path
*
******************************************************************************/
#ifndef _CHAL_AUDIOAIPATH_H_
#define _CHAL_AUDIOAIPATH_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup cHAL_Interface 
 * @{
 */

#define CHAL_AIFIFO_SIZE			128
#define	NUM_AUDIO_ADC_COEF			50


/**
*  @brief  This function is the Init entry point for CHAL audio input path 
*			which should be the	first function to call.
*
*  @param  baseAddr		(in)  the base address of this audio input.
*
*  @return CHAL_HANDLE	(out) CHAL handle for this audio input path instance
*
*******************************************************************************/
CHAL_HANDLE chal_audioaipath_Init(cUInt32 baseAddr);


/**
*  @brief  Enable DMA in Audio Input Path
*
*  @param  handle	(in) this Audio Input Path instance
*  @param  enable	(in) TURE - enable DMA.  FALSE: disable.
*
*  @return none
*
*******************************************************************************/
cVoid chal_audioaipath_EnableDMA(
        CHAL_HANDLE   handle, 
        Boolean       enable
        );

/**
*  @brief  Enable or disable audio input path
*
*  @param  handle	(in) this Audio Input Path instance
*  @param  enable	(in) 0(FALSE): to disable; 1(TRUE): to enable
*
*  @return none
*
*******************************************************************************/
void chal_audioaipath_Enable(
	   CHAL_HANDLE  handle,
	   Boolean		enable
	   );

/**
*  @brief  Configure the IIR filter coeff of audio input path
*
*  @param  handle	(in) this Audio Input Path instance
*  @param  channels	=1 for mono and =2 for stereo channel
*
*  @return none
*
*******************************************************************************/
void chal_audioaipath_CfgIIRCoeff(
	   CHAL_HANDLE  handle,
	   UInt32		channels
	   );
	   
/**
*  @brief  Select input source for audio input path
*
*  @param  handle    (in) this Audio Input Path instance
*  @param  sourceL (in) mic source selection for L ch
*  @param  sourceR (in) mic source selection for R ch
*
*  @return none
*
******************************************************************************/
void chal_audioaipath_SelectInput(
		CHAL_HANDLE					handle,
		CHAL_AUDIO_MIC_INPUT_en		sourceL,
		CHAL_AUDIO_MIC_INPUT_en		sourceR
		);

/**
*  @brief  Clear FIFO in Audio Input Path
*
*  @param  handle (in) this Audio Input Path instance
*
*  @return none
*
******************************************************************************/
cVoid chal_audioaipath_ClrFifo(CHAL_HANDLE handle);

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
void chal_audioaipath_SetFifoThres(
		CHAL_HANDLE handle,
		cUInt16		in_thres,
		cUInt16		out_thres
		);

/**
*  @brief  Read FIFO statusin Audio Input Path
*
*  @param  handle   (in)  handle to this Audio Input Path instance
*
*  @return cUInt16	(out) The value of fifo status register
*
******************************************************************************/
cUInt16 chal_audioaipath_ReadFifoStatus(CHAL_HANDLE handle);

/**
*  @brief  Read data from audio input FIFO
*
*  @note   This is for ARM read from FIFO.
*
*  @param  handle (in) this Audio Input Path instance
*  @param  dst    (in) source address of data buffer to hold readout
*  @param  cnt (in) number of words to read from FIFO
*
*  @return read cnt
*
******************************************************************************/
cUInt16 chal_audioaipath_ReadFifo(
		CHAL_HANDLE handle,
		cUInt32		*dst,
		cUInt16		length
		);

/**
*  @brief  Read remaining data from audio input FIFO
*
*  @note   This is for ARM read from FIFO.
* 
*  @param  handle (in) this Audio Input Path instance
*
*  @return none
*
******************************************************************************/
void chal_audioaipath_ReadFifoResidue(
		CHAL_HANDLE handle
		);

/**
*  @brief  get AUDIO FIFO address
*
*  @param  handle  (in) this Audio Input Path instance
*
*  @return UInt32
*
****************************************************************************/
cUInt32 chal_audioaipath_getAFIFO_addr( CHAL_HANDLE	handle );

/** @} */

#ifdef __cplusplus
}
#endif

#endif //_CHAL_AUDIOAIPATH_H_
