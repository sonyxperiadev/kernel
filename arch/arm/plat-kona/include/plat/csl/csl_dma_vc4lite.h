/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
*             @file     arch/arm/plat-kona/include/plat/csl/csl_dma_vc4lite.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/
/**
*
*   @file   csl_dma_vc4lite.h
*
*   @brief  VC4LITE DMA CSL layer 
*
****************************************************************************/
/**
*
* @defgroup VC4LITE DMAGroup Direct Memory Access
*
* @brief This group defines the APIs for DMA VC4LITE CSL layer
*
* @ingroup CSLGroup
*****************************************************************************/

#ifndef	_CSL_DMA_VC4LITE_H_
#define	_CSL_DMA_VC4LITE_H_

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * @addtogroup DMAVC4LiteGroup 
 * @{
 */

/**
*
*  VC4Lite DMA driver status definition
*
*****************************************************************************/
#define    DMA_VC4LITE_STATUS_t            DMA_VC4LITE_STATUS
	typedef enum {
		DMA_VC4LITE_STATUS_SUCCESS,	///< status success
		DMA_VC4LITE_STATUS_FAILURE,	///< status failure
		DMA_VC4LITE_STATUS_INVALID_INPUT,	///< statuc invaid input
	} DMA_VC4LITE_STATUS_t;

/**
*
*  VC4Lite DMA driver callback status definition
*
*****************************************************************************/
#define   DMA_VC4LITE_CALLBACK_STATUS_t    DMA_VC4LITE_CALLBACK_STATUS
	typedef enum {
		DMA_VC4LITE_CALLBACK_SUCCESS,	///< status success
		DMA_VC4LITE_CALLBACK_FAIL,	///< status failure
	} DMA_VC4LITE_CALLBACK_STATUS_t;

/**
*
*  VC4Lite DMA channel definition
*
*****************************************************************************/
	typedef enum {
		DMA_VC4LITE_CHANNEL_0 = 0,	///< channel 0
		DMA_VC4LITE_CHANNEL_1 = 1,	///< channel 1
		DMA_VC4LITE_CHANNEL_2 = 2,	///< channel 2
		DMA_VC4LITE_TOTAL_CHANNELS,
		DMA_VC4LITE_CHANNEL_INVALID,
	} DMA_VC4LITE_CHANNEL_t;

/**
*
*  VC4Lite DMA client ID definition
*
*****************************************************************************/
	typedef enum {
		DMA_VC4LITE_CLIENT_MEMORY = 0,	///< memory
		DMA_VC4LITE_CLIENT_DSI0,	///< DSI 0
		DMA_VC4LITE_CLIENT_DSI1,	///< DSI 1    
		DMA_VC4LITE_CLIENT_SMI,	///< SMI
		DMA_VC4LITE_CLIENT_SPI_TX,	///< SPI TX
		DMA_VC4LITE_CLIENT_SPI_RX,	///< SPI RX
	} DMA_VC4LITE_CLIENT_t;

/**
*
*  VC4Lite DMA burst length
*
*****************************************************************************/
	typedef enum {
		DMA_VC4LITE_BURST_LENGTH_1 = 0x00,	///< 1 word
		DMA_VC4LITE_BURST_LENGTH_2 = 0x01,	///< 2 word
		DMA_VC4LITE_BURST_LENGTH_3 = 0x02,	///< 3 word
		DMA_VC4LITE_BURST_LENGTH_4 = 0x03,	///< 4 word
		DMA_VC4LITE_BURST_LENGTH_5 = 0x04,	///< 5 word
		DMA_VC4LITE_BURST_LENGTH_6 = 0x05,	///< 6 word
		DMA_VC4LITE_BURST_LENGTH_7 = 0x06,	///< 7 word
		DMA_VC4LITE_BURST_LENGTH_8 = 0x07,	///< 8 word
		DMA_VC4LITE_BURST_LENGTH_9 = 0x08,	///< 9 word
		DMA_VC4LITE_BURST_LENGTH_10 = 0x09,	///< 10 word
		DMA_VC4LITE_BURST_LENGTH_11 = 0x0A,	///< 11 word
		DMA_VC4LITE_BURST_LENGTH_12 = 0x0B,	///< 12 word
		DMA_VC4LITE_BURST_LENGTH_13 = 0x0C,	///< 13 word
		DMA_VC4LITE_BURST_LENGTH_14 = 0x0D,	///< 14 word
		DMA_VC4LITE_BURST_LENGTH_15 = 0x0E,	///< 15 word
		DMA_VC4LITE_BURST_LENGTH_16 = 0x0F,	///< 16 word
	} DMA_VC4LITE_BURST_LENGTH_t;

/**
*
*  VC4Lite DMA transfer mode
*
*****************************************************************************/
	typedef enum {
		DMA_VC4LITE_XFER_MODE_LINERA = 0,	///<  linear transfer mode
		DMA_VC4LITE_XFER_MODE_2D = 1,	///<  2D transfer mode
	} DMA_VC4LITE_XFER_MODE_t;

/**
*
*  VC4Lite DMA transfer data
*
*****************************************************************************/
	typedef struct {
		UInt32 burstWriteEnable32;	///< support writes burst upto 8
		UInt32 srcAddr;	///< source address
		UInt32 dstAddr;	///< destination address
		UInt32 xferLength;	///< transfer length    
	} DMA_VC4LITE_XFER_DATA_t;

/**
*
*  VC4Lite DMA transfer 2D data
*
*****************************************************************************/
	typedef struct {
		UInt32 burstWriteEnable32;	///< support writes burst upto 8
		UInt32 srcAddr;	///< source address
		UInt32 dstAddr;	///< destination address
		UInt16 xXferLength;	///< X transfer length 
		UInt16 yXferLength;	///< Y transfer length         
	} DMA_VC4LITE_XFER_2DDATA_t;

/**
*
*  VC4Lite DMA transfer completion callback
*
*****************************************************************************/
	typedef void (*DMA_VC4LITE_CALLBACK_t) (DMA_VC4LITE_CALLBACK_STATUS
						status);

/**
*
*  VC4Lite DMA channel information definition
*
*****************************************************************************/
	typedef struct {
		DMA_VC4LITE_CLIENT_t srcID;	///< src client ID      
		DMA_VC4LITE_CLIENT_t dstID;	///< channel client ID
		DMA_VC4LITE_BURST_LENGTH_t burstLen;	///< burst length        
		DMA_VC4LITE_XFER_MODE_t xferMode;	///< transfer mode
		UInt16 dstStride;	///< dstination stride for 2D mode
		UInt16 srcStride;	///< source stride for 2D mode
		UInt32 autoFreeChan;	///< auto free channel flag when the DMA transfer is finished
		UInt32 waitResponse;	///< wait response    
		DMA_VC4LITE_CALLBACK_t callback;	///< callback function to notify DMA transfer is finished.

	} DMA_VC4LITE_CHANNEL_INFO_t;

/**
*
*  This function initialize dma
*  @param		void 
*
*  @return	    void
*
*****************************************************************************/
	DMA_VC4LITE_STATUS_t csl_dma_vc4lite_init(void);

/**
*
*  This function deinitialize dma
*  @param		void 
*
*  @return	    void
*
*****************************************************************************/
	DMA_VC4LITE_STATUS_t csl_dma_vc4lite_deinit(void);

/**
*
*  This function reset dma channel
*  @param		chanID (in) channel identification 
*
*  @return	    status (out) 0 or 1  
*
*****************************************************************************/
	DMA_VC4LITE_STATUS csl_dma_vc4lite_reset_channel(UInt16 chanID);

/**
*
*  This function configure dma channel
*  @param       chanID       (in) channel number
*  @param       pChanInfo    (in) pointer to dma channe info structure
*
*  @return	    void
*
*****************************************************************************/
	DMA_VC4LITE_STATUS csl_dma_vc4lite_config_channel(DMA_VC4LITE_CHANNEL_t
							  chanID,
							  DMA_VC4LITE_CHANNEL_INFO_t
							  * pChanInfo);

/**
*
*  This function allocates dma channel
*  @param		srcID (in) source identification
*  @param       dstID (in) destination identification
*
*  @return	    chan (out) channle identification
*
*****************************************************************************/
	Int32 csl_dma_vc4lite_obtain_channel(UInt8 srcID, UInt8 dstID);

/**
*
*  This function release dma channel
*  @param		chanID (in) channel identification
*
*  @return	    status (out) 0 or 1
*
*****************************************************************************/
	DMA_VC4LITE_STATUS csl_dma_vc4lite_release_channel(DMA_VC4LITE_CHANNEL_t
							   chanID);

/**
*
*  This function start dma channel transfer
*  @param		chanID (in) channel identification
*
*  @return	    void
*
*****************************************************************************/
	DMA_VC4LITE_STATUS csl_dma_vc4lite_start_transfer(DMA_VC4LITE_CHANNEL_t
							  chanID);

/**
*
*  This function stop dma channel trnasfer
*  @param		chanID (in) channel identification
*
*  @return	    void
*
*****************************************************************************/
	DMA_VC4LITE_STATUS csl_dma_vc4lite_stop_transfer(DMA_VC4LITE_CHANNEL_t
							 chanID);

/**
*
*  This function add data buffer for the DMA channel
*  @param		chanID (in) channel number
*  @param       pData     (in) pointer to dma channel data buffer 
*
*  @return	    status    (out) 0 or 1
*
*****************************************************************************/
	DMA_VC4LITE_STATUS csl_dma_vc4lite_add_data(DMA_VC4LITE_CHANNEL_t
						    chanID,
						    DMA_VC4LITE_XFER_DATA_t *
						    pData);

/**
*
*  This function add 2D data buffer for the DMA channel
*  @param		chanID (in) channel number
*  @param       pData     (in) pointer to dma channel data buffer 
*
*  @return	    status    (out) 0 or 1
*
*****************************************************************************/
	DMA_VC4LITE_STATUS csl_dma_vc4lite_add_data_ex(DMA_VC4LITE_CHANNEL_t
						       chanID,
						       DMA_VC4LITE_XFER_2DDATA_t
						       * pData);
/**
 * This function locks the DMA interface.
 *
 ****************************************************************************/
void csl_dma_lock(void);


/**
 * This function unlocks the DMA interface.
 *
 ****************************************************************************/
void csl_dma_unlock(void);


/**
 * This function polls the DMA interrupt status
 *
 ****************************************************************************/
void csl_dma_poll_int(int chanID);

#ifdef __cplusplus
}
#endif
#endif				/* _CSL_DMA_VC4LITE_H_ */
