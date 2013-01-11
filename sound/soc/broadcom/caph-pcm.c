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

#include "chal_caph.h"
#include "caph-pcm.h"
#include "caph-i2s.h"
#include "csl_caph_switch.h"

static struct snd_pcm_substream *substream_record;
static struct snd_pcm_substream *substream_playback;
/*Callback function for playback DMA interrupt*/
static void playback_callback(CSL_CAPH_DMA_CHNL_e chnl);
/*Callback function for capture DMA interrupt*/
static void record_callback(CSL_CAPH_DMA_CHNL_e chnl);

/* snd_pcm_runtime private data structure*/
struct caph_runtime_data {
	CSL_CAPH_DMA_CHNL_e dmaCH;
	CSL_CAPH_DMA_CHNL_FIFO_STATUS_e status;
	unsigned long dma_period;
	int num_block;
	unsigned long block_index;
	dma_addr_t dma_start;
	dma_addr_t dma_pos;
	dma_addr_t dma_end;
};

/* identify hardware playback capabilities */
static const struct snd_pcm_hardware caph_pcm_hardware_playback = {
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
	.periods_max = 2,

};
/* identify hardware capture capabilities */
static const struct snd_pcm_hardware caph_pcm_hardware_capture = {
	.info = (SNDRV_PCM_INFO_MMAP | SNDRV_PCM_INFO_INTERLEAVED |
		 SNDRV_PCM_INFO_BLOCK_TRANSFER | SNDRV_PCM_INFO_MMAP_VALID),
	.formats = SNDRV_PCM_FMTBIT_S16_LE,
	.rates =
	    (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_44100 |
	     SNDRV_PCM_RATE_48000),
	.rate_min = 8000,
	.rate_max = 48000,
	.channels_min = 1,
	.channels_max = 2,
	.buffer_bytes_max = PCM_MAX_CAPTURE_BUF_BYTES,
	.period_bytes_min = PCM_MIN_CAPTURE_PERIOD_BYTES,
	.period_bytes_max = PCM_MAX_CAPTURE_PERIOD_BYTES,
	.periods_min = 2,
	.periods_max = 2,

};

/*****************************************************************************
*
*  Function Name: caph_pcm_configure
*
*  Description: configure DMA
*
*****************************************************************************/
static void caph_pcm_configure(struct snd_pcm_substream *substream,
					struct caph_runtime_data *prtd)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *dai = rtd->cpu_dai;
	struct caph_i2s *i2s = snd_soc_dai_get_drvdata(dai);
	struct snd_dma_buffer *buf = &substream->dma_buffer;
	struct caph_pcm_config *config;
	CSL_CAPH_DMA_CONFIG_t dmaCfg;

	pr_info("caph-pcm: caph_pcm_configure()");
	if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		substream_record = substream;

		dmaCfg.direction = CSL_CAPH_DMA_OUT;
		dmaCfg.dmaCB = record_callback;
		dmaCfg.fifo = CSL_CAPH_CFIFO_FIFO2;
		config = &(i2s->pcm_config_capture);
	} else {
		substream_playback = substream;

		dmaCfg.direction = CSL_CAPH_DMA_IN;
		dmaCfg.dmaCB = playback_callback;
		dmaCfg.fifo = CSL_CAPH_CFIFO_FIFO1;
		config = &i2s->pcm_config_playback;
	}

	dmaCfg.dma_ch = csl_caph_dma_obtain_channel();
	dmaCfg.Tsize = CSL_AADMAC_TSIZE;
	dmaCfg.mem_addr = (void *)buf->addr;
	dmaCfg.dma_buf_size = prtd->dma_period;
	dmaCfg.n_dma_buf = prtd->num_block;
	dmaCfg.mem_size = prtd->dma_period*prtd->num_block;

	csl_caph_dma_config_channel(dmaCfg);
	csl_caph_dma_enable_intr(dmaCfg.dma_ch, CSL_CAPH_ARM);

	config->dmaCH = dmaCfg.dma_ch;
	config->fifo = dmaCfg.fifo;
	config->sw = csl_caph_switch_obtain_channel();
	prtd->dmaCH = config->dmaCH;
	snd_soc_dai_set_dma_data(dai, substream, config);
}

/*****************************************************************************
*
* Function Name: static void caph_pcm_start_transfer(struct caph_runtime_data
*				*prtd, struct snd_pcm_substream *substream)
*
* Description: Start DMA
*
*****************************************************************************/
static void caph_pcm_start_transfer(struct caph_runtime_data *prtd,
				    struct snd_pcm_substream *substream)
{
/*No need to start DMA again, it is already start in i2s_dai trigger function*/
}

/*****************************************************************************
*
* Function Name: static void caph_pcm_stop_transfer(struct caph_runtime_data
*				*prtd, struct snd_pcm_substream *substream)
*
* Description: Stop DMA
*
*****************************************************************************/
static void caph_pcm_stop_transfer(struct caph_runtime_data *prtd,
				   struct snd_pcm_substream *substream)
{
	CSL_CAPH_ARM_DSP_e owner = CSL_CAPH_ARM;
	pr_info("caph-pcm: caph_pcm_stop_transfer");
	csl_caph_dma_clear_intr(prtd->dmaCH, owner);
	csl_caph_dma_disable_intr(prtd->dmaCH, owner);
	csl_caph_dma_stop_transfer(prtd->dmaCH);
	csl_caph_dma_release_channel(prtd->dmaCH);
}

/*****************************************************************************
*
* Function Name: static void caph_pcm_dma_transfer_done(void *dev_id)
* Parameters:
* @dev_id: playback_substream or capture_substream
*
* Description: Change DMA position and inform to upper layer
*
*****************************************************************************/
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

/*****************************************************************************
*
* Function Name: static void playback_callback(CSL_CAPH_DMA_CHNL_e chnl)
*
* Description: Callback function for playback DMA interrupt
*
*****************************************************************************/
static void playback_callback(CSL_CAPH_DMA_CHNL_e chnl)
{
	struct snd_pcm_runtime *runtime = substream_playback->runtime;
	struct caph_runtime_data *prtd = runtime->private_data;
	CSL_CAPH_DMA_CONFIG_t dmaCfg;
	struct snd_dma_buffer *buf = &substream_playback->dma_buffer;
	CSL_CAPH_DMA_CHNL_FIFO_STATUS_e fifo_sataus = CSL_CAPH_READY_NONE;

	fifo_sataus = csl_caph_dma_read_ddrfifo_sw_status(chnl);
	if ((fifo_sataus & CSL_CAPH_READY_HIGH) == CSL_CAPH_READY_NONE) {

		prtd->status = CSL_CAPH_READY_HIGH;
		if (fifo_sataus == CSL_CAPH_READY_NONE) {
			pr_info("underrun condition\n");
			prtd->status = CSL_CAPH_READY_HIGHLOW;
			dmaCfg.mem_addr = (void *)(buf->addr +
					(prtd->block_index*prtd->dma_period));
			memset(phys_to_virt((UInt32)dmaCfg.mem_addr), 0,
						prtd->dma_period);
			csl_caph_dma_set_lobuffer_address(chnl,
							dmaCfg.mem_addr);
		}

		caph_pcm_dma_transfer_done(substream_playback);

		prtd->block_index++;
		if (prtd->block_index >= prtd->num_block)
			prtd->block_index = 0;
		dmaCfg.mem_addr = (void *)(buf->addr +
					(prtd->block_index*prtd->dma_period));
		csl_caph_dma_set_hibuffer_address(chnl, dmaCfg.mem_addr);

	} else if ((fifo_sataus & CSL_CAPH_READY_LOW) == CSL_CAPH_READY_NONE) {

		prtd->status = CSL_CAPH_READY_LOW;
		caph_pcm_dma_transfer_done(substream_playback);

		prtd->block_index++;
		if (prtd->block_index >= prtd->num_block)
			prtd->block_index = 0;
		dmaCfg.mem_addr = (void *)(buf->addr +
					(prtd->block_index*prtd->dma_period));
		csl_caph_dma_set_lobuffer_address(chnl, dmaCfg.mem_addr);

	}
}

/*****************************************************************************
*
* Function Name: static void record_callback(CSL_CAPH_DMA_CHNL_e chnl)
*
* Description: Callback function for capture DMA interrupt
*
*****************************************************************************/
static void record_callback(CSL_CAPH_DMA_CHNL_e chnl)
{
	struct snd_pcm_runtime *runtime = substream_record->runtime;
	struct caph_runtime_data *prtd = runtime->private_data;
	CSL_CAPH_DMA_CONFIG_t dmaCfg;
	struct snd_dma_buffer *buf = &substream_record->dma_buffer;
	CSL_CAPH_DMA_CHNL_FIFO_STATUS_e fifo_sataus = CSL_CAPH_READY_NONE;

	fifo_sataus = csl_caph_dma_read_ddrfifo_sw_status(chnl);
	if ((fifo_sataus & CSL_CAPH_READY_LOW) == CSL_CAPH_READY_NONE) {

		prtd->status = CSL_CAPH_READY_LOW;
		prtd->block_index++;
		if (prtd->block_index >= prtd->num_block)
			prtd->block_index = 0;
		dmaCfg.mem_addr = (void *)(buf->addr +
					(prtd->block_index*prtd->dma_period));
		csl_caph_dma_set_lobuffer_address(chnl, dmaCfg.mem_addr);

	} else if ((fifo_sataus & CSL_CAPH_READY_HIGH) == CSL_CAPH_READY_NONE) {

		prtd->status = CSL_CAPH_READY_HIGH;
		prtd->block_index++;
		if (prtd->block_index >= prtd->num_block)
			prtd->block_index = 0;
		dmaCfg.mem_addr = (void *)(buf->addr +
					(prtd->block_index*prtd->dma_period));
		csl_caph_dma_set_hibuffer_address(chnl, dmaCfg.mem_addr);

	}
	caph_pcm_dma_transfer_done(substream_record);
}

/*****************************************************************************
*
*  Function Name: caph_pcm_hw_params
*
*  Description: configure runtime dma buffer
*
*****************************************************************************/
static int caph_pcm_hw_params(struct snd_pcm_substream *substream,
			      struct snd_pcm_hw_params *params)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct caph_runtime_data *prtd = runtime->private_data;

	snd_pcm_set_runtime_buffer(substream, &substream->dma_buffer);
	runtime->dma_bytes = params_buffer_bytes(params);

	prtd->dma_period = params_period_bytes(params);
	prtd->num_block = 2;
	prtd->block_index = 1;
	prtd->dma_start = runtime->dma_addr;
	prtd->dma_pos = prtd->dma_start;
	prtd->dma_end = prtd->dma_start + runtime->dma_bytes;

	return 0;
}

/*****************************************************************************
*
*  Function Name: caph_pcm_hw_free
*
*  Description: set runtime dma buffer to NULL
*
*****************************************************************************/
static int caph_pcm_hw_free(struct snd_pcm_substream *substream)
{
	snd_pcm_set_runtime_buffer(substream, NULL);
	return 0;
}

/*****************************************************************************
*
*  Function Name: caph_pcm_prepare
*
*  Description: configure switch, cfifo, i2s port and DMA
*
*****************************************************************************/
static int caph_pcm_prepare(struct snd_pcm_substream *substream)
{
	struct caph_runtime_data *prtd = substream->runtime->private_data;

	caph_pcm_configure(substream, prtd);
	prtd->dma_pos = 0;
	return 0;
}

/*****************************************************************************
*
*  Function Name: caph_i2s_trigger
*
*  Description: Command handling function for start, stop for DMA
****************************************************************************/
static int caph_pcm_trigger(struct snd_pcm_substream *substream, int cmd)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct caph_runtime_data *prtd = runtime->private_data;
	pr_info("caph-pcm: caph_pcm_trigger() cmd: %d", cmd);
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

/*****************************************************************************
*
*  Function Name: caph_pcm_pointer
*
*  Description: Get DMA pointer in frames
*
*****************************************************************************/
static snd_pcm_uframes_t caph_pcm_pointer(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct caph_runtime_data *prtd = runtime->private_data;
	snd_pcm_uframes_t offset;

	offset = prtd->dma_pos + bytes_to_frames(runtime, 0);
	return offset;
}

/*****************************************************************************
*
*  Function Name: caph_pcm_open
*
*  Description: Open pcm device for playback or capture
*
*****************************************************************************/
static int caph_pcm_open(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct caph_runtime_data *prtd;

	prtd = kzalloc(sizeof(struct caph_runtime_data), GFP_KERNEL);
	if (prtd == NULL)
		return -ENOMEM;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		snd_soc_set_runtime_hwparams(substream,
						&caph_pcm_hardware_playback);
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		snd_soc_set_runtime_hwparams(substream,
						&caph_pcm_hardware_capture);

	runtime->private_data = prtd;
	return 0;
}

/*****************************************************************************
*
*  Function Name: caph_pcm_close
*
*  Description: Close pcm device for playback or capture
*
*****************************************************************************/
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

/*****************************************************************************
*
*  Function Name: caph_pcm_copy
*
*  Description: copy between user and kernel for playback and capture
*
*****************************************************************************/
static int caph_pcm_copy(struct snd_pcm_substream *substream, int channel,
	    snd_pcm_uframes_t pos, void __user *buf, snd_pcm_uframes_t count)
{

	struct snd_pcm_runtime *runtime = substream->runtime;
	struct caph_runtime_data *prtd = runtime->private_data;
	char *buf_start = runtime->dma_area;
	int period_bytes = frames_to_bytes(runtime, runtime->period_size);
	char *hwbuf = runtime->dma_area + frames_to_bytes(runtime, pos);
	int bytes_to_copy = frames_to_bytes(runtime, count);
	int not_copied = 0;
	char *new_hwbuf = NULL;
	int periods_copied;

	if (substream == substream_playback)
		not_copied = copy_from_user(hwbuf, buf, bytes_to_copy);
	else if (substream == substream_record)
		not_copied = copy_to_user(buf, hwbuf, bytes_to_copy);

	new_hwbuf = hwbuf + (bytes_to_copy - not_copied);
	if (not_copied) {
		pr_info(
			"%s: why didn't copy all the bytes?"
			" not_copied = %d, to_copy = %d\n",
			__func__, not_copied, bytes_to_copy);
		pr_info(
			"%s: stream = %d,"
			" buf_start = %p, hwbuf = %p, "
			"new_hwbuf = %p,period_bytes = %d,"
			" bytes_to_copy = %d, not_copied = %d\n",
			__func__, substream->number, buf_start, hwbuf,
		new_hwbuf, period_bytes, bytes_to_copy, not_copied);
	}

	periods_copied = bytes_to_copy/period_bytes;
	while (periods_copied--) {
		csl_caph_dma_set_ddrfifo_status(prtd->dmaCH, prtd->status);
		prtd->status = CSL_CAPH_READY_NONE;
	}
	return 0;
}

/*PCM device oprations*/
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
	.copy = caph_pcm_copy,
};

/*****************************************************************************
*
*  Function Name: int caph_pcm_preallocate_dma_buffer(struct snd_pcm *pcm,
*							int stream)
*  Parameters:
*  @pcm: snd_pcm handle
*  @stream: PLAYBACK or CAPTURE stream
*
*  Description: allocate DMA buffer area, addr, bytes for stream
*
*****************************************************************************/
static int caph_pcm_preallocate_dma_buffer(struct snd_pcm *pcm, int stream)
{
	struct snd_pcm_substream *substream = pcm->streams[stream].substream;
	struct snd_dma_buffer *buf = &substream->dma_buffer;
	size_t size;

	if (stream == SNDRV_PCM_STREAM_PLAYBACK)
		size = caph_pcm_hardware_playback.buffer_bytes_max;
	else
		size = caph_pcm_hardware_capture.buffer_bytes_max;

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

/*****************************************************************************
*
*  Function Name: void caph_pcm_free(struct snd_pcm *pcm)
*  Parameters:
*  @pcm: snd_pcm handle
*
*  Description: Free DMA buffer area, addr, bytes
*
*****************************************************************************/
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

/*****************************************************************************
*
*  Function Name: void caph_pcm_new(struct snd_soc_pcm_runtime *rtd)
*  Parameters:
*  @rtd: soc rundatime structure
*
*  Description: pcm_new function for snd_soc_platform_driver
*
*****************************************************************************/
int caph_pcm_new(struct snd_soc_pcm_runtime *rtd)
{
	int ret = 0;
	struct snd_card *card = rtd->card->snd_card;
	struct snd_pcm *pcm = rtd->pcm;

	if (!card->dev->dma_mask)
		card->dev->dma_mask = &caph_pcm_dmamask;

	if (!card->dev->coherent_dma_mask)
		card->dev->coherent_dma_mask = DMA_BIT_MASK(32);

	if (pcm->streams[SNDRV_PCM_STREAM_PLAYBACK].substream) {
		ret = caph_pcm_preallocate_dma_buffer(pcm,
			SNDRV_PCM_STREAM_PLAYBACK);
		if (ret)
			goto err;
	}

	if (pcm->streams[SNDRV_PCM_STREAM_CAPTURE].substream) {
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
	int ret;
	ret =  snd_soc_register_platform(&pdev->dev, &caph_soc_platform);
	if (ret)
		pr_info("caph-pcm: snd_soc_register_plateform() fails\n");
	return ret;

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

MODULE_DESCRIPTION("Broadcom SoC 21664 PCM driver");
MODULE_LICENSE("GPL");
