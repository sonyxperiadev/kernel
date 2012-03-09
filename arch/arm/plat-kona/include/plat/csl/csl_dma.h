/*****************************************************************************
*  Copyright 2001 - 2010 Broadcom Corporation.  All rights reserved.
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
*   @file   csl_dma.h
*
*   @brief  DMA CSL layer 
*
****************************************************************************/
/**
*
* @defgroup DMAGroup Direct Memory Access
*
* @brief This group defines the APIs for DMA CSL layer
*
* @ingroup CSLGroup
*****************************************************************************/

#ifndef	_CSL_DMA_H_
#define	_CSL_DMA_H_

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * @addtogroup DMAGroup 
 * @{
 */

/**
*
*  DMA channel structure definition
*
*****************************************************************************/
	typedef struct {
		Boolean bUsed;
		Boolean multiLLIs;
		Boolean busy;
		UInt8 src_peri_id;
		UInt8 dst_peri_id;
		Dma_Chan_Info chanInfo;
	} Dma_Channel;

/**
*
*  DMA channel descriptor structure list definition
*
*****************************************************************************/
#define DMA_DESCRIPTOR_NUM           256
	typedef struct {
		Dma_Chan_Desc desc[DMA_DESCRIPTOR_NUM];
	} Dma_Chan_List;

/**
*
*  DMA interrupt type definition
*
*****************************************************************************/
	typedef struct {
		UInt32 tcInt;
		UInt32 errInt;
	} DMA_Interrupt_t;

/**
*
*  DMA channel priority definition
*
*****************************************************************************/
#define DMA_CHAN_PRI_HIGH     0
#define DMA_CHAN_PRI_MEDIUM   1
#define DMA_CHAN_PRI_LOW      2

/**
*
*  DMA channel configuration register definition
*
*****************************************************************************/
#define DMA_CHCFG_ENABLE_ITC		(1<<15)
#define DMA_CHCFG_ENABLE_IE			(1<<14)
#define DMA_CHCFG_FLOW_CTRL_MASK	0x7
#define DMA_CHCFG_FLOW_CTRL_SHIFT	11
#define DMA_CHCFG_DESTPERI_MASK		0x1F
#define DMA_CHCFG_DESTPERI_SHIFT	6
#define DMA_CHCFG_SRCPERI_MASK		0x1F
#define DMA_CHCFG_SRCPERI_SHIFT		1

/**
*
*  This function initialize dma
*  @param		void 
*
*  @return	    void
*
*****************************************************************************/
	void csl_dma_init(void *base, UInt32 LL_virt, UInt32 LL_phy,
			  UInt32 LL_Size);

/**
*
*  This function deinitialize dma
*  @param		void 
*
*  @return	    void
*
*****************************************************************************/
	void csl_dma_deinit(void);

/**
*
*  This function reset dma channel
*  @param		chanID (in) channel identification 
*
*  @return	    status (out) 0 or 1  
*
*****************************************************************************/
	UInt32 csl_dma_reset_channel(UInt16 chanID);

/**
*
*  This function shutdown all dma channels
*  @param		void 
*
*  @return	    void
*
*****************************************************************************/
	void csl_dma_shutdown_all_channel(void);

/**
*
*  This function configure dma channel
*  @param       chanID       (in) channel number
*  @param       pChanInfo    (in) pointer to dma channe info structure
*
*  @return	    void
*
*****************************************************************************/
	Int32 csl_dma_config_channel(DMA_CHANNEL chanID,
				     Dma_Chan_Info * pChanInfo);

/**
*
*  This function allocates dma channel
*  @param		srcID (in) source identification
*  @param       dstID (in) destination identification
*
*  @return	    chan (out) channle identification
*
*****************************************************************************/
	Int32 csl_dma_obtain_channel(UInt8 srcID, UInt8 dstID);

/**
*
*  This function release dma channel
*  @param		chanID (in) channel identification
*
*  @return	    status (out) 0 or 1
*
*****************************************************************************/
	UInt32 csl_dma_release_channel(DMA_CHANNEL chanID);

/**
*
*  This function start dma channel transfer
*  @param		chanID (in) channel identification
*
*  @return	    void
*
*****************************************************************************/
	void csl_dma_start_transfer(DMA_CHANNEL chanID, DMADRV_LLI_T pLLI);

/**
*
*  This function stop dma channel trnasfer
*  @param		chanID (in) channel identification
*
*  @return	    void
*
*****************************************************************************/
	void csl_dma_stop_transfer(DMA_CHANNEL chanID);

/**
*
*  This function wait channel to be ready
*  @param		chanID (in) channel identification
*
*  @return	    void
*
*****************************************************************************/
	void csl_dma_wait_channel(DMA_CHANNEL chanID);

/**
*
*  This function force shutdow channel operation
*  @param		chanID (in) channel identification
*
*  @return	    void
*
*****************************************************************************/
	void csl_dma_force_shutdown_channel(DMA_CHANNEL chanID);

/**
*
*  This function shutdow channel operation
*  @param		chanID (in) channel identification
*
*  @return	    void
*
*****************************************************************************/
	void csl_dma_shutdown_channel(DMA_CHANNEL chanID);

/**
*
*  This function set the DMA list to form a closed loop 
*  @param		chanID (in) channel identification
*
*  @return	    void
*
*****************************************************************************/
	UInt32 csl_dma_set_repeat_mode(DMA_CHANNEL chanID);

/**
*
*  This function bind data buffer for the DMA channel
*  @param		chanID (in) channel number
*  @param       pData     (in) pointer to dma channel data buffer 
*
*  @return	    status    (out) 0 or 1
*
*****************************************************************************/
	Int32 csl_dma_bind_data(DMA_CHANNEL chanID,
				Dma_Data * pData,
				Boolean multiLLIs, DMADRV_LLI_T * pLLI);

/**
*
*  This function 
*  @param		intStatus (in)  a pointer to DMA_Interrupt_t
*
*  @return	    void
*
*****************************************************************************/
	void csl_dma_process_callback(DMA_Interrupt_t * intStatus);

/**
*
*  This function get DMA terminal count and error interrupt status
*  @param		intStatus (in)  a pointer to DMA_Interrupt_t
*
*  @return	    void
*
*****************************************************************************/
	void csl_dma_get_intr_status(DMA_Interrupt_t * intStatus);

/**
*
*  This function write a bit mask to clear DMA terminal count and error interrupt
*  @param		mask (in)  bit mask set for 0 or 1
*
*  @return	    void
*
*****************************************************************************/
	void csl_dma_clear_intr_status(UInt32 mask);

#ifdef __cplusplus
}
#endif
#endif				/* _CSL_DMA_H_ */
