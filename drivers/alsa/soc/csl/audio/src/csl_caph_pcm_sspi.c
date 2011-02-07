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
*  @file   csl_caph_pcm_sspi.c
*
*  @brief  PCM device CSL layer implementation
*
****************************************************************************/
//#include <string.h>
#include "mobcom_types.h"
#include "auddrv_def.h"
#include "chal_types.h"
//#include "chal_sspi.h"
#include "csl_caph_pcm_sspi.h"
//#include "brcm_rdb_sspil.h"
//#include "dbg.h"

#define SSPI_HW_WORD_LEN_32Bit                    32
#define SSPI_HW_WORD_LEN_25Bit                    25
#define SSPI_HW_WORD_LEN_24Bit                    24
#define SSPI_HW_WORD_LEN_16Bit                    16
#define SSPI_HW_WORD_LEN_8Bit                     8
//******************************************************************************
// Local Definitions
//******************************************************************************
//static chal_sspi_task_conf_t task_conf;
//static chal_sspi_seq_conf_t seq_conf;


//******************************************************************************
//Function Definition
//******************************************************************************


//******************************************************************************
//
//  Function Name:	pcm_config_dma
//  
//  Description:	This function ocnfigures DMA for PCM operation
//
//******************************************************************************
// Temporarily comment it out, since a compilation warning happens because
// this API is not called anywhere in the code at the moment.
#if 0
static CSL_PCM_OPSTATUS_t pcm_config_dma(CSL_PCM_HANDLE handle, csl_pcm_config_device_t *devCfg)
{
    CHAL_SSPI_HANDLE_t *pDevice = (CHAL_SSPI_HANDLE_t *)handle;
    UInt32 size;
//    CHAL_SSPI_FIFO_ID_t fifoID;

    size = devCfg->xferSize >> 2;

    if(handle == NULL)
        return CSL_PCM_ERR_HANDLE;
	    
    // 
    //  configure DMA for PCM operation
    //
    chal_sspi_dma_set_tx_size(pDevice, SSPI_DMA_CHAN_SEL_CHAN_TX0,(size > 0x400) ? 0x400 : size);        
    chal_sspi_dma_set_burstsize(pDevice, SSPI_DMA_CHAN_SEL_CHAN_TX0, CHAL_SSPI_DMA_BURSTSIZE_16BYTES);
    chal_sspi_dma_set_burstsize(pDevice, SSPI_DMA_CHAN_SEL_CHAN_RX0, CHAL_SSPI_DMA_BURSTSIZE_16BYTES);
    chal_sspi_enable_dma(pDevice, SSPI_DMA_CHAN_SEL_CHAN_TX0, SSPI_FIFO_ID_TX0, 1);
    chal_sspi_enable_dma(pDevice, SSPI_DMA_CHAN_SEL_CHAN_RX0, SSPI_FIFO_ID_RX0, 1);

    if(!devCfg->interleave) {
        chal_sspi_dma_set_tx_size(pDevice, SSPI_DMA_CHAN_SEL_CHAN_TX1,(size > 0x400) ? 0x400 : size);
        chal_sspi_dma_set_burstsize(pDevice, SSPI_DMA_CHAN_SEL_CHAN_TX1, CHAL_SSPI_DMA_BURSTSIZE_16BYTES);
        chal_sspi_dma_set_burstsize(pDevice, SSPI_DMA_CHAN_SEL_CHAN_RX1, CHAL_SSPI_DMA_BURSTSIZE_16BYTES);
        chal_sspi_enable_dma(pDevice, SSPI_DMA_CHAN_SEL_CHAN_TX1, SSPI_FIFO_ID_TX1, 1);
        chal_sspi_enable_dma(pDevice, SSPI_DMA_CHAN_SEL_CHAN_RX1, SSPI_FIFO_ID_RX1, 1);
    }
	
    return CSL_PCM_SUCCESS;
}
#endif
//******************************************************************************
//
//  Function Name:	csl_pcm_init
//  
//  Description:	This function initializes the CSL layer
//
//******************************************************************************
CSL_PCM_HANDLE csl_pcm_init(cUInt32 baseAddr)
{
    CSL_PCM_HANDLE handle;
#if 0
	CSL_PCM_HANDLE_t *pDevice;
	
    dprintf(DBG_L1, "+csl_pcm_init\r\n");
	
    handle = chal_sspi_init(baseAddr);    
    pDevice = (CSL_PCM_HANDLE_t *)handle;
    //How to use pDevice?
    pDevice = pDevice;
    
    if(handle == NULL)
    {
    	dprintf(1,"csl_pcm_init failed\r\n");
    	return NULL;
    }
    
    chal_sspi_set_type(handle, SSPI_TYPE_LITE);
    
    dprintf(DBG_L1, "-csl_pcm_init\r\n");
#endif    
    return handle;
    
}

//******************************************************************************
//
//  Function Name:	csl_pcm_deinit
//  
//  Description:	This function deinitializes the CSL layer
//
//******************************************************************************
CSL_PCM_OPSTATUS_t csl_pcm_deinit(CSL_PCM_HANDLE handle)
{
#if 0
    CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *)handle;
    CHAL_SSPI_STATUS_t status;
    
    dprintf(DBG_L1, "+csl_pcm_deinit\r\n");

    if(handle == NULL)
    {
    	dprintf(1,"csl_pcm_deinit failed\r\n");
    	return CSL_PCM_ERR_HANDLE;
    }
    
    status = chal_sspi_deinit(pDevice);
    
    dprintf(DBG_L1, "-csl_pcm_deinit\r\n");

    if(status == CHAL_SSPI_STATUS_SUCCESS)
        return CSL_PCM_SUCCESS;
    else if(status == CHAL_SSPI_STATUS_ILLEGAL_HANDLE)
        return CSL_PCM_ERR_HANDLE;
#endif    
    return CSL_PCM_SUCCESS;
}

//******************************************************************************
//
//  Function Name:	csl_pcm_start
//  
//  Description:	This function starts scheduler operation
//
//******************************************************************************
CSL_PCM_OPSTATUS_t csl_pcm_start(CSL_PCM_HANDLE handle, csl_pcm_config_device_t *config)
{
#if 0
    CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *)handle;
    CHAL_SSPI_STATUS_t status;
	
//    pcm_config_dma(handle, config);
		
	//
	// enable scheduler operation
	//
    status = chal_sspi_enable_scheduler(pDevice, 1);

    chal_sspi_enable_fifo_pio_start_stop_intr(pDevice, 
                                              SSPI_FIFO_ID_TX0, 
                                              TRUE, 
                                              TRUE); 
    chal_sspi_enable_fifo_pio_start_stop_intr(pDevice, 
                                              SSPI_FIFO_ID_TX1, 
                                              TRUE, 
                                              TRUE); 
    chal_sspi_enable_fifo_pio_start_stop_intr(pDevice, 
                                              SSPI_FIFO_ID_TX2, 
                                              TRUE, 
                                              TRUE); 
    chal_sspi_enable_fifo_pio_start_stop_intr(pDevice, 
                                              SSPI_FIFO_ID_TX3, 
                                              TRUE, 
                                              TRUE); 
    chal_sspi_enable_fifo_pio_start_stop_intr(pDevice, 
                                              SSPI_FIFO_ID_RX0, 
                                              TRUE, 
                                              TRUE); 
    chal_sspi_enable_fifo_pio_start_stop_intr(pDevice, 
                                              SSPI_FIFO_ID_RX1, 
                                              TRUE, 
                                              TRUE); 
    chal_sspi_enable_fifo_pio_start_stop_intr(pDevice, 
                                              SSPI_FIFO_ID_RX2, 
                                              TRUE, 
                                              TRUE); 
    chal_sspi_enable_fifo_pio_start_stop_intr(pDevice, 
                                              SSPI_FIFO_ID_RX3, 
                                              TRUE, 
                                              TRUE); 


    if(status == CHAL_SSPI_STATUS_SUCCESS)
        return CSL_PCM_SUCCESS;
    else if(status == CHAL_SSPI_STATUS_ILLEGAL_HANDLE)
        return CSL_PCM_ERR_HANDLE;
#endif    
    return CSL_PCM_SUCCESS;
}

//******************************************************************************
//
//  Function Name:	csl_pcm_stop_tx
//  
//  Description:	This function stops transmit operation
//
//******************************************************************************
CSL_PCM_OPSTATUS_t csl_pcm_stop_tx(CSL_PCM_HANDLE handle, UInt8 channel)
{
#if 0
    CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *)handle;
    CHAL_SSPI_STATUS_t status;

    //
    // reset transmit channel
    //
    if(channel == CSL_PCM_CHAN_TX0) {
        chal_sspi_fifo_reset(pDevice, SSPI_FIFO_ID_TX0);	
    }
    else if(channel == CSL_PCM_CHAN_TX1) {
        chal_sspi_fifo_reset(pDevice, SSPI_FIFO_ID_TX1);	
    }
    
    //
    // disable scheduler operation
    //
    status = chal_sspi_enable_scheduler(pDevice, 0);

    chal_sspi_enable_fifo_pio_start_stop_intr(pDevice, 
                                              SSPI_FIFO_ID_TX0, 
                                              FALSE, 
                                              FALSE); 
    chal_sspi_enable_fifo_pio_start_stop_intr(pDevice, 
                                              SSPI_FIFO_ID_TX1, 
                                              FALSE, 
                                              FALSE); 
    chal_sspi_enable_fifo_pio_start_stop_intr(pDevice, 
                                              SSPI_FIFO_ID_TX2, 
                                              FALSE, 
                                              FALSE); 
    chal_sspi_enable_fifo_pio_start_stop_intr(pDevice, 
                                              SSPI_FIFO_ID_TX3, 
                                              FALSE, 
                                              FALSE); 

    if(status == CHAL_SSPI_STATUS_SUCCESS)
        return CSL_PCM_SUCCESS;
    else if(status == CHAL_SSPI_STATUS_ILLEGAL_HANDLE)
        return CSL_PCM_ERR_HANDLE;
#endif    
    return CSL_PCM_SUCCESS;
}

//******************************************************************************
//
//  Function Name:	csl_pcm_stop_rx
//  
//  Description:	This function stops receive operation
//
//******************************************************************************
CSL_PCM_OPSTATUS_t csl_pcm_stop_rx(CSL_PCM_HANDLE handle, UInt8 channel)
{
#if 0
    CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *)handle;
    CHAL_SSPI_STATUS_t status;

    //
    // reset receive channel
    //
    if(channel == CSL_PCM_CHAN_RX0) {
        chal_sspi_fifo_reset(pDevice, SSPI_FIFO_ID_RX0);
    }
    else if (channel == CSL_PCM_CHAN_RX1) {    
        chal_sspi_fifo_reset(pDevice, SSPI_FIFO_ID_RX1);
    }

    //
    // disable scheduler operation
    //
    status = chal_sspi_enable_scheduler(pDevice, 0);

    chal_sspi_enable_fifo_pio_start_stop_intr(pDevice, 
                                              SSPI_FIFO_ID_RX0, 
                                              FALSE, 
                                              FALSE); 
    chal_sspi_enable_fifo_pio_start_stop_intr(pDevice, 
                                              SSPI_FIFO_ID_RX1, 
                                              FALSE, 
                                              FALSE); 
    chal_sspi_enable_fifo_pio_start_stop_intr(pDevice, 
                                              SSPI_FIFO_ID_RX2, 
                                              FALSE, 
                                              FALSE); 
    chal_sspi_enable_fifo_pio_start_stop_intr(pDevice, 
                                              SSPI_FIFO_ID_RX3, 
                                              FALSE, 
                                              FALSE); 

    if(status == CHAL_SSPI_STATUS_SUCCESS)
        return CSL_PCM_SUCCESS;
    else if(status == CHAL_SSPI_STATUS_ILLEGAL_HANDLE)
        return CSL_PCM_ERR_HANDLE;
#endif    
    return CSL_PCM_SUCCESS;
}


CSL_PCM_OPSTATUS_t csl_pcm_pause(CSL_PCM_HANDLE handle)
{
#if 0
    CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *)handle;
    CHAL_SSPI_STATUS_t status;

    //
    // disable scheduler operation
    //
    status = chal_sspi_enable_scheduler(pDevice, 0);

    if(status == CHAL_SSPI_STATUS_SUCCESS)
        return CSL_PCM_SUCCESS;
    else if(status == CHAL_SSPI_STATUS_ILLEGAL_HANDLE)
        return CSL_PCM_ERR_HANDLE;
#endif    
    return CSL_PCM_SUCCESS;
}


CSL_PCM_OPSTATUS_t csl_pcm_resume(CSL_PCM_HANDLE handle)
{
#if 0
    CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *)handle;
    CHAL_SSPI_STATUS_t status;

    //
    // enable scheduler operation
    //
    status = chal_sspi_enable_scheduler(pDevice, 1);

    if(status == CHAL_SSPI_STATUS_SUCCESS)
        return CSL_PCM_SUCCESS;
    else if(status == CHAL_SSPI_STATUS_ILLEGAL_HANDLE)
        return CSL_PCM_ERR_HANDLE;
#endif    
    return CSL_PCM_SUCCESS;
}

//******************************************************************************
//
//  Function Name:	csl_pcm_config
//  
//  Description:	This function configures SSPI as PCM operation
//
//******************************************************************************
CSL_PCM_OPSTATUS_t csl_pcm_config(CSL_PCM_HANDLE handle, csl_pcm_config_device_t *configDev, 
                                  csl_pcm_config_tx_t *configTx, csl_pcm_config_rx_t *configRx)
{
#if 0
    CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *)handle;
    cUInt32 frmMask = 1;
    CHAL_SSPI_PROT_t protocol;
    csl_pcm_config_device_t *devCfg = configDev;

    if(handle == NULL)
    {
    	dprintf(1,"csl_pcm_config failed\r\n");
        return CSL_PCM_ERR_HANDLE;
    }

    if((devCfg->protocol == CSL_PCM_PROTOCOL_MONO) && (devCfg->format == CSL_PCM_WORD_LENGTH_16_BIT))
        protocol = SSPI_PROT_MONO_16B_PCM;
    else if((devCfg->protocol == CSL_PCM_PROTOCOL_MONO) && (devCfg->format == CSL_PCM_WORD_LENGTH_24_BIT))
        protocol = SSPI_PROT_MONO_25B_PCM;
    else if((devCfg->protocol == CSL_PCM_PROTOCOL_STEREO) && (devCfg->format == CSL_PCM_WORD_LENGTH_16_BIT))
        protocol = SSPI_PROT_STEREO_16B_PCM;
    else if((devCfg->protocol == CSL_PCM_PROTOCOL_STEREO) && (devCfg->format == CSL_PCM_WORD_LENGTH_24_BIT))
        protocol = SSPI_PROT_STEREO_25B_PCM;    
    
    //
    // task_conf struct initialization
    //
    memset(&task_conf, 0, sizeof(task_conf));
    
    //
    // soft reset sspi instance
    //
    chal_sspi_soft_reset(pDevice);
    
    //
    // set sspi at idle state
    // 
    if(chal_sspi_set_idle_state(pDevice, protocol))
    {
        dprintf(1,"csl_pcm_config failed \r\n");
        return CSL_PCM_ERROR;
    }
    
    //if(chal_sspi_set_clk_src_select(&pDevice, SSPI_CLK_SRC_INTCLK))
    //    return SSPI_HW_ERROR;
	
    chal_sspi_set_clk_divider(handle, SSPI_CLK_DIVIDER0, 0);
	chal_sspi_set_clk_divider(handle, SSPI_CLK_REF_DIVIDER, 0);
	chal_sspi_set_clk_src_select(handle, SSPI_CLK_SRC_CAPHCLK);
	chal_sspi_set_caph_clk(handle, SSPI_CAPH_CLK_TRIG_48KHZ,
        SSPI_HW_WORD_LEN_16Bit, 2);

    switch(protocol)
    {
        case SSPI_PROT_MONO_16B_PCM:
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_RX0, SSPI_FIFO_SIZE_FULL);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_RX1, SSPI_FIFO_SIZE_NONE);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_RX2, SSPI_FIFO_SIZE_NONE);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_RX3, SSPI_FIFO_SIZE_NONE);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_TX0, SSPI_FIFO_SIZE_FULL);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_TX1, SSPI_FIFO_SIZE_NONE);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_TX2, SSPI_FIFO_SIZE_NONE);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_TX3, SSPI_FIFO_SIZE_NONE);

            chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_RX0, SSPI_FIFO_DATA_PACK_16BIT);
            chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_RX1, SSPI_FIFO_DATA_PACK_NONE);
            chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_TX0, SSPI_FIFO_DATA_PACK_16BIT);
            chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_TX1, SSPI_FIFO_DATA_PACK_NONE);
#if 0 // Need to comment out??
            chal_sspi_set_clk_divider(pDevice, SSPI_CLK_DIVIDER0, 0);
            chal_sspi_set_clk_divider(pDevice, SSPI_CLK_DIVIDER1, 1);
            chal_sspi_set_clk_divider(pDevice, SSPI_CLK_DIVIDER2, 15);
            chal_sspi_set_clk_divider(pDevice, SSPI_CLK_REF_DIVIDER, 17);
#endif            
            break;

        case SSPI_PROT_MONO_25B_PCM:
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_RX0, SSPI_FIFO_SIZE_FULL);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_RX1, SSPI_FIFO_SIZE_NONE);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_RX2, SSPI_FIFO_SIZE_NONE);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_RX3, SSPI_FIFO_SIZE_NONE);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_TX0, SSPI_FIFO_SIZE_FULL);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_TX1, SSPI_FIFO_SIZE_NONE);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_TX2, SSPI_FIFO_SIZE_NONE);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_TX3, SSPI_FIFO_SIZE_NONE);

            chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_RX0, SSPI_FIFO_DATA_PACK_NONE);
            chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_RX1, SSPI_FIFO_DATA_PACK_NONE);
            chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_TX0, SSPI_FIFO_DATA_PACK_NONE);
            chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_TX1, SSPI_FIFO_DATA_PACK_NONE);

//            chal_sspi_set_clk_divider(pDevice, SSPI_CLK_DIVIDER0, 0);
//            chal_sspi_set_clk_divider(pDevice, SSPI_CLK_DIVIDER1, 1);
//            chal_sspi_set_clk_divider(pDevice, SSPI_CLK_DIVIDER2, 15);
//            chal_sspi_set_clk_divider(pDevice, SSPI_CLK_REF_DIVIDER, 17);
            break;
        
        case SSPI_PROT_STEREO_16B_PCM:
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_RX0, SSPI_FIFO_SIZE_HALF);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_RX1, SSPI_FIFO_SIZE_HALF);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_RX2, SSPI_FIFO_SIZE_NONE);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_RX3, SSPI_FIFO_SIZE_NONE);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_TX0, SSPI_FIFO_SIZE_HALF);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_TX1, SSPI_FIFO_SIZE_HALF);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_TX2, SSPI_FIFO_SIZE_NONE);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_TX3, SSPI_FIFO_SIZE_NONE);

            chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_RX0, SSPI_FIFO_DATA_PACK_16BIT);
            chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_RX1, SSPI_FIFO_DATA_PACK_16BIT);
            chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_TX0, SSPI_FIFO_DATA_PACK_16BIT);
            chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_TX1, SSPI_FIFO_DATA_PACK_16BIT);

//            if(devCfg->protocol == CSL_PCM_PROTOCOL_STEREO) {
//                chal_sspi_set_clk_divider(pDevice, SSPI_CLK_DIVIDER0, 0);
//                chal_sspi_set_clk_divider(pDevice, SSPI_CLK_DIVIDER1, 1);
//                chal_sspi_set_clk_divider(pDevice, SSPI_CLK_DIVIDER2, 15);
//                chal_sspi_set_clk_divider(pDevice, SSPI_CLK_REF_DIVIDER, 7);
//            }
//            break;

        case SSPI_PROT_STEREO_25B_PCM:
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_RX0, SSPI_FIFO_SIZE_HALF);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_RX1, SSPI_FIFO_SIZE_HALF);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_RX2, SSPI_FIFO_SIZE_NONE);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_RX3, SSPI_FIFO_SIZE_NONE);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_TX0, SSPI_FIFO_SIZE_HALF);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_TX1, SSPI_FIFO_SIZE_HALF);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_TX2, SSPI_FIFO_SIZE_NONE);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_TX3, SSPI_FIFO_SIZE_NONE);

            chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_RX0, SSPI_FIFO_DATA_PACK_NONE);
            chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_RX1, SSPI_FIFO_DATA_PACK_NONE);
            chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_TX0, SSPI_FIFO_DATA_PACK_NONE);
            chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_TX1, SSPI_FIFO_DATA_PACK_NONE);

//            chal_sspi_set_clk_divider(pDevice, SSPI_CLK_DIVIDER0, 0);
//            chal_sspi_set_clk_divider(pDevice, SSPI_CLK_DIVIDER1, 1);
//            chal_sspi_set_clk_divider(pDevice, SSPI_CLK_DIVIDER2, 15);
//            chal_sspi_set_clk_divider(pDevice, SSPI_CLK_REF_DIVIDER, 7);
            break;
    
        case SSPI_PROT_3CHAN_16B_TDM_PCM:
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_RX0, SSPI_FIFO_SIZE_HALF);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_RX1, SSPI_FIFO_SIZE_QUARTER);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_RX2, SSPI_FIFO_SIZE_QUARTER);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_RX3, SSPI_FIFO_SIZE_NONE);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_TX0, SSPI_FIFO_SIZE_HALF);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_TX1, SSPI_FIFO_SIZE_QUARTER);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_TX2, SSPI_FIFO_SIZE_QUARTER);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_TX3, SSPI_FIFO_SIZE_NONE);

            chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_RX0, SSPI_FIFO_DATA_PACK_16BIT);
            chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_RX1, SSPI_FIFO_DATA_PACK_16BIT);
            chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_RX2, SSPI_FIFO_DATA_PACK_16BIT);
            chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_TX0, SSPI_FIFO_DATA_PACK_16BIT);
            chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_TX1, SSPI_FIFO_DATA_PACK_16BIT);
            chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_TX2, SSPI_FIFO_DATA_PACK_16BIT);

//            chal_sspi_set_clk_divider(pDevice, SSPI_CLK_DIVIDER0, 0);
//            chal_sspi_set_clk_divider(pDevice, SSPI_CLK_DIVIDER1, 1);
//            chal_sspi_set_clk_divider(pDevice, SSPI_CLK_DIVIDER2, 15);
//            chal_sspi_set_clk_divider(pDevice, SSPI_CLK_REF_DIVIDER, 4);
            break;

        case SSPI_PROT_4CHAN_16B_TDM_PCM:
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_RX0, SSPI_FIFO_SIZE_QUARTER);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_RX1, SSPI_FIFO_SIZE_QUARTER);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_RX2, SSPI_FIFO_SIZE_QUARTER);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_RX3, SSPI_FIFO_SIZE_QUARTER);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_TX0, SSPI_FIFO_SIZE_QUARTER);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_TX1, SSPI_FIFO_SIZE_QUARTER);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_TX2, SSPI_FIFO_SIZE_QUARTER);
            chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_TX3, SSPI_FIFO_SIZE_QUARTER);

            chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_RX0, SSPI_FIFO_DATA_PACK_16BIT);
            chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_RX1, SSPI_FIFO_DATA_PACK_16BIT);
            chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_RX2, SSPI_FIFO_DATA_PACK_16BIT);
            chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_RX3, SSPI_FIFO_DATA_PACK_16BIT);
            chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_TX0, SSPI_FIFO_DATA_PACK_16BIT);
            chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_TX1, SSPI_FIFO_DATA_PACK_16BIT);
            chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_TX2, SSPI_FIFO_DATA_PACK_16BIT);
            chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_TX3, SSPI_FIFO_DATA_PACK_16BIT);

//            chal_sspi_set_clk_divider(pDevice, SSPI_CLK_DIVIDER0, 0);
//            chal_sspi_set_clk_divider(pDevice, SSPI_CLK_DIVIDER1, 1);
//            chal_sspi_set_clk_divider(pDevice, SSPI_CLK_DIVIDER2, 15);
//            chal_sspi_set_clk_divider(pDevice, SSPI_CLK_REF_DIVIDER, 3);
            break;

        default:
//            chal_sspi_set_clk_divider(pDevice, SSPI_CLK_DIVIDER0, 0);
//            chal_sspi_set_clk_divider(pDevice, SSPI_CLK_DIVIDER1, 1);
//            chal_sspi_set_clk_divider(pDevice, SSPI_CLK_DIVIDER2, 15);
//            chal_sspi_set_clk_divider(pDevice, SSPI_CLK_REF_DIVIDER, 17);
            break;
    }

    //
    // SSPI mode configuration 
    //
    chal_sspi_set_mode(pDevice, (CHAL_SSPI_MODE_t)(devCfg->mode));
    
    // 
    // enable sspi operation
    // 
    chal_sspi_enable(pDevice, 1);

    // Need to figure out a good way to choose between the following two lines.
#if 0 
    // For DSP PCM test.
    chal_sspi_set_fifo_pio_threshhold(pDevice, SSPI_FIFO_ID_RX0, 0x1F, 0x1F);
#else    
    // For ARM PCM test.
    chal_sspi_set_fifo_pio_threshhold(pDevice, SSPI_FIFO_ID_RX0, 0x1, 0x1);
#endif


    chal_sspi_set_fifo_pio_threshhold(pDevice, SSPI_FIFO_ID_RX1, 0x1, 0x1);
    chal_sspi_set_fifo_pio_threshhold(pDevice, SSPI_FIFO_ID_RX2, 0x1, 0x1);
    chal_sspi_set_fifo_pio_threshhold(pDevice, SSPI_FIFO_ID_RX3, 0x1, 0x1);
    chal_sspi_set_fifo_pio_threshhold(pDevice, SSPI_FIFO_ID_TX0, 0x1, 0x1);
    chal_sspi_set_fifo_pio_threshhold(pDevice, SSPI_FIFO_ID_TX1, 0x1, 0x1);
    chal_sspi_set_fifo_pio_threshhold(pDevice, SSPI_FIFO_ID_TX2, 0x1, 0x1);
    chal_sspi_set_fifo_pio_threshhold(pDevice, SSPI_FIFO_ID_TX3, 0x1, 0x1);
    
    switch(protocol)
    {
    	case SSPI_PROT_MONO_16B_PCM:
    	case SSPI_PROT_MONO_25B_PCM:
            if(configTx->enable)
                chal_sspi_set_fifo_threshold(pDevice, SSPI_FIFO_ID_TX0, 0x10);
            task_conf.chan_sel = SSPI_CHAN_SEL_CHAN0;
            task_conf.cs_sel = SSPI_CS_SEL_CS0;
            task_conf.rx_sel = (configTx->loopback_enable) ? SSPI_RX_SEL_COPY_TX0
                                                : SSPI_RX_SEL_RX0;
            task_conf.tx_sel = SSPI_TX_SEL_TX0;
            task_conf.div_sel = SSPI_CLK_DIVIDER0;
            task_conf.seq_ptr = 0;

            // Max transfer size is set to 4K bytes in non_continuous mode transfer
            if((devCfg->xferSize >> 2) > 0x400) {
                task_conf.loop_cnt = 0;
                task_conf.continuous = 1;
            }
            else {
                task_conf.loop_cnt = (devCfg->xferSize >> 2) - 1;
                task_conf.continuous = 0;
            }

            task_conf.init_cond_mask = (configTx->enable) ? SSPI_TASK_INIT_COND_THRESHOLD_TX0 : 0;
            task_conf.wait_before_start = 1;
            if(chal_sspi_set_task(pDevice, 0, protocol, &task_conf))
                return(CSL_PCM_ERR_TASK);

            seq_conf.tx_enable = (configTx->enable || configRx->loopback_enable) ? TRUE : FALSE;
            seq_conf.rx_enable = (configRx->enable || configTx->loopback_enable) ? TRUE : FALSE;
            seq_conf.cs_activate = 1;
            seq_conf.cs_deactivate = 1;
            seq_conf.pattern_mode = 0;
            seq_conf.rep_cnt = 1;
            seq_conf.opcode = SSPI_SEQ_OPCODE_COND_JUMP;
            seq_conf.rx_fifo_sel = 0;
            seq_conf.tx_fifo_sel = 0;
            seq_conf.frm_sel = 0;
            seq_conf.rx_sidetone_on = 0;
            seq_conf.tx_sidetone_on = 0;
            seq_conf.next_pc = 0;
            if(chal_sspi_set_sequence(pDevice, 0, protocol, &seq_conf))
                return(CSL_PCM_ERR_SEQUENCE);

            seq_conf.tx_enable = FALSE;
            seq_conf.rx_enable = FALSE;
            seq_conf.cs_activate = 0;
            seq_conf.cs_deactivate = 0;
            seq_conf.pattern_mode = 0;
            seq_conf.rep_cnt = 0;
            seq_conf.opcode = SSPI_SEQ_OPCODE_STOP;
            seq_conf.rx_fifo_sel = 0;
            seq_conf.tx_fifo_sel = 0;
            seq_conf.frm_sel = 0;
            seq_conf.rx_sidetone_on = 0;
            seq_conf.tx_sidetone_on = 0;
            seq_conf.next_pc = 0;
            if(chal_sspi_set_sequence(pDevice, 1, protocol, &seq_conf))
                return(CSL_PCM_ERR_SEQUENCE);

            if(chal_sspi_set_frame(pDevice, &frmMask, protocol, (protocol==SSPI_PROT_MONO_25B_PCM) ? 24 : 16, 0))
                return(CSL_PCM_ERR_FRAME);
            break;    	

        case SSPI_PROT_STEREO_16B_PCM:
            if(configTx->enable) {
                chal_sspi_set_fifo_threshold(pDevice, SSPI_FIFO_ID_TX0, 0x10);
                chal_sspi_set_fifo_threshold(pDevice, SSPI_FIFO_ID_TX1, 0x10);
            }

            task_conf.chan_sel = SSPI_CHAN_SEL_CHAN0;
            task_conf.cs_sel = SSPI_CS_SEL_CS0;
            task_conf.rx_sel = (configTx->loopback_enable) ? SSPI_RX_SEL_COPY_TX0
                                                : SSPI_RX_SEL_RX0;
            task_conf.tx_sel = SSPI_TX_SEL_TX0;
            task_conf.div_sel = SSPI_CLK_DIVIDER0;
            task_conf.seq_ptr = 0;

            // max transfer size is set to 4K bytes in non_continuous mode transfer
            if((devCfg->xferSize >> 1) > 0x400) {
                task_conf.loop_cnt = 0;
                task_conf.continuous = 1;
            }
            else {
                task_conf.loop_cnt = (devCfg->xferSize >> 1) - 1;
                task_conf.continuous = 0;
            }
            task_conf.init_cond_mask = (configTx->enable) ?
                (SSPI_TASK_INIT_COND_THRESHOLD_TX0 | SSPI_TASK_INIT_COND_THRESHOLD_TX1) : 0;
            task_conf.wait_before_start = 1;
            if(chal_sspi_set_task(pDevice, 0, protocol, &task_conf))
                return(CSL_PCM_ERR_TASK);

            if(devCfg->protocol == CSL_PCM_PROTOCOL_STEREO) {
                seq_conf.tx_enable = (configTx->enable || configRx->loopback_enable)  ? TRUE : FALSE;
                seq_conf.rx_enable = (configRx->enable || configTx->loopback_enable)  ? TRUE : FALSE;
                seq_conf.cs_activate = 1;
                seq_conf.cs_deactivate = 1;
                seq_conf.pattern_mode = 0;
                seq_conf.rep_cnt = 0;
                seq_conf.opcode = SSPI_SEQ_OPCODE_NEXT_PC;
                seq_conf.rx_fifo_sel = 0;
                seq_conf.tx_fifo_sel = 0;
                seq_conf.frm_sel = 0;
                seq_conf.rx_sidetone_on = 0;
                seq_conf.tx_sidetone_on = 0;
                seq_conf.next_pc = 0;
                if(chal_sspi_set_sequence(pDevice, 0, protocol, &seq_conf))
                    return(CSL_PCM_ERR_SEQUENCE);

                if(!devCfg->ext_bits) {
                    seq_conf.tx_enable = (configTx->enable || configRx->loopback_enable)  ? TRUE : FALSE;
                    seq_conf.rx_enable = (configRx->enable || configTx->loopback_enable)  ? TRUE : FALSE;
                    seq_conf.cs_activate = 0;
                    seq_conf.cs_deactivate = 0;
                    seq_conf.pattern_mode = 0;
                    seq_conf.rep_cnt = 0;
                    seq_conf.opcode = SSPI_SEQ_OPCODE_COND_JUMP;
                    seq_conf.rx_fifo_sel = 1;
                    seq_conf.tx_fifo_sel = 1;
                    seq_conf.frm_sel = 0;
                    seq_conf.rx_sidetone_on = 0;
                    seq_conf.tx_sidetone_on = 0;
                    seq_conf.next_pc = 0;
                    if(chal_sspi_set_sequence(pDevice, 1, protocol, &seq_conf))
                        return(CSL_PCM_ERR_SEQUENCE);

                    seq_conf.tx_enable = FALSE;
                    seq_conf.rx_enable = FALSE;
                    seq_conf.cs_activate = 0;
                    seq_conf.cs_deactivate = 0;
                    seq_conf.pattern_mode = 0;
                    seq_conf.rep_cnt = 0;
                    seq_conf.opcode = SSPI_SEQ_OPCODE_STOP;
                    seq_conf.rx_fifo_sel = 0;
                    seq_conf.tx_fifo_sel = 0;
                    seq_conf.frm_sel = 0;
                    seq_conf.rx_sidetone_on = 0;
                    seq_conf.tx_sidetone_on = 0;
                    seq_conf.next_pc = 0;
                    if(chal_sspi_set_sequence(pDevice, 2, protocol, &seq_conf))
                        return(CSL_PCM_ERR_SEQUENCE);
                }
                else {
                    seq_conf.tx_enable = (configTx->enable || configRx->loopback_enable)  ? TRUE : FALSE;
                    seq_conf.rx_enable = (configRx->enable || configTx->loopback_enable)  ? TRUE : FALSE;
                    seq_conf.cs_activate = 0;
                    seq_conf.cs_deactivate = 0;
                    seq_conf.pattern_mode = 0;
                    seq_conf.rep_cnt = 0;
                    seq_conf.opcode = SSPI_SEQ_OPCODE_NEXT_PC;
                    seq_conf.rx_fifo_sel = 1;
                    seq_conf.tx_fifo_sel = 1;
                    seq_conf.frm_sel = 0;
                    seq_conf.rx_sidetone_on = 0;
                    seq_conf.tx_sidetone_on = 0;
                    seq_conf.next_pc = 0;
                    if(chal_sspi_set_sequence(pDevice, 1, protocol, &seq_conf))
                        return(CSL_PCM_ERR_SEQUENCE);

                    seq_conf.tx_enable = FALSE;
                    seq_conf.rx_enable = FALSE;
                    seq_conf.cs_activate = 0;
                    seq_conf.cs_deactivate = 0;
                    seq_conf.pattern_mode = 0;
                    seq_conf.rep_cnt = 0;
                    seq_conf.opcode = SSPI_SEQ_OPCODE_COND_JUMP;
                    seq_conf.rx_fifo_sel = 0;
                    seq_conf.tx_fifo_sel = 0;
                    seq_conf.frm_sel = 1;
                    seq_conf.rx_sidetone_on = 0;
                    seq_conf.tx_sidetone_on = 0;
                    seq_conf.next_pc = 0;
                    if(chal_sspi_set_sequence(pDevice, 2, protocol, &seq_conf))
                        return(CSL_PCM_ERR_SEQUENCE);

                    seq_conf.tx_enable = FALSE;
                    seq_conf.rx_enable = FALSE;
                    seq_conf.cs_activate = 0;
                    seq_conf.cs_deactivate = 0;
                    seq_conf.pattern_mode = 0;
                    seq_conf.rep_cnt = 0;
                    seq_conf.opcode = SSPI_SEQ_OPCODE_STOP;
                    seq_conf.rx_fifo_sel = 0;
                    seq_conf.tx_fifo_sel = 0;
                    seq_conf.frm_sel = 0;
                    seq_conf.rx_sidetone_on = 0;
                    seq_conf.tx_sidetone_on = 0;
                    seq_conf.next_pc = 0;
                    if(chal_sspi_set_sequence(pDevice, 3, protocol, &seq_conf))
                        return(CSL_PCM_ERR_SEQUENCE);
                }
            }
        
            if(chal_sspi_set_frame(pDevice, &frmMask, protocol, 16, devCfg->ext_bits))
                return(CSL_PCM_ERR_FRAME);
            break;
            
        case SSPI_PROT_STEREO_25B_PCM:
            if(configTx->enable) {
                chal_sspi_set_fifo_threshold(pDevice, SSPI_FIFO_ID_TX0, 0x10);
                chal_sspi_set_fifo_threshold(pDevice, SSPI_FIFO_ID_TX1, 0x10);
            }

            task_conf.chan_sel = SSPI_CHAN_SEL_CHAN0;
            task_conf.cs_sel = SSPI_CS_SEL_CS0;
            task_conf.rx_sel = SSPI_RX_SEL_RX0;
            task_conf.tx_sel = SSPI_TX_SEL_TX0;
            task_conf.div_sel = SSPI_CLK_DIVIDER0;
            task_conf.seq_ptr = 0;
            if((devCfg->xferSize >> 2) > 0x400) {
                task_conf.loop_cnt = 0;
                task_conf.continuous = 1;
            }
            else {
                task_conf.loop_cnt = (devCfg->xferSize >> 2) - 1;
                task_conf.continuous = 0;
            }
            task_conf.init_cond_mask = (configTx->enable) ?
                (SSPI_TASK_INIT_COND_THRESHOLD_TX0 | SSPI_TASK_INIT_COND_THRESHOLD_TX1) : 0;

            task_conf.wait_before_start = 1;
            if(chal_sspi_set_task(pDevice, 0, protocol, &task_conf))
                return(CSL_PCM_ERR_TASK);

            seq_conf.tx_enable = (configTx->enable)  ? FALSE : TRUE;
            seq_conf.rx_enable = (configRx->enable)  ? TRUE : FALSE;
            seq_conf.cs_activate = 1;
            seq_conf.cs_deactivate = 1;
            seq_conf.pattern_mode = 0;
            seq_conf.rep_cnt = 0;
            seq_conf.opcode = SSPI_SEQ_OPCODE_NEXT_PC;
            seq_conf.rx_fifo_sel = 0;
            seq_conf.tx_fifo_sel = 0;
            seq_conf.frm_sel = 0;
            seq_conf.rx_sidetone_on = 0;
            seq_conf.tx_sidetone_on = 0;
            seq_conf.next_pc = 0;
            if(chal_sspi_set_sequence(pDevice, 0, protocol, &seq_conf))
                return(CSL_PCM_ERR_SEQUENCE);

            if(!devCfg->ext_bits) {
                seq_conf.tx_enable = (configTx->enable)  ? FALSE : TRUE;
                seq_conf.rx_enable = (configRx->enable)  ? TRUE : FALSE;
                seq_conf.cs_activate = 0;
                seq_conf.cs_deactivate = 0;
                seq_conf.pattern_mode = 0;
                seq_conf.rep_cnt = 0;
                seq_conf.opcode = SSPI_SEQ_OPCODE_COND_JUMP;
                seq_conf.rx_fifo_sel = 1;
                seq_conf.tx_fifo_sel = 1;
                seq_conf.frm_sel = 0;
                seq_conf.rx_sidetone_on = 0;
                seq_conf.tx_sidetone_on = 0;
                seq_conf.next_pc = 0;
                if(chal_sspi_set_sequence(pDevice, 1, protocol, &seq_conf))
                    return(CSL_PCM_ERR_SEQUENCE);

                seq_conf.tx_enable = FALSE;
                seq_conf.rx_enable = FALSE;
                seq_conf.cs_activate = 0;
                seq_conf.cs_deactivate = 0;
                seq_conf.pattern_mode = 0;
                seq_conf.rep_cnt = 0;
                seq_conf.opcode = SSPI_SEQ_OPCODE_STOP;
                seq_conf.rx_fifo_sel = 0;
                seq_conf.tx_fifo_sel = 0;
                seq_conf.frm_sel = 0;
                seq_conf.rx_sidetone_on = 0;
                seq_conf.tx_sidetone_on = 0;
                seq_conf.next_pc = 0;
                if(chal_sspi_set_sequence(pDevice, 2, protocol, &seq_conf))
                    return(CSL_PCM_ERR_SEQUENCE);
            }
            else {
                seq_conf.tx_enable = (configTx->enable)  ? FALSE : TRUE;
                seq_conf.rx_enable = (configRx->enable)  ? TRUE : FALSE;
                seq_conf.cs_activate = 0;
                seq_conf.cs_deactivate = 0;
                seq_conf.pattern_mode = 0;
                seq_conf.rep_cnt = 0;
                seq_conf.opcode = SSPI_SEQ_OPCODE_NEXT_PC;
                seq_conf.rx_fifo_sel = 1;
                seq_conf.tx_fifo_sel = 1;
                seq_conf.frm_sel = 0;
                seq_conf.rx_sidetone_on = 0;
                seq_conf.tx_sidetone_on = 0;
                seq_conf.next_pc = 0;
                if(chal_sspi_set_sequence(pDevice, 1, protocol, &seq_conf))
                    return(CSL_PCM_ERR_SEQUENCE);

                seq_conf.tx_enable = FALSE;
                seq_conf.rx_enable = FALSE;
                seq_conf.cs_activate = 0;
                seq_conf.cs_deactivate = 0;
                seq_conf.pattern_mode = 0;
                seq_conf.rep_cnt = 0;
                seq_conf.opcode = SSPI_SEQ_OPCODE_COND_JUMP;
                seq_conf.rx_fifo_sel = 0;
                seq_conf.tx_fifo_sel = 0;
                seq_conf.frm_sel = 1;
                seq_conf.rx_sidetone_on = 0;
                seq_conf.tx_sidetone_on = 0;
                seq_conf.next_pc = 0;
                if(chal_sspi_set_sequence(pDevice, 2, protocol, &seq_conf))
                    return(CSL_PCM_ERR_SEQUENCE);

                seq_conf.tx_enable = FALSE;
                seq_conf.rx_enable = FALSE;
                seq_conf.cs_activate = 0;
                seq_conf.cs_deactivate = 0;
                seq_conf.pattern_mode = 0;
                seq_conf.rep_cnt = 0;
                seq_conf.opcode = SSPI_SEQ_OPCODE_STOP;
                seq_conf.rx_fifo_sel = 0;
                seq_conf.tx_fifo_sel = 0;
                seq_conf.frm_sel = 0;
                seq_conf.rx_sidetone_on = 0;
                seq_conf.tx_sidetone_on = 0;
                seq_conf.next_pc = 0;
                if(chal_sspi_set_sequence(pDevice, 3, protocol, &seq_conf))
                    return(CSL_PCM_ERR_SEQUENCE);
            }
            
            if(chal_sspi_set_frame(pDevice, &frmMask, protocol, 24, devCfg->ext_bits))
                return(CSL_PCM_ERR_FRAME);
            break;    	
    }
#endif    
    return CSL_PCM_SUCCESS;
}

//******************************************************************************
//
//  Function Name:	csl_pcm_get_tx0_fifo_data_port
//  
//  Description:	This function get transmit channel fifo port address
//
//******************************************************************************
UInt32 csl_pcm_get_tx0_fifo_data_port(CSL_PCM_HANDLE handle)
{
#if 0
    CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *)handle;
	return (UInt32)(pDevice->base+SSPIL_FIFO_ENTRY0TX_OFFSET);
#endif
	return 0;	
}
//******************************************************************************
//
//  Function Name:	csl_pcm_get_tx1_fifo_data_port
//  
//  Description:	This function get receive channel fifo port address
//
//******************************************************************************

UInt32 csl_pcm_get_tx1_fifo_data_port(CSL_PCM_HANDLE handle)
{
#if 0
    CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *)handle;
	return (UInt32)(pDevice->base+SSPIL_FIFO_ENTRY1TX_OFFSET);
#endif
	return 0;	
}

//******************************************************************************
//
//  Function Name:	csl_pcm_get_rx0_fifo_data_port
//  
//  Description:	This function get receive channel fifo port address
//
//******************************************************************************
UInt32 csl_pcm_get_rx0_fifo_data_port(CSL_PCM_HANDLE handle)
{
#if 0
	CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *)handle;
	return (UInt32)(pDevice->base+SSPIL_FIFO_ENTRY0RX_OFFSET);
#endif
	return 0;	
}
//******************************************************************************
//
//  Function Name:	csl_pcm_get_rx1_fifo_data_port
//  
//  Description:	This function get receive channel fifo port address
//
//******************************************************************************

UInt32 csl_pcm_get_rx1_fifo_data_port(CSL_PCM_HANDLE handle)
{
#if 0
	CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *)handle;
	return (UInt32)(pDevice->base+SSPIL_FIFO_ENTRY1RX_OFFSET);
#endif	
	return 0;
}
