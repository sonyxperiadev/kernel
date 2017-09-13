/*
 *  linux/drivers/mmc/host/sdhci.c - Secure Digital Host Controller Interface driver
 *
 *  Copyright (C) 2005-2008 Pierre Ossman, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * Thanks to the following companies for their support:
 *
 *     - JMicron (hardware and technical support)
 */

#include <linux/delay.h>
#include <linux/highmem.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/scatterlist.h>
#include <linux/regulator/consumer.h>
#include <linux/pm_runtime.h>

#include <linux/leds.h>

#include <linux/mmc/mmc.h>
#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include <linux/mmc/slot-gpio.h>
#include <linux/mmc/sdio.h>

#include <trace/events/mmc.h>

#include "sdhci.h"
#include "cmdq_hci.h"

#define DRIVER_NAME "sdhci"
#define SDHCI_SUSPEND_TIMEOUT 300 /* 300 ms */
#define SDHCI_PM_QOS_DEFAULT_DELAY 5 /* 5 ms */
#define SDHCI_MAX_PM_QOS_TIMEOUT   100 /* 100 ms */

#define DBG(f, x...) \
	pr_debug(DRIVER_NAME " [%s()]: " f, __func__,## x)

#if defined(CONFIG_LEDS_CLASS) || (defined(CONFIG_LEDS_CLASS_MODULE) && \
	defined(CONFIG_MMC_SDHCI_MODULE))
#define SDHCI_USE_LEDS_CLASS
#endif

#define MAX_TUNING_LOOP 40

#define SDHCI_DBG_DUMP_RS_INTERVAL (10 * HZ)
#define SDHCI_DBG_DUMP_RS_BURST 2

static unsigned int debug_quirks = 0;
static unsigned int debug_quirks2;

static void sdhci_finish_data(struct sdhci_host *);

static void sdhci_send_command(struct sdhci_host *, struct mmc_command *);
static void sdhci_finish_command(struct sdhci_host *);
static int sdhci_execute_tuning(struct mmc_host *mmc, u32 opcode);
static int sdhci_enhanced_strobe(struct mmc_host *mmc);
static void sdhci_tuning_timer(unsigned long data);
static void sdhci_enable_preset_value(struct sdhci_host *host, bool enable);
static bool sdhci_check_state(struct sdhci_host *);
static void sdhci_show_adma_error(struct sdhci_host *host);

#ifdef CONFIG_PM_RUNTIME
static int sdhci_runtime_pm_get(struct sdhci_host *host);
static int sdhci_runtime_pm_put(struct sdhci_host *host);
static void sdhci_dump_rpm_info(struct sdhci_host *host)
{
	struct mmc_host *mmc = host->mmc;

	pr_info("%s: rpmstatus[pltfm](runtime-suspend:usage_count:disable_depth)(%d:%d:%d)\n",
		mmc_hostname(mmc), mmc->parent->power.runtime_status,
		atomic_read(&mmc->parent->power.usage_count),
		mmc->parent->power.disable_depth);
}
#else
static inline int sdhci_runtime_pm_get(struct sdhci_host *host)
{
	return 0;
}
static inline int sdhci_runtime_pm_put(struct sdhci_host *host)
{
	return 0;
}
static void sdhci_dump_rpm_info(struct sdhci_host *host)
{
}
#endif

static inline int sdhci_get_async_int_status(struct sdhci_host *host)
{
	return (sdhci_readw(host, SDHCI_HOST_CONTROL2) &
		 SDHCI_CTRL_ASYNC_INT_ENABLE) >> 14;
}

static void sdhci_dump_state(struct sdhci_host *host)
{
	struct mmc_host *mmc = host->mmc;

	pr_info("%s: eMMC FW version: 0x%02x Manfid: 0x%06x\n",
		mmc_hostname(mmc), get_mmc_fw_version(host->mmc->card),
		get_mmc_manfid(host->mmc->card));
	pr_info("%s: clk: %d clk-gated: %d claimer: %s pwr: %d\n",
		mmc_hostname(mmc), host->clock, mmc->clk_gated,
		mmc->claimer->comm, host->pwr);
	sdhci_dump_rpm_info(host);
}

static void sdhci_dumpregs(struct sdhci_host *host)
{
	pr_info(DRIVER_NAME ": =========== REGISTER DUMP (%s)===========\n",
		mmc_hostname(host->mmc));

	pr_info(DRIVER_NAME ": Sys addr: 0x%08x | Version:  0x%08x\n",
		sdhci_readl(host, SDHCI_DMA_ADDRESS),
		sdhci_readw(host, SDHCI_HOST_VERSION));
	pr_info(DRIVER_NAME ": Blk size: 0x%08x | Blk cnt:  0x%08x\n",
		sdhci_readw(host, SDHCI_BLOCK_SIZE),
		sdhci_readw(host, SDHCI_BLOCK_COUNT));
	pr_info(DRIVER_NAME ": Argument: 0x%08x | Trn mode: 0x%08x\n",
		sdhci_readl(host, SDHCI_ARGUMENT),
		sdhci_readw(host, SDHCI_TRANSFER_MODE));
	pr_info(DRIVER_NAME ": Present:  0x%08x | Host ctl: 0x%08x\n",
		sdhci_readl(host, SDHCI_PRESENT_STATE),
		sdhci_readb(host, SDHCI_HOST_CONTROL));
	pr_info(DRIVER_NAME ": Power:    0x%08x | Blk gap:  0x%08x\n",
		sdhci_readb(host, SDHCI_POWER_CONTROL),
		sdhci_readb(host, SDHCI_BLOCK_GAP_CONTROL));
	pr_info(DRIVER_NAME ": Wake-up:  0x%08x | Clock:    0x%08x\n",
		sdhci_readb(host, SDHCI_WAKE_UP_CONTROL),
		sdhci_readw(host, SDHCI_CLOCK_CONTROL));
	pr_info(DRIVER_NAME ": Timeout:  0x%08x | Int stat: 0x%08x\n",
		sdhci_readb(host, SDHCI_TIMEOUT_CONTROL),
		sdhci_readl(host, SDHCI_INT_STATUS));
	pr_info(DRIVER_NAME ": Int enab: 0x%08x | Sig enab: 0x%08x\n",
		sdhci_readl(host, SDHCI_INT_ENABLE),
		sdhci_readl(host, SDHCI_SIGNAL_ENABLE));
	pr_info(DRIVER_NAME ": AC12 err: 0x%08x | Slot int: 0x%08x\n",
		host->auto_cmd_err_sts,
		sdhci_readw(host, SDHCI_SLOT_INT_STATUS));
	pr_info(DRIVER_NAME ": Caps:     0x%08x | Caps_1:   0x%08x\n",
		sdhci_readl(host, SDHCI_CAPABILITIES),
		sdhci_readl(host, SDHCI_CAPABILITIES_1));
	pr_info(DRIVER_NAME ": Cmd:      0x%08x | Max curr: 0x%08x\n",
		sdhci_readw(host, SDHCI_COMMAND),
		sdhci_readl(host, SDHCI_MAX_CURRENT));
	pr_info(DRIVER_NAME ": Resp 1:   0x%08x | Resp 0:   0x%08x\n",
		sdhci_readl(host, SDHCI_RESPONSE + 0x4),
		sdhci_readl(host, SDHCI_RESPONSE));
	pr_info(DRIVER_NAME ": Resp 3:   0x%08x | Resp 2:   0x%08x\n",
		sdhci_readl(host, SDHCI_RESPONSE + 0xC),
		sdhci_readl(host, SDHCI_RESPONSE + 0x8));
	pr_info(DRIVER_NAME ": Host ctl2: 0x%08x\n",
		sdhci_readw(host, SDHCI_HOST_CONTROL2));

	if (host->flags & SDHCI_USE_ADMA_64BIT) {
		pr_info(DRIVER_NAME ": ADMA Err: 0x%08x\n",
		       readl(host->ioaddr + SDHCI_ADMA_ERROR));
		pr_info(DRIVER_NAME ": ADMA Addr(0:31): 0x%08x | ADMA Addr(32:63): 0x%08x\n",
		       readl(host->ioaddr + SDHCI_ADMA_ADDRESS_LOW),
		       readl(host->ioaddr + SDHCI_ADMA_ADDRESS_HIGH));
	} else if (host->flags & SDHCI_USE_ADMA) {
		pr_info(DRIVER_NAME ": ADMA Err: 0x%08x | ADMA Ptr: 0x%08x\n",
		       readl(host->ioaddr + SDHCI_ADMA_ERROR),
		       readl(host->ioaddr + SDHCI_ADMA_ADDRESS_LOW));
	}

	if (host->ops->dump_vendor_regs)
		host->ops->dump_vendor_regs(host);
	sdhci_dump_state(host);
	pr_info(DRIVER_NAME ": ===========================================\n");
}

static ssize_t
show_sdhci_pm_qos_tout(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	struct sdhci_host *host = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%d ms\n", host->pm_qos_timeout_us);
}

static ssize_t
store_sdhci_pm_qos_tout(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct sdhci_host *host = dev_get_drvdata(dev);
	uint32_t value;
	unsigned long flags;

	if (!kstrtou32(buf, 0, &value)) {
		spin_lock_irqsave(&host->lock, flags);
		if (value <= SDHCI_MAX_PM_QOS_TIMEOUT && value > 1)
			host->pm_qos_timeout_us = value;
		spin_unlock_irqrestore(&host->lock, flags);
	}
	return count;
}

/*****************************************************************************\
 *                                                                           *
 * Low level functions                                                       *
 *                                                                           *
\*****************************************************************************/

static void sdhci_clear_set_irqs(struct sdhci_host *host, u32 clear, u32 set)
{
	u32 ier;

	ier = sdhci_readl(host, SDHCI_INT_ENABLE);
	ier &= ~clear;
	ier |= set;
	sdhci_writel(host, ier, SDHCI_INT_ENABLE);
	sdhci_writel(host, ier, SDHCI_SIGNAL_ENABLE);
}

static void sdhci_unmask_irqs(struct sdhci_host *host, u32 irqs)
{
	sdhci_clear_set_irqs(host, 0, irqs);
}

static void sdhci_mask_irqs(struct sdhci_host *host, u32 irqs)
{
	sdhci_clear_set_irqs(host, irqs, 0);
}

static void sdhci_set_card_detection(struct sdhci_host *host, bool enable)
{
	u32 present, irqs;

	if ((host->quirks & SDHCI_QUIRK_BROKEN_CARD_DETECTION) ||
	    (host->mmc->caps & MMC_CAP_NONREMOVABLE))
		return;

	present = sdhci_readl(host, SDHCI_PRESENT_STATE) &
			      SDHCI_CARD_PRESENT;
	irqs = present ? SDHCI_INT_CARD_REMOVE : SDHCI_INT_CARD_INSERT;

	if (enable)
		sdhci_unmask_irqs(host, irqs);
	else
		sdhci_mask_irqs(host, irqs);
}

static void sdhci_enable_card_detection(struct sdhci_host *host)
{
	sdhci_set_card_detection(host, true);
}

static void sdhci_disable_card_detection(struct sdhci_host *host)
{
	sdhci_set_card_detection(host, false);
}

static void sdhci_reset(struct sdhci_host *host, u8 mask)
{
	unsigned long timeout;
	u32 uninitialized_var(ier);

	if (host->quirks & SDHCI_QUIRK_NO_CARD_NO_RESET) {
		if (!(sdhci_readl(host, SDHCI_PRESENT_STATE) &
			SDHCI_CARD_PRESENT))
			return;
	}

	if (host->quirks & SDHCI_QUIRK_RESTORE_IRQS_AFTER_RESET)
		ier = sdhci_readl(host, SDHCI_INT_ENABLE);

retry_reset:
	if (host->ops->platform_reset_enter)
		host->ops->platform_reset_enter(host, mask);

	sdhci_writeb(host, mask, SDHCI_SOFTWARE_RESET);

	if (mask & SDHCI_RESET_ALL)
		host->clock = 0;

	/* Wait max 100 ms */
	timeout = 100000;

	if (host->ops->check_power_status && host->pwr &&
	    (mask & SDHCI_RESET_ALL))
		host->ops->check_power_status(host, REQ_BUS_OFF);

	/* hw clears the bit when it's done */
	while (sdhci_readb(host, SDHCI_SOFTWARE_RESET) & mask) {
		if (timeout == 0) {
			pr_err("%s: Reset 0x%x never completed.\n",
				mmc_hostname(host->mmc), (int)mask);
			if ((host->quirks2 & SDHCI_QUIRK2_USE_RESET_WORKAROUND)
				&& host->ops->reset_workaround) {
				if (!host->reset_wa_applied) {
					/*
					 * apply the workaround and issue
					 * reset again.
					 */
					host->ops->reset_workaround(host, 1);
					host->reset_wa_applied = 1;
					host->reset_wa_cnt++;
					goto retry_reset;
				} else {
					pr_err("%s: Reset 0x%x failed with workaround\n",
						mmc_hostname(host->mmc),
						(int)mask);
					/* clear the workaround */
					host->ops->reset_workaround(host, 0);
					host->reset_wa_applied = 0;
				}
			}

			sdhci_dumpregs(host);
			return;
		}
		timeout--;
		udelay(1);
	}

	if (host->ops->platform_reset_exit)
		host->ops->platform_reset_exit(host, mask);

	if ((host->quirks2 & SDHCI_QUIRK2_USE_RESET_WORKAROUND) &&
		host->ops->reset_workaround && host->reset_wa_applied) {
		pr_info("%s: Reset 0x%x successful with workaround\n",
			mmc_hostname(host->mmc), (int)mask);
		/* clear the workaround */
		host->ops->reset_workaround(host, 0);
		host->reset_wa_applied = 0;
	}
	/* clear pending normal/error interrupt status */
	sdhci_writel(host, sdhci_readl(host, SDHCI_INT_STATUS),
			SDHCI_INT_STATUS);

	if (host->quirks & SDHCI_QUIRK_RESTORE_IRQS_AFTER_RESET)
		sdhci_clear_set_irqs(host, SDHCI_INT_ALL_MASK, ier);

	if (host->flags & (SDHCI_USE_SDMA | SDHCI_USE_ADMA)) {
		if ((host->ops->enable_dma) && (mask & SDHCI_RESET_ALL))
			host->ops->enable_dma(host);
	}
	if (host->is_crypto_en)
		host->crypto_reset_reqd = true;
}

static void sdhci_set_ios(struct mmc_host *mmc, struct mmc_ios *ios);

static void sdhci_init(struct sdhci_host *host, int soft)
{
	if (soft)
		sdhci_reset(host, SDHCI_RESET_CMD|SDHCI_RESET_DATA);
	else
		sdhci_reset(host, SDHCI_RESET_ALL);

	sdhci_clear_set_irqs(host, SDHCI_INT_ALL_MASK,
		SDHCI_INT_BUS_POWER | SDHCI_INT_DATA_END_BIT |
		SDHCI_INT_DATA_CRC | SDHCI_INT_DATA_TIMEOUT | SDHCI_INT_INDEX |
		SDHCI_INT_END_BIT | SDHCI_INT_CRC | SDHCI_INT_TIMEOUT |
		SDHCI_INT_DATA_END | SDHCI_INT_RESPONSE |
			     SDHCI_INT_AUTO_CMD_ERR);

	if (soft) {
		/* force clock reconfiguration */
		host->clock = 0;
		sdhci_set_ios(host->mmc, &host->mmc->ios);
	}
}

static void sdhci_reinit(struct sdhci_host *host)
{
	sdhci_init(host, 0);
	/*
	 * Retuning stuffs are affected by different cards inserted and only
	 * applicable to UHS-I cards. So reset these fields to their initial
	 * value when card is removed.
	 */
	if (host->flags & SDHCI_USING_RETUNING_TIMER) {
		host->flags &= ~SDHCI_USING_RETUNING_TIMER;

		del_timer_sync(&host->tuning_timer);
		host->flags &= ~SDHCI_NEEDS_RETUNING;
		host->mmc->max_blk_count =
			(host->quirks & SDHCI_QUIRK_NO_MULTIBLOCK) ? 1 : 65535;
	}
	sdhci_enable_card_detection(host);
}

static void sdhci_activate_led(struct sdhci_host *host)
{
	u8 ctrl;

	ctrl = sdhci_readb(host, SDHCI_HOST_CONTROL);
	ctrl |= SDHCI_CTRL_LED;
	sdhci_writeb(host, ctrl, SDHCI_HOST_CONTROL);
}

static void sdhci_deactivate_led(struct sdhci_host *host)
{
	u8 ctrl;

	ctrl = sdhci_readb(host, SDHCI_HOST_CONTROL);
	ctrl &= ~SDHCI_CTRL_LED;
	sdhci_writeb(host, ctrl, SDHCI_HOST_CONTROL);
}

#ifdef SDHCI_USE_LEDS_CLASS
static void sdhci_led_control(struct led_classdev *led,
	enum led_brightness brightness)
{
	struct sdhci_host *host = container_of(led, struct sdhci_host, led);
	unsigned long flags;

	spin_lock_irqsave(&host->lock, flags);

	if (host->runtime_suspended || sdhci_check_state(host))
		goto out;

	if (brightness == LED_OFF)
		sdhci_deactivate_led(host);
	else
		sdhci_activate_led(host);
out:
	spin_unlock_irqrestore(&host->lock, flags);
}
#endif

/*****************************************************************************\
 *                                                                           *
 * Core functions                                                            *
 *                                                                           *
\*****************************************************************************/

static void sdhci_read_block_pio(struct sdhci_host *host)
{
	unsigned long flags;
	size_t blksize, len, chunk;
	u32 uninitialized_var(scratch);
	u8 *buf;

	DBG("PIO reading\n");

	blksize = host->data->blksz;
	chunk = 0;

	local_irq_save(flags);

	while (blksize) {
		if (!sg_miter_next(&host->sg_miter))
			BUG();

		len = min(host->sg_miter.length, blksize);

		blksize -= len;
		host->sg_miter.consumed = len;

		buf = host->sg_miter.addr;

		while (len) {
			if (chunk == 0) {
				scratch = sdhci_readl(host, SDHCI_BUFFER);
				chunk = 4;
			}

			*buf = scratch & 0xFF;

			buf++;
			scratch >>= 8;
			chunk--;
			len--;
		}
	}

	sg_miter_stop(&host->sg_miter);

	local_irq_restore(flags);
}

static void sdhci_write_block_pio(struct sdhci_host *host)
{
	unsigned long flags;
	size_t blksize, len, chunk;
	u32 scratch;
	u8 *buf;

	DBG("PIO writing\n");

	blksize = host->data->blksz;
	chunk = 0;
	scratch = 0;

	local_irq_save(flags);

	while (blksize) {
		if (!sg_miter_next(&host->sg_miter))
			BUG();

		len = min(host->sg_miter.length, blksize);

		blksize -= len;
		host->sg_miter.consumed = len;

		buf = host->sg_miter.addr;

		while (len) {
			scratch |= (u32)*buf << (chunk * 8);

			buf++;
			chunk++;
			len--;

			if ((chunk == 4) || ((len == 0) && (blksize == 0))) {
				sdhci_writel(host, scratch, SDHCI_BUFFER);
				chunk = 0;
				scratch = 0;
			}
		}
	}

	sg_miter_stop(&host->sg_miter);

	local_irq_restore(flags);
}

static void sdhci_transfer_pio(struct sdhci_host *host)
{
	u32 mask;

	BUG_ON(!host->data);

	if (host->blocks == 0)
		return;

	if (host->data->flags & MMC_DATA_READ)
		mask = SDHCI_DATA_AVAILABLE;
	else
		mask = SDHCI_SPACE_AVAILABLE;

	/*
	 * Some controllers (JMicron JMB38x) mess up the buffer bits
	 * for transfers < 4 bytes. As long as it is just one block,
	 * we can ignore the bits.
	 */
	if ((host->quirks & SDHCI_QUIRK_BROKEN_SMALL_PIO) &&
		(host->data->blocks == 1))
		mask = ~0;

	while (sdhci_readl(host, SDHCI_PRESENT_STATE) & mask) {
		if (host->quirks & SDHCI_QUIRK_PIO_NEEDS_DELAY)
			udelay(100);

		if (host->data->flags & MMC_DATA_READ)
			sdhci_read_block_pio(host);
		else
			sdhci_write_block_pio(host);

		host->blocks--;
		if (host->blocks == 0)
			break;
	}

	DBG("PIO transfer complete.\n");
}

static char *sdhci_kmap_atomic(struct scatterlist *sg, unsigned long *flags)
{
	local_irq_save(*flags);
	return kmap_atomic(sg_page(sg)) + sg->offset;
}

static void sdhci_kunmap_atomic(void *buffer, unsigned long *flags)
{
	kunmap_atomic(buffer);
	local_irq_restore(*flags);
}

static void sdhci_set_adma_desc(struct sdhci_host *host, u8 *desc,
				dma_addr_t addr, int len, unsigned cmd)
{
	__le16 *cmdlen = (__le16 __force *)desc;

	/*
	 * SDHCI specification says ADMA descriptors should be 4 byte
	 * or 8 byte aligned, so using 16 or 32 or 64bit operations
	 * should be safe.
	 */

	cmdlen[0] = cpu_to_le16(cmd);
	cmdlen[1] = cpu_to_le16(len);

	if (host->flags & SDHCI_USE_ADMA_64BIT) {
		__le64 *dataddr = (__le64 __force *)(desc + 4);
		dataddr[0] = cpu_to_le64(addr);
	} else {
		__le32 *dataddr = (__le32 __force *)(desc + 4);
		dataddr[0] = cpu_to_le32(addr);
	}
}

static int sdhci_pre_dma_transfer(struct sdhci_host *host,
				  struct mmc_data *data,
				  struct sdhci_next *next)
{
	int sg_count;
	unsigned long flags;

	spin_lock_irqsave(&host->next_lock, flags);

	if (!next && data->host_cookie &&
	    data->host_cookie != host->next_data.cookie) {
		printk(KERN_WARNING "[%s] invalid cookie: data->host_cookie %d"
		       " host->next_data.cookie %d\n",
		       __func__, data->host_cookie, host->next_data.cookie);
		data->host_cookie = 0;
	}

	/* Check if next job is already prepared */
	if (next ||
	    (!next && data->host_cookie != host->next_data.cookie)) {
		sg_count = dma_map_sg(mmc_dev(host->mmc), data->sg,
				      data->sg_len,
				      (data->flags & MMC_DATA_WRITE) ?
				      DMA_TO_DEVICE : DMA_FROM_DEVICE);
	} else {
		sg_count = host->next_data.sg_count;
		host->next_data.sg_count = 0;
	}

	if (sg_count == 0) {
		spin_unlock_irqrestore(&host->next_lock, flags);
		return -EINVAL;
	}

	if (next) {
		next->sg_count = sg_count;
		data->host_cookie = ++next->cookie < 0 ? 1 : next->cookie;
	} else
		host->sg_count = sg_count;

	spin_unlock_irqrestore(&host->next_lock, flags);

	return sg_count;
}

static int sdhci_adma_table_pre(struct sdhci_host *host,
	struct mmc_data *data)
{
	u8 *desc;
	u8 *align;
	dma_addr_t addr;
	dma_addr_t align_addr;
	int len, offset;

	struct scatterlist *sg;
	int i;
	char *buffer;
	unsigned long flags;

	/*
	 * The spec does not specify endianness of descriptor table.
	 * We currently guess that it is LE.
	 */

	host->sg_count = sdhci_pre_dma_transfer(host, data, NULL);
	if (host->sg_count < 0)
		goto fail;

	desc = host->adma_desc;
	align = host->align_buffer;

	align_addr = host->align_addr;

	for_each_sg(data->sg, sg, host->sg_count, i) {
		addr = sg_dma_address(sg);
		len = sg_dma_len(sg);

		if (!(host->quirks2 & SDHCI_QUIRK2_ADMA_SKIP_DATA_ALIGNMENT)) {
			/*
			 * The SDHCI specification states that ADMA addresses
			 * must be 32-bit aligned for 32-bit ADMA or 64-bit
			 * aligned for 64-bit ADMA. If they aren't, then we use
			 * a bounce buffer for the (up to three for 32-bit and
			 * up to seven for 64-bit) bytes that screw up the
			 * alignment.
			 */
			offset = (host->align_bytes
				- (addr & (host->align_bytes - 1)))
				& (host->align_bytes - 1);
			if (offset) {
				if (data->flags & MMC_DATA_WRITE) {
					buffer = sdhci_kmap_atomic(sg, &flags);
					/*
					 * This check is intended here to verify
					 * if the page offset plus alignment
					 * bytes is indeed within the same page.
					 */
				WARN_ON(((long)buffer & (PAGE_SIZE - 1)) >
					(PAGE_SIZE - (host->align_bytes - 1)));
					memcpy(align, buffer, offset);
					sdhci_kunmap_atomic(buffer, &flags);
				}

				/* tran, valid */
				sdhci_set_adma_desc(host, desc, align_addr,
						offset, 0x21);

				BUG_ON(offset > 65536);

				align += host->align_bytes;
				align_addr += host->align_bytes;

				desc += host->adma_desc_line_sz;

				addr += offset;
				len -= offset;
			}
		}

		BUG_ON(len > 65536);

		if (len) {
			/* tran, valid */
			sdhci_set_adma_desc(host, desc, addr, len, 0x21);
			desc += host->adma_desc_line_sz;
		}

		/*
		 * If this triggers then we have a calculation bug
		 * somewhere. :/
		 */
		WARN_ON((desc - host->adma_desc) > host->adma_desc_sz);

	}

	if (host->quirks & SDHCI_QUIRK_NO_ENDATTR_IN_NOPDESC) {
		/*
		* Mark the last descriptor as the terminating descriptor
		*/
		if (desc != host->adma_desc) {
			desc -= host->adma_desc_line_sz;
			desc[0] |= 0x2; /* end */
		}
	} else {
		/*
		* Add a terminating entry.
		*/

		/* nop, end, valid */
		sdhci_set_adma_desc(host, desc, 0, 0, 0x3);
	}

	return 0;

fail:
	return -EINVAL;
}

static void sdhci_adma_table_post(struct sdhci_host *host,
	struct mmc_data *data)
{
	int direction = (data->flags & MMC_DATA_WRITE)
				     ? DMA_TO_DEVICE : DMA_FROM_DEVICE;
	struct scatterlist *sg;
	int i, size;
	u8 *align;
	char *buffer;
	unsigned long flags;
	bool has_unaligned = false;
	u32 command = SDHCI_GET_CMD(sdhci_readw(host, SDHCI_COMMAND));

	trace_mmc_adma_table_post(command, data->sg_len);

	if (!(host->quirks2 & SDHCI_QUIRK2_ADMA_SKIP_DATA_ALIGNMENT)
			&& (data->flags & MMC_DATA_READ)) {

		/* Do a quick scan of the SG list for any unaligned mappings */
		for_each_sg(data->sg, sg, host->sg_count, i) {
			if (sg_dma_address(sg) & (host->align_bytes - 1)) {
				has_unaligned = true;
				break;
			}
		}
	}

	if (!(host->quirks2 & SDHCI_QUIRK2_ADMA_SKIP_DATA_ALIGNMENT)
	     && has_unaligned) {
		dma_sync_sg_for_cpu(mmc_dev(host->mmc), data->sg,
			data->sg_len, direction);

		align = host->align_buffer;

		for_each_sg(data->sg, sg, host->sg_count, i) {
			if (sg_dma_address(sg) & (host->align_bytes - 1)) {
				size = host->align_bytes - (sg_dma_address(sg)
					& (host->align_bytes - 1));

				buffer = sdhci_kmap_atomic(sg, &flags);
				/*
				 * This check is intended here to verify if the
				 * page offset plus alignment bytes is indeed
				 * within the same page.
				 */
				WARN_ON(((long)buffer & (PAGE_SIZE - 1)) >
					(PAGE_SIZE - (host->align_bytes - 1)));
				memcpy(buffer, align, size);
				sdhci_kunmap_atomic(buffer, &flags);

				align += host->align_bytes;
			}
		}
	}

	if (!data->host_cookie)
		dma_unmap_sg(mmc_dev(host->mmc), data->sg, data->sg_len,
			     direction);
}

static u8 sdhci_calc_timeout(struct sdhci_host *host, struct mmc_command *cmd)
{
	u8 count;
	struct mmc_data *data = cmd->data;
	unsigned target_timeout, current_timeout;
	u32 curr_clk = 0; /* In KHz */

	/*
	 * If the host controller provides us with an incorrect timeout
	 * value, just skip the check and use 0xE.  The hardware may take
	 * longer to time out, but that's much better than having a too-short
	 * timeout value.
	 */
	if (host->quirks & SDHCI_QUIRK_BROKEN_TIMEOUT_VAL)
		return 0xE;

	/* Unspecified timeout, assume max */
	if (!data && !cmd->cmd_timeout_ms)
		return 0xE;

	/* timeout in us */
	if (!data)
		target_timeout = cmd->cmd_timeout_ms * 1000;
	else {
		target_timeout = data->timeout_ns / 1000;
		if (host->clock)
			target_timeout += data->timeout_clks / host->clock;
	}

	/*
	 * Figure out needed cycles.
	 * We do this in steps in order to fit inside a 32 bit int.
	 * The first step is the minimum timeout, which will have a
	 * minimum resolution of 6 bits:
	 * (1) 2^13*1000 > 2^22,
	 * (2) host->timeout_clk < 2^16
	 *     =>
	 *     (1) / (2) > 2^6
	 */
	count = 0;
	if (host->quirks2 & SDHCI_QUIRK2_ALWAYS_USE_BASE_CLOCK) {
		curr_clk = host->clock / 1000;
		if (host->quirks2 & SDHCI_QUIRK2_DIVIDE_TOUT_BY_4)
			curr_clk /= 4;
		current_timeout = (1 << 13) * 1000 / curr_clk;
	} else {
		current_timeout = (1 << 13) * 1000 / host->timeout_clk;
	}
	while (current_timeout < target_timeout) {
		count++;
		current_timeout <<= 1;
		if (count >= 0xF)
			break;
	}

	if (!(host->quirks2 & SDHCI_QUIRK2_USE_RESERVED_MAX_TIMEOUT)) {
		if (count >= 0xF) {
			DBG("%s: Too large timeout 0x%x requested for CMD%d!\n",
			    mmc_hostname(host->mmc), count, cmd->opcode);
			count = 0xE;
		}
	}

	return count;
}

static void sdhci_set_transfer_irqs(struct sdhci_host *host)
{
	u32 pio_irqs = SDHCI_INT_DATA_AVAIL | SDHCI_INT_SPACE_AVAIL;
	u32 dma_irqs = SDHCI_INT_DMA_END | SDHCI_INT_ADMA_ERROR;

	if (host->flags & SDHCI_REQ_USE_DMA)
		sdhci_clear_set_irqs(host, pio_irqs, dma_irqs);
	else
		sdhci_clear_set_irqs(host, dma_irqs, pio_irqs);
}

static void sdhci_set_blk_size_reg(struct sdhci_host *host, unsigned int blksz,
				   unsigned int sdma_boundary)
{
	if (host->flags & SDHCI_USE_ADMA)
		sdhci_writew(host, SDHCI_MAKE_BLKSZ(0, blksz),
			     SDHCI_BLOCK_SIZE);
	else
		sdhci_writew(host, SDHCI_MAKE_BLKSZ(sdma_boundary, blksz),
			     SDHCI_BLOCK_SIZE);
}

static void sdhci_prepare_data(struct sdhci_host *host, struct mmc_command *cmd)
{
	u8 count;
	u8 ctrl;
	struct mmc_data *data = cmd->data;
	int ret;

	WARN_ON(host->data);

	if (data || (cmd->flags & MMC_RSP_BUSY)) {
		count = sdhci_calc_timeout(host, cmd);
		sdhci_writeb(host, count, SDHCI_TIMEOUT_CONTROL);
	}

	if (!data)
		return;

	/* Sanity checks */
	BUG_ON(data->blksz * data->blocks > host->mmc->max_req_size);
	BUG_ON(data->blksz > host->mmc->max_blk_size);
	BUG_ON(data->blocks > 65535);

	host->data = data;
	host->data_early = 0;
	host->data->bytes_xfered = 0;

	if (host->flags & (SDHCI_USE_SDMA | SDHCI_USE_ADMA))
		host->flags |= SDHCI_REQ_USE_DMA;

	if ((host->quirks2 & SDHCI_QUIRK2_USE_PIO_FOR_EMMC_TUNING) &&
		(cmd->opcode ==  MMC_SEND_TUNING_BLOCK_HS200 ||
		cmd->opcode == MMC_SEND_TUNING_BLOCK_HS400))
		host->flags &= ~SDHCI_REQ_USE_DMA;

	/*
	 * FIXME: This doesn't account for merging when mapping the
	 * scatterlist.
	 */
	if (host->flags & SDHCI_REQ_USE_DMA) {
		int broken, i;
		struct scatterlist *sg;

		broken = 0;
		if (host->flags & SDHCI_USE_ADMA) {
			if (host->quirks & SDHCI_QUIRK_32BIT_ADMA_SIZE)
				broken = 1;
		} else {
			if (host->quirks & SDHCI_QUIRK_32BIT_DMA_SIZE)
				broken = 1;
		}

		if (unlikely(broken)) {
			for_each_sg(data->sg, sg, data->sg_len, i) {
				if (sg->length & 0x3) {
					DBG("Reverting to PIO because of "
						"transfer size (%d)\n",
						sg->length);
					host->flags &= ~SDHCI_REQ_USE_DMA;
					break;
				}
			}
		}
	}

	/*
	 * The assumption here being that alignment is the same after
	 * translation to device address space.
	 */
	if (host->flags & SDHCI_REQ_USE_DMA) {
		int broken, i;
		struct scatterlist *sg;

		broken = 0;
		if (host->flags & SDHCI_USE_ADMA) {
			/*
			 * As we use 3 byte chunks to work around
			 * alignment problems, we need to check this
			 * quirk.
			 */
			if (host->quirks & SDHCI_QUIRK_32BIT_ADMA_SIZE)
				broken = 1;
		} else {
			if (host->quirks & SDHCI_QUIRK_32BIT_DMA_ADDR)
				broken = 1;
		}

		if (unlikely(broken)) {
			for_each_sg(data->sg, sg, data->sg_len, i) {
				if (sg->offset & 0x3) {
					DBG("Reverting to PIO because of "
						"bad alignment\n");
					host->flags &= ~SDHCI_REQ_USE_DMA;
					break;
				}
			}
		}
	}

	if (host->flags & SDHCI_REQ_USE_DMA) {
		if (host->flags & SDHCI_USE_ADMA) {
			trace_mmc_adma_table_pre(cmd->opcode, data->sg_len);
			ret = sdhci_adma_table_pre(host, data);
			if (ret) {
				/*
				 * This only happens when someone fed
				 * us an invalid request.
				 */
				WARN_ON(1);
				host->flags &= ~SDHCI_REQ_USE_DMA;
			} else {
				if (host->flags & SDHCI_USE_ADMA_64BIT) {
					sdhci_writel(host,
						(u32)host->adma_addr,
						SDHCI_ADMA_ADDRESS_LOW);
					sdhci_writel(host,
					  (u32)((u64)host->adma_addr
						  >> SDHCI_HI_SHIFT),
					  SDHCI_ADMA_ADDRESS_HIGH);
				} else {
					sdhci_writel(host, host->adma_addr,
						SDHCI_ADMA_ADDRESS_LOW);
				}
			}
		} else {
			int sg_cnt;

			sg_cnt = sdhci_pre_dma_transfer(host, data, NULL);
			if (sg_cnt == 0) {
				/*
				 * This only happens when someone fed
				 * us an invalid request.
				 */
				WARN_ON(1);
				host->flags &= ~SDHCI_REQ_USE_DMA;
			} else {
				WARN_ON(sg_cnt != 1);
				sdhci_writel(host, sg_dma_address(data->sg),
					SDHCI_DMA_ADDRESS);
			}
		}
	}

	/*
	 * Always adjust the DMA selection as some controllers
	 * (e.g. JMicron) can't do PIO properly when the selection
	 * is ADMA.
	 */
	if (host->version >= SDHCI_SPEC_200) {
		ctrl = sdhci_readb(host, SDHCI_HOST_CONTROL);
		ctrl &= ~SDHCI_CTRL_DMA_MASK;
		if ((host->flags & SDHCI_REQ_USE_DMA) &&
			(host->flags & SDHCI_USE_ADMA)) {
			if (host->flags & SDHCI_USE_ADMA_64BIT)
				ctrl |= SDHCI_CTRL_ADMA64;
			else
				ctrl |= SDHCI_CTRL_ADMA32;
		} else {
			ctrl |= SDHCI_CTRL_SDMA;
		}
		sdhci_writeb(host, ctrl, SDHCI_HOST_CONTROL);
	}

	if (!(host->flags & SDHCI_REQ_USE_DMA)) {
		int flags;

		flags = SG_MITER_ATOMIC;
		if (host->data->flags & MMC_DATA_READ)
			flags |= SG_MITER_TO_SG;
		else
			flags |= SG_MITER_FROM_SG;
		sg_miter_start(&host->sg_miter, data->sg, data->sg_len, flags);
		host->blocks = data->blocks;
	}

	sdhci_set_transfer_irqs(host);

	/* Set the DMA boundary value and block size */
	sdhci_set_blk_size_reg(host, data->blksz, SDHCI_DEFAULT_BOUNDARY_ARG);
	sdhci_writew(host, data->blocks, SDHCI_BLOCK_COUNT);
}

static void sdhci_set_transfer_mode(struct sdhci_host *host,
	struct mmc_command *cmd)
{
	u16 mode;
	struct mmc_data *data = cmd->data;

	if (data == NULL)
		return;

	WARN_ON(!host->data);

	mode = SDHCI_TRNS_BLK_CNT_EN;
	if (mmc_op_multi(cmd->opcode) || data->blocks > 1) {
		mode |= SDHCI_TRNS_MULTI;
		/*
		 * If we are sending CMD23, CMD12 never gets sent
		 * on successful completion (so no Auto-CMD12).
		 */
		if (!host->mrq->sbc && (host->flags & SDHCI_AUTO_CMD12))
			mode |= SDHCI_TRNS_AUTO_CMD12;
		else if (host->mrq->sbc && (host->flags & SDHCI_AUTO_CMD23)) {
			mode |= SDHCI_TRNS_AUTO_CMD23;
			sdhci_writel(host, host->mrq->sbc->arg, SDHCI_ARGUMENT2);
		}
	}

	if (data->flags & MMC_DATA_READ) {
		mode |= SDHCI_TRNS_READ;
		if (host->ops->toggle_cdr) {
			if ((cmd->opcode == MMC_SEND_TUNING_BLOCK_HS200) ||
				(cmd->opcode == MMC_SEND_TUNING_BLOCK_HS400) ||
				(cmd->opcode == MMC_SEND_TUNING_BLOCK))
				host->ops->toggle_cdr(host, false);
			else
				host->ops->toggle_cdr(host, true);
		}
	}
	if (host->ops->toggle_cdr && (data->flags & MMC_DATA_WRITE))
		host->ops->toggle_cdr(host, false);
	if (host->flags & SDHCI_REQ_USE_DMA)
		mode |= SDHCI_TRNS_DMA;

	sdhci_writew(host, mode, SDHCI_TRANSFER_MODE);
}

static void sdhci_finish_data(struct sdhci_host *host)
{
	struct mmc_data *data;

	BUG_ON(!host->data);

	data = host->data;
	host->data = NULL;

	if (host->flags & SDHCI_REQ_USE_DMA) {
		if (host->flags & SDHCI_USE_ADMA)
			sdhci_adma_table_post(host, data);
		else {
			if (!data->host_cookie)
				dma_unmap_sg(mmc_dev(host->mmc), data->sg,
					     data->sg_len,
					     (data->flags & MMC_DATA_READ) ?
					     DMA_FROM_DEVICE : DMA_TO_DEVICE);
		}
	}

	/*
	 * The specification states that the block count register must
	 * be updated, but it does not specify at what point in the
	 * data flow. That makes the register entirely useless to read
	 * back so we have to assume that nothing made it to the card
	 * in the event of an error.
	 */
	if (data->error)
		data->bytes_xfered = 0;
	else
		data->bytes_xfered = data->blksz * data->blocks;

	/*
	 * Need to send CMD12 if -
	 * a) open-ended multiblock transfer (no CMD23)
	 * b) error in multiblock transfer
	 */
	if (data->stop &&
	    (data->error ||
	     !host->mrq->sbc)) {

		/*
		 * The controller needs a reset of internal state machines
		 * upon error conditions.
		 */
		if (data->error) {
			sdhci_reset(host, SDHCI_RESET_CMD);
			sdhci_reset(host, SDHCI_RESET_DATA);
		}

		sdhci_send_command(host, data->stop);
	} else
		tasklet_schedule(&host->finish_tasklet);
}

#define SDHCI_REQUEST_TIMEOUT	10 /* Default request timeout in seconds */

static void sdhci_send_command(struct sdhci_host *host, struct mmc_command *cmd)
{
	int flags;
	u32 mask;
	unsigned long timeout;

	WARN_ON(host->cmd);

	/* Wait max 10 ms */
	timeout = 10000;

	mask = SDHCI_CMD_INHIBIT;
	if ((cmd->data != NULL) || (cmd->flags & MMC_RSP_BUSY))
		mask |= SDHCI_DATA_INHIBIT;

	/* We shouldn't wait for data inihibit for stop commands, even
	   though they might use busy signaling */
	if (host->mrq->data && (cmd == host->mrq->data->stop))
		mask &= ~SDHCI_DATA_INHIBIT;

	while (sdhci_readl(host, SDHCI_PRESENT_STATE) & mask) {
		if (timeout == 0) {
			pr_err("%s: Controller never released "
				"inhibit bit(s).\n", mmc_hostname(host->mmc));
			sdhci_dumpregs(host);
			cmd->error = -EIO;
			tasklet_schedule(&host->finish_tasklet);
			return;
		}
		timeout--;
		udelay(1);
	}

	mod_timer(&host->timer, jiffies + SDHCI_REQUEST_TIMEOUT * HZ);

	if (cmd->cmd_timeout_ms > SDHCI_REQUEST_TIMEOUT * MSEC_PER_SEC)
		mod_timer(&host->timer, jiffies +
				(msecs_to_jiffies(cmd->cmd_timeout_ms * 2)));

	host->cmd = cmd;

	sdhci_prepare_data(host, cmd);

	sdhci_writel(host, cmd->arg, SDHCI_ARGUMENT);

	sdhci_set_transfer_mode(host, cmd);

	if ((cmd->flags & MMC_RSP_136) && (cmd->flags & MMC_RSP_BUSY)) {
		pr_err("%s: Unsupported response type!\n",
			mmc_hostname(host->mmc));
		cmd->error = -EINVAL;
		tasklet_schedule(&host->finish_tasklet);
		return;
	}

	if (!(cmd->flags & MMC_RSP_PRESENT))
		flags = SDHCI_CMD_RESP_NONE;
	else if (cmd->flags & MMC_RSP_136)
		flags = SDHCI_CMD_RESP_LONG;
	else if (cmd->flags & MMC_RSP_BUSY)
		flags = SDHCI_CMD_RESP_SHORT_BUSY;
	else
		flags = SDHCI_CMD_RESP_SHORT;

	if (cmd->flags & MMC_RSP_CRC)
		flags |= SDHCI_CMD_CRC;
	if (cmd->flags & MMC_RSP_OPCODE)
		flags |= SDHCI_CMD_INDEX;

	/* CMD19 is special in that the Data Present Select should be set */
	if (cmd->data || cmd->opcode == MMC_SEND_TUNING_BLOCK ||
	    cmd->opcode == MMC_SEND_TUNING_BLOCK_HS400 ||
	    cmd->opcode == MMC_SEND_TUNING_BLOCK_HS200)
		flags |= SDHCI_CMD_DATA;

	if (cmd->data)
		host->data_start_time = ktime_get();
	trace_mmc_cmd_rw_start(cmd->opcode, cmd->arg, cmd->flags);
	sdhci_writew(host, SDHCI_MAKE_CMD(cmd->opcode, flags), SDHCI_COMMAND);
}

static void sdhci_finish_command(struct sdhci_host *host)
{
	int i;

	BUG_ON(host->cmd == NULL);

	if (host->cmd->flags & MMC_RSP_PRESENT) {
		if (host->cmd->flags & MMC_RSP_136) {
			/* CRC is stripped so we need to do some shifting. */
			for (i = 0;i < 4;i++) {
				host->cmd->resp[i] = sdhci_readl(host,
					SDHCI_RESPONSE + (3-i)*4) << 8;
				if (i != 3)
					host->cmd->resp[i] |=
						sdhci_readb(host,
						SDHCI_RESPONSE + (3-i)*4-1);
			}
		} else {
			host->cmd->resp[0] = sdhci_readl(host, SDHCI_RESPONSE);
		}
	}

	/* Finished CMD23, now send actual command. */
	if (host->cmd == host->mrq->sbc) {
		host->cmd->error = 0;
		host->cmd = NULL;
		sdhci_send_command(host, host->mrq->cmd);
	} else {

		/* Processed actual command. */
		if (host->data && host->data_early)
			sdhci_finish_data(host);

		if (!host->cmd->data)
			tasklet_schedule(&host->finish_tasklet);

		host->cmd = NULL;
	}
}

static u16 sdhci_get_preset_value(struct sdhci_host *host)
{
	u16 ctrl, preset = 0;

	ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);

	switch (ctrl & SDHCI_CTRL_UHS_MASK) {
	case SDHCI_CTRL_UHS_SDR12:
		preset = sdhci_readw(host, SDHCI_PRESET_FOR_SDR12);
		break;
	case SDHCI_CTRL_UHS_SDR25:
		preset = sdhci_readw(host, SDHCI_PRESET_FOR_SDR25);
		break;
	case SDHCI_CTRL_UHS_SDR50:
		preset = sdhci_readw(host, SDHCI_PRESET_FOR_SDR50);
		break;
	case SDHCI_CTRL_UHS_SDR104:
		preset = sdhci_readw(host, SDHCI_PRESET_FOR_SDR104);
		break;
	case SDHCI_CTRL_UHS_DDR50:
		preset = sdhci_readw(host, SDHCI_PRESET_FOR_DDR50);
		break;
	default:
		pr_warn("%s: Invalid UHS-I mode selected\n",
			mmc_hostname(host->mmc));
		preset = sdhci_readw(host, SDHCI_PRESET_FOR_SDR12);
		break;
	}
	return preset;
}

static void sdhci_set_clock(struct sdhci_host *host, unsigned int clock)
{
	int div = 0; /* Initialized for compiler warning */
	int real_div = div, clk_mul = 1;
	u16 clk = 0;
	unsigned long timeout;
	unsigned long flags;

	spin_lock_irqsave(&host->lock, flags);
	if (clock && clock == host->clock)
		goto ret;

	host->mmc->actual_clock = 0;

	if (host->ops->set_clock) {
		spin_unlock_irqrestore(&host->lock, flags);
		host->ops->set_clock(host, clock);
		spin_lock_irqsave(&host->lock, flags);
		if (host->quirks & SDHCI_QUIRK_NONSTANDARD_CLOCK)
			goto ret;
	}

	if (host->clock)
		sdhci_writew(host, 0, SDHCI_CLOCK_CONTROL);

	if (clock == 0)
		goto out;

	if (host->version >= SDHCI_SPEC_300) {
		if (sdhci_readw(host, SDHCI_HOST_CONTROL2) &
			SDHCI_CTRL_PRESET_VAL_ENABLE) {
			u16 pre_val;

			clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
			pre_val = sdhci_get_preset_value(host);
			div = (pre_val & SDHCI_PRESET_SDCLK_FREQ_MASK)
				>> SDHCI_PRESET_SDCLK_FREQ_SHIFT;
			if (host->clk_mul &&
				(pre_val & SDHCI_PRESET_CLKGEN_SEL_MASK)) {
				clk = SDHCI_PROG_CLOCK_MODE;
				real_div = div + 1;
				clk_mul = host->clk_mul;
			} else {
				real_div = max_t(int, 1, div << 1);
			}
			goto clock_set;
		}

		/*
		 * Check if the Host Controller supports Programmable Clock
		 * Mode.
		 */
		if (host->clk_mul) {
			for (div = 1; div <= 1024; div++) {
				if ((host->max_clk * host->clk_mul / div)
					<= clock)
					break;
			}
			/*
			 * Set Programmable Clock Mode in the Clock
			 * Control register.
			 */
			clk = SDHCI_PROG_CLOCK_MODE;
			real_div = div;
			clk_mul = host->clk_mul;
			div--;
		} else {
			/* Version 3.00 divisors must be a multiple of 2. */
			if (host->max_clk <= clock)
				div = 1;
			else {
				for (div = 2; div < SDHCI_MAX_DIV_SPEC_300;
				     div += 2) {
					if ((host->max_clk / div) <= clock)
						break;
				}
			}
			real_div = div;
			div >>= 1;
		}
	} else {
		/* Version 2.00 divisors must be a power of 2. */
		for (div = 1; div < SDHCI_MAX_DIV_SPEC_200; div *= 2) {
			if ((host->max_clk / div) <= clock)
				break;
		}
		real_div = div;
		div >>= 1;
	}

clock_set:
	if (real_div)
		host->mmc->actual_clock = (host->max_clk * clk_mul) / real_div;

	if (host->quirks2 & SDHCI_QUIRK2_ALWAYS_USE_BASE_CLOCK)
		div = 0;

	clk |= (div & SDHCI_DIV_MASK) << SDHCI_DIVIDER_SHIFT;
	clk |= ((div & SDHCI_DIV_HI_MASK) >> SDHCI_DIV_MASK_LEN)
		<< SDHCI_DIVIDER_HI_SHIFT;
	clk |= SDHCI_CLOCK_INT_EN;
	sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);

	/* Wait max 20 ms */
	timeout = 20000;
	while (!((clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL))
		& SDHCI_CLOCK_INT_STABLE)) {
		if (timeout == 0) {
			pr_err("%s: Internal clock never "
				"stabilised.\n", mmc_hostname(host->mmc));
			sdhci_dumpregs(host);
			goto ret;
		}
		timeout--;
		spin_unlock_irq(&host->lock);
		usleep_range(900, 1100);
		spin_lock_irq(&host->lock);
	}

	clk |= SDHCI_CLOCK_CARD_EN;
	sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);

out:
	host->clock = clock;
ret:
	spin_unlock_irqrestore(&host->lock, flags);
}

static inline unsigned long sdhci_update_clock(struct sdhci_host *host,
					       unsigned long flags)
{
	unsigned int clock;

	clock = host->clock;
	host->clock = 0;
	spin_unlock_irqrestore(&host->lock, flags);
	sdhci_set_clock(host, clock);
	spin_lock_irqsave(&host->lock, flags);
	return flags;
}

static int sdhci_set_power(struct sdhci_host *host, unsigned short power)
{
	u8 pwr = 0;

	if (power != (unsigned short)-1) {
		switch (1 << power) {
		case MMC_VDD_165_195:
			pwr = SDHCI_POWER_180;
			break;
		case MMC_VDD_29_30:
		case MMC_VDD_30_31:
			pwr = SDHCI_POWER_300;
			break;
		case MMC_VDD_32_33:
		case MMC_VDD_33_34:
			pwr = SDHCI_POWER_330;
			break;
		default:
			BUG();
		}
	}

	if (host->pwr == pwr)
		return -1;

	host->pwr = pwr;

	if (pwr == 0) {
		sdhci_writeb(host, 0, SDHCI_POWER_CONTROL);
		if (host->ops->check_power_status)
			host->ops->check_power_status(host, REQ_BUS_OFF);
		return 0;
	}

	/*
	 * Spec says that we should clear the power reg before setting
	 * a new value. Some controllers don't seem to like this though.
	 */
	if (!(host->quirks & SDHCI_QUIRK_SINGLE_POWER_WRITE)) {
		sdhci_writeb(host, 0, SDHCI_POWER_CONTROL);
		if (host->ops->check_power_status)
			host->ops->check_power_status(host, REQ_BUS_OFF);
	}

	/*
	 * At least the Marvell CaFe chip gets confused if we set the voltage
	 * and set turn on power at the same time, so set the voltage first.
	 */
	if (host->quirks & SDHCI_QUIRK_NO_SIMULT_VDD_AND_POWER) {
		sdhci_writeb(host, pwr, SDHCI_POWER_CONTROL);
		if (host->ops->check_power_status)
			host->ops->check_power_status(host, REQ_BUS_ON);
	}

	pwr |= SDHCI_POWER_ON;

	sdhci_writeb(host, pwr, SDHCI_POWER_CONTROL);
	if (host->ops->check_power_status)
		host->ops->check_power_status(host, REQ_BUS_ON);

	/*
	 * Some controllers need an extra 10ms delay of 10ms before they
	 * can apply clock after applying power
	 */
	if (host->quirks & SDHCI_QUIRK_DELAY_AFTER_POWER)
		mdelay(10);

	return power;
}

static void sdhci_pm_qos_remove_work(struct work_struct *work)
{
	struct sdhci_host *host = container_of(work, struct sdhci_host,
						pm_qos_work.work);
	struct sdhci_host_qos *host_qos = host->host_qos;
	int vote;

	mutex_lock(&host->qos_lock);
	if (unlikely(host->last_qos_policy == -EINVAL)) {
		goto out;
	}
	vote = host->last_qos_policy;

	if (unlikely(!host_qos[vote].cpu_dma_latency_us))
		goto out;

	pm_qos_update_request(&(host_qos[vote].pm_qos_req_dma),
				PM_QOS_DEFAULT_VALUE);

	host->last_qos_policy = -EINVAL;
out:
	mutex_unlock(&host->qos_lock);
}

static inline int sdhci_get_host_qos_index(struct mmc_host *mmc,
			struct mmc_request *mrq)
{
	struct sdhci_host *host = mmc_priv(mmc);
	int vote = -1;

	if (host->host_use_default_qos)
		return SDHCI_QOS_READ_WRITE;

	if (mrq && mrq->cmd) {
		switch (mrq->cmd->opcode) {
		case MMC_READ_SINGLE_BLOCK:
		case MMC_READ_MULTIPLE_BLOCK:
			vote = SDHCI_QOS_READ;
			break;
		default:
			vote = SDHCI_QOS_WRITE;
			break;
		}
	} else if (!mrq) {
		vote = SDHCI_QOS_READ_WRITE;
	}

	return vote;
}
static void __sdhci_update_pm_qos(struct mmc_host *mmc, struct mmc_request *mrq)
{
	struct sdhci_host *host = mmc_priv(mmc);
	struct sdhci_host_qos *host_qos = host->host_qos;
	u32 pol_index = 0;
	int vote;

	mutex_lock(&host->qos_lock);
	vote = sdhci_get_host_qos_index(mmc, mrq);

	if (unlikely(vote == -1)) {
		WARN("%s: invalid SDHCI vote type (%d)\n",
				mmc_hostname(mmc), vote);
		goto out;
	}

	if (unlikely(!host_qos[vote].cpu_dma_latency_us))
		goto out;

	/*
	 * check if we support dynamic qos
	 * otherwise pol_index is 0
	 */
	if (host_qos[vote].cpu_dma_latency_tbl_sz > 1)
		pol_index = host->power_policy;

	if ((host->last_qos_policy != -EINVAL) &&
		(host->last_qos_policy != vote)) {
		pm_qos_update_request(
			&(host_qos[host->last_qos_policy].pm_qos_req_dma),
			PM_QOS_DEFAULT_VALUE);
		}
	pm_qos_update_request(&(host_qos[vote].pm_qos_req_dma),
		host_qos[vote].cpu_dma_latency_us[pol_index]);
	host->last_qos_policy = vote;
out:
	mutex_unlock(&host->qos_lock);
	return;
}

static void sdhci_update_pm_qos(struct mmc_host *mmc,
		struct mmc_request *mrq, bool enable)
{
	struct sdhci_host *host = mmc_priv(mmc);
	int delay;

	if (enable) {
		cancel_delayed_work_sync(&host->pm_qos_work);
		__sdhci_update_pm_qos(mmc, mrq);
	} else if (!enable) {
		/*
		 * In performance mode, release QoS vote after a higher timeout
		 * to make sure back-to-back requests don't suffer from
		 * latencies that are involved to wake CPU from low power modes
		 * in cases where the CPU may go into low power mode as soon as
		 * QoS vote is released.
		 */

		if (host->power_policy == SDHCI_POWER_SAVE_MODE)
			delay = host->pm_qos_timeout_us / 2;
		else
			delay = 2 * host->pm_qos_timeout_us;
		schedule_delayed_work(&host->pm_qos_work,
				msecs_to_jiffies(delay));
	}

	return;
}


/*****************************************************************************\
 *                                                                           *
 * MMC callbacks                                                             *
 *                                                                           *
\*****************************************************************************/

static int sdhci_enable(struct mmc_host *mmc)
{
	struct sdhci_host *host = mmc_priv(mmc);

	if (host->ops->platform_bus_voting)
		host->ops->platform_bus_voting(host, 1);

	return 0;
}

static int sdhci_disable(struct mmc_host *mmc)
{
	struct sdhci_host *host = mmc_priv(mmc);

	if (host->ops->platform_bus_voting)
		host->ops->platform_bus_voting(host, 0);

	return 0;
}

static inline void sdhci_update_power_policy(struct sdhci_host *host,
		enum sdhci_power_policy policy)
{
	host->power_policy = policy;
}

static void sdhci_notify_pm_status(struct mmc_host *mmc, enum dev_state state)
{
	struct sdhci_host *host = mmc_priv(mmc);

	if (host->ops->notify_pm_status)
		host->ops->notify_pm_status(host, state);
}

static int sdhci_notify_load(struct mmc_host *mmc, enum mmc_load state)
{
	int err = 0;
	struct sdhci_host *host = mmc_priv(mmc);

	switch (state) {
	case MMC_LOAD_HIGH:
		sdhci_update_power_policy(host, SDHCI_PERFORMANCE_MODE);
		break;
	case MMC_LOAD_INIT:
		sdhci_update_power_policy(host, SDHCI_PERFORMANCE_MODE_INIT);
		break;
	case MMC_LOAD_LOW:
		sdhci_update_power_policy(host, SDHCI_POWER_SAVE_MODE);
		break;
	default:
		err = -EINVAL;
		break;
	}

	if (host->ops->notify_load)
		err = host->ops->notify_load(host, state);

	return err;
}

static void sdhci_pre_req(struct mmc_host *mmc, struct mmc_request *mrq,
			  bool is_first_req)
{
	struct sdhci_host *host = mmc_priv(mmc);

	if (mrq->data->host_cookie) {
		mrq->data->host_cookie = 0;
		return;
	}

	if (host->flags & SDHCI_REQ_USE_DMA)
		if (sdhci_pre_dma_transfer(host, mrq->data, &host->next_data) < 0)
			mrq->data->host_cookie = 0;
}

static void sdhci_post_req(struct mmc_host *mmc, struct mmc_request *mrq,
			   int err)
{
	struct sdhci_host *host = mmc_priv(mmc);
	struct mmc_data *data = mrq->data;

	if (host->flags & SDHCI_REQ_USE_DMA) {
		dma_unmap_sg(mmc_dev(host->mmc), data->sg, data->sg_len,
			     (data->flags & MMC_DATA_WRITE) ?
			     DMA_TO_DEVICE : DMA_FROM_DEVICE);
		data->host_cookie = 0;
	}
}

static bool sdhci_check_state(struct sdhci_host *host)
{
	struct mmc_host *mmc = host->mmc;

	if (!host->clock || !host->pwr ||
	    (mmc_use_core_runtime_pm(mmc) ?
	     pm_runtime_suspended(mmc->parent) : 0))
		return true;
	else
		return false;
}

static bool sdhci_check_auto_tuning(struct sdhci_host *host,
				  struct mmc_command *cmd)
{
	if (((cmd->opcode != MMC_READ_SINGLE_BLOCK) &&
	     (cmd->opcode != MMC_READ_MULTIPLE_BLOCK) &&
	     (cmd->opcode != SD_IO_RW_EXTENDED)) || (host->clock < 100000000))
		return false;
	else if (host->mmc->ios.timing == MMC_TIMING_MMC_HS200 ||
		 host->mmc->ios.timing == MMC_TIMING_UHS_SDR104)
		return true;
	else
		return false;
}

static int sdhci_get_tuning_cmd(struct sdhci_host *host)
{
	if (!host->mmc || !host->mmc->card)
		return 0;
	/*
	 * If we are here, all conditions have already been true
	 * and the card can either be an eMMC or SD/SDIO
	 */
	if (mmc_card_mmc(host->mmc->card))
		return MMC_SEND_TUNING_BLOCK_HS200;
	else
		return MMC_SEND_TUNING_BLOCK;
}

void sdhci_unvote_all_pm_qos(struct sdhci_host *host)
{
	struct sdhci_host_qos *host_qos = host->host_qos;

	cancel_delayed_work_sync(&host->pm_qos_work);

	/*
	 * This explicitly unvote all pm_qos request from sdhci driver.
	 * This call can be used by LLD before going to suspend to
	 * make sure that no qos vote has been held up after
	 * driver suspend.
	 */
	mutex_lock(&host->qos_lock);
	if (host_qos[SDHCI_QOS_READ_WRITE].cpu_dma_latency_us) {
		if (host->host_use_default_qos ||
				(host->mmc->caps2 &  MMC_CAP2_CMD_QUEUE))
			pm_qos_update_request(
			&(host_qos[SDHCI_QOS_READ_WRITE].pm_qos_req_dma),
				PM_QOS_DEFAULT_VALUE);

		if (!host->host_use_default_qos) {
			pm_qos_update_request(
				&(host_qos[SDHCI_QOS_READ].pm_qos_req_dma),
				PM_QOS_DEFAULT_VALUE);
			pm_qos_update_request(
				&(host_qos[SDHCI_QOS_WRITE].pm_qos_req_dma),
				PM_QOS_DEFAULT_VALUE);
		}
	}
	mutex_unlock(&host->qos_lock);
	pr_debug("%s: %s: unvote ===all pm_qos=== votes\n",
			mmc_hostname(host->mmc), __func__);
	return;
}
EXPORT_SYMBOL(sdhci_unvote_all_pm_qos);

void sdhci_cfg_irq(struct sdhci_host *host, bool enable, bool sync)
{
	if (enable && !host->irq_enabled) {
		enable_irq(host->irq);
		host->irq_enabled = true;
	} else if (!enable && host->irq_enabled) {
		if (sync)
			disable_irq(host->irq);
		else
			disable_irq_nosync(host->irq);
		host->irq_enabled = false;
	}
}
EXPORT_SYMBOL(sdhci_cfg_irq);

static int sdhci_crypto_cfg(struct sdhci_host *host, struct mmc_request *mrq,
		u32 slot)
{
	int err = 0;

	if (host->crypto_reset_reqd && host->ops->crypto_engine_reset) {
		err = host->ops->crypto_engine_reset(host);
		if (err) {
			pr_err("%s: crypto reset failed\n",
					mmc_hostname(host->mmc));
			goto out;
		}
		host->crypto_reset_reqd = false;
	}

	if (host->ops->crypto_engine_cfg) {
		err = host->ops->crypto_engine_cfg(host, mrq, slot);
		if (err) {
			pr_err("%s: failed to configure crypto\n",
					mmc_hostname(host->mmc));
			goto out;
		}
	}
out:
	return err;
}

static void sdhci_request(struct mmc_host *mmc, struct mmc_request *mrq)
{
	struct sdhci_host *host;
	int present;
	unsigned long flags;
	u32 tuning_opcode;

	host = mmc_priv(mmc);

	sdhci_runtime_pm_get(host);
	sdhci_update_pm_qos(mmc, mrq, true);
	if (sdhci_check_state(host)) {
		sdhci_dump_state(host);
		WARN(1, "sdhci in bad state");
		mrq->cmd->error = -EIO;
		if (mrq->data)
			mrq->data->error = -EIO;
		mmc_request_done(host->mmc, mrq);
		sdhci_runtime_pm_put(host);
		return;
	}

	/*
	 * Firstly check card presence from cd-gpio.  The return could
	 * be one of the following possibilities:
	 *     negative: cd-gpio is not available
	 *     zero: cd-gpio is used, and card is removed
	 *     one: cd-gpio is used, and card is present
	 */
	present = mmc_gpio_get_cd(host->mmc);
	if (present < 0) {
		/* If polling, assume that the card is always present. */
		if (host->quirks & SDHCI_QUIRK_BROKEN_CARD_DETECTION)
			present = 1;
		else
			present = sdhci_readl(host, SDHCI_PRESENT_STATE) &
					SDHCI_CARD_PRESENT;
	}

	spin_lock_irqsave(&host->lock, flags);

	WARN_ON(host->mrq != NULL);

#ifndef SDHCI_USE_LEDS_CLASS
	if (!(host->quirks2 & SDHCI_QUIRK2_BROKEN_LED_CONTROL))
		sdhci_activate_led(host);
#endif

	/*
	 * Ensure we don't send the STOP for non-SET_BLOCK_COUNTED
	 * requests if Auto-CMD12 is enabled.
	 */
	if (!mrq->sbc && (host->flags & SDHCI_AUTO_CMD12)) {
		if (mrq->stop) {
			mrq->data->stop = NULL;
			mrq->stop = NULL;
		}
	}

	host->mrq = mrq;

	if (!present || host->flags & SDHCI_DEVICE_DEAD) {
		host->mrq->cmd->error = -ENOMEDIUM;
		tasklet_schedule(&host->finish_tasklet);
	} else {
		u32 present_state;

		present_state = sdhci_readl(host, SDHCI_PRESENT_STATE);
		/*
		 * Check if the re-tuning timer has already expired and there
		 * is no on-going data transfer. If so, we need to execute
		 * tuning procedure before sending command.
		 */
		if ((mrq->cmd->opcode != MMC_SEND_TUNING_BLOCK) &&
		    (mrq->cmd->opcode != MMC_SEND_TUNING_BLOCK_HS400) &&
		    (mrq->cmd->opcode != MMC_SEND_TUNING_BLOCK_HS200) &&
		    (host->flags & SDHCI_NEEDS_RETUNING) &&
		    !(present_state & (SDHCI_DOING_WRITE | SDHCI_DOING_READ))) {
			if (mmc->card) {
				/* eMMC uses cmd21 but sd and sdio use cmd19 */
				tuning_opcode =
					mmc->card->type == MMC_TYPE_MMC ?
					MMC_SEND_TUNING_BLOCK_HS200 :
					MMC_SEND_TUNING_BLOCK;
				host->mrq = NULL;
				host->flags &= ~SDHCI_NEEDS_RETUNING;
				spin_unlock_irqrestore(&host->lock, flags);
				sdhci_execute_tuning(mmc, tuning_opcode);
				spin_lock_irqsave(&host->lock, flags);

				/* Restore original mmc_request structure */
				host->mrq = mrq;
			}
		}

		if (host->ops->config_auto_tuning_cmd) {
			if (sdhci_check_auto_tuning(host, mrq->cmd))
				host->ops->config_auto_tuning_cmd(host, true,
					sdhci_get_tuning_cmd(host));
			else
				host->ops->config_auto_tuning_cmd(host, false,
					sdhci_get_tuning_cmd(host));
		}

		if (host->is_crypto_en) {
			spin_unlock_irqrestore(&host->lock, flags);
			if (sdhci_crypto_cfg(host, mrq, 0))
				goto end_req;
			spin_lock_irqsave(&host->lock, flags);
		}

		if (mrq->sbc && !(host->flags & SDHCI_AUTO_CMD23))
			sdhci_send_command(host, mrq->sbc);
		else
			sdhci_send_command(host, mrq->cmd);
	}

	mmiowb();
	spin_unlock_irqrestore(&host->lock, flags);
	return;
end_req:
	mrq->cmd->error = -EIO;
	if (mrq->data)
		mrq->data->error = -EIO;
	mmc_request_done(host->mmc, mrq);
	sdhci_runtime_pm_put(host);
}

static void sdhci_cfg_async_intr(struct sdhci_host *host, bool enable)
{
	if (!host->async_int_supp)
		return;

	if (enable)
		sdhci_writew(host,
			     sdhci_readw(host, SDHCI_HOST_CONTROL2) |
			     SDHCI_CTRL_ASYNC_INT_ENABLE,
			     SDHCI_HOST_CONTROL2);
	else
		sdhci_writew(host, sdhci_readw(host, SDHCI_HOST_CONTROL2) &
			     ~SDHCI_CTRL_ASYNC_INT_ENABLE,
			     SDHCI_HOST_CONTROL2);
}

static void sdhci_do_set_ios(struct sdhci_host *host, struct mmc_ios *ios)
{
	unsigned long flags;
	int vdd_bit = -1;
	u8 ctrl;
	int ret;

	mutex_lock(&host->ios_mutex);
	if (host->flags & SDHCI_DEVICE_DEAD) {
		if (host->vmmc && ios->power_mode == MMC_POWER_OFF)
			mmc_regulator_set_ocr(host->mmc, host->vmmc, 0);
		mutex_unlock(&host->ios_mutex);
		return;
	}

	spin_lock_irqsave(&host->lock, flags);
	/* lock is being released intermittently below, hence disable irq */
	sdhci_cfg_irq(host, false, false);
	spin_unlock_irqrestore(&host->lock, flags);
	if (ios->clock) {
		sdhci_set_clock(host, ios->clock);
		if (host->async_int_supp && sdhci_get_async_int_status(host)) {
			if (host->disable_sdio_irq_deferred) {
				pr_debug("%s: %s: disable sdio irq\n",
					 mmc_hostname(host->mmc), __func__);
				host->mmc->ops->enable_sdio_irq(host->mmc, 0);
				host->disable_sdio_irq_deferred = false;
			}
			spin_lock_irqsave(&host->lock, flags);
			sdhci_cfg_async_intr(host, false);
			spin_unlock_irqrestore(&host->lock, flags);
			pr_debug("%s: %s: unconfig async intr\n",
				 mmc_hostname(host->mmc), __func__);
		}
	}
	/*
	 * The controller clocks may be off during power-up and we may end up
	 * enabling card clock before giving power to the card. Hence, during
	 * MMC_POWER_UP enable the controller clock and turn-on the regulators.
	 * The mmc_power_up would provide the necessary delay before turning on
	 * the clocks to the card.
	 */
	if (ios->power_mode & MMC_POWER_UP) {
		if (host->ops->enable_controller_clock) {
			ret = host->ops->enable_controller_clock(host);
			if (ret) {
				pr_err("%s: enabling controller clock: failed: %d\n",
				       mmc_hostname(host->mmc), ret);
			} else {
				vdd_bit = sdhci_set_power(host, ios->vdd);

				if (host->vmmc && vdd_bit != -1)
					mmc_regulator_set_ocr(host->mmc,
							      host->vmmc,
							      vdd_bit);
			}
		}
		/*
		 * make sure interrupts are enabled, these are the same
		 * interrupts which get enabled in sdhci_init().
		 */
		sdhci_clear_set_irqs(host, 0,
			SDHCI_INT_BUS_POWER | SDHCI_INT_DATA_END_BIT |
			SDHCI_INT_DATA_CRC | SDHCI_INT_DATA_TIMEOUT |
			SDHCI_INT_INDEX |
			SDHCI_INT_END_BIT | SDHCI_INT_CRC | SDHCI_INT_TIMEOUT |
			SDHCI_INT_DATA_END | SDHCI_INT_RESPONSE |
			SDHCI_INT_AUTO_CMD_ERR);
	}
	spin_lock_irqsave(&host->lock, flags);
	if (!host->clock) {
		sdhci_cfg_irq(host, true, false);
		spin_unlock_irqrestore(&host->lock, flags);
		mutex_unlock(&host->ios_mutex);
		return;
	}
	spin_unlock_irqrestore(&host->lock, flags);

	if (host->version >= SDHCI_SPEC_300 &&
		(ios->power_mode == MMC_POWER_UP))
		sdhci_enable_preset_value(host, false);

	if (!host->ops->enable_controller_clock && (ios->power_mode &
						    (MMC_POWER_UP |
						     MMC_POWER_ON))) {
		vdd_bit = sdhci_set_power(host, ios->vdd);

		if (host->vmmc && vdd_bit != -1)
			mmc_regulator_set_ocr(host->mmc, host->vmmc, vdd_bit);
	}

	spin_lock_irqsave(&host->lock, flags);
	if (host->ops->platform_send_init_74_clocks)
		host->ops->platform_send_init_74_clocks(host, ios->power_mode);

	/*
	 * If your platform has 8-bit width support but is not a v3 controller,
	 * or if it requires special setup code, you should implement that in
	 * platform_bus_width().
	 */
	if (host->ops->platform_bus_width) {
		host->ops->platform_bus_width(host, ios->bus_width);
	} else {
		ctrl = sdhci_readb(host, SDHCI_HOST_CONTROL);
		if (ios->bus_width == MMC_BUS_WIDTH_8) {
			ctrl &= ~SDHCI_CTRL_4BITBUS;
			if (host->version >= SDHCI_SPEC_300)
				ctrl |= SDHCI_CTRL_8BITBUS;
		} else {
			if (host->version >= SDHCI_SPEC_300)
				ctrl &= ~SDHCI_CTRL_8BITBUS;
			if (ios->bus_width == MMC_BUS_WIDTH_4)
				ctrl |= SDHCI_CTRL_4BITBUS;
			else
				ctrl &= ~SDHCI_CTRL_4BITBUS;
		}
		sdhci_writeb(host, ctrl, SDHCI_HOST_CONTROL);
	}

	ctrl = sdhci_readb(host, SDHCI_HOST_CONTROL);

	if ((ios->timing == MMC_TIMING_SD_HS ||
	     ios->timing == MMC_TIMING_MMC_HS)
	    && !(host->quirks & SDHCI_QUIRK_NO_HISPD_BIT))
		ctrl |= SDHCI_CTRL_HISPD;
	else
		ctrl &= ~SDHCI_CTRL_HISPD;

	if (host->version >= SDHCI_SPEC_300) {
		u16 clk, ctrl_2;

		/* In case of UHS-I modes, set High Speed Enable */
		if ((ios->timing == MMC_TIMING_MMC_HS400) ||
		    (ios->timing == MMC_TIMING_MMC_HS200) ||
		    (ios->timing == MMC_TIMING_UHS_SDR50) ||
		    (ios->timing == MMC_TIMING_UHS_SDR104) ||
		    (ios->timing == MMC_TIMING_UHS_DDR50) ||
		    (ios->timing == MMC_TIMING_UHS_SDR25))
			ctrl |= SDHCI_CTRL_HISPD;

		ctrl_2 = sdhci_readw(host, SDHCI_HOST_CONTROL2);
		if (!(ctrl_2 & SDHCI_CTRL_PRESET_VAL_ENABLE)) {
			sdhci_writeb(host, ctrl, SDHCI_HOST_CONTROL);
			/*
			 * We only need to set Driver Strength if the
			 * preset value enable is not set.
			 */
			ctrl_2 &= ~SDHCI_CTRL_DRV_TYPE_MASK;
			if (ios->drv_type == MMC_SET_DRIVER_TYPE_A)
				ctrl_2 |= SDHCI_CTRL_DRV_TYPE_A;
			else if (ios->drv_type == MMC_SET_DRIVER_TYPE_C)
				ctrl_2 |= SDHCI_CTRL_DRV_TYPE_C;

			sdhci_writew(host, ctrl_2, SDHCI_HOST_CONTROL2);
		} else {
			/*
			 * According to SDHC Spec v3.00, if the Preset Value
			 * Enable in the Host Control 2 register is set, we
			 * need to reset SD Clock Enable before changing High
			 * Speed Enable to avoid generating clock gliches.
			 */

			/* Reset SD Clock Enable */
			clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
			clk &= ~SDHCI_CLOCK_CARD_EN;
			sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);

			sdhci_writeb(host, ctrl, SDHCI_HOST_CONTROL);

			/* Re-enable SD Clock */
			if (ios->clock)
				flags = sdhci_update_clock(host, flags);
		}

		/* Reset SD Clock Enable */
		clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
		clk &= ~SDHCI_CLOCK_CARD_EN;
		sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);

		if (host->ops->set_uhs_signaling)
			host->ops->set_uhs_signaling(host, ios->timing);
		else {
			ctrl_2 = sdhci_readw(host, SDHCI_HOST_CONTROL2);
			/* Select Bus Speed Mode for host */
			ctrl_2 &= ~SDHCI_CTRL_UHS_MASK;
			if (ios->timing == MMC_TIMING_MMC_HS400)
				ctrl_2 |= SDHCI_CTRL_HS_SDR200;
			else if (ios->timing == MMC_TIMING_MMC_HS200)
				ctrl_2 |= SDHCI_CTRL_HS_SDR200;
			else if (ios->timing == MMC_TIMING_UHS_SDR12)
				ctrl_2 |= SDHCI_CTRL_UHS_SDR12;
			else if (ios->timing == MMC_TIMING_UHS_SDR25)
				ctrl_2 |= SDHCI_CTRL_UHS_SDR25;
			else if (ios->timing == MMC_TIMING_UHS_SDR50)
				ctrl_2 |= SDHCI_CTRL_UHS_SDR50;
			else if (ios->timing == MMC_TIMING_UHS_SDR104)
				ctrl_2 |= SDHCI_CTRL_UHS_SDR104;
			else if (ios->timing == MMC_TIMING_UHS_DDR50)
				ctrl_2 |= SDHCI_CTRL_UHS_DDR50;
			sdhci_writew(host, ctrl_2, SDHCI_HOST_CONTROL2);
		}

		if (!(host->quirks2 & SDHCI_QUIRK2_PRESET_VALUE_BROKEN) &&
				((ios->timing == MMC_TIMING_UHS_SDR12) ||
				 (ios->timing == MMC_TIMING_UHS_SDR25) ||
				 (ios->timing == MMC_TIMING_UHS_SDR50) ||
				 (ios->timing == MMC_TIMING_UHS_SDR104) ||
				 (ios->timing == MMC_TIMING_UHS_DDR50))) {
			u16 preset;

			sdhci_enable_preset_value(host, true);
			preset = sdhci_get_preset_value(host);
			ios->drv_type = (preset & SDHCI_PRESET_DRV_MASK)
				>> SDHCI_PRESET_DRV_SHIFT;
		}

		/* Re-enable SD Clock */
		if (ios->clock)
			flags = sdhci_update_clock(host, flags);
	} else
		sdhci_writeb(host, ctrl, SDHCI_HOST_CONTROL);

	spin_unlock_irqrestore(&host->lock, flags);
	/*
	 * Some (ENE) controllers go apeshit on some ios operation,
	 * signalling timeout and CRC errors even on CMD0. Resetting
	 * it on each ios seems to solve the problem.
	 */
	if(host->quirks & SDHCI_QUIRK_RESET_CMD_DATA_ON_IOS)
		sdhci_reset(host, SDHCI_RESET_CMD | SDHCI_RESET_DATA);

	/*
	 * Reset the chip on each power off.
	 * Should clear out any weird states.
	 */
	if (ios->power_mode == MMC_POWER_OFF) {
		sdhci_writel(host, 0, SDHCI_SIGNAL_ENABLE);
		sdhci_reinit(host);
		vdd_bit = sdhci_set_power(host, -1);
		if (host->vmmc && vdd_bit != -1)
			mmc_regulator_set_ocr(host->mmc, host->vmmc, vdd_bit);
		if ((host->quirks & SDHCI_QUIRK_BROKEN_CARD_DETECTION) ||
		    (host->mmc->caps & MMC_CAP_NONREMOVABLE)) {
			/* clear all the pending interrupts */
			sdhci_writel(host, sdhci_readl(host, SDHCI_INT_STATUS),
				     SDHCI_INT_STATUS);
			sdhci_mask_irqs(host, SDHCI_INT_ALL_MASK);
		}
	}
	if (!ios->clock) {
		if (host->async_int_supp && host->mmc->card &&
		    mmc_card_sdio(host->mmc->card)) {
			sdhci_cfg_async_intr(host, true);
			pr_debug("%s: %s: config async intr\n",
				mmc_hostname(host->mmc), __func__);
		}
		sdhci_set_clock(host, ios->clock);
	}

	if (ios->power_mode == MMC_POWER_OFF) {
		/* Keep interrupt line disabled as card is anyway powered off */
		if ((host->quirks & SDHCI_QUIRK_BROKEN_CARD_DETECTION) ||
		    (host->mmc->caps & MMC_CAP_NONREMOVABLE))
			goto out;
	}

	spin_lock_irqsave(&host->lock, flags);
	sdhci_cfg_irq(host, true, false);
	spin_unlock_irqrestore(&host->lock, flags);
out:
	mmiowb();
	mutex_unlock(&host->ios_mutex);
}

static void sdhci_set_ios(struct mmc_host *mmc, struct mmc_ios *ios)
{
	struct sdhci_host *host = mmc_priv(mmc);

	sdhci_runtime_pm_get(host);
	sdhci_do_set_ios(host, ios);
	sdhci_runtime_pm_put(host);
}

static int sdhci_do_get_cd(struct sdhci_host *host)
{
	int gpio_cd = mmc_gpio_get_cd(host->mmc);

	if (host->flags & SDHCI_DEVICE_DEAD)
		return 0;

	/* If polling/nonremovable, assume that the card is always present. */
	if (host->mmc->caps & MMC_CAP_NONREMOVABLE)
		return 1;

	/*
	 * Try slot gpio detect before checking for the broken card detection
	 * quirk. There might be hosts that might have broken card detection
	 * but still provide a gpio for card detection.
	 */
	if (!IS_ERR_VALUE(gpio_cd))
		return !!gpio_cd;

	if (host->quirks & SDHCI_QUIRK_BROKEN_CARD_DETECTION)
		return 1;

	/* Host native card detect */
	return !!(sdhci_readl(host, SDHCI_PRESENT_STATE) & SDHCI_CARD_PRESENT);
}

static int sdhci_get_cd(struct mmc_host *mmc)
{
	struct sdhci_host *host = mmc_priv(mmc);
	int ret;

	sdhci_runtime_pm_get(host);
	ret = sdhci_do_get_cd(host);
	sdhci_runtime_pm_put(host);
	return ret;
}

static int sdhci_check_ro(struct sdhci_host *host)
{
	unsigned long flags;
	int is_readonly;

	spin_lock_irqsave(&host->lock, flags);

	if (host->flags & SDHCI_DEVICE_DEAD)
		is_readonly = 0;
	else if (host->ops->get_ro)
		is_readonly = host->ops->get_ro(host);
	else
		is_readonly = !(sdhci_readl(host, SDHCI_PRESENT_STATE)
				& SDHCI_WRITE_PROTECT);

	spin_unlock_irqrestore(&host->lock, flags);

	/* This quirk needs to be replaced by a callback-function later */
	return host->quirks & SDHCI_QUIRK_INVERTED_WRITE_PROTECT ?
		!is_readonly : is_readonly;
}

#define SAMPLE_COUNT	5

static int sdhci_do_get_ro(struct sdhci_host *host)
{
	int i, ro_count;

	if (!(host->quirks & SDHCI_QUIRK_UNSTABLE_RO_DETECT))
		return sdhci_check_ro(host);

	ro_count = 0;
	for (i = 0; i < SAMPLE_COUNT; i++) {
		if (sdhci_check_ro(host)) {
			if (++ro_count > SAMPLE_COUNT / 2)
				return 1;
		}
		msleep(30);
	}
	return 0;
}

static void sdhci_hw_reset(struct mmc_host *mmc)
{
	struct sdhci_host *host = mmc_priv(mmc);

	if (host->ops && host->ops->hw_reset)
		host->ops->hw_reset(host);
}

static int sdhci_get_ro(struct mmc_host *mmc)
{
	struct sdhci_host *host = mmc_priv(mmc);
	int ret;

	sdhci_runtime_pm_get(host);
	ret = sdhci_do_get_ro(host);
	sdhci_runtime_pm_put(host);
	return ret;
}

static void sdhci_enable_sdio_irq_nolock(struct sdhci_host *host, int enable)
{
	if (host->flags & SDHCI_DEVICE_DEAD)
		goto out;

	if (!enable && !host->clock) {
		pr_debug("%s: %s: defered disabling card intr\n",
			 host->mmc ? mmc_hostname(host->mmc) : "null",
			 __func__);
		host->disable_sdio_irq_deferred = true;
		return;
	}

	if (enable)
		host->flags |= SDHCI_SDIO_IRQ_ENABLED;
	else
		host->flags &= ~SDHCI_SDIO_IRQ_ENABLED;

	/* SDIO IRQ will be enabled as appropriate in runtime resume */
	if (host->runtime_suspended)
		goto out;

	if (enable)
		sdhci_unmask_irqs(host, SDHCI_INT_CARD_INT);
	else
		sdhci_mask_irqs(host, SDHCI_INT_CARD_INT);
out:
	mmiowb();
}

static void sdhci_enable_sdio_irq(struct mmc_host *mmc, int enable)
{
	struct sdhci_host *host = mmc_priv(mmc);
	unsigned long flags;

	spin_lock_irqsave(&host->lock, flags);
	sdhci_enable_sdio_irq_nolock(host, enable);
	spin_unlock_irqrestore(&host->lock, flags);
}

static int sdhci_do_start_signal_voltage_switch(struct sdhci_host *host,
						struct mmc_ios *ios)
{
	u16 ctrl;
	int ret;

	/*
	 * Signal Voltage Switching is only applicable for Host Controllers
	 * v3.00 and above.
	 */
	if (host->version < SDHCI_SPEC_300)
		return 0;

	ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);

	switch (ios->signal_voltage) {
	case MMC_SIGNAL_VOLTAGE_330:
		/* Set 1.8V Signal Enable in the Host Control2 register to 0 */
		ctrl &= ~SDHCI_CTRL_VDD_180;
		sdhci_writew(host, ctrl, SDHCI_HOST_CONTROL2);
		if (host->ops->check_power_status)
			host->ops->check_power_status(host, REQ_IO_HIGH);

		if (host->vqmmc) {
			ret = regulator_set_voltage(host->vqmmc, 2700000, 3600000);
			if (ret) {
				pr_warning("%s: Switching to 3.3V signalling voltage "
						" failed\n", mmc_hostname(host->mmc));
				return -EIO;
			}
		}
		/* Wait for 5ms */
		usleep_range(5000, 5500);

		/* 3.3V regulator output should be stable within 5 ms */
		ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);
		if (!(ctrl & SDHCI_CTRL_VDD_180))
			return 0;

		pr_warning("%s: 3.3V regulator output did not became stable\n",
				mmc_hostname(host->mmc));

		return -EAGAIN;
	case MMC_SIGNAL_VOLTAGE_180:
		if (host->vqmmc) {
			ret = regulator_set_voltage(host->vqmmc,
					1700000, 1950000);
			if (ret) {
				pr_warning("%s: Switching to 1.8V signalling voltage "
						" failed\n", mmc_hostname(host->mmc));
				return -EIO;
			}
		}

		/*
		 * Enable 1.8V Signal Enable in the Host Control2
		 * register
		 */
		ctrl |= SDHCI_CTRL_VDD_180;
		sdhci_writew(host, ctrl, SDHCI_HOST_CONTROL2);
		if (host->ops->check_power_status)
			host->ops->check_power_status(host, REQ_IO_LOW);

		/* Wait for 5ms */
		usleep_range(5000, 5500);

		/* 1.8V regulator output should be stable within 5 ms */
		ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);
		if (ctrl & SDHCI_CTRL_VDD_180)
			return 0;

		pr_warning("%s: 1.8V regulator output did not became stable\n",
				mmc_hostname(host->mmc));

		return -EAGAIN;
	case MMC_SIGNAL_VOLTAGE_120:
		if (host->vqmmc) {
			ret = regulator_set_voltage(host->vqmmc, 1100000, 1300000);
			if (ret) {
				pr_warning("%s: Switching to 1.2V signalling voltage "
						" failed\n", mmc_hostname(host->mmc));
				return -EIO;
			}
		}
		return 0;
	default:
		/* No signal voltage switch required */
		return 0;
	}
}

static int sdhci_start_signal_voltage_switch(struct mmc_host *mmc,
	struct mmc_ios *ios)
{
	struct sdhci_host *host = mmc_priv(mmc);
	int err;

	if (host->version < SDHCI_SPEC_300)
		return 0;
	sdhci_runtime_pm_get(host);
	err = sdhci_do_start_signal_voltage_switch(host, ios);
	sdhci_runtime_pm_put(host);
	return err;
}

static int sdhci_card_busy(struct mmc_host *mmc)
{
	struct sdhci_host *host = mmc_priv(mmc);
	u32 present_state;

	sdhci_runtime_pm_get(host);
	/* Check whether DAT[3:0] is 0000 */
	present_state = sdhci_readl(host, SDHCI_PRESENT_STATE);
	sdhci_runtime_pm_put(host);

	return !(present_state & SDHCI_DATA_LVL_MASK);
}

static int sdhci_enhanced_strobe(struct mmc_host *mmc)
{
	struct sdhci_host *host = mmc_priv(mmc);
	int err = -EINVAL;

	sdhci_runtime_pm_get(host);
	if (host->ops->enhanced_strobe)
		err = host->ops->enhanced_strobe(host);
	sdhci_runtime_pm_put(host);

	return err;
}

static int sdhci_execute_tuning(struct mmc_host *mmc, u32 opcode)
{
	struct sdhci_host *host;
	u16 ctrl;
	u32 ier = 0;
	int tuning_loop_counter = MAX_TUNING_LOOP;
	unsigned long timeout;
	int err = 0;
	bool requires_tuning_nonuhs = false;

	host = mmc_priv(mmc);

	sdhci_runtime_pm_get(host);
	disable_irq(host->irq);
	spin_lock(&host->lock);

	ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);

	/*
	 * The Host Controller needs tuning only in case of SDR104 mode
	 * and for SDR50 mode when Use Tuning for SDR50 is set in the
	 * Capabilities register.
	 * If the Host Controller supports the HS400/HS200 mode then the
	 * tuning function has to be executed.
	 */
	if ((((ctrl & SDHCI_CTRL_UHS_MASK) == SDHCI_CTRL_UHS_SDR50) &&
	    (host->flags & SDHCI_SDR50_NEEDS_TUNING)) ||
	     (host->flags & SDHCI_HS200_NEEDS_TUNING) ||
	     (host->flags & SDHCI_HS400_NEEDS_TUNING))
		requires_tuning_nonuhs = true;

	if (((ctrl & SDHCI_CTRL_UHS_MASK) == SDHCI_CTRL_UHS_SDR104) ||
	    requires_tuning_nonuhs)
		ctrl |= SDHCI_CTRL_EXEC_TUNING;
	else {
		spin_unlock(&host->lock);
		enable_irq(host->irq);
		sdhci_runtime_pm_put(host);
		return 0;
	}

	if (host->ops->execute_tuning) {
		spin_unlock(&host->lock);
		enable_irq(host->irq);
		err = host->ops->execute_tuning(host, opcode);
		disable_irq(host->irq);
		spin_lock(&host->lock);
		goto out;
	}
	sdhci_writew(host, ctrl, SDHCI_HOST_CONTROL2);

	/*
	 * As per the Host Controller spec v3.00, tuning command
	 * generates Buffer Read Ready interrupt, so enable that.
	 *
	 * Note: The spec clearly says that when tuning sequence
	 * is being performed, the controller does not generate
	 * interrupts other than Buffer Read Ready interrupt. But
	 * to make sure we don't hit a controller bug, we _only_
	 * enable Buffer Read Ready interrupt here.
	 */
	ier = sdhci_readl(host, SDHCI_INT_ENABLE);
	sdhci_clear_set_irqs(host, ier, SDHCI_INT_DATA_AVAIL);

	/*
	 * Issue CMD19 repeatedly till Execute Tuning is set to 0 or the number
	 * of loops reaches 40 times or a timeout of 150ms occurs.
	 */
	timeout = 150;
	do {
		struct mmc_command cmd = {0};
		struct mmc_request mrq = {NULL};

		if (!tuning_loop_counter && !timeout)
			break;

		cmd.opcode = opcode;
		cmd.arg = 0;
		cmd.flags = MMC_RSP_R1 | MMC_CMD_ADTC;
		cmd.retries = 0;
		cmd.data = NULL;
		cmd.error = 0;

		mrq.cmd = &cmd;
		host->mrq = &mrq;

		/*
		 * In response to CMD19, the card sends 64 bytes of tuning
		 * block to the Host Controller. So we set the block size
		 * to 64 here.
		 */
		if ((cmd.opcode == MMC_SEND_TUNING_BLOCK_HS400) ||
		    (cmd.opcode == MMC_SEND_TUNING_BLOCK_HS200)) {
			if (mmc->ios.bus_width == MMC_BUS_WIDTH_8)
				sdhci_set_blk_size_reg(host, 128, 7);
			else if (mmc->ios.bus_width == MMC_BUS_WIDTH_4)
				sdhci_set_blk_size_reg(host, 64, 7);
		} else {
			sdhci_set_blk_size_reg(host, 64, 7);
		}

		/*
		 * The tuning block is sent by the card to the host controller.
		 * So we set the TRNS_READ bit in the Transfer Mode register.
		 * This also takes care of setting DMA Enable and Multi Block
		 * Select in the same register to 0.
		 */
		sdhci_writew(host, SDHCI_TRNS_READ, SDHCI_TRANSFER_MODE);

		sdhci_send_command(host, &cmd);

		host->cmd = NULL;
		host->mrq = NULL;

		spin_unlock(&host->lock);
		enable_irq(host->irq);

		/* Wait for Buffer Read Ready interrupt */
		wait_event_interruptible_timeout(host->buf_ready_int,
					(host->tuning_done == 1),
					msecs_to_jiffies(50));
		disable_irq(host->irq);
		spin_lock(&host->lock);

		if (!host->tuning_done) {
			pr_info(DRIVER_NAME ": Timeout waiting for "
				"Buffer Read Ready interrupt during tuning "
				"procedure, falling back to fixed sampling "
				"clock\n");
			ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);
			ctrl &= ~SDHCI_CTRL_TUNED_CLK;
			ctrl &= ~SDHCI_CTRL_EXEC_TUNING;
			sdhci_writew(host, ctrl, SDHCI_HOST_CONTROL2);

			err = -EIO;
			goto out;
		}

		host->tuning_done = 0;

		ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);
		tuning_loop_counter--;
		timeout--;
		mdelay(1);
	} while (ctrl & SDHCI_CTRL_EXEC_TUNING);

	/*
	 * The Host Driver has exhausted the maximum number of loops allowed,
	 * so use fixed sampling frequency.
	 */
	if (!tuning_loop_counter || !timeout) {
		ctrl &= ~SDHCI_CTRL_TUNED_CLK;
		sdhci_writew(host, ctrl, SDHCI_HOST_CONTROL2);
	} else {
		if (!(ctrl & SDHCI_CTRL_TUNED_CLK)) {
			pr_info(DRIVER_NAME ": Tuning procedure"
				" failed, falling back to fixed sampling"
				" clock\n");
			err = -EIO;
		}
	}

out:
	/*
	 * If this is the very first time we are here, we start the retuning
	 * timer. Since only during the first time, SDHCI_NEEDS_RETUNING
	 * flag won't be set, we check this condition before actually starting
	 * the timer.
	 */
	if (!(host->flags & SDHCI_NEEDS_RETUNING) && host->tuning_count &&
	    (host->tuning_mode == SDHCI_TUNING_MODE_1)) {
		host->flags |= SDHCI_USING_RETUNING_TIMER;
		mod_timer(&host->tuning_timer, jiffies +
			host->tuning_count * HZ);
		/* Tuning mode 1 limits the maximum data length to 4MB */
		mmc->max_blk_count = (4 * 1024 * 1024) / mmc->max_blk_size;
	} else {
		host->flags &= ~SDHCI_NEEDS_RETUNING;
		/* Reload the new initial value for timer */
		if (host->tuning_mode == SDHCI_TUNING_MODE_1)
			mod_timer(&host->tuning_timer, jiffies +
				host->tuning_count * HZ);
	}

	/*
	 * In case tuning fails, host controllers which support re-tuning can
	 * try tuning again at a later time, when the re-tuning timer expires.
	 * So for these controllers, we return 0. Since there might be other
	 * controllers who do not have this capability, we return error for
	 * them. SDHCI_USING_RETUNING_TIMER means the host is currently using
	 * a retuning timer to do the retuning for the card.
	 */
	if (err && (host->flags & SDHCI_USING_RETUNING_TIMER))
		err = 0;

	sdhci_clear_set_irqs(host, SDHCI_INT_DATA_AVAIL, ier);
	spin_unlock(&host->lock);
	enable_irq(host->irq);
	sdhci_runtime_pm_put(host);

	return err;
}


static void sdhci_enable_preset_value(struct sdhci_host *host, bool enable)
{
	u16 ctrl;

	/* Host Controller v3.00 defines preset value registers */
	if (host->version < SDHCI_SPEC_300)
		return;

	if (host->quirks2 & SDHCI_QUIRK2_BROKEN_PRESET_VALUE)
		return;

	ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);

	/*
	 * We only enable or disable Preset Value if they are not already
	 * enabled or disabled respectively. Otherwise, we bail out.
	 */
	if (enable && !(ctrl & SDHCI_CTRL_PRESET_VAL_ENABLE)) {
		ctrl |= SDHCI_CTRL_PRESET_VAL_ENABLE;
		sdhci_writew(host, ctrl, SDHCI_HOST_CONTROL2);
		host->flags |= SDHCI_PV_ENABLED;
	} else if (!enable && (ctrl & SDHCI_CTRL_PRESET_VAL_ENABLE)) {
		ctrl &= ~SDHCI_CTRL_PRESET_VAL_ENABLE;
		sdhci_writew(host, ctrl, SDHCI_HOST_CONTROL2);
		host->flags &= ~SDHCI_PV_ENABLED;
	}
}

static void sdhci_card_event(struct mmc_host *mmc)
{
	struct sdhci_host *host = mmc_priv(mmc);
	unsigned long flags;

	if (host->quirks & SDHCI_QUIRK_BROKEN_CARD_DETECTION)
		return;

	spin_lock_irqsave(&host->lock, flags);

	/* Check host->mrq first in case we are runtime suspended */
	if (host->mrq &&
	    !(sdhci_readl(host, SDHCI_PRESENT_STATE) & SDHCI_CARD_PRESENT)) {
		pr_err("%s: Card removed during transfer!\n",
			mmc_hostname(host->mmc));
		pr_err("%s: Resetting controller.\n",
			mmc_hostname(host->mmc));

		sdhci_reset(host, SDHCI_RESET_CMD);
		sdhci_reset(host, SDHCI_RESET_DATA);

		host->mrq->cmd->error = -ENOMEDIUM;
		tasklet_schedule(&host->finish_tasklet);
	}

	spin_unlock_irqrestore(&host->lock, flags);
}

static int sdhci_stop_request(struct mmc_host *mmc)
{
	struct sdhci_host *host = mmc_priv(mmc);
	unsigned long flags;
	struct mmc_data *data;
	int ret = 0;

	spin_lock_irqsave(&host->lock, flags);
	if (!host->mrq || !host->data) {
		ret = MMC_BLK_NO_REQ_TO_STOP;
		goto out;
	}

	data = host->data;

	if (host->ops->disable_data_xfer)
		host->ops->disable_data_xfer(host);

	sdhci_reset(host, SDHCI_RESET_CMD | SDHCI_RESET_DATA);

	if (host->flags & SDHCI_REQ_USE_DMA) {
		if (host->flags & SDHCI_USE_ADMA) {
			sdhci_adma_table_post(host, data);
		} else {
			if (!data->host_cookie)
				dma_unmap_sg(mmc_dev(host->mmc), data->sg,
					     data->sg_len,
					     (data->flags & MMC_DATA_READ) ?
					     DMA_FROM_DEVICE : DMA_TO_DEVICE);
		}
	}
	del_timer(&host->timer);
	host->mrq = NULL;
	host->cmd = NULL;
	host->data = NULL;
out:
	spin_unlock_irqrestore(&host->lock, flags);
	return ret;
}

static unsigned int sdhci_get_xfer_remain(struct mmc_host *mmc)
{
	struct sdhci_host *host = mmc_priv(mmc);
	u32 present_state = 0;

	present_state = sdhci_readl(host, SDHCI_PRESENT_STATE);

	return present_state & SDHCI_DOING_WRITE;
}

static const struct mmc_host_ops sdhci_ops = {
	.pre_req	= sdhci_pre_req,
	.post_req	= sdhci_post_req,
	.request	= sdhci_request,
	.set_ios	= sdhci_set_ios,
	.get_cd		= sdhci_get_cd,
	.get_ro		= sdhci_get_ro,
	.hw_reset	= sdhci_hw_reset,
	.enable_sdio_irq = sdhci_enable_sdio_irq,
	.start_signal_voltage_switch	= sdhci_start_signal_voltage_switch,
	.execute_tuning			= sdhci_execute_tuning,
	.enhanced_strobe		= sdhci_enhanced_strobe,
	.card_event			= sdhci_card_event,
	.card_busy	= sdhci_card_busy,
	.enable		= sdhci_enable,
	.disable	= sdhci_disable,
	.stop_request = sdhci_stop_request,
	.get_xfer_remain = sdhci_get_xfer_remain,
	.notify_load	= sdhci_notify_load,
	.notify_pm_status	= sdhci_notify_pm_status,
};

/*****************************************************************************\
 *                                                                           *
 * Tasklets                                                                  *
 *                                                                           *
\*****************************************************************************/

static void sdhci_tasklet_card(unsigned long param)
{
	struct sdhci_host *host = (struct sdhci_host*)param;

	sdhci_card_event(host->mmc);

	mmc_detect_change(host->mmc, msecs_to_jiffies(200));
}

static void sdhci_tasklet_finish(unsigned long param)
{
	struct sdhci_host *host;
	unsigned long flags;
	struct mmc_request *mrq;

	host = (struct sdhci_host*)param;

	spin_lock_irqsave(&host->lock, flags);

        /*
         * If this tasklet gets rescheduled while running, it will
         * be run again afterwards but without any active request.
         */
	if (!host->mrq) {
		spin_unlock_irqrestore(&host->lock, flags);
		return;
	}

	del_timer(&host->timer);

	mrq = host->mrq;

	/*
	 * The controller needs a reset of internal state machines
	 * upon error conditions.
	 */
	if (!(host->flags & SDHCI_DEVICE_DEAD) &&
	    ((mrq->cmd && mrq->cmd->error) ||
		 (mrq->data && (mrq->data->error ||
		  (mrq->data->stop && mrq->data->stop->error))) ||
		   (host->quirks & SDHCI_QUIRK_RESET_AFTER_REQUEST))) {

		/* Some controllers need this kick or reset won't work here */
		if (host->quirks & SDHCI_QUIRK_CLOCK_BEFORE_RESET)
			/* This is to force an update */
			flags = sdhci_update_clock(host, flags);

		/* Spec says we should do both at the same time, but Ricoh
		   controllers do not like that. */
		sdhci_reset(host, SDHCI_RESET_CMD);
		sdhci_reset(host, SDHCI_RESET_DATA);
	} else {
		if (host->quirks2 & SDHCI_QUIRK2_RDWR_TX_ACTIVE_EOT)
			sdhci_reset(host, SDHCI_RESET_DATA);
	}

	host->mrq = NULL;
	host->cmd = NULL;
	host->data = NULL;
	host->auto_cmd_err_sts = 0;

#ifndef SDHCI_USE_LEDS_CLASS
	if (!(host->quirks2 & SDHCI_QUIRK2_BROKEN_LED_CONTROL))
		sdhci_deactivate_led(host);
#endif

	mmiowb();
	spin_unlock_irqrestore(&host->lock, flags);

	sdhci_update_pm_qos(host->mmc, NULL, false);
	mmc_request_done(host->mmc, mrq);
	sdhci_runtime_pm_put(host);
}

static void sdhci_timeout_timer(unsigned long data)
{
	struct sdhci_host *host;
	unsigned long flags;

	host = (struct sdhci_host*)data;

	spin_lock_irqsave(&host->lock, flags);

	if (host->mrq) {
		if (!host->mrq->cmd->ignore_timeout) {
			pr_err("%s: Timeout waiting for hardware interrupt.\n",
			       mmc_hostname(host->mmc));
			if (host->data) {
				sdhci_show_adma_error(host);
			} else {
				if (host->mmc->card &&
						!mmc_card_sdio(host->mmc->card))
					sdhci_dumpregs(host);
			}
		}

		if (host->data) {
			pr_info("%s: bytes to transfer: %d transferred: %d\n",
				mmc_hostname(host->mmc),
				(host->data->blksz * host->data->blocks),
				(sdhci_readw(host, SDHCI_BLOCK_SIZE) & 0xFFF) *
				sdhci_readw(host, SDHCI_BLOCK_COUNT));
			host->data->error = -ETIMEDOUT;
			sdhci_finish_data(host);
		} else {
			if (host->cmd)
				host->cmd->error = -ETIMEDOUT;
			else
				host->mrq->cmd->error = -ETIMEDOUT;

			tasklet_schedule(&host->finish_tasklet);
		}
	}

	mmiowb();
	spin_unlock_irqrestore(&host->lock, flags);
}

static void sdhci_tuning_timer(unsigned long data)
{
	struct sdhci_host *host;
	unsigned long flags;

	host = (struct sdhci_host *)data;

	spin_lock_irqsave(&host->lock, flags);

	host->flags |= SDHCI_NEEDS_RETUNING;

	spin_unlock_irqrestore(&host->lock, flags);
}

/*****************************************************************************\
 *                                                                           *
 * Interrupt handling                                                        *
 *                                                                           *
\*****************************************************************************/

static void sdhci_cmd_irq(struct sdhci_host *host, u32 intmask)
{
	u16 auto_cmd_status;
	u32 command;
	BUG_ON(intmask == 0);

	if (!host->cmd) {
		pr_err("%s: Got command interrupt 0x%08x even "
			"though no command operation was in progress.\n",
			mmc_hostname(host->mmc), (unsigned)intmask);
		sdhci_dumpregs(host);
		return;
	}

	trace_mmc_cmd_rw_end(host->cmd->opcode, intmask,
				sdhci_readl(host, SDHCI_RESPONSE));

	if (intmask & SDHCI_INT_TIMEOUT)
		host->cmd->error = -ETIMEDOUT;
	else if (intmask & (SDHCI_INT_CRC | SDHCI_INT_END_BIT |
			SDHCI_INT_INDEX))
		host->cmd->error = -EILSEQ;

	if (intmask & SDHCI_INT_AUTO_CMD_ERR) {
		auto_cmd_status = host->auto_cmd_err_sts;
		pr_err_ratelimited("%s: %s: AUTO CMD err sts 0x%08x\n",
			mmc_hostname(host->mmc), __func__, auto_cmd_status);
		if (auto_cmd_status & (SDHCI_AUTO_CMD12_NOT_EXEC |
				       SDHCI_AUTO_CMD_INDEX_ERR |
				       SDHCI_AUTO_CMD_ENDBIT_ERR))
			host->cmd->error = -EIO;
		else if (auto_cmd_status & SDHCI_AUTO_CMD_TIMEOUT_ERR)
			host->cmd->error = -ETIMEDOUT;
		else if (auto_cmd_status & SDHCI_AUTO_CMD_CRC_ERR)
			host->cmd->error = -EILSEQ;
	}

	if (host->cmd->error) {
		command = SDHCI_GET_CMD(sdhci_readw(host,
						    SDHCI_COMMAND));
		if (host->cmd->error == -EILSEQ &&
		    (command != MMC_SEND_TUNING_BLOCK_HS400) &&
		    (command != MMC_SEND_TUNING_BLOCK_HS200) &&
		    (command != MMC_SEND_TUNING_BLOCK) &&
		    (command != MMC_SEND_STATUS))
				host->flags |= SDHCI_NEEDS_RETUNING;
		tasklet_schedule(&host->finish_tasklet);
		return;
	}

	/*
	 * The host can send and interrupt when the busy state has
	 * ended, allowing us to wait without wasting CPU cycles.
	 * Unfortunately this is overloaded on the "data complete"
	 * interrupt, so we need to take some care when handling
	 * it.
	 *
	 * Note: The 1.0 specification is a bit ambiguous about this
	 *       feature so there might be some problems with older
	 *       controllers.
	 */
	if (host->cmd->flags & MMC_RSP_BUSY) {
		if (host->cmd->data)
			DBG("Cannot wait for busy signal when also "
				"doing a data transfer");
		else if (!(host->quirks & SDHCI_QUIRK_NO_BUSY_IRQ))
			return;

		/* The controller does not support the end-of-busy IRQ,
		 * fall through and take the SDHCI_INT_RESPONSE */
	}

	if (intmask & SDHCI_INT_RESPONSE)
		sdhci_finish_command(host);
}

static void sdhci_show_adma_error(struct sdhci_host *host)
{
	const char *name = mmc_hostname(host->mmc);
	u8 *desc = host->adma_desc;
	__le16 *len;
	u8 attr;

	sdhci_dumpregs(host);

	while (true) {
		len = (__le16 *)(desc + 2);
		attr = *desc;

		if (host->flags & SDHCI_USE_ADMA_64BIT) {
			__le64 *dma = (__le64 *)(desc + 4);
			pr_info("%s: %p: DMA %llx, LEN 0x%04x, Attr=0x%02x\n",
			    name, desc, (long long)le64_to_cpu(*dma),
			    le16_to_cpu(*len), attr);
		} else {
			__le32 *dma = (__le32 *)(desc + 4);
			pr_info("%s: %p: DMA 0x%08x, LEN 0x%04x, Attr=0x%02x\n",
			    name, desc, le32_to_cpu(*dma), le16_to_cpu(*len),
			    attr);
		}

		desc += host->adma_desc_line_sz;

		if (attr & 2)
			break;
	}
}

static void sdhci_data_irq(struct sdhci_host *host, u32 intmask)
{
	u32 command;
	bool pr_msg = false;
	BUG_ON(intmask == 0);

	command = SDHCI_GET_CMD(sdhci_readw(host, SDHCI_COMMAND));
	trace_mmc_data_rw_end(command, intmask);

	/* CMD19 generates _only_ Buffer Read Ready interrupt */
	if (intmask & SDHCI_INT_DATA_AVAIL) {
		if (!(host->quirks2 & SDHCI_QUIRK2_NON_STANDARD_TUNING) &&
			(command == MMC_SEND_TUNING_BLOCK ||
			command == MMC_SEND_TUNING_BLOCK_HS200 ||
			command == MMC_SEND_TUNING_BLOCK_HS400)) {
			host->tuning_done = 1;
			wake_up(&host->buf_ready_int);
			return;
		}
	}

	if (!host->data) {
		/*
		 * The "data complete" interrupt is also used to
		 * indicate that a busy state has ended. See comment
		 * above in sdhci_cmd_irq().
		 */
		if (host->cmd && (host->cmd->flags & MMC_RSP_BUSY)) {
			if (intmask & SDHCI_INT_DATA_END) {
				sdhci_finish_command(host);
				return;
			}
			if (host->quirks2 &
				SDHCI_QUIRK2_IGNORE_DATATOUT_FOR_R1BCMD)
				return;
		}

		pr_err("%s: Got data interrupt 0x%08x even "
			"though no data operation was in progress.\n",
			mmc_hostname(host->mmc), (unsigned)intmask);
		sdhci_dumpregs(host);

		return;
	}

	if (intmask & SDHCI_INT_DATA_TIMEOUT)
		host->data->error = -ETIMEDOUT;
	else if (intmask & SDHCI_INT_DATA_END_BIT)
		host->data->error = -EILSEQ;
	else if ((intmask & SDHCI_INT_DATA_CRC) &&
		(command != MMC_BUS_TEST_R))
		host->data->error = -EILSEQ;
	else if (intmask & SDHCI_INT_ADMA_ERROR) {
		pr_err("%s: ADMA error\n", mmc_hostname(host->mmc));
		sdhci_show_adma_error(host);
		host->data->error = -EIO;
		if (host->ops->adma_workaround)
			host->ops->adma_workaround(host, intmask);
	}
	if (host->data->error) {
		if (intmask & (SDHCI_INT_DATA_CRC | SDHCI_INT_DATA_TIMEOUT)) {
			if ((command != MMC_SEND_TUNING_BLOCK_HS400) &&
			    (command != MMC_SEND_TUNING_BLOCK_HS200) &&
			    (command != MMC_SEND_TUNING_BLOCK)) {
				pr_msg = true;
				if (intmask & SDHCI_INT_DATA_CRC)
					host->flags |= SDHCI_NEEDS_RETUNING;
			}
		} else {
			pr_msg = true;
		}
		if (pr_msg && __ratelimit(&host->dbg_dump_rs)) {
			pr_err("%s: data txfr (0x%08x) error: %d after %lld ms\n",
			       mmc_hostname(host->mmc), intmask,
			       host->data->error, ktime_to_ms(ktime_sub(
			       ktime_get(), host->data_start_time)));
			sdhci_dumpregs(host);
		}
		sdhci_finish_data(host);
	} else {
		if (intmask & (SDHCI_INT_DATA_AVAIL | SDHCI_INT_SPACE_AVAIL))
			sdhci_transfer_pio(host);

		/*
		 * We currently don't do anything fancy with DMA
		 * boundaries, but as we can't disable the feature
		 * we need to at least restart the transfer.
		 *
		 * According to the spec sdhci_readl(host, SDHCI_DMA_ADDRESS)
		 * should return a valid address to continue from, but as
		 * some controllers are faulty, don't trust them.
		 */
		if (intmask & SDHCI_INT_DMA_END) {
			u32 dmastart, dmanow;
			dmastart = sg_dma_address(host->data->sg);
			dmanow = dmastart + host->data->bytes_xfered;
			/*
			 * Force update to the next DMA block boundary.
			 */
			dmanow = (dmanow &
				~(SDHCI_DEFAULT_BOUNDARY_SIZE - 1)) +
				SDHCI_DEFAULT_BOUNDARY_SIZE;
			host->data->bytes_xfered = dmanow - dmastart;
			DBG("%s: DMA base 0x%08x, transferred 0x%06x bytes,"
				" next 0x%08x\n",
				mmc_hostname(host->mmc), dmastart,
				host->data->bytes_xfered, dmanow);
			sdhci_writel(host, dmanow, SDHCI_DMA_ADDRESS);
		}

		if (intmask & SDHCI_INT_DATA_END) {
			if (host->cmd) {
				/*
				 * Data managed to finish before the
				 * command completed. Make sure we do
				 * things in the proper order.
				 */
				host->data_early = 1;
			} else {
				sdhci_finish_data(host);
			}
		}
	}
}

#ifdef CONFIG_MMC_CQ_HCI
static int sdhci_get_cmd_err(u32 intmask)
{
	if (intmask & SDHCI_INT_TIMEOUT)
		return -ETIMEDOUT;
	else if (intmask & (SDHCI_INT_CRC | SDHCI_INT_END_BIT |
			    SDHCI_INT_INDEX))
		return -EILSEQ;
	return 0;
}

static int sdhci_get_data_err(u32 intmask)
{
	if (intmask & SDHCI_INT_DATA_TIMEOUT)
		return -ETIMEDOUT;
	else if (intmask & (SDHCI_INT_DATA_END_BIT | SDHCI_INT_DATA_CRC))
		return -EILSEQ;
	else if (intmask & SDHCI_INT_ADMA_ERROR)
		return -EIO;
	return 0;
}

static irqreturn_t sdhci_cmdq_irq(struct sdhci_host *host, u32 intmask)
{
	int err = 0;
	u32 mask = 0;
	irqreturn_t ret;

	if (intmask & SDHCI_INT_CMD_MASK)
		err = sdhci_get_cmd_err(intmask);
	else if (intmask & SDHCI_INT_DATA_MASK)
		err = sdhci_get_data_err(intmask);

	ret = cmdq_irq(host->mmc, err);
	if (err) {
		/* Clear the error interrupts */
		mask = intmask & SDHCI_INT_ERROR_MASK;
		sdhci_writel(host, mask, SDHCI_INT_STATUS);
	}
	return ret;

}

#else
static irqreturn_t sdhci_cmdq_irq(struct sdhci_host *host, u32 intmask)
{
	pr_err("%s: Received cmdq-irq when disabled !!!!\n",
		mmc_hostname(host->mmc));
	return IRQ_NONE;
}
#endif

static irqreturn_t sdhci_irq(int irq, void *dev_id)
{
	irqreturn_t result;
	struct sdhci_host *host = dev_id;
	u32 intmask, unexpected = 0;
	int cardint = 0, max_loops = 16;

	spin_lock(&host->lock);

	if (host->runtime_suspended) {
		spin_unlock(&host->lock);
		pr_warning("%s: got irq while runtime suspended\n",
		       mmc_hostname(host->mmc));
		return IRQ_HANDLED;
	}

	if (!host->clock && host->mmc->card &&
	    mmc_card_sdio(host->mmc->card)) {
		/* SDIO async. interrupt is level-sensitive */
		sdhci_cfg_irq(host, false, false);
		pr_debug("%s: got async-irq: clocks: %d gated: %d host-irq[en:1/dis:0]: %d\n",
			mmc_hostname(host->mmc), host->clock,
			host->mmc->clk_gated, host->irq_enabled);
		spin_unlock(&host->lock);
		/* prevent suspend till the ksdioirqd runs or resume happens */
		if ((host->mmc->dev_status == DEV_SUSPENDING) ||
		    (host->mmc->dev_status == DEV_SUSPENDED))
			pm_wakeup_event(&host->mmc->card->dev,
					SDHCI_SUSPEND_TIMEOUT);
		else
			mmc_signal_sdio_irq(host->mmc);
		return IRQ_HANDLED;
	} else if (!host->clock) {
		/*
		 * As clocks are disabled, controller registers might not be
		 * accessible hence return from here.
		 */
		pr_err_ratelimited("%s: %s: clocks are disabled !!!\n",
				   mmc_hostname(host->mmc), __func__);
		spin_unlock(&host->lock);
		return IRQ_HANDLED;
	}
	intmask = sdhci_readl(host, SDHCI_INT_STATUS);

	if (!intmask || intmask == 0xffffffff) {
		result = IRQ_NONE;
		goto out;
	}

again:
	if (host->mmc->card && mmc_card_cmdq(host->mmc->card) &&
	    !mmc_host_halt(host->mmc)) {
		pr_debug("*** %s: cmdq intr: 0x%08x\n",
				mmc_hostname(host->mmc),
				intmask);
		result = sdhci_cmdq_irq(host, intmask);
		if (result == IRQ_HANDLED)
			goto out;
	}

	DBG("*** %s got interrupt: 0x%08x\n",
		mmc_hostname(host->mmc), intmask);

	if (intmask & (SDHCI_INT_CARD_INSERT | SDHCI_INT_CARD_REMOVE)) {
		u32 present = sdhci_readl(host, SDHCI_PRESENT_STATE) &
			      SDHCI_CARD_PRESENT;

		/*
		 * There is a observation on i.mx esdhc.  INSERT bit will be
		 * immediately set again when it gets cleared, if a card is
		 * inserted.  We have to mask the irq to prevent interrupt
		 * storm which will freeze the system.  And the REMOVE gets
		 * the same situation.
		 *
		 * More testing are needed here to ensure it works for other
		 * platforms though.
		 */
		sdhci_mask_irqs(host, present ? SDHCI_INT_CARD_INSERT :
						SDHCI_INT_CARD_REMOVE);
		sdhci_unmask_irqs(host, present ? SDHCI_INT_CARD_REMOVE :
						  SDHCI_INT_CARD_INSERT);

		sdhci_writel(host, intmask & (SDHCI_INT_CARD_INSERT |
			     SDHCI_INT_CARD_REMOVE), SDHCI_INT_STATUS);
		intmask &= ~(SDHCI_INT_CARD_INSERT | SDHCI_INT_CARD_REMOVE);
		tasklet_schedule(&host->card_tasklet);
	}

	if (intmask & SDHCI_INT_CMD_MASK) {
		if (intmask & SDHCI_INT_AUTO_CMD_ERR)
			host->auto_cmd_err_sts = sdhci_readw(host,
					SDHCI_AUTO_CMD_ERR);
		sdhci_writel(host, intmask & SDHCI_INT_CMD_MASK,
			SDHCI_INT_STATUS);
		if ((host->quirks2 & SDHCI_QUIRK2_SLOW_INT_CLR) &&
		    (host->clock <= 400000))
			udelay(40);
		sdhci_cmd_irq(host, intmask & SDHCI_INT_CMD_MASK);
	}

	if (intmask & SDHCI_INT_DATA_MASK) {
		sdhci_writel(host, intmask & SDHCI_INT_DATA_MASK,
			SDHCI_INT_STATUS);
		if ((host->quirks2 & SDHCI_QUIRK2_SLOW_INT_CLR) &&
		    (host->clock <= 400000))
			udelay(40);
		sdhci_data_irq(host, intmask & SDHCI_INT_DATA_MASK);
	}

	intmask &= ~(SDHCI_INT_CMD_MASK | SDHCI_INT_DATA_MASK);

	intmask &= ~SDHCI_INT_ERROR;

	if (intmask & SDHCI_INT_BUS_POWER) {
		pr_err("%s: Card is consuming too much power!\n",
			mmc_hostname(host->mmc));
		sdhci_writel(host, SDHCI_INT_BUS_POWER, SDHCI_INT_STATUS);
	}

	intmask &= ~SDHCI_INT_BUS_POWER;

	if (intmask & SDHCI_INT_CARD_INT)
		cardint = 1;

	intmask &= ~SDHCI_INT_CARD_INT;

	if (intmask) {
		unexpected |= intmask;
		sdhci_writel(host, intmask, SDHCI_INT_STATUS);
	}

	result = IRQ_HANDLED;

	intmask = sdhci_readl(host, SDHCI_INT_STATUS);
	if (intmask && --max_loops)
		goto again;
out:
	spin_unlock(&host->lock);

	if (unexpected) {
		pr_err("%s: Unexpected interrupt 0x%08x.\n",
			   mmc_hostname(host->mmc), unexpected);
		sdhci_dumpregs(host);
	}
	/*
	 * We have to delay this as it calls back into the driver.
	 */
	if (cardint) {
		/* clks are on, but suspend may be in progress */
		if (host->mmc->dev_status == DEV_SUSPENDING)
			pm_wakeup_event(&host->mmc->card->dev,
					SDHCI_SUSPEND_TIMEOUT);
		mmc_signal_sdio_irq(host->mmc);
	}
	return result;
}

/*****************************************************************************\
 *                                                                           *
 * Suspend/resume                                                            *
 *                                                                           *
\*****************************************************************************/

#ifdef CONFIG_PM
void sdhci_enable_irq_wakeups(struct sdhci_host *host)
{
	u8 val;
	u8 mask = SDHCI_WAKE_ON_INSERT | SDHCI_WAKE_ON_REMOVE
			| SDHCI_WAKE_ON_INT;

	val = sdhci_readb(host, SDHCI_WAKE_UP_CONTROL);
	val |= mask ;
	/* Avoid fake wake up */
	if (host->quirks & SDHCI_QUIRK_BROKEN_CARD_DETECTION)
		val &= ~(SDHCI_WAKE_ON_INSERT | SDHCI_WAKE_ON_REMOVE);
	sdhci_writeb(host, val, SDHCI_WAKE_UP_CONTROL);
}
EXPORT_SYMBOL_GPL(sdhci_enable_irq_wakeups);

void sdhci_disable_irq_wakeups(struct sdhci_host *host)
{
	u8 val;
	u8 mask = SDHCI_WAKE_ON_INSERT | SDHCI_WAKE_ON_REMOVE
			| SDHCI_WAKE_ON_INT;

	val = sdhci_readb(host, SDHCI_WAKE_UP_CONTROL);
	val &= ~mask;
	sdhci_writeb(host, val, SDHCI_WAKE_UP_CONTROL);
}
EXPORT_SYMBOL_GPL(sdhci_disable_irq_wakeups);

int sdhci_suspend_host(struct sdhci_host *host)
{
	int ret;

	if (host->ops->platform_suspend)
		host->ops->platform_suspend(host);

	sdhci_disable_card_detection(host);

	/* Disable tuning since we are suspending */
	if (host->flags & SDHCI_USING_RETUNING_TIMER) {
		del_timer_sync(&host->tuning_timer);
		host->flags &= ~SDHCI_NEEDS_RETUNING;
	}

	ret = mmc_suspend_host(host->mmc);
	if (ret) {
		if (host->flags & SDHCI_USING_RETUNING_TIMER) {
			host->flags |= SDHCI_NEEDS_RETUNING;
			mod_timer(&host->tuning_timer, jiffies +
					host->tuning_count * HZ);
		}

		sdhci_enable_card_detection(host);

		return ret;
	}

	if (!device_may_wakeup(mmc_dev(host->mmc))) {
		sdhci_mask_irqs(host, SDHCI_INT_ALL_MASK);
		free_irq(host->irq, host);
	} else {
		sdhci_enable_irq_wakeups(host);
		enable_irq_wake(host->irq);
	}
	return ret;
}

EXPORT_SYMBOL_GPL(sdhci_suspend_host);

int sdhci_resume_host(struct sdhci_host *host)
{
	int ret;

	if (host->flags & (SDHCI_USE_SDMA | SDHCI_USE_ADMA)) {
		if (host->ops->enable_dma)
			host->ops->enable_dma(host);
	}

	if (!device_may_wakeup(mmc_dev(host->mmc))) {
		ret = request_irq(host->irq, sdhci_irq, IRQF_SHARED,
				  mmc_hostname(host->mmc), host);
		if (ret)
			return ret;
	} else {
		sdhci_disable_irq_wakeups(host);
		disable_irq_wake(host->irq);
	}

	if ((host->mmc->pm_flags & MMC_PM_KEEP_POWER) &&
	    (host->quirks2 & SDHCI_QUIRK2_HOST_OFF_CARD_ON)) {
		/* Card keeps power but host controller does not */
		sdhci_init(host, 0);
		host->pwr = 0;
		host->clock = 0;
		sdhci_do_set_ios(host, &host->mmc->ios);
	} else {
		sdhci_init(host, (host->mmc->pm_flags & MMC_PM_KEEP_POWER));
		mmiowb();
	}

	ret = mmc_resume_host(host->mmc);
	sdhci_enable_card_detection(host);

	if (host->ops->platform_resume)
		host->ops->platform_resume(host);

	/* Set the re-tuning expiration flag */
	if (host->flags & SDHCI_USING_RETUNING_TIMER)
		host->flags |= SDHCI_NEEDS_RETUNING;

	return ret;
}

EXPORT_SYMBOL_GPL(sdhci_resume_host);
#endif /* CONFIG_PM */

#ifdef CONFIG_PM_RUNTIME

static int sdhci_runtime_pm_get(struct sdhci_host *host)
{
	if (!mmc_use_core_runtime_pm(host->mmc))
		return pm_runtime_get_sync(host->mmc->parent);
	else
		return 0;
}

static int sdhci_runtime_pm_put(struct sdhci_host *host)
{
	if (!mmc_use_core_runtime_pm(host->mmc)) {
		pm_runtime_mark_last_busy(host->mmc->parent);
		return pm_runtime_put_autosuspend(host->mmc->parent);
	} else {
		return 0;
	}
}

int sdhci_runtime_suspend_host(struct sdhci_host *host)
{
	unsigned long flags;
	int ret = 0;

	/* Disable tuning since we are suspending */
	if (host->flags & SDHCI_USING_RETUNING_TIMER) {
		del_timer_sync(&host->tuning_timer);
		host->flags &= ~SDHCI_NEEDS_RETUNING;
	}

	spin_lock_irqsave(&host->lock, flags);
	sdhci_mask_irqs(host, SDHCI_INT_ALL_MASK);
	spin_unlock_irqrestore(&host->lock, flags);

	synchronize_irq(host->irq);

	spin_lock_irqsave(&host->lock, flags);
	host->runtime_suspended = true;
	spin_unlock_irqrestore(&host->lock, flags);

	return ret;
}
EXPORT_SYMBOL_GPL(sdhci_runtime_suspend_host);

int sdhci_runtime_resume_host(struct sdhci_host *host)
{
	unsigned long flags;
	int ret = 0, host_flags = host->flags;

	if (host_flags & (SDHCI_USE_SDMA | SDHCI_USE_ADMA)) {
		if (host->ops->enable_dma)
			host->ops->enable_dma(host);
	}

	sdhci_init(host, 0);

	/* Force clock and power re-program */
	host->pwr = 0;
	host->clock = 0;
	sdhci_do_set_ios(host, &host->mmc->ios);

	sdhci_do_start_signal_voltage_switch(host, &host->mmc->ios);
	if ((host_flags & SDHCI_PV_ENABLED) &&
		!(host->quirks2 & SDHCI_QUIRK2_PRESET_VALUE_BROKEN)) {
		spin_lock_irqsave(&host->lock, flags);
		sdhci_enable_preset_value(host, true);
		spin_unlock_irqrestore(&host->lock, flags);
	}

	/* Set the re-tuning expiration flag */
	if (host->flags & SDHCI_USING_RETUNING_TIMER)
		host->flags |= SDHCI_NEEDS_RETUNING;

	spin_lock_irqsave(&host->lock, flags);

	host->runtime_suspended = false;

	/* Enable SDIO IRQ */
	if ((host->flags & SDHCI_SDIO_IRQ_ENABLED))
		sdhci_enable_sdio_irq_nolock(host, true);

	/* Enable Card Detection */
	sdhci_enable_card_detection(host);

	spin_unlock_irqrestore(&host->lock, flags);

	return ret;
}
EXPORT_SYMBOL_GPL(sdhci_runtime_resume_host);

#endif

/*****************************************************************************\
 *                                                                           *
 * Device allocation/registration                                            *
 *                                                                           *
\*****************************************************************************/

struct sdhci_host *sdhci_alloc_host(struct device *dev,
	size_t priv_size)
{
	struct mmc_host *mmc;
	struct sdhci_host *host;

	WARN_ON(dev == NULL);

	mmc = mmc_alloc_host(sizeof(struct sdhci_host) + priv_size, dev);
	if (!mmc)
		return ERR_PTR(-ENOMEM);

	host = mmc_priv(mmc);
	host->mmc = mmc;

	spin_lock_init(&host->lock);
	spin_lock_init(&host->next_lock);
	mutex_init(&host->ios_mutex);
	ratelimit_state_init(&host->dbg_dump_rs, SDHCI_DBG_DUMP_RS_INTERVAL,
			SDHCI_DBG_DUMP_RS_BURST);

	return host;
}

EXPORT_SYMBOL_GPL(sdhci_alloc_host);

#ifdef CONFIG_ARCH_DMA_ADDR_T_64BIT
static int sdhci_is_adma2_64bit(struct sdhci_host *host)
{
	u32 caps;

	caps = (host->quirks & SDHCI_QUIRK_MISSING_CAPS) ? host->caps :
		sdhci_readl(host, SDHCI_CAPABILITIES);

	if (caps & SDHCI_CAN_64BIT)
		return 1;
	return 0;
}
#else
static int sdhci_is_adma2_64bit(struct sdhci_host *host)
{
	return 0;
}
#endif

#ifdef CONFIG_MMC_CQ_HCI
static void sdhci_cmdq_set_transfer_params(struct mmc_host *mmc)
{
	struct sdhci_host *host = mmc_priv(mmc);
	u8 ctrl;

	if (host->version >= SDHCI_SPEC_200) {
		ctrl = sdhci_readb(host, SDHCI_HOST_CONTROL);
		ctrl &= ~SDHCI_CTRL_DMA_MASK;
		if (host->flags & SDHCI_USE_ADMA_64BIT)
			ctrl |= SDHCI_CTRL_ADMA64;
		else
			ctrl |= SDHCI_CTRL_ADMA32;
		sdhci_writeb(host, ctrl, SDHCI_HOST_CONTROL);
	}
	if (host->ops->toggle_cdr)
		host->ops->toggle_cdr(host, false);
}

static void sdhci_cmdq_clear_set_irqs(struct mmc_host *mmc, bool clear)
{
	struct sdhci_host *host = mmc_priv(mmc);
	u32 ier = 0;

	ier &= ~SDHCI_INT_ALL_MASK;

	if (clear) {
		ier = SDHCI_INT_CMDQ_EN | SDHCI_INT_ERROR_MASK;
		sdhci_writel(host, ier, SDHCI_INT_ENABLE);
		sdhci_writel(host, ier, SDHCI_SIGNAL_ENABLE);
	} else {
		ier = SDHCI_INT_BUS_POWER | SDHCI_INT_DATA_END_BIT |
			     SDHCI_INT_DATA_CRC | SDHCI_INT_DATA_TIMEOUT |
			     SDHCI_INT_INDEX | SDHCI_INT_END_BIT |
			     SDHCI_INT_CRC | SDHCI_INT_TIMEOUT |
			     SDHCI_INT_DATA_END | SDHCI_INT_RESPONSE |
			     SDHCI_INT_AUTO_CMD_ERR;
		sdhci_writel(host, ier, SDHCI_INT_ENABLE);
		sdhci_writel(host, ier, SDHCI_SIGNAL_ENABLE);
	}
}

static void sdhci_cmdq_set_data_timeout(struct mmc_host *mmc, u32 val)
{
	struct sdhci_host *host = mmc_priv(mmc);

	sdhci_writeb(host, val, SDHCI_TIMEOUT_CONTROL);
}

static void sdhci_cmdq_dump_vendor_regs(struct mmc_host *mmc)
{
	struct sdhci_host *host = mmc_priv(mmc);

	sdhci_dumpregs(host);
}

static int sdhci_cmdq_init(struct sdhci_host *host, struct mmc_host *mmc,
			   bool dma64)
{
	return cmdq_init(host->cq_host, mmc, dma64);
}

static void sdhci_cmdq_set_block_size(struct mmc_host *mmc)
{
	struct sdhci_host *host = mmc_priv(mmc);

	sdhci_set_blk_size_reg(host, 512, 0);
}

static void sdhci_enhanced_strobe_mask(struct mmc_host *mmc, bool set)
{
	struct sdhci_host *host = mmc_priv(mmc);

	if (host->ops->enhanced_strobe_mask)
		host->ops->enhanced_strobe_mask(host, set);
}

static void sdhci_cmdq_clear_set_dumpregs(struct mmc_host *mmc, bool set)
{
	struct sdhci_host *host = mmc_priv(mmc);

	if (host->ops->clear_set_dumpregs)
		host->ops->clear_set_dumpregs(host, set);
}
static int sdhci_cmdq_crypto_cfg(struct mmc_host *mmc,
		struct mmc_request *mrq, u32 slot)
{
	struct sdhci_host *host = mmc_priv(mmc);

	if (!host->is_crypto_en)
		return 0;

	return sdhci_crypto_cfg(host, mrq, slot);
}

static void sdhci_cmdq_crypto_cfg_reset(struct mmc_host *mmc, unsigned int slot)
{
	struct sdhci_host *host = mmc_priv(mmc);

	if (!host->is_crypto_en)
		return;

	if (host->ops->crypto_cfg_reset)
		host->ops->crypto_cfg_reset(host, slot);
}

static void sdhci_cmdq_post_cqe_halt(struct mmc_host *mmc)
{
	struct sdhci_host *host = mmc_priv(mmc);

	sdhci_writel(host, sdhci_readl(host, SDHCI_INT_ENABLE) |
			SDHCI_INT_RESPONSE, SDHCI_INT_ENABLE);
	sdhci_writel(host, SDHCI_INT_RESPONSE, SDHCI_INT_STATUS);
}

static void sdhci_cmdq_update_pm_qos(struct mmc_host *mmc,
		struct mmc_request *mrq, bool enable)
{

	return sdhci_update_pm_qos(mmc, mrq, enable);
}

#else
static void sdhci_cmdq_set_transfer_params(struct mmc_host *mmc)
{

}
static void sdhci_cmdq_clear_set_irqs(struct mmc_host *mmc, bool clear)
{

}

static void sdhci_cmdq_set_data_timeout(struct mmc_host *mmc, u32 val)
{

}

static void sdhci_cmdq_dump_vendor_regs(struct mmc_host *mmc)
{

}

static int sdhci_cmdq_init(struct sdhci_host *host, struct mmc_host *mmc,
			   bool dma64)
{
	return -ENOSYS;
}

static void sdhci_cmdq_set_block_size(struct mmc_host *mmc)
{

}

static void sdhci_enhanced_strobe_mask(struct mmc_host *mmc, bool set)
{

}

static void sdhci_cmdq_clear_set_dumpregs(struct mmc_host *mmc, bool set)
{

}
static int sdhci_cmdq_crypto_cfg(struct mmc_host *mmc,
		struct mmc_request *mrq, u32 slot)
{
	return 0;
}

static void sdhci_cmdq_crypto_cfg_reset(struct mmc_host *mmc, unsigned int slot)
{

}
static void sdhci_cmdq_post_cqe_halt(struct mmc_host *mmc)
{
}
static void sdhci_cmdq_update_pm_qos(struct mmc_host *mmc,
		struct mmc_request *mrq, bool enable)
{
}
#endif

static const struct cmdq_host_ops sdhci_cmdq_ops = {
	.clear_set_irqs = sdhci_cmdq_clear_set_irqs,
	.set_data_timeout = sdhci_cmdq_set_data_timeout,
	.dump_vendor_regs = sdhci_cmdq_dump_vendor_regs,
	.set_block_size = sdhci_cmdq_set_block_size,
	.clear_set_dumpregs = sdhci_cmdq_clear_set_dumpregs,
	.enhanced_strobe_mask = sdhci_enhanced_strobe_mask,
	.crypto_cfg	= sdhci_cmdq_crypto_cfg,
	.crypto_cfg_reset	= sdhci_cmdq_crypto_cfg_reset,
	.post_cqe_halt = sdhci_cmdq_post_cqe_halt,
	.pm_qos_update = sdhci_cmdq_update_pm_qos,
	.set_transfer_params = sdhci_cmdq_set_transfer_params,
};

int sdhci_add_host(struct sdhci_host *host)
{
	struct mmc_host *mmc;
	u32 caps[2] = {0, 0};
	u32 max_current_caps;
	unsigned int ocr_avail;
	int ret;

	WARN_ON(host == NULL);
	if (host == NULL)
		return -EINVAL;

	mmc = host->mmc;

	if (debug_quirks)
		host->quirks = debug_quirks;
	if (debug_quirks2)
		host->quirks2 = debug_quirks2;

	sdhci_reset(host, SDHCI_RESET_ALL);

	host->version = sdhci_readw(host, SDHCI_HOST_VERSION);
	host->version = (host->version & SDHCI_SPEC_VER_MASK)
				>> SDHCI_SPEC_VER_SHIFT;
	if (host->version > SDHCI_SPEC_300) {
		pr_err("%s: Unknown controller version (%d). "
			"You may experience problems.\n", mmc_hostname(mmc),
			host->version);
	}

	caps[0] = (host->quirks & SDHCI_QUIRK_MISSING_CAPS) ? host->caps :
		sdhci_readl(host, SDHCI_CAPABILITIES);

	if (host->version >= SDHCI_SPEC_300)
		caps[1] = (host->quirks & SDHCI_QUIRK_MISSING_CAPS) ?
			host->caps1 :
			sdhci_readl(host, SDHCI_CAPABILITIES_1);

	if (host->quirks & SDHCI_QUIRK_FORCE_DMA)
		host->flags |= SDHCI_USE_SDMA;
	else if (!(caps[0] & SDHCI_CAN_DO_SDMA))
		DBG("Controller doesn't have SDMA capability\n");
	else
		host->flags |= SDHCI_USE_SDMA;

	if ((host->quirks & SDHCI_QUIRK_BROKEN_DMA) &&
		(host->flags & SDHCI_USE_SDMA)) {
		DBG("Disabling DMA as it is marked broken\n");
		host->flags &= ~SDHCI_USE_SDMA;
	}

	if ((host->version >= SDHCI_SPEC_200) &&
		(caps[0] & SDHCI_CAN_DO_ADMA2)) {
		host->flags |= SDHCI_USE_ADMA;
		if (sdhci_is_adma2_64bit(host))
			host->flags |= SDHCI_USE_ADMA_64BIT;
	}

	if ((host->quirks & SDHCI_QUIRK_BROKEN_ADMA) &&
		(host->flags & SDHCI_USE_ADMA)) {
		DBG("Disabling ADMA as it is marked broken\n");
		host->flags &= ~SDHCI_USE_ADMA;
	}

	if (host->flags & (SDHCI_USE_SDMA | SDHCI_USE_ADMA)) {
		if (host->ops->enable_dma) {
			if (host->ops->enable_dma(host)) {
				pr_warning("%s: No suitable DMA "
					"available. Falling back to PIO.\n",
					mmc_hostname(mmc));
				host->flags &=
					~(SDHCI_USE_SDMA | SDHCI_USE_ADMA);
			}
		}
	}

	if (host->flags & SDHCI_USE_ADMA) {
		/*
		 * We need to allocate descriptors for all sg entries
		 * (128/max_segments) and potentially one alignment transfer for
		 * each of those entries.
		 */
		if (host->ops->get_max_segments)
			host->adma_max_desc = host->ops->get_max_segments();
		else
			host->adma_max_desc = 128;

		if (host->flags & SDHCI_USE_ADMA_64BIT) {
			host->adma_desc_line_sz = 12;
			host->align_bytes = 8;
		} else {
			host->adma_desc_line_sz = 8;
			host->align_bytes = 4;
		}
		host->adma_desc_sz = (host->adma_max_desc * 2 + 1) *
						host->adma_desc_line_sz;
		host->align_buf_sz = host->adma_max_desc * host->align_bytes;

		pr_debug("%s: %s: dma_desc_size: %d\n",
			mmc_hostname(host->mmc), __func__, host->adma_desc_sz);
		host->adma_desc = dma_alloc_coherent(mmc_dev(host->mmc),
						     host->adma_desc_sz,
						     &host->adma_addr,
						     GFP_KERNEL);
		host->align_buffer = dma_alloc_coherent(mmc_dev(host->mmc),
							host->align_buf_sz,
							&host->align_addr,
							GFP_KERNEL);
		if (!host->adma_desc || !host->align_buffer) {
			dma_free_coherent(mmc_dev(host->mmc),
					  host->adma_desc_sz,
					  host->adma_desc,
					  host->adma_addr);
			dma_free_coherent(mmc_dev(host->mmc),
					  host->align_buf_sz,
					  host->align_buffer,
					  host->align_addr);
			pr_warn("%s: Unable to allocate ADMA "
				"buffers. Falling back to standard DMA.\n",
				mmc_hostname(mmc));
			host->flags &= ~SDHCI_USE_ADMA;
			host->adma_desc = NULL;
			host->align_buffer = NULL;
		} else if ((host->adma_addr & (host->align_bytes - 1)) ||
			   (host->align_addr & (host->align_bytes - 1))) {
			dma_free_coherent(mmc_dev(host->mmc),
					  host->adma_desc_sz,
					  host->adma_desc,
					  host->adma_addr);
			dma_free_coherent(mmc_dev(host->mmc),
					  host->align_buf_sz,
					  host->align_buffer,
					  host->align_addr);
			pr_warn("%s: Unable to allocate aligned ADMA buffers.\n",
				   mmc_hostname(mmc));
			host->flags &= ~SDHCI_USE_ADMA;
			host->adma_desc = NULL;
			host->align_buffer = NULL;
		}
	}

	host->next_data.cookie = 1;

	/*
	 * If we use DMA, then it's up to the caller to set the DMA
	 * mask, but PIO does not need the hw shim so we set a new
	 * mask here in that case.
	 */
	if (!(host->flags & (SDHCI_USE_SDMA | SDHCI_USE_ADMA))) {
		host->dma_mask = DMA_BIT_MASK(64);
		mmc_dev(host->mmc)->dma_mask = &host->dma_mask;
	}

	if (host->version >= SDHCI_SPEC_300)
		host->max_clk = (caps[0] & SDHCI_CLOCK_V3_BASE_MASK)
			>> SDHCI_CLOCK_BASE_SHIFT;
	else
		host->max_clk = (caps[0] & SDHCI_CLOCK_BASE_MASK)
			>> SDHCI_CLOCK_BASE_SHIFT;

	host->max_clk *= 1000000;
	if (mmc->caps2 & MMC_CAP2_CLK_SCALE)
		sdhci_update_power_policy(host, SDHCI_PERFORMANCE_MODE_INIT);
	else
		sdhci_update_power_policy(host, SDHCI_PERFORMANCE_MODE);
	if (host->max_clk == 0 || host->quirks &
			SDHCI_QUIRK_CAP_CLOCK_BASE_BROKEN) {
		if (!host->ops->get_max_clock) {
			pr_err("%s: Hardware doesn't specify base clock "
			       "frequency.\n", mmc_hostname(mmc));
			return -ENODEV;
		}
		host->max_clk = host->ops->get_max_clock(host);
	}

	/*
	 * In case of Host Controller v3.00, find out whether clock
	 * multiplier is supported.
	 */
	host->clk_mul = (caps[1] & SDHCI_CLOCK_MUL_MASK) >>
			SDHCI_CLOCK_MUL_SHIFT;

	/*
	 * In case the value in Clock Multiplier is 0, then programmable
	 * clock mode is not supported, otherwise the actual clock
	 * multiplier is one more than the value of Clock Multiplier
	 * in the Capabilities Register.
	 */
	if (host->clk_mul)
		host->clk_mul += 1;

	/*
	 * Set host parameters.
	 */
	mmc->ops = &sdhci_ops;
	mmc->f_max = host->max_clk;
	if (host->ops->get_min_clock)
		mmc->f_min = host->ops->get_min_clock(host);
	else if (host->version >= SDHCI_SPEC_300) {
		if (host->clk_mul) {
			mmc->f_min = (host->max_clk * host->clk_mul) / 1024;
			mmc->f_max = host->max_clk * host->clk_mul;
		} else
			mmc->f_min = host->max_clk / SDHCI_MAX_DIV_SPEC_300;
	} else
		mmc->f_min = host->max_clk / SDHCI_MAX_DIV_SPEC_200;

	host->timeout_clk =
		(caps[0] & SDHCI_TIMEOUT_CLK_MASK) >> SDHCI_TIMEOUT_CLK_SHIFT;
	if (host->timeout_clk == 0) {
		if (host->ops->get_timeout_clock) {
			host->timeout_clk = host->ops->get_timeout_clock(host);
		} else if (!(host->quirks &
				SDHCI_QUIRK_DATA_TIMEOUT_USES_SDCLK)) {
			pr_err("%s: Hardware doesn't specify timeout clock "
			       "frequency.\n", mmc_hostname(mmc));
			return -ENODEV;
		}
	}
	if (caps[0] & SDHCI_TIMEOUT_CLK_UNIT)
		host->timeout_clk *= 1000;

	if (host->quirks & SDHCI_QUIRK_DATA_TIMEOUT_USES_SDCLK)
		host->timeout_clk = mmc->f_max / 1000;

	if (!(host->quirks2 & SDHCI_QUIRK2_USE_MAX_DISCARD_SIZE))
		mmc->max_discard_to = (1 << 27) / host->timeout_clk;

	mmc->caps |= MMC_CAP_SDIO_IRQ | MMC_CAP_ERASE | MMC_CAP_CMD23;

	if (host->quirks & SDHCI_QUIRK_MULTIBLOCK_READ_ACMD12)
		host->flags |= SDHCI_AUTO_CMD12;

	/* Auto-CMD23 stuff only works in ADMA or PIO. */
	if ((host->version >= SDHCI_SPEC_300) &&
	    ((host->flags & SDHCI_USE_ADMA) ||
	     !(host->flags & SDHCI_USE_SDMA))) {
		host->flags |= SDHCI_AUTO_CMD23;
		DBG("%s: Auto-CMD23 available\n", mmc_hostname(mmc));
	} else {
		DBG("%s: Auto-CMD23 unavailable\n", mmc_hostname(mmc));
	}

	/*
	 * A controller may support 8-bit width, but the board itself
	 * might not have the pins brought out.  Boards that support
	 * 8-bit width must set "mmc->caps |= MMC_CAP_8_BIT_DATA;" in
	 * their platform code before calling sdhci_add_host(), and we
	 * won't assume 8-bit width for hosts without that CAP.
	 */
	if (!(host->quirks & SDHCI_QUIRK_FORCE_1_BIT_DATA))
		mmc->caps |= MMC_CAP_4_BIT_DATA;

	if (host->quirks2 & SDHCI_QUIRK2_HOST_NO_CMD23)
		mmc->caps &= ~MMC_CAP_CMD23;

	if (caps[0] & SDHCI_CAN_DO_HISPD)
		mmc->caps |= MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED;

	/*
	 * Enable polling on when card detection is broken and no card detect
	 * gpio is present.
	 */
	if ((host->quirks & SDHCI_QUIRK_BROKEN_CARD_DETECTION) &&
	    !(host->mmc->caps & MMC_CAP_NONREMOVABLE) &&
	    (mmc_gpio_get_cd(host->mmc) < 0) &&
	    !(host->mmc->caps2 & MMC_CAP2_NONHOTPLUG))
		mmc->caps |= MMC_CAP_NEEDS_POLL;
	/* If vqmmc regulator and no 1.8V signalling, then there's no UHS */
	host->vqmmc = regulator_get(mmc_dev(mmc), "vqmmc");
	if (IS_ERR_OR_NULL(host->vqmmc)) {
		if (PTR_ERR(host->vqmmc) < 0) {
			pr_info("%s: no vqmmc regulator found\n",
				mmc_hostname(mmc));
			host->vqmmc = NULL;
		}
	} else {
		ret = regulator_enable(host->vqmmc);
		if (!regulator_is_supported_voltage(host->vqmmc, 1700000,
			1950000))
			caps[1] &= ~(SDHCI_SUPPORT_SDR104 |
					SDHCI_SUPPORT_SDR50 |
					SDHCI_SUPPORT_DDR50);
		if (ret) {
			pr_warn("%s: Failed to enable vqmmc regulator: %d\n",
				mmc_hostname(mmc), ret);
			host->vqmmc = NULL;
		}
	}

	if (host->quirks2 & SDHCI_QUIRK2_NO_1_8_V)
		caps[1] &= ~(SDHCI_SUPPORT_SDR104 | SDHCI_SUPPORT_SDR50 |
		       SDHCI_SUPPORT_DDR50);

	/* Any UHS-I mode in caps implies SDR12 and SDR25 support. */
	if (caps[1] & (SDHCI_SUPPORT_SDR104 | SDHCI_SUPPORT_SDR50 |
		       SDHCI_SUPPORT_DDR50))
		mmc->caps |= MMC_CAP_UHS_SDR12 | MMC_CAP_UHS_SDR25;

	/* SDR104 supports also implies SDR50 support */
	if (caps[1] & SDHCI_SUPPORT_SDR104)
		mmc->caps |= MMC_CAP_UHS_SDR104 | MMC_CAP_UHS_SDR50;
	else if (caps[1] & SDHCI_SUPPORT_SDR50)
		mmc->caps |= MMC_CAP_UHS_SDR50;

	if (caps[1] & SDHCI_SUPPORT_DDR50)
		mmc->caps |= MMC_CAP_UHS_DDR50;

	/* Does the host need tuning for SDR50? */
	if (caps[1] & SDHCI_USE_SDR50_TUNING)
		host->flags |= SDHCI_SDR50_NEEDS_TUNING;

	/* Does the host need tuning for HS200? */
	if (mmc->caps2 & MMC_CAP2_HS200)
		host->flags |= SDHCI_HS200_NEEDS_TUNING;

	/* Does the host need tuning for HS400? */
	if (mmc->caps2 & MMC_CAP2_HS400)
		host->flags |= SDHCI_HS400_NEEDS_TUNING;

	/* Driver Type(s) (A, C, D) supported by the host */
	if (caps[1] & SDHCI_DRIVER_TYPE_A)
		mmc->caps |= MMC_CAP_DRIVER_TYPE_A;
	if (caps[1] & SDHCI_DRIVER_TYPE_C)
		mmc->caps |= MMC_CAP_DRIVER_TYPE_C;
	if (caps[1] & SDHCI_DRIVER_TYPE_D)
		mmc->caps |= MMC_CAP_DRIVER_TYPE_D;

	/* Initial value for re-tuning timer count */
	host->tuning_count = (caps[1] & SDHCI_RETUNING_TIMER_COUNT_MASK) >>
			      SDHCI_RETUNING_TIMER_COUNT_SHIFT;

	/*
	 * In case Re-tuning Timer is not disabled, the actual value of
	 * re-tuning timer will be 2 ^ (n - 1).
	 */
	if (host->tuning_count)
		host->tuning_count = 1 << (host->tuning_count - 1);

	/* Re-tuning mode supported by the Host Controller */
	host->tuning_mode = (caps[1] & SDHCI_RETUNING_MODE_MASK) >>
			     SDHCI_RETUNING_MODE_SHIFT;

	ocr_avail = 0;

	host->vmmc = regulator_get(mmc_dev(mmc), "vmmc");
	if (IS_ERR_OR_NULL(host->vmmc)) {
		if (PTR_ERR(host->vmmc) < 0) {
			pr_info("%s: no vmmc regulator found\n",
				mmc_hostname(mmc));
			host->vmmc = NULL;
		}
	}

#ifdef CONFIG_REGULATOR
	/*
	 * Voltage range check makes sense only if regulator reports
	 * any voltage value.
	 */
	if (host->vmmc && regulator_get_voltage(host->vmmc) > 0) {
		ret = regulator_is_supported_voltage(host->vmmc, 2700000,
			3600000);
		if ((ret <= 0) || (!(caps[0] & SDHCI_CAN_VDD_330)))
			caps[0] &= ~SDHCI_CAN_VDD_330;
		if ((ret <= 0) || (!(caps[0] & SDHCI_CAN_VDD_300)))
			caps[0] &= ~SDHCI_CAN_VDD_300;
		ret = regulator_is_supported_voltage(host->vmmc, 1700000,
			1950000);
		if ((ret <= 0) || (!(caps[0] & SDHCI_CAN_VDD_180)))
			caps[0] &= ~SDHCI_CAN_VDD_180;
	}
#endif /* CONFIG_REGULATOR */

	/*
	 * According to SD Host Controller spec v3.00, if the Host System
	 * can afford more than 150mA, Host Driver should set XPC to 1. Also
	 * the value is meaningful only if Voltage Support in the Capabilities
	 * register is set. The actual current value is 4 times the register
	 * value.
	 */
	max_current_caps = sdhci_readl(host, SDHCI_MAX_CURRENT);
	if (!max_current_caps && host->vmmc) {
		u32 curr = regulator_get_current_limit(host->vmmc);
		if (curr > 0) {

			/* convert to SDHCI_MAX_CURRENT format */
			curr = curr/1000;  /* convert to mA */
			curr = curr/SDHCI_MAX_CURRENT_MULTIPLIER;

			curr = min_t(u32, curr, SDHCI_MAX_CURRENT_LIMIT);
			max_current_caps =
				(curr << SDHCI_MAX_CURRENT_330_SHIFT) |
				(curr << SDHCI_MAX_CURRENT_300_SHIFT) |
				(curr << SDHCI_MAX_CURRENT_180_SHIFT);
		}
	}

	if (caps[0] & SDHCI_CAN_VDD_330) {
		ocr_avail |= MMC_VDD_32_33 | MMC_VDD_33_34;

		mmc->max_current_330 = ((max_current_caps &
				   SDHCI_MAX_CURRENT_330_MASK) >>
				   SDHCI_MAX_CURRENT_330_SHIFT) *
				   SDHCI_MAX_CURRENT_MULTIPLIER;
	}
	if (caps[0] & SDHCI_CAN_VDD_300) {
		ocr_avail |= MMC_VDD_29_30 | MMC_VDD_30_31;

		mmc->max_current_300 = ((max_current_caps &
				   SDHCI_MAX_CURRENT_300_MASK) >>
				   SDHCI_MAX_CURRENT_300_SHIFT) *
				   SDHCI_MAX_CURRENT_MULTIPLIER;
	}
	if (caps[0] & SDHCI_CAN_VDD_180) {
		ocr_avail |= MMC_VDD_165_195;

		mmc->max_current_180 = ((max_current_caps &
				   SDHCI_MAX_CURRENT_180_MASK) >>
				   SDHCI_MAX_CURRENT_180_SHIFT) *
				   SDHCI_MAX_CURRENT_MULTIPLIER;
	}

	mmc->ocr_avail = ocr_avail;
	mmc->ocr_avail_sdio = ocr_avail;
	if (host->ocr_avail_sdio)
		mmc->ocr_avail_sdio &= host->ocr_avail_sdio;
	mmc->ocr_avail_sd = ocr_avail;
	if (host->ocr_avail_sd)
		mmc->ocr_avail_sd &= host->ocr_avail_sd;
	else /* normal SD controllers don't support 1.8V */
		mmc->ocr_avail_sd &= ~MMC_VDD_165_195;
	mmc->ocr_avail_mmc = ocr_avail;
	if (host->ocr_avail_mmc)
		mmc->ocr_avail_mmc &= host->ocr_avail_mmc;

	if (mmc->ocr_avail == 0) {
		pr_err("%s: Hardware doesn't report any "
			"support voltages.\n", mmc_hostname(mmc));
		return -ENODEV;
	}

	/*
	 * Maximum number of segments. Depends on if the hardware
	 * can do scatter/gather or not.
	 */
	if (host->flags & SDHCI_USE_ADMA)
		mmc->max_segs = host->adma_max_desc;
	else if (host->flags & SDHCI_USE_SDMA)
		mmc->max_segs = 1;
	else/* PIO */
		mmc->max_segs = 128;

	/*
	 * Maximum number of sectors in one transfer. Limited by DMA boundary
	 * size (512KiB), unless specified by platform specific driver. Each
	 * descriptor can transfer a maximum of 64KB.
	 */
	if (host->flags & SDHCI_USE_ADMA)
		mmc->max_req_size = (host->adma_max_desc * 65536);
	else
		mmc->max_req_size = 524288;

	/*
	 * Maximum segment size. Could be one segment with the maximum number
	 * of bytes. When doing hardware scatter/gather, each entry cannot
	 * be larger than 64 KiB though.
	 */
	if (host->flags & SDHCI_USE_ADMA) {
		if (host->quirks & SDHCI_QUIRK_BROKEN_ADMA_ZEROLEN_DESC)
			mmc->max_seg_size = 65535;
		else
			mmc->max_seg_size = 65536;
	} else {
		mmc->max_seg_size = mmc->max_req_size;
	}

	/*
	 * Maximum block size. This varies from controller to controller and
	 * is specified in the capabilities register.
	 */
	if (host->quirks & SDHCI_QUIRK_FORCE_BLK_SZ_2048) {
		mmc->max_blk_size = 2;
	} else {
		mmc->max_blk_size = (caps[0] & SDHCI_MAX_BLOCK_MASK) >>
				SDHCI_MAX_BLOCK_SHIFT;
		if (mmc->max_blk_size >= 3) {
			pr_warning("%s: Invalid maximum block size, "
				"assuming 512 bytes\n", mmc_hostname(mmc));
			mmc->max_blk_size = 0;
		}
	}

	mmc->max_blk_size = 512 << mmc->max_blk_size;

	/*
	 * Maximum block count.
	 */
	mmc->max_blk_count = (host->quirks & SDHCI_QUIRK_NO_MULTIBLOCK) ? 1 : 65535;

	/*
	 * Init tasklets.
	 */
	tasklet_init(&host->card_tasklet,
		sdhci_tasklet_card, (unsigned long)host);
	tasklet_init(&host->finish_tasklet,
		sdhci_tasklet_finish, (unsigned long)host);

	setup_timer(&host->timer, sdhci_timeout_timer, (unsigned long)host);

	INIT_DELAYED_WORK(&host->pm_qos_work, sdhci_pm_qos_remove_work);
	if (host->version >= SDHCI_SPEC_300) {
		init_waitqueue_head(&host->buf_ready_int);

		/* Initialize re-tuning timer */
		init_timer(&host->tuning_timer);
		host->tuning_timer.data = (unsigned long)host;
		host->tuning_timer.function = sdhci_tuning_timer;
	}

	ret = request_irq(host->irq, sdhci_irq, IRQF_SHARED,
		mmc_hostname(mmc), host);
	if (ret) {
		pr_err("%s: Failed to request IRQ %d: %d\n",
		       mmc_hostname(mmc), host->irq, ret);
		goto untasklet;
	}

	host->irq_enabled = true;

	sdhci_init(host, 0);

#ifdef CONFIG_MMC_DEBUG
	sdhci_dumpregs(host);
#endif

#ifdef SDHCI_USE_LEDS_CLASS
	if (!(host->quirks2 & SDHCI_QUIRK2_BROKEN_LED_CONTROL)) {
		snprintf(host->led_name, sizeof(host->led_name),
			"%s::", mmc_hostname(mmc));
		host->led.name = host->led_name;
		host->led.brightness = LED_OFF;
		host->led.default_trigger = mmc_hostname(mmc);
		host->led.brightness_set = sdhci_led_control;

		ret = led_classdev_register(mmc_dev(mmc), &host->led);
		if (ret) {
			pr_err("%s: Failed to register LED device: %d\n",
			       mmc_hostname(mmc), ret);
			goto reset;
		}
	}
#endif

	mmiowb();
	if (host->host_qos[SDHCI_QOS_READ_WRITE].cpu_dma_latency_us) {
		host->pm_qos_timeout_us = SDHCI_PM_QOS_DEFAULT_DELAY;
		if (host->host_use_default_qos ||
				(mmc->caps2 &  MMC_CAP2_CMD_QUEUE)) {
			pm_qos_add_request(
			&(host->host_qos[SDHCI_QOS_READ_WRITE].pm_qos_req_dma),
				PM_QOS_CPU_DMA_LATENCY,
				PM_QOS_DEFAULT_VALUE);
		}
		if (!host->host_use_default_qos) {
			pm_qos_add_request(
			    &(host->host_qos[SDHCI_QOS_READ].pm_qos_req_dma),
			    PM_QOS_CPU_DMA_LATENCY,
			    PM_QOS_DEFAULT_VALUE);
			pm_qos_add_request(
			    &(host->host_qos[SDHCI_QOS_WRITE].pm_qos_req_dma),
			    PM_QOS_CPU_DMA_LATENCY,
			    PM_QOS_DEFAULT_VALUE);
		}
		host->last_qos_policy = -EINVAL;
		host->pm_qos_tout.show = show_sdhci_pm_qos_tout;
		host->pm_qos_tout.store = store_sdhci_pm_qos_tout;
		sysfs_attr_init(&host->pm_qos_tout.attr);
		host->pm_qos_tout.attr.name = "pm_qos_unvote_delay";
		host->pm_qos_tout.attr.mode = S_IRUGO | S_IWUSR;
		ret = device_create_file(mmc_dev(mmc), &host->pm_qos_tout);
		if (ret)
			pr_err("%s: cannot create pm_qos_unvote_delay %d\n",
					mmc_hostname(mmc), ret);
		mutex_init(&host->qos_lock);
	}
	if (caps[0] & SDHCI_ASYNC_INTR)
		host->async_int_supp = true;

	if (host->quirks2 & SDHCI_QUIRK2_IGN_DATA_END_BIT_ERROR)
		sdhci_clear_set_irqs(host, SDHCI_INT_DATA_END_BIT, 0);

	if (mmc->caps2 &  MMC_CAP2_CMD_QUEUE) {
		bool dma64 = (host->flags & SDHCI_USE_ADMA_64BIT) ?
			true : false;
		ret = sdhci_cmdq_init(host, mmc, dma64);
		if (ret)
			pr_err("%s: CMDQ init: failed (%d)\n",
			       mmc_hostname(host->mmc), ret);
		else
			host->cq_host->ops = &sdhci_cmdq_ops;
	}
	pr_info("%s: SDHCI controller on %s [%s] using %s in %s mode\n",
		mmc_hostname(mmc), host->hw_name, dev_name(mmc_dev(mmc)),
		(host->flags & SDHCI_USE_ADMA) ?
		((host->flags & SDHCI_USE_ADMA_64BIT) ?
		"64-bit ADMA" : "32-bit ADMA") :
		((host->flags & SDHCI_USE_SDMA) ? "DMA" : "PIO"),
		((mmc->caps2 &  MMC_CAP2_CMD_QUEUE) && !ret) ?
		"CMDQ" : "legacy");

	sdhci_enable_card_detection(host);

	mmc_add_host(mmc);
	return 0;

#ifdef SDHCI_USE_LEDS_CLASS
reset:
	sdhci_reset(host, SDHCI_RESET_ALL);
	sdhci_mask_irqs(host, SDHCI_INT_ALL_MASK);
	free_irq(host->irq, host);
#endif
untasklet:
	tasklet_kill(&host->card_tasklet);
	tasklet_kill(&host->finish_tasklet);

	return ret;
}

EXPORT_SYMBOL_GPL(sdhci_add_host);

void sdhci_remove_host(struct sdhci_host *host, int dead)
{
	unsigned long flags;

	if (dead) {
		spin_lock_irqsave(&host->lock, flags);

		host->flags |= SDHCI_DEVICE_DEAD;

		if (host->mrq) {
			pr_err("%s: Controller removed during "
				" transfer!\n", mmc_hostname(host->mmc));

			host->mrq->cmd->error = -ENOMEDIUM;
			tasklet_schedule(&host->finish_tasklet);
		}

		spin_unlock_irqrestore(&host->lock, flags);
	}

	sdhci_update_power_policy(host, SDHCI_POWER_SAVE_MODE);
	sdhci_disable_card_detection(host);
	if (host->host_qos[SDHCI_QOS_READ_WRITE].cpu_dma_latency_us) {
		if (host->host_use_default_qos ||
				(host->mmc->caps2 &  MMC_CAP2_CMD_QUEUE))
			pm_qos_remove_request(
			&(host->host_qos[SDHCI_QOS_READ_WRITE].pm_qos_req_dma));

		if (!host->host_use_default_qos) {
			pm_qos_remove_request(
			&(host->host_qos[SDHCI_QOS_READ].pm_qos_req_dma));
			pm_qos_remove_request(
			&(host->host_qos[SDHCI_QOS_WRITE].pm_qos_req_dma));
		}
	}
	mmc_remove_host(host->mmc);

#ifdef SDHCI_USE_LEDS_CLASS
	if (!(host->quirks2 & SDHCI_QUIRK2_BROKEN_LED_CONTROL))
		led_classdev_unregister(&host->led);
#endif

	if (!dead)
		sdhci_reset(host, SDHCI_RESET_ALL);

	sdhci_mask_irqs(host, SDHCI_INT_ALL_MASK);
	free_irq(host->irq, host);

	del_timer_sync(&host->timer);

	tasklet_kill(&host->card_tasklet);
	tasklet_kill(&host->finish_tasklet);

	if (host->vmmc) {
		regulator_disable(host->vmmc);
		regulator_put(host->vmmc);
	}

	if (host->vqmmc) {
		regulator_disable(host->vqmmc);
		regulator_put(host->vqmmc);
	}

	if (host->adma_desc)
		dma_free_coherent(mmc_dev(host->mmc), host->adma_desc_sz,
				  host->adma_desc, host->adma_addr);
	if (host->align_buffer)
		dma_free_coherent(mmc_dev(host->mmc), host->align_buf_sz,
				  host->align_buffer, host->align_addr);

	host->adma_desc = NULL;
	host->align_buffer = NULL;
}

EXPORT_SYMBOL_GPL(sdhci_remove_host);

void sdhci_free_host(struct sdhci_host *host)
{
	mmc_free_host(host->mmc);
}

EXPORT_SYMBOL_GPL(sdhci_free_host);

/*****************************************************************************\
 *                                                                           *
 * Driver init/exit                                                          *
 *                                                                           *
\*****************************************************************************/

static int __init sdhci_drv_init(void)
{
	pr_info(DRIVER_NAME
		": Secure Digital Host Controller Interface driver\n");
	pr_info(DRIVER_NAME ": Copyright(c) Pierre Ossman\n");

	return 0;
}

static void __exit sdhci_drv_exit(void)
{
}

module_init(sdhci_drv_init);
module_exit(sdhci_drv_exit);

module_param(debug_quirks, uint, 0444);
module_param(debug_quirks2, uint, 0444);

MODULE_AUTHOR("Pierre Ossman <pierre@ossman.eu>");
MODULE_DESCRIPTION("Secure Digital Host Controller Interface core driver");
MODULE_LICENSE("GPL");

MODULE_PARM_DESC(debug_quirks, "Force certain quirks.");
MODULE_PARM_DESC(debug_quirks2, "Force certain other quirks.");
