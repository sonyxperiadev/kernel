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
*   @file   csl_caph_cfifo.h
*
*   @brief  This file contains the definition for csl cfifo layer
*
****************************************************************************/


#ifndef _CSL_CAPH_CFIFO_
#define _CSL_CAPH_CFIFO_
#include "csl_caph.h"
/**
*
*  @brief  initialize the caph cfifo block
*
*  @param   baseAddress  (in) mapped address of the caph cfifo block to be initialized
*
*  @return 
*****************************************************************************/
void csl_caph_cfifo_init(UInt32 baseAddress);
/**
*
*  @brief  deinitialize the caph cfifo 
*
*  @param  void
*
*  @return void
*****************************************************************************/
void csl_caph_cfifo_deinit(void);
/**
*
*  @brief  assign a free caph cfifo buffer based on data information
*
*  @param   dataFormat  (in) format of the data to be transffered through this buffer
*  @param   sampleRate  (in) Sample Rate of the data to be transffered through this buffer
*
*  @return CSL_CAPH_CFIFO_FIFO_e   assigned CFIFO buffer
*****************************************************************************/
CSL_CAPH_CFIFO_FIFO_e csl_caph_cfifo_obtain_fifo(CSL_CAPH_DATAFORMAT_e dataFormat,
                                                 CSL_CAPH_CFIFO_SAMPLERATE_e sampleRate);

/**
*
*  @brief  assign a free caph cfifo buffer based on data information
*
*  @param   dataFormat  (in) format of the data to be transffered through this buffer
*  @param   sampleRate  (in) Sample Rate of the data to be transffered through this buffer
*
*  @return CSL_CAPH_CFIFO_FIFO_e   assigned CFIFO buffer
*****************************************************************************/
CSL_CAPH_CFIFO_FIFO_e csl_caph_cfifo_ssp_obtain_fifo(CSL_CAPH_DATAFORMAT_e dataFormat,
                                                 CSL_CAPH_CFIFO_SAMPLERATE_e sampleRate);

/**
*
*  @brief  get a fifo's threshold
*  
*  @param   fifo  (in) CFIFO fifo
*
*  @return UInt16   threshold of the CFIFO buffer
*****************************************************************************/
UInt16 csl_caph_cfifo_get_fifo_thres(CSL_CAPH_CFIFO_FIFO_e fifo);

/**
*
*  @brief  release a caph cfifo buffer back to the pool
*
*  @param   fifo  (in) caph cfifo
*
*  @return void
*****************************************************************************/
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
*****************************************************************************/
void csl_caph_cfifo_config_fifo(CSL_CAPH_CFIFO_FIFO_e fifo, 
                                CSL_CAPH_CFIFO_DIRECTION_e  direction, 
                                UInt16 threshold);
/**
*
*  @brief  Get the caph cfifo buffer address 
*
*  @param   fifo  (in) caph cfifo id
*
*  @return UInt32   buffer address
*****************************************************************************/
UInt32 csl_caph_cfifo_get_fifo_addr(CSL_CAPH_CFIFO_FIFO_e fifo);

/**
*
*  @brief  start the transferring on the caph cfifo buffer 
*
*  @param   fifo  (in) caph cfifo
*
*  @return void
*****************************************************************************/
void csl_caph_cfifo_start_fifo(CSL_CAPH_CFIFO_FIFO_e fifo);

/**
*
*  @brief  stop the data tranffering on the caph cfifo buffer 
*
*  @param   fifo  (in) caph cfifo
*
*  @return void
*****************************************************************************/
void csl_caph_cfifo_stop_fifo(CSL_CAPH_CFIFO_FIFO_e fifo);

/**
*
*  @brief  read caph cfifo buffer status
*
*  @param   fifo  (in) caph cfifo
*
*  @return  status
*****************************************************************************/
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
*****************************************************************************/
UInt16 csl_caph_cfifo_read_fifo(CSL_CAPH_CFIFO_FIFO_e csl_fifo, 
                                UInt32* data, 
                                UInt16 size);
#endif // _CSL_CAPH_CFIFO_

