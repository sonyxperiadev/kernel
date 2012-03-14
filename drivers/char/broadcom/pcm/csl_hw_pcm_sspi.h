/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

/**
*
*   @file   csl_caph_pcm_sspi.h
*
*   @brief  PCM CSL layer header file
*
****************************************************************************/

#ifndef _CSL_HW_PCM_SSPI_H_
#define _CSL_HW_PCM_SSPI_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup CSL_Interface for pcm
 * @{
 */

// undef this for RHEA, define for SAMOA
// #define SSPI_TDM_MODE

/**
*
*  device CSL handle definition
*
*****************************************************************************/
#define CSL_PCM_HANDLE CHAL_HANDLE

//According to PCM/I2S SSP CSL design, the transfer size for pcm and i2s should be 
//defined as following.  If the size is less then the following value, the transferring
//will be one-time shot. Otherwise it will be transferred continuously.
#define CSL_PCM_SSP_TSIZE   8192

/**
* CAPH SSP ID
******************************************************************************/
	typedef enum {
		CSL_CAPH_SSP_NONE,
		CSL_CAPH_SSP_4,	//SSPI1 --- ASIC SSPI4, SSPI2 --- ASIC SSPI3
		CSL_CAPH_SSP_3
	} CSL_CAPH_SSP_e;

	typedef struct {
		cUInt32 base;
		cUInt32 type;
	} CSL_PCM_HANDLE_t;

/**
*
*  device CSL operation status
*
*****************************************************************************/
	typedef enum {
		CSL_PCM_SUCCESS = 0,
		CSL_PCM_ERR_HANDLE,
		CSL_PCM_ERR_CORE,
		CSL_PCM_ERR_PROT,
		CSL_PCM_ERR_TASK,
		CSL_PCM_ERR_SEQUENCE,
		CSL_PCM_ERR_FRAME,
		CSL_PCM_ERR_DMA,
		CSL_PCM_ERROR
	} CSL_PCM_OPSTATUS_t;

/**
*
*  device CSL mode selection
*
*****************************************************************************/
	typedef enum {
		CSL_PCM_MASTER_MODE,	///< Master mode 
		CSL_PCM_SLAVE_MODE,	///< Slave mode
	} csl_pcm_mode_t;

/**
*
*  device CSL protocol selection
*
*****************************************************************************/
	typedef enum {
		CSL_PCM_PROTOCOL_MONO,
		CSL_PCM_PROTOCOL_STEREO,
		CSL_PCM_PROTOCOL_3CHANNEL,
		CSL_PCM_PROTOCOL_4CHANNEL,
		CSL_PCM_PROTOCOL_INTERLEAVE_3CHANNEL,
		CSL_PCM_PROTOCOL_INTERLEAVE_4CHANNEL,
	} csl_pcm_protocol_t;

/**
*
*  transmit channel index
*
*****************************************************************************/
	typedef enum {
		CSL_PCM_CHAN_TX0,	///< transmit channel index 0     
		CSL_PCM_CHAN_TX1,	///< transmit channel index 1
		CSL_NUM_TX_INDEX	///< number of transmit channel
	} csl_pcm_tx_channel_t;

/**
*
*  receive channel index
*
*****************************************************************************/
	typedef enum {
		CSL_PCM_CHAN_RX0,	///< receive channel index 0
		CSL_PCM_CHAN_RX1,	///< receive channel index 1
		CSL_NUM_RX_INDEX	///< number of receive channel
	} csl_pcm_rx_channel_t;

/**
*
*  device dma size selection
*
*****************************************************************************/
	typedef enum {
		CSL_PCM_DMA_SIZE_1,	///< DMA Size 1
		CSL_PCM_DMA_SIZE_4,	///< DMA Size 4
		CSL_PCM_DMA_SIZE_8,	///< DMA Size 8
		CSL_PCM_DMA_SIZE_16,	///< DMA Size 16
		CSL_PCM_DMA_SIZE_32,	///< DMA Size 32
		CSL_PCM_DMA_SIZE_64,	///< DMA Size 64
		CSL_PCM_DMA_SIZE_128	///< DMA Size 128
	} csl_pcm_dma_size_t;

/**
*
*  device sample rate
*
*****************************************************************************/
	typedef enum {
		CSL_PCM_SAMPLERATE_48000HZ,	///< 48000HZ
		CSL_PCM_SAMPLERATE_96000HZ	///< 96000HZ
	} csl_pcm_sample_rate_t;

/**
*
*  transfer format
*
*****************************************************************************/
	typedef enum {
		CSL_PCM_WORD_LENGTH_16_BIT,	///< unpacked 16 bit word length
		CSL_PCM_WORD_LENGTH_24_BIT,	///< 25 bit word length
		CSL_PCM_WORD_LENGTH_PACK_16_BIT,	///< packed 16 bit word length
	} csl_pcm_data_format_t;

	typedef enum {
		CSL_CAPH_ARM,
		CSL_CAPH_DSP,
	} CSL_CAPH_ARM_DSP_e;

/**
* CAPH OWNER: ARM or DSP
******************************************************************************/
	typedef enum {
		CAPH_ARM,
		CAPH_DSP,
		CAPH_SSP,
	} CAPH_ARM_DSP_e;

/**
*
*  csl_pcm_config_tx_t struct for transmit configuration
*
*****************************************************************************/
	typedef struct {
		Boolean enable;	///< enable/disable flag for transmit
		csl_pcm_tx_channel_t channel;	///< stereo or mono
		csl_pcm_sample_rate_t sampleRate;	///< sample rate
		Boolean loopback_enable;
	} csl_pcm_config_tx_t;

/**
*
*  csl_pcm_config_rx_t struct for receive configuration
*
*****************************************************************************/
	typedef struct {
		Boolean enable;	///< enable/disable flag for receive
		csl_pcm_sample_rate_t sampleRate;	///< sample rate
		csl_pcm_rx_channel_t channel;	///< stereo or mono
		Boolean loopback_enable;
	} csl_pcm_config_rx_t;

/**
*
*  device descriptor struct
*
*****************************************************************************/
	typedef struct {
		csl_pcm_mode_t mode;	///< master/slave mode          
		Boolean interleave;	///< interleave operation
		csl_pcm_protocol_t protocol;	///< data tansfer protocol
		csl_pcm_data_format_t format;	///< data word length 
		UInt32 xferSize;
		UInt32 ext_bits;
		UInt32 sample_rate;
	} csl_pcm_config_device_t;

/**
*
*  @brief  This function initializes the PCM CSL layer
*
*  @param  base address
*
*  @return device CSL handle
*
*****************************************************************************/
	CSL_PCM_HANDLE hw_csl_pcm_init(UInt32 baseAddr, UInt32 caphIntcHandle);

/**
*  @brief  This function deitializes the PCM CSL layer
*
*  @param  device CSL handle
*
*  @return driver CSL layer operation status
*
*****************************************************************************/
	CSL_PCM_OPSTATUS_t hw_csl_pcm_deinit(CSL_PCM_HANDLE handle);

/**
*  @brief  This function configures driver CSL layer
*
*  @param  device CSL handle
*  @param  device descriptor struct
*  @param  transmit configuration struct
*  @param  receive configuration struct
*
*  @return driver CSL layer operation status
*
*****************************************************************************/
	CSL_PCM_OPSTATUS_t hw_csl_pcm_config(CSL_PCM_HANDLE handle,
					     UInt32 xferSize);

/**
*
*  @brief  Enable PCM Scheduler
*
*  @param  device CSL handle
*  @param  scheduler status
*
*  @return driver CSL layer operation status
*
*****************************************************************************/
	CSL_PCM_OPSTATUS_t hw_csl_pcm_enable_scheduler(CSL_PCM_HANDLE handle,
						       Boolean enable);

/**
*  @brief  Start PCM
*
*  @param  device CSL handle
*  @param  device descriptor struct
*
*  @return driver CSL layer operation status
*
*****************************************************************************/
	CSL_PCM_OPSTATUS_t hw_csl_pcm_start(CSL_PCM_HANDLE handle);

/**
*
*  @brief  Start PCM TX
*
*  @param  device CSL handle
*  @param  transmit channel number
*
*  @return driver CSL layer operation status
*
*****************************************************************************/
	CSL_PCM_OPSTATUS_t hw_csl_pcm_start_tx(CSL_PCM_HANDLE handle);

/**
*
*  @brief  Start PCM RX
*
*  @param  device CSL handle
*  @param  transmit channel number
*
*  @return driver CSL layer operation status
*
*****************************************************************************/
	CSL_PCM_OPSTATUS_t hw_csl_pcm_start_rx(CSL_PCM_HANDLE handle);

/**
*
*  @brief  Stop PCM TX
*
*  @param  device CSL handle
*  @param  transmit channel number
*
*  @return driver CSL layer operation status
*
*****************************************************************************/
	CSL_PCM_OPSTATUS_t hw_csl_pcm_stop_tx(CSL_PCM_HANDLE handle);

/**
*
*  @brief  Stop PCM RX
*
*  @param  device CSL handle
*  @param  receive channel number
*
*  @return driver CSL layer operation status
*
*****************************************************************************/
	CSL_PCM_OPSTATUS_t hw_csl_pcm_stop_rx(CSL_PCM_HANDLE handle);

	CSL_PCM_OPSTATUS_t csl_pcm_resume(CSL_PCM_HANDLE handle);
	CSL_PCM_OPSTATUS_t csl_pcm_pause(CSL_PCM_HANDLE handle);

/**
*
*  @brief  Get TX channel port address
*
*  @param  device CSL handle
*  @param  transmit channel number
*
*  @return transmit channel port address
*
*****************************************************************************/
	UInt32 hw_csl_pcm_get_tx0_fifo_data_port(CSL_PCM_HANDLE handle);

/**
*
*  @brief  Get RX channel port address
*
*  @param  device CSL handle
*  @param  receive channel number
*
*  @return receive channel port address
*
*****************************************************************************/
	UInt32 hw_csl_pcm_get_rx0_fifo_data_port(CSL_PCM_HANDLE handle);

/**
*
*  @brief  Get TX channel port address
*
*  @param  device CSL handle
*  @param  transmit channel number
*
*  @return transmit channel port address
*
*****************************************************************************/
	UInt32 hw_csl_pcm_get_tx1_fifo_data_port(CSL_PCM_HANDLE handle);

/**
*
*  @brief  Get RX channel port address
*
*  @param  device CSL handle
*  @param  receive channel number
*
*  @return receive channel port address
*
*****************************************************************************/
	UInt32 hw_csl_pcm_get_rx1_fifo_data_port(CSL_PCM_HANDLE handle);

/**
*
*  @brief  enable caph pcm intr  
*
*  @param   csl_owner  (in) owner of this caph pcm channel
*  @param   csl_sspid  (in) ssp of this caph pcm channel
*
*  @return void
*****************************************************************************/

	void hw_csl_caph_intc_enable_pcm_intr(CSL_CAPH_ARM_DSP_e csl_owner,
					      CSL_CAPH_SSP_e csl_sspid);
/**
*
*  @brief  disable caph pcm intr  
*
*  @param   csl_owner  (in) owner of this caph pcm channel
*  @param   csl_sspid  (in) ssp of this caph pcm channel
*
*
*  @return void
*****************************************************************************/
	void hw_csl_caph_intc_disable_pcm_intr(CSL_CAPH_ARM_DSP_e csl_owner,
					       CSL_CAPH_SSP_e csl_sspid);

#ifdef __cplusplus
}
#endif
#endif				/* _CSL_PCM_SSPI_H_ */
