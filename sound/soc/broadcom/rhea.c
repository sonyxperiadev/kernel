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
*  @file   rhea.c
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

int gAudioDebugLevel;
module_param(gAudioDebugLevel, int, 0);

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

static int caph_i2s_startup(struct snd_pcm_substream *substream)
{

	return 0;
}

static struct snd_soc_ops caph_i2s_ops = {
	.startup = caph_i2s_startup,
};

static struct snd_soc_dai_link caph_dai = {
	.name = "caph",
	.stream_name = "caph",
	.cpu_dai_name = "caph-i2s.0",
	.platform_name = "caph-pcm-audio.0",
	.codec_dai_name = "dit-hifi",
	.codec_name = "spdif-dit",
	.init = caph_soc_init,
	.ops = &caph_i2s_ops,
};

static struct snd_soc_card rhea = {
	.name = "rhea",
	.dai_link = &caph_dai,
	.num_links = 1,
};

static struct platform_device *caph_snd_spdif_dit_device;
static struct platform_device *rhea_snd_device;

static int __init rhea_init(void)
{
	int ret;

	caph_snd_spdif_dit_device = platform_device_alloc("spdif-dit", -1);
	if (!caph_snd_spdif_dit_device)
		return -ENOMEM;

	ret = platform_device_add(caph_snd_spdif_dit_device);
	if (ret)
		goto err_unset_spif;

	rhea_snd_device = platform_device_alloc("soc-audio", -1);

	if (!rhea_snd_device)
		return -ENOMEM;

	platform_set_drvdata(rhea_snd_device, &rhea);

	ret = platform_device_add(rhea_snd_device);
	if (ret) {
		pr_err("rhea snd: Failed to add snd soc device: %d\n", ret);
		goto err_unset_pdata;
	}

	return 0;

err_unset_pdata:
	platform_set_drvdata(rhea_snd_device, NULL);
	platform_device_put(rhea_snd_device);
err_unset_spif:
	platform_device_put(caph_snd_spdif_dit_device);

	return ret;
}

module_init(rhea_init);

static void __exit rhea_exit(void)
{
	platform_device_unregister(rhea_snd_device);
}

module_exit(rhea_exit);

MODULE_DESCRIPTION("Broadcom SoC 21654 PCM driver");
MODULE_LICENSE("GPL");
