#include <linux/interrupt.h>

#include "bcm2835.h"

/* hardware definition */
static struct snd_pcm_hardware snd_bcm2835_playback_hw = {
	.info = (SNDRV_PCM_INFO_MMAP |
			SNDRV_PCM_INFO_INTERLEAVED |
			SNDRV_PCM_INFO_BLOCK_TRANSFER |
			SNDRV_PCM_INFO_MMAP_VALID),
	.formats =          SNDRV_PCM_FMTBIT_S16_LE,
	.rates =            SNDRV_PCM_RATE_44100,
	.rate_min =         44100,
	.rate_max =         44100,
	.channels_min =     1,
	.channels_max =     2,
	.buffer_bytes_max = 14336, // Can increase to 128k
	.period_bytes_min = 2048,
	.period_bytes_max = 14336, // Can increase to 128k
	.periods_min =      1,
	.periods_max =      7,
};

#if 0
/* hardware definition */
static struct snd_pcm_hardware snd_bcm2835_capture_hw = {
    .info = (SNDRV_PCM_INFO_MMAP |
            SNDRV_PCM_INFO_INTERLEAVED |
            SNDRV_PCM_INFO_BLOCK_TRANSFER |
            SNDRV_PCM_INFO_MMAP_VALID),
    .formats =          SNDRV_PCM_FMTBIT_S16_LE,
    .rates =            SNDRV_PCM_RATE_8000_48000,
    .rate_min =         8000,
    .rate_max =         48000,
    .channels_min =     2,
    .channels_max =     2,
    .buffer_bytes_max = 32768,
    .period_bytes_min = 4096,
    .period_bytes_max = 32768,
    .periods_min =      1,
    .periods_max =      1024,
};
#endif


void snd_bcm2835_pcm_interrupt_task(unsigned long data)
{
	struct snd_pcm_substream *substream = (struct snd_pcm_substream *)data;
	struct snd_pcm_runtime *runtime = substream->runtime;
	bcm2835_alsa_stream_t *alsa_stream = runtime->private_data;

	printk(KERN_INFO"%s() .. IN\n", __func__);

	if (alsa_stream->open)
		bcm2835_audio_retrieve_buffers(alsa_stream);

	printk("pcm int task.. updating pos cur: %d + %d max: %d\n", alsa_stream->pos, alsa_stream->period_size, alsa_stream->buffer_size);
	alsa_stream->pos += alsa_stream->period_size;
	alsa_stream->pos %= alsa_stream->buffer_size;

	snd_pcm_period_elapsed(substream);

	printk(KERN_INFO"%s() .. OUT\n", __func__);

	return;
}

static void snd_bcm2835_playback_free(struct snd_pcm_runtime *runtime)
{
	kfree(runtime->private_data);
}

static irqreturn_t bcm2835_playback_fifo_irq(int irq, void *dev_id)
{
	bcm2835_alsa_stream_t *alsa_stream = (bcm2835_alsa_stream_t *)dev_id;

	printk(KERN_INFO"%s() .. IN\n", __func__);

//	tasklet_schedule(&alsa_stream->pcm_int_task);
	if (alsa_stream->open)
		bcm2835_audio_retrieve_buffers(alsa_stream);

	printk("pcm int task.. updating pos cur: %d + %d max: %d\n", alsa_stream->pos, alsa_stream->period_size, alsa_stream->buffer_size);
	alsa_stream->pos += alsa_stream->period_size;
	alsa_stream->pos %= alsa_stream->buffer_size;

	snd_pcm_period_elapsed(alsa_stream->substream);

	printk(KERN_INFO"%s() .. OUT\n", __func__);

	return IRQ_HANDLED;
}

/* open callback */
static int snd_bcm2835_playback_open(struct snd_pcm_substream *substream)
{
	bcm2835_chip_t *chip = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	bcm2835_alsa_stream_t *alsa_stream;
	int err;

	printk(KERN_INFO"%s() .. IN\n", __func__);
	alsa_stream = kzalloc(sizeof(bcm2835_alsa_stream_t), GFP_KERNEL);
	if (alsa_stream == NULL) {
		return -ENOMEM;
	}

	alsa_stream->chip = chip;
	alsa_stream->substream = substream;

	/* Make sure this is locked */
	sema_init(&alsa_stream->control_sem, 0);
	spin_lock_init(&alsa_stream->lock);
	alsa_stream->post_control_sem = 0;
	alsa_stream->dest = AUDIO_DEST_HDMI;
	INIT_LIST_HEAD(&alsa_stream->buffer_list);
	alsa_stream->fifo_irq_handler = bcm2835_playback_fifo_irq;
	alsa_stream->buffer_count = 0;

	runtime->private_data = alsa_stream;
	runtime->private_free = snd_bcm2835_playback_free;
	runtime->hw = snd_bcm2835_playback_hw;

	tasklet_init(&alsa_stream->pcm_int_task, snd_bcm2835_pcm_interrupt_task, (unsigned long)substream);
	tasklet_enable(&alsa_stream->pcm_int_task);

	err = bcm2835_audio_open(alsa_stream);
	if (err != 0) {
		kfree(alsa_stream);
	}
	
	alsa_stream->open = 1;
	printk(KERN_INFO"%s() .. OUT\n", __func__);
	return err;
}

/* close callback */
static int snd_bcm2835_playback_close(struct snd_pcm_substream *substream)
{
	/* the hardware-specific codes will be here */

	struct snd_pcm_runtime *runtime = substream->runtime;
	bcm2835_alsa_stream_t *alsa_stream = runtime->private_data;

	printk(KERN_INFO"%s() .. IN\n", __func__);
	printk("Alsa close\n");
	BUG_ON(alsa_stream->running);

        tasklet_disable_nosync(&alsa_stream->pcm_int_task);
	tasklet_kill(&alsa_stream->pcm_int_task);

	if (alsa_stream->open) {
		alsa_stream->open = 0;
		bcm2835_audio_close(alsa_stream);
	}

	kfree(alsa_stream);

	printk(KERN_INFO"%s() .. OUT\n", __func__);
	return 0;
}

#if 0
/* open callback */
static int snd_bcm2835_capture_open(struct snd_pcm_substream *substream)
{
    bcm2835_chip_t *chip = snd_pcm_substream_chip(substream);
    struct snd_pcm_runtime *runtime = substream->runtime;

    runtime->hw = snd_bcm2835_capture_hw;
    /* more hardware-initialization will be done here */
    ....
        return 0;
}

/* close callback */
static int snd_bcm2835_capture_close(struct snd_pcm_substream *substream)
{
    bcm2835_chip_t *chip = snd_pcm_substream_chip(substream);
    /* the hardware-specific codes will be here */
    ....
        return 0;

}
#endif

/* hw_params callback */
static int snd_bcm2835_pcm_hw_params(struct snd_pcm_substream *substream,
        struct snd_pcm_hw_params *params)
{
	int err;
	struct snd_pcm_runtime *runtime = substream->runtime;
	bcm2835_alsa_stream_t *alsa_stream = (bcm2835_alsa_stream_t *)runtime->private_data;

	printk(KERN_INFO"%s() .. IN\n", __func__);
	err = snd_pcm_lib_malloc_pages(substream,
			params_buffer_bytes(params));
	if (err < 0) {
		printk(KERN_ERR"%s: pcm_lib_malloc failed to allocated pages for buffers\n", __func__);
		return err;
	}

	err = bcm2835_audio_set_params(alsa_stream, params_channels(params),
				params_rate(params), snd_pcm_format_width(params_format(params)));
	if (err < 0) {
		printk(KERN_ERR"%s: error setting hw params\n", __func__);
	}

	printk(KERN_INFO"%s() .. OUT\n", __func__);
	return err;
}

/* hw_free callback */
static int snd_bcm2835_pcm_hw_free(struct snd_pcm_substream *substream)
{
	return snd_pcm_lib_free_pages(substream);
}

/* prepare callback */
static int snd_bcm2835_pcm_prepare(struct snd_pcm_substream *substream)
{
	//    bcm2835_chip_t *chip = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	bcm2835_alsa_stream_t *alsa_stream = runtime->private_data;

	printk(KERN_INFO"%s() .. IN\n", __func__);
	alsa_stream->pos = 0;
	alsa_stream->buffer_size = snd_pcm_lib_buffer_bytes(substream);
	alsa_stream->period_size = snd_pcm_lib_period_bytes(substream);

	printk(KERN_INFO"%s() .. OUT\n", __func__);
	return 0;
}

/* trigger callback */
static int snd_bcm2835_pcm_trigger(struct snd_pcm_substream *substream,
        int cmd)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	bcm2835_alsa_stream_t *alsa_stream = runtime->private_data;
	int err = 0;

	printk(KERN_INFO"%s() .. IN\n", __func__);
	switch (cmd) {
		case SNDRV_PCM_TRIGGER_START:
			printk("bcm2835_AUDIO_TRIGGER_START\n");
			if (!alsa_stream->running) {
				alsa_stream->running = 1;
				err = bcm2835_audio_start(alsa_stream);
				if (err != 0)
					alsa_stream->running = 0;
			}
			break;
		case SNDRV_PCM_TRIGGER_STOP:
			printk("bcm2835_AUDIO_TRIGGER_STOP\n");
			if (alsa_stream->running) {
				err = bcm2835_audio_stop(alsa_stream);
				if (err == 0)
					alsa_stream->running = 0;
			}
			break;
		default:
			err = -EINVAL;
	}

	printk(KERN_INFO"%s() .. OUT\n", __func__);
	return err;
}

/* pointer callback */
static snd_pcm_uframes_t
snd_bcm2835_pcm_pointer(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	bcm2835_alsa_stream_t *alsa_stream = runtime->private_data;

	printk(KERN_INFO"%s() .. IN\n", __func__);
	printk("pcm_pointer %u\n",  alsa_stream->pos);

	printk(KERN_INFO"%s() .. OUT\n", __func__);
	return bytes_to_frames(runtime, alsa_stream->pos);
}

static int snd_bcm2835_pcm_copy(struct snd_pcm_substream *substream, int channel,
        snd_pcm_uframes_t pos, void *src, snd_pcm_uframes_t count)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	bcm2835_alsa_stream_t *alsa_stream = runtime->private_data;

	printk(KERN_INFO"%s() .. IN\n", __func__);
	printk("copy... %d\n", frames_to_bytes(runtime, count));

	return bcm2835_audio_write(alsa_stream, frames_to_bytes(runtime, count), src);
}

#if 0
static int snd_bcm2835_pcm_silence(struct snd_pcm_substream *substream, int channel,
        snd_pcm_uframes_t post, snd_pcm_uframes_t count)
{
    //    memset(buf,0);
    //    bcm2835_audio_write(frames_to_bytes(runetime, count), buf);
}
#endif

/* operators */
static struct snd_pcm_ops snd_bcm2835_playback_ops = {
	.open =        snd_bcm2835_playback_open,
	.close =       snd_bcm2835_playback_close,
	.ioctl =       snd_pcm_lib_ioctl,
	.hw_params =   snd_bcm2835_pcm_hw_params,
	.hw_free =     snd_bcm2835_pcm_hw_free,
	.prepare =     snd_bcm2835_pcm_prepare,
	.trigger =     snd_bcm2835_pcm_trigger,
	.pointer =     snd_bcm2835_pcm_pointer,
	.copy    =     snd_bcm2835_pcm_copy,
	/* .silence =     snd_bcm2835_pcm_silence, */
};

#if 0
/* operators */
static struct snd_pcm_ops snd_bcm2835_capture_ops = {
    .open =        snd_bcm2835_capture_open,
    .close =       snd_bcm2835_capture_close,
    .ioctl =       snd_pcm_lib_ioctl,
    .hw_params =   snd_bcm2835_pcm_hw_params,
_    .hw_free =     snd_bcm2835_pcm_hw_free,
    .prepare =     snd_bcm2835_pcm_prepare,
    .trigger =     snd_bcm2835_pcm_trigger,
    .pointer =     snd_bcm2835_pcm_pointer,
};
#endif
/*
 *  definitions of capture are omitted here...
 */

/* create a pcm device */
int __devinit snd_bcm2835_new_pcm(bcm2835_chip_t *chip)
{
	struct snd_pcm *pcm;
	int err;

	err = snd_pcm_new(chip->card, "bcm2835 ALSA", 0, 1, 0, &pcm);
	if (err < 0) 
	return err;
	pcm->private_data = chip;
	strcpy(pcm->name, "bcm2835 ALSA");
	chip->pcm = pcm;
	/* set operators */
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK,
	    &snd_bcm2835_playback_ops);
#if 0
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_CAPTURE,
	    &snd_bcm2835_capture_ops);
#endif
	/* pre-allocation of buffers */
	/* NOTE: this may fail */
	snd_pcm_lib_preallocate_pages_for_all(pcm, SNDRV_DMA_TYPE_CONTINUOUS,
				snd_dma_continuous_data(GFP_KERNEL), 64*1024, 64*1024);
	printk(KERN_INFO"%s() .. OUT\n", __func__);
	return 0;
}


