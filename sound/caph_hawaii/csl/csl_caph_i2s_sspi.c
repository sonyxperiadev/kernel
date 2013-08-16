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
#include "chal_types.h"
//#include <plat/chal/chal_sspi.h>  //Kishore add Capri SSPI files
#include "chal_sspi_hawaii.h"
#include "csl_caph_i2s_sspi.h"
#include "brcm_rdb_sspil.h"
#include "brcm_rdb_padctrlreg.h"
#include "brcm_rdb_sysmap.h"
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
#define  MAX(x, y) (((x) > (y)) ? (x) : (y))

static chal_sspi_task_conf_t tk_conf;
static chal_sspi_seq_conf_t seq_conf;
static chal_sspi_frm_conf_t frm_conf;
static SSPI_hw_status_t SSPI_hw_i2s_init(CSL_HANDLE handle,
		CSL_I2S_CONFIG_t *config);
static void set_fifo_pack(CSL_HANDLE handle, CHAL_SSPI_FIFO_ID_t fifo_id,
						  UInt32 word_len, Boolean pack)
{
	if (pack) {
		if (word_len <= 8)
			chal_sspi_set_fifo_pack(handle, fifo_id,
					SSPI_FIFO_DATA_PACK_8BIT);
		else if (word_len <= 16)
			chal_sspi_set_fifo_pack(handle, fifo_id,
					SSPI_FIFO_DATA_PACK_16BIT);
		else
			chal_sspi_set_fifo_pack(handle, fifo_id,
					SSPI_FIFO_DATA_PACK_NONE);
	} else
		chal_sspi_set_fifo_pack(handle, fifo_id,
				SSPI_FIFO_DATA_PACK_NONE);
}

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
 CSL_I2S_CONFIG_t *config)
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
			 "csl_i2s_config:: handle %p mode %d txEn %d"
			 "rxEn %d txLbEn %d"
			 "rxLbEn %d trans_size %ld.\r\n",
			 handle, config->mode, config->tx_ena, config->rx_ena,
			 config->tx_loopback_ena, config->rx_loopback_ena,
			 config->trans_size);
	aTrace
			(LOG_AUDIO_CSL,
			 "csl_i2s_config:: prot %ld interleave %d sr %ld.\r\n",
			 config->prot, config->interleave, config->sampleRate);

	pDevice = (CSL_SSPI_HANDLE_T *)handle;

	status = SSPI_hw_i2s_init(handle, config);
	if (status) {
		aTrace(LOG_AUDIO_CSL,
				"SSPI_hw_i2s_init failed \r\n");
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
	chal_sspi_fifo_reset(handle, SSPI_FIFO_ID_TX2);
	chal_sspi_fifo_reset(handle, SSPI_FIFO_ID_TX3);

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
	chal_sspi_fifo_reset(handle, SSPI_FIFO_ID_RX2);
	chal_sspi_fifo_reset(handle, SSPI_FIFO_ID_RX3);

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
	uint32_t frmMask = 1;
	CHAL_SSPI_PROT_t prot;
	uint32_t dword_sz, sample_len, seq_idx;

	if (config->prot == SSPI_HW_I2S_MODE1)
		prot = SSPI_PROT_I2S_MODE1;
	else if (config->prot == SSPI_HW_I2S_MODE2)
		prot = SSPI_PROT_I2S_MODE2;
	else
		return SSPI_HW_ERR_PROT;

	memset(&tk_conf, 0, sizeof(tk_conf));
	memset(&seq_conf, 0, sizeof(seq_conf));
	memset(&frm_conf, 0, sizeof(frm_conf));

	chal_sspi_soft_reset(handle);
	if (config->mode == CSL_I2S_MASTER_MODE)
		chal_sspi_set_mode(handle, SSPI_MODE_MASTER);
	else
		chal_sspi_set_mode(handle, SSPI_MODE_SLAVE);
	if (chal_sspi_set_idle_state(handle, prot))
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
	sample_len = MAX(config->rx_delay_bits + config->rx_len,
			config->tx_len + config->tx_prepad_bits +
			config->tx_postpad_bits);

	/* use caph clock */
	chal_sspi_set_clk_divider(handle, SSPI_CLK_REF_DIVIDER, 0);
	chal_sspi_set_clk_src_select(handle, SSPI_CLK_SRC_CAPHCLK);
	chal_sspi_set_caph_clk(handle, config->sampleRate, sample_len * 2);

	set_fifo_pack(handle, SSPI_FIFO_ID_RX0,
			config->rx_len, config->rx_pack);
	set_fifo_pack(handle, SSPI_FIFO_ID_TX0,
			config->tx_len, config->tx_pack);

	chal_sspi_enable(handle, 1);

	// Kishore - add Rhea code here or get Capri SSP
	chal_sspi_get_max_fifo_size(handle, &dword_sz);
	dword_sz >>= 2; /* change to size in DWord */
	if (config->interleave) {
		chal_sspi_set_fifo_size(handle,
			SSPI_FIFO_ID_RX0, SSPI_FIFO_SIZE_FULL);
		chal_sspi_set_fifo_size(handle,
			SSPI_FIFO_ID_RX1, SSPI_FIFO_SIZE_NONE);
		chal_sspi_set_fifo_size(handle,
			SSPI_FIFO_ID_RX2, SSPI_FIFO_SIZE_NONE);
		chal_sspi_set_fifo_size(handle,
			SSPI_FIFO_ID_RX3, SSPI_FIFO_SIZE_NONE);
		chal_sspi_set_fifo_size(handle,
			SSPI_FIFO_ID_TX0, SSPI_FIFO_SIZE_FULL);
		chal_sspi_set_fifo_size(handle,
			SSPI_FIFO_ID_TX1, SSPI_FIFO_SIZE_NONE);
		chal_sspi_set_fifo_size(handle,
			SSPI_FIFO_ID_TX2, SSPI_FIFO_SIZE_NONE);
		chal_sspi_set_fifo_size(handle,
			SSPI_FIFO_ID_TX3, SSPI_FIFO_SIZE_NONE);
		if (config->rx_ena) {
			chal_sspi_set_fifo_pio_threshhold(handle,
					SSPI_FIFO_ID_RX0, dword_sz-4, 0x3);
		}
		if (config->tx_ena) {
			chal_sspi_set_fifo_pio_threshhold(handle,
					SSPI_FIFO_ID_TX0, 0x3, dword_sz-4);
		}
	} else {
		chal_sspi_set_fifo_size(handle,
			SSPI_FIFO_ID_RX0, SSPI_FIFO_SIZE_HALF);
		chal_sspi_set_fifo_size(handle,
			SSPI_FIFO_ID_RX1, SSPI_FIFO_SIZE_HALF);
		chal_sspi_set_fifo_size(handle,
			SSPI_FIFO_ID_RX2, SSPI_FIFO_SIZE_NONE);
		chal_sspi_set_fifo_size(handle,
			SSPI_FIFO_ID_RX3, SSPI_FIFO_SIZE_NONE);
		chal_sspi_set_fifo_size(handle,
			SSPI_FIFO_ID_TX0, SSPI_FIFO_SIZE_HALF);
		chal_sspi_set_fifo_size(handle,
			SSPI_FIFO_ID_TX1, SSPI_FIFO_SIZE_HALF);
		chal_sspi_set_fifo_size(handle,
			SSPI_FIFO_ID_TX2, SSPI_FIFO_SIZE_NONE);
		chal_sspi_set_fifo_size(handle,
			SSPI_FIFO_ID_TX3, SSPI_FIFO_SIZE_NONE);
		set_fifo_pack(handle, SSPI_FIFO_ID_RX1,
					config->rx_len, config->rx_pack);
		set_fifo_pack(handle, SSPI_FIFO_ID_TX1,
					config->tx_len, config->tx_pack);
		if (config->rx_ena) {
			chal_sspi_set_fifo_pio_threshhold(handle,
					SSPI_FIFO_ID_RX0, dword_sz/2-4, 0x3);
			chal_sspi_set_fifo_pio_threshhold(handle,
					SSPI_FIFO_ID_RX1, dword_sz/2-4, 0x3);
		}
		if (config->tx_ena) {
			chal_sspi_set_fifo_pio_threshhold(handle,
					SSPI_FIFO_ID_TX0, 0x3, dword_sz/2-4);
			chal_sspi_set_fifo_pio_threshhold(handle,
					SSPI_FIFO_ID_TX1, 0x3, dword_sz/2-4);
		}
	}

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
	tk_conf.init_cond_mask |= (config->tx_ena) ?
				SSPI_TASK_INIT_COND_THRESHOLD_TX0 : 0;
	if (!config->interleave)
		tk_conf.init_cond_mask |= (config->tx_ena) ?
				SSPI_TASK_INIT_COND_THRESHOLD_TX1 : 0;
	tk_conf.wait_before_start = 1;

	if (chal_sspi_set_task(handle, 0, prot, &tk_conf))
		return SSPI_HW_ERR_TASK;

	seq_idx = 0;
	if (config->mode == CSL_I2S_SLAVE_MODE) {
		/* In slave mode the 1st sequence do nothing. It is used to
		 *make sure the tx/rx data can be latched correctly. */
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
		if (chal_sspi_set_sequence(handle, seq_idx, prot, &seq_conf))
			return SSPI_HW_ERR_SEQUENCE;
		seq_idx++;
	}

	seq_conf.tx_enable = (config->tx_ena ||
				config->rx_loopback_ena) ? 1 : 0;
	seq_conf.rx_enable = (config->rx_ena ||
				config->tx_loopback_ena) ? 1 : 0;
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
	if (chal_sspi_set_sequence(handle, seq_idx, prot, &seq_conf))
		return SSPI_HW_ERR_SEQUENCE;

	seq_idx++;
	if (config->interleave) {
		seq_conf.cs_activate = 1;
		seq_conf.cs_deactivate = 0;
		seq_conf.opcode = SSPI_SEQ_OPCODE_COND_JUMP;
		seq_conf.rx_fifo_sel = 0;
		seq_conf.tx_fifo_sel = 0;
	} else {
		seq_conf.cs_activate = 1;
		seq_conf.cs_deactivate = 0;
		seq_conf.opcode = SSPI_SEQ_OPCODE_COND_JUMP;
		seq_conf.rx_fifo_sel = 1;
		seq_conf.tx_fifo_sel = 1;
	}
	if (chal_sspi_set_sequence(handle, seq_idx, prot, &seq_conf))
			return SSPI_HW_ERR_SEQUENCE;

	seq_idx++;
	seq_conf.tx_enable = 0;
	seq_conf.rx_enable = 0;
	seq_conf.cs_activate = 0;
	seq_conf.cs_deactivate = 0;
	seq_conf.opcode = SSPI_SEQ_OPCODE_STOP;
	seq_conf.rx_fifo_sel = 0;
	seq_conf.tx_fifo_sel = 0;
	if (chal_sspi_set_sequence(handle, seq_idx, prot, &seq_conf))
		return SSPI_HW_ERR_SEQUENCE;

	frm_conf.rx_delay_bits = config->rx_delay_bits;
	frm_conf.rx_len = config->rx_len;
	frm_conf.tx_len = config->tx_len;
	frm_conf.tx_prepad_bits = config->tx_prepad_bits;
	frm_conf.tx_postpad_bits = config->tx_postpad_bits;
	frm_conf.tx_padval = config->tx_padval;

	// Kishore - add Rhea code here or get Capri SSP
	if (chal_sspi_set_i2s_frame(handle, &frmMask, prot, frm_conf))
		return SSPI_HW_ERR_FRAME;

	return SSPI_HW_NOERR;
}
