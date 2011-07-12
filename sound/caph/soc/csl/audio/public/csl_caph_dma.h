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
*   @file   csl_caph_dma.h
*
*   @brief  This file contains the definition for csl dma layer
*
****************************************************************************/


#ifndef _CSL_CAPH_DMA_
#define _CSL_CAPH_DMA_
#include "csl_caph.h"
/**
*
*  @brief  initialize the caph dma block
*
*  @param   baseAddress  (in) mapped address of the caph dma block to be initialized
*
*  @return 
*****************************************************************************/
void csl_caph_dma_init(UInt32 baseAddressDma, UInt32 baseAddressIntc);
/**
*
*  @brief  deinitialize the caph dma 
*
*  @param  void
*
*  @return void
*****************************************************************************/
void csl_caph_dma_deinit(void);
/**
*
*  @brief  obtain a caph dma channel
*
*  @param  void
*
*  @return CSL_CAPH_DMA_CHNL_e
*****************************************************************************/
CSL_CAPH_DMA_CHNL_e csl_caph_dma_obtain_channel(void);
/**
*
*  @brief  obtain a given caph dma channel
*
*  @param  CSL_CAPH_DMA_CHNL_e
*
*  @return CSL_CAPH_DMA_CHNL_e
*****************************************************************************/
CSL_CAPH_DMA_CHNL_e csl_caph_dma_obtain_given_channel(CSL_CAPH_DMA_CHNL_e csl_caph_aadmac_ch);    
/**
*
*  @brief  obtain a connected cfifo from dma for dsp
*
*  @param  CSL_CAPH_DMA_CHNL_e
*
*  @return CSL_CAPH_CFIFO_FIFO_e
*****************************************************************************/
CSL_CAPH_CFIFO_FIFO_e csl_caph_dma_get_csl_cfifo(CSL_CAPH_DMA_CHNL_e dmaCH);
/**
*
*  @brief  release a caph dma channel
*
*  @param  CSL_CAPH_DMA_CHNL_e chnl
*
*  @return void
*****************************************************************************/
void csl_caph_dma_release_channel(CSL_CAPH_DMA_CHNL_e chnl);
/**
*
*  @brief  configure the caph dma channel 
*
*  @param   chnl_config  (in) caph dma channel configuration parameter
*
*  @return void
*****************************************************************************/
void csl_caph_dma_config_channel(CSL_CAPH_DMA_CONFIG_t chnl_config);
/**
*
*  @brief  set the caph dma buf addr 
*
*  @param   chnl_config  (in) caph dma channel configuration parameter
*
*  @return void
*****************************************************************************/
void csl_caph_dma_set_buffer_address(CSL_CAPH_DMA_CONFIG_t chnl_config);
/**
*
*  @brief  switch DMA to access another memory buffer
*
*  @param   chnl_config  (in) caph dma channel configuration parameter
*
*  @return void
*****************************************************************************/
void csl_caph_dma_switch_buffer(CSL_CAPH_DMA_CONFIG_t chnl_config);
/**
*
*  @brief  start the transferring on the caph dma channel 
*
*  @param   chnl  (in) caph dma channel
*
*  @return void
*****************************************************************************/
void csl_caph_dma_start_transfer(CSL_CAPH_DMA_CHNL_e chnl);

/**
*
*  @brief  stop the data tranffering on the caph dma buffer 
*
*  @param   chnl  (in) caph dma channel
*
*  @return void
*****************************************************************************/
void csl_caph_dma_stop_transfer(CSL_CAPH_DMA_CHNL_e chnl);
/**
*
*  @brief  enable caph dma intr 
*
*  @param   chnl  (in) caph dma channel
*  @param   csl_owner  (in) owner of caph dma channel
*
*  @return void
*****************************************************************************/
void csl_caph_dma_enable_intr(CSL_CAPH_DMA_CHNL_e chnl, CSL_CAPH_ARM_DSP_e csl_owner);
/**
*
*  @brief  disable caph dma intr  
*
*  @param   chnl  (in) caph dma channel
*  @param   csl_owner  (in) owner of caph dma channel
*
*  @return void
*****************************************************************************/
void csl_caph_dma_disable_intr(CSL_CAPH_DMA_CHNL_e chnl, CSL_CAPH_ARM_DSP_e csl_owner);
/**
*
*  @brief  enable caph tapin intr 
*
*  @param   chnl  (in) caph src channel
*  @param   csl_owner  (in) owner of this caph src channel
*
*  @return void
*****************************************************************************/
void csl_caph_intc_enable_tapin_intr(CSL_CAPH_SRCM_INCHNL_e chnl, CSL_CAPH_ARM_DSP_e csl_owner);
/**
*
*  @brief  disable caph tapin intr  
*
*  @param   chnl  (in) caph src channel
*  @param   csl_owner  (in) owner of this caph src channel
*
*  @return void
*****************************************************************************/
void csl_caph_intc_disable_tapin_intr(CSL_CAPH_SRCM_INCHNL_e chnl, CSL_CAPH_ARM_DSP_e csl_owner);
/**
*
*  @brief  enable caph tapout intr 
*
*  @param   chnl  (in) caph src channel
*  @param   csl_owner  (in) owner of this caph src channel
*
*  @return void
*****************************************************************************/
void csl_caph_intc_enable_tapout_intr(CSL_CAPH_SRCM_INCHNL_e chnl, CSL_CAPH_ARM_DSP_e csl_owner);
/**
*
*  @brief  disable caph tapout intr  
*
*  @param   chnl  (in) caph src channel
*  @param   csl_owner  (in) owner of this caph src channel
*
*  @return void
*****************************************************************************/
void csl_caph_intc_disable_tapout_intr(CSL_CAPH_SRCM_INCHNL_e chnl, CSL_CAPH_ARM_DSP_e csl_owner);
/**
*
*  @brief  enable caph pcm intr  
*
*  @param   csl_owner  (in) owner of this caph pcm channel
*  @param   csl_sspid  (in) ssp of this caph pcm channel
*
*  @return void
*****************************************************************************/
void csl_caph_intc_enable_pcm_intr(CSL_CAPH_ARM_DSP_e csl_owner, CSL_CAPH_SSP_e csl_sspid);
/**
*
*  @brief  disable caph pcm intr  
*
*  @param   csl_owner  (in) owner of this caph pcm channel
*  @param   csl_sspid  (in) ssp of this caph pcm channel
*
*
*  @return void
*****************************************************************************/
void csl_caph_intc_disable_pcm_intr(CSL_CAPH_ARM_DSP_e csl_owner, CSL_CAPH_SSP_e csl_sspid);
/**
*
*  @brief  get caph dma intr 
*
*  @param   chnl  (in) caph dma channel
*  @param   csl_owner  (in) owner of caph dma channel
*
*  @return Boolean
*****************************************************************************/
Boolean csl_caph_dma_get_intr(CSL_CAPH_DMA_CHNL_e chnl, CSL_CAPH_ARM_DSP_e csl_owner);
/**
*
*  @brief  set caph dma ddr fifo status 
*
*  @param   chnl  (in) caph dma channel
*  @param   status (in) ddr fifo status
*
*  @return void
*****************************************************************************/
void csl_caph_dma_set_ddrfifo_status(CSL_CAPH_DMA_CHNL_e chnl,  
				CSL_CAPH_DMA_CHNL_FIFO_STATUS_e status);
/**
*
*  @brief  read caph dma ddr fifo sw status 
*
*  @param   chnl  (in) caph dma channel
*
*  @return csl ddr fifo sw status
*****************************************************************************/
CSL_CAPH_DMA_CHNL_FIFO_STATUS_e csl_caph_dma_read_ddrfifo_sw_status(
							CSL_CAPH_DMA_CHNL_e chnl)  ;
/**
*
*  @brief  read caph dma ddr fifo status 
*
*  @param   chnl  (in) caph dma channel
*
*  @return csl ddr fifo status
*****************************************************************************/
CSL_CAPH_DMA_CHNL_FIFO_STATUS_e csl_caph_dma_read_ddrfifo_status(
							CSL_CAPH_DMA_CHNL_e chnl)  ;
/**
*
*  @brief  clear caph dma ddr fifo status 
*
*  @param   chnl  (in) caph dma channel
*
*  @return void
*****************************************************************************/
void csl_caph_dma_clear_ddrfifo_status(CSL_CAPH_DMA_CHNL_e chnl);
/**
*
*  @brief  clear caph dma intr 
*
*  @param   chnl  (in) caph dma channel
*  @param   csl_owner  (in) owner of caph dma channel
*
*  @return void
*****************************************************************************/
void csl_caph_dma_clear_intr(CSL_CAPH_DMA_CHNL_e chnl, CSL_CAPH_ARM_DSP_e csl_owner);
/**
*
*  @brief  process caph dma callback 
*
*  @param   void
*
*  @return void
*****************************************************************************/
void csl_caph_dma_process_interrupt(void);

/**
*
*  @brief  get the recorded status of a DMA channel 
*
*  @param   chnl  (in) caph dma channel
*
*  @return channel status
*****************************************************************************/
CSL_CAPH_DMA_CHNL_FIFO_STATUS_e csl_caph_dma_get_ddrfifo_status(CSL_CAPH_DMA_CHNL_e chnl);

/**
*
*  @brief  Clear the recorded status of a DMA channel 
*
*  @param   chnl  (in) caph dma channel
*
*  @return 
*****************************************************************************/
void csl_caph_dma_clr_ddrfifo_status(CSL_CAPH_DMA_CHNL_e chnl);

/**
*
*  @brief  read the dma req count
*
*  @param   chnl  (in) caph dma channel
*
*  @return reqcount readings
*****************************************************************************/

UInt8 csl_caph_dma_read_reqcount(CSL_CAPH_DMA_CHNL_e chnl);

/**
*
*  @brief  read current mem pointer
*
*  @param   chnl  (in) caph dma channel
*
*  @return current mem pointer
*****************************************************************************/

UInt16 csl_caph_dma_read_currmempointer(CSL_CAPH_DMA_CHNL_e chnl);

/**
*
*  @brief  read dma channel time stamp
*
*  @param   chnl  (in) caph dma channel
*
*  @return time stamp
*****************************************************************************/

UInt32 csl_caph_dma_read_timestamp(CSL_CAPH_DMA_CHNL_e chnl);
#endif // _CSL_CAPH_DMA_

