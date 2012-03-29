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
*   @file   chal_caph_other.h
*
*   @brief  This file contains the definition for caph audioh and ssp CHA layer
*
****************************************************************************/

#ifndef _CHAL_CAPH_OTHER_
#define _CHAL_CAPH_OTHER_

#include <plat/chal/chal_types.h>
#include "chal_caph.h"

/**
*
*  @brief  initialize the caph audioh block
*
*  @param   baseAddress  (in) mapped address of the caph audioh block
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
*  @param   baseAddress  (in) mapped address of the caph ssp block
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

#endif /* _CHAL_CAPH_OTHER_ */
