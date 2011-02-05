/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/bcm_i2sdai.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/
/**
*
*   @file   bcm_i2sdai.h
*   @brief  This file defines the I2S Digital Audio Interface
*
*****************************************************************************/

#ifndef __LINUX_BCM_I2SDAI_H__
#define __LINUX_BCM_I2SDAI_H__

#include <plat/cpu.h>
/** I2S device */
struct i2s_device {
	int sample_rate_div;		/**< sample rate divider */
	int i2s_handle;				/**< handle to i2s device */
	int i2s_handle_state;		/**< handle state */
	int channels;				/**< channels */
	int tx_op_mode;				/**< transmit op mode */
	int rx_op_mode;				/**< receive op mode */
	int tx_active;				/**< transmit active */
	int rx_active;				/**< receive active */
	struct clk *i2s_clk_int;	/**< pointer to internal clock */
	struct clk *i2s_clk_ext;	/**< pointer to external clock */
	spinlock_t i2s_lock;		/**< spinlock */
};

enum SAMPLE_RATE {
	BCLK_DIV_1500 = 0x0,
	BCLK_DIV_1496 = 0x1,
	BCLK_DIV_1088 = 0x2,
	BCLK_DIV_1000 = 0x3,
	BCLK_DIV_750 = 0x4,
	BCLK_DIV_544 = 0x5,
	BCLK_DIV_500 = 0x6,
	BCLK_DIV_375 = 0x7,
	BCLK_DIV_272 = 0x8,
	BCLK_DIV_250 = 0x9,
	BCLK_DIV_136 = 0xA,
	BCLK_DIV_125 = 0xB,
	BCLK_DIV_68 = 0xC,
	BCLK_DIV_34 = 0xD,
};

enum DMA_SIZE {
	SAMPLE_16 = 0x3,
	SAMPLE_32 = 0x4,
	SAMPLE_64 = 0x5,
	SAMPLE_128 = 0x6
};

enum PLAY_DIRECTION {
	PLAYBACK = 0,
	CAPTURE = 1
};

enum CLK_SEL {
	I2S_INT_CLK,
	I2S_EXT_CLK
};

enum INT_CLK_RATE {
	RATE_12_MHZ,
	RATE_6_MHZ,
	RATE_3_MHZ,
	RATE_1_5_MHZ,
	RATE_INVALID
};

enum SPDIF_PREAMBLE {
	SPDIF_PREAMBLE_A = 0x1,
	SPDIF_PREAMBLE_B = 0x4,
	SPDIF_PREAMBLE_C = 0x8
};
struct i2s_aud_buf {
	size_t size_bytes;
	unsigned int *virt_ptr;
	dma_addr_t phys_ptr;
};

struct dma_i2s {
    int sample_rate;
    int channels;
    struct platform_device *pdev;
    int tx_active;
    int rx_active;
    int tx_channel;
    int rx_channel;
    dma_addr_t
    current_chunk;
    struct i2s_aud_buf dma_buf;
};
#define CLK_I2S_INT_DIV_12_0M	0x00c
#define CLK_I2S_INT_DIV_3_0M	0x033
#define CLK_I2S_INT_DIV_2_4M	0x040
#define CLK_I2S_INT_DIV_1_5M	0x067
#define CLK_I2S_INT_DIV_2_048M	0x04B
#define CLK_I2S_INT_DIV_1_536M	0x064
#define CLK_I2S_INT_DIV_512K	0x12F
#define CLK_I2S_INT_DIV_256K	0x260
#define CLK_I2S_INT_DIV_128K	0x4C1

enum I2S_SAMPLE_RATE {
	I2S_SAMPLERATE_8000HZ,	/**< 8000HZ */
	I2S_SAMPLERATE_11030HZ,	/**< 11030HZ */
	I2S_SAMPLERATE_12000HZ,	/**< 12000HZ */
	I2S_SAMPLERATE_16000HZ,	/**< 16000HZ */
	I2S_SAMPLERATE_22060HZ,	/**< 22060HZ */
	I2S_SAMPLERATE_24000HZ,	/**< 24000HZ */
	I2S_SAMPLERATE_32000HZ,	/**< 32000HZ */
	I2S_SAMPLERATE_48000HZ,	/**< 48000HZ */
	I2S_SAMPLERATE_44100HZ,	/**< 44100HZ */
	I2S_SAMPLERATE_96000HZ	/**< 96000HZ */
};

enum I2S_DMA_SIZE {
	TXRXDMASIZE_16 = 0x3,	/**< DMA Size 16 */
	TXRXDMASIZE_32 = 0x4,	/**< DMA Size 32 */
	TXRXDMASIZE_64 = 0x5	/**< DMA Size 64 */
};

/*****************************************************************************/

/* *************************************************************************** */
/* Function Name:       use_i2s_device() */
/* Description:         Request i2S instance. */
/* Parameters:         none */
/* Return:             i2S  device Instance ; NULL otherwise */
/* *************************************************************************** */
struct i2s_device *use_i2s_device(void);

/* *************************************************************************** */
/* Function Name:       release_i2s_device() */
/* Description:         Release i2S device. */
/* Parameters:          i2S  device Instance */
/* Return:              None */
/* *************************************************************************** */
void release_i2s_device(struct i2s_device *i2s_handle);

/* *************************************************************************** */
/* Function Name:       bcm_i2s_cfg_play_rec_interrupt() */
/* Description:         Configure  Playback/Record Interrupts and enable TX/Rx FIFO. */
/* Parameters:          handle          i2S device Instance */
/*                      int_period      Playback/Capture DMA interrupt period */
/*                      direction       Playback / Capture */
/*                      en_dis          Enable/Disable the FIFO */
/* Return:              returns 0 on success; -EINVAL otherwise */
/* *************************************************************************** */
unsigned int bcm_i2s_cfg_play_rec_interrupt(struct i2s_device *handle,
					    unsigned int int_period,
					    int direction, bool en_dis);

/* *************************************************************************** */
/* Function Name:       bcm_i2s_get_interrupt_status() */
/* Description:         Read the status of Interrupts */
/* Parameters:          handle          i2S device Instance */
/*                      direction       Playback / Capture */
/* Return:              returns 0 on success; -EINVAL otherwise */
/* *************************************************************************** */
unsigned int bcm_i2s_get_interrupt_status(struct i2s_device *handle,
					  unsigned int direction);

/* *************************************************************************** */
/* Function Name:       bcm_i2s_fifo_empty_int_cfg() */
/* Description:         Enable Interrupts when TX/Rx FIFO is empty. */
/* Parameters:          handle          i2S device Instance */
/*                      direction       Playback / Capture */
/*                      en_dis          Enable/Disable the FIFO */
/* Return:              returns 0 on success; -EINVAL otherwise */
/* *************************************************************************** */
unsigned int bcm_i2s_fifo_empty_int_cfg(struct i2s_device *handle,
					unsigned int direction, bool en_dis);

/* *************************************************************************** */
/* Function Name:       bcm_i2s_txrx_fifo_enable() */
/* Description:         Enable TX/Rx FIFO . */
/* Parameters:          handle          i2S device Instance */
/*                      direction       Playback / Capture */
/* Return:              returns 0 on success; -EINVAL otherwise */
/* *************************************************************************** */
unsigned int bcm_i2s_txrx_fifo_enable(struct i2s_device *handle,
				      unsigned int direction);

/* *************************************************************************** */
/* Function Name:       bcm_i2s_txrx_fifo_disable() */
/* Description:         Disable TX/Rx FIFO . */
/* Parameters:          handle          i2S device Instance */
/*                      direction       Playback / Capture */
/* Return:              returns 0 on success; -EINVAL otherwise */
/* *************************************************************************** */
unsigned int bcm_i2s_txrx_fifo_disable(struct i2s_device *handle,
				       unsigned int direction);

/* *************************************************************************** */
/* Function Name:       bcm_i2s_flush_fifo() */
/* Description:         Flush Tx / RX FIFO. Good to use this after a playback */
/*                      or capture is complete or before releasing the device. */
/* Parameters:          handle          i2S device Instance */
/*                      direction       Playback / Capture */
/* Return:              returns 0 on success; -EINVAL otherwise */
/* *************************************************************************** */
unsigned int bcm_i2s_flush_fifo(struct i2s_device *handle,
				unsigned int direction);

/* *************************************************************************** */
/* Function Name:       bcm_i2s_sample_rate_div() */
/* Description:         Specify the Sample rate divider ratio */
/* Parameters:          handle          i2S device Instance */
/*                      rate            sample rate ratio */
/*                      direction       Playback / Capture */
/* Return:              returns 0 on success; -EINVAL otherwise */
/* *************************************************************************** */
unsigned int bcm_i2s_sample_rate_div(struct i2s_device *handle,
				     int rate, int direction);

/* *************************************************************************** */
/* Function Name:       bcm_i2s_interface_mode() */
/* Description:         Configure Master/Slave mode for playback */
/* Parameters:          handle          i2S device Instance */
/*                      interface mode  Master/Slave mode for playback */
/*                      channels        no. of channels in the audio stream */
/* Return:              returns 0 on success; -EINVAL otherwise */
/* *************************************************************************** */
unsigned int bcm_i2s_interface_mode(struct i2s_device *handle,
				    unsigned int interface_mode,
				    unsigned int channels);

/* *************************************************************************** */
/* Function Name:       bcm_i2s_interface_enable() */
/* Description:         Enable the I2s TX / RX interface for transmission or receive. */
/* Parameters:          handle          i2S device Instance */
/*                      play            Enable Playback */
/*                      rec             Enable Capture */
/*                      sample_sir      Specify order of samples in FIFO. */
/* Return:              returns 0 on success; -EINVAL otherwise */
/* *************************************************************************** */
unsigned int bcm_i2s_interface_enable(struct i2s_device *handle, bool play,
				      bool rec, int sample_dir);

/* *************************************************************************** */
/* Function Name:       bcm_i2s_interface_disable() */
/* Description:         Disable the I2s TX / RX interface. */
/* Parameters:          handle          i2S device Instance */
/*                      play            Enable Playback */
/*                      rec             Enable Capture */
/* Return:              returns 0 on success; -EINVAL otherwise */
/* *************************************************************************** */
unsigned int bcm_i2s_interface_disable(struct i2s_device *handle, bool play,
				       bool rec);

/* *************************************************************************** */
/* Function Name:       bcm_i2s_loopback_cfg() */
/* Description:         Enable/Disable  Loopback mode of I2S. Used only for debugging */
/* Parameters:          handle          i2S device Instance */
/*                      en_dis          Enable or disable Loopback */
/* Return:              returns 0 on success; -EINVAL otherwise */
/* *************************************************************************** */
unsigned int bcm_i2s_loopback_cfg(struct i2s_device *handle, bool en_dis);

/* *************************************************************************** */
/* Function Name:       bcm_i2s_setup_mono_record() */
/* Description:         Configure Mono Record Mode */
/* Parameters:          handle          i2S device Instance */
/*                      channel         Specify Mono Recording Channel. */
/*                      rec_mode        No. of channels in the audio stream. */
/*                      avg_mode        Enable or disable Loopback */
/* Return:              returns 0 on success; -EINVAL otherwise */
/* *************************************************************************** */
unsigned int bcm_i2s_setup_mono_record(struct i2s_device *handle, int channel,
				       int rec_mode, int avg_mode);

/* *************************************************************************** */
/* Function Name:       bcm_i2s_config_dma() */
/* Description:         Use i2S in DMA Mode. */
/* Parameters:          handle          i2S device Instance */
/*                      direction       Playback / Capture */
/*                      threshold       Set FIFO threshold levels in DMA Mode. */
/* Return:              returns 0 on success; -EINVAL otherwise */
/* *************************************************************************** */
unsigned int bcm_i2s_config_dma(struct i2s_device *handle,
				unsigned int direction,
				enum DMA_SIZE threshold);

/* *************************************************************************** */
/* Function Name:       bcm_i2s_disable_dma() */
/* Description:         Disable DMA Mode. */
/* Parameters:          handle          i2S device Instance */
/*                      direction       Playback / Capture */
/* Return:              returns 0 on success; -EINVAL otherwise */
/* *************************************************************************** */
unsigned int bcm_i2s_disable_dma(struct i2s_device *handle,
				 unsigned int direction);

/* *************************************************************************** */
/* Function Name:       bcm_i2s_get_channel_status() */
/* Description:         Returns the current status of the DAI. */
/* Parameters:          handle          i2S device Instance */
/* Return:              returns 0 on success; -EINVAL otherwise */
/* *************************************************************************** */
unsigned int bcm_i2s_get_channel_status(struct i2s_device *handle);

/* *************************************************************************** */
/* Function Name:       bcm_i2s_sofreset_dai() */
/* Description:         Softresets the DAI. Used before starting other configuration. */
/* Parameters:          handle          i2S device Instance */
/* Return:              returns 0 on success ; -EINVAL otherwise */
/* *************************************************************************** */
void bcm_i2s_sofreset_dai(struct i2s_device *handle);

/* *************************************************************************** */
/* Function Name:       bcm_i2s_enable_clk() */
/* Description:         Enable Internal/External clock for I2S module. */
/* Parameters:          handle          i2S device Instance */
/*                      clk             Internal/External Clock */
/*                      rate            Rate of Internal Clock */
/* Return:              returns 0 on success ; -EINVAL otherwise */
/* *************************************************************************** */
int bcm_i2s_enable_clk(struct i2s_device *handle, enum CLK_SEL clk,
		       enum INT_CLK_RATE rate);

/* *************************************************************************** */
/* Function Name:       bcm_i2s_disable_clk() */
/* Description:         Disable Internal/External clock for I2S module. */
/* Parameters:          handle          i2S device Instance */
/*                      clk             Internal/External Clock */
/* Return:              returns 0 on success ; -EINVAL otherwise */
/* *************************************************************************** */
int bcm_i2s_disable_clk(struct i2s_device *handle, enum CLK_SEL clk);

#if (!cpu_is_bcm2153())
/* *************************************************************************** */
/* Function Name:       bcm_i2s_config_data_out() */
/* Description:         Configure pad DAOLR when it's not in use.. */
/* Parameters:          handle          i2S device Instance */
/*                      lr_mode         DAOLR_CTRL value */
/*                      ctrl            DAOD_CTRL value */
/* Return:              returns 0 on success; -EINVAL otherwise */
/* *************************************************************************** */
unsigned int bcm_i2s_config_data_out(struct i2s_device *handle,
				     unsigned int lr_mode, unsigned int ctrl);

/* *************************************************************************** */
/* Function Name:       bcm_i2s_config_data_in() */
/* Description:         Configure pad DAILR when it's not in use.. */
/* Parameters:          handle          i2S device Instance */
/*                      lr_mode         DAILR_CTRL value */
/*                      ctrl            DAID_CTRL value */
/* Return:              returns 0 on success; -EINVAL otherwise */
/* *************************************************************************** */
unsigned int bcm_i2s_config_data_in(struct i2s_device *handle,
				    unsigned int lr_mode, unsigned int ctrl);

/* *************************************************************************** */
/* Function Name:       bcm_i2s_txrx_chnl_size() */
/* Description:         configure Tx/Rx channel size */
/* Parameters:          handle          i2S device Instance */
/*                      direction       PLAYBACK / Capture */
/*                      threshold       Channel size. */
/* Return:              returns 0 on success; -EINVAL otherwise */
/* *************************************************************************** */

unsigned int bcm_i2s_txrx_chnl_size(struct i2s_device *handle,
				    unsigned int direction, unsigned int size);

/* *************************************************************************** */
/* Function Name:       bcm_i2s_data_depth_mod() */
/* Description:         Configure Audio Data Depth. */
/* Parameters:          handle          i2S device Instance */
/*                      direction       PLAYBACK / Capture */
/*                      threshold       Set data depth. */
/* Return:              returns 0 on success; -EINVAL otherwise */
/* *************************************************************************** */
unsigned int bcm_i2s_data_depth_mod(struct i2s_device *handle,
				    unsigned int direction, int data_depth);

/* *************************************************************************** */
/* Function Name:       bcm_i2s_data_depth_mod() */
/* Description:         Configure Audio Data Depth. */
/* Parameters:          handle          i2S device Instance */
/*                      direction       PLAYBACK / Capture */
/*                      data_depth      Set custom data depth. */
/* Return:              returns 0 on success; -EINVAL otherwise */
/* *************************************************************************** */
unsigned int bcm_i2s_data_depth_cfg(struct i2s_device *handle,
				    unsigned int direction, int custom_depth);

/* *************************************************************************** */
/* Function Name:       bcm_i2s_data_align_enable() */
/* Description:         Enable MSB align of data in FIFO */
/* Parameters:          handle          i2S device Instance */
/*                      direction       PLAYBACK / Capture */
/* Return:              returns 0 on success; -EINVAL otherwise */
/* *************************************************************************** */
unsigned int bcm_i2s_data_align_enable(struct i2s_device *handle,
				       unsigned int direction);

/* *************************************************************************** */
/* Function Name:       bcm_i2s_data_align_disable() */
/* Description:         Disable MSB align of data in FIFO */
/* Parameters:          handle          i2S device Instance */
/*                      direction       PLAYBACK / Capture */
/* Return:              returns 0 on success; -EINVAL otherwise */
/* *************************************************************************** */
unsigned int bcm_i2s_data_align_disable(struct i2s_device *handle,
					unsigned int direction);

/* *************************************************************************** */
/* Function Name:       bcm_i2s_just_mode_en() */
/* Description:         Configure Justified I2S mode. */
/* Parameters:          handle          i2S device Instance */
/*                      direction       PLAYBACK / Capture */
/* Return:              returns 0 on success; -EINVAL otherwise */
/* *************************************************************************** */
unsigned int bcm_i2s_just_mode_en(struct i2s_device *handle,
				  unsigned int direction);

/* *************************************************************************** */
/* Function Name:       bcm_i2s_just_mode_dis() */
/* Description:         Configure normal I2S mode. */
/* Parameters:          handle          i2S device Instance */
/*                      direction       PLAYBACK / Capture */
/*                      threshold       Set FIFO threshold levels in DMA Mode. */
/* Return:              returns 0 on success; -EINVAL otherwise */
/* *************************************************************************** */
unsigned int bcm_i2s_just_mode_dis(struct i2s_device *handle,
				   unsigned int direction);

/* *************************************************************************** */
/* Function Name:       bcm_i2s_sclk_stop() */
/* Description:         Configure Audio SCLK STOP Register. */
/* Parameters:          handle          i2S device Instance */
/*                      sck_run_cnt     Number of cycle (=SCK_RUN_CNT + 1) that */
/*                                       SCK will run AFTER channel started. */
/*                      prerun_cnt      Number of cycle that SCK will run BEFORE channel started. */
/* Return:              returns 0 on success; -EINVAL otherwise */
/* *************************************************************************** */
unsigned int bcm_i2s_sclk_stop(struct i2s_device *handle, int sck_run_cnt,
			       int prerun_cnt);

/* *************************************************************************** */
/* Function Name:       bcm_i2s_spdif_enable() */
/* Description:         Enable spdif mode. */
/* Parameters:          handle          i2S device Instance */
/*                      spdif_ch        Set SPDIF Preambles Channel Code. */
/* Return:              returns 0 on success; -EINVAL otherwise */
/* *************************************************************************** */
unsigned int bcm_i2s_spdif_enable(struct i2s_device *handle, int spdif_ch);

/* *************************************************************************** */
/* Function Name:       bcm_i2s_spdif_set_mode() */
/* Description:         . */
/* Parameters:          handle          i2S device Instance */
/*                      preambe         Set PREAMBLE MAP */
/* Return:              returns 0 on success; -EINVAL otherwise */
/* *************************************************************************** */
unsigned int bcm_i2s_spdif_set_mode(struct i2s_device *handle,
				    enum SPDIF_PREAMBLE preamble);
#endif /*(!cpu_is_bcm2153()) */

#endif		/*__LINUX_BCM_I2SDAI_H__*/
