#include <linux/device.h>
#include <sound/core.h>
#include <sound/initval.h>
#include <sound/pcm.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/mm.h>
#include <linux/syscalls.h>
#include <asm/uaccess.h>
#include <linux/slab.h>

#include "ipc_fifo.h"
#include "bcm2835.h"

#ifdef AUDIO_VERBSE_DEBUG_ENABLE
/* Must be called with ARM<->VC fifo lock held */
static inline void dump_fifo(bcm2835_alsa_stream_t *alsa_stream)
{

	audio_info("Out fifo->\n");
	audio_info(" write - 0x%08x\n read - 0x%08x\n base - 0%08x\n size - %d\n entry_size - %d\n",
			*(uint32_t *)alsa_stream->out_fifo.write, *(uint32_t *)alsa_stream->out_fifo.read, alsa_stream->out_fifo.base, alsa_stream->out_fifo.size,
			alsa_stream->out_fifo.entry_size);

	audio_info("In fifo->\n");
	audio_info(" write - 0x%08x\n read - 0x%08x\n base - 0%08x\n size - %d\n entry_size - %d\n",
			*(uint32_t *)alsa_stream->in_fifo.write, *(uint32_t *)alsa_stream->in_fifo.read, alsa_stream->in_fifo.base, alsa_stream->in_fifo.size,
			alsa_stream->in_fifo.entry_size);


}
#else
#define dump_fifo(x) do {} while(0)
#endif

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

	audio_debug(" .. IN\n");

	intstat = readl(chip->reg_base + AUDIO_INTSTAT_OFFSET);
	audio_debug(" .. int state: %08x\n", intstat);
	/* clear intstat for next interrupt */
	writel(0, chip->reg_base + AUDIO_INTSTAT_OFFSET);

	/* is it a control ack ? */
	if (intstat & INTSTAT_CONTROL_MASK) {
		uint32_t old_status = alsa_stream->status;
		alsa_stream->status = readl(chip->reg_base + AUDIO_STATUS_OFFSET);
		audio_debug(" .. control ack\n");
		if (alsa_stream->control != alsa_stream->status) {
			audio_alert("control (0x%08x) != status (0x%08x)\n",
					alsa_stream->control, alsa_stream->status);
		}

		/* post semaphore only for enable/disable */
		if ((old_status ^ alsa_stream->status) & CTRL_EN_MASK)
			up(&alsa_stream->control_sem);
	}

	/* is it a fifo irq ? */
	if ((intstat & INTSTAT_FIFO_MASK) && alsa_stream->enable_fifo_irq) {
		audio_debug(" .. fifo ack %08x\n", alsa_stream->fifo_irq_handler);
		if (alsa_stream->fifo_irq_handler)
			ret = alsa_stream->fifo_irq_handler(irq, dev_id);
	}

	audio_debug(" .. OUT\n");

	return IRQ_HANDLED;
}

int bcm2835_audio_open(bcm2835_alsa_stream_t *alsa_stream)
{
	int err;
#ifdef DUMP_RAW_DATA
	mm_segment_t old_fs;
	int open_flags;
#endif
	bcm2835_chip_t *chip = alsa_stream->chip;

	audio_debug(" .. IN\n");
	audio_info("dest = %d, reg_base = 0x%08x\n", chip->dest, (uint32_t)chip->reg_base);

	err = request_irq(chip->irq, bcm2835_audio_irq, IRQF_DISABLED, "bcm2835 audio irq", (void *)alsa_stream);
	if (err < 0) {
		printk(KERN_ERR"Failed to request IRQ for bcm2835 playback\n");
		goto err_request_irq;
	}


	/* setup the destination and enable device*/
	writel(chip->dest, chip->reg_base + AUDIO_DESTINATION_OFFSET);
	alsa_stream->control = readl(chip->reg_base + AUDIO_CONTROL_OFFSET);
	if (alsa_stream->control & CTRL_EN_MASK) {
		audio_alert("Alsa device was already enabled !\n");
		audio_alert("Going ahead with rest of the initialisation ..\n");
	} else {

		alsa_stream->control &= ~(CTRL_MUTE_MASK);
		alsa_stream->control |= (1 << CTRL_EN_SHIFT);
		writel(chip->volume, chip->reg_base + AUDIO_VOLUME_OFFSET);
		writel(alsa_stream->control, chip->reg_base + AUDIO_CONTROL_OFFSET);
        mb();
		/* ring the doorbell */
		ipc_notify_vc_event(chip->irq);
	}

	/* wait for it .. */
	audio_info("waiting for audio to be enabled ..\n");
	down(&alsa_stream->control_sem);

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
	err = bcm2835_audio_retrieve_buffers(alsa_stream);
	if (err == 0) {
		audio_error(" Could not retrieve any hw buffers after device was enabled\n");
	}

#ifdef DUMP_RAW_DATA
	/* save the sys stack */
	old_fs = get_fs();
	set_fs(get_ds());

	// open the file
	open_flags = O_WRONLY | O_CREAT;

	alsa_stream->file = sys_open("/system/alsa.raw", open_flags, 0777);
	if (alsa_stream->file < 0) {
		printk(KERN_EMERG"%s : Failed to open raw file\n",__func__);
	}
	set_fs(old_fs);
#endif /* DUMP_RAW_DATA */

	audio_info(" success !\n");
	audio_debug(" .. OUT\n");

	return 0;

err_request_irq:
	audio_debug(" .. OUT2\n");
	return err;
}

void bcm2835_audio_close(bcm2835_alsa_stream_t *alsa_stream)
{
	bcm2835_chip_t *chip = alsa_stream->chip;
#ifdef DUMP_RAW_DATA
	mm_segment_t old_fs;
#endif
	audio_debug(" .. IN\n");

	/* Stop period elapsed irqs right now */
	alsa_stream->enable_fifo_irq = 0;

	mb();
	/* Free up all buffers in our queue */
	bcm2835_audio_flush_buffers(alsa_stream);

	/* Wait till we get playback stopped ack .. */
	audio_info("Waiting for playback to stop ..\n");

	while (alsa_stream->status & STAT_PLAY_MASK);

	alsa_stream->control = readl(chip->reg_base + AUDIO_CONTROL_OFFSET);

	alsa_stream->control &= ~(CTRL_EN_MASK);
	writel(alsa_stream->control, chip->reg_base + AUDIO_CONTROL_OFFSET);
    mb();
	/* ring the doorbell */
	ipc_notify_vc_event(chip->irq);

	/* wait for it .. (with timeout ??) */
	audio_info("waiting for audio to be disabled ..\n");
	down(&alsa_stream->control_sem);

	audio_info("Freeing alsa irq (%d)\n", chip->irq);

	free_irq(chip->irq, (void *)alsa_stream);

#ifdef DUMP_RAW_DATA
	old_fs = get_fs();
	set_fs(get_ds());
	sys_close(alsa_stream->file);
	set_fs(old_fs);
#endif
	audio_debug(" .. OUT\n");
	return;
}

int bcm2835_audio_set_params(bcm2835_alsa_stream_t *alsa_stream, uint32_t channels, uint32_t samplerate, uint32_t bps)
{
	bcm2835_chip_t *chip = alsa_stream->chip;

	audio_debug(" .. IN\n");

	if (channels < 1 || channels > 2) {
		audio_error(" channels (%d) not supported\n", channels);
		return -EINVAL;
	}

	if (samplerate != 44100) {
		audio_error(" samplerate (%d) not supported\n", samplerate);
		return -EINVAL;
	}

	if (bps != 8 && bps != 16) {
		audio_error(" Bits per sample (%d) not supported\n", bps);
		return -EINVAL;
	}

	audio_info(" Setting ALSA channels(%d), samplerate(%d), bits-per-sample(%d)\n",
								 channels, samplerate, bps);

	writel(channels, chip->reg_base + AUDIO_CHANNELS_OFFSET);
	writel(samplerate, chip->reg_base + AUDIO_SAMPLE_RATE_OFFSET);
	writel(bps, chip->reg_base + AUDIO_BIT_RATE_OFFSET);

	audio_debug(" .. OUT\n");
	return 0;
}

int bcm2835_audio_start(bcm2835_alsa_stream_t *alsa_stream)
{
	bcm2835_chip_t *chip = alsa_stream->chip;

	audio_debug(" .. IN\n");

	alsa_stream->control = readl(chip->reg_base + AUDIO_CONTROL_OFFSET);
	alsa_stream->status = readl(chip->reg_base + AUDIO_STATUS_OFFSET);

	BUG_ON(!(alsa_stream->control & CTRL_EN_MASK));

    if (alsa_stream->control != alsa_stream->status) {
        printk("control != status %d!=%d\n", alsa_stream->control, alsa_stream->status);
        if (alsa_stream->control & (1 << CTRL_PLAY_SHIFT)) {
            printk("playback already started!\n");
            return -1;
        }
    }

	alsa_stream->control |= (1 << CTRL_PLAY_SHIFT);
	writel(alsa_stream->control, chip->reg_base + AUDIO_CONTROL_OFFSET);
    mb();
	/* ring the doorbell */
	ipc_notify_vc_event(chip->irq);

	alsa_stream->enable_fifo_irq = 1;

	audio_debug(" .. OUT\n");

	return 0;
}

int bcm2835_audio_stop(bcm2835_alsa_stream_t *alsa_stream)
{
	bcm2835_chip_t *chip = alsa_stream->chip;

	audio_debug(" .. IN\n");

	alsa_stream->control = readl(chip->reg_base + AUDIO_CONTROL_OFFSET);

	BUG_ON(!(alsa_stream->control & CTRL_EN_MASK));

    if (alsa_stream->control != alsa_stream->status) {
        printk("control != status %d!=%d\n", alsa_stream->control, alsa_stream->status);
        if (!(alsa_stream->control & (1 << CTRL_PLAY_SHIFT))) {
            printk("playback already stopped!\n");
            return -1;
        }
    }

	alsa_stream->control &= ~(CTRL_PLAY_MASK);
	writel(alsa_stream->control, chip->reg_base + AUDIO_CONTROL_OFFSET);
    mb();
	/* ring the doorbell */
	ipc_notify_vc_event(chip->irq);

	audio_debug(" .. OUT\n");

	return 0;
}

int bcm2835_audio_write(bcm2835_alsa_stream_t *alsa_stream, uint32_t count, void *src)
{
	bcm2835_chip_t *chip = alsa_stream->chip;
	struct list_head *p, *next;
	bcm2835_audio_buffer_t *buffer;
	AUDIO_FIFO_ENTRY_T entry;
	size_t copy_size;
#ifdef DUMP_RAW_DATA
	mm_segment_t old_fs;
	int write_count;
#endif

	/* Check the number of buffers required for the job first and see if
	 * that fits into current available buffers to us
	 */

	audio_debug(" .. IN\n");

	if (count == 0)
		goto out;

	/* Block till we have enough free buffers : assuming 2K buffers */
	while (atomic_read(&alsa_stream->buffer_count) * AUDIO_IPC_BLOCK_BUFFER_SIZE < count) {
		if (down_trylock(&alsa_stream->buffers_update_sem)) {
			/* Poke VC to read from the fifo ..*/
			audio_alert("No space to wrie on this alsa device\n");
			audio_alert("Availabe buffers(%d), size(%d), total needed (%d)\n",
					atomic_read(&alsa_stream->buffer_count),
					(atomic_read(&alsa_stream->buffer_count) * AUDIO_IPC_BLOCK_BUFFER_SIZE), count);
			audio_info(" Trying to wake VC up ..\n");
			dump_fifo(alsa_stream);
            mb();
			ipc_notify_vc_event(chip->irq);
		}
	}

#ifdef DUMP_RAW_DATA
	old_fs = get_fs();
	set_fs(get_ds());
	write_count = sys_write(alsa_stream->file, src, count);
	if (write_count != count) {
		printk(KERN_EMERG"%s: write to file failed: written (%d), asked (%d)\n",__func__, write_count, count);
	}
	set_fs(old_fs);
#endif

	list_for_each_safe(p, next, &alsa_stream->buffer_list) {

		if (count == 0)
			break;

		copy_size = 0;

		buffer = list_entry(p, bcm2835_audio_buffer_t, link);

        /* If first time getting this buffer */
        if (buffer->start == NULL) {
            audio_debug(" ioremapping phys_addr (0x%08x)\n",__VC_BUS_TO_ARM_PHYS_ADDR(buffer->bus_addr));
            buffer->start = (uint8_t *)ioremap(__VC_BUS_TO_ARM_PHYS_ADDR(buffer->bus_addr), buffer->size);
        }

        /* Check if ioremap failed */
		if (buffer->start == NULL) {
			audio_error(" Failed to Ioremap buffer from phys(0x%08x), size(%d)\n",
					(uint32_t)__VC_BUS_TO_ARM_PHYS_ADDR(buffer->bus_addr), buffer->size);
			kfree(buffer);
			break;
		}

		copy_size = count > buffer->data_left ? buffer->data_left : count;
		audio_debug(" Copying into buffer (%d: %08x offset: %d),for size (%d)\n", buffer->buffer_id, buffer->start, buffer->size - buffer->data_left, copy_size);
		memcpy((buffer->start + buffer->size - buffer->data_left), src, copy_size);
		buffer->data_left -= copy_size;
		count -= copy_size;

		if (buffer->data_left) {
			/* This buffer is only partially filled, so not doing
			 * anything */
			BUG_ON(count);
			audio_debug("partial filled buf\n");
			break;
		}

		entry.buffer_id = buffer->buffer_id;
		entry.buffer_size = buffer->size;
		entry.buffer_ptr = buffer->bus_addr;
		audio_debug("sending buf to VC id: %08x sz: %08x adr: %08x cnt: %d\n", buffer->buffer_id, buffer->size, buffer->bus_addr, atomic_read(&alsa_stream->buffer_count));
		iounmap(buffer->start);
		list_del_init(p);
		atomic_dec(&alsa_stream->buffer_count);
		kfree(buffer);

		/* add into out fifo, we know its not full as long as we are
		 * runnnig this loop
		 */
		BUG_ON(ipc_fifo_full(&alsa_stream->out_fifo));
		ipc_fifo_write(&alsa_stream->out_fifo, &entry);
	}
    mb();
	/* ring the doorbell */
	ipc_notify_vc_event(chip->irq);
	
out:
	audio_debug(" .. OUT\n");
	return 0;
}

uint32_t bcm2835_audio_retrieve_buffers(bcm2835_alsa_stream_t *alsa_stream)
{
	bcm2835_audio_buffer_t *buffer;
	AUDIO_FIFO_ENTRY_T entry;
	unsigned long flags;
	uint32_t retrieved = 0;

	audio_debug(" .. IN\n");

	/* get the lock first */
	spin_lock_irqsave(&alsa_stream->lock, flags);
//	bcm2835_audio_fifo_get_lock(alsa_stream);

	while (!ipc_fifo_empty(&alsa_stream->in_fifo)) {
		memset(&entry, 0, sizeof(entry));
		ipc_fifo_read(&alsa_stream->in_fifo, &entry);
		if (entry.buffer_ptr != 0) {
			buffer = kzalloc(sizeof(*buffer), GFP_KERNEL | GFP_ATOMIC);
			buffer->buffer_id = entry.buffer_id;
			buffer->bus_addr = entry.buffer_ptr;
			buffer->size = entry.buffer_size;
			buffer->data_left = entry.buffer_size;
			audio_debug(" Adding buffer(%d) @ bus_add(0x%08x), size (%d) to list\n", buffer->buffer_id, (uint32_t)buffer->bus_addr, buffer->size);
			list_add_tail(&buffer->link, &alsa_stream->buffer_list);
			mb();
			// Make sure data is copied before inc
			atomic_inc(&alsa_stream->buffer_count);
			retrieved++;
			up(&alsa_stream->buffers_update_sem);
		} else {
			audio_error(" Input fifo had a NULL entry\n");
			audio_error(" Breaking out now \n");
			break;
		}
	}

	dump_fifo(alsa_stream);

	audio_info(" Buffers avail(%d), retrieved (%d)\n", atomic_read(&alsa_stream->buffer_count), retrieved);
	/* release the lock here */
//	bcm2835_audio_fifo_put_lock(alsa_stream);
	spin_unlock_irqrestore(&alsa_stream->lock, flags);
	audio_debug(" .. OUT\n");

	return retrieved;
}

void bcm2835_audio_flush_buffers(bcm2835_alsa_stream_t *alsa_stream)
{
	struct list_head *p, *next;
	bcm2835_audio_buffer_t *buffer;
	unsigned long flags;

	audio_debug(" .. IN\n");

	spin_lock_irqsave(&alsa_stream->lock, flags);

	audio_info("(%d) buffers still in the queue, flushing them out\n", atomic_read(&alsa_stream->buffer_count));

	list_for_each_safe(p, next, &alsa_stream->buffer_list) {
		buffer = list_entry(p, bcm2835_audio_buffer_t, link);

		if (buffer->start != NULL)
			iounmap(buffer->start);

		list_del_init(p);
		atomic_dec(&alsa_stream->buffer_count);
		kfree(buffer);
	}

	spin_unlock_irqrestore(&alsa_stream->lock, flags);
	
	audio_debug(" .. OUT\n");

	return;
}
