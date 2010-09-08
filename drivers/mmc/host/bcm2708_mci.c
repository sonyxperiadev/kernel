/*
 *  linux/drivers/mmc/host/bcm2708_mci.c - Broadcom BCM2708 MCI driver
 *
 *  Copyright (C) 2010 Broadcom, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/slab.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/highmem.h>
#include <linux/log2.h>
#include <linux/mmc/host.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/sd.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/scatterlist.h>
#include <linux/dma-mapping.h>

#include <asm/cacheflush.h>
#include <asm/div64.h>
#include <asm/io.h>
#include <asm/sizes.h>
//#include <asm/mach/mmc.h>

#include "bcm2708_mci.h"

#define DRIVER_NAME "bcm2708_mci"

#define DBG(host,fmt,args...)	\
	pr_debug("%s: %s: " fmt, mmc_hostname(host->mmc), __func__ , args)

//#define USE_DMA

#define BCM2708_DMA_ACTIVE	(1 << 0)
#define BCM2708_DMA_INT		(1 << 2)

#define BCM2708_DMA_INT_EN	(1 << 0)
#define BCM2708_DMA_D_INC	(1 << 4)
#define BCM2708_DMA_D_WIDTH	(1 << 5)
#define BCM2708_DMA_D_DREQ	(1 << 6)
#define BCM2708_DMA_S_INC	(1 << 8)
#define BCM2708_DMA_S_WIDTH	(1 << 9)
#define BCM2708_DMA_S_DREQ	(1 << 10)

#define	BCM2708_DMA_PER_MAP(x)	((x) << 16)

#define BCM2708_DMA_DREQ_SDHOST	13

#define BCM2708_DMA4_CS		0x400
#define BCM2708_DMA4_ADDR	0x404

static void do_command(void __iomem *base, u32 c, u32 a)
{
	writel(a, base + BCM2708_MCI_ARGUMENT);
	writel(c | BCM2708_MCI_ENABLE, base + BCM2708_MCI_COMMAND);

	while (readl(base + BCM2708_MCI_COMMAND) & BCM2708_MCI_ENABLE);
}

static void discard_words(void __iomem *base, int words)
{
	int i;
	for (i = 0; i < words; i++) {
		while (!(readl(base + BCM2708_MCI_STATUS) & BCM2708_MCI_DATAFLAG));
		readl(base + BCM2708_MCI_DATA);
	}
}

#define CACHE_LINE_MASK 31

static int suitable_for_dma(struct scatterlist *sg_ptr, int sg_len)
{
	int i;

	for (i = 0; i < sg_len; i++) {
		if (sg_ptr[i].offset & CACHE_LINE_MASK || sg_ptr[i].length & CACHE_LINE_MASK)
			return 0;
	}

	return 1;
}

static void
bcm2708_mci_start_command(struct bcm2708_mci_host *host, struct mmc_command *cmd, struct mmc_data *data)
{
	void __iomem *mmc_base = host->mmc_base;
	void __iomem *dma_base = host->dma_base;
	u32 status;
	u32 c;

	DBG(host, "op %02x arg %08x flags %08x\n",
	    cmd->opcode, cmd->arg, cmd->flags);

	int redo = 0;
back:

	/*
	 * clear the controller status register
	 */

	writel(-1, mmc_base + BCM2708_MCI_STATUS);

	/*
	 * build the command register write, incorporating no
         * response, long response, busy, read and write flags
	 */

	c = cmd->opcode;
	if (cmd->flags & MMC_RSP_PRESENT) {
		if (cmd->flags & MMC_RSP_136)
			c |= BCM2708_MCI_LONGRESP;
	} else
		c |= BCM2708_MCI_NORESP;
	if (cmd->flags & MMC_RSP_BUSY)
		c |= BCM2708_MCI_BUSY;
	if (data) {
		if (data->flags & MMC_DATA_READ)
			c |= BCM2708_MCI_READ;
		else
			c |= BCM2708_MCI_WRITE;
	}

	/*
         * run the command and wait for it to complete
	 */

	do_command(mmc_base, c, cmd->arg);

	/*
	 * retrieve the response and error (if any)
	 */

	status = readl(mmc_base + BCM2708_MCI_STATUS);

	if (cmd->flags & MMC_RSP_136) {
		cmd->resp[3] = readl(mmc_base + BCM2708_MCI_RESPONSE0);
		cmd->resp[2] = readl(mmc_base + BCM2708_MCI_RESPONSE1);
		cmd->resp[1] = readl(mmc_base + BCM2708_MCI_RESPONSE2);
		cmd->resp[0] = readl(mmc_base + BCM2708_MCI_RESPONSE3);
//		printk("%08x:%08x:%08x:%08x %08x\n", cmd->resp[3], cmd->resp[2], cmd->resp[1], cmd->resp[0], status);
	} else {
		cmd->resp[0] = readl(mmc_base + BCM2708_MCI_RESPONSE0);
//		printk("%08x %08x\n", cmd->resp[0], status);
	}

	if (status & BCM2708_MCI_CMDTIMEOUT) {
		printk(KERN_ERR "mmc driver saw timeout with opcode = %d, data = 0x%08x, timeout = %d", cmd->opcode, data, readl(mmc_base + BCM2708_MCI_TIMEOUT));
		if (data)
			printk(" data->sg_len = %d\n", data->sg_len);
		else
			printk("\n");
		if (!redo) {
			printk("redo\n");
			redo = 1;
			goto back;
		} else
			cmd->error = -ETIMEDOUT;
	}

	/*
	 * pump data if necessary
	 */

	if (data) {
		unsigned int sg_len = data->sg_len;
		struct scatterlist *sg_ptr = data->sg;

		data->bytes_xfered = 0;

#ifdef USE_DMA
		if (suitable_for_dma(sg_ptr, sg_len)) {
			int i, count = dma_map_sg(&host->dev->dev, sg_ptr, sg_len, data->flags & MMC_DATA_READ ? DMA_FROM_DEVICE : DMA_TO_DEVICE);

			for (i = 0; i < count; i++) {
				BCM2708_DMA_CB_T *cb = &host->cb_base[i];

				if (data->flags & MMC_DATA_READ) {
					cb->info = BCM2708_DMA_PER_MAP(BCM2708_DMA_DREQ_SDHOST)|BCM2708_DMA_S_DREQ|BCM2708_DMA_D_WIDTH|BCM2708_DMA_D_INC;
					cb->src = 0x7e202040;
					cb->dst = sg_dma_address(&sg_ptr[i]);
				} else {
					cb->info = BCM2708_DMA_PER_MAP(BCM2708_DMA_DREQ_SDHOST)|BCM2708_DMA_S_WIDTH|BCM2708_DMA_S_INC|BCM2708_DMA_D_DREQ;
					cb->src = sg_dma_address(&sg_ptr[i]);
					cb->dst = 0x7e202040;
				}
 
				cb->length = sg_dma_len(&sg_ptr[i]);
				cb->stride = 0;

				if (i == count - 1) {
					cb->info |= BCM2708_DMA_INT_EN;
					cb->next = 0;
				} else 
					cb->next = host->cb_handle + (i + 1) * sizeof(BCM2708_DMA_CB_T);

				cb->pad[0] = 0;
				cb->pad[1] = 0;

				data->bytes_xfered += sg_ptr[i].length;
			}

			dsb();	// data barrier operation

			writel(host->cb_handle, dma_base + BCM2708_DMA4_ADDR);
			writel(BCM2708_DMA_ACTIVE, dma_base + BCM2708_DMA4_CS);

			down(&host->sem);

//			while ((readl(dma_base + BCM2708_DMA4_CS) & BCM2708_DMA_ACTIVE));

			dma_unmap_sg(&host->dev->dev, sg_ptr, sg_len, data->flags & MMC_DATA_READ ? DMA_FROM_DEVICE : DMA_TO_DEVICE);
		} else
#endif
		while (sg_len) {
			unsigned long flags;

			/*
			 * map the current scatter buffer
			 */

			char *buffer = bcm2708_mci_kmap_atomic(sg_ptr, &flags);

			/*
			 * pump the data
			 */

			u32 *ptr = (u32 *)(buffer);
			u32 *lim = (u32 *)(buffer + sg_ptr->length);

			while (ptr < lim) {
				while (!(readl(mmc_base + BCM2708_MCI_STATUS) & BCM2708_MCI_DATAFLAG));

				if (data->flags & MMC_DATA_READ)
					*ptr++ = readl(mmc_base + BCM2708_MCI_DATA);
				else
					writel(*ptr++, mmc_base + BCM2708_MCI_DATA);
			}

			/*
			 * unmap the buffer
			 */

			bcm2708_mci_kunmap_atomic(buffer, &flags);

			/*
			 * if we were reading, and we have completed this
			 * page, ensure that the data cache is coherent
			 */

			if (data->flags & MMC_DATA_READ)
				flush_dcache_page(sg_page(sg_ptr));

			data->bytes_xfered += sg_ptr->length;

			sg_ptr++;
			sg_len--; 
		}

		if (host->is_acmd && cmd->opcode == SD_APP_SEND_SCR)
			discard_words(mmc_base, 126);
		if (host->is_acmd && cmd->opcode == SD_APP_SEND_NUM_WR_BLKS) 
			discard_words(mmc_base, 127);
		if (!host->is_acmd && cmd->opcode == SD_SWITCH)
			discard_words(mmc_base, 112);

		if (data->stop)
			bcm2708_mci_start_command(host, data->stop, 0);
	}

	/*
	 * remember if we're an application command
	 */

	host->is_acmd = cmd->opcode == MMC_APP_CMD;
}

static void bcm2708_mci_request(struct mmc_host *mmc, struct mmc_request *mrq)
{
	struct bcm2708_mci_host *host = mmc_priv(mmc);

	if (mrq->data && !is_power_of_2(mrq->data->blksz)) {
		printk(KERN_ERR "%s: Unsupported block size (%d bytes)\n",
			mmc_hostname(mmc), mrq->data->blksz);
		mrq->cmd->error = -EINVAL;
		mmc_request_done(mmc, mrq);
		return;
	}

	bcm2708_mci_start_command(host, mrq->cmd, mrq->data);

	mmc_request_done(host->mmc, mrq);
}

static void bcm2708_mci_set_ios(struct mmc_host *mmc, struct mmc_ios *ios)
{
}

/*
 * Handle completion of command and data transfers.
 */

static irqreturn_t bcm2708_mci_command_irq(int irq, void *dev_id)
{
	struct bcm2708_mci_host *host = dev_id;

	writel(BCM2708_DMA_INT, host->dma_base + BCM2708_DMA4_CS);

	printk(KERN_ERR "irq\n");

	return IRQ_RETVAL(0);
}

static irqreturn_t bcm2708_mci_data_irq(int irq, void *dev_id)
{
	struct bcm2708_mci_host *host = dev_id;

	writel(BCM2708_DMA_INT, host->dma_base + BCM2708_DMA4_CS);

	dsb();

	up(&host->sem);

	return IRQ_RETVAL(0);
}

static const struct mmc_host_ops bcm2708_mci_ops = {
	.request	= bcm2708_mci_request,
	.set_ios	= bcm2708_mci_set_ios,
};

static int __devinit bcm2708_mci_probe(struct platform_device *pdev)
{
	struct mmc_host *mmc;
	struct bcm2708_mci_host *host;
	struct resource *mmc_res;
	struct resource *dma_res;
	struct resource *dat_res;
	struct resource *cmd_res;
	int ret;

	mmc = mmc_alloc_host(sizeof(struct bcm2708_mci_host), &pdev->dev);
	if (!mmc) {
		ret = -ENOMEM;
		dev_dbg(&pdev->dev, "couldn't allocate mmc host\n");
		goto fail0;
	}

	host = mmc_priv(mmc);
	host->mmc = mmc;

	host->dev = pdev;

	sema_init(&host->sem, 0);

	host->cb_base = dma_alloc_writecombine(&pdev->dev, SZ_4K, &host->cb_handle, GFP_KERNEL);
	if (!host->cb_base) {
		ret = -ENOMEM;
		goto fail1;
	}

	mmc_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!mmc_res) {
		ret = -ENXIO;
		goto fail2;
	}

	if (!request_mem_region(mmc_res->start, mmc_res->end - mmc_res->start + 1, DRIVER_NAME)) {
		ret = -EBUSY;
		goto fail2;
	}

	dma_res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (!dma_res) {
		ret = -ENXIO;
		goto fail3;
	}

	if (!request_mem_region(dma_res->start, dma_res->end - dma_res->start + 1, DRIVER_NAME)) {
		ret = -EBUSY;
		goto fail3;
	}

	/*
	 * Map I/O regions
	 */

	host->mmc_base = ioremap(mmc_res->start, resource_size(mmc_res));
	if (!host->mmc_base) {
		ret = -ENOMEM;
		goto fail4;
	}

	host->dma_base = ioremap(dma_res->start, resource_size(dma_res));
	if (!host->dma_base) {
		ret = -ENOMEM;
		goto fail5;
	}

	/*
	 * Grab interrupts.
	 */

	dat_res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!dat_res) {
		ret = -ENXIO;
		goto fail6;
	}

	ret = request_irq(dat_res->start, bcm2708_mci_data_irq, IRQF_SHARED, DRIVER_NAME " (dat)", host);
	if (ret) {
		goto fail6;
	}
#if 0
	cmd_res = platform_get_resource(pdev, IORESOURCE_IRQ, 1);
	if (!cmd_res) {
		ret = -ENXIO;
		goto fail6;
	}

	ret = request_irq(cmd_res->start, bcm2708_mci_command_irq, IRQF_SHARED, DRIVER_NAME " (cmd)", host);
	if (ret) {
		goto fail6;
	}
#endif
	host->is_acmd = 0;

	mmc->ops = &bcm2708_mci_ops;
	mmc->f_min = 375000;
	mmc->f_max = 25000000;
	mmc->ocr_avail = MMC_VDD_32_33 | MMC_VDD_33_34;

	/*
	 * We can do SGIO
	 */
	mmc->max_hw_segs = 16;
	mmc->max_phys_segs = NR_SG;

	/*
	 * Since we only have a 16-bit data length register, we must
	 * ensure that we don't exceed 2^16-1 bytes in a single request.
	 */
	mmc->max_req_size = 65535;

	/*
	 * Set the maximum segment size.  Since we aren't doing DMA
	 * (yet) we are only limited by the data length register.
	 */
	mmc->max_seg_size = mmc->max_req_size;

	/*
	 * Block size can be up to 2048 bytes, but must be a power of two.
	 */
	mmc->max_blk_size = 2048;

	/*
	 * No limit on the number of blocks transferred.
	 */
	mmc->max_blk_count = mmc->max_req_size;

	/*
	 * We support 4-bit data (at least on the DB)
	 */

	mmc->caps |= MMC_CAP_4_BIT_DATA;

	mmc_add_host(mmc);

	printk(KERN_INFO "%s: BCM2708 SD host at 0x%08llx 0x%08llx\n",
		mmc_hostname(mmc),
		(unsigned long long)mmc_res->start, (unsigned long long)dma_res->start);

	return 0;

fail6:
	iounmap(host->dma_base);
fail5:
	iounmap(host->mmc_base);
fail4:
	release_mem_region(dma_res->start, dma_res->end - dma_res->start + 1);
fail3:
	release_mem_region(mmc_res->start, mmc_res->end - mmc_res->start + 1);
fail2:
	dma_free_writecombine(&pdev->dev, SZ_4K, host->cb_base, host->cb_handle);
fail1:
	mmc_free_host(mmc);
fail0:
	dev_err(&pdev->dev, "probe failed, err %d\n", ret);
	return ret;
}

static int __devexit bcm2708_mci_remove(struct platform_device *pdev)
{
	struct mmc_host *mmc = platform_get_drvdata(pdev);

	if (mmc) {
		struct bcm2708_mci_host *host = mmc_priv(mmc);
		struct resource *res;

		mmc_remove_host(mmc);

		iounmap(host->mmc_base);
		iounmap(host->dma_base);

		res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
		release_mem_region(res->start, resource_size(res));
		res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
		release_mem_region(res->start, resource_size(res));

		dma_free_writecombine(&pdev->dev, SZ_4K, host->cb_base, host->cb_handle);

		mmc_free_host(mmc);
		platform_set_drvdata(pdev, NULL);

		return 0;
	} else
		return -1;
}

#ifdef CONFIG_PM
static int bcm2708_mci_suspend(struct platform_device *dev, pm_message_t state)
{
	struct mmc_host *mmc = platform_get_drvdata(dev);
	int ret = 0;

	if (mmc) {
		ret = mmc_suspend_host(mmc, state);
	}

	return ret;
}

static int bcm2708_mci_resume(struct platform_device *dev)
{
	struct mmc_host *mmc = platform_get_drvdata(dev);
	int ret = 0;

	if (mmc) {
		ret = mmc_resume_host(mmc);
	}

	return ret;
}
#else
#define bcm2708_mci_suspend	NULL
#define bcm2708_mci_resume	NULL
#endif

static struct platform_driver bcm2708_mci_driver = {
	.probe		= bcm2708_mci_probe,
	.remove		= bcm2708_mci_remove,
	.suspend	= bcm2708_mci_suspend,
	.resume		= bcm2708_mci_resume,
	.driver		= {
		.name	= DRIVER_NAME,
		.owner  = THIS_MODULE,
	},
};

static int __init bcm2708_mci_init(void)
{
	return platform_driver_register(&bcm2708_mci_driver);
}

static void __exit bcm2708_mci_exit(void)
{
	platform_driver_unregister(&bcm2708_mci_driver);
}

module_init(bcm2708_mci_init);
module_exit(bcm2708_mci_exit);

MODULE_DESCRIPTION("BCM2708 Multimedia Card Interface driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:bcm2708_mci");
