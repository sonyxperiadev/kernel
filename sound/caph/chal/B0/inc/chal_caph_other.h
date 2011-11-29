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
*   @file   chal_caph_other.h
*
*   @brief  This file contains the definition for caph audioh and ssp CHA layer
*
****************************************************************************/




#ifndef _CHAL_CAPH_OTHER_
#define _CHAL_CAPH_OTHER_

#include "chal_types.h"
#include "chal_caph.h"


/**
*
*  @brief  initialize the caph audioh block
*
*  @param   baseAddress  (in) mapped address of the caph audioh block to be initialized
*
*  @return CHAL_HANDLE
*****************************************************************************/
CHAL_HANDLE chal_caph_other_audioh_init(cUInt32 baseAddress);

/**
*
*  @brief  deinitialize the caph audioh block
*
*  @param   handle  (in) handle of the caph audioh block to be deinitialized
*
*  @return cVoid
*****************************************************************************/
cVoid chal_caph_other_audioh_deinit(CHAL_HANDLE handle);

/**
*
*  @brief  get the fifo address of the caph audioh block
*
*  @param  handle  (in) handle of the caph audioh block
*  @param  fifo  (in) caph audioh block fifo id
*
*  @return cUInt16
*****************************************************************************/
cUInt16 chal_caph_audioh_get_fifo_addr(CHAL_HANDLE handle, 
			CAPH_AUDIOH_FIFO_e fifo);

/**
*
*  @brief  initialize the caph ssp block
*
*  @param   baseAddress  (in) mapped address of the caph ssp block to be initialized
*
*  @return CHAL_HANDLE
*****************************************************************************/
CHAL_HANDLE chal_caph_other_ssp_init(cUInt32 baseAddress);

/**
*
*  @brief  deinitialize the caph ssp block
*
*  @param   handle  (in) handle of the caph ssp block to be deinitialized
*
*  @return cVoid
*****************************************************************************/
cVoid chal_caph_other_ssp_deinit(CHAL_HANDLE handle);

/**
*
*  @brief  get the fifo address of the caph ssp block
*
*  @param  handle  (in) handle of the caph ssp block
*
*  @return cUInt16
*****************************************************************************/
cUInt16 chal_caph_ssp_get_fifo_addr(CHAL_HANDLE handle);

#endif // _CHAL_CAPH_OTHER_

