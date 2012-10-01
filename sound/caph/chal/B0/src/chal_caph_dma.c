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

/****************************************************************************
* local typedef declarations
*****************************************************************************/

/****************************************************************************
* local variable definitions
*****************************************************************************/
/* chal control block where all information is stored */
static chal_caph_dma_cb_t chal_caph_dma_cb;
static chal_caph_dma_funcs_t chal_caph_dma_funcs = { 0 };

/****************************************************************************
* local function declarations
*****************************************************************************/
static cVoid chal_caph_dma_default_set_hibuffer(CHAL_HANDLE handle,
						CAPH_DMA_CHANNEL_e channel,
						cUInt32 address, cUInt32 size);

/******************************************************************************
* local function definitions
*******************************************************************************/

/****************************************************************************
*
*  Function Name: CHAL_HANDLE chal_caph_dma_init(cUInt32 baseAddress)
*
*  Description: init CAPH DMA block
*
****************************************************************************/
CHAL_HANDLE chal_caph_dma_init(cUInt32 baseAddress)
{
	cUInt8 ch;

	/* Go through all the channels and set them not allocated */
	for (ch = 0; ch < CHAL_CAPH_DMA_MAX_CHANNELS; ch++)
		chal_caph_dma_cb.alloc_status[ch] = FALSE;

	/* Set the register base address to the caller supplied base address */
	chal_caph_dma_cb.base = baseAddress;
	chal_caph_dma_funcs.set_hibuffer = chal_caph_dma_default_set_hibuffer;
	chal_caph_dma_platform_init(&chal_caph_dma_funcs);

	/* Initialize AADMAC wrap size for all channels, any number greater than
	   1 is fine */
	{
		CHAL_HANDLE handle = (CHAL_HANDLE) (&chal_caph_dma_cb);
		CAPH_DMA_CHANNEL_e caph_aadmac_ch;
		for (ch = 0; ch < CHAL_CAPH_DMA_MAX_CHANNELS; ch++) {
			caph_aadmac_ch = (CAPH_DMA_CHANNEL_e) (1UL << ch);
			chal_caph_dma_set_buffer(handle, caph_aadmac_ch, 0,
						 0x100);
		}
	}
	return (CHAL_HANDLE) (&chal_caph_dma_cb);
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_dma_deinit(CHAL_HANDLE handle)
*
*  Description: deinit CAPH DMA block
*
****************************************************************************/
cVoid chal_caph_dma_deinit(CHAL_HANDLE handle)
{
	chal_caph_dma_cb_t *pchal_cb = (chal_caph_dma_cb_t *) handle;
	cUInt8 ch;

	/* Go through all the channels and set them not allocated */
	for (ch = 0; ch < CHAL_CAPH_DMA_MAX_CHANNELS; ch++)
		pchal_cb->alloc_status[ch] = FALSE;

	/* Reset the register base address */
	pchal_cb->base = 0;

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_dma_enable(CHAL_HANDLE handle,
*			cUInt16 channel)
*
*  Description: enable CAPH DMA channel
*
****************************************************************************/
cVoid chal_caph_dma_enable(CHAL_HANDLE handle, cUInt16 channel)
{
	cUInt32 base = ((chal_caph_dma_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 reg_val;

	/* Find the channel we are looking for */
	for (index = 0; index < CHAL_CAPH_DMA_MAX_CHANNELS; index++) {
		if ((1UL << index) & channel) {
			/* found the channel, enable the channel */
			reg_val =
			    BRCM_READ_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_CR_2,
					      (index *
					       CHAL_CAPH_DMA_CH_REG_SIZE));
			reg_val |=
			    CPH_AADMAC_CH1_AADMAC_CR_2_CH1_AADMAC_EN_MASK;
			BRCM_WRITE_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_CR_2,
					   (index * CHAL_CAPH_DMA_CH_REG_SIZE),
					   reg_val);
		}

	}

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_dma_disable(CHAL_HANDLE handle,
*			cUInt16 channel)
*
*  Description: disable CAPH DMA channel
*
****************************************************************************/
cVoid chal_caph_dma_disable(CHAL_HANDLE handle, cUInt16 channel)
{
	cUInt32 base = ((chal_caph_dma_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 reg_val;

	/* Find the channel we are looking for */
	for (index = 0; index < CHAL_CAPH_DMA_MAX_CHANNELS; index++) {
		if ((1UL << index) & channel) {
			/* found the channel, disable the channel */
			reg_val =
			    BRCM_READ_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_CR_2,
					      (index *
					       CHAL_CAPH_DMA_CH_REG_SIZE));
			reg_val &=
			    ~CPH_AADMAC_CH1_AADMAC_CR_2_CH1_AADMAC_EN_MASK;
			BRCM_WRITE_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_CR_2,
					   (index * CHAL_CAPH_DMA_CH_REG_SIZE),
					   reg_val);
		}

	}

	return;
}

/****************************************************************************
*
*  Function Name: CAPH_DMA_CHANNEL_e chal_caph_dma_alloc_channel
* (CHAL_HANDLE handle)
*
*  Description: allocate CAPH DMA channel
*
****************************************************************************/
CAPH_DMA_CHANNEL_e chal_caph_dma_alloc_channel(CHAL_HANDLE handle)
{
	chal_caph_dma_cb_t *pchal_cb = (chal_caph_dma_cb_t *) handle;
	cUInt32 ch;

	/* Look for a free (non-allocated) channel  */

	for (ch = 2; ch < CHAL_CAPH_DMA_MAX_CHANNELS; ch++) {
		if (pchal_cb->alloc_status[ch] == FALSE) {
			/* Found one */
			break;
		}
	}


	if (ch < CHAL_CAPH_DMA_MAX_CHANNELS) {
		/* Found a free channel */
		pchal_cb->alloc_status[ch] = TRUE;
	}

	/* Convert to CAPH_DMA_CHANNEL_e format and return */
	return (CAPH_DMA_CHANNEL_e) (1UL << ch);
}

/****************************************************************************
*
*  Function Name: CAPH_DMA_CHANNEL_e chal_caph_dma_alloc_given_channel
*	(CHAL_HANDLE handle)
*
*  Description: allocate a given CAPH DMA channel
*
****************************************************************************/
CAPH_DMA_CHANNEL_e chal_caph_dma_alloc_given_channel(CHAL_HANDLE handle,
						     CAPH_DMA_CHANNEL_e channel)
{
	chal_caph_dma_cb_t *pchal_cb = (chal_caph_dma_cb_t *) handle;
	cUInt32 ch = 0;

	if (channel != CAPH_DMA_CH_VOID) {
		/* Look whether the given channel is allocated or not  */
		for (; ch < CHAL_CAPH_DMA_MAX_CHANNELS; ch++) {
			if ((1UL << ch) & channel) {
				if (pchal_cb->alloc_status[ch] == FALSE) {
					/* Requested channel is not allocated */
					break;
				}
			}
		}

		/* Requested channel is already allocated */
	} else {
		/* Look for a free (non-allocated) channel  */
		for (; ch < CHAL_CAPH_DMA_MAX_CHANNELS; ch++) {
			if (pchal_cb->alloc_status[ch] == FALSE) {
				/* Found one */
				break;
			}
		}
	}

	if (ch < CHAL_CAPH_DMA_MAX_CHANNELS) {
		/* Found a free channel */
		pchal_cb->alloc_status[ch] = TRUE;
	}

	/* Convert to CAPH_DMA_CHANNEL_e format and return */
	return (CAPH_DMA_CHANNEL_e) (1UL << ch);
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_dma_free_channel(CHAL_HANDLE handle,
*			CAPH_DMA_CHANNEL_e channel)
*
*  Description: free CAPH DMA channel
*
****************************************************************************/
cVoid chal_caph_dma_free_channel(CHAL_HANDLE handle, CAPH_DMA_CHANNEL_e channel)
{
	chal_caph_dma_cb_t *pchal_cb = (chal_caph_dma_cb_t *) handle;
	cUInt8 index;

	/* Find the channel we are looking for */
	for (index = 0; index < CHAL_CAPH_DMA_MAX_CHANNELS; index++) {
		if ((1UL << index) & channel) {
			/* found the channel, make this channel free */
			/* for next allocation */
			pchal_cb->alloc_status[index] = FALSE;
			break;
		}

	}

	return;
}


/****************************************************************************
*
*  Function Name: cVoid chal_caph_dma_clear_register(CHAL_HANDLE handle,
*                   CAPH_DMA_CHANNEL_e channel)
*
*  Description: Clear CR1 and CR2 registers before starting configuration
*
****************************************************************************/
cVoid chal_caph_dma_clear_register(CHAL_HANDLE handle,
				  CAPH_DMA_CHANNEL_e channel)
{
	cUInt32 base = ((chal_caph_dma_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 cr = 0;

	/* Find the channel we are looking for */
	for (index = 0; index < CHAL_CAPH_DMA_MAX_CHANNELS; index++) {
		if ((1UL << index) & channel) {

			/* Apply the settings in the hardware */
			BRCM_WRITE_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_CR_2,
					   (index * CHAL_CAPH_DMA_CH_REG_SIZE),
					   cr);

			BRCM_WRITE_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_CR_1,
					   (index * CHAL_CAPH_DMA_CH_REG_SIZE),
					   cr);
			break;
		}

	}

	return;
}


/****************************************************************************
*
*  Function Name: cVoid chal_caph_dma_set_direction(CHAL_HANDLE handle,
*                   CAPH_DMA_CHANNEL_e channel,
*			CAPH_CFIFO_CHNL_DIRECTION_e direction)
*
*  Description: config CAPH DMA channel transfer direction
*
****************************************************************************/
cVoid chal_caph_dma_set_direction(CHAL_HANDLE handle,
				  CAPH_DMA_CHANNEL_e channel,
				  CAPH_CFIFO_CHNL_DIRECTION_e direction)
{
	cUInt32 base = ((chal_caph_dma_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 cr = 0;

	/* Find the channel we are looking for */
	for (index = 0; index < CHAL_CAPH_DMA_MAX_CHANNELS; index++) {
		if ((1UL << index) & channel) {
			/* Get the current configuration */
			cr = BRCM_READ_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_CR_2,
					       (index *
						CHAL_CAPH_DMA_CH_REG_SIZE));

			/* configure the direction of transfer */
			cr &=
			    ~CPH_AADMAC_CH1_AADMAC_CR_2_CH1_AADMAC_IN_OUT_MASK;
			cr |=
			    (direction <<
			CPH_AADMAC_CH1_AADMAC_CR_2_CH1_AADMAC_IN_OUT_SHIFT);

			/* Apply the settings in the hardware */
			BRCM_WRITE_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_CR_2,
					   (index * CHAL_CAPH_DMA_CH_REG_SIZE),
					   cr);
			break;
		}

	}

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_dma_set_cfifo(CHAL_HANDLE handle,
*                   CAPH_DMA_CHANNEL_e channel,
*			CAPH_CFIFO_e            cfifo_id)
*
*  Description: config CAPH DMA channel cfifo channel id
*
****************************************************************************/
cVoid chal_caph_dma_set_cfifo(CHAL_HANDLE handle,
			      CAPH_DMA_CHANNEL_e channel, CAPH_CFIFO_e cfifo_id)
{
	cUInt32 base = ((chal_caph_dma_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 cr = 0;
	cUInt8 cfifo_ch;

	/* Find the channel we are looking for */
	for (index = 0; index < CHAL_CAPH_DMA_MAX_CHANNELS; index++) {
		if ((1UL << index) & channel) {
			cr = BRCM_READ_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_CR_2,
					       (index *
						CHAL_CAPH_DMA_CH_REG_SIZE));

			/* configure the CFIFO channel */
			cr &=
			    ~CPH_AADMAC_CH1_AADMAC_CR_2_CH1_AADMAC_FIFO_CH_MASK;

			/* Find the cfifo channel we are looking for */
			for (cfifo_ch = 0;
			     cfifo_ch < CHAL_CAPH_DMA_MAX_CHANNELS;
			     cfifo_ch++) {
				if ((1UL << cfifo_ch) & cfifo_id) {
					cr |=
					    (cfifo_ch <<
			CPH_AADMAC_CH1_AADMAC_CR_2_CH1_AADMAC_FIFO_CH_SHIFT);
					break;
				}
			}

			/* Apply the settings in the hardware */
			BRCM_WRITE_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_CR_2,
					   (index * CHAL_CAPH_DMA_CH_REG_SIZE),
					   cr);
			break;
		}

	}

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_dma_set_tsize(CHAL_HANDLE handle,
*                   CAPH_DMA_CHANNEL_e channel,
*			cUInt8              tsize)
*
*  Description: config CAPH DMA channel transfer size
*
****************************************************************************/
cVoid chal_caph_dma_set_tsize(CHAL_HANDLE handle,
			      CAPH_DMA_CHANNEL_e channel, cUInt8 tsize)
{
	cUInt32 base = ((chal_caph_dma_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 cr = 0;

	/* Find the channel we are looking for */
	for (index = 0; index < CHAL_CAPH_DMA_MAX_CHANNELS; index++) {
		if ((1UL << index) & channel) {
			cr = BRCM_READ_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_CR_2,
					       (index *
						CHAL_CAPH_DMA_CH_REG_SIZE));

			/* Convert the tsize to the register value,
			   tSIZE should be in number of 32bit transfers -1 */
			if (tsize > sizeof(cUInt32))
				tsize = (tsize / sizeof(cUInt32)) - 1;
			else
				tsize = 0;

			/* Configure the transfer size per request */
			cr &=
	~CPH_AADMAC_CH1_AADMAC_CR_2_CH1_AADMAC_TSIZE_PER_REQ_MASK;
			cr |=
			    (tsize <<
	CPH_AADMAC_CH1_AADMAC_CR_2_CH1_AADMAC_TSIZE_PER_REQ_SHIFT);

			/* Apply the settings in the hardware */
			BRCM_WRITE_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_CR_2,
					   (index * CHAL_CAPH_DMA_CH_REG_SIZE),
					   cr);
			break;
		}

	}

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_dma_set_buffer(CHAL_HANDLE handle,
*                   CAPH_DMA_CHANNEL_e channel,
*			cUInt32             address,
*                   cUInt32             size)
*
*  Description: config CAPH DMA channel buffer parameters (address and size)
*
****************************************************************************/
cVoid chal_caph_dma_set_buffer(CHAL_HANDLE handle,
			       CAPH_DMA_CHANNEL_e channel,
			       cUInt32 address, cUInt32 size)
{
	cUInt32 base = ((chal_caph_dma_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 cr = 0;
	cUInt32 maxs = CPH_AADMAC_DMA_MAX_WRAP_SIZE;
	cUInt32 extds = 0;

	if (channel == CAPH_DMA_CH1 || channel == CAPH_DMA_CH2)
		maxs = CPH_AADMAC_DMA_CH1_2_MAX_WRAP_SIZE;

	if (size > maxs)
		size = maxs;

	/* Set the extended size */
	extds = size >> 16;
	size &= CPH_AADMAC_CH1_AADMAC_CR_2_CH1_AADMAC_WRAP_MASK;

	/* Find the channel we are looking for */
	for (index = 0; index < CHAL_CAPH_DMA_MAX_CHANNELS; index++) {
		if ((1UL << index) & channel) {
			/* Set the DMA buffer Address */
			address &=
			    CPH_AADMAC_CH1_AADMAC_CR_1_CH1_AADMAC_BASE_MASK;
			BRCM_WRITE_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_CR_1,
					   (index * CHAL_CAPH_DMA_CH_REG_SIZE),
					   address);

			cr = BRCM_READ_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_CR_2,
					       (index *
						CHAL_CAPH_DMA_CH_REG_SIZE));

			/* Configure the size of buffer  */
			cr &= ~CPH_AADMAC_CH1_AADMAC_CR_2_CH1_AADMAC_WRAP_MASK;
			cr |=
			    (size <<
			     CPH_AADMAC_CH1_AADMAC_CR_2_CH1_AADMAC_WRAP_SHIFT);

			/* Apply the settings in the hardware */
			BRCM_WRITE_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_CR_2,
					   (index * CHAL_CAPH_DMA_CH_REG_SIZE),
					   cr);

			if (channel == CAPH_DMA_CH1 ||
				channel == CAPH_DMA_CH2) {
				cr = BRCM_READ_REG(base,
					CPH_AADMAC_CH1_2_EXTENDED_WRAP);
				if (channel == CAPH_DMA_CH1) {
					cr &=
	(~CPH_AADMAC_CH1_2_EXTENDED_WRAP_CH1_AADMAC_WRAP_HIGH_MASK);
					cr |=
					    (extds <<
	CPH_AADMAC_CH1_2_EXTENDED_WRAP_CH1_AADMAC_WRAP_HIGH_SHIFT);
				} else {
					cr &=
	(~CPH_AADMAC_CH1_2_EXTENDED_WRAP_CH2_AADMAC_WRAP_HIGH_MASK);
					cr |=
					    (extds <<
	CPH_AADMAC_CH1_2_EXTENDED_WRAP_CH2_AADMAC_WRAP_HIGH_SHIFT);
				}
				/* Apply the settings in the hardware */
				BRCM_WRITE_REG(base,
					       CPH_AADMAC_CH1_2_EXTENDED_WRAP,
					       cr);
			}
			break;
		}

	}

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_dma_set_buffer_address(CHAL_HANDLE handle,
*                   CAPH_DMA_CHANNEL_e channel,
*			cUInt32             address)
*
*  Description: set CAPH DMA channel buffer address
*
****************************************************************************/
cVoid chal_caph_dma_set_buffer_address(CHAL_HANDLE handle,
				       CAPH_DMA_CHANNEL_e channel,
				       cUInt32 address)
{
	cUInt32 base = ((chal_caph_dma_cb_t *) handle)->base;
	cUInt8 index;

	/* Find the channel we are looking for */
	for (index = 0; index < CHAL_CAPH_DMA_MAX_CHANNELS; index++) {
		if ((1UL << index) & channel) {
			/* Set the DMA buffer Address */
			address &=
			    CPH_AADMAC_CH1_AADMAC_CR_1_CH1_AADMAC_BASE_MASK;
			BRCM_WRITE_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_CR_1,
					   (index * CHAL_CAPH_DMA_CH_REG_SIZE),
					   address);
			break;
		}
	}

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_dma_set_buffer(CHAL_HANDLE handle,
*                   CAPH_DMA_CHANNEL_e channel,
*			cUInt32             address,
*                   cUInt32             size)
*
*  Description: config CAPH DMA channel buffer parameters (address and size)
*
****************************************************************************/
cVoid chal_caph_dma_set_hibuffer(CHAL_HANDLE handle,
				 CAPH_DMA_CHANNEL_e channel,
				 cUInt32 address, cUInt32 size)
{
	chal_caph_dma_funcs.set_hibuffer(handle, channel, address, size);
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
cVoid chal_caph_dma_set_ddrfifo_status(CHAL_HANDLE handle,
				       CAPH_DMA_CHANNEL_e channel,
				       CAPH_DMA_CHNL_FIFO_STATUS_e status)
{
	chal_caph_dma_funcs.set_ddrfifo_status(handle, channel, status);
	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_dma_clr_ddrfifo_status(CHAL_HANDLE handle,
*			CAPH_DMA_CHANNEL_e channel,
*			CAPH_DMA_CHNL_FIFO_STATUS_e status)
*
*  Description: set CAPH DMA ddr fifo status
*
****************************************************************************/
cVoid chal_caph_dma_clr_ddrfifo_status(CHAL_HANDLE handle,
				       CAPH_DMA_CHANNEL_e channel,
				       CAPH_DMA_CHNL_FIFO_STATUS_e status)
{
	chal_caph_dma_funcs.clr_ddrfifo_status(handle, channel, status);
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
cVoid chal_caph_dma_clr_channel_fifo(CHAL_HANDLE handle, cUInt16 channel)
{
	chal_caph_dma_funcs.clr_channel_fifo(handle, channel);
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
CAPH_DMA_CHNL_FIFO_STATUS_e chal_caph_dma_read_ddrfifo_sw_status(
				CHAL_HANDLE	handle,
				CAPH_DMA_CHANNEL_e	channel)
{
	return chal_caph_dma_funcs.read_ddrfifo_sw_status(handle, channel);
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_dma_read_ddrfifo_status(CHAL_HANDLE handle,
*			CAPH_DMA_CHANNEL_e channel)
*
*  Description: read CAPH DMA ddr fifo status
*
****************************************************************************/
CAPH_DMA_CHNL_FIFO_STATUS_e chal_caph_dma_read_ddrfifo_status(CHAL_HANDLE
							      handle,
							      CAPH_DMA_CHANNEL_e
							      channel)
{
	cUInt32 base = ((chal_caph_dma_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 sr = (cUInt32) CAPH_READY_NONE;

	/* Find the channel we are looking for */
	for (index = 0; index < CHAL_CAPH_DMA_MAX_CHANNELS; index++) {
		if ((1UL << index) & channel) {
			/* found the channel, Get the channel status */
			sr = BRCM_READ_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_SR_1,
					       (index *
						CHAL_CAPH_DMA_CH_REG_SIZE));

			/* Retrieve the DDR FIFO staus information */
			sr &=
			    (CPH_AADMAC_CH1_AADMAC_SR_1_CH1_HW_READY_HIGH_MASK |
			     CPH_AADMAC_CH1_AADMAC_SR_1_CH1_HW_READY_LOW_MASK);
			sr >>=
			    CPH_AADMAC_CH1_AADMAC_SR_1_CH1_HW_READY_LOW_SHIFT;
			break;
		}
	}

	return (CAPH_DMA_CHNL_FIFO_STATUS_e) sr;
}

/****************************************************************************
*
*  Function Name: cUInt8 chal_caph_dma_read_reqcount(CHAL_HANDLE handle,
*			CAPH_DMA_CHANNEL_e channel)
*
*  Description: read CAPH DMA request count
*
****************************************************************************/
cUInt8 chal_caph_dma_read_reqcount(CHAL_HANDLE handle,
				   CAPH_DMA_CHANNEL_e channel)
{
	cUInt32 base = ((chal_caph_dma_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 sr = 0;

	/* Find the channel we are looking for */
	for (index = 0; index < CHAL_CAPH_DMA_MAX_CHANNELS; index++) {
		if ((1UL << index) & channel) {
			/* found the channel, Get the channel status */
			sr = BRCM_READ_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_SR_1,
					       (index *
						CHAL_CAPH_DMA_CH_REG_SIZE));

			/* Retrieve the request count information from status */
			sr &=
		CPH_AADMAC_CH1_AADMAC_SR_1_CH1_AADMAC_REQUEST_COUNT_MASK;
			sr >>=
		CPH_AADMAC_CH1_AADMAC_SR_1_CH1_AADMAC_REQUEST_COUNT_SHIFT;
			break;
		}

	}

	return (cUInt8) sr;
}

/****************************************************************************
*
*  Function Name: cUInt16 chal_caph_dma_read_currmempointer(CHAL_HANDLE handle,
*			CAPH_DMA_CHANNEL_e channel)
*
*  Description: read CAPH DMA current memory pointer
*
****************************************************************************/
cUInt16 chal_caph_dma_read_currmempointer(CHAL_HANDLE handle,
					  CAPH_DMA_CHANNEL_e channel)
{
	cUInt32 base = ((chal_caph_dma_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 sr = 0;

	/* Find the channel we are looking for */
	for (index = 0; index < CHAL_CAPH_DMA_MAX_CHANNELS; index++) {
		if ((1UL << index) & channel) {
			/* Get the channel status */
			sr = BRCM_READ_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_SR_1,
					       (index *
						CHAL_CAPH_DMA_CH_REG_SIZE));

			/* Extract Current Memory pointer info */
			sr &=
		CPH_AADMAC_CH1_AADMAC_SR_1_CH1_AADMAC_CURRENT_MEM_PNTR_MASK;
			sr >>=
		CPH_AADMAC_CH1_AADMAC_SR_1_CH1_AADMAC_CURRENT_MEM_PNTR_SHIFT;
			break;
		}

	}

	/* return the Current DMA Memory Pointer */
	return (cUInt16) sr;
}

/****************************************************************************
*
*  Function Name: cUInt16 chal_caph_dma_check_dmabuffer(CHAL_HANDLE handle,
*			CAPH_DMA_CHANNEL_e channel)
*
*  Description: check which buffer is being used by dma
*
****************************************************************************/
cUInt16 chal_caph_dma_check_dmabuffer(CHAL_HANDLE handle,
					  CAPH_DMA_CHANNEL_e channel)
{
	cUInt32 base = ((chal_caph_dma_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 sr = 0;

	/* Find the channel we are looking for */
	for (index = 0; index < CHAL_CAPH_DMA_MAX_CHANNELS; index++) {
		if ((1UL << index) & channel) {
			/* Get the channel status */
			sr = BRCM_READ_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_SR_1,
					       (index *
						CHAL_CAPH_DMA_CH_REG_SIZE));

			/* Extract Current buffer used */
			sr &=
		(CPH_AADMAC_CH1_AADMAC_SR_1_CH1_LO_SEL_MASK |
		CPH_AADMAC_CH1_AADMAC_SR_1_CH1_HI_SEL_MASK);
			sr >>=
		CPH_AADMAC_CH1_AADMAC_SR_1_CH1_LO_SEL_SHIFT;
			break;
		}

	}

	/* return the Current buffer being used */
	return (cUInt16) sr;
}


/****************************************************************************
*
*  Function Name: cUInt32 chal_caph_dma_read_timestamp(CHAL_HANDLE handle,
*			CAPH_DMA_CHANNEL_e channel)
*
*  Description: read CAPH DMA channel timestamp
*
****************************************************************************/
cUInt32 chal_caph_dma_read_timestamp(CHAL_HANDLE handle,
				     CAPH_DMA_CHANNEL_e channel)
{
	cUInt32 base = ((chal_caph_dma_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 ts = 0;

	/* Find the channel we are looking for */
	for (index = 0; index < CHAL_CAPH_DMA_MAX_TS_CHANNELS; index++) {
		if ((1UL << index) & channel) {
			/* Read the timestamp of selected channel */
			ts = BRCM_READ_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_TS,
					       (index *
						CHAL_CAPH_DMA_CH_TS_REG_SIZE));
			break;
		}

	}

	/* return the timestamp */
	return ts;
}

static cVoid chal_caph_dma_default_set_hibuffer(CHAL_HANDLE handle,
						CAPH_DMA_CHANNEL_e channel,
						cUInt32 address, cUInt32 size)
{
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_dma_en_hibuffer(CHAL_HANDLE handle,
*			CAPH_DMA_CHANNEL_e channel)
*
*  Description: enable CAPH DMA channel hi buffer
*
****************************************************************************/
cVoid chal_caph_dma_en_hibuffer(CHAL_HANDLE handle, CAPH_DMA_CHANNEL_e channel)
{
	cUInt32 base = ((chal_caph_dma_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 cr = 0;

	/* Find the channel we are looking for */
	for (index = 0; index < CHAL_CAPH_DMA_MAX_CHANNELS; index++) {
		if ((1UL << index) & channel) {
			/* enable the use of hi buffer */
			cr = BRCM_READ_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_CR_2,
					       (index *
						CHAL_CAPH_DMA_CH_REG_SIZE));

			/* Configure the use of buffer base address register */
			cr |=
			    (1 <<
		CPH_AADMAC_CH1_AADMAC_CR_2_CH1_AADMAC_HIGH_BASE_EN_SHIFT);

			/* Apply the settings in the hardware */
			BRCM_WRITE_REG_IDX(base, CPH_AADMAC_CH1_AADMAC_CR_2,
					   (index * CHAL_CAPH_DMA_CH_REG_SIZE),
					   cr);
			break;
		}

	}

	return;
}
