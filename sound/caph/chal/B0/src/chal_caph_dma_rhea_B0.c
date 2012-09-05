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
*  @file   chal_caph_dma.c
*
*  @brief  chal layer driver for caph dma block
*
****************************************************************************/

#include "chal_caph.h"
#include "chal_caph_dma.h"
#include <mach/rdb/brcm_rdb_cph_aadmac.h>
#include <mach/rdb/brcm_rdb_util.h>
#include <linux/io.h>
#include <mach/cpu.h>

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

static cVoid chal_caph_dma_rheaB0_set_hibuffer(CHAL_HANDLE handle,
					       CAPH_DMA_CHANNEL_e channel,
					       cUInt32 address, cUInt32 size);

static cVoid chal_caph_dma_rheaB0_set_ddrfifo_status(CHAL_HANDLE handle,
						     CAPH_DMA_CHANNEL_e channel,
						     CAPH_DMA_CHNL_FIFO_STATUS_e
						     status);

static cVoid chal_caph_dma_rheaB0_clr_ddrfifo_status(CHAL_HANDLE handle,
						     CAPH_DMA_CHANNEL_e channel,
						     CAPH_DMA_CHNL_FIFO_STATUS_e
						     status);

static cVoid chal_caph_dma_rheaB0_clr_channel_fifo(CHAL_HANDLE handle,
						   cUInt16 channel);

static CAPH_DMA_CHNL_FIFO_STATUS_e
chal_caph_dma_rheaB0_read_ddrfifo_sw_status(CHAL_HANDLE handle,
					    CAPH_DMA_CHANNEL_e channel);

/****************************************************************************
*
*  Function Name: CHAL_HANDLE chal_caph_dma_platform_init
* (chal_caph_dma_funcs_t *pfuncs)
*
*  Description: init platform-specific CAPH DMA functions.
*
****************************************************************************/
cBool chal_caph_dma_platform_init(chal_caph_dma_funcs_t *pfuncs)
{
	pfuncs->set_ddrfifo_status = chal_caph_dma_rheaB0_set_ddrfifo_status;
	pfuncs->clr_channel_fifo = chal_caph_dma_rheaB0_clr_channel_fifo;
	pfuncs->clr_ddrfifo_status = chal_caph_dma_rheaB0_clr_ddrfifo_status;
	pfuncs->read_ddrfifo_sw_status =
	    chal_caph_dma_rheaB0_read_ddrfifo_sw_status;
	pfuncs->set_hibuffer = chal_caph_dma_rheaB0_set_hibuffer;
	return TRUE;
}

/****************************************************************************
 local function definitions
****************************************************************************/

/****************************************************************************
*
*  Function Name: cVoid chal_caph_dma_set_buffer(CHAL_HANDLE handle,
*                   CAPH_DMA_CHANNEL_e  channel,
*			    cUInt32                       address,
*                   cUInt32                       size)
*
*  Description: config CAPH DMA channel buffer parameters (address and size)
*
****************************************************************************/
static cVoid chal_caph_dma_rheaB0_set_hibuffer(CHAL_HANDLE handle,
					       CAPH_DMA_CHANNEL_e channel,
					       cUInt32 address, cUInt32 size)
{
	cUInt32 base = ((chal_caph_dma_cb_t *) handle)->base;
	cUInt8 index;
	/*cUInt32 cr = 0;*/

	/* Find the channel we are looking for */
	for (index = 0; index < CHAL_CAPH_DMA_MAX_CHANNELS; index++) {
		if ((1UL << index) & channel) {
			/* Set the DMA buffer Address */
			address &=
			CPH_AADMAC_CH1_AADMAC_CR_3_CH1_AADMAC_HIGH_BASE_MASK;
			BRCM_WRITE_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_CR_3,
					   index, address);
#if 0
			/* enable the use of hi buffer */
			cr = BRCM_READ_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_CR_2,
					       (index *
						CHAL_CAPH_DMA_CH_REG_SIZE));

			/* Configure the use of buffer base address register */
			cr &=
		~CPH_AADMAC_CH1_AADMAC_CR_2_CH1_AADMAC_HIGH_BASE_EN_MASK;
			cr |=
			    (1 <<
		CPH_AADMAC_CH1_AADMAC_CR_2_CH1_AADMAC_HIGH_BASE_EN_SHIFT);

			/* Apply the settings in the hardware */
			BRCM_WRITE_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_CR_2,
					   (index * CHAL_CAPH_DMA_CH_REG_SIZE),
					   cr);
#endif
			break;
		}

	}

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_dma_set_ddrfifo_status(CHAL_HANDLE handle,
*			CAPH_DMA_CHANNEL_e channel,
*			CAPH_DMA_CHNL_FIFO_STATUS_e status)
*
*  Description: set CAPH DMA ddr fifo status
*
****************************************************************************/
static cVoid chal_caph_dma_rheaB0_set_ddrfifo_status(CHAL_HANDLE handle,
						     CAPH_DMA_CHANNEL_e channel,
						     CAPH_DMA_CHNL_FIFO_STATUS_e
						     status)
{
	cUInt32 base = ((chal_caph_dma_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 cr = 0;

	if (cpu_is_rhea_B0()) {

		/*-- Rhea B0-- */

	for (index = 0; index < CHAL_CAPH_DMA_MAX_CHANNELS; index++) {
		if ((1UL << index) & channel) {
			/* found the channel, Set the DDR fifo status */
			cr = BRCM_READ_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_SR_1,
					       (index *
						CHAL_CAPH_DMA_CH_REG_SIZE));
			if (status == CAPH_READY_NONE) {
				cr &=
			(~CPH_AADMAC_CH1_AADMAC_SR_1_CH1_SW_READY_LOW_MASK);
				cr &=
			(~CPH_AADMAC_CH1_AADMAC_SR_1_CH1_SW_READY_HIGH_MASK);
				BRCM_WRITE_REG_IDX(base,
						   CPH_AADMAC_CH1_AADMAC_SR_1,
						   (index *
						    CHAL_CAPH_DMA_CH_REG_SIZE),
						   cr);
			} else {
#if 0
/* this secion of code has a bug:
*/
				if (status & CAPH_READY_LOW) {
					cr = CAPH_READY_LOW <<
			CPH_AADMAC_CH1_AADMAC_SR_1_CH1_SW_READY_LOW_SHIFT;
					BRCM_WRITE_REG_IDX(base,
						CPH_AADMAC_CH1_AADMAC_SR_1,
						(index *
						CHAL_CAPH_DMA_CH_REG_SIZE),
						cr);
				}
				if (status & CAPH_READY_HIGH) {
					cr |=
					    CAPH_READY_HIGH <<
			CPH_AADMAC_CH1_AADMAC_SR_1_CH1_SW_READY_LOW_SHIFT;
					BRCM_WRITE_REG_IDX(base,
						CPH_AADMAC_CH1_AADMAC_SR_1,
						(index *
						CHAL_CAPH_DMA_CH_REG_SIZE),
						cr);
				}
#else
				cr |=
				    (status <<
			CPH_AADMAC_CH1_AADMAC_SR_1_CH1_SW_READY_LOW_SHIFT);
				BRCM_WRITE_REG_IDX(base,
						   CPH_AADMAC_CH1_AADMAC_SR_1,
						   (index *
						    CHAL_CAPH_DMA_CH_REG_SIZE),
						   cr);
#endif
			}
			break;
		}
	}
	} else {

	/*-- Rhea B1 --*/

	/* Find the channel we are looking for */
	for (index = 0; index < CHAL_CAPH_DMA_MAX_CHANNELS; index++) {
		if ((1UL << index) & channel) {
			/* found the channel we are looking for, Set the
			   DDR fifo status */
			cr = BRCM_READ_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_SR_1,
					       (index *
						CHAL_CAPH_DMA_CH_REG_SIZE));

			/* mark to preserve HW_RDY bits */
			cr |= CPH_AADMAC_CH1_AADMAC_SR_1_CH1_HW_READY_LOW_MASK |
			    CPH_AADMAC_CH1_AADMAC_SR_1_CH1_HW_READY_HIGH_MASK;
			if (status == CAPH_READY_NONE) {
				/* set Reset bit */
				cr |=
			CPH_AADMAC_CH1_AADMAC_SR_1_CH1_AADMAC_FIFO_RST_MASK;
				BRCM_WRITE_REG_IDX(base,
						   CPH_AADMAC_CH1_AADMAC_SR_1,
						   (index *
						    CHAL_CAPH_DMA_CH_REG_SIZE),
						   cr);

				/* clear Reset bit */
				cr = BRCM_READ_REG_IDX(base,
					CPH_AADMAC_CH1_AADMAC_SR_1,
					(index*CHAL_CAPH_DMA_CH_REG_SIZE));
				cr &=
			~CPH_AADMAC_CH1_AADMAC_SR_1_CH1_AADMAC_FIFO_RST_MASK;
				cr |=
			CPH_AADMAC_CH1_AADMAC_SR_1_CH1_HW_READY_LOW_MASK
			| CPH_AADMAC_CH1_AADMAC_SR_1_CH1_HW_READY_HIGH_MASK;

				BRCM_WRITE_REG_IDX(base,
						   CPH_AADMAC_CH1_AADMAC_SR_1,
						   (index *
						    CHAL_CAPH_DMA_CH_REG_SIZE),
						   cr);
				return;
			}

	/* cr already set to CPH_AADMAC_CH1_AADMAC_SR_1_CH1_HW_READY_LOW_MASK |
			   CPH_AADMAC_CH1_AADMAC_SR_1_CH1_HW_READY_HIGH_MASK  */
#if 0 /*no need to separate low and high writes*/
			if (status & CAPH_READY_LOW) {
				cr |=
				    CAPH_READY_LOW <<
			CPH_AADMAC_CH1_AADMAC_SR_1_CH1_SW_READY_LOW_SHIFT;
				BRCM_WRITE_REG_IDX(base,
						   CPH_AADMAC_CH1_AADMAC_SR_1,
						   (index *
						    CHAL_CAPH_DMA_CH_REG_SIZE),
						   cr);
			}
			if (status & CAPH_READY_HIGH) {
				cr =
			CPH_AADMAC_CH1_AADMAC_SR_1_CH1_HW_READY_LOW_MASK |
			CPH_AADMAC_CH1_AADMAC_SR_1_CH1_HW_READY_HIGH_MASK;
				cr |=
				    CAPH_READY_HIGH <<
			CPH_AADMAC_CH1_AADMAC_SR_1_CH1_SW_READY_LOW_SHIFT;
				BRCM_WRITE_REG_IDX(base,
						   CPH_AADMAC_CH1_AADMAC_SR_1,
						   (index *
						    CHAL_CAPH_DMA_CH_REG_SIZE),
						   cr);
			}
#else
			cr |= (status <<
			CPH_AADMAC_CH1_AADMAC_SR_1_CH1_SW_READY_LOW_SHIFT);
			BRCM_WRITE_REG_IDX(base,
					   CPH_AADMAC_CH1_AADMAC_SR_1,
					   (index *
					    CHAL_CAPH_DMA_CH_REG_SIZE),
					   cr);
#endif

			break;
		}
	}

	}
	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_dma_clr_ddrfifo_status(CHAL_HANDLE handle,
*			CAPH_DMA_CHANNEL_e channel,
*			CAPH_DMA_CHNL_FIFO_STATUS_e status)
*
*  Description: clear CAPH DMA ddr fifo status
*
****************************************************************************/
static cVoid chal_caph_dma_rheaB0_clr_ddrfifo_status(CHAL_HANDLE handle,
						     CAPH_DMA_CHANNEL_e channel,
						     CAPH_DMA_CHNL_FIFO_STATUS_e
						     status)
{
	cUInt32 base = ((chal_caph_dma_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 sr;

	/*if (cpu_is_rhea_B0()) {*/

		/*-- Rhea B0 --*/

	/* Find the channel */
	for (index = 0; index < CHAL_CAPH_DMA_MAX_CHANNELS; index++) {
		if ((1UL << index) & channel) {
			/* found the channel, Get the channel status */
			sr = BRCM_READ_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_SR_1,
					       (index *
						CHAL_CAPH_DMA_CH_REG_SIZE));
			/* write 0 to clear the bits that had been set */
			sr &=
			    ~((status & CAPH_READY_HIGHLOW) <<
			  CPH_AADMAC_CH1_AADMAC_SR_1_CH1_HW_READY_LOW_SHIFT);
			BRCM_WRITE_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_SR_1,
					   (index * CHAL_CAPH_DMA_CH_REG_SIZE),
					   sr);
			break;
		}
	}
#if 0 /*B0 code does the same thing*/
	} else {

		/*-- Rhea B1 --*/

		sr = CPH_AADMAC_CH1_AADMAC_SR_1_CH1_HW_READY_LOW_MASK |
	    CPH_AADMAC_CH1_AADMAC_SR_1_CH1_HW_READY_HIGH_MASK;

	/* Find the channel we are looking for */
	for (index = 0; index < CHAL_CAPH_DMA_MAX_CHANNELS; index++) {
		if ((1UL << index) & channel) {
			if (status & CAPH_READY_LOW) {
				/* Write 0 to clear the
				specified HW_RDY_LOW bit */
				sr = (~CAPH_READY_LOW & CAPH_READY_HIGHLOW) <<
			CPH_AADMAC_CH1_AADMAC_SR_1_CH1_HW_READY_LOW_SHIFT;
				BRCM_WRITE_REG_IDX(base,
						   CPH_AADMAC_CH1_AADMAC_SR_1,
						   (index *
						    CHAL_CAPH_DMA_CH_REG_SIZE),
						   sr);
			}
			if (status & CAPH_READY_HIGH) {
				/* Write 0 to clear specified HW_RDY_HIGH bit */
				sr = (~CAPH_READY_HIGH & CAPH_READY_HIGHLOW) <<
			CPH_AADMAC_CH1_AADMAC_SR_1_CH1_HW_READY_LOW_SHIFT;
				BRCM_WRITE_REG_IDX(base,
						   CPH_AADMAC_CH1_AADMAC_SR_1,
						   (index *
						    CHAL_CAPH_DMA_CH_REG_SIZE),
						   sr);
			}
			break;
		}

	}

	}
#endif
	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_dma_clr_channel_fifo(CHAL_HANDLE handle,
*			cUInt16 fifo)
*
*  Description: clear dma channel internal fifo
*
****************************************************************************/
static cVoid chal_caph_dma_rheaB0_clr_channel_fifo(CHAL_HANDLE handle,
						   cUInt16 channel)
{
	cUInt32 base = ((chal_caph_dma_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 cr;

/*B1 version preserves HW_READY bits, but somehow it may cause crash under
  stress (audio log enabled)
*/
	/*if (cpu_is_rhea_B0()) {*/

		/*-- Rhea B0 --*/

	/* Find the FIFOs we are looking for */
	for (index = 0; index < CHAL_CAPH_DMA_MAX_CHANNELS; index++) {
		if ((1UL << index) & channel) {
			/* found the Channel, Disable the FIFO */
			cr = BRCM_READ_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_SR_1,
					       (index *
						CHAL_CAPH_DMA_CH_REG_SIZE));

			/* Send Reset Pulse to Hardware. First make sure is 0,
				set to 1, then clear to 0 */
			/* Clear Reset */
			cr &=
			  ~CPH_AADMAC_CH1_AADMAC_SR_1_CH1_AADMAC_FIFO_RST_MASK;
			BRCM_WRITE_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_SR_1,
					   (index * CHAL_CAPH_DMA_CH_REG_SIZE),
					   cr);

			/* Start Reset  process on Hardware */
			cr = BRCM_READ_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_SR_1,
					       (index *
						CHAL_CAPH_DMA_CH_REG_SIZE));
			cr |=
			    CPH_AADMAC_CH1_AADMAC_SR_1_CH1_AADMAC_FIFO_RST_MASK;
			BRCM_WRITE_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_SR_1,
					   (index * CHAL_CAPH_DMA_CH_REG_SIZE),
					   cr);

			/* Clear Reset */
			cr = BRCM_READ_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_SR_1,
					       (index *
						CHAL_CAPH_DMA_CH_REG_SIZE));
			cr &=
			~CPH_AADMAC_CH1_AADMAC_SR_1_CH1_AADMAC_FIFO_RST_MASK;
			BRCM_WRITE_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_SR_1,
					   (index * CHAL_CAPH_DMA_CH_REG_SIZE),
					   cr);
		}
	}
#if 0
	} else {
		/*-- Rhea B1 --*/

		/* Find the FIFOs we are looking for */
	for (index = 0; index < CHAL_CAPH_DMA_MAX_CHANNELS; index++) {
		if ((1UL << index) & channel) {
			/* SW_RDY bits are cleared by
			writing 0, then 1, and 0 to RST bit */
			/* Clear Reset by writing 0
			to FIFO_RST bits, while still preserving HW_RDY bits */
			cr = BRCM_READ_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_SR_1,
					       (index *
						CHAL_CAPH_DMA_CH_REG_SIZE));
			cr &=
			~CPH_AADMAC_CH1_AADMAC_SR_1_CH1_AADMAC_FIFO_RST_MASK;
			cr |= CPH_AADMAC_CH1_AADMAC_SR_1_CH1_HW_READY_LOW_MASK
			| CPH_AADMAC_CH1_AADMAC_SR_1_CH1_HW_READY_HIGH_MASK;
			BRCM_WRITE_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_SR_1,
					   (index * CHAL_CAPH_DMA_CH_REG_SIZE),
					   cr);

			/* Set Reset bit */
			cr = BRCM_READ_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_SR_1,
					       (index *
						CHAL_CAPH_DMA_CH_REG_SIZE));
			cr |= CPH_AADMAC_CH1_AADMAC_SR_1_CH1_HW_READY_LOW_MASK |
			   CPH_AADMAC_CH1_AADMAC_SR_1_CH1_HW_READY_HIGH_MASK |
			   CPH_AADMAC_CH1_AADMAC_SR_1_CH1_AADMAC_FIFO_RST_MASK;
			BRCM_WRITE_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_SR_1,
					   (index * CHAL_CAPH_DMA_CH_REG_SIZE),
					   cr);

			/* Clear Reset bit */
			cr = BRCM_READ_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_SR_1,
					       (index *
						CHAL_CAPH_DMA_CH_REG_SIZE));
			cr &=
			~CPH_AADMAC_CH1_AADMAC_SR_1_CH1_AADMAC_FIFO_RST_MASK;
			cr |= CPH_AADMAC_CH1_AADMAC_SR_1_CH1_HW_READY_LOW_MASK |
			CPH_AADMAC_CH1_AADMAC_SR_1_CH1_HW_READY_HIGH_MASK;
			BRCM_WRITE_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_SR_1,
					   (index * CHAL_CAPH_DMA_CH_REG_SIZE),
					   cr);
		}

	}
	}
#endif
	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_dma_read_ddrfifo_sw_status(CHAL_HANDLE handle,
*			CAPH_DMA_CHANNEL_e channel)
*
*  Description: read CAPH DMA ddr fifo sw status
*
****************************************************************************/
static CAPH_DMA_CHNL_FIFO_STATUS_e
chal_caph_dma_rheaB0_read_ddrfifo_sw_status(CHAL_HANDLE handle,
					    CAPH_DMA_CHANNEL_e channel)
{
	cUInt32 base = ((chal_caph_dma_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 cr = (cUInt32) CAPH_READY_NONE;

	/* Find the channel */
	for (index = 0; index < CHAL_CAPH_DMA_MAX_CHANNELS; index++) {
		if ((1UL << index) & channel) {
			/* found the channel, Get the channel status */
			cr = BRCM_READ_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_SR_1,
					       (index *
						CHAL_CAPH_DMA_CH_REG_SIZE));

			/* Retrieve the DDR FIFO staus from status */
			cr &=
			   (CPH_AADMAC_CH1_AADMAC_SR_1_CH1_SW_READY_HIGH_MASK |
			    CPH_AADMAC_CH1_AADMAC_SR_1_CH1_SW_READY_LOW_MASK);
			cr >>=
			    CPH_AADMAC_CH1_AADMAC_SR_1_CH1_SW_READY_LOW_SHIFT;
			break;
		}
	}

	return (CAPH_DMA_CHNL_FIFO_STATUS_e) cr;
}
