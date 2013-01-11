
#include <linux/string.h>
#include <linux/kernel.h>

#include "sspi_helpers.h"

/****************************************************************************
*
* NAME:  SSPI_hw_init
*
*
*  Description:  This function initializes the SSPI core that specified by core Idx.
*
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
SSPI_hw_status_t SSPI_hw_init(SSPI_hw_core_t *pCore)
{
    SSPI_hw_status_t status=SSPI_HW_NOERR;
    
    if (pCore->bIniitialized)
        return SSPI_HW_NOERR;


    pCore->handle = chal_sspi_init((unsigned int)pCore->base);

#if    0
    chal_ccu_unlock_khub_clk_mgr();
    chal_hub_clk_init (MM_IO_BASE_HUB_CLK);
    /* Enable all the AUDIOH clocks, 26M, 156M, 2p4M, 6p5M  */
    regVal = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_APB_CLK_EN_MASK;
    regVal |= KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_APB_HW_SW_GATING_SEL_MASK;
    regVal |= KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_156M_CLK_EN_MASK;
    regVal |= KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_156M_HW_SW_GATING_SEL_MASK;
    regVal |= KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_26M_CLK_EN_MASK;
    regVal |= KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_26M_HW_SW_GATING_SEL_MASK;
    regVal |= KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_2P4M_CLK_EN_MASK;
    regVal |= KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_2P4M_HW_SW_GATING_SEL_MASK;
    regVal |= KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_APB_HYST_VAL_MASK;
    regVal |= KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_156M_HYST_EN_MASK;
    regVal |= KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_156M_HYST_VAL_MASK;
    regVal |= KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_26M_HYST_EN_MASK;
    regVal |= KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_26M_HYST_VAL_MASK;
    regVal |= KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_2P4M_HYST_EN_MASK;
    regVal |= KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_2P4M_HYST_VAL_MASK;
    
    WRITE_REG32((HUB_CLK_BASE_ADDR+KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_OFFSET) ,regVal);

    /* Enable all the CAPH clocks */
    regVal = KHUB_CLK_MGR_REG_CAPH_DIV_CAPH_SRCMIXER_PLL_SELECT_CMD_REF_312M_CLK;
    regVal |= (1<<KHUB_CLK_MGR_REG_CAPH_DIV_CAPH_SRCMIXER_DIV_SHIFT);
    WRITE_REG32((HUB_CLK_BASE_ADDR+KHUB_CLK_MGR_REG_CAPH_DIV_OFFSET),regVal);
    
    regVal = KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_CAPH_SRCMIXER_TRIGGER_MASK;
    WRITE_REG32((HUB_CLK_BASE_ADDR+KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET),regVal);
    
    regVal = KHUB_CLK_MGR_REG_CAPH_CLKGATE_CAPH_SRCMIXER_CLK_EN_MASK;
    regVal |= KHUB_CLK_MGR_REG_CAPH_CLKGATE_CAPH_SRCMIXER_HW_SW_GATING_SEL_MASK;
    regVal |= KHUB_CLK_MGR_REG_CAPH_CLKGATE_CAPH_SRCMIXER_HYST_VAL_MASK;
    
    WRITE_REG32((HUB_CLK_BASE_ADDR+KHUB_CLK_MGR_REG_CAPH_CLKGATE_OFFSET) ,regVal);

    regVal = KHUB_CLK_MGR_REG_APB10_CLKGATE_APB10_CLK_EN_MASK;
    WRITE_REG32((HUB_CLK_BASE_ADDR+KHUB_CLK_MGR_REG_APB10_CLKGATE_OFFSET),regVal);
    chal_ccu_lock_khub_clk_mgr();    

    switch(pCore->core_id)
    {
       case SSPI_CORE_ID_SSP0:

          /* Initialize the clock manager 0x1:52m/26m/26m/26m/26m */
          chal_ccu_set_kps_policy_freq (chal_ccu_policy_0, chal_ccu_kps_policy_freq_52_26);
          chal_ccu_set_kps_policy_freq (chal_ccu_policy_1, chal_ccu_kps_policy_freq_52_26);
          chal_ccu_set_kps_policy_freq (chal_ccu_policy_2, chal_ccu_kps_policy_freq_52_26);
          chal_ccu_set_kps_policy_freq (chal_ccu_policy_3, chal_ccu_kps_policy_freq_52_26);
          clockHandle = chal_ccu_sspi_clk_init (MM_IO_BASE_SLV_CLK);

          /* Initialize the clock */
          chal_ccu_set_sspi0_clock (clockHandle, pCore->bit_rate);
#if CFG_GLOBAL_CHIP != BCM21653 /* BIGISLAND */
          /* Setup interface pins for SSPI0 */
          gpiomux_free (GPIO_81_SSP0_TXD_KP_ROW_OP_7_UARTB2_UCTSN_RFGPO_0);
          gpiomux_free (GPIO_82_SSP0_RXD_KP_ROW_OP_6_UARTB2_URTSN_RFGPO_1);
          gpiomux_free (GPIO_83_SSP0_CLK_KP_ROW_OP_5_UARTB2_UTXD_RFGPO_2);
          gpiomux_free (GPIO_84_SSP0_FS_KP_ROW_OP_4_UARTB2_URXD_RFGPO_3);

          gpiomux_request (GPIO_81_SSP0_TXD_KP_ROW_OP_7_UARTB2_UCTSN_RFGPO_0, chipregHw_PIN_FUNCTION_ALT01, "ssp0_tx");
          gpiomux_request (GPIO_82_SSP0_RXD_KP_ROW_OP_6_UARTB2_URTSN_RFGPO_1, chipregHw_PIN_FUNCTION_ALT01, "ssp0_rx");
          gpiomux_request (GPIO_83_SSP0_CLK_KP_ROW_OP_5_UARTB2_UTXD_RFGPO_2, chipregHw_PIN_FUNCTION_ALT01, "ssp0_clk");
          gpiomux_request (GPIO_84_SSP0_FS_KP_ROW_OP_4_UARTB2_URXD_RFGPO_3, chipregHw_PIN_FUNCTION_ALT01, "ssp0_fs");
#endif
          break;

       case SSPI_CORE_ID_SSP1:
          /* ssp4 clock is used for ssp1 */
          /* Initialize the clock manager 0x1:52m/26m/26m/26m/26m */
          /* Set the frequency policy 0x1:52m/52m/52m/52m/52m (for SSPI4) */
          chal_ccu_set_khub_policy_freq (chal_ccu_policy_0, chal_ccu_khub_policy_freq_52_52_52_52);
          chal_ccu_set_khub_policy_freq (chal_ccu_policy_1, chal_ccu_khub_policy_freq_52_52_52_52);
          chal_ccu_set_khub_policy_freq (chal_ccu_policy_2, chal_ccu_khub_policy_freq_52_52_52_52);
          chal_ccu_set_khub_policy_freq (chal_ccu_policy_3, chal_ccu_khub_policy_freq_52_52_52_52);
          clockHandle = chal_ccu_sspi_clk_init (MM_IO_BASE_HUB_CLK);

          /* Initialize the clock  */
          chal_ccu_set_sspi4_clock (clockHandle, pCore->bit_rate);

#if CFG_GLOBAL_CHIP != BCM21653 /* BIGISLAND */
          /* Setup interface pins for SSPI1  */
          gpiomux_free (GPIO_77_SSP1_TXD_KP_ROW_OP_3_UARTB3_UCTSN_WCDMA_SYNC);
          gpiomux_free (GPIO_78_SSP1_RXD_KP_ROW_OP_2_UARTB3_URTSN_WCDMA_CLK3);
          gpiomux_free (GPIO_79_SSP1_CLK_KP_ROW_OP_1_UARTB3_UTXD_WCDMA_CLK2);
          gpiomux_free (GPIO_80_SSP1_FS_KP_ROW_OP_0_UARTB3_URXD_WCDMA_CLK1);

          gpiomux_request (GPIO_77_SSP1_TXD_KP_ROW_OP_3_UARTB3_UCTSN_WCDMA_SYNC, chipregHw_PIN_FUNCTION_ALT01, "ssp1_tx");
          gpiomux_request (GPIO_78_SSP1_RXD_KP_ROW_OP_2_UARTB3_URTSN_WCDMA_CLK3, chipregHw_PIN_FUNCTION_ALT01, "ssp1_rx");
          gpiomux_request (GPIO_79_SSP1_CLK_KP_ROW_OP_1_UARTB3_UTXD_WCDMA_CLK2, chipregHw_PIN_FUNCTION_ALT01, "ssp1_clk");
          gpiomux_request (GPIO_80_SSP1_FS_KP_ROW_OP_0_UARTB3_URXD_WCDMA_CLK1, chipregHw_PIN_FUNCTION_ALT01, "ssp1_fs");
#endif
          break;

       case SSPI_CORE_ID_SSP2:

          /* Initialize the clock manager 0x1:52m/26m/26m/26m/26m */
          chal_ccu_set_kps_policy_freq (chal_ccu_policy_0, chal_ccu_kps_policy_freq_52_26);
          chal_ccu_set_kps_policy_freq (chal_ccu_policy_1, chal_ccu_kps_policy_freq_52_26);
          chal_ccu_set_kps_policy_freq (chal_ccu_policy_2, chal_ccu_kps_policy_freq_52_26);
          chal_ccu_set_kps_policy_freq (chal_ccu_policy_3, chal_ccu_kps_policy_freq_52_26);
          clockHandle = chal_ccu_sspi_clk_init (MM_IO_BASE_SLV_CLK);

          /* Initialize the clock */
          chal_ccu_set_sspi2_clock (clockHandle, pCore->bit_rate);
#if CFG_GLOBAL_CHIP != BCM21653 /* BIGISLAND */
          /* Setup interface pins for SSPI2 */
          gpiomux_free (GPIO_69_SSP2_FS_2_VC_PWM_0_CAWAKE_WCDMA_DEBUG_7);
          gpiomux_free (GPIO_70_SSP2_TXD_1_IrTx_CAREADY_WCDMA_DEBUG_6);
          gpiomux_free (GPIO_71_SSP2_RXD_1_IrRx_CAFLAG_WCDMA_DEBUG_5);
          gpiomux_free (GPIO_72_SSP2_FS_1_IrRtsSd_CADATA_WCDMA_DEBUG_4);
          gpiomux_free (GPIO_73_SSP2_TXD_0_UARTB4_UCTSN_ACWAKE_WCDMA_DEBUG_3);
          gpiomux_free (GPIO_74_SSP2_RXD_0_UARTB4_URTSN_ACREADY_WCDMA_DEBUG_2);
          gpiomux_free (GPIO_75_SSP2_CLK_UARTB4_UTXD_ACFLAG_WCDMA_DEBUG_1);
          gpiomux_free (GPIO_76_SSP2_FS_0_UARTB4_URXD_ACDATA_WCDMA_DEBUG_0);

          gpiomux_request (GPIO_69_SSP2_FS_2_VC_PWM_0_CAWAKE_WCDMA_DEBUG_7, chipregHw_PIN_FUNCTION_ALT01, "ssp2_fs_2");
          gpiomux_request (GPIO_70_SSP2_TXD_1_IrTx_CAREADY_WCDMA_DEBUG_6, chipregHw_PIN_FUNCTION_ALT01, "ssp2_tx_1");
          gpiomux_request (GPIO_71_SSP2_RXD_1_IrRx_CAFLAG_WCDMA_DEBUG_5, chipregHw_PIN_FUNCTION_ALT01, "ssp2_rx_1");
          gpiomux_request (GPIO_72_SSP2_FS_1_IrRtsSd_CADATA_WCDMA_DEBUG_4, chipregHw_PIN_FUNCTION_ALT01, "ssp2_fs_1");
          gpiomux_request (GPIO_73_SSP2_TXD_0_UARTB4_UCTSN_ACWAKE_WCDMA_DEBUG_3, chipregHw_PIN_FUNCTION_ALT01, "ssp2_tx_0");
          gpiomux_request (GPIO_74_SSP2_RXD_0_UARTB4_URTSN_ACREADY_WCDMA_DEBUG_2, chipregHw_PIN_FUNCTION_ALT01, "ssp2_rx_0");
          gpiomux_request (GPIO_75_SSP2_CLK_UARTB4_UTXD_ACFLAG_WCDMA_DEBUG_1, chipregHw_PIN_FUNCTION_ALT01, "ssp2_clk");
          gpiomux_request (GPIO_76_SSP2_FS_0_UARTB4_URXD_ACDATA_WCDMA_DEBUG_0, chipregHw_PIN_FUNCTION_ALT01, "ssp2_fs_0");
#endif          
          break;

       case SSPI_CORE_ID_SSP3:
          /* Set the frequency policy 0x1:52m/52m/52m/52m/52m (for SSPI3) */
          chal_ccu_set_khub_policy_freq (chal_ccu_policy_0, chal_ccu_khub_policy_freq_52_52_52_52);
          chal_ccu_set_khub_policy_freq (chal_ccu_policy_1, chal_ccu_khub_policy_freq_52_52_52_52);
          chal_ccu_set_khub_policy_freq (chal_ccu_policy_2, chal_ccu_khub_policy_freq_52_52_52_52);
          chal_ccu_set_khub_policy_freq (chal_ccu_policy_3, chal_ccu_khub_policy_freq_52_52_52_52);
          clockHandle = chal_ccu_sspi_clk_init (MM_IO_BASE_HUB_CLK);

          /* Initialize the clock */
          chal_ccu_set_sspi3_clock (clockHandle, pCore->bit_rate);
#if CFG_GLOBAL_CHIP != BCM21653 /* BIGISLAND */
          /* Setup interface pins for SSPI3 */
          gpiomux_free (GPIO_64_SSP3_EXTCLK_VC_TESTDEBUG_CLK_VC_SPI_SCLK);
          gpiomux_free (GPIO_65_SSP3_TXD_VC_I2S_SDO_VC_SPI_MOSI);
          gpiomux_free (GPIO_66_SSP3_RXD_VC_I2S_SDI_VC_SPI_MISO);
          gpiomux_free (GPIO_67_SSP3_CLK_VC_I2S_SCK_VC_SPI_CE0_N);
          gpiomux_free (GPIO_68_SSP3_FS_VC_I2S_WSIO_VC_SPI_CE1_N);

          gpiomux_request (GPIO_64_SSP3_EXTCLK_VC_TESTDEBUG_CLK_VC_SPI_SCLK, chipregHw_PIN_FUNCTION_ALT01, "ssp3_ext_clk");
          gpiomux_request (GPIO_65_SSP3_TXD_VC_I2S_SDO_VC_SPI_MOSI, chipregHw_PIN_FUNCTION_ALT01, "ssp3_tx");
          gpiomux_request (GPIO_66_SSP3_RXD_VC_I2S_SDI_VC_SPI_MISO, chipregHw_PIN_FUNCTION_ALT01, "ssp3_rx");
          gpiomux_request (GPIO_67_SSP3_CLK_VC_I2S_SCK_VC_SPI_CE0_N, chipregHw_PIN_FUNCTION_ALT01, "ssp3_clk");
          gpiomux_request (GPIO_68_SSP3_FS_VC_I2S_WSIO_VC_SPI_CE1_N, chipregHw_PIN_FUNCTION_ALT01, "ssp3_fs");
#endif
          break;

       default:
          return SSPI_HW_ERR_CORE;
    }    
#endif

    pCore->bIniitialized = TRUE;

    return status;
}


SSPI_hw_status_t SSPI_hw_i2c_init(SSPI_hw_core_t *pCore)
{
    uint32_t frmMask = 3;  // Provide 2 frames to configure

    if(!pCore->handle)
        return(SSPI_HW_ERR_HANDLE);

    chal_sspi_soft_reset(pCore->handle);
	chal_sspi_set_mode(pCore->handle, SSPI_MODE_MASTER);
    if(chal_sspi_set_idle_state(pCore->handle, SSPI_PROT_I2C))
        return SSPI_HW_ERROR;

	chal_sspi_set_fifo_size(pCore->handle, SSPI_FIFO_ID_RX0, SSPI_FIFO_SIZE_FULL);
	chal_sspi_set_fifo_size(pCore->handle, SSPI_FIFO_ID_RX1, SSPI_FIFO_SIZE_NONE);
	chal_sspi_set_fifo_size(pCore->handle, SSPI_FIFO_ID_RX2, SSPI_FIFO_SIZE_NONE);
	chal_sspi_set_fifo_size(pCore->handle, SSPI_FIFO_ID_RX3, SSPI_FIFO_SIZE_NONE);
	chal_sspi_set_fifo_size(pCore->handle, SSPI_FIFO_ID_TX0, SSPI_FIFO_SIZE_FULL);
	chal_sspi_set_fifo_size(pCore->handle, SSPI_FIFO_ID_TX1, SSPI_FIFO_SIZE_NONE);
	chal_sspi_set_fifo_size(pCore->handle, SSPI_FIFO_ID_TX2, SSPI_FIFO_SIZE_NONE);	
	chal_sspi_set_fifo_size(pCore->handle, SSPI_FIFO_ID_TX3, SSPI_FIFO_SIZE_NONE);
	
    chal_sspi_set_fifo_pack(pCore->handle, SSPI_FIFO_ID_RX0, SSPI_FIFO_DATA_PACK_8BIT);
    chal_sspi_set_fifo_pack(pCore->handle, SSPI_FIFO_ID_TX0, SSPI_FIFO_DATA_PACK_8BIT);

    	/* 52MHZ % (4 * 26 * 2) = 250 KHZ */
	chal_sspi_set_clk_divider(pCore->handle, SSPI_CLK_DIVIDER0, 3);
	chal_sspi_set_clk_divider(pCore->handle, SSPI_CLK_DIVIDER1, 1);	
	chal_sspi_set_clk_divider(pCore->handle, SSPI_CLK_REF_DIVIDER, 25);	

	chal_sspi_enable(pCore->handle, 1);
	chal_sspi_clear_intr(pCore->handle, SSPIL_INTR_STATUS_SCHEDULER, 0);

	if (chal_sspi_set_i2c_frame(pCore->handle, &frmMask, 8))
		return(SSPI_HW_ERR_FRAME);

	if(chal_sspi_set_pattern(pCore->handle, 0, CHAL_SSPI_PATT_TYPE_I2C_STD_START))
		return(SSPI_HW_ERROR);

	if(chal_sspi_set_pattern(pCore->handle, 1, CHAL_SSPI_PATT_TYPE_I2C_STD_STOP))
		return(SSPI_HW_ERROR);


	return BCM_SUCCESS;
}

SSPI_hw_status_t SSPI_hw_i2c_intr_enable(SSPI_hw_core_t *pCore)
{
    uint32_t intrMask = ( SSPIL_INTR_ENABLE_FIFO_UNDERRUN | SSPIL_INTR_ENABLE_FIFO_OVERRUN | SSPIL_INTR_ENABLE_SCHEDULER | SSPIL_INTR_ENABLE_APB_TX_ERROR | SSPIL_INTR_ENABLE_APB_RX_ERROR);
	chal_sspi_clear_intr(pCore->handle, ~0, ~0);

	chal_sspi_enable_intr(pCore->handle, intrMask);

	return BCM_SUCCESS;

}

#if 0
SSPI_hw_status_t SSPI_hw_i2c_send_start_sequence(SSPI_hw_core_t *pCore)
{
	chal_sspi_task_conf_t tk_conf;
	chal_sspi_seq_conf_t seq_conf;

	if (!pCore->handle)
		return(SSPI_HW_ERR_HANDLE);

	memset(&tk_conf, 0, sizeof(tk_conf));
	tk_conf.cs_sel = SSPI_CS_SEL_CS0;
	tk_conf.chan_sel = SSPI_CHAN_SEL_CHAN0;
    	tk_conf.rx_sel = SSPI_RX_SEL_COPY_TX0;
    	tk_conf.tx_sel = SSPI_TX_SEL_TX0;
	tk_conf.div_sel = SSPI_CLK_DIVIDER0;
	tk_conf.seq_ptr = 0;
	tk_conf.loop_cnt = 0;
	tk_conf.init_cond_mask = 0;
    	if(chal_sspi_set_task(pCore->handle, 0, SSPI_PROT_I2C, &tk_conf))
		return(SSPI_HW_ERR_TASK);

	memset(&seq_conf, 0, sizeof(seq_conf));
	seq_conf.tx_enable = 0;
	seq_conf.rx_enable = 0;
	seq_conf.cs_activate = 0;
	seq_conf.cs_deactivate = 0;
	seq_conf.pattern_mode = 1;
	seq_conf.rep_cnt = 0;
	seq_conf.opcode = SSPI_SEQ_OPCODE_STOP;
	seq_conf.rx_fifo_sel = 0;
	seq_conf.tx_fifo_sel = 0;
	seq_conf.frm_sel = 0;
	seq_conf.rx_sidetone_on = 0;
	seq_conf.tx_sidetone_on = 0;
	seq_conf.next_pc = 0;
	if(chal_sspi_set_sequence(pCore->handle, 0, SSPI_PROT_I2C, &seq_conf))
		return(SSPI_HW_ERR_SEQUENCE);


	chal_sspi_enable_scheduler(pCore->handle, 1);

	return BCM_SUCCESS;
}


SSPI_hw_status_t SSPI_hw_i2c_send_stop_sequence(SSPI_hw_core_t *pCore)
{
	chal_sspi_task_conf_t tk_conf;
	chal_sspi_seq_conf_t seq_conf;

	if (!pCore->handle)
		return(SSPI_HW_ERR_HANDLE);

	memset(&tk_conf, 0, sizeof(tk_conf));
	tk_conf.cs_sel = SSPI_CS_SEL_CS0;
	tk_conf.chan_sel = SSPI_CHAN_SEL_CHAN0;
    	tk_conf.rx_sel = SSPI_RX_SEL_COPY_TX0;
    	tk_conf.tx_sel = SSPI_TX_SEL_TX0;
	tk_conf.div_sel = SSPI_CLK_DIVIDER0;
	tk_conf.seq_ptr = 0;
	tk_conf.loop_cnt = 0;
	tk_conf.init_cond_mask = 0;
    	if(chal_sspi_set_task(pCore->handle, 0, SSPI_PROT_I2C, &tk_conf))
		return(SSPI_HW_ERR_TASK);

	memset(&seq_conf, 0, sizeof(seq_conf));
	seq_conf.tx_enable = 0;
	seq_conf.rx_enable = 0;
	seq_conf.cs_activate = 0;
	seq_conf.cs_deactivate = 1;
	seq_conf.pattern_mode = 1;
	seq_conf.rep_cnt = 0;
	seq_conf.opcode = SSPI_SEQ_OPCODE_STOP;
	seq_conf.rx_fifo_sel = 0;
	seq_conf.tx_fifo_sel = 0;
	seq_conf.frm_sel = 1;
	seq_conf.rx_sidetone_on = 0;
	seq_conf.tx_sidetone_on = 0;
	seq_conf.next_pc = 0;
	if(chal_sspi_set_sequence(pCore->handle, 0, SSPI_PROT_I2C, &seq_conf))
		return(SSPI_HW_ERR_SEQUENCE);
	
	chal_sspi_enable_scheduler(pCore->handle, 1);
	
	return BCM_SUCCESS;
}
#endif

SSPI_hw_status_t SSPI_hw_i2c_do_transaction(SSPI_hw_core_t *pCore, 
					 unsigned char *rx_buf, unsigned int rx_len, 
					 unsigned char *tx_buf, unsigned int tx_len,
					 unsigned int buf_len, SSPI_hw_i2c_transaction_t tran_type)
{
	chal_sspi_task_conf_t tk_conf;
	chal_sspi_seq_conf_t seq_conf;


	if (!pCore->handle)
		return(SSPI_HW_ERR_HANDLE);

	memset(&tk_conf, 0, sizeof(tk_conf));
	tk_conf.cs_sel = SSPI_CS_SEL_CS0;
	tk_conf.chan_sel = SSPI_CHAN_SEL_CHAN0;
	tk_conf.rx_sel = SSPI_RX_SEL_COPY_TX0;
	tk_conf.tx_sel = SSPI_TX_SEL_TX0;
	tk_conf.div_sel = SSPI_CLK_DIVIDER0;
	tk_conf.seq_ptr = 0;
	tk_conf.loop_cnt = 0;
	tk_conf.init_cond_mask = 0;
	if(chal_sspi_set_task(pCore->handle, 0, SSPI_PROT_I2C, &tk_conf))
		return(SSPI_HW_ERR_TASK);

	if(tran_type == I2C_WRRD_TRANSACTION) {
		memset(&seq_conf, 0, sizeof(seq_conf));
		seq_conf.tx_enable = 0;
		seq_conf.rx_enable = 0;
		seq_conf.cs_activate = 0;
		seq_conf.cs_deactivate = 0;
		seq_conf.pattern_mode = 1;
		seq_conf.rep_cnt = 0;
		seq_conf.opcode = SSPI_SEQ_OPCODE_NEXT_PC;
		seq_conf.rx_fifo_sel = 0;
		seq_conf.tx_fifo_sel = 0;
		seq_conf.frm_sel = 0;
		seq_conf.rx_sidetone_on = 0;
		seq_conf.tx_sidetone_on = 0;
		seq_conf.next_pc = 0;
		if(chal_sspi_set_sequence(pCore->handle, 0, SSPI_PROT_I2C, &seq_conf))
			return(SSPI_HW_ERR_SEQUENCE);

		seq_conf.tx_enable = 1;
		seq_conf.rx_enable = 1;
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
		if(chal_sspi_set_sequence(pCore->handle, 1, SSPI_PROT_I2C, &seq_conf))
			return(SSPI_HW_ERR_SEQUENCE);

		seq_conf.tx_enable = 1;
		seq_conf.rx_enable = 1;
		seq_conf.cs_activate = 1;
		seq_conf.cs_deactivate = 0;
		seq_conf.pattern_mode = 0;
		seq_conf.rep_cnt = tx_len - 1;
		seq_conf.opcode = SSPI_SEQ_OPCODE_NEXT_PC;
		seq_conf.rx_fifo_sel = 0;
		seq_conf.tx_fifo_sel = 0;
		seq_conf.frm_sel = 0;
		seq_conf.rx_sidetone_on = 0;
		seq_conf.tx_sidetone_on = 0;
		seq_conf.next_pc = 0;
		if(chal_sspi_set_sequence(pCore->handle, 2, SSPI_PROT_I2C, &seq_conf))
			return(SSPI_HW_ERR_SEQUENCE);

		seq_conf.tx_enable = 0;
		seq_conf.rx_enable = 0;
		seq_conf.cs_activate = 0;
		seq_conf.cs_deactivate = 0;
		seq_conf.pattern_mode = 1;
		seq_conf.rep_cnt = 0;
		seq_conf.opcode = SSPI_SEQ_OPCODE_NEXT_PC;
		seq_conf.rx_fifo_sel = 0;
		seq_conf.tx_fifo_sel = 0;
		seq_conf.frm_sel = 0;
		seq_conf.rx_sidetone_on = 0;
		seq_conf.tx_sidetone_on = 0;
		seq_conf.next_pc = 0;
		if(chal_sspi_set_sequence(pCore->handle, 3, SSPI_PROT_I2C, &seq_conf))
			return(SSPI_HW_ERR_SEQUENCE);

		seq_conf.tx_enable = 1;
		seq_conf.rx_enable = 1;
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
		if(chal_sspi_set_sequence(pCore->handle, 4, SSPI_PROT_I2C, &seq_conf))
			return(SSPI_HW_ERR_SEQUENCE);

		seq_conf.tx_enable = 1;
		seq_conf.rx_enable = 1;
		seq_conf.cs_activate = 1;
		seq_conf.cs_deactivate = 0;
		seq_conf.pattern_mode = 0;
		seq_conf.rep_cnt = rx_len - 1;
		seq_conf.opcode = SSPI_SEQ_OPCODE_NEXT_PC;
		seq_conf.rx_fifo_sel = 0;
		seq_conf.tx_fifo_sel = 0;
		seq_conf.frm_sel = 1;
		seq_conf.rx_sidetone_on = 0;
		seq_conf.tx_sidetone_on = 0;
		seq_conf.next_pc = 0;
		if(chal_sspi_set_sequence(pCore->handle, 5, SSPI_PROT_I2C, &seq_conf))
			return(SSPI_HW_ERR_SEQUENCE);

		seq_conf.tx_enable = 0;
		seq_conf.rx_enable = 0;
		seq_conf.cs_activate = 0;
		seq_conf.cs_deactivate = 1;
		seq_conf.pattern_mode = 1;
		seq_conf.rep_cnt = 0;
		seq_conf.opcode = SSPI_SEQ_OPCODE_STOP;
		seq_conf.rx_fifo_sel = 0;
		seq_conf.tx_fifo_sel = 0;
		seq_conf.frm_sel = 1;
		seq_conf.rx_sidetone_on = 0;
		seq_conf.tx_sidetone_on = 0;
		seq_conf.next_pc = 0;
		if(chal_sspi_set_sequence(pCore->handle, 6, SSPI_PROT_I2C, &seq_conf))
			return(SSPI_HW_ERR_SEQUENCE);
	}
	else if(tran_type == I2C_RDONLY_TRANSACTION) {
		memset(&seq_conf, 0, sizeof(seq_conf));
		seq_conf.tx_enable = 0;
		seq_conf.rx_enable = 0;
		seq_conf.cs_activate = 0;
		seq_conf.cs_deactivate = 0;
		seq_conf.pattern_mode = 1;
		seq_conf.rep_cnt = 0;
		seq_conf.opcode = SSPI_SEQ_OPCODE_NEXT_PC;
		seq_conf.rx_fifo_sel = 0;
		seq_conf.tx_fifo_sel = 0;
		seq_conf.frm_sel = 0;
		seq_conf.rx_sidetone_on = 0;
		seq_conf.tx_sidetone_on = 0;
		seq_conf.next_pc = 0;
		if(chal_sspi_set_sequence(pCore->handle, 0, SSPI_PROT_I2C, &seq_conf))
			return(SSPI_HW_ERR_SEQUENCE);

		seq_conf.tx_enable = 1;
		seq_conf.rx_enable = 1;
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
		if(chal_sspi_set_sequence(pCore->handle, 1, SSPI_PROT_I2C, &seq_conf))
			return(SSPI_HW_ERR_SEQUENCE);

		seq_conf.tx_enable = 1;
		seq_conf.rx_enable = 1;
		seq_conf.cs_activate = 1;
		seq_conf.cs_deactivate = 0;
		seq_conf.pattern_mode = 0;
		seq_conf.rep_cnt = rx_len - 1;
		seq_conf.opcode = SSPI_SEQ_OPCODE_NEXT_PC;
		seq_conf.rx_fifo_sel = 0;
		seq_conf.tx_fifo_sel = 0;
		seq_conf.frm_sel = 1;
		seq_conf.rx_sidetone_on = 0;
		seq_conf.tx_sidetone_on = 0;
		seq_conf.next_pc = 0;
		if(chal_sspi_set_sequence(pCore->handle, 2, SSPI_PROT_I2C, &seq_conf))
			return(SSPI_HW_ERR_SEQUENCE);

		seq_conf.tx_enable = 0;
		seq_conf.rx_enable = 0;
		seq_conf.cs_activate = 0;
		seq_conf.cs_deactivate = 1;
		seq_conf.pattern_mode = 1;
		seq_conf.rep_cnt = 0;
		seq_conf.opcode = SSPI_SEQ_OPCODE_STOP;
		seq_conf.rx_fifo_sel = 0;
		seq_conf.tx_fifo_sel = 0;
		seq_conf.frm_sel = 1;
		seq_conf.rx_sidetone_on = 0;
		seq_conf.tx_sidetone_on = 0;
		seq_conf.next_pc = 0;
		if(chal_sspi_set_sequence(pCore->handle, 3, SSPI_PROT_I2C, &seq_conf))
			return(SSPI_HW_ERR_SEQUENCE);
	}
	else {
		memset(&seq_conf, 0, sizeof(seq_conf));
		seq_conf.tx_enable = 0;
		seq_conf.rx_enable = 0;
		seq_conf.cs_activate = 0;
		seq_conf.cs_deactivate = 0;
		seq_conf.pattern_mode = 1;
		seq_conf.rep_cnt = 0;
		seq_conf.opcode = SSPI_SEQ_OPCODE_NEXT_PC;
		seq_conf.rx_fifo_sel = 0;
		seq_conf.tx_fifo_sel = 0;
		seq_conf.frm_sel = 0;
		seq_conf.rx_sidetone_on = 0;
		seq_conf.tx_sidetone_on = 0;
		seq_conf.next_pc = 0;
		if(chal_sspi_set_sequence(pCore->handle, 0, SSPI_PROT_I2C, &seq_conf))
			return(SSPI_HW_ERR_SEQUENCE);


		memset(&seq_conf, 0, sizeof(seq_conf));
		seq_conf.tx_enable = 1;
		seq_conf.rx_enable = 1;
		seq_conf.cs_activate = 1;
		seq_conf.cs_deactivate = 0;
		seq_conf.pattern_mode = 0;
		seq_conf.rep_cnt = tx_len; // tx_len also includes the byte for Slave Addr
		seq_conf.opcode = SSPI_SEQ_OPCODE_NEXT_PC;
		seq_conf.rx_fifo_sel = 0;
		seq_conf.tx_fifo_sel = 0;
		seq_conf.frm_sel = 0;
		seq_conf.rx_sidetone_on = 0;
		seq_conf.tx_sidetone_on = 0;
		seq_conf.next_pc = 0;
		if(chal_sspi_set_sequence(pCore->handle, 1, SSPI_PROT_I2C, &seq_conf))
			return(SSPI_HW_ERR_SEQUENCE);

		memset(&seq_conf, 0, sizeof(seq_conf));
		seq_conf.tx_enable = 0;
		seq_conf.rx_enable = 0;
		seq_conf.cs_activate = 0;
		seq_conf.cs_deactivate = 1;
		seq_conf.pattern_mode = 1;
		seq_conf.rep_cnt = 0;
		seq_conf.opcode = SSPI_SEQ_OPCODE_STOP;
		seq_conf.rx_fifo_sel = 0;
		seq_conf.tx_fifo_sel = 0;
		seq_conf.frm_sel = 1;
		seq_conf.rx_sidetone_on = 0;
		seq_conf.tx_sidetone_on = 0;
		seq_conf.next_pc = 0;
		if(chal_sspi_set_sequence(pCore->handle, 2, SSPI_PROT_I2C, &seq_conf))
			return(SSPI_HW_ERR_SEQUENCE);
	}

	// tx_buf and rx_buf actually have the same length
	chal_sspi_write_data(pCore->handle, SSPI_FIFO_ID_TX0, SSPI_PROT_I2C, tx_buf, buf_len);
	chal_sspi_enable_scheduler(pCore->handle, 1);
	chal_sspi_read_data(pCore->handle, SSPI_FIFO_ID_RX0, SSPI_PROT_I2C, rx_buf, buf_len);
	return 0;
}


void SSPI_hw_i2c_prepare_for_xfr(SSPI_hw_core_t *pCore)
{
 	chal_sspi_fifo_reset(pCore->handle, SSPI_FIFO_ID_RX0);
	chal_sspi_fifo_reset(pCore->handle, SSPI_FIFO_ID_TX0);
}


#if 0
SSPI_hw_status_t SSPI_hw_i2c_read_byte_and_write_ack(SSPI_hw_core_t *pCore, unsigned char *byte, int no_ack)
{
	chal_sspi_task_conf_t tk_conf;
	chal_sspi_seq_conf_t seq_conf;
	unsigned char pData[4];

	if (!pCore->handle)
		return(SSPI_HW_ERR_HANDLE);

	memset(&tk_conf, 0, sizeof(tk_conf));
	tk_conf.cs_sel = SSPI_CS_SEL_CS0;
	tk_conf.chan_sel = SSPI_CHAN_SEL_CHAN0;
    	tk_conf.rx_sel = SSPI_RX_SEL_COPY_TX0;
    	tk_conf.tx_sel = SSPI_TX_SEL_TX0;
	tk_conf.div_sel = SSPI_CLK_DIVIDER0;
	tk_conf.seq_ptr = 0;
	tk_conf.loop_cnt = 0;
	tk_conf.init_cond_mask = 0;
    	if(chal_sspi_set_task(pCore->handle, 0, SSPI_PROT_I2C, &tk_conf))
		return(SSPI_HW_ERR_TASK);

	memset(&seq_conf, 0, sizeof(seq_conf));
	seq_conf.tx_enable = 1;
	seq_conf.rx_enable = 1;
	seq_conf.cs_activate = 1;
	seq_conf.cs_deactivate = 0;
	seq_conf.pattern_mode = 0;
	seq_conf.rep_cnt = 0;
	seq_conf.opcode = SSPI_SEQ_OPCODE_STOP;
	seq_conf.rx_fifo_sel = 0;
	seq_conf.tx_fifo_sel = 0;
	seq_conf.frm_sel = 1;
	seq_conf.rx_sidetone_on = 0;
	seq_conf.tx_sidetone_on = 0;
	seq_conf.next_pc = 0;
	if(chal_sspi_set_sequence(pCore->handle, 0, SSPI_PROT_I2C, &seq_conf))
		return(SSPI_HW_ERR_SEQUENCE);

	pData[0] = 0xff;  
	pData[1] = pData[2] = pData[3] = 0;
	chal_sspi_write_data(pCore->handle, SSPI_FIFO_ID_TX0, SSPI_PROT_I2C, byte, 1);

	chal_sspi_enable_scheduler(pCore->handle, 1);

	chal_sspi_read_data(pCore->handle, SSPI_FIFO_ID_RX0, SSPI_PROT_I2C, pData, 1);

	return BCM_SUCCESS;
}
#endif


void SSPI_hw_i2c_read_and_ack_intr(SSPI_hw_core_t *pCore, uint32_t *status, uint32_t *det_status)
{
	(void)chal_sspi_get_intr_status(pCore->handle, status, det_status);
	(void)chal_sspi_clear_intr(pCore->handle, *status, *det_status);
}


