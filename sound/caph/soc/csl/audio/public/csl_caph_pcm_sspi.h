/*****************************************************************************
*
*    (c) 2001-2010 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or
* its licensors, and may only be used, duplicated, modified or distributed
* pursuant to the terms and conditions of a separate, written license
* agreement executed between you and Broadcom (an "Authorized License").
* Except as set forth in an Authorized License, Broadcom grants no license
* (express or implied), right to use, or waiver of any kind with respect to
* the Software, and Broadcom expressly reserves all rights in and to the
* Software and all intellectual property rights therein.
* IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS
* SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
* ALL USE OF THE SOFTWARE.  
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*    IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
*    FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
*    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU
*    ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
*****************************************************************************/
/**
*
*   @file   csl_caph_pcm_sspi.h
*
*   @brief  PCM CSL layer header file
*
****************************************************************************/

#ifndef _CSL_PCM_SSPI_H_
#define _CSL_PCM_SSPI_H_

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

typedef struct
{
    cUInt32 base;
    cUInt32 type;
} CSL_PCM_HANDLE_t;

/**
*
*  device CSL operation status
*
*****************************************************************************/
typedef enum
{
    CSL_PCM_SUCCESS=0,
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
typedef enum
{
    CSL_PCM_MASTER_MODE,                            ///< Master mode 
    CSL_PCM_SLAVE_MODE,                             ///< Slave mode
} csl_pcm_mode_t;  

/**
*
*  device CSL protocol selection
*
*****************************************************************************/
typedef enum
{
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
typedef enum
{
    CSL_PCM_CHAN_TX0,                               ///< transmit channel index 0     
    CSL_PCM_CHAN_TX1,                               ///< transmit channel index 1
    CSL_NUM_TX_INDEX                                ///< number of transmit channel
} csl_pcm_tx_channel_t;

/**
*
*  receive channel index
*
*****************************************************************************/
typedef enum
{
    CSL_PCM_CHAN_RX0,                               ///< receive channel index 0
    CSL_PCM_CHAN_RX1,                               ///< receive channel index 1
    CSL_NUM_RX_INDEX  	                            ///< number of receive channel
} csl_pcm_rx_channel_t;

/**
*
*  device dma size selection
*
*****************************************************************************/
typedef enum
{
    CSL_PCM_DMA_SIZE_1,		                        ///< DMA Size 1
    CSL_PCM_DMA_SIZE_4,		                        ///< DMA Size 4
    CSL_PCM_DMA_SIZE_8,		                        ///< DMA Size 8
    CSL_PCM_DMA_SIZE_16,		                    ///< DMA Size 16
    CSL_PCM_DMA_SIZE_32,		                    ///< DMA Size 32
    CSL_PCM_DMA_SIZE_64,		                    ///< DMA Size 64
    CSL_PCM_DMA_SIZE_128		                    ///< DMA Size 128
} csl_pcm_dma_size_t;

/**
*
*  device sample rate
*
*****************************************************************************/
typedef enum
{
    CSL_PCM_SAMPLERATE_48000HZ,                     ///< 48000HZ
    CSL_PCM_SAMPLERATE_96000HZ                      ///< 96000HZ
} csl_pcm_sample_rate_t;

/**
*
*  transfer format
*
*****************************************************************************/
typedef enum
{
  CSL_PCM_WORD_LENGTH_16_BIT,                       ///< 16 bit word length
  CSL_PCM_WORD_LENGTH_24_BIT                        ///< 25 bit word length
} csl_pcm_data_format_t;

/**
*
*  csl_pcm_config_tx_t struct for transmit configuration
*
*****************************************************************************/
typedef struct
{
	Boolean                    enable;              ///< enable/disable flag for transmit
	csl_pcm_tx_channel_t       channel;	            ///< stereo or mono
	csl_pcm_sample_rate_t      sampleRate;	        ///< sample rate
	Boolean                    loopback_enable;
} csl_pcm_config_tx_t;

/**
*
*  csl_pcm_config_rx_t struct for receive configuration
*
*****************************************************************************/
typedef struct
{
	Boolean                    enable;              ///< enable/disable flag for receive
	csl_pcm_sample_rate_t 	   sampleRate; 	        ///< sample rate
    csl_pcm_rx_channel_t       channel;		        ///< stereo or mono
    Boolean                    loopback_enable;
} csl_pcm_config_rx_t;

/**
*
*  device descriptor struct
*
*****************************************************************************/
typedef struct
{
	csl_pcm_mode_t             mode;				///< master/slave mode 		
	Boolean                    interleave;          ///< interleave operation
	csl_pcm_protocol_t         protocol;            ///< data tansfer protocol
	csl_pcm_data_format_t      format;              ///< data word length 
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
CSL_PCM_HANDLE csl_pcm_init(cUInt32 baseAddr);

/**
*  @brief  This function deitializes the PCM CSL layer
*
*  @param  device CSL handle
*
*  @return driver CSL layer operation status
*
*****************************************************************************/
CSL_PCM_OPSTATUS_t csl_pcm_deinit(CSL_PCM_HANDLE handle);

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
CSL_PCM_OPSTATUS_t csl_pcm_config
(
    CSL_PCM_HANDLE handle,
    csl_pcm_config_device_t *configDev, 
    csl_pcm_config_tx_t *configTx,
    csl_pcm_config_rx_t *configRx
);

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
CSL_PCM_OPSTATUS_t csl_pcm_enable_scheduler(CSL_PCM_HANDLE handle, Boolean enable);

/**
*  @brief  Start PCM
*
*  @param  device CSL handle
*  @param  device descriptor struct
*
*  @return driver CSL layer operation status
*
*****************************************************************************/
CSL_PCM_OPSTATUS_t csl_pcm_start(CSL_PCM_HANDLE handle, csl_pcm_config_device_t *config);

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
CSL_PCM_OPSTATUS_t csl_pcm_start_tx(CSL_PCM_HANDLE handle, UInt8 channel);

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
CSL_PCM_OPSTATUS_t csl_pcm_start_rx(CSL_PCM_HANDLE handle, UInt8 channel);

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
CSL_PCM_OPSTATUS_t csl_pcm_stop_tx(CSL_PCM_HANDLE handle, UInt8 channel);

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
CSL_PCM_OPSTATUS_t csl_pcm_stop_rx(CSL_PCM_HANDLE handle, UInt8 channel);


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
UInt32 csl_pcm_get_tx0_fifo_data_port(CSL_PCM_HANDLE handle);

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
UInt32 csl_pcm_get_rx0_fifo_data_port(CSL_PCM_HANDLE handle);

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
UInt32 csl_pcm_get_tx1_fifo_data_port(CSL_PCM_HANDLE handle);

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
UInt32 csl_pcm_get_rx1_fifo_data_port(CSL_PCM_HANDLE handle);

#ifdef __cplusplus
}
#endif

#endif /* _CSL_PCM_SSPI_H_ */
