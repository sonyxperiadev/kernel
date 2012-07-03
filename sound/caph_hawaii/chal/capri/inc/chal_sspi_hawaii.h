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
/**
*
*  @file   chal_sspi.h
*  @brief  SSPI cHAL interface
*  @note
*
*****************************************************************************/
#ifndef CHAL_SSPI_H__
#define CHAL_SSPI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <mach/rdb/brcm_rdb_sspil.h>

#include <plat/chal/chal_common.h>
#include <plat/chal/chal_types.h>

/**
 * @addtogroup cHAL_Interface
 * @{
 */

/**
* SSPI Macros
*****************************************************************************/
#define CHAL_SSPI_CAPH_CLOCK_RATE   26000000
#define CHAL_SSPI_MAX_WORD_LEN      32
#define CHAL_SSPI_MAX_CHANNELS      4

/**
* SSPI status
******************************************************************************/
	typedef enum {
		CHAL_SSPI_STATUS_SUCCESS,
		CHAL_SSPI_STATUS_FAILURE,
		CHAL_SSPI_STATUS_ILLEGAL_HANDLE,
		CHAL_SSPI_STATUS_ILLEGAL_BASE,
		CHAL_SSPI_STATUS_ILLEGAL_TYPE,
		CHAL_SSPI_STATUS_ILLEGAL_INDEX,
		CHAL_SSPI_STATUS_ILLEGAL_FRAME,
		CHAL_SSPI_STATUS_ILLEGAL_FIFO,
		CHAL_SSPI_STATUS_ILLEGAL_CHAN,
		CHAL_SSPI_STATUS_ILLEGAL_PARA,
	} CHAL_SSPI_STATUS_t;

/**
* SSPI core type
******************************************************************************/
	typedef enum {
		SSPI_TYPE_FULL,
		SSPI_TYPE_LITE,
	} CHAL_SSPI_TYPE_t;

/**
* SSPI mode type
******************************************************************************/
	typedef enum {
		SSPI_MODE_MASTER,
		SSPI_MODE_SLAVE,
	} CHAL_SSPI_MODE_t;

/**
* SSPI protocol type
******************************************************************************/
	typedef enum {
		SSPI_PROT_SPI_MODE0,
		SSPI_PROT_SPI_MODE1,
		SSPI_PROT_SPI_MODE2,
		SSPI_PROT_SPI_MODE3,
		SSPI_PROT_TI_SSP,
		SSPI_PROT_NATIONAL_MICROWIRE,
		SSPI_PROT_I2C,
		SSPI_PROT_I2S_MODE1,
		SSPI_PROT_I2S_MODE2,
		SSPI_PROT_AUDIO_LEFT_AJUST,
		SSPI_PROT_AUDIO_RIGHT_AJUST,
		SSPI_PROT_SPDIF_OUT,
		SSPI_PROT_DEFAULT_PCM,
	} CHAL_SSPI_PROT_t;

/**
* SSPI clock divider
******************************************************************************/
	typedef enum {
		SSPI_CLK_DIVIDER0,
		SSPI_CLK_DIVIDER1,
		SSPI_CLK_DIVIDER2,
		SSPI_CLK_REF_DIVIDER,
		SSPI_CLK_SIDETONE_DIVIDER,
	} CHAL_SSPI_CLK_DIVIDER_t;

/**
* SSPI clock source type
******************************************************************************/
	typedef enum {
		SSPI_CLK_SRC_INTCLK,
		SSPI_CLK_SRC_EXTCLK,
		SSPI_CLK_SRC_AUDIOCLK,
		SSPI_CLK_SRC_CAPHCLK,
	} CHAL_SSPI_CLK_SRC_t;

/**
* SSPI FIFO size, can be none, quarter, half or full of the physical FIFO size
******************************************************************************/
	typedef enum {
		SSPI_FIFO_SIZE_NONE,
		SSPI_FIFO_SIZE_QUARTER,
		SSPI_FIFO_SIZE_HALF,
		SSPI_FIFO_SIZE_FULL,
	} CHAL_SSPI_FIFO_SIZE_t;

/**
* SSPI FIFO endianess type
******************************************************************************/
	typedef enum {
		SSPI_FIFO_ENDIAN_BIG,
		SSPI_FIFO_ENDIAN_LITTLE,
	} CHAL_SSPI_FIFO_ENDIAN_t;

/**
* SSPI FIFO data packing type
******************************************************************************/
	typedef enum {
		SSPI_FIFO_DATA_PACK_NONE,
		SSPI_FIFO_DATA_PACK_8BIT,
		SSPI_FIFO_DATA_PACK_16BIT,
	} CHAL_SSPI_FIFO_DATA_PACK_t;

/**
* SSPI FIFO data size
******************************************************************************/
	typedef enum {
		SSPI_FIFO_DATA_SIZE_32BIT,
		SSPI_FIFO_DATA_SIZE_8BIT,
		SSPI_FIFO_DATA_SIZE_16BIT,
	} CHAL_SSPI_FIFO_DATA_SIZE_t;

/**
* SSPI FIFO ID
******************************************************************************/
	typedef enum {
		SSPI_FIFO_ID_RX0,
		SSPI_FIFO_ID_RX1,
		SSPI_FIFO_ID_RX2,
		SSPI_FIFO_ID_RX3,
		SSPI_FIFO_ID_TX0,
		SSPI_FIFO_ID_TX1,
		SSPI_FIFO_ID_TX2,
		SSPI_FIFO_ID_TX3,
	} CHAL_SSPI_FIFO_ID_t;

/**
* SSPI FIFO data read/write access size, can be 8, 16 or 32 bits in width
******************************************************************************/
	typedef enum {
		SPI_FIFO_DATA_RWSIZE_32,
		SPI_FIFO_DATA_RWSIZE_8,
		SPI_FIFO_DATA_RWSIZE_16,
		SPI_FIFO_DATA_RWSIZE_RESERVED,
	} CHAL_SSPI_FIFO_DATA_RWSIZE_t;

/**
* SSPI pattern type
******************************************************************************/
	typedef enum {
		CHAL_SSPI_PATT_TYPE_I2C_STD_START,
		CHAL_SSPI_PATT_TYPE_I2C_STD_STOP,
		CHAL_SSPI_PATT_TYPE_I2C_STD_RESTART,
		CHAL_SSPI_PATT_TYPE_I2C_FS_START,
		CHAL_SSPI_PATT_TYPE_I2C_FS_STOP,
		CHAL_SSPI_PATT_TYPE_I2C_FS_RESTART,
		CHAL_SSPI_PATT_TYPE_I2C_HS_STOP,
		CHAL_SSPI_PATT_TYPE_I2C_HS_RESTART,
	} CHAL_SSPI_PATT_TYPE_t;

/**
* SSPI DMA burst size
******************************************************************************/
	typedef enum {
		CHAL_SSPI_DMA_BURSTSIZE_16BYTES,
		CHAL_SSPI_DMA_BURSTSIZE_64BYTES,
	} CHAL_SSPI_DMA_BURSTSIZE_t;

/**
* SSPI chip-select line selection
******************************************************************************/
	typedef enum {
		SSPI_CS_SEL_CS0,
		SSPI_CS_SEL_CS1,
		SSPI_CS_SEL_CS2,
		SSPI_CS_SEL_CS3,
	} CHAL_SSPI_CS_SEL_t;

/**
* SSPI RX line selection
******************************************************************************/
	typedef enum {
		SSPI_RX_SEL_RX0,
		SSPI_RX_SEL_RX1,
		SSPI_RX_SEL_COPY_TX0,
		SSPI_RX_SEL_COPY_TX1,
	} CHAL_SSPI_RX_SEL_t;

/**
* SSPI TX line selection
******************************************************************************/
	typedef enum {
		SSPI_TX_SEL_TX0,
		SSPI_TX_SEL_TX1,
		SSPI_TX_SEL_COPY_RX0,
		SSPI_TX_SEL_COPY_RX1,
	} CHAL_SSPI_TX_SEL_t;

/**
* SSPI channel selection, can be Channel 0 or Channel 1. Channel 0 uses
* RX 0 and TX0 lines while Channel 1 uses RX 1 and TX 1 lines
******************************************************************************/
	typedef enum {
		SSPI_CHAN_SEL_CHAN0,
		SSPI_CHAN_SEL_CHAN1,
	} CHAL_SSPI_CHAN_SEL_t;

/**
* SSPI DMA channel selection
******************************************************************************/
	typedef enum {
		SSPI_DMA_CHAN_SEL_CHAN_RX0,
		SSPI_DMA_CHAN_SEL_CHAN_RX1,
		SSPI_DMA_CHAN_SEL_CHAN_TX0,
		SSPI_DMA_CHAN_SEL_CHAN_TX1,
	} CHAL_SSPI_DMA_CHAN_SEL_t;

/**
* SSPI task initial condition bits, sepecify which FIFO thresholds are checked
* before starting the task
******************************************************************************/
	typedef enum {
		SSPI_TASK_INIT_COND_THRESHOLD_RX0 = (1 << 0),
		SSPI_TASK_INIT_COND_THRESHOLD_RX1 = (1 << 1),
		SSPI_TASK_INIT_COND_THRESHOLD_RX2 = (1 << 2),
		SSPI_TASK_INIT_COND_THRESHOLD_RX3 = (1 << 3),
		SSPI_TASK_INIT_COND_THRESHOLD_TX0 = (1 << 4),
		SSPI_TASK_INIT_COND_THRESHOLD_TX1 = (1 << 5),
		SSPI_TASK_INIT_COND_THRESHOLD_TX2 = (1 << 6),
		SSPI_TASK_INIT_COND_THRESHOLD_TX3 = (1 << 7),
	} CHAL_SSPI_TASK_INIT_COND_t;

/**
* SSPI bit rate
******************************************************************************/
	typedef enum {
		CHAL_SSPI_BIT_RATE_64kHz,
		CHAL_SSPI_BIT_RATE_128kHz,
		CHAL_SSPI_BIT_RATE_256kHz,
		CHAL_SSPI_BIT_RATE_352_8kHz,
		CHAL_SSPI_BIT_RATE_512kHz,
		CHAL_SSPI_BIT_RATE_600kHz,
		CHAL_SSPI_BIT_RATE_705_6kHz,
		CHAL_SSPI_BIT_RATE_1200kHz,
		CHAL_SSPI_BIT_RATE_1411_2kHz,
		CHAL_SSPI_BIT_RATE_1536kHz,
		CHAL_SSPI_BIT_RATE_1600kHz,
		CHAL_SSPI_BIT_RATE_2400kHz,
		CHAL_SSPI_BIT_RATE_2822_4kHz,
		CHAL_SSPI_BIT_RATE_3120kHz,
		CHAL_SSPI_BIT_RATE_3200kHz,
		CHAL_SSPI_BIT_RATE_4800kHz,
		CHAL_SSPI_BIT_RATE_11140kHz,
		CHAL_SSPI_BIT_RATE_12MHz,
		CHAL_SSPI_BIT_RATE_12288kHz,
		CHAL_SSPI_BIT_RATE_13MHz,
		CHAL_SSPI_BIT_RATE_15600kHz,
		CHAL_SSPI_BIT_RATE_19500kHz,
		CHAL_SSPI_BIT_RATE_22280kHz,
		CHAL_SSPI_BIT_RATE_24MHz,
		CHAL_SSPI_BIT_RATE_26MHz,
		CHAL_SSPI_BIT_RATE_31200kHz,
		CHAL_SSPI_BIT_RATE_39MHz,
		CHAL_SSPI_BIT_RATE_48MHz,
		CHAL_SSPI_BIT_RATE_52MHz,
	} CHAL_SSPI_BIT_RATE_t;

/**
* SSPI task configure structure
******************************************************************************/
	typedef struct {
		CHAL_SSPI_CS_SEL_t cs_sel;
		CHAL_SSPI_RX_SEL_t rx_sel;
		CHAL_SSPI_TX_SEL_t tx_sel;
		CHAL_SSPI_CHAN_SEL_t chan_sel;
		CHAL_SSPI_CLK_DIVIDER_t div_sel;
		uint32_t continuous;
		uint32_t wait_before_start;
		uint32_t not_do_once_only;
		uint32_t loop_cnt;
		uint32_t seq_ptr;
		uint32_t init_cond_mask;
	} chal_sspi_task_conf_t;

/**
* SSPI sequence Opcode
******************************************************************************/
	enum {
		SSPI_SEQ_OPCODE_NEXT_PC,
		SSPI_SEQ_OPCODE_COND_JUMP,
		SSPI_SEQ_OPCODE_ALT_JUMP,
		SSPI_SEQ_OPCODE_STOP,
	};

/**
* SSPI sequence configure structure
******************************************************************************/
	typedef struct {
		uint32_t opcode:2;
		uint32_t pattern_mode:1;
		uint32_t next_pc:5;
		uint32_t rep_cnt:8;
		uint32_t frm_sel:3;
		uint32_t clk_idle:1;
		uint32_t cs_activate:1;
		uint32_t cs_deactivate:1;
		uint32_t tx_enable:1;
		uint32_t tx_fifo_sel:2;
		uint32_t tx_sidetone_on:1;
		uint32_t rx_enable:1;
		uint32_t rx_fifo_sel:2;
		uint32_t rx_sidetone_on:1;
	} chal_sspi_seq_conf_t;

/**
* SSPI core frame configure structure
*****************************************************************************/
	struct chal_sspi_frm_conf_t {
		uint32_t rx_len; /* rx data length */
		/* # of bits of delay before starting rx data */
		uint32_t rx_delay_bits;
		uint32_t tx_len; /* tx data length */
		/* # of bits to pad before the actual tx data */
		uint32_t tx_prepad_bits;
		/* # of bits to pad after the actual tx data */
		uint32_t tx_postpad_bits;
		uint32_t tx_padval; /* value of the pad bits, 1 or 0 */
};
#define chal_sspi_frm_conf_t struct chal_sspi_frm_conf_t

/**
* SSPI core configure structure
*****************************************************************************/
	typedef struct {
		uint8_t max_fifo_size;
		uint8_t num_fifo;
		uint8_t num_channnel;
		uint8_t num_cs;
		uint8_t sidtone_on;
	} chal_sspi_config_t;

/**
* SSPI globle handle structure, each SSPI keeps an instance of the structure
*****************************************************************************/
	typedef struct {
		uint32_t base;
		uint32_t num_frames; /* Max number of frames */
		uint32_t num_seqs; /* Max number of sequences */
		uint32_t num_tasks; /* Max number of tasks */
		uint32_t num_fifos; /* Max number of logical fifos */
		uint32_t fifo_maxsize; /* the size of the physical fifo */
		uint32_t num_dma; /* Max number of DMA channels for RX/TX */
		uint32_t num_cs; /* Max number of CS lines supported */
		uint32_t num_tx; /* Max number of TX/RX lines supported */
} CHAL_SSPI_HANDLE_t;

/**
* SSPI frame format extetion type
******************************************************************************/
	typedef enum {

		SSPI_FRAME_EXT_DEFAULT,
		SSPI_FRAME_EXT_SPI_HS,	/* SPI highspeed frame */
	} CHAL_SSPI_FRAME_EXT_t;
	 /**/
/**
* SSPI interrupt-enable bits
*****************************************************************************/
#define SSPIL_INTR_ENABLE_DMA_TX0 \
	(SSPIL_INTERRUPT_ENABLE_DMA_TX0_INTERRUPT_ENB_MASK)
#define SSPIL_INTR_ENABLE_DMA_RX0 \
	(SSPIL_INTERRUPT_ENABLE_DMA_RX0_INTERRUPT_ENB_MASK)
#define SSPIL_INTR_ENABLE_DMA_TX1 \
	(SSPIL_INTERRUPT_ENABLE_DMA_TX1_INTERRUPT_ENB_MASK)
#define SSPIL_INTR_ENABLE_DMA_RX1 \
	(SSPIL_INTERRUPT_ENABLE_DMA_RX1_INTERRUPT_ENB_MASK)
#define SSPIL_INTR_ENABLE_AJUMP \
	(SSPIL_INTERRUPT_ENABLE_AJUMP_INTERRUPT_ENB_MASK)
#define SSPIL_INTR_ENABLE_FIFO_OVERRUN \
	(SSPIL_INTERRUPT_ENABLE_FIFO_OVERRUN_INTERRUPT_ENB_MASK)
#define SSPIL_INTR_ENABLE_FIFO_UNDERRUN \
	(SSPIL_INTERRUPT_ENABLE_FIFO_UNDERRUN_INTERRUPT_ENB_MASK)
#define SSPIL_INTR_ENABLE_MASTER_CLKEXT_TOUT \
	(SSPIL_INTERRUPT_ENABLE_MASTER_CLKEXT_TIMEOUT_INTERRUPT_ENB_MASK)
#define SSPIL_INTR_ENABLE_SLAVE_TOUT \
	(SSPIL_INTERRUPT_ENABLE_SLAVE_TIMEOUT_INTERRUPT_ENB_MASK)
#define SSPIL_INTR_ENABLE_SLAVE_PROT \
	(SSPIL_INTERRUPT_ENABLE_SLAVE_PROTOCOL_INTERRUPT_ENB_MASK)
#define SSPIL_INTR_ENABLE_PIO_TX_START \
	(SSPIL_INTERRUPT_ENABLE_PIO_TX_START_INTERRUPT_ENB_MASK)
#define SSPIL_INTR_ENABLE_PIO_TX_STOP \
	(SSPIL_INTERRUPT_ENABLE_PIO_TX_STOP_INTERRUPT_ENB_MASK)
#define SSPIL_INTR_ENABLE_PIO_RX_START \
	(SSPIL_INTERRUPT_ENABLE_PIO_RX_START_INTERRUPT_ENB_MASK)
#define SSPIL_INTR_ENABLE_PIO_RX_STOP \
	(SSPIL_INTERRUPT_ENABLE_PIO_RX_STOP_INTERRUPT_ENB_MASK)
#define SSPIL_INTR_ENABLE_APB_TX_ERROR \
	(SSPIL_INTERRUPT_ENABLE_APB_TX_ERROR_INTERRUPT_ENB_MASK)
#define SSPIL_INTR_ENABLE_APB_RX_ERROR \
	(SSPIL_INTERRUPT_ENABLE_APB_RX_ERROR_INTERRUPT_ENB_MASK)
#define SSPIL_INTR_ENABLE_SCHEDULER \
	(SSPIL_INTERRUPT_ENABLE_SCHEDULER_INTERRUPT_ENB_MASK)
/**
* SSPI interrupt-status bits
*****************************************************************************/
#define SSPIL_INTR_STATUS_DMA_TX0 (SSPIL_INTERRUPT_STATUS_DMA_TX0_STATUS_MASK)
#define SSPIL_INTR_STATUS_DMA_RX0 (SSPIL_INTERRUPT_STATUS_DMA_RX0_STATUS_MASK)
#define SSPIL_INTR_STATUS_DMA_TX1 (SSPIL_INTERRUPT_STATUS_DMA_TX1_STATUS_MASK)
#define SSPIL_INTR_STATUS_DMA_RX1 (SSPIL_INTERRUPT_STATUS_DMA_RX1_STATUS_MASK)
#define SSPIL_INTR_STATUS_AJUMP   (SSPIL_INTERRUPT_STATUS_AJUMP_STATUS_MASK)
#define SSPIL_INTR_STATUS_FIFO_OVERRUN \
	(SSPIL_INTERRUPT_STATUS_FIFO_OVERRUN_STATUS_MASK)
#define SSPIL_INTR_STATUS_FIFO_UNDERRUN \
	(SSPIL_INTERRUPT_STATUS_FIFO_UNDERRUN_STATUS_MASK)
#define SSPIL_INTR_STATUS_MASTER_CLKEXT_TOUT \
	(SSPIL_INTERRUPT_STATUS_MASTER_CLKEXT_TIMEOUT_STATUS_MASK)
#define SSPIL_INTR_STATUS_SLAVE_TOUT \
	(SSPIL_INTERRUPT_STATUS_SLAVE_TIMEOUT_STATUS_MASK)
#define SSPIL_INTR_STATUS_SLAVE_PROT \
	(SSPIL_INTERRUPT_STATUS_SLAVE_PROTOCOL_STATUS_MASK)
#define SSPIL_INTR_STATUS_PIO_TX_START \
	(SSPIL_INTERRUPT_STATUS_PIO_TX_START_STATUS_MASK)
#define SSPIL_INTR_STATUS_PIO_TX_STOP \
	(SSPIL_INTERRUPT_STATUS_PIO_TX_STOP_STATUS_MASK)
#define SSPIL_INTR_STATUS_PIO_RX_START \
	(SSPIL_INTERRUPT_STATUS_PIO_RX_START_STATUS_MASK)
#define SSPIL_INTR_STATUS_PIO_RX_STOP \
	(SSPIL_INTERRUPT_STATUS_PIO_RX_STOP_STATUS_MASK)
#define SSPIL_INTR_STATUS_APB_TX_ERROR \
	(SSPIL_INTERRUPT_STATUS_APB_TX_ERROR_ACCESS_STATUS_MASK)
#define SSPIL_INTR_STATUS_APB_RX_ERROR \
	(SSPIL_INTERRUPT_STATUS_APB_RX_ERROR_ACCESS_STATUS_MASK)
#define SSPIL_INTR_STATUS_SCHEDULER \
	(SSPIL_INTERRUPT_STATUS_SCHEDULER_STATUS_MASK)
#define SSPIL_INTR_STATUS_COPY_SSPI_INT \
	(SSPIL_INTERRUPT_STATUS_COPY_SSPI_INT_MASK)
#define SSPIL_DETAIL_INTR_STS_FIFO_OVERRUN_RX0 \
	(1 << (SSPIL_DETAIL_INTERRUPT_STATUS_FIFO_OVERRUN_VECTOR_SHIFT))
#define SSPIL_DETAIL_INTR_STS_FIFO_OVERRUN_RX1 \
	(1 << ((SSPIL_DETAIL_INTERRUPT_STATUS_FIFO_OVERRUN_VECTOR_SHIFT)+1))
#define SSPIL_DETAIL_INTR_STS_FIFO_OVERRUN_RX2 \
	(1 << ((SSPIL_DETAIL_INTERRUPT_STATUS_FIFO_OVERRUN_VECTOR_SHIFT)+2))
#define SSPIL_DETAIL_INTR_STS_FIFO_OVERRUN_RX3 \
	(1U << ((SSPIL_DETAIL_INTERRUPT_STATUS_FIFO_OVERRUN_VECTOR_SHIFT)+3))
#define SSPIL_DETAIL_INTR_STS_FIFO_UNDERRUN_TX0 \
	(1 << (SSPIL_DETAIL_INTERRUPT_STATUS_FIFO_UNDERRUN_VECTOR_SHIFT))
#define SSPIL_DETAIL_INTR_STS_FIFO_UNDERRUN_TX1 \
	(1 << ((SSPIL_DETAIL_INTERRUPT_STATUS_FIFO_UNDERRUN_VECTOR_SHIFT)+1))
#define SSPIL_DETAIL_INTR_STS_FIFO_UNDERRUN_TX2 \
	(1 << ((SSPIL_DETAIL_INTERRUPT_STATUS_FIFO_UNDERRUN_VECTOR_SHIFT)+2))
#define SSPIL_DETAIL_INTR_STS_FIFO_UNDERRUN_TX3 \
	(1 << ((SSPIL_DETAIL_INTERRUPT_STATUS_FIFO_UNDERRUN_VECTOR_SHIFT)+3))
#define SSPIL_DETAIL_INTR_STS_PIO_TX_START_TX0 \
	(1 << (SSPIL_DETAIL_INTERRUPT_STATUS_PIO_TX_START_VECTOR_SHIFT))
#define SSPIL_DETAIL_INTR_STS_PIO_TX_START_TX1 \
	(1 << ((SSPIL_DETAIL_INTERRUPT_STATUS_PIO_TX_START_VECTOR_SHIFT)+1))
#define SSPIL_DETAIL_INTR_STS_PIO_TX_START_TX2 \
	(1 << ((SSPIL_DETAIL_INTERRUPT_STATUS_PIO_TX_START_VECTOR_SHIFT)+2))
#define SSPIL_DETAIL_INTR_STS_PIO_TX_START_TX3 \
	(1 << ((SSPIL_DETAIL_INTERRUPT_STATUS_PIO_TX_START_VECTOR_SHIFT)+3))
#define SSPIL_DETAIL_INTR_STS_PIO_TX_STOP_TX0 \
	(1 << (SSPIL_DETAIL_INTERRUPT_STATUS_PIO_TX_STOP_VECTOR_SHIFT))
#define SSPIL_DETAIL_INTR_STS_PIO_TX_STOP_TX1 \
	(1 << ((SSPIL_DETAIL_INTERRUPT_STATUS_PIO_TX_STOP_VECTOR_SHIFT)+1))
#define SSPIL_DETAIL_INTR_STS_PIO_TX_STOP_TX2 \
	(1 << ((SSPIL_DETAIL_INTERRUPT_STATUS_PIO_TX_STOP_VECTOR_SHIFT)+2))
#define SSPIL_DETAIL_INTR_STS_PIO_TX_STOP_TX3 \
	(1 << ((SSPIL_DETAIL_INTERRUPT_STATUS_PIO_TX_STOP_VECTOR_SHIFT)+3))
#define SSPIL_DETAIL_INTR_STS_PIO_RX_START_RX0 \
	(1 << (SSPIL_DETAIL_INTERRUPT_STATUS_PIO_RX_START_VECTOR_SHIFT))
#define SSPIL_DETAIL_INTR_STS_PIO_RX_START_RX1 \
	(1 << ((SSPIL_DETAIL_INTERRUPT_STATUS_PIO_RX_START_VECTOR_SHIFT)+1))
#define SSPIL_DETAIL_INTR_STS_PIO_RX_START_RX2 \
	(1 << ((SSPIL_DETAIL_INTERRUPT_STATUS_PIO_RX_START_VECTOR_SHIFT)+2))
#define SSPIL_DETAIL_INTR_STS_PIO_RX_START_RX3 \
	(1 << ((SSPIL_DETAIL_INTERRUPT_STATUS_PIO_RX_START_VECTOR_SHIFT)+3))
#define SSPIL_DETAIL_INTR_STS_PIO_RX_STOP_RX0 \
	(1 << (SSPIL_DETAIL_INTERRUPT_STATUS_PIO_RX_STOP_VECTOR_SHIFT))
#define SSPIL_DETAIL_INTR_STS_PIO_RX_STOP_RX1 \
	(1 << ((SSPIL_DETAIL_INTERRUPT_STATUS_PIO_RX_STOP_VECTOR_SHIFT)+1))
#define SSPIL_DETAIL_INTR_STS_PIO_RX_STOP_RX2 \
	(1 << ((SSPIL_DETAIL_INTERRUPT_STATUS_PIO_RX_STOP_VECTOR_SHIFT)+2))
#define SSPIL_DETAIL_INTR_STS_PIO_RX_STOP_RX3 \
	(1 << ((SSPIL_DETAIL_INTERRUPT_STATUS_PIO_RX_STOP_VECTOR_SHIFT)+3))
#define SSPIL_DETAIL_INTR_STS_APB_TX_ERR_TX0 \
	(1 << (SSPIL_DETAIL_INTERRUPT_STATUS_APB_TX_ERROR_ACCESS_VECTOR_SHIFT))
#define SSPIL_DETAIL_INTR_STS_APB_TX_ERR_TX1 \
	(1 << \
	((SSPIL_DETAIL_INTERRUPT_STATUS_APB_TX_ERROR_ACCESS_VECTOR_SHIFT)+1))
#define SSPIL_DETAIL_INTR_STS_APB_TX_ERR_TX2 \
	(1 << \
	((SSPIL_DETAIL_INTERRUPT_STATUS_APB_TX_ERROR_ACCESS_VECTOR_SHIFT)+2))
#define SSPIL_DETAIL_INTR_STS_APB_TX_ERR_TX3 \
	(1 << \
	((SSPIL_DETAIL_INTERRUPT_STATUS_APB_TX_ERROR_ACCESS_VECTOR_SHIFT)+3))
#define SSPIL_DETAIL_INTR_STS_APB_RX_ERR_RX0 \
	(1 << \
	(SSPIL_DETAIL_INTERRUPT_STATUS_APB_RX_ERROR_ACCESS_VECTOR_SHIFT))
#define SSPIL_DETAIL_INTR_STS_APB_RX_ERR_RX1 \
	(1 << \
	((SSPIL_DETAIL_INTERRUPT_STATUS_APB_RX_ERROR_ACCESS_VECTOR_SHIFT)+1))
#define SSPIL_DETAIL_INTR_STS_APB_RX_ERR_RX2 \
	(1 << \
	((SSPIL_DETAIL_INTERRUPT_STATUS_APB_RX_ERROR_ACCESS_VECTOR_SHIFT)+2))
#define SSPIL_DETAIL_INTR_STS_APB_RX_ERR_RX3 \
	(1 << \
	((SSPIL_DETAIL_INTERRUPT_STATUS_APB_RX_ERROR_ACCESS_VECTOR_SHIFT)+3))
/**
*
*  @brief  Initialize SSPI CHAL internal data structure
*
*  @param    base_addr (in) mapped address of the SSPI core to be initialized
*
*  @return CHAL handle for this SSPI core instance
*****************************************************************************/
	 CHAL_HANDLE chal_sspi_init(uint32_t baseAddr);

/**
*  @brief  De-initialize SSPI CHAL internal data structure
*
*  @param    handle    (in) SSPI CHAL handle to be deinitialized
*
*  @return SSPI status
*****************************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_deinit(CHAL_HANDLE handle);

/**
*
*  @brief  Get the maximum physical FIFO size
*
*  @param    handle    (in) SSPI CHAL handle
*  @param    fifo_size (out) the physical fifo size for the SSPI core
*
*  @return SSPI status
******************************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_get_max_fifo_size(CHAL_HANDLE handle,
					    uint32_t *fifo_size);

/**
*  @brief   Returns address offset of the FIFO from the base address
*
*  @param   None
*
*  @return  Address offset value
*****************************************************************************/
	uint32_t chal_sspi_rx0_get_dma_port_addr_offset(void);

/**
*  @brief   Returns address offset of the FIFO from the base address
*
*  @param   None
*
*  @return  Address offset value
*****************************************************************************/
	uint32_t chal_sspi_rx1_get_dma_port_addr_offset(void);

/**
*  @brief   Returns address offset of the FIFO from the base address
*
*  @param   None
*
*  @return  Address offset value
*****************************************************************************/
	uint32_t chal_sspi_rx2_get_dma_port_addr_offset(void);

/**
*  @brief   Returns address offset of the FIFO from the base address
*
*  @param   None
*
*  @return  Address offset value
*****************************************************************************/
	uint32_t chal_sspi_rx3_get_dma_port_addr_offset(void);

/**
*  @brief   Returns address offset of the FIFO from the base address
*
*  @param   None
*
*  @return  Address offset value
*****************************************************************************/
	uint32_t chal_sspi_tx0_get_dma_port_addr_offset(void);

/**
*  @brief   Returns address offset of the FIFO from the base address
*
*  @param   None
*
*  @return  Address offset value
*****************************************************************************/
	uint32_t chal_sspi_tx1_get_dma_port_addr_offset(void);

/**
*  @brief   Returns address offset of the FIFO from the base address
*
*  @param   None
*
*  @return  Address offset value
*****************************************************************************/
	uint32_t chal_sspi_tx2_get_dma_port_addr_offset(void);

/**
*  @brief   Returns address offset of the FIFO from the base address
*
*  @param   None
*
*  @return  Address offset value
*****************************************************************************/
	uint32_t chal_sspi_tx3_get_dma_port_addr_offset(void);


/**
*
*  @brief  Set SSPI mode
*
*  @param    handle (in) SSPI CHAL handle
*  @param    mode   (in) SSPI core operation mode (master or slave)
*
*  @return SSPI status
*******************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_set_mode(CHAL_HANDLE handle,
					      CHAL_SSPI_MODE_t mode);

/**
*
*  @brief  Enalble/Disable SSPI core
*
*  @param    handle (in) SSPI CHAL handle
*  @param    off_on (in) Enable -1, Disable - 0
*
*  @return SSPI status
********************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_enable(CHAL_HANDLE handle,
					    uint32_t off_on);

/**
*
*  @brief  Soft reset SSPI core
*
*  @param    handle (in) SSPI CHAL handle
*
*  @return SSPI status
*****************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_soft_reset(CHAL_HANDLE handle);

/**
*
*  @brief  Enalble/Disable sidetone functionality
*
*  @param    handle (in) SSPI CHAL handle
*  @param    off_on (in) Enable -1, Disable - 0
*
*  @return SSPI status
********************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_enable_sidetone(CHAL_HANDLE handle,
						     uint32_t off_on);

/**
*
*  @brief  Enalble/Disable DMA data transfer
*
*  @param    handle  (in) SSPI CHAL handle
*  @param    channel (in) the channel number of which DMA is set. SSPI(Full or
*                         Lite) has two channels for Rx and two channels for Tx
*  @param    fifo_id (in) the logical FIFO, of which DMA is set
*  @param    off_on  (in) Enable -1, Disable - 0
*
*  @return SSPI status
***********************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_enable_dma(CHAL_HANDLE handle,
						CHAL_SSPI_DMA_CHAN_SEL_t
						channel,
						CHAL_SSPI_FIFO_ID_t fifo_id,
						uint32_t off_on);

/**
*
*  @brief  Set the minimum size of each dma request
*
*  @param    handle  (in) SSPI CHAL handle
*  @param    channel (in) the channel number of which DMA is set. SSPI(Full or
*                         Lite) has two channels for Rx and two channels for Tx
*  @param    size    (in) minimum size of each dma burst
*
*  @return SSPI status
*****************************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_dma_set_burstsize(CHAL_HANDLE handle,
						       CHAL_SSPI_DMA_CHAN_SEL_t
						       channel,
						       CHAL_SSPI_DMA_BURSTSIZE_t
						       size);

/**
*
*  @brief  Send dma request for remaining words, even when burst size not met
*
*  @param    handle  (in) SSPI CHAL handle
*  @param    channel (in) the channel number of which DMA is set. SSPI(Full or
*                         Lite) has two channels for Rx and two channels for Tx
*
*  @return SSPI status
***********************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_dma_flush(CHAL_HANDLE handle,
					       CHAL_SSPI_DMA_CHAN_SEL_t
					       channel);

/**
*
*  @brief  Set the number of words to transfer
*
*  @param    handle  (in) SSPI CHAL handle
*  @param    channel (in) the channel number of which DMA is set. SSPI(Full or
*                         Lite) has two channels for Rx and two channels for Tx
*  @param    size    (in) the number words t transfer
*
*  @return SSPI status
***********************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_dma_set_tx_size(CHAL_HANDLE handle,
						     CHAL_SSPI_DMA_CHAN_SEL_t
						     channel, uint32_t size);

/**
*
*  @brief  Enalble/Disable DMA data transfer
*
*  @param    handle  (in) SSPI CHAL handle
*  @param    channel (in) the channel number of which DMA is set. SSPI(Full or
*                         Lite) has two channels for Rx and two channels for Tx
*  @param    fifo_id (out) the logical FIFO, of which DMA is set
*  @param    off_on  (out) Enable -1, Disable - 0
*
*  @return SSPI status
***********************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_get_dma_status(CHAL_HANDLE handle,
						    CHAL_SSPI_DMA_CHAN_SEL_t
						    channel,
						    CHAL_SSPI_FIFO_ID_t
						    *fifo_id,
						    uint32_t *off_on);

/**
*
*  @brief  Set the idle state values to be driven on CS, CLK, TX, RX lines
*          until a task initializes them
*
*  @param    handle  (in) SSPI CHAL handle
*  @param    prot    (in) protocol type, on which the idle values are set based
*
*  @return SSPI status
******************************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_set_idle_state(CHAL_HANDLE handle,
						    CHAL_SSPI_PROT_t prot);

/**
*
*  @brief  Configure a frame definition for I2C in the the frame table
*
*  @param    handle      (in) SSPI CHAL handle
*  @param    *frm_bitmap (io) the in parameter is the bitmap of the
*                                 available frames for CHAL to use. The out
*                                 parameter is the bitmap of the frames that
*                                 CHAL actually programmed.
*  @param    word_len    (in) length of word for the specified protocol
*
*  @return SSPI status
******************************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_set_i2c_frame(CHAL_HANDLE handle,
						    uint32_t *frm_bitmap,
							uint32_t word_len);

/**
*
*  @brief  Configure a frame definition for SPI in the the frame table
*
*  @param    handle      (in) SSPI CHAL handle
*  @param    *frm_bitmap (io) the in parameter is the bitmap of the
*                                 available frames for CHAL to use. The out
*                                 parameter is the bitmap of the frames that
*                                 CHAL actually programmed.
*  @param    prot        (in) protocol type, on which the frame definition is
*                             set based
*  @param    word_len    (in) length of word for the specified protocol
*
*  @return SSPI status
******************************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_set_spi_frame(CHAL_HANDLE handle,
					uint32_t *frm_bitmap,
					CHAL_SSPI_PROT_t prot,
					uint32_t word_len,
					CHAL_SSPI_FRAME_EXT_t ext_setup);


/**
*
*  @brief  Configure a frame definition for I2S in the the frame table
*
*  @param    handle      (in) SSPI CHAL handle
*  @param    *frm_bitmap (io) the in parameter is the bitmap of the
*                                 available frames for CHAL to use. The out
*                                 parameter is the bitmap of the frames that
*                                 CHAL actually programmed.
*  @param    prot        (in) protocol type, on which the frame definition is
*                             set based
*  @param    word_len    (in) length of word for the specified protocol
*
*  @return SSPI status
******************************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_set_i2s_frame(CHAL_HANDLE handle,
						uint32_t *frm_bitmap,
						CHAL_SSPI_PROT_t prot,
						chal_sspi_frm_conf_t frm_conf);

/**
*
*  @brief  Configure a frame definition for PCM in the the frame table
*
*  @param    handle      (in) SSPI CHAL handle
*  @param    *frm_bitmap (io) the in parameter is the bitmap of the
*                                 available frames for CHAL to use. The out
*                                 parameter is the bitmap of the frames that
*                                 CHAL actually programmed.
*  @param    prot        (in) protocol type, on which the frame definition is
*                             set based
*  @param    word_len    (in) length of word for the specified protocol
*  @param    ext_bits    (in) dummy-bit period before or after transferring
*                             the valid data bits in the frame. This parameter
*                             is used for Left/Right justified Audio protocol,
*                             I2S Extended Mode, PCM Extended Mode and
*                             3-channel TDM/PCM mode. It is ignored for all
*                             other protocols.
*
*  @return SSPI status
******************************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_set_pcm_frame(CHAL_HANDLE handle,
						uint32_t *frm_bitmap,
						chal_sspi_frm_conf_t *frm_conf);


/**
*
*  @brief  Configure a frame definition in the the frame table
*
*  @param    handle   (in) SSPI CHAL handle
*  @param    patt_idx (in) index of the pattern entry to be configured
*  @param    type     (in) pattern type, on which the pattern definition is
*                          set based
*
*  @return SSPI status
******************************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_set_pattern(CHAL_HANDLE handle,
						 uint32_t patt_idx,
						 CHAL_SSPI_PATT_TYPE_t type);

/**
*
*  @brief  Configure a sequence instruction in the the sequencer table
*
*  @param    handle  (in) SSPI CHAL handle
*  @param    seq_idx (in) index of the sequence entry to be configured
*  @param    prot    (in) protocol type, on which the sequence instruction is
*                         set based
*
*  @return SSPI status
******************************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_set_sequence(CHAL_HANDLE handle,
						  uint32_t seq_idx,
						  CHAL_SSPI_PROT_t prot,
						  chal_sspi_seq_conf_t
						  *seq_conf);

/**
*
*  @brief  Configure a task in the the task table for task scheduler to execute
*
*  @param    handle   (in) SSPI CHAL handle
*  @param    task_idx (in) index of the task entry to be configured
*  @param    prot     (in) protocol type, on which the task is set based
*
*  @return SSPI status
******************************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_set_task(CHAL_HANDLE handle,
					      uint32_t task_idx,
					      CHAL_SSPI_PROT_t prot,
					      chal_sspi_task_conf_t *tk_conf);

/**
*
*  @brief  Setup divisor value for the clock dividers in SSPI core
*
*  @param    handle      (in) SSPI CHAL handle
*  @param    clk_divider (in) the clk divider to be configured
*  @param    clk_div     (in) the divisor value used for the specified divider
*
*  @return SSPI status
******************************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_set_clk_divider(CHAL_HANDLE handle,
						     CHAL_SSPI_CLK_DIVIDER_t
						     clk_divider,
						     uint32_t clk_div);

/**
*
*  @brief  Get the currently configured divisor value of a clock divider in
*          SSPI core
*
*  @param    handle      (in) SSPI CHAL handle
*  @param    clk_divider (in) the clk divider to get the divisor value
*  @param    clk_div     (out) the divisor value used for the specified divider
*
*  @return SSPI status
******************************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_get_clk_divider(CHAL_HANDLE handle,
						     CHAL_SSPI_CLK_DIVIDER_t
						     clk_divider,
						     uint32_t *clk_div);

/**
*
*  @brief  Select to use one of the three SSPI clock source
*
*  @param    handle  (in) SSPI CHAL handle
*  @param    clk_src (in) the clk source to select
*
*  @return SSPI status
*****************************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_set_clk_src_select(CHAL_HANDLE handle,
							CHAL_SSPI_CLK_SRC_t
							clk_src);

/**
*
*  @brief  Get the currently selected SSPI clock clock source
*
*  @param    handle  (in) SSPI CHAL handle
*  @param    clk_src (out) the clk source currently selected
*
*  @return SSPI status
******************************************************************************/

	CHAL_SSPI_STATUS_t chal_sspi_get_clk_src_select(CHAL_HANDLE handle,
							uint32_t *clk_src);

/**
*
*  @brief  Set the SSPI clock, which is generated from CAPH clock
*
*  @param    handle         (in) SSPI CHAL handle
*  @param    sample_rate       (in) the sample transfer rate
*  @param    sample_len       (in) the number of bits of each sample. If there
*    multiple channels per sample, sample_len is the sum of all channel length.
*
*  @return SSPI status
******************************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_set_caph_clk(CHAL_HANDLE handle,
						uint32_t sample_rate,
						uint32_t sample_len);

/* FIFO control APIs */

/**
*
*  @brief  Setup a logical FIFO size to one of four different
*          size (full, half, quarter or none of the physical FIFO size)
*
*  @param    handle    (in) SSPI CHAL handle
*  @param    fifo_id   (in) the logical FIFO, of which the size is set
*  @param    fifo_size (in) the FIFO size to set
*
*  @return SSPI status
******************************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_set_fifo_size(CHAL_HANDLE handle,
						   CHAL_SSPI_FIFO_ID_t fifo_id,
						   CHAL_SSPI_FIFO_SIZE_t
						   fifo_size);

/**
*
*  @brief  Get the currently configured size of a logical FIFO
*          or none of the physical FIFO size)
*
*  @param    handle    (in) SSPI CHAL handle
*  @param    fifo_id   (in) the logical FIFO to get the fifo size
*  @param    fifo_size (out) the currently configured FIFO size
*
*  @return SSPI status
******************************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_get_fifo_size(CHAL_HANDLE handle,
						   CHAL_SSPI_FIFO_ID_t fifo_id,
						   uint32_t *fifo_size);

/**
*
*  @brief  Setup the endianess used to pack data in a logical FIFO
*
*  @param    handle      (in) SSPI CHAL handle
*  @param    fifo_id     (in) the logical FIFO, of which the endianess is set
*  @param    fifo_endian (in) the endianness used in the logical FIFO
*
*  @return SSPI status
******************************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_set_fifo_endian(CHAL_HANDLE handle,
						     CHAL_SSPI_FIFO_ID_t
						     fifo_id,
						     CHAL_SSPI_FIFO_ENDIAN_t
						     fifo_endian);

/**
*
*  @brief  Get the endianess currently used to pack data in a logical FIFO
*
*  @param    handle      (in) SSPI CHAL handle
*  @param    fifo_id     (in) the logical FIFO to get the endianess
*  @param    fifo_endian (out) the endianness curently used in the logical FIFO
*
*  @return SSPI status
*****************************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_get_fifo_endian(CHAL_HANDLE handle,
						     CHAL_SSPI_FIFO_ID_t
						     fifo_id,
						     uint32_t *fifo_endian);

/**
*
*  @brief  Setup the FIFO threshold (# of bytes), which can be used by a task
*          to identify the initial condition before starting any tranaction.
*
*  @param    handle     (in) SSPI CHAL handle
*  @param    fifo_id    (in) the logical FIFO, of which the threshold is set
*  @param    fifo_thres (in) the threshold value to be set
*
*  @return SSPI status
*****************************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_set_fifo_threshold(CHAL_HANDLE handle,
							CHAL_SSPI_FIFO_ID_t
							fifo_id,
							uint32_t fifo_thres);

/**
*
*  @brief  Get the currently configured FIFO threshold, which can be used by a
*          task to identify the initial condition before starting any
*          tranaction.
*
*  @param    handle     (in) SSPI CHAL handle
*  @param    fifo_id    (in) the logical FIFO to get the threshold
*  @param    fifo_thres (out) the currently configured threshold value
*
*  @return SSPI status
******************************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_get_fifo_threshold(CHAL_HANDLE handle,
							CHAL_SSPI_FIFO_ID_t
							fifo_id,
							uint32_t *fifo_thres);

/**
*
*  @brief  Setup one of the three ways (unpack, 8-bit or 16-bit) to pack data
*          in the logical FIFO
*
*  @param    handle    (in) SSPI CHAL handle
*  @param    fifo_id   (in) the logical FIFO to configure
*  @param    fifo_pack (in) the data-packing way to set for the FIFO
*
*  @return SSPI status
******************************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_set_fifo_pack(CHAL_HANDLE handle,
						   CHAL_SSPI_FIFO_ID_t fifo_id,
						   CHAL_SSPI_FIFO_DATA_PACK_t
						   fifo_pack);

/**
*
*  @brief  Get the currently configured data-packing way for the specified
*          logical FIFO
*
*  @param    handle    (in) SSPI CHAL handle
*  @param    fifo_id   (in) the logical FIFO to get the data-packing way
*  @param    fifo_pack (out) the data-packing way currently configured
*
*  @return SSPI status
******************************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_get_fifo_pack(CHAL_HANDLE handle,
						   CHAL_SSPI_FIFO_ID_t fifo_id,
						   uint32_t *fifo_pack);

/**
*
*  @brief  Setup one of the three ways (32-bit, 16-bit or 8-bit) for data size
*          in the logical FIFO
*
*  @param    handle    (in) SSPI CHAL handle
*  @param    fifo_id   (in) the logical FIFO to configure
*  @param    fifo_size (in) the data-size way to set for the FIFO
*
*  @return SSPI status
******************************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_set_fifo_data_size(CHAL_HANDLE handle,
							CHAL_SSPI_FIFO_ID_t
							fifo_id,
							CHAL_SSPI_FIFO_DATA_SIZE_t
							fifo_size);

/**
*
*  @brief  Get the currently configured data-size way for the specified
*          logical FIFO
*
*  @param    handle    (in) SSPI CHAL handle
*  @param    fifo_id   (in) the logical FIFO to get the data-packing way
*  @param    fifo_size (out) the data-size way currently configured
*
*  @return SSPI status
******************************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_get_fifo_data_size(CHAL_HANDLE handle,
							CHAL_SSPI_FIFO_ID_t
							fifo_id,
							uint32_t *fifo_size);

/**
*
*  @brief  Reset a logical FIFO
*
*  @param    handle    (in) SSPI CHAL handle
*  @param    fifo_id   (in) the logical FIFO to reset
*
*  @return SSPI status
******************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_fifo_reset(CHAL_HANDLE handle,
						CHAL_SSPI_FIFO_ID_t fifo_id);

/**
*
*  @brief  Setup a repeat count to READ/WRITE the FIFO data multiple times
*
*  @param    handle    (in) SSPI CHAL handle
*  @param    fifo_id   (in) the logical FIFO, of which the size is set
*  @param    fifo_rcnt (in) the repeat count to READ/WRITE FIFO data
*                           multiple times
*
*  @return SSPI status
******************************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_set_fifo_repeat_count(CHAL_HANDLE handle,
							   CHAL_SSPI_FIFO_ID_t
							   fifo_id,
							   uint8_t fifo_rcnt);

/**
*
*  @brief  Get the repeat count that READ/WRITE the FIFO data multiple times
*
*  @param    handle    (in)  SSPI CHAL handle
*  @param    fifo_id   (in)  the logical FIFO, of which the size is set
*  @param    fifo_rcnt (out) the repeat count to READ/WRITE FIFO data
*                            multiple times
*
*  @return SSPI status
******************************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_get_fifo_repeat_count(CHAL_HANDLE handle,
							   CHAL_SSPI_FIFO_ID_t
							   fifo_id,
							   uint8_t *
							   fifo_rcnt);

/**
*
*  @brief  For Rx FIFOs, get the # of bytes currently in the FIFO
*          For Tx FIFOs, get the # of free bytes currently in the FIFO
*
*  @param    handle     (in)  SSPI CHAL handle
*  @param    fifo_id    (in)  the logical FIFO, of which the size is set
*  @param    fifo_level (out) the # of bytes in the READ/WRITE FIFO
*
*  @return SSPI status
******************************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_get_fifo_level(CHAL_HANDLE handle,
						    CHAL_SSPI_FIFO_ID_t
						    fifo_id,
						    uint16_t *fifo_level);

/**
*
*  @brief  Check if the specified FIFO is full or not.
*
*  @param    handle     (in)  SSPI CHAL handle
*  @param    fifo_id    (in)  the logical FIFO, of which the size is set
*
*  @return   1 if the fifo is full, otherwise 0.
******************************************************************************/
	_Bool chal_sspi_check_fifo_full(CHAL_HANDLE handle,
					CHAL_SSPI_FIFO_ID_t fifo_id);

/**
*
*  @brief  Check if the specified FIFO is empty or not.
*
*  @param    handle     (in)  SSPI CHAL handle
*  @param    fifo_id    (in)  the logical FIFO, of which the size is set
*
*  @return   1 if the fifo is empty, otherwise 0.
******************************************************************************/
	_Bool chal_sspi_check_fifo_empty(CHAL_HANDLE handle,
					 CHAL_SSPI_FIFO_ID_t fifo_id);

/**
*
*  @brief  Setup PIO start and stop threshhold
*
*  @param    handle      (in)  SSPI CHAL handle
*  @param    fifo_id     (in)  the logical FIFO, of which the size is set
*  @param    start_thres (in)  the minimum number of words inside fifo needed
*                              to generate a pio_start_interrupt and start a
*                              thresmet generation.
*  @param    stop_thres  (in)  the minimum number of free words needed to
*                              stop a thresmet generation
*
*  @return SSPI status
******************************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_set_fifo_pio_threshhold(CHAL_HANDLE
							     handle,
							     CHAL_SSPI_FIFO_ID_t
							     fifo_id,
							     uint32_t
							     start_thres,
							     uint32_t
							     stop_thres);

/**
*
*  @brief  Get PIO start and stop threshhold
*
*  @param    handle      (in)  SSPI CHAL handle
*  @param    fifo_id     (in)  the logical FIFO, of which the size is set
*  @param    start_thres (out) the minimum number of words inside fifo needed
*                              to generate a pio_start_interrupt and start a
*                              thresmet generation.
*  @param    stop_thres  (out) the minimum number of free words needed to
*                              stop a thresmet generation
*
*  @return SSPI status
******************************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_get_fifo_pio_threshhold(CHAL_HANDLE
							     handle,
							     CHAL_SSPI_FIFO_ID_t
							     fifo_id,
							     uint32_t *
							     start_thres,
							     uint32_t *
							     stop_thres);

/**
*
*  @brief  Enable/Disable PIO start and stop Interrupts
*
*  @param    handle   (in)  SSPI CHAL handle
*  @param    fifo_id  (in)  the logical FIFO, of which the size is set
*  @param    start_en (in)  Enable/Disable PIO start interrupt
*                           Enable - 1, Disable - 0
*  @param    stop_en (in)   Enable/Disable PIO stop interrupt
*                           Enable - 1, Disable - 0
*
*  @return SSPI status
******************************************************************************/
	CHAL_SSPI_STATUS_t
	    chal_sspi_enable_fifo_pio_start_stop_intr(CHAL_HANDLE handle,
						      CHAL_SSPI_FIFO_ID_t
						      fifo_id,
						      uint32_t start_en,
						      uint32_t stop_en);

/**
*
*  @brief  Read data from a logical RX FIFO and copy the data a buffer
*          pointed by buf;
*
*  @param    handle  (in) SSPI CHAL handle
*  @param    fifo_id (in) the logical FIFO to read
*  @param    prot    (in) protocol type, over which the data is transferred
*  @param    buf     (out) the buffer to hold the receiving data
*  @param    size    (in) the number of bytes to receive
*
*  @return  size     Number of bytes read. Return 0 if any errors occur
*************************************************************************/
	uint32_t chal_sspi_read_data(CHAL_HANDLE handle,
				     CHAL_SSPI_FIFO_ID_t fifo_id,
				     CHAL_SSPI_PROT_t prot,
				     uint8_t *buf, uint32_t size);

/**
*
*  @brief  Copy data from a buffer pointed by buf to a logical TX FIFO
*
*  @param    handle  (in) SSPI CHAL handle
*  @param    fifo_id (in) the logical FIFO to write
*  @param    prot    (in) protocol type, over which the data is transferred
*  @param    buf     (in) the buffer to hold the transmitting data
*  @param    size    (in) the number of bytes to transmit
*
*  @return  size     Number of bytes written. Return 0 if any errors occur
*************************************************************************/
	uint32_t chal_sspi_write_data(CHAL_HANDLE handle,
				      CHAL_SSPI_FIFO_ID_t fifo_id,
				      CHAL_SSPI_PROT_t prot,
				      uint8_t *buf, uint32_t size);

/**
*
*  @brief  Read one data entry from a logical RX FIFO.
*
*  @param    handle  (in) SSPI CHAL handle
*  @param    fifo_id (in) the logical FIFO to read
*  @param    data    (out) the buffer to hold the receiving data
*
*  @return SSPI status
*************************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_read_fifo(CHAL_HANDLE handle,
					       CHAL_SSPI_FIFO_ID_t fifo_id,
					       uint32_t *data);

/**
*
*  @brief  Write one data entry to a logical TX FIFO
*
*  @param    handle  (in) SSPI CHAL handle
*  @param    fifo_id (in) the logical FIFO to write
*  @param    data    (in) the data entry to write in the fifo
*
*  @return SSPI status
*************************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_write_fifo(CHAL_HANDLE handle,
						CHAL_SSPI_FIFO_ID_t fifo_id,
						uint32_t data);

/**
*
*  @brief  Enalble/Disable SSPI interrups
*
*  @param    handle    (in) SSPI CHAL handle
*  @param    intr_mask (in) Enable - if the corresponding bit of intr_mask
*                          of is set.
*                          Disable - if the corresponding bit of intr_mask
*                          of is reset
*
*  @return SSPI status
***********************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_enable_intr(CHAL_HANDLE handle,
						 uint32_t intr_mask);

/**
*
*  @brief  Enalble/Disable SSPI error interrups
*
*  @param    handle    (in) SSPI CHAL handle
*  @param    intr_mask (in) Enable - if the corresponding bit of intr_mask
*                          of is set.
*                          Disable - if the corresponding bit of intr_mask
*                          of is reset
*
*  @return SSPI status
***********************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_enable_error_intr(CHAL_HANDLE handle,
						       uint32_t intr_mask);

/**
*
*  @brief  Get current SSPI interrupt enable mask
*
*  @param    handle     (in)  SSPI CHAL handle
*  @param    intr_mask  (out) Interrupt mask. It is a bitmask shows which SSPI
*                             interrupts are enabled currently
*
*  @return SSPI status
***********************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_get_intr_mask(CHAL_HANDLE handle,
						   uint32_t *intr_mask);

/**
*
*  @brief  Get current SSPI error interrupt enable mask
*
*  @param    handle     (in)  SSPI CHAL handle
*  @param    intr_mask  (out) Interrupt mask. It is a bitmask shows which SSPI
*                             error interrupts are enabled currently
*
*  @return SSPI status
***********************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_get_error_intr_mask(CHAL_HANDLE handle,
							 uint32_t *intr_mask);

/**
*
*  @brief  Get current SSPI interrupt status
*
*  @param    handle     (in)  SSPI CHAL handle
*  @param    status     (out) Interrupt status. It is a bitmask shows what kind
*                             of SSPI interrupts received
*  @param    det_status (out) Detailed interrupt status. It is a bitmask shows
*                             the detailed info of SSPI interrupts received
*
*  @return SSPI status
***********************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_get_intr_status(CHAL_HANDLE handle,
						     uint32_t *status,
						     uint32_t *det_status);

/**
*
*  @brief  Clear SSPI interrups
*
*  @param    handle        (in) SSPI CHAL handle
*  @param    intr_mask     (in) Interrupt mask
*                               - Clear the interrupt bits based on intr_mask.
*  @param    det_intr_mask (in) Detailed interrupt mask
*                               - Clear the interrupt bits based on
*                                 det_intr_mask. Some interrupt bits can not be
*                                 cleared before the related bits are cleared
*                                 first in detailed interrupt status register.
*
*  @return SSPI status
***********************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_clear_intr(CHAL_HANDLE handle,
						uint32_t intr_mask,
						uint32_t det_intr_mask);

/**
*
*  @brief  Enalble/Disable the SSPI task
*
*  @param    handle   (in) SSPI CHAL handle
*  @param    task_idx (in) index of the task entry to be configured
*  @param    off_on   (in) Enable -1, Disable - 0
*
*  @return SSPI status
*****************************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_enable_task(CHAL_HANDLE handle,
						 uint32_t task_idx,
						 uint32_t off_on);

/**
*
*  @brief  Enalble/Disable SSPI task scheduler to start/stop to execute the
*          configured tasks
*
*  @param    handle (in) SSPI CHAL handle
*  @param    off_on (in) Enable -1, Disable - 0
*
*  @return SSPI status
*****************************************************************************/
	CHAL_SSPI_STATUS_t chal_sspi_enable_scheduler(CHAL_HANDLE handle,
						      uint32_t off_on);

/** @} */

#ifdef __cplusplus
}
#endif
#endif				/* CHAL_SSPI_H__ */
