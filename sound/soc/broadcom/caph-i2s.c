/******************************************************************************/
/* Copyright 2009, 2010 Broadcom Corporation.  All rights reserved.           */
/*     Unless you and Broadcom execute a separate written software license    */
/*     agreement governing use of this software, this software is licensed to */
/*     you under the terms of the GNU General Public License version 2        */
/*    (the GPL), available at                                                 */
/*                                                                            */
/*          http://www.broadcom.com/licenses/GPLv2.php                        */
/*                                                                            */
/*     with the following added to such license:                              */
/*                                                                            */
/*     As a special exception, the copyright holders of this software give    */
/*     you permission to link this software with independent modules, and to  */
/*     copy and distribute the resulting executable under terms of your       */
/*     choice, provided that you also meet, for each linked independent       */
/*     module, the terms and conditions of the license of that module.        */
/*     An independent module is a module which is not derived from this       */
/*     software.  The special exception does not apply to any modifications   */
/*     of the software.                                                       */
/*                                                                            */
/*     Notwithstanding the above, under no circumstances may you combine this */
/*     software in any way with any other Broadcom software provided under a  */
/*     license other than the GPL, without Broadcom's express prior written   */
/*     consent.                                                               */
/******************************************************************************/

/**
*
*  @file   caph_i2s.c
*
*
****************************************************************************/

#include <linux/init.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include <linux/clk.h>
#include <linux/delay.h>

#include <linux/dma-mapping.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/initval.h>

#include <string.h>
#include "mobcom_types.h"
#include "chal_types.h"
#include "chal_sspi.h"
#include "csl_caph_i2s_sspi.h"
#include "brcm_rdb_sspil.h"
#include "brcm_rdb_padctrlreg.h"
#include "brcm_rdb_sysmap.h"
#include "log.h"
#include "resultcode.h"
#include "io_map.h"
#include "chal_caph.h"
#include "csl_caph.h"
#include "csl_caph_cfifo.h"
#include "csl_caph_switch.h"
#include "csl_caph_hwctrl.h"

#include "caph-i2s.h"
#include "caph-pcm.h"

struct caph_i2s {
	struct resource *mem;
	void __iomem *base;
	dma_addr_t phys_base;

	struct clk *clk_aic;
	struct clk *clk_i2s;

	CSL_HANDLE fmHandleSSP;
	CAPH_SWITCH_TRIGGER_e fmTxTrigger;
	CAPH_SWITCH_TRIGGER_e fmRxTrigger;
	CSL_I2S_CONFIG_t fmCfg;
	Boolean fmTxRunning;
	Boolean fmRxRunning;

	struct caph_pcm_config pcm_config_playback;
	struct caph_pcm_config pcm_config_capture;
};

static inline uint32_t caph_i2s_read(const struct caph_i2s *i2s,
				     unsigned int reg)
{
	return 0;
}

static inline void caph_i2s_write(const struct caph_i2s *i2s,
				  unsigned int reg, uint32_t value)
{
}

static int caph_i2s_startup(struct snd_pcm_substream *substream,
			    struct snd_soc_dai *dai)
{
	return 0;
}

static void caph_i2s_shutdown(struct snd_pcm_substream *substream,
			      struct snd_soc_dai *dai)
{

}

static int caph_i2s_trigger(struct snd_pcm_substream *substream, int cmd,
			    struct snd_soc_dai *dai)
{
	struct caph_i2s *i2s = snd_soc_dai_get_drvdata(dai);
	struct caph_pcm_config *pcm_config;

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
			pcm_config = &i2s->pcm_config_playback;
		else
			pcm_config = &i2s->pcm_config_capture;

		/* start cfifo */
		csl_caph_cfifo_start_fifo(pcm_config->fifo);

		/* start switch */
		csl_caph_switch_start_transfer(pcm_config->sw);

		/* start i2s */
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			if (!i2s->fmTxRunning) {
				csl_sspi_enable_scheduler(i2s->fmHandleSSP, 1);
				csl_i2s_start_tx(i2s->fmHandleSSP, &i2s->fmCfg);
				i2s->fmTxRunning = TRUE;
			}
		} else {
			if (!i2s->fmRxRunning) {
				csl_sspi_enable_scheduler(i2s->fmHandleSSP, 1);
				csl_i2s_start_rx(i2s->fmHandleSSP, &i2s->fmCfg);
				i2s->fmRxRunning = TRUE;
			}
		}

		/* start DMA */
		csl_caph_dma_start_transfer(pcm_config->dmaCH);
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
			pcm_config = &i2s->pcm_config_playback;
		else
			pcm_config = &i2s->pcm_config_capture;

		/* stop cfifo */
		csl_caph_cfifo_stop_fifo(pcm_config->fifo);
		csl_caph_cfifo_release_fifo(pcm_config->fifo);

		/* stop swicth */
		csl_caph_switch_stop_transfer(pcm_config->sw);
		csl_caph_switch_release_channel(pcm_config->sw);

		/* stop i2s */
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			if (i2s->fmTxRunning == TRUE) {
				csl_i2s_stop_tx(i2s->fmHandleSSP);
				i2s->fmTxRunning = FALSE;
				if (i2s->fmTxRunning == FALSE
				    && i2s->fmRxRunning == FALSE)
					csl_sspi_enable_scheduler(i2s->
								  fmHandleSSP,
								  0);
			}
		} else {

			if (i2s->fmRxRunning == TRUE) {
				csl_i2s_stop_rx(i2s->fmHandleSSP);
				i2s->fmRxRunning = FALSE;
				if (i2s->fmTxRunning == FALSE
				    && i2s->fmRxRunning == FALSE)
					csl_sspi_enable_scheduler(i2s->
								  fmHandleSSP,
								  0);
			}
		}
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int caph_i2s_set_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
	return 0;
}

static int caph_i2s_hw_params(struct snd_pcm_substream *substream,
			      struct snd_pcm_hw_params *params,
			      struct snd_soc_dai *dai)
{
	struct caph_i2s *i2s = snd_soc_dai_get_drvdata(dai);
	struct caph_pcm_config *pcm_config;
	CSL_CAPH_CFIFO_FIFO_e fifo;
	CSL_CAPH_CFIFO_DIRECTION_e direction = CSL_CAPH_CFIFO_IN;
	UInt16 threshold;
	CSL_CAPH_SWITCH_CONFIG_t swCfg;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		pcm_config = &i2s->pcm_config_playback;
	else
		pcm_config = &i2s->pcm_config_capture;

	/* configur CFIFO */
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		fifo = CSL_CAPH_CFIFO_FIFO1;

	} else {
		fifo = CSL_CAPH_CFIFO_FIFO2;
		direction = CSL_CAPH_CFIFO_OUT;

	}
	threshold = csl_caph_cfifo_get_fifo_thres(fifo);
	csl_caph_cfifo_config_fifo(fifo, direction, threshold);

	/* configur swicth */
	pcm_config->sw = csl_caph_switch_obtain_channel();
	swCfg.chnl = pcm_config->sw;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		swCfg.FIFO_srcAddr = csl_caph_cfifo_get_fifo_addr(fifo);
		swCfg.trigger = i2s->fmTxTrigger;
		swCfg.dataFmt = CSL_CAPH_16BIT_STEREO;
		swCfg.FIFO_dstAddr =
		    csl_i2s_get_tx0_fifo_data_port(i2s->fmHandleSSP);
	} else {
		swCfg.FIFO_srcAddr =
		    csl_i2s_get_rx0_fifo_data_port(i2s->fmHandleSSP);
		swCfg.trigger = i2s->fmRxTrigger;
		swCfg.dataFmt = CSL_CAPH_16BIT_STEREO;
		swCfg.FIFO_dstAddr = csl_caph_cfifo_get_fifo_addr(fifo);
	}
	csl_caph_switch_config_channel(swCfg);

	/* configur i2s port */
	if (!i2s->fmTxRunning && !i2s->fmRxRunning) {
		i2s->fmCfg.mode = CSL_I2S_MASTER_MODE;
		i2s->fmCfg.tx_ena = 1;
		i2s->fmCfg.rx_ena = 1;
		i2s->fmCfg.tx_loopback_ena = 0;
		i2s->fmCfg.rx_loopback_ena = 0;

		i2s->fmCfg.trans_size = CSL_I2S_SSP_TSIZE;
		i2s->fmCfg.prot = SSPI_HW_I2S_MODE2;
		i2s->fmCfg.interleave = TRUE;

		i2s->fmCfg.sampleRate = CSL_I2S_16BIT_48000HZ;
		csl_i2s_config(i2s->fmHandleSSP, &i2s->fmCfg);
	}

	/* configur DMA channel */
	pcm_config->dmaCH = csl_caph_dma_obtain_channel();
	pcm_config->fifo = fifo;
	snd_soc_dai_set_dma_data(dai, substream, pcm_config);

	return 0;
}

static int caph_i2s_set_sysclk(struct snd_soc_dai *dai, int clk_id,
			       unsigned int freq, int dir)
{
	return 0;
}

static int caph_i2s_suspend(struct snd_soc_dai *dai)
{
	return 0;
}

static int caph_i2s_resume(struct snd_soc_dai *dai)
{
	return 0;
}

static int caph_i2s_dai_probe(struct snd_soc_dai *dai)
{
	return 0;
}

static int caph_i2s_dai_remove(struct snd_soc_dai *dai)
{
	return 0;
}

static struct snd_soc_dai_ops caph_i2s_dai_ops = {
	.startup = caph_i2s_startup,
	.shutdown = caph_i2s_shutdown,
	.trigger = caph_i2s_trigger,
	.hw_params = caph_i2s_hw_params,
	.set_fmt = caph_i2s_set_fmt,
	.set_sysclk = caph_i2s_set_sysclk,
};

static struct snd_soc_dai_driver caph_i2s_dai = {
	.probe = caph_i2s_dai_probe,
	.remove = caph_i2s_dai_remove,
	.playback = {
		     .channels_min = 1,
		     .channels_max = 2,
		     .rates = SNDRV_PCM_RATE_48000,
		     .formats = SNDRV_PCM_FMTBIT_S16_LE,
		     },
	.capture = {
		    .channels_min = 2,
		    .channels_max = 2,
		    .rates = SNDRV_PCM_RATE_48000,
		    .formats = SNDRV_PCM_FMTBIT_S16_LE,
		    },
	.symmetric_rates = 1,
	.ops = &caph_i2s_dai_ops,
	.suspend = caph_i2s_suspend,
	.resume = caph_i2s_resume,
};

static int __devinit caph_i2s_dev_probe(struct platform_device *pdev)
{
	struct caph_i2s *i2s;
	int ret;

	i2s = kzalloc(sizeof(*i2s), GFP_KERNEL);
	if (!i2s)
		return -ENOMEM;

	/* initialize caph  */
	csl_caph_hwctrl_init();
	csl_caph_ControlHWClock(TRUE);

	/* use SSP4 as I2S port */

	i2s->fmTxTrigger = CAPH_SSP4_TX0_TRIGGER;
	i2s->fmRxTrigger = CAPH_SSP4_RX0_TRIGGER;
	i2s->fmHandleSSP = (CSL_HANDLE) csl_i2s_init(KONA_SSP4_BASE_VA);
	i2s->fmTxRunning = FALSE;
	i2s->fmRxRunning = FALSE;

	platform_set_drvdata(pdev, i2s);
	ret = snd_soc_register_dai(&pdev->dev, &caph_i2s_dai);

	if (ret) {
		dev_err(&pdev->dev, "Failed to register DAI\n");
		goto err_register_i2s;
	}

	return 0;

err_register_i2s:

	kfree(i2s);
	csl_caph_ControlHWClock(FALSE);

	return ret;
}

static int __devexit caph_i2s_dev_remove(struct platform_device *pdev)
{
	struct caph_i2s *i2s = platform_get_drvdata(pdev);

	csl_caph_hwctrl_deinit();
	snd_soc_unregister_dai(&pdev->dev);

	clk_put(i2s->clk_i2s);
	clk_put(i2s->clk_aic);

	iounmap(i2s->base);
	release_mem_region(i2s->mem->start, resource_size(i2s->mem));

	platform_set_drvdata(pdev, NULL);
	kfree(i2s);
	csl_caph_ControlHWClock(FALSE);

	return 0;
}

static struct platform_driver caph_i2s_driver = {
	.probe = caph_i2s_dev_probe,
	.remove = __devexit_p(caph_i2s_dev_remove),
	.driver = {
		   .name = "caph-i2s",
		   .owner = THIS_MODULE,
		   },
};

static int __init caph_i2s_init(void)
{
	return platform_driver_register(&caph_i2s_driver);
}

module_init(caph_i2s_init);

static void __exit caph_i2s_exit(void)
{
	platform_driver_unregister(&caph_i2s_driver);
}

module_exit(caph_i2s_exit);

MODULE_DESCRIPTION("Broadcom 21654 SoC I2S driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:21654");
