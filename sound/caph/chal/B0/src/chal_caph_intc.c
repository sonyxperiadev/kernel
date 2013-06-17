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
*  @file   chal_caph_intc.c
*
*  @brief  chal layer driver for caph intc
*
****************************************************************************/

#include "chal_caph_intc.h"
#include <mach/rdb/brcm_rdb_ahintc.h>
#include <mach/rdb/brcm_rdb_util.h>

/****************************************************************************
*                        G L O B A L   S E C T I O N
*****************************************************************************/

/****************************************************************************
* global variable definitions
*****************************************************************************/

/****************************************************************************
*                         L O C A L   S E C T I O N
*****************************************************************************/

/****************************************************************************
* local macro declarations
*****************************************************************************/
#define CHAL_CAPH_INTC_SUBSYS_REG_SIZE	\
((AHINTC_ENABLE_DSP_STD_0_OFFSET - \
AHINTC_ENABLE_ARM_STD_0_OFFSET)/sizeof(cUInt32))

/****************************************************************************
* local typedef declarations
*****************************************************************************/

/****************************************************************************
* local variable definitions
*****************************************************************************/

/****************************************************************************
* local function declarations
*****************************************************************************/

/******************************************************************************
* local function definitions
*******************************************************************************/

#define CHAL_CAPH_INTC_SUBSYS_REG_SIZE	\
((AHINTC_ENABLE_DSP_STD_0_OFFSET -	\
AHINTC_ENABLE_ARM_STD_0_OFFSET)/sizeof(cUInt32))

/****************************************************************************
*
*  Function Name: CHAL_HANDLE chal_caph_intc_init(cUInt32 baseAddress)
*
*  Description: Initialize CAPH INTC
*
****************************************************************************/
CHAL_HANDLE chal_caph_intc_init(cUInt32 baseAddress)
{
	return (CHAL_HANDLE) baseAddress;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_deinit(CHAL_HANDLE handle)
*
*  Description: Deinitialize CAPH INTC
*
****************************************************************************/
cVoid chal_caph_intc_deinit(CHAL_HANDLE handle)
{
	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_reset(CHAL_HANDLE handle)
*
*  Description: Reset CAPH INTC
*
****************************************************************************/
cVoid chal_caph_intc_reset(CHAL_HANDLE handle)
{
	cUInt32 base = (cUInt32) handle;

	/* Create a reset Pulse */
	/* Make sure reset bit is 0 */
	BRCM_WRITE_REG(base, AHINTC_AHINTC_CONTROL, 0x0000);

	/* Set Rest to 1 */
	BRCM_WRITE_REG(base, AHINTC_AHINTC_CONTROL,
		       AHINTC_AHINTC_CONTROL_AHINTC_SOFT_RESET_MASK);

	/* Set Rest to 0 */
	BRCM_WRITE_REG(base, AHINTC_AHINTC_CONTROL, 0x0000);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_enable_cfifo_intr(CHAL_HANDLE handle,
*            cUInt16 fifo,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Enable CAPH INTC CFIFO INT
*
****************************************************************************/
cVoid chal_caph_intc_enable_cfifo_intr(CHAL_HANDLE handle,
				       cUInt16 fifo, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_en;

	int_en =
	    BRCM_READ_REG_IDX(base, AHINTC_ENABLE_ARM_STD_0,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_en |=
	    (fifo << AHINTC_ENABLE_ARM_STD_0_ENABLE_A_S_CFIFO_CH16_1_SHIFT);
	BRCM_WRITE_REG_IDX(base, AHINTC_ENABLE_ARM_STD_0,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_en);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_disable_cfifo_intr(CHAL_HANDLE handle,
*            cUInt16 fifo,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Disable CAPH INTC CFIFO INT
*
****************************************************************************/
cVoid chal_caph_intc_disable_cfifo_intr(CHAL_HANDLE handle,
					cUInt16 fifo, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_en;
	cUInt32 int_dis =
	    (fifo << AHINTC_ENABLE_ARM_STD_0_ENABLE_A_S_CFIFO_CH16_1_SHIFT);

	int_en =
	    BRCM_READ_REG_IDX(base, AHINTC_ENABLE_ARM_STD_0,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_en &= (~int_dis);
	BRCM_WRITE_REG_IDX(base, AHINTC_ENABLE_ARM_STD_0,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_en);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_clr_cfifo_intr(CHAL_HANDLE handle,
*            cUInt16 fifo,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Clear CAPH INTC CFIFO INT
*
****************************************************************************/
cVoid chal_caph_intc_clr_cfifo_intr(CHAL_HANDLE handle,
				    cUInt16 fifo, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_clr;

	int_clr =
	    BRCM_READ_REG_IDX(base, AHINTC_CLEAR_ARM_STD_0,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_clr |=
	    (fifo << AHINTC_CLEAR_ARM_STD_0_CLEAR_A_S_CFIFO_CH16_1_SHIFT);
	BRCM_WRITE_REG_IDX(base, AHINTC_CLEAR_ARM_STD_0,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_clr);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_read_cfifo_intr(CHAL_HANDLE handle,
*            cUInt16 fifo,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Read CAPH INTC CFIFO INT
*
****************************************************************************/
cUInt16 chal_caph_intc_read_cfifo_intr(CHAL_HANDLE handle, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_sts;

	int_sts =
	    BRCM_READ_REG_IDX(base, AHINTC_STATUS_ARM_STD_0,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_sts &= AHINTC_STATUS_ARM_STD_0_STATUS_A_S_CFIFO_CH16_1_MASK;
	int_sts >>= AHINTC_STATUS_ARM_STD_0_STATUS_A_S_CFIFO_CH16_1_SHIFT;

	return int_sts;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_enable_cfifo_int_err(CHAL_HANDLE handle,
*            cUInt16 fifo,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Enable CAPH INTC CFIFO ERR INT
*
****************************************************************************/
cVoid chal_caph_intc_enable_cfifo_int_err(CHAL_HANDLE handle,
					  cUInt16 fifo, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_en;

	int_en =
	    BRCM_READ_REG_IDX(base, AHINTC_ENABLE_ARM_ERR_0,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_en |=
	    (fifo << AHINTC_ENABLE_ARM_ERR_0_ENABLE_A_E_CFIFO_CH16_1_SHIFT);
	BRCM_WRITE_REG_IDX(base, AHINTC_ENABLE_ARM_ERR_0,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_en);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_disable_cfifo_int_err(CHAL_HANDLE handle,
*            cUInt16 fifo,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Disable CAPH INTC CFIFO ERR INT
*
****************************************************************************/
cVoid chal_caph_intc_disable_cfifo_int_err(CHAL_HANDLE handle,
					   cUInt16 fifo, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_en;
	cUInt32 int_dis =
	    (fifo << AHINTC_ENABLE_ARM_ERR_0_ENABLE_A_E_CFIFO_CH16_1_SHIFT);

	int_en =
	    BRCM_READ_REG_IDX(base, AHINTC_ENABLE_ARM_ERR_0,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_en &= (~int_dis);
	BRCM_WRITE_REG_IDX(base, AHINTC_ENABLE_ARM_ERR_0,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_en);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_clr_cfifo_int_err(CHAL_HANDLE handle,
*            cUInt16 fifo,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Clear CAPH INTC CFIFO ERR INT
*
****************************************************************************/
cVoid chal_caph_intc_clr_cfifo_int_err(CHAL_HANDLE handle,
				       cUInt16 fifo, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_clr;

	int_clr =
	    BRCM_READ_REG_IDX(base, AHINTC_CLEAR_ARM_ERR_0,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_clr |=
	    (fifo << AHINTC_CLEAR_ARM_ERR_0_CLEAR_A_E_CFIFO_CH16_1_SHIFT);
	BRCM_WRITE_REG_IDX(base, AHINTC_CLEAR_ARM_ERR_0,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_clr);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_read_cfifo_int_err(CHAL_HANDLE handle,
*            cUInt16 fifo,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Read CAPH INTC CFIFO ERR INT
*
****************************************************************************/
cUInt16 chal_caph_intc_read_cfifo_int_err(CHAL_HANDLE handle,
					  CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_sts;

	int_sts =
	    BRCM_READ_REG_IDX(base, AHINTC_STATUS_ARM_ERR_0,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_sts &= AHINTC_STATUS_ARM_ERR_0_STATUS_A_E_CFIFO_CH16_1_MASK;
	int_sts >>= AHINTC_STATUS_ARM_ERR_0_STATUS_A_E_CFIFO_CH16_1_SHIFT;

	return int_sts;
}

/****************************************************************************
*
*  Function: cVoid chal_caph_intc_set_cfifo_int_detect_level
*	(CHAL_HANDLE handle,
*            cUInt16 fifo,
*            cUInt16 detectLevel)
*
*  Description: Set CAPH INTC CFIFO INT detect level
*
****************************************************************************/
cVoid chal_caph_intc_set_cfifo_int_detect_level(CHAL_HANDLE handle,
						cUInt16 fifo,
						cUInt16 detectLevel)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 level_detect;

	level_detect = BRCM_READ_REG(base, AHINTC_LEVELDETECT_0);

	/* Clear the required channel settings */
	level_detect &=
	    (~(fifo << AHINTC_LEVELDETECT_0_LEVEL_CFIFO_CH16_1_SHIFT));
	/* Add the level detection settings for the given channels */
	level_detect |=
	    ((fifo & detectLevel) <<
	     AHINTC_LEVELDETECT_0_LEVEL_CFIFO_CH16_1_SHIFT);

	/* Apply the settings */
	BRCM_WRITE_REG(base, AHINTC_LEVELDETECT_0, level_detect);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_enable_dma_intr(CHAL_HANDLE handle,
*            cUInt16 channel,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Enable CAPH INTC DMA INT
*
****************************************************************************/
cVoid chal_caph_intc_enable_dma_intr(CHAL_HANDLE handle,
				     cUInt16 channel, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_en;

	int_en =
	    BRCM_READ_REG_IDX(base, AHINTC_ENABLE_ARM_STD_0,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_en |=
	    (channel << AHINTC_ENABLE_ARM_STD_0_ENABLE_A_S_AADMAC_CH16_1_SHIFT);
	BRCM_WRITE_REG_IDX(base, AHINTC_ENABLE_ARM_STD_0,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_en);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_disable_dma_intr(CHAL_HANDLE handle,
*            cUInt16 channel,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Disable CAPH INTC DMA INT
*
****************************************************************************/
cVoid chal_caph_intc_disable_dma_intr(CHAL_HANDLE handle,
				      cUInt16 channel, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_en;
	cUInt32 int_dis =
	    (channel << AHINTC_ENABLE_ARM_STD_0_ENABLE_A_S_AADMAC_CH16_1_SHIFT);

	int_en =
	    BRCM_READ_REG_IDX(base, AHINTC_ENABLE_ARM_STD_0,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_en &= (~int_dis);
	BRCM_WRITE_REG_IDX(base, AHINTC_ENABLE_ARM_STD_0,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_en);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_clr_dma_intr(CHAL_HANDLE handle,
*            cUInt16 channel,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Clear CAPH INTC DMA INT
*
****************************************************************************/
cVoid chal_caph_intc_clr_dma_intr(CHAL_HANDLE handle,
				  cUInt16 channel, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_clr;

	int_clr =
	    BRCM_READ_REG_IDX(base, AHINTC_CLEAR_ARM_STD_0,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_clr |=
	    (channel << AHINTC_CLEAR_ARM_STD_0_CLEAR_A_S_AADMAC_CH16_1_SHIFT);
	BRCM_WRITE_REG_IDX(base, AHINTC_CLEAR_ARM_STD_0,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_clr);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_read_dma_intr(CHAL_HANDLE handle,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Read CAPH INTC DMA INT
*
****************************************************************************/
cUInt16 chal_caph_intc_read_dma_intr(CHAL_HANDLE handle, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_sts;

	int_sts =
	    BRCM_READ_REG_IDX(base, AHINTC_STATUS_ARM_STD_0,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_sts &= AHINTC_STATUS_ARM_STD_0_STATUS_A_S_AADMAC_CH16_1_MASK;
	int_sts >>= AHINTC_STATUS_ARM_STD_0_STATUS_A_S_AADMAC_CH16_1_SHIFT;

	return int_sts;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_enable_dma_int_err(CHAL_HANDLE handle,
*            cUInt16 channel,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Enable CAPH INTC DMA INT err
*
****************************************************************************/
cVoid chal_caph_intc_enable_dma_int_err(CHAL_HANDLE handle,
					cUInt16 channel, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_en;

	int_en =
	    BRCM_READ_REG_IDX(base, AHINTC_ENABLE_ARM_ERR_0,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_en |=
	    (channel << AHINTC_ENABLE_ARM_ERR_0_ENABLE_A_E_AADMAC_CH16_1_SHIFT);
	BRCM_WRITE_REG_IDX(base, AHINTC_ENABLE_ARM_ERR_0,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_en);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_disable_dma_int_err(CHAL_HANDLE handle,
*            cUInt16 channel,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Disable CAPH INTC DMA INT err
*
****************************************************************************/
cVoid chal_caph_intc_disable_dma_int_err(CHAL_HANDLE handle,
					 cUInt16 channel, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_en;
	cUInt32 int_dis =
	    (channel << AHINTC_ENABLE_ARM_ERR_0_ENABLE_A_E_AADMAC_CH16_1_SHIFT);

	int_en =
	    BRCM_READ_REG_IDX(base, AHINTC_ENABLE_ARM_ERR_0,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_en &= (~int_dis);
	BRCM_WRITE_REG_IDX(base, AHINTC_ENABLE_ARM_ERR_0,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_en);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_clr_dma_int_err(CHAL_HANDLE handle,
*            cUInt16 channel,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Clear CAPH INTC DMA INT err
*
****************************************************************************/
cVoid chal_caph_intc_clr_dma_int_err(CHAL_HANDLE handle,
				     cUInt16 channel, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_clr;

	int_clr =
	    BRCM_READ_REG_IDX(base, AHINTC_CLEAR_ARM_ERR_0,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_clr |=
	    (channel << AHINTC_CLEAR_ARM_ERR_0_CLEAR_A_E_AADMAC_CH16_1_SHIFT);
	BRCM_WRITE_REG_IDX(base, AHINTC_CLEAR_ARM_ERR_0,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_clr);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_read_dma_int_err(CHAL_HANDLE handle,
*            cUInt16 channel,
*            CAPH_ARM_DSP_e owner)
*
*  Description: read CAPH INTC DMA INT err
*
****************************************************************************/
cUInt16 chal_caph_intc_read_dma_int_err(CHAL_HANDLE handle,
					CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_sts;

	int_sts =
	    BRCM_READ_REG_IDX(base, AHINTC_STATUS_ARM_ERR_0,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_sts &= AHINTC_STATUS_ARM_ERR_0_STATUS_A_E_AADMAC_CH16_1_MASK;
	int_sts >>= AHINTC_STATUS_ARM_ERR_0_STATUS_A_E_AADMAC_CH16_1_SHIFT;

	return int_sts;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_set_dma_int_detect_level
*	(CHAL_HANDLE handle,
*            cUInt16 channel,
*            cUInt16 detectLevel)
*
*  Description: Set CAPH INTC DMA INT detect level
*
****************************************************************************/
cVoid chal_caph_intc_set_dma_int_detect_level(CHAL_HANDLE handle,
					      cUInt16 channel,
					      cUInt16 detectLevel)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 level_detect;

	level_detect = BRCM_READ_REG(base, AHINTC_LEVELDETECT_0);

	/* Clear the required channel settings */
	level_detect &=
	    (~(channel << AHINTC_LEVELDETECT_0_LEVEL_AADMAC_CH16_1_SHIFT));
	/* Add the level detection settings for the given channels */
	level_detect |=
	    ((channel & detectLevel) <<
	     AHINTC_LEVELDETECT_0_LEVEL_AADMAC_CH16_1_SHIFT);

	/* Apply the settings */
	BRCM_WRITE_REG(base, AHINTC_LEVELDETECT_0, level_detect);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_enable_ssp_intr(CHAL_HANDLE handle,
*            cUInt8 ssp,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Enable CAPH INTC SSP INT
*
****************************************************************************/
cVoid chal_caph_intc_enable_ssp_intr(CHAL_HANDLE handle,
				     cUInt8 ssp, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_en;

	int_en =
	    BRCM_READ_REG_IDX(base, AHINTC_ENABLE_ARM_STD_1,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_en |= (ssp << AHINTC_ENABLE_ARM_STD_1_ENABLE_A_S_SSPI_3_SHIFT);
	BRCM_WRITE_REG_IDX(base, AHINTC_ENABLE_ARM_STD_1,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_en);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_disable_ssp_intr(CHAL_HANDLE handle,
*            cUInt8 ssp,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Disable CAPH INTC SSP INT
*
****************************************************************************/
cVoid chal_caph_intc_disable_ssp_intr(CHAL_HANDLE handle,
				      cUInt8 ssp, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_en;
	cUInt32 int_dis =
	    (ssp << AHINTC_ENABLE_ARM_STD_1_ENABLE_A_S_SSPI_3_SHIFT);

	int_en =
	    BRCM_READ_REG_IDX(base, AHINTC_ENABLE_ARM_STD_1,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_en &= ~(int_dis);
	BRCM_WRITE_REG_IDX(base, AHINTC_ENABLE_ARM_STD_1,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_en);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_clr_ssp_intr(CHAL_HANDLE handle,
*            cUInt8 ssp,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Clear CAPH INTC SSP INT
*
****************************************************************************/
cVoid chal_caph_intc_clr_ssp_intr(CHAL_HANDLE handle,
				  cUInt8 ssp, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_clr;

	int_clr =
	    BRCM_READ_REG_IDX(base, AHINTC_CLEAR_ARM_STD_1,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_clr |= (ssp << AHINTC_CLEAR_ARM_STD_1_CLEAR_A_S_SSPI_3_SHIFT);
	BRCM_WRITE_REG_IDX(base, AHINTC_CLEAR_ARM_STD_1,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_clr);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_read_ssp_intr(CHAL_HANDLE handle,
*            cUInt8 ssp,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Read CAPH INTC SSP INT
*
****************************************************************************/
cUInt16 chal_caph_intc_read_ssp_intr(CHAL_HANDLE handle,
				     cUInt8 ssp, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_sts;

	int_sts =
	    BRCM_READ_REG_IDX(base, AHINTC_STATUS_ARM_STD_1,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_sts &=
	    (AHINTC_CLEAR_ARM_STD_1_CLEAR_A_S_SSPI_3_MASK |
	     AHINTC_CLEAR_ARM_STD_1_CLEAR_A_S_SSPI_4_MASK);
	int_sts >>= AHINTC_CLEAR_ARM_STD_1_CLEAR_A_S_SSPI_3_SHIFT;

	return int_sts;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_enable_ssp_int_err(CHAL_HANDLE handle,
*            cUInt8 ssp,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Enable CAPH INTC SSP INT err
*
****************************************************************************/
cVoid chal_caph_intc_enable_ssp_int_err(CHAL_HANDLE handle,
					cUInt8 ssp, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_en;

	int_en =
	    BRCM_READ_REG_IDX(base, AHINTC_ENABLE_ARM_ERR_1,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_en |= (ssp << AHINTC_ENABLE_ARM_ERR_1_ENABLE_A_E_SSPI_3_SHIFT);
	BRCM_WRITE_REG_IDX(base, AHINTC_ENABLE_ARM_ERR_1,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_en);

	return;
}

/****************************************************************************
*
*  Function : cVoid chal_caph_intc_disable_ssp_int_err(CHAL_HANDLE handle,
*	cUInt8 ssp, CAPH_ARM_DSP_e owner)
*
*  Description: Disable CAPH INTC SSP INT err
*
****************************************************************************/
cVoid chal_caph_intc_disable_ssp_int_err(CHAL_HANDLE handle,
					 cUInt8 ssp, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_en;
	cUInt32 int_dis =
	    (ssp << AHINTC_ENABLE_ARM_ERR_1_ENABLE_A_E_SSPI_3_SHIFT);

	int_en =
	    BRCM_READ_REG_IDX(base, AHINTC_ENABLE_ARM_ERR_1,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_en &= ~(int_dis);
	BRCM_WRITE_REG_IDX(base, AHINTC_ENABLE_ARM_ERR_1,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_en);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_clr_ssp_int_err(CHAL_HANDLE handle,
*            cUInt8 ssp,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Clear CAPH INTC SSP INT err
*
****************************************************************************/
cVoid chal_caph_intc_clr_ssp_int_err(CHAL_HANDLE handle,
				     cUInt8 ssp, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_clr;

	int_clr =
	    BRCM_READ_REG_IDX(base, AHINTC_CLEAR_ARM_ERR_1,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_clr |= (ssp << AHINTC_CLEAR_ARM_ERR_1_CLEAR_A_E_SSPI_3_SHIFT);
	BRCM_WRITE_REG_IDX(base, AHINTC_CLEAR_ARM_ERR_1,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_clr);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_read_ssp_int_err(CHAL_HANDLE handle,
*            cUInt8 ssp,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Read CAPH INTC SSP INT err
*
****************************************************************************/
cUInt16 chal_caph_intc_read_ssp_int_err(CHAL_HANDLE handle,
					CAPH_SSP_e ssp, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_sts;

	int_sts =
	    BRCM_READ_REG_IDX(base, AHINTC_STATUS_ARM_ERR_1,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_sts &=
	    (AHINTC_CLEAR_ARM_ERR_1_CLEAR_A_E_SSPI_3_MASK |
	     AHINTC_CLEAR_ARM_ERR_1_CLEAR_A_E_SSPI_4_MASK);
	int_sts >>= AHINTC_CLEAR_ARM_ERR_1_CLEAR_A_E_SSPI_3_SHIFT;

	return int_sts;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_set_ssp_int_detect_level
*	(CHAL_HANDLE handle,
*            cUInt8 ssp,
*            cUInt8 detectLevel)
*
*  Description: Set CAPH INTC SSP INT detect level
*
****************************************************************************/
cVoid chal_caph_intc_set_ssp_int_detect_level(CHAL_HANDLE handle,
					      cUInt8 ssp, cUInt8 detectLevel)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 level_detect;

	level_detect = BRCM_READ_REG(base, AHINTC_LEVELDETECT_1);

	/* Clear the required ssp settings */
	level_detect &= (~(ssp << AHINTC_LEVELDETECT_1_LEVEL_SSPI_3_SHIFT));
	/* Add the level detection settings for the given channels */
	level_detect |=
	    ((ssp & detectLevel) << AHINTC_LEVELDETECT_1_LEVEL_SSPI_3_SHIFT);

	/* Apply the settings */
	BRCM_WRITE_REG(base, AHINTC_LEVELDETECT_1, level_detect);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_enable_switch_intr(CHAL_HANDLE handle,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Enable CAPH INTC SWITCH INT
*
****************************************************************************/
cVoid chal_caph_intc_enable_switch_intr(CHAL_HANDLE handle,
					CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_en;

	int_en =
	    BRCM_READ_REG_IDX(base, AHINTC_ENABLE_ARM_STD_1,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_en |= (AHINTC_ENABLE_ARM_STD_1_ENABLE_A_S_ASW_MASK);
	BRCM_WRITE_REG_IDX(base, AHINTC_ENABLE_ARM_STD_1,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_en);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_disable_switch_intr(CHAL_HANDLE handle,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Disable CAPH INTC SWITCH INT
*
****************************************************************************/
cVoid chal_caph_intc_disable_switch_intr(CHAL_HANDLE handle,
					 CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_en;

	int_en =
	    BRCM_READ_REG_IDX(base, AHINTC_ENABLE_ARM_STD_1,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_en &= ~(AHINTC_ENABLE_ARM_STD_1_ENABLE_A_S_ASW_MASK);
	BRCM_WRITE_REG_IDX(base, AHINTC_ENABLE_ARM_STD_1,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_en);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_clr_switch_intr(CHAL_HANDLE handle,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Clear CAPH INTC SWITCH INT
*
****************************************************************************/
cVoid chal_caph_intc_clr_switch_intr(CHAL_HANDLE handle, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_clr;

	int_clr =
	    BRCM_READ_REG_IDX(base, AHINTC_CLEAR_ARM_STD_1,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_clr |= (AHINTC_CLEAR_ARM_STD_1_CLEAR_A_S_ASW_MASK);
	BRCM_WRITE_REG_IDX(base, AHINTC_CLEAR_ARM_STD_1,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_clr);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_read_switch_intr(CHAL_HANDLE handle,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Read CAPH INTC SWITCH INT
*
****************************************************************************/
cUInt8 chal_caph_intc_read_switch_intr(CHAL_HANDLE handle, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_sts;

	int_sts =
	    BRCM_READ_REG_IDX(base, AHINTC_STATUS_ARM_STD_1,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_sts |= (AHINTC_STATUS_ARM_STD_1_STATUS_A_S_ASW_MASK);
	int_sts >>= (AHINTC_STATUS_ARM_STD_1_STATUS_A_S_ASW_SHIFT);

	return int_sts;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_enable_switch_intr_err
*		(CHAL_HANDLE handle,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Enable CAPH INTC SWITCH INT err
*
****************************************************************************/
cVoid chal_caph_intc_enable_switch_int_err(CHAL_HANDLE handle,
					   CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_en;

	int_en =
	    BRCM_READ_REG_IDX(base, AHINTC_ENABLE_ARM_ERR_1,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_en |= (AHINTC_ENABLE_ARM_ERR_1_ENABLE_A_E_ASW_MASK);
	BRCM_WRITE_REG_IDX(base, AHINTC_ENABLE_ARM_ERR_1,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_en);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_disable_switch_intr_err
*	(CHAL_HANDLE handle,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Disable CAPH INTC SWITCH INT err
*
****************************************************************************/
cVoid chal_caph_intc_disable_switch_int_err(CHAL_HANDLE handle,
					    CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_en;

	int_en =
	    BRCM_READ_REG_IDX(base, AHINTC_ENABLE_ARM_ERR_1,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_en &= ~(AHINTC_ENABLE_ARM_ERR_1_ENABLE_A_E_ASW_MASK);
	BRCM_WRITE_REG_IDX(base, AHINTC_ENABLE_ARM_ERR_1,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_en);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_clr_switch_intr_err(CHAL_HANDLE handle,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Clear CAPH INTC SWITCH INT err
*
****************************************************************************/
cVoid chal_caph_intc_clr_switch_int_err(CHAL_HANDLE handle,
					CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_clr;

	int_clr =
	    BRCM_READ_REG_IDX(base, AHINTC_CLEAR_ARM_ERR_1,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_clr |= (AHINTC_CLEAR_ARM_ERR_1_CLEAR_A_E_ASW_MASK);
	BRCM_WRITE_REG_IDX(base, AHINTC_CLEAR_ARM_ERR_1,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_clr);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_read_switch_intr_err(CHAL_HANDLE handle,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Read CAPH INTC SWITCH INT err
*
****************************************************************************/
cUInt8 chal_caph_intc_read_switch_int_err(CHAL_HANDLE handle,
					  CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_sts;

	int_sts =
	    BRCM_READ_REG_IDX(base, AHINTC_STATUS_ARM_ERR_1,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_sts &= (AHINTC_STATUS_ARM_ERR_1_STATUS_A_E_ASW_MASK);
	int_sts >>= (AHINTC_STATUS_ARM_ERR_1_STATUS_A_E_ASW_SHIFT);

	return int_sts;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_set_switch_int_detect_level
*	(CHAL_HANDLE handle,
*            cUInt8 detectLevel)
*
*  Description: Set CAPH INTC SWITCH INT detect level
*
****************************************************************************/
cVoid chal_caph_intc_set_switch_int_detect_level(CHAL_HANDLE handle,
						 cUInt8 detectLevel)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 level_detect;

	level_detect = BRCM_READ_REG(base, AHINTC_LEVELDETECT_1);
	level_detect &= ~(AHINTC_LEVELDETECT_1_LEVEL_ASW_MASK);
	level_detect |= (detectLevel << AHINTC_LEVELDETECT_1_LEVEL_ASW_SHIFT);
	BRCM_WRITE_REG(base, AHINTC_LEVELDETECT_1, level_detect);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_enable_audioh_intr(CHAL_HANDLE handle,
*            cUInt8 fifo,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Enable CAPH INTC AUDIOH INT
*
****************************************************************************/
cVoid chal_caph_intc_enable_audioh_intr(CHAL_HANDLE handle,
					cUInt8 fifo, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_en;

	int_en =
	    BRCM_READ_REG_IDX(base, AHINTC_ENABLE_ARM_STD_1,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_en |= (fifo << AHINTC_ENABLE_ARM_STD_1_ENABLE_A_S_AUDIO_EP_SHIFT);
	BRCM_WRITE_REG_IDX(base, AHINTC_ENABLE_ARM_STD_1,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_en);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_disable_audioh_intr(CHAL_HANDLE handle,
*            cUInt8 fifo,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Disable CAPH INTC AUDIOH INT
*
****************************************************************************/
cVoid chal_caph_intc_disable_audioh_intr(CHAL_HANDLE handle,
					 cUInt8 fifo, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_en;
	cUInt32 int_dis =
	    (fifo << AHINTC_ENABLE_ARM_STD_1_ENABLE_A_S_AUDIO_EP_SHIFT);

	int_en =
	    BRCM_READ_REG_IDX(base, AHINTC_ENABLE_ARM_STD_1,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_en &= (~int_dis);
	BRCM_WRITE_REG_IDX(base, AHINTC_ENABLE_ARM_STD_1,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_en);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_clr_audioh_intr(CHAL_HANDLE handle,
*            cUInt8 fifo,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Clear CAPH INTC AUDIOH INT
*
****************************************************************************/
cVoid chal_caph_intc_clr_audioh_intr(CHAL_HANDLE handle,
				     cUInt8 fifo, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_clr;

	int_clr =
	    BRCM_READ_REG_IDX(base, AHINTC_CLEAR_ARM_STD_1,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_clr |= (fifo << AHINTC_CLEAR_ARM_STD_1_CLEAR_A_S_AUDIO_EP_SHIFT);
	BRCM_WRITE_REG_IDX(base, AHINTC_CLEAR_ARM_STD_1,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_clr);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_read_audioh_intr(CHAL_HANDLE handle,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Read CAPH INTC AUDIOH INT
*
****************************************************************************/
cUInt8 chal_caph_intc_read_audioh_intr(CHAL_HANDLE handle, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_sts;

	int_sts =
	    BRCM_READ_REG_IDX(base, AHINTC_STATUS_ARM_STD_1,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_sts &= (AHINTC_STATUS_ARM_STD_1_STATUS_A_S_AUDIO_EP_MASK);
	int_sts >>= (AHINTC_STATUS_ARM_STD_1_STATUS_A_S_AUDIO_EP_SHIFT);

	return int_sts;
}

/****************************************************************************
*
*  Function: cVoid chal_caph_intc_enable_audioh_intr_err(CHAL_HANDLE handle,
*            cUInt8 fifo,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Enable CAPH INTC AUDIOH INT err
*
****************************************************************************/
cVoid chal_caph_intc_enable_audioh_int_err(CHAL_HANDLE handle,
					   cUInt8 fifo, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_en;

	int_en =
	    BRCM_READ_REG_IDX(base, AHINTC_ENABLE_ARM_ERR_1,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_en |= (fifo << AHINTC_ENABLE_ARM_ERR_1_ENABLE_A_E_AUDIO_HS_SHIFT);
	BRCM_WRITE_REG_IDX(base, AHINTC_ENABLE_ARM_ERR_1,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_en);

	return;
}

/****************************************************************************
*
*  Function: cVoid chal_caph_intc_disable_audioh_intr_err(CHAL_HANDLE handle,
*            cUInt8 fifo,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Disable CAPH INTC AUDIOH INT err
*
****************************************************************************/
cVoid chal_caph_intc_disable_audioh_int_err(CHAL_HANDLE handle,
					    cUInt8 fifo, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_en;
	cUInt32 int_dis =
	    (fifo << AHINTC_ENABLE_ARM_ERR_1_ENABLE_A_E_AUDIO_HS_SHIFT);

	int_en =
	    BRCM_READ_REG_IDX(base, AHINTC_ENABLE_ARM_ERR_1,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_en &= (~int_dis);
	BRCM_WRITE_REG_IDX(base, AHINTC_ENABLE_ARM_ERR_1,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_en);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_clr_audioh_intr_err(CHAL_HANDLE handle,
*            cUInt8 fifo,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Clear CAPH INTC AUDIOH INT err
*
****************************************************************************/
cVoid chal_caph_intc_clr_audioh_int_err(CHAL_HANDLE handle,
					cUInt8 fifo, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_clr;

	int_clr =
	    BRCM_READ_REG_IDX(base, AHINTC_CLEAR_ARM_ERR_1,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_clr |= (fifo << AHINTC_CLEAR_ARM_ERR_1_CLEAR_A_E_AUDIO_EP_SHIFT);
	BRCM_WRITE_REG_IDX(base, AHINTC_CLEAR_ARM_ERR_1,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_clr);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_read_audioh_intr_err(CHAL_HANDLE handle,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Read CAPH INTC AUDIOH INT err
*
****************************************************************************/
cUInt8 chal_caph_intc_read_audioh_int_err(CHAL_HANDLE handle,
					  CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_sts;

	int_sts =
	    BRCM_READ_REG_IDX(base, AHINTC_STATUS_ARM_ERR_1,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_sts &= (AHINTC_STATUS_ARM_ERR_1_STATUS_A_E_AUDIO_EP_MASK);
	int_sts >>= (AHINTC_STATUS_ARM_ERR_1_STATUS_A_E_AUDIO_EP_SHIFT);

	return int_sts;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_set_audioh_int_detect_level
*	(CHAL_HANDLE handle,
*            cUInt8 fifo,
*            cUInt8 detectLevel)
*
*  Description: Set CAPH INTC AUDIOH INT detect level
*
****************************************************************************/
cVoid chal_caph_intc_set_audioh_int_detect_level(CHAL_HANDLE handle,
						 cUInt8 fifo,
						 cUInt8 detectLevel)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 level_detect;

	level_detect = BRCM_READ_REG(base, AHINTC_LEVELDETECT_1);

	/* Clear the required ssp settings */
	level_detect &= (~(fifo << AHINTC_LEVELDETECT_1_LEVEL_AUDIO_EP_SHIFT));
	/* Add the level detection settings for the given channels */
	level_detect |=
	    ((fifo & detectLevel) << AHINTC_LEVELDETECT_1_LEVEL_AUDIO_EP_SHIFT);

	/* Apply the settings */
	BRCM_WRITE_REG(base, AHINTC_LEVELDETECT_1, level_detect);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_enable_tap_intr(CHAL_HANDLE handle,
*            cUInt8 channel,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Enable CAPH INTC TAP INT
*
****************************************************************************/
cVoid chal_caph_intc_enable_tap_intr(CHAL_HANDLE handle,
				     cUInt8 channel, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_en;

	int_en =
	    BRCM_READ_REG_IDX(base, AHINTC_ENABLE_ARM_STD_1,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_en |=
	    (channel << AHINTC_ENABLE_ARM_STD_1_ENABLE_A_S_TAP_INCH4_1_SHIFT);
	BRCM_WRITE_REG_IDX(base, AHINTC_ENABLE_ARM_STD_1,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_en);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_enable_tapout_intr(CHAL_HANDLE handle,
*            cUInt8 channel,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Enable CAPH INTC TAPOUT INT
*
****************************************************************************/
cVoid chal_caph_intc_enable_tapout_intr(CHAL_HANDLE handle,
					cUInt8 channel, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_en;

	int_en =
	    BRCM_READ_REG_IDX(base, AHINTC_ENABLE_ARM_STD_1,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_en |=
	    (channel << AHINTC_ENABLE_ARM_STD_1_ENABLE_A_S_TAP_OUTCH4_1_SHIFT);
	BRCM_WRITE_REG_IDX(base, AHINTC_ENABLE_ARM_STD_1,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_en);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_disable_tap_intr(CHAL_HANDLE handle,
*            cUInt8 channel,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Disable CAPH INTC TAP INT
*
****************************************************************************/
cVoid chal_caph_intc_disable_tap_intr(CHAL_HANDLE handle,
				      cUInt8 channel, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_en;
	cUInt32 int_dis =
	    (channel << AHINTC_ENABLE_ARM_STD_1_ENABLE_A_S_TAP_INCH4_1_SHIFT);

	int_en =
	    BRCM_READ_REG_IDX(base, AHINTC_ENABLE_ARM_STD_1,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_en &= ~(int_dis);
	BRCM_WRITE_REG_IDX(base, AHINTC_ENABLE_ARM_STD_1,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_en);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_disable_tapout_intr(CHAL_HANDLE handle,
*            cUInt8 channel,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Disable CAPH INTC TAPOUT INT
*
****************************************************************************/
cVoid chal_caph_intc_disable_tapout_intr(CHAL_HANDLE handle,
					 cUInt8 channel, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_en;
	cUInt32 int_dis =
	    (channel << AHINTC_ENABLE_ARM_STD_1_ENABLE_A_S_TAP_OUTCH4_1_SHIFT);

	int_en =
	    BRCM_READ_REG_IDX(base, AHINTC_ENABLE_ARM_STD_1,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_en &= ~(int_dis);
	BRCM_WRITE_REG_IDX(base, AHINTC_ENABLE_ARM_STD_1,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_en);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_clr_tap_intr(CHAL_HANDLE handle,
*            cUInt8 channel,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Clear CAPH INTC TAP INT
*
****************************************************************************/
cVoid chal_caph_intc_clr_tap_intr(CHAL_HANDLE handle,
				  cUInt8 channel, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_clr;

	int_clr =
	    BRCM_READ_REG_IDX(base, AHINTC_CLEAR_ARM_STD_1,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_clr |=
	    (channel << AHINTC_CLEAR_ARM_STD_1_CLEAR_A_S_TAP_INCH4_1_SHIFT);
	BRCM_WRITE_REG_IDX(base, AHINTC_CLEAR_ARM_STD_1,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_clr);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_read_tap_intr(CHAL_HANDLE handle,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Read CAPH INTC TAP INT
*
****************************************************************************/
cUInt8 chal_caph_intc_read_tap_intr(CHAL_HANDLE handle, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_sts;

	int_sts =
	    BRCM_READ_REG_IDX(base, AHINTC_STATUS_ARM_STD_1,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_sts &=
	    (AHINTC_STATUS_ARM_STD_1_STATUS_A_S_TAP_INCH4_1_MASK |
	     AHINTC_STATUS_ARM_STD_1_STATUS_A_S_TAP_OUTCH4_1_MASK);
	int_sts >>= (AHINTC_STATUS_ARM_STD_1_STATUS_A_S_TAP_INCH4_1_SHIFT);

	return int_sts;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_enable_tap_int_err(CHAL_HANDLE handle,
*            cUInt8 channel,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Enable CAPH INTC TAP INT err
*
****************************************************************************/
cVoid chal_caph_intc_enable_tap_int_err(CHAL_HANDLE handle,
					cUInt8 channel, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_en;

	int_en =
	    BRCM_READ_REG_IDX(base, AHINTC_ENABLE_ARM_ERR_1,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_en |=
	    (channel << AHINTC_ENABLE_ARM_ERR_1_ENABLE_A_E_TAP_INCH4_1_SHIFT);
	BRCM_WRITE_REG_IDX(base, AHINTC_ENABLE_ARM_ERR_1,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_en);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_disable_tap_int_err(CHAL_HANDLE handle,
*            cUInt8 channel,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Disable CAPH INTC TAP INT err
*
****************************************************************************/
cVoid chal_caph_intc_disable_tap_int_err(CHAL_HANDLE handle,
					 cUInt8 channel, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_en;
	cUInt32 int_dis =
	    (channel << AHINTC_ENABLE_ARM_ERR_1_ENABLE_A_E_TAP_INCH4_1_SHIFT);

	int_en =
	    BRCM_READ_REG_IDX(base, AHINTC_ENABLE_ARM_ERR_1,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_en &= ~(int_dis);
	BRCM_WRITE_REG_IDX(base, AHINTC_ENABLE_ARM_ERR_1,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_en);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_clr_tap_int_err(CHAL_HANDLE handle,
*            cUInt8 channel,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Clear CAPH INTC TAP INT err
*
****************************************************************************/
cVoid chal_caph_intc_clr_tap_int_err(CHAL_HANDLE handle,
				     cUInt8 channel, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_clr;

	int_clr =
	    BRCM_READ_REG_IDX(base, AHINTC_CLEAR_ARM_ERR_1,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_clr |=
	    (channel << AHINTC_CLEAR_ARM_ERR_1_CLEAR_A_E_TAP_INCH4_1_SHIFT);
	BRCM_WRITE_REG_IDX(base, AHINTC_CLEAR_ARM_ERR_1,
			   (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE), int_clr);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_read_tap_int_err(CHAL_HANDLE handle,
*            CAPH_ARM_DSP_e owner)
*
*  Description: Read CAPH INTC TAP INT err
*
****************************************************************************/
cUInt8 chal_caph_intc_read_tap_int_err(CHAL_HANDLE handle, CAPH_ARM_DSP_e owner)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 int_sts;

	int_sts =
	    BRCM_READ_REG_IDX(base, AHINTC_STATUS_ARM_ERR_1,
			      (owner * CHAL_CAPH_INTC_SUBSYS_REG_SIZE));
	int_sts &=
	    (AHINTC_STATUS_ARM_ERR_1_STATUS_A_E_TAP_INCH4_1_MASK |
	     AHINTC_STATUS_ARM_ERR_1_STATUS_A_E_TAP_OUTCH4_1_MASK);
	int_sts >>= (AHINTC_STATUS_ARM_ERR_1_STATUS_A_E_TAP_INCH4_1_SHIFT);

	return int_sts;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_intc_set_tap_int_detect_level
*	(CHAL_HANDLE handle,
*            cUInt8 channel,
*            cUInt8 detectLevel)
*
*  Description: Set CAPH INTC TAP INT detect level
*
****************************************************************************/
cVoid chal_caph_intc_set_tap_int_detect_level(CHAL_HANDLE handle,
					      cUInt8 channel,
					      cUInt8 detectLevel)
{
	cUInt32 base = (cUInt32) handle;
	cUInt32 level_detect;

	level_detect = BRCM_READ_REG(base, AHINTC_LEVELDETECT_1);

	/* Clear the required ssp settings */
	level_detect &=
	    (~(channel << AHINTC_LEVELDETECT_1_LEVEL_TAP_INCH4_1_SHIFT));
	/* Add the level detection settings for the given channels */
	level_detect |=
	    ((channel & detectLevel) <<
	     AHINTC_LEVELDETECT_1_LEVEL_TAP_INCH4_1_SHIFT);

	/* Apply the settings */
	BRCM_WRITE_REG(base, AHINTC_LEVELDETECT_1, level_detect);

	return;
}
