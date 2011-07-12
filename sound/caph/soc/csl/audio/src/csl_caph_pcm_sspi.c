/******************************************************************************/
/*																			  */
/*	Copyright 2011  Broadcom Corporation									  */
/*																			  */
/*	Unless you and Broadcom execute a separate written software license		  */
/*	agreement governing use of this software, this software is licensed to	  */
/*	you under the terms of the GNU General Public License version 2 (the GPL),*/
/*	available at															  */
/*																			  */
/*		http://www.broadcom.com/licenses/GPLv2.php							  */
/*																			  */
/*	with the following added to such license:								  */
/*																			  */
/*		As a special exception, the copyright holders of this software give	  */
/*		you permission to link this software with independent modules, and to */
/*		copy and distribute the resulting executable under terms of your	  */
/*		choice, provided that you also meet, for each linked independent	  */
/*		module, the terms and conditions of the license of that module.		  */
/*		An independent module is a module which is not derived from this	  */
/*		software. The special exception does not apply to any modifications	  */
/*		of the software.													  */
/*																			  */
/*		Notwithstanding the above, under no circumstances may you combine	  */
/*		this software in any way with any other Broadcom software provided	  */
/*		under a license other than the GPL, without Broadcom's express prior  */
/*		written	consent.													  */
/*																			  */
/******************************************************************************/
/**
*
*  @file   csl_caph_pcm_sspi.c
*
*  @brief  PCM device CSL layer implementation
*
****************************************************************************/
#include <string.h>
#include "mobcom_types.h"
#include "auddrv_def.h"
#include "chal_types.h"
#include "chal_sspi.h"
#include "csl_caph_pcm_sspi.h"
#include "brcm_rdb_padctrlreg.h"
#include "brcm_rdb_sysmap.h"
#include "brcm_rdb_sspil.h"
#include "log.h"

#define SSPI_HW_WORD_LEN_32Bit					32
#define SSPI_HW_WORD_LEN_25Bit					25
#define SSPI_HW_WORD_LEN_24Bit					24
#define SSPI_HW_WORD_LEN_16Bit					16
#define SSPI_HW_WORD_LEN_8Bit					8
#define SSPI_HW_DUMMY_BITS_9					9
#define SSPI_HW_DUMMY_BITS_2					2
#define SSPI_HW_DUMMY_BITS_1					1
#define SSPI_HW_FRAME0_MASK						(1 << 0)
#define SSPI_HW_FRAME1_MASK						(1 << 1)
#define SSPI_HW_FRAME2_MASK						(1 << 2)
#define SSPI_HW_FRAME3_MASK						(1 << 3)


//******************************************************************************
// Local Definitions
//******************************************************************************
static chal_sspi_task_conf_t task_conf;
static chal_sspi_seq_conf_t seq_conf;

//******************************************************************************
//
//  Function Name:	csl_pcm_init
//  
//  Description:	This function initializes the CSL layer
//
//******************************************************************************
CSL_PCM_HANDLE csl_pcm_init(cUInt32 baseAddr)
{
	CSL_PCM_HANDLE handle = 0;
	CSL_PCM_HANDLE_t *pDevice;

	Log_DebugPrintf(LOGID_SOC_AUDIO, "+csl_pcm_init\r\n");
	
    handle = chal_sspi_init(baseAddr);    
    pDevice = (CSL_PCM_HANDLE_t *)handle;
	if(handle == NULL) {
    	Log_DebugPrintf(LOGID_SOC_AUDIO,"csl_pcm_init failed\r\n");
    	return NULL;
    }
    
    chal_sspi_set_type(handle, SSPI_TYPE_LITE);

	Log_DebugPrintf(LOGID_SOC_AUDIO, "-csl_pcm_init base address 0x%x\r\n",
					(unsigned int)pDevice->base);
	
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
	CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *)handle;
	CHAL_SSPI_STATUS_t status;

	Log_DebugPrintf(LOGID_SOC_AUDIO, "+csl_pcm_deinit\r\n");

	if(handle == NULL) {
    	Log_DebugPrintf(LOGID_SOC_AUDIO,"csl_pcm_deinit failed\r\n");
    	return CSL_PCM_ERR_HANDLE;
    }
    
    status = chal_sspi_deinit(pDevice);

	Log_DebugPrintf(LOGID_SOC_AUDIO, "-csl_pcm_deinit\r\n");

	if(status == CHAL_SSPI_STATUS_SUCCESS)
		return CSL_PCM_SUCCESS;
	else if(status == CHAL_SSPI_STATUS_ILLEGAL_HANDLE)
		return CSL_PCM_ERR_HANDLE;
	return CSL_PCM_SUCCESS;
}

//******************************************************************************
//
//  Function Name:	csl_pcm_enable_scheduler
//  
//  Description:	This function starts scheduler operation
//
//******************************************************************************
CSL_PCM_OPSTATUS_t csl_pcm_enable_scheduler(CSL_PCM_HANDLE handle,
											Boolean enable)
{
    CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *)handle;

    if (!pDevice)
		return CSL_PCM_ERR_HANDLE;
	
    chal_sspi_enable_scheduler(pDevice, enable);

    return CSL_PCM_SUCCESS;
}	
	
//******************************************************************************
//
//  Function Name:	csl_pcm_start
//  
//  Description:	This function starts scheduler operation
//
//******************************************************************************
CSL_PCM_OPSTATUS_t csl_pcm_start(CSL_PCM_HANDLE handle,
								 csl_pcm_config_device_t *config)
{
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
    return CSL_PCM_SUCCESS;
}

//******************************************************************************
//
//  Function Name:	csl_pcm_start_tx
//  
//  Description:	This function starts transmit operation
//
//******************************************************************************
CSL_PCM_OPSTATUS_t csl_pcm_start_tx(CSL_PCM_HANDLE handle, UInt8 channel)
{
    CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *)handle;

    if (!pDevice)
		return CSL_PCM_ERR_HANDLE;

    if(channel == CSL_PCM_CHAN_TX0) {
        chal_sspi_fifo_reset(pDevice, SSPI_FIFO_ID_TX0);	
        chal_sspi_enable_fifo_pio_start_stop_intr(pDevice, 
                                              SSPI_FIFO_ID_TX0, 
                                              TRUE, 
                                              TRUE); 		
    }
    else if(channel == CSL_PCM_CHAN_TX1) {
        chal_sspi_fifo_reset(pDevice, SSPI_FIFO_ID_TX1);	
        chal_sspi_enable_fifo_pio_start_stop_intr(pDevice, 
                                              SSPI_FIFO_ID_TX1, 
                                              TRUE, 
                                              TRUE); 		
    }
	
    return CSL_PCM_SUCCESS;
}

//******************************************************************************
//
//  Function Name:	csl_pcm_start_rx
//  
//  Description:	This function starts receive operation
//
//******************************************************************************
CSL_PCM_OPSTATUS_t csl_pcm_start_rx(CSL_PCM_HANDLE handle, UInt8 channel)
{
    CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *)handle;
	
    if (!pDevice)
		return CSL_PCM_ERR_HANDLE;
	
    if(channel == CSL_PCM_CHAN_RX0) {
        chal_sspi_fifo_reset(pDevice, SSPI_FIFO_ID_RX0);
        chal_sspi_enable_fifo_pio_start_stop_intr(pDevice, 
                                              SSPI_FIFO_ID_RX0, 
                                              TRUE, 
                                              TRUE); 		
    }
    else if (channel == CSL_PCM_CHAN_RX1) {    
        chal_sspi_fifo_reset(pDevice, SSPI_FIFO_ID_RX1);
        chal_sspi_enable_fifo_pio_start_stop_intr(pDevice, 
                                              SSPI_FIFO_ID_RX1, 
                                              TRUE, 
                                              TRUE); 		
    }
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
    CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *)handle;

    if (!pDevice)
		return CSL_PCM_ERR_HANDLE;

    if(channel == CSL_PCM_CHAN_TX0) {
        chal_sspi_fifo_reset(pDevice, SSPI_FIFO_ID_TX0);	
        chal_sspi_enable_fifo_pio_start_stop_intr(pDevice, 
                                              SSPI_FIFO_ID_TX0, 
                                              FALSE, 
                                              FALSE); 		
    }
    else if(channel == CSL_PCM_CHAN_TX1) {
        chal_sspi_fifo_reset(pDevice, SSPI_FIFO_ID_TX1);	
        chal_sspi_enable_fifo_pio_start_stop_intr(pDevice, 
                                              SSPI_FIFO_ID_TX1, 
                                              FALSE, 
                                              FALSE); 		
    }
	
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
    CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *)handle;
	
    if (!pDevice)
		return CSL_PCM_ERR_HANDLE;
	
    if(channel == CSL_PCM_CHAN_RX0) {
        chal_sspi_fifo_reset(pDevice, SSPI_FIFO_ID_RX0);
        chal_sspi_enable_fifo_pio_start_stop_intr(pDevice, 
                                              SSPI_FIFO_ID_RX0, 
                                              FALSE, 
                                              FALSE); 		
    }
    else if (channel == CSL_PCM_CHAN_RX1) {    
        chal_sspi_fifo_reset(pDevice, SSPI_FIFO_ID_RX1);
        chal_sspi_enable_fifo_pio_start_stop_intr(pDevice, 
                                              SSPI_FIFO_ID_RX1, 
                                              FALSE, 
                                              FALSE); 		
    }
    return CSL_PCM_SUCCESS;
}


CSL_PCM_OPSTATUS_t csl_pcm_pause(CSL_PCM_HANDLE handle)
{
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
    return CSL_PCM_SUCCESS;
}


CSL_PCM_OPSTATUS_t csl_pcm_resume(CSL_PCM_HANDLE handle)
{
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
    return CSL_PCM_SUCCESS;
}

//******************************************************************************
//
//  Function Name:	csl_pcm_config
//  
//  Description:	This function configures SSPI as PCM operation
//
//******************************************************************************
CSL_PCM_OPSTATUS_t csl_pcm_config(CSL_PCM_HANDLE handle,
								  csl_pcm_config_device_t *configDev,
								  csl_pcm_config_tx_t *configTx,
								  csl_pcm_config_rx_t *configRx)
{
    CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *)handle;
	uint32_t frmMask = SSPI_HW_FRAME0_MASK;
	CHAL_SSPI_PROT_t protocol;
    csl_pcm_config_device_t *devCfg = configDev;
	uint32_t intrMask;

	if(handle == NULL) {
    	Log_DebugPrintf(LOGID_SOC_AUDIO,"csl_pcm_config failed\r\n");
        return CSL_PCM_ERR_HANDLE;
    }

	if((devCfg->protocol == CSL_PCM_PROTOCOL_MONO) &&
		(devCfg->format == CSL_PCM_WORD_LENGTH_16_BIT))
        protocol = SSPI_PROT_MONO_16B_PCM;
	else if((devCfg->protocol == CSL_PCM_PROTOCOL_MONO) &&
			(devCfg->format == CSL_PCM_WORD_LENGTH_24_BIT))
        protocol = SSPI_PROT_MONO_25B_PCM;
	else if((devCfg->protocol == CSL_PCM_PROTOCOL_STEREO) &&
			(devCfg->format == CSL_PCM_WORD_LENGTH_16_BIT))
        protocol = SSPI_PROT_STEREO_16B_PCM;
	else if((devCfg->protocol == CSL_PCM_PROTOCOL_STEREO) &&
			(devCfg->format == CSL_PCM_WORD_LENGTH_24_BIT))
        protocol = SSPI_PROT_STEREO_25B_PCM; 
	else if((devCfg->protocol == CSL_PCM_PROTOCOL_3CHANNEL) &&
			(devCfg->format == CSL_PCM_WORD_LENGTH_16_BIT)) {
		protocol = SSPI_PROT_3CHAN_16B_TDM_PCM;
		frmMask = SSPI_HW_FRAME0_MASK | SSPI_HW_FRAME1_MASK;
	}
	else if((devCfg->protocol == CSL_PCM_PROTOCOL_4CHANNEL) &&
			(devCfg->format == CSL_PCM_WORD_LENGTH_16_BIT)) {
		protocol = SSPI_PROT_4CHAN_16B_TDM_PCM;
		frmMask = SSPI_HW_FRAME0_MASK | SSPI_HW_FRAME1_MASK;
	}
    // only 16B case was tested for the following 2 protocols	
	else if((devCfg->protocol == CSL_PCM_PROTOCOL_INTERLEAVE_3CHANNEL) &&
			(devCfg->format == CSL_PCM_WORD_LENGTH_16_BIT)) {
		protocol = SSPI_PROT_STEREO_16B_PCM;
		frmMask = SSPI_HW_FRAME0_MASK | SSPI_HW_FRAME1_MASK;
	}
	else if((devCfg->protocol == CSL_PCM_PROTOCOL_INTERLEAVE_3CHANNEL) &&
			(devCfg->format == CSL_PCM_WORD_LENGTH_24_BIT)) {
		protocol = SSPI_PROT_STEREO_25B_PCM;
		frmMask = SSPI_HW_FRAME0_MASK | SSPI_HW_FRAME1_MASK;
	}
	else if((devCfg->protocol == CSL_PCM_PROTOCOL_INTERLEAVE_4CHANNEL) &&
			(devCfg->format == CSL_PCM_WORD_LENGTH_16_BIT)) {
		protocol = SSPI_PROT_STEREO_16B_PCM;
		frmMask = SSPI_HW_FRAME0_MASK | SSPI_HW_FRAME1_MASK;
	}
	else if((devCfg->protocol == CSL_PCM_PROTOCOL_INTERLEAVE_4CHANNEL) &&
			(devCfg->format == CSL_PCM_WORD_LENGTH_24_BIT)) {
		protocol = SSPI_PROT_STEREO_25B_PCM;
		frmMask = SSPI_HW_FRAME0_MASK | SSPI_HW_FRAME1_MASK;
	}
    else
        return CSL_PCM_ERR_PROT;	
    //
    // task_conf struct initialization
    //
    memset(&task_conf, 0, sizeof(task_conf));
    
    //
    // soft reset sspi instance
    //
    chal_sspi_soft_reset(pDevice);

	chal_sspi_get_intr_mask(pDevice, &intrMask);
	intrMask |= (SSPIL_INTR_ENABLE_PIO_TX_START |
				 SSPIL_INTR_ENABLE_PIO_TX_STOP |
				 SSPIL_INTR_ENABLE_PIO_RX_START |
				 SSPIL_INTR_ENABLE_PIO_RX_STOP);
	// need to disable all other interrupts to avoid confusing dsp 03-02-11
	chal_sspi_enable_intr(pDevice, intrMask & 0x000000F0);

    //
    // set sspi at idle state
    // 
	if(chal_sspi_set_idle_state(pDevice, protocol)) {
		Log_DebugPrintf(LOGID_SOC_AUDIO,"csl_pcm_config failed \r\n");
		return CSL_PCM_ERROR;
	}
	
    chal_sspi_set_clk_divider(handle, SSPI_CLK_DIVIDER0, 0);
	chal_sspi_set_clk_divider(handle, SSPI_CLK_REF_DIVIDER, 0);
	chal_sspi_set_clk_src_select(handle, SSPI_CLK_SRC_CAPHCLK);
#ifdef SSPI_TDM_MODE //SSPI_TDM_TEST	
	// to be used with new chal code
	chal_sspi_set_caph_clk(handle, SSPI_CAPH_CLK_TRIG_48KHZ_3CH_16B_24B,
	SSPI_HW_WORD_LEN_16Bit, 3);
	if (configDev->sample_rate == 8000) {
		chal_sspi_set_fifo_repeat_count(handle, SSPI_FIFO_ID_RX0, 5);
		chal_sspi_set_fifo_repeat_count(handle, SSPI_FIFO_ID_TX0, 5);
	}
	else if (configDev->sample_rate == 16000) {
		chal_sspi_set_fifo_repeat_count(handle, SSPI_FIFO_ID_RX0, 2);
		chal_sspi_set_fifo_repeat_count(handle, SSPI_FIFO_ID_TX0, 2);
	}		
#else
	//standard PCM 
	if (configDev->sample_rate == 8000) {
		chal_sspi_set_caph_clk(handle, SSPI_CAPH_CLK_TRIG_8kHz,
		SSPI_HW_WORD_LEN_16Bit, 1);
	}
	else if (configDev->sample_rate == 16000) {
		chal_sspi_set_caph_clk(handle, SSPI_CAPH_CLK_TRIG_16kHz,
		SSPI_HW_WORD_LEN_16Bit, 1);
	}
	chal_sspi_set_fifo_repeat_count(handle, SSPI_FIFO_ID_RX0, 0);
	chal_sspi_set_fifo_repeat_count(handle, SSPI_FIFO_ID_TX0, 0);
#endif

	switch(protocol) {
	case SSPI_PROT_MONO_16B_PCM:
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_RX0, SSPI_FIFO_SIZE_FULL);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_RX1, SSPI_FIFO_SIZE_NONE);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_RX2, SSPI_FIFO_SIZE_NONE);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_RX3, SSPI_FIFO_SIZE_NONE);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_TX0, SSPI_FIFO_SIZE_FULL);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_TX1, SSPI_FIFO_SIZE_NONE);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_TX2, SSPI_FIFO_SIZE_NONE);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_TX3, SSPI_FIFO_SIZE_NONE);

		chal_sspi_set_fifo_pack(pDevice,
								SSPI_FIFO_ID_RX0, SSPI_FIFO_DATA_PACK_NONE);
		chal_sspi_set_fifo_pack(pDevice,
								SSPI_FIFO_ID_RX1, SSPI_FIFO_DATA_PACK_NONE);
		chal_sspi_set_fifo_pack(pDevice,
								SSPI_FIFO_ID_TX0, SSPI_FIFO_DATA_PACK_NONE);
		chal_sspi_set_fifo_pack(pDevice,
								SSPI_FIFO_ID_TX1, SSPI_FIFO_DATA_PACK_NONE);
		break;

	case SSPI_PROT_MONO_25B_PCM:
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_RX0, SSPI_FIFO_SIZE_FULL);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_RX1, SSPI_FIFO_SIZE_NONE);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_RX2, SSPI_FIFO_SIZE_NONE);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_RX3, SSPI_FIFO_SIZE_NONE);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_TX0, SSPI_FIFO_SIZE_FULL);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_TX1, SSPI_FIFO_SIZE_NONE);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_TX2, SSPI_FIFO_SIZE_NONE);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_TX3, SSPI_FIFO_SIZE_NONE);

		chal_sspi_set_fifo_pack(pDevice,
								SSPI_FIFO_ID_RX0, SSPI_FIFO_DATA_PACK_NONE);
		chal_sspi_set_fifo_pack(pDevice,
								SSPI_FIFO_ID_RX1, SSPI_FIFO_DATA_PACK_NONE);
		chal_sspi_set_fifo_pack(pDevice,
								SSPI_FIFO_ID_TX0, SSPI_FIFO_DATA_PACK_NONE);
		chal_sspi_set_fifo_pack(pDevice,
								SSPI_FIFO_ID_TX1, SSPI_FIFO_DATA_PACK_NONE);

		break;

	case SSPI_PROT_STEREO_16B_PCM:
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_RX0, SSPI_FIFO_SIZE_HALF);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_RX1, SSPI_FIFO_SIZE_HALF);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_RX2, SSPI_FIFO_SIZE_NONE);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_RX3, SSPI_FIFO_SIZE_NONE);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_TX0, SSPI_FIFO_SIZE_HALF);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_TX1, SSPI_FIFO_SIZE_HALF);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_TX2, SSPI_FIFO_SIZE_NONE);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_TX3, SSPI_FIFO_SIZE_NONE);

		chal_sspi_set_fifo_pack(pDevice,
								SSPI_FIFO_ID_RX0, SSPI_FIFO_DATA_PACK_16BIT);
		chal_sspi_set_fifo_pack(pDevice,
								SSPI_FIFO_ID_RX1, SSPI_FIFO_DATA_PACK_16BIT);
		chal_sspi_set_fifo_pack(pDevice,
								SSPI_FIFO_ID_TX0, SSPI_FIFO_DATA_PACK_16BIT);
		chal_sspi_set_fifo_pack(pDevice,
								SSPI_FIFO_ID_TX1, SSPI_FIFO_DATA_PACK_16BIT);

		break;

	case SSPI_PROT_STEREO_25B_PCM:
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_RX0, SSPI_FIFO_SIZE_HALF);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_RX1, SSPI_FIFO_SIZE_HALF);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_RX2, SSPI_FIFO_SIZE_NONE);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_RX3, SSPI_FIFO_SIZE_NONE);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_TX0, SSPI_FIFO_SIZE_HALF);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_TX1, SSPI_FIFO_SIZE_HALF);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_TX2, SSPI_FIFO_SIZE_NONE);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_TX3, SSPI_FIFO_SIZE_NONE);

		chal_sspi_set_fifo_pack(pDevice,
								SSPI_FIFO_ID_RX0, SSPI_FIFO_DATA_PACK_NONE);
		chal_sspi_set_fifo_pack(pDevice,
								SSPI_FIFO_ID_RX1, SSPI_FIFO_DATA_PACK_16BIT);
		chal_sspi_set_fifo_pack(pDevice,
								SSPI_FIFO_ID_TX0, SSPI_FIFO_DATA_PACK_NONE);
		chal_sspi_set_fifo_pack(pDevice,
								SSPI_FIFO_ID_TX1, SSPI_FIFO_DATA_PACK_16BIT);

		break;

	case SSPI_PROT_3CHAN_16B_TDM_PCM:
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_RX0, SSPI_FIFO_SIZE_HALF);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_RX1, SSPI_FIFO_SIZE_QUARTER);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_RX2, SSPI_FIFO_SIZE_QUARTER);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_RX3, SSPI_FIFO_SIZE_NONE);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_TX0, SSPI_FIFO_SIZE_HALF);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_TX1, SSPI_FIFO_SIZE_QUARTER);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_TX2, SSPI_FIFO_SIZE_QUARTER);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_TX3, SSPI_FIFO_SIZE_NONE);

		chal_sspi_set_fifo_pack(pDevice,
								SSPI_FIFO_ID_RX0, SSPI_FIFO_DATA_PACK_16BIT);
		chal_sspi_set_fifo_pack(pDevice,
								SSPI_FIFO_ID_RX1, SSPI_FIFO_DATA_PACK_16BIT);
		chal_sspi_set_fifo_pack(pDevice,
								SSPI_FIFO_ID_RX2, SSPI_FIFO_DATA_PACK_16BIT);
		chal_sspi_set_fifo_pack(pDevice,
								SSPI_FIFO_ID_TX0, SSPI_FIFO_DATA_PACK_16BIT);
		chal_sspi_set_fifo_pack(pDevice,
								SSPI_FIFO_ID_TX1, SSPI_FIFO_DATA_PACK_16BIT);
		chal_sspi_set_fifo_pack(pDevice,
								SSPI_FIFO_ID_TX2, SSPI_FIFO_DATA_PACK_16BIT);

		break;

	case SSPI_PROT_4CHAN_16B_TDM_PCM:
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_RX0, SSPI_FIFO_SIZE_QUARTER);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_RX1, SSPI_FIFO_SIZE_QUARTER);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_RX2, SSPI_FIFO_SIZE_QUARTER);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_RX3, SSPI_FIFO_SIZE_QUARTER);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_TX0, SSPI_FIFO_SIZE_QUARTER);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_TX1, SSPI_FIFO_SIZE_QUARTER);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_TX2, SSPI_FIFO_SIZE_QUARTER);
		chal_sspi_set_fifo_size(pDevice,
								SSPI_FIFO_ID_TX3, SSPI_FIFO_SIZE_QUARTER);

		chal_sspi_set_fifo_pack(pDevice,
								SSPI_FIFO_ID_RX0, SSPI_FIFO_DATA_PACK_16BIT);
		chal_sspi_set_fifo_pack(pDevice,
								SSPI_FIFO_ID_RX1, SSPI_FIFO_DATA_PACK_16BIT);
		chal_sspi_set_fifo_pack(pDevice,
								SSPI_FIFO_ID_RX2, SSPI_FIFO_DATA_PACK_16BIT);
		chal_sspi_set_fifo_pack(pDevice,
								SSPI_FIFO_ID_RX3, SSPI_FIFO_DATA_PACK_16BIT);
		chal_sspi_set_fifo_pack(pDevice,
								SSPI_FIFO_ID_TX0, SSPI_FIFO_DATA_PACK_16BIT);
		chal_sspi_set_fifo_pack(pDevice,
								SSPI_FIFO_ID_TX1, SSPI_FIFO_DATA_PACK_16BIT);
		chal_sspi_set_fifo_pack(pDevice,
								SSPI_FIFO_ID_TX2, SSPI_FIFO_DATA_PACK_16BIT);
		chal_sspi_set_fifo_pack(pDevice,
								SSPI_FIFO_ID_TX3, SSPI_FIFO_DATA_PACK_16BIT);

		break;

	default:
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

    // setting from asic team
    chal_sspi_set_fifo_pio_threshhold(pDevice, SSPI_FIFO_ID_RX0, 0xf, 0x1);
    chal_sspi_set_fifo_pio_threshhold(pDevice, SSPI_FIFO_ID_RX1, 0x3, 0x3);
    chal_sspi_set_fifo_pio_threshhold(pDevice, SSPI_FIFO_ID_TX0, 0x1, 0x1);
    chal_sspi_set_fifo_pio_threshhold(pDevice, SSPI_FIFO_ID_TX1, 0x3, 0x3);
    
	switch(protocol) {
    	case SSPI_PROT_MONO_16B_PCM:
    	case SSPI_PROT_MONO_25B_PCM:
            if(configTx->enable)
                //chal_sspi_set_fifo_threshold(pDevice, SSPI_FIFO_ID_TX0, 0x10);
                chal_sspi_set_fifo_threshold(pDevice, SSPI_FIFO_ID_TX0, 0x00);
            task_conf.chan_sel = SSPI_CHAN_SEL_CHAN0;
            task_conf.cs_sel = SSPI_CS_SEL_CS0;
		task_conf.rx_sel = (configTx->loopback_enable)
						   ? SSPI_RX_SEL_COPY_TX0 : SSPI_RX_SEL_RX0;
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

		task_conf.init_cond_mask = (configTx->enable)
								   ? SSPI_TASK_INIT_COND_THRESHOLD_TX0 : 0;
            task_conf.wait_before_start = 1;
            if(chal_sspi_set_task(pDevice, 0, protocol, &task_conf))
                return(CSL_PCM_ERR_TASK);

		seq_conf.tx_enable = (configTx->enable || configRx->loopback_enable)
							 ? TRUE : FALSE;
		seq_conf.rx_enable = (configRx->enable || configTx->loopback_enable)
							 ? TRUE : FALSE;
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

		if(chal_sspi_set_frame(pDevice, &frmMask, protocol,
							   (protocol==SSPI_PROT_MONO_25B_PCM) ? 24 : 16,
							   0))
                return(CSL_PCM_ERR_FRAME);
            break;    	

        case SSPI_PROT_STEREO_16B_PCM:
        case SSPI_PROT_STEREO_25B_PCM:			
            if(configTx->enable) {
#ifdef SSPI_TDM_MODE		   
                //chal_sspi_set_fifo_threshold(pDevice, SSPI_FIFO_ID_TX0, 0x08);
                chal_sspi_set_fifo_threshold(pDevice, SSPI_FIFO_ID_TX0, 0x0);

                chal_sspi_set_fifo_threshold(pDevice, SSPI_FIFO_ID_TX1, 0x0);
#else
                // use default value per suggestion from asic team
#if 0                
               
                chal_sspi_set_fifo_threshold(pDevice, SSPI_FIFO_ID_TX0, 0x10);
                chal_sspi_set_fifo_threshold(pDevice, SSPI_FIFO_ID_TX1, 0x0);
#else
              
               chal_sspi_set_fifo_threshold(pDevice, SSPI_FIFO_ID_TX0, 0x0);
                chal_sspi_set_fifo_threshold(pDevice, SSPI_FIFO_ID_TX1, 0x0);
#endif			
#endif
            }

            task_conf.chan_sel = SSPI_CHAN_SEL_CHAN0;
            task_conf.cs_sel = SSPI_CS_SEL_CS0;
		task_conf.rx_sel = (configTx->loopback_enable)
						   ? SSPI_RX_SEL_COPY_TX0 : SSPI_RX_SEL_RX0;
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
								   (SSPI_TASK_INIT_COND_THRESHOLD_TX0 |
									SSPI_TASK_INIT_COND_THRESHOLD_TX1) : 0;
            task_conf.wait_before_start = 1;
            if(chal_sspi_set_task(pDevice, 0, protocol, &task_conf))
                return(CSL_PCM_ERR_TASK);

            if(devCfg->protocol == CSL_PCM_PROTOCOL_STEREO) {
			seq_conf.tx_enable = (configTx->enable ||
								  configRx->loopback_enable) ? TRUE : FALSE;
			seq_conf.rx_enable = (configRx->enable ||
								  configTx->loopback_enable) ? TRUE : FALSE;
                seq_conf.cs_activate = 1;
                seq_conf.cs_deactivate = 1;
                seq_conf.pattern_mode = 0;
                seq_conf.rep_cnt = 0;
                seq_conf.opcode = SSPI_SEQ_OPCODE_NEXT_PC;
                seq_conf.rx_fifo_sel = 0;
                seq_conf.tx_fifo_sel = 0;
                //seq_conf.frm_sel = 0;
                seq_conf.frm_sel = 1;
                seq_conf.rx_sidetone_on = 0;
                seq_conf.tx_sidetone_on = 0;
                seq_conf.next_pc = 0;
                if(chal_sspi_set_sequence(pDevice, 0, protocol, &seq_conf))
                    return(CSL_PCM_ERR_SEQUENCE);

			seq_conf.tx_enable = (configTx->enable ||
								  configRx->loopback_enable)
								 ? TRUE : FALSE;
			seq_conf.rx_enable = (configRx->enable ||
								  configTx->loopback_enable)
								 ? TRUE : FALSE;
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
        else if(devCfg->protocol == CSL_PCM_PROTOCOL_INTERLEAVE_3CHANNEL) {
			seq_conf.tx_enable = (configTx->enable || configRx->loopback_enable)
								 ? 1 : 0;
			seq_conf.rx_enable = (configRx->enable || configTx->loopback_enable)
								 ? 1 : 0;
            seq_conf.cs_activate = 1;
            seq_conf.cs_deactivate = 1;
            seq_conf.pattern_mode = 0;
            seq_conf.rep_cnt = 0;
            seq_conf.opcode = SSPI_SEQ_OPCODE_NEXT_PC;
            seq_conf.rx_fifo_sel = 0;
            seq_conf.tx_fifo_sel = 0;
            seq_conf.frm_sel = 1;
            seq_conf.rx_sidetone_on = 0;
            seq_conf.tx_sidetone_on = 0;
            seq_conf.next_pc = 0;
            if(chal_sspi_set_sequence(handle, 0, protocol, &seq_conf))
                return(CSL_PCM_ERR_SEQUENCE);

			seq_conf.tx_enable = (configTx->enable || configRx->loopback_enable)
								 ? 1 : 0;
			seq_conf.rx_enable = (configRx->enable || configTx->loopback_enable)
								 ? 1 : 0;
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
            if(chal_sspi_set_sequence(handle, 1, protocol, &seq_conf))
                return(CSL_PCM_ERR_SEQUENCE);

			seq_conf.tx_enable = (configTx->enable ||
								  configRx->loopback_enable) ? 1 : 0;
			seq_conf.rx_enable = (configRx->enable ||
								  configTx->loopback_enable) ? 1 : 0;
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
                if(chal_sspi_set_sequence(handle, 2, protocol, &seq_conf))
                    return(CSL_PCM_ERR_SEQUENCE);

                seq_conf.tx_enable = 0;
                seq_conf.rx_enable = 0;
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
                if(chal_sspi_set_sequence(handle, 3, protocol, &seq_conf))
                    return(CSL_PCM_ERR_SEQUENCE);
            }
        else if (devCfg->protocol == CSL_PCM_PROTOCOL_INTERLEAVE_4CHANNEL) { // pCore->prot == SSPI_HW_INTERLEAVE_NOKIA_PCM_4CHANNEL
			seq_conf.tx_enable = (configTx->enable || configRx->loopback_enable)
								  ? 1 : 0;
			seq_conf.rx_enable = (configRx->enable || configTx->loopback_enable)
								  ? 1 : 0;
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
            if(chal_sspi_set_sequence(handle, 0, protocol, &seq_conf))
                return(CSL_PCM_ERR_SEQUENCE);

			seq_conf.tx_enable = (configTx->enable || configRx->loopback_enable)
								 ? 1 : 0;
			seq_conf.rx_enable = (configRx->enable || configTx->loopback_enable)
								 ? 1 : 0;
            seq_conf.cs_activate = 0;
            seq_conf.cs_deactivate = 0;
            seq_conf.pattern_mode = 0;
            seq_conf.rep_cnt = 0;
            seq_conf.opcode = SSPI_SEQ_OPCODE_NEXT_PC;
            seq_conf.rx_fifo_sel = 0;
            seq_conf.tx_fifo_sel = 0;
            seq_conf.frm_sel = 0;
            seq_conf.rx_sidetone_on = 0;
            seq_conf.tx_sidetone_on = 0;
            seq_conf.next_pc = 0;
            if(chal_sspi_set_sequence(handle, 1, protocol, &seq_conf))
                return(CSL_PCM_ERR_SEQUENCE);

			seq_conf.tx_enable = (configTx->enable || configRx->loopback_enable)
								  ? 1 : 0;
			seq_conf.rx_enable = (configRx->enable || configTx->loopback_enable)
								  ? 1 : 0;
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
            if(chal_sspi_set_sequence(handle, 2, protocol, &seq_conf))
                return(CSL_PCM_ERR_SEQUENCE);

			seq_conf.tx_enable = (configTx->enable ||
								  configRx->loopback_enable) ? 1 : 0;
			seq_conf.rx_enable = (configRx->enable ||
								  configTx->loopback_enable) ? 1 : 0;
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
                if(chal_sspi_set_sequence(handle, 3, protocol, &seq_conf))
                    return(CSL_PCM_ERR_SEQUENCE);

                seq_conf.tx_enable = 0;
                seq_conf.rx_enable = 0;
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
                if(chal_sspi_set_sequence(handle, 4, protocol, &seq_conf))
                    return(CSL_PCM_ERR_SEQUENCE);
            }

            if (0 /*devCfg->format == CSL_PCM_WORD_LENGTH_24_BIT*/)
            {   // should we use 25bit here???
	            // if(chal_sspi_set_frame(pDevice, &frmMask, protocol, SSPI_HW_WORD_LEN_16Bit, SSPI_HW_DUMMY_BITS_9))
			if(chal_sspi_set_frame(pDevice, &frmMask, protocol,
								   SSPI_HW_WORD_LEN_24Bit, 0))
		            return(CSL_PCM_ERR_FRAME);
            }
            else
            {
                // to make it work as expected, need to use these settings before set frame 
                if (devCfg->protocol == CSL_PCM_PROTOCOL_INTERLEAVE_3CHANNEL)
#ifndef SSPI_TDM_MODE					
			{
				protocol = SSPI_PROT_3CHAN_16B_TDM_PCM;
				devCfg->ext_bits = 2;
			}
#else
			// to be used with new chal code
			{
				protocol = SSPI_PROT_3CHAN_16B_24B_TDM_PCM;
				devCfg->ext_bits = 0;
			}
#endif
                else if (devCfg->protocol == CSL_PCM_PROTOCOL_INTERLEAVE_4CHANNEL)
    			{ 
					protocol = SSPI_PROT_4CHAN_16B_TDM_PCM; 
					devCfg->ext_bits = 1;
				}
                else
    			{ 
					protocol = SSPI_PROT_STEREO_16B_PCM; 
					devCfg->ext_bits = 0;
				}		
    		
			if(chal_sspi_set_frame(pDevice, &frmMask, protocol,
								   SSPI_HW_WORD_LEN_16Bit,
								   devCfg->ext_bits))
                    return(CSL_PCM_ERR_FRAME);
            }
            break;

    case SSPI_PROT_3CHAN_16B_TDM_PCM:
        if(configTx->enable) {
            chal_sspi_set_fifo_threshold(handle, SSPI_FIFO_ID_TX0, 0x10);
            chal_sspi_set_fifo_threshold(handle, SSPI_FIFO_ID_TX1, 0x10);
            chal_sspi_set_fifo_threshold(handle, SSPI_FIFO_ID_TX2, 0x10);
        }

        task_conf.chan_sel = SSPI_CHAN_SEL_CHAN0;
        task_conf.cs_sel = SSPI_CS_SEL_CS0;
        task_conf.rx_sel = (configTx->loopback_enable) ? SSPI_RX_SEL_COPY_TX0
                                                : SSPI_RX_SEL_RX0;
        task_conf.tx_sel = SSPI_TX_SEL_TX0;
        task_conf.div_sel = SSPI_CLK_DIVIDER0;
        task_conf.seq_ptr = 0;
        task_conf.init_cond_mask = SSPI_TASK_INIT_COND_THRESHOLD_TX0 |
                                 SSPI_TASK_INIT_COND_THRESHOLD_TX1 |
                                 SSPI_TASK_INIT_COND_THRESHOLD_TX2;

        if((devCfg->xferSize>> 1) > 0x400) {
            task_conf.loop_cnt = 0;
            task_conf.continuous = 1;
        }
        else {
            task_conf.loop_cnt = (devCfg->xferSize>> 1) - 1;
            task_conf.continuous = 0;
        }
        task_conf.init_cond_mask = (configTx->enable) ?
								   (SSPI_TASK_INIT_COND_THRESHOLD_TX0 |
									SSPI_TASK_INIT_COND_THRESHOLD_TX1 |
                                                 SSPI_TASK_INIT_COND_THRESHOLD_TX2) : 0;

        task_conf.wait_before_start = 1;
        if(chal_sspi_set_task(handle, 0, protocol, &task_conf))
                return(CSL_PCM_ERR_TASK);

		seq_conf.tx_enable = (configTx->enable || configRx->loopback_enable)
							 ? 1 : 0;
		seq_conf.rx_enable = (configRx->enable || configTx->loopback_enable)
							 ? 1 : 0;
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
        if(chal_sspi_set_sequence(handle, 0, protocol, &seq_conf))
            return(CSL_PCM_ERR_SEQUENCE);

		seq_conf.tx_enable = (configTx->enable || configRx->loopback_enable)
							 ? 1 : 0;
		seq_conf.rx_enable = (configRx->enable || configTx->loopback_enable)
							 ? 1 : 0;
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
        if(chal_sspi_set_sequence(handle, 1, protocol, &seq_conf))
            return(CSL_PCM_ERR_SEQUENCE);

        if(!devCfg->ext_bits) {
			seq_conf.tx_enable = (configTx->enable || configRx->loopback_enable)
								 ? 1 : 0;
			seq_conf.rx_enable = (configRx->enable || configTx->loopback_enable)
								 ? 1 : 0;
            seq_conf.cs_activate = 0;
            seq_conf.cs_deactivate = 0;
            seq_conf.pattern_mode = 0;
            seq_conf.rep_cnt = 0;
            seq_conf.opcode = SSPI_SEQ_OPCODE_COND_JUMP;
            seq_conf.rx_fifo_sel = 2;
            seq_conf.tx_fifo_sel = 2;
            seq_conf.frm_sel = 0;
            seq_conf.rx_sidetone_on = 0;
            seq_conf.tx_sidetone_on = 0;
            seq_conf.next_pc = 0;
            if(chal_sspi_set_sequence(handle, 2, protocol, &seq_conf))
                return(CSL_PCM_ERR_SEQUENCE);

            seq_conf.tx_enable = 0;
            seq_conf.rx_enable = 0;
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
            if(chal_sspi_set_sequence(handle, 3, protocol, &seq_conf))
                return(CSL_PCM_ERR_SEQUENCE);
        }
        else {
			seq_conf.tx_enable = (configTx->enable || configRx->loopback_enable)
								 ? 1 : 0;
			seq_conf.rx_enable = (configRx->enable || configTx->loopback_enable)
								 ? 1 : 0;
            seq_conf.cs_activate = 0;
            seq_conf.cs_deactivate = 0;
            seq_conf.pattern_mode = 0;
            seq_conf.rep_cnt = 0;
            seq_conf.opcode = SSPI_SEQ_OPCODE_NEXT_PC;
            seq_conf.rx_fifo_sel = 2;
            seq_conf.tx_fifo_sel = 2;
            seq_conf.frm_sel = 0;
            seq_conf.rx_sidetone_on = 0;
            seq_conf.tx_sidetone_on = 0;
            seq_conf.next_pc = 0;
            if(chal_sspi_set_sequence(handle, 2, protocol, &seq_conf))
                return(CSL_PCM_ERR_SEQUENCE);

            seq_conf.tx_enable = 0;
            seq_conf.rx_enable = 0;
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
            if(chal_sspi_set_sequence(handle, 3, protocol, &seq_conf))
                return(CSL_PCM_ERR_SEQUENCE);

            seq_conf.tx_enable = 0;
            seq_conf.rx_enable = 0;
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
            if(chal_sspi_set_sequence(handle, 4, protocol, &seq_conf))
                return(CSL_PCM_ERR_SEQUENCE);

        }

        if(chal_sspi_set_frame(handle, &frmMask, protocol, 16, 2))
            return(CSL_PCM_ERR_FRAME);

        break;
    case SSPI_PROT_4CHAN_16B_TDM_PCM:
        if(configTx->enable) {
            chal_sspi_set_fifo_threshold(handle, SSPI_FIFO_ID_TX0, 0x10);
            chal_sspi_set_fifo_threshold(handle, SSPI_FIFO_ID_TX1, 0x10);
            chal_sspi_set_fifo_threshold(handle, SSPI_FIFO_ID_TX2, 0x10);
            chal_sspi_set_fifo_threshold(handle, SSPI_FIFO_ID_TX3, 0x10);
        }

        task_conf.chan_sel = SSPI_CHAN_SEL_CHAN0;
        task_conf.cs_sel = SSPI_CS_SEL_CS0;
        task_conf.rx_sel = (configTx->loopback_enable) ? SSPI_RX_SEL_COPY_TX0
                                                : SSPI_RX_SEL_RX0;
        task_conf.tx_sel = SSPI_TX_SEL_TX0;
        task_conf.div_sel = SSPI_CLK_DIVIDER0;
        task_conf.seq_ptr = 0;

        if((devCfg->xferSize>> 1) > 0x400) {
            task_conf.loop_cnt = 0;
            task_conf.continuous = 1;
        }
        else {
            task_conf.loop_cnt = (devCfg->xferSize>> 1) - 1;
            task_conf.continuous = 0;
        }
        task_conf.init_cond_mask = (configTx->enable) ?
								   (SSPI_TASK_INIT_COND_THRESHOLD_TX0 |
									SSPI_TASK_INIT_COND_THRESHOLD_TX1 |
									SSPI_TASK_INIT_COND_THRESHOLD_TX2 |
									SSPI_TASK_INIT_COND_THRESHOLD_TX3) : 0;

        task_conf.wait_before_start = 1;
        if(chal_sspi_set_task(handle, 0, protocol, &task_conf))
                return(CSL_PCM_ERR_TASK);

		seq_conf.tx_enable = (configTx->enable || configRx->loopback_enable)
							 ? 1 : 0;
		seq_conf.rx_enable = (configRx->enable || configTx->loopback_enable)
							 ? 1 : 0;
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
        if(chal_sspi_set_sequence(handle, 0, protocol, &seq_conf))
            return(CSL_PCM_ERR_SEQUENCE);

		seq_conf.tx_enable = (configTx->enable || configRx->loopback_enable)
							 ? 1 : 0;
		seq_conf.rx_enable = (configRx->enable || configTx->loopback_enable)
							 ? 1 : 0;
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
        if(chal_sspi_set_sequence(handle, 1, protocol, &seq_conf))
            return(CSL_PCM_ERR_SEQUENCE);

		seq_conf.tx_enable = (configTx->enable || configRx->loopback_enable)
							 ? 1 : 0;
		seq_conf.rx_enable = (configRx->enable || configTx->loopback_enable)
							 ? 1 : 0;
        seq_conf.cs_activate = 0;
        seq_conf.cs_deactivate = 0;
        seq_conf.pattern_mode = 0;
        seq_conf.rep_cnt = 0;
        seq_conf.opcode = SSPI_SEQ_OPCODE_NEXT_PC;
        seq_conf.rx_fifo_sel = 2;
        seq_conf.tx_fifo_sel = 2;
        seq_conf.frm_sel = 0;
        seq_conf.rx_sidetone_on = 0;
        seq_conf.tx_sidetone_on = 0;
        seq_conf.next_pc = 0;
        if(chal_sspi_set_sequence(handle, 2, protocol, &seq_conf))
            return(CSL_PCM_ERR_SEQUENCE);

        if(!devCfg->ext_bits) {
			seq_conf.tx_enable = (configTx->enable|| configRx->loopback_enable)
								 ? 1 : 0;
			seq_conf.rx_enable = (configRx->enable|| configTx->loopback_enable)
								 ? 1 : 0;
            seq_conf.cs_activate = 0;
            seq_conf.cs_deactivate = 0;
            seq_conf.pattern_mode = 0;
            seq_conf.rep_cnt = 0;
            seq_conf.opcode = SSPI_SEQ_OPCODE_COND_JUMP;
            seq_conf.rx_fifo_sel = 3;
            seq_conf.tx_fifo_sel = 3;
            seq_conf.frm_sel = 0;
            seq_conf.rx_sidetone_on = 0;
            seq_conf.tx_sidetone_on = 0;
            seq_conf.next_pc = 0;
            if(chal_sspi_set_sequence(handle, 3, protocol, &seq_conf))
                return(CSL_PCM_ERR_SEQUENCE);

            seq_conf.tx_enable = 0;
            seq_conf.rx_enable = 0;
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
            if(chal_sspi_set_sequence(handle, 4, protocol, &seq_conf))
                return(CSL_PCM_ERR_SEQUENCE);
        }
        else {
			seq_conf.tx_enable = (configTx->enable || configRx->loopback_enable)
								 ? 1 : 0;
			seq_conf.rx_enable = (configRx->enable || configTx->loopback_enable)
								 ? 1 : 0;
            seq_conf.cs_activate = 0;
            seq_conf.cs_deactivate = 0;
            seq_conf.pattern_mode = 0;
            seq_conf.rep_cnt = 0;
            seq_conf.opcode = SSPI_SEQ_OPCODE_NEXT_PC;
            seq_conf.rx_fifo_sel = 3;
            seq_conf.tx_fifo_sel = 3;
            seq_conf.frm_sel = 0;
            seq_conf.rx_sidetone_on = 0;
            seq_conf.tx_sidetone_on = 0;
            seq_conf.next_pc = 0;
            if(chal_sspi_set_sequence(handle, 3, protocol, &seq_conf))
                return(CSL_PCM_ERR_SEQUENCE);

            seq_conf.tx_enable = 0;
            seq_conf.rx_enable = 0;
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
            if(chal_sspi_set_sequence(handle, 4, protocol, &seq_conf))
                return(CSL_PCM_ERR_SEQUENCE);

            seq_conf.tx_enable = 0;
            seq_conf.rx_enable = 0;
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
            if(chal_sspi_set_sequence(handle, 5, protocol, &seq_conf))
                return(CSL_PCM_ERR_SEQUENCE);

        }

        if(chal_sspi_set_frame(handle, &frmMask, protocol, 16, 1))
            return(CSL_PCM_ERR_FRAME);

        break;			
		default:
			break;
    }
    
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
    CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *)handle;
	return (UInt32)(pDevice->base+SSPIL_FIFO_ENTRY0TX_OFFSET);
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
    CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *)handle;
	return (UInt32)(pDevice->base+SSPIL_FIFO_ENTRY1TX_OFFSET);
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
	CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *)handle;
	return (UInt32)(pDevice->base+SSPIL_FIFO_ENTRY0RX_OFFSET);
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
	CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *)handle;
	return (UInt32)(pDevice->base+SSPIL_FIFO_ENTRY1RX_OFFSET);
}
