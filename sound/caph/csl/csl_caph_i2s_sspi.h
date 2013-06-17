/**************************************************************************
 * Copyright 2009, 2010 Broadcom Corporation.  All rights reserved.          */
/*                                                                        */
/*     Unless you and Broadcom execute a separate written software license*/
/*     agreement governing use of this software, this software is licensed*/
/*     to you under the terms of the GNU General Public License version 2 */
/*     (the GPL), available at                                            */
/*                                                                        */
/*     http://www.broadcom.com/licenses/GPLv2.php                         */
/*                                                                        */
/*     with the following added to such license:                          */
/*                                                                        */
/*     As a special exception, the copyright holders of this software give*/
/*     you permission to link this software with independent modules, and */
/*     to copy and distribute the resulting executable under terms of your*/
/*     choice, provided that you also meet, for each linked             */
/*     independent module, the terms and conditions of the license of that*/
/*     module.An independent module is a module which is not derived from */
/*     this software.  The special exception does not apply to any        */
/*     modifications of the software.                                     */
/*                                                                        */
/*     Notwithstanding the above, under no circumstances may you combine  */
/*     this software in any way with any other Broadcom software provided */
/*     under a license other than the GPL,                                */
/*     without Broadcom's express prior written consent.                  */
/*                                                                        */
/**************************************************************************/
/**
*
*  @file   csl_caph_i2s_sspi.h
*  @brief  CSL layer to using SSPI cHAL interface to define I2S interface
*  @note
*
*****************************************************************************/

#ifndef CSL_I2S_H__
#define CSL_I2S_H__

/**
 * @addtogroup CSL_Interface for I2S
 * @{
 */

/*
 * According to PCM/I2S SSP CSL design, the transfer size for pcm and i2s
 * should be defined as following.  If the size is less then the following
 * value, the transferring will be one-time shot. Otherwise it will be
 * transferred continuously.
 */

#define CSL_I2S_SSP_TSIZE   4096

/**
* typedefs from cHal SSPI
*/

struct CSL_SSPI_HANDLE_T {
	cUInt32 base;
	cUInt32 type;
};

#define CSL_SSPI_HANDLE_T struct CSL_SSPI_HANDLE_T

/**
* SSPI Error Code
*/
enum SSPI_hw_status_t {
	SSPI_HW_NOERR,
	SSPI_HW_ERR_HANDLE,
	SSPI_HW_ERR_CORE,
	SSPI_HW_ERR_PROT,
	SSPI_HW_ERR_TASK,
	SSPI_HW_ERR_SEQUENCE,
	SSPI_HW_ERR_FRAME,
	SSPI_HW_ERR_DMA,
	SSPI_HW_ERROR
};

#define SSPI_hw_status_t enum SSPI_hw_status_t

/**
* SSPI Mode
*/
enum SSPI_hw_mode_t {
	SSPI_HW_MASTER,
	SSPI_HW_SLAVE
};

#define SSPI_hw_mode_t enum SSPI_hw_mode_t

/**
* SSPI protocol type
*/
enum SSPI_hw_protocol_t {
	SSPI_HW_I2C,
	SSPI_HW_I2S_MODE1,
	SSPI_HW_I2S_MODE2,
	SSPI_HW_PCM_MONO,
	SSPI_HW_PCM_STEREO,
	SSPI_HW_PCM_3CHANNEL,
	SSPI_HW_PCM_4CHANNEL,
	SSPI_HW_SPI_MODE0,
	SSPI_HW_SPI_MODE1,
	SSPI_HW_SPI_MODE2,
	SSPI_HW_SPI_MODE3
};

#define SSPI_hw_protocol_t enum SSPI_hw_protocol_t

/**
* Host controller Structure
* this stucture has host controller specific parameters
*/
struct SSPI_hw_core_t {
	UInt8 core_id;
	SSPI_hw_mode_t mode;
	Boolean rx_ena;
	Boolean tx_ena;
	Boolean rx_loopback_ena;
	Boolean tx_loopback_ena;
	UInt8 word_len;
	UInt32 ext_bits;
	UInt32 trans_size;
	UInt8 *rxBuf;
	UInt8 *txBuf;
	CSL_SSPI_HANDLE_T *handle;
	UInt8 a[10];
	SSPI_hw_protocol_t prot;
	UInt32 loopCnt;
	/* set the flag for status of core, whether it is initialized or not */
	Boolean bIniitialized;
};

#define SSPI_hw_core_t struct SSPI_hw_core_t

#define CSL_HANDLE CHAL_HANDLE
#define CSL_TX_ENABLE		1
#define	CSL_RX_ENABLE		1
	/* Mode: 0 - I2S_MODE1, 1 - I2S_MODE2*/
#define CSL_I2S_MODE1		SSPI_PROT_I2S_MODE1
#define CSL_I2S_MODE2		SSPI_PROT_I2S_MODE2

/**
* CSL_I2S ERROR CODE
*/
enum CSL_I2S_ERR_CODE {
	CSL_I2S_SUCCESS = 0,
	CSL_I2S_ERROR = -1
};

#define CSL_I2S_ERR_CODE enum CSL_I2S_ERR_CODE

/**
* I2S RX Channel define
*/
enum CSL_I2S_RX_CHAN_t {
	CSL_I2S_RX_LEFT_CHAN,	/*recording from left channel in mono mode*/
	CSL_I2S_RX_RIGHT_CHAN,	/*recording from right channel in mono mode*/
	CSL_I2S_RX_AEVERAGE	/*recording with (L+R)/2 in mono mode*/
};

#define CSL_I2S_RX_CHAN_t enum CSL_I2S_RX_CHAN_t

/**
* I2S Channel Selection
*/
enum CSL_I2S_CHANNEL_SEL_t {
	CSL_I2S_CHANNEL_SEL_STEREO,	/*Select Stereo*/
	CSL_I2S_CHANNEL_SEL_MONO	/*Select mono*/
};

#define CSL_I2S_CHANNEL_SEL_t enum CSL_I2S_CHANNEL_SEL_t

/**
* I2S mode, can be only master or slave
*/
enum CSL_I2S_MODE_SEL_t {
	CSL_I2S_SLAVE_MODE,	/*Slave mode*/
	CSL_I2S_MASTER_MODE	/*Master mode*/
};

#define CSL_I2S_MODE_SEL_t enum CSL_I2S_MODE_SEL_t

/**
* I2S dma size in number of samples
*/
enum CSL_I2S_DMA_SIZE_t {
	CSL_I2S_DMA_SIZE_1,	/*DMA Size 1*/
	CSL_I2S_DMA_SIZE_4,	/*DMA Size 4*/
	CSL_I2S_DMA_SIZE_8,	/*DMA Size 8*/
	CSL_I2S_DMA_SIZE_16,	/*DMA Size 16*/
	CSL_I2S_DMA_SIZE_32,	/*DMA Size 32*/
	CSL_I2S_DMA_SIZE_64,	/*DMA Size 64*/
	CSL_I2S_DMA_SIZE_128	/*DMA Size 128*/
};

#define CSL_I2S_DMA_SIZE_t enum CSL_I2S_DMA_SIZE_t

/**
* I2S sampling Rate
*/
enum CSL_I2S_SAMPLE_RATE_t {
	CSL_I2S_16BIT_4000HZ,	/*4000HZ*/
	CSL_I2S_16BIT_8000HZ,	/*8000HZ*/
	CSL_I2S_16BIT_16000HZ,	/*16000HZ*/
	CSL_I2S_16BIT_48000HZ,	/*48000HZ*/
	CSL_I2S_32BIT_8000HZ,	/*8000HZ*/
	CSL_I2S_25BIT_48000HZ,	/*48000HZ*/
};

#define CSL_I2S_SAMPLE_RATE_t enum CSL_I2S_SAMPLE_RATE_t

/**
* I2S dma enable
*/
enum CSL_I2S_DMA_SEL_t {
	CSL_I2S_DMA_DISABLE,	/*Disable DMA*/
	CSL_I2S_DMA_ENABLE	/*Enable DMA*/
};

#define CSL_I2S_DMA_SEL_t enum CSL_I2S_DMA_SEL_t

/**
* I2S Configuration Structure
*/
struct CSL_I2S_CONFIG_t {
	UInt32 mode;	/*Master or slave mode*/
	UInt32 tx_ena;
	UInt32 rx_ena;
	UInt32 tx_loopback_ena;
	UInt32 rx_loopback_ena;
	UInt32 trans_size;
	UInt32 prot;
	UInt32 interleave;
	CSL_I2S_SAMPLE_RATE_t sampleRate;	/*sample rate*/
};

#define CSL_I2S_CONFIG_t struct CSL_I2S_CONFIG_t

/**
*
*  @brief  Initialize SSPI as I2S
*
*  @param    base_addr (in) mapped address of the SSPI core to be initialized
*
*  @return CSL handle for this I2S/SSPI core instance
*/
CSL_HANDLE csl_i2s_init(cUInt32 baseAddr);
/**
*  @brief  De-initialize I2S/SSPI CSL internal data structure
*
*  @param    handle    (in) I2S CSL handle to be deinitialized
*
*  @return SSPI status
*/
void csl_i2s_deinit(CSL_HANDLE hContext);
/**
*  @brief  Config I2S, such as mode, protocol, size, tx sampling
*          rate and rx sampling rate
*
*  @param    handle    (in) I2S CSL handl
*  @param    CSL_I2S_CONFIG_t (in) I2S CSL config struct
*
*  @return none
*/
void csl_i2s_config(CSL_HANDLE hContext, CSL_I2S_CONFIG_t *config);

/**
*  @brief  Start I2S Rx
*
*  @param    handle    (in) I2S CSL handle
*  @param    CSL_I2S_CONFIG_t (in) I2S CSL config struct
*
*  @return none
*/
void csl_i2s_start_rx(CSL_HANDLE handle, CSL_I2S_CONFIG_t *config);

/**
*  @brief  Start I2S Tx
*
*  @param    handle    (in) I2S CSL handle
*  @param    CSL_I2S_CONFIG_t (in) I2S CSL config struct
*
*  @return none
*/
void csl_i2s_start_tx(CSL_HANDLE handle, CSL_I2S_CONFIG_t *config);
/**
*
*  @brief  Stop I2S TX
*
*  @param   handle    (in) I2S CSL handle
*
*  @return none
*/
void csl_i2s_stop_tx(CSL_HANDLE handle);
/**
*
*  @brief  Stop I2S RX
*
*  @param   handle    (in) I2S CSL handle
*
*  @return none
*/
	void csl_i2s_stop_rx(CSL_HANDLE handle);
/**
*
*  @brief  Enable/Disable SSPI scheduler
*
*  @param   handle    (in) I2S CSL handle
*  @param   on_off    (in) Enable -1, Disable - 0
*
*  @return none
*/
	void csl_sspi_enable_scheduler(CSL_HANDLE handle, UInt32 on_off);
/**
*
*  @brief  Get channel 0 TX port address
*
*  @param   handle    (in) I2S CSL handle
*
*  @return none
*/
	UInt32 csl_i2s_get_tx0_fifo_data_port(CSL_HANDLE handle);
/**
*
*  @brief  Get channel 1 TX port address
*
*  @param   handle    (in) I2S CSL handle
*
*  @return none
*/
	UInt32 csl_i2s_get_tx1_fifo_data_port(CSL_HANDLE handle);
/**
*
*  @brief  Get channel 0 RX port address
*
*  @param   handle    (in) I2S CSL handle
*
*  @return none
*/
	UInt32 csl_i2s_get_rx0_fifo_data_port(CSL_HANDLE handle);
/**
*
*  @brief  Get channel 1 RX port address
*
*  @param   handle    (in) I2S CSL handle
*
*  @return none
*/
UInt32 csl_i2s_get_rx1_fifo_data_port(CSL_HANDLE handle);

#endif				/* SPI_TEST_H__ */
