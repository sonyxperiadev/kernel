/****************************************************************************
*
*	Copyright (c) 2009-2012 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license
*   agreement governing use of this software, this software is licensed to you
*   under the terms of the GNU General Public License version 2, available
*   at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
*   Notwithstanding the above, under no circumstances may you combine this
*   software in any way with any other Broadcom software provided under a
*   license other than the GPL, without Broadcom's express prior written
*   consent.
*
****************************************************************************/
#ifndef _SSPI_HELPERS_H_
#define _SSPI_HELPERS_H_

#include <plat/chal/chal_sspi_hawaii.h>

typedef unsigned int UINT32;
typedef int BOOLEAN;
typedef unsigned char UINT8;

/****************************************************************************
***
***  "PUBLIC" (GLOBAL) DEFINITIONS AND DECLARATIONS
***
****************************************************************************/

/****************************************************************************
***  macros
****************************************************************************/

/****************************************************************************
***  constants
****************************************************************************/
#define SSPI_HW_MAX_SSPI_CORES                    4
#define SSPI_HW_MAX_SSPI_CHANNELS                 4
#define SSPI_HW_MAX_SSPI_DMA_CHANNELS             4
#define SSPI_HW_WORD_LEN_32Bit                    32
#define SSPI_HW_WORD_LEN_25Bit                    25
#define SSPI_HW_WORD_LEN_24Bit                    24
#define SSPI_HW_WORD_LEN_16Bit                    16
#define SSPI_HW_WORD_LEN_8Bit                     8
#define SSPI_HW_DUMMY_BITS_9                      9
#define SSPI_HW_DUMMY_BITS_2                      2
#define SSPI_HW_DUMMY_BITS_1                      1
#define SSPI_HW_FRAME0_MASK                      (1 << 0)
#define SSPI_HW_FRAME1_MASK                      (1 << 1)
#define SSPI_HW_FRAME2_MASK                      (1 << 2)
#define SSPI_HW_FRAME3_MASK                      (1 << 3)

#define SSPI_HW_MAX_SFLASH_SIZE                   (512 * 1024)

#define SSPI_HW_DEFAULT_PIO_SIZE                16
#define SSPI_HW_INTR_CFG_MODE_CONT                 0x1

#define SSPI_HW_FIFO_RX0                 0x0
#define SSPI_HW_FIFO_RX1                 0x1
#define SSPI_HW_FIFO_RX2                 0x2
#define SSPI_HW_FIFO_RX3                 0x3
#define SSPI_HW_FIFO_TX0                 0x4
#define SSPI_HW_FIFO_TX1                 0x5
#define SSPI_HW_FIFO_TX2                 0x6
#define SSPI_HW_FIFO_TX3                 0x7

/*  Each SSPI core supports 4 DMA channels */
#define SSPI_HW_DMA_RX0                 0x0
#define SSPI_HW_DMA_TX0                 0x1
#define SSPI_HW_DMA_RX1                 0x2
#define SSPI_HW_DMA_TX1                 0x3

#define SSPI_HW_DEF_PIO_START_THRES    0x3
#define SSPI_HW_DEF_PIO_STOP_THRES     0x3

/****************************************************************************
***  typdef's
****************************************************************************/
/* SSPI error status */
typedef enum {
	BCM_SUCCESS = 0,
	SSPI_HW_NOERR = BCM_SUCCESS,
	SSPI_HW_ERR_HANDLE,
	SSPI_HW_ERR_CORE,
	SSPI_HW_ERR_PROT,
	SSPI_HW_ERR_TASK,
	SSPI_HW_ERR_SEQUENCE,
	SSPI_HW_ERR_FRAME,
	SSPI_HW_ERR_DMA,
	SSPI_HW_ERR_INTERRUPT,
	SSPI_HW_ERR_TIMEOUT,
	SSPI_HW_ERR_NACK,
	SSPI_HW_ERR_CAPH,
	SSPI_HW_ERR_CAPH_START,
	SSPI_HW_ERR_CAPH_STOP,
	SSPI_HW_ERR_NOT_SUPPORTED,
	SSPI_HW_ERR_PARA,
	SSPI_HW_ERROR
} SSPI_hw_status_t;

typedef enum {
	SSPI_HW_MASTER,
	SSPI_HW_SLAVE
} SSPI_hw_mode_t;

typedef enum {
	SSPI_HW_I2C,
	SSPI_HW_I2S_START,
	SSPI_HW_I2S_MODE1 = SSPI_HW_I2S_START,
	SSPI_HW_I2S_MODE2,
	SSPI_HW_INTERLEAVE_I2S_MODE1,
	SSPI_HW_INTERLEAVE_I2S_MODE2,
	SSPI_HW_PCM_START,
	SSPI_HW_PCM_MONO = SSPI_HW_PCM_START,
	SSPI_HW_PCM_STEREO,
	SSPI_HW_PCM_3CHANNEL,
	SSPI_HW_PCM_4CHANNEL,
	SSPI_HW_INTERLEAVE_PCM_STEREO,
	SSPI_HW_INTERLEAVE_PCM_3CHANNEL,
	SSPI_HW_INTERLEAVE_NOKIA_PCM_3CHANNEL,
	SSPI_HW_INTERLEAVE_PCM_4CHANNEL,
	SSPI_HW_INTERLEAVE_NOKIA_PCM_4CHANNEL,
	SSPI_HW_INTERLEAVE_PCM_16CHANNEL,
	SSPI_HW_SPI_START,
	SSPI_HW_SPI_MODE0 = SSPI_HW_SPI_START,
	SSPI_HW_SPI_MODE1,
	SSPI_HW_SPI_MODE2,
	SSPI_HW_SPI_MODE3,
} SSPI_hw_protocol_t;

typedef enum {
	SSPI_CORE_ID_SSP0,
	SSPI_CORE_ID_SSP1,
	SSPI_CORE_ID_SSP2,
	SSPI_CORE_ID_SSP3,
	SSPI_CORE_ID_SSP4,
	SSPI_CORE_ID_INV,
} SSPI_Core_ID_t;

typedef enum {
	SSPI_CORE_SW_PEER_MEM,	/*  To memory */
	SSPI_CORE_SW_PEER_AUDIO_EP,	/*  To Audio Earphone */
	SSPI_CORE_SW_PEER_AUDIO_HS,	/*  To Audio Headset */
	SSPI_CORE_SW_PEER_SRC,	/*  To Sample rate convert block */
	SSPI_CORE_SW_PEER_INVALID,
} SSPI_core_sw_peer_t;

typedef enum {
	SSPI_HW_INTR_STATE_INVALID,
	SSPI_HW_INTR_STATE_INIT,
	SSPI_HW_INTR_STATE_RUNNING,
	SSPI_HW_INTR_STATE_COMPLETE,
} SSPI_hw_intr_state_t;

/* SSPI interrupt handling infomation structure */
typedef struct {
	SSPI_hw_intr_state_t intrCurState;
	UINT32 intrCfgMode;
	UINT32 txBufOffset[SSPI_HW_MAX_SSPI_CHANNELS];
	UINT32 rxBufOffset[SSPI_HW_MAX_SSPI_CHANNELS];
} SSPI_hw_intr_hdl_info_t;

/* Host controller Structure */
/* this stucture has host controller specific parameters */
typedef struct {
	void *base;
	UINT32 core_idx;
	SSPI_Core_ID_t core_id;
	SSPI_hw_mode_t mode;
	BOOLEAN rx_ena;
	BOOLEAN tx_ena;
	BOOLEAN rx_loopback_ena;
	BOOLEAN tx_loopback_ena;
	UINT8 word_len;
	UINT32 ext_bits;
	UINT32 trans_size;
	UINT8 *rxBuf[SSPI_HW_MAX_SSPI_CHANNELS];
	UINT8 *txBuf[SSPI_HW_MAX_SSPI_CHANNELS];
	void *handle;
	SSPI_hw_protocol_t prot;
	CHAL_SSPI_BIT_RATE_t bit_rate;
	UINT32 loopCnt;
	UINT32 bIniitialized;
} SSPI_hw_core_t;

typedef enum {
	I2C_RDONLY_TRANSACTION = 0,
	I2C_WRONLY_TRANSACTION,
	I2C_WRRD_TRANSACTION,
} SSPI_hw_i2c_transaction_t;

extern SSPI_hw_status_t SSPI_hw_init(SSPI_hw_core_t *pCore);

extern SSPI_hw_status_t SSPI_hw_i2c_init(SSPI_hw_core_t *pCore);

extern SSPI_hw_status_t SSPI_hw_i2c_intr_enable(SSPI_hw_core_t *pCore);

#if 0
extern SSPI_hw_status_t SSPI_hw_i2c_send_start_sequence(SSPI_hw_core_t *pCore);

extern SSPI_hw_status_t SSPI_hw_i2c_send_stop_sequence(SSPI_hw_core_t *pCore);

extern SSPI_hw_status_t SSPI_hw_i2c_write_byte_and_read_ack(SSPI_hw_core_t *
							    pCore,
							    unsigned char *byte,
							    int *ack);
#endif

extern SSPI_hw_status_t SSPI_hw_i2c_do_transaction(SSPI_hw_core_t *pCore,
						   unsigned char *rx_buf,
						   unsigned int rx_len,
						   unsigned char *tx_buf,
						   unsigned int tx_len,
						   unsigned int buf_len,
						   SSPI_hw_i2c_transaction_t
						   tran_type);

extern void SSPI_hw_i2c_prepare_for_xfr(SSPI_hw_core_t *pCore);

#if 0
extern SSPI_hw_status_t SSPI_hw_i2c_read_byte_and_write_ack(SSPI_hw_core_t *
							    pCore,
							    unsigned char *byte,
							    int no_ack);
#endif

extern void SSPI_hw_i2c_read_and_ack_intr(SSPI_hw_core_t *pCore,
					  uint32_t *status,
					  uint32_t *det_status);

#endif /* _SSPI_HELPERS_H_ */
