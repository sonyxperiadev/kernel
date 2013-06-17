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
*   @file   chal_caph_dma.h
*
*   @brief  This file contains the definition for caph dma CHA layer
*
****************************************************************************/




#ifndef _CHAL_CAPH_DMA_
#define _CHAL_CAPH_DMA_

#include <plat/chal/chal_types.h>
#include <chal/chal_caph.h>



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
*  @return void
*****************************************************************************/
void chal_caph_dma_deinit(CHAL_HANDLE handle);

/**
*
*  @brief  enable the caph dma channel
*
*  @param  handle  (in) handle of the caph dma block
*  @param  channel  (in) caph dma block channel id
*
*  @return void
*****************************************************************************/
void chal_caph_dma_enable(CHAL_HANDLE handle,
			cUInt16 channel);

/**
*
*  @brief  disable the caph dma channel
*
*  @param  handle  (in) handle of the caph dma block
*  @param  channel  (in) caph dma block channel id
*
*  @return void
*****************************************************************************/
void chal_caph_dma_disable(CHAL_HANDLE handle,
			cUInt16 channel);

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
CAPH_DMA_CHANNEL_e chal_caph_dma_alloc_given_channel(CHAL_HANDLE handle, CAPH_DMA_CHANNEL_e channel);

/**
*
*  @brief  free caph dma channel
*
*  @param  handle  (in) handle of the caph dma block
*  @param  channel  (in) channel id of the caph dma block
*
*  @return void
*****************************************************************************/
void chal_caph_dma_free_channel(CHAL_HANDLE handle,
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
*  @return void
*****************************************************************************/
void chal_caph_dma_set_direction(CHAL_HANDLE handle,
            CAPH_DMA_CHANNEL_e  channel,
			CAPH_CFIFO_CHNL_DIRECTION_e direction);

/**
*
*  @brief  Set caph dma channel cfifo id
*
*  @param  handle  (in) handle of the caph dma block
*  @param  channel  (in) caph dma channel id
*  @param  cfifo_id  (in) caph dma channel CFIFO channel id
*
*  @return void
*****************************************************************************/
void chal_caph_dma_set_cfifo(CHAL_HANDLE handle,
            CAPH_DMA_CHANNEL_e  channel,
			CAPH_CFIFO_e        cfifo_id);

/**
*
*  @brief  Set caph dma channel trasfer size
*
*  @param  handle  (in) handle of the caph dma block
*  @param  channel  (in) caph dma channel id
*  @param  cfifo_id  (in) caph dma channel t size (in bytes)
*
*  @return void
*****************************************************************************/
void chal_caph_dma_set_tsize(CHAL_HANDLE handle,
            CAPH_DMA_CHANNEL_e  channel,
			cUInt8              tsize);

/**
*
*  @brief  Set caph dma channel trasfer size
*
*  @param  handle  (in) handle of the caph dma block
*  @param  channel  (in) caph dma channel id
*  @param  address  (in) caph dma channel buffer address
*  @param  size        (in) caph dma channel buffer size (in bytes)
*
*  @return void
*****************************************************************************/
void chal_caph_dma_set_buffer(CHAL_HANDLE handle,
            CAPH_DMA_CHANNEL_e  channel,
			cUInt32             address,
			cUInt32             size);

/**
*
*  @brief  Set caph dma channel address
*
*  @param  handle  (in) handle of the caph dma block
*  @param  channel  (in) caph dma channel id
*  @param  address  (in) caph dma channel buffer address
*
*  @return void
*****************************************************************************/
void chal_caph_dma_set_buffer_address(CHAL_HANDLE handle,
            								CAPH_DMA_CHANNEL_e  channel,
											cUInt32             address);

/**
*
*  @brief  set caph dma ddr fifo status
*
*  @param  handle  (in) handle of the caph dma block
*  @param  channel  (in) caph dma channel id
*  @param  status  (in) caph dma channel fifo status
*
*  @return void
*****************************************************************************/
void chal_caph_dma_set_ddrfifo_status(CHAL_HANDLE handle,
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
*  @return void
*****************************************************************************/
void chal_caph_dma_clr_ddrfifo_status(CHAL_HANDLE handle,
			CAPH_DMA_CHANNEL_e channel,
			CAPH_DMA_CHNL_FIFO_STATUS_e status);

/****************************************************************************
*
*  Function Name: void chal_caph_dma_clr_channel_fifo(CHAL_HANDLE handle,
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
*  @return void
*****************************************************************************/
void chal_caph_dma_clr_channel_fifo(CHAL_HANDLE handle,
			cUInt16 channel);
/**
*
*  @brief  read caph dma ddr fifo sw status
*
*  @param  handle  (in) handle of the caph dma block
*  @param  channel  (in) caph dma channel id
*
*  @return CAPH_DMA_CHNL_FIFO_STATUS_e
*****************************************************************************/
CAPH_DMA_CHNL_FIFO_STATUS_e chal_caph_dma_read_ddrfifo_sw_status(CHAL_HANDLE handle,
			CAPH_DMA_CHANNEL_e channel);
/**
*
*  @brief  read caph dma ddr fifo status
*
*  @param  handle  (in) handle of the caph dma block
*  @param  channel  (in) caph dma channel id
*
*  @return CAPH_DMA_CHNL_FIFO_STATUS_e
*****************************************************************************/
CAPH_DMA_CHNL_FIFO_STATUS_e chal_caph_dma_read_ddrfifo_status(CHAL_HANDLE handle,
			CAPH_DMA_CHANNEL_e channel);

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

/****************************************************************************
*
*  Function Name: void chal_caph_dma_nonstop_enable(CHAL_HANDLE handle
*                                               , CAPH_DMA_CHANNEL_e  channel)
*
*  Description: enable non-stop DMA
*
****************************************************************************/
void chal_caph_dma_nonstop_enable(CHAL_HANDLE handle, CAPH_DMA_CHANNEL_e  channel);


/****************************************************************************
*
*  Function Name: void chal_caph_dma_nonstop_disable(CHAL_HANDLE handle
*                                               , CAPH_DMA_CHANNEL_e  channel)
*
*  Description: disable non-stop DMA
*
****************************************************************************/
void chal_caph_dma_nonstop_disable(CHAL_HANDLE handle,
	CAPH_DMA_CHANNEL_e  channel);

/****************************************************************************
*
*  Function Name: void chal_caph_dma_set_hibuffer(CHAL_HANDLE handle,
*                                         CAPH_DMA_CHANNEL_e  channel,
*			                  cUInt32            address,
*						cUInt32 size)
*
*  Description: config CAPH DMA channel buffer parameters (address and size)
*
****************************************************************************/
void chal_caph_dma_set_hibuffer(CHAL_HANDLE handle, CAPH_DMA_CHANNEL_e  channel,
	cUInt32 address, cUInt32 size);

/****************************************************************************
*
*  Function Name: void chal_caph_dma_en_hibuffer(CHAL_HANDLE handle,
*                                         CAPH_DMA_CHANNEL_e  channel)
*
*  Description: enable CAPH DMA channel hi buffer
*
****************************************************************************/
cVoid chal_caph_dma_en_hibuffer(CHAL_HANDLE handle, CAPH_DMA_CHANNEL_e channel);

/****************************************************************************
*
*  Function Name: void chal_caph_dma_autogate_status(CHAL_HANDLE handle)
*
*  Description: get CAPH DMA autogate status
*
****************************************************************************/
cUInt32 chal_caph_dma_autogate_status(CHAL_HANDLE handle);

/****************************************************************************
*
*  Function Name: void chal_caph_dma_set_autogate(CHAL_HANDLE handle,
*							boolvalue)
*  Description: value = TRUE/FALSE Set/Reset CAPH DMA aadmac autogating
*               Autogating can be set only in Java
****************************************************************************/
void chal_caph_dma_set_autogate(CHAL_HANDLE handle, bool value);

#endif /* _CHAL_CAPH_DMA_ */

