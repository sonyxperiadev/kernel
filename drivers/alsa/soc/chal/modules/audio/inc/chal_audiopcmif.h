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
* @file   chal_audiopcmif.h
* @brief  Athena Audio cHAL interface to PCM Interface
*
*****************************************************************************/
#ifndef _CHAL_AUDIOPCMIF_H_
#define _CHAL_AUDIOPCMIF_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup cHAL_Interface 
 * @{
 */

#define CHAL_AUDIOPCMIF_FIFO_SIZE		8 

typedef enum
{
	CHAL_AUDIO_PCMIF_Mode_8KHz,  ///< 0: PCM works in 8K mode. (Bit rate 200K, Data rate 8K)
	CHAL_AUDIO_PCMIF_Mode_User   ///< 1: PCM works in the user defined mode
} CHAL_AUDIO_PCMIF_Mode_en;


/**
*  @brief  This function is the Init entry point for CHAL pcm interface 
*			which should be the	first function to call.
*
*  @param  baseAddr			(in)  the base address of this pcm interface.
*  @param  syscfg_baseAddr	(in)  the base address of syscfg.
*
*  @return CHAL_HANDLE	(out) CHAL handle for this pcm interface instance
*
****************************************************************************/
CHAL_HANDLE chal_audiopcmif_Init( cUInt32 baseAddr, cUInt32 syscfg_baseAddr );



/**
*  @brief  Configure PCM Interface
*
*  @param  handle (in) this PCM Interface instance
*  @param  mode   (in)
*
*  @return none
*
****************************************************************************/
cVoid chal_audiopcmif_SetMode(
		CHAL_HANDLE				handle,
		CHAL_AUDIO_PCMIF_Mode_en mode
		);

/**
*  @brief  Configure user mode in PCM Interface
*
*  @param  handle      (in) this PCM Interface instance
*  @param  bitFactor   (in) 
*  @param  dataFactor  (in) 
*
*  @return none
*
****************************************************************************/
cVoid chal_audiopcmif_CfgUserMode(
		CHAL_HANDLE handle, 
		cUInt16		bitFactor, 
		cUInt16		dataFactor
		);

/**
*  @brief  Enable or Disable PCM Interface
*
*  @param  handle (in) this PCM Interface instance
*  @param  enable (in) 0(FALSE): to disable; 1(TRUE): to enable
*
*  @return none
*
****************************************************************************/
cVoid chal_audiopcmif_Enable(
		CHAL_HANDLE handle,
		Boolean		enable
		);

/**
*  @brief  Clear FIFO in PCM Interface
*
*  @param  handle (in) this Audio Output Path instance
*
*  @return none
*
****************************************************************************/
cVoid chal_audiopcmif_ClrFifo(CHAL_HANDLE handle);

/**
*  @brief  Set FIFO threshold in PCM Interface
*
*  @param  handle    (in) this PCM Interface instance
*  @param  tx_thres  (in) TX fifo threshold
*  @param  rx_thres  (in) RX fifo threshold
*
*  @return none
*
****************************************************************************/
cVoid chal_audiopcmif_SetFifoThres(
        CHAL_HANDLE handle,
		cUInt16		tx_thres,
		cUInt16		rx_thres
		);

/**
*  @brief  Read FIFO statusin PCM Interface
*
*  @param  handle    (in) this PCM Interface instance
*
*  @return The value of fifo status register
*
****************************************************************************/
cUInt16 chal_audiopcmif_ReadFifoStatus(CHAL_HANDLE handle);

/**
*  @brief  Write data to PCM I/F output FIFO
*
*  @param  handle (in) this PCM Interface instance
*  @param  src (in) 
*  @param  length (in) number of words to put in FIFO
*
*  @return none
*
****************************************************************************/
cVoid chal_audiopcmif_WriteFifo(
		CHAL_HANDLE handle, 
		cUInt16		*src, 
		cUInt16		length
		);

/**
*  @brief  Read data from PCM I/F input FIFO
*
*  @param  handle (in) this PCM Interface instance
*  @param  src    (in) address of buffer
*  @param  length (in) number of words to put in FIFO
*
*  @return none
****************************************************************************/
cVoid chal_audiopcmif_ReadFifo(
		CHAL_HANDLE handle,
		cUInt16		*src,
		cUInt16		length
		);

/** @} */

#ifdef __cplusplus
}
#endif

#endif //_CHAL_AUDIOPCMIF_H_
