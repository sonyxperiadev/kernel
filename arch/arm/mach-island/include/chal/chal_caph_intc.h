/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement governing use 
of this software, this software is licensed to you under the terms of the GNU General Public 
License version 2, available at http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software in any way 
with any other Broadcom software provided under a license other than the GPL, without 
Broadcom's express prior written consent.
*******************************************************************************************/

/**
*
*   @file   chal_caph_intc.h
*
*   @brief  This file contains the definition for caph intc (AHINTC block) CHA layer
*
****************************************************************************/




#ifndef _CHAL_CAPH_INTC_
#define _CHAL_CAPH_INTC_

#include <plat/chal/chal_types.h>
#include <chal/chal_caph.h>


/**
*
*  @brief  initialize the caph intc 
*
*  @param   baseAddress  (in) mapped address of the caph intc block to be initialized
*
*  @return CHAL_HANDLE
*****************************************************************************/
CHAL_HANDLE chal_caph_intc_init(uint32_t baseAddress);

/**
*
*  @brief  deinitialize the caph intc 
*
*  @param   handle  (in) caph intc block CHAL_HANDLE to be deinitialized
*
*  @return void
*****************************************************************************/
void chal_caph_intc_deinit(CHAL_HANDLE handle);

/**
*
*  @brief  reset the caph intc 
*
*  @param   handle  (in) caph intc block CHAL_HANDLE to be reset
*
*  @return void
*****************************************************************************/
void chal_caph_intc_reset(CHAL_HANDLE handle);

/**
*
*  @brief  enable the caph intc cfifo int 
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   fifo  (in) fifos in CFIFO
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_enable_cfifo_intr(CHAL_HANDLE handle, 
			uint16_t fifo, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  disable the caph intc cfifo int 
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   fifo  (in) fifos in CFIFO
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_disable_cfifo_intr(CHAL_HANDLE handle, 
			uint16_t fifo, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  clear the caph intc cfifo int 
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   fifo  (in) fifos in CFIFO
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_clr_cfifo_intr(CHAL_HANDLE handle, 
			uint16_t fifo, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  read the caph intc cfifo int status
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   owner  (in) owner of the caph intc block
*
*  @return uint16_t
*****************************************************************************/
uint16_t chal_caph_intc_read_cfifo_intr(CHAL_HANDLE handle, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  enable the caph intc cfifo err ints
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   fifo  (in) fifos in CFIFO
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_enable_cfifo_int_err(CHAL_HANDLE handle, 
			uint16_t fifo, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  disable the caph intc cfifo err ints
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   fifo  (in) fifos in CFIFO
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_disable_cfifo_int_err(CHAL_HANDLE handle, 
			uint16_t fifo, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  clear the caph intc cfifo err ints
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   fifo  (in) fifos in CFIFO
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_clr_cfifo_int_err(CHAL_HANDLE handle, 
			uint16_t fifo, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  read the caph intc cfifo err ints status
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   owner  (in) owner of the caph intc block
*
*  @return uint16_t
*****************************************************************************/
uint16_t chal_caph_intc_read_cfifo_int_err(CHAL_HANDLE handle, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  set the caph intc cfifo ints detect level
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   fifo  (in) fifos in CFIFO
*  @param   detectLevel  (in) int detect level
*
*  @return void
*****************************************************************************/
void chal_caph_intc_set_cfifo_int_detect_level(CHAL_HANDLE handle, 
			uint16_t fifo, 
			uint16_t detectLevel);

/**
*
*  @brief  enable the caph intc dma ints
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   fifo  (in) fifos in CFIFO
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_enable_dma_intr(CHAL_HANDLE handle, 
			uint16_t channel, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  disable the caph intc dma ints
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   fifo  (in) fifos in CFIFO
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_disable_dma_intr(CHAL_HANDLE handle, 
			uint16_t channel, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  clear the caph intc dma ints
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   fifo  (in) fifos in CFIFO
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_clr_dma_intr(CHAL_HANDLE handle, 
			uint16_t channel, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  read the caph intc dma ints status
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   owner  (in) owner of the caph intc block
*
*  @return uint16_t
*****************************************************************************/
uint16_t chal_caph_intc_read_dma_intr(CHAL_HANDLE handle, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  enable the caph intc dma err ints
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   fifo  (in) fifos in CFIFO
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_enable_dma_int_err(CHAL_HANDLE handle, 
			uint16_t channel, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  disable the caph intc dma err ints
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   fifo  (in) fifos in CFIFO
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_disable_dma_int_err(CHAL_HANDLE handle, 
			uint16_t channel, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  clear the caph intc dma err ints
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   fifo  (in) fifos in CFIFO
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_clr_dma_int_err(CHAL_HANDLE handle, 
			uint16_t channel, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  read the caph intc dma err ints status
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   owner  (in) owner of the caph intc block
*
*  @return uint16_t
*****************************************************************************/
uint16_t chal_caph_intc_read_dma_int_err(CHAL_HANDLE handle, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  set the caph intc dma ints detect level
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   fifo  (in) fifos in CFIFO
*  @param   detectLevel  (in) int detect level
*
*  @return void
*****************************************************************************/
void chal_caph_intc_set_dma_int_detect_level(CHAL_HANDLE handle, 
			uint16_t channel, 
			uint16_t detectLevel);

/**
*
*  @brief  enable the caph intc ssp ints
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   ssp  (in) ssps in ssp block
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_enable_ssp_intr(CHAL_HANDLE handle, 
			uint8_t ssp, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  disable the caph intc ssp ints
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   ssp  (in) ssps in ssp block
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_disable_ssp_intr(CHAL_HANDLE handle, 
			uint8_t ssp, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  clear the caph intc ssp ints
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   ssp  (in) ssps in ssp block
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_clr_ssp_intr(CHAL_HANDLE handle, 
			uint8_t ssp, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  read the caph intc ssp ints status
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   ssp  (in) ssps in ssp block
*  @param   owner  (in) owner of the caph intc block
*
*  @return uint16_t
*****************************************************************************/
uint16_t chal_caph_intc_read_ssp_intr(CHAL_HANDLE handle, 
			uint8_t ssp, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  enable the caph intc ssp err ints
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   ssp  (in) ssps in ssp block
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_enable_ssp_int_err(CHAL_HANDLE handle, 
			uint8_t ssp, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  disable the caph intc ssp err ints
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   ssp  (in) ssps in ssp block
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_disable_ssp_int_err(CHAL_HANDLE handle, 
			uint8_t ssp, 
			CAPH_ARM_DSP_e owner);
/**
*
*  @brief  clear the caph intc ssp err ints
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   ssp  (in) ssps in ssp block
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_clr_ssp_int_err(CHAL_HANDLE handle, 
			uint8_t ssp, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  read the caph intc ssp err ints status
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   ssp  (in) ssps in ssp block
*  @param   owner  (in) owner of the caph intc block
*
*  @return uint16_t
*****************************************************************************/			
uint16_t chal_caph_intc_read_ssp_int_err(CHAL_HANDLE handle, 
			CAPH_SSP_e ssp, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  set the caph intc ssp ints detect level
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   ssp  (in) ssps in ssp block
*  @param   detectLevel  (in) int detect level
*
*  @return void
*****************************************************************************/
void chal_caph_intc_set_ssp_int_detect_level(CHAL_HANDLE handle, 
			uint8_t ssp, 
			uint8_t detectLevel);

/**
*
*  @brief  enable the caph intc switch int
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_enable_switch_intr(CHAL_HANDLE handle, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  disable the caph intc switch int
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_disable_switch_intr(CHAL_HANDLE handle, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  clear the caph intc switch int
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_clr_switch_intr(CHAL_HANDLE handle, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  read the caph intc switch int status
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   owner  (in) owner of the caph intc block
*
*  @return uint8_t
*****************************************************************************/
uint8_t chal_caph_intc_read_switch_intr(CHAL_HANDLE handle, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  enable the caph intc switch err int
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_enable_switch_int_err(CHAL_HANDLE handle, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  disable the caph intc switch err int
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_disable_switch_int_err(CHAL_HANDLE handle, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  clear the caph intc switch err int
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_clr_switch_int_err(CHAL_HANDLE handle, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  read the caph intc switch err int status
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   owner  (in) owner of the caph intc block
*
*  @return uint8_t
*****************************************************************************/
uint8_t chal_caph_intc_read_switch_int_err(CHAL_HANDLE handle, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  set the caph intc switch ints detect level
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   detectLevel  (in) int detect level
*
*  @return void
*****************************************************************************/
void chal_caph_intc_set_switch_int_detect_level(CHAL_HANDLE handle,
			uint8_t detectLevel);

/**
*
*  @brief  enable the caph intc audioh int
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   fifo      (in) fifos in audioh
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_enable_audioh_intr(CHAL_HANDLE handle, 
			uint8_t fifo, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  disable the caph intc audioh int
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   fifo      (in) fifos in audioh
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_disable_audioh_intr(CHAL_HANDLE handle, 
			uint8_t fifo, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  clear the caph intc audioh int
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   fifo      (in) fifos in audioh
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_clr_audioh_intr(CHAL_HANDLE handle, 
			uint8_t fifo, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  read the caph intc audioh int status
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   owner  (in) owner of the caph intc block
*
*  @return uint8_t
*****************************************************************************/
uint8_t chal_caph_intc_read_audioh_intr(CHAL_HANDLE handle, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  enable the caph intc audioh err int
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   fifo      (in) fifos in audioh
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_enable_audioh_int_err(CHAL_HANDLE handle, 
			uint8_t fifo, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  disable the caph intc audioh err int
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   fifo      (in) fifos in audioh
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_disable_audioh_int_err(CHAL_HANDLE handle, 
			uint8_t fifo, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  clear the caph intc audioh err int
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   fifo      (in) fifos in audioh
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_clr_audioh_int_err(CHAL_HANDLE handle, 
			uint8_t fifo, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  read the caph intc audioh err int status
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   owner  (in) owner of the caph intc block
*
*  @return uint8_t
*****************************************************************************/
uint8_t chal_caph_intc_read_audioh_int_err(CHAL_HANDLE handle, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  set the caph intc audioh ints detect level
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   fifo      (in) fifos in audioh
*  @param   detectLevel  (in) int detect level
*
*  @return void
*****************************************************************************/
void chal_caph_intc_set_audioh_int_detect_level(CHAL_HANDLE handle, 
			uint8_t fifo, 
			uint8_t detectLevel);

/**
*
*  @brief  enable the caph intc tap int
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   channel      (in) channels in taps
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_enable_tap_intr(CHAL_HANDLE handle, 
			uint8_t channel, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  disable the caph intc tap int
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   channel      (in) channels in taps
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_disable_tap_intr(CHAL_HANDLE handle, 
			uint8_t channel, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  clear the caph intc tap int
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   channel      (in) channels in taps
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_clr_tap_intr(CHAL_HANDLE handle, 
			uint8_t channel, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  read the caph intc tap int status
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   owner  (in) owner of the caph intc block
*
*  @return uint8_t
*****************************************************************************/
uint8_t chal_caph_intc_read_tap_intr(CHAL_HANDLE handle, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  enable the caph intc tap err int
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   channel      (in) channels in taps
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_enable_tap_int_err(CHAL_HANDLE handle, 
			uint8_t channel, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  disable the caph intc tap err int
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   channel      (in) channels in taps
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_disable_tap_int_err(CHAL_HANDLE handle, 
			uint8_t channel, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  clear the caph intc tap err int
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   channel      (in) channels in taps
*  @param   owner  (in) owner of the caph intc block
*
*  @return void
*****************************************************************************/
void chal_caph_intc_clr_tap_int_err(CHAL_HANDLE handle, 
			uint8_t channel, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  read the caph intc tap err int status
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   owner  (in) owner of the caph intc block
*
*  @return uint8_t
*****************************************************************************/
uint8_t chal_caph_intc_read_tap_int_err(CHAL_HANDLE handle, 
			CAPH_ARM_DSP_e owner);

/**
*
*  @brief  set the caph intc tap ints detect level
*
*  @param   handle  (in) CHAL_HANDLE of the caph intc block
*  @param   channel      (in) channels in taps
*  @param   detectLevel  (in) int detect level
*
*  @return void
*****************************************************************************/
void chal_caph_intc_set_tap_int_detect_level(CHAL_HANDLE handle, 
			uint8_t channel, 
			uint8_t detectLevel);
#endif /* _CHAL_CAPH_INTC_ */

