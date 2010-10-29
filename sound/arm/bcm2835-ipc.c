#include <linux/device.h>
#include <sound/core.h>
#include <sound/initval.h>
#include <sound/pcm.h>
#include <linux/io.h>
#include <linux/interrupt.h>

#include "ipc_fifo.h"
#include "bcm2835.h"



static inline void dump_fifo(bcm2835_alsa_stream_t *alsa_stream)
{

	printk(KERN_INFO"Out fifo->\n");
	printk(KERN_INFO" write - 0x%08x\n read - 0x%08x\n base - 0%08x\n size - %d\n entry_size - %d\n",
			*(uint32_t *)alsa_stream->out_fifo.write, *(uint32_t *)alsa_stream->out_fifo.read, alsa_stream->out_fifo.base, alsa_stream->out_fifo.size,
			alsa_stream->out_fifo.entry_size);

	printk(KERN_INFO"In fifo->\n");
	printk(KERN_INFO" write - 0x%08x\n read - 0x%08x\n base - 0%08x\n size - %d\n entry_size - %d\n",
			*(uint32_t *)alsa_stream->in_fifo.write, *(uint32_t *)alsa_stream->in_fifo.read, alsa_stream->in_fifo.base, alsa_stream->in_fifo.size,
			alsa_stream->in_fifo.entry_size);


}

void bcm2835_audio_fifo_get_lock(bcm2835_alsa_stream_t *alsa_stream)
{
	bcm2835_chip_t *chip = alsa_stream->chip;

	writel(1, chip->reg_base + AUDIO_FLAG1_OFFSET);
	writel(0, chip->reg_base + AUDIO_TURN_OFFSET);

	mb();

	/* busy wait until VC has given it up ... */
	while ((readl(chip->reg_base + AUDIO_FLAG0_OFFSET) == 1) && (readl(chip->reg_base + AUDIO_TURN_OFFSET) == 0)); 

}

void bcm2835_audio_fifo_put_lock(bcm2835_alsa_stream_t *alsa_stream)
{
	bcm2835_chip_t *chip = alsa_stream->chip;

	writel(0, chip->reg_base + AUDIO_FLAG1_OFFSET);
	mb();
}

static irqreturn_t bcm2835_audio_irq(int irq, void *dev_id)
{
	bcm2835_alsa_stream_t *alsa_stream = (bcm2835_alsa_stream_t *)dev_id;
	bcm2835_chip_t *chip = alsa_stream->chip;
	uint32_t intstat;
	int ret = IRQ_HANDLED;

	intstat = readl(chip->reg_base + AUDIO_INTSTAT_OFFSET);
	/* clear intstat for next interrupt */
	writel(0, chip->reg_base + AUDIO_INTSTAT_OFFSET);

	/* is it a control ack ? */
	if (intstat & INTSTAT_CONTROL_MASK) {
		alsa_stream->status = readl(chip->reg_base + AUDIO_STATUS_OFFSET);
		printk(KERN_INFO"%s: Got control irq with status (0x%08x)\n",__func__, alsa_stream->status);
		if (alsa_stream->post_control_sem) {
			alsa_stream->post_control_sem = 0;
			up(&alsa_stream->control_sem);
		}
	}

	/* is it a fifo irq ? */
	if (intstat & INTSTAT_FIFO_MASK) {
		ret = alsa_stream->fifo_irq_handler(irq, dev_id);
	}

	printk(KERN_INFO"%s() .. OUT\n", __func__);
	return IRQ_HANDLED;
}

int bcm2835_audio_open(bcm2835_alsa_stream_t *alsa_stream)
{
	bcm2835_chip_t *chip = alsa_stream->chip;
	uint32_t control;
	int err;
	unsigned long flags;

	printk(KERN_INFO"%s() .. IN\n", __func__);
	printk(KERN_INFO"%s:dest = %d, reg_base = 0x%08x\n", __func__, alsa_stream->dest, (uint32_t)chip->reg_base);
	
	err = request_irq(chip->irq, bcm2835_audio_irq, IRQF_DISABLED, "bcm2835 audio irq", (void *)alsa_stream);
	if (err < 0) {
		printk(KERN_ERR"Failed to request IRQ for bcm2835 playback\n");
		goto err_request_irq;
	}


	/* setup the destination and enable device*/
	writel(alsa_stream->dest, chip->reg_base + AUDIO_DESTINATION_OFFSET);
	control = readl(chip->reg_base + AUDIO_CONTROL_OFFSET);
	if (control & CTRL_EN_MASK) {
		printk(KERN_NOTICE"%s:Alsa device was already enabled !\n", __func__);
		printk(KERN_ALERT"%s: Going ahead with rest of the initialisation ..\n", __func__);
	} else {

		spin_lock_irqsave(&alsa_stream->lock, flags);
		control &= ~(CTRL_MUTE_MASK);
		control |= (1 << CTRL_EN_SHIFT);
		/* default volume .. ?? */
		writel(100, chip->reg_base + AUDIO_VOLUME_OFFSET);
		writel(control, chip->reg_base + AUDIO_CONTROL_OFFSET);

		alsa_stream->post_control_sem = 1;

		/* ring the doorbell */
		ipc_notify_vc_event(chip->irq);
		spin_unlock_irqrestore(&alsa_stream->lock, flags);

		printk(KERN_INFO"%s: Waiting for alsa device to be enabled\n", __func__);
		down(&alsa_stream->control_sem);

		/* verify */
		if (!(alsa_stream->status & CTRL_EN_MASK)) {
			printk(KERN_ERR"%s: Got config irq, but device still not enabled, getting out now\n", __func__);
			err = -ENODEV;
			goto err_enable_device;
		}
	}
	

	ipc_fifo_setup_no_reset(&alsa_stream->out_fifo, (chip->reg_base + AUDIO_OUT_WRITE_PTR_OFFSET),
						(chip->reg_base + AUDIO_OUT_READ_PTR_OFFSET),
						ipc_bus_to_virt(readl(chip->reg_base + AUDIO_OUT_FIFO_START_OFFSET)),
						readl(chip->reg_base + AUDIO_OUT_FIFO_SIZE_OFFSET),
						readl(chip->reg_base + AUDIO_OUT_FIFO_ENTRY_OFFSET));

	ipc_fifo_setup_no_reset(&alsa_stream->in_fifo, (chip->reg_base + AUDIO_IN_WRITE_PTR_OFFSET),
						(chip->reg_base + AUDIO_IN_READ_PTR_OFFSET),
						ipc_bus_to_virt(readl(chip->reg_base + AUDIO_IN_FIFO_START_OFFSET)),
						readl(chip->reg_base + AUDIO_IN_FIFO_SIZE_OFFSET),
						readl(chip->reg_base + AUDIO_IN_FIFO_ENTRY_OFFSET));

	dump_fifo(alsa_stream);

	/* Get available buffers .. */
	bcm2835_audio_retrieve_buffers(alsa_stream);

	printk(KERN_INFO"%s: success !\n", __func__);
	printk(KERN_INFO"%s() .. OUT\n", __func__);
	return 0;

err_enable_device:
	free_irq(chip->irq, (void *)alsa_stream);
err_request_irq:
	printk(KERN_INFO"%s() .. OUT2\n", __func__);
	return err;
}

void bcm2835_audio_close(bcm2835_alsa_stream_t *alsa_stream)
{
	bcm2835_chip_t *chip = alsa_stream->chip;
	uint32_t control;
	unsigned long flags;

	printk(KERN_INFO"%s() .. IN\n", __func__);

	bcm2835_audio_flush_buffers(alsa_stream);

	control = readl(chip->reg_base + AUDIO_CONTROL_OFFSET);

	spin_lock_irqsave(&alsa_stream->lock, flags);
	control &= ~(CTRL_EN_MASK);
	writel(control, chip->reg_base + AUDIO_CONTROL_OFFSET);

	alsa_stream->post_control_sem = 1;

	/* ring the doorbell */
	ipc_notify_vc_event(chip->irq);

	spin_unlock_irqrestore(&alsa_stream->lock, flags);

	printk(KERN_INFO"%s: Waiting for alsa device to be Disabled\n", __func__);
	down(&alsa_stream->control_sem);

	/* verify */
	if (alsa_stream->status & CTRL_EN_MASK)
		printk(KERN_ERR"%s: Got config irq, but ALSA device still not disabled\n", __func__);
	else
		printk(KERN_ERR"%s: ALSA device disabled successfully\n", __func__);

	printk(KERN_INFO"%s() .. OUT\n", __func__);
	return;
}

int bcm2835_audio_set_params(bcm2835_alsa_stream_t *alsa_stream, uint32_t channels, uint32_t samplerate, uint32_t bps)
{
	bcm2835_chip_t *chip = alsa_stream->chip;

	printk(KERN_INFO"%s() .. IN\n", __func__);

	if (channels < 1 && channels > 2) {
		printk(KERN_ERR"%s: channels (%d) not supported\n", __func__, channels);
		return -EINVAL;
	}

	if (samplerate != 44100) {
		printk(KERN_ERR"%s: samplerate (%d) not supported\n", __func__, samplerate);
		return -EINVAL;
	}

	if (bps !=8 && bps != 16) {
		printk(KERN_ERR"%s: Bits per sample (%d) not supported\n", __func__, bps);
		return -EINVAL;
	}

	printk(KERN_INFO"%s: Setting ALSA channels(%d), samplerate(%d), bits-per-sample(%d)\n",
								__func__, channels, samplerate, bps);

	writel(channels, chip->reg_base + AUDIO_CHANNELS_OFFSET);
	writel(samplerate, chip->reg_base + AUDIO_SAMPLE_RATE_OFFSET);
	writel(bps, chip->reg_base + AUDIO_BIT_RATE_OFFSET);

	printk(KERN_INFO"%s() .. OUT\n", __func__);
	return 0;
}

int bcm2835_audio_start(bcm2835_alsa_stream_t *alsa_stream)
{
	bcm2835_chip_t *chip = alsa_stream->chip;
	uint32_t control;
	unsigned long flags;

	printk(KERN_INFO"%s() .. IN\n", __func__);

	control = readl(chip->reg_base + AUDIO_CONTROL_OFFSET);

	BUG_ON(!(control & CTRL_EN_MASK));

	spin_lock_irqsave(&alsa_stream->lock, flags);
	control |= (1 << CTRL_PLAY_SHIFT);
	writel(control, chip->reg_base + AUDIO_CONTROL_OFFSET);

	alsa_stream->post_control_sem = 0;
	/* ring the doorbell */
	ipc_notify_vc_event(chip->irq);

	spin_unlock_irqrestore(&alsa_stream->lock, flags);


	/* Trigger callbacks are atomic, so, dont wait for ack to come back,
	 * just move on
	 */
#if 0 
	printk(KERN_INFO"%s: Waiting for alsa device to start playing\n", __func__);
	down(&alsa_stream->control_sem);

	/* verify */
	if (!(alsa_stream->status & CTRL_PLAY_MASK)) {
		printk(KERN_ERR"%s: Got config irq, but ALSA device still not playing\n", __func__);
		err = 0;
	} else {
		printk(KERN_ERR"%s: ALSA device playback started successfully\n", __func__);
		err = -EAGAIN;
	}
#endif

	printk(KERN_INFO"%s() .. OUT\n", __func__);

	return 0;
}

int bcm2835_audio_stop(bcm2835_alsa_stream_t *alsa_stream)
{
	bcm2835_chip_t *chip = alsa_stream->chip;
	uint32_t control;
	unsigned long flags;

	printk(KERN_INFO"%s() .. IN\n", __func__);

	control = readl(chip->reg_base + AUDIO_CONTROL_OFFSET);

	BUG_ON(!(control & CTRL_EN_MASK));

	spin_lock_irqsave(&alsa_stream->lock, flags);
	control &= ~(CTRL_PLAY_MASK);
	writel(control, chip->reg_base + AUDIO_CONTROL_OFFSET);

	alsa_stream->post_control_sem = 0;

	/* ring the doorbell */
	ipc_notify_vc_event(chip->irq);

	spin_unlock_irqrestore(&alsa_stream->lock, flags);

#if 0
	printk(KERN_INFO"%s: Waiting for alsa device to stop playing\n", __func__);
	down(&alsa_stream->control_sem);

	/* verify */
	if (alsa_stream->status & CTRL_PLAY_MASK) {
		printk(KERN_ERR"%s: Got config irq, but ALSA device did not stop playback\n", __func__);
		err = 0;
	} else {
		printk(KERN_ERR"%s: ALSA device playback stopped successfully\n", __func__);
		err = -EAGAIN;
	}
#endif

	printk(KERN_INFO"%s() .. OUT\n", __func__);

	return 0;
}


int bcm2835_audio_write(bcm2835_alsa_stream_t *alsa_stream, uint32_t count, void *src)
{
	bcm2835_chip_t *chip = alsa_stream->chip;
	struct list_head *p, *next;
	bcm2835_audio_buffer_t *buffer;
	AUDIO_FIFO_ENTRY_T entry;

	/* Check the number of buffers required for the job first and see if
	 * that fits into current available buffers to us
	 */

	printk(KERN_INFO"%s() .. IN\n", __func__);

	/* Short check: assumin 2K buffers */
	if (alsa_stream->buffer_count * SZ_2K < count)
		return -ENOSPC;

	bcm2835_audio_fifo_get_lock(alsa_stream);

	list_for_each_safe(p, next, &alsa_stream->buffer_list) {
		size_t copy_size = 0; 
		buffer = list_entry(p, bcm2835_audio_buffer_t, link);

		if (count == 0)
			break;
		
		buffer->start = ioremap(__bus_to_phys(buffer->bus_addr), buffer->size);
		if (buffer->start == NULL) {
			printk(KERN_ERR"%s: Failed to Ioremap buffer from phys(0x%08x), size(%d)\n",
					__func__, (uint32_t)__bus_to_phys(buffer->bus_addr), buffer->size);
			kfree(buffer);
			break;
		}

		copy_size = count > buffer->data_left ? buffer->data_left : count;
		printk(KERN_INFO"%s: Copying into buffer @0x%08x, for size (%d)\n", __func__, buffer->start, copy_size);

		memcpy((buffer->start + buffer->size - buffer->data_left), src, copy_size);
		buffer->data_left -= copy_size;
		count -= copy_size;

		if (buffer->data_left) {
			/* This buffer is only partially filled, so not doing
			 * anything */
			BUG_ON(count);
			break;
		}

		entry.buffer_id = buffer->buffer_id;
		entry.buffer_size = buffer->size;
		entry.buffer_ptr = buffer->bus_addr;
		iounmap(buffer->start);
		list_del_init(p);
		alsa_stream->buffer_count--;
		kfree(buffer);

		/* add into out fifo, we know its not full as long as we are
		 * runnnig this loop
		 */
		ipc_fifo_write(&alsa_stream->out_fifo, &entry);
	}
	
	/* ring the doorbell */
	ipc_notify_vc_event(chip->irq);

	bcm2835_audio_fifo_put_lock(alsa_stream);
	
	printk(KERN_INFO"%s() .. OUT\n", __func__);
	return 0;
}

void bcm2835_audio_retrieve_buffers(bcm2835_alsa_stream_t *alsa_stream)
{
	bcm2835_audio_buffer_t *buffer;
	AUDIO_FIFO_ENTRY_T entry;

	printk(KERN_INFO"%s() .. IN\n", __func__);

	printk(KERN_INFO"%s: Busy waiting for audio fifo lock ..\n", __func__);
	/* get the lock first */
	bcm2835_audio_fifo_get_lock(alsa_stream);

	while (!ipc_fifo_empty(&alsa_stream->in_fifo)) {
		memset(&entry, 0, sizeof(entry));
		ipc_fifo_read(&alsa_stream->in_fifo, &entry);
		if (entry.buffer_ptr != 0) {
			buffer = kzalloc(sizeof(*buffer), GFP_KERNEL | GFP_ATOMIC);
			buffer->buffer_id = entry.buffer_id;
			buffer->bus_addr = entry.buffer_ptr;
			buffer->size = entry.buffer_size;
			buffer->data_left = entry.buffer_size;
			printk(KERN_INFO"%s: Adding buffer @ bus_add(0x%08x), size (%d) to list\n", __func__, (uint32_t)buffer->bus_addr, buffer->size);
			list_add_tail(&buffer->link, &alsa_stream->buffer_list);
			alsa_stream->buffer_count++;
		} else {
			printk(KERN_ERR"%s: Input fifo had a NULL entry\n", __func__);
			printk(KERN_ERR"%s: Breaking out now \n", __func__);
			break;
		}
	}

	printk(KERN_INFO"Fifo Scan complete !!\n");
	dump_fifo(alsa_stream);
	printk(KERN_INFO"%s: Buffers retrieved (%d)\n", __func__, alsa_stream->buffer_count);
	/* release the lock here */
	bcm2835_audio_fifo_put_lock(alsa_stream);
	printk(KERN_INFO"%s() .. OUT\n", __func__);
}

void bcm2835_audio_flush_buffers(bcm2835_alsa_stream_t *alsa_stream)
{
	bcm2835_chip_t *chip = alsa_stream->chip;
	struct list_head *p, *next;
	bcm2835_audio_buffer_t *buffer;
	AUDIO_FIFO_ENTRY_T entry;

	/* Check the number of buffers required for the job first and see if
	 * that fits into current available buffers to us
	 */

	printk(KERN_INFO"%s() .. IN\n", __func__);

	bcm2835_audio_fifo_get_lock(alsa_stream);

	list_for_each_safe(p, next, &alsa_stream->buffer_list) {
		buffer = list_entry(p, bcm2835_audio_buffer_t, link);

		entry.buffer_id = buffer->buffer_id;
		entry.buffer_size = buffer->size;
		entry.buffer_ptr = buffer->bus_addr;
		iounmap(buffer->start);
		list_del_init(p);
		alsa_stream->buffer_count--;

		/* add into out fifo, we know its not full as long as we are
		 * runnnig this loop
		 */
		ipc_fifo_write(&alsa_stream->out_fifo, &entry);
	}

	bcm2835_audio_fifo_put_lock(alsa_stream);
	
	/* ring the doorbell */
	ipc_notify_vc_event(chip->irq);
	
	printk(KERN_INFO"%s() .. OUT\n", __func__);

	return;
}
