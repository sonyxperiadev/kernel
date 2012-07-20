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
*  @file   chal_caph_switch.c
*
*  @brief  chal layer driver for caph switch driver
*
****************************************************************************/

#include "chal_caph_switch.h"
#include <mach/rdb/brcm_rdb_cph_ssasw.h>
#include <mach/rdb/brcm_rdb_util.h>

/****************************************************************************
*			G L O B A L   S E C T I O N
****************************************************************************/

/****************************************************************************
 global variable definitions
****************************************************************************/

/****************************************************************************
*			L O C A L   S E C T I O N
****************************************************************************/

/****************************************************************************
 local macro declarations
****************************************************************************/
/* Max number of chnnels supported by Hardware */
#define CHAL_CAPH_SWITCH_MAX_CHANNELS      16

/* Max number of chnnels supported by Hardware */
#define CHAL_CAPH_SWITCH_MAX_DESTINATIONS      4

/* Total Registers Size of each SWITCH channel  (CR1 + CR2 + SR1) */
#define CHAL_CAPH_SWITCH_CH_REG_SIZE       \
((CPH_SSASW_CH02_SRC_OFFSET - CPH_SSASW_CH01_SRC_OFFSET)/sizeof(cUInt32))

#define CHAL_CAPH_SWITCH_DST_REG_SIZE       \
((CPH_SSASW_CH02_DST2_OFFSET - CPH_SSASW_CH02_DST1_OFFSET)/sizeof(cUInt32))

/****************************************************************************
 local typedef declarations
****************************************************************************/
struct _chal_caph_swwitch_cb_t {
	cUInt32 base;		/* Register Base address */
	cBool alloc_status[CHAL_CAPH_SWITCH_MAX_CHANNELS];
};
#define chal_caph_swwitch_cb_t struct _chal_caph_swwitch_cb_t

/****************************************************************************
* local variable definitions
***************************************************************************/
/* chal control block where all information is stored */
static chal_caph_swwitch_cb_t chal_caph_switchc_cb;

/****************************************************************************
 local function declarations
****************************************************************************/

/******************************************************************************
 local function definitions
******************************************************************************/

/****************************************************************************
*
*  Function Name: CHAL_HANDLE chal_caph_switch_init(cUInt32 baseAddress)
*
*  Description: Initialize CAPH ASW
*
****************************************************************************/
CHAL_HANDLE chal_caph_switch_init(cUInt32 baseAddress)
{
	cUInt8 ch;

	/* Go through all the channels and set them not allocated */
	for (ch = 0; ch < CHAL_CAPH_SWITCH_MAX_CHANNELS; ch++)
		chal_caph_switchc_cb.alloc_status[ch] = FALSE;

	/* Set the register base address to the caller supplied base address */
	chal_caph_switchc_cb.base = baseAddress;

	/* Need to set SSASW_NOC.SSASW_NOC_EN and SSASW_NOC.SSASW_CLKGATE_BYPASS
	 * bits during init. This may be a temporary solution.
	 */
	/*   chal_caph_switch_enable_clock_bypass(
		(CHAL_HANDLE)(&chal_caph_switchc_cb)); */
	chal_caph_switch_enable_clock((CHAL_HANDLE) (&chal_caph_switchc_cb));

	return (CHAL_HANDLE) (&chal_caph_switchc_cb);
}

/****************************************************************************
*
*  Function Name: Void chal_caph_switch_deinit(CHAL_HANDLE handle)
*
*  Description: Deinitialize CAPH ASW
*
****************************************************************************/
cVoid chal_caph_switch_deinit(CHAL_HANDLE handle)
{
	chal_caph_swwitch_cb_t *pchal_cb = (chal_caph_swwitch_cb_t *) handle;
	cUInt8 ch;

	/* Go through all the channels and set them not allocated */
	for (ch = 0; ch < CHAL_CAPH_SWITCH_MAX_CHANNELS; ch++)
		pchal_cb->alloc_status[ch] = FALSE;

	/* Need to unset SSASW_NOC.SSASW_NOC_EN and
	 SSASW_NOC.SSASW_CLKGATE_BYPASS bits during init.
	This may be a temporary solution.
	 */
/*    chal_caph_switch_disable_clock_bypass(handle); */
	chal_caph_switch_disable_clock(handle);

	/* Reset the register base address */
	pchal_cb->base = 0;

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_switch_select_src(CHAL_HANDLE handle,
*            CAPH_SWITCH_CHNL_e channel,
*            cUInt16 fifo_address)
*
*  Description: CAPH ASW src selection
*
****************************************************************************/
cVoid chal_caph_switch_select_src(CHAL_HANDLE handle,
				  CAPH_SWITCH_CHNL_e channel, cUInt16 address)
{
	cUInt32 base = ((chal_caph_swwitch_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 reg_val;

	/* Find the channel we are looking for */
	for (index = 0; index < CHAL_CAPH_SWITCH_MAX_CHANNELS; index++) {
		if ((1UL << index) & channel) {
			/* found the channel, enable the channel */
			reg_val =
			    BRCM_READ_REG_IDX(base, CPH_SSASW_CH01_SRC,
					      (index *
					       CHAL_CAPH_SWITCH_CH_REG_SIZE));
			reg_val &= ~CPH_SSASW_CH01_SRC_CH01_SRC_ADDR_MASK;
			reg_val |=
			    (address & CPH_SSASW_CH01_SRC_CH01_SRC_ADDR_MASK);
			BRCM_WRITE_REG_IDX(base, CPH_SSASW_CH01_SRC,
					   (index *
					    CHAL_CAPH_SWITCH_CH_REG_SIZE),
					   reg_val);

			break;
		}

	}

	return;
}

/****************************************************************************
*
*  Function Name: CAPH_DST_STATUS_e chal_caph_switch_add_dst(CHAL_HANDLE handle,
*            CAPH_SWITCH_CHNL_e channel,
*            cUInt16 fifo_address)
*
*  Description: CAPH ASW add dst
*
****************************************************************************/
CAPH_DST_STATUS_e chal_caph_switch_add_dst(CHAL_HANDLE handle,
					   CAPH_SWITCH_CHNL_e channel,
					   cUInt16 address)
{
	cUInt32 base = ((chal_caph_swwitch_cb_t *) handle)->base;
	cUInt8 ch_idx;
	cUInt16 reg_idx;
	cUInt8 dst;
	cUInt32 reg_val;

	address &= CPH_SSASW_CH01_DST1_CH01_DST1_ADDR_MASK;

	/* Check for if this destination address is already allocated else where
		on the same channel as well as on all other channels */
	for (ch_idx = 0; ch_idx < CHAL_CAPH_SWITCH_MAX_CHANNELS; ch_idx++) {
		for (dst = 0; dst < CHAL_CAPH_SWITCH_MAX_DESTINATIONS; dst++) {
			reg_idx =
			    (ch_idx * CHAL_CAPH_SWITCH_CH_REG_SIZE) +
			    (dst * CHAL_CAPH_SWITCH_DST_REG_SIZE);
			reg_val =
			    BRCM_READ_REG_IDX(base, CPH_SSASW_CH01_DST1,
					      reg_idx);
			if ((reg_val & CPH_SSASW_CH01_DST1_CH01_DST1_EN_MASK)
			    &&
			    ((reg_val & CPH_SSASW_CH01_DST1_CH01_DST1_ADDR_MASK)
			     == address)) {
				return CAPH_DST_USED;
			}
		}

	}

	/* Find the channel we are looking for */
	for (ch_idx = 0; ch_idx < CHAL_CAPH_SWITCH_MAX_CHANNELS; ch_idx++) {
		if ((1UL << ch_idx) & channel) {
			/* found the channel, enable the channel */
			for (dst = 0; dst < CHAL_CAPH_SWITCH_MAX_DESTINATIONS;
			     dst++) {
				reg_idx =
				    (ch_idx * CHAL_CAPH_SWITCH_CH_REG_SIZE) +
				    (dst * CHAL_CAPH_SWITCH_DST_REG_SIZE);
				reg_val =
				    BRCM_READ_REG_IDX(base, CPH_SSASW_CH01_DST1,
						      reg_idx);
				if (!
				    (reg_val &
				     CPH_SSASW_CH01_DST1_CH01_DST1_EN_MASK)) {
					reg_val |=
					CPH_SSASW_CH01_DST1_CH01_DST1_EN_MASK;
					reg_val &=
				~CPH_SSASW_CH01_DST1_CH01_DST1_ADDR_MASK;
					reg_val |= address;
					BRCM_WRITE_REG_IDX(base,
							   CPH_SSASW_CH01_DST1,
							   reg_idx, reg_val);
					return CAPH_DST_OK;
				}
			}

			return CAPH_DST_NONE;
		}

	}

	return CAPH_DST_NONE;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_switch_remove_dst(CHAL_HANDLE handle,
*            CAPH_SWITCH_CHNL_e channel,
*            cUInt16 fifo_address)
*
*  Description: CAPH ASW remove dst
*
****************************************************************************/
cVoid chal_caph_switch_remove_dst(CHAL_HANDLE handle,
				  CAPH_SWITCH_CHNL_e channel, cUInt16 address)
{
	cUInt32 base = ((chal_caph_swwitch_cb_t *) handle)->base;
	cUInt8 ch_idx;
	cUInt16 reg_idx;
	cUInt8 dst;
	cUInt32 reg_val;
	register cUInt32 loopCount;

	address &= CPH_SSASW_CH01_DST1_CH01_DST1_ADDR_MASK;

	/* Find the channel we are looking for */
	for (ch_idx = 0; ch_idx < CHAL_CAPH_SWITCH_MAX_CHANNELS; ch_idx++) {
		if ((1UL << ch_idx) & channel) {
			/* found the channel, Look for the destination */
			for (dst = 0; dst < CHAL_CAPH_SWITCH_MAX_DESTINATIONS;
			     dst++) {
				reg_idx =
				    (ch_idx * CHAL_CAPH_SWITCH_CH_REG_SIZE) +
				    (dst * CHAL_CAPH_SWITCH_DST_REG_SIZE);
				reg_val =
				    BRCM_READ_REG_IDX(base, CPH_SSASW_CH01_DST1,
						      reg_idx);
				if ((reg_val &
				     CPH_SSASW_CH01_DST1_CH01_DST1_ADDR_MASK) ==
				    address) {
					/* Clear destination information */
					reg_val &=
					~CPH_SSASW_CH01_DST1_CH01_DST1_EN_MASK;
					BRCM_WRITE_REG_IDX(base,
							   CPH_SSASW_CH01_DST1,
							   reg_idx, reg_val);

/* ASIC team:
	When there are more than one destination, first disable
	the destination without changing the destination address
	in CHxx_DST register(only change the destination enable
	to 0, dest_addr unchanged). Then read back this CHxx_DST
	register five times. This is to insert some delay to
	make sure the source had stopped sending transfer to the
	old address. */
					for (loopCount = 5; loopCount != 0;
					     loopCount--)
						reg_val =
						    BRCM_READ_REG_IDX(base,
							CPH_SSASW_CH01_DST1,
							reg_idx);
					reg_val &=
				~CPH_SSASW_CH01_DST1_CH01_DST1_ADDR_MASK;
					BRCM_WRITE_REG_IDX(base,
							   CPH_SSASW_CH01_DST1,
							   reg_idx, reg_val);
					break;
				}
			}

			break;
		}

	}

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_switch_enable(CHAL_HANDLE handle,
*            cUInt16 channel)
*
*  Description: CAPH ASW enable
*
****************************************************************************/
cVoid chal_caph_switch_enable(CHAL_HANDLE handle, cUInt16 channel)
{
	cUInt32 base = ((chal_caph_swwitch_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 reg_val;

	/* Find the channel we are looking for */
	for (index = 0; index < CHAL_CAPH_SWITCH_MAX_CHANNELS; index++) {
		if ((1UL << index) & channel) {
			/* found the channel, enable the channel */
			reg_val =
			    BRCM_READ_REG_IDX(base, CPH_SSASW_CH01_SRC,
					      (index *
					       CHAL_CAPH_SWITCH_CH_REG_SIZE));
			reg_val |= CPH_SSASW_CH01_SRC_CH01_SRC_EN_MASK;
			BRCM_WRITE_REG_IDX(base, CPH_SSASW_CH01_SRC,
					   (index *
					    CHAL_CAPH_SWITCH_CH_REG_SIZE),
					   reg_val);
		}

	}

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_switch_disable(CHAL_HANDLE handle,
*            cUInt16 channel)
*
*  Description: CAPH ASW disable
*
****************************************************************************/
cVoid chal_caph_switch_disable(CHAL_HANDLE handle, cUInt16 channel)
{
	cUInt32 base = ((chal_caph_swwitch_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 reg_val;

	/* Find the channel we are looking for */
	for (index = 0; index < CHAL_CAPH_SWITCH_MAX_CHANNELS; index++) {
		if ((1UL << index) & channel) {
			/* found the channel, disable the channel */
			reg_val =
			    BRCM_READ_REG_IDX(base, CPH_SSASW_CH01_SRC,
					      (index *
					       CHAL_CAPH_SWITCH_CH_REG_SIZE));
			reg_val &= ~CPH_SSASW_CH01_SRC_CH01_SRC_EN_MASK;
			BRCM_WRITE_REG_IDX(base, CPH_SSASW_CH01_SRC,
					   (index *
					    CHAL_CAPH_SWITCH_CH_REG_SIZE),
					   reg_val);
		}

	}

	return;
}

/****************************************************************************
*
*  Function Name: CAPH_SWITCH_CHNL_e chal_caph_switch_alloc_channel
*  (CHAL_HANDLE handle)
*
*  Description: CAPH ASW channel allocation
*
****************************************************************************/
CAPH_SWITCH_CHNL_e chal_caph_switch_alloc_channel(CHAL_HANDLE handle)
{
	chal_caph_swwitch_cb_t *pchal_cb = (chal_caph_swwitch_cb_t *) handle;
	cUInt32 ch = 0;

	if (ch == 0) {
		/* Look for a free (non-allocated) channel  */
		for (; ch < CHAL_CAPH_SWITCH_MAX_CHANNELS; ch++) {
			if (pchal_cb->alloc_status[ch] == FALSE) {
				/* Found one */
				break;
			}
		}
	}

	if (ch < CHAL_CAPH_SWITCH_MAX_CHANNELS) {
		/* Found a free channel */
		pchal_cb->alloc_status[ch] = TRUE;
	}

	/* Convert to CAPH_DMA_CHANNEL_e format and return */
	return (CAPH_SWITCH_CHNL_e) (1UL << ch);
}

/****************************************************************************
*
*  Function Name: CAPH_SWITCH_CHNL_e chal_caph_switch_alloc_given_channel
*  (CHAL_HANDLE handle, CAPH_SWITCH_CHNL_e channel)
*
*  Description: allocate a given CAPH SWITCH channel
*
****************************************************************************/
CAPH_SWITCH_CHNL_e chal_caph_switch_alloc_given_channel(CHAL_HANDLE handle,
							CAPH_SWITCH_CHNL_e
							channel)
{
	chal_caph_swwitch_cb_t *pchal_cb = (chal_caph_swwitch_cb_t *) handle;
	cUInt32 ch = 0;

	if (channel != CAPH_SWITCH_CH_VOID) {
		/* Look whether the given channel is allocated or not  */
		for (; ch < CHAL_CAPH_SWITCH_MAX_CHANNELS; ch++) {
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
		for (; ch < CHAL_CAPH_SWITCH_MAX_CHANNELS; ch++) {
			if (pchal_cb->alloc_status[ch] == FALSE) {
				/* Found one */
				break;
			}
		}
	}

	if (ch < CHAL_CAPH_SWITCH_MAX_CHANNELS) {
		/* Found a free channel */
		pchal_cb->alloc_status[ch] = TRUE;
	}

	/* Convert to CAPH_DMA_CHANNEL_e format and return */
	return (CAPH_SWITCH_CHNL_e) (1UL << ch);
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_switch_free_channel
*  (CHAL_HANDLE handle, CAPH_SWITCH_CHNL_e channel)
*
*  Description: CAPH ASW free channel
*
****************************************************************************/
cVoid chal_caph_switch_free_channel(CHAL_HANDLE handle,
				    CAPH_SWITCH_CHNL_e channel)
{
	chal_caph_swwitch_cb_t *pchal_cb = (chal_caph_swwitch_cb_t *) handle;
	cUInt8 index;
	cUInt8 dst;
	cUInt16 reg_idx;
	cUInt32 reg_val;
	register UInt32 loopCount;

	/* Find the channel we are looking for */
	for (index = 0; index < CHAL_CAPH_SWITCH_MAX_CHANNELS; index++) {
		if ((1UL << index) & channel) {
			/* found the channel we are looking for,
				make this channel free for next allocation */
			pchal_cb->alloc_status[index] = FALSE;

			/* Clear the SRC register */
			reg_idx = (index * CHAL_CAPH_SWITCH_CH_REG_SIZE);
			reg_val =
			    BRCM_READ_REG_IDX(pchal_cb->base,
					      CPH_SSASW_CH01_SRC, reg_idx);
			reg_val &= ~CPH_SSASW_CH01_SRC_CH01_SRC_EN_MASK;
			BRCM_WRITE_REG_IDX(pchal_cb->base, CPH_SSASW_CH01_SRC,
					   reg_idx, reg_val);

			/* Add delay: 20 cycles of CAPH 26MHz clock, */
			/* ASIC team (Chin-Sieh Less):
			Instead of doing a timing loops, just do 5 reads to
			any SSASW register. It should give 20 cycles delay. */
			for (loopCount = 5; loopCount != 0; loopCount--)
				reg_val =
				    BRCM_READ_REG_IDX(pchal_cb->base,
						      CPH_SSASW_CH01_SRC,
						      reg_idx);
			reg_val &= ~CPH_SSASW_CH01_SRC_CH01_TWO_WORDS_MASK;
			reg_val &= ~CPH_SSASW_CH01_SRC_CH01_TRIGGER_SEL_MASK;
			reg_val &= ~CPH_SSASW_CH01_SRC_CH01_SRC_ADDR_MASK;
			BRCM_WRITE_REG_IDX(pchal_cb->base, CPH_SSASW_CH01_SRC,
					   reg_idx, reg_val);

			/* Clear all the DST registers */
			for (dst = 0; dst < CHAL_CAPH_SWITCH_MAX_DESTINATIONS;
			     dst++) {
				reg_idx =
				    (index * CHAL_CAPH_SWITCH_CH_REG_SIZE) +
				    (dst * CHAL_CAPH_SWITCH_DST_REG_SIZE);
				reg_val =
				    BRCM_READ_REG_IDX(pchal_cb->base,
						      CPH_SSASW_CH01_DST1,
						      reg_idx);

				/* Clear destination information */
				reg_val &=
				    ~CPH_SSASW_CH01_DST1_CH01_DST1_EN_MASK;
				reg_val &=
				    ~CPH_SSASW_CH01_DST1_CH01_DST1_ADDR_MASK;
				BRCM_WRITE_REG_IDX(pchal_cb->base,
						   CPH_SSASW_CH01_DST1, reg_idx,
						   reg_val);
			}

			break;
		}

	}

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_switch_set_datafmt(CHAL_HANDLE handle,
*            CAPH_SWITCH_CHNL_e channel,
*            CAPH_DATA_FORMAT_e dataFormat)
*
*  Description: CAPH ASW set data format: mono/stereo/16b/24b
*
****************************************************************************/
cVoid chal_caph_switch_set_datafmt(CHAL_HANDLE handle,
				   CAPH_SWITCH_CHNL_e channel,
				   CAPH_DATA_FORMAT_e dataFormat)
{
	cUInt32 base = ((chal_caph_swwitch_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 reg_val;

	/* Find the channel we are looking for */
	for (index = 0; index < CHAL_CAPH_SWITCH_MAX_CHANNELS; index++) {
		if ((1UL << index) & channel) {
			/* found the channel we are looking for,
				disable the channel */
			reg_val =
			    BRCM_READ_REG_IDX(base, CPH_SSASW_CH01_SRC,
					      (index *
					       CHAL_CAPH_SWITCH_CH_REG_SIZE));

			/* Configure the trigger selection */
			reg_val &= ~CPH_SSASW_CH01_SRC_CH01_TWO_WORDS_MASK;
			if (dataFormat == CAPH_STEREO_24BIT) {
				reg_val |=
				    CPH_SSASW_CH01_SRC_CH01_TWO_WORDS_MASK;
			}

			/* Apply the settings to hardware */
			BRCM_WRITE_REG_IDX(base, CPH_SSASW_CH01_SRC,
					   (index *
					    CHAL_CAPH_SWITCH_CH_REG_SIZE),
					   reg_val);

			break;
		}

	}

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_switch_select_trigger(CHAL_HANDLE handle,
*            CAPH_SWITCH_CHNL_e channel,
*            CAPH_SWITCH_TRIGGER_e trigger)
*
*  Description: CAPH ASW data tx  trigger selection
*
****************************************************************************/
cVoid chal_caph_switch_select_trigger(CHAL_HANDLE handle,
				      CAPH_SWITCH_CHNL_e channel,
				      CAPH_SWITCH_TRIGGER_e trigger)
{
	cUInt32 base = ((chal_caph_swwitch_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 reg_val;

	/* Find the channel we are looking for */
	for (index = 0; index < CHAL_CAPH_SWITCH_MAX_CHANNELS; index++) {
		if ((1UL << index) & channel) {
			/* found the channel we are looking for,
				disable the channel */
			reg_val =
			    BRCM_READ_REG_IDX(base, CPH_SSASW_CH01_SRC,
					      (index *
					       CHAL_CAPH_SWITCH_CH_REG_SIZE));

			/* Configure the trigger selection */
			reg_val &= ~CPH_SSASW_CH01_SRC_CH01_TRIGGER_SEL_MASK;
			reg_val |=
			    (trigger <<
			     CPH_SSASW_CH01_SRC_CH01_TRIGGER_SEL_SHIFT);

			/* Apply the settings to hardware */
			BRCM_WRITE_REG_IDX(base, CPH_SSASW_CH01_SRC,
					   (index *
					    CHAL_CAPH_SWITCH_CH_REG_SIZE),
					   reg_val);

			break;
		}

	}

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_switch_enable_clock(CHAL_HANDLE handle)
*
*  Description: CAPH ASW enable clock for data tx
*
*  - set M0_RATIO=value
*  - wait
*  - set SSASW_NOC_EN=1
****************************************************************************/
cVoid chal_caph_switch_enable_clock(CHAL_HANDLE handle)
{
	cUInt32 base = ((chal_caph_swwitch_cb_t *) handle)->base;
	cUInt32 reg_val, loop;

	chal_caph_switch_set_clock0(handle, 0x6, 0x659);

	for (loop = 5; loop != 0; loop--)
		reg_val = BRCM_READ_REG(base, CPH_SSASW_SSASW_NOC);

	reg_val = BRCM_READ_REG(base, CPH_SSASW_SSASW_NOC);
	reg_val |= (CPH_SSASW_SSASW_NOC_SSASW_NOC_EN_MASK);
	BRCM_WRITE_REG(base, CPH_SSASW_SSASW_NOC, reg_val);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_switch_disable_clock(CHAL_HANDLE handle)
*
*  Description: CAPH ASW disable clock
*  - set M0_RATIO=0
*  - wait a 3 CK26MHz clocks (CK96KHZ_en should never go high again)
*  - set SSASW_NOC_EN=0
*  - could also set M0_RATIO value here
****************************************************************************/
cVoid chal_caph_switch_disable_clock(CHAL_HANDLE handle)
{
	cUInt32 base = ((chal_caph_swwitch_cb_t *) handle)->base;
	cUInt32 reg_val, loop;

	reg_val = BRCM_READ_REG(base, CPH_SSASW_SSASW_MN0_DIVIDER);
	reg_val &= ~CPH_SSASW_SSASW_MN0_DIVIDER_M0_RATIO_MASK;
	BRCM_WRITE_REG(base, CPH_SSASW_SSASW_MN0_DIVIDER, reg_val);

	for (loop = 5; loop != 0; loop--)
		reg_val = BRCM_READ_REG(base, CPH_SSASW_SSASW_NOC);

	reg_val = BRCM_READ_REG(base, CPH_SSASW_SSASW_NOC);
	reg_val &= ~(CPH_SSASW_SSASW_NOC_SSASW_NOC_EN_MASK);
	BRCM_WRITE_REG(base, CPH_SSASW_SSASW_NOC, reg_val);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_switch_enable_clock_bypass
*  (CHAL_HANDLE handle)
*
*  Description: CAPH ASW enable clock bypass for data tx
*
****************************************************************************/
cVoid chal_caph_switch_enable_clock_bypass(CHAL_HANDLE handle)
{
	cUInt32 base = ((chal_caph_swwitch_cb_t *) handle)->base;
	cUInt32 reg_val;

	reg_val = BRCM_READ_REG(base, CPH_SSASW_SSASW_NOC);
	reg_val |= (CPH_SSASW_SSASW_NOC_SSASW_CLKGATE_BYPASS_MASK);
	BRCM_WRITE_REG(base, CPH_SSASW_SSASW_NOC, reg_val);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_switch_disable_clock_bypass
*  (CHAL_HANDLE handle)
*
*  Description: CAPH ASW disable clock bypass
*
****************************************************************************/
cVoid chal_caph_switch_disable_clock_bypass(CHAL_HANDLE handle)
{
	cUInt32 base = ((chal_caph_swwitch_cb_t *) handle)->base;
	cUInt32 reg_val;

	reg_val = BRCM_READ_REG(base, CPH_SSASW_SSASW_NOC);
	reg_val &= ~(CPH_SSASW_SSASW_NOC_SSASW_CLKGATE_BYPASS_MASK);
	BRCM_WRITE_REG(base, CPH_SSASW_SSASW_NOC, reg_val);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_switch_set_clock0(CHAL_HANDLE handle,
*            cUInt8 m_ratio,
*            cUInt16 n_ratio)
*
*  Description: CAPH ASW set clock0
*
****************************************************************************/
cVoid chal_caph_switch_set_clock0(CHAL_HANDLE handle,
				  cUInt8 m_ratio, cUInt16 n_ratio)
{
	cUInt32 base = ((chal_caph_swwitch_cb_t *) handle)->base;
	cUInt32 reg_val;

	reg_val = BRCM_READ_REG(base, CPH_SSASW_SSASW_MN0_DIVIDER);

	/* Configure the trigger selection */
	reg_val &= ~CPH_SSASW_SSASW_MN0_DIVIDER_M0_RATIO_MASK;
	reg_val &= ~CPH_SSASW_SSASW_MN0_DIVIDER_N0_RATIO_MASK;

	reg_val |= (m_ratio << CPH_SSASW_SSASW_MN0_DIVIDER_M0_RATIO_SHIFT);
	reg_val |= (n_ratio << CPH_SSASW_SSASW_MN0_DIVIDER_N0_RATIO_SHIFT);

	/* Apply the settings to hardware */
	BRCM_WRITE_REG(base, CPH_SSASW_SSASW_MN0_DIVIDER, reg_val);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_switch_set_clock1(CHAL_HANDLE handle,
*
*  Description: CAPH ASW set clock1
*
****************************************************************************/
cVoid chal_caph_switch_set_clock1(CHAL_HANDLE handle,
				  cUInt8 m_ratio, cUInt16 n_ratio)
{
	cUInt32 base = ((chal_caph_swwitch_cb_t *) handle)->base;
	cUInt32 reg_val;

	reg_val = BRCM_READ_REG(base, CPH_SSASW_SSASW_MN1_DIVIDER);

	/* Configure the trigger selection */
	reg_val &= ~CPH_SSASW_SSASW_MN1_DIVIDER_M1_RATIO_MASK;
	reg_val &= ~CPH_SSASW_SSASW_MN1_DIVIDER_N1_RATIO_MASK;

	reg_val |= (m_ratio << CPH_SSASW_SSASW_MN1_DIVIDER_M1_RATIO_SHIFT);
	reg_val |= (n_ratio << CPH_SSASW_SSASW_MN1_DIVIDER_N1_RATIO_SHIFT);

	/* Apply the settings to hardware */
	BRCM_WRITE_REG(base, CPH_SSASW_SSASW_MN1_DIVIDER, reg_val);

	return;
}

/****************************************************************************
*
*  Function Name: cUInt32 chal_caph_switch_read_err_status(CHAL_HANDLE handle)
*
*  Description: CAPH ASW read data tx err status
*
****************************************************************************/
cUInt32 chal_caph_switch_read_err_status(CHAL_HANDLE handle)
{
	cUInt32 base = ((chal_caph_swwitch_cb_t *) handle)->base;
	cUInt32 reg_val;

	reg_val = BRCM_READ_REG(base, CPH_SSASW_SSASW_ERR);

	return reg_val;
}

/****************************************************************************
*
*  Function Name: cUInt8 chal_caph_switch_read_time(CHAL_HANDLE handle)
*
*  Description: CAPH ASW read time (PREADY_MAX_TIME)
*
****************************************************************************/
cUInt8 chal_caph_switch_read_time(CHAL_HANDLE handle)
{
	cUInt32 base = ((chal_caph_swwitch_cb_t *) handle)->base;
	cUInt32 reg_val;

	reg_val = BRCM_READ_REG(base, CPH_SSASW_PREADY_MAX_TIME);

	return reg_val;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_switch_write_time(CHAL_HANDLE handle,
*            cUInt8 time)
*
*  Description: CAPH ASW write time (PREADY_MAX_TIME)
*
****************************************************************************/
cVoid chal_caph_switch_write_time(CHAL_HANDLE handle, cUInt8 time)
{
	cUInt32 base = ((chal_caph_swwitch_cb_t *) handle)->base;
	cUInt32 reg_val;

	reg_val = BRCM_READ_REG(base, CPH_SSASW_PREADY_MAX_TIME);
	reg_val &= ~(CPH_SSASW_PREADY_MAX_TIME_PREADY_MAX_TIME_MASK);
	reg_val |= (time & CPH_SSASW_PREADY_MAX_TIME_PREADY_MAX_TIME_MASK);
	BRCM_WRITE_REG(base, CPH_SSASW_PREADY_MAX_TIME, reg_val);

	return;
}
