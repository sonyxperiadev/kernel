/****************************************************************************
*
*     Copyright (c) 2009 Broadcom Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           Broadcom Corporation
*           5300 California Ave
*           Irvine, California 92617
*
*     All information contained in this document is Broadcom Corporation
*     company private, proprietary, and trade secret.
*
****************************************************************************
*
*        Block Level Test System   BLTS
*
*
* FILENAME:  SSPI_hw.h
*
*
*  Description:  Header file for "public" access information to code in
*                  SSPI_hw.c for SSPI block tests low level hardware functions
*
*
*  Notes:
*
****************************************************************************/
#ifndef _SSPI_HW_H_
#define _SSPI_HW_H_

#include "chal_sspi.h"

typedef unsigned int UINT32;
typedef int          BOOLEAN;
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

#define SSPI_HW_MAX_SFLASH_SIZE                   (512*1024)

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

// Each SSPI core supports 4 DMA channels
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
typedef enum
{
    BCM_SUCCESS		= 0,
    SSPI_HW_NOERR	= BCM_SUCCESS,
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
}SSPI_hw_status_t;

typedef enum
{
    SSPI_HW_MASTER,
    SSPI_HW_SLAVE
}SSPI_hw_mode_t;

typedef enum
{
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
}SSPI_hw_protocol_t;


typedef enum
{
    SSPI_CORE_ID_SSP0,
    SSPI_CORE_ID_SSP1,
    SSPI_CORE_ID_SSP2,
    SSPI_CORE_ID_SSP3,
    SSPI_CORE_ID_SSP4,
    SSPI_CORE_ID_INV,
}SSPI_Core_ID_t;

typedef enum
{
    SSPI_CORE_SW_PEER_MEM,  // To memory
    SSPI_CORE_SW_PEER_AUDIO_EP, // To Audio Earphone
    SSPI_CORE_SW_PEER_AUDIO_HS, // To Audio Headset
    SSPI_CORE_SW_PEER_SRC, // To Sample rate convert block
    SSPI_CORE_SW_PEER_INVALID,
}SSPI_core_sw_peer_t;


typedef enum
{
    // Tx sinewave data to audioh
    SSPI_HW_CAPH_CONF_ID_TX_SINEWAVE_AUD,
    // Rx sinewave data from audioh
    SSPI_HW_CAPH_CONF_ID_RX_SINEWAVE_AUD,
    // Tx mem data and loopback it to mem
    // 1. Memory Tx data to CFIFO and switch SSPI Tx FIFO
    // 2. SSPI Tx and loopback to SSPI RX FIFO
    // 3. Switch from SSPI RX fifo to CFIFO and DMA to memory
    SSPI_HW_CAPH_CONF_ID_TXRX_MEM,
    // 1. Memory Tx 8KHz sinewave data to CFIFO and switch SSPI Tx FIFO
    // 2. SSPI Tx and loopback to SSPI RX FIFO
    // 3. Switch from SSPI RX fifo to SRC and upsampling to 48KHz
    // 4. Switch from SRC to AUDIOH
    SSPI_HW_CAPH_CONF_ID_TXRX_SINEWAVE_SRC_AUD,
    // Rx 8KHz sinewave data from SRC, which down-sample the 48KHz data from audioh
    SSPI_HW_CAPH_CONF_ID_RX_SINEWAVE_SRC_AUD,
    // 1. Memory Tx 48KHz sinewave data to CFIFO and switch SSPI Tx FIFO
    // 2. SSPI Tx and loopback to SSPI RX FIFO
    // 3. Switch from SSPI RX fifo to SRC and downsampling to 8KHz
    // 4. Switch from SRC to CFIFO and DMA to memory
    SSPI_HW_CAPH_CONF_ID_TXRX_SINEWAVE_SRC,
    // 1. Memory Tx 44.1KHz sinewave data to CFIFO and switch to SRC and up-sample
    //    to 48KHz.
    // 2. Switch from SRC to SSPI Tx FIFO
    // 2. SSPI Tx and loopback to SSPI RX FIFO
    // 3. Switch from SSPI RX fifo to CFIFO and DMA to memory
    SSPI_HW_CAPH_CONF_ID_TXRX_SINEWAVE_MEM_SRC_MEM,
    // With this confID, if use 1 or 2 channels, ch1 or ch2 get input from
    // AUDIOH VIN and loopback to AUDIO VOUT.
    // 1. Switch from ADDIOH VIN to SSPI Tx FIFO
    // 2. SSPI Tx and loopback to SSPI RX FIFO
    // 3. Switch from SSPI RX fifo to AUDIOH VOUT
    // if use 3 or 4 channels, ch1 and ch2 behave same but ch3 or ch4 get input
    // from memory and loopback to memory loopback to memory
    SSPI_HW_CAPH_CONF_ID_TXRX_AUDIN_AUDOUT_MEMIN_MEMOUT,

    SSPI_HW_CAPH_CONF_ID_ARM_SET_TXRX_PATH,
    SSPI_HW_CAPH_CONF_ID_DSP_SET_TXRX_PATH,

    SSPI_HW_CAPH_CONF_ID_INV,  // Invalid conf ID
}SSPI_hw_caph_conf_ID_t;

typedef enum
{
    SSPI_HW_INTR_STATE_INVALID,
    SSPI_HW_INTR_STATE_INIT,
    SSPI_HW_INTR_STATE_RUNNING,
    SSPI_HW_INTR_STATE_COMPLETE,
}SSPI_hw_intr_state_t;

/* SSPI interrupt handling infomation structure */
typedef struct
{
    SSPI_hw_intr_state_t   intrCurState;
    UINT32 intrCfgMode;
    UINT32 txBufOffset[SSPI_HW_MAX_SSPI_CHANNELS];
    UINT32 rxBufOffset[SSPI_HW_MAX_SSPI_CHANNELS];
}SSPI_hw_intr_hdl_info_t;

/* Host controller Structure */
/* this stucture has host controller specific parameters */
typedef struct
{
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
}SSPI_hw_core_t;

typedef enum {
	I2C_READ_TRANSACTION = 0,
	I2C_WRITE_TRANSACTION,
} SSPI_hw_i2c_transaction_t;

extern SSPI_hw_status_t SSPI_hw_init(SSPI_hw_core_t *pCore);

extern SSPI_hw_status_t SSPI_hw_i2c_init(SSPI_hw_core_t *pCore);

extern SSPI_hw_status_t SSPI_hw_i2c_intr_enable(SSPI_hw_core_t *pCore);


#if 0
extern SSPI_hw_status_t SSPI_hw_i2c_send_start_sequence(SSPI_hw_core_t *pCore);

extern SSPI_hw_status_t SSPI_hw_i2c_send_stop_sequence(SSPI_hw_core_t *pCore);

extern SSPI_hw_status_t SSPI_hw_i2c_write_byte_and_read_ack(SSPI_hw_core_t *pCore, unsigned char *byte, int *ack);
#endif

extern SSPI_hw_status_t SSPI_hw_i2c_do_write_transaction(SSPI_hw_core_t *pCore, 
						 unsigned char *rx_buf, 
						 unsigned int rx_len, 
						 unsigned char *tx_buf,
						 unsigned int tx_len);

extern SSPI_hw_status_t SSPI_hw_i2c_do_read_transaction(SSPI_hw_core_t *pCore, 
						 unsigned char *rx_buf, 
						 unsigned int rx_len, 
						 unsigned char *tx_buf,
						 unsigned int tx_len);

extern void SSPI_hw_i2c_prepare_for_xfr(SSPI_hw_core_t *pCore);

#if 0
extern SSPI_hw_status_t SSPI_hw_i2c_read_byte_and_write_ack(SSPI_hw_core_t *pCore, unsigned char *byte, int no_ack);
#endif

extern void SSPI_hw_i2c_read_and_ack_intr(SSPI_hw_core_t *pCore, uint32_t *status, uint32_t *det_status);

#endif /* _SSPI_HW_H_ */

