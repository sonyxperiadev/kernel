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
*   @file   chal_caph_cfifo.h
*
*   @brief  This file contains the definition for caph cfifo CHA layer
*
****************************************************************************/


#ifndef _CHAL_CAPH_CFIFO_
#define _CHAL_CAPH_CFIFO_

#include <plat/chal/chal_types.h>
#include <chal/chal_caph.h>



/**
*
*  @brief  initialize the caph cfifo block
*
*  @param   baseAddress  (in) mapped address of the caph cfifo block to be initialized
*
*  @return CHAL_HANDLE
*****************************************************************************/
CHAL_HANDLE chal_caph_cfifo_init(cUInt32 baseAddress);

/**
*
*  @brief  deinitialize the caph cfifo
*
*  @param   handle  (in) caph cfifo block CHAL_HANDLE to be deinitialized
*
*  @return void
*****************************************************************************/
void chal_caph_cfifo_deinit(CHAL_HANDLE handle);

/**
*
*  @brief  get the caph cfifo address
*
*  @param   handle  (in) caph cfifo block CHAL_HANDLE
*  @param   fifo  (in) caph cfifo id
*
*  @return cUInt32
*****************************************************************************/
cUInt32 chal_caph_cfifo_get_fifo_addr(CHAL_HANDLE handle,
			CAPH_CFIFO_e fifo);

/**
*
*  @brief  get the caph cfifo address
*
*  @param   handle  (in) caph cfifo block CHAL_HANDLE
*  @param   fifo  (in) caph cfifo id
*
*  @return cUInt32
*****************************************************************************/
cUInt32 chal_caph_cfifo_get_fifo_addr_offset(CHAL_HANDLE handle,
			CAPH_CFIFO_e fifo);

/**
*
*  @brief  get the caph cfifo index
*
*  @param   fifo  (in) caph cfifo id
*
*  @return  index to the CFIFO
*****************************************************************************/
int chal_caph_cfifo_get_fifo_index(CAPH_CFIFO_e fifo);

/**
*
*  @brief  allocate a FIFO from the caph cfifo
*
*  @param   handle  (in) caph cfifo block CHAL_HANDLE
*
*  @return CAPH_CFIFO_e allocated fifo id
*****************************************************************************/
CAPH_CFIFO_e chal_caph_cfifo_alloc_channel(CHAL_HANDLE handle);

/**
*
*  @brief  allocate a given FIFO from the caph cfifo
*
*  @param   handle  (in) caph cfifo block CHAL_HANDLE
*  @param   channel  (in) caph cfifo id to be allocated
*
*  @return CAPH_CFIFO_e allocated fifo id
*****************************************************************************/
CAPH_CFIFO_e chal_caph_cfifo_alloc_given_channel(CHAL_HANDLE handle, CAPH_CFIFO_e channel);

/**
*
*  @brief  free an already allocated FIFO from the caph cfifo
*
*  @param   handle  (in) caph cfifo block CHAL_HANDLE
*  @param   fifo       (in) allocated fifo id
*
*  @return CAPH_CFIFO_e allocated fifo id
*****************************************************************************/
void chal_caph_cfifo_free_channel(CHAL_HANDLE handle, CAPH_CFIFO_e fifo);

/**
*
*  @brief  enable the caph cfifo
*
*  @param   handle  (in) caph cfifo block CHAL_HANDLE
*  @param   fifo  (in) caph cfifo
*
*  @return void
*****************************************************************************/
void chal_caph_cfifo_enable(CHAL_HANDLE handle,
			cUInt16 fifo);

/**
*
*  @brief  disable the caph cfifo
*
*  @param   handle  (in) caph cfifo block CHAL_HANDLE
*  @param   fifo  (in) caph cfifo
*
*  @return void
*****************************************************************************/
void chal_caph_cfifo_disable(CHAL_HANDLE handle,
			cUInt16 fifo);

/**
*
*  @brief  config the caph cfifo channel data direction
*
*  @param   handle  (in) caph cfifo block CHAL_HANDLE
*  @param   fifo  (in) caph cfifo id
*  @param   direction  (in) caph cfifo direction: in or out
*
*  @return void
*****************************************************************************/
void chal_caph_cfifo_set_direction(CHAL_HANDLE handle,
			CAPH_CFIFO_e fifo,
			CAPH_CFIFO_CHNL_DIRECTION_e direction);

/**
*
*  @brief  config the caph cfifo channel Start address
*
*  @param   handle  (in) caph cfifo block CHAL_HANDLE
*  @param   fifo  (in) caph cfifo id
*  @param   size  (in) caph cfifo buf start address in internal cFIFO
*
*  @return void
*****************************************************************************/
void chal_caph_cfifo_set_address(CHAL_HANDLE handle,
			CAPH_CFIFO_e fifo,
			cUInt32      address);

/**
*
*  @brief  config the caph cfifo channel fifo size
*
*  @param   handle  (in) caph cfifo block CHAL_HANDLE
*  @param   fifo  (in) caph cfifo id
*  @param   size  (in) caph cfifo buf size (in bytes)
*
*  @return void
*****************************************************************************/
void chal_caph_cfifo_set_size(CHAL_HANDLE handle,
			CAPH_CFIFO_e fifo,
			cUInt16      size);

/**
*
*  @brief  add the caph cfifo to queue
*
*  @param   handle  (in) caph cfifo block CHAL_HANDLE
*  @param   fifo  (in) caph cfifo
*  @param   queue  (in) caph cfifo queue id
*
*  @return void
*****************************************************************************/
void chal_caph_cfifo_queue_add_fifo(CHAL_HANDLE handle,
			cUInt16 fifo,
			CAPH_CFIFO_QUEUE_e queue);

/**
*
*  @brief  remove the caph cfifo from queue
*
*  @param   handle  (in) caph cfifo block CHAL_HANDLE
*  @param   fifo  (in) caph cfifo
*  @param   queue  (in) caph cfifo queue id
*
*  @return void
*****************************************************************************/
void chal_caph_cfifo_queue_remove_fifo(CHAL_HANDLE handle,
			cUInt16 fifo,
			CAPH_CFIFO_QUEUE_e queue);

/**
*
*  @brief  Set CFIFO channel panic timeout value
*
*  @param   handle  (in) caph cfifo block CHAL_HANDLE
*  @param   timeout  (in) caph panic timeout value
*
*  @return void
*****************************************************************************/
void chal_caph_cfifo_set_panic_timer(CHAL_HANDLE handle,
			cUInt8 timeout);

/**
*
*  @brief  set the caph cfifo arbiter pattern
*
*  @param   handle  (in) caph cfifo block CHAL_HANDLE
*  @param   key  (in) caph cfifo arbiter pattern
*
*  @return void
*****************************************************************************/
void chal_caph_cfifo_set_arb(CHAL_HANDLE handle,
			cUInt32 key);

/**
*
*  @brief  set the caph cfifo thresholds
*
*  @param   handle  (in) caph cfifo block CHAL_HANDLE
*  @param   fifo  (in) caph cfifo id
*  @param   thres  (in) caph cfifo threshold
*  @param   thres2  (in) caph cfifo threshold2
*
*  @return void
*****************************************************************************/
void chal_caph_cfifo_set_fifo_thres(CHAL_HANDLE handle,
			CAPH_CFIFO_e fifo,
			cUInt16 thres,
			cUInt16 thres2);

/**
*
*  @brief  clear the caph cfifo
*
*  @param   handle  (in) caph cfifo block CHAL_HANDLE
*  @param   fifo  (in) caph cfifo
*
*  @return void
*****************************************************************************/
void chal_caph_cfifo_clr_fifo(CHAL_HANDLE handle,
			cUInt16 fifo);

/**
*
*  @brief  enable the caph cfifo int
*
*  @param   handle  (in) caph cfifo block CHAL_HANDLE
*  @param   fifo  (in) caph cfifo
*
*  @return void
*****************************************************************************/
void chal_caph_cfifo_int_enable(CHAL_HANDLE handle,
			cUInt16 fifo);

/**
*
*  @brief  disable the caph cfifo int
*
*  @param   handle  (in) caph cfifo block CHAL_HANDLE
*  @param   fifo  (in) caph cfifo
*
*  @return void
*****************************************************************************/
void chal_caph_cfifo_int_disable(CHAL_HANDLE handle,
			cUInt16 fifo);

/**
*
*  @brief  enable the caph cfifo err int
*
*  @param   handle  (in) caph cfifo block CHAL_HANDLE
*  @param   fifo  (in) caph cfifo
*
*  @return void
*****************************************************************************/
void chal_caph_cfifo_int_err_enable(CHAL_HANDLE handle,
			cUInt16 fifo);

/**
*
*  @brief  disable the caph cfifo err int
*
*  @param   handle  (in) caph cfifo block CHAL_HANDLE
*  @param   fifo  (in) caph cfifo
*
*  @return void
*****************************************************************************/
void chal_caph_cfifo_int_err_disable(CHAL_HANDLE handle,
			cUInt16 fifo);

/**
*
*  @brief  read the caph cfifo status
*
*  @param   handle  (in) caph cfifo block CHAL_HANDLE
*  @param   fifo  (in) caph cfifo id
*
*  @return cUInt32
*****************************************************************************/
cUInt32 chal_caph_cfifo_read_fifo_status(CHAL_HANDLE handle,
			CAPH_CFIFO_e fifo);

/**
*
*  @brief  write the caph cfifo
*
*  @param   handle  (in) caph cfifo block CHAL_HANDLE
*  @param   fifo  (in) caph cfifo id
*  @param   data  (in) addr of the data
*  @param   size  (in) size of the data in bytes
*  @param   forceovf  (in) flag to specify force write even there is overflow
*
*  @return cUInt16 actual number of bytes written
*****************************************************************************/
cUInt16 chal_caph_cfifo_write_fifo(CHAL_HANDLE handle,
			CAPH_CFIFO_e fifo,
			cUInt32* data,
			cUInt16 size,
			_Bool   forceovf);

/**
*
*  @brief  read the caph cfifo
*
*  @param   handle  (in) caph cfifo block CHAL_HANDLE
*  @param   fifo  (in) caph cfifo id
*  @param   data  (in) addr of the data
*  @param   size  (in) size of the data in bytes
*  @param   forceovf  (in) flag to specify force read even there is underflow
*
*  @return cUInt16 actual number of bytes read
*****************************************************************************/
cUInt16 chal_caph_cfifo_read_fifo(CHAL_HANDLE handle,
			CAPH_CFIFO_e fifo,
			cUInt32* data,
			cUInt16 size,
			_Bool   forceudf);

/**
*
*  @brief  read the caph cfifo timestamp
*
*  @param   handle  (in) caph cfifo block CHAL_HANDLE
*  @param   fifo  (in) caph cfifo id
*
*  @return cUInt32
*****************************************************************************/
cUInt32 chal_caph_cfifo_read_timestamp(CHAL_HANDLE handle,
			CAPH_CFIFO_e fifo);

cVoid chal_caph_fifo_clear_register(CHAL_HANDLE handle, CAPH_CFIFO_e chal_fifo);

#endif /* _CHAL_CAPH_CFIFO_ */

