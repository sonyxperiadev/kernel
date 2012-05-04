/****************************************************************************/
/*     Copyright 2009, 2010 Broadcom Corporation.  All rights reserved.     */
/*     Unless you and Broadcom execute a separate written software license  */
/*	   agreement governing                                              */
/*     use of this software, this software is licensed to you under the     */
/*	   terms of the GNU General Public License version 2 (the GPL),     */
/*     available at                                                         */
/*                                                                          */
/*          http://www.broadcom.com/licenses/GPLv2.php                      */
/*                                                                          */
/*     with the following added to such license:                            */
/*                                                                          */
/*     As a special exception, the copyright holders of this software give  */
/*     you permission to link this software with independent modules, and   */
/*     to copy and distribute the resulting executable under terms of your  */
/*     choice, provided that you also meet, for each linked independent     */
/*     module, the terms and conditions of the license of that module.      */
/*     An independent module is a module which is not derived from this     */
/*     software.  The special exception does not apply to any modifications */
/*     of the software.                                                     */
/*                                                                          */
/*     Notwithstanding the above, under no circumstances may you combine    */
/*     this software in any way with any other Broadcom software provided   */
/*     under a license other than the GPL, without Broadcom's express prior */
/*     written consent.                                                     */
/*                                                                          */
/****************************************************************************/

/**
*
*  @file   csl_caph_i2s_sspi.c
*  @brief  CSL layer to using SSPI cHAL interface to define I2S interface
*  @note
*
*****************************************************************************/
/*
 * include header file declarations
 */
#include <string.h>
#include "mobcom_types.h"
#include <plat/chal/chal_types.h>
#include <plat/chal/chal_sspi.h>
#include "csl_caph_i2s_sspi.h"
#include <mach/rdb/brcm_rdb_sspil.h>
#include <mach/rdb/brcm_rdb_padctrlreg.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include "audio_trace.h"
/*
 * global variable declarations
 */

/*
 * extern variable declarations
 */
/*
 * static function prototype declarations
 */


/*
 * local define declarations
 */
#define SSPI_HW_WORD_LEN_32Bit					32
#define SSPI_HW_WORD_LEN_25Bit					25
#define SSPI_HW_WORD_LEN_24Bit					24
#define SSPI_HW_WORD_LEN_16Bit					16
#define SSPI_HW_WORD_LEN_8Bit					8

static chal_sspi_task_conf_t tk_conf;
static chal_sspi_seq_conf_t seq_conf;
static SSPI_hw_status_t SSPI_hw_i2s_init(CSL_HANDLE handle,
		CSL_I2S_CONFIG_t *config);
static SSPI_hw_status_t SSPI_hw_interleave_init(CSL_HANDLE handle,
		CSL_I2S_CONFIG_t *config);
static SSPI_hw_status_t SSPI_hw_i2s_slave_init(CSL_HANDLE handle,
		CSL_I2S_CONFIG_t *config);
static SSPI_hw_status_t SSPI_hw_interleave_slave_init(CSL_HANDLE handle,
		CSL_I2S_CONFIG_t *config);

/*
 *
 *  Function Name: csl_i2s_init(cUInt32 baseAddr)
 *
 *  Description: Initialize I2S
 *
 */
CSL_HANDLE csl_i2s_init(cUInt32 baseAddr)
{
	CSL_HANDLE handle = 0;
	CSL_SSPI_HANDLE_T *pDevice;
	aTrace(LOG_AUDIO_CSL, "+csl_i2s_init\n");

	handle = chal_sspi_init(baseAddr);
	pDevice = (CSL_SSPI_HANDLE_T *) handle;
	if (pDevice)
		aTrace(LOG_AUDIO_CSL,
				"base address in csl 0x%x \r\n",
				(unsigned int)pDevice->base);
	aTrace(LOG_AUDIO_CSL, "-csl_i2s_init \r\n");

	return handle;

}

/*
 *
 *  Function Name: csl_i2s_deinit(cUInt32 baseAddr)
 *
 *  Description: De-Initialize I2S
 *
 */
void csl_i2s_deinit(CSL_HANDLE handle)
{

	chal_sspi_deinit(handle);
}

/*
 *
 *  Function Name: csl_i2s_config(CSL_HANDLE handle,
 CSL_I2S_CONFIG_t *config,
 *			 CSL_I2S_CONFIG_TX_t *configTx,
 *			 CSL_I2S_CONFIG_RX_t *configRx)
 *
 *  Description: Funtion to config SSPI as I2S
 *
 */
void csl_i2s_config(CSL_HANDLE handle, CSL_I2S_CONFIG_t *config)
{
	CSL_SSPI_HANDLE_T *pDevice = NULL;
	SSPI_hw_status_t status = SSPI_HW_NOERR;

	aTrace
			(LOG_AUDIO_CSL,
			 "csl_i2s_config:: handle %p mode %ld txEn %ld"
			 "rxEn %ld txLbEn %ld"
			 "rxLbEn %ld trans_size %ld.\r\n",
			 handle, config->mode, config->tx_ena, config->rx_ena,
			 config->tx_loopback_ena, config->rx_loopback_ena,
			 config->trans_size);
	aTrace
			(LOG_AUDIO_CSL,
			 "csl_i2s_config:: prot %ld interleave %ld sr %d.\r\n",
			 config->prot, config->interleave, config->sampleRate);

	if (config->mode == CSL_I2S_MASTER_MODE) {
		pDevice = (CSL_SSPI_HANDLE_T *) handle;
		/*aTrace(LOG_AUDIO_CSL, "Master Mode base
		 * address 0x%x \r\n", pDevice->base);
		 */
		/*aTrace(LOG_AUDIO_CSL, "handle 0x%x \r\n",
		 * handle);
		 */

		if (!config->interleave)
			status = SSPI_hw_i2s_init(handle, config);
		else
			status = SSPI_hw_interleave_init(handle, config);

		if (status) {
			aTrace(LOG_AUDIO_CSL,
					"SSPI_hw_i2s_init failed \r\n");
		}

	} else {
		if (!config->interleave)
			status = SSPI_hw_i2s_slave_init(handle, config);
		else
			status = SSPI_hw_interleave_slave_init(handle, config);

		if (pDevice != NULL) {
			/* aTrace(LOG_AUDIO_CSL, "Slave
			 * Mode base address 0x%x \r\n", pDevice->base);
			 */
		}
	}
	aTrace(LOG_AUDIO_CSL, "-csl_i2s_config \r\n");
	return;

}

/*
 *
 *  Function Name: csl_i2s_start_rx(CSL_HANDLE handle, CSL_I2S_CONFIG_t *config)
 *
 *  Description: I2S/SSPI start
 *
 */
void csl_i2s_start_rx(CSL_HANDLE handle, CSL_I2S_CONFIG_t *config)
{
	aTrace(LOG_AUDIO_CSL, "+csl_i2s_start \r\n");
	aTrace(LOG_AUDIO_CSL, "config->trans_size 0x%x \r\n",
			(unsigned int)config->trans_size);
	aTrace(LOG_AUDIO_CSL, "handle 0x%x \r\n",
			(unsigned int)handle);
	aTrace(LOG_AUDIO_CSL, "config 0x%x \r\n",
			(unsigned int)config);

	chal_sspi_enable_fifo_pio_start_stop_intr(handle,
			SSPI_FIFO_ID_RX0, TRUE, TRUE);
	chal_sspi_enable_fifo_pio_start_stop_intr(handle,
			SSPI_FIFO_ID_RX1, TRUE, TRUE);
	chal_sspi_enable_fifo_pio_start_stop_intr(handle,
			SSPI_FIFO_ID_RX2, TRUE, TRUE);
	chal_sspi_enable_fifo_pio_start_stop_intr(handle,
			SSPI_FIFO_ID_RX3, TRUE, TRUE);
	return;
}

/*
 *
 *  Function Name: csl_i2s_start_tx(CSL_HANDLE handle, CSL_I2S_CONFIG_t *config)
 *
 *  Description: I2S/SSPI start
 *
 */
void csl_i2s_start_tx(CSL_HANDLE handle, CSL_I2S_CONFIG_t *config)
{
	aTrace(LOG_AUDIO_CSL, "+csl_i2s_start_tx \r\n");
	aTrace(LOG_AUDIO_CSL, "config->trans_size 0x%x \r\n",
			(unsigned int)config->trans_size);
	aTrace(LOG_AUDIO_CSL, "handle 0x%x \r\n",
			(unsigned int)handle);
	aTrace(LOG_AUDIO_CSL, "config 0x%x \r\n",
			(unsigned int)config);

	chal_sspi_enable_fifo_pio_start_stop_intr(handle,
			SSPI_FIFO_ID_TX0, TRUE, TRUE);
	chal_sspi_enable_fifo_pio_start_stop_intr(handle,
			SSPI_FIFO_ID_TX1, TRUE, TRUE);
	chal_sspi_enable_fifo_pio_start_stop_intr(handle,
			SSPI_FIFO_ID_TX2, TRUE, TRUE);
	chal_sspi_enable_fifo_pio_start_stop_intr(handle,
			SSPI_FIFO_ID_TX3, TRUE, TRUE);
	return;
}

/*
 *
 *  Function Name: csl_i2s_stop_tx(CSL_HANDLE handle)
 *
 *  Description: I2S/SSPI TX stop
 *
 */
void csl_i2s_stop_tx(CSL_HANDLE handle)
{
	aTrace(LOG_AUDIO_CSL, "%s\n", __func__);
	chal_sspi_fifo_reset(handle, SSPI_FIFO_ID_TX0);
	chal_sspi_fifo_reset(handle, SSPI_FIFO_ID_TX1);
	chal_sspi_fifo_reset(handle, SSPI_FIFO_ID_TX0);
	chal_sspi_fifo_reset(handle, SSPI_FIFO_ID_TX1);

	chal_sspi_enable_fifo_pio_start_stop_intr(handle,
			SSPI_FIFO_ID_TX0,
			FALSE, FALSE);
	chal_sspi_enable_fifo_pio_start_stop_intr(handle,
			SSPI_FIFO_ID_TX1,
			FALSE, FALSE);
	chal_sspi_enable_fifo_pio_start_stop_intr(handle,
			SSPI_FIFO_ID_TX2,
			FALSE, FALSE);
	chal_sspi_enable_fifo_pio_start_stop_intr(handle,
			SSPI_FIFO_ID_TX3,
			FALSE, FALSE);
	return;
}

/*
 *
 *  Function Name: csl_i2s_stop_rx(CSL_HANDLE handle)
 *
 *  Description: I2S/SSPI RX stop
 *
 */
void csl_i2s_stop_rx(CSL_HANDLE handle)
{
	aTrace(LOG_AUDIO_CSL, "%s\n", __func__);
	chal_sspi_fifo_reset(handle, SSPI_FIFO_ID_RX0);
	chal_sspi_fifo_reset(handle, SSPI_FIFO_ID_RX1);
	chal_sspi_fifo_reset(handle, SSPI_FIFO_ID_RX0);
	chal_sspi_fifo_reset(handle, SSPI_FIFO_ID_RX1);

	chal_sspi_enable_fifo_pio_start_stop_intr(handle,
			SSPI_FIFO_ID_RX0,
			FALSE, FALSE);
	chal_sspi_enable_fifo_pio_start_stop_intr(handle,
			SSPI_FIFO_ID_RX1,
			FALSE, FALSE);
	chal_sspi_enable_fifo_pio_start_stop_intr(handle,
			SSPI_FIFO_ID_RX2,
			FALSE, FALSE);
	chal_sspi_enable_fifo_pio_start_stop_intr(handle,
			SSPI_FIFO_ID_RX3,
			FALSE, FALSE);
	return;
}

/*
 *
 *  Function Name: csl_sspi_enable_scheduler( CSL_HANDLE handle,
 *                                            UInt32 on_off)
 *
 *  Description: Enable/Disable the SSPI scheduler to excute the
 *  configured tasks
 *
 */
void csl_sspi_enable_scheduler(CSL_HANDLE handle, UInt32 on_off)
{
	/*enable/disable  master */
	chal_sspi_enable_scheduler(handle, on_off);
}

UInt32 csl_i2s_get_tx0_fifo_data_port(CSL_HANDLE handle)
{
	CSL_SSPI_HANDLE_T *pDevice = (CSL_SSPI_HANDLE_T *) handle;
	return (UInt32) (pDevice->base + SSPIL_FIFO_ENTRY0TX_OFFSET);
}

UInt32 csl_i2s_get_rx0_fifo_data_port(CSL_HANDLE handle)
{
	CSL_SSPI_HANDLE_T *pDevice = (CSL_SSPI_HANDLE_T *) handle;
	return (UInt32) (pDevice->base + SSPIL_FIFO_ENTRY0RX_OFFSET);
}

UInt32 csl_i2s_get_tx1_fifo_data_port(CSL_HANDLE handle)
{
	CSL_SSPI_HANDLE_T *pDevice = (CSL_SSPI_HANDLE_T *) handle;
	return (UInt32) (pDevice->base + SSPIL_FIFO_ENTRY1TX_OFFSET);
}

UInt32 csl_i2s_get_rx1_fifo_data_port(CSL_HANDLE handle)
{
	CSL_SSPI_HANDLE_T *pDevice = (CSL_SSPI_HANDLE_T *) handle;
	return (UInt32) (pDevice->base + SSPIL_FIFO_ENTRY1RX_OFFSET);
}

/*
 *
 * NAME:  SSPI_hw_i2s_init
 *
 *
 *  Description:  This function initializes the SSPI core that
 *  specified by coreIdx.as two channel I2S, Master Mode,
 *  left and Right seperated
 *
 *  Parameters:
 *     SSPI_hw_core_t *pCore - SSPI core pointer
 *
 *  Returns:  SSPI_hw_status_t
 *
 *  Notes:
 *
 */
static SSPI_hw_status_t SSPI_hw_i2s_init(CSL_HANDLE handle,
		CSL_I2S_CONFIG_t *config)
{
	uint32_t frmMask = 1, word_len = 16;
	CHAL_SSPI_PROT_t mode;
	/*      CHAL_SSPI_CLK_SRC_t clk_source; */
	CHAL_SSPI_FIFO_DATA_PACK_t fifo_pack = SSPI_FIFO_DATA_PACK_16BIT;
	cUInt32 clk_div = 0;
	/*    CSL_SSPI_HANDLE_T *pDevice = (CSL_SSPI_HANDLE_T *)handle; */

	if (config->prot == SSPI_HW_I2S_MODE1)
		mode = SSPI_PROT_I2S_MODE1;
	else if (config->prot == SSPI_HW_I2S_MODE2)
		mode = SSPI_PROT_I2S_MODE2;
	else
		return SSPI_HW_ERR_PROT;

#if 0
	switch (config->sampleRate) {
	case CSL_I2S_16BIT_4000HZ:
		clk_source = SSPI_CLK_SRC_AUDIOCLK;
		fifo_pack = SSPI_FIFO_DATA_PACK_16BIT;
		clk_div = 11;
		break;
	case CSL_I2S_16BIT_8000HZ:
		clk_source = SSPI_CLK_SRC_AUDIOCLK;
		fifo_pack = SSPI_FIFO_DATA_PACK_16BIT;
		clk_div = 5;
		break;
	case CSL_I2S_16BIT_16000HZ:
		clk_source = SSPI_CLK_SRC_AUDIOCLK;
		fifo_pack = SSPI_FIFO_DATA_PACK_16BIT;
		clk_div = 2;
		break;
	case CSL_I2S_16BIT_48000HZ:
		clk_source = SSPI_CLK_SRC_AUDIOCLK;
		fifo_pack = SSPI_FIFO_DATA_PACK_16BIT;
		clk_div = 0;
		break;
	case CSL_I2S_32BIT_8000HZ:
		clk_source = SSPI_CLK_SRC_AUDIOCLK;
		fifo_pack = SSPI_FIFO_DATA_PACK_NONE;
		clk_div = 2;
		aTrace
				(LOG_AUDIO_CSL,
				 "sample Rate = CSL_I2S_32BIT_8000HZ \r\n");
		break;
	case CSL_I2S_25BIT_48000HZ:
		clk_source = SSPI_CLK_SRC_INTCLK;
		fifo_pack = SSPI_FIFO_DATA_PACK_NONE;
		clk_div = 0;
		aTrace
				(LOG_AUDIO_CSL,
				 "sample Rate = CSL_I2S_25BIT_48000HZ \r\n");
		break;
	default:
		aTrace
				(LOG_AUDIO_CSL, "unknown rate setting \r\n");
		return SSPI_HW_ERR_PROT;
	}
#endif

	memset(&tk_conf, 0, sizeof(tk_conf));
	chal_sspi_soft_reset(handle);

	if (chal_sspi_set_idle_state(handle, mode))
		return SSPI_HW_ERROR;
#if 0
#ifndef FPGA_VERSION
	if (chal_sspi_set_clk_src_select(handle, clk_source))
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

	word_len = 16;
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

	chal_sspi_set_fifo_threshold(handle,
			(config->
			 rx_ena) ? SSPI_FIFO_ID_RX0 :
			SSPI_FIFO_ID_TX0, 0x10);
	chal_sspi_set_fifo_threshold(handle,
			(config->
			 rx_ena) ? SSPI_FIFO_ID_RX1 :
			SSPI_FIFO_ID_TX1, 0x10);

	chal_sspi_set_fifo_pio_threshhold(handle,
			(config->
			 rx_ena) ? SSPI_FIFO_ID_RX0 :
			SSPI_FIFO_ID_TX0, 0x3, 0x3);
	chal_sspi_set_fifo_pio_threshhold(handle,
			(config->
			 rx_ena) ? SSPI_FIFO_ID_RX1 :
			SSPI_FIFO_ID_TX1, 0x3, 0x3);

	tk_conf.chan_sel = SSPI_CHAN_SEL_CHAN0;
	tk_conf.cs_sel = SSPI_CS_SEL_CS0;
	tk_conf.rx_sel = (config->tx_loopback_ena) ? SSPI_RX_SEL_COPY_TX0
		: SSPI_RX_SEL_RX0;
	tk_conf.tx_sel = SSPI_TX_SEL_TX0;
	tk_conf.div_sel = SSPI_CLK_DIVIDER0;
	tk_conf.seq_ptr = 0;
	/* Max transfer size is set to 4K bytes in non_continuous
	 * mode transfer
	 */
	if ((config->trans_size >> 1) > 0x400) {
		tk_conf.loop_cnt = 0;
		tk_conf.continuous = 1;
	} else {
		tk_conf.loop_cnt = (config->trans_size >> 1) - 1;
		tk_conf.continuous = 0;
	}
	tk_conf.init_cond_mask = (config->rx_ena) ?
		(SSPI_TASK_INIT_COND_THRESHOLD_RX0 |
		 SSPI_TASK_INIT_COND_THRESHOLD_RX1)
		: (SSPI_TASK_INIT_COND_THRESHOLD_TX0 |
				SSPI_TASK_INIT_COND_THRESHOLD_TX1);
	tk_conf.wait_before_start = 1;

	if (chal_sspi_set_task(handle, 0, mode, &tk_conf))
		return SSPI_HW_ERR_TASK;

	seq_conf.tx_enable = (config->tx_ena
			|| config->rx_loopback_ena) ? 1 : 0;
	seq_conf.rx_enable = (config->rx_ena
			|| config->tx_loopback_ena) ? 1 : 0;
	seq_conf.cs_activate = 0; /* left channel, CS is LOW */
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
	if (chal_sspi_set_sequence(handle, 0, mode, &seq_conf))
		return SSPI_HW_ERR_SEQUENCE;

	seq_conf.tx_enable = (config->tx_ena
			|| config->rx_loopback_ena) ? 1 : 0;
	seq_conf.rx_enable = (config->rx_ena
			|| config->tx_loopback_ena) ? 1 : 0;
	seq_conf.cs_activate = 1; /* right channel, CS is HIGH */
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
	if (chal_sspi_set_sequence(handle, 1, mode, &seq_conf))
		return SSPI_HW_ERR_SEQUENCE;

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
	if (chal_sspi_set_sequence(handle, 2, mode, &seq_conf))
		return SSPI_HW_ERR_SEQUENCE;

	if (chal_sspi_set_frame(handle, &frmMask, mode, word_len, 0))
		return SSPI_HW_ERR_FRAME;

	return SSPI_HW_NOERR;
}

/*
 *
 * NAME:  SPI_hw_interleave_init
 *
 *  Description:  This function initializes the SSPI core as I2S
 *  that specified by coreIdx as one channel I2S, master mode.
 *  The Left and Right channel are interleaved
 *
 *  Parameters:
 *     SSPI_hw_core_t *pCore - SSPI core pointer
 *
 *  Returns:  SSPI_hw_status_t
 *
 *  Notes:
 */
static SSPI_hw_status_t SSPI_hw_interleave_init(CSL_HANDLE handle,
		CSL_I2S_CONFIG_t *config)
{
	uint32_t frmMask = 1, word_len = 16;
	CHAL_SSPI_PROT_t mode;
	/*CHAL_SSPI_CLK_SRC_t clk_source;*/
	/*CHAL_SSPI_FIFO_DATA_PACK_t fifo_pack;*/
	/*cUInt32 clk_div;*/
	/*CSL_SSPI_HANDLE_T *pDevice = (CSL_SSPI_HANDLE_T *)handle;*/

	if (config->prot == SSPI_HW_I2S_MODE1)
		mode = SSPI_PROT_I2S_MODE1;
	else if (config->prot == SSPI_HW_I2S_MODE2)
		mode = SSPI_PROT_I2S_MODE2;
	else
		return SSPI_HW_ERR_PROT;

#if 0
	switch (config->sampleRate) {
	case CSL_I2S_16BIT_4000HZ:
		clk_source = SSPI_CLK_SRC_AUDIOCLK;
		fifo_pack = SSPI_FIFO_DATA_PACK_16BIT;
		clk_div = 11;
		aTrace
				(LOG_AUDIO_CSL,
				 "sample Rate = CSL_I2S_16BIT_4000HZ \r\n");
		break;
	case CSL_I2S_16BIT_8000HZ:
		clk_source = SSPI_CLK_SRC_AUDIOCLK;
		fifo_pack = SSPI_FIFO_DATA_PACK_16BIT;
		clk_div = 5;
		aTrace
				(LOG_AUDIO_CSL,
				 "sample Rate = CSL_I2S_16BIT_8000HZ \r\n");
		break;
	case CSL_I2S_16BIT_16000HZ:
		clk_source = SSPI_CLK_SRC_AUDIOCLK;
		fifo_pack = SSPI_FIFO_DATA_PACK_16BIT;
		clk_div = 2;
		aTrace
				(LOG_AUDIO_CSL,
				 "sample Rate = CSL_I2S_16BIT_16000HZ \r\n");
		break;
	case CSL_I2S_16BIT_48000HZ:
		clk_source = SSPI_CLK_SRC_AUDIOCLK;
		fifo_pack = SSPI_FIFO_DATA_PACK_16BIT;
		clk_div = 0;
		aTrace
				(LOG_AUDIO_CSL,
				 "sample Rate = CSL_I2S_16BIT_48000HZ \r\n");
		break;
	case CSL_I2S_32BIT_8000HZ:
		clk_source = SSPI_CLK_SRC_AUDIOCLK;
		fifo_pack = SSPI_FIFO_DATA_PACK_NONE;
		clk_div = 2;
		aTrace
				(LOG_AUDIO_CSL,
				 "sample Rate = CSL_I2S_32BIT_8000HZ \r\n");
		break;
	case CSL_I2S_25BIT_48000HZ:
		clk_source = SSPI_CLK_SRC_INTCLK;
		fifo_pack = SSPI_FIFO_DATA_PACK_NONE;
		clk_div = 0;
		aTrace
				(LOG_AUDIO_CSL,
				 "sample Rate = CSL_I2S_25BIT_48000HZ \r\n");
		break;
	default:
		aTrace
				(LOG_AUDIO_CSL, "unknown rate setting \r\n");
		return SSPI_HW_ERR_PROT;
	}
#endif

	memset(&tk_conf, 0, sizeof(tk_conf));

	chal_sspi_soft_reset(handle);
	if (chal_sspi_set_idle_state(handle, mode))
		return SSPI_HW_ERROR;
#if 0
#ifndef FPGA_VERSION
	if (chal_sspi_set_clk_src_select(handle, clk_source))
		/*if(chal_sspi_set_clk_src_select(handle,
		 * SSPI_CLK_SRC_AUDIOCLK))
		 */
		/*if(chal_sspi_set_clk_src_select(handle,
		 * SSPI_CLK_SRC_INTCLK))
		 */
		return SSPI_HW_ERROR;
#endif
#endif

	chal_sspi_set_clk_divider(handle, SSPI_CLK_DIVIDER0, 0);
	chal_sspi_set_clk_divider(handle, SSPI_CLK_REF_DIVIDER, 0);
	chal_sspi_set_clk_src_select(handle, SSPI_CLK_SRC_CAPHCLK);
	chal_sspi_set_caph_clk(handle, SSPI_CAPH_CLK_TRIG_48kHz,
			SSPI_HW_WORD_LEN_16Bit, 2);

	chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_RX0, SSPI_FIFO_SIZE_FULL);
	chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_RX1, SSPI_FIFO_SIZE_NONE);
	chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_RX2, SSPI_FIFO_SIZE_NONE);
	chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_RX3, SSPI_FIFO_SIZE_NONE);
	chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_TX0, SSPI_FIFO_SIZE_FULL);
	chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_TX1, SSPI_FIFO_SIZE_NONE);
	chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_TX2, SSPI_FIFO_SIZE_NONE);
	chal_sspi_set_fifo_size(handle, SSPI_FIFO_ID_TX3, SSPI_FIFO_SIZE_NONE);

	/*chal_sspi_set_fifo_pack(handle, SSPI_FIFO_ID_RX0, fifo_pack);*/
	/*chal_sspi_set_fifo_pack(handle, SSPI_FIFO_ID_TX0, fifo_pack);*/
	chal_sspi_set_fifo_pack(handle, SSPI_FIFO_ID_RX0,
			SSPI_FIFO_DATA_PACK_16BIT);
	chal_sspi_set_fifo_pack(handle, SSPI_FIFO_ID_TX0,
			SSPI_FIFO_DATA_PACK_16BIT);

	/*chal_sspi_set_clk_divider(handle, SSPI_CLK_DIVIDER0, 0);*/
#if 0
	chal_sspi_set_clk_divider(handle, SSPI_CLK_DIVIDER0, clk_div);
	chal_sspi_set_clk_divider(handle, SSPI_CLK_DIVIDER1, 1);
	chal_sspi_set_clk_divider(handle, SSPI_CLK_DIVIDER2, 2);
	chal_sspi_set_clk_divider(handle, SSPI_CLK_REF_DIVIDER, 4);
#endif

	chal_sspi_set_mode(handle, SSPI_MODE_MASTER);
	chal_sspi_enable(handle, 1);

	/* chal_sspi_set_fifo_threshold(handle,
	 * (config->rx_ena) ? SSPI_FIFO_ID_RX0 : SSPI_FIFO_ID_TX0,
	 * 0x10);
	 */

	if (config->rx_ena) {
		chal_sspi_set_fifo_pio_threshhold(handle, SSPI_FIFO_ID_RX0,
				0x1c, 0x3);
	}

	if (config->tx_ena) {
		chal_sspi_set_fifo_pio_threshhold(handle, SSPI_FIFO_ID_TX0, 0x3,
				0x1c);
	}

	chal_sspi_set_fifo_pio_threshhold(handle,
			(config->
			 rx_ena) ? SSPI_FIFO_ID_RX1 :
			SSPI_FIFO_ID_TX1, 0x3, 0x3);

	if (config->tx_ena)
		chal_sspi_set_fifo_threshold(handle, SSPI_FIFO_ID_TX0, 0x10);

	tk_conf.chan_sel = SSPI_CHAN_SEL_CHAN0;
	tk_conf.cs_sel = SSPI_CS_SEL_CS0;
	tk_conf.rx_sel = (config->tx_loopback_ena) ? SSPI_RX_SEL_COPY_TX0
		: SSPI_RX_SEL_RX0;
	tk_conf.tx_sel = SSPI_TX_SEL_TX0;
	tk_conf.div_sel = SSPI_CLK_DIVIDER0;
	tk_conf.seq_ptr = 0;
	/* Max transfer size is set to 4K bytes in non_continuous
	 * mode transfer
	 */
	if ((config->trans_size >> 1) > 0x400) {
		tk_conf.loop_cnt = 0;
		tk_conf.continuous = 1;
	} else {
		tk_conf.loop_cnt = (config->trans_size >> 1) - 1;
		tk_conf.continuous = 0;
	}
	tk_conf.init_cond_mask = (config->rx_ena) ?
		SSPI_TASK_INIT_COND_THRESHOLD_RX0 :
		SSPI_TASK_INIT_COND_THRESHOLD_TX0;
	tk_conf.wait_before_start = 1;

	if (chal_sspi_set_task(handle, 0, mode, &tk_conf))
		return SSPI_HW_ERR_TASK;

	switch (config->prot) {
	case SSPI_HW_I2S_MODE1:
	case SSPI_HW_I2S_MODE2:
		if (config->prot == SSPI_HW_I2S_MODE1)
			mode = SSPI_PROT_I2S_MODE1;
		else
			mode = SSPI_PROT_I2S_MODE2;

		seq_conf.tx_enable = (config->tx_ena || config->rx_loopback_ena)
			? 1 : 0;
		seq_conf.rx_enable = (config->rx_ena || config->tx_loopback_ena)
			? 1 : 0;
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
		if (chal_sspi_set_sequence(handle, 0, mode, &seq_conf))
			return SSPI_HW_ERR_SEQUENCE;

		seq_conf.tx_enable = (config->tx_ena || config->rx_loopback_ena)
			? 1 : 0;
		seq_conf.rx_enable = (config->rx_ena || config->tx_loopback_ena)
			? 1 : 0;
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
		if (chal_sspi_set_sequence(handle, 1, mode, &seq_conf))
			return SSPI_HW_ERR_SEQUENCE;

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
		if (chal_sspi_set_sequence(handle, 2, mode, &seq_conf))
			return SSPI_HW_ERR_SEQUENCE;

		if (chal_sspi_set_frame(handle, &frmMask, mode, word_len, 0))
			return SSPI_HW_ERR_FRAME;

		break;

	default:
		return SSPI_HW_ERR_PROT;
	}
	return SSPI_HW_NOERR;
}

/*
 *
 * NAME:  SSPI_hw_i2s_init
 *
 *  Description:  This function initializes the SSPI core that specified
 *  by coreIdx.as two channel I2S, Slave Mode, left and Right seperated
 *
 *  Parameters:
 *     SSPI_hw_core_t *pCore - SSPI core pointer
 *
 *  Returns:  SSPI_hw_status_t
 *
 *  Notes:
 *
 */
static SSPI_hw_status_t SSPI_hw_i2s_slave_init(CSL_HANDLE handle,
		CSL_I2S_CONFIG_t *config)
{
	uint32_t frmMask = 1;
	CHAL_SSPI_PROT_t mode;

	if (config->prot == SSPI_HW_I2S_MODE1)
		mode = SSPI_PROT_I2S_MODE1;
	else if (config->prot == SSPI_HW_I2S_MODE2)
		mode = SSPI_PROT_I2S_MODE2;
	else
		return SSPI_HW_ERR_PROT;

	memset(&tk_conf, 0, sizeof(tk_conf));
	chal_sspi_soft_reset(handle);
	if (chal_sspi_set_idle_state(handle, mode))
		return SSPI_HW_ERROR;

	chal_sspi_soft_reset(handle);
	chal_sspi_set_mode(handle, SSPI_MODE_SLAVE);

	if (chal_sspi_set_idle_state(handle, SSPI_PROT_I2S_MODE2))
		return SSPI_HW_ERROR;

#ifndef FPGA_VERSION
	if (chal_sspi_set_clk_src_select(handle, SSPI_CLK_SRC_AUDIOCLK))
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

	chal_sspi_set_fifo_pack(handle, SSPI_FIFO_ID_RX0,
			SSPI_FIFO_DATA_PACK_16BIT);
	chal_sspi_set_fifo_pack(handle, SSPI_FIFO_ID_RX1,
			SSPI_FIFO_DATA_PACK_16BIT);
	chal_sspi_set_fifo_pack(handle, SSPI_FIFO_ID_TX0,
			SSPI_FIFO_DATA_PACK_16BIT);
	chal_sspi_set_fifo_pack(handle, SSPI_FIFO_ID_TX1,
			SSPI_FIFO_DATA_PACK_16BIT);

	chal_sspi_set_clk_divider(handle, SSPI_CLK_REF_DIVIDER, 8);

	chal_sspi_enable(handle, 1);
	/*chal_sspi_clear_intr(handle, SSPI_INTR_STATUS_SCHEDULER);*/
	if (config->tx_ena) {
		chal_sspi_set_fifo_threshold(handle, SSPI_FIFO_ID_TX0, 0x10);
		chal_sspi_set_fifo_threshold(handle, SSPI_FIFO_ID_TX1, 0x10);

		chal_sspi_set_fifo_pio_threshhold(handle, SSPI_FIFO_ID_TX0, 0x3,
				0x3);
		chal_sspi_set_fifo_pio_threshhold(handle, SSPI_FIFO_ID_TX1, 0x3,
				0x3);

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
		(SSPI_TASK_INIT_COND_THRESHOLD_TX0 |
		 SSPI_TASK_INIT_COND_THRESHOLD_TX1)
		: 0;
	if (config->tx_ena)
		tk_conf.wait_before_start = 1;
	else
		tk_conf.wait_before_start = 0;

	if (chal_sspi_set_task(handle, 0, SSPI_PROT_I2S_MODE2, &tk_conf))
		return SSPI_HW_ERR_TASK;

	/* In slave mode the 1st sequence do nothing.
	 * It is used to make sure the tx/rx
	 * data can be latched correctly.
	 */
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
	if (chal_sspi_set_sequence(handle, 0, SSPI_PROT_I2S_MODE2, &seq_conf))
		return SSPI_HW_ERR_SEQUENCE;

	seq_conf.tx_enable = (config->tx_ena || config->rx_loopback_ena)
		? TRUE : FALSE;
	seq_conf.rx_enable = (config->rx_ena || config->tx_loopback_ena)
		? TRUE : FALSE;
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
	if (chal_sspi_set_sequence(handle, 1, SSPI_PROT_I2S_MODE2, &seq_conf))
		return SSPI_HW_ERR_SEQUENCE;

	seq_conf.tx_enable = (config->tx_ena || config->rx_loopback_ena)
		? TRUE : FALSE;
	seq_conf.rx_enable = (config->rx_ena || config->tx_loopback_ena)
		? TRUE : FALSE;
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
	if (chal_sspi_set_sequence(handle, 2, SSPI_PROT_I2S_MODE2, &seq_conf))
		return SSPI_HW_ERR_SEQUENCE;

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
	if (chal_sspi_set_sequence(handle, 3, mode, &seq_conf))
		return SSPI_HW_ERR_SEQUENCE;

	if (chal_sspi_set_frame(handle, &frmMask, SSPI_PROT_I2S_MODE2, 16, 0))
		return SSPI_HW_ERR_FRAME;

	return SSPI_HW_NOERR;
}

/*
 *
 * NAME:  SPI_hw_interleave_init
 *
 *
 *  Description:  This function initializes the SSPI core as I2S that
 *  specified by coreIdx as one channel I2S, slave mode.
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
 */
static SSPI_hw_status_t SSPI_hw_interleave_slave_init(CSL_HANDLE handle,
		CSL_I2S_CONFIG_t *config)
{
	uint32_t frmMask = 1, word_len = 16;
	CHAL_SSPI_PROT_t mode;

	if (config->prot == SSPI_HW_I2S_MODE1)
		mode = SSPI_PROT_I2S_MODE1;
	else if (config->prot == SSPI_HW_I2S_MODE2)
		mode = SSPI_PROT_I2S_MODE2;
	else
		return SSPI_HW_ERR_PROT;

	memset(&tk_conf, 0, sizeof(tk_conf));
	chal_sspi_soft_reset(handle);
	if (chal_sspi_set_idle_state(handle, mode))
		return SSPI_HW_ERROR;

	chal_sspi_soft_reset(handle);
	chal_sspi_set_mode(handle, SSPI_MODE_SLAVE);

	if (chal_sspi_set_idle_state(handle, mode))
		return SSPI_HW_ERROR;

#ifndef FPGA_VERSION
	if (chal_sspi_set_clk_src_select(handle, SSPI_CLK_SRC_AUDIOCLK))
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

	chal_sspi_set_fifo_pack(handle, SSPI_FIFO_ID_RX0,
			SSPI_FIFO_DATA_PACK_16BIT);
	chal_sspi_set_fifo_pack(handle, SSPI_FIFO_ID_TX0,
			SSPI_FIFO_DATA_PACK_16BIT);

	chal_sspi_set_clk_divider(handle, SSPI_CLK_DIVIDER0, 0);
	chal_sspi_set_clk_divider(handle, SSPI_CLK_DIVIDER1, 1);
	chal_sspi_set_clk_divider(handle, SSPI_CLK_DIVIDER2, 15);
	chal_sspi_set_clk_divider(handle, SSPI_CLK_REF_DIVIDER, 4);

	chal_sspi_enable(handle, 1);
	chal_sspi_clear_intr(handle, SSPIL_INTR_STATUS_SCHEDULER, 0);

	if (config->tx_ena) {
		chal_sspi_set_fifo_threshold(handle, SSPI_FIFO_ID_TX0, 0x10);

		chal_sspi_set_fifo_pio_threshhold(handle, SSPI_FIFO_ID_TX0, 0x3,
				0x3);
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
	if (config->tx_ena)
		tk_conf.wait_before_start = 1;
	else
		tk_conf.wait_before_start = 0;

	if (chal_sspi_set_task(handle, 0, mode, &tk_conf))
		return SSPI_HW_ERR_TASK;

	/*In slave mode the 1st sequence do nothing.
	 * It is used to make sure the tx/rx
	 * data can be latched correctly.
	 */
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
	if (chal_sspi_set_sequence(handle, 0, mode, &seq_conf))
		return SSPI_HW_ERR_SEQUENCE;

	seq_conf.tx_enable = (config->tx_ena || config->rx_loopback_ena)
		? TRUE : FALSE;
	seq_conf.rx_enable = (config->rx_ena || config->tx_loopback_ena)
		? TRUE : FALSE;
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
	if (chal_sspi_set_sequence(handle, 1, mode, &seq_conf))
		return SSPI_HW_ERR_SEQUENCE;

	seq_conf.tx_enable = (config->tx_ena || config->rx_loopback_ena)
		? TRUE : FALSE;
	seq_conf.rx_enable = (config->rx_ena || config->tx_loopback_ena)
		? TRUE : FALSE;
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
	if (chal_sspi_set_sequence(handle, 2, mode, &seq_conf))
		return SSPI_HW_ERR_SEQUENCE;

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
	if (chal_sspi_set_sequence(handle, 3, mode, &seq_conf))
		return SSPI_HW_ERR_SEQUENCE;

	if (chal_sspi_set_frame(handle, &frmMask, mode, word_len, 0))
		return SSPI_HW_ERR_FRAME;

	return SSPI_HW_NOERR;
}
