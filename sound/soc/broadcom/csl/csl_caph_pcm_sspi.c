/******************************************************************************/
/*									*/
/* Copyright 2011  Broadcom Corporation					*/
/*									*/
/* Unless you and Broadcom execute a separate written software license	*/
/* agreement governing use of this software, this software is licensed to */
/* you under the terms of the GNU General Public License version 2 (the GPL), */
/* available at								*/
/*									*/
/*	http://www.broadcom.com/licenses/GPLv2.php			*/
/*									*/
/*	with the following added to such license:			*/
/*									 */
/* As a special exception, the copyright holders of this software give	*/
/* you permission to link this software with independent modules, and to */
/* copy and distribute the resulting executable under terms of your	  */
/* choice, provided that you also meet, for each linked independent	 */
/* module, the terms and conditions of the license of that module.	*/
/* An independent module is a module which is not derived from this	*/
/* software. The special exception does not apply to any modifications	*/
/* of the software.							*/
/*									*/
/* Notwithstanding the above, under no circumstances may you combine	*/
/* this software in any way with any other Broadcom software provided	*/
/* under a license other than the GPL, without Broadcom's express prior */
/* written	consent.						*/
/*									*/
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
#include "chal_types.h"
#include "chal_caph.h"
//#include <plat/chal/chal_sspi.h>
#include "chal_sspi_hawaii.h"
#include "chal_caph_intc.h"
#include "csl_caph.h"
#include "csl_caph_pcm_sspi.h"
#include "brcm_rdb_padctrlreg.h"
#include "brcm_rdb_sysmap.h"
#include "brcm_rdb_sspil.h"
#include "audio_trace.h"
#include <mach/rdb/brcm_rdb_khub_rst_mgr_reg.h>
#include <linux/io.h>
#include <mach/io_map.h>
#include <mach/memory.h>

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
#define MAX_TRANS_SIZE							4096

/******************************************************************************
* Local Definitions
******************************************************************************/
static chal_sspi_task_conf_t task_conf;
static chal_sspi_seq_conf_t seq_conf;
static CHAL_HANDLE intc_handle = 0x0;
static chal_sspi_frm_conf_t frm_arr[CHAL_SSPI_MAX_CHANNELS];
static chal_sspi_frm_conf_t frm_arr[CHAL_SSPI_MAX_CHANNELS];

static void set_all_fifo_size(CSL_PCM_HANDLE_t *pDevice, UInt32 num_fifo)
{
	switch (num_fifo) {
	case 1:
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
		break;

	case 2:
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
		break;

	case 3:
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
		break;

	case 4:
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
		break;

	default:
		chal_sspi_set_fifo_size(pDevice,
			SSPI_FIFO_ID_RX0, SSPI_FIFO_SIZE_NONE);
		chal_sspi_set_fifo_size(pDevice,
			SSPI_FIFO_ID_RX1, SSPI_FIFO_SIZE_NONE);
		chal_sspi_set_fifo_size(pDevice,
			SSPI_FIFO_ID_RX2, SSPI_FIFO_SIZE_NONE);
		chal_sspi_set_fifo_size(pDevice,
			SSPI_FIFO_ID_RX3, SSPI_FIFO_SIZE_NONE);
		chal_sspi_set_fifo_size(pDevice,
			SSPI_FIFO_ID_TX0, SSPI_FIFO_SIZE_NONE);
		chal_sspi_set_fifo_size(pDevice,
			SSPI_FIFO_ID_TX1, SSPI_FIFO_SIZE_NONE);
		chal_sspi_set_fifo_size(pDevice,
			SSPI_FIFO_ID_TX2, SSPI_FIFO_SIZE_NONE);
		chal_sspi_set_fifo_size(pDevice,
			SSPI_FIFO_ID_TX3, SSPI_FIFO_SIZE_NONE);
		break;
	}
}

static void set_fifo_pack(CSL_PCM_HANDLE handle, UInt32 fifo_num,
			UInt32 word_len, Boolean pack, Boolean rx)
{
	if (pack) {
		if (rx) {
			if (word_len <= 8) {
				chal_sspi_set_fifo_pack(handle,
					SSPI_FIFO_ID_RX0 + fifo_num,
					SSPI_FIFO_DATA_PACK_8BIT);
			} else if (word_len <= 16) {
				chal_sspi_set_fifo_pack(handle,
					SSPI_FIFO_ID_RX0 + fifo_num,
					SSPI_FIFO_DATA_PACK_16BIT);
			} else {
				chal_sspi_set_fifo_pack(handle,
					SSPI_FIFO_ID_RX0 + fifo_num,
					SSPI_FIFO_DATA_PACK_NONE);
			}
		} else {
			if (word_len <= 8) {
				chal_sspi_set_fifo_pack(handle,
					SSPI_FIFO_ID_TX0 + fifo_num,
					SSPI_FIFO_DATA_PACK_8BIT);
			} else if (word_len <= 16) {
				chal_sspi_set_fifo_pack(handle,
					SSPI_FIFO_ID_TX0 + fifo_num,
					SSPI_FIFO_DATA_PACK_16BIT);
			} else {
				chal_sspi_set_fifo_pack(handle,
					SSPI_FIFO_ID_TX0 + fifo_num,
					SSPI_FIFO_DATA_PACK_NONE);
			}
		}
	} else {
		if (rx)
			chal_sspi_set_fifo_pack(handle,
					SSPI_FIFO_ID_RX0 + fifo_num,
					SSPI_FIFO_DATA_PACK_NONE);
		else
			chal_sspi_set_fifo_pack(handle,
					SSPI_FIFO_ID_TX0 + fifo_num,
					SSPI_FIFO_DATA_PACK_NONE);
	}
}

/*
 *  Description: soft reset ssp port
 */
static void csl_pcm_reset(CSL_PCM_HANDLE_t *pDevice)
{
	u32 *reg, *reg2, reg_value, data, shift, reg_wr;

	if (pDevice->base == (cUInt32)KONA_SSP3_BASE_VA) {
		shift = KHUB_RST_MGR_REG_SOFT_RSTN1_SSP3_SOFT_RSTN_SHIFT;
		aTrace(LOG_AUDIO_CSL, "csl_pcm_reset::reset SSP3 port, "
			"shift %d\n", shift);
	} else if (pDevice->base == (cUInt32)KONA_SSP4_BASE_VA) {
		shift = KHUB_RST_MGR_REG_SOFT_RSTN1_SSP4_SOFT_RSTN_SHIFT;
		aTrace(LOG_AUDIO_CSL, "csl_pcm_reset::reset SSP4 port, "
			"shift %d\n", shift);
#if 0		
	} else if (pDevice->base == (cUInt32)KONA_SSP6_BASE_VA) {
		shift = KHUB_RST_MGR_REG_SOFT_RSTN1_SSP6_SOFT_RSTN_SHIFT;
		aTrace(LOG_AUDIO_CSL, "csl_pcm_reset::reset SSP6 port, "
			"shift %d\n", shift);
#endif		
	} else {
		aError("csl_pcm_reset::AUDIO ERROR invalid base 0x%x\n",
			(u32)pDevice->base);
		return;
	}

	reg = (u32 *)ioremap_nocache(HUB_RST_BASE_ADDR, 3*sizeof(u32));
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
	iounmap(reg);
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

	intc_handle = (CHAL_HANDLE)caphIntcHandle;

	aTrace(LOG_AUDIO_CSL, "-csl_pcm_init base address 0x%x\r\n",
			(unsigned int)pDevice->base);

	return handle;
}

/******************************************************************************
*
* Function Name:	csl_pcm_deinit
*
*  Description:	This function deinitializes the CSL layer
*
******************************************************************************/
CSL_PCM_OPSTATUS_t csl_pcm_deinit(CSL_PCM_HANDLE handle)
{
	CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *)handle;
	CHAL_SSPI_STATUS_t status;

	aTrace(LOG_AUDIO_CSL, "+csl_pcm_deinit\r\n");

	if (handle == NULL) {
		aError("csl_pcm_deinit failed\r\n");
		return CSL_PCM_ERR_HANDLE;
	}

    status = chal_sspi_deinit(pDevice);

	aTrace(LOG_AUDIO_CSL, "-csl_pcm_deinit\r\n");

	if(status == CHAL_SSPI_STATUS_SUCCESS)
		return CSL_PCM_SUCCESS;
	else if(status == CHAL_SSPI_STATUS_ILLEGAL_HANDLE)
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
    CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *)handle;

	aTrace(LOG_AUDIO_CSL,
			"csl_pcm_enable_scheduler:: handle %p enable %d.\r\n",
			handle, enable);
	if (!pDevice)
		return CSL_PCM_ERR_HANDLE;

    chal_sspi_enable_scheduler(pDevice, enable);

    return CSL_PCM_SUCCESS;
}

/******************************************************************************
*
*  Function Name:	csl_pcm_start
*
*  Description:	This function starts scheduler operation
*
*****************************************************************************/
CSL_PCM_OPSTATUS_t csl_pcm_start(CSL_PCM_HANDLE handle,
				 csl_pcm_config_device_t *config)
{
	CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *) handle;
	CHAL_SSPI_STATUS_t status;

	aTrace(LOG_AUDIO_CSL, "csl_pcm_start::"
			      "handle %p.\r\n", handle);
	aTrace(LOG_AUDIO_CSL,
			      "csl_pcm_start:: cfgDev mode %d protocol %d"
			      "size %ld \r\n",
			      config->mode,
			      config->protocol,
			      config->xferSize);

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

/******************************************************************************
*
*  Function Name:	csl_pcm_start_tx
*
*  Description:	This function starts transmit operation
*
******************************************************************************/
CSL_PCM_OPSTATUS_t csl_pcm_start_tx(CSL_PCM_HANDLE handle, UInt8 channel)
{
    CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *)handle;

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

/******************************************************************************
*
*  Function Name:	csl_pcm_start_rx
*
*  Description:	This function starts receive operation
*
******************************************************************************/
CSL_PCM_OPSTATUS_t csl_pcm_start_rx(CSL_PCM_HANDLE handle, UInt8 channel)
{
    CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *)handle;

	aTrace(LOG_AUDIO_CSL,
			"csl_pcm_start_rx:: handle %p, channel %d.\r\n", handle,
			channel);
	if (!pDevice)
		return CSL_PCM_ERR_HANDLE;

    if(channel == CSL_PCM_CHAN_RX0) {
        chal_sspi_fifo_reset(pDevice, SSPI_FIFO_ID_RX0);
        chal_sspi_enable_fifo_pio_start_stop_intr(pDevice,
                                              SSPI_FIFO_ID_RX0,
                                              TRUE,
                                              TRUE);
	} else if (channel == CSL_PCM_CHAN_RX1) {
        chal_sspi_fifo_reset(pDevice, SSPI_FIFO_ID_RX1);
        chal_sspi_enable_fifo_pio_start_stop_intr(pDevice,
                                              SSPI_FIFO_ID_RX1,
                                              TRUE,
                                              TRUE);
    }
    return CSL_PCM_SUCCESS;
}

/******************************************************************************
*
*  Function Name:	csl_pcm_stop_tx
*
*  Description:	This function stops transmit operation
*
******************************************************************************/
CSL_PCM_OPSTATUS_t csl_pcm_stop_tx(CSL_PCM_HANDLE handle, UInt8 channel)
{
    CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *)handle;

	aTrace(LOG_AUDIO_CSL, "%s handle %p, chan %d\n",
		__func__, handle, channel);
	if (!pDevice)
		return CSL_PCM_ERR_HANDLE;

    if(channel == CSL_PCM_CHAN_TX0) {
        chal_sspi_fifo_reset(pDevice, SSPI_FIFO_ID_TX0);
        chal_sspi_enable_fifo_pio_start_stop_intr(pDevice,
                                              SSPI_FIFO_ID_TX0,
                                              FALSE,
                                              FALSE);
	} else if (channel == CSL_PCM_CHAN_TX1) {
        chal_sspi_fifo_reset(pDevice, SSPI_FIFO_ID_TX1);
        chal_sspi_enable_fifo_pio_start_stop_intr(pDevice,
                                              SSPI_FIFO_ID_TX1,
                                              FALSE,
                                              FALSE);
    }

    return CSL_PCM_SUCCESS;
}

/******************************************************************************
*
*  Function Name:	csl_pcm_stop_rx
*
*  Description:	This function stops receive operation
*
******************************************************************************/
CSL_PCM_OPSTATUS_t csl_pcm_stop_rx(CSL_PCM_HANDLE handle, UInt8 channel)
{
    CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *)handle;

	aTrace(LOG_AUDIO_CSL, "%s handle %p, chan %d\n",
		__func__, handle, channel);

	if (!pDevice)
		return CSL_PCM_ERR_HANDLE;

    if(channel == CSL_PCM_CHAN_RX0) {
        chal_sspi_fifo_reset(pDevice, SSPI_FIFO_ID_RX0);
        chal_sspi_enable_fifo_pio_start_stop_intr(pDevice,
                                              SSPI_FIFO_ID_RX0,
                                              FALSE,
                                              FALSE);
	} else if (channel == CSL_PCM_CHAN_RX1) {
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

/******************************************************************************
*
*  Function Name:	csl_pcm_config
*
*  Description:	This function configures SSPI as PCM operation
*
******************************************************************************/
CSL_PCM_OPSTATUS_t csl_pcm_config(CSL_PCM_HANDLE handle,
				csl_pcm_config_device_t *configDev)
{
	CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *)handle;
	uint32_t frmMask = SSPI_HW_FRAME0_MASK;
	uint32_t sample_len = 0, seq_idx, dword_sz;
	csl_pcm_config_device_t *devCfg = configDev;
	uint32_t intrMask = 0, i, rate = 0, tmplen;

	aTrace
	      (LOG_AUDIO_CSL, "csl_pcm_config:: handle %p.\r\n", handle);
	aTrace
	      (LOG_AUDIO_CSL,
	       "csl_pcm_config:: cfgDev mode %d protocol %d tx_loopback %d"
	       "rx_loopback %d size %ld num of channels %d.\r\n",
	       configDev->mode, configDev->protocol, configDev->tx_lpbk,
	       configDev->rx_lpbk, configDev->xferSize,
	       configDev->num_ch_info);
	for (i = 0; i < configDev->num_ch_info; i++) {
		aTrace
		(LOG_AUDIO_CSL,
		 "csl_pcm_config:: ch %d sr %ld, num of interleaved "
		 "channels %d \r\n"
		 "rx_ena %d, rx_len %d rx_delay_bits %d rx_pack %d \r\n"
		 "tx_ena %d tx_len %d tx_pack %d tx_prepad_bits %d \r\n"
		 "tx_postpad_bits %d tx_padval %d \r\n",
		 i, configDev->ch_info[i].sample_rate,
		 configDev->ch_info[i].num_intrlvd_ch,
		 configDev->ch_info[i].rx_ena,
		 configDev->ch_info[i].rx_len,
		 configDev->ch_info[i].rx_delay_bits,
		 configDev->ch_info[i].rx_pack, configDev->ch_info[i].tx_ena,
		 configDev->ch_info[i].tx_len, configDev->ch_info[i].tx_pack,
		 configDev->ch_info[i].tx_prepad_bits,
		 configDev->ch_info[i].tx_postpad_bits,
		 configDev->ch_info[i].tx_padval);
	}

	if (handle == NULL) {
		aError("csl_pcm_config failed\r\n");
		return CSL_PCM_ERR_HANDLE;
	}

	/*task_conf struct initialization */
	memset(&task_conf, 0, sizeof(task_conf));
	memset(&seq_conf, 0, sizeof(seq_conf));
	memset(&frm_arr, 0, sizeof(frm_arr));

	csl_pcm_reset(pDevice);

	/*soft reset sspi instance */
	chal_sspi_soft_reset(handle);

	/* SSPI mode configuration */
	if (devCfg->mode == CSL_PCM_MASTER_MODE)
		chal_sspi_set_mode(handle, SSPI_MODE_MASTER);
	else
		chal_sspi_set_mode(handle, SSPI_MODE_SLAVE);

	chal_sspi_get_intr_mask(handle, &intrMask);
	aTrace(LOG_AUDIO_CSL, "csl_pcm_config:: intrMask1 0x%x.\r\n",
			intrMask);

	if (devCfg->protocol == CSL_PCM_PROTOCOL_VOICECALL) {
		/*for voice call only enable RX START */
		intrMask |= SSPIL_INTR_ENABLE_PIO_RX_START;
		/*intrMask |= (SSPIL_INTR_ENABLE_PIO_TX_START |
		   SSPIL_INTR_ENABLE_PIO_TX_STOP |
		   SSPIL_INTR_ENABLE_PIO_RX_START |
		   SSPIL_INTR_ENABLE_PIO_RX_STOP); */
		intrMask |=
		    (SSPIL_INTR_ENABLE_PIO_TX_START |
		     SSPIL_INTR_ENABLE_PIO_RX_START);
	} else {
		/*intrMask |= (SSPIL_INTR_ENABLE_PIO_TX_START |
		   SSPIL_INTR_ENABLE_PIO_TX_STOP |
		   SSPIL_INTR_ENABLE_PIO_RX_START |
		   SSPIL_INTR_ENABLE_PIO_RX_STOP); */
		intrMask |=
		    (SSPIL_INTR_ENABLE_PIO_TX_START |
		     SSPIL_INTR_ENABLE_PIO_RX_START);
	}
	aTrace(LOG_AUDIO_CSL, "csl_pcm_config:: intrMask2 0x%x.\r\n",
			intrMask);
	/*need to disable all other interrupts to avoid confusing
	  dsp 03-02-11 */
	chal_sspi_enable_intr(pDevice, intrMask & 0x000000F0);

	/*set sspi at idle state */
	if (chal_sspi_set_idle_state(handle, SSPI_PROT_DEFAULT_PCM)) {
		aError("csl_pcm_config failed \r\n");
		return CSL_PCM_ERROR;
	}

	chal_sspi_set_clk_divider(handle, SSPI_CLK_DIVIDER0, 0);
	chal_sspi_set_clk_divider(handle, SSPI_CLK_REF_DIVIDER, 0);
	chal_sspi_set_clk_src_select(handle, SSPI_CLK_SRC_CAPHCLK);
	for (i = 0; i < devCfg->num_ch_info; i++) {
		rate = MAX(devCfg->ch_info[i].sample_rate, rate);
		tmplen = MAX((devCfg->ch_info[i].rx_delay_bits +
					devCfg->ch_info[i].rx_len),
					(devCfg->ch_info[i].tx_len +
					devCfg->ch_info[i].tx_prepad_bits +
					devCfg->ch_info[i].tx_postpad_bits)) *
					devCfg->ch_info[i].num_intrlvd_ch;
		sample_len += tmplen;
	}

	for (i = 0; i < devCfg->num_ch_info; i++) {
		frmMask |= (SSPI_HW_FRAME0_MASK << i);
		frm_arr[i].rx_delay_bits = devCfg->ch_info[i].rx_delay_bits;
		frm_arr[i].rx_len = devCfg->ch_info[i].rx_len;
		frm_arr[i].tx_len = devCfg->ch_info[i].tx_len;
		frm_arr[i].tx_padval = devCfg->ch_info[i].tx_padval;
		frm_arr[i].tx_prepad_bits = devCfg->ch_info[i].tx_prepad_bits;
		frm_arr[i].tx_postpad_bits = devCfg->ch_info[i].tx_postpad_bits;

		chal_sspi_set_fifo_repeat_count(handle, SSPI_FIFO_ID_RX0,
				rate / devCfg->ch_info[i].sample_rate - 1);
		chal_sspi_set_fifo_repeat_count(handle, SSPI_FIFO_ID_TX0,
				rate / devCfg->ch_info[i].sample_rate - 1);
		set_fifo_pack(handle, i, devCfg->ch_info[i].rx_len,
				devCfg->ch_info[i].rx_pack, 1);
		set_fifo_pack(handle, i, devCfg->ch_info[i].tx_len,
				devCfg->ch_info[i].tx_pack, 0);
	}
	chal_sspi_set_caph_clk(handle, rate, sample_len);
	// Kishore - add Rhea code or get capri SSP code
	
	
	set_all_fifo_size(handle, configDev->num_ch_info);

	/* enable sspi operation */
	chal_sspi_enable(handle, 1);
	
	// Kishore - add Rhea code here or get Capri SSP

	chal_sspi_get_max_fifo_size(handle, &dword_sz);
	dword_sz >>= 2; /* change to size in DWord */

	if (devCfg->num_ch_info == 1) {
		/* setting from asic team */
		/* config for audio mode, may divert from voice settings. */
		/* SSPI send out RX_Start interrupt when there is 4 sample
		* in RX0 fifo and Rx_stop interrupt when there are 3 samples
		* in the RX0 fifo. SSPI send out TX_Start interrupt when there
		* are less 3 samples in TX0 fifo and Tx_stop interrupt when
		* there are 4 samples in the TX0 fifo.
		*/
		chal_sspi_set_fifo_pio_threshhold(
				handle, SSPI_FIFO_ID_RX0, dword_sz - 4 , 0x3);
		chal_sspi_set_fifo_pio_threshhold(
				handle, SSPI_FIFO_ID_TX0, 0x3, dword_sz - 4);
	} else if (devCfg->num_ch_info == 2) {
		chal_sspi_set_fifo_pio_threshhold(
				handle, SSPI_FIFO_ID_RX0, dword_sz/2 - 4 , 0x3);
		chal_sspi_set_fifo_pio_threshhold(
				handle, SSPI_FIFO_ID_TX0, 0x3, dword_sz/2 - 4);
		chal_sspi_set_fifo_pio_threshhold(
				handle, SSPI_FIFO_ID_RX1, dword_sz/2 - 4 , 0x3);
		chal_sspi_set_fifo_pio_threshhold(
				handle, SSPI_FIFO_ID_TX1, 0x3, dword_sz/2 - 4);
	} else {
		chal_sspi_set_fifo_pio_threshhold(
				handle, SSPI_FIFO_ID_RX0, dword_sz/4 - 4 , 0x3);
		chal_sspi_set_fifo_pio_threshhold(
				handle, SSPI_FIFO_ID_TX0, 0x3, dword_sz/4 - 4);
		chal_sspi_set_fifo_pio_threshhold(
				handle, SSPI_FIFO_ID_RX1, dword_sz/4 - 4 , 0x3);
		chal_sspi_set_fifo_pio_threshhold(
				handle, SSPI_FIFO_ID_TX1, 0x3, dword_sz/4 - 4);
		chal_sspi_set_fifo_pio_threshhold(
				handle, SSPI_FIFO_ID_RX2, dword_sz/4 - 4 , 0x3);
		chal_sspi_set_fifo_pio_threshhold(
				handle, SSPI_FIFO_ID_TX2, 0x3, dword_sz/4 - 4);
		chal_sspi_set_fifo_pio_threshhold(
				handle, SSPI_FIFO_ID_RX3, dword_sz/4 - 4 , 0x3);
		chal_sspi_set_fifo_pio_threshhold(
				handle, SSPI_FIFO_ID_TX3, 0x3, dword_sz/4 - 4);
	}

	task_conf.chan_sel = SSPI_CHAN_SEL_CHAN0;
	task_conf.cs_sel = SSPI_CS_SEL_CS0;
	task_conf.rx_sel = (devCfg->tx_lpbk)
				? SSPI_RX_SEL_COPY_TX0 : SSPI_RX_SEL_RX0;
	task_conf.tx_sel = SSPI_TX_SEL_TX0;
	task_conf.div_sel = SSPI_CLK_DIVIDER0;
	task_conf.seq_ptr = 0;

	/* Max transfer size is set to 4K frames in non_continuous
	 * mode transfer, say word_len is 16bits, the max transfer size
	 * is 4Kx2=8K bytes. If Max transfer size is greater 4K frames,
	 * we just use continuous-mode transfer */
	if (devCfg->xferSize > MAX_TRANS_SIZE) {
		task_conf.loop_cnt = 0;
		task_conf.continuous = 1;
	} else {
		task_conf.loop_cnt = devCfg->xferSize - 1;
		task_conf.continuous = 0;
	}
	task_conf.wait_before_start = 1;

	if (chal_sspi_set_task(handle, 0, SSPI_PROT_DEFAULT_PCM, &task_conf))
		return CSL_PCM_ERR_TASK;

	seq_idx = 0;
	for (i = 0; i < devCfg->num_ch_info; i++) {
		if (devCfg->ch_info[i].tx_ena) {
			chal_sspi_set_fifo_threshold(handle,
					SSPI_FIFO_ID_TX0+i, 0x0);
		}
		task_conf.init_cond_mask |= (devCfg->ch_info[i].tx_ena) ?
				(SSPI_TASK_INIT_COND_THRESHOLD_TX0 << i) : 0;

		seq_conf.tx_enable =
			(devCfg->ch_info[i].tx_ena || devCfg->rx_lpbk) ?
			TRUE : FALSE;
		seq_conf.rx_enable =
			(devCfg->ch_info[i].rx_ena || devCfg->tx_lpbk) ?
			TRUE : FALSE;
		/* Turn on both cs_activate and cs_deactivate for the first
		 * seq to send the CS pulse in master mode.
		 * Turn on cs_activate and turn off cs_deactivate for the first
		 * seq to in slave mode */
		seq_conf.cs_activate = (seq_idx) ? 0 : 1;
		seq_conf.cs_deactivate = (seq_idx) ? 0 :
			((devCfg->mode == CSL_PCM_SLAVE_MODE) ? 0 : 1);
		seq_conf.pattern_mode = 0;
		seq_conf.rep_cnt = 1;
		seq_conf.opcode = SSPI_SEQ_OPCODE_COND_JUMP;
		seq_conf.rx_fifo_sel = 0;
		seq_conf.tx_fifo_sel = 0;
		seq_conf.frm_sel = 0;
		seq_conf.rx_sidetone_on = 0;
		seq_conf.tx_sidetone_on = 0;
		seq_conf.next_pc = 0;

		if (seq_idx == 0) {
			seq_conf.rep_cnt = 0;
			if (devCfg->ch_info[i].num_intrlvd_ch > 1) {
				seq_conf.opcode = SSPI_SEQ_OPCODE_NEXT_PC;
				if (chal_sspi_set_sequence(handle, seq_idx,
					SSPI_PROT_DEFAULT_PCM, &seq_conf))
					return CSL_PCM_ERR_SEQUENCE;
				seq_idx++;
				seq_conf.cs_activate = 0;
				seq_conf.cs_deactivate = 0;
				seq_conf.rep_cnt = devCfg->num_ch_info - 2;
			}
			seq_conf.opcode = (devCfg->num_ch_info > 1) ?
					SSPI_SEQ_OPCODE_NEXT_PC :
					SSPI_SEQ_OPCODE_COND_JUMP;
		} else { /* not the first sequence */
			seq_conf.cs_activate = 0;
			seq_conf.cs_deactivate = 0;
			seq_conf.rep_cnt =
				devCfg->ch_info[i].num_intrlvd_ch - 1;

			seq_conf.opcode = (devCfg->num_ch_info - 1 > i) ?
					SSPI_SEQ_OPCODE_NEXT_PC :
					SSPI_SEQ_OPCODE_COND_JUMP;
		}
		if (chal_sspi_set_sequence(handle, seq_idx,
					SSPI_PROT_DEFAULT_PCM, &seq_conf)) {
			return CSL_PCM_ERR_SEQUENCE;
		}
		seq_idx++;
	}
	seq_conf.cs_activate = 0;
	seq_conf.cs_deactivate = 0;
	seq_conf.tx_enable = FALSE;
	seq_conf.rx_enable = FALSE;
	seq_conf.rep_cnt = 0;
	seq_conf.opcode = SSPI_SEQ_OPCODE_STOP;
	seq_conf.rx_fifo_sel = 0;
	seq_conf.tx_fifo_sel = 0;
	seq_conf.frm_sel = 0;
	seq_conf.next_pc = 0;
	if (chal_sspi_set_sequence(handle, seq_idx,
				SSPI_PROT_DEFAULT_PCM, &seq_conf))
		return CSL_PCM_ERR_SEQUENCE;

		// Kishore - add Rhea code here or get Capri SSP
	if (chal_sspi_set_pcm_frame(handle, &frmMask, frm_arr))
		return CSL_PCM_ERR_FRAME;

	return CSL_PCM_SUCCESS;
}

/******************************************************************************
*
*  Function Name:	csl_pcm_get_tx0_fifo_data_port
*
*  Description:	This function get transmit channel fifo port address
*
*******************************************************************************/
UInt32 csl_pcm_get_tx0_fifo_data_port(CSL_PCM_HANDLE handle)
{
    CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *)handle;
	if (pDevice)
		return (UInt32)(pDevice->base+SSPIL_FIFO_ENTRY0TX_OFFSET);
	else
		return 0;
}
/******************************************************************************
*
*  Function Name:	csl_pcm_get_tx1_fifo_data_port
*
*  Description:	This function get receive channel fifo port address
*
*******************************************************************************/

UInt32 csl_pcm_get_tx1_fifo_data_port(CSL_PCM_HANDLE handle)
{
    CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *)handle;
	return (UInt32)(pDevice->base+SSPIL_FIFO_ENTRY1TX_OFFSET);
}

/******************************************************************************
*
*  Function Name:	csl_pcm_get_rx0_fifo_data_port
*
*  Description:	This function get receive channel fifo port address
*
*******************************************************************************/
UInt32 csl_pcm_get_rx0_fifo_data_port(CSL_PCM_HANDLE handle)
{
	CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *)handle;
	if (pDevice)
		return (UInt32)(pDevice->base+SSPIL_FIFO_ENTRY0RX_OFFSET);
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
	CSL_PCM_HANDLE_t *pDevice = (CSL_PCM_HANDLE_t *)handle;
	return (UInt32)(pDevice->base+SSPIL_FIFO_ENTRY1RX_OFFSET);
}

/****************************************************************************
*
*  Function Name: void csl_caph_intc_enable_pcm_intr(CSL_CAPH_ARM_DSP_e
*		csl_owner, CSL_CAPH_SSP_e csl_sspid)
*
*  Description: enable pcm intr
*
****************************************************************************/
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
	else if (csl_sspid == CSL_CAPH_SSP_6)
		chal_caph_intc_enable_ssp_intr(intc_handle, 4, owner);
    else
		/*should not get here. */
		audio_xassert(0, csl_sspid);
}

/****************************************************************************
*
*  Function Name: void csl_caph_intc_disable_pcm_intr(
*	CSL_CAPH_ARM_DSP_e csl_owner, CSL_CAPH_SSP_e csl_sspid)
*
*  Description: disable pcm intr
*
****************************************************************************/
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
    else if (csl_sspid == CSL_CAPH_SSP_6)
        chal_caph_intc_disable_ssp_intr(intc_handle, 4, owner);
	else
		/* should not get here. */
		audio_xassert(0, csl_sspid);
}

