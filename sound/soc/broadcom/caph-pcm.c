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
*  @file   caph_pcm.c
*
*
****************************************************************************/

#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include <linux/dma-mapping.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

#include "caph-pcm.h"

static struct snd_pcm_substream *substream_record;
static struct snd_pcm_substream *substream_playback;
static void playback_callback(CSL_CAPH_DMA_CHNL_e chnl);
static void record_callback(CSL_CAPH_DMA_CHNL_e chnl);

struct caph_runtime_data {
	CSL_CAPH_DMA_CHNL_e dmaCH;
	unsigned long dma_period;
	dma_addr_t dma_start;
	dma_addr_t dma_pos;
	dma_addr_t dma_end;
};

/* identify hardware playback capabilities */
static const struct snd_pcm_hardware caph_pcm_hardware = {
	.info = (SNDRV_PCM_INFO_MMAP | SNDRV_PCM_INFO_INTERLEAVED |
		 SNDRV_PCM_INFO_BLOCK_TRANSFER | SNDRV_PCM_INFO_MMAP_VALID |
		 SNDRV_PCM_INFO_PAUSE),
	.formats = SNDRV_PCM_FMTBIT_S16_LE,
	.rates =
	    (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_44100 |
	     SNDRV_PCM_RATE_48000),
	.rate_min = 8000,
	.rate_max = 48000,
	.channels_min = 1,
	.channels_max = 2,
	.buffer_bytes_max = PCM_MAX_PLAYBACK_BUF_BYTES,
	.period_bytes_min = PCM_MIN_PLAYBACK_PERIOD_BYTES,
	.period_bytes_max = PCM_MAX_PLAYBACK_PERIOD_BYTES,
	.periods_min = 2,
	.periods_max = 2,	/*limitation for RHEA */

};

static void caph_pcm_start_transfer(struct caph_runtime_data *prtd,
				    struct snd_pcm_substream *substream)
{
}

static void caph_pcm_stop_transfer(struct caph_runtime_data *prtd,
				   struct snd_pcm_substream *substream)
{
	CSL_CAPH_ARM_DSP_e owner = CSL_CAPH_ARM;

	csl_caph_dma_clear_intr(prtd->dmaCH, owner);
	csl_caph_dma_disable_intr(prtd->dmaCH, owner);
	csl_caph_dma_stop_transfer(prtd->dmaCH);
	csl_caph_dma_release_channel(prtd->dmaCH);
}

static void caph_pcm_dma_transfer_done(void *dev_id)
{
	struct snd_pcm_substream *substream = dev_id;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct caph_runtime_data *prtd = runtime->private_data;

	prtd->dma_pos += runtime->period_size;
	if (prtd->dma_pos >= runtime->buffer_size)
		prtd->dma_pos = 0;

	snd_pcm_period_elapsed(substream);

}

static void playback_callback(CSL_CAPH_DMA_CHNL_e chnl)
{

	if ((csl_caph_dma_read_ddrfifo_sw_status(chnl) & CSL_CAPH_READY_LOW) ==
	    CSL_CAPH_READY_NONE) {

		/*  printk(KERN_INFO "fill low half ch=0x%x \r\n", chnl); */

		csl_caph_dma_set_ddrfifo_status(chnl, CSL_CAPH_READY_LOW);
	}

	if ((csl_caph_dma_read_ddrfifo_sw_status(chnl) & CSL_CAPH_READY_HIGH) ==
	    CSL_CAPH_READY_NONE) {

		/* printk(KERN_INFO "fill high half ch=0x%x \r\n", chnl); */

		csl_caph_dma_set_ddrfifo_status(chnl, CSL_CAPH_READY_HIGH);
	}

	caph_pcm_dma_transfer_done(substream_playback);
}

static void record_callback(CSL_CAPH_DMA_CHNL_e chnl)
{
	if ((csl_caph_dma_read_ddrfifo_sw_status(chnl) & CSL_CAPH_READY_LOW) ==
	    CSL_CAPH_READY_NONE) {
		csl_caph_dma_set_ddrfifo_status(chnl, CSL_CAPH_READY_LOW);
	}

	if ((csl_caph_dma_read_ddrfifo_sw_status(chnl) & CSL_CAPH_READY_HIGH) ==
	    CSL_CAPH_READY_NONE) {
		csl_caph_dma_set_ddrfifo_status(chnl, CSL_CAPH_READY_HIGH);
	}

	caph_pcm_dma_transfer_done(substream_record);
}

static int caph_pcm_hw_params(struct snd_pcm_substream *substream,
			      struct snd_pcm_hw_params *params)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct caph_runtime_data *prtd = runtime->private_data;
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_dma_buffer *buf = &substream->dma_buffer;
	struct caph_pcm_config *config;
	CSL_CAPH_DMA_CONFIG_t dmaCfg;

	config = snd_soc_dai_get_dma_data(rtd->cpu_dai, substream);
	if (!config)
		return 0;

	if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		substream_record = substream;
		dmaCfg.direction = CSL_CAPH_DMA_OUT;
		dmaCfg.dmaCB = record_callback;
	} else {
		substream_playback = substream;
		dmaCfg.direction = CSL_CAPH_DMA_IN;
		dmaCfg.dmaCB = playback_callback;
	}
	dmaCfg.dma_ch = config->dmaCH;
	dmaCfg.fifo = config->fifo;
	dmaCfg.Tsize = CSL_AADMAC_TSIZE;
	dmaCfg.mem_addr = (void *)buf->addr;
	dmaCfg.mem_size = buf->bytes;

	csl_caph_dma_config_channel(dmaCfg);
	csl_caph_dma_enable_intr(dmaCfg.dma_ch, CSL_CAPH_ARM);

	snd_pcm_set_runtime_buffer(substream, &substream->dma_buffer);
	runtime->dma_bytes = params_buffer_bytes(params);

	prtd->dmaCH = config->dmaCH;
	prtd->dma_period = params_period_bytes(params);
	prtd->dma_start = runtime->dma_addr;
	prtd->dma_pos = prtd->dma_start;
	prtd->dma_end = prtd->dma_start + runtime->dma_bytes;
	return 0;
}

static int caph_pcm_hw_free(struct snd_pcm_substream *substream)
{

	snd_pcm_set_runtime_buffer(substream, NULL);
	return 0;
}

static int caph_pcm_prepare(struct snd_pcm_substream *substream)
{
	struct caph_runtime_data *prtd = substream->runtime->private_data;
	prtd->dma_pos = 0;
	return 0;
}

static int caph_pcm_trigger(struct snd_pcm_substream *substream, int cmd)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct caph_runtime_data *prtd = runtime->private_data;

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
		caph_pcm_start_transfer(prtd, substream);
		break;
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		caph_pcm_stop_transfer(prtd, substream);
		break;
	default:
		break;
	}
	return 0;
}

static snd_pcm_uframes_t caph_pcm_pointer(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct caph_runtime_data *prtd = runtime->private_data;
	snd_pcm_uframes_t offset;

	offset = prtd->dma_pos + bytes_to_frames(runtime, 0);
	return offset;
}

static int caph_pcm_open(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct caph_runtime_data *prtd;

	prtd = kzalloc(sizeof(*prtd), GFP_KERNEL);
	if (prtd == NULL)
		return -ENOMEM;

	snd_soc_set_runtime_hwparams(substream, &caph_pcm_hardware);

	runtime->private_data = prtd;
	return 0;
}

static int caph_pcm_close(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct caph_runtime_data *prtd = runtime->private_data;

	kfree(prtd);
	return 0;
}

static int caph_pcm_mmap(struct snd_pcm_substream *substream,
			 struct vm_area_struct *vma)
{
	return remap_pfn_range(vma, vma->vm_start,
			       substream->dma_buffer.addr >> PAGE_SHIFT,
			       vma->vm_end - vma->vm_start, vma->vm_page_prot);
}

static struct snd_pcm_ops caph_pcm_ops = {
	.open = caph_pcm_open,
	.close = caph_pcm_close,
	.ioctl = snd_pcm_lib_ioctl,
	.hw_params = caph_pcm_hw_params,
	.hw_free = caph_pcm_hw_free,
	.prepare = caph_pcm_prepare,
	.trigger = caph_pcm_trigger,
	.pointer = caph_pcm_pointer,
	.mmap = caph_pcm_mmap,
};

static int caph_pcm_preallocate_dma_buffer(struct snd_pcm *pcm, int stream)
{
	struct snd_pcm_substream *substream = pcm->streams[stream].substream;
	struct snd_dma_buffer *buf = &substream->dma_buffer;
	size_t size = caph_pcm_hardware.buffer_bytes_max;

	buf->dev.type = SNDRV_DMA_TYPE_DEV;
	buf->dev.dev = pcm->card->dev;
	buf->private_data = NULL;

	buf->area = dma_alloc_writecombine(pcm->card->dev, size,
					   &buf->addr, GFP_KERNEL);
	if (!buf->area)
		return -ENOMEM;

	buf->bytes = size;
	return 0;
}

static void caph_pcm_free(struct snd_pcm *pcm)
{
	struct snd_pcm_substream *substream;
	struct snd_dma_buffer *buf;
	int stream;

	for (stream = 0; stream < SNDRV_PCM_STREAM_LAST; ++stream) {
		substream = pcm->streams[stream].substream;
		if (!substream)
			continue;

		buf = &substream->dma_buffer;
		if (!buf->area)
			continue;

		dma_free_writecombine(pcm->card->dev, buf->bytes, buf->area,
				      buf->addr);
		buf->area = NULL;
	}
}

static u64 caph_pcm_dmamask = DMA_BIT_MASK(32);

int caph_pcm_new(struct snd_card *card, struct snd_soc_dai *dai,
		 struct snd_pcm *pcm)
{
	int ret = 0;

	if (!card->dev->dma_mask)
		card->dev->dma_mask = &caph_pcm_dmamask;

	if (!card->dev->coherent_dma_mask)
		card->dev->coherent_dma_mask = DMA_BIT_MASK(32);

	if (dai->driver->playback.channels_min) {
		ret = caph_pcm_preallocate_dma_buffer(pcm,
			SNDRV_PCM_STREAM_PLAYBACK);
		if (ret)
			goto err;
	}

	if (dai->driver->capture.channels_min) {
		ret = caph_pcm_preallocate_dma_buffer(pcm,
						      SNDRV_PCM_STREAM_CAPTURE);
		if (ret)
			goto err;
	}

err:
	return ret;
}

static struct snd_soc_platform_driver caph_soc_platform = {
	.ops = &caph_pcm_ops,
	.pcm_new = caph_pcm_new,
	.pcm_free = caph_pcm_free,
};

static int __devinit caph_pcm_probe(struct platform_device *pdev)
{
	return snd_soc_register_platform(&pdev->dev, &caph_soc_platform);
}

static int __devexit caph_pcm_remove(struct platform_device *pdev)
{
	snd_soc_unregister_platform(&pdev->dev);
	return 0;
}

static struct platform_driver caph_pcm_driver = {
	.probe = caph_pcm_probe,
	.remove = __devexit_p(caph_pcm_remove),
	.driver = {
		   .name = "caph-pcm-audio",
		   .owner = THIS_MODULE,
		   },
};

static int __init caph_soc_platform_init(void)
{
	return platform_driver_register(&caph_pcm_driver);
}

module_init(caph_soc_platform_init);

static void __exit caph_soc_platform_exit(void)
{
	return platform_driver_unregister(&caph_pcm_driver);
}

module_exit(caph_soc_platform_exit);

MODULE_DESCRIPTION("Broadcom SoC 21654 PCM driver");
MODULE_LICENSE("GPL");
