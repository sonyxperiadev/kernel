/****************************************************************************/
/*     Copyright 2009, 2010 Broadcom Corporation.  All rights reserved.     */
/*     Unless you and Broadcom execute a separate written software license  */
/*	   agreement governing                                              */
/*     use of this software, this software is licensed to you under the     */
/*	   terms of the GNU General Public License version 2 (the GPL),     */
/*     available at                                                         */
/*                                                                          */
/*          http://www.broadcom.com/licenses/GPLv2.php                      */
/*                                                                          */
/*     with the following added to such license:                            */
/*                                                                          */
/*     As a special exception, the copyright holders of this software give  */
/*     you permission to link this software with independent modules, and   */
/*     to copy and distribute the resulting executable under terms of your  */
/*     choice, provided that you also meet, for each linked independent     */
/*     module, the terms and conditions of the license of that module.      */
/*     An independent module is a module which is not derived from this     */
/*     software.  The special exception does not apply to any modifications */
/*     of the software.                                                     */
/*                                                                          */
/*     Notwithstanding the above, under no circumstances may you combine    */
/*     this software in any way with any other Broadcom software provided   */
/*     under a license other than the GPL, without Broadcom's express prior */
/*     written consent.                                                     */
/*                                                                          */
/****************************************************************************/

/**
*
*   @file   chal_caph_dma.h
*
*   @brief  This file contains the definition for caph dma CHA layer
*
****************************************************************************/

#ifndef _CHAL_CAPH_DMA_
#define _CHAL_CAPH_DMA_

#include <plat/chal/chal_types.h>
#include "chal_caph.h"

/* Max number of chnnels supported by Hardware */
#define CHAL_CAPH_DMA_MAX_CHANNELS      16
/* Max number of Timestamp chnnels supported by Hardware */
#define CHAL_CAPH_DMA_MAX_TS_CHANNELS   4
/* Total Registers Size of each DMA channel  (CR1 + CR2 + SR1) */
#define CHAL_CAPH_DMA_CH_REG_SIZE       \
((CPH_AADMAC_CH2_AADMAC_CR_1_OFFSET -	\
CPH_AADMAC_CH1_AADMAC_CR_1_OFFSET)/sizeof(cUInt32))

/* Timestamp registers Size of each DMA channel (TS) */
#define CHAL_CAPH_DMA_CH_TS_REG_SIZE	\
((CPH_AADMAC_CH2_AADMAC_TS_OFFSET -	\
CPH_AADMAC_CH1_AADMAC_TS_OFFSET)/sizeof(cUInt32))

#define CPH_AADMAC_DMA_MAX_WRAP_SIZE          0x10000
#define CPH_AADMAC_DMA_CH1_2_MAX_WRAP_SIZE    0x1000000

struct _chal_caph_dma_cb_t {
	cUInt32 base;		/* Register Base address */
	cBool alloc_status[CHAL_CAPH_DMA_MAX_CHANNELS];
};
#define chal_caph_dma_cb_t struct _chal_caph_dma_cb_t

struct _chal_caph_dma_funcs_t {
	cVoid(*set_hibuffer) (CHAL_HANDLE handle, CAPH_DMA_CHANNEL_e channel,
			      cUInt32 address, cUInt32 size);
	cVoid(*set_ddrfifo_status) (CHAL_HANDLE, CAPH_DMA_CHANNEL_e,
				    CAPH_DMA_CHNL_FIFO_STATUS_e);
	cVoid(*clr_ddrfifo_status) (CHAL_HANDLE handle,
				    CAPH_DMA_CHANNEL_e channel,
				    CAPH_DMA_CHNL_FIFO_STATUS_e status);
	cVoid(*clr_channel_fifo) (CHAL_HANDLE handle, cUInt16 channel);
	CAPH_DMA_CHNL_FIFO_STATUS_e(*read_ddrfifo_sw_status) (
				CHAL_HANDLE	handle,
				CAPH_DMA_CHANNEL_e	channel);
};
#define chal_caph_dma_funcs_t struct _chal_caph_dma_funcs_t
/**
*
*  @brief  initialize the caph dma block
*
*  @param   baseAddress  (in) mapped address of the caph dma block to be initialized
*
*  @return CHAL_HANDLE
*****************************************************************************/
cBool chal_caph_dma_platform_init(chal_caph_dma_funcs_t *dma_func);

/**
*
*  @brief  initialize the caph dma block
*
*  @param   baseAddress  (in) mapped address of the caph dma block to be initialized
*
*  @return CHAL_HANDLE
*****************************************************************************/
CHAL_HANDLE chal_caph_dma_init(cUInt32 baseAddress);

/**
*
*  @brief  deinitialize the caph dma block
*
*  @param   handle  (in) handle of the caph dma block to be deinitialized
*
*  @return cVoid
*****************************************************************************/
cVoid chal_caph_dma_deinit(CHAL_HANDLE handle);

/**
*
*  @brief  enable the caph dma channel
*
*  @param  handle  (in) handle of the caph dma block
*  @param  channel  (in) caph dma block channel id
*
*  @return cVoid
*****************************************************************************/
cVoid chal_caph_dma_enable(CHAL_HANDLE handle, cUInt16 channel);

/**
*
*  @brief  disable the caph dma channel
*
*  @param  handle  (in) handle of the caph dma block
*  @param  channel  (in) caph dma block channel id
*
*  @return cVoid
*****************************************************************************/
cVoid chal_caph_dma_disable(CHAL_HANDLE handle, cUInt16 channel);

/**
*
*  @brief  allocate caph dma channel
*
*  @param  handle  (in) handle of the caph dma block
*
*  @return CAPH_DMA_CHANNEL_e
*****************************************************************************/
CAPH_DMA_CHANNEL_e chal_caph_dma_alloc_channel(CHAL_HANDLE handle);

/**
*
*  @brief  allocate caph dma channel
*
*  @param  handle  (in) handle of the caph dma block
*  @param  channel  (in) DMA channel ID to allocate
*
*  @return CAPH_DMA_CHANNEL_e
*****************************************************************************/
CAPH_DMA_CHANNEL_e chal_caph_dma_alloc_given_channel(CHAL_HANDLE handle,
						     CAPH_DMA_CHANNEL_e
						     channel);

/**
*
*  @brief  free caph dma channel
*
*  @param  handle  (in) handle of the caph dma block
*  @param  channel  (in) channel id of the caph dma block
*
*  @return cVoid
*****************************************************************************/
cVoid chal_caph_dma_free_channel(CHAL_HANDLE handle,
				 CAPH_DMA_CHANNEL_e channel);


/**
*
*  @brief  free caph dma channel
*
*  @param  handle  (in) handle of the caph dma block
*  @param  channel  (in) channel id of the caph dma block
*
*  @return cVoid
*
****************************************************************************/

cVoid chal_caph_dma_clear_register(CHAL_HANDLE handle,
				  CAPH_DMA_CHANNEL_e channel);


/**
*
*  @brief  Set caph dma channel data direction
*
*  @param  handle  (in) handle of the caph dma block
*  @param  channel  (in) caph dma channel id
*  @param  direction  (in) caph dma channel data transfer direction
*
*  @return cVoid
*****************************************************************************/
cVoid chal_caph_dma_set_direction(CHAL_HANDLE handle,
				  CAPH_DMA_CHANNEL_e channel,
				  CAPH_CFIFO_CHNL_DIRECTION_e direction);

/**
*
*  @brief  Set caph dma channel cfifo id
*
*  @param  handle  (in) handle of the caph dma block
*  @param  channel  (in) caph dma channel id
*  @param  cfifo_id  (in) caph dma channel CFIFO channel id
*
*  @return cVoid
*****************************************************************************/
cVoid chal_caph_dma_set_cfifo(CHAL_HANDLE handle,
			      CAPH_DMA_CHANNEL_e channel,
			      CAPH_CFIFO_e cfifo_id);

/**
*
*  @brief  Set caph dma channel trasfer size
*
*  @param  handle  (in) handle of the caph dma block
*  @param  channel  (in) caph dma channel id
*  @param  cfifo_id  (in) caph dma channel t size (in bytes)
*
*  @return cVoid
*****************************************************************************/
cVoid chal_caph_dma_set_tsize(CHAL_HANDLE handle,
			      CAPH_DMA_CHANNEL_e channel, cUInt8 tsize);

/**
*
*  @brief  Set caph dma channel trasfer size
*
*  @param  handle  (in) handle of the caph dma block
*  @param  channel  (in) caph dma channel id
*  @param  address  (in) caph dma channel buffer address
*  @param  size        (in) caph dma channel buffer size (in bytes)
*
*  @return cVoid
*****************************************************************************/
cVoid chal_caph_dma_set_buffer(CHAL_HANDLE handle,
			       CAPH_DMA_CHANNEL_e channel,
			       cUInt32 address, cUInt32 size);

/**
*
*  @brief  Set caph dma channel buffer address
*
*  @param  handle  (in) handle of the caph dma block
*  @param  channel  (in) caph dma channel id
*  @param  address  (in) caph dma channel buffer address
*
*  @return cVoid
*****************************************************************************/
cVoid chal_caph_dma_set_buffer_address(CHAL_HANDLE handle,
				       CAPH_DMA_CHANNEL_e channel,
				       cUInt32 address);

cVoid chal_caph_dma_set_hibuffer(CHAL_HANDLE handle,
				 CAPH_DMA_CHANNEL_e channel,
				 cUInt32 address, cUInt32 size);

/**
*
*  @brief  set caph dma ddr fifo status
*
*  @param  handle  (in) handle of the caph dma block
*  @param  channel  (in) caph dma channel id
*  @param  status  (in) caph dma channel fifo status
*
*  @return cVoid
*****************************************************************************/
cVoid chal_caph_dma_set_ddrfifo_status(CHAL_HANDLE handle,
				       CAPH_DMA_CHANNEL_e channel,
				       CAPH_DMA_CHNL_FIFO_STATUS_e status);

/**
*
*  @brief  clear caph dma ddr fifo status
*
*  @param  handle  (in) handle of the caph dma block
*  @param  channel  (in) caph dma channel id
*  @param  status  (in) caph dma channel fifo hw status
*
*  @return cVoid
*****************************************************************************/
cVoid chal_caph_dma_clr_ddrfifo_status(CHAL_HANDLE handle,
				       CAPH_DMA_CHANNEL_e channel,
				       CAPH_DMA_CHNL_FIFO_STATUS_e status);

/****************************************************************************
*
*  Function Name: cVoid chal_caph_dma_clr_channel_fifo(CHAL_HANDLE handle,
*			cUInt16 fifo)
*
*  Description: clear dma channel internal fifo
*
****************************************************************************/
/**
*
*  @brief  clear dma channel internal fifo
*
*  @param  handle  (in) handle of the caph dma block
*  @param  channel  (in) caph dma channel(s) bitmap
*
*  @return cVoid
*****************************************************************************/
cVoid chal_caph_dma_clr_channel_fifo(CHAL_HANDLE handle, cUInt16 channel);
/**
*
*  @brief  read caph dma ddr fifo sw status
*
*  @param  handle  (in) handle of the caph dma block
*  @param  channel  (in) caph dma channel id
*
*  @return CAPH_DMA_CHNL_FIFO_STATUS_e
*****************************************************************************/
CAPH_DMA_CHNL_FIFO_STATUS_e chal_caph_dma_read_ddrfifo_sw_status(
				CHAL_HANDLE	handle,
				CAPH_DMA_CHANNEL_e	channel);
/**
*
*  @brief  read caph dma ddr fifo status
*
*  @param  handle  (in) handle of the caph dma block
*  @param  channel  (in) caph dma channel id
*
*  @return CAPH_DMA_CHNL_FIFO_STATUS_e
*****************************************************************************/
CAPH_DMA_CHNL_FIFO_STATUS_e chal_caph_dma_read_ddrfifo_status(
				CHAL_HANDLE	handle,
				CAPH_DMA_CHANNEL_e	channel);

/**
*
*  @brief  read caph dma request counts
*
*  @param  handle  (in) handle of the caph dma block
*  @param  channel  (in) caph dma channel id
*
*  @return cUInt8
*****************************************************************************/
cUInt8 chal_caph_dma_read_reqcount(CHAL_HANDLE handle,
				   CAPH_DMA_CHANNEL_e channel);

/**
*
*  @brief  read caph dma current memory pointer
*
*  @param  handle  (in) handle of the caph dma block
*  @param  channel  (in) caph dma channel id
*
*  @return cUInt16
*****************************************************************************/
cUInt16 chal_caph_dma_read_currmempointer(CHAL_HANDLE handle,
					  CAPH_DMA_CHANNEL_e channel);

/**
*
*  @brief  check caph ping pong buffer used by dma
*
*  @param  handle  (in) handle of the caph dma block
*  @param  channel  (in) caph dma channel id
*
*  @return cUInt16
*****************************************************************************/
cUInt16 chal_caph_dma_check_dmabuffer(CHAL_HANDLE handle,
					  CAPH_DMA_CHANNEL_e channel);

/**
*
*  @brief  read caph dma timestamp
*
*  @param  handle  (in) handle of the caph dma block
*  @param  channel  (in) caph dma channel id
*
*  @return cUInt32
*****************************************************************************/
cUInt32 chal_caph_dma_read_timestamp(CHAL_HANDLE handle,
				     CAPH_DMA_CHANNEL_e channel);

/**
*
*  @brief  enable dma channel hi buffer
*
*  @param  handle  (in) handle of the caph dma block
*  @param  channel  (in) caph dma channel id
*
*  @return cVoid
*****************************************************************************/
cVoid chal_caph_dma_en_hibuffer(CHAL_HANDLE handle, CAPH_DMA_CHANNEL_e channel);

#endif /* _CHAL_CAPH_DMA_ */
