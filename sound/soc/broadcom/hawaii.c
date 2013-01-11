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
*  @file   hawaii.c
*
*
****************************************************************************/
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <linux/gpio.h>
#include "audio_trace.h"

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <linux/gpio.h>
#include "audio_trace.h"

#include "mobcom_types.h"
#include "chal_types.h"
#include "chal_sspi.h"
#include "chal_caph.h"
#include "csl_caph.h"
#include "resultcode.h"
#include "csl_caph_i2s_sspi.h"
#include "csl_caph_cfifo.h"
#include "csl_caph_switch.h"
#include "csl_caph_hwctrl.h"
#include "caph-i2s.h"

#include "clock.h"
#include "clk.h"
#include "brcm_rdb_khub_clk_mgr_reg.h"

int gAudioDebugLevel = 15;
module_param(gAudioDebugLevel, int, 0);
static Boolean useclk = FALSE;

#define CAPH_SOC_DAIFMT (SND_SOC_DAIFMT_I2S)

static int caph_soc_init(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_codec *codec = rtd->codec;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	int ret;
	ret = snd_soc_dai_set_fmt(cpu_dai, CAPH_SOC_DAIFMT);
	if (ret < 0) {
		dev_err(codec->dev, "Failed to set cpu dai format: %d\n", ret);
		return ret;
	}
	return 0;
}

static int hawaii_startup(struct snd_pcm_substream *substream)
{
	return 0;
}

/*****************************************************************************
*
*  Function Name: hawaii_hw_params
*
*  Description: Enable clocks
*
*****************************************************************************/
static int hawaii_hw_params(struct snd_pcm_substream *substream,
					struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct caph_i2s *i2s = snd_soc_dai_get_drvdata(cpu_dai);

	if (i2s->fmTxRunning == 0 && i2s->fmRxRunning == 0 && !useclk) {
		csl_caph_ControlHWClock(TRUE);
		ssp_ControlHWClock(TRUE);
		useclk = TRUE;
	}
	return 0;
}

/*****************************************************************************
*
*  Function Name: hawaii_hw_free
*
*  Description: Dissable clocks, if both playback and capture is finished
*
*****************************************************************************/
static int hawaii_hw_free(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct caph_i2s *i2s = snd_soc_dai_get_drvdata(cpu_dai);

	if (i2s->fmTxRunning == 0 && i2s->fmRxRunning == 0 && useclk) {
		ssp_ControlHWClock(FALSE);
		csl_caph_ControlHWClock(FALSE);
		useclk = FALSE;
	}
	return 0;
}

static struct snd_soc_ops hawaii_ops = {
	.startup = hawaii_startup,
	.hw_params = hawaii_hw_params,
	.hw_free = hawaii_hw_free,
};

static struct snd_soc_dai_link hawaii_dai = {
	.name = "caph",
	.stream_name = "caph",
	.cpu_dai_name = "caph-i2s.0",
	.platform_name = "caph-pcm-audio.0",
	.codec_dai_name = "dit-hifi",
	.codec_name = "spdif-dit.0",
	.init = caph_soc_init,
	.ops = &hawaii_ops,
};

static struct snd_soc_card hawaii = {
	.name = "hawaii",
	.owner = THIS_MODULE,
	.dai_link = &hawaii_dai,
	.num_links = 1,
};

static int __devinit hawaii_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card = &hawaii;
	int ret;
	card->dev = &pdev->dev;

	ret = snd_soc_register_card(card);
	if (ret)
		pr_info(
			"Machine driver: hawaii_probe()"
			"snd_soc_register_card() failed: %d\n", ret);

	return ret;
}

static int __devexit hawaii_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);
	snd_soc_unregister_card(card);
	return 0;
}

static struct platform_driver hawaii_driver = {
	.probe = hawaii_probe,
	.remove = __devexit_p(hawaii_remove),
	.driver = {
		   .name = "hawaii-audio",
		   .owner = THIS_MODULE,
		   },
};
static int __init hawaii_init(void)
{
	int ret;
	ret = platform_driver_register(&hawaii_driver);
	if (ret)
		pr_info(
			"Machine driver: hawaii_init():"
			" platform_driver_register fails %d", ret);
	return ret;
}

module_init(hawaii_init);

static void __exit hawaii_exit(void)
{
	return platform_driver_unregister(&hawaii_driver);
}

module_exit(hawaii_exit);

MODULE_DESCRIPTION("Broadcom SoC 21664 PCM driver");
MODULE_LICENSE("GPL");
