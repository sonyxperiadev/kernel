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
*  @file   chal_dma.h
*  @brief  DMA cHAL interface
*  @note  
*
*****************************************************************************/
#ifndef	_CHAL_DMA_H_
#define	_CHAL_DMA_H_


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @addtogroup cHAL_Interface 
 * @{
 */

/**
* defines 
******************************************************************************/
#define DMA_CHAN_PRI_HIGH     0
#define DMA_CHAN_PRI_MEDIUM   1
#define DMA_CHAN_PRI_LOW      2
#define DMA_OWNERSHIP_SOFT    0
#define DMA_OWNERSHIP_HW      1

/**
* DMA status
******************************************************************************/
typedef enum {
    CHAL_DMA_STATUS_SUCCESS            = 0x00,    ///<
    CHAL_DMA_STATUS_FAILURE            = 0x01,    ///<
    CHAL_DMA_STATUS_INVALID_PARAMETER  = 0x02,    ///<
    CHAL_DMA_STATUS_INVALID_STATE      = 0x03     ///<
} CHAL_DMA_STATUS_t;

/**
* DMA capabilities
******************************************************************************/
typedef struct
{
    cUInt32  numOfChannel;
    cUInt32  chanLLISize;
    cUInt32  maxBurstSize;
    cUInt32  maxXferSize;
    cUInt32  maxLLINodeNum;
    cUInt32  fixedWordWidth;
}ChalDmaDevCapabilities_t;

/**
* DMA Node memory
******************************************************************************/
typedef struct
{
    cUInt32  LLIVirtualAddr;
    cUInt32  LLIPhysicalAddr;
    cUInt32  size;
}ChalDmaDevNodeMemory_t;

/**
* DMA channel configuration
******************************************************************************/
typedef struct
{ 
    cUInt32  locked;
    cUInt32  tcIntMask;
    cUInt32  errIntMask;
    cUInt32  flowCtrl;
    cUInt32  dstPeripheral;
    cUInt32  srcPeripheral;
    cUInt32  nodeNumRequested;
}ChalDmaChanConfig_t;

/**
* DMA channel control
******************************************************************************/
typedef struct
{
    cUInt32  tcIntEnable;
    cUInt32  prot;
    cUInt32  dstIncrement;
    cUInt32  srcIncrement;
    cUInt32  dstMaster;
    cUInt32  srcMaster;
    cUInt32  dstWidth;
    cUInt32  srcWidth;
    cUInt32  dstBSize;
    cUInt32  srcBSize;
}ChalDmaChanControl_t;

/**
* DMA buffer description
******************************************************************************/
typedef struct
{
    cUInt32  firstBuffer; //signalling first buffer of a dma transfer
    cUInt32  src;
    cUInt32  dst;
    cUInt32  size;
    ChalDmaChanControl_t control;
}ChalDmaBufferDesc_t;

/**
* DMA interrupt status
******************************************************************************/
typedef struct 
{  
    cUInt32 tcInt;
    cUInt32 errInt;
}ChalDmaIntStatus_t;

/**
* DMA definition
******************************************************************************/
#define ASSOCIATE_CHANNEL       8
#define ASSOC_CHAN_NONE         0xFFFFFFFF

/**
*
*  @brief   Initialize DMA hardware and software interface.
*
*  @param   baseAddress (in) mapped address of DMA controller
*
*  @return  Handle of this DMA instance
*
*  @note    This function initializes the DMA controller by doing the followings:
*           1. clear TC interrupt and Error interrupts
*           2. clear channel data structures
*           3. clear channel descriptors list 
*           4. disable all channels
*           5. disable dma core.
*           6. returns capabilities information
*
******************************************************************************/
CHAL_HANDLE chal_dma_init(cUInt32 baseAddress);

/**
*
*  @brief   enable dmac 
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*
*  @return  
*
******************************************************************************/
void chal_dma_enable_dmac(CHAL_HANDLE handle);

/**
*
*  @brief   disable dmac 
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*
*  @return  
*
******************************************************************************/
void chal_dma_disable_dmac(CHAL_HANDLE handle);

/**
*
*  @brief   Clear the DMA controller's interrupt status
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*  @param   mask     (in) bits mask
*
*  @return  
*
******************************************************************************/
void chal_dma_clear_int_status(CHAL_HANDLE handle, cUInt32 mask);

/**
*
*  @brief   Device level configuration 
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*  @param   pDevNode (in) Allocated physical memory information for DMA link node 
*
*  @return  1, always success. 
*
*  @note    DMA Link List Item memory is allocated be driver and passed to cHAL
*           in This function. It should be contigous uncached memory. 
******************************************************************************/
cUInt32 chal_dma_config_device_memory(CHAL_HANDLE handle, ChalDmaDevNodeMemory_t *pDevNode);

/**
*
*  @brief   Get individual capability of the device 
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*  @param   pCap     (out) Pointer to capabilities
*
*  @return  1, always success.  
*
*  @note
******************************************************************************/
cUInt32 chal_dma_get_capabilities(CHAL_HANDLE handle, ChalDmaDevCapabilities_t *pCap);

/**
*
*  @brief   Reset a single channel 
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*  @param   channel  (in) DMA channel
*
*  @return  0, failed; 1, success. 
*
*  @note
******************************************************************************/
cUInt32 chal_dma_reset_channel(CHAL_HANDLE handle, cUInt16 channel);

/**
*
*  @brief   Terminate a DMA transfer gracefully without data lose 
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*  @param   channel  (in) DMA channel 
*
*  @return  0, failed; 1, success. 
*
*  @note
******************************************************************************/
void chal_dma_shutdown_chan(CHAL_HANDLE handle, cUInt32 channel);


/**
*
*  @brief   Disable all the active channel and shutdown DMA controller 
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*
*
*  @note
******************************************************************************/
void chal_dma_shutdown_all_channels(CHAL_HANDLE handle);

/**
*
*  @brief   Get a specific DMA channel 
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*  @param   chan     (in) DMA channel to be used 
*
*
*  @note    
******************************************************************************/
cUInt32 chal_dma_allocate_specific_channel(CHAL_HANDLE handle, cUInt32 chan);
             
/**
*
*  @brief   Configure a dma channel, i.e., in this case channel configure reg 
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*  @param   chan     (in) DMA channel to be configgured 
*  @param   assocChan  (in) DMA associate channel number
*  @param   pConfig  (in) Pointer to channel config parameters 
*
*  @return  Channel number configured if success. 0xffffffff if failed. 
*
*  @note
******************************************************************************/
cUInt32 chal_dma_config_channel(
    CHAL_HANDLE handle, 
    cUInt32 chan, 
    cUInt32 assocChan, 
    ChalDmaChanConfig_t *pConfig
);

/**
*
*  @brief   Add buffer to a DMA channel 
*
*  @param   handle      (in) Handle returned in chal_dma_init()
*  @param   chan        (in) channel number to add buffer.
*  @param   pDesc       (in) DMA buffer description information. 
*  @param   lliXferSize (in) LLI transfer size
*
*  @return  Number of bytes that are not yet queued for dma transfer. 
*
*  @note
*     1. Assume channel is allocated and configured. It assumes no Link List 
*        item is configured.
*     2. Calling function need to check whether DMA can accomadate the entire 
*        transfer or not. Call again for remaining data not yet transfered
******************************************************************************/
cUInt32 chal_dma_add_buffer(CHAL_HANDLE handle, cUInt32 chan, ChalDmaBufferDesc_t *pDesc,
                            cUInt32 lliXferSize);


/**
*
*  @brief   Free the channel previously allocated 
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*  @param   channel  (in) DMA channel to be freed 
*
*  @return  reture status
*
*  @note
******************************************************************************/
CHAL_DMA_STATUS_t chal_dma_free_channel(CHAL_HANDLE handle, cUInt32 channel);

/**
*
*  @brief   Wait for a channel 
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*  @param   channel  (in) DMA channel number 
*
*
*  @note
******************************************************************************/
void chal_dma_wait_channel(CHAL_HANDLE handle, cUInt32 channel);

/**
*
*  @brief   Start a DMA transfer by reloading DMA registers 
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*  @param   channel  (in) DMA channel number 
*  @param   assocChan  (in) DMA associate channel number 
* 
*
*  @note
******************************************************************************/
void chal_dma_start_transfer(
    CHAL_HANDLE handle, 
    cUInt32 channel,
    cUInt32 assocChan
);

/**
*
*  @brief   Stop DMA transfer on the specified channel and lost all data in the fifo 
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*  @param   channel  (in) DMA channel number  
*
*
*  @note
******************************************************************************/
void chal_dma_force_shutdown_chan(CHAL_HANDLE handle, cUInt32 channel);

/**
*
*  @brief   Halt DMA transfer on the specified channel and lost all data in the fifo 
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*  @param   channel  (in) DMA channel number  
*
*
*  @note
******************************************************************************/
void chal_dma_halt_chan(CHAL_HANDLE handle, cUInt32 channel);

/**
*
*  @brief   Disable DMA device. This call does not reset channel and
*           clean up related data structure
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*
*
*  @note
******************************************************************************/
void chal_dma_disable_device(CHAL_HANDLE handle);


/**
*
*  @brief   Set circular mode for LLI 
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*  @param   channel  (in) DMA channel number 
*
*  @return  0, failed; 1, success. 
*
*  @note
******************************************************************************/
cUInt32 chal_dma_set_circular_mode(CHAL_HANDLE handle, cUInt32 channel);

/**
*
*  @brief   Returns the DMA controller's interrupt status 
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*
*  @return  Bit fields of DMA interrupt. 
*
*  @note
******************************************************************************/
void chal_dma_get_int_status(CHAL_HANDLE handle, ChalDmaIntStatus_t *intStatus);

/**
*
*  @brief   Set a device to generate software DMA burst request 
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*
*  @param   device   (in) The device ID to be used.
*
*
*  @note
******************************************************************************/
void chal_dma_soft_burst_request(CHAL_HANDLE handle, cUInt32 device);

/**
*
*  @brief   Set a device to generate software DMA burst request 
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*  @param   device   (in) 
*
*  @return  None 
*
*  @note
******************************************************************************/
void chal_dma_soft_bst_last_request(CHAL_HANDLE handle, cUInt32 device);

/**
*
*  @brief   Set a device to generate software single DMA request 
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*  @param   device   (in) The device ID to be used.
*
*
*  @note
******************************************************************************/
void chal_dma_soft_single_request(CHAL_HANDLE handle, cUInt32 device);

/**
*
*  @brief   Set a device to generate software single DMA request 
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*  @param   device   (in) The device ID to be used.
*
*
*  @note
******************************************************************************/
void chal_dma_soft_sig_last_request(CHAL_HANDLE handle, cUInt32 device);

/**
*
*  @brief   Enables or disables synchronization logic for the DMA 
*           request signals. A bit set to 0 enables the synchronization 
*           logic for a particular group of DMA requests. A bit set to 
*           1 disables the synchronization logic for a particular group 
*           of DMA requests. By default it is reset to 0, and synchronization
*           logic enabled
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*  @param   mask     (in) The group bit to be set
*
*
*  @note
******************************************************************************/
void chal_dma_set_sync(CHAL_HANDLE handle, cUInt16 mask);

/**
*
*  @brief   Returns the DMA controller's channel status 
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*
*  @return  Bit fields of DMA channel status 
*
*  @note
******************************************************************************/
cUInt32 chal_dma_get_channel_status(CHAL_HANDLE handle);


/** @} */

#ifdef __cplusplus
}
#endif

#endif /* _CHAL_DMA_H_ */
