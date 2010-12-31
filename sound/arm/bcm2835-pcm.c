#include <linux/interrupt.h>
#include <linux/slab.h>

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
	.buffer_bytes_max = 57334, /* Can increase to 128k */
	.period_bytes_min = 8192,
	.period_bytes_max = 8192, /* Can increase to 128k */
	.periods_min =      1,
	.periods_max =      8,
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

static void snd_bcm2835_playback_free(struct snd_pcm_runtime *runtime)
{
	audio_info("Freeing up alsa stream here ..\n");
	kfree(runtime->private_data);
}

static irqreturn_t bcm2835_playback_fifo_irq(int irq, void *dev_id)
{
	bcm2835_alsa_stream_t *alsa_stream = (bcm2835_alsa_stream_t *)dev_id;
	uint32_t consumed = 0;

	audio_debug(" .. IN\n");

	if (alsa_stream->open)
		consumed = bcm2835_audio_retrieve_buffers(alsa_stream);

	/* We get called only if playback was triggered, So, the number of buffers we retrieve in
	 * each iteration are the buffers that have been played out already
	 */
	audio_info("updating pos cur: %d + %d max: %d\n", alsa_stream->pos,
				(consumed * AUDIO_IPC_BLOCK_BUFFER_SIZE), alsa_stream->buffer_size);
	alsa_stream->pos += (consumed * AUDIO_IPC_BLOCK_BUFFER_SIZE);
	alsa_stream->pos %= alsa_stream->buffer_size;

	snd_pcm_period_elapsed(alsa_stream->substream);

	audio_debug(" .. OUT\n");

	return IRQ_HANDLED;
}

/* open callback */
static int snd_bcm2835_playback_open(struct snd_pcm_substream *substream)
{
	bcm2835_chip_t *chip = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	bcm2835_alsa_stream_t *alsa_stream;
	int err;

	audio_debug(" .. IN\n");
	alsa_stream = kzalloc(sizeof(bcm2835_alsa_stream_t), GFP_KERNEL);
	if (alsa_stream == NULL) {
		return -ENOMEM;
	}

	/* Initialise alsa_stream */
	alsa_stream->chip = chip;
	alsa_stream->substream = substream;

	sema_init(&alsa_stream->buffers_update_sem, 0);
	sema_init(&alsa_stream->control_sem, 0);
	spin_lock_init(&alsa_stream->lock);

	/* List of buffers we can write to .. */
	INIT_LIST_HEAD(&alsa_stream->buffer_list);

	/* Enabled in start trigger, called on each "fifo irq" after that */
	alsa_stream->enable_fifo_irq = 0;
	alsa_stream->fifo_irq_handler = bcm2835_playback_fifo_irq;
	alsa_stream->buffer_count = 0;

	runtime->private_data = alsa_stream;
	runtime->private_free = snd_bcm2835_playback_free;
	runtime->hw = snd_bcm2835_playback_hw;

	err = bcm2835_audio_open(alsa_stream);
	if (err != 0) {
		kfree(alsa_stream);
		return err;
	}

	alsa_stream->open = 1;
	audio_debug(" .. OUT\n");

	return err;
}

/* close callback */
static int snd_bcm2835_playback_close(struct snd_pcm_substream *substream)
{
	/* the hardware-specific codes will be here */

	struct snd_pcm_runtime *runtime = substream->runtime;
	bcm2835_alsa_stream_t *alsa_stream = runtime->private_data;

	audio_debug(" .. IN\n");
	audio_info("Alsa close\n");

	/*
	 * Call stop if it's still running. This happens when app
	 * is force killed and we don't get a stop trigger.
	 */
	if (alsa_stream->running) {
		int err;
		err = bcm2835_audio_stop(alsa_stream);
		alsa_stream->running = 0;
		if (err != 0)
			audio_error(" Failed to STOP alsa device\n");
	}

	alsa_stream->period_size = 0;
	alsa_stream->buffer_size = 0;

	if (alsa_stream->open) {
		alsa_stream->open = 0;
		bcm2835_audio_close(alsa_stream);
	}

	/*
	 * Do not free up alsa_stream here, it will be freed up by
	 * runtime->private_free callback we registered in *_open above
	 */

	audio_debug(" .. OUT\n");

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

	audio_debug(" .. IN\n");
	err = snd_pcm_lib_malloc_pages(substream,
			params_buffer_bytes(params));
	if (err < 0) {
		audio_error(" pcm_lib_malloc failed to allocated pages for buffers\n");
		return err;
	}

	err = bcm2835_audio_set_params(alsa_stream, params_channels(params),
				params_rate(params), snd_pcm_format_width(params_format(params)));
	if (err < 0) {
		audio_error(" error setting hw params\n");
	}

	audio_debug(" .. OUT\n");

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
	struct snd_pcm_runtime *runtime = substream->runtime;
	bcm2835_alsa_stream_t *alsa_stream = runtime->private_data;

	audio_debug(" .. IN\n");
	alsa_stream->pos = 0;
	alsa_stream->buffer_size = snd_pcm_lib_buffer_bytes(substream);
	alsa_stream->period_size = snd_pcm_lib_period_bytes(substream);

	audio_debug(" .. OUT\n");
	return 0;
}

/* trigger callback */
static int snd_bcm2835_pcm_trigger(struct snd_pcm_substream *substream,
        int cmd)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	bcm2835_alsa_stream_t *alsa_stream = runtime->private_data;
	int err = 0;

	audio_debug(" .. IN\n");
	switch (cmd) {
		case SNDRV_PCM_TRIGGER_START:
			audio_info("bcm2835_AUDIO_TRIGGER_START\n");
			if (!alsa_stream->running) {
				err = bcm2835_audio_start(alsa_stream);
				if (err == 0)
					alsa_stream->running = 1;
			}
			break;
		case SNDRV_PCM_TRIGGER_STOP:
			audio_info("bcm2835_AUDIO_TRIGGER_STOP\n");
			if (alsa_stream->running) {
				err = bcm2835_audio_stop(alsa_stream);
				if (err != 0)
					audio_error(" Failed to STOP alsa device\n");
				alsa_stream->running = 0;
			}
			break;
		default:
			err = -EINVAL;
	}

	audio_debug(" .. OUT\n");
	return err;
}

/* pointer callback */
static snd_pcm_uframes_t
snd_bcm2835_pcm_pointer(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	bcm2835_alsa_stream_t *alsa_stream = runtime->private_data;

	audio_debug(" .. IN\n");
	audio_info("pcm_pointer %u\n",  alsa_stream->pos);

	audio_debug(" .. OUT\n");
	return bytes_to_frames(runtime, alsa_stream->pos);
}

static int snd_bcm2835_pcm_copy(struct snd_pcm_substream *substream, int channel,
        snd_pcm_uframes_t pos, void *src, snd_pcm_uframes_t count)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	bcm2835_alsa_stream_t *alsa_stream = runtime->private_data;

	audio_debug(" .. IN\n");
	audio_info("copy... %d\n", frames_to_bytes(runtime, count));

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
	chip->dest = AUDIO_DEST_LOCAL;
	chip->volume = 0;
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

	audio_debug(" .. OUT\n");

	return 0;
}


