/*****************************************************************************
* Copyright 2001 - 2009 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/
/**
*  
*  @file   csl_caph_i2s_sspi.c
*  @brief  CSL layer to using SSPI cHAL interface to define I2S interface
*  @note
*
*****************************************************************************/
//===========================================================================
// inlcude header file declarations
//
#include <string.h>
#include "mobcom_types.h"
#include "chal_types.h"
#ifdef LMP_BUILD
#include "chal_sspi.h"
#include "dbg.h"
#endif
#include "csl_caph_i2s_sspi.h"
#include "brcm_rdb_sspil.h"
#include "brcm_rdb_padctrlreg.h"
#include "brcm_rdb_sysmap_a9.h"
#include "log.h"

//===========================================================================
// global variable declarations
//g

//===========================================================================
// extern variable declarations
//

//===========================================================================
// static function prototype declarations
//

//===========================================================================
// local define declarations
//
#define SSPI_HW_WORD_LEN_32Bit                    32
#define SSPI_HW_WORD_LEN_25Bit                    25
#define SSPI_HW_WORD_LEN_24Bit                    24
#define SSPI_HW_WORD_LEN_16Bit                    16
#define SSPI_HW_WORD_LEN_8Bit                     8
#if 0
static chal_sspi_task_conf_t tk_conf;
static chal_sspi_seq_conf_t seq_conf;
static SSPI_hw_status_t SSPI_hw_i2s_init(CSL_HANDLE handle, CSL_I2S_CONFIG_t *config);
static SSPI_hw_status_t SSPI_hw_interleave_init(CSL_HANDLE handle, CSL_I2S_CONFIG_t *config);
static SSPI_hw_status_t SSPI_hw_i2s_slave_init(CSL_HANDLE handle, CSL_I2S_CONFIG_t *config);
static SSPI_hw_status_t SSPI_hw_interleave_slave_init(CSL_HANDLE handle, CSL_I2S_CONFIG_t *config);
#endif
//===========================================================================
// static function declarations
// 
/****************************************************************************
*
*  Function Name: csl_i2s_config_pinmux(UInt32 address)
*
*  Description: config sspi pinmux
*
****************************************************************************/
#if 0 //disabled to remove gcc compile warnings
static void csl_i2s_config_pinmux(UInt32 address)
{	
    UInt32 regVal;

    Log_DebugPrintf(LOGID_SOC_AUDIO, "+csl_i2s_config_pinmux \n");

#define PIN_MUX_ALT0    0
#define PIN_MUX_ALT1    (1 << (PADCTRLREG_GPIO93_PINSEL_GPIO93_SHIFT))
#define PIN_MUX_ALT2    (2 << (PADCTRLREG_GPIO93_PINSEL_GPIO93_SHIFT))
#define PIN_MUX_ALT3    (3 << (PADCTRLREG_GPIO93_PINSEL_GPIO93_SHIFT))
#define PIN_MUX_ALT4    (4 << (PADCTRLREG_GPIO93_PINSEL_GPIO93_SHIFT))
#define PIN_MUX_ALT5    (5 << (PADCTRLREG_GPIO93_PINSEL_GPIO93_SHIFT))

#define  READ_REG32(reg)             ( *((volatile int *) (reg)) )
#define  WRITE_REG32(reg, value)     ( *((volatile int *) (reg)) = (int) (value) )
#define  MASK_REG32(reg, mask)   (WRITE_REG32((reg), (READ_REG32((reg)) & (~(mask)))))
    if (address == SSP4_BASE_ADDR) {
        // Set pin share for SSP4. Keep this code in case FM is configured to use SSP4.
        // SSP4-FS
        regVal = (READ_REG32(PAD_CTRL_BASE_ADDR + PADCTRLREG_GPIO94_OFFSET) &
                    (~(PADCTRLREG_GPIO94_PINSEL_GPIO94_MASK | PADCTRLREG_GPIO94_PUP_GPIO94_MASK |
                       PADCTRLREG_GPIO94_IND_GPIO94_MASK))) | PIN_MUX_ALT2;
        WRITE_REG32(PAD_CTRL_BASE_ADDR + PADCTRLREG_GPIO94_OFFSET, regVal);

        // SSP4-CLK
        regVal = (READ_REG32(PAD_CTRL_BASE_ADDR + PADCTRLREG_GPIO32_OFFSET) &
                    (~(PADCTRLREG_GPIO32_PINSEL_GPIO32_MASK | PADCTRLREG_GPIO32_PUP_GPIO32_MASK |
                       PADCTRLREG_GPIO32_IND_GPIO32_MASK))) | PIN_MUX_ALT2;
        WRITE_REG32(PAD_CTRL_BASE_ADDR + PADCTRLREG_GPIO32_OFFSET, regVal);

        // SSP4-TX
        regVal = (READ_REG32(PAD_CTRL_BASE_ADDR + PADCTRLREG_DCLK4_OFFSET) &
                    (~(PADCTRLREG_DCLK4_PINSEL_DCLK4_MASK |
                       PADCTRLREG_DCLK4_PUP_DCLK4_MASK))) |
                    PADCTRLREG_DCLK4_IND_DCLK4_MASK |
                    PADCTRLREG_DCLK4_HYS_EN_DCLK4_MASK |
                    PIN_MUX_ALT2;
        WRITE_REG32(PAD_CTRL_BASE_ADDR + PADCTRLREG_DCLK4_OFFSET, regVal);

        // SSP4-RX
        regVal = (READ_REG32(PAD_CTRL_BASE_ADDR + PADCTRLREG_DCLKREQ4_OFFSET) &
                    (~(PADCTRLREG_DCLKREQ4_PINSEL_DCLKREQ4_MASK |
                       PADCTRLREG_DCLKREQ4_PUP_DCLKREQ4_MASK))) |
                    PADCTRLREG_DCLKREQ4_HYS_EN_DCLKREQ4_MASK |
                    PIN_MUX_ALT2;
        WRITE_REG32(PAD_CTRL_BASE_ADDR + PADCTRLREG_DCLKREQ4_OFFSET, regVal);
		Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_i2s_config_pinmux: SSP4 \n"); 
	}
	else if (address == SSP3_BASE_ADDR) {
       // Set pin share for SSP3
        // SYN
        regVal = (READ_REG32(PAD_CTRL_BASE_ADDR + PADCTRLREG_GPIO15_OFFSET) &
                    (~(PADCTRLREG_GPIO15_PINSEL_GPIO15_MASK | PADCTRLREG_GPIO15_PUP_GPIO15_MASK |
                       PADCTRLREG_GPIO15_IND_GPIO15_MASK))) | PIN_MUX_ALT4;
        WRITE_REG32(PAD_CTRL_BASE_ADDR + PADCTRLREG_GPIO15_OFFSET, regVal);

        // CLK
        regVal = (READ_REG32(PAD_CTRL_BASE_ADDR + PADCTRLREG_GPIO14_OFFSET) &
                    (~(PADCTRLREG_GPIO14_PINSEL_GPIO14_MASK | PADCTRLREG_GPIO14_PUP_GPIO14_MASK |
                       PADCTRLREG_GPIO14_IND_GPIO14_MASK))) | PIN_MUX_ALT4;
        WRITE_REG32(PAD_CTRL_BASE_ADDR + PADCTRLREG_GPIO14_OFFSET, regVal);

        // DO
        regVal = (READ_REG32(PAD_CTRL_BASE_ADDR + PADCTRLREG_GPIO07_OFFSET) &
                    (~(PADCTRLREG_GPIO07_PINSEL_GPIO07_MASK | PADCTRLREG_GPIO07_PUP_GPIO07_MASK |
                       PADCTRLREG_GPIO07_IND_GPIO07_MASK))) | PIN_MUX_ALT4;
        WRITE_REG32(PAD_CTRL_BASE_ADDR + PADCTRLREG_GPIO07_OFFSET, regVal);

        // DI
        regVal = (READ_REG32(PAD_CTRL_BASE_ADDR + PADCTRLREG_GPIO06_OFFSET) &
                    (~(PADCTRLREG_GPIO06_PINSEL_GPIO06_MASK | PADCTRLREG_GPIO06_PUP_GPIO06_MASK |
                       PADCTRLREG_GPIO06_IND_GPIO06_MASK))) | PIN_MUX_ALT4;
        WRITE_REG32(PAD_CTRL_BASE_ADDR + PADCTRLREG_GPIO06_OFFSET, regVal);
		Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_i2s_config_pinmux: SSP3 \n"); 
	}
	else{
		Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_i2s_config_pinmux: Wrong SSP, not support yet \n"); 
	}

	Log_DebugPrintf(LOGID_SOC_AUDIO, "-csl_i2s_config_pinmux \n"); 
}
#endif
/****************************************************************************
*
*  Function Name: csl_i2s_init(cUInt32 baseAddr)
*
*  Description: Initialize I2S
*
****************************************************************************/
CSL_HANDLE csl_i2s_init(cUInt32 baseAddr)
{
	CSL_HANDLE handle = 0;
#if 0
	CSL_SSPI_HANDLE_T *pDevice;	
   	Log_DebugPrintf(LOGID_SOC_AUDIO, "+csl_i2s_init \n");

    handle = chal_sspi_init(baseAddr);
	pDevice = (CSL_SSPI_HANDLE_T *)handle;
	csl_i2s_config_pinmux(baseAddr);

   	Log_DebugPrintf(LOGID_SOC_AUDIO, "base address in csl 0x%x \r\n", pDevice->base);
   	Log_DebugPrintf(LOGID_SOC_AUDIO, "-csl_i2s_init \r\n");
#endif
	return handle;

}

/****************************************************************************
*
*  Function Name: csl_i2s_deinit(cUInt32 baseAddr)
*
*  Description: De-Initialize I2S
*
****************************************************************************/
void csl_i2s_deinit(CSL_HANDLE handle)
{
#if 0
	chal_sspi_deinit(handle);
#endif	
}

/****************************************************************************
*
*  Function Name: csl_i2s_config(CSL_HANDLE handle,CSL_I2S_CONFIG_t *config, 
*					CSL_I2S_CONFIG_TX_t *configTx, CSL_I2S_CONFIG_RX_t *configRx)
*
*  Description: Funtion to config SSPI as I2S
*
****************************************************************************/
void csl_i2s_config(CSL_HANDLE handle,CSL_I2S_CONFIG_t *config)
{
#if 0
//	UInt32 dma_trans_size=4096;
	CSL_SSPI_HANDLE_T *pDevice;
    SSPI_hw_status_t status=SSPI_HW_NOERR;

   	Log_DebugPrintf(LOGID_SOC_AUDIO, "+csl_i2s_config \r\n");

	if (config->mode == CSL_I2S_MASTER_MODE){ 
		pDevice = (CSL_SSPI_HANDLE_T *)handle;
    	Log_DebugPrintf(LOGID_SOC_AUDIO, "Master Mode base address 0x%x \r\n", pDevice->base);
    	Log_DebugPrintf(LOGID_SOC_AUDIO, "handle 0x%x \r\n", handle);


		if(!config->interleave)
          status=SSPI_hw_i2s_init(handle, config);
		else
		  status=SSPI_hw_interleave_init(handle, config);

        if(status)
        {
    	    Log_DebugPrintf(LOGID_SOC_AUDIO, "SSPI_hw_i2s_init failed \r\n");
        }

	}
	else{
		if(!config->interleave)
		  status=SSPI_hw_i2s_slave_init(handle, config);
		else
		  status=SSPI_hw_interleave_slave_init(handle, config);
	
   	    Log_DebugPrintf(LOGID_SOC_AUDIO, "Slave Mode base address 0x%x \r\n", pDevice->base);
	}
   	Log_DebugPrintf(LOGID_SOC_AUDIO, "-csl_i2s_config \r\n");
#endif
	return;

}

/****************************************************************************
*
*  Function Name: csl_i2s_start(CSL_HANDLE handle, CSL_I2S_CONFIG_t *config)
*
*  Description: I2S/SSPI start
*
****************************************************************************/
void csl_i2s_start(CSL_HANDLE handle, CSL_I2S_CONFIG_t *config)
{
#if 0
//	UInt32 dma_trans_size=4096;

   	Log_DebugPrintf(LOGID_SOC_AUDIO, "+csl_i2s_start \r\n");
   	Log_DebugPrintf(LOGID_SOC_AUDIO, "config->trans_size 0x%x \r\n",config->trans_size);
   	Log_DebugPrintf(LOGID_SOC_AUDIO, "handle 0x%x \r\n", handle);
   	Log_DebugPrintf(LOGID_SOC_AUDIO, "config 0x%x \r\n", config);

//	SSPI_hw_DMA_init(handle, config);
	chal_sspi_enable_scheduler(handle, 1);

    chal_sspi_enable_fifo_pio_start_stop_intr(handle, 
                                              SSPI_FIFO_ID_TX0, 
                                              TRUE, 
                                              TRUE); 
    chal_sspi_enable_fifo_pio_start_stop_intr(handle, 
                                              SSPI_FIFO_ID_TX1, 
                                              TRUE, 
                                              TRUE); 
    chal_sspi_enable_fifo_pio_start_stop_intr(handle, 
                                              SSPI_FIFO_ID_TX2, 
                                              TRUE, 
                                              TRUE); 
    chal_sspi_enable_fifo_pio_start_stop_intr(handle, 
                                              SSPI_FIFO_ID_TX3, 
                                              TRUE, 
                                              TRUE); 
    chal_sspi_enable_fifo_pio_start_stop_intr(handle, 
                                              SSPI_FIFO_ID_RX0, 
                                              TRUE, 
                                              TRUE); 
    chal_sspi_enable_fifo_pio_start_stop_intr(handle, 
                                              SSPI_FIFO_ID_RX1, 
                                              TRUE, 
                                              TRUE); 
    chal_sspi_enable_fifo_pio_start_stop_intr(handle, 
                                              SSPI_FIFO_ID_RX2, 
                                              TRUE, 
                                              TRUE); 
    chal_sspi_enable_fifo_pio_start_stop_intr(handle, 
                                              SSPI_FIFO_ID_RX3, 
                                              TRUE, 
                                              TRUE); 
#endif
	return;
}

/****************************************************************************
*
*  Function Name: csl_i2s_stop_tx(CSL_HANDLE handle)
*
*  Description: I2S/SSPI TX stop
*
****************************************************************************/
void csl_i2s_stop_tx(CSL_HANDLE handle)
{
#if 0
	chal_sspi_fifo_reset(handle, SSPI_FIFO_ID_TX0);
	chal_sspi_fifo_reset(handle, SSPI_FIFO_ID_TX1);
	chal_sspi_fifo_reset(handle, SSPI_FIFO_ID_TX0);
	chal_sspi_fifo_reset(handle, SSPI_FIFO_ID_TX1);

	//disable  master
	chal_sspi_enable_scheduler(handle, 0);


    chal_sspi_enable_fifo_pio_start_stop_intr(handle, 
                                              SSPI_FIFO_ID_TX0, 
                                              FALSE, 
                                              FALSE); 
    chal_sspi_enable_fifo_pio_start_stop_intr(handle, 
                                              SSPI_FIFO_ID_TX1, 
                                              FALSE, 
                                              FALSE); 
    chal_sspi_enable_fifo_pio_start_stop_intr(handle, 
                                              SSPI_FIFO_ID_TX2, 
                                              FALSE, 
                                              FALSE); 
    chal_sspi_enable_fifo_pio_start_stop_intr(handle, 
                                              SSPI_FIFO_ID_TX3, 
                                              FALSE, 
                                              FALSE); 

#endif    
	return;
}

/****************************************************************************
*
*  Function Name: csl_i2s_stop_rx(CSL_HANDLE handle)
*
*  Description: I2S/SSPI RX stop
*
****************************************************************************/
void csl_i2s_stop_rx(CSL_HANDLE handle)
{
#if 0
	chal_sspi_fifo_reset(handle, SSPI_FIFO_ID_RX0);
	chal_sspi_fifo_reset(handle, SSPI_FIFO_ID_RX1);
	chal_sspi_fifo_reset(handle, SSPI_FIFO_ID_RX0);
	chal_sspi_fifo_reset(handle, SSPI_FIFO_ID_RX1);

	//disable  master
	chal_sspi_enable_scheduler(handle, 0);

    chal_sspi_enable_fifo_pio_start_stop_intr(handle, 
                                              SSPI_FIFO_ID_RX0, 
                                              FALSE, 
                                              FALSE); 
    chal_sspi_enable_fifo_pio_start_stop_intr(handle, 
                                              SSPI_FIFO_ID_RX1, 
                                              FALSE, 
                                              FALSE); 
    chal_sspi_enable_fifo_pio_start_stop_intr(handle, 
                                              SSPI_FIFO_ID_RX2, 
                                              FALSE, 
                                              FALSE); 
    chal_sspi_enable_fifo_pio_start_stop_intr(handle, 
                                              SSPI_FIFO_ID_RX3, 
                                              FALSE, 
                                              FALSE); 
#endif
	return;
}

UInt32 csl_i2s_get_tx0_fifo_data_port(CSL_HANDLE handle)
{
#if 0
	CSL_SSPI_HANDLE_T *pDevice = (CSL_SSPI_HANDLE_T *)handle;
	
	return (UInt32)(pDevice->base+SSPIL_FIFO_ENTRY0TX_OFFSET);
#endif
	return 0	;
}

UInt32 csl_i2s_get_rx0_fifo_data_port(CSL_HANDLE handle)
{
#if 0
	CSL_SSPI_HANDLE_T *pDevice = (CSL_SSPI_HANDLE_T *)handle;
	return (UInt32)(pDevice->base+SSPIL_FIFO_ENTRY0RX_OFFSET);
#endif
	return 0	;
}
UInt32 csl_i2s_get_tx1_fifo_data_port(CSL_HANDLE handle)
{
#if 0

	CSL_SSPI_HANDLE_T *pDevice = (CSL_SSPI_HANDLE_T *)handle;
	return (UInt32)(pDevice->base+SSPIL_FIFO_ENTRY1TX_OFFSET);
#endif
	return 0	;

}

UInt32 csl_i2s_get_rx1_fifo_data_port(CSL_HANDLE handle)
{
#if 0
	CSL_SSPI_HANDLE_T *pDevice = (CSL_SSPI_HANDLE_T *)handle;
	return (UInt32)(pDevice->base+SSPIL_FIFO_ENTRY1RX_OFFSET);
#endif
	return 0	;
	
}

#if 0 //disabled to remove gcc compile warnings
/****************************************************************************
*
* NAME:  SSPI_hw_i2s_init
*
*
*  Description:  This function initializes the SSPI core that specified by coreIdx.
*	as two channel I2S, Master Mode, left and Right seperated
*
*  Parameters:
*     SSPI_hw_core_t *pCore - SSPI core pointer
*
*  Returns:  SSPI_hw_status_t
*
*
*
*  Notes:
*
****************************************************************************/
static SSPI_hw_status_t SSPI_hw_i2s_init(CSL_HANDLE handle, CSL_I2S_CONFIG_t *config)
{
#if 0
    cUInt32 frmMask = 1;
    CHAL_SSPI_PROT_t mode;
//	CHAL_SSPI_CLK_SRC_t clk_source;
	CHAL_SSPI_FIFO_DATA_PACK_t fifo_pack;
	cUInt32 clk_div;
	cUInt32 temp;
//    CSL_SSPI_HANDLE_T *pDevice = (CSL_SSPI_HANDLE_T *)handle;

    if(config->prot == SSPI_HW_I2S_MODE1)
        mode = SSPI_PROT_I2S_MODE1;
    else if(config->prot == SSPI_HW_I2S_MODE2)
        mode = SSPI_PROT_I2S_MODE2;
    else
        return SSPI_HW_ERR_PROT;

	temp = *(volatile int *)0x3502F000;
	*(volatile int *)0x3502F000 = temp | 0x1;

#if 0
	switch (config->sampleRate) 
	{
		case CSL_I2S_16BIT_4000HZ:
			clk_source = SSPI_CLK_SRC_AUDIOCLK;
			fifo_pack = SSPI_FIFO_DATA_PACK_16BIT;
			clk_div = 11;
   	        Log_DebugPrintf(LOGID_SOC_AUDIO, "sample Rate = CSL_I2S_16BIT_4000HZ \r\n");
			break;
		case CSL_I2S_16BIT_8000HZ:
			clk_source = SSPI_CLK_SRC_AUDIOCLK;
			fifo_pack = SSPI_FIFO_DATA_PACK_16BIT;
			clk_div = 5;
   	        Log_DebugPrintf(LOGID_SOC_AUDIO, "sample Rate = CSL_I2S_16BIT_8000HZ \r\n");
			break;
		case CSL_I2S_16BIT_16000HZ:
			clk_source = SSPI_CLK_SRC_AUDIOCLK;
			fifo_pack = SSPI_FIFO_DATA_PACK_16BIT;
			clk_div = 2;
   	        Log_DebugPrintf(LOGID_SOC_AUDIO, "sample Rate = CSL_I2S_16BIT_16000HZ \r\n");
			break;
		case CSL_I2S_16BIT_48000HZ:
			clk_source = SSPI_CLK_SRC_AUDIOCLK;
			fifo_pack = SSPI_FIFO_DATA_PACK_16BIT;
			clk_div = 0;
   	        Log_DebugPrintf(LOGID_SOC_AUDIO, "sample Rate = CSL_I2S_16BIT_48000HZ \r\n");
			break;
		case CSL_I2S_32BIT_8000HZ:
			clk_source = SSPI_CLK_SRC_AUDIOCLK;
			fifo_pack = SSPI_FIFO_DATA_PACK_NONE;
			clk_div = 2;
   	        Log_DebugPrintf(LOGID_SOC_AUDIO, "sample Rate = CSL_I2S_32BIT_8000HZ \r\n");
			break;
		case CSL_I2S_25BIT_48000HZ:
			clk_source = SSPI_CLK_SRC_INTCLK;
			fifo_pack = SSPI_FIFO_DATA_PACK_NONE;
			clk_div = 0;
   	        Log_DebugPrintf(LOGID_SOC_AUDIO, "sample Rate = CSL_I2S_25BIT_48000HZ \r\n");
			break;
	default:
        Log_DebugPrintf(LOGID_SOC_AUDIO, "unknown rate setting \r\n");
		return SSPI_HW_ERR_PROT;
	}
#endif

    memset(&tk_conf, 0, sizeof(tk_conf));
    chal_sspi_soft_reset(handle);

    if(chal_sspi_set_idle_state(handle, mode))
        return SSPI_HW_ERROR;
#if 0
#ifndef FPGA_VERSION
    if(chal_sspi_set_clk_src_select(handle, clk_source))
        return SSPI_HW_ERROR;
#endif
#endif

    chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_RX0, SSPI_FIFO_SIZE_HALF);
    chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_RX1, SSPI_FIFO_SIZE_HALF);
    chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_RX2, SSPI_FIFO_SIZE_NONE);
    chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_RX3, SSPI_FIFO_SIZE_NONE);
    chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_TX0, SSPI_FIFO_SIZE_HALF);
    chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_TX1, SSPI_FIFO_SIZE_HALF);
    chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_TX2, SSPI_FIFO_SIZE_NONE);
    chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_TX3, SSPI_FIFO_SIZE_NONE);

    chal_sspi_set_fifo_pack(handle, SSPI_FIFO_ID_RX0, fifo_pack);
    chal_sspi_set_fifo_pack(handle, SSPI_FIFO_ID_RX1, fifo_pack);
    chal_sspi_set_fifo_pack(handle, SSPI_FIFO_ID_TX0, fifo_pack);
    chal_sspi_set_fifo_pack(handle, SSPI_FIFO_ID_TX1, fifo_pack);

    chal_sspi_set_clk_divider(handle, SSPI_CLK_DIVIDER0, clk_div);
    chal_sspi_set_clk_divider(handle, SSPI_CLK_DIVIDER1, 1);
    chal_sspi_set_clk_divider(handle, SSPI_CLK_DIVIDER2, 15);
    chal_sspi_set_clk_divider(handle, SSPI_CLK_REF_DIVIDER, 4);

    chal_sspi_set_mode(handle, SSPI_MODE_MASTER);
    chal_sspi_enable(handle, 1);

    chal_sspi_set_fifo_threshold(handle,(config->rx_ena) ? SSPI_FIFO_ID_RX0 : SSPI_FIFO_ID_TX0,0x10);
    chal_sspi_set_fifo_threshold(handle,(config->rx_ena) ? SSPI_FIFO_ID_RX1 : SSPI_FIFO_ID_TX1,0x10);

    chal_sspi_set_fifo_pio_threshhold(handle,(config->rx_ena) ? SSPI_FIFO_ID_RX0 : SSPI_FIFO_ID_TX0, 0x3, 0x3);
    chal_sspi_set_fifo_pio_threshhold(handle,(config->rx_ena) ? SSPI_FIFO_ID_RX1 : SSPI_FIFO_ID_TX1, 0x3, 0x3);
    
    tk_conf.chan_sel = SSPI_CHAN_SEL_CHAN0;
    tk_conf.cs_sel = SSPI_CS_SEL_CS0;
    tk_conf.rx_sel = (config->tx_loopback_ena) ? SSPI_RX_SEL_COPY_TX0 : SSPI_RX_SEL_RX0;
    tk_conf.tx_sel = SSPI_TX_SEL_TX0;
    tk_conf.div_sel = SSPI_CLK_DIVIDER0;
    tk_conf.seq_ptr = 0;
    // Max transfer size is set to 4K bytes in non_continuous mode transfer
    if((config->trans_size >> 1) > 0x400) {
        tk_conf.loop_cnt = 0;
        tk_conf.continuous = 1;
    }
    else {
        tk_conf.loop_cnt = (config->trans_size >> 1) - 1;
        tk_conf.continuous = 0;
    }
    tk_conf.init_cond_mask = (config->rx_ena) ? (SSPI_TASK_INIT_COND_THRESHOLD_RX0 |
                                         SSPI_TASK_INIT_COND_THRESHOLD_RX1) :
                                      (SSPI_TASK_INIT_COND_THRESHOLD_TX0 |
                                       SSPI_TASK_INIT_COND_THRESHOLD_TX1);
    tk_conf.wait_before_start = 1;

    if(chal_sspi_set_task(handle, 0, mode, &tk_conf))
        return(SSPI_HW_ERR_TASK);

    seq_conf.tx_enable = (config->tx_ena || config->rx_loopback_ena) ? 1 : 0;
    seq_conf.rx_enable = (config->rx_ena || config->tx_loopback_ena) ? 1 : 0;
    seq_conf.cs_activate = 1;
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
    if(chal_sspi_set_sequence(handle, 0, mode, &seq_conf))
        return(SSPI_HW_ERR_SEQUENCE);

    seq_conf.tx_enable = (config->tx_ena || config->rx_loopback_ena) ? 1 : 0;
    seq_conf.rx_enable = (config->rx_ena || config->tx_loopback_ena) ? 1 : 0;
    seq_conf.cs_activate = 0;
    seq_conf.cs_deactivate = 1;
    seq_conf.pattern_mode = 0;
    seq_conf.rep_cnt = 0;
    seq_conf.opcode = SSPI_SEQ_OPCODE_COND_JUMP;
    seq_conf.rx_fifo_sel = 1;
    seq_conf.tx_fifo_sel = 1;
    seq_conf.frm_sel = 0;
    seq_conf.rx_sidetone_on = 0;
    seq_conf.tx_sidetone_on = 0;
    seq_conf.next_pc = 0;
    if(chal_sspi_set_sequence(handle, 1, mode, &seq_conf))
        return(SSPI_HW_ERR_SEQUENCE);

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
    if(chal_sspi_set_sequence(handle, 2, mode, &seq_conf))
        return(SSPI_HW_ERR_SEQUENCE);

    if(chal_sspi_set_frame(handle, &frmMask, mode, 16, 0))
        return(SSPI_HW_ERR_FRAME);
#endif
    return SSPI_HW_NOERR;
}
/****************************************************************************
*
* NAME:  SPI_hw_interleave_init
*
*
*  Description:  This function initializes the SSPI core as I2S that specified by coreIdx as
*  one channel I2S, master mode.
*  The Left and Right channel are interleaved
*
*  Parameters:
*     SSPI_hw_core_t *pCore - SSPI core pointer
*
*  Returns:  SSPI_hw_status_t
*
*
*
*  Notes:
*
****************************************************************************/
static SSPI_hw_status_t SSPI_hw_interleave_init(CSL_HANDLE handle, CSL_I2S_CONFIG_t *config)
{
#if 0
    cUInt32 frmMask = 1;
    CHAL_SSPI_PROT_t mode;
//	CHAL_SSPI_CLK_SRC_t clk_source;
//	CHAL_SSPI_FIFO_DATA_PACK_t fifo_pack;
//	cUInt32 clk_div;
//    CSL_SSPI_HANDLE_T *pDevice = (CSL_SSPI_HANDLE_T *)handle;

    if(config->prot == SSPI_HW_I2S_MODE1)
        mode = SSPI_PROT_I2S_MODE1;
    else if(config->prot == SSPI_HW_I2S_MODE2)
        mode = SSPI_PROT_I2S_MODE2;
    else
        return SSPI_HW_ERR_PROT;


#if 0
	switch (config->sampleRate) 
	{
		case CSL_I2S_16BIT_4000HZ:
			clk_source = SSPI_CLK_SRC_AUDIOCLK;
			fifo_pack = SSPI_FIFO_DATA_PACK_16BIT;
			clk_div = 11;
   	        Log_DebugPrintf(LOGID_SOC_AUDIO, "sample Rate = CSL_I2S_16BIT_4000HZ \r\n");
			break;
		case CSL_I2S_16BIT_8000HZ:
			clk_source = SSPI_CLK_SRC_AUDIOCLK;
			fifo_pack = SSPI_FIFO_DATA_PACK_16BIT;
			clk_div = 5;
   	        Log_DebugPrintf(LOGID_SOC_AUDIO, "sample Rate = CSL_I2S_16BIT_8000HZ \r\n");
			break;
		case CSL_I2S_16BIT_16000HZ:
			clk_source = SSPI_CLK_SRC_AUDIOCLK;
			fifo_pack = SSPI_FIFO_DATA_PACK_16BIT;
			clk_div = 2;
   	        Log_DebugPrintf(LOGID_SOC_AUDIO, "sample Rate = CSL_I2S_16BIT_16000HZ \r\n");
			break;
		case CSL_I2S_16BIT_48000HZ:
			clk_source = SSPI_CLK_SRC_AUDIOCLK;
			fifo_pack = SSPI_FIFO_DATA_PACK_16BIT;
			clk_div = 0;
   	        Log_DebugPrintf(LOGID_SOC_AUDIO, "sample Rate = CSL_I2S_16BIT_48000HZ \r\n");
			break;
		case CSL_I2S_32BIT_8000HZ:
			clk_source = SSPI_CLK_SRC_AUDIOCLK;
			fifo_pack = SSPI_FIFO_DATA_PACK_NONE;
			clk_div = 2;
   	        Log_DebugPrintf(LOGID_SOC_AUDIO, "sample Rate = CSL_I2S_32BIT_8000HZ \r\n");
			break;
		case CSL_I2S_25BIT_48000HZ:
			clk_source = SSPI_CLK_SRC_INTCLK;
			fifo_pack = SSPI_FIFO_DATA_PACK_NONE;
			clk_div = 0;
   	        Log_DebugPrintf(LOGID_SOC_AUDIO, "sample Rate = CSL_I2S_25BIT_48000HZ \r\n");
			break;
	default:
        Log_DebugPrintf(LOGID_SOC_AUDIO, "unknown rate setting \r\n");
        return SSPI_HW_ERR_PROT;
	}
#endif

    memset(&tk_conf, 0, sizeof(tk_conf));

    chal_sspi_soft_reset(handle);
    if(chal_sspi_set_idle_state(handle, mode))
        return SSPI_HW_ERROR;
#if 0
#ifndef FPGA_VERSION
    if(chal_sspi_set_clk_src_select(handle, clk_source))
//    if(chal_sspi_set_clk_src_select(handle, SSPI_CLK_SRC_AUDIOCLK))
//    if(chal_sspi_set_clk_src_select(handle, SSPI_CLK_SRC_INTCLK))
        return SSPI_HW_ERROR;
#endif
#endif

    chal_sspi_set_clk_divider(handle, SSPI_CLK_DIVIDER0, 0);
	chal_sspi_set_clk_divider(handle, SSPI_CLK_REF_DIVIDER, 0);
	chal_sspi_set_clk_src_select(handle, SSPI_CLK_SRC_CAPHCLK);
	chal_sspi_set_caph_clk(handle, SSPI_CAPH_CLK_TRIG_48KHZ,
        SSPI_HW_WORD_LEN_16Bit, 2);


    chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_RX0, SSPI_FIFO_SIZE_FULL);
    chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_RX1, SSPI_FIFO_SIZE_NONE);
    chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_RX2, SSPI_FIFO_SIZE_NONE);
    chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_RX3, SSPI_FIFO_SIZE_NONE);
    chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_TX0, SSPI_FIFO_SIZE_FULL);
    chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_TX1, SSPI_FIFO_SIZE_NONE);
    chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_TX2, SSPI_FIFO_SIZE_NONE);
    chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_TX3, SSPI_FIFO_SIZE_NONE);

//    chal_sspi_set_fifo_pack(handle, SSPI_FIFO_ID_RX0, fifo_pack);
//    chal_sspi_set_fifo_pack(handle, SSPI_FIFO_ID_TX0, fifo_pack);
    chal_sspi_set_fifo_pack(handle, SSPI_FIFO_ID_RX0, SSPI_FIFO_DATA_PACK_16BIT);
    chal_sspi_set_fifo_pack(handle, SSPI_FIFO_ID_TX0, SSPI_FIFO_DATA_PACK_16BIT);

//    chal_sspi_set_clk_divider(handle, SSPI_CLK_DIVIDER0, 0);
#if 0
    chal_sspi_set_clk_divider(handle, SSPI_CLK_DIVIDER0, clk_div);
    chal_sspi_set_clk_divider(handle, SSPI_CLK_DIVIDER1, 1);
    chal_sspi_set_clk_divider(handle, SSPI_CLK_DIVIDER2, 2);
    chal_sspi_set_clk_divider(handle, SSPI_CLK_REF_DIVIDER, 4);
#endif

    chal_sspi_set_mode(handle, SSPI_MODE_MASTER);
    chal_sspi_enable(handle, 1);

//    chal_sspi_set_fifo_threshold(handle,
//                                 (config->rx_ena) ? SSPI_FIFO_ID_RX0 : SSPI_FIFO_ID_TX0,
//                                 0x10);

    chal_sspi_set_fifo_pio_threshhold(handle,(config->rx_ena) ? SSPI_FIFO_ID_RX0 : SSPI_FIFO_ID_TX0, 0x3, 0x3);
    chal_sspi_set_fifo_pio_threshhold(handle,(config->rx_ena) ? SSPI_FIFO_ID_RX1 : SSPI_FIFO_ID_TX1, 0x3, 0x3);


   if(config->tx_ena) 
        chal_sspi_set_fifo_threshold(handle, SSPI_FIFO_ID_TX0, 0x10);

    tk_conf.chan_sel = SSPI_CHAN_SEL_CHAN0;
    tk_conf.cs_sel = SSPI_CS_SEL_CS0;
    tk_conf.rx_sel = (config->tx_loopback_ena) ? SSPI_RX_SEL_COPY_TX0
                                                : SSPI_RX_SEL_RX0;
    tk_conf.tx_sel = SSPI_TX_SEL_TX0;
    tk_conf.div_sel = SSPI_CLK_DIVIDER0;
    tk_conf.seq_ptr = 0;
    // Max transfer size is set to 4K bytes in non_continuous mode transfer
    if((config->trans_size >> 1) > 0x400) {
        tk_conf.loop_cnt = 0;
        tk_conf.continuous = 1;
    }
    else {
        tk_conf.loop_cnt = (config->trans_size >> 1) - 1;
        tk_conf.continuous = 0;
    }
    tk_conf.init_cond_mask = (config->rx_ena) ? SSPI_TASK_INIT_COND_THRESHOLD_RX0
                                             : SSPI_TASK_INIT_COND_THRESHOLD_TX0;
    tk_conf.wait_before_start = 1;

    if(chal_sspi_set_task(handle, 0, mode, &tk_conf))
        return(SSPI_HW_ERR_TASK);

    switch(config->prot)
    {
    case SSPI_HW_I2S_MODE1:
    case SSPI_HW_I2S_MODE2:
        if(config->prot == SSPI_HW_I2S_MODE1)
            mode = SSPI_PROT_I2S_MODE1;
        else
            mode = SSPI_PROT_I2S_MODE2;

        seq_conf.tx_enable = (config->tx_ena || config->rx_loopback_ena) ? 1 : 0;
        seq_conf.rx_enable = (config->rx_ena || config->tx_loopback_ena) ? 1 : 0;
        seq_conf.cs_activate = 1;
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
        if(chal_sspi_set_sequence(handle, 0, mode, &seq_conf))
            return(SSPI_HW_ERR_SEQUENCE);

        seq_conf.tx_enable = (config->tx_ena || config->rx_loopback_ena) ? 1 : 0;
        seq_conf.rx_enable = (config->rx_ena || config->tx_loopback_ena) ? 1 : 0;
        seq_conf.cs_activate = 0;
        seq_conf.cs_deactivate = 1;
        seq_conf.pattern_mode = 0;
        seq_conf.rep_cnt = 0;
        seq_conf.opcode = SSPI_SEQ_OPCODE_COND_JUMP;
        seq_conf.rx_fifo_sel = 0;
        seq_conf.tx_fifo_sel = 0;
        seq_conf.frm_sel = 0;
        seq_conf.rx_sidetone_on = 0;
        seq_conf.tx_sidetone_on = 0;
        seq_conf.next_pc = 0;
        if(chal_sspi_set_sequence(handle, 1, mode, &seq_conf))
            return(SSPI_HW_ERR_SEQUENCE);

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
        if(chal_sspi_set_sequence(handle, 2, mode, &seq_conf))
            return(SSPI_HW_ERR_SEQUENCE);

        if(chal_sspi_set_frame(handle, &frmMask, mode, 16, 0))
            return(SSPI_HW_ERR_FRAME);

        break;

    default:
        return SSPI_HW_ERR_PROT;
    }
#endif    
    return SSPI_HW_NOERR;
}

/****************************************************************************
*
* NAME:  SSPI_hw_i2s_init
*
*
*  Description:  This function initializes the SSPI core that specified by coreIdx.
*	as two channel I2S, Slave Mode, left and Right seperated
*
*  Parameters:
*     SSPI_hw_core_t *pCore - SSPI core pointer
*
*  Returns:  SSPI_hw_status_t
*
*
*
*  Notes:
*
****************************************************************************/
static SSPI_hw_status_t SSPI_hw_i2s_slave_init(CSL_HANDLE handle, CSL_I2S_CONFIG_t *config)
{
#if 0
    cUInt32 frmMask = 1;
    CHAL_SSPI_PROT_t mode;

    if(config->prot == SSPI_HW_I2S_MODE1)
        mode = SSPI_PROT_I2S_MODE1;
    else if(config->prot == SSPI_HW_I2S_MODE2)
        mode = SSPI_PROT_I2S_MODE2;
    else
        return SSPI_HW_ERR_PROT;

    memset(&tk_conf, 0, sizeof(tk_conf));
    chal_sspi_soft_reset(handle);
    if(chal_sspi_set_idle_state(handle, mode))
        return SSPI_HW_ERROR;


    chal_sspi_soft_reset(handle);
    chal_sspi_set_mode(handle, SSPI_MODE_SLAVE);

    if(chal_sspi_set_idle_state(handle, SSPI_PROT_I2S_MODE2))
        return SSPI_HW_ERROR;

#ifndef FPGA_VERSION
    if(chal_sspi_set_clk_src_select(handle, SSPI_CLK_SRC_AUDIOCLK))
        return SSPI_HW_ERROR;
#endif

    chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_RX0, SSPI_FIFO_SIZE_HALF);
    chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_RX1, SSPI_FIFO_SIZE_HALF);
    chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_RX2, SSPI_FIFO_SIZE_NONE);
    chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_RX3, SSPI_FIFO_SIZE_NONE);
    chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_TX0, SSPI_FIFO_SIZE_HALF);
    chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_TX1, SSPI_FIFO_SIZE_HALF);
    chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_TX2, SSPI_FIFO_SIZE_NONE);
    chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_TX3, SSPI_FIFO_SIZE_NONE);

    chal_sspi_set_fifo_pack(handle, SSPI_FIFO_ID_RX0, SSPI_FIFO_DATA_PACK_16BIT);
    chal_sspi_set_fifo_pack(handle, SSPI_FIFO_ID_RX1, SSPI_FIFO_DATA_PACK_16BIT);
    chal_sspi_set_fifo_pack(handle, SSPI_FIFO_ID_TX0, SSPI_FIFO_DATA_PACK_16BIT);
    chal_sspi_set_fifo_pack(handle, SSPI_FIFO_ID_TX1, SSPI_FIFO_DATA_PACK_16BIT);

    chal_sspi_set_clk_divider(handle, SSPI_CLK_REF_DIVIDER, 8);

    chal_sspi_enable(handle, 1);
//    chal_sspi_clear_intr(handle, SSPI_INTR_STATUS_SCHEDULER);
    if(config->tx_ena) {
        chal_sspi_set_fifo_threshold(handle, SSPI_FIFO_ID_TX0, 0x10);
        chal_sspi_set_fifo_threshold(handle, SSPI_FIFO_ID_TX1, 0x10);
        
        chal_sspi_set_fifo_pio_threshhold(handle, SSPI_FIFO_ID_TX0, 0x3, 0x3);
        chal_sspi_set_fifo_pio_threshhold(handle, SSPI_FIFO_ID_TX1, 0x3, 0x3);
        
    }
    tk_conf.chan_sel = SSPI_CHAN_SEL_CHAN0;
    tk_conf.cs_sel = SSPI_CS_SEL_CS0;
    tk_conf.rx_sel = SSPI_RX_SEL_RX0;
    tk_conf.tx_sel = SSPI_TX_SEL_TX0;
    tk_conf.div_sel = SSPI_CLK_DIVIDER0;
    tk_conf.seq_ptr = 0;
    tk_conf.loop_cnt = 0;
    tk_conf.continuous = 1;
    tk_conf.init_cond_mask = (config->tx_ena) ?
        (SSPI_TASK_INIT_COND_THRESHOLD_TX0 | SSPI_TASK_INIT_COND_THRESHOLD_TX1) : 0;
    if(config->tx_ena)
        tk_conf.wait_before_start = 1;
    else
        tk_conf.wait_before_start = 0;

    if(chal_sspi_set_task(handle, 0, SSPI_PROT_I2S_MODE2, &tk_conf))
        return(SSPI_HW_ERR_TASK);

    // In slave mode the 1st sequence do nothing. It is used to make sure the tx/rx
    // data can be latched correctly.
    seq_conf.tx_enable = FALSE;
    seq_conf.rx_enable = FALSE;
    seq_conf.cs_activate = 0;
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
    if(chal_sspi_set_sequence(handle, 0, SSPI_PROT_I2S_MODE2, &seq_conf))
        return(SSPI_HW_ERR_SEQUENCE);

    seq_conf.tx_enable = (config->tx_ena || config->rx_loopback_ena) ? TRUE : FALSE;
    seq_conf.rx_enable = (config->rx_ena || config->tx_loopback_ena) ? TRUE : FALSE;
    seq_conf.cs_activate = 1;
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
    if(chal_sspi_set_sequence(handle, 1, SSPI_PROT_I2S_MODE2, &seq_conf))
        return(SSPI_HW_ERR_SEQUENCE);

    seq_conf.tx_enable = (config->tx_ena || config->rx_loopback_ena) ? TRUE : FALSE;
    seq_conf.rx_enable = (config->rx_ena || config->tx_loopback_ena) ? TRUE : FALSE;
    seq_conf.cs_activate = 0;
    seq_conf.cs_deactivate = 1;
    seq_conf.pattern_mode = 0;
    seq_conf.rep_cnt = 0;
    seq_conf.opcode = SSPI_SEQ_OPCODE_COND_JUMP;
    seq_conf.rx_fifo_sel = 1;
    seq_conf.tx_fifo_sel = 1;
    seq_conf.frm_sel = 0;
    seq_conf.rx_sidetone_on = 0;
    seq_conf.tx_sidetone_on = 0;
    seq_conf.next_pc = 1;
    if(chal_sspi_set_sequence(handle, 2, SSPI_PROT_I2S_MODE2, &seq_conf))
        return(SSPI_HW_ERR_SEQUENCE);

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
    if(chal_sspi_set_sequence(handle, 3, mode, &seq_conf))
        return(SSPI_HW_ERR_SEQUENCE);

    if(chal_sspi_set_frame(handle, &frmMask, SSPI_PROT_I2S_MODE2, 16, 0))
        return(SSPI_HW_ERR_FRAME);
#endif

    return SSPI_HW_NOERR;
}
/****************************************************************************
*
* NAME:  SPI_hw_interleave_init
*
*
*  Description:  This function initializes the SSPI core as I2S that specified by coreIdx as
*  one channel I2S, slave mode.
*  The Left and Right channel are interleaved
*
*  Parameters:
*     SSPI_hw_core_t *pCore - SSPI core pointer
*
*  Returns:  SSPI_hw_status_t
*
*
*
*  Notes:
*
****************************************************************************/
static SSPI_hw_status_t SSPI_hw_interleave_slave_init(CSL_HANDLE handle, CSL_I2S_CONFIG_t *config)
{
#if 0
    cUInt32 frmMask = 1;
    CHAL_SSPI_PROT_t mode;

    if(config->prot == SSPI_HW_I2S_MODE1)
        mode = SSPI_PROT_I2S_MODE1;
    else if(config->prot == SSPI_HW_I2S_MODE2)
        mode = SSPI_PROT_I2S_MODE2;
    else
        return SSPI_HW_ERR_PROT;

    memset(&tk_conf, 0, sizeof(tk_conf));
    chal_sspi_soft_reset(handle);
    if(chal_sspi_set_idle_state(handle, mode))
        return SSPI_HW_ERROR;


    chal_sspi_soft_reset(handle);
    chal_sspi_set_mode(handle, SSPI_MODE_SLAVE);

    if(chal_sspi_set_idle_state(handle, mode))
        return SSPI_HW_ERROR;

#ifndef FPGA_VERSION
	if(chal_sspi_set_clk_src_select(handle, SSPI_CLK_SRC_AUDIOCLK))
        return SSPI_HW_ERROR;
#endif

    chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_RX0, SSPI_FIFO_SIZE_FULL);
    chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_RX1, SSPI_FIFO_SIZE_NONE);
    chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_RX2, SSPI_FIFO_SIZE_NONE);
    chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_RX3, SSPI_FIFO_SIZE_NONE);
    chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_TX0, SSPI_FIFO_SIZE_FULL);
    chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_TX1, SSPI_FIFO_SIZE_NONE);
    chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_TX2, SSPI_FIFO_SIZE_NONE);
    chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_TX3, SSPI_FIFO_SIZE_NONE);

    chal_sspi_set_fifo_pack(handle, SSPI_FIFO_ID_RX0, SSPI_FIFO_DATA_PACK_16BIT);
    chal_sspi_set_fifo_pack(handle, SSPI_FIFO_ID_TX0, SSPI_FIFO_DATA_PACK_16BIT);

    chal_sspi_set_clk_divider(handle, SSPI_CLK_DIVIDER0, 0);
    chal_sspi_set_clk_divider(handle, SSPI_CLK_DIVIDER1, 1);
    chal_sspi_set_clk_divider(handle, SSPI_CLK_DIVIDER2, 15);
    chal_sspi_set_clk_divider(handle, SSPI_CLK_REF_DIVIDER, 4);

    chal_sspi_enable(handle, 1);
	chal_sspi_clear_intr(handle, SSPIL_INTR_STATUS_SCHEDULER, 0);

    if(config->tx_ena) {
        chal_sspi_set_fifo_threshold(handle, SSPI_FIFO_ID_TX0, 0x10);
        
        chal_sspi_set_fifo_pio_threshhold(handle, SSPI_FIFO_ID_TX0, 0x3, 0x3);
    }
    tk_conf.chan_sel = SSPI_CHAN_SEL_CHAN0;
    tk_conf.cs_sel = SSPI_CS_SEL_CS0;
    tk_conf.rx_sel = SSPI_RX_SEL_RX0;
    tk_conf.tx_sel = SSPI_TX_SEL_TX0;
    tk_conf.div_sel = SSPI_CLK_DIVIDER0;
    tk_conf.seq_ptr = 0;
    tk_conf.loop_cnt = 0;
    tk_conf.continuous = 1;
	tk_conf.init_cond_mask = (config->tx_ena) ?
		SSPI_TASK_INIT_COND_THRESHOLD_TX0 : 0;
	if(config->tx_ena)
		tk_conf.wait_before_start = 1;
	else
		tk_conf.wait_before_start = 0;

    if(chal_sspi_set_task(handle, 0, mode, &tk_conf))
        return(SSPI_HW_ERR_TASK);

    // In slave mode the 1st sequence do nothing. It is used to make sure the tx/rx
    // data can be latched correctly.
    seq_conf.tx_enable = FALSE;
    seq_conf.rx_enable = FALSE;
    seq_conf.cs_activate = 0;
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
    if(chal_sspi_set_sequence(handle, 0, mode, &seq_conf))
        return(SSPI_HW_ERR_SEQUENCE);

	seq_conf.tx_enable = (config->tx_ena || config->rx_loopback_ena) ? TRUE : FALSE;
    seq_conf.rx_enable = (config->rx_ena || config->tx_loopback_ena) ? TRUE : FALSE;
    seq_conf.cs_activate = 1;
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
    if(chal_sspi_set_sequence(handle, 1, mode, &seq_conf))
        return(SSPI_HW_ERR_SEQUENCE);

    seq_conf.tx_enable = (config->tx_ena || config->rx_loopback_ena) ? TRUE : FALSE;
    seq_conf.rx_enable = (config->rx_ena || config->tx_loopback_ena) ? TRUE : FALSE;
    seq_conf.cs_activate = 0;
    seq_conf.cs_deactivate = 1;
    seq_conf.pattern_mode = 0;
    seq_conf.rep_cnt = 0;
    seq_conf.opcode = SSPI_SEQ_OPCODE_COND_JUMP;
    seq_conf.rx_fifo_sel = 1;
    seq_conf.tx_fifo_sel = 1;
    seq_conf.frm_sel = 0;
    seq_conf.rx_sidetone_on = 0;
    seq_conf.tx_sidetone_on = 0;
    seq_conf.next_pc = 1;
    if(chal_sspi_set_sequence(handle, 2, mode, &seq_conf))
        return(SSPI_HW_ERR_SEQUENCE);

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
    if(chal_sspi_set_sequence(handle, 3, mode, &seq_conf))
        return(SSPI_HW_ERR_SEQUENCE);

    if(chal_sspi_set_frame(handle, &frmMask, mode, 16, 0))
        return(SSPI_HW_ERR_FRAME);
#endif

    return SSPI_HW_NOERR;
}
#endif
