/**************************************************************************
 * Copyright 2009, 2010 Broadcom Corporation.  All rights reserved.          */
/*                                                                        */
/*     Unless you and Broadcom execute a separate written software license*/
/*     agreement governing use of this software, this software is licensed*/
/*     to you under the terms of the GNU General Public License version 2 */
/*     (the GPL), available at                                            */
/*                                                                        */
/*     http://www.broadcom.com/licenses/GPLv2.php                         */
/*                                                                        */
/*     with the following added to such license:                          */
/*                                                                        */
/*     As a special exception, the copyright holders of this software give*/
/*     you permission to link this software with independent modules, and */
/*     to copy and distribute the resulting executable under terms of your*/
/*     choice, provided that you also meet, for each linked             */
/*     independent module, the terms and conditions of the license of that*/
/*     module.An independent module is a module which is not derived from */
/*     this software.  The special exception does not apply to any        */
/*     modifications of the software.                                     */
/*                                                                        */
/*     Notwithstanding the above, under no circumstances may you combine  */
/*     this software in any way with any other Broadcom software provided */
/*     under a license other than the GPL,                                */
/*     without Broadcom's express prior written consent.                  */
/*                                                                        */
/**************************************************************************/
/**
*
*  @file   csl_caph_pcm_sspi.c
*
*  @brief  PCM device CSL layer implementation
*
****************************************************************************/
#include <string.h>
#include "mobcom_types.h"
#include <plat/chal/chal_types.h>
#include "chal_caph.h"
#include <plat/chal/chal_sspi.h>
#include "chal_caph_intc.h"
#include "csl_caph.h"
#include "csl_caph_pcm_sspi.h"
#include <mach/rdb/brcm_rdb_padctrlreg.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_sspil.h>
#include "audio_trace.h"
#include <mach/rdb/brcm_rdb_khub_rst_mgr_reg.h>
#include <linux/io.h>
#include <mach/io_map.h>

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

/******************************************************************************
* Local Definitions
******************************************************************************/
static chal_sspi_task_conf_t task_conf;
static chal_sspi_seq_conf_t seq_conf;
static CHAL_HANDLE intc_handle;

/*
 *  Description: soft reset ssp port
 */
static void csl_pcm_reset(CSL_PCM_HANDLE_t *pDevice)
{
	u32 *reg, *reg2, reg_value, data, shift, reg_wr;

	if (pDevice->base == KONA_SSP3_BASE_VA) {
		shift = KHUB_RST_MGR_REG_SOFT_RSTN1_SSP3_SOFT_RSTN_SHIFT;
		aTrace(LOG_AUDIO_CSL, "csl_pcm_reset::reset SSP3 port, "
			"shift %d\n", shift);
	} else if (pDevice->base == KONA_SSP4_BASE_VA) {
		shift = KHUB_RST_MGR_REG_SOFT_RSTN1_SSP4_SOFT_RSTN_SHIFT;
		aTrace(LOG_AUDIO_CSL, "csl_pcm_reset::reset SSP4 port, "
			"shift %d\n", shift);
	} else {
		aError("csl_pcm_reset::AUDIO ERROR invalid base 0x%x\n",
			(u32)pDevice->base);
		return;
	}

	reg = (u32 *)KONA_HUB_RST_VA;
	reg_wr = readl(reg);

	if ((reg_wr & KHUB_RST_MGR_REG_WR_ACCESS_RSTMGR_ACC_MASK) == 0) {
		data = reg_wr |
			(0xa5a5<<KHUB_RST_MGR_REG_WR_ACCESS_PASSWORD_SHIFT) |
			(1<<KHUB_RST_MGR_REG_WR_ACCESS_RSTMGR_ACC_SHIFT);
		writel(data, reg); /*to get access*/
		/*aError("csl_pcm_reset::RST_WR 0x%x --> 0x%x\n",
			reg_wr, data);*/
	}

	reg2 = reg + (KHUB_RST_MGR_REG_SOFT_RSTN1_OFFSET>>2);
	reg_value = readl(reg2);
	data = reg_value & (~(1<<shift)); /*reset*/
	writel(data, reg2);

	/*aError("csl_pcm_reset::RST_RSTN1 0x%08x --> 0x%08x\n",
		reg_value, data);*/

	reg_value = readl(reg2);
	data = reg_value | (1<<shift); /*set*/
	writel(data, reg2);

	/*aError("csl_pcm_reset::RST_RSTN1 0x%08x --> 0x%08x\n",
		reg_value, data);*/

	/*if ((reg_wr & KHUB_RST_MGR_REG_WR_ACCESS_RSTMGR_ACC_MASK) == 0) {
		data = reg_wr |
			(0xa5a5<<KHUB_RST_MGR_REG_WR_ACCESS_PASSWORD_SHIFT);
		writel(data, reg);
	}*/
}

/*
 *
 *  Function Name:      csl_pcm_init
 *
 *  Description:        This function initializes the CSL layer
 *
 */
CSL_PCM_HANDLE csl_pcm_init(UInt32 baseAddr, UInt32 caphIntcHandle)
{
	CSL_PCM_HANDLE handle = 0;
	CSL_PCM_HANDLE_t *pDevice;

	aTrace(LOG_AUDIO_CSL, "+csl_pcm_init\r\n");

	handle = chal_sspi_init(baseAddr);
	pDevice = (CSL_PCM_HANDLE_t *) handle;
	if (handle == NULL) {
		aError("csl_pcm_init failed\r\n");
		return NULL;
	}

	chal_sspi_set_type(handle, SSPI_TYPE_LITE);

	intc_handle = (CHAL_HANDLE) caphIntcHandle;

	aTrace(LOG_AUDIO_CSL, "-csl_pcm_init base address 0x%x\r\n",
			(unsigned int)pDevice->base);

	return handle;
}

/*
 *
 *  Function Name:      csl_pcm_deinit
 *
 *  Description:        This function deinitializes the CSL layer
 *
 */
CSL_PCM_OPSTATUS_t csl_pcm_deinit(CSL_PCM_HANDLE handle)
{
	CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *) handle;
	CHAL_SSPI_STATUS_t status;

	aTrace(LOG_AUDIO_CSL, "+csl_pcm_deinit\r\n");

	if (handle == NULL) {
		aError("csl_pcm_deinit failed\r\n");
		return CSL_PCM_ERR_HANDLE;
	}

	status = chal_sspi_deinit(pDevice);

	aTrace(LOG_AUDIO_CSL, "-csl_pcm_deinit\r\n");

	if (status == CHAL_SSPI_STATUS_SUCCESS)
		return CSL_PCM_SUCCESS;
	else if (status == CHAL_SSPI_STATUS_ILLEGAL_HANDLE)
		return CSL_PCM_ERR_HANDLE;
	return CSL_PCM_SUCCESS;
}

/*
 *  Function Name:      csl_pcm_enable_scheduler
 *
 *  Description:        This function starts scheduler operation
 *
 */
CSL_PCM_OPSTATUS_t csl_pcm_enable_scheduler(CSL_PCM_HANDLE handle,
					    Boolean enable)
{
	CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *) handle;

	aTrace(LOG_AUDIO_CSL,
			"csl_pcm_enable_scheduler:: handle %p enable %d.\r\n",
			handle, enable);
	if (!pDevice)
		return CSL_PCM_ERR_HANDLE;

	chal_sspi_enable_scheduler(pDevice, enable);

	return CSL_PCM_SUCCESS;
}

/*
 *
 *  Function Name:      csl_pcm_start
 *
 *  Description:        This function starts scheduler operation
 *
 */
CSL_PCM_OPSTATUS_t csl_pcm_start(CSL_PCM_HANDLE handle,
				 csl_pcm_config_device_t *config)
{
	CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *) handle;
	CHAL_SSPI_STATUS_t status;

	aTrace(LOG_AUDIO_CSL, "csl_pcm_start::"
			      "handle %p.\r\n", handle);
	aTrace(LOG_AUDIO_CSL,
			      "csl_pcm_start:: cfgDev mode %d interleave %d protocol %d"
			      "format %d size %ld bits %ld sr %ld.\r\n",
			      config->mode, config->interleave,
			      config->protocol, config->format,
			      config->xferSize, config->ext_bits,
			      config->sample_rate);

	/*pcm_config_dma(handle, config); */
	/*enable scheduler operation */
	status = chal_sspi_enable_scheduler(pDevice, 1);

	chal_sspi_enable_fifo_pio_start_stop_intr(pDevice,
						  SSPI_FIFO_ID_TX0, TRUE, TRUE);
	chal_sspi_enable_fifo_pio_start_stop_intr(pDevice,
						  SSPI_FIFO_ID_TX1, TRUE, TRUE);
	chal_sspi_enable_fifo_pio_start_stop_intr(pDevice,
						  SSPI_FIFO_ID_TX2, TRUE, TRUE);
	chal_sspi_enable_fifo_pio_start_stop_intr(pDevice,
						  SSPI_FIFO_ID_TX3, TRUE, TRUE);
	chal_sspi_enable_fifo_pio_start_stop_intr(pDevice,
						  SSPI_FIFO_ID_RX0, TRUE, TRUE);
	chal_sspi_enable_fifo_pio_start_stop_intr(pDevice,
						  SSPI_FIFO_ID_RX1, TRUE, TRUE);
	chal_sspi_enable_fifo_pio_start_stop_intr(pDevice,
						  SSPI_FIFO_ID_RX2, TRUE, TRUE);
	chal_sspi_enable_fifo_pio_start_stop_intr(pDevice,
						  SSPI_FIFO_ID_RX3, TRUE, TRUE);

	if (status == CHAL_SSPI_STATUS_SUCCESS)
		return CSL_PCM_SUCCESS;
	else if (status == CHAL_SSPI_STATUS_ILLEGAL_HANDLE)
		return CSL_PCM_ERR_HANDLE;
	return CSL_PCM_SUCCESS;
}

/*
 *
 *  Function Name:      csl_pcm_start_tx
 *
 *  Description:        This function starts transmit operation
 *
 */
CSL_PCM_OPSTATUS_t csl_pcm_start_tx(CSL_PCM_HANDLE handle, UInt8 channel)
{
	CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *) handle;

	aTrace(LOG_AUDIO_CSL,
			"csl_pcm_start_tx:: handle %p, channel %d.\r\n", handle,
			channel);
	if (!pDevice)
		return CSL_PCM_ERR_HANDLE;

	if (channel == CSL_PCM_CHAN_TX0) {
		chal_sspi_fifo_reset(pDevice, SSPI_FIFO_ID_TX0);
		chal_sspi_enable_fifo_pio_start_stop_intr(pDevice,
							  SSPI_FIFO_ID_TX0,
							  TRUE, TRUE);
	} else if (channel == CSL_PCM_CHAN_TX1) {
		chal_sspi_fifo_reset(pDevice, SSPI_FIFO_ID_TX1);
		chal_sspi_enable_fifo_pio_start_stop_intr(pDevice,
							  SSPI_FIFO_ID_TX1,
							  TRUE, TRUE);
	}

	return CSL_PCM_SUCCESS;
}

/*
 *
 *  Function Name:      csl_pcm_start_rx
 *
 *  Description:        This function starts receive operation
 *
 */
CSL_PCM_OPSTATUS_t csl_pcm_start_rx(CSL_PCM_HANDLE handle, UInt8 channel)
{
	CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *) handle;

	aTrace(LOG_AUDIO_CSL,
			"csl_pcm_start_rx:: handle %p, channel %d.\r\n", handle,
			channel);
	if (!pDevice)
		return CSL_PCM_ERR_HANDLE;

	if (channel == CSL_PCM_CHAN_RX0) {
		chal_sspi_fifo_reset(pDevice, SSPI_FIFO_ID_RX0);
		chal_sspi_enable_fifo_pio_start_stop_intr(pDevice,
							  SSPI_FIFO_ID_RX0,
							  TRUE, TRUE);
	} else if (channel == CSL_PCM_CHAN_RX1) {
		chal_sspi_fifo_reset(pDevice, SSPI_FIFO_ID_RX1);
		chal_sspi_enable_fifo_pio_start_stop_intr(pDevice,
							  SSPI_FIFO_ID_RX1,
							  TRUE, TRUE);
	}
	return CSL_PCM_SUCCESS;
}

/*
 *
 *  Function Name:      csl_pcm_stop_tx
 *
 *  Description:        This function stops transmit operation
 *
 */
CSL_PCM_OPSTATUS_t csl_pcm_stop_tx(CSL_PCM_HANDLE handle, UInt8 channel)
{
	CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *) handle;

	aTrace(LOG_AUDIO_CSL, "%s handle %p, chan %d\n",
		__func__, handle, channel);
	if (!pDevice)
		return CSL_PCM_ERR_HANDLE;

	if (channel == CSL_PCM_CHAN_TX0) {
		chal_sspi_fifo_reset(pDevice, SSPI_FIFO_ID_TX0);
		chal_sspi_enable_fifo_pio_start_stop_intr(pDevice,
							  SSPI_FIFO_ID_TX0,
							  FALSE, FALSE);
	} else if (channel == CSL_PCM_CHAN_TX1) {
		chal_sspi_fifo_reset(pDevice, SSPI_FIFO_ID_TX1);
		chal_sspi_enable_fifo_pio_start_stop_intr(pDevice,
							  SSPI_FIFO_ID_TX1,
							  FALSE, FALSE);
	}

	return CSL_PCM_SUCCESS;
}

/*
 *
 *  Function Name:      csl_pcm_stop_rx
 *
 *  Description:        This function stops receive operation
 *
 */
CSL_PCM_OPSTATUS_t csl_pcm_stop_rx(CSL_PCM_HANDLE handle, UInt8 channel)
{
	CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *) handle;

	aTrace(LOG_AUDIO_CSL, "%s handle %p, chan %d\n",
		__func__, handle, channel);

	if (!pDevice)
		return CSL_PCM_ERR_HANDLE;

	if (channel == CSL_PCM_CHAN_RX0) {
		chal_sspi_fifo_reset(pDevice, SSPI_FIFO_ID_RX0);
		chal_sspi_enable_fifo_pio_start_stop_intr(pDevice,
							  SSPI_FIFO_ID_RX0,
							  FALSE, FALSE);
	} else if (channel == CSL_PCM_CHAN_RX1) {
		chal_sspi_fifo_reset(pDevice, SSPI_FIFO_ID_RX1);
		chal_sspi_enable_fifo_pio_start_stop_intr(pDevice,
							  SSPI_FIFO_ID_RX1,
							  FALSE, FALSE);
	}
	return CSL_PCM_SUCCESS;
}

CSL_PCM_OPSTATUS_t csl_pcm_pause(CSL_PCM_HANDLE handle)
{
	CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *) handle;
	CHAL_SSPI_STATUS_t status;

	/*disable scheduler operation */
	status = chal_sspi_enable_scheduler(pDevice, 0);

	if (status == CHAL_SSPI_STATUS_SUCCESS)
		return CSL_PCM_SUCCESS;
	else if (status == CHAL_SSPI_STATUS_ILLEGAL_HANDLE)
		return CSL_PCM_ERR_HANDLE;
	return CSL_PCM_SUCCESS;
}

CSL_PCM_OPSTATUS_t csl_pcm_resume(CSL_PCM_HANDLE handle)
{
	CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *) handle;
	CHAL_SSPI_STATUS_t status;

	/*enable scheduler operation */
	status = chal_sspi_enable_scheduler(pDevice, 1);

	if (status == CHAL_SSPI_STATUS_SUCCESS)
		return CSL_PCM_SUCCESS;
	else if (status == CHAL_SSPI_STATUS_ILLEGAL_HANDLE)
		return CSL_PCM_ERR_HANDLE;
	return CSL_PCM_SUCCESS;
}

/*
 *
 *  Function Name:      csl_pcm_config
 *
 *  Description:        This function configures SSPI as PCM operation
 *
 */
CSL_PCM_OPSTATUS_t csl_pcm_config(CSL_PCM_HANDLE handle,
				  csl_pcm_config_device_t *configDev,
				  csl_pcm_config_tx_t *configTx,
				  csl_pcm_config_rx_t *configRx)
{
	CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *) handle;
	uint32_t frmMask = SSPI_HW_FRAME0_MASK;
	CHAL_SSPI_PROT_t protocol;
	csl_pcm_config_device_t *devCfg = configDev;
	uint32_t intrMask;

	aTrace
	      (LOG_AUDIO_CSL, "csl_pcm_config:: handle %p.\r\n", handle);
	aTrace
	      (LOG_AUDIO_CSL,
	       "csl_pcm_config:: cfgDev mode %d interleave %d protocol %d"
	       "format %d size %ld bits %ld sr %ld.\r\n",
	       configDev->mode, configDev->interleave, configDev->protocol,
	       configDev->format, configDev->xferSize, configDev->ext_bits,
	       configDev->sample_rate);
	aTrace
	      (LOG_AUDIO_CSL,
	       "csl_pcm_config:: cfgTx ena %d ch %d sr %d lpbEna %d.\r\n",
	       configTx->enable, configTx->channel, configTx->sampleRate,
	       configTx->loopback_enable);
	aTrace
	      (LOG_AUDIO_CSL,
	       "csl_pcm_config:: cfgRx ena %d ch %d sr %d lpbEna %d.\r\n",
	       configRx->enable, configRx->channel, configRx->sampleRate,
	       configRx->loopback_enable);

	if (handle == NULL) {
		aError("csl_pcm_config failed\r\n");
		return CSL_PCM_ERR_HANDLE;
	}

	if ((devCfg->protocol == CSL_PCM_PROTOCOL_MONO) &&
	    (devCfg->format == CSL_PCM_WORD_LENGTH_16_BIT
	     || devCfg->format == CSL_PCM_WORD_LENGTH_PACK_16_BIT))
		protocol = SSPI_PROT_MONO_16B_PCM;
	else if ((devCfg->protocol == CSL_PCM_PROTOCOL_MONO) &&
		 (devCfg->format == CSL_PCM_WORD_LENGTH_24_BIT))
		protocol = SSPI_PROT_MONO_25B_PCM;
	else if ((devCfg->protocol == CSL_PCM_PROTOCOL_STEREO) &&
		 (devCfg->format == CSL_PCM_WORD_LENGTH_16_BIT))
		protocol = SSPI_PROT_STEREO_16B_PCM;
	else if ((devCfg->protocol == CSL_PCM_PROTOCOL_STEREO) &&
		 (devCfg->format == CSL_PCM_WORD_LENGTH_24_BIT))
		protocol = SSPI_PROT_STEREO_25B_PCM;
	else if ((devCfg->protocol == CSL_PCM_PROTOCOL_3CHANNEL) &&
		 (devCfg->format == CSL_PCM_WORD_LENGTH_16_BIT)) {
		protocol = SSPI_PROT_3CHAN_16B_TDM_PCM;
		frmMask = SSPI_HW_FRAME0_MASK | SSPI_HW_FRAME1_MASK;
	} else if ((devCfg->protocol == CSL_PCM_PROTOCOL_4CHANNEL) &&
		   (devCfg->format == CSL_PCM_WORD_LENGTH_16_BIT)) {
		protocol = SSPI_PROT_4CHAN_16B_TDM_PCM;
		frmMask = SSPI_HW_FRAME0_MASK | SSPI_HW_FRAME1_MASK;
	}
	/*only 16B case was tested for the following 2 protocols */
	else if ((devCfg->protocol == CSL_PCM_PROTOCOL_INTERLEAVE_3CHANNEL) &&
		 ((devCfg->format == CSL_PCM_WORD_LENGTH_16_BIT) ||
		 /* dsp changed to use 16bit pack data */
		 (devCfg->format == CSL_PCM_WORD_LENGTH_PACK_16_BIT))) {
		protocol = SSPI_PROT_STEREO_16B_PCM;
		frmMask = SSPI_HW_FRAME0_MASK | SSPI_HW_FRAME1_MASK;
	} else if ((devCfg->protocol == CSL_PCM_PROTOCOL_INTERLEAVE_3CHANNEL) &&
		   (devCfg->format == CSL_PCM_WORD_LENGTH_24_BIT)) {
		protocol = SSPI_PROT_STEREO_25B_PCM;
		frmMask = SSPI_HW_FRAME0_MASK | SSPI_HW_FRAME1_MASK;
	} else if ((devCfg->protocol == CSL_PCM_PROTOCOL_INTERLEAVE_4CHANNEL) &&
		   (devCfg->format == CSL_PCM_WORD_LENGTH_16_BIT)) {
		protocol = SSPI_PROT_STEREO_16B_PCM;
		frmMask = SSPI_HW_FRAME0_MASK | SSPI_HW_FRAME1_MASK;
	} else if ((devCfg->protocol == CSL_PCM_PROTOCOL_INTERLEAVE_4CHANNEL) &&
		   (devCfg->format == CSL_PCM_WORD_LENGTH_24_BIT)) {
		protocol = SSPI_PROT_STEREO_25B_PCM;
		frmMask = SSPI_HW_FRAME0_MASK | SSPI_HW_FRAME1_MASK;
	} else
		return CSL_PCM_ERR_PROT;
	/*task_conf struct initialization */
	memset(&task_conf, 0, sizeof(task_conf));

	csl_pcm_reset(pDevice);

	/*soft reset sspi instance */
	chal_sspi_soft_reset(pDevice);

	chal_sspi_get_intr_mask(pDevice, &intrMask);
	aTrace(LOG_AUDIO_CSL, "csl_pcm_config:: intrMask1 0x%x.\r\n",
			intrMask);

	if (devCfg->format == CSL_PCM_WORD_LENGTH_PACK_16_BIT) {
		/*intrMask |= (SSPIL_INTR_ENABLE_PIO_TX_START |
		   SSPIL_INTR_ENABLE_PIO_TX_STOP |
		   SSPIL_INTR_ENABLE_PIO_RX_START |
		   SSPIL_INTR_ENABLE_PIO_RX_STOP); */
		intrMask |=
		    (SSPIL_INTR_ENABLE_PIO_TX_START |
		     SSPIL_INTR_ENABLE_PIO_RX_START);
	} else {		/*for voice call only enable RX START */
		intrMask |= SSPIL_INTR_ENABLE_PIO_RX_START;
	}
	aTrace(LOG_AUDIO_CSL, "csl_pcm_config:: intrMask2 0x%x.\r\n",
			intrMask);
	/*need to disable all other interrupts to avoid confusing
	  dsp 03-02-11 */
	chal_sspi_enable_intr(pDevice, intrMask & 0x000000F0);

	/*set sspi at idle state */
	if (chal_sspi_set_idle_state(pDevice, protocol)) {
		aError("csl_pcm_config failed \r\n");
		return CSL_PCM_ERROR;
	}

	chal_sspi_set_clk_divider(handle, SSPI_CLK_DIVIDER0, 0);
	chal_sspi_set_clk_divider(handle, SSPI_CLK_REF_DIVIDER, 0);
	chal_sspi_set_clk_src_select(handle, SSPI_CLK_SRC_CAPHCLK);

	if (devCfg->protocol == CSL_PCM_PROTOCOL_INTERLEAVE_3CHANNEL) {
		chal_sspi_set_caph_clk(handle, SSPI_CAPH_CLK_TRIG_48kHz,
				       SSPI_HW_WORD_LEN_16Bit, 3);
		if (configDev->sample_rate == 8000) {
			chal_sspi_set_fifo_repeat_count(handle,
							SSPI_FIFO_ID_RX0, 5);
			chal_sspi_set_fifo_repeat_count(handle,
							SSPI_FIFO_ID_TX0, 5);
		} else if (configDev->sample_rate == 16000) {
			chal_sspi_set_fifo_repeat_count(handle,
							SSPI_FIFO_ID_RX0, 2);
			chal_sspi_set_fifo_repeat_count(handle,
							SSPI_FIFO_ID_TX0, 2);
		}
	} else {
		/*standard PCM */
		if (configDev->sample_rate == 8000) {
			chal_sspi_set_caph_clk(handle, SSPI_CAPH_CLK_TRIG_8kHz,
					       SSPI_HW_WORD_LEN_16Bit, 1);
		} else if (configDev->sample_rate == 16000) {
			chal_sspi_set_caph_clk(handle, SSPI_CAPH_CLK_TRIG_16kHz,
					       SSPI_HW_WORD_LEN_16Bit, 1);
		}
		chal_sspi_set_fifo_repeat_count(handle, SSPI_FIFO_ID_RX0, 0);
		chal_sspi_set_fifo_repeat_count(handle, SSPI_FIFO_ID_TX0, 0);
	}

	/*to avoid both coverity and compiler warnings,
	 * can not use switch here.
	 */
	if (protocol == SSPI_PROT_MONO_16B_PCM) {
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

		if (devCfg->format == CSL_PCM_WORD_LENGTH_PACK_16_BIT) {
			chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_RX0,
						SSPI_FIFO_DATA_PACK_16BIT);
			chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_RX1,
						SSPI_FIFO_DATA_PACK_16BIT);
			chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_TX0,
						SSPI_FIFO_DATA_PACK_16BIT);
			chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_TX1,
						SSPI_FIFO_DATA_PACK_16BIT);
		} else {
			chal_sspi_set_fifo_pack(pDevice,
						SSPI_FIFO_ID_RX0,
						SSPI_FIFO_DATA_PACK_NONE);
			chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_RX1,
						SSPI_FIFO_DATA_PACK_NONE);
			chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_TX0,
						SSPI_FIFO_DATA_PACK_NONE);
			chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_TX1,
						SSPI_FIFO_DATA_PACK_NONE);
		}
	} else if (protocol == SSPI_PROT_MONO_25B_PCM) {
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
					SSPI_FIFO_ID_RX0,
					SSPI_FIFO_DATA_PACK_NONE);
		chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_RX1,
					SSPI_FIFO_DATA_PACK_NONE);
		chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_TX0,
					SSPI_FIFO_DATA_PACK_NONE);
		chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_TX1,
					SSPI_FIFO_DATA_PACK_NONE);

	} else if (protocol == SSPI_PROT_STEREO_16B_PCM) {
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

		/* for pcm slot, dsp has different requirement of data format */
		if (devCfg->format == CSL_PCM_WORD_LENGTH_PACK_16_BIT) {
			chal_sspi_set_fifo_pack(pDevice,
						SSPI_FIFO_ID_RX0,
						SSPI_FIFO_DATA_PACK_16BIT);
			chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_TX0,
						SSPI_FIFO_DATA_PACK_16BIT);
		} else {
			chal_sspi_set_fifo_pack(pDevice,
					SSPI_FIFO_ID_RX0,
					SSPI_FIFO_DATA_PACK_NONE);
			chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_TX0,
					SSPI_FIFO_DATA_PACK_NONE);
		}
		/* for fm slot, data is always packed */
		chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_RX1,
					SSPI_FIFO_DATA_PACK_16BIT);
		chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_TX1,
					SSPI_FIFO_DATA_PACK_16BIT);

	} else if (protocol == SSPI_PROT_STEREO_25B_PCM) {
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
					SSPI_FIFO_ID_RX0,
					SSPI_FIFO_DATA_PACK_NONE);
		chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_RX1,
					SSPI_FIFO_DATA_PACK_16BIT);
		chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_TX0,
					SSPI_FIFO_DATA_PACK_NONE);
		chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_TX1,
					SSPI_FIFO_DATA_PACK_16BIT);

	} else if (protocol == SSPI_PROT_3CHAN_16B_TDM_PCM) {
		chal_sspi_set_fifo_size(pDevice,
					SSPI_FIFO_ID_RX0, SSPI_FIFO_SIZE_HALF);
		chal_sspi_set_fifo_size(pDevice,
					SSPI_FIFO_ID_RX1,
					SSPI_FIFO_SIZE_QUARTER);
		chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_RX2,
					SSPI_FIFO_SIZE_QUARTER);
		chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_RX3,
					SSPI_FIFO_SIZE_NONE);
		chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_TX0,
					SSPI_FIFO_SIZE_HALF);
		chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_TX1,
					SSPI_FIFO_SIZE_QUARTER);
		chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_TX2,
					SSPI_FIFO_SIZE_QUARTER);
		chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_TX3,
					SSPI_FIFO_SIZE_NONE);

		chal_sspi_set_fifo_pack(pDevice,
					SSPI_FIFO_ID_RX0,
					SSPI_FIFO_DATA_PACK_16BIT);
		chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_RX1,
					SSPI_FIFO_DATA_PACK_16BIT);
		chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_RX2,
					SSPI_FIFO_DATA_PACK_16BIT);
		chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_TX0,
					SSPI_FIFO_DATA_PACK_16BIT);
		chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_TX1,
					SSPI_FIFO_DATA_PACK_16BIT);
		chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_TX2,
					SSPI_FIFO_DATA_PACK_16BIT);

	} else if (protocol == SSPI_PROT_4CHAN_16B_TDM_PCM) {
		chal_sspi_set_fifo_size(pDevice,
					SSPI_FIFO_ID_RX0,
					SSPI_FIFO_SIZE_QUARTER);
		chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_RX1,
					SSPI_FIFO_SIZE_QUARTER);
		chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_RX2,
					SSPI_FIFO_SIZE_QUARTER);
		chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_RX3,
					SSPI_FIFO_SIZE_QUARTER);
		chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_TX0,
					SSPI_FIFO_SIZE_QUARTER);
		chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_TX1,
					SSPI_FIFO_SIZE_QUARTER);
		chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_TX2,
					SSPI_FIFO_SIZE_QUARTER);
		chal_sspi_set_fifo_size(pDevice, SSPI_FIFO_ID_TX3,
					SSPI_FIFO_SIZE_QUARTER);

		chal_sspi_set_fifo_pack(pDevice,
					SSPI_FIFO_ID_RX0,
					SSPI_FIFO_DATA_PACK_16BIT);
		chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_RX1,
					SSPI_FIFO_DATA_PACK_16BIT);
		chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_RX2,
					SSPI_FIFO_DATA_PACK_16BIT);
		chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_RX3,
					SSPI_FIFO_DATA_PACK_16BIT);
		chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_TX0,
					SSPI_FIFO_DATA_PACK_16BIT);
		chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_TX1,
					SSPI_FIFO_DATA_PACK_16BIT);
		chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_TX2,
					SSPI_FIFO_DATA_PACK_16BIT);
		chal_sspi_set_fifo_pack(pDevice, SSPI_FIFO_ID_TX3,
					SSPI_FIFO_DATA_PACK_16BIT);
	}
	/*SSPI mode configuration */
	chal_sspi_set_mode(pDevice, (CHAL_SSPI_MODE_t) (devCfg->mode));

	/*enable sspi operation */
	chal_sspi_enable(pDevice, 1);

	/* setting from asic team */
	/*config for audio mode, may divert from voice settings. */
	if (devCfg->format == CSL_PCM_WORD_LENGTH_PACK_16_BIT) {
		chal_sspi_set_fifo_pio_threshhold(pDevice, SSPI_FIFO_ID_RX0,
						  0x1c, 0x3);
		chal_sspi_set_fifo_pio_threshhold(pDevice, SSPI_FIFO_ID_TX0,
						  0x3, 0x1c);
	} else {		/*voice mode */
		/*chal_sspi_set_fifo_pio_threshhold
		 * (pDevice, SSPI_FIFO_ID_RX0, 0xf, 0x1);
		 */
		/*chal_sspi_set_fifo_pio_threshhold(pDevice, SSPI_FIFO_ID_TX0,
		 * 0x1, 0x1); this requires bigger CFIFO size, does not work
		 * well with SRC either.
		 */
		/*0x1f/0x3 combo does not work for DMA-based BT call:
		 * DSP keeps sending unknown interrupts.
		 */
		/*chal_sspi_set_fifo_pio_threshhold(pDevice,
		 * SSPI_FIFO_ID_RX0, 0x1f, 0x3); SSPI send out RX_Start
		 * interrupt when there is 1 sample in RX0 fifo and Rx_stop
		 * interrupt when there are 3 samples in the RX0 fifo.
		 */
		/*SSPI send out RX_Start interrupt when there is 4 sample in RX0
		 * fifo and Rx_stop interrupt when there are 3 samples in the
		 * RX0 fifo.
		 */
		chal_sspi_set_fifo_pio_threshhold(pDevice, SSPI_FIFO_ID_RX0,
						  0x1c, 0x3);
		/*SSPI send out TX_Start interrupt when there are less 3
		 * samples in TX0 fifo and Tx_stop interrupt when there
		 * are 4 samples in the TX0 fifo.
		 */
		chal_sspi_set_fifo_pio_threshhold(pDevice, SSPI_FIFO_ID_TX0,
						  0x3, 0x1c);
	}

	if (devCfg->protocol == CSL_PCM_PROTOCOL_INTERLEAVE_3CHANNEL) {
		chal_sspi_set_fifo_pio_threshhold(pDevice, SSPI_FIFO_ID_RX0,
						  0xc, 0x3);
		chal_sspi_set_fifo_pio_threshhold(pDevice, SSPI_FIFO_ID_TX0,
						  0x3, 0xc);
	}

	chal_sspi_set_fifo_pio_threshhold(pDevice, SSPI_FIFO_ID_RX1, 0x3, 0x3);
	chal_sspi_set_fifo_pio_threshhold(pDevice, SSPI_FIFO_ID_TX1, 0x3, 0x3);

	switch (protocol) {
	case SSPI_PROT_MONO_16B_PCM:
	case SSPI_PROT_MONO_25B_PCM:
		if (configTx->enable)
			/*chal_sspi_set_fifo_threshold(pDevice,
			 * SSPI_FIFO_ID_TX0, 0x10);
			 */
			chal_sspi_set_fifo_threshold(pDevice, SSPI_FIFO_ID_TX0,
						     0x00);
		task_conf.chan_sel = SSPI_CHAN_SEL_CHAN0;
		task_conf.cs_sel = SSPI_CS_SEL_CS0;
		task_conf.rx_sel = (configTx->loopback_enable)
		    ? SSPI_RX_SEL_COPY_TX0 : SSPI_RX_SEL_RX0;
		/*task_conf.rx_sel = SSPI_RX_SEL_COPY_TX0; */
		/*loopback tx to rx */
		task_conf.tx_sel = SSPI_TX_SEL_TX0;
		task_conf.div_sel = SSPI_CLK_DIVIDER0;
		task_conf.seq_ptr = 0;

		/*Max transfer size is set to 4K bytes in
		 * non_continuous mode transfer
		 */
		if ((devCfg->xferSize >> 2) > 0x400) {
			task_conf.loop_cnt = 0;
			task_conf.continuous = 1;
		} else {
			task_conf.loop_cnt = (devCfg->xferSize >> 2) - 1;
			task_conf.continuous = 0;
		}

		task_conf.init_cond_mask = (configTx->enable)
		    ? SSPI_TASK_INIT_COND_THRESHOLD_TX0 : 0;
		task_conf.wait_before_start = 1;
		if (chal_sspi_set_task(pDevice, 0, protocol, &task_conf))
			return CSL_PCM_ERR_TASK;

		seq_conf.tx_enable = (configTx->enable
				      || configRx->loopback_enable)
		    ? TRUE : FALSE;
		seq_conf.rx_enable = (configRx->enable
				      || configTx->loopback_enable)
		    ? TRUE : FALSE;
		seq_conf.cs_activate = 1;

		if (devCfg->mode == CSL_PCM_SLAVE_MODE)
			seq_conf.cs_deactivate = 0;
		else
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
		if (chal_sspi_set_sequence(pDevice, 0, protocol, &seq_conf))
			return CSL_PCM_ERR_SEQUENCE;

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
		if (chal_sspi_set_sequence(pDevice, 1, protocol, &seq_conf))
			return CSL_PCM_ERR_SEQUENCE;

		if (chal_sspi_set_frame(pDevice, &frmMask, protocol,
					(protocol ==
					 SSPI_PROT_MONO_25B_PCM) ? 24 : 16, 0))
			return CSL_PCM_ERR_FRAME;
		break;

	case SSPI_PROT_STEREO_16B_PCM:
	case SSPI_PROT_STEREO_25B_PCM:
		if (configTx->enable) {
			chal_sspi_set_fifo_threshold(pDevice, SSPI_FIFO_ID_TX0,
						     0x0);
			chal_sspi_set_fifo_threshold(pDevice, SSPI_FIFO_ID_TX1,
						     0x0);
		}

		task_conf.chan_sel = SSPI_CHAN_SEL_CHAN0;
		task_conf.cs_sel = SSPI_CS_SEL_CS0;
		task_conf.rx_sel = (configTx->loopback_enable)
		    ? SSPI_RX_SEL_COPY_TX0 : SSPI_RX_SEL_RX0;
		task_conf.tx_sel = SSPI_TX_SEL_TX0;
		task_conf.div_sel = SSPI_CLK_DIVIDER0;
		task_conf.seq_ptr = 0;

		/*max transfer size is set to 4K bytes
		 * in non_continuous mode transfer
		 */
		if ((devCfg->xferSize >> 1) > 0x400) {
			task_conf.loop_cnt = 0;
			task_conf.continuous = 1;
		} else {
			task_conf.loop_cnt = (devCfg->xferSize >> 1) - 1;
			task_conf.continuous = 0;
		}
		task_conf.init_cond_mask = (configTx->enable) ?
		    (SSPI_TASK_INIT_COND_THRESHOLD_TX0 |
		     SSPI_TASK_INIT_COND_THRESHOLD_TX1) : 0;
		task_conf.wait_before_start = 1;
		if (chal_sspi_set_task(pDevice, 0, protocol, &task_conf))
			return CSL_PCM_ERR_TASK;

		if (devCfg->protocol == CSL_PCM_PROTOCOL_STEREO) {
			seq_conf.tx_enable = (configTx->enable ||
					      configRx->
					      loopback_enable) ? TRUE : FALSE;
			seq_conf.rx_enable = (configRx->enable
					      || configTx->
					      loopback_enable) ? TRUE : FALSE;
			seq_conf.cs_activate = 1;
			seq_conf.cs_deactivate = 1;
			seq_conf.pattern_mode = 0;
			seq_conf.rep_cnt = 0;
			seq_conf.opcode = SSPI_SEQ_OPCODE_NEXT_PC;
			seq_conf.rx_fifo_sel = 0;
			seq_conf.tx_fifo_sel = 0;
			/*seq_conf.frm_sel = 0; */
			seq_conf.frm_sel = 1;
			seq_conf.rx_sidetone_on = 0;
			seq_conf.tx_sidetone_on = 0;
			seq_conf.next_pc = 0;
			if (chal_sspi_set_sequence
			    (pDevice, 0, protocol, &seq_conf))
				return CSL_PCM_ERR_SEQUENCE;

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
			if (chal_sspi_set_sequence
			    (pDevice, 1, protocol, &seq_conf))
				return CSL_PCM_ERR_SEQUENCE;

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
			if (chal_sspi_set_sequence
			    (pDevice, 2, protocol, &seq_conf))
				return CSL_PCM_ERR_SEQUENCE;
		} else if (devCfg->protocol ==
			   CSL_PCM_PROTOCOL_INTERLEAVE_3CHANNEL) {
			seq_conf.tx_enable = (configTx->enable
					      || configRx->loopback_enable)
			    ? 1 : 0;
			seq_conf.rx_enable = (configRx->enable
					      || configTx->loopback_enable)
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
			if (chal_sspi_set_sequence
			    (handle, 0, protocol, &seq_conf))
				return CSL_PCM_ERR_SEQUENCE;

			seq_conf.tx_enable = (configTx->enable
					      || configRx->loopback_enable)
			    ? 1 : 0;
			seq_conf.rx_enable = (configRx->enable
					      || configTx->loopback_enable)
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
			if (chal_sspi_set_sequence
			    (handle, 1, protocol, &seq_conf))
				return CSL_PCM_ERR_SEQUENCE;

			seq_conf.tx_enable = (configTx->enable ||
					      configRx->
					      loopback_enable) ? 1 : 0;
			seq_conf.rx_enable = (configRx->enable
					      || configTx->
					      loopback_enable) ? 1 : 0;
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
			if (chal_sspi_set_sequence
			    (handle, 2, protocol, &seq_conf))
				return CSL_PCM_ERR_SEQUENCE;

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
			if (chal_sspi_set_sequence
			    (handle, 3, protocol, &seq_conf))
				return CSL_PCM_ERR_SEQUENCE;
			/* pCore->prot ==
			   SSPI_HW_INTERLEAVE_NOKIA_PCM_4CHANNEL */
		} else if (devCfg->protocol ==
			   CSL_PCM_PROTOCOL_INTERLEAVE_4CHANNEL) {
			seq_conf.tx_enable = (configTx->enable
					      || configRx->loopback_enable)
			    ? 1 : 0;
			seq_conf.rx_enable = (configRx->enable
					      || configTx->loopback_enable)
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
			if (chal_sspi_set_sequence
			    (handle, 0, protocol, &seq_conf))
				return CSL_PCM_ERR_SEQUENCE;

			seq_conf.tx_enable = (configTx->enable
					      || configRx->loopback_enable)
			    ? 1 : 0;
			seq_conf.rx_enable = (configRx->enable
					      || configTx->loopback_enable)
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
			if (chal_sspi_set_sequence
			    (handle, 1, protocol, &seq_conf))
				return CSL_PCM_ERR_SEQUENCE;

			seq_conf.tx_enable = (configTx->enable
					      || configRx->loopback_enable)
			    ? 1 : 0;
			seq_conf.rx_enable = (configRx->enable
					      || configTx->loopback_enable)
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
			if (chal_sspi_set_sequence
			    (handle, 2, protocol, &seq_conf))
				return CSL_PCM_ERR_SEQUENCE;

			seq_conf.tx_enable = (configTx->enable ||
					      configRx->
					      loopback_enable) ? 1 : 0;
			seq_conf.rx_enable = (configRx->enable
					      || configTx->
					      loopback_enable) ? 1 : 0;
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
			if (chal_sspi_set_sequence
			    (handle, 3, protocol, &seq_conf))
				return CSL_PCM_ERR_SEQUENCE;

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
			if (chal_sspi_set_sequence
			    (handle, 4, protocol, &seq_conf))
				return CSL_PCM_ERR_SEQUENCE;
		}

		/* should we use 25bit here??? */
		if (0 /*devCfg->format == CSL_PCM_WORD_LENGTH_24_BIT */) {
			/*if(chal_sspi_set_frame(pDevice, &frmMask, protocol,
			 * SSPI_HW_WORD_LEN_16Bit, SSPI_HW_DUMMY_BITS_9))
			 */
			if (chal_sspi_set_frame(pDevice, &frmMask, protocol,
						SSPI_HW_WORD_LEN_24Bit, 0))
				return CSL_PCM_ERR_FRAME;
		} else {
			/*to make it work as expected, need to use
			 * these settings before set frame
			 */
			if (devCfg->protocol ==
			    CSL_PCM_PROTOCOL_INTERLEAVE_3CHANNEL)
				/*to be used with new chal code */
			{
				protocol = SSPI_PROT_3CHAN_16B_TDM_PCM;
				devCfg->ext_bits = 0;
			}

			else if (devCfg->protocol ==
				 CSL_PCM_PROTOCOL_INTERLEAVE_4CHANNEL) {
				protocol = SSPI_PROT_4CHAN_16B_TDM_PCM;
				devCfg->ext_bits = 1;
			} else {
				protocol = SSPI_PROT_STEREO_16B_PCM;
				devCfg->ext_bits = 0;
			}

			if (chal_sspi_set_frame(pDevice, &frmMask, protocol,
						SSPI_HW_WORD_LEN_16Bit,
						devCfg->ext_bits))
				return CSL_PCM_ERR_FRAME;
		}
		break;

	case SSPI_PROT_3CHAN_16B_TDM_PCM:
		if (configTx->enable) {
			chal_sspi_set_fifo_threshold(handle, SSPI_FIFO_ID_TX0,
						     0x10);
			chal_sspi_set_fifo_threshold(handle, SSPI_FIFO_ID_TX1,
						     0x10);
			chal_sspi_set_fifo_threshold(handle, SSPI_FIFO_ID_TX2,
						     0x10);
		}

		task_conf.chan_sel = SSPI_CHAN_SEL_CHAN0;
		task_conf.cs_sel = SSPI_CS_SEL_CS0;
		task_conf.rx_sel =
		    (configTx->
		     loopback_enable) ? SSPI_RX_SEL_COPY_TX0 : SSPI_RX_SEL_RX0;
		task_conf.tx_sel = SSPI_TX_SEL_TX0;
		task_conf.div_sel = SSPI_CLK_DIVIDER0;
		task_conf.seq_ptr = 0;
		task_conf.init_cond_mask = SSPI_TASK_INIT_COND_THRESHOLD_TX0 |
		    SSPI_TASK_INIT_COND_THRESHOLD_TX1 |
		    SSPI_TASK_INIT_COND_THRESHOLD_TX2;

		if ((devCfg->xferSize >> 1) > 0x400) {
			task_conf.loop_cnt = 0;
			task_conf.continuous = 1;
		} else {
			task_conf.loop_cnt = (devCfg->xferSize >> 1) - 1;
			task_conf.continuous = 0;
		}
		task_conf.init_cond_mask = (configTx->enable) ?
		    (SSPI_TASK_INIT_COND_THRESHOLD_TX0 |
		     SSPI_TASK_INIT_COND_THRESHOLD_TX1 |
		     SSPI_TASK_INIT_COND_THRESHOLD_TX2) : 0;

		task_conf.wait_before_start = 1;
		if (chal_sspi_set_task(handle, 0, protocol, &task_conf))
			return CSL_PCM_ERR_TASK;

		seq_conf.tx_enable = (configTx->enable
				      || configRx->loopback_enable)
		    ? 1 : 0;
		seq_conf.rx_enable = (configRx->enable
				      || configTx->loopback_enable)
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
		if (chal_sspi_set_sequence(handle, 0, protocol, &seq_conf))
			return CSL_PCM_ERR_SEQUENCE;

		seq_conf.tx_enable = (configTx->enable
				      || configRx->loopback_enable)
		    ? 1 : 0;
		seq_conf.rx_enable = (configRx->enable
				      || configTx->loopback_enable)
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
		if (chal_sspi_set_sequence(handle, 1, protocol, &seq_conf))
			return CSL_PCM_ERR_SEQUENCE;

		if (!devCfg->ext_bits) {
			seq_conf.tx_enable = (configTx->enable
					      || configRx->loopback_enable)
			    ? 1 : 0;
			seq_conf.rx_enable = (configRx->enable
					      || configTx->loopback_enable)
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
			if (chal_sspi_set_sequence
			    (handle, 2, protocol, &seq_conf))
				return CSL_PCM_ERR_SEQUENCE;

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
			if (chal_sspi_set_sequence
			    (handle, 3, protocol, &seq_conf))
				return CSL_PCM_ERR_SEQUENCE;
		} else {
			seq_conf.tx_enable = (configTx->enable
					      || configRx->loopback_enable)
			    ? 1 : 0;
			seq_conf.rx_enable = (configRx->enable
					      || configTx->loopback_enable)
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
			if (chal_sspi_set_sequence
			    (handle, 2, protocol, &seq_conf))
				return CSL_PCM_ERR_SEQUENCE;

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
			if (chal_sspi_set_sequence
			    (handle, 3, protocol, &seq_conf))
				return CSL_PCM_ERR_SEQUENCE;

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
			if (chal_sspi_set_sequence
			    (handle, 4, protocol, &seq_conf))
				return CSL_PCM_ERR_SEQUENCE;

		}

		if (chal_sspi_set_frame(handle, &frmMask, protocol, 16, 2))
			return CSL_PCM_ERR_FRAME;

		break;
	case SSPI_PROT_4CHAN_16B_TDM_PCM:
		if (configTx->enable) {
			chal_sspi_set_fifo_threshold(handle, SSPI_FIFO_ID_TX0,
						     0x10);
			chal_sspi_set_fifo_threshold(handle, SSPI_FIFO_ID_TX1,
						     0x10);
			chal_sspi_set_fifo_threshold(handle, SSPI_FIFO_ID_TX2,
						     0x10);
			chal_sspi_set_fifo_threshold(handle, SSPI_FIFO_ID_TX3,
						     0x10);
		}

		task_conf.chan_sel = SSPI_CHAN_SEL_CHAN0;
		task_conf.cs_sel = SSPI_CS_SEL_CS0;
		task_conf.rx_sel =
		    (configTx->
		     loopback_enable) ? SSPI_RX_SEL_COPY_TX0 : SSPI_RX_SEL_RX0;
		task_conf.tx_sel = SSPI_TX_SEL_TX0;
		task_conf.div_sel = SSPI_CLK_DIVIDER0;
		task_conf.seq_ptr = 0;

		if ((devCfg->xferSize >> 1) > 0x400) {
			task_conf.loop_cnt = 0;
			task_conf.continuous = 1;
		} else {
			task_conf.loop_cnt = (devCfg->xferSize >> 1) - 1;
			task_conf.continuous = 0;
		}
		task_conf.init_cond_mask = (configTx->enable) ?
		    (SSPI_TASK_INIT_COND_THRESHOLD_TX0 |
		     SSPI_TASK_INIT_COND_THRESHOLD_TX1 |
		     SSPI_TASK_INIT_COND_THRESHOLD_TX2 |
		     SSPI_TASK_INIT_COND_THRESHOLD_TX3) : 0;

		task_conf.wait_before_start = 1;
		if (chal_sspi_set_task(handle, 0, protocol, &task_conf))
			return CSL_PCM_ERR_TASK;

		seq_conf.tx_enable = (configTx->enable
				      || configRx->loopback_enable)
		    ? 1 : 0;
		seq_conf.rx_enable = (configRx->enable
				      || configTx->loopback_enable)
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
		if (chal_sspi_set_sequence(handle, 0, protocol, &seq_conf))
			return CSL_PCM_ERR_SEQUENCE;

		seq_conf.tx_enable = (configTx->enable
				      || configRx->loopback_enable)
		    ? 1 : 0;
		seq_conf.rx_enable = (configRx->enable
				      || configTx->loopback_enable)
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
		if (chal_sspi_set_sequence(handle, 1, protocol, &seq_conf))
			return CSL_PCM_ERR_SEQUENCE;

		seq_conf.tx_enable = (configTx->enable
				      || configRx->loopback_enable)
		    ? 1 : 0;
		seq_conf.rx_enable = (configRx->enable
				      || configTx->loopback_enable)
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
		if (chal_sspi_set_sequence(handle, 2, protocol, &seq_conf))
			return CSL_PCM_ERR_SEQUENCE;

		if (!devCfg->ext_bits) {
			seq_conf.tx_enable = (configTx->enable
					      || configRx->loopback_enable)
			    ? 1 : 0;
			seq_conf.rx_enable = (configRx->enable
					      || configTx->loopback_enable)
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
			if (chal_sspi_set_sequence
			    (handle, 3, protocol, &seq_conf))
				return CSL_PCM_ERR_SEQUENCE;

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
			if (chal_sspi_set_sequence
			    (handle, 4, protocol, &seq_conf))
				return CSL_PCM_ERR_SEQUENCE;
		} else {
			seq_conf.tx_enable = (configTx->enable
					      || configRx->loopback_enable)
			    ? 1 : 0;
			seq_conf.rx_enable = (configRx->enable
					      || configTx->loopback_enable)
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
			if (chal_sspi_set_sequence
			    (handle, 3, protocol, &seq_conf))
				return CSL_PCM_ERR_SEQUENCE;

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
			if (chal_sspi_set_sequence
			    (handle, 4, protocol, &seq_conf))
				return CSL_PCM_ERR_SEQUENCE;

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
			if (chal_sspi_set_sequence
			    (handle, 5, protocol, &seq_conf))
				return CSL_PCM_ERR_SEQUENCE;

		}

		if (chal_sspi_set_frame(handle, &frmMask, protocol, 16, 1))
			return CSL_PCM_ERR_FRAME;

		break;
	default:
		break;
	}

	/*
	chal_sspi_enable_fifo_pio_start_stop_intr(pDevice, SSPI_FIFO_ID_TX0,
						  TRUE, TRUE);
	chal_sspi_enable_fifo_pio_start_stop_intr(pDevice, SSPI_FIFO_ID_TX1,
						  TRUE, TRUE);
	chal_sspi_enable_fifo_pio_start_stop_intr(pDevice, SSPI_FIFO_ID_TX2,
						  TRUE, TRUE);
	chal_sspi_enable_fifo_pio_start_stop_intr(pDevice, SSPI_FIFO_ID_TX3,
						  TRUE, TRUE);*/
	return CSL_PCM_SUCCESS;
}

/*
 *
 *  Function Name:      csl_pcm_get_tx0_fifo_data_port
 *
 *  Description:        This function get transmit channel fifo port address
 *
 */
UInt32 csl_pcm_get_tx0_fifo_data_port(CSL_PCM_HANDLE handle)
{
	CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *) handle;
	if (pDevice)
		return (UInt32) (pDevice->base + SSPIL_FIFO_ENTRY0TX_OFFSET);
	else
		return 0;
}

/*
 *
 *  Function Name:      csl_pcm_get_tx1_fifo_data_port
 *
 *  Description:        This function get receive channel fifo port address
 *
 */
UInt32 csl_pcm_get_tx1_fifo_data_port(CSL_PCM_HANDLE handle)
{
	CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *) handle;
	return (UInt32) (pDevice->base + SSPIL_FIFO_ENTRY1TX_OFFSET);
}

/*
 *
 *  Function Name:      csl_pcm_get_rx0_fifo_data_port
 *
 *  Description:        This function get receive channel fifo port address
 *
 */
UInt32 csl_pcm_get_rx0_fifo_data_port(CSL_PCM_HANDLE handle)
{
	CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *) handle;
	if (pDevice)
		return (UInt32) (pDevice->base + SSPIL_FIFO_ENTRY0RX_OFFSET);
	else
		return 0;
}

/*
*  Function Name:      csl_pcm_get_rx1_fifo_data_port
*
*  Description:        This function get receive channel fifo port address
*
*/
UInt32 csl_pcm_get_rx1_fifo_data_port(CSL_PCM_HANDLE handle)
{
	CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *) handle;
	return (UInt32) (pDevice->base + SSPIL_FIFO_ENTRY1RX_OFFSET);
}

/*
 *
 *  Function Name: void csl_caph_intc_enable_pcm_intr(
 *  CSL_CAPH_ARM_DSP_e csl_owner, CSL_CAPH_SSP_e csl_sspid)
 *
 *  Description: enable pcm intr
 *
 */
void csl_caph_intc_enable_pcm_intr(CSL_CAPH_ARM_DSP_e csl_owner,
				   CSL_CAPH_SSP_e csl_sspid)
{
	CAPH_ARM_DSP_e owner = CAPH_ARM;

	aTrace(LOG_AUDIO_CSL,
			"csl_caph_intc_enable_pcm_intr:: owner %d, ssp %d.\n",
			csl_owner, csl_sspid);

	if (csl_owner == CSL_CAPH_DSP)
		owner = CAPH_DSP;

	if (csl_sspid == CSL_CAPH_SSP_3)
		chal_caph_intc_enable_ssp_intr(intc_handle, 1, owner);
	else if (csl_sspid == CSL_CAPH_SSP_4)
		chal_caph_intc_enable_ssp_intr(intc_handle, 2, owner);
	else
		/*should not get here. */
		audio_xassert(0, csl_sspid);
}

/*
 *
 *  Function Name: void csl_caph_intc_disable_pcm_intr(
 *  CSL_CAPH_ARM_DSP_e csl_owner, CSL_CAPH_SSP_e csl_sspid)
 *
 *  Description: disable pcm intr
 *
 */
void csl_caph_intc_disable_pcm_intr(CSL_CAPH_ARM_DSP_e csl_owner,
				    CSL_CAPH_SSP_e csl_sspid)
{
	CAPH_ARM_DSP_e owner = CAPH_ARM;

	aTrace(LOG_AUDIO_CSL,
			"csl_caph_intc_disable_pcm_intr:: owner %d, ssp %d.\n",
			csl_owner, csl_sspid);

	if (csl_owner == CSL_CAPH_DSP)
		owner = CAPH_DSP;

	if (csl_sspid == CSL_CAPH_SSP_3)
		chal_caph_intc_disable_ssp_intr(intc_handle, 1, owner);
	else if (csl_sspid == CSL_CAPH_SSP_4)
		chal_caph_intc_disable_ssp_intr(intc_handle, 2, owner);
	else
		/*should not get here.*/
		audio_xassert(0, csl_sspid);
}
