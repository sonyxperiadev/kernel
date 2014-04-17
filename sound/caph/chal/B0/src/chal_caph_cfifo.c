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
*  @file   chal_caph_cfifo.c
*
*  @brief  chal layer driver for caph cfifo driver
*
****************************************************************************/

#include "chal_caph_cfifo.h"
#include <mach/rdb/brcm_rdb_cph_cfifo.h>
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
/* Max number of FIFOs supported by Hardware */
#define CHAL_CAPH_CFIFO_MAX_FIFOS      16

/* Max number of Timestamp chnnels supported by Hardware */
#define CHAL_CAPH_CFIFO_MAX_TS_CHANNELS   4

/* PADDR register size for each channel */
#define CHAL_CAPH_CFIFO_PADDR_REG_SIZE	\
((CPH_CFIFO_CH2_PADDR_OFFSET-CPH_CFIFO_CH1_PADDR_OFFSET)/sizeof(cUInt32))

/* Control register size for each channel */
#define CHAL_CAPH_CFIFO_CR_REG_SIZE	\
((CPH_CFIFO_CPH_CR_2_OFFSET - CPH_CFIFO_CPH_CR_1_OFFSET)/sizeof(cUInt32))

/* CTL register size for each channel */
#define CHAL_CAPH_CFIFO_CTL_REG_SIZE	\
((CPH_CFIFO_CPH_CTL_2_OFFSET - CPH_CFIFO_CPH_CTL_1_OFFSET)/sizeof(cUInt32))

/* Time stamp register size for each channel */
#define CHAL_CAPH_CFIFO_TS_REG_SIZE	\
((CPH_CFIFO_CPH_CFIFO_TIMESTAMP_CH2_OFFSET -	\
CPH_CFIFO_CPH_CFIFO_TIMESTAMP_CH1_OFFSET)/sizeof(cUInt32))

/* Read CFIFO_CTL_X register contents */
#define CHAL_READ_CFIFO_CTL_IDX(b, i, val)	\
	do {	\
		if (i < 8) {	\
			val = BRCM_READ_REG_IDX(b, CPH_CFIFO_CPH_CTL_1,	\
			((i/2)*CHAL_CAPH_CFIFO_CTL_REG_SIZE));	\
		} else {	\
			val = BRCM_READ_REG_IDX(b, CPH_CFIFO_CPH_CTL_5,	\
			(((i-8)/2)*CHAL_CAPH_CFIFO_CTL_REG_SIZE));	\
		}	\
	} while (0)

/* Write CFIFO_CTL_X register */
#define CHAL_WRITE_CFIFO_CTL_IDX(b, i, val)	\
	do {	\
		if (i < 8) {	\
			BRCM_WRITE_REG_IDX(b, CPH_CFIFO_CPH_CTL_1,	\
			((i/2)*CHAL_CAPH_CFIFO_CTL_REG_SIZE), val);	\
		} else {	\
			BRCM_WRITE_REG_IDX(b, CPH_CFIFO_CPH_CTL_5,	\
			(((i-8)/2)*CHAL_CAPH_CFIFO_CTL_REG_SIZE), val);	\
		}	\
	} while (0)

/****************************************************************************
* local typedef declarations
*****************************************************************************/

struct _chal_caph_cfifo_cb_t {
	cUInt32 base;		/* Register Base address */
	cBool alloc_status[CHAL_CAPH_CFIFO_MAX_FIFOS];	/* alloc fifo status */
	cUInt16 addr[CHAL_CAPH_CFIFO_MAX_FIFOS];	/* Size of each FIFO */
	cUInt16 size[CHAL_CAPH_CFIFO_MAX_FIFOS];	/* Size of each FIFO */
};
#define chal_caph_cfifo_cb_t struct _chal_caph_cfifo_cb_t

/****************************************************************************
* local variable definitions
*****************************************************************************/
/* chal control block where all information is stored */
static chal_caph_cfifo_cb_t chal_caph_cfifo_cb;

/****************************************************************************
* local function declarations
*****************************************************************************/

/******************************************************************************
* local function definitions
*******************************************************************************/

/****************************************************************************
*
*  Function Name: CHAL_HANDLE chal_caph_cfifo_init(cUInt32 baseAddress)
*
*  Description: init CAPH CFIFO block
*
****************************************************************************/
CHAL_HANDLE chal_caph_cfifo_init(cUInt32 baseAddress)
{
	cUInt8 fifo;

	/* Go through all the FIFOs and set them not allocated */
	for (fifo = 0; fifo < CHAL_CAPH_CFIFO_MAX_FIFOS; fifo++)
		chal_caph_cfifo_cb.alloc_status[fifo] = FALSE;

	/* Set the register base address to the caller supplied base address */
	chal_caph_cfifo_cb.base = baseAddress;

	return (CHAL_HANDLE) (&chal_caph_cfifo_cb);
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_cfifo_deinit(CHAL_HANDLE handle)
*
*  Description: deinit CAPH CFIFO block
*
****************************************************************************/
cVoid chal_caph_cfifo_deinit(CHAL_HANDLE handle)
{
	chal_caph_cfifo_cb_t *pchal_cb = (chal_caph_cfifo_cb_t *) handle;
	cUInt8 fifo;

	/* Go through all the FIFOs and set them not allocated */
	for (fifo = 0; fifo < CHAL_CAPH_CFIFO_MAX_FIFOS; fifo++)
		pchal_cb->alloc_status[fifo] = FALSE;

	/* Reset the register base address */
	pchal_cb->base = 0;

	return;
}

/****************************************************************************
*
*  Function Name: cUInt16 chal_caph_cfifo_get_fifo_addr(CHAL_HANDLE handle,
*			CAPH_CFIFO_e fifo)
*
*  Description: get CAPH CFIFO address
*
****************************************************************************/
cUInt32 chal_caph_cfifo_get_fifo_addr(CHAL_HANDLE handle, CAPH_CFIFO_e fifo)
{
	cUInt32 base = ((chal_caph_cfifo_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 addr = 0;

	/* Find the FIFOs we are looking for */
	for (index = 0; index < CHAL_CAPH_CFIFO_MAX_FIFOS; index++) {
		if ((1UL << index) & fifo) {
			addr =
			    (base +
			     index * CHAL_CAPH_CFIFO_PADDR_REG_SIZE *
			     sizeof(cUInt32));
			break;
		}

	}

	return addr;
}

/****************************************************************************
*
*  Function Name: CAPH_CFIFO_CHANNEL_e chal_caph_cfifo_alloc_channel
* (CHAL_HANDLE handle)
*
*  Description: allocate CAPH CFIFO channel
*
****************************************************************************/
CAPH_CFIFO_e chal_caph_cfifo_alloc_channel(CHAL_HANDLE handle)
{
	chal_caph_cfifo_cb_t *pchal_cb = (chal_caph_cfifo_cb_t *) handle;
	cUInt32 fifo = 0;

	if (fifo == 0) {
		/* Look for a free (non-allocated) channel  */
		for (; fifo < CHAL_CAPH_CFIFO_MAX_FIFOS; fifo++) {
			if (pchal_cb->alloc_status[fifo] == FALSE) {
				/* Found one */
				break;
			}
		}
	}

	if (fifo < CHAL_CAPH_CFIFO_MAX_FIFOS) {
		/* Found a free channel */
		pchal_cb->alloc_status[fifo] = TRUE;
	}

	/* Convert to CAPH_CFIFO_CHANNEL_e format and return */
	return (CAPH_CFIFO_e) (1UL << fifo);
}

/****************************************************************************
*
*  Function Name: CAPH_CFIFO_CHANNEL_e chal_caph_cfifo_alloc_channel
* (CHAL_HANDLE handle, CAPH_CFIFO_e channel)
*
*  Description: allocate a given CAPH CFIFO channel
*
****************************************************************************/
CAPH_CFIFO_e chal_caph_cfifo_alloc_given_channel(CHAL_HANDLE handle,
						 CAPH_CFIFO_e channel)
{
	chal_caph_cfifo_cb_t *pchal_cb = (chal_caph_cfifo_cb_t *) handle;
	cUInt32 ch = 0;

	if (channel != CAPH_CFIFO_VOID) {
		/* Look whether the given channel is allocated or not  */
		for (; ch < CHAL_CAPH_CFIFO_MAX_FIFOS; ch++) {
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
		for (; ch < CHAL_CAPH_CFIFO_MAX_FIFOS; ch++) {
			if (pchal_cb->alloc_status[ch] == FALSE) {
				/* Found one */
				break;
			}
		}
	}

	if (ch < CHAL_CAPH_CFIFO_MAX_FIFOS) {
		/* Found a free channel */
		pchal_cb->alloc_status[ch] = TRUE;
	}

	/* Convert to CAPH_DMA_CHANNEL_e format and return */
	return (CAPH_CFIFO_e) (1UL << ch);
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_cfifo_free_channel(CHAL_HANDLE handle,
*			CAPH_CFIFO_CHANNEL_e channel)
*
*  Description: free CAPH CFIFO channel
*
****************************************************************************/
cVoid chal_caph_cfifo_free_channel(CHAL_HANDLE handle, CAPH_CFIFO_e fifo)
{
	chal_caph_cfifo_cb_t *pchal_cb = (chal_caph_cfifo_cb_t *) handle;
	cUInt8 index;

	/* Find the channel we are looking for */
	for (index = 0; index < CHAL_CAPH_CFIFO_MAX_FIFOS; index++) {
		if ((1UL << index) & fifo) {
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
*  Function Name: cVoid chal_caph_cfifo_enable(CHAL_HANDLE handle,
*			cUInt16 fifo)
*
*  Description: enable CAPH CFIFOs
*
****************************************************************************/
cVoid chal_caph_cfifo_enable(CHAL_HANDLE handle, cUInt16 fifo)
{
	cUInt32 base = ((chal_caph_cfifo_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 reg_val;

	/* Find the FIFOs we are looking for */
	for (index = 0; index < CHAL_CAPH_CFIFO_MAX_FIFOS; index++) {
		if ((1UL << index) & fifo) {
			/* found the FIFO we are looking for, enable the FIFO */
			reg_val =
			    BRCM_READ_REG_IDX(base, CPH_CFIFO_CPH_CR_1,
					      (index *
					       CHAL_CAPH_CFIFO_CR_REG_SIZE));
			reg_val |=
			    CPH_CFIFO_CPH_CR_1_CH1_CENTRAL_FIFO_ENABLE_MASK;
			BRCM_WRITE_REG_IDX(base, CPH_CFIFO_CPH_CR_1,
					   (index *
					    CHAL_CAPH_CFIFO_CR_REG_SIZE),
					   reg_val);
		}

	}

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_cfifo_disable(CHAL_HANDLE handle,
*			cUInt16 fifo)
*
*  Description: disable CAPH CFIFOs
*
****************************************************************************/
cVoid chal_caph_cfifo_disable(CHAL_HANDLE handle, cUInt16 fifo)
{
	cUInt32 base = ((chal_caph_cfifo_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 reg_val;

	/* Find the FIFOs we are looking for */
	for (index = 0; index < CHAL_CAPH_CFIFO_MAX_FIFOS; index++) {
		if ((1UL << index) & fifo) {
			/* found the FIFO, Disable the FIFO */
			reg_val =
			    BRCM_READ_REG_IDX(base, CPH_CFIFO_CPH_CR_1,
					      (index *
					       CHAL_CAPH_CFIFO_CR_REG_SIZE));
			reg_val &=
			    ~CPH_CFIFO_CPH_CR_1_CH1_CENTRAL_FIFO_ENABLE_MASK;
			BRCM_WRITE_REG_IDX(base, CPH_CFIFO_CPH_CR_1,
					   (index *
					    CHAL_CAPH_CFIFO_CR_REG_SIZE),
					   reg_val);
		}

	}

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_cfifo_set_direction(CHAL_HANDLE handle,
*			CAPH_CFIFO_e fifo,
*			CAPH_CFIFO_CHNL_DIRECTION_e direction)
*
*  Description: config CAPH CFIFO channel transfer direction
*
****************************************************************************/
cVoid chal_caph_cfifo_set_direction(CHAL_HANDLE handle,
				    CAPH_CFIFO_e fifo,
				    CAPH_CFIFO_CHNL_DIRECTION_e direction)
{
	cUInt32 base = ((chal_caph_cfifo_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 reg_val;

	/* Find the FIFOs we are looking for */
	for (index = 0; index < CHAL_CAPH_CFIFO_MAX_FIFOS; index++) {
		if ((1UL << index) & fifo) {
			reg_val =
			    BRCM_READ_REG_IDX(base, CPH_CFIFO_CPH_CR_1,
					      (index *
					       CHAL_CAPH_CFIFO_CR_REG_SIZE));

			/* configure direction */
			reg_val &= ~CPH_CFIFO_CPH_CR_1_CH1_IN_OUT_MASK;
			reg_val |=
			    (direction << CPH_CFIFO_CPH_CR_1_CH1_IN_OUT_SHIFT);

			/* Apply the settings to the register */
			BRCM_WRITE_REG_IDX(base, CPH_CFIFO_CPH_CR_1,
					   (index *
					    CHAL_CAPH_CFIFO_CR_REG_SIZE),
					   reg_val);
			break;
		}

	}

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_cfifo_set_address(CHAL_HANDLE handle,
*			CAPH_CFIFO_e fifo,
*			cUInt32 address)
*
*  Description: config CAPH CFIFO channel start address
*
****************************************************************************/
cVoid chal_caph_cfifo_set_address(CHAL_HANDLE handle,
				  CAPH_CFIFO_e fifo, cUInt32 address)
{
	cUInt32 base = ((chal_caph_cfifo_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 reg_val;

	/* Find the FIFOs we are looking for */
	for (index = 0; index < CHAL_CAPH_CFIFO_MAX_FIFOS; index++) {
		if ((1UL << index) & fifo) {
			/* Need to revisit this function as we will be */
			/* pre-configuring the FIFO size and address */
			/* How does the caller knows the address and size */
			/* found the FIFO we are looking for */
			CHAL_READ_CFIFO_CTL_IDX(base, index, reg_val);

			((chal_caph_cfifo_cb_t *) handle)->addr[index] =
			    address;

			address /= sizeof(cUInt32);

			if (index & 0x01) {
				/* configure start address */
				reg_val &=
				    ~CPH_CFIFO_CPH_CTL_1_CH2_START_ADDR_MASK;
				reg_val |=
				    ((address &
				      CPH_CFIFO_CPH_CTL_1_CH1_START_ADDR_MASK)
				     <<
				     CPH_CFIFO_CPH_CTL_1_CH2_START_ADDR_SHIFT);
			} else {
				/* configure start address */
				reg_val &=
				    ~CPH_CFIFO_CPH_CTL_1_CH1_START_ADDR_MASK;
				reg_val |=
				    ((address &
				      CPH_CFIFO_CPH_CTL_1_CH1_START_ADDR_MASK)
				     <<
				     CPH_CFIFO_CPH_CTL_1_CH1_START_ADDR_SHIFT);
			}

			/* Apply the settings to the register */
			CHAL_WRITE_CFIFO_CTL_IDX(base, index, reg_val);

			break;
		}

	}

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_cfifo_set_size(CHAL_HANDLE handle,
*			CAPH_CFIFO_e fifo,
*			cUInt16 size)
*
*  Description: config CAPH CFIFO channel
*
****************************************************************************/
cVoid chal_caph_cfifo_set_size(CHAL_HANDLE handle,
			       CAPH_CFIFO_e fifo, cUInt16 size)
{
	cUInt32 base = ((chal_caph_cfifo_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 reg_val;

	/* Convert ths size to 32bit value */
	size = size / sizeof(cUInt32);

	/* Find the FIFOs we are looking for */
	for (index = 0; index < CHAL_CAPH_CFIFO_MAX_FIFOS; index++) {
		if ((1UL << index) & fifo) {
			/* Need to revisit as we will be */
			/* pre-configuring the FIFO size and address */
			/* How caller to get address and size */

			/* found the FIFO we are looking for */
			CHAL_READ_CFIFO_CTL_IDX(base, index, reg_val);

			((chal_caph_cfifo_cb_t *) handle)->size[index] = size;

			/* Convert it in to register format */
			if (size >= 128)
				size = (size / 64) - 1;
			else
				size = 0;
			if (index & 0x01) {
				/* configure size */
				reg_val &=
				    ~CPH_CFIFO_CPH_CTL_1_CH2_RING_SIZE_MASK;
				reg_val |=
				    ((size & 0x1F) <<
				     CPH_CFIFO_CPH_CTL_1_CH2_RING_SIZE_SHIFT);
			} else {
				/* configure size */
				reg_val &=
				    ~CPH_CFIFO_CPH_CTL_1_CH1_RING_SIZE_MASK;
				reg_val |=
				    ((size & 0x1F) <<
				     CPH_CFIFO_CPH_CTL_1_CH1_RING_SIZE_SHIFT);
			}

			/* Apply the settings to the register */
			CHAL_WRITE_CFIFO_CTL_IDX(base, index, reg_val);

			break;
		}

	}

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_cfifo_queue_add_fifo(CHAL_HANDLE handle,
*			cUInt16 fifo,
*			CAPH_CFIFO_QUEUE_e queue)
*
*  Description: add fifos to queue
*
****************************************************************************/
cVoid chal_caph_cfifo_queue_add_fifo(CHAL_HANDLE handle,
				     cUInt16 fifo, CAPH_CFIFO_QUEUE_e queue)
{
	cUInt32 base = ((chal_caph_cfifo_cb_t *) handle)->base;
	cUInt32 arb;

	switch (queue) {

	case CAPH_CFIFO_QUEUE2:

		/* put it in Q2 */
		arb = BRCM_READ_REG(base, CPH_CFIFO_CPH_ARB_CTL_1);
		arb |= (fifo << CPH_CFIFO_CPH_ARB_CTL_1_Q2_MASK_SHIFT);
		BRCM_WRITE_REG(base, CPH_CFIFO_CPH_ARB_CTL_1, arb);

		break;

	case CAPH_CFIFO_QUEUE3:

		/* put it in Q3 */
		arb = BRCM_READ_REG(base, CPH_CFIFO_CPH_ARB_CTL_2);
		arb |= (fifo << CPH_CFIFO_CPH_ARB_CTL_2_Q3_MASK_SHIFT);
		BRCM_WRITE_REG(base, CPH_CFIFO_CPH_ARB_CTL_2, arb);

		break;

	default:

		/* put it in Q1 (default) */
		arb = BRCM_READ_REG(base, CPH_CFIFO_CPH_ARB_CTL_1);
		arb |= (fifo << CPH_CFIFO_CPH_ARB_CTL_1_Q1_MASK_SHIFT);
		BRCM_WRITE_REG(base, CPH_CFIFO_CPH_ARB_CTL_1, arb);

		break;

	}

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_cfifo_queue_remove_fifo(CHAL_HANDLE handle,
*			cUInt16 fifo,
*			CAPH_CFIFO_QUEUE_e queue)
*
*  Description: remove fifos from queue
*
****************************************************************************/
cVoid chal_caph_cfifo_queue_remove_fifo(CHAL_HANDLE handle,
					cUInt16 fifo, CAPH_CFIFO_QUEUE_e queue)
{
	cUInt32 base = ((chal_caph_cfifo_cb_t *) handle)->base;
	cUInt32 arb;
	cUInt32 rem_arb;

	switch (queue) {

	case CAPH_CFIFO_QUEUE2:

		/* take it from Q2 */
		arb = BRCM_READ_REG(base, CPH_CFIFO_CPH_ARB_CTL_1);

		rem_arb = (fifo << CPH_CFIFO_CPH_ARB_CTL_1_Q2_MASK_SHIFT);
		arb &= ~rem_arb;
		BRCM_WRITE_REG(base, CPH_CFIFO_CPH_ARB_CTL_1, arb);

		break;

	case CAPH_CFIFO_QUEUE3:

		/* take it from Q3 */
		arb = BRCM_READ_REG(base, CPH_CFIFO_CPH_ARB_CTL_2);

		rem_arb = (fifo << CPH_CFIFO_CPH_ARB_CTL_2_Q3_MASK_SHIFT);
		arb &= ~rem_arb;
		BRCM_WRITE_REG(base, CPH_CFIFO_CPH_ARB_CTL_2, arb);

		break;

	default:

		/* take it from Q1 (default) */
		arb = BRCM_READ_REG(base, CPH_CFIFO_CPH_ARB_CTL_1);
		rem_arb = (fifo << CPH_CFIFO_CPH_ARB_CTL_1_Q1_MASK_SHIFT);
		arb &= ~rem_arb;
		BRCM_WRITE_REG(base, CPH_CFIFO_CPH_ARB_CTL_1, arb);

		break;

	}

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_cfifo_set_panic_timer(CHAL_HANDLE handle,
*			cUInt8 timeout)
*
*  Description: set CFIFO channel panic timeout value
*
****************************************************************************/
cVoid chal_caph_cfifo_set_panic_timer(CHAL_HANDLE handle, cUInt8 timeout)
{
	cUInt32 base = ((chal_caph_cfifo_cb_t *) handle)->base;
	cUInt32 reg_val;

	/* Read the panic timer value */
	reg_val = BRCM_READ_REG(base, CPH_CFIFO_CPH_ARB_CTL_2);

	/* Clear the current  timeout value */
	reg_val &= ~CPH_CFIFO_CPH_ARB_CTL_2_CH_TIMER_DEFAULTS_MASK;
	/* Add the new  timeout value */
	reg_val |= (timeout << CPH_CFIFO_CPH_ARB_CTL_2_CH_TIMER_DEFAULTS_SHIFT);

	/* Program the panic timer value */
	BRCM_WRITE_REG(base, CPH_CFIFO_CPH_ARB_CTL_2, reg_val);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_cfifo_set_arb(CHAL_HANDLE handle,
*			cUInt32 key)
*
*  Description: set arb pattern
*
****************************************************************************/
cVoid chal_caph_cfifo_set_arb(CHAL_HANDLE handle, cUInt32 key)
{
	cUInt32 base = ((chal_caph_cfifo_cb_t *) handle)->base;
	cUInt32 arb;

	/* Get the current arbitration pattern */
	arb = BRCM_READ_REG(base, CPH_CFIFO_CPH_ARB_CTL_3);

	/* clear and set the new pattern */
	arb &= ~CPH_CFIFO_CPH_ARB_CTL_3_ARB_PATTERN_MASK;
	arb |=
	    ((key & CPH_CFIFO_CPH_ARB_CTL_3_ARB_PATTERN_MASK) <<
	     CPH_CFIFO_CPH_ARB_CTL_3_ARB_PATTERN_SHIFT);

	/* Apply the changes to the Hardware */
	BRCM_WRITE_REG(base, CPH_CFIFO_CPH_ARB_CTL_3, arb);

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_cfifo_set_fifo_thres(CHAL_HANDLE handle,
*			CAPH_CFIFO_e fifo,
*			cUInt16 thres,
*			cUInt16 thres2)
*
*  Description: set fifo thresholds
*
****************************************************************************/
cVoid chal_caph_cfifo_set_fifo_thres(CHAL_HANDLE handle,
				     CAPH_CFIFO_e fifo,
				     cUInt16 thres, cUInt16 thres2)
{
	cUInt32 base = ((chal_caph_cfifo_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 cr;

	/* Convert them to 32bit sizes */
	thres = thres / sizeof(cUInt32);
	thres2 = thres2 / sizeof(cUInt32);

	/* Find the FIFOs we are looking for */
	for (index = 0; index < CHAL_CAPH_CFIFO_MAX_FIFOS; index++) {
		if ((1UL << index) & fifo) {
			/* found the FIFO, Disable the FIFO */
			cr = BRCM_READ_REG_IDX(base, CPH_CFIFO_CPH_CR_1,
					       (index *
						CHAL_CAPH_CFIFO_CR_REG_SIZE));

			/* Clear and configure the threshold */
			cr &= ~CPH_CFIFO_CPH_CR_1_CH1_THRES_MASK;
			thres &=
			    (CPH_CFIFO_CPH_CR_1_CH1_THRES_MASK >>
			     CPH_CFIFO_CPH_CR_1_CH1_THRES_SHIFT);
			cr |= (thres << CPH_CFIFO_CPH_CR_1_CH1_THRES_SHIFT);

			/* Clear and configure the threshold2 */
			cr &= ~CPH_CFIFO_CPH_CR_1_CH1_THRES2_MASK;
			thres2 &=
			    (CPH_CFIFO_CPH_CR_1_CH1_THRES2_MASK >>
			     CPH_CFIFO_CPH_CR_1_CH1_THRES2_SHIFT);
			cr |= (thres2 << CPH_CFIFO_CPH_CR_1_CH1_THRES2_SHIFT);

			/* Apply the changes to hardware */
			BRCM_WRITE_REG_IDX(base, CPH_CFIFO_CPH_CR_1,
					   (index *
					    CHAL_CAPH_CFIFO_CR_REG_SIZE), cr);

			break;
		}

	}

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_cfifo_clr_fifo(CHAL_HANDLE handle,
*			cUInt16 fifo)
*
*  Description: clear fifo
*
****************************************************************************/
cVoid chal_caph_cfifo_clr_fifo(CHAL_HANDLE handle, cUInt16 fifo)
{
	cUInt32 base = ((chal_caph_cfifo_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 cr;

	/* Find the FIFOs we are looking for */
	for (index = 0; index < CHAL_CAPH_CFIFO_MAX_FIFOS; index++) {
		if ((1UL << index) & fifo) {
			/* found the FIFO, Disable the FIFO */
			cr = BRCM_READ_REG_IDX(base, CPH_CFIFO_CPH_CR_1,
					       (index *
						CHAL_CAPH_CFIFO_CR_REG_SIZE));

			/* Send Reset Pulse to the Hardware.*/
			/* First make sure it is 0, set to 1, then clear to 0 */
			/* Clear Reset */
			cr &= ~CPH_CFIFO_CPH_CR_1_CH1_CLR_FIFO_MASK;
			BRCM_WRITE_REG_IDX(base, CPH_CFIFO_CPH_CR_1,
					   (index *
					    CHAL_CAPH_CFIFO_CR_REG_SIZE), cr);

			/* Start Reset  process on Hardware */
			cr = BRCM_READ_REG_IDX(base, CPH_CFIFO_CPH_CR_1,
					       (index *
						CHAL_CAPH_CFIFO_CR_REG_SIZE));
			cr |= CPH_CFIFO_CPH_CR_1_CH1_CLR_FIFO_MASK;
			BRCM_WRITE_REG_IDX(base, CPH_CFIFO_CPH_CR_1,
					   (index *
					    CHAL_CAPH_CFIFO_CR_REG_SIZE), cr);

			/* Clear Reset */
			cr = BRCM_READ_REG_IDX(base, CPH_CFIFO_CPH_CR_1,
					       (index *
						CHAL_CAPH_CFIFO_CR_REG_SIZE));
			cr &= ~CPH_CFIFO_CPH_CR_1_CH1_CLR_FIFO_MASK;
			BRCM_WRITE_REG_IDX(base, CPH_CFIFO_CPH_CR_1,
					   (index *
					    CHAL_CAPH_CFIFO_CR_REG_SIZE), cr);
		}

	}

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_cfifo_int_enable(CHAL_HANDLE handle,
*			cUInt16 fifo)
*
*  Description: enable the fifo int
*
****************************************************************************/
cVoid chal_caph_cfifo_int_enable(CHAL_HANDLE handle, cUInt16 fifo)
{
	cUInt32 base = ((chal_caph_cfifo_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 cr;

	/* Find the FIFOs we are looking for */
	for (index = 0; index < CHAL_CAPH_CFIFO_MAX_FIFOS; index++) {
		if ((1UL << index) & fifo) {
			/* found the FIFO, Disable the FIFO */
			cr = BRCM_READ_REG_IDX(base, CPH_CFIFO_CPH_CR_1,
					       (index *
						CHAL_CAPH_CFIFO_CR_REG_SIZE));

			/* Enable interrupt */
			cr |= CPH_CFIFO_CPH_CR_1_CH1_INTR_EN_MASK;

			/* Apply the setting to Hardware */
			BRCM_WRITE_REG_IDX(base, CPH_CFIFO_CPH_CR_1,
					   (index *
					    CHAL_CAPH_CFIFO_CR_REG_SIZE), cr);
		}

	}

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_cfifo_int_err_enable(CHAL_HANDLE handle,
*			cUInt16 fifo)
*
*  Description: enable the fifo err int
*
****************************************************************************/
cVoid chal_caph_cfifo_int_err_enable(CHAL_HANDLE handle, cUInt16 fifo)
{
	cUInt32 base = ((chal_caph_cfifo_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 cr;

	/* Find the FIFOs we are looking for */
	for (index = 0; index < CHAL_CAPH_CFIFO_MAX_FIFOS; index++) {
		if ((1UL << index) & fifo) {
			/* found the FIFO, Disable the FIFO */
			cr = BRCM_READ_REG_IDX(base, CPH_CFIFO_CPH_CR_1,
					       (index *
						CHAL_CAPH_CFIFO_CR_REG_SIZE));

			/* Enable interrupt */
			cr |= CPH_CFIFO_CPH_CR_1_CH1_ERR_INT_ENABLE_MASK;

			/* Apply the setting to Hardware */
			BRCM_WRITE_REG_IDX(base, CPH_CFIFO_CPH_CR_1,
					   (index *
					    CHAL_CAPH_CFIFO_CR_REG_SIZE), cr);
		}

	}

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_cfifo_int_disable(CHAL_HANDLE handle,
*			cUInt16 fifo)
*
*  Description: enable the fifo int
*
****************************************************************************/
cVoid chal_caph_cfifo_int_disable(CHAL_HANDLE handle, cUInt16 fifo)
{
	cUInt32 base = ((chal_caph_cfifo_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 cr;

	/* Find the FIFOs we are looking for */
	for (index = 0; index < CHAL_CAPH_CFIFO_MAX_FIFOS; index++) {
		if ((1UL << index) & fifo) {
			/* found the FIFO, Disable the FIFO */
			cr = BRCM_READ_REG_IDX(base, CPH_CFIFO_CPH_CR_1,
					       (index *
						CHAL_CAPH_CFIFO_CR_REG_SIZE));

			/* Enable interrupt */
			cr &= ~CPH_CFIFO_CPH_CR_1_CH1_INTR_EN_MASK;

			/* Apply the setting to Hardware */
			BRCM_WRITE_REG_IDX(base, CPH_CFIFO_CPH_CR_1,
					   (index *
					    CHAL_CAPH_CFIFO_CR_REG_SIZE), cr);
		}

	}

	return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_cfifo_int_err_disable(CHAL_HANDLE handle,
*			cUInt16 fifo)
*
*  Description: enable the fifo err int
*
****************************************************************************/
cVoid chal_caph_cfifo_int_err_disable(CHAL_HANDLE handle, cUInt16 fifo)
{
	cUInt32 base = ((chal_caph_cfifo_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 cr;

	/* Find the FIFOs we are looking for */
	for (index = 0; index < CHAL_CAPH_CFIFO_MAX_FIFOS; index++) {
		if ((1UL << index) & fifo) {
			/* found the FIFO, Disable the FIFO */
			cr = BRCM_READ_REG_IDX(base, CPH_CFIFO_CPH_CR_1,
					       (index *
						CHAL_CAPH_CFIFO_CR_REG_SIZE));

			/* Enable interrupt */
			cr &= ~CPH_CFIFO_CPH_CR_1_CH1_ERR_INT_ENABLE_MASK;

			/* Apply the setting to Hardware */
			BRCM_WRITE_REG_IDX(base, CPH_CFIFO_CPH_CR_1,
					   (index *
					    CHAL_CAPH_CFIFO_CR_REG_SIZE), cr);
		}

	}

	return;
}

/****************************************************************************
*
*  Function Name: cUInt32 chal_caph_cfifo_read_fifo_status(CHAL_HANDLE handle,
*			CAPH_CFIFO_e fifo)
*
*  Description: read fifo status
*
****************************************************************************/
cUInt32 chal_caph_cfifo_read_fifo_status(CHAL_HANDLE handle, CAPH_CFIFO_e fifo)
{
	cUInt32 base = ((chal_caph_cfifo_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 status = 0;

	/* Find the FIFOs we are looking for */
	for (index = 0; index < CHAL_CAPH_CFIFO_MAX_FIFOS; index++) {
		if ((1UL << index) & fifo) {
			/* found the FIFO, Disable the FIFO */
			status =
			    BRCM_READ_REG_IDX(base, CPH_CFIFO_CPH_SR_1,
					      (index *
					       CHAL_CAPH_CFIFO_CR_REG_SIZE));
/*            status >>= CPH_CFIFO_CPH_SR_1_CH1_EMPTY_ENTRY_SHIFT; */
			break;
		}

	}

	return status;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_cfifo_write_fifo(CHAL_HANDLE handle,
*			CAPH_CFIFO_e fifo,
*			cUInt32* data,
*			cUInt16 size,
*                   cBool   forceovf)
*
*  Description: write fifo
*
****************************************************************************/
cUInt16 chal_caph_cfifo_write_fifo(CHAL_HANDLE handle,
				   CAPH_CFIFO_e fifo,
				   cUInt32 *data, cUInt16 size, cBool forceovf)
{
	cUInt32 base = ((chal_caph_cfifo_cb_t *) handle)->base;
	cUInt8 findex;
	cUInt16 dindex;
	cUInt32 fsize = 0;
	cUInt32 cr = 0;

	/* Convert the size into 32bit words */
	size = size / sizeof(cUInt32);

	/* Find the FIFOs we are looking for */
	for (findex = 0; findex < CHAL_CAPH_CFIFO_MAX_FIFOS; findex++) {
		if ((1UL << findex) & fifo) {
			/* Check whether the FIFO is writable or not */
			/* found the FIFO, Disable the FIFO */
			cr = BRCM_READ_REG_IDX(base, CPH_CFIFO_CPH_CR_1,
					       (findex *
						CHAL_CAPH_CFIFO_CR_REG_SIZE));

			if (cr & CPH_CFIFO_CPH_CR_1_CH1_IN_OUT_MASK) {
				fsize =
				    BRCM_READ_REG_IDX(base, CPH_CFIFO_CPH_SR_1,
					(findex *
					CHAL_CAPH_CFIFO_CR_REG_SIZE));
				fsize &=
				    CPH_CFIFO_CPH_SR_1_CH1_EMPTY_ENTRY_MASK;
				fsize >>=
				    CPH_CFIFO_CPH_SR_1_CH1_EMPTY_ENTRY_SHIFT;

				if (forceovf == FALSE) {
					if (size > fsize)
						size = fsize;
				}

				dindex = 0;
				while (dindex < size) {
					BRCM_WRITE_REG_IDX(base,
						CPH_CFIFO_CH1_PADDR,
						(findex *
						CHAL_CAPH_CFIFO_PADDR_REG_SIZE),
						data[dindex++]);
				}
			}
			break;
		}

	}

	return size * sizeof(cUInt32);
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_cfifo_read_fifo(CHAL_HANDLE handle,
*			CAPH_CFIFO_e fifo,
*			cUInt32* data,
*			cUInt16 size,
*                   cBool   forceudf)
*
*  Description: read fifo
*
****************************************************************************/
cUInt16 chal_caph_cfifo_read_fifo(CHAL_HANDLE handle,
				  CAPH_CFIFO_e fifo,
				  cUInt32 *data, cUInt16 size, cBool forceudf)
{
	cUInt32 base = ((chal_caph_cfifo_cb_t *) handle)->base;
	cUInt8 findex;
	cUInt16 dindex;
	cUInt32 fsize = 0;
	cUInt32 cr = 0;

	/* Convert the size into 32bit words */
	size = size / sizeof(cUInt32);

	/* Find the FIFOs we are looking for */
	for (findex = 0; findex < CHAL_CAPH_CFIFO_MAX_FIFOS; findex++) {
		if ((1UL << findex) & fifo) {
			/* Check whether the FIFO is writable or not */
			/* found the FIFO, Disable the FIFO */
			cr = BRCM_READ_REG_IDX(base, CPH_CFIFO_CPH_CR_1,
					       (findex *
						CHAL_CAPH_CFIFO_CR_REG_SIZE));

			if (!(cr & CPH_CFIFO_CPH_CR_1_CH1_IN_OUT_MASK)) {
				fsize =
				    BRCM_READ_REG_IDX(base, CPH_CFIFO_CPH_SR_1,
						      (findex *
					CHAL_CAPH_CFIFO_CR_REG_SIZE));
				fsize &=
				    CPH_CFIFO_CPH_SR_1_CH1_EMPTY_ENTRY_MASK;
				fsize >>=
				    CPH_CFIFO_CPH_SR_1_CH1_EMPTY_ENTRY_SHIFT;

				fsize =
				    ((chal_caph_cfifo_cb_t *) handle)->
				    size[findex] - fsize;
				if (forceudf == FALSE) {
					if (size > fsize)
						size = fsize;
				}

				dindex = 0;
				while (dindex < size) {
					data[dindex++] =
						BRCM_READ_REG_IDX(base,
						CPH_CFIFO_CH1_PADDR,
						(findex *
					CHAL_CAPH_CFIFO_PADDR_REG_SIZE));
				}
			}
			break;
		}

	}

	return size * sizeof(cUInt32);
}

/****************************************************************************
*
*  Function Name: cUInt32 chal_caph_cfifo_read_timestamp(CHAL_HANDLE handle,
			CAPH_CFIFO_e fifo)
*
*  Description: read TS for channel 1 to 4
*
****************************************************************************/
cUInt32 chal_caph_cfifo_read_timestamp(CHAL_HANDLE handle, CAPH_CFIFO_e fifo)
{
	cUInt32 base = ((chal_caph_cfifo_cb_t *) handle)->base;
	cUInt8 index;
	cUInt32 ts = 0;

	/* Find the FIFOs we are looking for */
	for (index = 0; index < CHAL_CAPH_CFIFO_MAX_TS_CHANNELS; index++) {
		if ((1UL << index) & fifo) {
			/* Check whether the FIFO is writable or not */
			/* found the FIFO, Disable the FIFO */
			ts = BRCM_READ_REG_IDX(base,
			CPH_CFIFO_CPH_CFIFO_TIMESTAMP_CH1,
			(index * CHAL_CAPH_CFIFO_TS_REG_SIZE));
			break;
		}

	}

	return ts;
}
