/*****************************************************************************
* Copyright 2001 - 2009 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

/****************************************************************************
* inlcude header file declarations
*/
#include <linux/kernel.h>

/* #include "chal_os_types.h" */
#include "chal_sspi_hawaii.h"


/*
 * ===========================================================================
 *  extern variable declarations
 *
 */

/*
 * ===========================================================================
 *  local macro declarations
 */
#define MAX_SSPI_FULL_FRAMES      8
#define MAX_SSPI_FULL_PATTERNS    4
#define MAX_SSPI_FULL_SEQUENCES   32
#define MAX_SSPI_FULL_TASKS       8
#define MAX_SSPI_FULL_CHANNELS    2
#define MAX_SSPI_FULL_FIFOS       4
#define MAX_SSPI_FULL_FIFO_SIZE   2048

#define MAX_SSPI_LITE_FRAMES      4
#define MAX_SSPI_LITE_PATTERNS    4
#define MAX_SSPI_LITE_SEQUENCES   8
#define MAX_SSPI_LITE_TASKS       2
#define MAX_SSPI_LITE_CHANNELS    1
#define MAX_SSPI_LITE_FIFOS       4
#define MAX_SSPI_LITE_FIFO_SIZE   128

#define MAX_CHAL_SSPI_BLOCKS    6

#define SSPI_SCLK_CYCLE_MINUS1    0xF
#define SSPI_MASTER_CLK_CYCLE_PER_BIT    0x2

/* addresses occupied by each FRAME, SEQ, TASK, FIFO */
#define SSPI_FRAME_SIZE            0x10
#define SSPI_SEQ_SIZE              0x4
#define SSPI_TASK_SIZE             0x8
#define SSPI_FIFO_ENTRYTX_SIZE     0x80
#define SSPI_FIFO_ENTRYRX_SIZE     0x80
#define SSPI_FIFOTX_CTL_SIZE       0x10
#define SSPI_FIFORX_CTL_SIZE       0x10
#define SSPI_FIFOTX_PIO_THOLD_SIZE  0x4
#define SSPI_FIFORX_PIO_THOLD_SIZE  0x4
#define SSPI_FIFOTX_THOLD_SIZE     0x10
#define SSPI_FIFORX_THOLD_SIZE     0x10
#define SSPI_PAT_SIZE              0x10
#define SSPI_PAT_CTL_SIZE          0x4

/*
 * ===========================================================================
 *  local type definitions
 */
/**
* SSPI CAPH clock trigger type
******************************************************************************/
enum CHAL_SSPI_CAPH_CLK_TRIG_t {
	SSPI_CAPH_CLK_TRIG_INVALID,
	SSPI_CAPH_CLK_TRIG_4kHz,
	SSPI_CAPH_CLK_TRIG_8kHz,
	SSPI_CAPH_CLK_TRIG_12kHz,
	SSPI_CAPH_CLK_TRIG_16kHz,
	SSPI_CAPH_CLK_TRIG_24kHz,
	SSPI_CAPH_CLK_TRIG_32kHz,
	SSPI_CAPH_CLK_TRIG_48kHz,
	SSPI_CAPH_CLK_TRIG_96kHz,
	SSPI_CAPH_CLK_TRIG_11DOT025kHz,
	SSPI_CAPH_CLK_TRIG_22DOT05kHz,
	SSPI_CAPH_CLK_TRIG_44DOT1kHz,
	SSPI_CAPH_CLK_TRIG_MAX,
};
#define CHAL_SSPI_CAPH_CLK_TRIG_t enum CHAL_SSPI_CAPH_CLK_TRIG_t

/* general purpose */
#define FLD_CLEAR(val, mask) ((val) &= ~(mask))
#define FLD_SET(reg_val, fld_val, shift, mask)			\
	do {							\
		(reg_val) &= ~(mask);				\
		(reg_val) |= (((fld_val) << (shift)) & (mask));	\
	} while (0)
#define FLD_GET(reg_val, shift, mask) (((reg_val) & (mask)) >> (shift))

#define REG_SEQ_ADDR(idx, reg)  ((reg) + (idx) * SSPI_SEQ_SIZE)
#define REG_TASK_ADDR(idx, reg) ((reg) + (idx) * SSPI_TASK_SIZE)
#define REG_FRAME_ADDR(idx, reg) ((reg) + (idx) * SSPI_FRAME_SIZE)
#define REG_FIFO_ENTRYTX_ADDR(idx, reg) ((reg) + (idx) * SSPI_FIFO_ENTRYTX_SIZE)
#define REG_FIFO_ENTRYRX_ADDR(idx, reg) ((reg) + (idx) * SSPI_FIFO_ENTRYRX_SIZE)
#define REG_FIFOTX_CTL_ADDR(idx, reg) ((reg) + (idx) * SSPI_FIFOTX_CTL_SIZE)
#define REG_FIFORX_CTL_ADDR(idx, reg) ((reg) + (idx) * SSPI_FIFORX_CTL_SIZE)
#define REG_FIFOTX_THOLD_ADDR(idx, reg) ((reg) + (idx) * SSPI_FIFOTX_THOLD_SIZE)
#define REG_FIFORX_THOLD_ADDR(idx, reg) ((reg) + (idx) * SSPI_FIFORX_THOLD_SIZE)
#define REG_FIFOTX_PIO_THOLD_ADDR(idx, reg)	\
		((reg) + (idx) * SSPI_FIFOTX_PIO_THOLD_SIZE)
#define REG_FIFORX_PIO_THOLD_ADDR(idx, reg)	\
		((reg) + (idx) * SSPI_FIFORX_PIO_THOLD_SIZE)
#define REG_PAT_ADDR(idx, reg) ((reg) + (idx) * SSPI_PAT_SIZE)
#define REG_PAT_CTL_ADDR(idx, reg) ((reg) + (idx) * SSPI_PAT_CTL_SIZE)

#define SSPI_VALUE_IN_RANGE(v, m) ((v < m) ? v : 0)

/**
*
* Debug level
*
*****************************************************************************/
#define CDBG_ZERO       0
#define CDBG_ERRO       1
#define CDBG_WARN       2
#define CDBG_INFO       3
#define CDBG_INFO2      4

/*
 * ===========================================================================
 *  global variable declarations
 *
 */
CHAL_SSPI_HANDLE_t sspi_dev[MAX_CHAL_SSPI_BLOCKS] = {
	[0] = {.base = 0,},
	[1] = {.base = 0,},
	[2] = {.base = 0,},
	[3] = {.base = 0,},
	[4] = {.base = 0,},
	[5] = {.base = 0,},
};

/**
*
*  @brief  Set the CAPH clock generation parameters
*
*  @param    handle         (in) SSPI CHAL handle
*  @param    trig           (in) the trigger selection
*  @param    num_cycle_edge (in) the number of 26MHz period between each clock
*                                edges, minus one
*  @param    num_clk_edge   (in) the number of clock edges between each
*                                resynchronization, minus one
*  @param    enable         (in) enable/disable the 26MHz clock
*                                resynchronization mecanism
*
*  @return SSPI status
******************************************************************************/
static CHAL_SSPI_STATUS_t chal_sspi_set_caph_clk_gen(CHAL_HANDLE handle,
						CHAL_SSPI_CAPH_CLK_TRIG_t trig,
						uint32_t num_cycle_edge,
						uint32_t num_clk_edge,
						uint32_t enable);

#if 0
/**
*
*  @brief  Get the CAPH clock generation parameters
*
*  @param    handle         (in)  SSPI CHAL handle
*  @param    trig           (out) the trigger selection
*  @param    num_cycle_edge (out) the number of 26MHz period between each clock
*                                 edges, minus one
*  @param    num_clk_edge   (out) the number of clock edges between each
*                                 resynchronization, minus one
*  @param    enable         (out) enable/disable the 26MHz clock
*                                 resynchronization mecanism
*
*  @return SSPI status
******************************************************************************/
static CHAL_SSPI_STATUS_t chal_sspi_get_caph_clk_gen(CHAL_HANDLE handle,
						CHAL_SSPI_CAPH_CLK_TRIG_t *trig,
						uint32_t *num_cycle_edge,
						uint32_t *num_clk_edge,
						uint32_t *enable);
#endif

/****************************************************************************
*
*  Function Name: CHAL_HANDLE chal_sspi_init(uint32_t base)
*
*  Description: Initialize SSPI CHAL internal data structure
*
****************************************************************************/
CHAL_HANDLE chal_sspi_init(uint32_t baseAddr)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) NULL;
	uint32_t i, val;

	chal_dprintf(CDBG_INFO, "+chal_sspi_init\n");

	if (!baseAddr) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return (CHAL_HANDLE)pDevice;
	}

	/* Don't re-init a block */
	for (i = 0; i < MAX_CHAL_SSPI_BLOCKS; i++)
		if (sspi_dev[i].base == baseAddr)
			return (CHAL_HANDLE)&sspi_dev[i];

	/* find a free slot for this new block */
	for (i = 0; i < MAX_CHAL_SSPI_BLOCKS; i++) {
		if (!sspi_dev[i].base)
			break;
	}

	if (i != MAX_CHAL_SSPI_BLOCKS) {
		sspi_dev[i].base = baseAddr;
		pDevice = &sspi_dev[i];
		val = CHAL_REG_READ32(pDevice->base +
					SSPIL_CONFIGURATION_CHECK_OFFSET);
		sspi_dev[i].num_frames = FLD_GET(
			val, SSPIL_CONFIGURATION_CHECK_FRAME_NUMBER_SHIFT,
			SSPIL_CONFIGURATION_CHECK_FRAME_NUMBER_MASK) + 1;
		sspi_dev[i].num_tasks = FLD_GET(
			val, SSPIL_CONFIGURATION_CHECK_TASK_NUMBER_SHIFT,
				SSPIL_CONFIGURATION_CHECK_TASK_NUMBER_MASK) + 1;
		sspi_dev[i].num_seqs = FLD_GET(
			val, SSPIL_CONFIGURATION_CHECK_SEQUENCE_NUMBER_SHIFT,
			SSPIL_CONFIGURATION_CHECK_SEQUENCE_NUMBER_MASK) + 1;
		sspi_dev[i].num_fifos = FLD_GET(
			val, SSPIL_CONFIGURATION_CHECK_FIFO_NUMBER_SHIFT,
			SSPIL_CONFIGURATION_CHECK_FIFO_NUMBER_MASK) + 1;
		sspi_dev[i].num_dma = FLD_GET(
			val, SSPIL_CONFIGURATION_CHECK_DMA_NUMBER_SHIFT,
			SSPIL_CONFIGURATION_CHECK_DMA_NUMBER_MASK) + 1;
		sspi_dev[i].num_cs = FLD_GET(
			val, SSPIL_CONFIGURATION_CHECK_CS_NUMBER_SHIFT,
			SSPIL_CONFIGURATION_CHECK_CS_NUMBER_MASK) + 1;
		sspi_dev[i].num_tx = FLD_GET(
			val, SSPIL_CONFIGURATION_CHECK_TX_NUMBER_SHIFT,
			SSPIL_CONFIGURATION_CHECK_TX_NUMBER_MASK) + 1;
		sspi_dev[i].fifo_maxsize = FLD_GET(
			val, SSPIL_CONFIGURATION_CHECK_FIFO_MAXSIZE_SHIFT,
			SSPIL_CONFIGURATION_CHECK_FIFO_MAXSIZE_MASK) + 1;
		switch (FLD_GET(
			val, SSPIL_CONFIGURATION_CHECK_FIFO_MAXSIZE_SHIFT,
			SSPIL_CONFIGURATION_CHECK_FIFO_MAXSIZE_MASK)) {
		case 11:
			sspi_dev[i].fifo_maxsize = 2048;
			break;
		case 9:
			sspi_dev[i].fifo_maxsize = 512;
			break;
		case 7:
		default:
			sspi_dev[i].fifo_maxsize = 128;
			break;
		}
	} else {
		chal_dprintf(CDBG_ERRO,
			     "ERROR: chal_sspi_init: bad instance\n");
		//sspi_dev[i].base = (uint32_t) NULL;
		return (CHAL_HANDLE)NULL;
	}
	if ((sspi_dev[i].num_frames < MAX_SSPI_LITE_FRAMES) ||
		(sspi_dev[i].num_tasks < MAX_SSPI_LITE_TASKS) ||
		(sspi_dev[i].num_seqs < MAX_SSPI_LITE_SEQUENCES) ||
		(sspi_dev[i].num_fifos < MAX_SSPI_LITE_FIFOS)) {
		chal_dprintf(CDBG_ERR,
			"Could not get SSPI resource configuration\n");
		return (CHAL_HANDLE)NULL;
	}
	/* Return non-null value */

	chal_dprintf(CDBG_INFO, "-chal_sspi_init(pDevice = 0x%08x\r\n)",
		     (unsigned int)pDevice);

	return (CHAL_HANDLE)pDevice;

}

/****************************************************************************
*
*  Function Name: CHAL_HANDLE chal_sspi_deinit(CHAL_HANDLE handle)
*
*  Description: De-initialize SSPI CHAL internal data structure
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_deinit(CHAL_HANDLE handle)
{
	uint32_t i;
	if (!handle) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
	}

	for (i = 0; i < MAX_CHAL_SSPI_BLOCKS; i++)
		if ((uint32_t)handle == (uint32_t)&sspi_dev[i]) {
			sspi_dev[i].base = 0;
			return CHAL_SSPI_STATUS_SUCCESS;
		}
	return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
}

/****************************************************************************
*
*  Function Name: uint32_t chal_sspi_get_max_fifo_size()
*
*  Description: Returns the physical fifo size. Rx and Tx fifos have
*  the same max size.
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_get_max_fifo_size(CHAL_HANDLE handle,
							uint32_t *fifo_size)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *)handle;

	if (!handle) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
	}
	*fifo_size = pDevice->fifo_maxsize;

	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: uint32_t chal_sspi_rx0_get_dma_port_addr_offset()
*
*  Description: Returns address offset of the FIFO from the base address
*
****************************************************************************/
uint32_t chal_sspi_rx0_get_dma_port_addr_offset(void)
{
	return SSPIL_FIFO_ENTRY0RX_OFFSET;
}

/****************************************************************************
*
*  Function Name: uint32_t chal_sspi_rx1_get_dma_port_addr_offset()
*
*  Description: Returns address offset of the FIFO from the base address
*
****************************************************************************/
uint32_t chal_sspi_rx1_get_dma_port_addr_offset(void)
{
	return SSPIL_FIFO_ENTRY1RX_OFFSET;
}

/****************************************************************************
*
*  Function Name: uint32_t chal_sspi_rx2_get_dma_port_addr_offset()
*
*  Description: Returns address offset of the FIFO from the base address
*
****************************************************************************/
uint32_t chal_sspi_rx2_get_dma_port_addr_offset(void)
{
	return SSPIL_FIFO_ENTRY2RX_OFFSET;
}

/****************************************************************************
*
*  Function Name: uint32_t chal_sspi_rx3_get_dma_port_addr_offset()
*
*  Description: Returns address offset of the FIFO from the base address
*
****************************************************************************/
uint32_t chal_sspi_rx3_get_dma_port_addr_offset(void)
{
	return SSPIL_FIFO_ENTRY3RX_OFFSET;
}

/****************************************************************************
*
*  Function Name: uint32_t chal_sspi_tx0_get_dma_port_addr_offset()
*
*  Description: Returns address offset of the FIFO from the base address
*
****************************************************************************/
uint32_t chal_sspi_tx0_get_dma_port_addr_offset(void)
{
	return SSPIL_FIFO_ENTRY0TX_OFFSET;
}

/****************************************************************************
*
*  Function Name: uint32_t chal_sspi_tx1_get_dma_port_addr_offset()
*
*  Description: Returns address offset of the FIFO from the base address
*
****************************************************************************/
uint32_t chal_sspi_tx1_get_dma_port_addr_offset(void)
{
	return SSPIL_FIFO_ENTRY1TX_OFFSET;
}

/****************************************************************************
*
*  Function Name: uint32_t chal_sspi_tx2_get_dma_port_addr_offset()
*
*  Description: Returns address offset of the FIFO from the base address
*
****************************************************************************/
uint32_t chal_sspi_tx2_get_dma_port_addr_offset(void)
{
	return SSPIL_FIFO_ENTRY2TX_OFFSET;
}

/****************************************************************************
*
*  Function Name: uint32_t chal_sspi_tx3_get_dma_port_addr_offset()
*
*  Description: Returns address offset of the FIFO from the base address
*
****************************************************************************/
uint32_t chal_sspi_tx3_get_dma_port_addr_offset(void)
{
	return SSPIL_FIFO_ENTRY3TX_OFFSET;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_set_mode(
*                                  CHAL_HANDLE handle,
*                                  CHAL_SSPI_MODE_t mode)
*
*  Description: Set SSPI mode
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_set_mode(CHAL_HANDLE handle, CHAL_SSPI_MODE_t mode)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t rdvalue;

	if (!handle) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
	}

	rdvalue = CHAL_REG_READ32(pDevice->base + SSPIL_CONTROL_OFFSET);
	if (mode == SSPI_MODE_SLAVE)
		rdvalue |= SSPIL_CONTROL_SSPI_SLAVE_MASK;
	else
		rdvalue &= ~SSPIL_CONTROL_SSPI_SLAVE_MASK;
	CHAL_REG_WRITE32(pDevice->base + SSPIL_CONTROL_OFFSET, rdvalue);
	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_enable(
*                                  CHAL_HANDLE handle,
*                                  uint32_t off_on)
*
*  Description: Enable SSPI core
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_enable(CHAL_HANDLE handle, uint32_t off_on)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t rdvalue;

	rdvalue = CHAL_REG_READ32(pDevice->base + SSPIL_CONTROL_OFFSET);
	if (off_on)
		rdvalue |= SSPIL_CONTROL_SSPI_ENABLE_MASK;
	else
		rdvalue &= ~SSPIL_CONTROL_SSPI_ENABLE_MASK;
	CHAL_REG_WRITE32(pDevice->base + SSPIL_CONTROL_OFFSET, rdvalue);
	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_soft_reset(
*                                  CHAL_HANDLE handle)
*
*  Description: Soft-reset the SSPI core
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_soft_reset(CHAL_HANDLE handle)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t rdvalue;

	if (!handle) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
	}

	rdvalue = CHAL_REG_READ32(pDevice->base + SSPIL_CONTROL_OFFSET);
	CHAL_REG_WRITE32(pDevice->base + SSPIL_CONTROL_OFFSET,
			 rdvalue | SSPIL_CONTROL_SSPI_SOFT_RESET_MASK);
	return CHAL_SSPI_STATUS_SUCCESS;

}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_set_i2c_frame(
*                                  CHAL_HANDLE handle,
*                                  uint32_t *frm_bitmap,
*                                  uint32_t word_len)
*
*  Description: Setup a SSPI i2c frame
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_set_i2c_frame(CHAL_HANDLE handle,
					uint32_t *frm_bitmap,
					uint32_t word_len)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *)handle;
	uint32_t val = 0, frm_idx = 0, frm_idx0 = 0xFFFFFFFF;

	if (!handle || !frm_bitmap) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_PARA;
	}
	/* Mask out the unused bits of frm_bitmap */
	*frm_bitmap &= (1 << pDevice->num_frames) - 1;

	if (!(*frm_bitmap))
		return CHAL_SSPI_STATUS_ILLEGAL_FRAME;

	/* Get the idx of the first frame */
	while (!(*frm_bitmap & (1 << frm_idx)))
		frm_idx++;

	frm_idx0 = frm_idx;
	*frm_bitmap &= ~(1<<frm_idx++);
	if (!(*frm_bitmap))
		return CHAL_SSPI_STATUS_ILLEGAL_FRAME;

	while (!(*frm_bitmap & (1 << frm_idx)))
		frm_idx++;

	/* Setup the Tx Frame*/
	val = (72 << SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_CS_SIZE_SHIFT) |
		  SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_CS_IDLE_VALUE_MASK |
		  SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_TX_IDLE_VALUE_MASK |
		  SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_TXOEN_IDLE_VALUE_MASK;
	CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx0,
					pDevice->base +
					SSPIL_FRAME0_CS_IDLE_DEF_OFFSET),
					val);

	/* SCLK_START 2, SCLK_END 70, SCLK_RATIO 1:8*/
	val = (2 << SSPIL_FRAME0_SCLK_DEF_FRAME0_SCLK_START_SHIFT) |
		  (70 << SSPIL_FRAME0_SCLK_DEF_FRAME0_SCLK_END_SHIFT) |
		  (2 << SSPIL_FRAME0_SCLK_DEF_FRAME0_SCLK_RATIO_SHIFT);
	CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx0,
					pDevice->base +
					SSPIL_FRAME0_SCLK_DEF_OFFSET),
					val);

	/*
	 *  TXOEN_START 0x0, TXOEN_END 8*8, TX_WORD_LENGTH 0x7, TX_START 0x0,
	 *  TX_EXTENDED_RIGHT 0x0, TX_EXTENDED_LEFT 0x0
	 */
	val = (0 << SSPIL_FRAME0_TX_DEF_FRAME0_TX_START_SHIFT) |
		(0 << SSPIL_FRAME0_TX_DEF_FRAME0_TXOEN_START_SHIFT) |
		((word_len * 8) << SSPIL_FRAME0_TX_DEF_FRAME0_TXOEN_END_SHIFT) |
		(word_len-1) << SSPIL_FRAME0_TX_DEF_FRAME0_TX_WORD_LENGTH_SHIFT;
	CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx0,
				pDevice->base + SSPIL_FRAME0_TX_DEF_OFFSET),
				val);

	/* RX_WORD_LENGTH: 8, RX_START: 14 */
	val = ((word_len - 1) <<
			SSPIL_FRAME0_RX_DEF_FRAME0_RX_WORD_LENGTH_SHIFT) |
		  (14 << SSPIL_FRAME0_RX_DEF_FRAME0_RX_START_SHIFT);
	CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx0,
				pDevice->base + SSPIL_FRAME0_RX_DEF_OFFSET),
				val);
	*frm_bitmap = 1 << frm_idx0;

	/* Setup the Rx Frame*/
	val = ((72) << SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_CS_SIZE_SHIFT) |
		  SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_CS_IDLE_VALUE_MASK |
		  SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_TX_IDLE_VALUE_MASK |
		  SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_TXOEN_IDLE_VALUE_MASK;
	CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx,
			pDevice->base + SSPIL_FRAME0_CS_IDLE_DEF_OFFSET),
			val);

	/* SCLK_START 2, SCLK_END 70, FIRST_ACTIVE_EDGE 0, SCLK_RATIO 1:8*/
	val = (2 << SSPIL_FRAME0_SCLK_DEF_FRAME0_SCLK_START_SHIFT) |
		  (70 << SSPIL_FRAME0_SCLK_DEF_FRAME0_SCLK_END_SHIFT) |
		  (2 << SSPIL_FRAME0_SCLK_DEF_FRAME0_SCLK_RATIO_SHIFT);
	CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx,
				pDevice->base + SSPIL_FRAME0_SCLK_DEF_OFFSET),
				val);

	/*
	 *  TXOEN_START 0x0, TXOEN_END 8*8, TX_WORD_LENGTH 0x7, TX_START 0x0,
	 *  TX_EXTENDED_RIGHT 0x0, TX_EXTENDED_LEFT 0x0
	 */
	val = (8 << SSPIL_FRAME0_TX_DEF_FRAME0_TX_START_SHIFT) |
		  (8 << SSPIL_FRAME0_TX_DEF_FRAME0_TXOEN_START_SHIFT) |
		  (((word_len + 1) * 8) <<
				SSPIL_FRAME0_TX_DEF_FRAME0_TXOEN_END_SHIFT) |
		  ((word_len - 1) <<
			SSPIL_FRAME0_TX_DEF_FRAME0_TX_WORD_LENGTH_SHIFT);
	CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx,
				pDevice->base + SSPIL_FRAME0_TX_DEF_OFFSET),
				val);

	/* RX_WORD_LENGTH: 8, RX_START: 6 */
	val = ((word_len-1) <<
			SSPIL_FRAME0_RX_DEF_FRAME0_RX_WORD_LENGTH_SHIFT) |
		  (6 << SSPIL_FRAME0_RX_DEF_FRAME0_RX_START_SHIFT);
	CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx,
				pDevice->base + SSPIL_FRAME0_RX_DEF_OFFSET),
				val);
	*frm_bitmap |= 1 << frm_idx;

	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_set_spi_frame(
*                                  CHAL_HANDLE handle,
*                                  uint32_t *frm_bitmap,
*                                  CHAL_SSPI_PROT_t prot,
*                                  uint32_t word_len,
*                                  CHAL_SSPI_FRAME_EXT_t ext_setup)
*
*  Description: Setup a SSPI SPI frame
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_set_spi_frame(CHAL_HANDLE handle,
						uint32_t *frm_bitmap,
						CHAL_SSPI_PROT_t prot,
						uint32_t word_len,
						CHAL_SSPI_FRAME_EXT_t ext_setup)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *)handle;
	uint32_t val = 0, frm_idx = 0, mode;

	if (!handle || !frm_bitmap) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_PARA;
	}
	/* Mask out the unused bits of frm_bitmap */
	*frm_bitmap &= (1 << pDevice->num_frames) - 1;

	if (!(*frm_bitmap))
		return CHAL_SSPI_STATUS_ILLEGAL_FRAME;

	mode = CHAL_REG_READ32(pDevice->base + SSPIL_CONTROL_OFFSET) &
		   SSPIL_CONTROL_SSPI_SLAVE_MASK;

	/* Get the idx of the first frame */
	while (!(*frm_bitmap & (1 << frm_idx)))
		frm_idx++;

	switch (prot) {
	case SSPI_PROT_SPI_MODE0:
		/*
		 *  Default frame setup for SPI_MODE0:
		 *  CS_SIZE word_len * 2, CS_ACTIVE 0x0, CS_IDLE_VALUE 0x1,
		 *  SCLK_IDLE_VALUE 0x0, TX_IDLE_VALUE 0x0, TXOEN_IDLE_VALUE 0x0
		 */
		val = ((word_len * SSPI_MASTER_CLK_CYCLE_PER_BIT) <<
			SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_CS_SIZE_SHIFT) |
			(0x2 <<
			SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_CS_ACTIVE_SHIFT) |
			SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_CS_IDLE_VALUE_MASK;
		CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx,
			pDevice->base + SSPIL_FRAME0_CS_IDLE_DEF_OFFSET),
			val);

		/* SCLK_START 0x2, SCLK_END word_len * 2, SCLK_RATIO 1:2 */
		val = (0x2 << SSPIL_FRAME0_SCLK_DEF_FRAME0_SCLK_START_SHIFT) |
			  (((word_len * SSPI_MASTER_CLK_CYCLE_PER_BIT) + 1)<<
				SSPIL_FRAME0_SCLK_DEF_FRAME0_SCLK_END_SHIFT);
		CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx,
			pDevice->base + SSPIL_FRAME0_SCLK_DEF_OFFSET),
			val);

		/*
		 *  TXOEN_START 0x0, TXOEN_END 0x00, TX_WORD_LENGTH word_len-1,
		 *  TX_START 0x1 for low speed and 0x0 for high speed,
		 *  TX_EXTENDED_RIGHT 0x0, TX_EXTENDED_LEFT 0x0
		 */
		val = (ext_setup == SSPI_FRAME_EXT_SPI_HS) ? 0 :
			  (0x1 << SSPIL_FRAME0_TX_DEF_FRAME0_TX_START_SHIFT);
		val |= (word_len-1) <<
				SSPIL_FRAME0_TX_DEF_FRAME0_TX_WORD_LENGTH_SHIFT;
		CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx,
			pDevice->base + SSPIL_FRAME0_TX_DEF_OFFSET),
			val);

		/* RX_WORD_LENGTH word_len-1, RX_START 0x2*/
		val = ((word_len-1) <<
			SSPIL_FRAME0_RX_DEF_FRAME0_RX_WORD_LENGTH_SHIFT) |
			(0x2 << SSPIL_FRAME0_RX_DEF_FRAME0_RX_START_SHIFT);
		CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx,
				pDevice->base + SSPIL_FRAME0_RX_DEF_OFFSET),
				val);
		*frm_bitmap = 1 << frm_idx;
		break;

	case SSPI_PROT_SPI_MODE1:
		/* Default frame setup for SPI_MODE1:
		 * CS_SIZE word_len * 2, CS_ACTIVE 0x0, CS_IDLE_VALUE 0x1,
		 * SCLK_IDLE_VALUE 0x0, TX_IDLE_VALUE 0x0, TXOEN_IDLE_VALUE 0x0
		 */
		val = ((word_len * SSPI_MASTER_CLK_CYCLE_PER_BIT) <<
			SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_CS_SIZE_SHIFT) |
			(0x2 <<
			SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_CS_ACTIVE_SHIFT) |
			SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_CS_IDLE_VALUE_MASK;
		CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx,
			pDevice->base + SSPIL_FRAME0_CS_IDLE_DEF_OFFSET),
			val);

		/* SCLK_START 0x2, SCLK_END word_len * 2, SCLK_RATIO 1:2 */
		val = (0x2 << SSPIL_FRAME0_SCLK_DEF_FRAME0_SCLK_START_SHIFT) |
			(((word_len * SSPI_MASTER_CLK_CYCLE_PER_BIT) + 1) <<
				SSPIL_FRAME0_SCLK_DEF_FRAME0_SCLK_END_SHIFT);
		CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx,
				pDevice->base + SSPIL_FRAME0_SCLK_DEF_OFFSET),
				val);

		/* TXOEN_START 0x0, TXOEN_END 0x00, TX_WORD_LENGTH word_len-1,
		 * TX_START 0x2 for low speed and 0x1 for high speed,
		 * TX_EXTENDED_RIGHT 0x0, TX_EXTENDED_LEFT 0x0
		 */
		val = (ext_setup == SSPI_FRAME_EXT_SPI_HS) ?
			  (0x1 << SSPIL_FRAME0_TX_DEF_FRAME0_TX_START_SHIFT) :
			  (0x2 << SSPIL_FRAME0_TX_DEF_FRAME0_TX_START_SHIFT);
		val |= (word_len - 1) <<
			SSPIL_FRAME0_TX_DEF_FRAME0_TX_WORD_LENGTH_SHIFT;
		CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx,
				pDevice->base + SSPIL_FRAME0_TX_DEF_OFFSET),
				val);

		/* RX_WORD_LENGTH word_len-1, RX_START 0x3 */
		val = ((word_len - 1) <<
			SSPIL_FRAME0_RX_DEF_FRAME0_RX_WORD_LENGTH_SHIFT) |
			(0x3 << SSPIL_FRAME0_RX_DEF_FRAME0_RX_START_SHIFT);
		CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx,
				pDevice->base + SSPIL_FRAME0_RX_DEF_OFFSET),
				val);
		*frm_bitmap = 1 << frm_idx;
		break;

	case SSPI_PROT_SPI_MODE2:
		/* Default frame setup for SPI_MODE2:
		 * CS_SIZE word_len * 2, CS_ACTIVE 0x0, CS_IDLE_VALUE 0x1,
		 * SCLK_IDLE_VALUE 0x1, TX_IDLE_VALUE 0x0, TXOEN_IDLE_VALUE 0x0
		 */
		val = ((word_len * SSPI_MASTER_CLK_CYCLE_PER_BIT) <<
			SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_CS_SIZE_SHIFT) |
			(0x2 <<
			SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_CS_ACTIVE_SHIFT) |
			SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_CS_IDLE_VALUE_MASK |
			SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_SCLK_IDLE_VALUE_MASK;
		CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx,
			pDevice->base + SSPIL_FRAME0_CS_IDLE_DEF_OFFSET),
			val);

		/* SCLK_START 0x2, SCLK_END word_len * 2, SCLK_RATIO 1:2 */
		val = (0x2 << SSPIL_FRAME0_SCLK_DEF_FRAME0_SCLK_START_SHIFT) |
			(((word_len * SSPI_MASTER_CLK_CYCLE_PER_BIT) + 1)<<
				SSPIL_FRAME0_SCLK_DEF_FRAME0_SCLK_END_SHIFT);
		CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx,
				pDevice->base + SSPIL_FRAME0_SCLK_DEF_OFFSET),
				val);

		/* TXOEN_START 0x0, TXOEN_END 0x00, TX_WORD_LENGTH word_len-1,
		 * TX_START 0x1 for low speed and 0x0 for high speed,
		 * TX_EXTENDED_RIGHT 0x0, TX_EXTENDED_LEFT 0x0
		 */
		val = (ext_setup == SSPI_FRAME_EXT_SPI_HS) ? 0 :
			  (0x1 << SSPIL_FRAME0_TX_DEF_FRAME0_TX_START_SHIFT);
		val |= (word_len - 1) <<
			SSPIL_FRAME0_TX_DEF_FRAME0_TX_WORD_LENGTH_SHIFT;
		CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx,
				pDevice->base + SSPIL_FRAME0_TX_DEF_OFFSET),
				val);

		/* RX_WORD_LENGTH word_len-1, RX_START 0x2 */
		val = ((word_len - 1) <<
			SSPIL_FRAME0_RX_DEF_FRAME0_RX_WORD_LENGTH_SHIFT) |
			(0x2 << SSPIL_FRAME0_RX_DEF_FRAME0_RX_START_SHIFT);
		CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx,
				pDevice->base + SSPIL_FRAME0_RX_DEF_OFFSET),
				val);
		*frm_bitmap = 1 << frm_idx;
		break;

	case SSPI_PROT_SPI_MODE3:
		/* Default frame setup for SPI_MODE3:
		 * CS_SIZE word_len * 2, CS_ACTIVE 0x0, CS_IDLE_VALUE 0x1,
		 * SCLK_IDLE_VALUE 0x1, TX_IDLE_VALUE 0x0, TXOEN_IDLE_VALUE 0x0
		 */
		val = ((word_len * SSPI_MASTER_CLK_CYCLE_PER_BIT) <<
			SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_CS_SIZE_SHIFT) |
			(0x2 <<
			SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_CS_ACTIVE_SHIFT) |
			SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_CS_IDLE_VALUE_MASK |
			SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_SCLK_IDLE_VALUE_MASK;
		CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx,
			pDevice->base + SSPIL_FRAME0_CS_IDLE_DEF_OFFSET),
			val);

		/* SCLK_START 0x2, SCLK_END word_len * 2, SCLK_RATIO 1:2 */
		val = (0x2 << SSPIL_FRAME0_SCLK_DEF_FRAME0_SCLK_START_SHIFT) |
			  (((word_len * SSPI_MASTER_CLK_CYCLE_PER_BIT) + 1) <<
				SSPIL_FRAME0_SCLK_DEF_FRAME0_SCLK_END_SHIFT);
		CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx,
				pDevice->base + SSPIL_FRAME0_SCLK_DEF_OFFSET),
				val);

		/* TXOEN_START 0x0, TXOEN_END 0x00, TX_WORD_LENGTH word_len-1,
		 * TX_START 0x2 for low speed and 0x1 for high speed,
		 * TX_EXTENDED_RIGHT 0x0, TX_EXTENDED_LEFT 0x0
		 */
		val = (ext_setup == SSPI_FRAME_EXT_SPI_HS) ?
			  (0x1 << SSPIL_FRAME0_TX_DEF_FRAME0_TX_START_SHIFT) :
			  (0x2 << SSPIL_FRAME0_TX_DEF_FRAME0_TX_START_SHIFT);
		val |= (word_len - 1) <<
			SSPIL_FRAME0_TX_DEF_FRAME0_TX_WORD_LENGTH_SHIFT;
		CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx,
				pDevice->base + SSPIL_FRAME0_TX_DEF_OFFSET),
				val);

		/* RX_WORD_LENGTH word_len-1, RX_START 0x3 */
		val = ((word_len - 1) <<
			SSPIL_FRAME0_RX_DEF_FRAME0_RX_WORD_LENGTH_SHIFT) |
			(0x3 << SSPIL_FRAME0_RX_DEF_FRAME0_RX_START_SHIFT);
		CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx,
				pDevice->base + SSPIL_FRAME0_RX_DEF_OFFSET),
				val);
		*frm_bitmap = 1 << frm_idx;
		break;

	default:
		return CHAL_SSPI_STATUS_FAILURE;

	}
	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_set_i2s_frame(
*                                  CHAL_HANDLE handle,
*                                  uint32_t *frm_bitmap,
*                                  CHAL_SSPI_PROT_t prot,
*                                  chal_sspi_frm_conf_t frm_conf)
*
*  Description: Setup a SSPI I2S frame
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_set_i2s_frame(CHAL_HANDLE handle,
				uint32_t *frm_bitmap,
				CHAL_SSPI_PROT_t prot,
				chal_sspi_frm_conf_t frm_conf)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *)handle;
	uint32_t val = 0, frm_idx = 0, cs_len, mode;

	if (!handle || !frm_bitmap) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_PARA;
	}
	/* Mask out the unused bits of frm_bitmap */
	*frm_bitmap &= (1 << pDevice->num_frames) - 1;

	if (!(*frm_bitmap))
		return CHAL_SSPI_STATUS_ILLEGAL_FRAME;

	mode = CHAL_REG_READ32(pDevice->base + SSPIL_CONTROL_OFFSET) &
		   SSPIL_CONTROL_SSPI_SLAVE_MASK;

	/* Get the idx of the first frame */
	while (!(*frm_bitmap & (1 << frm_idx)))
		frm_idx++;

	cs_len = frm_conf.tx_len + frm_conf.tx_prepad_bits +
			  frm_conf.tx_postpad_bits;
	cs_len = ((frm_conf.rx_len + frm_conf.rx_delay_bits) > cs_len) ?
			(frm_conf.rx_len + frm_conf.rx_delay_bits) : cs_len;
	switch (prot) {
	case SSPI_PROT_I2S_MODE1:
		/*
		 *  TXOEN_IDLE_VALUE: 0, TX_IDLE_VALUE: 0, CS_IDLE_VALUE: 0,
		 *  CS_SIZE: 32, CS_ACTIVE: 0
		 */
		val = ((cs_len * SSPI_MASTER_CLK_CYCLE_PER_BIT) <<
			SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_CS_SIZE_SHIFT) |
			((frm_conf.tx_padval ? 1 : 0) <<
			SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_TX_IDLE_VALUE_SHIFT);
		CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx,
			pDevice->base + SSPIL_FRAME0_CS_IDLE_DEF_OFFSET),
			val);

		/* SCLK_START: -1, SCLK_END: 33, SCLK_RATIO: 1:2*/
		val = (SSPI_SCLK_CYCLE_MINUS1 <<
			SSPIL_FRAME0_SCLK_DEF_FRAME0_SCLK_START_SHIFT) |
			(((cs_len << 1) + 1) <<
			SSPIL_FRAME0_SCLK_DEF_FRAME0_SCLK_END_SHIFT) |
			(0 << SSPIL_FRAME0_SCLK_DEF_FRAME0_SCLK_RATIO_SHIFT);
		CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx,
				pDevice->base + SSPIL_FRAME0_SCLK_DEF_OFFSET),
				val);

		/*
		 *  TXOEN_START 0x0, TXOEN_END 0x00, TX_WORD_LENGTH 0xf,
		 *  TX_START 0x0, TX_EXTENDED_RIGHT 0x0, TX_EXTENDED_LEFT 0x0
		 */
		val = (frm_conf.tx_prepad_bits <<
			SSPIL_FRAME0_TX_DEF_FRAME0_TX_START_SHIFT) |
			((frm_conf.tx_len - 1) <<
			SSPIL_FRAME0_TX_DEF_FRAME0_TX_WORD_LENGTH_SHIFT);
		CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx,
				pDevice->base + SSPIL_FRAME0_TX_DEF_OFFSET),
				val);

		/* RX_WORD_LENGTH: 16, RX_START: 1*/
		val = ((frm_conf.rx_len - 1) <<
			SSPIL_FRAME0_RX_DEF_FRAME0_RX_WORD_LENGTH_SHIFT) |
			((1 + frm_conf.rx_delay_bits) <<
				SSPIL_FRAME0_RX_DEF_FRAME0_RX_START_SHIFT);
		CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx,
				pDevice->base + SSPIL_FRAME0_RX_DEF_OFFSET),
				val);
		*frm_bitmap = 1 << frm_idx;
		break;

	case SSPI_PROT_I2S_MODE2:
		if (!mode) {
			/*
			 *  TXOEN_IDLE_VALUE: 0, TX_IDLE_VALUE: 0,
			 * CS_IDLE_VALUE: 0, CS_SIZE: 32, CS_ACTIVE: 0
			 */
			val = (((cs_len) * SSPI_MASTER_CLK_CYCLE_PER_BIT) <<
			  SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_CS_SIZE_SHIFT) |
			  ((frm_conf.tx_padval ? 1 : 0) <<
			  SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_TX_IDLE_VALUE_SHIFT);
		} else {
			/*
			 *  TXOEN_IDLE_VALUE: 0, TX_IDLE_VALUE: 0,
			 *  CS_IDLE_VALUE: 0, CS_SIZE: 32, CS_ACTIVE: 0
			 */
			val = ((cs_len) * SSPI_MASTER_CLK_CYCLE_PER_BIT) <<
			 SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_CS_SIZE_SHIFT |
			 SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_SCLK_ACTIVE_EDGE_MASK |
			 SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_CS_IDLE_VALUE_MASK |
			 ((frm_conf.tx_padval ? 1 : 0) <<
			 SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_TX_IDLE_VALUE_SHIFT);
		}
		CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx,
			pDevice->base + SSPIL_FRAME0_CS_IDLE_DEF_OFFSET),
			val);
		if (!mode) {
			/* SCLK_START: -1, SCLK_END: 35, SCLK_RATIO: 1:2*/
			val = (0xF <<
			 SSPIL_FRAME0_SCLK_DEF_FRAME0_SCLK_START_SHIFT) |
			 ((((cs_len) * SSPI_MASTER_CLK_CYCLE_PER_BIT) + 3)
			 << SSPIL_FRAME0_SCLK_DEF_FRAME0_SCLK_END_SHIFT) |
			  (0 << SSPIL_FRAME0_SCLK_DEF_FRAME0_SCLK_RATIO_SHIFT);
			CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx,
				pDevice->base + SSPIL_FRAME0_SCLK_DEF_OFFSET),
				val);
		}
		/*
		 *  TXOEN_START 0x0, TXOEN_END 0x00, TX_WORD_LENGTH 0xf,
		 * TX_START 0x2, TX_EXTENDED_RIGHT 0x0, TX_EXTENDED_LEFT 0x0
		 */
		val = ((2 + frm_conf.tx_prepad_bits) <<
			SSPIL_FRAME0_TX_DEF_FRAME0_TX_START_SHIFT) |
			((frm_conf.tx_len - 1) <<
			  SSPIL_FRAME0_TX_DEF_FRAME0_TX_WORD_LENGTH_SHIFT);

		CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx,
				pDevice->base + SSPIL_FRAME0_TX_DEF_OFFSET),
				val);

		/* RX_WORD_LENGTH: 16, RX_START 3*/
		val = ((frm_conf.rx_len - 1) <<
			SSPIL_FRAME0_RX_DEF_FRAME0_RX_WORD_LENGTH_SHIFT) |
			((3 + frm_conf.rx_delay_bits) <<
				  SSPIL_FRAME0_RX_DEF_FRAME0_RX_START_SHIFT);

		CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx,
				pDevice->base + SSPIL_FRAME0_RX_DEF_OFFSET),
				val);
		*frm_bitmap = 1 << frm_idx;
		break;

	default:
		return CHAL_SSPI_STATUS_FAILURE;

	}
	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_set_pcm_frame(
*                                  CHAL_HANDLE handle,
*                                  uint32_t *frm_bitmap,
*                                  CHAL_SSPI_PROT_t prot,
*                                  uint32_t word_len,
*                                  uint32_t ext_bits)
*
*  Description: Setup a SSPI frame
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_set_pcm_frame(CHAL_HANDLE handle,
						uint32_t *frm_bitmap,
						chal_sspi_frm_conf_t *frm_conf)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *)handle;
	uint32_t val = 0, frm_idx = 0, i = 0, tmp, mode, cs_len;

	if (!handle || !frm_bitmap) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_PARA;
	}
	tmp = *frm_bitmap;
	/* Mask out the unused bits of frm_bitmap */
	tmp &= (1 << pDevice->num_frames) - 1;

	if (!tmp)
		return CHAL_SSPI_STATUS_ILLEGAL_FRAME;

	mode = CHAL_REG_READ32(pDevice->base + SSPIL_CONTROL_OFFSET) &
		   SSPIL_CONTROL_SSPI_SLAVE_MASK;

	while (tmp) {
		/* Get the idx of the first frame */
		if (!(tmp & (1 << frm_idx))) {
			frm_idx++;
			continue;
		}
		tmp &= ~(1 << frm_idx);

		cs_len = frm_conf[i].tx_len + frm_conf[i].tx_prepad_bits +
				 frm_conf[i].tx_postpad_bits;
		cs_len =
		  ((frm_conf[i].rx_len + frm_conf[i].rx_delay_bits) > cs_len) ?
		  (frm_conf[i].rx_len + frm_conf[i].rx_delay_bits) : cs_len;

		if (!mode) {
			/*
			 *  TXOEN_IDLE_VALUE: 0, TX_IDLE_VALUE: 0,
			 *  CS_IDLE_VALUE: 0, CS_SIZE: 32, CS_ACTIVE: 2
			 */
			val = ((cs_len << 1) <<
			 SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_CS_SIZE_SHIFT) |
			 (2 <<
			 SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_CS_ACTIVE_SHIFT) |
			 ((frm_conf[i].tx_padval ? 1 : 0) <<
			  SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_TX_IDLE_VALUE_SHIFT);
		} else {
			/*
			 *  TXOEN_IDLE_VALUE: 0, TX_IDLE_VALUE: 0,
			 * CS_IDLE_VALUE: 0, CS_SIZE: 32, CS_ACTIVE: 0
			 */
			val = ((cs_len * SSPI_MASTER_CLK_CYCLE_PER_BIT) <<
			 SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_CS_SIZE_SHIFT) |
			 ((frm_conf[i].tx_padval ? 1 : 0) <<
			 SSPIL_FRAME0_CS_IDLE_DEF_FRAME0_TX_IDLE_VALUE_SHIFT);
		}
		CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx,
			pDevice->base + SSPIL_FRAME0_CS_IDLE_DEF_OFFSET),
			val);

		if (!mode) {
			/* SCLK_START: 0, SCLK_END: 35, SCLK_RATIO: 1:2*/
			val = ((cs_len * SSPI_MASTER_CLK_CYCLE_PER_BIT) <<
			  SSPIL_FRAME0_SCLK_DEF_FRAME0_SCLK_END_SHIFT) |
			  (0 << SSPIL_FRAME0_SCLK_DEF_FRAME0_SCLK_RATIO_SHIFT);
			CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx,
				pDevice->base + SSPIL_FRAME0_SCLK_DEF_OFFSET),
				val);
		}

		if (!mode) {
			/*
			 *  TXOEN_START 0x0, TXOEN_END 0x00, TX_WORD_LENGTH 0xf,
			 *  TX_START 0x2, TX_EXTENDED_RIGHT 0x0,
			 *  TX_EXTENDED_LEFT 0x0
			 */
			val = ((2 + frm_conf[i].tx_prepad_bits) <<
			  SSPIL_FRAME0_TX_DEF_FRAME0_TX_START_SHIFT) |
			  ((frm_conf[i].tx_len-1) <<
			  SSPIL_FRAME0_TX_DEF_FRAME0_TX_WORD_LENGTH_SHIFT);
		} else {
			/*
			 *  Since 2 clk cycles are required to rx/tx one bit
			 *  of data, TXOEN_END is set to
			 *  (word_len << 1)+2 = word_len * 2+2
			 */
			val = ((2 + frm_conf[i].tx_prepad_bits) <<
			  SSPIL_FRAME0_TX_DEF_FRAME0_TX_START_SHIFT) |
			  ((2 + frm_conf[i].tx_prepad_bits) <<
			  SSPIL_FRAME0_TX_DEF_FRAME0_TXOEN_START_SHIFT) |
			  ((((frm_conf[i].tx_len + frm_conf[i].tx_prepad_bits) *
			  SSPI_MASTER_CLK_CYCLE_PER_BIT)+2) <<
			  SSPIL_FRAME0_TX_DEF_FRAME0_TXOEN_END_SHIFT) |
			  ((frm_conf[i].tx_len-1) <<
			  SSPIL_FRAME0_TX_DEF_FRAME0_TX_WORD_LENGTH_SHIFT);
		}
		CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx,
				pDevice->base + SSPIL_FRAME0_TX_DEF_OFFSET),
				val);
		if (!mode) {
			/* RX_WORD_LENGTH: 16, RX_START 3 */
			val = ((frm_conf[i].rx_len - 1) <<
			  SSPIL_FRAME0_RX_DEF_FRAME0_RX_WORD_LENGTH_SHIFT) |
			  ((3 +  frm_conf[i].rx_delay_bits) <<
			  SSPIL_FRAME0_RX_DEF_FRAME0_RX_START_SHIFT);
		} else {
			val = ((frm_conf[i].rx_len - 1) <<
			  SSPIL_FRAME0_RX_DEF_FRAME0_RX_WORD_LENGTH_SHIFT) |
			  ((3 +  frm_conf[i].rx_delay_bits) <<
			  SSPIL_FRAME0_RX_DEF_FRAME0_RX_START_SHIFT);
		}
		CHAL_REG_WRITE32(REG_FRAME_ADDR(frm_idx,
				pDevice->base + SSPIL_FRAME0_RX_DEF_OFFSET),
				val);
		i++;
	}
	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_set_pattern(
*                                  CHAL_HANDLE handle,
*                                  uint32_t patt_idx,
*                                  CHAL_SSPI_PROT_t prot)
*
*  Description: Setup a SSPI pattern
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_set_pattern(CHAL_HANDLE handle,
					 uint32_t patt_idx,
					 CHAL_SSPI_PATT_TYPE_t type)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t val;

	if (!handle) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
	}

	if (patt_idx >= MAX_SSPI_FULL_PATTERNS)
		return CHAL_SSPI_STATUS_ILLEGAL_INDEX;

	switch (type) {
	case CHAL_SSPI_PATT_TYPE_I2C_STD_START:
		val = 0;
		val = (8 << SSPIL_PATTERN_0_CONTROL_PATTERN0_LENGTH_SHIFT) |
		    (0xF << SSPIL_PATTERN_0_CONTROL_PATTERN0_INITIAL_SHIFT) |
		    (0xF << SSPIL_PATTERN_0_CONTROL_PATTERN0_FINAL_SHIFT);
		CHAL_REG_WRITE32(REG_PAT_CTL_ADDR
				(patt_idx,
				pDevice->base +
				SSPIL_PATTERN_0_CONTROL_OFFSET),
				val);
		CHAL_REG_WRITE32(REG_PAT_ADDR
				 (patt_idx,
				  pDevice->base + SSPIL_PATTERN_0_CS_OFFSET),
				 SSPIL_PATTERN_0_CS_PATTERN0_CS_MASK);
		CHAL_REG_WRITE32(REG_PAT_ADDR
				 (patt_idx,
				  pDevice->base + SSPIL_PATTERN_0_SCLK_OFFSET),
				 0x3F <<
				 SSPIL_PATTERN_0_SCLK_PATTERN0_SCLK_SHIFT);
		CHAL_REG_WRITE32(REG_PAT_ADDR
				 (patt_idx,
				  pDevice->base + SSPIL_PATTERN_0_TX_OFFSET),
				 0x3 << SSPIL_PATTERN_0_TX_PATTERN0_TX_SHIFT);
		CHAL_REG_WRITE32(REG_PAT_ADDR
				 (patt_idx,
				  pDevice->base + SSPIL_PATTERN_0_TXOEN_OFFSET),
				 0x2 <<
				 SSPIL_PATTERN_0_TXOEN_PATTERN0_TXOEN_SHIFT);
		break;

	case CHAL_SSPI_PATT_TYPE_I2C_STD_STOP:
		val = 0;
		val = (8 << SSPIL_PATTERN_0_CONTROL_PATTERN0_LENGTH_SHIFT) |
		    (0xF << SSPIL_PATTERN_0_CONTROL_PATTERN0_INITIAL_SHIFT) |
		    (0xF << SSPIL_PATTERN_0_CONTROL_PATTERN0_FINAL_SHIFT) |
		    SSPIL_PATTERN_0_CONTROL_PATTERN0_IDLE_EXTENSION_MASK |
		    SSPIL_PATTERN_0_CONTROL_PATTERN0_EXTENSION_CHECK_MASK;
		CHAL_REG_WRITE32(REG_PAT_CTL_ADDR
				(patt_idx,
				pDevice->base +
				SSPIL_PATTERN_0_CONTROL_OFFSET),
				val);
		CHAL_REG_WRITE32(REG_PAT_ADDR
				 (patt_idx,
				  pDevice->base + SSPIL_PATTERN_0_CS_OFFSET),
				 SSPIL_PATTERN_0_CS_PATTERN0_CS_MASK);
		CHAL_REG_WRITE32(REG_PAT_ADDR
				 (patt_idx,
				  pDevice->base + SSPIL_PATTERN_0_SCLK_OFFSET),
				 0xFC <<
				 SSPIL_PATTERN_0_SCLK_PATTERN0_SCLK_SHIFT);
		CHAL_REG_WRITE32(REG_PAT_ADDR
				 (patt_idx,
				pDevice->base + SSPIL_PATTERN_0_TX_OFFSET),
				0xC0 << SSPIL_PATTERN_0_TX_PATTERN0_TX_SHIFT);
		CHAL_REG_WRITE32(REG_PAT_ADDR
				 (patt_idx,
				  pDevice->base + SSPIL_PATTERN_0_TXOEN_OFFSET),
				 0x2 <<
				 SSPIL_PATTERN_0_TXOEN_PATTERN0_TXOEN_SHIFT);
		break;
	case CHAL_SSPI_PATT_TYPE_I2C_STD_RESTART:
		/*TODO: */
		break;
	default:
		break;

	}
	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_set_idle_state(
*                                  CHAL_HANDLE handle,
*                                  CHAL_SSPI_PROT_t prot)
*
*  Description: Setup the idle state of SSPI signal lines before startint
*               any task.
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_set_idle_state(CHAL_HANDLE handle,
					    CHAL_SSPI_PROT_t prot)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t mode;

	if (!handle) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
	}

	mode = CHAL_REG_READ32(pDevice->base + SSPIL_CONTROL_OFFSET) &
	    SSPIL_CONTROL_SSPI_SLAVE_MASK;
	if (mode) {
		/*
		 *  In slave mode
		 *  Default set for slave mode:
		 *  STATE_MACHINE_CONTROL:
		 *  MASTER_CS0_IDLE  - high, MASTER_CSOEN0_IDLE  - high,
		 *  MASTER_SCLK_IDLE - high, MASTER_SCLKOEN_IDLE - high
		 *  MS_TX0_IDLE     -  high, MS_TXOEN0_IDLE      - low
		 */
		CHAL_REG_WRITE32(pDevice->base +
			SSPIL_STATE_MACHINE_CONTROL_OFFSET,
			(SSPIL_STATE_MACHINE_CONTROL_MASTER_CS0_IDLE_MASK
			|
			SSPIL_STATE_MACHINE_CONTROL_MASTER_CSOEN0_IDLE_MASK
			|
			SSPIL_STATE_MACHINE_CONTROL_MASTER_SCLK_IDLE_MASK
			|
			SSPIL_STATE_MACHINE_CONTROL_MASTER_SCLKOEN_IDLE_MASK));

		return CHAL_SSPI_STATUS_SUCCESS;

	}

	/* In master mode */
	switch (prot) {
	case SSPI_PROT_SPI_MODE0:
	case SSPI_PROT_SPI_MODE1:
		/*
		 *  Default set for SPI_MODE0:
		 *  STATE_MACHINE_CONTROL:
		 *  MASTER_CS0_IDLE  - high, MASTER_CS1_IDLE - high,
		 *  MASTER_CS2_IDLE  - high, MASTER_CS3_IDLE - high,
		 *  MASTER_SCLK_IDLE - low,  MS_TX0_IDLE     - high,
		 *  MS_TX1_IDLE      - high
		 */
		CHAL_REG_WRITE32(pDevice->base +
			SSPIL_STATE_MACHINE_CONTROL_OFFSET,
			(SSPIL_STATE_MACHINE_CONTROL_MASTER_CS0_IDLE_MASK
			|
			SSPIL_STATE_MACHINE_CONTROL_MS_TX0_IDLE_MASK));
		break;

	case SSPI_PROT_SPI_MODE2:
	case SSPI_PROT_SPI_MODE3:

		/* Default set for SPI_MODE3:
		 * STATE_MACHINE_CONTROL:
		 * MASTER_CS0_IDLE  - high, MASTER_CS1_IDLE - high,
		 * MASTER_CS2_IDLE  - high, MASTER_CS3_IDLE - high,
		 * MASTER_SCLK_IDLE - high,  MS_TX0_IDLE     - high,
		 * MS_TX1_IDLE      - high
		 */
		CHAL_REG_WRITE32(pDevice->base +
			SSPIL_STATE_MACHINE_CONTROL_OFFSET,
			(SSPIL_STATE_MACHINE_CONTROL_MASTER_CS0_IDLE_MASK
			|
			SSPIL_STATE_MACHINE_CONTROL_MASTER_SCLK_IDLE_MASK));
		break;

	case SSPI_PROT_I2C:
		/*
		 *  Default set for I2C:
		 *  STATE_MACHINE_CONTROL:
		 *  MASTER_CS0_IDLE  - high, MASTER_CS1_IDLE - high,
		 *  MASTER_CS2_IDLE  - high, MASTER_CS3_IDLE - high,
		 *  MASTER_SCLK_IDLE - high, MS_TX0_IDLE     - high,
		 *  MS_TX1_IDLE      - high
		 */

		CHAL_REG_WRITE32(pDevice->base +
			SSPIL_STATE_MACHINE_CONTROL_OFFSET,
			(SSPIL_STATE_MACHINE_CONTROL_MASTER_CS0_IDLE_MASK
			|
			SSPIL_STATE_MACHINE_CONTROL_MASTER_CSOEN0_IDLE_MASK
			|
			SSPIL_STATE_MACHINE_CONTROL_MASTER_SCLK_IDLE_MASK
			|
			SSPIL_STATE_MACHINE_CONTROL_MASTER_SCLKOEN_IDLE_MASK
			| SSPIL_STATE_MACHINE_CONTROL_MS_TX0_IDLE_MASK
			|
			SSPIL_STATE_MACHINE_CONTROL_MS_TXOEN0_IDLE_MASK));
		break;

	case SSPI_PROT_I2S_MODE1:
	case SSPI_PROT_I2S_MODE2:
	case SSPI_PROT_DEFAULT_PCM:
		/*
		 *  Default set for I2S_MODE0:
		 *  STATE_MACHINE_CONTROL:
		 *  MASTER_CS0_IDLE  - low, MASTER_CS1_IDLE - low,
		 *  MASTER_CS2_IDLE  - low, MASTER_CS3_IDLE - low,
		 *  MASTER_SCLK_IDLE - low, MS_TX0_IDLE     - low,
		 *  MS_TX1_IDLE      - low
		 */
		CHAL_REG_WRITE32(pDevice->base +
				 SSPIL_STATE_MACHINE_CONTROL_OFFSET, 0);
		break;

	default:
		return CHAL_SSPI_STATUS_ILLEGAL_PARA;

	}
	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_set_sequence(
*                                  CHAL_HANDLE handle,
*                                  uint32_t seq_idx,
*                                  CHAL_SSPI_PROT_t prot,
*                                  chal_sspi_sec_conf *seq_conf)
*
*  Description: Setup a SSPI sequence
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_set_sequence(CHAL_HANDLE handle,
					  uint32_t seq_idx,
					  CHAL_SSPI_PROT_t prot,
					  chal_sspi_seq_conf_t *seq_conf)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t val;

	if (!handle) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
	}

	if (seq_idx >= pDevice->num_seqs)
			return CHAL_SSPI_STATUS_ILLEGAL_INDEX;

	val = 0;
	val = (seq_conf->opcode << SSPIL_SEQUENCE_0_SEQ0_OPCODE_SHIFT) |
	    (seq_conf->pattern_mode << SSPIL_SEQUENCE_0_SEQ0_PATTERNMODE_SHIFT)
	    | (seq_conf->next_pc << SSPIL_SEQUENCE_0_SEQ0_NEXT_PC_SHIFT) |
	    (seq_conf->rep_cnt << SSPIL_SEQUENCE_0_SEQ0_REPEAT_COUNT_SHIFT)
	    | (seq_conf->frm_sel << SSPIL_SEQUENCE_0_SEQ0_FRAME_DEF_SEL_SHIFT) |
	    (seq_conf->clk_idle << SSPIL_SEQUENCE_0_SEQ0_CLK_IDLE_SHIFT) |
	    (seq_conf->cs_activate << SSPIL_SEQUENCE_0_SEQ0_CS_ACTIVATE_SHIFT)
	    | (seq_conf->cs_deactivate <<
	       SSPIL_SEQUENCE_0_SEQ0_CS_DEACTIVATE_SHIFT) | (seq_conf->tx_enable
							     <<
		SSPIL_SEQUENCE_0_SEQ0_TRANSMIT_SHIFT)
	    | (seq_conf->tx_fifo_sel << SSPIL_SEQUENCE_0_SEQ0_FIFOTX_SEL_SHIFT)
	    | (seq_conf->rx_enable << SSPIL_SEQUENCE_0_SEQ0_RECEIVE_SHIFT) |
	    (seq_conf->rx_fifo_sel << SSPIL_SEQUENCE_0_SEQ0_FIFORX_SEL_SHIFT);

	switch (prot) {
	case SSPI_PROT_I2C:
		/*
		 *  Default sequence setup for SPI_MODE0:
		 *  SEQ0_CLK_IDLE: 0, SEQ0_TXOEN_ACTIVE: 1
		 */
		val |= (seq_conf->tx_enable) ?
		    SSPIL_SEQUENCE_0_SEQ0_TXOEN_ACTIVE_MASK : 0;
		break;

	case SSPI_PROT_SPI_MODE0:
	case SSPI_PROT_SPI_MODE1:
	case SSPI_PROT_SPI_MODE2:
	case SSPI_PROT_SPI_MODE3:
	case SSPI_PROT_I2S_MODE1:
	case SSPI_PROT_I2S_MODE2:
	case SSPI_PROT_DEFAULT_PCM:
		/*
		 *  Default sequence setup for SPI_MODE0:
		 *  SEQ0_CLK_IDLE: 0, SEQ0_TXOEN_ACTIVE: 0
		 */
		break;

	default:
		return CHAL_SSPI_STATUS_ILLEGAL_PARA;

	}
	CHAL_REG_WRITE32(REG_SEQ_ADDR(seq_idx,
				      pDevice->base + SSPIL_SEQUENCE_0_OFFSET),
			 val);

	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_set_task(
*                                  CHAL_HANDLE handle,
*                                  uint32_t task_idx,
*                                  CHAL_SSPI_PROT_t prot,
*                                  chal_sspi_task_conf_t *tk_conf)
*
*  Description: Setup a SSPI task
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_set_task(CHAL_HANDLE handle,
				      uint32_t task_idx,
				      CHAL_SSPI_PROT_t prot,
				      chal_sspi_task_conf_t *tk_conf)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t msb_val = 0, lsb_val = 0;

	if (!handle) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
	}

	if (task_idx >= pDevice->num_tasks)
			return CHAL_SSPI_STATUS_ILLEGAL_INDEX;

	switch (prot) {
	case SSPI_PROT_SPI_MODE0:
	case SSPI_PROT_SPI_MODE1:
	case SSPI_PROT_SPI_MODE2:
	case SSPI_PROT_SPI_MODE3:
		/*
		 *  Default Task setup for SPI
		 *  TASK_DO_ONCE_ONLY 1, TASK0_TXOEN_CS 1, TASK_LSBFIRST 0,
		 *  TASK0_FIFOERROR_STOP 0, TASK0_FIFO_BEH 1,
		 *  TASK0_SKIP 0, TASK0_ENABLE 1
		 */
		msb_val |= SSPIL_TASK0_DESC_MSB_TASK0_FIFO_BEH_MASK |
		    SSPIL_TASK0_DESC_MSB_TASK0_ENABLE_MASK |
		    SSPIL_TASK0_DESC_MSB_TASK0_RESET_REPEATCOUNTER_MASK;

		break;

	case SSPI_PROT_I2C:
		msb_val |= SSPIL_TASK0_DESC_MSB_TASK0_SCLK_IDLE_EXTENSION_MASK |
		    (2 << SSPIL_TASK0_DESC_MSB_TASK0_SCLK_SCLKOEN_SEL_SHIFT) |
		    (2 << SSPIL_TASK0_DESC_MSB_TASK0_TX_TXOEN_SEL_SHIFT) |
		    SSPIL_TASK0_DESC_MSB_TASK0_FIFO_BEH_MASK |
		    SSPIL_TASK0_DESC_MSB_TASK0_RESET_REPEATCOUNTER_MASK |
		    SSPIL_TASK0_DESC_MSB_TASK0_ENABLE_MASK;

		break;

	case SSPI_PROT_I2S_MODE1:
	case SSPI_PROT_I2S_MODE2:
	case SSPI_PROT_DEFAULT_PCM:
		/* TX_TXOEN: 0, SCLK_SCLKOEN: 0 */
		msb_val |= SSPIL_TASK0_DESC_MSB_TASK0_RESET_REPEATCOUNTER_MASK |
		    SSPIL_TASK0_DESC_MSB_TASK0_ENABLE_MASK;

		break;

	default:
		return CHAL_SSPI_STATUS_ILLEGAL_PARA;

	}

	lsb_val |=
	    (tk_conf->loop_cnt << SSPIL_TASK0_DESC_LSB_TASK0_LOOP_SHIFT) |
	    (tk_conf->seq_ptr << SSPIL_TASK0_DESC_LSB_TASK0_SEQ_POINTER_SHIFT)
	    | (tk_conf->init_cond_mask <<
	       SSPIL_TASK0_DESC_LSB_TASK0_INITIAL_CONDITION_SHIFT);
	CHAL_REG_WRITE32(REG_TASK_ADDR
			 (task_idx,
			  pDevice->base + SSPIL_TASK0_DESC_LSB_OFFSET),
			 lsb_val);

	msb_val |= ((tk_conf->wait_before_start) ?
		    SSPIL_TASK0_DESC_MSB_TASK0_WBS_MASK : 0) |
	    ((tk_conf->continuous) ?
	     SSPIL_TASK0_DESC_MSB_TASK0_CONTINUOUS_MASK : 0) |
	    ((tk_conf->not_do_once_only) ?
	     0 : SSPIL_TASK0_DESC_MSB_TASK0_DO_ONCE_ONLY_MASK) |
	    (tk_conf->rx_sel << SSPIL_TASK0_DESC_MSB_TASK0_RX_SELECT_SHIFT) |
	    (tk_conf->tx_sel << SSPIL_TASK0_DESC_MSB_TASK0_TX_SELECT_SHIFT) |
	    (tk_conf->div_sel << SSPIL_TASK0_DESC_MSB_TASK0_CLK_SELECT_SHIFT);
	CHAL_REG_WRITE32(REG_TASK_ADDR(task_idx,
				       pDevice->base +
				       SSPIL_TASK0_DESC_MSB_OFFSET), msb_val);

	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_set_clk_divider(
*                                  CHAL_SSPI_CLK_DIVISOR clk_divisor,
*                                  uint32_t clk_div)
*
*  Description: Set the divider value for the given divisor
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_set_clk_divider(CHAL_HANDLE handle,
					     CHAL_SSPI_CLK_DIVIDER_t
					     clk_divider, uint32_t clk_div)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t val = 0;

	if (!handle) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
	}

	val = CHAL_REG_READ32(pDevice->base + SSPIL_CLKDIV_OFFSET);
	switch (clk_divider) {
	case SSPI_CLK_DIVIDER0:
		FLD_SET(val, clk_div, SSPIL_CLKDIV_CLKDIV_FACTOR_0_SHIFT,
			SSPIL_CLKDIV_CLKDIV_FACTOR_0_MASK);
		break;

	case SSPI_CLK_DIVIDER1:
		FLD_SET(val, clk_div, SSPIL_CLKDIV_CLKDIV_FACTOR_1_SHIFT,
			SSPIL_CLKDIV_CLKDIV_FACTOR_1_MASK);
		break;

	case SSPI_CLK_DIVIDER2:
		FLD_SET(val, clk_div, SSPIL_CLKDIV_CLKDIV_FACTOR_2_SHIFT,
			SSPIL_CLKDIV_CLKDIV_FACTOR_2_MASK);
		break;

	case SSPI_CLK_REF_DIVIDER:
		FLD_SET(val, clk_div, SSPIL_CLKDIV_CLKDIV_FACTOR_REF_SHIFT,
			SSPIL_CLKDIV_CLKDIV_FACTOR_REF_MASK);
		break;

	case SSPI_CLK_SIDETONE_DIVIDER:
	default:
		return CHAL_SSPI_STATUS_ILLEGAL_PARA;
	}

	CHAL_REG_WRITE32(pDevice->base + SSPIL_CLKDIV_OFFSET, val);
	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_get_clk_divider(
*                                  CHAL_SSPI_CLK_DIVIDER_t clk_divider,
*                                  uint32_t *clk_div)
*
*  Description: Get the divider value
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_get_clk_divider(CHAL_HANDLE handle,
					     CHAL_SSPI_CLK_DIVIDER_t
					     clk_divider, uint32_t *clk_div)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t val = 0;

	if (!handle || !clk_div) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_PARA;
	}

	val = CHAL_REG_READ32(pDevice->base + SSPIL_CLKDIV_OFFSET);
	switch (clk_divider) {
	case SSPI_CLK_DIVIDER0:
		*clk_div = FLD_GET(val, SSPIL_CLKDIV_CLKDIV_FACTOR_0_SHIFT,
				   SSPIL_CLKDIV_CLKDIV_FACTOR_0_MASK);
		break;

	case SSPI_CLK_DIVIDER1:
		*clk_div = FLD_GET(val, SSPIL_CLKDIV_CLKDIV_FACTOR_1_SHIFT,
				   SSPIL_CLKDIV_CLKDIV_FACTOR_1_MASK);
		break;

	case SSPI_CLK_DIVIDER2:
		*clk_div = FLD_GET(val, SSPIL_CLKDIV_CLKDIV_FACTOR_2_SHIFT,
				   SSPIL_CLKDIV_CLKDIV_FACTOR_2_MASK);
		break;

	case SSPI_CLK_REF_DIVIDER:
		*clk_div = FLD_GET(val, SSPIL_CLKDIV_CLKDIV_FACTOR_REF_SHIFT,
				   SSPIL_CLKDIV_CLKDIV_FACTOR_REF_MASK);
		break;

	case SSPI_CLK_SIDETONE_DIVIDER:
	default:
		return CHAL_SSPI_STATUS_FAILURE;
	}
	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_set_clk_src_select(
*                                  CHAL_HANDLE handle,
*                                  CHAL_SSPI_CLK_SRC_t clk_src)
*
*  Description: Set SSPI clock source
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_set_clk_src_select(CHAL_HANDLE handle,
						CHAL_SSPI_CLK_SRC_t clk_src)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t val = 0;

	if (!handle) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
	}

	val = CHAL_REG_READ32(pDevice->base + SSPIL_CLKDIV_OFFSET);
	FLD_SET(val, clk_src, SSPIL_CLKDIV_CLK_CHOICE_SHIFT,
		SSPIL_CLKDIV_CLK_CHOICE_MASK);
	CHAL_REG_WRITE32(pDevice->base + SSPIL_CLKDIV_OFFSET, val);
	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_get_clk_src_select(
*                                  CHAL_HANDLE handle,
*                                  uint32_t *clk_src)
*
*  Description: Get the SSPI clock source
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_get_clk_src_select(CHAL_HANDLE handle,
						uint32_t *clk_src)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t val = 0;
	if (!handle || !clk_src) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_PARA;
	}

	val = CHAL_REG_READ32(pDevice->base + SSPIL_CLKDIV_OFFSET);
	*clk_src = FLD_GET(val, SSPIL_CLKDIV_CLK_CHOICE_SHIFT,
			   SSPIL_CLKDIV_CLK_CHOICE_MASK);
	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_set_caph_clk_gen(
*                                            CHAL_HANDLE handle,
*                                            CHAL_SSPI_CAPH_CLK_TRIG_t trig,
*                                            uint32_t num_cycle_edge,
*                                            uint32_t num_clk_edge,
*                                            uint32_t enable)
*
*  Description: Set the CAPH clock generation paramters
*
****************************************************************************/
static CHAL_SSPI_STATUS_t chal_sspi_set_caph_clk_gen(CHAL_HANDLE handle,
					      CHAL_SSPI_CAPH_CLK_TRIG_t trig,
					      uint32_t num_cycle_edge,
					      uint32_t num_clk_edge,
					      uint32_t enable)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t val = 0;

	if (!handle) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
	}

	FLD_SET(val, trig, SSPIL_CLKGEN_CLKGEN_TRIGGER_SEL_SHIFT,
		SSPIL_CLKGEN_CLKGEN_TRIGGER_SEL_MASK);
	FLD_SET(val, num_cycle_edge, SSPIL_CLKGEN_CLKGEN_NB_CYCLE_EDGE_SHIFT,
		SSPIL_CLKGEN_CLKGEN_NB_CYCLE_EDGE_MASK);
	FLD_SET(val, num_clk_edge, SSPIL_CLKGEN_CLKGEN_NB_EDGES_SHIFT,
		SSPIL_CLKGEN_CLKGEN_NB_EDGES_MASK);
	FLD_SET(val, enable, SSPIL_CLKGEN_CLKGEN_RESYNC_ENABLE_SHIFT,
		SSPIL_CLKGEN_CLKGEN_RESYNC_ENABLE_MASK);

	CHAL_REG_WRITE32(pDevice->base + SSPIL_CLKGEN_OFFSET, val);
	return CHAL_SSPI_STATUS_SUCCESS;
}

#if 0
/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_get_caph_clk_gen(
*                                            CHAL_HANDLE handle,
*                                            CHAL_SSPI_CAPH_CLK_TRIG_t *trig,
*                                            uint32_t *num_cycle_edge,
*                                            uint32_t *num_clk_edge,
*                                            uint32_t *enable)
*
*  Description: get the CAPH clock generation paramters
*
****************************************************************************/
static CHAL_SSPI_STATUS_t chal_sspi_get_caph_clk_gen(CHAL_HANDLE handle,
					      CHAL_SSPI_CAPH_CLK_TRIG_t *trig,
					      uint32_t *num_cycle_edge,
					      uint32_t *num_clk_edge,
					      uint32_t *enable)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t val = 0;

	if (!handle) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
	}

	val = CHAL_REG_READ32(pDevice->base + SSPIL_CLKGEN_OFFSET);

	*trig = (CHAL_SSPI_CAPH_CLK_TRIG_t)FLD_GET(val,
		SSPIL_CLKGEN_CLKGEN_TRIGGER_SEL_SHIFT,
		SSPIL_CLKGEN_CLKGEN_TRIGGER_SEL_MASK);
	*num_cycle_edge = FLD_GET(val, SSPIL_CLKGEN_CLKGEN_NB_CYCLE_EDGE_SHIFT,
				  SSPIL_CLKGEN_CLKGEN_NB_CYCLE_EDGE_MASK);
	*num_clk_edge = FLD_GET(val, SSPIL_CLKGEN_CLKGEN_NB_EDGES_SHIFT,
				SSPIL_CLKGEN_CLKGEN_NB_EDGES_MASK);
	*enable = FLD_GET(val, SSPIL_CLKGEN_CLKGEN_RESYNC_ENABLE_SHIFT,
			  SSPIL_CLKGEN_CLKGEN_RESYNC_ENABLE_MASK);
	return CHAL_SSPI_STATUS_SUCCESS;
}
#endif

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_set_caph_clk(
*                                            CHAL_HANDLE handle,
*                                            uint32_t sample_rate,
*                                            uint32_t sample_len)
*
*  Description: Set the CAPH clock generation paramters
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_set_caph_clk(CHAL_HANDLE handle,
							  uint32_t sample_rate,
							  uint32_t sample_len)
{
	uint32_t num_cycle;
	CHAL_SSPI_CAPH_CLK_TRIG_t trig;

	if (!handle) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
	}

	switch (sample_rate) {
	case 4000:
		trig = SSPI_CAPH_CLK_TRIG_4kHz;
		break;
	case 8000:
		trig = SSPI_CAPH_CLK_TRIG_8kHz;
		break;
	case 12000:
		trig = SSPI_CAPH_CLK_TRIG_12kHz;
		break;
	case 16000:
		trig = SSPI_CAPH_CLK_TRIG_16kHz;
		break;
	case 24000:
		trig = SSPI_CAPH_CLK_TRIG_24kHz;
		break;
	case 32000:
		trig = SSPI_CAPH_CLK_TRIG_32kHz;
		break;
	case 48000:
		trig = SSPI_CAPH_CLK_TRIG_48kHz;
		break;
	case 96000:
		trig = SSPI_CAPH_CLK_TRIG_96kHz;
		break;
	case 44100:
		trig = SSPI_CAPH_CLK_TRIG_44DOT1kHz;
		break;
	case 22050:
		trig = SSPI_CAPH_CLK_TRIG_22DOT05kHz;
		break;
	case 11025:
		trig = SSPI_CAPH_CLK_TRIG_11DOT025kHz;
		break;
	default:
		return CHAL_SSPI_STATUS_ILLEGAL_PARA;
	}

	if (trig == SSPI_CAPH_CLK_TRIG_44DOT1kHz) {
		/*
		 *  Since the SSPI issue, to get the correct 44.1K sample rate,
		 *  num_cycle need be set 1 less than the normal value.
		 */
		num_cycle = CHAL_SSPI_CAPH_CLOCK_RATE /
				(sample_rate * sample_len) - 1;
	} else
		num_cycle = CHAL_SSPI_CAPH_CLOCK_RATE /
				(sample_rate * sample_len);

	chal_sspi_set_caph_clk_gen(handle, trig,
				   num_cycle / 2 - 1,
				   sample_len * 2 - 1, 1);

	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_set_fifo_size(
*                                  CHAL_HANDLE handle,
*                                  CHAL_SSPI_FIFO_ID_t fifo_id,
*                                  CHAL_SSPI_FIFO_SIZE_t fifo_size)
*
*  Description: Set SSPI FIFI size for the given FIFO
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_set_fifo_size(CHAL_HANDLE handle,
					   CHAL_SSPI_FIFO_ID_t fifo_id,
					   CHAL_SSPI_FIFO_SIZE_t fifo_size)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t val = 0;

	if (!handle) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
	}

	val = CHAL_REG_READ32(pDevice->base + SSPIL_FIFO_SHARING_OFFSET);
	switch (fifo_id) {
	case SSPI_FIFO_ID_TX0:
		FLD_SET(val, fifo_size, SSPIL_FIFO_SHARING_FIFO_TX_0_SIZE_SHIFT,
			SSPIL_FIFO_SHARING_FIFO_TX_0_SIZE_MASK);
		break;

	case SSPI_FIFO_ID_TX1:
		FLD_SET(val, fifo_size, SSPIL_FIFO_SHARING_FIFO_TX_1_SIZE_SHIFT,
			SSPIL_FIFO_SHARING_FIFO_TX_1_SIZE_MASK);
		break;

	case SSPI_FIFO_ID_TX2:
		FLD_SET(val, fifo_size, SSPIL_FIFO_SHARING_FIFO_TX_2_SIZE_SHIFT,
			SSPIL_FIFO_SHARING_FIFO_TX_2_SIZE_MASK);
		break;

	case SSPI_FIFO_ID_TX3:
		FLD_SET(val, fifo_size, SSPIL_FIFO_SHARING_FIFO_TX_3_SIZE_SHIFT,
			SSPIL_FIFO_SHARING_FIFO_TX_3_SIZE_MASK);
		break;

	case SSPI_FIFO_ID_RX0:
		FLD_SET(val, fifo_size, SSPIL_FIFO_SHARING_FIFO_RX_0_SIZE_SHIFT,
			SSPIL_FIFO_SHARING_FIFO_RX_0_SIZE_MASK);
		break;

	case SSPI_FIFO_ID_RX1:
		FLD_SET(val, fifo_size, SSPIL_FIFO_SHARING_FIFO_RX_1_SIZE_SHIFT,
			SSPIL_FIFO_SHARING_FIFO_RX_1_SIZE_MASK);
		break;

	case SSPI_FIFO_ID_RX2:
		FLD_SET(val, fifo_size, SSPIL_FIFO_SHARING_FIFO_RX_2_SIZE_SHIFT,
			SSPIL_FIFO_SHARING_FIFO_RX_2_SIZE_MASK);
		break;

	case SSPI_FIFO_ID_RX3:
		FLD_SET(val, fifo_size, SSPIL_FIFO_SHARING_FIFO_RX_3_SIZE_SHIFT,
			SSPIL_FIFO_SHARING_FIFO_RX_3_SIZE_MASK);
		break;

	default:
		return CHAL_SSPI_STATUS_ILLEGAL_FIFO;
	}

	CHAL_REG_WRITE32(pDevice->base + SSPIL_FIFO_SHARING_OFFSET, val);
	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_get_fifo_size(
*                                  CHAL_HANDLE handle,
*                                  CHAL_SSPI_FIFO_ID_t fifo_id,
*                                  CHAL_SSPI_FIFO_SIZE_t *fifo_size)
*
*  Description: Get the FIFO size
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_get_fifo_size(CHAL_HANDLE handle,
					   CHAL_SSPI_FIFO_ID_t fifo_id,
					   uint32_t *fifo_size)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t val = 0;

	if (!handle || !fifo_size) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_PARA;
	}

	val = CHAL_REG_READ32(pDevice->base + SSPIL_FIFO_SHARING_OFFSET);
	switch (fifo_id) {
	case SSPI_FIFO_ID_TX0:
		*fifo_size =
		    FLD_GET(val, SSPIL_FIFO_SHARING_FIFO_TX_0_SIZE_SHIFT,
			    SSPIL_FIFO_SHARING_FIFO_TX_0_SIZE_MASK);
		break;

	case SSPI_FIFO_ID_TX1:
		*fifo_size =
		    FLD_GET(val, SSPIL_FIFO_SHARING_FIFO_TX_1_SIZE_SHIFT,
			    SSPIL_FIFO_SHARING_FIFO_TX_1_SIZE_MASK);
		break;

	case SSPI_FIFO_ID_TX2:
		*fifo_size =
		    FLD_GET(val, SSPIL_FIFO_SHARING_FIFO_TX_2_SIZE_SHIFT,
			    SSPIL_FIFO_SHARING_FIFO_TX_2_SIZE_MASK);
		break;

	case SSPI_FIFO_ID_TX3:
		*fifo_size =
		    FLD_GET(val, SSPIL_FIFO_SHARING_FIFO_TX_3_SIZE_SHIFT,
			    SSPIL_FIFO_SHARING_FIFO_TX_3_SIZE_MASK);
		break;

	case SSPI_FIFO_ID_RX0:
		*fifo_size =
		    FLD_GET(val, SSPIL_FIFO_SHARING_FIFO_RX_0_SIZE_SHIFT,
			    SSPIL_FIFO_SHARING_FIFO_RX_0_SIZE_MASK);
		break;

	case SSPI_FIFO_ID_RX1:
		*fifo_size =
		    FLD_GET(val, SSPIL_FIFO_SHARING_FIFO_RX_1_SIZE_SHIFT,
			    SSPIL_FIFO_SHARING_FIFO_RX_1_SIZE_MASK);
		break;

	case SSPI_FIFO_ID_RX2:
		*fifo_size =
		    FLD_GET(val, SSPIL_FIFO_SHARING_FIFO_RX_2_SIZE_SHIFT,
			    SSPIL_FIFO_SHARING_FIFO_RX_2_SIZE_MASK);
		break;

	case SSPI_FIFO_ID_RX3:
		*fifo_size =
		    FLD_GET(val, SSPIL_FIFO_SHARING_FIFO_RX_3_SIZE_SHIFT,
			    SSPIL_FIFO_SHARING_FIFO_RX_3_SIZE_MASK);
		break;

	default:
		return CHAL_SSPI_STATUS_ILLEGAL_FIFO;
	}
	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_set_fifo_pack(
*                                  CHAL_HANDLE handle,
*                                  CHAL_SSPI_FIFO_ID_t fifo_id,
*                                  CHAL_SSPI_FIFO_DATA_PACK_t fifo_pack)
*
*  Description: Set the FIFO data packing type
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_set_fifo_pack(CHAL_HANDLE handle,
					   CHAL_SSPI_FIFO_ID_t fifo_id,
					   CHAL_SSPI_FIFO_DATA_PACK_t fifo_pack)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t val = 0, reg = 0;

	if (!handle) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
	}

	if (fifo_id >= SSPI_FIFO_ID_TX0) {
		reg = REG_FIFOTX_CTL_ADDR(fifo_id - SSPI_FIFO_ID_TX0,
					  pDevice->base +
					  SSPIL_FIFOTX_0_CONTROL_OFFSET);
		val = CHAL_REG_READ32(reg);
		FLD_SET(val, fifo_pack,
			SSPIL_FIFOTX_0_CONTROL_FIFOTX0_DATA_PACKING_SHIFT,
			SSPIL_FIFOTX_0_CONTROL_FIFOTX0_DATA_PACKING_MASK);
	} else {
		reg = REG_FIFORX_CTL_ADDR(fifo_id,
					  pDevice->base +
					  SSPIL_FIFORX_0_CONTROL_OFFSET);
		val = CHAL_REG_READ32(reg);
		FLD_SET(val, fifo_pack,
			SSPIL_FIFORX_0_CONTROL_FIFORX0_DATA_PACKING_SHIFT,
			SSPIL_FIFORX_0_CONTROL_FIFORX0_DATA_PACKING_MASK);
	}
	CHAL_REG_WRITE32(reg, val);

	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_get_fifo_pack(
*                                  CHAL_HANDLE handle,
*                                  CHAL_SSPI_FIFO_ID_t fifo_id,
*                                  CHAL_SSPI_FIFO_DATA_PACK_t *fifo_pack)
*
*  Description: Get the FIFO data packing type
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_get_fifo_pack(CHAL_HANDLE handle,
					   CHAL_SSPI_FIFO_ID_t fifo_id,
					   uint32_t *fifo_pack)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t val = 0;

	if (!handle || !fifo_pack) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_PARA;
	}

	if (fifo_id >= SSPI_FIFO_ID_TX0) {
		val =
		    CHAL_REG_READ32(REG_FIFOTX_CTL_ADDR
				    (fifo_id - SSPI_FIFO_ID_TX0,
				     pDevice->base +
				     SSPIL_FIFOTX_0_CONTROL_OFFSET));
		*fifo_pack =
		    FLD_GET(val,
			    SSPIL_FIFOTX_0_CONTROL_FIFOTX0_DATA_PACKING_SHIFT,
			    SSPIL_FIFOTX_0_CONTROL_FIFOTX0_DATA_PACKING_MASK);
	} else {
		val = CHAL_REG_READ32(REG_FIFORX_CTL_ADDR(fifo_id,
						pDevice->base +
						SSPIL_FIFORX_0_CONTROL_OFFSET));
		*fifo_pack =
		    FLD_GET(val,
			    SSPIL_FIFORX_0_CONTROL_FIFORX0_DATA_PACKING_SHIFT,
			    SSPIL_FIFORX_0_CONTROL_FIFORX0_DATA_PACKING_MASK);
	}

	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_set_fifo_endian(
*                                  CHAL_HANDLE handle,
*                                  CHAL_SSPI_FIFO_ID_t fifo_id,
*                                  CHAL_SSPI_FIFO_ENDIAN_t fifo_endian)
*
*  Description: Set the FIFO data endianess
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_set_fifo_endian(CHAL_HANDLE handle,
					     CHAL_SSPI_FIFO_ID_t fifo_id,
					     CHAL_SSPI_FIFO_ENDIAN_t
					     fifo_endian)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t val = 0, reg = 0;

	if (!handle) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
	}

	if (fifo_id >= SSPI_FIFO_ID_TX0) {
		reg = REG_FIFOTX_CTL_ADDR(fifo_id - SSPI_FIFO_ID_TX0,
					  pDevice->base +
					  SSPIL_FIFOTX_0_CONTROL_OFFSET);
		val = CHAL_REG_READ32(reg);
		FLD_SET(val, fifo_endian,
			SSPIL_FIFOTX_0_CONTROL_FIFOTX0_ENDIANESS_SHIFT,
			SSPIL_FIFOTX_0_CONTROL_FIFOTX0_ENDIANESS_MASK);
	} else {
		reg = REG_FIFORX_CTL_ADDR(fifo_id,
					  pDevice->base +
					  SSPIL_FIFORX_0_CONTROL_OFFSET);
		val = CHAL_REG_READ32(reg);
		FLD_SET(val, fifo_endian,
			SSPIL_FIFORX_0_CONTROL_FIFORX0_ENDIANESS_SHIFT,
			SSPIL_FIFORX_0_CONTROL_FIFORX0_ENDIANESS_MASK);
	}
	CHAL_REG_WRITE32(reg, val);

	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_get_fifo_endian(
*                                  CHAL_HANDLE handle,
*                                  CHAL_SSPI_FIFO_ID_t fifo_id,
*                                  CHAL_SSPI_FIFO_ENDIAN_t *fifo_endian)
*
*  Description: Get the FIFO data endianess
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_get_fifo_endian(CHAL_HANDLE handle,
					     CHAL_SSPI_FIFO_ID_t fifo_id,
					     uint32_t *fifo_endian)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t val = 0;

	if (!handle || !fifo_endian) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_PARA;
	}

	if (fifo_id >= SSPI_FIFO_ID_TX0) {
		val =
		    CHAL_REG_READ32(REG_FIFOTX_CTL_ADDR
				    (fifo_id - SSPI_FIFO_ID_TX0,
				     pDevice->base +
				     SSPIL_FIFOTX_0_CONTROL_OFFSET));
		*fifo_endian =
		    FLD_GET(val, SSPIL_FIFOTX_0_CONTROL_FIFOTX0_ENDIANESS_SHIFT,
			    SSPIL_FIFOTX_0_CONTROL_FIFOTX0_ENDIANESS_MASK);
	} else {
		val = CHAL_REG_READ32(REG_FIFORX_CTL_ADDR(fifo_id,
						pDevice->base +
						SSPIL_FIFORX_0_CONTROL_OFFSET));
		*fifo_endian =
		    FLD_GET(val, SSPIL_FIFORX_0_CONTROL_FIFORX0_ENDIANESS_SHIFT,
			    SSPIL_FIFORX_0_CONTROL_FIFORX0_ENDIANESS_MASK);
	}

	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_set_fifo_threshold(
*                                  CHAL_HANDLE handle,
*                                  CHAL_SSPI_FIFO_ID_t fifo_id,
*                                  uint32_t fifo_thres)
*
*  Description: Set the FIFI threshold before starting any task
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_set_fifo_threshold(CHAL_HANDLE handle,
						CHAL_SSPI_FIFO_ID_t fifo_id,
						uint32_t fifo_thres)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t val = 0, reg = 0;

	if (!handle) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
	}

	if (fifo_id >= SSPI_FIFO_ID_TX0) {
		reg = REG_FIFOTX_THOLD_ADDR(fifo_id - SSPI_FIFO_ID_TX0,
					    pDevice->base +
					    SSPIL_FIFOTX_0_THRESHOLD_OFFSET);
		val = CHAL_REG_READ32(reg);
		FLD_SET(val, fifo_thres,
			SSPIL_FIFOTX_0_THRESHOLD_FIFOTX0_TRESHOLD_SHIFT,
			SSPIL_FIFOTX_0_THRESHOLD_FIFOTX0_TRESHOLD_MASK);
	} else {
		reg = REG_FIFORX_THOLD_ADDR(fifo_id,
					    pDevice->base +
					    SSPIL_FIFORX_0_THRESHOLD_OFFSET);
		val = CHAL_REG_READ32(reg);
		FLD_SET(val, fifo_thres,
			SSPIL_FIFORX_0_THRESHOLD_FIFORX0_TRESHOLD_SHIFT,
			SSPIL_FIFORX_0_THRESHOLD_FIFORX0_TRESHOLD_MASK);
	}
	CHAL_REG_WRITE32(reg, val);

	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_get_fifo_threshold(
*                                  CHAL_HANDLE handle,
*                                  CHAL_SSPI_FIFO_ID_t fifo_id,
*                                  uint32_t *fifo_thres)
*
*  Description: Get the FIFO threshold
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_get_fifo_threshold(CHAL_HANDLE handle,
						CHAL_SSPI_FIFO_ID_t fifo_id,
						uint32_t *fifo_thres)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t val = 0;

	if (!handle || !fifo_thres) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_PARA;
	}

	if (fifo_id >= SSPI_FIFO_ID_TX0) {
		val =
		    CHAL_REG_READ32(REG_FIFOTX_THOLD_ADDR
				    (fifo_id - SSPI_FIFO_ID_TX0,
				     pDevice->base +
				     SSPIL_FIFOTX_0_THRESHOLD_OFFSET));
		*fifo_thres =
		    FLD_GET(val,
			    SSPIL_FIFOTX_0_THRESHOLD_FIFOTX0_TRESHOLD_SHIFT,
			    SSPIL_FIFOTX_0_THRESHOLD_FIFOTX0_TRESHOLD_MASK);
	} else {
		val = CHAL_REG_READ32(REG_FIFORX_THOLD_ADDR(fifo_id,
					pDevice->base +
					SSPIL_FIFORX_0_THRESHOLD_OFFSET));
		*fifo_thres =
		    FLD_GET(val,
			    SSPIL_FIFORX_0_THRESHOLD_FIFORX0_TRESHOLD_SHIFT,
			    SSPIL_FIFORX_0_THRESHOLD_FIFORX0_TRESHOLD_MASK);
	}

	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_set_fifo_data_size(
*                                  CHAL_HANDLE handle,
*                                  CHAL_SSPI_FIFO_ID_t fifo_id,
*                                  CHAL_SSPI_FIFO_DATA_SIZE_t fifo_size)
*
*  Description: Set the FIFO data write size
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_set_fifo_data_size(CHAL_HANDLE handle,
						CHAL_SSPI_FIFO_ID_t fifo_id,
						CHAL_SSPI_FIFO_DATA_SIZE_t
						fifo_size)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	cUInt32 val = 0, reg = 0;

	if (!handle) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
	}

	if (fifo_id >= SSPI_FIFO_ID_TX0) {
		reg = REG_FIFOTX_CTL_ADDR(fifo_id - SSPI_FIFO_ID_TX0,
					  pDevice->base +
					  SSPIL_FIFOTX_0_CONTROL_OFFSET);
		val = CHAL_REG_READ32(reg);
		FLD_SET(val, fifo_size,
			SSPIL_FIFOTX_0_CONTROL_FIFOTX0_DATA_WRITE_SIZE_SHIFT,
			SSPIL_FIFOTX_0_CONTROL_FIFOTX0_DATA_WRITE_SIZE_MASK);
	} else {
		reg = REG_FIFORX_CTL_ADDR(fifo_id,
					  pDevice->base +
					  SSPIL_FIFORX_0_CONTROL_OFFSET);
		val = CHAL_REG_READ32(reg);
		FLD_SET(val, fifo_size,
			SSPIL_FIFORX_0_CONTROL_FIFORX0_DATA_READ_SIZE_SHIFT,
			SSPIL_FIFORX_0_CONTROL_FIFORX0_DATA_READ_SIZE_MASK);
	}
	CHAL_REG_WRITE32(reg, val);

	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_get_fifo_data_size(
*                                  CHAL_HANDLE handle,
*                                  CHAL_SSPI_FIFO_ID_t fifo_id,
*                                  uint32_t *fifo_pack)
*
*  Description: Get the FIFO data write size
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_get_fifo_data_size(CHAL_HANDLE handle,
						CHAL_SSPI_FIFO_ID_t fifo_id,
						uint32_t *fifo_size)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t val = 0;

	if (!handle || !fifo_size) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_PARA;
	}

	if (fifo_id >= SSPI_FIFO_ID_TX0) {
		val =
		    CHAL_REG_READ32(REG_FIFOTX_CTL_ADDR
				    (fifo_id - SSPI_FIFO_ID_TX0,
				     pDevice->base +
				     SSPIL_FIFOTX_0_CONTROL_OFFSET));
		*fifo_size =
		    FLD_GET(val,
			SSPIL_FIFOTX_0_CONTROL_FIFOTX0_DATA_WRITE_SIZE_SHIFT,
			SSPIL_FIFOTX_0_CONTROL_FIFOTX0_DATA_WRITE_SIZE_MASK);
	} else {
		val = CHAL_REG_READ32(REG_FIFORX_CTL_ADDR(fifo_id,
						pDevice->base +
						SSPIL_FIFORX_0_CONTROL_OFFSET));
		*fifo_size =
		    FLD_GET(val,
			    SSPIL_FIFORX_0_CONTROL_FIFORX0_DATA_READ_SIZE_SHIFT,
			    SSPIL_FIFORX_0_CONTROL_FIFORX0_DATA_READ_SIZE_MASK);
	}

	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_fifo_reset(
*                                  CHAL_HANDLE handle,
*                                  CHAL_SSPI_FIFO_ID_t fifo_id)
*
*  Description: Reset the FIFO
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_fifo_reset(CHAL_HANDLE handle,
					CHAL_SSPI_FIFO_ID_t fifo_id)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t val = 0, reg = 0;

	if (!handle) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
	}

	if (fifo_id >= SSPI_FIFO_ID_TX0) {
		reg = REG_FIFOTX_CTL_ADDR(fifo_id - SSPI_FIFO_ID_TX0,
					  pDevice->base +
					  SSPIL_FIFOTX_0_CONTROL_OFFSET);
		val =
		    CHAL_REG_READ32(reg) |
		    SSPIL_FIFOTX_0_CONTROL_FIFOTX0_RESET_MASK;
		CHAL_REG_WRITE32(reg, val);
		/* poll until reset bit goes back to 0 */
		while (CHAL_REG_READ32(reg) &
			SSPIL_FIFOTX_0_CONTROL_FIFOTX0_RESET_MASK)
			;
	} else {
		reg = REG_FIFORX_CTL_ADDR(fifo_id,
					  pDevice->base +
					  SSPIL_FIFORX_0_CONTROL_OFFSET);
		val =
		    CHAL_REG_READ32(reg) |
		    SSPIL_FIFORX_0_CONTROL_FIFORX0_RESET_MASK;
		CHAL_REG_WRITE32(reg, val);
		/* poll until reset bit goes back to 0 */
		while (CHAL_REG_READ32(reg) &
			SSPIL_FIFORX_0_CONTROL_FIFORX0_RESET_MASK)
			;
	}

	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_set_fifo_repeat_count(
*                                  CHAL_HANDLE handle,
*                                  CHAL_SSPI_FIFO_ID_t fifo_id,
*                                  CUint8 fifo_rcnt)
*
*  Description: Setup a repeat count to READ/WRITE the FIFO data multiple times
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_set_fifo_repeat_count(CHAL_HANDLE handle,
						   CHAL_SSPI_FIFO_ID_t fifo_id,
						   uint8_t fifo_rcnt)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t val = 0, reg = 0;

	if (!handle) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
	}

	if (fifo_id >= SSPI_FIFO_ID_TX0) {
		reg = REG_FIFOTX_CTL_ADDR(fifo_id - SSPI_FIFO_ID_TX0,
					  pDevice->base +
					  SSPIL_FIFOTX_0_CONTROL_OFFSET);
		val = CHAL_REG_READ32(reg);
		FLD_SET(val, fifo_rcnt,
			SSPIL_FIFOTX_0_CONTROL_FIFOTX0_REPEAT_READ_SHIFT,
			SSPIL_FIFOTX_0_CONTROL_FIFOTX0_REPEAT_READ_MASK);
	} else {
		reg = REG_FIFORX_CTL_ADDR(fifo_id,
					  pDevice->base +
					  SSPIL_FIFORX_0_CONTROL_OFFSET);
		val = CHAL_REG_READ32(reg);
		FLD_SET(val, fifo_rcnt,
			SSPIL_FIFORX_0_CONTROL_FIFORX0_REPEAT_WRITE_SHIFT,
			SSPIL_FIFORX_0_CONTROL_FIFORX0_REPEAT_WRITE_MASK);
	}
	CHAL_REG_WRITE32(reg, val);

	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_get_fifo_repeat_count(
*                                  CHAL_HANDLE handle,
*                                  CHAL_SSPI_FIFO_ID_t fifo_id,
*                                  CHAL_SSPI_FIFO_SIZE_t fifo_rcnt)
*
*  Description: Get the repeat count that READ/WRITE the FIFO data multiple time
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_get_fifo_repeat_count(CHAL_HANDLE handle,
						   CHAL_SSPI_FIFO_ID_t fifo_id,
						   uint8_t *fifo_rcnt)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t val = 0;

	if (!handle || !fifo_rcnt) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_PARA;
	}

	if (fifo_id >= SSPI_FIFO_ID_TX0) {
		val =
		    CHAL_REG_READ32(REG_FIFOTX_CTL_ADDR
				    (fifo_id - SSPI_FIFO_ID_TX0,
				     pDevice->base +
				     SSPIL_FIFOTX_0_CONTROL_OFFSET));
		*fifo_rcnt =
		    FLD_GET(val,
			    SSPIL_FIFOTX_0_CONTROL_FIFOTX0_REPEAT_READ_SHIFT,
			    SSPIL_FIFOTX_0_CONTROL_FIFOTX0_REPEAT_READ_MASK);
	} else {
		val = CHAL_REG_READ32(REG_FIFORX_CTL_ADDR(fifo_id,
						pDevice->base +
						SSPIL_FIFORX_0_CONTROL_OFFSET));
		*fifo_rcnt =
		    FLD_GET(val,
			    SSPIL_FIFORX_0_CONTROL_FIFORX0_REPEAT_WRITE_SHIFT,
			    SSPIL_FIFORX_0_CONTROL_FIFORX0_REPEAT_WRITE_MASK);
	}

	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_get_fifo_level(
*                                  CHAL_HANDLE handle,
*                                  CHAL_SSPI_FIFO_ID_t fifo_id,
*                                  uint16_t fifo_level)
*
*  Description: For Rx FIFOs, get # of bytes currently in the specified FIFO
*               For Tx FIFOs, get # of free bytes currently in the specified
*               FIFO
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_get_fifo_level(CHAL_HANDLE handle,
					    CHAL_SSPI_FIFO_ID_t fifo_id,
					    uint16_t *fifo_level)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t val = 0;

	if (!handle || !fifo_level) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_PARA;
	}

	if (fifo_id >= SSPI_FIFO_ID_TX0) {
		val =
		    CHAL_REG_READ32(REG_FIFOTX_CTL_ADDR
				    (fifo_id - SSPI_FIFO_ID_TX0,
				     pDevice->base +
				     SSPIL_FIFOTX_0_CONTROL_OFFSET));
		*fifo_level =
		    FLD_GET(val, SSPIL_FIFOTX_0_CONTROL_FIFOTX0_LEVEL_SHIFT,
			    SSPIL_FIFOTX_0_CONTROL_FIFOTX0_LEVEL_MASK);
	} else {
		val = CHAL_REG_READ32(REG_FIFORX_CTL_ADDR(fifo_id,
						pDevice->base +
						SSPIL_FIFORX_0_CONTROL_OFFSET));

		*fifo_level = FLD_GET(val,
				SSPIL_FIFORX_0_CONTROL_FIFORX0_LEVEL_SHIFT,
				SSPIL_FIFORX_0_CONTROL_FIFORX0_LEVEL_MASK);
	}

	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_check_fifo_full(
*                                  CHAL_HANDLE handle,
*                                  CHAL_SSPI_FIFO_ID_t fifo_id)
*
*  Description: Check if the specified FIFO is full or not.
*
****************************************************************************/
_Bool chal_sspi_check_fifo_full(CHAL_HANDLE handle, CHAL_SSPI_FIFO_ID_t fifo_id)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t val = 0;

	if (fifo_id >= SSPI_FIFO_ID_TX0) {
		val =
		    CHAL_REG_READ32(REG_FIFOTX_CTL_ADDR
				    (fifo_id - SSPI_FIFO_ID_TX0,
				     pDevice->base +
				     SSPIL_FIFOTX_0_CONTROL_OFFSET));
		val &= SSPIL_FIFOTX_0_CONTROL_FIFOTX0_FULL_MASK;
	} else {
		val = CHAL_REG_READ32(REG_FIFORX_CTL_ADDR(fifo_id,
						pDevice->base +
						SSPIL_FIFORX_0_CONTROL_OFFSET));

		val &= SSPIL_FIFORX_0_CONTROL_FIFORX0_FULL_MASK;
	}
	return val ? 1 : 0;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_check_fifo_empty(
*                                  CHAL_HANDLE handle,
*                                  CHAL_SSPI_FIFO_ID_t fifo_id)
*
*  Description: Check if the specified FIFO is empty or not.
*
****************************************************************************/
_Bool chal_sspi_check_fifo_empty(CHAL_HANDLE handle,
				 CHAL_SSPI_FIFO_ID_t fifo_id)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t val = 0;

	if (fifo_id >= SSPI_FIFO_ID_TX0) {
		val =
		    CHAL_REG_READ32(REG_FIFOTX_CTL_ADDR
				    (fifo_id - SSPI_FIFO_ID_TX0,
				     pDevice->base +
				     SSPIL_FIFOTX_0_CONTROL_OFFSET));
		val &= SSPIL_FIFOTX_0_CONTROL_FIFOTX0_EMPTY_MASK;
	} else {
		val = CHAL_REG_READ32(REG_FIFORX_CTL_ADDR(fifo_id,
						pDevice->base +
						SSPIL_FIFORX_0_CONTROL_OFFSET));

		val &= SSPIL_FIFORX_0_CONTROL_FIFORX0_EMPTY_MASK;
	}
	return val ? 1 : 0;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_set_fifo_pio_threshhold(
*                                  CHAL_HANDLE handle,
*                                  CHAL_SSPI_FIFO_ID_t fifo_id,
*                                  uint32_t start_thres,
*                                  uint32_t stop_thres)
*
*  Description: Setup PIO start and stop threshhold
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_set_fifo_pio_threshhold(CHAL_HANDLE handle,
						     CHAL_SSPI_FIFO_ID_t
						     fifo_id,
						     uint32_t start_thres,
						     uint32_t stop_thres)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t val = 0, reg = 0;

	if (!handle) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
	}

	if (fifo_id >= SSPI_FIFO_ID_TX0) {
		reg = REG_FIFOTX_PIO_THOLD_ADDR(fifo_id - SSPI_FIFO_ID_TX0,
					pDevice->base +
					SSPIL_FIFOTX_0_PIO_THRESHOLD_OFFSET);

		FLD_SET(val, start_thres,
		SSPIL_FIFOTX_0_PIO_THRESHOLD_FIFOTX0_PIO_START_THRESH_SHIFT,
		SSPIL_FIFOTX_0_PIO_THRESHOLD_FIFOTX0_PIO_START_THRESH_MASK);
		FLD_SET(val, stop_thres,
		SSPIL_FIFOTX_0_PIO_THRESHOLD_FIFOTX0_PIO_STOP_THRESH_SHIFT,
		SSPIL_FIFOTX_0_PIO_THRESHOLD_FIFOTX0_PIO_STOP_THRESH_MASK);
		FLD_SET(val, stop_thres,
		SSPIL_FIFOTX_0_PIO_THRESHOLD_FIFOTX0_PIO_STOP_ERR_THRESH_SHIFT,
		SSPIL_FIFOTX_0_PIO_THRESHOLD_FIFOTX0_PIO_STOP_ERR_THRESH_MASK);
	} else {
		reg = REG_FIFORX_PIO_THOLD_ADDR(fifo_id - SSPI_FIFO_ID_RX0,
					pDevice->base +
					SSPIL_FIFORX_0_PIO_THRESHOLD_OFFSET);

		FLD_SET(val, start_thres,
		SSPIL_FIFORX_0_PIO_THRESHOLD_FIFORX0_PIO_START_THRESH_SHIFT,
		SSPIL_FIFORX_0_PIO_THRESHOLD_FIFORX0_PIO_START_THRESH_MASK);
		FLD_SET(val, stop_thres,
		SSPIL_FIFORX_0_PIO_THRESHOLD_FIFORX0_PIO_STOP_THRESH_SHIFT,
		SSPIL_FIFORX_0_PIO_THRESHOLD_FIFORX0_PIO_STOP_THRESH_MASK);
		FLD_SET(val, stop_thres,
		SSPIL_FIFORX_0_PIO_THRESHOLD_FIFORX0_PIO_STOP_ERR_THRESH_SHIFT,
		SSPIL_FIFORX_0_PIO_THRESHOLD_FIFORX0_PIO_STOP_ERR_THRESH_MASK);
	}

	CHAL_REG_WRITE32(reg, val);
	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_get_fifo_pio_threshhold(
*                                  CHAL_HANDLE handle,
*                                  CHAL_SSPI_FIFO_ID_t fifo_id,
*                                  uint32_t *start_thres,
*                                  uint32_t *stop_thres)
*
*  Description: Get PIO start and stop threshhold
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_get_fifo_pio_threshhold(CHAL_HANDLE handle,
						     CHAL_SSPI_FIFO_ID_t
						     fifo_id,
						     uint32_t *start_thres,
						     uint32_t *stop_thres)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t val = 0, reg = 0;

	if (!handle) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
	}

	if (fifo_id >= SSPI_FIFO_ID_TX0) {
		reg = REG_FIFOTX_PIO_THOLD_ADDR(fifo_id - SSPI_FIFO_ID_TX0,
					pDevice->base +
					SSPIL_FIFOTX_0_PIO_THRESHOLD_OFFSET);

		val = CHAL_REG_READ32(reg);
		*start_thres = FLD_GET(val,
		SSPIL_FIFOTX_0_PIO_THRESHOLD_FIFOTX0_PIO_START_THRESH_SHIFT,
		SSPIL_FIFOTX_0_PIO_THRESHOLD_FIFOTX0_PIO_START_THRESH_MASK);

		*stop_thres = FLD_GET(val,
		SSPIL_FIFOTX_0_PIO_THRESHOLD_FIFOTX0_PIO_STOP_THRESH_SHIFT,
		SSPIL_FIFOTX_0_PIO_THRESHOLD_FIFOTX0_PIO_STOP_THRESH_MASK);
	} else {
		reg = REG_FIFORX_PIO_THOLD_ADDR(fifo_id - SSPI_FIFO_ID_RX0,
					pDevice->base +
					SSPIL_FIFORX_0_PIO_THRESHOLD_OFFSET);

		val = CHAL_REG_READ32(reg);
		*start_thres = FLD_GET(val,
		SSPIL_FIFORX_0_PIO_THRESHOLD_FIFORX0_PIO_START_THRESH_SHIFT,
		SSPIL_FIFORX_0_PIO_THRESHOLD_FIFORX0_PIO_START_THRESH_MASK);

		*stop_thres = FLD_GET(val,
		SSPIL_FIFORX_0_PIO_THRESHOLD_FIFORX0_PIO_STOP_THRESH_SHIFT,
		SSPIL_FIFORX_0_PIO_THRESHOLD_FIFORX0_PIO_STOP_THRESH_MASK);
	}

	CHAL_REG_WRITE32(reg, val);
	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_enable_fifo_pio_start_stop_intr(
*                                  CHAL_HANDLE handle,
*                                  CHAL_SSPI_FIFO_ID_t fifo_id,
*                                  uint32_t start_en,
*                                  uint32_t stop_en)
*
*  Description: Enable/Disable PIO start and stop interrupts
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_enable_fifo_pio_start_stop_intr(CHAL_HANDLE handle,
							     CHAL_SSPI_FIFO_ID_t
							     fifo_id,
							     uint32_t start_en,
							     uint32_t stop_en)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t val = 0, reg = 0;

	if (!handle) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
	}

	if (fifo_id >= SSPI_FIFO_ID_TX0) {
		reg = REG_FIFOTX_CTL_ADDR(fifo_id - SSPI_FIFO_ID_TX0,
					  pDevice->base +
					  SSPIL_FIFOTX_0_CONTROL_OFFSET);
		val = CHAL_REG_READ32(reg);
		FLD_SET(val, start_en,
			SSPIL_FIFOTX_0_CONTROL_FIFOTX0_PIO_START_ENABLE_SHIFT,
			SSPIL_FIFOTX_0_CONTROL_FIFOTX0_PIO_START_ENABLE_MASK);
		FLD_SET(val, stop_en,
			SSPIL_FIFOTX_0_CONTROL_FIFOTX0_PIO_STOP_ENABLE_SHIFT,
			SSPIL_FIFOTX_0_CONTROL_FIFOTX0_PIO_STOP_ENABLE_MASK);
	} else {
		reg = REG_FIFORX_CTL_ADDR(fifo_id,
					  pDevice->base +
					  SSPIL_FIFORX_0_CONTROL_OFFSET);
		val = CHAL_REG_READ32(reg);
		FLD_SET(val, start_en,
			SSPIL_FIFORX_0_CONTROL_FIFORX0_PIO_START_ENABLE_SHIFT,
			SSPIL_FIFORX_0_CONTROL_FIFORX0_PIO_START_ENABLE_MASK);
		FLD_SET(val, stop_en,
			SSPIL_FIFORX_0_CONTROL_FIFORX0_PIO_STOP_ENABLE_SHIFT,
			SSPIL_FIFORX_0_CONTROL_FIFORX0_PIO_STOP_ENABLE_MASK);
	}
	CHAL_REG_WRITE32(reg, val);

	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_enable_dma(
*                                  CHAL_HANDLE handle,
*                                  CHAL_SSPI_DMA_CHAN_SEL_t channel,
*                                  CHAL_SSPI_FIFO_ID_t fifo_id,
*                                  uint32_t off_on)
*
*  Description: Enable the SSPI DMA channel
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_enable_dma(CHAL_HANDLE handle,
					CHAL_SSPI_DMA_CHAN_SEL_t channel,
					CHAL_SSPI_FIFO_ID_t fifo_id,
					uint32_t off_on)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t val = 0;

	if (!handle) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
	}

	switch (channel) {
	case SSPI_DMA_CHAN_SEL_CHAN_RX0:
		if (fifo_id > SSPI_FIFO_ID_RX3)
			return CHAL_SSPI_STATUS_ILLEGAL_FIFO;

		val =
		    CHAL_REG_READ32(pDevice->base +
				    SSPIL_DMA_RX0_CONTROL_OFFSET);
		FLD_SET(val, off_on, SSPIL_DMA_RX0_CONTROL_DMA_RX0_ENABLE_SHIFT,
			SSPIL_DMA_RX0_CONTROL_DMA_RX0_ENABLE_MASK);
		FLD_SET(val, fifo_id,
			SSPIL_DMA_RX0_CONTROL_DMA_RX0_LINK_FIFO_SHIFT,
			SSPIL_DMA_RX0_CONTROL_DMA_RX0_LINK_FIFO_MASK);
		CHAL_REG_WRITE32(pDevice->base + SSPIL_DMA_RX0_CONTROL_OFFSET,
				 val);

		break;

	case SSPI_DMA_CHAN_SEL_CHAN_RX1:
		if (fifo_id > SSPI_FIFO_ID_RX3)
			return CHAL_SSPI_STATUS_ILLEGAL_FIFO;

		val =
		    CHAL_REG_READ32(pDevice->base +
				    SSPIL_DMA_RX1_CONTROL_OFFSET);
		FLD_SET(val, off_on, SSPIL_DMA_RX1_CONTROL_DMA_RX1_ENABLE_SHIFT,
			SSPIL_DMA_RX1_CONTROL_DMA_RX1_ENABLE_MASK);
		FLD_SET(val, fifo_id,
			SSPIL_DMA_RX1_CONTROL_DMA_RX1_LINK_FIFO_SHIFT,
			SSPIL_DMA_RX1_CONTROL_DMA_RX1_LINK_FIFO_MASK);
		CHAL_REG_WRITE32(pDevice->base + SSPIL_DMA_RX1_CONTROL_OFFSET,
				 val);

		break;

	case SSPI_DMA_CHAN_SEL_CHAN_TX0:
		if (fifo_id < SSPI_FIFO_ID_TX0)
			return CHAL_SSPI_STATUS_ILLEGAL_FIFO;

		fifo_id -= SSPI_FIFO_ID_TX0;
		val =
		    CHAL_REG_READ32(pDevice->base +
				    SSPIL_DMA_TX0_CONTROL_OFFSET);
		FLD_SET(val, off_on, SSPIL_DMA_TX0_CONTROL_DMA_TX0_ENABLE_SHIFT,
			SSPIL_DMA_TX0_CONTROL_DMA_TX0_ENABLE_MASK);
		FLD_SET(val, fifo_id,
			SSPIL_DMA_TX0_CONTROL_DMA_TX0_LINK_FIFO_SHIFT,
			SSPIL_DMA_TX0_CONTROL_DMA_TX0_LINK_FIFO_MASK);
		CHAL_REG_WRITE32(pDevice->base + SSPIL_DMA_TX0_CONTROL_OFFSET,
				 val);

		break;

	case SSPI_DMA_CHAN_SEL_CHAN_TX1:
		if (fifo_id < SSPI_FIFO_ID_TX0)
			return CHAL_SSPI_STATUS_ILLEGAL_FIFO;

		fifo_id -= SSPI_FIFO_ID_TX0;
		val =
		    CHAL_REG_READ32(pDevice->base +
				    SSPIL_DMA_TX1_CONTROL_OFFSET);
		FLD_SET(val, off_on, SSPIL_DMA_TX1_CONTROL_DMA_TX1_ENABLE_SHIFT,
			SSPIL_DMA_TX1_CONTROL_DMA_TX1_ENABLE_MASK);
		FLD_SET(val, fifo_id,
			SSPIL_DMA_TX1_CONTROL_DMA_TX1_LINK_FIFO_SHIFT,
			SSPIL_DMA_TX1_CONTROL_DMA_TX1_LINK_FIFO_MASK);
		CHAL_REG_WRITE32(pDevice->base + SSPIL_DMA_TX1_CONTROL_OFFSET,
				 val);

		break;

	default:
		return CHAL_SSPI_STATUS_ILLEGAL_CHAN;
	}

	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_dma_flush(
*                                  CHAL_SSPI_DMA_CHAN_SEL_t handle,
*                                  CHAL_SSPI_CHAN_SEL_t channel)
*
*  Description: Flush the data currently in the DMA channel, even there are
*               data less than the burst size
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_dma_flush(CHAL_HANDLE handle,
				       CHAL_SSPI_DMA_CHAN_SEL_t channel)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t val = 0;

	if (!handle) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
	}

	if (channel == SSPI_DMA_CHAN_SEL_CHAN_RX0) {
		val =
		    CHAL_REG_READ32(pDevice->base +
				    SSPIL_DMA_RX0_CONTROL_OFFSET);
		val |= SSPIL_DMA_RX0_CONTROL_DMA_RX0_FLUSH_MASK;
		CHAL_REG_WRITE32(pDevice->base + SSPIL_DMA_RX0_CONTROL_OFFSET,
				 val);
	} else if (channel == SSPI_DMA_CHAN_SEL_CHAN_RX1) {
		val =
		    CHAL_REG_READ32(pDevice->base +
				    SSPIL_DMA_RX1_CONTROL_OFFSET);
		val |= SSPIL_DMA_RX1_CONTROL_DMA_RX1_FLUSH_MASK;
	} else
		return CHAL_SSPI_STATUS_ILLEGAL_CHAN;

	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_dma_set_burstsize(
*                                  CHAL_HANDLE handle,
*                                  CHAL_SSPI_DMA_CHAN_SEL_t channel,
*                                  CHAL_SSPI_DMA_BURSTSIZE_t size)
*
*  Description: Set SSPI DMA burst size
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_dma_set_burstsize(CHAL_HANDLE handle,
					       CHAL_SSPI_DMA_CHAN_SEL_t channel,
					       CHAL_SSPI_DMA_BURSTSIZE_t size)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t val = 0;

	if (!handle) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
	}

	switch (channel) {
	case SSPI_DMA_CHAN_SEL_CHAN_RX0:
		val =
		    CHAL_REG_READ32(pDevice->base +
				    SSPIL_DMA_RX0_CONTROL_OFFSET);
		val |= size << SSPIL_DMA_RX0_CONTROL_DMA_RX0_BURSTSIZE_SHIFT;
		CHAL_REG_WRITE32(pDevice->base + SSPIL_DMA_RX0_CONTROL_OFFSET,
				 val);

		break;

	case SSPI_DMA_CHAN_SEL_CHAN_RX1:
		val =
		    CHAL_REG_READ32(pDevice->base +
				    SSPIL_DMA_RX1_CONTROL_OFFSET);
		val |= size << SSPIL_DMA_RX1_CONTROL_DMA_RX1_BURSTSIZE_SHIFT;
		CHAL_REG_WRITE32(pDevice->base + SSPIL_DMA_RX1_CONTROL_OFFSET,
				 val);

		break;

	case SSPI_DMA_CHAN_SEL_CHAN_TX0:
		val =
		    CHAL_REG_READ32(pDevice->base +
				    SSPIL_DMA_TX0_CONTROL_OFFSET);
		val |= size << SSPIL_DMA_TX0_CONTROL_DMA_TX0_BURSTSIZE_SHIFT;
		CHAL_REG_WRITE32(pDevice->base + SSPIL_DMA_TX0_CONTROL_OFFSET,
				 val);

		break;

	case SSPI_DMA_CHAN_SEL_CHAN_TX1:
		val =
		    CHAL_REG_READ32(pDevice->base +
				    SSPIL_DMA_TX1_CONTROL_OFFSET);
		val |= size << SSPIL_DMA_TX1_CONTROL_DMA_TX1_BURSTSIZE_SHIFT;
		CHAL_REG_WRITE32(pDevice->base + SSPIL_DMA_TX1_CONTROL_OFFSET,
				 val);

		break;

	default:
		return CHAL_SSPI_STATUS_ILLEGAL_CHAN;
	}

	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_dma_set_tx_size(
*                                  CHAL_HANDLE handle,
*                                  CHAL_SSPI_DMA_CHAN_SEL_t channel,
*                                  uint32_t size)
*
*  Description: Set SSPI DMA transfer size.
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_dma_set_tx_size(CHAL_HANDLE handle,
					     CHAL_SSPI_DMA_CHAN_SEL_t channel,
					     uint32_t size)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t val = 0;

	if (!handle) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
	}

	if (channel == SSPI_DMA_CHAN_SEL_CHAN_TX0) {
		val =
		    CHAL_REG_READ32(pDevice->base +
				    SSPIL_DMA_TX0_CONTROL_OFFSET);
		FLD_SET(val, size, SSPIL_DMA_TX0_CONTROL_DMA_TX0_SIZE_SHIFT,
			SSPIL_DMA_TX0_CONTROL_DMA_TX0_SIZE_MASK);
		CHAL_REG_WRITE32(pDevice->base + SSPIL_DMA_TX0_CONTROL_OFFSET,
				 val);
	} else if (channel == SSPI_DMA_CHAN_SEL_CHAN_TX1) {
		val =
		    CHAL_REG_READ32(pDevice->base +
				    SSPIL_DMA_TX1_CONTROL_OFFSET);
		FLD_SET(val, size, SSPIL_DMA_TX1_CONTROL_DMA_TX1_SIZE_SHIFT,
			SSPIL_DMA_TX1_CONTROL_DMA_TX1_SIZE_MASK);
		CHAL_REG_WRITE32(pDevice->base + SSPIL_DMA_TX1_CONTROL_OFFSET,
				 val);
	} else
		return CHAL_SSPI_STATUS_ILLEGAL_CHAN;

	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_get_dma_status(
*                                  CHAL_HANDLE handle,
*                                  CHAL_SSPI_DMA_CHAN_SEL_t channel,
*                                  CHAL_SSPI_FIFO_ID_t *fifo_id,
*                                  uint32_t *off_on)
*
*  Description: Get the current SSPI DMA status
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_get_dma_status(CHAL_HANDLE handle,
					    CHAL_SSPI_DMA_CHAN_SEL_t channel,
					    CHAL_SSPI_FIFO_ID_t *fifo_id,
					    uint32_t *off_on)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t val = 0;

	if (!handle || !fifo_id || !off_on) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_PARA;
	}

	switch (channel) {
	case SSPI_DMA_CHAN_SEL_CHAN_RX0:
		val =
		    CHAL_REG_READ32(pDevice->base +
				    SSPIL_DMA_RX0_CONTROL_OFFSET);

		*(uint32_t *)fifo_id = FLD_GET(val,
				SSPIL_DMA_RX0_CONTROL_DMA_RX0_LINK_FIFO_SHIFT,
				SSPIL_DMA_RX0_CONTROL_DMA_RX0_LINK_FIFO_MASK);
		*off_on =
		    FLD_GET(val, SSPIL_DMA_RX0_CONTROL_DMA_RX0_ENABLE_SHIFT,
			    SSPIL_DMA_RX0_CONTROL_DMA_RX0_ENABLE_MASK);

		break;

	case SSPI_DMA_CHAN_SEL_CHAN_RX1:
		val =
		    CHAL_REG_READ32(pDevice->base +
				    SSPIL_DMA_RX1_CONTROL_OFFSET);

		*(uint32_t *)fifo_id = FLD_GET(val,
				SSPIL_DMA_RX1_CONTROL_DMA_RX1_LINK_FIFO_SHIFT,
				SSPIL_DMA_RX1_CONTROL_DMA_RX1_LINK_FIFO_MASK);
		*off_on =
		    FLD_GET(val, SSPIL_DMA_RX1_CONTROL_DMA_RX1_ENABLE_SHIFT,
			    SSPIL_DMA_RX1_CONTROL_DMA_RX1_ENABLE_MASK);

		break;

	case SSPI_DMA_CHAN_SEL_CHAN_TX0:
		val =
		    CHAL_REG_READ32(pDevice->base +
				    SSPIL_DMA_TX0_CONTROL_OFFSET);

		*(uint32_t *)fifo_id = FLD_GET(val,
				SSPIL_DMA_TX0_CONTROL_DMA_TX0_LINK_FIFO_SHIFT,
				SSPIL_DMA_TX0_CONTROL_DMA_TX0_LINK_FIFO_MASK)
		    + SSPI_FIFO_ID_TX0;
		*off_on =
		    FLD_GET(val, SSPIL_DMA_TX0_CONTROL_DMA_TX0_ENABLE_SHIFT,
			    SSPIL_DMA_TX0_CONTROL_DMA_TX0_ENABLE_MASK);

		break;

	case SSPI_DMA_CHAN_SEL_CHAN_TX1:
		val =
		    CHAL_REG_READ32(pDevice->base +
				    SSPIL_DMA_TX1_CONTROL_OFFSET);

		*(uint32_t *)fifo_id = FLD_GET(val,
				SSPIL_DMA_TX1_CONTROL_DMA_TX1_LINK_FIFO_SHIFT,
				SSPIL_DMA_TX1_CONTROL_DMA_TX1_LINK_FIFO_MASK)
		    + SSPI_FIFO_ID_TX0;
		*off_on =
		    FLD_GET(val, SSPIL_DMA_TX1_CONTROL_DMA_TX1_ENABLE_SHIFT,
			    SSPIL_DMA_TX1_CONTROL_DMA_TX1_ENABLE_MASK);

		break;

	default:
		return CHAL_SSPI_STATUS_ILLEGAL_CHAN;
	}

	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: uint32_t chal_sspi_read_data(
*                                  CHAL_HANDLE handle,
*                                  CHAL_SSPI_FIFO_ID_t fifo_id,
*                                  CHAL_SSPI_PROT_t prot,
*                                  uint8_t *buf,
*                                  uint32_t size)
*
*  Description: Read data from SSPI FIFO. It is used for PIO mode only
*
****************************************************************************/
uint32_t chal_sspi_read_data(CHAL_HANDLE handle,
			     CHAL_SSPI_FIFO_ID_t fifo_id,
			     CHAL_SSPI_PROT_t prot, uint8_t *buf, uint32_t size)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t tmp = size, val, width = SPI_FIFO_DATA_RWSIZE_RESERVED;
	uint32_t ctlbase, entbase;

	if (!handle || !buf || (fifo_id > SSPI_FIFO_ID_RX3)) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return 0;
	}
	ctlbase = REG_FIFORX_CTL_ADDR(fifo_id, pDevice->base +
						SSPIL_FIFORX_0_CONTROL_OFFSET);
	entbase = REG_FIFO_ENTRYRX_ADDR(fifo_id, pDevice->base +
						SSPIL_FIFO_ENTRY0RX_OFFSET);

	switch (prot) {
	case SSPI_PROT_SPI_MODE0:
	case SSPI_PROT_SPI_MODE1:
	case SSPI_PROT_SPI_MODE2:
	case SSPI_PROT_SPI_MODE3:
	case SSPI_PROT_I2S_MODE1:
	case SSPI_PROT_I2S_MODE2:
	case SSPI_PROT_DEFAULT_PCM:
		while (tmp > 3) {
			if (width != SPI_FIFO_DATA_RWSIZE_32) {
				val = CHAL_REG_READ32(ctlbase);
				FLD_SET(val, SPI_FIFO_DATA_RWSIZE_32,
			SSPIL_FIFORX_0_CONTROL_FIFORX0_DATA_READ_SIZE_SHIFT,
			SSPIL_FIFORX_0_CONTROL_FIFORX0_DATA_READ_SIZE_MASK);
				CHAL_REG_WRITE32(ctlbase, val);
				width = SPI_FIFO_DATA_RWSIZE_32;
			}

			if (((CHAL_REG_READ32(ctlbase) &
			      SSPIL_FIFORX_0_CONTROL_FIFORX0_LEVEL_MASK) >>
			     SSPIL_FIFORX_0_CONTROL_FIFORX0_LEVEL_SHIFT) < 4)
				continue;
			else {
				*((uint32_t *)buf) = CHAL_REG_READ32(entbase);
				buf += 4;
				tmp -= 4;
			}
		}

		if (!tmp)
			break;

		switch (tmp) {
		case 1:
			val = CHAL_REG_READ32(ctlbase);
			FLD_SET(val, SPI_FIFO_DATA_RWSIZE_8,
			SSPIL_FIFORX_0_CONTROL_FIFORX0_DATA_READ_SIZE_SHIFT,
			SSPIL_FIFORX_0_CONTROL_FIFORX0_DATA_READ_SIZE_MASK)
				;
			CHAL_REG_WRITE32(ctlbase, val);

			while (CHAL_REG_READ32(ctlbase) &
				SSPIL_FIFORX_0_CONTROL_FIFORX0_EMPTY_MASK)
				;

			*buf = (uint8_t)CHAL_REG_READ32(entbase);
			break;

		case 2:
			val = CHAL_REG_READ32(ctlbase);
			FLD_SET(val, SPI_FIFO_DATA_RWSIZE_16,
			SSPIL_FIFORX_0_CONTROL_FIFORX0_DATA_READ_SIZE_SHIFT,
			SSPIL_FIFORX_0_CONTROL_FIFORX0_DATA_READ_SIZE_MASK)
				;
			CHAL_REG_WRITE32(ctlbase, val);

			while (((CHAL_REG_READ32(ctlbase) &
				SSPIL_FIFORX_0_CONTROL_FIFORX0_LEVEL_MASK) >>
				SSPIL_FIFORX_0_CONTROL_FIFORX0_LEVEL_SHIFT) <
				2)
				;

			*((uint16_t *)buf) = (uint16_t)CHAL_REG_READ32(entbase);
			break;

		case 3:
			val = CHAL_REG_READ32(ctlbase);
			FLD_SET(val, SPI_FIFO_DATA_RWSIZE_16,
			SSPIL_FIFORX_0_CONTROL_FIFORX0_DATA_READ_SIZE_SHIFT,
			SSPIL_FIFORX_0_CONTROL_FIFORX0_DATA_READ_SIZE_MASK)
				;
			CHAL_REG_WRITE32(ctlbase, val);

			while (((CHAL_REG_READ32(ctlbase) &
				SSPIL_FIFORX_0_CONTROL_FIFORX0_LEVEL_MASK) >>
				SSPIL_FIFORX_0_CONTROL_FIFORX0_LEVEL_SHIFT) <
				2)
				;
			*((uint16_t *)buf) = (uint16_t)CHAL_REG_READ32(entbase);
			buf += 2;

			val = CHAL_REG_READ32(ctlbase);
			FLD_SET(val, SPI_FIFO_DATA_RWSIZE_8,
			SSPIL_FIFORX_0_CONTROL_FIFORX0_DATA_READ_SIZE_SHIFT,
			SSPIL_FIFORX_0_CONTROL_FIFORX0_DATA_READ_SIZE_MASK)
				;
			CHAL_REG_WRITE32(ctlbase, val);

			while (CHAL_REG_READ32(ctlbase) &
				SSPIL_FIFORX_0_CONTROL_FIFORX0_EMPTY_MASK)
				;

			*buf = (uint8_t)CHAL_REG_READ32(entbase);

			break;

		default:
			break;
		}
		/* Always change read size back to 32bit */
		val = CHAL_REG_READ32(ctlbase);
		FLD_SET(val, SPI_FIFO_DATA_RWSIZE_32,
			SSPIL_FIFORX_0_CONTROL_FIFORX0_DATA_READ_SIZE_SHIFT,
			SSPIL_FIFORX_0_CONTROL_FIFORX0_DATA_READ_SIZE_MASK)
			;
		CHAL_REG_WRITE32(ctlbase, val);

		break;

	case SSPI_PROT_I2C:
		val = CHAL_REG_READ32(ctlbase);
		FLD_SET(val, 1,
			SSPIL_FIFORX_0_CONTROL_FIFORX0_DATA_READ_SIZE_SHIFT,
			SSPIL_FIFORX_0_CONTROL_FIFORX0_DATA_READ_SIZE_MASK)
			;
		CHAL_REG_WRITE32(ctlbase, val);

		while (tmp) {
			if (CHAL_REG_READ32(ctlbase) &
			    SSPIL_FIFORX_0_CONTROL_FIFORX0_EMPTY_MASK)
				continue;
			else {
				*buf = (uint8_t)CHAL_REG_READ8(entbase);
				buf++;
				tmp--;
			}
		}
		break;

	default:
		return 0;

	}
	return size;
}

/****************************************************************************
*
*  Function Name: uint32_t chal_sspi_write_data(
*                                  CHAL_HANDLE handle,
*                                  CHAL_SSPI_FIFO_ID_t fifo_id,
*                                  CHAL_SSPI_PROT_t prot,
*                                  uint8_t *buf,
*                                  uint32_t size
*
*  Description: Write data to SSPI FIFO. It is used for PIO mode only.
*
****************************************************************************/
uint32_t chal_sspi_write_data(CHAL_HANDLE handle,
			      CHAL_SSPI_FIFO_ID_t fifo_id,
			      CHAL_SSPI_PROT_t prot,
			      uint8_t *buf, uint32_t size)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t tmp = size, val = 0, width = SPI_FIFO_DATA_RWSIZE_RESERVED;
	uint32_t ctlbase, entbase;

	if (!handle || !buf || (fifo_id < SSPI_FIFO_ID_TX0)) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return 0;
	}

	fifo_id -= SSPI_FIFO_ID_TX0;
	ctlbase = REG_FIFOTX_CTL_ADDR(fifo_id,
				      pDevice->base +
				      SSPIL_FIFOTX_0_CONTROL_OFFSET);
	entbase =
	    REG_FIFO_ENTRYTX_ADDR(fifo_id,
				  pDevice->base + SSPIL_FIFO_ENTRY0TX_OFFSET);

	switch (prot) {
	case SSPI_PROT_SPI_MODE0:
	case SSPI_PROT_SPI_MODE1:
	case SSPI_PROT_SPI_MODE2:
	case SSPI_PROT_SPI_MODE3:
	case SSPI_PROT_I2S_MODE1:
	case SSPI_PROT_I2S_MODE2:
	case SSPI_PROT_DEFAULT_PCM:
		while (tmp > 3) {
			if (width != SPI_FIFO_DATA_RWSIZE_32) {
				val = CHAL_REG_READ32(ctlbase);
				FLD_SET(val, SPI_FIFO_DATA_RWSIZE_32,
			SSPIL_FIFOTX_0_CONTROL_FIFOTX0_DATA_WRITE_SIZE_SHIFT,
			SSPIL_FIFOTX_0_CONTROL_FIFOTX0_DATA_WRITE_SIZE_MASK);
				CHAL_REG_WRITE32(ctlbase, val);
				width = SPI_FIFO_DATA_RWSIZE_32;
			}
			if (CHAL_REG_READ32(ctlbase) &
			    SSPIL_FIFOTX_0_CONTROL_FIFOTX0_FULL_MASK)
				continue;
			else {
				CHAL_REG_WRITE32(entbase, *((uint32_t *)buf));
				buf += 4;
				tmp -= 4;
			}
		}
		if (!tmp)
			break;

		switch (tmp) {
		case 1:
			val = CHAL_REG_READ32(ctlbase);
			FLD_SET(val, SPI_FIFO_DATA_RWSIZE_8,
			SSPIL_FIFOTX_0_CONTROL_FIFOTX0_DATA_WRITE_SIZE_SHIFT,
			SSPIL_FIFOTX_0_CONTROL_FIFOTX0_DATA_WRITE_SIZE_MASK);
			CHAL_REG_WRITE32(ctlbase, val);

			CHAL_REG_WRITE32(entbase, *buf);

			break;

		case 2:
			val = CHAL_REG_READ32(ctlbase);
			FLD_SET(val, SPI_FIFO_DATA_RWSIZE_16,
			SSPIL_FIFOTX_0_CONTROL_FIFOTX0_DATA_WRITE_SIZE_SHIFT,
			SSPIL_FIFOTX_0_CONTROL_FIFOTX0_DATA_WRITE_SIZE_MASK);
			CHAL_REG_WRITE32(ctlbase, val);
			CHAL_REG_WRITE32(entbase, *((uint16_t *)buf));

			break;

		case 3:
			val = CHAL_REG_READ32(ctlbase);
			FLD_SET(val, SPI_FIFO_DATA_RWSIZE_16,
			SSPIL_FIFOTX_0_CONTROL_FIFOTX0_DATA_WRITE_SIZE_SHIFT,
			SSPIL_FIFOTX_0_CONTROL_FIFOTX0_DATA_WRITE_SIZE_MASK);
			CHAL_REG_WRITE32(ctlbase, val);
			CHAL_REG_WRITE32(entbase, *((uint16_t *)buf));
			buf += 2;

			val = CHAL_REG_READ32(ctlbase);
			FLD_SET(val, SPI_FIFO_DATA_RWSIZE_8,
			SSPIL_FIFOTX_0_CONTROL_FIFOTX0_DATA_WRITE_SIZE_SHIFT,
			SSPIL_FIFOTX_0_CONTROL_FIFOTX0_DATA_WRITE_SIZE_MASK);
			CHAL_REG_WRITE32(ctlbase, val);
			CHAL_REG_WRITE32(entbase, *buf);
			break;

		default:
			break;
		}

		/* Always change write size back to 32bit */
		val = CHAL_REG_READ32(ctlbase);
		FLD_SET(val, SPI_FIFO_DATA_RWSIZE_32,
			SSPIL_FIFOTX_0_CONTROL_FIFOTX0_DATA_WRITE_SIZE_SHIFT,
			SSPIL_FIFOTX_0_CONTROL_FIFOTX0_DATA_WRITE_SIZE_MASK);
		CHAL_REG_WRITE32(ctlbase, val);
		CHAL_REG_WRITE32(entbase, *buf);
		break;

	case SSPI_PROT_I2C:
		val = CHAL_REG_READ32(ctlbase);
		FLD_SET(val, 1,
			SSPIL_FIFOTX_0_CONTROL_FIFOTX0_DATA_WRITE_SIZE_SHIFT,
			SSPIL_FIFOTX_0_CONTROL_FIFOTX0_DATA_WRITE_SIZE_MASK);
		CHAL_REG_WRITE32(ctlbase, val);

		while (tmp) {
			if (CHAL_REG_READ32(ctlbase) &
			    SSPIL_FIFOTX_0_CONTROL_FIFOTX0_FULL_MASK)
				return size - tmp;
			else {
				CHAL_REG_WRITE8(entbase, *buf);
				buf++;
				tmp--;
			}
		}
		break;

	default:
		return 0;

	}
	return size;
}

/****************************************************************************
*
*  Function Name: uint32_t chal_sspi_read_fifo(
*                                  CHAL_HANDLE handle,
*                                  CHAL_SSPI_FIFO_ID_t fifo_id,
*                                  uint32_t *date)
*
*  Description: Read one data entry from SSPI FIFO.
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_read_fifo(CHAL_HANDLE handle,
				       CHAL_SSPI_FIFO_ID_t fifo_id,
				       uint32_t *data)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;

	uint32_t entbase;

	if (!handle || !data || (fifo_id > SSPI_FIFO_ID_RX3)) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_PARA;
	}
	entbase = REG_FIFO_ENTRYRX_ADDR(fifo_id, pDevice->base +
						SSPIL_FIFO_ENTRY0RX_OFFSET);
	*data = CHAL_REG_READ32(entbase);
	return CHAL_SSPI_STATUS_SUCCESS;

}

/****************************************************************************
*
*  Function Name: uint32_t chal_sspi_write_fifo(
*                                  CHAL_HANDLE handle,
*                                  CHAL_SSPI_FIFO_ID_t fifo_id,
*                                  uint32_t data)
*
*  Description: Write one data entry to SSPI FIFO.
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_write_fifo(CHAL_HANDLE handle,
					CHAL_SSPI_FIFO_ID_t fifo_id,
					uint32_t data)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t entbase;

	if (!handle || (fifo_id < SSPI_FIFO_ID_TX0)) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_PARA;
	}
	entbase = REG_FIFO_ENTRYTX_ADDR(fifo_id - SSPI_FIFO_ID_TX0,
						 pDevice->base +
						 SSPIL_FIFO_ENTRY0TX_OFFSET);
	CHAL_REG_WRITE32(entbase, data);
	return CHAL_SSPI_STATUS_SUCCESS;

}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_enable_intr(
*                                               CHAL_HANDLE handle,
*                                               uint32_t intr_mask)
*
*  Description: Enable SSPI interrupts
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_enable_intr(CHAL_HANDLE handle, uint32_t intr_mask)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;

	if (!handle) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
	}

	CHAL_REG_WRITE32(pDevice->base + SSPIL_INTERRUPT_ENABLE_OFFSET,
			 intr_mask);
	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_enable_error_intr(
*                                               CHAL_HANDLE handle,
*                                               uint32_t intr_mask)
*
*  Description: Enable SSPI error interrupts
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_enable_error_intr(CHAL_HANDLE handle,
					       uint32_t intr_mask)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;

	if (!handle) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
	}

	CHAL_REG_WRITE32(pDevice->base + SSPIL_INTERRUPT_ERROR_ENABLE_OFFSET,
			 intr_mask);
	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_get_intr_mask(
*                                               CHAL_HANDLE handle,
*                                               uint32_t *intr_mask)
*
*  Description: Get current SSPI interrupt mask
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_get_intr_mask(CHAL_HANDLE handle,
					   uint32_t *intr_mask)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;

	if (!handle) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
	}

	*intr_mask =
	    CHAL_REG_READ32(pDevice->base + SSPIL_INTERRUPT_ENABLE_OFFSET);
	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_get_error_intr_mask(
*                                               CHAL_HANDLE handle,
*                                               uint32_t *intr_mask)
*
*  Description: Get current SSPI error interrupt mask
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_get_error_intr_mask(CHAL_HANDLE handle,
						 uint32_t *intr_mask)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;

	if (!handle) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
	}

	*intr_mask = CHAL_REG_READ32(pDevice->base +
				     SSPIL_INTERRUPT_ERROR_ENABLE_OFFSET);
	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_enable_intr(
*                                               CHAL_HANDLE handle,
*                                               uint32_t *status)
*
*  Description: Get the current SSPI interrupt status
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_get_intr_status(CHAL_HANDLE handle,
					     uint32_t *status,
					     uint32_t *det_status)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;

	if (!handle || !status) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_PARA;
	}

	*status =
	    CHAL_REG_READ32(pDevice->base + SSPIL_INTERRUPT_STATUS_OFFSET);
	if (det_status)
		*det_status = CHAL_REG_READ32(pDevice->base +
				SSPIL_DETAIL_INTERRUPT_STATUS_OFFSET);

	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_clear_intr(CHAL_HANDLE handle,
*                                                         uint32_t intr_mask,
*                                                         uint32_t det_intr_mask
*							 )
*
*  Description: Clear the current SSPI interrupt status
***********************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_clear_intr(CHAL_HANDLE handle,
					uint32_t intr_mask,
					uint32_t det_intr_mask)
{
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;

	if (!handle) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
	}

	if (det_intr_mask)
		CHAL_REG_WRITE32(pDevice->base +
				 SSPIL_DETAIL_INTERRUPT_STATUS_OFFSET,
				 det_intr_mask);

	CHAL_REG_WRITE32(pDevice->base + SSPIL_INTERRUPT_STATUS_OFFSET,
			 intr_mask);

	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_enable_task(
*                                            CHAL_HANDLE handle,
*                                            uint32_t off_on)
*
*  Description: Enable the SSPI task
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_enable_task(CHAL_HANDLE handle,
					 uint32_t task_idx, uint32_t off_on)
{
	/* enable scheduler */
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t val;

	if (!handle) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
	}

	if (task_idx >= pDevice->num_tasks)
		return CHAL_SSPI_STATUS_ILLEGAL_INDEX;

	val = 0;
	val = CHAL_REG_READ32(REG_TASK_ADDR(task_idx,
					    pDevice->base +
					    SSPIL_TASK0_DESC_MSB_OFFSET));
	FLD_SET(val, off_on, SSPIL_TASK0_DESC_MSB_TASK0_ENABLE_SHIFT,
		SSPIL_TASK0_DESC_MSB_TASK0_ENABLE_MASK);

	CHAL_REG_WRITE32(REG_TASK_ADDR(task_idx,
				       pDevice->base +
				       SSPIL_TASK0_DESC_MSB_OFFSET), val);
	return CHAL_SSPI_STATUS_SUCCESS;
}

/****************************************************************************
*
*  Function Name: CHAL_SSPI_STATUS_t chal_sspi_enable_scheduler(
*                                            CHAL_HANDLE handle,
*                                            uint32_t off_on)
*
*  Description: Enable the SSPI scheduler to excute the configured tasks
*
****************************************************************************/
CHAL_SSPI_STATUS_t chal_sspi_enable_scheduler(CHAL_HANDLE handle,
					      uint32_t off_on)
{
	/* enable scheduler */
	CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *) handle;
	uint32_t val;

	if (!handle) {
		chal_dprintf(CDBG_ERRO, "invalid argument\n");
		return CHAL_SSPI_STATUS_ILLEGAL_HANDLE;
	}

	val = CHAL_REG_READ32(pDevice->base + SSPIL_SCHEDULER_CONTROL_OFFSET);
	FLD_SET(val, off_on,
		SSPIL_SCHEDULER_CONTROL_SCH_ENABLE_SHIFT,
		SSPIL_SCHEDULER_CONTROL_SCH_ENABLE_MASK);
	CHAL_REG_WRITE32(pDevice->base + SSPIL_SCHEDULER_CONTROL_OFFSET, val);
	return CHAL_SSPI_STATUS_SUCCESS;
}
