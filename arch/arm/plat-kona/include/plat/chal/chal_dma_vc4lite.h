/*****************************************************************************
*  Copyright 2001 - 2009 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/
/**
*
*  @file   chal_dma_vc4lite.h
*  @brief  VC4lite DMA cHAL interface
*  @note  
*
*****************************************************************************/
#ifndef	_CHAL_DMA_VC4LITE_H_
#define	_CHAL_DMA_VC4LITE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "chal_types.h"

/**
 * @addtogroup cHAL_Interface 
 * @{
 */

/**
* DMA channel
******************************************************************************/
	typedef enum {
		CHAL_DMA_VC4LITE_CHANNEL_0 = 0,	///< DMA channel 0
		CHAL_DMA_VC4LITE_CHANNEL_1,	///< DMA channel 1
		CHAL_DMA_VC4LITE_CHANNEL_2,	///< DMA channel 2
		CHAL_TOTAL_DMA_VC4LITE_CHANNELS	///< the total DMA channels
	} CHAL_DMA_CHANNEL_t;

/**
* DMA status
******************************************************************************/
	typedef enum {
		CHAL_DMA_VC4LITE_STATUS_SUCCESS = 0x00,	///< success status
		CHAL_DMA_VC4LITE_STATUS_FAILURE = 0x01,	///< failure status
		CHAL_DMA_VC4LITE_STATUS_INVALID_PARAMETER = 0x02,	///< bad parameter
		CHAL_DMA_VC4LITE_STATUS_INVALID_STATE = 0x03,	///< invalid state
	} CHAL_DMA_VC4LITE_STATUS_t;

/**
* DMA state
******************************************************************************/
#define CHAL_DMA_VC4LITE_STATE_ERROR            0x00000001	///< error state
#define CHAL_DMA_VC4LITE_STATE_DREQ_PAUSED      0x00000004	///< Dreq pause state
#define CHAL_DMA_VC4LITE_STATE_PAUSED           0x00000008	///< DMA pause state
#define CHAL_DMA_VC4LITE_STATE_DREQ             0x00000008	///< Dreq state
#define CHAL_DMA_VC4LITE_STATE_END              0x00000010	///< DMA finish state
#define CHAL_DMA_VC4LITE_STATE_INVALID          0xffffffff	///< DMA invalid state

/**
* DMA data request ID
******************************************************************************/
	typedef enum {
		CHAL_DMA_VC4LITE_DREQ_NONE = 0,	///< no DREQ
		CHAL_DMA_VC4LITE_DREQ_DSI0 = 4,	///< DSI channel 0
		CHAL_DMA_VC4LITE_DREQ_DSI1 = 5,	///< DSI channel 1
		CHAL_DMA_VC4LITE_DREQ_SMI = 1,	///< SMI
		CHAL_DMA_VC4LITE_DREQ_SPI_TX = 2,	///< SPI_TX
		CHAL_DMA_VC4LITE_DREQ_SPI_RX = 3	///< SPI_RX         
	} CHAL_DMA_VC4LITE_DREQ_ID_t;

/**
* DMA operation mode
******************************************************************************/
	typedef enum {
		CHAL_DMA_VC4LITE_LINEAR_MODE = 0,	///< linear mode
		CHAL_DMA_VC4LITE_2D_MODE	///< 2D mode
	} CHAL_DMA_VC4LITE_MODE_t;

/**
* DMA transfer length
******************************************************************************/
	typedef union {
		struct {
			cUInt16 xLen;	///< x length for 2D mode
			cUInt16 yLen;	///< y length for 2D mode
		} XferXYLen;
		cUInt32 len;	///< length for linear mode
	} ChalVc4liteDmaXferLength_t;

/**
* DMA control block informaiton
******************************************************************************/
	typedef struct {
		cUInt32 burstWriteEnable32;	///< support writes burst upto 8
		cUInt32 noWideBurst;	///< no wide writes as a 2 beat burst (0/1)
		cUInt32 waitCycles;	///< add wait cycles after each DMA read/write
		cUInt32 srcDreqID;	///< src DREQ ID
		cUInt32 dstDreqID;	///< dst DREQ ID
		cUInt32 srcIgnoreRead;	///< ignore read 
		cUInt32 burstLength;	///< burst length
		cUInt32 srcXferWidth;	///< source transfer width
		cUInt32 srcAddrIncrement;	///< source address increment
		cUInt32 dstIgnoreWrite;	///< destination ignore write
		cUInt32 dstXferWidth;	///< destination transfer width
		cUInt32 dstAddrIncrement;	///< destination address increment
		cUInt32 waitResponse;	///< wait for a write response
		CHAL_DMA_VC4LITE_MODE_t xferMode;	///< transfer mode
		void __iomem *srcAddr;	///< source address
		void __iomem *dstAddr;	///< desitnation address
		ChalVc4liteDmaXferLength_t xferLength;	///< transfer length
		cUInt16 srcStride;	///< source stride for 2D mode
		cUInt16 dstStride;	///< destination stride for 2D mode
	} ChalDmaCtrlBlkInfo_t;

/**
*
*  @brief   Initialize VC4LITE DMA hardware and software interface.
*
*  @param   baseAddress (in) mapped address of DMA controller
*
*  @return  Handle of this DMA instance
*
*  @note    
*
******************************************************************************/
	CHAL_HANDLE chal_dma_vc4lite_init(cUInt32 baseAddress);

/**
*
*  @brief   Clear the VC4LITE DMA controller's interrupt status
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*  @param   channel  (in) DMA channel number 
*
*  @return  DMA status
*
******************************************************************************/
	void chal_dma_vc4lite_clear_int_status(CHAL_HANDLE handle,
					       cUInt32 channel);

/**
*
*  @brief   get the VC4LITE DMA controller's interrupt status
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*  @param   channel  (in) DMA channel number 
*  @param   intStatus  (out) if the intStatus is set to 1, it represents 
*                            the interrupt is generated for the channel
*
*  @return  DMA status
*
******************************************************************************/
	CHAL_DMA_VC4LITE_STATUS_t chal_dma_vc4lite_get_int_status(CHAL_HANDLE
								  handle,
								  cUInt32
								  channel,
								  cBool *
								  intStatus);

/**
*
*  @brief   Prepare a DMA transfer  
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*  @param   channel  (in) DMA channel number 
*  @param   ctrlBlk  (in) buffer to store the DMA control block
* 
*  @return  DMA status  
*
*  @note
******************************************************************************/
	CHAL_DMA_VC4LITE_STATUS_t chal_dma_vc4lite_prepare_transfer(CHAL_HANDLE
								    handle,
								    cUInt32
								    channel,
								    cVoid *
								    ctrlBlkList
#ifdef UNDER_LINUX
								    ,
								    cVoid *
								    ctrlBlkListPHYS
#endif
	    );

/**
*
*  @brief   abort a DMA channel transfer for current DMA control block
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*  @param   channel  (in) DMA channel number 
* 
*  @return  DMA status  
*
*  @note
******************************************************************************/
	CHAL_DMA_VC4LITE_STATUS_t chal_dma_vc4lite_abort_transfer(CHAL_HANDLE
								  handle,
								  cUInt32
								  channel);

/**
*
*  @brief   enable the DMA channel
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*  @param   channel  (in) DMA channel number 
*
*  @return  DMA status  
*
*  @note
******************************************************************************/
	CHAL_DMA_VC4LITE_STATUS_t chal_dma_vc4lite_enable_channel(CHAL_HANDLE
								  handle,
								  cUInt32
								  channel);

/**
*
*  @brief   disable the DMA channel
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*  @param   channel  (in) DMA channel number 
*
*  @return  DMA status  
*
*  @note
******************************************************************************/
	CHAL_DMA_VC4LITE_STATUS_t chal_dma_vc4lite_disable_channel(CHAL_HANDLE
								   handle,
								   cUInt32
								   channel);

/**
*
*  @brief   reset the DMA channel
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*  @param   channel  (in) DMA channel number 
*
*  @return  DMA status  
*
*  @note
******************************************************************************/
	CHAL_DMA_VC4LITE_STATUS_t chal_dma_vc4lite_reset_channel(CHAL_HANDLE
								 handle,
								 cUInt32
								 channel);

/**
*
*  @brief   get the state of the DMA channel
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*  @param   channel  (in) DMA channel number 
*
*  @return  DMA state
*
*  @note
******************************************************************************/
	cUInt32 chal_dma_vc4lite_get_channel_state(CHAL_HANDLE handle,
						   cUInt32 channel);

/**
*
*  @brief   get the control block size
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*
*  @return  DMA control block size  
*
*  @note
******************************************************************************/
	cUInt32 chal_dma_vc4lite_get_ctrlblk_size(CHAL_HANDLE handle);

//******************************************************************************
//
//  @brief   enable the interrupt for the specified DMA channel
//
//  @param   handle   (in) Handle returned in chal_dma_init()
//  @param   channel  (in) DMA channel number 
//
//  @return  DMA status
//
//******************************************************************************
	CHAL_DMA_VC4LITE_STATUS_t chal_dma_vc4lite_enable_int(CHAL_HANDLE
							      handle,
							      cUInt32 channel);

/**
*
*  @brief   get the control block list
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*  @param   ctrlBlkList  (in/out) the ctrl block list with the new added the 
*                                 control block info. The parameter can be set
*                                 to NULL in case of the first ctrl block
*  @param   curCtrlBlk   (in) the memory address for the current ctrl block
*  @param   curCtrlBlkSize  (in) the memory size for the current ctrl block
*  @param   curCtrlBlkInfo  (in) the current control block information
*
*  @return  DMA control block size  
*
*  @note
******************************************************************************/
	CHAL_DMA_VC4LITE_STATUS_t
	    chal_dma_vc4lite_build_ctrlblk_list(CHAL_HANDLE handle,
						cVoid * ctlBlkList,
						cVoid * ctlBlkListPhys,
						cUInt32 ctlBlkItemNum,
						cUInt32 ctlBlkMemSize,
						ChalDmaCtrlBlkInfo_t *
						curCtrlBlkInfo);

/** @} */

#ifdef __cplusplus
}
#endif
#endif				/* _CHAL_DMA_VC4LITE_H_ */
