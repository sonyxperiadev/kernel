/****************************************************************************/
/*     Copyright 2009-2012  Broadcom Corporation.  All rights reserved.     */
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
*  @file   chal_caph_other.c
*
*  @brief  chal layer driver for caph audioh and ssp driver
*
****************************************************************************/

#include "chal_caph_other.h"

/****************************************************************************
			G L O B A L   S E C T I O N
****************************************************************************/

/****************************************************************************
 global variable definitions
****************************************************************************/

/****************************************************************************
			L O C A L   S E C T I O N
****************************************************************************/

/****************************************************************************
 local macro declarations
****************************************************************************/

/****************************************************************************
 local typedef declarations
****************************************************************************/

/****************************************************************************
 local variable definitions
****************************************************************************/

/****************************************************************************
 local function declarations
****************************************************************************/

/******************************************************************************
 local function definitions
******************************************************************************/

/****************************************************************************
*
*  Function Name: CHAL_HANDLE chal_caph_other_audioh_init(cUInt32 baseAddress)
*
*  Description: init CAPH AUDIOH block
*
****************************************************************************/
CHAL_HANDLE chal_caph_other_audioh_init(cUInt32 baseAddress)
{
	CHAL_HANDLE dummy = 0;
	return dummy;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_other_audioh_deinit(CHAL_HANDLE handle)
*
*  Description: deinit CAPH AUDIOH block
*
****************************************************************************/
cVoid chal_caph_other_audioh_deinit(CHAL_HANDLE handle)
{
	return;
}

/****************************************************************************
*
*  Function Name: cUInt16 chal_caph_audioh_get_fifo_addr(CHAL_HANDLE handle,
*			CAPH_AUDIOH_FIFO_e fifo)
*
*  Description: get CAPH AUDIOH fifo address
*
****************************************************************************/
cUInt16 chal_caph_audioh_get_fifo_addr(CHAL_HANDLE handle,
				       CAPH_AUDIOH_FIFO_e fifo)
{
	cUInt16 dummy = 0;
	return dummy;
}

/****************************************************************************
*
*  Function Name: CHAL_HANDLE chal_caph_other_ssp_init(cUInt32 baseAddress)
*
*  Description: init CAPH SSP block
*
****************************************************************************/
CHAL_HANDLE chal_caph_other_ssp_init(cUInt32 baseAddress)
{
	CHAL_HANDLE dummy = 0;
	return dummy;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_other_ssp_deinit(CHAL_HANDLE handle)
*
*  Description: deinit CAPH SSP block
*
****************************************************************************/
cVoid chal_caph_other_ssp_deinit(CHAL_HANDLE handle)
{
	return;
}

/****************************************************************************
*
*  Function Name: cUInt16 chal_caph_ssp_get_fifo_addr(CHAL_HANDLE handle)
*
*  Description: get CAPH SSP fifo address
*
****************************************************************************/
cUInt16 chal_caph_ssp_get_fifo_addr(CHAL_HANDLE handle)
{
	cUInt16 dummy = 0;
	return dummy;
}
