/**************************************************************************
 * Copyright 2009, 2010 Broadcom Corporation.  All rights reserved.          */
/*                                                                        */
/*     Unless you and Broadcom execute a separate written software license*/
/*     agreement governing use of this software, this software is licensed*/
/*     to you under the terms of the GNU General Public License version 2 */
/*     (the GPL), available at                                            */
/*                                                                        */
/*     http://www.broadcom.com/licenses/GPLv2.php                         */
/*                                                                        */
/*     with the following added to such license:                          */
/*                                                                        */
/*     As a special exception, the copyright holders of this software give*/
/*     you permission to link this software with independent modules, and */
/*     to copy and distribute the resulting executable under terms of your*/
/*     choice, provided that you also meet, for each linked             */
/*     independent module, the terms and conditions of the license of that*/
/*     module.An independent module is a module which is not derived from */
/*     this software.  The special exception does not apply to any        */
/*     modifications of the software.                                     */
/*                                                                        */
/*     Notwithstanding the above, under no circumstances may you combine  */
/*     this software in any way with any other Broadcom software provided */
/*     under a license other than the GPL,                                */
/*     without Broadcom's express prior written consent.                  */
/*                                                                        */
/**************************************************************************/
/**
*
*   @file   csl_caph_cfifo.h
*
*   @brief  This file contains the definition for csl cfifo layer
*
****************************************************************************/
#ifndef _CSL_CAPH_CFIFO_
#define _CSL_CAPH_CFIFO_
/* total ring buffer size for cfifo. make sure sync with cfifo config table */
#define CSL_CFIFO_TOTAL_SIZE 0x2000
/**
* CAPH CFIFO Data Sample Rate
******************************************************************************/
enum CSL_CAPH_CFIFO_SAMPLERATE_e {
	CSL_CAPH_SRCM_UNDEFINED,
	CSL_CAPH_SRCM_8KHZ,
	CSL_CAPH_SRCM_16KHZ,
	CSL_CAPH_SRCM_48KHZ,
};

#define CSL_CAPH_CFIFO_SAMPLERATE_e enum CSL_CAPH_CFIFO_SAMPLERATE_e

/**
* CAPH CFIFO FIFO buffer direction: IN: DDR->CFIFO, OUT: CFIFO->DDR
******************************************************************************/
enum CSL_CAPH_CFIFO_DIRECTION_e {
	CSL_CAPH_CFIFO_OUT = 0x00,
	CSL_CAPH_CFIFO_IN = 0x01,
};

#define CSL_CAPH_CFIFO_DIRECTION_e enum CSL_CAPH_CFIFO_DIRECTION_e

/**
* CAPH CFIFO FIFO buffer table
******************************************************************************/
struct CSL_CFIFO_TABLE_t {
	CSL_CAPH_CFIFO_FIFO_e fifo;
	UInt16 address;
	UInt16 size;
	UInt16 threshold;
	UInt8 owner;
	UInt8 status;
	CSL_CAPH_DMA_CHNL_e dmaCH;
};

#define CSL_CFIFO_TABLE_t struct CSL_CFIFO_TABLE_t

/**
*
*  @brief  initialize the caph cfifo block
*
*  @param   baseAddress  (in) mapped address of the caph cfifo block to be initialized
*
*  @return
*/
void csl_caph_cfifo_init(UInt32 baseAddress);
/**
*
*  @brief  deinitialize the caph cfifo
*
*  @param  void
*
*  @return void
*/
void csl_caph_cfifo_deinit(void);
/**
*
*  @brief  assign a free caph cfifo buffer based on data information
*
*  @param   dataFormat  (in) format of the data to be transffered through this buffer
*  @param   sampleRate  (in) Sample Rate of the data to be transffered through this buffer
*
*  @return CSL_CAPH_CFIFO_FIFO_e   assigned CFIFO buffer
*/
CSL_CAPH_CFIFO_FIFO_e csl_caph_cfifo_obtain_fifo(CSL_CAPH_DATAFORMAT_e
		dataFormat,
		CSL_CAPH_CFIFO_SAMPLERATE_e
		sampleRate);

/**
*
*  @brief  assign a free caph cfifo buffer based on data information
*
*  @param   dataFormat  (in) format of the data to be transffered through this buffer
*  @param   sampleRate  (in) Sample Rate of the data to be transffered through this buffer
*
*  @return CSL_CAPH_CFIFO_FIFO_e   assigned CFIFO buffer
*/
CSL_CAPH_CFIFO_FIFO_e csl_caph_cfifo_ssp_obtain_fifo(CSL_CAPH_DATAFORMAT_e
		dataFormat,
		CSL_CAPH_CFIFO_SAMPLERATE_e
		sampleRate);

/**
*
*  @brief  get a fifo's threshold
*
*  @param   fifo  (in) CFIFO fifo
*
*  @return UInt16   threshold of the CFIFO buffer
*/
UInt16 csl_caph_cfifo_get_fifo_thres(CSL_CAPH_CFIFO_FIFO_e fifo);

/**
*
*  @brief  release a caph cfifo buffer back to the pool
*
*  @param   fifo  (in) caph cfifo
*
*  @return void
*/
void csl_caph_cfifo_release_fifo(CSL_CAPH_CFIFO_FIFO_e fifo);
/**
*
*  @brief  configure the caph cfifo
*
*  @param   fifo  (in) caph cfifo buffer
*  @param   direction  (in) caph cfifo buffer direction
*  @param   threshold  (in) caph cfifo buffer threshold
*
*  @return void
*/
void csl_caph_cfifo_config_fifo(CSL_CAPH_CFIFO_FIFO_e fifo,
		CSL_CAPH_CFIFO_DIRECTION_e direction,
		UInt16 threshold);

/**
*
*  @brief  Get the caph cfifo buffer address
*
*  @param   fifo  (in) caph cfifo id
*
*  @return UInt32   buffer address
*/
UInt32 csl_caph_cfifo_get_fifo_addr(CSL_CAPH_CFIFO_FIFO_e fifo);

/**
*
*  @brief  start the transferring on the caph cfifo buffer
*
*  @param   fifo  (in) caph cfifo
*
*  @return void
*/
void csl_caph_cfifo_start_fifo(CSL_CAPH_CFIFO_FIFO_e fifo);

/**
*
*  @brief  stop the data tranffering on the caph cfifo buffer
*
*  @param   fifo  (in) caph cfifo
*
*  @return void
*/
void csl_caph_cfifo_stop_fifo(CSL_CAPH_CFIFO_FIFO_e fifo);

/**
*
*  @brief  read caph cfifo buffer status
*
*  @param   fifo  (in) caph cfifo
*
*  @return  status
*/
UInt32 csl_caph_cfifo_read_fifo_status(CSL_CAPH_CFIFO_FIFO_e csl_fifo);

/**
*
*  @brief  read caph cfifo buffer data
*
*  @param   fifo  (in) caph cfifo
*  @param   data  (out) pointer to the data buffer
*  @param   size  (in) size of the data buffer
*
*  @return  number of data read
*/
UInt16 csl_caph_cfifo_read_fifo(CSL_CAPH_CFIFO_FIFO_e csl_fifo,
		UInt32 *data, UInt16 size);
/*
*
* Function Name: CSL_CAPH_CFIFO_FIFO_e csl_caph_cfifo_ssp_obtain_fifo(
* CSL_CAPH_DATAFOMAT_e dataFormat,
* CSL_CAPH_CFIFO_SAMPLERATE_e sampleRate)
*
* Description: Obtain a CAPH CFIFO buffer
*
*/
CSL_CAPH_CFIFO_FIFO_e csl_caph_cfifo_ssp_obtain_fifo(CSL_CAPH_DATAFORMAT_e
		dataFormat,
		CSL_CAPH_CFIFO_SAMPLERATE_e
		sampleRate);

/*
*
*  Function Name:CSL_CAPH_CFIFO_FIFO_e csl_caph_cfifo_get_fifo_by_dma(
*                                          CSL_CAPH_DMA_CHNL_e dmaCH)
*
*  Description: get csl cfifo which is linked to this dma chan for dsp
*
*/
CSL_CAPH_CFIFO_FIFO_e csl_caph_cfifo_get_fifo_by_dma(CSL_CAPH_DMA_CHNL_e dmaCH);

#endif /* _CSL_CAPH_CFIFO_ */
